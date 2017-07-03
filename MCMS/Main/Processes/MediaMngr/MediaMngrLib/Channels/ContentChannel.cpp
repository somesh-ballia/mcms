#include "ContentChannel.h"
#include "MediaChannel.h"
#include "VideoChannel.h"
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
///////////////////////   CContentChannel
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//					MESSAGE_MAP
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CContentChannel)

PEND_MESSAGE_MAP(CContentChannel,CVideoChannel);

/////////////////////////////////////////////////////////////////////////////

CContentChannel::CContentChannel() : CVideoChannel()
{

}

/////////////////////////////////////////////////////////////////////////////

CContentChannel::CContentChannel(CTaskApp* pOwnerTask, INT32 channelDirection)
		: CVideoChannel(pOwnerTask, channelDirection)
{

	m_mediaLibrary = ::GetMediaMngrCfg()->GetContentLibrary();
}

/////////////////////////////////////////////////////////////////////////////

CContentChannel::~CContentChannel()
{
}

/////////////////////////////////////////////////////////////////////////////
//					GetMessageMap
/////////////////////////////////////////////////////////////////////////////

void*  CContentChannel::GetMessageMap()
{
  return (void*)m_msgEntries;
}


/////////////////////////////////////////////////////////////////////////////
//					NameOf
/////////////////////////////////////////////////////////////////////////////

const char*  CContentChannel::NameOf() const
{
	return "CContentChannel";
}



/////////////////////////////////////////////////////////////////////////////
//					HandleEvent
/////////////////////////////////////////////////////////////////////////////

void  CContentChannel::HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode)
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

string CContentChannel::ChannelData()
{
	string str = "";
	str = CMediaChannel::ChannelData();

	//add relevant data for Content Channel
	//str += "\some content parameter: ";
	//str += GetContentParameterStr();

	return str.c_str();
}

/////////////////////////////////////////////////////////////////////////////

int CContentChannel::SetupMediaFile()
{
	//get next media item name
	m_mediaLibrary = ::GetMediaMngrCfg()->GetContentLibrary();
	string mediaItemName = m_mediaLibrary->GetNextMediaItemName();

	m_strFileName = mediaItemName;

	if  (eH263CapCode == m_capTypeCode)
		m_strFileName += "_PTC263";
	else if (eH264CapCode == m_capTypeCode)
		m_strFileName += "_PTC264";
	else
		m_strFileName += "_PTCUNKNOWN";

	m_strFileName += "_BR";

	//get bitrate in string
	string bitratestr = GetBitrateStr(GetBitRate());
	m_strFileName += bitratestr;
	m_strFileName += ".cont";

	//primary full file name
	string fullPrimaryFileName = ::GetMediaMngrCfg()->GetPrimaryContentFileReadPath();

	//library name
	fullPrimaryFileName += "/";
	fullPrimaryFileName += mediaItemName;

	//file name
	fullPrimaryFileName += "/";
	fullPrimaryFileName += m_strFileName;

	TRACEINTO << GetParticipantTicket() << " CContentChannel::SetupMediaFile - fullPrimaryFileName: " << fullPrimaryFileName;

	//fetching the video buffer - primary
	m_mediaRepositoryElement = ::GetMediaRepository()->GetContentDB()->GetMediaElement(fullPrimaryFileName);
	if (m_mediaRepositoryElement == NULL)
	{
		TRACEINTO << GetParticipantTicket() << " MM WARNING CContentChannel::SetupMediaFile - primary search. Loading from install path...";

		//installation full file name
		string fullInstallationFileName = ::GetMediaMngrCfg()->GetInstallationContentFileReadPath();

		//library name
		fullInstallationFileName += "/";
		fullInstallationFileName += mediaItemName;

		//file name
		fullInstallationFileName += "/";
		fullInstallationFileName += m_strFileName;

		TRACEINTO << GetParticipantTicket() << " CContentChannel::SetupMediaFile - install path: " << fullInstallationFileName;

		//fetching the content buffer - installation
		m_mediaRepositoryElement = ::GetMediaRepository()->GetContentDB()->GetMediaElement(fullInstallationFileName);
		if (m_mediaRepositoryElement == NULL)
		{
			CLargeString description;
			description << GetParticipantTicket() << " MM ERROR CContentChannel::SetupMediaFile - file not found. install path: " << fullInstallationFileName;

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

//void CContentChannel::SendMedia()
//{
//	CVideoChannel::SendMedia();
//}


/////////////////////////////////////////////////////////////////////////////
string CContentChannel::GetBitrateStr(DWORD bitrate)
{
	string bitratestr = "";

	if (bitrate < 1280)
		bitratestr = VIDEO_64_KBPS;
	else if (bitrate >= 1280)
		bitratestr = VIDEO_128_KBPS;

	return bitratestr;
}



