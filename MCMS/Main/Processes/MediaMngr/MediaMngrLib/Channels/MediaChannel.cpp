#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#include "MediaChannel.h"
//#include "VideoChannel.h"
#include "AudioChannel.h"
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

#ifndef __DISABLE_ICE__
#include "IceSocket.h"
#endif 	//__DISABLE_ICE__



extern void SendMessageToGideonSimApp(CSegment& rParam);




/*extern "C"
{
	#include "AESLib.h"
}*/

#if !defined(FAR)
	#define FAR
#endif /* INT8 */

#if !defined(TYPE_Uint8)
	typedef unsigned char Uint8, *pUint8;
	#define TYPE_Uint8
#endif /* UINT8 */

#if !defined(TYPE_Uint32)
	typedef unsigned int Uint32, *pUint32;
	#define TYPE_Uint32
#endif /* UINT8 */


extern "C"
{
	FAR void EncryptCBC(Uint8 *pucPayload, Uint32 unByteCntr, Uint32 *punIVector, Uint32 *punExpRoundKeyStartPtr);
	FAR void DecryptCBC(Uint8 *pucPayload, Uint32 unByteCntr, Uint32 *punIVector, Uint32 *punExpRoundKeyStartPtr);

	FAR void RijndaelKeySetupEnc(Uint32 *punExpandedRoundKey, const Uint8 *pucCipherKey);
	FAR void RijndaelKeySetupDec(Uint32	*punExpandedRoundKey, const Uint8 *pucCipherKey);

	FAR void InitEncryptionTables();
}




////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////////   CMediaChannel
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//					MESSAGE_MAP
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CMediaChannel)

	ONEVENT( LIST_EVENT_MESSAGE,				ANYCASE,   CMediaChannel::OnListEventMessageAll)
	ONEVENT( ACTIVATE_EVENT_MESSAGE,			ANYCASE,   CMediaChannel::OnActivateEventMessageAll)
//	ONEVENT( CHANNEL_IN_PARAM_EVENT_MESSAGE,	ANYCASE,   CMediaChannel::OnChannelInParamEventMessageAll)

PEND_MESSAGE_MAP(CMediaChannel,CStateMachine);



CMediaChannel::CMediaChannel() 
	: CStateMachine(),
	m_pSocketOut(0),
	m_pSocketIn(0),
	m_iceRtpChannelID(0)
{

}

CMediaChannel::CMediaChannel(CTaskApp* pOwnerTask, INT32 channelDirection) 
	: CStateMachine(pOwnerTask),
	m_pSocketOut(0),
	m_pSocketIn(0),
	m_iceRtpChannelID(0)
{
	//init parameters
	m_channelDirection = channelDirection;
	m_capTypeCode = eUnknownAlgorithemCapCode;
	m_bitRate = 0;
	m_payloadType = _UnKnown;
	m_dtmfPayloadType = _UnKnown;
	m_isH263Plus = 0;
	m_annexesMask = 0;
	m_customMaxMbpsValue = 0;
	m_maxFramesPerPacket = 0;
	m_eVideoProtocol = E_VIDEO_PROTOCOL_DUMMY;

	memset((void*) &m_taLocalAddress, 0, sizeof(mcTransportAddress));
	memset((void*) &m_taRemoteAddress, 0, sizeof(mcTransportAddress));

	memset((void *)&m_local_addr, '\0', sizeof(m_local_addr));
	memset((void *)&m_remote_addr, '\0', sizeof(m_remote_addr));

	memset(m_recvBuffer, 0, sizeof(TMediaHeaderIpNetwork));


	m_pSocketOut = NULL;
	m_pSocketIn = NULL;

//	m_bWriteFlag = FALSE;
	//Incoming channel params
	m_tIncomingChannelParam.bReadFlag = FALSE;
	m_tIncomingChannelParam.bCheckSeqNumber = FALSE;
	m_tIncomingChannelParam.bIntraSeqNumber = FALSE;
	m_tIncomingChannelParam.bDetectIntra = FALSE;
	m_tIncomingChannelParam.bCheckTimeStamp = FALSE;
	m_tIncomingChannelParam.bIntraTimeStamp = FALSE;
	m_tIncomingChannelParam.bDecryptMedia = FALSE;
	m_tIncomingChannelParam.bWriteMedia = FALSE;
	m_tIncomingChannelParam.eHeaderType = E_MM_NO_HEADER;

	m_1stIntraPacketWasDetected = FALSE;
	m_2ndIntraPacketWasDetected = FALSE;
	m_3rdIntraPacketWasDetected = FALSE;
	m_h263HeaderSize = -1;

	m_pFile = NULL;
	m_sFullFileName = "";
	m_mediaBuffer = NULL;
	m_mediaBufferIndex = 0;
	m_writtenFileSize = 0;
	m_frameBufferIndex = 0;


	//list of event messages
	for (int i = 0; i < MAX_EVENT_MESSAGE_STACK_SIZE; i++)
	{
		m_pEventMessageArr[i] = NULL;
	}


	m_bOpenUdpPort = FALSE;
	m_bRtpUpdatePort = FALSE;
	m_bVideoOutParam = FALSE;

	m_bIsChannelOpen = FALSE;


	//media
	m_mediaFileBuffer = NULL;
	m_fileSize = 0;
	m_readInd = 0;
	m_firstConnection = 0;
	m_lastReadInd = 0;
	m_loopCounter = 0;
	m_saveLastReadInd = 0;
	m_saveLoopCounter = 0;

	m_mediaRepositoryElement = NULL;

	m_packetSeqNumber = 0;
	m_lastPacketSeqNumber = 0;
	m_saveLastPacketSeqNumber = 0;

	// time stamp variables
	m_currentFrameTS = 0;	// the current base time stamp (TS retrieved from media file)
	m_prevUsedTS = 0;		// the last used TS - as sent for the last frame (TS calculated for transmission)
	m_prevBaseTS = 0;		// the last base TS - the one of the last frame (TS retrieved from media file)
	m_eofTsFactor = 0;		// for time stamps separation between the last frame in file and the first one of the next loop

///	m_firstBufferTS = 0;	// the first TS of the first packet in this call as appears in file

	m_strFileName = "";

	m_timeToNextTransmission = 0;
	m_numOfPacketsForTransmission = 0;

	m_fraction = 0;

	m_bIsDtmfSession = FALSE;

	for (int i = 0; i < MAX_PACKETS_PER_FRAME; i++)
	{
		memset(m_rtpPacketsArr, 0, MAX_PACKET_SIZE_ENCRYPTION);	//set all buffer to 0s
		m_rtpPacketsSize[i] = 0;
	}


	//m_bIsFirstLoopOnFile = TRUE;

	m_participantTicket = "";

	m_lastRxPacketSeqNumber = 0;

	m_mediaLibrary = NULL;

	//Encryption
	m_unEncryptionType = kUnKnownMediaType;
	memset(m_aucSessionKey, '0', sizeOf128Key);
	memset(m_aunExpandedRoundKey, '0', NUM_WORDS_FOR_EXPANDED_KEY);

	// save parameters for Intra
	m_boardId 			= (DWORD)-1;
	m_subBoardId 		= (DWORD)-1;
	m_unitId 			= (DWORD)-1;
	m_conferenceId 		= (DWORD)-1;
	m_partyId 			= (DWORD)-1;
	m_connectionId 		= (DWORD)-1;

}


CMediaChannel::~CMediaChannel()
{
	for (int i = 0; i < MAX_EVENT_MESSAGE_STACK_SIZE; i++)
		POBJDELETE(m_pEventMessageArr[i]);
}

/////////////////////////////////////////////////////////////////////////////
//					GetMessageMap
/////////////////////////////////////////////////////////////////////////////
void*  CMediaChannel::GetMessageMap()
{
  return (void*)m_msgEntries;
}


/////////////////////////////////////////////////////////////////////////////
//					NameOf
/////////////////////////////////////////////////////////////////////////////
const char*  CMediaChannel::NameOf() const
{
	return "CMediaChannel";
}



/////////////////////////////////////////////////////////////////////////////
//					HandleEvent
/////////////////////////////////////////////////////////////////////////////
void  CMediaChannel::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	switch ( opCode )
	{

		default:
		{         // all other messages
			DispatchEvent(opCode, pMsg);
			break;
		}
	}

}

//////////////////////////////////////////

void  CMediaChannel::UpdateMediaChannelParams( CSegment* pParam )
{
	*pParam >> m_boardId;
	*pParam >> m_subBoardId;
	*pParam >> m_unitId;
	*pParam >> m_conferenceId;
	*pParam >> m_partyId;
	*pParam >> m_connectionId;
}

//////////////////////////////////////////

string CMediaChannel::ChannelData()
{
	string str = "";
	str += "\nChannel Name: ";
	str += NameOf();

	str += "\nDirection: ";
	str += GetChannelDirectionStr();

	str += "\nCapCodeType: ";
	str += GetCapTypeCodeStr();

	str += "\nBitRate: ";
	str += GetBitRateStr();

	str += "\nPayloadType: ";
	str += GetPayloadTypeStr();

	str += "\nDtmfPayloadType: ";
	str += GetDtmfPayloadTypeStr();

	str += "\nIsH263Plus: ";
	str += GetIsH263PlusStr();

	str += "\nAnnexesMask: ";
	str += GetAnnexesPlusStr();

	str += "\nCustomMaxMbps: ";
	str += GetCustomMaxMbpsValueStr();

	str += "\nMaxFramesPerPacket: ";
	str += GetMaxFramesPerPacketStr();

	//Encryption
	str += "\nEncryptionType: ";
	str += GetEncryptionTypeStr();

	str += "\nEncryptionSessionKey: ";
	str += GetEncryptionSessionKeyStr();


	str += "\n";

	return str;
}



int CMediaChannel::GetTimeToNextTransmission(DWORD timeStamp)
{
	return 0;
}

DWORD CMediaChannel::GetTimeToNextTransmissionMl(DWORD timeStamp)
{
	return 0;
}

//////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// received message from EPConnectionObj
/////////////////////////////////////////////////////////////////////////////
void CMediaChannel::OnListEventMessageAll(CSegment* pParam)
{
	//TRACEINTO << "CMediaChannel::OnListEventMessageAll";

	char  sOpCode[256];
	OPCODE opCode;

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	*pParam	>> sOpCode
			>> opCode;
	pMplProtocol->DeSerialize(*pParam);

	//TRACEINTO << "CMediaChannel::OnListEventMessageAll sOpCode:" << sOpCode;

	const string sOpCodeStr = sOpCode;

	ListEventMessage(sOpCodeStr, opCode, pMplProtocol);

	POBJDELETE(pMplProtocol);
}


//////////////////////////////////////////

void CMediaChannel::OnActivateEventMessageAll(CSegment* pParam)
{
	//TRACEINTO << "CMediaChannel::OnActivateEventMessageAll";

	char  sOpCode[256];
	OPCODE opCode;

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	*pParam	>> sOpCode
			>> opCode;
	pMplProtocol->DeSerialize(*pParam);

	//TRACEINTO << "CMediaChannel::OnActivateEventMessageAll sOpCode:" << sOpCode;

	const string sOpCodeStr = sOpCode;

	ActivateEventMessage(sOpCodeStr, opCode, pMplProtocol);

	POBJDELETE(pMplProtocol);
}



//////////////////////////////////////////
/*
void CMediaChannel::OnChannelInParamEventMessageAll(CSegment* pMsgIncomingParams)
{
	*pMsgIncomingParams	>> m_tIncomingChannelParam.bReadFlag
					>> m_tIncomingChannelParam.bCheckSeqNumber
					>> m_tIncomingChannelParam.bIntraSeqNumber
					>> m_tIncomingChannelParam.bCheckTimeStamp
					>> m_tIncomingChannelParam.bIntraTimeStamp
					>> m_tIncomingChannelParam.bDecryptMedia
					>> m_tIncomingChannelParam.bWriteMedia;
//					>> (DWORD)m_tIncomingChannelParam.eHeaderType;

}
*/


//////////////////////////////////////////

BOOL CMediaChannel::ListEventMessage(const string opCodeStr, OPCODE opCode, CMplMcmsProtocol* pMplProtocol)
{
	TEventMessage* pEventMessage = new TEventMessage();
	pEventMessage->str_event_opcode = opCodeStr;
	pEventMessage->event_opcode = opCode;
	pEventMessage->mcms_protocol = pMplProtocol;

	for (int i = 0; i < MAX_EVENT_MESSAGE_STACK_SIZE; i++)
	{
		if (m_pEventMessageArr[i] == NULL)
		{
			m_pEventMessageArr[i] = pEventMessage;

			//TRACEINTO << "CMediaChannel::ListEventMessage opcode:" << opCodeStr << " (" << (int)opCode <<  ")";
			return TRUE;
		}
	}
	return FALSE;
}

//////////////////////////////////////////

void CMediaChannel::ActivateEventMessage(const string opCodeStr, OPCODE opCode, CMplMcmsProtocol* pMplProtocol)
{
	//TRACEINTO << "CMediaChannel::ActivateEventMessage opcode:" << opCodeStr << " (" << (int)opCode <<  ")";

	switch (opCode)
	{
		case CONFPARTY_CM_OPEN_UDP_PORT_REQ:
		case SIP_CM_OPEN_UDP_PORT_REQ:
		{
			TOpenUdpPortOrUpdateUdpAddrMessageStruct* pStruct = (TOpenUdpPortOrUpdateUdpAddrMessageStruct*)pMplProtocol->GetData();

			//Get CG address
			m_taLocalAddress = pStruct->tCmOpenUdpPortOrUpdateUdpAddr.CmLocalUdpAddressIp; //CG
			//Get EP address
			m_taRemoteAddress = pStruct->tCmOpenUdpPortOrUpdateUdpAddr.CmRemoteUdpAddressIp; //EP

			m_iceRtpChannelID=pStruct->tCmOpenUdpPortOrUpdateUdpAddr.ice_channel_rtp_id;

			if (m_channelDirection == MEDIA_DIRECTION_OUT)
			{
				PrepareMediaOutAddressing();
			}
			else if (m_channelDirection == MEDIA_DIRECTION_IN)
			{
				PrepareMediaInAddressing();
			}

			break;
		}

		case H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:
		case SIP_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:
		{
			SaveChannelParams(pMplProtocol);
			break;
		}

		case H323_RTP_UPDATE_CHANNEL_REQ:
		case SIP_RTP_UPDATE_CHANNEL_REQ:
		{
			TUpdateRtpChannelReq* pStruct = (TUpdateRtpChannelReq*)pMplProtocol->GetData();

			break;
		}

		//closing channel
		case CONFPARTY_CM_CLOSE_UDP_PORT_REQ:
		case SIP_CM_CLOSE_UDP_PORT_REQ:
		{
			if ((0 == (strcmp(NameOf(), "CVideoChannel"))) ||
				(0 == (strcmp(NameOf(), "CAudioChannel"))))    // ||		(GetTaskName() == "ContentChannel"))
			{
				CloseChannel();
			}
			return;
		}

		//intra
		case VIDEO_FAST_UPDATE_REQ:
		{
			RestartMediaTx();
			break;
		}


		//dtmf
		case AUDIO_PLAY_TONE_REQ:
		{
			SPlayToneStruct* tPlayToneStruct = (SPlayToneStruct*)pMplProtocol->getpData();
			((CAudioChannel*)this)->SendDTMF(tPlayToneStruct);
			return;
		}

		//content - on/off
		case ART_CONTENT_ON_REQ:
		case ART_CONTENT_OFF_REQ:
		{
			TContentOnOffReq* pContentStruct = (TContentOnOffReq*)pMplProtocol->GetData();
			APIU32 contentMode = pContentStruct->bunIsOnOff;
			TRACEINTO << GetParticipantTicket() << " CMediaChannel::ActivateEventMessage contentMode: " << contentMode;

			//content ON
			if (m_channelDirection==MEDIA_DIRECTION_OUT && contentMode==1)
			{
				StartMediaTx();
			}
			//content OFF
			else if (m_channelDirection==MEDIA_DIRECTION_OUT && contentMode==0)
			{
				StopMediaTx();
			}

			return;
		}

		//Video update param
///		case VIDEO_ENCODER_UPDATE_PARAM_REQ:
///		{
///			ENCODER_PARAM_S* pEncoderParamsStruct = (ENCODER_PARAM_S*)pMplProtocol->getpData();
///			((CVideoChannel*)this)->EncoderUpdateVideoParam(pEncoderParamsStruct);
///			return;
///		}
	}


	//start Tx or Rx
	if (m_bRtpUpdatePort && m_bOpenUdpPort && (0 == m_firstConnection))
	{
		m_firstConnection = 1;


		// Start Media Tx for Video Out will start when Video Param arrive !!! - after TB_MSG_OPEN_PORT_REQ arrives!!
		if (m_channelDirection == MEDIA_DIRECTION_OUT && (0 == (strcmp(NameOf(), "CVideoChannel"))))
		{
			if (m_bVideoOutParam)
				StartMediaTx();
			else
				return;
		}


		if ((0 == (strcmp(NameOf(), "CVideoChannel"))) || (0 == (strcmp(NameOf(), "CAudioChannel"))))
		{
			// start Tx process
			if (m_channelDirection == MEDIA_DIRECTION_OUT)
			{
				StartMediaTx();
			}
			// start Rx process
			else if (m_channelDirection == MEDIA_DIRECTION_IN)
			{
				StartMediaRx();
			}
		}
	}


}


////////////////////////////////////////////////////////

void CMediaChannel::PrepareMediaOutAddressing()
{
	if (m_taRemoteAddress.ipVersion == eIpVersion4)
	{
		char szRemoteIP[IP_ADDRESS_LEN];
		::SystemDWORDToIpString(m_taRemoteAddress.addr.v4.ip, szRemoteIP);
		TRACEINTO << GetParticipantTicket() << " CMediaChannel::PrepareMediaOutAddressing remoteIP:" << szRemoteIP << " remotePort: " << m_taRemoteAddress.port << "  Name: " << NameOf();

		//set destination address
		//////////////////////////
		m_remote_addr.sin_family      = AF_INET;
		m_remote_addr.sin_port        = htons(m_taRemoteAddress.port);
		m_remote_addr.sin_addr.s_addr = inet_addr(szRemoteIP);

		//ok for test
		//m_remote_addr.sin_port        = htons(5555);
		//m_remote_addr.sin_addr.s_addr = inet_addr("172.22.192.26");

		m_bOpenUdpPort = TRUE;
	}
	else
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::PrepareMediaOutAddressing - Local address is NOT ip ver 4." << "  Name: " << NameOf();
	}
}

//////////////////////////////////////////

void CMediaChannel::PrepareMediaInAddressing()
{
	if (m_taLocalAddress.ipVersion == eIpVersion4)
	{
		char szLocalIP[IP_ADDRESS_LEN];
		::SystemDWORDToIpString(m_taLocalAddress.addr.v4.ip, szLocalIP);
		TRACEINTO << GetParticipantTicket() << " CMediaChannel::PrepareMediaInAddressing localIp:" << szLocalIP << " localPort: " << m_taLocalAddress.port << "   Name: " << NameOf();

		//set local address
		//////////////////////////
		m_local_addr.sin_family      = AF_INET;
		m_local_addr.sin_port        = htons(m_taLocalAddress.port);
		m_local_addr.sin_addr.s_addr = inet_addr(szLocalIP);

		//ok for test
		//m_local_addr.sin_port        = htons(5555);
		//m_local_addr.sin_addr.s_addr = htonl(INADDR_ANY);


		m_bOpenUdpPort = TRUE;

	}
	else
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::PrepareMediaInAddressing - Remote address is NOT ip ver 4. " << "  Name: " << NameOf();
	}
}


//////////////////////////////////////////

void CMediaChannel::SaveChannelParams(CMplMcmsProtocol* pMplProtocol)
{
	TUpdatePortOpenRtpChannelReq* pStruct = (TUpdatePortOpenRtpChannelReq*)pMplProtocol->GetData();

	m_capTypeCode = (CapEnum)pStruct->unCapTypeCode;
	m_bitRate = pStruct->tUpdateRtpSpecificChannelParams.unBitRate;
	m_payloadType = pStruct->tUpdateRtpSpecificChannelParams.unPayloadType;
	m_dtmfPayloadType = pStruct->tUpdateRtpSpecificChannelParams.unDtmfPayloadType;
	m_isH263Plus = pStruct->tUpdateRtpSpecificChannelParams.bunIsH263Plus;
	m_annexesMask = pStruct->tUpdateRtpSpecificChannelParams.unAnnexesMask;
	m_customMaxMbpsValue = pStruct->tUpdateRtpSpecificChannelParams.unCustomMaxMbpsValue;
	m_maxFramesPerPacket = pStruct->tUpdateRtpSpecificChannelParams.unMaxFramesPerPacket;

	//Encryption
	m_unEncryptionType = (EenMediaType)pStruct->unEncryptionType;
	if (GetEncryptionType() == kAES_CBC)
	{
		TRACEINTO << "CMediaChannel::SaveChannelParams - encrypted channel";
		InitEncryptionTables();
		memcpy(m_aucSessionKey, pStruct->aucSessionKey, sizeOf128Key);
		RijndaelKeySetupEnc(m_aunExpandedRoundKey, m_aucSessionKey);
	}

	m_bRtpUpdatePort = TRUE;
}


//////////////////////////////////////////

void CMediaChannel::CloseChannel()
{
	if (!m_bIsChannelOpen)
	{
		TRACEINTO << GetParticipantTicket() << " CMediaChannel::CloseChannel channel is signed as closed " << m_channelDirection << "  Name: " << NameOf();
		return;
	}

	if (m_channelDirection == MEDIA_DIRECTION_OUT)
	{
		TRACEINTO << GetParticipantTicket() << " CMediaChannel::CloseChannel Tx start " << "  Name: " << NameOf();

		CloseTxSocket();

		m_bIsChannelOpen = FALSE;

		TRACEINTO << GetParticipantTicket() << " CMediaChannel::CloseChannel Tx end  " << "  Name: " << NameOf();
	}
	else if (m_channelDirection == MEDIA_DIRECTION_IN)
	{
		TRACEINTO << GetParticipantTicket() << " CMediaChannel::CloseChannel Rx start " << "  Name: " << NameOf();

		CloseRxSocket();

		m_bIsChannelOpen = FALSE;

		TRACEINTO << GetParticipantTicket() << " CMediaChannel::CloseChannel Rx end  " << "  Name: " << NameOf();
	}
}

////////////////////////////////////////////////////////



////////////////////////////////////////////////////////
//				Media Sockets Tx
////////////////////////////////////////////////////////


void CMediaChannel::StartMediaTx()
{
	DumpEventMessageList();

	// get media name and load to repository (if not exists) and set local pointer to data
	int status = SetupMediaFile();
	if( STATUS_OK != status)
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::StartMediaTx - Error in SetupMediaFile.  Name: " << NameOf();
		return;
	}

	//socket init
	CreateTxSocket();
	//InitTxSocket();


	//media processing
	PrepareMediaFrame();

	m_bIsChannelOpen = TRUE;
}

////////////////////////////////////////////////////////


void CMediaChannel::StopMediaTx()
{
	m_readInd = 0;
	m_lastPacketSeqNumber = 0;

	if (m_mediaRepositoryElement != NULL)
		m_mediaRepositoryElement->DecRefCounter();
}

////////////////////////////////////////////////////////


void CMediaChannel::CreateTxSocket()
{
	TRACEINTO << GetParticipantTicket() << " CMediaChannel::CreateTxSocket " << "  Name: " << NameOf();

	if(m_pSocketOut) {
		delete m_pSocketOut;
		m_pSocketOut=0;
	}
	
	if(m_iceRtpChannelID) {
#ifndef __DISABLE_ICE__
		m_pSocketOut = new IceSocket(m_iceRtpChannelID);
#endif
	}
	else {
		m_pSocketOut = new CUdpSocket();

		if (m_pSocketOut != NULL) {
			TRACEINTO << GetParticipantTicket() << " CMediaChannel::CreateTxSocket done " << "  Name: " << NameOf();
		}
		else {
			TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::CreateTxSocket m_pSocketOut is NULL " << "  Name: " << NameOf();
			return;
		}

	/*	int fDescriptor = m_pSocketOut->CreateUdpSocket();

		if (fDescriptor == STATUS_ERROR)
		{
			TRACEINTO << GetParticipantTicket() << " MM ERROR  CMediaChannel::CreateTxSocket - CreateUdpSocket() ERROR " << "  Name: " << NameOf();
			return;
		}*/
	}

}

////////////////////////////////////////////////////////
#if 0
void CMediaChannel::InitTxSocket()
{
	TRACEINTO << GetParticipantTicket() << " CMediaChannel::InitTxSocket " << "  Name: " << NameOf();

	int retVal = STATUS_ERROR;
	if (m_pSocketOut != NULL)
	{
		retVal = m_pSocketOut->InitUdpSocket();
		if (retVal == STATUS_ERROR)
		{
			TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::InitTxSocket. InitUdpSocket Failed " << "  Name: " << NameOf();
			return;
		}
	}
	else
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::InitTxSocket m_pSocketOut is NULL " << "  Name: " << NameOf();
		return;
	}

	TRACEINTO << GetParticipantTicket() << " CMediaChannel::InitTxSocket done Name: " << NameOf();
	return;
}
#endif
////////////////////////////////////////////////////////

void CMediaChannel::SendTxSocket()
{
	const char* buffer = "##########   TEST UDP TX   ###########";

	SendTxSocket(buffer);
}

////////////////////////////////////////////////////////

void CMediaChannel::SendTxSocket(const char* buffer)
{
	if (m_pSocketOut != NULL)
	{
		m_pSocketOut->SendTo(buffer, (struct sockaddr*)&m_remote_addr, strlen(buffer) );
	}
	else
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::SendTxSocket - m_pSocketOut is NULL " << " Name: " << NameOf();
	}
}

////////////////////////////////////////////////////////

void CMediaChannel::SendTxSocket(const BYTE* buffer, DWORD size)
{
	if (m_pSocketOut != NULL)
	{
		m_pSocketOut->SendTo((const char*)buffer, (struct sockaddr*)&m_remote_addr, size);
	}
	else
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::SendTxSocket - m_pSocketOut is NULL " << " Name: " << NameOf();
	}
}

////////////////////////////////////////////////////////


void CMediaChannel::CloseTxSocket()
{
	TRACEINTO << GetParticipantTicket() << " CMediaChannel::CloseTxSocket " << "  Name: " << NameOf();

	if (m_pSocketOut != NULL)
	{
		/*int retVal = STATUS_ERROR;
		retVal = m_pSocketOut->CloseUdpSocket();

		if (retVal == STATUS_ERROR)
		{
			TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::CloseTxSocket. CloseUdpSocket Failed " << "  Name: " << NameOf();
			return;
		}*/

		POBJDELETE(m_pSocketOut);


		TRACEINTO << GetParticipantTicket() << " CMediaChannel::CloseTxSocket - done " << "  Name: " << NameOf();
	}
	else
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::CloseTxSocket - m_pSocketOut is NULL " << "  Name: " << NameOf();
	}
}


////////////////////////////////////////////////////////


void CMediaChannel::DumpEventMessageList()
{
	string str = "CMediaChannel::DumpEventMessageList:" + ChannelData();
	str += "\n";
	for (int i = 0; i < MAX_EVENT_MESSAGE_STACK_SIZE; i++)
	{
		if (m_pEventMessageArr[i] != NULL)
		{
			char indexStr[20];
			sprintf(indexStr ,"%d", i);
			string s = indexStr;
			string tmp =  "index:" + s;
			tmp += " data: ";
			tmp += m_pEventMessageArr[i]->str_event_opcode;
			str += tmp;
			str += "\n";
		}
	}
	TRACEINTO << GetParticipantTicket() << " " <<  str;
}


////////////////////////////////////////////////////////


void CMediaChannel::OnTimerSendMedia()
{
	if (m_bIsChannelOpen == FALSE)
		return;

	//decrease timer by (10 milli sec) = (1/100 sec)
	m_timeToNextTransmission = m_timeToNextTransmission - 10;

	if (m_timeToNextTransmission < 10)
	{
		//TRACEINTO << "CMediaChannel::OnTimerSendMedia sending... m_timeToNextTransmission=" << m_timeToNextTransmission << " name: " << NameOf();

		// send all packets of current frame
		SendMedia();

		// prepare next frame -> to be sent on the next timer
		PrepareMediaFrame();
	}
}

////////////////////////////////////////////////////////

void CMediaChannel::OnTimerRecvMedia()
{
	if (m_bIsChannelOpen == FALSE)
		return;

//	if (m_bWriteFlag)
	if (m_tIncomingChannelParam.bReadFlag)
	{
		//Receive Socket
		ReceiveRxSocket();
	}
}


////////////////////////////////////////////////////////


void CMediaChannel::PrepareMediaFrame()
{
	BYTE tempCmHeader[sizeof(TMediaHeaderIpNetwork)];	// 40 bytes
	BYTE tempRtpHeader[sizeof(TRtpHeader)];	// 12 bytes

	// TS retrieved from file
	DWORD bigEndTimeStamp = 0;
	DWORD litEndTimeStamp = 0;
	// TS calculated for transmission
	DWORD litEndTmpTS = 0;
	DWORD bigEndTmpTS = 0;
	// variables to support TS handling
	DWORD savedLastUsedTS = m_prevUsedTS;
	BOOL firstFrameNotFirstLoop = FALSE;

	int packetIndexInFrame = 0;
	m_numOfPacketsForTransmission = 0; // reset also at the end of SendMedia() - needed here when the prepare is called twice without sending media (update)
	int increaseIndexParam = 0;		// for packet padding (size should be in multiples of 4Byte)

	BYTE packetMarker = 0;
	WORD savedPacketSeqNumber = m_packetSeqNumber;


	if ((m_readInd == 0) && (m_packetSeqNumber > 0))	// this is the beginning of the file but not the beginning of media transmission for this channel
	{
		firstFrameNotFirstLoop = TRUE;
	}


	do
	{
		//check if index is bigger than file size
		if (m_readInd > m_fileSize)
		{
			TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::PrepareMediaFrame - m_readInd = " << m_readInd << " is bigger than m_fileSize = " << m_fileSize;
			m_readInd = 0;
			m_lastPacketSeqNumber = 0;
		}

		//check if packet index in frame has reached the limit
		if (packetIndexInFrame == MAX_PACKETS_PER_FRAME)
		{
			TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::PrepareMediaFrame - packetIndexInFrame = " << MAX_PACKETS_PER_FRAME;
			return;
		}

		//save last readInd
		m_lastReadInd = m_readInd;

		// get CM header (size: 40 Bytes)
		memcpy(tempCmHeader, &m_mediaFileBuffer[m_readInd], sizeof(TMediaHeaderIpNetwork));

		//set RTP packet size
		m_rtpPacketsSize[packetIndexInFrame] = ((TMediaHeaderIpNetwork*)(tempCmHeader))->tCommonHeader.nPayloadSize;

		//check current packet size limit
		if (m_rtpPacketsSize[packetIndexInFrame] > MAX_PACKET_SIZE)
		{
			TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::PrepareMediaFrame - RTP packet size = " << m_rtpPacketsSize[packetIndexInFrame] << " is bigger than " << MAX_PACKET_SIZE;
			return;
		}

		// increase readIndex to point after the CM header
		m_readInd = m_readInd + sizeof(TMediaHeaderIpNetwork);

		// get current packet (size: payloadSize)
		memcpy(m_rtpPacketsArr[packetIndexInFrame], &m_mediaFileBuffer[m_readInd], m_rtpPacketsSize[packetIndexInFrame]);
		increaseIndexParam = 0;
		int sizeMod4 = m_rtpPacketsSize[packetIndexInFrame] % 4;	// packet size should be in multiples of 4Bytes
		if (0 != sizeMod4)		// if not divided by 4B without remainder - round off up
			increaseIndexParam = 4 - sizeMod4;

		// increase index to point after current packet
		m_readInd = m_readInd + m_rtpPacketsSize[packetIndexInFrame] + increaseIndexParam;


		/////////////////////////////////////////////////////
		//Time stamp
		// get time stamp from buffer (base value from file)
		memcpy(tempRtpHeader, &m_rtpPacketsArr[packetIndexInFrame][0], sizeof(TRtpHeader));
		bigEndTimeStamp = ((TRtpHeader*)(tempRtpHeader))->unTimeStamp;

		// convert DWORD endian-ness for time stamp
		litEndTimeStamp = (bigEndTimeStamp>>24) | (((bigEndTimeStamp>>16) & 0xFF)<<8) | (((bigEndTimeStamp>>8) & 0xFF)<<16) | ((bigEndTimeStamp & 0xFF)<<24);
		if (packetIndexInFrame == 0)	// this is the first packet of this frame
			m_currentFrameTS = litEndTimeStamp;	// save the current base time stamp (as appears in the media file)

		// calculate the new time stamp for transmission (the value that should be set in the sent packets)
		if (packetIndexInFrame == 0)	// calculate only for first packet in the frame
		{
			if (0 == m_packetSeqNumber)		///(m_firstBufferTS == 0)
			{	// in case this is the first frame transmitted in this call
				// TS = base TS (from file)
				litEndTmpTS = litEndTimeStamp;
				//happens only once at beginning of buffer transmission
				///m_firstBufferTS = litEndTmpTS;
			}
			else if (firstFrameNotFirstLoop)
			{	// in case this is the first frame of the file but not the first one of this call:
				// TS = prev sent TS + EOF factor (for separation between the last frame in file and the first one of the next loop)
				litEndTmpTS = m_prevUsedTS + m_eofTsFactor;
			}
			else
			{	// in case this is not the first frame in file:
				// TS = prev sent TS + [current base TS (from file) - prev base TS (from file)]
				litEndTmpTS = m_prevUsedTS + litEndTimeStamp - m_prevBaseTS;
			}

			// save parameters for next frame:
			// the last used TS (TS for transmission)
			m_prevUsedTS = litEndTmpTS;
			// the last base time stamp (TS as appears in file)
			m_prevBaseTS = m_currentFrameTS;	// This is the first packet TS in array of packets that was used for current frame

			// convert litEndTmpTS into big endian
			bigEndTmpTS = (litEndTmpTS>>24) | (((litEndTmpTS>>16) & 0xFF)<<8) | (((litEndTmpTS>>8) & 0xFF)<<16) | ((litEndTmpTS & 0xFF)<<24);
		}

		// set TS value in packets array for transmission
		//memcpy(&m_rtpPacketsArr[packetIndexInFrame][4], &bigEndTmpTS, sizeof(DWORD));
		DWORD* ptrTS = (DWORD*)&m_rtpPacketsArr[packetIndexInFrame][4];
		*ptrTS = bigEndTmpTS;

		/////////////////////////////////////////////////////




		////////////////////////////////////////////////
		//Payload type - set payload type from signaling
		BYTE payloadTypeMarker = ((TRtpHeader*)(tempRtpHeader))->markerPlusPayloadType;
		packetMarker = payloadTypeMarker & 0x80;
		payloadTypeMarker = (m_payloadType & 0xFF) | packetMarker;
		//memcpy(&m_rtpPacketsArr[packetIndexInFrame][1], &payloadTypeMarker, sizeof(BYTE));
		m_rtpPacketsArr[packetIndexInFrame][1] = payloadTypeMarker;
		/////////////////////////////////////////////////////


		////////////////////////////////////////////////////////////////
		//Check sequential packet numbering - only on first loop on file
		if (/*m_bIsFirstLoopOnFile == TRUE &&*/ (litEndTimeStamp == m_currentFrameTS))
		{
			UINT16 bigOrgSeqNumber = ((TRtpHeader*)(tempRtpHeader))->usSequenceNumber;
			UINT16 curPacketSeqNumber = (bigOrgSeqNumber>>8) | (bigOrgSeqNumber<<8);
			//TRACEINTO << GetParticipantTicket() << " CMediaChannel::PrepareMediaFrame - curPacketSeqNumber = " << curPacketSeqNumber;


			if (m_lastPacketSeqNumber != 0)
			{
				if ((curPacketSeqNumber - m_lastPacketSeqNumber) != 1)
				{
					CLargeString description;
					description << GetParticipantTicket()
								<< " MM ERROR CMediaChannel-Mismatch packet number-"
					            << " curSeqNum=" << curPacketSeqNumber
					            << " lastSeqNum=" << m_lastPacketSeqNumber
					            << " sentSeqNum=" << m_packetSeqNumber
					            << " m_readInd=" << m_readInd
					            << " m_lastReadInd=" << m_lastReadInd
					            << " m_loopCounter=" << m_loopCounter
					            << " file=" << m_strFileName;

					TRACEINTO << description.GetString();

					CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
										777,
										SYSTEM_MESSAGE,
										description.GetString() ,
										FALSE);

					//m_bIsFirstLoopOnFile = FALSE;
				}
			}
			m_lastPacketSeqNumber = curPacketSeqNumber;
		}
		////////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////
		//Set Sequence number
		//convert seq number into big endian
		WORD convertedSeqNumber = (m_packetSeqNumber >> 8) | ((m_packetSeqNumber & 0xFF) << 8);
		//memcpy(&m_rtpPacketsArr[packetIndexInFrame][2], &convertedSeqNumber, sizeof(WORD));
		WORD* ptrSeqNumber = (WORD*)&m_rtpPacketsArr[packetIndexInFrame][2];
		*ptrSeqNumber = convertedSeqNumber;
		/////////////////////////////////////////////////////




		/////////////////////////////////////////////////////
		//counts the number of packets in the current frame
		m_numOfPacketsForTransmission++;
		// preparation for next packet
		packetIndexInFrame++;
		m_packetSeqNumber++;


		//check if end of file and packet is without marker -> drop frame
		if (m_readInd >= m_fileSize)
		{
			if (0 == (strcmp(NameOf(), "CVideoChannel")))
			{
				if (packetMarker==0 && (litEndTimeStamp==m_currentFrameTS))
				{
					TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::PrepareMediaFrame - last frame is without marked packet. m_readInd = " << m_readInd << " m_fileSize = " << m_fileSize << " obj: " << NameOf();

					//reset parameters
					bigEndTimeStamp = 0;
					litEndTimeStamp = 0;
					packetIndexInFrame = 0;
					m_numOfPacketsForTransmission = 0;
					increaseIndexParam = 0;
					packetMarker = 0;
					m_prevUsedTS = savedLastUsedTS;
					m_packetSeqNumber = savedPacketSeqNumber;
					m_readInd = 0;
					m_lastPacketSeqNumber = 0;
					m_currentFrameTS = 0; //just to make the condition TRUE ....check what happens with 'continue'//
					m_loopCounter++;
				}

				///return;	// next frame will be prepared and sent on the next timer (added because of the encrypted HD problem)
			}
		}



	} while	((litEndTimeStamp == m_currentFrameTS) && (m_readInd < m_fileSize));


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// move m_readInd back to start of last packet that was read (this is the first packet for next transmission) and decrease packetSeqNumber
	if (litEndTimeStamp != m_currentFrameTS)
	{
		m_readInd = m_readInd - (sizeof(TMediaHeaderIpNetwork) + m_rtpPacketsSize[packetIndexInFrame-1] + increaseIndexParam);
		m_packetSeqNumber--;
		m_numOfPacketsForTransmission--;
	}


	//////////////////////////////////////////////////////////////////////////////////////
	//buffer m_readInd is over file size --> loop again over the file or prepare the next dtmf
	if (m_readInd >= m_fileSize)
	{
		TRACEINTO << GetParticipantTicket() << " CMediaChannel::PrepareMediaFrame - m_readInd >= m_fileSize" << "  Name: " << NameOf();

		m_readInd = 0;
		m_lastPacketSeqNumber = 0;

		//m_bIsFirstLoopOnFile = FALSE;

		m_loopCounter++;

		//check if this is the end of a dtmf transmission
		if (m_bIsDtmfSession == TRUE)
		{
			((CAudioChannel*)this)->PrepareNextDtmf();
		}
	}

	//m_timeToNextTransmission = GetTimeToNextTransmission(litEndTimeStamp);
	DWORD timeToNextTx = GetTimeToNextTransmissionMl(litEndTimeStamp);
	m_timeToNextTransmission += timeToNextTx;
}

/////////////////////////////////////////////////////////////////////////////

void CMediaChannel::SendMedia()
{
	if (GetEncryptionType() == kAES_CBC)
	{
		DWORD	punIVector[4] = {0};
		DWORD payloadSize = 0;
		int paddingSize = 0;
		int sizeMod16 = 0;
		int lastByteIndexInPacket = 0;
		DWORD unSeqNumber = 0;
		DWORD unTimeStamp = 0;
		DWORD maxePayloadSize = MAX_NUM_OF_BYTES_IN_UDP_DATAGRAM;

		for (int j = 0; j < m_numOfPacketsForTransmission; j++)	// last packet is for next transmission
		{
			payloadSize = m_rtpPacketsSize[j] - sizeof(TRtpHeader);\
			if (payloadSize > maxePayloadSize)
			{
				PTRACE2(eLevelInfoNormal,"CMediaChannel::SendMedia : File Name - ",m_strFileName.c_str());
				PASSERTMSG(1, "payloadSize > MAX_NUM_OF_BYTES_IN_UDP_DATAGRAM");
				continue;
			}
			sizeMod16 = payloadSize % 16;	// packet size should be in multiples of 16 Bytes
			paddingSize = 0;

			//unset RTP header 'P' field
			m_rtpPacketsArr[j][0] &= 0xDF;

			if (0 != sizeMod16)
			{
				paddingSize = 16 - sizeMod16;

				//set RTP header 'P' field
				m_rtpPacketsArr[j][0] |= 0x20;


				//set padding cipher to 0s
				memset(&m_rtpPacketsArr[j][sizeof(TRtpHeader) + payloadSize], '0', paddingSize);

				lastByteIndexInPacket = sizeof(TRtpHeader) + payloadSize + paddingSize -1;
				//Set padding size in last byte of packet
				BYTE* pLastByteInPacket = (BYTE*)&m_rtpPacketsArr[j][lastByteIndexInPacket];
				*pLastByteInPacket = (BYTE)paddingSize;
			}

			/////////////////////////////////////////////////////
			//Set Initial Vector values

			unSeqNumber = (m_rtpPacketsArr[j][2]<<8) | m_rtpPacketsArr[j][3];

			unTimeStamp = m_rtpPacketsArr[j][4]<<24 | (m_rtpPacketsArr[j][5]<<16) | (m_rtpPacketsArr[j][6]<<8) | m_rtpPacketsArr[j][7];

			punIVector[0] = (unSeqNumber << 16) | (unTimeStamp >> 16);
			punIVector[1] = (unTimeStamp << 16) | (unSeqNumber & 0xFFFF);
			punIVector[2] = unTimeStamp;
			punIVector[3] = punIVector[0];


			EncryptCBC((Uint8*)&m_rtpPacketsArr[j][sizeof(TRtpHeader)] , //encrypt only payload
								payloadSize,
				 				punIVector,
				 				m_aunExpandedRoundKey);


			//set packet size
			m_rtpPacketsSize[j] += paddingSize;
		}
	}


	//TRACEINTO << "CMediaChannel::SendMedia - m_numOfPacketsForTransmission: " << m_numOfPacketsForTransmission;
	for (int i = 0; i < m_numOfPacketsForTransmission; i++)	// last packet is for next transmission
	{
		//TRACEINTO << "CMediaChannel::SendMedia - index: " << i << " m_rtpPacketsSize: " << m_rtpPacketsSize[i];
		SendTxSocket(m_rtpPacketsArr[i], m_rtpPacketsSize[i]);
	}

	m_numOfPacketsForTransmission = 0;
}

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////





////////////////////////////////////////////////////////
//					Media Sockets Rx
////////////////////////////////////////////////////////


void CMediaChannel::StartMediaRx()
{
	DumpEventMessageList();

	//socket init
	CreateRxSocket();
	//InitRxSocket();

	//init Rx writing mechanism
//	if (m_bWriteFlag)
	if (m_tIncomingChannelParam.bReadFlag)
	{
		if (m_tIncomingChannelParam.bWriteMedia)
			StartWriteUdpSocket();

		ReceiveRxSocket();
	}



	m_bIsChannelOpen = TRUE;
}

////////////////////////////////////////////////////////


void CMediaChannel::CreateRxSocket()
{
	TRACEINTO << GetParticipantTicket() << " CMediaChannel::CreateRxSocket " << "  Name: " << NameOf();

	if(m_pSocketIn) {
		delete m_pSocketIn;
		m_pSocketIn=0;
	}

	if(m_iceRtpChannelID) {
#ifndef __DISABLE_ICE__
		m_pSocketIn=new IceSocket(m_iceRtpChannelID);
#endif
	}
	else {
		char szLocalIP[IP_ADDRESS_LEN];
		::SystemDWORDToIpString(ntohl(m_local_addr.sin_addr.s_addr), szLocalIP);
		TRACEINTO << GetParticipantTicket() << " CMediaChannel::InitRxSocket family: " << m_local_addr.sin_family << " ip: " << szLocalIP << " port: " << ntohs(m_local_addr.sin_port) << " Name: " << NameOf();

		//DWORD family = AF_INET;
		DWORD port = m_local_addr.sin_port;
		DWORD ipaddress = m_local_addr.sin_addr.s_addr;

		m_pSocketIn = new CUdpSocket(ipaddress, port);

		if (m_pSocketIn != NULL)
		{
			TRACEINTO << GetParticipantTicket() << " CMediaChannel::CreateRxSocket done " << "  Name: " << NameOf();
		}
		else
		{
			TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::CreateRxSocket m_pSocketIn is NULL " << "  Name: " << NameOf();
			return;
		}

		/*int fDescriptor = m_pSocketIn->CreateUdpSocket();

		if (fDescriptor == STATUS_ERROR)
		{
			TRACEINTO << GetParticipantTicket() << " MM ERROR  CMediaChannel::CreateRxSocket - CreateUdpSocket() ERROR " << "  Name: " << NameOf();
			return;
		}*/
	}

}

////////////////////////////////////////////////////////
#if 0
void CMediaChannel::InitRxSocket()
{
	TRACEINTO << GetParticipantTicket() << " CMediaChannel::InitRxSocket " << "  Name: " << NameOf();

	char szLocalIP[IP_ADDRESS_LEN];
	::SystemDWORDToIpString(ntohl(m_local_addr.sin_addr.s_addr), szLocalIP);
	TRACEINTO << GetParticipantTicket() << " CMediaChannel::InitRxSocket family: " << m_local_addr.sin_family << " ip: " << szLocalIP << " port: " << ntohs(m_local_addr.sin_port) << " Name: " << NameOf();

	DWORD family = AF_INET;
	DWORD port = m_local_addr.sin_port;
	DWORD ipaddress = m_local_addr.sin_addr.s_addr;

	int retVal = STATUS_ERROR;
	if (m_pSocketIn != NULL)
	{
		retVal = m_pSocketIn->InitUdpSocket(ipaddress, port);
		if (retVal == STATUS_ERROR)
		{
			TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::InitRxSocket. InitUdpSocket Failed. Name: " << NameOf();
			return;
		}
	}
	else
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::InitRxSocket m_pSocketIn is NULL. Name: " << NameOf();
		return;
	}

}
#endif

////////////////////////////////////////////////////////


void CMediaChannel::ReceiveRxSocket()
{
	TRACEINTO << "CMediaChannel::ReceiveRxSocket Name: " << NameOf();

	if (m_pSocketIn == NULL)
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::ReceiveRxSocket - m_pSocketIn is NULL Name: " << NameOf();
		return;
	}

	int loopCounter = 0;	// just for debug

	while(TRUE)
	{
		int selectResult = m_pSocketIn->Select(0);
		//TRACEINTO << "CMediaChannel::ReceiveRxSocket selectResult=" << selectResult << " Name: " << NameOf();

		if (selectResult > 0)	// read socket buffer
		{
			if (m_pSocketIn != NULL)
			{
				int sizeRecv = m_pSocketIn->RecvFrom((char*)&m_recvBuffer[sizeof(TMediaHeaderIpNetwork)], MAX_NUM_OF_BYTES_IN_UDP_DATAGRAM, (struct sockaddr*)&m_from_addr);

				if (sizeRecv == STATUS_ERROR)
				{
					TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::ReceiveRxSocket - RecvFrom: STATUS_ERROR Name: " << NameOf();
					break;
				}
				else
				{
					//TRACEINTO << "CMediaChannel::ReceiveRxSocket RecvFrom buffer arrived - sizeRecv: " << sizeRecv << " Name: " << NameOf();

					loopCounter++;	// just for debug



					// set CM header (rtp size) --> check what this is for...
					TMediaHeaderIpNetwork* pCM = (TMediaHeaderIpNetwork*)m_recvBuffer;
					pCM->tCommonHeader.nPayloadSize = sizeRecv;


					// Check sequence number correctness
					if (m_tIncomingChannelParam.bCheckSeqNumber)
					{
//						TRACEINTO << "CMediaChannel::ReceiveRxSocket - Check sequence number correctness - Name: " << NameOf();

						// gets current packet sequence number
						TRtpHeader* pRtpHeader = (TRtpHeader*)&m_recvBuffer[sizeof(TMediaHeaderIpNetwork)];
						UINT16 bigOrgSeqNumber = pRtpHeader->usSequenceNumber;
						UINT16 curPacketSeqNumber = (bigOrgSeqNumber>>8) | (bigOrgSeqNumber<<8);

						// Checks if legal Sequence-Number (comparing to the previous one)
						int retStatus = ChecksLegalPacketSeqNumber( curPacketSeqNumber );
						m_lastRxPacketSeqNumber = curPacketSeqNumber;
						if (retStatus == STATUS_ERROR)
						{
							// Do something... - maybe we should ask for intra here...
						}
					}

					// Check if Intra - only in case of video channel
					if ((m_tIncomingChannelParam.bDetectIntra) && (0 == (strcmp(NameOf(), "CVideoChannel"))))
					{
//						TRACEINTO << "CMediaChannel::ReceiveRxSocket - Detecting Intra ";
						BOOL intraWasReceived = FALSE;

						if (E_VIDEO_PROTOCOL_H264 == m_eVideoProtocol)
						{
							intraWasReceived = CheckIfFullH264IntraWasDetected();
						}

						if (E_VIDEO_PROTOCOL_H263 == m_eVideoProtocol)
						{
							intraWasReceived = CheckIfFullH263IntraWasDetected();
						}

						if (intraWasReceived)
						{
							TRACEINTO << "CMediaChannel::ReceiveRxSocket - a whole valid INTRA frame was received!"
									  << m_eVideoProtocol;
							// send sync indication to MCMS
							// TBD...
						}
					}



					// checks if last packet in frame
					int isLastPacketInFrame = TRUE;
					//TBD....

					// Save to Disk as media file
					if ( /*m_bWriteFlag*/ m_tIncomingChannelParam.bWriteMedia
						&& (m_mediaBuffer != NULL))
					{
						TRACEINTO << "CMediaChannel::ReceiveRxSocket - Write to file - Name: " << NameOf();
						SaveMediaAndWrite( sizeRecv, isLastPacketInFrame );
					}
				}
			}
			else
			{
				TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::ReceiveRxSocket m_pSocketIn is NULL Name: " << NameOf();
				break;
			}
		}
		else
		{
			if (loopCounter > 20)
			{
				TRACEINTO << GetParticipantTicket() << " CMediaChannel::ReceiveRxSocket loopCounter = " << loopCounter << " Name: " << NameOf();
			}
			break;
		}
	}
}

////////////////////////////////////////////////////////

int CMediaChannel::ChecksLegalPacketSeqNumber( UINT16 curPacketSeqNumber )
{
	if (m_lastRxPacketSeqNumber != 0)
	{
		if ((curPacketSeqNumber-m_lastRxPacketSeqNumber) != 1)
		{
			CLargeString description;
			description << GetParticipantTicket()
						<< " MM ERROR CMediaChannel::ReceiveRxSocket - Mismatch packet number:"
						<< " curPacketSeqNumber=" << curPacketSeqNumber
						<< " m_lastRxPacketSeqNumber=" << m_lastRxPacketSeqNumber
						<< " Name: " << NameOf();

			TRACEINTO << description.GetString();

			CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
								777,
								SYSTEM_MESSAGE,
								description.GetString() ,
								FALSE);

			return STATUS_ERROR;
		}
	}

	return STATUS_OK;
}


////////////////////////////////////////////////////////

BOOL CMediaChannel::CheckIfFullH264IntraWasDetected()
{
	BOOL wholeIntraReceived = FALSE;
	BYTE* pFirstPayloadByte = (BYTE*)&m_recvBuffer[sizeof(TMediaHeaderIpNetwork)+sizeof(TRtpHeader)];
	BYTE leastSegHalfByte = *pFirstPayloadByte & 0x0F;

	switch (leastSegHalfByte)
	{
		case 0x07:
		{
			if (!m_1stIntraPacketWasDetected && !m_2ndIntraPacketWasDetected && !m_3rdIntraPacketWasDetected)
				m_1stIntraPacketWasDetected = TRUE;		// 1st intra packet
			else
			{
				TRACEINTO << "CMediaChannel::CheckIfFullH264IntraWasDetected - illegal intra packet >> first intra packet was received in a bad timing!";
				m_1stIntraPacketWasDetected = TRUE;		//???
				m_2ndIntraPacketWasDetected = FALSE;
				m_3rdIntraPacketWasDetected = FALSE;
			}
			break;
		}
		case 0x08:
		{
			if (m_1stIntraPacketWasDetected && !m_2ndIntraPacketWasDetected && !m_3rdIntraPacketWasDetected)
				m_2ndIntraPacketWasDetected = TRUE;		// 2nd intra packet
			else
			{
				TRACEINTO << "CMediaChannel::CheckIfFullH264IntraWasDetected - illegal intra packet >> second intra packet was received in a bad timing!";
				m_1stIntraPacketWasDetected = FALSE;
				m_2ndIntraPacketWasDetected = FALSE;
				m_3rdIntraPacketWasDetected = FALSE;
			}
			break;
		}
		case 0x05:
		{
			if (m_1stIntraPacketWasDetected && m_2ndIntraPacketWasDetected)
			{
				if (!m_3rdIntraPacketWasDetected)
					m_3rdIntraPacketWasDetected = TRUE;		// 3rd intra packet

				// 3rd intra packet and on
				if (CheckMarkerForEndOfFrame())
				{
					wholeIntraReceived = TRUE;
					m_1stIntraPacketWasDetected = FALSE;
					m_2ndIntraPacketWasDetected = FALSE;
					m_3rdIntraPacketWasDetected = FALSE;
				}
			}
			else
			{
				TRACEINTO << "CMediaChannel::CheckIfFullH264IntraWasDetected - illegal intra packet >> third intra packet was received in a bad timing!";
				m_1stIntraPacketWasDetected = FALSE;
				m_2ndIntraPacketWasDetected = FALSE;
				m_3rdIntraPacketWasDetected = FALSE;
			}

			break;
		}
		case 0x01:
		{
			// inter packet
			break;
		}
		default:
		{
			TRACEINTO << "CMediaChannel::CheckIfFullH264IntraWasDetected - illegal packet!";
			break;
		}
	}
	return wholeIntraReceived;
}



////////////////////////////////////////////////////////

BOOL CMediaChannel::CheckIfFullH263IntraWasDetected()
{
	BOOL wholeIntraReceived = FALSE;

	if (CheckMarkerForEndOfFrame())
	{	// End of frame
		if (m_1stIntraPacketWasDetected)
		{	// first intra packet was detected and we have waited for the end of the frame
			wholeIntraReceived = TRUE;
			m_1stIntraPacketWasDetected = FALSE;
		}
		else
		{ 	// still waiting for an intra
			// nothing to do
		}
	}
	else
	{	// NOT end of frame
		if (CheckIfH263StartOfFrame())
		{
			if (CheckIfH263Intra1stPacket())
			{	// First intra packet
           		if (!m_1stIntraPacketWasDetected)
           		{	// fisrt packet of intra wasn't received before
            		m_1stIntraPacketWasDetected = TRUE;
           		}
				else
				{	// intra was not fully received (last packet of intra was not detected yet) but start of new intra frame is detected
					TRACEINTO << "CMediaChannel::CheckIfFullH263IntraWasDetected - illegal intra packet >> first intra packet was received in a bad timing!";
					m_1stIntraPacketWasDetected = TRUE; //???
				}
			}
			else
			{	// Start frame but not an intra
           		if (m_1stIntraPacketWasDetected)
				{	// intra was not fully received (last packet of intra was not detected yet) but start of new inter frame is detected
					TRACEINTO << "CMediaChannel::CheckIfFullH263IntraWasDetected - illegal intra packet >> not all of intra packets were received!";
					m_1stIntraPacketWasDetected = FALSE;
				}
				else
				{	// start of inter frame received without interfering any intra frame
					// nothing to do
				}
			}
		}
		else
		{	// not end of frame and not start of frame
           	// nothing to do
		}
	}

	return wholeIntraReceived;
}



////////////////////////////////////////////////////////

BOOL CMediaChannel::CheckIfH263StartOfFrame()
{
    BOOL bStartOfFrame = FALSE;
	BYTE* pStartOfH263PayloadHeader = (BYTE*)&m_recvBuffer[sizeof(TMediaHeaderIpNetwork)+sizeof(TRtpHeader)];
	m_h263HeaderSize = 0;

    // Get payload type from RTP header
    TRtpHeader* pRtpHeader = (TRtpHeader*)&m_recvBuffer[sizeof(TMediaHeaderIpNetwork)];
	BYTE markerPlusPayloadType = pRtpHeader->markerPlusPayloadType;
	BYTE payloadType = markerPlusPayloadType & 0x7F;

	if (payloadType >= __FIRST_DPT && payloadType <= __LAST_DPT)	// dynamic payload type
	{
		//////////////////////////////////////////////////////////////////////////
		// This is H.263+ packet (Economic packetization)
		//
		// Start of frame/GOB   - There is no PSC/H.263 header. Instead there are 2 bytes (04 00)
		//                        Need to replace (04 00) with PSC (00 00)
		// Start of MB          - Added 2 bytes of zeros (00 00) - Remove them - Not relevant here!
		//
		//////////////////////////////////////////////////////////////////////////

    	// H.263 Payload Header
		// Read 2 bytes from file (May be "00 00" or "04 00")
		WORD h263PayloadHeader = pStartOfH263PayloadHeader[0];
		h263PayloadHeader <<= 8;
		h263PayloadHeader += pStartOfH263PayloadHeader[1];

		if (h263PayloadHeader == 0x0400)
		{
			// This is a start code - replace with 00 00 when checking PSC (22 first bytes of payload)
			bStartOfFrame = TRUE;
		}
		else if (!(h263PayloadHeader == 0x0000))
		{
			TRACEINTO << "CMediaChannel::CheckIfFullH263IntraWasDetected >> Received Unknown H.263+ Header: "
					  << h263PayloadHeader;
		}
	}

	else if (payloadType == _H263)
	{
		//////////////////////////////////////////////////////////////////////////
		// This is H.263 packet
		//
		// Start of frame/GOB - 4 Bytes of H.263 header (Mode A)
		// Start of MB - 8 Bytes of H.263 header (Mode B)
		// has additional information MV, start bits etc..)
		// Start of MB with PB frame support- 12 Bytes of H.263 header (Mode C)
		//
		// H.263 Payload header structure
		// ==============================
		// Mode A:
		// -------
		//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		// |F|P|SBIT |EBIT | SRC |I|U|S|A|R  |DBQ| TRB |  TR   |
		// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		//
		// Packets will be cut on GOB boundary or picture boundary.
		// Mode A packets allways start with the H.263 PSC (Picture Start Code)
		// or GOB (But do not necessarily contain complete GOB)
		// 4 Bytes are used for the Mode A H.263 payload header.
		// F=0
		//
		// Mode B:
		// -------
		//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		// |F|P|SBIT |EBIT | SRC | QUANT |  GOBN | MBA     |R  |
		// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		// |I|U|S|A| HMV1    | VMV1    | HMV2    | VMV2    |
		// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		//
		// In this mode, an H.263 bitstream can be fragmented at MB boundaries.
		// Whenever a packet starts at a MB boundary, this mode shall be used
		// without PB-frames option.
		// F=1 && P=0
		//
		//////////////////////////////////////////////////////////////////////////

    	// Read two first bits - check which mode is currently in use
		// Mode A
		if ((pStartOfH263PayloadHeader[0]>>7) == 0x00)
		{
			m_h263HeaderSize = 4;
		}
		// Mode B
		else if ((pStartOfH263PayloadHeader[0]>>6) == 0x02)
		{
			// Mode B, Read 4 more bytes
			m_h263HeaderSize = 8;
		}
		// Mode C
		else if ((pStartOfH263PayloadHeader[0]>>6) == 0x03)
		{
			// Mode C, Read 8 more bytes
			m_h263HeaderSize = 12;
		}
		// Unknown mode for payload header
		else
		{
			TRACEINTO << "CMediaChannel::CheckIfFullH263IntraWasDetected >> Received Unknown H.263 Payload Header mode: "
					  << (pStartOfH263PayloadHeader[0]>>6);
		}
	}


  	// handle payload
    BYTE* pPayloadStart = (BYTE*)&m_recvBuffer[sizeof(TMediaHeaderIpNetwork)+sizeof(TRtpHeader)+m_h263HeaderSize];
	DWORD startOfFrameBits = 0;

	DWORD payloadStart = pPayloadStart[0];
	payloadStart <<= 8;
	payloadStart += pPayloadStart[1];
	payloadStart <<= 8;
	payloadStart += pPayloadStart[2];
	payloadStart <<= 8;
	payloadStart += pPayloadStart[3];

    if ((payloadType != _H263) && (bStartOfFrame == TRUE))	// H263+ with header 04 00 - start of frame
		startOfFrameBits = payloadStart & 0x0000FC00;		// ignore 2 first bytes (replace with 00 00)
    else
	    startOfFrameBits = payloadStart & 0xFFFFFC00;

	// Check if start of frame : 00000000 00000000 100000xx
    if (0x8000 == startOfFrameBits)
    {
    	bStartOfFrame = TRUE;
    }
    else
    {
		bStartOfFrame = FALSE;
    }

	return bStartOfFrame;
}



////////////////////////////////////////////////////////

BOOL CMediaChannel::CheckIfH263Intra1stPacket()
{
	BOOL bH263Intra1stPacket = FALSE;
	if (-1 == m_h263HeaderSize)
	{
		TRACEINTO << "CMediaChannel::CheckIfH263Intra1stPacket >> ERROR - illegal h263HeaderSize"
				  << " - (check if CMediaChannel::CheckIfFullH263IntraWasDetected was not called before this function...) ";
		return FALSE;
	}

    BYTE* pPayloadStart = (BYTE*)&m_recvBuffer[sizeof(TMediaHeaderIpNetwork)+sizeof(TRtpHeader)+m_h263HeaderSize];

	//Check for intra bit
	if(((*(pPayloadStart + 4)) & 0x1c) != 0x1c) //check if it isn't plus type
    {
    	if ( 0 == ((*(pPayloadStart + 4)) & 0x02) ) //bit number 9 in ptype is 0
        {
        	bH263Intra1stPacket = TRUE;
        }
        else
        {
        	bH263Intra1stPacket = FALSE;
        }
	}
    else
    {
    	if (((*(pPayloadStart + 5)) & 0x80) && (!((*(pPayloadStart + 7)) & 0x1c))) //UFEP = 001 && bits (1-3)
    	{
        	bH263Intra1stPacket = TRUE;
        }
        else
        {
        	bH263Intra1stPacket = FALSE;
        }
    }

	return bH263Intra1stPacket;
}


////////////////////////////////////////////////////////

BOOL CMediaChannel::CheckMarkerForEndOfFrame()
{
	TRtpHeader* pRtpHeader = (TRtpHeader*)&m_recvBuffer[sizeof(TMediaHeaderIpNetwork)];
	UINT8 markerPlusPayloadType = pRtpHeader->markerPlusPayloadType;
	BYTE marker = (markerPlusPayloadType & 0x80) >> 7;

	return marker;
}


////////////////////////////////////////////////////////

int	CMediaChannel::SaveMediaAndWrite( int sizeRecv, int lastPacketInFrame )
{
	// need to write to file on "Frame" size and not "Packet size" !!!

	int rtpPacketPlusCMHeaderSize = sizeRecv + sizeof(TMediaHeaderIpNetwork);

	//if buffer will be over written -> write media buffer to file
	if ( (m_mediaBufferIndex + rtpPacketPlusCMHeaderSize + 4) > MEDIA_WRITE_BUFFER_SIZE)	// 4 for padding
	{
		if (m_writtenFileSize < MAX_MEDIA_WRITE_FILE_SIZE)
		{
			WriteBuffer();	// write on "frame" boundary, update index
		}
		//init buffer index
//		m_mediaBufferIndex = 0;
	}

	//copy recv buffer from socket to media buffer
	if ( (m_mediaBufferIndex + rtpPacketPlusCMHeaderSize + 4) <= MEDIA_WRITE_BUFFER_SIZE )
	{
		//code for entering a NULL header + remove the rtp header
		/*memset(&m_mediaBuffer[m_mediaBufferIndex], 0, 1);
		memset(&m_mediaBuffer[m_mediaBufferIndex+1], 0, 1);
		memset(&m_mediaBuffer[m_mediaBufferIndex+2], 0, 1);
		memset(&m_mediaBuffer[m_mediaBufferIndex+3], 1, 1);
		memcpy(&m_mediaBuffer[m_mediaBufferIndex+4], &m_recvBuffer[12], sizeRecv-12);*/

		memcpy(&m_mediaBuffer[m_mediaBufferIndex], m_recvBuffer, rtpPacketPlusCMHeaderSize);
		m_mediaBufferIndex += rtpPacketPlusCMHeaderSize;

		//padding
		int sizeMod4 = sizeRecv % 4;	// adjusting to current PCI demand (4 Bytes alignment)
		if (0 != sizeMod4)		// if not divided by 4 Bytes without remainder - round off up
			m_mediaBufferIndex += (4 - sizeMod4);
		if (TRUE == lastPacketInFrame)
			m_frameBufferIndex = m_mediaBufferIndex;	// update the write index for the next writing

	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////

void CMediaChannel::CloseRxSocket()
{
	TRACEINTO << GetParticipantTicket() << " CMediaChannel::CloseRxSocket Name: " << NameOf();

	if (m_pSocketIn != NULL)
	{
//		if (m_bWriteFlag)
		if (m_tIncomingChannelParam.bWriteMedia)
		{
//			m_bWriteFlag = FALSE;
			m_tIncomingChannelParam.bWriteMedia = FALSE;
			if (m_writtenFileSize < MAX_MEDIA_WRITE_FILE_SIZE)
			{
				if (m_mediaBuffer != NULL)
					WriteBuffer();
			}
			FileClose();
		}
		POBJDELETE(m_mediaBuffer);


		/*int retVal = STATUS_ERROR;

		retVal = m_pSocketIn->CloseUdpSocket();

		if (retVal == STATUS_ERROR)
		{
			TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::CloseRxSocket. CloseUdpSocket Failed Name: " << NameOf();
			return;
		}*/

		POBJDELETE(m_pSocketIn);

		TRACEINTO << GetParticipantTicket() << " CMediaChannel::CloseRxSocket - done Name: " << NameOf();
	}
	else
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::CloseRxSocket - m_pSocketIn is NULL Name: " << NameOf();
	}

}


////////////////////////////////////////////////////////


void CMediaChannel::StartWriteUdpSocket()
{
	TRACEINTO << GetParticipantTicket() << " CMediaChannel::StartWriteUdpSocket Name: " << NameOf();

	if (m_pSocketIn == NULL)
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::StartWriteUdpSocket - m_pSocketIn is NULL Name: " << NameOf();
		return;
	}


	m_sFullFileName  = ::GetMediaMngrCfg()->GetMediaFileWritePath();

	CStructTm localTime;
	SystemGetTime(localTime);
	char timeStr[128];

	int short_year = localTime.m_year-2000;
	char    str_short_year [4];
	snprintf(str_short_year, sizeof(str_short_year), "%02d",short_year);

	snprintf(timeStr, sizeof(timeStr), "_%02d.%02d.%02d_%02d%02d%02d", localTime.m_day, localTime.m_mon, short_year,
											 localTime.m_hour, localTime.m_min, localTime.m_sec);

	string fileName = NameOf();
	fileName += timeStr;

	m_sFullFileName += "/";
	m_sFullFileName += fileName;

	int openFileStatus = FileOpen();
	if (openFileStatus == STATUS_OK)
	{
		//init buffer in memory for writing upon buffer is full
		m_mediaBuffer = new BYTE[MEDIA_WRITE_BUFFER_SIZE];
	}
}

////////////////////////////////////////////////////////


int CMediaChannel::WriteBuffer()
{
	TRACEINTO << GetParticipantTicket() << " CMediaChannel::WriteBuffer Name: " << NameOf();

	size_t numWrite = fwrite(m_mediaBuffer, sizeof(BYTE), m_mediaBufferIndex, m_pFile);
	m_writtenFileSize += m_mediaBufferIndex;

	// update write index (consider frame boundary)
	m_mediaBufferIndex = 0;	// need to correct...

	TRACEINTO << GetParticipantTicket() << " CMediaChannel::WriteBuffer num bytes writen = " << numWrite << " Name: " << NameOf();

	if (numWrite != (size_t)m_mediaBufferIndex)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::WriteBuffer fwrite() errno=" << myerrno << " description: " << myerrnostr.c_str();
		return STATUS_ERROR;
	}

	//flush the buffer to disk
	int flushStatus = fflush(m_pFile);
	if (flushStatus != STATUS_OK)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::WriteBuffer fflush() errno=" << myerrno << " description: " << myerrnostr.c_str() << " flushStatus=" << flushStatus;
		return STATUS_ERROR;
	}

	//print current file size
	FileSize();

	return numWrite;
}


/////////////////////////////////////////////////////////////////////////////

int CMediaChannel::FileOpen()
{
	TRACEINTO << GetParticipantTicket() << " CMediaChannel::FileOpen Name: " << NameOf();

	// check if file exists
	if (IsFileExists( m_sFullFileName.c_str()))
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::FileOpen - File allready exists: " <<  m_sFullFileName.c_str();
		return STATUS_ERROR;
	}

	// open file for writing binary
	m_pFile = fopen( m_sFullFileName.c_str(), "wb" );

	if (m_pFile == NULL)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::FileOpen - File Open Error: " <<  m_sFullFileName.c_str() << " errno=" << myerrno << " description: " << myerrnostr.c_str();
		return STATUS_ERROR;
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

int CMediaChannel::FileClose()
{
	TRACEINTO << GetParticipantTicket() << " CMediaChannel::FileClose Name: " << NameOf();

	//close the fd until flush operation will come
	int closeStatus = fclose(m_pFile);
	if (closeStatus != STATUS_OK)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << GetParticipantTicket() << " MM ERROR CMediaChannel::FileClose - fclose  errno=" << myerrno << " description: " << myerrnostr.c_str();
		return STATUS_ERROR;
	}

	m_pFile = NULL;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

int CMediaChannel::FileSize()
{
	int fileSize = GetFileSize(m_sFullFileName);

	TRACEINTO << GetParticipantTicket() << " CMediaChannel::FileSize = " << fileSize << " Name: " << NameOf();

	return fileSize;
}



////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////







//////////////////////////////////////////

INT32 CMediaChannel::GetChannelDirection() const
{
	return m_channelDirection;
}

//////////////////////////////////////////

CapEnum CMediaChannel::GetCapTypeCode() const
{
	return m_capTypeCode;
}

//////////////////////////////////////////

string CMediaChannel::GetCapTypeCodeStr() const
{
	char* capCodeStr = ::GetCapCodeNameStringOnly(m_capTypeCode);

	string str = capCodeStr;
	return str;
}

//////////////////////////////////////////

APIU32 CMediaChannel::GetBitRate() const
{
	return m_bitRate;
}

//////////////////////////////////////////

string CMediaChannel::GetBitRateStr() const
{
	char bitRateStr[20];
	sprintf(bitRateStr ,"%u", m_bitRate);
	string str = bitRateStr;
	return str;
}

//////////////////////////////////////////

APIU32 CMediaChannel::GetPayloadType() const
{
	return m_payloadType;
}

//////////////////////////////////////////

string CMediaChannel::GetPayloadTypeStr() const
{
	char payloadTypeStr[20];
	sprintf(payloadTypeStr ,"%u", m_payloadType);
	string str = payloadTypeStr;
	return str;
}

//////////////////////////////////////////

APIU32 CMediaChannel::GetDtmfPayloadType() const
{
	return m_dtmfPayloadType;
}

//////////////////////////////////////////

string CMediaChannel::GetDtmfPayloadTypeStr() const
{
	char dtmfPayloadTypeStr[20];
	sprintf(dtmfPayloadTypeStr ,"%u", m_dtmfPayloadType);
	string str = dtmfPayloadTypeStr;
	return str;
}

//////////////////////////////////////////

APIU32 CMediaChannel::GetIsH263Plus() const
{
	return m_isH263Plus;
}

//////////////////////////////////////////

string CMediaChannel::GetIsH263PlusStr() const
{
	if (m_isH263Plus)
		return "H263 Plus - On";
	else
		return "H263 Plus - Off";
}

//////////////////////////////////////////

APIU32 CMediaChannel::GetAnnexesPlus() const
{
	return m_annexesMask;
}

//////////////////////////////////////////

string CMediaChannel::GetAnnexesPlusStr() const
{
	if (m_annexesMask)
		return "Annexes Plus - On";
	else
		return "Annexes Plus - Off";
}

//////////////////////////////////////////

APIU32 CMediaChannel::GetCustomMaxMbpsValue() const
{
	return m_customMaxMbpsValue;
}

//////////////////////////////////////////

string CMediaChannel::GetCustomMaxMbpsValueStr() const
{
	char customMaxMbpsStr[20];
	sprintf(customMaxMbpsStr ,"%u", m_customMaxMbpsValue);
	string str = customMaxMbpsStr;
	return str;
}

//////////////////////////////////////////

APIU32 CMediaChannel::GetMaxFramesPerPacket() const
{
	return m_maxFramesPerPacket;
}

//////////////////////////////////////////

string CMediaChannel::GetMaxFramesPerPacketStr() const
{
	char maxFramesPerPacketStr[20];
	sprintf(maxFramesPerPacketStr ,"%u", m_maxFramesPerPacket);
	string str = maxFramesPerPacketStr;
	return str;
}


//////////////////////////////////////////
//Encryption
//////////////////////////////////////////

//////////////////////////////////////////

EenMediaType CMediaChannel::GetEncryptionType() const
{
	return m_unEncryptionType;
}

//////////////////////////////////////////

string CMediaChannel::GetEncryptionTypeStr() const
{
	char encryptionTypeStr[20];
	sprintf(encryptionTypeStr ,"%u", m_unEncryptionType);
	string str = encryptionTypeStr;
	return str;
}
//////////////////////////////////////////

APIU8* CMediaChannel::GetEncryptionSessionKey()
{
	return m_aucSessionKey;
}

//////////////////////////////////////////

string CMediaChannel::GetEncryptionSessionKeyStr() const
{
	char sessionKeyStr[sizeOf128Key];
	sprintf(sessionKeyStr ,"%u", m_aucSessionKey);
	string str = sessionKeyStr;
	return str;
}




//////////////////////////////////////////

string CMediaChannel::GetChannelDirectionStr() const
{
	if (m_channelDirection == MEDIA_DIRECTION_IN)
	{
		return " In";
	}
	else if (m_channelDirection == MEDIA_DIRECTION_OUT)
	{
		return " Out";
	}
	else if (m_channelDirection == MEDIA_DIRECTION_IN_OUT)
	{
		return " In & Out";
	}
	else
	{
		return " Uninitialized";
	}

}


////////////////////////////////////////////////////////////////////////////


void CMediaChannel::SetParticipantTicket(string participantTicket)
{
	m_participantTicket = participantTicket;
}

////////////////////////////////////////////////////////////////////////////

string CMediaChannel::GetParticipantTicket()
{
	return m_participantTicket;
}








////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////////   CFeccChannel
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//					MESSAGE_MAP
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CFeccChannel)

//ONEVENT(EXTENSION_DTMF_TIMER		,ACTIVE	      ,CIvrCntl::OnTimerSendfExt)

PEND_MESSAGE_MAP(CFeccChannel,CMediaChannel);


CFeccChannel::CFeccChannel() : CMediaChannel()
{

}

/////////////////////////////////////////////////////////////////////////////

CFeccChannel::CFeccChannel(CTaskApp* pOwnerTask, INT32 channelDirection)
		: CMediaChannel(pOwnerTask, channelDirection)
{

}

/////////////////////////////////////////////////////////////////////////////

CFeccChannel::~CFeccChannel()
{
}

/////////////////////////////////////////////////////////////////////////////
//					GetMessageMap
/////////////////////////////////////////////////////////////////////////////

void*  CFeccChannel::GetMessageMap()
{
  return (void*)m_msgEntries;
}


/////////////////////////////////////////////////////////////////////////////
//					NameOf
/////////////////////////////////////////////////////////////////////////////

const char*  CFeccChannel::NameOf() const
{
	return "CFeccChannel";
}


/////////////////////////////////////////////////////////////////////////////
//					HandleEvent
/////////////////////////////////////////////////////////////////////////////

void  CFeccChannel::HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode)
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

string CFeccChannel::ChannelData()
{
	string str = "";
	str = CMediaChannel::ChannelData();

	//add relevant data for Fecc Channel
	//str += "\some fecc parameter: ";
	//str += GetFeccParameterStr();

	return str.c_str();
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////




///////////////////
//Externals Methods
///////////////////

char* MediaTypeAndDirectionToString(APIU32 channelIndex)
{
	switch (channelIndex)
	{
		//Video
		case VIDEO_OUT_CHANNEL_INDEX:
		{
			return "Video Out";
		}
		case VIDEO_IN_CHANNEL_INDEX:
		{
			return "Video In";
		}
		//Audio
		case AUDIO_OUT_CHANNEL_INDEX:
		{
			return "Audio Out";
		}
		case AUDIO_IN_CHANNEL_INDEX:
		{
			return "Audio In";
		}
		//Content
		case CONTENT_OUT_CHANNEL_INDEX:
		{
			return "Content Out";
		}
		case CONTENT_IN_CHANNEL_INDEX:
		{
			return "Content In";
		}
		//Fecc
		case FECC_OUT_CHANNEL_INDEX:
		{
			return "Fecc Out";
		}
		case FECC_IN_CHANNEL_INDEX:
		{
			return "Fecc In";
		}

		default:
		{
			return "Invalid Media Type";
		}
	}

	return NULL;
}

