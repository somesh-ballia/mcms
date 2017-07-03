#include "AudioChannel.h"
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

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////////   CAudioChannel
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//					MESSAGE_MAP
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CAudioChannel)

	ONEVENT( AUDIO_OUT_RESET_EVENT_MESSAGE,			ANYCASE,   CAudioChannel::OnAudioOutResetMessageAll)
	ONEVENT( AUDIO_OUT_UPDATE_CHANNEL_EVENT_MESSAGE,ANYCASE,   CAudioChannel::OnAudioOutUpdateChannelMessageAll)

PEND_MESSAGE_MAP(CAudioChannel,CMediaChannel);

/////////////////////////////////////////////////////////////////////////////

CAudioChannel::CAudioChannel() : CMediaChannel()
{

}

/////////////////////////////////////////////////////////////////////////////

CAudioChannel::CAudioChannel(CTaskApp* pOwnerTask, INT32 channelDirection)
		: CMediaChannel(pOwnerTask, channelDirection)
{
	m_numOfDtmf = 0;
	m_currDtmfIndex = 0;

	m_saveMediaFileBuffer = NULL;
	m_saveFileSize = 0;
	m_saveReadInd = 0;

	m_eofTsFactor = 160;

	if (::GetAudioRecording() == TRUE)
	{
//		m_bWriteFlag = TRUE;
		m_tIncomingChannelParam.bReadFlag = TRUE;
		m_tIncomingChannelParam.bWriteMedia = TRUE;
	}

	if (::GetAudioCheckSeqNum() == TRUE)
	{
		m_tIncomingChannelParam.bReadFlag = TRUE;
		m_tIncomingChannelParam.bCheckSeqNumber = TRUE;
	}

	m_mediaLibrary = ::GetMediaMngrCfg()->GetAudioLibrary();

	m_bSendSilentStream = FALSE;
}

/////////////////////////////////////////////////////////////////////////////

CAudioChannel::~CAudioChannel()
{
	if (m_channelDirection == MEDIA_DIRECTION_OUT)
	{
		TRACEINTO << GetParticipantTicket() << " CAudioChannel::~CAudioChannel() Out";
	}
	else if (m_channelDirection == MEDIA_DIRECTION_IN)
	{
		TRACEINTO << GetParticipantTicket() << " CAudioChannel::~CAudioChannel() In";
	}
}

/////////////////////////////////////////////////////////////////////////////
//					GetMessageMap
/////////////////////////////////////////////////////////////////////////////

void*  CAudioChannel::GetMessageMap()
{
  return (void*)m_msgEntries;
}


/////////////////////////////////////////////////////////////////////////////
//					NameOf
/////////////////////////////////////////////////////////////////////////////

const char*  CAudioChannel::NameOf() const
{
	return "CAudioChannel";
}



/////////////////////////////////////////////////////////////////////////////
//					HandleEvent
/////////////////////////////////////////////////////////////////////////////

void  CAudioChannel::HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode)
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

string CAudioChannel::ChannelData()
{
	string str = "";
	str = CMediaChannel::ChannelData();

	//add relevant data for Audio Channel
	//str += "\some audio parameter: ";
	//str += GetAudioParameterStr();

	return str.c_str();
}

////////////////////////////////////////////////////////

int CAudioChannel::SetupMediaFile()
{
	//get next media item name
	m_mediaLibrary = ::GetMediaMngrCfg()->GetAudioLibrary();
	string mediaItemName = m_mediaLibrary->GetNextMediaItemName();

	m_strFileName = mediaItemName;
	m_strFileName += "_PTC";

	string capCodeTypeStr = "";

	switch (m_capTypeCode)
	{
		//G711
		case eG711Ulaw64kCapCode:
		{
			capCodeTypeStr = "G711U_BR64";
			break;
		}
		case eG711Alaw64kCapCode:
		{
			capCodeTypeStr = "G711A_BR64";
			break;
		}

		//G722
		case eG722_48kCapCode:
		{
			capCodeTypeStr = "G722_BR48";
			break;
		}
		case eG722_56kCapCode:
		{
			capCodeTypeStr = "G722_BR56";
			break;
		}
		case eG722_64kCapCode:
		{
			capCodeTypeStr = "G722_BR64";
			break;
		}


		//G729
		case eG729CapCode:
		case eG729AnnexACapCode:
		case eG729wAnnexBCapCode:
		case eG729AnnexAwAnnexBCapCode:
		{
			capCodeTypeStr = "G729_BR8";
			break;
		}

		//G722.1
		case eG7221_24kCapCode:
		{
			capCodeTypeStr = "G722.1_BR24";
			break;
		}
		case eG7221_32kCapCode:
		{
			capCodeTypeStr = "G722.1_BR32";
			break;
		}


		//Siren14
		case eSiren14_24kCapCode:
		{
			capCodeTypeStr = "SIREN14_BR24";
			break;
		}
		case eSiren14_32kCapCode:
		{
			capCodeTypeStr = "SIREN14_BR32";
			break;
		}
		case eSiren14_48kCapCode:
		{
			capCodeTypeStr = "SIREN14_BR48";
			break;
		}

		//Siren14 Stereo
		case eSiren14Stereo_48kCapCode:
		{
			capCodeTypeStr = "SIREN14STEREO_BR48";
			break;
		}
		case eSiren14Stereo_56kCapCode:
		{
			capCodeTypeStr = "SIREN14STEREO_BR56";
			break;
		}
		case eSiren14Stereo_64kCapCode:
		{
			capCodeTypeStr = "SIREN14STEREO_BR64";
			break;
		}
		case eSiren14Stereo_96kCapCode:
		{
			capCodeTypeStr = "SIREN14STEREO_BR96";
			break;
		}


		//G722.1.C
		case eG7221C_24kCapCode:
		{
			capCodeTypeStr = "G722.1.C_BR24";
			break;
		}
		case eG7221C_32kCapCode:
		{
			capCodeTypeStr = "G722.1.C_BR32";
			break;
		}
		case eG7221C_48kCapCode:
		{
			capCodeTypeStr = "G722.1.C_BR48";
			break;
		}



		//G723.1
		/*case eG7231CapCode:
		case eG7231AnnexCapCode:
		{
			m_strFileName += "G723.1_BR7";
			break;
		}
		*/



		//G719
		case eG719_32kCapCode:
		{
			capCodeTypeStr = "G719_BR32";
			break;
		}
		case eG719_48kCapCode:
		{
			capCodeTypeStr = "G719_BR48";
			break;
		}
		case eG719_64kCapCode:
		{
			capCodeTypeStr = "G719_BR64";
			break;
		}

		//G719 Stereo
		case eG719Stereo_64kCapCode:
		{
			capCodeTypeStr = "G719STEREO_BR64";
			break;
		}
		case eG719Stereo_96kCapCode:
		{
			capCodeTypeStr = "G719STEREO_BR96";
			break;
		}
		case eG719Stereo_128kCapCode:
		{
			capCodeTypeStr = "G719STEREO_BR128";
			break;
		}


		//Siren22
		case eSiren22_32kCapCode:
		{
			capCodeTypeStr = "SIREN22_BR32";
			break;
		}
		case eSiren22_48kCapCode:
		{
			capCodeTypeStr = "SIREN22_BR48";
			break;
		}
		case eSiren22_64kCapCode:
		{
			capCodeTypeStr = "SIREN22_BR64";
			break;
		}


		//Siren22 Stereo
		case eSiren22Stereo_64kCapCode:
		{
			capCodeTypeStr = "SIREN22STEREO_BR64";
			break;
		}
		case eSiren22Stereo_96kCapCode:
		{
			capCodeTypeStr = "SIREN22STEREO_BR96";
			break;
		}
		case eSiren22Stereo_128kCapCode:
		{
			capCodeTypeStr = "SIREN22STEREO_BR128";
			break;
		}
		case eAAC_LDCapCode:
		{
			capCodeTypeStr = "AAC_LD";
			break;
		}
		//SirenLPR
		case eSirenLPR_32kCapCode:
		{
			capCodeTypeStr = "SIRENLPR_BR32";
			break;
		}
		case eSirenLPR_48kCapCode:
		{
			capCodeTypeStr = "SIRENLPR_BR48";
			break;
		}

		case eSirenLPR_64kCapCode:
		{
			capCodeTypeStr = "SIRENLPR_BR64";
			break;
		}
		//SirenLPR Stereo
		case eSirenLPRStereo_64kCapCode:
		{
			capCodeTypeStr = "SIRENLPRSTEREO_BR64";
			break;
		}
		case eSirenLPRStereo_96kCapCode:
		{
			capCodeTypeStr = "SIRENLPRSTEREO_BR96";
			break;
		}
		case eSirenLPRStereo_128kCapCode:
		{
			capCodeTypeStr = "SIRENLPRSTEREO_BR128";
			break;
		}

		default:
			CLargeString description;
			description << GetParticipantTicket()
					    << " MM ERROR CAudioChannel::SetupMediaFile - MM does not support this Audio CODEC: "
					    << GetCapTypeCodeStr() << ".  Abort !";

			TRACEINTO << description.GetString();

			CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
								999,
								SYSTEM_MESSAGE,
								description.GetString() ,
								FALSE);


			return STATUS_ERROR;
	}

	//add the cap code type
	m_strFileName += capCodeTypeStr;

	if(m_bSendSilentStream)
	{
		m_strFileName += "_SILENT";
	}

	//add the file suffix
	m_strFileName += ".aud";


	//primary full file name
	string fullPrimaryFileName = ::GetMediaMngrCfg()->GetPrimaryAudioFileReadPath();

	//library name
	fullPrimaryFileName += "/";
	fullPrimaryFileName += mediaItemName;

	//file name
	fullPrimaryFileName += "/";
	fullPrimaryFileName += m_strFileName;


	TRACEINTO << GetParticipantTicket() << " CAudioChannel::SetupMediaFile - fullPrimaryFileName: " << fullPrimaryFileName;

	//fetching the audio buffer - primary
	m_mediaRepositoryElement = ::GetMediaRepository()->GetAudioDB()->GetMediaElement(fullPrimaryFileName);
	if (m_mediaRepositoryElement == NULL)
	{
		TRACEINTO << GetParticipantTicket() << " MM WARNING CAudioChannel::SetupMediaFile - primary search. Loading from install path...";

		//installation full file name
		string fullInstallationFileName = ::GetMediaMngrCfg()->GetInstallationAudioFileReadPath();

		//library name
		fullInstallationFileName += "/";
		fullInstallationFileName += mediaItemName;

		//file name
		fullInstallationFileName += "/";
		fullInstallationFileName += m_strFileName;

		TRACEINTO << GetParticipantTicket() << " CAudioChannel::SetupMediaFile - install path: " << fullInstallationFileName;

		//fetching the audio buffer - installation
		m_mediaRepositoryElement = ::GetMediaRepository()->GetAudioDB()->GetMediaElement(fullInstallationFileName);
		if (m_mediaRepositoryElement == NULL)
		{
			CLargeString description;
			description << GetParticipantTicket() << " MM ERROR CAudioChannel::SetupMediaFile - file not found. install path: " << fullInstallationFileName;

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


//////////////////////////////////////////

int CAudioChannel::GetTimeToNextTransmission(DWORD timeStamp)
{
	DWORD deltaTS = timeStamp - m_currentFrameTS;
	//TRACEINTO << "CAudioChannel::GetTimeToNextTransmission@ - deltaTS: " << deltaTS;

	int transmitFactor;

	switch (m_capTypeCode)
	{
		//G711
		case eG711Alaw64kCapCode:
		case eG711Alaw56kCapCode:
		case eG711Ulaw64kCapCode:
		case eG711Ulaw56kCapCode:
		//G722
		case eG722_64kCapCode:
		case eG722_56kCapCode:
		case eG722_48kCapCode:
		//G729
		case eG729CapCode:
		case eG729AnnexACapCode:
		case eG729wAnnexBCapCode:
		case eG729AnnexAwAnnexBCapCode:
		//G723.1
		case eG7231CapCode:
		case eG7231AnnexCapCode:
		{
			transmitFactor = 8;
			break;
		}


		//G722.1
		case eG7221_32kCapCode:
		case eG7221_24kCapCode:
		{
			transmitFactor = 16;
			break;
		}

		//Siren14
		case eSiren14_48kCapCode:
		case eSiren14_32kCapCode:
		case eSiren14_24kCapCode:
		case eSiren14Stereo_48kCapCode:
		case eSiren14Stereo_56kCapCode:
		case eSiren14Stereo_64kCapCode:
		case eSiren14Stereo_96kCapCode:
		//G722.1.C
		case eG7221C_48kCapCode:
		case eG7221C_32kCapCode:
		case eG7221C_24kCapCode:
		case eG7221C_CapCode:
		{
			transmitFactor = 32;
			break;
		}

		//G719
		case eG719_32kCapCode:
		case eG719_48kCapCode:
		case eG719_64kCapCode:
		case eG719_96kCapCode:
		case eG719_128kCapCode:
		case eG719Stereo_64kCapCode:
		case eG719Stereo_96kCapCode:
		case eG719Stereo_128kCapCode:
		//Siren22
		case eSiren22_32kCapCode:
		case eSiren22_48kCapCode:
		case eSiren22_64kCapCode:
		case eSiren22Stereo_64kCapCode:
		case eSiren22Stereo_96kCapCode:
		case eSiren22Stereo_128kCapCode:
		// SirenLPR
		case eSirenLPR_32kCapCode:
		case eSirenLPR_48kCapCode:
		case eSirenLPR_64kCapCode:
		case eSirenLPRStereo_64kCapCode:
		case eSirenLPRStereo_96kCapCode:
		case eSirenLPRStereo_128kCapCode:
		{
			transmitFactor = 48;
			break;
		}


		default:

			transmitFactor = 8;
	}

	//TRACEINTO << "CAudioChannel::GetTimeToNextTransmission@@ - transmitFactor: " << transmitFactor;

	DWORD timeToSleep = deltaTS/transmitFactor;

	//TRACEINTO << "CAudioChannel::GetTimeToNextTransmission@@ - time to sleep: " << timeToSleep;


	DWORD currentFraction = timeToSleep % 10;

	if (currentFraction != 0)
	{
		m_fraction += currentFraction;
		//TRACEINTO << "CAudioChannel::GetTimeToNextTransmission@@ - currentFraction: " << currentFraction;
		//TRACEINTO << "CAudioChannel::GetTimeToNextTransmission@@ - m_fraction: " << m_fraction;

		if (m_fraction >= 10)
		{
			timeToSleep += 10;
			m_fraction -= 10;
		}
	}

	//TRACEINTO << "CAudioChannel::GetTimeToNextTransmission@@ - timeToSleep: " << timeToSleep;
	//TRACEINTO << "CAudioChannel::GetTimeToNextTransmission@@ - m_fraction: " << m_fraction;


	timeToSleep /= 10; // becuase TIMER is in 1/100 sec

	//TRACEINTO << "$$Audio - timeToSleep= " << timeToSleep;

	return timeToSleep;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CAudioChannel::GetTimeToNextTransmissionMl(DWORD timeStamp)
{
	DWORD deltaTS = timeStamp - m_currentFrameTS;
	//TRACEINTO << "CAudioChannel::GetTimeToNextTransmission@ - deltaTS: " << deltaTS;

	int transmitFactor;

	switch (m_capTypeCode)
	{
		//G711
		case eG711Alaw64kCapCode:
		case eG711Ulaw64kCapCode:
		case eG711Alaw56kCapCode:
		case eG711Ulaw56kCapCode:
		//G722
		case eG722_64kCapCode:
		case eG722_56kCapCode:
		case eG722_48kCapCode:
		//G729
		case eG729CapCode:
		case eG729AnnexACapCode:
		case eG729wAnnexBCapCode:
		case eG729AnnexAwAnnexBCapCode:
		//G723.1
		case eG7231CapCode:
		case eG7231AnnexCapCode:
		{
			transmitFactor = 8;
			break;
		}


		//G722.1
		case eG7221_32kCapCode:
		case eG7221_24kCapCode:
		{
			transmitFactor = 16;
			break;
		}

		//Siren14
		case eSiren14_48kCapCode:
		case eSiren14_32kCapCode:
		case eSiren14_24kCapCode:
		case eSiren14Stereo_48kCapCode:
		case eSiren14Stereo_56kCapCode:
		case eSiren14Stereo_64kCapCode:
		case eSiren14Stereo_96kCapCode:
		//G722.1.C
		case eG7221C_48kCapCode:
		case eG7221C_32kCapCode:
		case eG7221C_24kCapCode:
		case eG7221C_CapCode:
		{
			transmitFactor = 32;
			break;
		}

		//G719
		case eG719_32kCapCode:
		case eG719_48kCapCode:
		case eG719_64kCapCode:
		case eG719_96kCapCode:
		case eG719_128kCapCode:
		case eG719Stereo_64kCapCode:
		case eG719Stereo_96kCapCode:
		case eG719Stereo_128kCapCode:
		//Siren22
		case eSiren22_32kCapCode:
		case eSiren22_48kCapCode:
		case eSiren22_64kCapCode:
		case eSiren22Stereo_64kCapCode:
		case eSiren22Stereo_96kCapCode:
		case eSiren22Stereo_128kCapCode:
		// SirenLPR
		case eSirenLPR_32kCapCode:
		case eSirenLPR_48kCapCode:
		case eSirenLPR_64kCapCode:
		case eSirenLPRStereo_64kCapCode:
		case eSirenLPRStereo_96kCapCode:
		case eSirenLPRStereo_128kCapCode:
		{
			transmitFactor = 48;
			break;
		}


		default:

			transmitFactor = 8;
	}

	DWORD timeToSleep = deltaTS/transmitFactor;

	//TRACEINTO << "CAudioChannel::GetTimeToNextTransmissionMl - timeToSleep= " << timeToSleep;

	return timeToSleep;
}

/////////////////////////////////////////////////////////////////////////////

//void CAudioChannel::SendMedia()
//{
//	if (GetEncryptionType() == kAES_CBC)
//	{
//		DWORD payloadSize = m_rtpPacketsSize[0] - sizeof(TRtpHeader);
//
//		int sizeMod16 = payloadSize % 16;	// packet size should be in multiples of 16 Bytes
//		int paddingSize = 0;
//
//		//unset RTP header 'P' field
//		m_rtpPacketsArr[0][0] &= 0xDF;
//
//		if (0 != sizeMod16)
//		{
//			paddingSize = 16 - sizeMod16;
//
//			//set RTP header 'P' field
//			m_rtpPacketsArr[0][0] |= 0x20;
//
//
//			//set padding cipher to 0s
//			memset(&m_rtpPacketsArr[0][sizeof(TRtpHeader) + payloadSize], '0', paddingSize);
//
//			int lastByteIndexInPacket = sizeof(TRtpHeader) + payloadSize + paddingSize -1;
//			//Set padding size in last byte of packet
//			BYTE* pLastByteInPacket = (BYTE*)&m_rtpPacketsArr[0][lastByteIndexInPacket];
//			*pLastByteInPacket = (BYTE)paddingSize;
//		}
//
//		/////////////////////////////////////////////////////
//		//Set Initial Vector values
//
//		DWORD unSeqNumber = (m_rtpPacketsArr[0][2]<<8) | m_rtpPacketsArr[0][3];
//
//		DWORD unTimeStamp = m_rtpPacketsArr[0][4]<<24 | (m_rtpPacketsArr[0][5]<<16) | (m_rtpPacketsArr[0][6]<<8) | m_rtpPacketsArr[0][7];
//
//		DWORD	punIVector[4] = {0};
//		punIVector[0] = (unSeqNumber << 16) | (unTimeStamp >> 16);
//		punIVector[1] = (unTimeStamp << 16) | (unSeqNumber & 0xFFFF);
//		punIVector[2] = unTimeStamp;
//		punIVector[3] = punIVector[0];
//
//		EncryptCBC((Uint8*)&m_rtpPacketsArr[0][sizeof(TRtpHeader)] , //encrypt only payload
//							payloadSize,
//			 				punIVector,
//			 				m_aunExpandedRoundKey);
//
//
//		//set packet size
//		m_rtpPacketsSize[0] += paddingSize;
//
//	}
//
//	//TRACEINTO << "CAudioChannel::SendMedia - packet index: 0  (Encrypted)m_rtpPacketsSize: " << (DWORD*)m_rtpPacketsArr[0];
//	SendTxSocket(m_rtpPacketsArr[0], m_rtpPacketsSize[0]);
//}


/////////////////////////////////////////////////////////////////////////////

void CAudioChannel::OnAudioOutResetMessageAll(CSegment* pParam)
{
	TRACEINTO << GetParticipantTicket() << " CAudioChannel::OnAudioOutResetMessageAll";

	m_readInd = 0;
	m_lastPacketSeqNumber = 0;
	m_lastReadInd = 0;
	m_loopCounter = 0;
}

void CAudioChannel::OnAudioOutUpdateChannelMessageAll(CSegment* pParam)
{
	TRACEINTO << GetParticipantTicket() << " CAudioChannel::OnAudioOutUpdateChannelMessageAll file name: " << m_strFileName;

	int status = SetupMediaFile();
	if( STATUS_OK != status)
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CAudioChannel::OnAudioOutUpdateChannelMessageAll - Error in SetupMediaFile.";
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
///////
//DTMF
//////
/////////////////////////////////////////////////////////////////////////////

void CAudioChannel::SendDTMF(SPlayToneStruct* tPlayToneStruct)
{
	TRACEINTO << GetParticipantTicket() << " CAudioChannel::SendDTMF";

	//check if already in DTMF session
	if (m_bIsDtmfSession == TRUE)
	{
		//copy new dtmf tones to the end of dtmf buffer array
		int i = 0;
		for (;i < (int)tPlayToneStruct->numOfTones; i++)
		{
			if (tPlayToneStruct->tone[i].tTone != E_AUDIO_TONE_SILENCE)
			{
				if (m_numOfDtmf < MAX_DTMF_LEN)
					m_dtmfBuffer[m_numOfDtmf++] = (EAudioTone)tPlayToneStruct->tone[i].tTone;
				else
					TRACEINTO << GetParticipantTicket() << " MM ERROR CAudioChannel::SendDTMF dtmf overflow, ignored: " << EAudioToneNames[tPlayToneStruct->tone[i].tTone];
			}
		}

		TRACEINTO << GetParticipantTicket() << " CAudioChannel::SendDTMF after adding dtmf to current buffer";
		PrintDtmfBuffer();

		if (m_currDtmfIndex > 0)
		{
			//re-order dtmf buffer array
			int j = m_currDtmfIndex;
			for (i = 0;i < (m_numOfDtmf-m_currDtmfIndex); i++, j++)
			{
				m_dtmfBuffer[i] = m_dtmfBuffer[j];
				m_dtmfBuffer[j] = E_AUDIO_TONE_DUMMY;
			}

			//update the number of dtmf tones in the buffer array
			m_numOfDtmf = i;

			TRACEINTO << GetParticipantTicket() << " CAudioChannel::SendDTMF after re-ordering dtmf buffer";
			PrintDtmfBuffer();
		}

		return;
	}



	//fills the m_dtmfBuffer
	m_numOfDtmf = 0;
	for (int i = 0;i < (int)tPlayToneStruct->numOfTones; i++)
	{
		if (tPlayToneStruct->tone[i].tTone != E_AUDIO_TONE_SILENCE)
		{
			if (m_numOfDtmf < MAX_DTMF_LEN)
				m_dtmfBuffer[m_numOfDtmf++] = (EAudioTone)tPlayToneStruct->tone[i].tTone;
			else
				TRACEINTO << GetParticipantTicket() << " MM ERROR CAudioChannel::SendDTMF dtmf overflow, ignored: " << EAudioToneNames[tPlayToneStruct->tone[i].tTone];
		}
	}

	PrintDtmfBuffer();


	TRACEINTO << GetParticipantTicket() << " CAudioChannel::SendDTMF m_numOfDtmf=" << m_numOfDtmf;


	m_currDtmfIndex = 0;

	CDtmfElement* dtmfElement = ::GetMediaRepository()->GetDtmfDB()->GetDtmfElement(GetCapTypeCode(), m_dtmfBuffer[m_currDtmfIndex]);
	if (dtmfElement == NULL)
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CAudioChannel::SendDTMF dtmfElement is NULL at m_dtmfBuffer index=" << m_currDtmfIndex;
		return;
	}

	if (dtmfElement->GetDtmfBufferSize() == 0)
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CAudioChannel::SendDTMF dtmfElement buffer is empty at m_dtmfBuffer index=" << m_currDtmfIndex;
		return;
	}

	TRACEINTO << GetParticipantTicket() << " CAudioChannel::SendDTMF tone: " << EAudioToneNames[m_dtmfBuffer[m_currDtmfIndex]] << " dtmf file name: " << dtmfElement->GetFullFileName();

	//save original params
	m_saveMediaFileBuffer = m_mediaFileBuffer;
	m_saveFileSize = m_fileSize;
	m_saveReadInd = m_readInd;
	m_saveLastReadInd = m_lastReadInd;
	m_saveLoopCounter = m_loopCounter;
	m_saveLastPacketSeqNumber = m_lastPacketSeqNumber;



	//entering DTMF session
	SetDtmfSession(TRUE);

	m_mediaFileBuffer = dtmfElement->GetDtmfBuffer();
	m_fileSize = dtmfElement->GetDtmfBufferSize();
	m_readInd = 0;
	m_lastReadInd = 0;
	m_loopCounter = 0;
	m_lastPacketSeqNumber = 0;

	// call prepare dtmf frame, to be transmited in the next timer - only in the FIRST dtmf session
	PrepareMediaFrame();
}

/////////////////////////////////////////////////////////////////////////////

void CAudioChannel::PrepareNextDtmf()
{
	TRACEINTO << GetParticipantTicket() << " CAudioChannel::PrepareNextDtmf";

	//increase dtmf current index
	m_currDtmfIndex++;

	//check if no more dtmf to transmit
	if (m_currDtmfIndex == m_numOfDtmf)
	{
		TRACEINTO << GetParticipantTicket() << " CAudioChannel::PrepareNextDtmf - No more dtmf to transmit. Exiting dtmf mode";

		//exiting DTMF session
		SetDtmfSession(FALSE);

		return;
	}

	//get the next dtmf to transmit
	CDtmfElement* dtmfElement = ::GetMediaRepository()->GetDtmfDB()->GetDtmfElement(GetCapTypeCode(), m_dtmfBuffer[m_currDtmfIndex]);
	if (dtmfElement == NULL)
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CAudioChannel::PrepareNextDtmf dtmfElement is NULL at m_dtmfBuffer index=" << m_currDtmfIndex
				<< " Exiting DTMF session.";

		//exiting DTMF session
		SetDtmfSession(FALSE);

		return;
	}

	if (dtmfElement->GetDtmfBufferSize() == 0)
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CAudioChannel::PrepareNextDtmf dtmfElement buffer is empty at m_dtmfBuffer index=" << m_currDtmfIndex
				<< " Exiting DTMF session.";

		//exiting DTMF session
		SetDtmfSession(FALSE);

		return;
	}

	TRACEINTO << GetParticipantTicket() << " CAudioChannel::PrepareNextDtmf tone: " << EAudioToneNames[m_dtmfBuffer[m_currDtmfIndex]] << " dtmf file name: " << dtmfElement->GetFullFileName();

	//set the params for next dtmf
	m_mediaFileBuffer = dtmfElement->GetDtmfBuffer();
	m_fileSize = dtmfElement->GetDtmfBufferSize();
	m_readInd = 0;
	m_lastReadInd = 0;
	m_loopCounter = 0;
	m_lastPacketSeqNumber = 0;
}

/////////////////////////////////////////////////////////////////////////////

void CAudioChannel::SetDtmfSession(BOOL isDtmfSession)
{
	if (isDtmfSession == TRUE)
	{
		TRACEINTO << GetParticipantTicket() << " CAudioChannel::SetDtmfSession - entering dtmf session";
		m_bIsDtmfSession = TRUE;
	}
	else
	{
		TRACEINTO << GetParticipantTicket() << " CAudioChannel::SetDtmfSession - exiting dtmf session";
		m_bIsDtmfSession = FALSE;

		//restore saved param
		m_mediaFileBuffer = m_saveMediaFileBuffer;
		m_fileSize = m_saveFileSize;
		m_readInd = m_saveReadInd;
		m_lastReadInd = m_saveLastReadInd;
		m_loopCounter = m_saveLoopCounter;
		m_lastPacketSeqNumber = m_saveLastPacketSeqNumber;

		//initialize dtmf array
		for (int i=0; i<MAX_DTMF_LEN; i++)
			m_dtmfBuffer[i] = E_AUDIO_TONE_DUMMY;

		m_numOfDtmf = 0;
		m_currDtmfIndex = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CAudioChannel::PrintDtmfBuffer()
{
	char tmp[MAX_DTMF_LEN];
	int i = 0;
	for (;i < m_numOfDtmf; i++)
		tmp[i] = ::GetTone(m_dtmfBuffer[i]);

	tmp[i] = '\0';

	TRACEINTO << GetParticipantTicket() << " CAudioChannel::PrintDtmfBuffer current dtmf buffer={" << tmp << "}";
}
