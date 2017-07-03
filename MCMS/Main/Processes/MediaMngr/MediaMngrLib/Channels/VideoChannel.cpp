#include "VideoChannel.h"
#include "MediaChannel.h"
#include "TraceStream.h"
#include "Trace.h"
#include "OpcodesMcmsCardMngrIpMedia.h"
#include "IpCmReq.h"
#include "IpChannelParams.h"
#include "IpMfaOpcodes.h"
#include "IpRtpReq.h"
#include "IpCommonUtilTrace.h"
#include "ChannelParams.h"
#include "OsFileIF.h"
#include "OpcodesMcmsVideo.h"
#include "H221.h"
#include "OpcodesMcmsAudio.h"
#include "AudioApiDefinitionsStrings.h"
#include "MediaRepository.h"
#include "DtmfAlgDB.h"

#include "Segment.h"

#include "FaultsDefines.h"
#include "HlogApi.h"

#include <cerrno>
#include "OsFileIF.h"

#include "StructTm.h"

#include <arpa/inet.h>


extern void SendMessageToGideonSimApp(CSegment& rParam);


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////////   CVideoChannel
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//					MESSAGE_MAP
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CVideoChannel)

	ONEVENT( VIDEO_OUT_PARAM_EVENT_MESSAGE,					ANYCASE,   	CVideoChannel::OnVideoOutParamMessageAll)
	ONEVENT( VIDEO_IN_PARAM_EVENT_MESSAGE,					ANYCASE,   	CVideoChannel::OnVideoInParamMessageAll)
	ONEVENT( TIMER_ENABLE_INTRA,							ANYCASE,   	CVideoChannel::OnTimerEnableIntra)
	ONEVENT( VIDEO_OUT_UPDATE_PARAM_EVENT_MESSAGE,			ANYCASE,   	CVideoChannel::OnVideoOutUpdateParamMessageAll)
	ONEVENT( VIDEO_OUT_RESET_EVENT_MESSAGE,					ANYCASE,   	CVideoChannel::OnVideoOutResetMessageAll)
	ONEVENT( VIDEO_IN_UPDATE_PIC_EVENT_MESSAGE,				ANYCASE,	CVideoChannel::OnVideoInUpdatePicMessageAll)
	ONEVENT( VIDEO_OUT_ENCODER_UPDATE_PARAM_EVENT_MESSAGE,	ANYCASE,	CVideoChannel::OnVideoOutEncoderUpdateParamMessageAll)


PEND_MESSAGE_MAP(CVideoChannel,CMediaChannel);

/////////////////////////////////////////////////////////////////////////////

CVideoChannel::CVideoChannel() : CMediaChannel()
{

}

/////////////////////////////////////////////////////////////////////////////

CVideoChannel::CVideoChannel(CTaskApp* pOwnerTask, INT32 channelDirection)
		: CMediaChannel(pOwnerTask, channelDirection)
{
	m_framePosition = 0;
	m_bAllowIntraNow = TRUE;
	m_intraDelayTimeResponse = ::GetMediaMngrCfg()->GetIntraDelayTimeResponse();
	m_eofTsFactor = 3600;

	if (::GetVideoRecording() == TRUE)
	{
//		m_bWriteFlag = TRUE;
		m_tIncomingChannelParam.bReadFlag = TRUE;
		m_tIncomingChannelParam.bWriteMedia = TRUE;
	}
	
	if (::GetVideoCheckSeqNum() == TRUE)
	{
		m_tIncomingChannelParam.bReadFlag = TRUE;
		m_tIncomingChannelParam.bCheckSeqNumber = TRUE;
	}

	if (::GetDetectIntra() == TRUE)
	{
		m_tIncomingChannelParam.bReadFlag = TRUE;
		m_tIncomingChannelParam.bDetectIntra = TRUE;
	}

	m_mediaLibrary = ::GetMediaMngrCfg()->GetVideoLibrary();
	m_videoItemName = "";
}

/////////////////////////////////////////////////////////////////////////////


CVideoChannel::~CVideoChannel()
{
	if (m_channelDirection == MEDIA_DIRECTION_OUT)
	{
		TRACEINTO << GetParticipantTicket() << " CVideoChannel::~CVideoChannel() Out";
	}
	else if (m_channelDirection == MEDIA_DIRECTION_IN)
	{
		TRACEINTO << GetParticipantTicket() << " CVideoChannel::~CVideoChannel() In";
	}
}

/////////////////////////////////////////////////////////////////////////////
//					GetMessageMap
/////////////////////////////////////////////////////////////////////////////

void*  CVideoChannel::GetMessageMap()
{
  return (void*)m_msgEntries;
}


/////////////////////////////////////////////////////////////////////////////
//					NameOf
/////////////////////////////////////////////////////////////////////////////

const char*  CVideoChannel::NameOf() const
{
	return "CVideoChannel";
}



/////////////////////////////////////////////////////////////////////////////
//					HandleEvent
/////////////////////////////////////////////////////////////////////////////

void  CVideoChannel::HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode)
{

	switch ( opCode )
	{
		default:
		{         // all other messages
			DispatchEvent(opCode,pMsg);
			break;
		}
	}

}

/////////////////////////////////////////////////////////////////////////////


string CVideoChannel::ChannelData()
{
	string str = "";
	str = CMediaChannel::ChannelData();

	//add relevant data for Video Channel
	//str += "\some video parameter: ";
	//str += GetVideoParameterStr();

	return str.c_str();
}


/////////////////////////////////////////////////////////////////////////////


int CVideoChannel::SetupMediaFile()
{
	//primary full file name
	string fullPrimaryFileName = ::GetMediaMngrCfg()->GetPrimaryVideoFileReadPath();

	//library name
	fullPrimaryFileName += "/";
	fullPrimaryFileName += m_videoItemName;

	//file name
	fullPrimaryFileName += "/";
	fullPrimaryFileName += m_strFileName;

	TRACEINTO << GetParticipantTicket() << " CVideoChannel::SetupMediaFile - fullPrimaryFileName: " << fullPrimaryFileName;

	//fetching the video buffer - primary
	m_mediaRepositoryElement = ::GetMediaRepository()->GetVideoDB()->GetMediaElement(fullPrimaryFileName);
	if (m_mediaRepositoryElement == NULL)
	{
		TRACEINTO << GetParticipantTicket() << " MM WARNING CVideoChannel::SetupMediaFile - primary search. Loading from install path...";

		//installation full file name
		string fullInstallationFileName = ::GetMediaMngrCfg()->GetInstallationVideoFileReadPath();

		//library name
		fullInstallationFileName += "/";
		fullInstallationFileName += m_videoItemName;

		//file name
		fullInstallationFileName += "/";
		fullInstallationFileName += m_strFileName;

		TRACEINTO << GetParticipantTicket() << " CVideoChannel::SetupMediaFile - install path: " << fullInstallationFileName;

		//fetching the video buffer - installation
		m_mediaRepositoryElement = ::GetMediaRepository()->GetVideoDB()->GetMediaElement(fullInstallationFileName);
		if (m_mediaRepositoryElement == NULL)
		{
			CLargeString description;
			description << GetParticipantTicket() << " MM ERROR CVideoChannel::SetupMediaFile - file not found. install path: " << fullInstallationFileName;

			TRACEINTO << description.GetString();

			CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
								888,
								SYSTEM_MESSAGE,
								description.GetString(),
								FALSE);

			return STATUS_ERROR;
		}
	}

	m_mediaFileBuffer = m_mediaRepositoryElement->GetDataBuffer();
	m_fileSize = m_mediaRepositoryElement->GetSize();

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

int CVideoChannel::GetTimeToNextTransmission(DWORD timeStamp/*, TBD - frameRate */ )
{
	DWORD deltaTimeStamp = timeStamp - m_currentFrameTS;

	//TRACEINTO << "$$ - deltaTimeStamp= " << deltaTimeStamp;

	deltaTimeStamp = (deltaTimeStamp*10)/9;

	int timeToSleep = deltaTimeStamp / 1000;

	//TRACEINTO << "$$ - timeToSleep= " << timeToSleep;

	DWORD currentFraction = deltaTimeStamp - (timeToSleep * 1000);

	//TRACEINTO << "$$ - currentFraction= " << currentFraction;

	m_fraction += currentFraction;

	//TRACEINTO << "$$ - m_fraction= " << m_fraction;

	if (m_fraction >= 1000)
	{
		timeToSleep++;

		//TRACEINTO << "$$ fraction- timeToSleep= " << timeToSleep;

		m_fraction -= 1000;
	}

	//TRACEINTO << "$$Video - timeToSleep= " << timeToSleep;

	return timeToSleep;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CVideoChannel::GetTimeToNextTransmissionMl(DWORD timeStamp)
{
	DWORD deltaTimeStamp = timeStamp - m_currentFrameTS;

	deltaTimeStamp = (deltaTimeStamp*10)/9;

	int timeToSleep = deltaTimeStamp / 100;

	//TRACEINTO << "CVideoChannel::GetTimeToNextTransmissionMl - timeToSleep= " << timeToSleep;

	return timeToSleep;
}

/////////////////////////////////////////////////////////////////////////////

//void CVideoChannel::SendMedia()
//{
//	if (GetEncryptionType() == kAES_CBC)
//	{
//		DWORD	punIVector[4] = {0};
//		DWORD payloadSize = 0;
//		int paddingSize = 0;
//		int sizeMod16 = 0;
//		int lastByteIndexInPacket = 0;
//		DWORD unSeqNumber = 0;
//		DWORD unTimeStamp = 0;
//
//		for (int j = 0; j < m_numOfPacketsForTransmission-1; j++)	// last packet is for next transmission
//		{
//			payloadSize = m_rtpPacketsSize[j] - sizeof(TRtpHeader);
//
//			sizeMod16 = payloadSize % 16;	// packet size should be in multiples of 16 Bytes
//			paddingSize = 0;
//
//			//unset RTP header 'P' field
//			m_rtpPacketsArr[j][0] &= 0xDF;
//
//			if (0 != sizeMod16)
//			{
//				paddingSize = 16 - sizeMod16;
//
//				//set RTP header 'P' field
//				m_rtpPacketsArr[j][0] |= 0x20;
//
//
//				//set padding cipher to 0s
//				memset(&m_rtpPacketsArr[j][sizeof(TRtpHeader) + payloadSize], '0', paddingSize);
//
//				lastByteIndexInPacket = sizeof(TRtpHeader) + payloadSize + paddingSize -1;
//				//Set padding size in last byte of packet
//				BYTE* pLastByteInPacket = (BYTE*)&m_rtpPacketsArr[j][lastByteIndexInPacket];
//				*pLastByteInPacket = (BYTE)paddingSize;
//			}
//
//			/////////////////////////////////////////////////////
//			//Set Initial Vector values
//
//			unSeqNumber = (m_rtpPacketsArr[j][2]<<8) | m_rtpPacketsArr[j][3];
//
//			unTimeStamp = m_rtpPacketsArr[j][4]<<24 | (m_rtpPacketsArr[j][5]<<16) | (m_rtpPacketsArr[j][6]<<8) | m_rtpPacketsArr[j][7];
//
//			punIVector[0] = (unSeqNumber << 16) | (unTimeStamp >> 16);
//			punIVector[1] = (unTimeStamp << 16) | (unSeqNumber & 0xFFFF);
//			punIVector[2] = unTimeStamp;
//			punIVector[3] = punIVector[0];
//
//
//
//			EncryptCBC((Uint8*)&m_rtpPacketsArr[j][sizeof(TRtpHeader)] , //encrypt only payload
//								payloadSize,
//				 				punIVector,
//				 				m_aunExpandedRoundKey);
//
//
//			//set packet size
//			m_rtpPacketsSize[j] += paddingSize;
//		}
//	}
//
//
//	//TRACEINTO << "CVideoChannel::SendMedia - m_numOfPacketsForTransmission: " << m_numOfPacketsForTransmission;
//	for (int i = 0; i < m_numOfPacketsForTransmission-1; i++)	// last packet is for next transmission
//	{
//		//TRACEINTO << "CVideoChannel::SendMedia - index: " << i << " m_rtpPacketsSize: " << m_rtpPacketsSize[i];
//		SendTxSocket(m_rtpPacketsArr[i], m_rtpPacketsSize[i]);
//	}
//
//	m_numOfPacketsForTransmission = 0;
//}

/////////////////////////////////////////////////////////////////////////////

void CVideoChannel::RestartMediaTx()
{
	TRACEINTO << GetParticipantTicket() << " CVideoChannel::RestartMediaTx";

	if (m_bAllowIntraNow == TRUE)
	{
		TRACEINTO << GetParticipantTicket() << " CVideoChannel::RestartMediaTx invoke intra m_lastPacketSeqNumber = " << m_lastPacketSeqNumber;
		m_readInd = 0;
		m_lastPacketSeqNumber = 0;
		m_lastReadInd = 0;
		m_loopCounter = 0;

		m_bAllowIntraNow = FALSE;

		StartTimer (TIMER_ENABLE_INTRA, m_intraDelayTimeResponse);
	}
}


/////////////////////////////////////////////////////////////////////////////

void CVideoChannel::OnTimerEnableIntra(CSegment* pParam)
{
	TRACEINTO << GetParticipantTicket() << " CVideoChannel::OnTimerEnableIntra";

	m_bAllowIntraNow = TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CVideoChannel::OnVideoOutParamMessageAll(CSegment* pParam)
{
	TRACEINTO << GetParticipantTicket() << " CVideoChannel::OnVideoOutParamMessageAll";

	DWORD protocol, bitrate, res, framerate, profileType;
	*pParam	>> protocol;

	m_eVideoProtocol = (EVideoProtocol)protocol;

	*pParam	>> bitrate
			>> res
			>> framerate
			>> profileType;


	m_tCurrentVideoParam.nVideoBitRate = bitrate;
	m_tCurrentVideoParam.eVideoResolution = (EMMVideoResolution)res;
	m_tCurrentVideoParam.eVideoFrameRate = (EMMVideoFrameRate)framerate;
	m_tCurrentVideoParam.eVideoProfileType = (EMMVideoProfileType)profileType;


	TRACEINTO << GetParticipantTicket() << " CVideoChannel::OnVideoOutParamMessageAll res=" << EMMVideoResolutionNames[m_tCurrentVideoParam.eVideoResolution]
	                                    << " framerate=" << EShortMMVideoFrameRateNames[m_tCurrentVideoParam.eVideoFrameRate]
	                                    << " bitrate=" << m_tCurrentVideoParam.nVideoBitRate
	                                    << " profileType=" << m_tCurrentVideoParam.eVideoProfileType;


	//build video out file name
	////////////////////////////

	//get bitrate in string
	string bitratestr = GetBitrateStr(bitrate);

	//get next media item name
	m_mediaLibrary = ::GetMediaMngrCfg()->GetVideoLibrary();
	m_videoItemName = m_mediaLibrary->GetNextMediaItemName();
	m_strFileName = m_videoItemName;

	if (m_eVideoProtocol == E_VIDEO_PROTOCOL_H264)
	{
		m_strFileName += "_PTC264";
		if (E_MM_VIDEO_PROFILE_HIGH == m_tCurrentVideoParam.eVideoProfileType)
		{
			m_strFileName += "_PROF";		//High Profile (HI)
			m_strFileName += EShortMMVideoProfileTypeNames[m_tCurrentVideoParam.eVideoProfileType];
		}
	}
	else if (m_eVideoProtocol==E_VIDEO_PROTOCOL_H263)
		m_strFileName += "_PTC263";
	else
		m_strFileName += "_PTCUNKNOWN";

	m_strFileName += "_RES";
	m_strFileName += EShortMMVideoResolutionNames[m_tCurrentVideoParam.eVideoResolution];
	m_strFileName += "_FR";
	m_strFileName += EShortMMVideoFrameRateNames[m_tCurrentVideoParam.eVideoFrameRate];
	m_strFileName += "_BR";
	m_strFileName += bitratestr;
	m_strFileName += ".vid";

	TRACEINTO << GetParticipantTicket() << " CVideoChannel::OnVideoOutParamMessageAll file name: " << m_strFileName;

	if (FALSE == m_bVideoOutParam)	// this is the first time TB_MSG_OPEN_PORT_REQ is received for this call
	{
		m_bVideoOutParam = TRUE;
		if (m_bRtpUpdatePort && m_bOpenUdpPort)
			StartMediaTx();
	}
	else
	{	// this is not the first time this function is called:
		// encoder received TB_MSG_CLOSE_PORT_REQ & TB_MSG_OPEN_PORT_REQ due to change of remote caps
		// in this case - we will change the  media file for transmission & reset the relevant parameters (similar to update)

		int status = SetupMediaFile();
		if( STATUS_OK != status)
		{
			TRACEINTO << GetParticipantTicket() << " MM ERROR CVideoChannel::OnVideoOutParamMessageAll - Error in SetupMediaFile.";
			return;
		}

		// reset media params
		m_readInd = 0;
		m_lastReadInd = 0;
		m_loopCounter = 0;
		m_lastPacketSeqNumber = 0;

		// call prepare media frame to be transmitted in the next timer
		PrepareMediaFrame();
	}
}


/////////////////////////////////////////////////////////////////////////////

void CVideoChannel::OnVideoInParamMessageAll(CSegment* pParam)
{
	TRACEINTO << GetParticipantTicket() << " CVideoChannel::OnVideoInParamMessageAll";

	DWORD protocol;
	*pParam	>> protocol;

	m_eVideoProtocol = (EVideoProtocol)protocol;

}


/////////////////////////////////////////////////////////////////////////////

void CVideoChannel::OnVideoOutResetMessageAll(CSegment* pParam)
{
	TRACEINTO << GetParticipantTicket() << " CVideoChannel::OnVideoOutResetMessageAll";

	m_bAllowIntraNow = TRUE;
	RestartMediaTx();
}

/////////////////////////////////////////////////////////////////////////////

void CVideoChannel::OnVideoInUpdatePicMessageAll(CSegment* pParam)
{
	TRACEINTO << GetParticipantTicket() << " CVideoChannel::OnVideoInUpdatePicMessageAll";
	RequestFastUpdateFromEP();
}

/////////////////////////////////////////////////////////////////////////////

string CVideoChannel::GetBitrateStr(DWORD bitrate)
{
	string bitratestr = "";

	if (bitrate < 128000)
		bitratestr = VIDEO_64_KBPS;
	if (bitrate >= 128000 && bitrate < 192000)
		bitratestr = VIDEO_128_KBPS;

	else if (bitrate >= 192000 && bitrate < 256000)
		bitratestr = VIDEO_192_KBPS;

	else if (bitrate >= 256000 && bitrate < 320000)
		bitratestr = VIDEO_256_KBPS;
	else if (bitrate >= 320000 && bitrate < 384000)
		bitratestr = VIDEO_320_KBPS;
	else if (bitrate >= 384000 && bitrate < 512000)
		bitratestr = VIDEO_448_KBPS;
	else if (bitrate >= 512000 && bitrate < 768000)
		bitratestr = VIDEO_704_KBPS;

	else if (bitrate >= 768000 && bitrate < 832000)
		bitratestr = VIDEO_768_KBPS;
	else if (bitrate >= 832000 && bitrate < 1024000)
		bitratestr = VIDEO_960_KBPS;

	else if (bitrate >= 1024000 && bitrate < 1152000)
		bitratestr = VIDEO_1088_KBPS;
	else if (bitrate >= 1152000 && bitrate < 1280000)
		bitratestr = VIDEO_1216_KBPS;
	else if (bitrate >= 1280000 && bitrate < 1472000)
		bitratestr = VIDEO_1408_KBPS;

	else if (bitrate >= 1472000 && bitrate < 1536000)
		bitratestr = VIDEO_1472_KBPS;

	else if (bitrate >= 1536000 && bitrate < 1728000)
		bitratestr = VIDEO_1664_KBPS;
	else if (bitrate >= 1728000 && bitrate < 1920000)
		bitratestr = VIDEO_1856_KBPS;

	else if (bitrate >= 1920000 && bitrate < 2048000)
		bitratestr = VIDEO_1984_KBPS;
	else if (bitrate >= 2048000 && bitrate < 2560000)
		bitratestr = VIDEO_2496_KBPS;
	else if (bitrate >= 2560000 && bitrate < 3072000)
		bitratestr = VIDEO_3008_KBPS;
	else if (bitrate >= 3072000 && bitrate < 3584000)
		bitratestr = VIDEO_3520_KBPS;
	else if (bitrate >= 3584000 && bitrate < 4096000)
		bitratestr = VIDEO_4032_KBPS;

	else if (bitrate >= 4096000 && bitrate < 6144000)
		bitratestr = VIDEO_6080_KBPS;

	else if (bitrate >= 6144000 && bitrate < 8192000)
		bitratestr = VIDEO_8128_KBPS;

	return bitratestr;
}

/////////////////////////////////////////////////////////////////////////////

void CVideoChannel::OnVideoOutUpdateParamMessageAll(CSegment* pParam)
{
	DWORD contentBitrate, contentMode;
	*pParam	>> contentBitrate;
	*pParam	>> contentMode;


	UpdateVideoParam(contentBitrate, contentMode);

	//build current video-out file name
	///////////////////////////////////

	//get bitrate in string
	string bitratestr = GetBitrateStr(m_tCurrentVideoParam.nVideoBitRate);

	m_strFileName = m_strFileName.substr(0, 4); //temp for changing the video movie

	if (m_eVideoProtocol == E_VIDEO_PROTOCOL_H264)
	{
		m_strFileName += "_PTC264";
		if (E_MM_VIDEO_PROFILE_HIGH == m_tCurrentVideoParam.eVideoProfileType)
		{
			m_strFileName += "_PROF";		//High Profile (HI)
			m_strFileName += EShortMMVideoProfileTypeNames[m_tCurrentVideoParam.eVideoProfileType];
		}
	}
	else if (m_eVideoProtocol==E_VIDEO_PROTOCOL_H263)
		m_strFileName += "_PTC263";
	else
		m_strFileName += "_PTCUNKNOWN";

	m_strFileName += "_RES";
	m_strFileName += EShortMMVideoResolutionNames[m_tCurrentVideoParam.eVideoResolution];
	m_strFileName += "_FR";
	m_strFileName += EShortMMVideoFrameRateNames[m_tCurrentVideoParam.eVideoFrameRate];
	m_strFileName += "_BR";
	m_strFileName += bitratestr;
	m_strFileName += ".vid";

	TRACEINTO << GetParticipantTicket() << " CVideoChannel::OnVideoOutUpdateParamMessageAll file name: " << m_strFileName;

	int status = SetupMediaFile();
	if( STATUS_OK != status)
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CVideoChannel::OnVideoOutUpdateParamMessageAll - Error in SetupMediaFile.";
		return;
	}

	// reset media params
	m_readInd = 0;
	m_lastReadInd = 0;
	m_loopCounter = 0;
	m_lastPacketSeqNumber = 0;


	// call prepare media frame to be transmited in the next timer - only in the FIRST dtmf session
	PrepareMediaFrame();
}


/////////////////////////////////////////////////////////////////////////////

void CVideoChannel::UpdateVideoParam(DWORD contentBitrate, DWORD contentMode)
{
	DWORD newVideoBitrate = 0;
	// content mode ON --> save video data
	if (contentMode)
	{
		m_tSaveContentVideoParam.nVideoBitRate = m_tCurrentVideoParam.nVideoBitRate;
		m_tSaveContentVideoParam.eVideoResolution = m_tCurrentVideoParam.eVideoResolution;
		m_tSaveContentVideoParam.eVideoFrameRate = m_tCurrentVideoParam.eVideoFrameRate;
		m_tSaveContentVideoParam.eVideoProfileType = m_tCurrentVideoParam.eVideoProfileType;

		//update current video param
		m_tCurrentVideoParam.nVideoBitRate = m_tCurrentVideoParam.nVideoBitRate - contentBitrate;

		TRACEINTO << GetParticipantTicket() << " CVideoChannel::UpdateVideoParam new Video Bitrate=" << m_tCurrentVideoParam.nVideoBitRate;
	}
	else //content mode OFF --> retrieve saved video params
 	{
		m_tCurrentVideoParam.nVideoBitRate = m_tSaveContentVideoParam.nVideoBitRate;
		m_tCurrentVideoParam.eVideoResolution = m_tSaveContentVideoParam.eVideoResolution;
		m_tCurrentVideoParam.eVideoFrameRate = m_tSaveContentVideoParam.eVideoFrameRate;
		m_tCurrentVideoParam.eVideoProfileType = m_tSaveContentVideoParam.eVideoProfileType;
	}
}

/////////////////////////////////////////////////////////////////////////////

void  CVideoChannel::RequestFastUpdateFromEP()
{
	if (m_boardId == (DWORD)-1)
	{
		TRACEINTO << "CVideoChannel::RequestFastUpdateFromEP - MM Error - boardId not set ";
		return;
	}

	TRACEINTO << "CVideoChannel::RequestFastUpdateFromEP ";

	CSegment  msg;
	msg << (DWORD) IP_RTP_VIDEO_UPDATE_PIC_IND
		<< m_boardId
		<< m_subBoardId
		<< m_unitId
		<< m_conferenceId
		<< m_partyId
		<< m_connectionId;

	::SendMessageToGideonSimApp(msg);

}


/////////////////////////////////////////////////////////////////////////////

void CVideoChannel::OnVideoOutEncoderUpdateParamMessageAll(CSegment* pParam)
{
	TRACEINTO << GetParticipantTicket() << " CVideoChannel::OnVideoOutEncoderUpdateParamMessageAll";

	DWORD protocol, bitrate, res, framerate, profileType;
	*pParam	>> protocol;

	m_eVideoProtocol = (EVideoProtocol)protocol;

	*pParam	>> bitrate
			>> res
			>> framerate
			>> profileType;


	m_tCurrentVideoParam.nVideoBitRate = bitrate;
	m_tCurrentVideoParam.eVideoResolution = (EMMVideoResolution)res;
	m_tCurrentVideoParam.eVideoFrameRate = (EMMVideoFrameRate)framerate;
	m_tCurrentVideoParam.eVideoProfileType = (EMMVideoProfileType)profileType;


	TRACEINTO << GetParticipantTicket() << " CVideoChannel::OnVideoOutEncoderUpdateParamMessageAll res=" << EMMVideoResolutionNames[m_tCurrentVideoParam.eVideoResolution]
	                                    << " framerate=" << EShortMMVideoFrameRateNames[m_tCurrentVideoParam.eVideoFrameRate]
	                                    << " bitrate=" << m_tCurrentVideoParam.nVideoBitRate
	                                    << " profileType=" << m_tCurrentVideoParam.eVideoProfileType;

	//build video out file name
	////////////////////////////

	//get bitrate in string
	string bitratestr = GetBitrateStr(bitrate);

	//get next media item name
	m_mediaLibrary = ::GetMediaMngrCfg()->GetVideoLibrary();
	m_videoItemName = m_mediaLibrary->GetNextMediaItemName();
	m_strFileName = m_videoItemName;

	if (m_eVideoProtocol == E_VIDEO_PROTOCOL_H264)
	{
		m_strFileName += "_PTC264";
		if (E_MM_VIDEO_PROFILE_HIGH == m_tCurrentVideoParam.eVideoProfileType)
		{
			m_strFileName += "_PROF";		//High Profile (HI)
			m_strFileName += EShortMMVideoProfileTypeNames[m_tCurrentVideoParam.eVideoProfileType];
		}
	}
	else if (m_eVideoProtocol==E_VIDEO_PROTOCOL_H263)
		m_strFileName += "_PTC263";
	else
		m_strFileName += "_PTCUNKNOWN";

	m_strFileName += "_RES";
	m_strFileName += EShortMMVideoResolutionNames[m_tCurrentVideoParam.eVideoResolution];
	m_strFileName += "_FR";
	m_strFileName += EShortMMVideoFrameRateNames[m_tCurrentVideoParam.eVideoFrameRate];
	m_strFileName += "_BR";
	m_strFileName += bitratestr;
	m_strFileName += ".vid";


	int status = SetupMediaFile();
	if( STATUS_OK != status)
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CVideoChannel::OnVideoOutEncoderUpdateParamMessageAll - Error in SetupMediaFile.";
		return;
	}

	TRACEINTO << GetParticipantTicket() << " CVideoChannel::OnVideoOutEncoderUpdateParamMessageAll file name: " << m_strFileName;


	// reset media params
	m_readInd = 0;
	m_lastReadInd = 0;
	m_loopCounter = 0;
	m_lastPacketSeqNumber = 0;


	// call prepare media frame to be transmitted in the next timer
	PrepareMediaFrame();

}


