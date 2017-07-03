

/*
 * MccfCdrPackageResponse.cpp
 *
 *  Created on: Dec 17, 2012
 *      Author: sshafrir
 */

#include "MccfCdrPackageResponse.h"
#include "OpcodesMcmsInternal.h"
#include "Segment.h"
#include "TaskApi.h"
#include "IpCommonUtilTrace.h"
#include "TraceStream.h"
#include <string>
//#include "DialogTerminate.h"
//#include "MscPolycomMixer.h"


void CMccfCdrPackageResponse::ResponseReportMsg(const COsQueue clientRspMbx, CPrtMontrBaseParams* pIpVideoCdrChannelMonitor[],
	CIpChannelDetails* m_pIpVideoCdrChannelDetails[],/*std::string partyConnection, const char* strConfName,*/
	const char* confId, const char* partyId )
{

	//build response report msg: very important not to change the order of the following lines (according to xml)!!!
	int numberOfChannels = 0;

	if (pIpVideoCdrChannelMonitor[0]->GetConnectionStatus() == YES) //video
		numberOfChannels++;
	if (pIpVideoCdrChannelMonitor[1]->GetConnectionStatus() == YES) //content
		numberOfChannels++;

	if (numberOfChannels == 0)
	{
		FPTRACE(eLevelInfoNormal," CMccfCdrPackageResponse::ResponseReportMsg - numberOfChannels == 0 => don't send mscPolycomMixer msg to DMA");
		return;
	}

	FPTRACE(eLevelInfoNormal," CMccfCdrPackageResponse::ResponseReportMsg");

	MscPolycomMixer mscPolycomMixer;
	mscPolycomMixer.m_version = "1.0";

	EventElement eventElement;
	ParticipantInfoNotify infoNotify;

	Channels& channelsType = infoNotify.m_channels;

	//ChannelType* channelType = new ChannelType[numberOfChannels];
	//std::list <ChannelType>  channel;
	FTRACEINTO << "numberOfChannels:" << numberOfChannels;
	for (int i = 0; i < numberOfChannels; ++i)
	{
		Channel channelType;

		if (pIpVideoCdrChannelMonitor[i]->GetMaxFrameRate() > 0)
		{
			channelType.m_resolutionFrameRate.m_max.m_width = ((CAdvanceVideo*)pIpVideoCdrChannelMonitor[i])->GetMaxResolutionWidth();
			channelType.m_resolutionFrameRate.m_max.m_height = ((CAdvanceVideo*)pIpVideoCdrChannelMonitor[i])->GetMaxResolutionHeight();
			channelType.m_resolutionFrameRate.m_max.m_frameRate = pIpVideoCdrChannelMonitor[i]->GetMaxFrameRate();
			channelType.m_resolutionFrameRate.m_min.m_width = ((CAdvanceVideo*)pIpVideoCdrChannelMonitor[i])->GetMinResolutionWidth();
			channelType.m_resolutionFrameRate.m_min.m_height = ((CAdvanceVideo*)pIpVideoCdrChannelMonitor[i])->GetMinResolutionHeight();
			channelType.m_resolutionFrameRate.m_min.m_frameRate = pIpVideoCdrChannelMonitor[i]->GetMinFrameRate();
			channelType.m_totalNumberOfPackets.m_number = pIpVideoCdrChannelMonitor[i]->GetNumOfPackets();
			channelType.m_totalNumberOfLostPackets.m_number = pIpVideoCdrChannelMonitor[i]->GetPacketLoss();
			channelType.m_jitter.m_average = pIpVideoCdrChannelMonitor[i]->GetJitter();
			channelType.m_jitter.m_peak = pIpVideoCdrChannelMonitor[i]->GetJitterPeak();
			//channelType.m_latency.m_average = pIpVideoCdrChannelMonitor[i]->GetLatency();
			//channelType.m_latency.m_peak =

			if (i == 0)
				channelType.m_channelName =  "video";
			else
				channelType.m_channelName =  "content";
		}

		channelsType.m_channel.push_back(channelType);
	}

	//channelsType.m_channel = channel;
	//((ParticipantInfoNotifyType*)((EventElementType*)mscPolycomMixer->m_pEvent)->m_pParticipant_Info_Notify)->m_channels = channelsType;
	infoNotify.m_partyCallId = partyId;
	infoNotify.m_confId = confId;
	//eventElement.m_pParticipant_Info_Notify = participantInfoNotifyType;
	//eventElement.m_id = //todo
	eventElement.m_subscriptionState = eConferenceInfoSubscriptionState_Active; //is it right?
	//eventElement.m_urgent = todo
	//eventElement.m_expires = todo
	//eventElement.m_minExpires = todo
	//eventElement.m_reason = todo
	//eventElement.m_retryAfter = todo
	//.m_channels.m_channelName = eContent;

	eventElement.m_pParticipantInfoNotify = &infoNotify;
	mscPolycomMixer.m_pEvent = &eventElement;

	FTRACEINTO << mscPolycomMixer;

	//return the message to MCCF->DMA
	CSegment* pSeg = new CSegment;
	*pSeg << mscPolycomMixer;

	CTaskApi api; // api to tx
	api.CreateOnlyApi(clientRspMbx); //*state.pClientRspMbx);
	api.SendMsg(pSeg, MCCF_CDR_PACKAGE_RESPONSE);
}

///////////////////////////////////////////////////////////////////////////////////////////
