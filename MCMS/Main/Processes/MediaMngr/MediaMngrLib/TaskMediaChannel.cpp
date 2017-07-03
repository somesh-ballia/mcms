#include "TaskMediaChannel.h"
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
#include "MediaMngrCfg.h"
#include "OpcodesMcmsVideo.h"
#include "H221.h"
#include "OpcodesMcmsAudio.h"
#include "AudioApiDefinitionsStrings.h"
#include "MediaRepository.h"
#include "DtmfAlgDB.h"


#include <arpa/inet.h>


 
#define FRAME_POSITION_FIRST	1
#define FRAME_POSITION_SECOND	2
#define FRAME_POSITION_THIRD	3

//////////////////////////////////////////
//CTaskMediaChannel
//////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CTaskMediaChannel)

	ONEVENT( LIST_EVENT_MESSAGE,		ANYCASE,   CTaskMediaChannel::OnListEventMessageAll)
	ONEVENT( ACTIVATE_EVENT_MESSAGE,	ANYCASE,   CTaskMediaChannel::OnActivateEventMessageAll)
	ONEVENT( TIMER_SEND_MEDIA,			ANYCASE,   CTaskMediaChannel::OnTimerSendMedia)
	
PEND_MESSAGE_MAP(CTaskMediaChannel, CTaskApp);



CTaskMediaChannel::CTaskMediaChannel(INT32 channelDirection)
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
	
	memset((void*) &m_taLocalAddress, 0, sizeof(mcTransportAddress));
	memset((void*) &m_taRemoteAddress, 0, sizeof(mcTransportAddress));

	memset((void *)&m_local_addr, '\0', sizeof(m_local_addr));
	memset((void *)&m_remote_addr, '\0', sizeof(m_remote_addr));
	
	
	
	m_pUdpSocketOut = NULL;
	m_pUdpListenSocketTaskApi = NULL;
	
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

	m_mediaRepositoryElement = NULL;
	
	m_packetSeqNumber = 0;
	
	m_currentFrameTS = 0;
	m_iterationDeltaTS = 0;
	m_firstBufferTS = 0;
	m_lastUsedTS = 0;
	m_eofFactorTS = 0;
	
	m_strFileName = "";
	
	m_timeToNextTransmission = 0;
	m_numOfPacketsForTrans = 0;
	
	m_fraction = 0;
	
	m_bIsDtmfSession = FALSE;
	
	for (int i = 0; i < MAX_PACKETS_PER_FRAME; i++)
	{
		memset( m_packetsArr, 0, MAX_PACKET_SIZE);	//set all buffer to 0s
		m_payloadSize[i] = 0;
	}
}


CTaskMediaChannel::~CTaskMediaChannel()
{
	if (m_channelDirection == MEDIA_DIRECTION_OUT)
	{
		TRACEINTO << "CTaskMediaChannel::~CTaskMediaChannel() Out";
	}
	else if (m_channelDirection == MEDIA_DIRECTION_IN)
	{
		TRACEINTO << "CTaskMediaChannel::~CTaskMediaChannel() In";
	}
	
	
	CloseChannel();
	
	
	//list of event messages
	for (int i = 0; i < MAX_EVENT_MESSAGE_STACK_SIZE; i++)
	{
		POBJDELETE(m_pEventMessageArr[i]);
	}
	
	//media
	if (m_mediaRepositoryElement != NULL)
		m_mediaRepositoryElement->DecRefCounter();
}



/////////////////////////////////////////////////////////////////////////////

void  CTaskMediaChannel::Create(CSegment& appParam)
{
	CTaskApp::Create(appParam);
}

/////////////////////////////////////////////////////////////////////////////

void  CTaskMediaChannel::InitTask()
{
	TRACEINTO << "CTaskMediaChannel::InitTask pid: " << getpid() <<  " taskid:" << GetTaskId();
}

/////////////////////////////////////////////////////////////////////////////

void* CTaskMediaChannel::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////

void CTaskMediaChannel::SelfKill()
{
	TRACEINTO << "CTaskMediaChannel::SelfKill: m_channelDirection: " << m_channelDirection;
	DeleteAllTimers();
	CTaskApp::SelfKill();
}

//////////////////////////////////////////

string CTaskMediaChannel::ChannelData()
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
	
	str += "\n";
	
	return str;
}

//////////////////////////////////////////

INT32 CTaskMediaChannel::GetChannelDirection() const
{
	return m_channelDirection;
}

//////////////////////////////////////////

CapEnum CTaskMediaChannel::GetCapTypeCode() const
{
	return m_capTypeCode;
}

//////////////////////////////////////////

string CTaskMediaChannel::GetCapTypeCodeStr() const
{
	/*CObjString* pContentStr = new CObjString("", 50);	
	//CapCodeType
	::GetCapCodeName(m_capTypeCode, *pContentStr);
	const string sCapCodeTypeStr = pContentStr->GetString();
	
	POBJDELETE(pContentStr);*/
	
	const char* capCodeStr = CapEnumToString(m_capTypeCode);
	
	string str = capCodeStr;
	return str;
}

//////////////////////////////////////////

APIU32 CTaskMediaChannel::GetBitRate() const
{
	return m_bitRate;
}

//////////////////////////////////////////

string CTaskMediaChannel::GetBitRateStr() const
{
	char bitRateStr[20];
	sprintf(bitRateStr ,"%u", m_bitRate);
	string str = bitRateStr;
	return str;
}

//////////////////////////////////////////

APIU32 CTaskMediaChannel::GetPayloadType() const
{
	return m_payloadType;
}

//////////////////////////////////////////

string CTaskMediaChannel::GetPayloadTypeStr() const
{
	char payloadTypeStr[20];
	sprintf(payloadTypeStr ,"%u", m_payloadType);
	string str = payloadTypeStr;
	return str;	
}

//////////////////////////////////////////

APIU32 CTaskMediaChannel::GetDtmfPayloadType() const
{
	return m_dtmfPayloadType;
}

//////////////////////////////////////////

string CTaskMediaChannel::GetDtmfPayloadTypeStr() const
{
	char dtmfPayloadTypeStr[20];
	sprintf(dtmfPayloadTypeStr ,"%u", m_dtmfPayloadType);
	string str = dtmfPayloadTypeStr;
	return str;	
}

//////////////////////////////////////////

APIU32 CTaskMediaChannel::GetIsH263Plus() const
{
	return m_isH263Plus;
}

//////////////////////////////////////////

string CTaskMediaChannel::GetIsH263PlusStr() const
{
	if (m_isH263Plus)
		return "H263 Plus - On";
	else
		return "H263 Plus - Off";	
}

//////////////////////////////////////////

APIU32 CTaskMediaChannel::GetAnnexesPlus() const
{
	return m_annexesMask;
}

//////////////////////////////////////////

string CTaskMediaChannel::GetAnnexesPlusStr() const
{
	if (m_annexesMask)
		return "Annexes Plus - On";
	else
		return "Annexes Plus - Off";
}

//////////////////////////////////////////
	
APIU32 CTaskMediaChannel::GetCustomMaxMbpsValue() const
{
	return m_customMaxMbpsValue;
}

//////////////////////////////////////////

string CTaskMediaChannel::GetCustomMaxMbpsValueStr() const
{
	char customMaxMbpsStr[20];
	sprintf(customMaxMbpsStr ,"%u", m_customMaxMbpsValue);
	string str = customMaxMbpsStr;
	return str;	
}

//////////////////////////////////////////

APIU32 CTaskMediaChannel::GetMaxFramesPerPacket() const
{
	return m_maxFramesPerPacket;
}

//////////////////////////////////////////

string CTaskMediaChannel::GetMaxFramesPerPacketStr() const
{
	char maxFramesPerPacketStr[20];
	sprintf(maxFramesPerPacketStr ,"%u", m_maxFramesPerPacket);
	string str = maxFramesPerPacketStr;
	return str;
}

//////////////////////////////////////////

string CTaskMediaChannel::GetChannelDirectionStr() const
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

//////////////////////////////////////////

TEventMessage* CTaskMediaChannel::GetEventMessage(OPCODE eventOpcode)
{
	for (int i = 0; i < MAX_EVENT_MESSAGE_STACK_SIZE; i++)
	{
		if (eventOpcode == m_pEventMessageArr[i]->event_opcode)
		{
			return m_pEventMessageArr[i];
		}		
	}
	
	return NULL;
}

//////////////////////////////////////////

BOOL CTaskMediaChannel::ListEventMessage(const string opCodeStr, OPCODE opCode, CMplMcmsProtocol* pMplProtocol)
{
	for (int i = 0; i < MAX_EVENT_MESSAGE_STACK_SIZE; i++)
	{
		if (m_pEventMessageArr[i] == NULL)
		{
			TEventMessage* pEventMessage = new TEventMessage();
			pEventMessage->str_event_opcode = opCodeStr;
			pEventMessage->event_opcode = opCode;
			pEventMessage->mcms_protocol = pMplProtocol;
			
			m_pEventMessageArr[i] = pEventMessage;
			
			TRACEINTO << "CTaskMediaChannel::ListEventMessage opcode:" << opCodeStr << " (" << (int)opCode <<  ")";
			return TRUE;
		}
	}
	
	// in case there is no free place we ignore it
	return FALSE;
}


//////////////////////////////////////////

void CTaskMediaChannel::ActivateEventMessage(const string opCodeStr, OPCODE opCode, CMplMcmsProtocol* pMplProtocol)
{
	TRACEINTO << "CTaskMediaChannel::ActivateEventMessage opcode:" << opCodeStr << " (" << (int)opCode <<  ")";
	
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
			if ((GetTaskName() == "TaskVideoChannel") ||
				(GetTaskName() == "TaskAudioChannel") ) //|| (GetTaskName() == "TaskContentChannel"))
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
			((CTaskAudioChannel*)this)->SendDTMF(tPlayToneStruct);
			return;
		}
		
		//content - on/off
		case ART_CONTENT_ON_REQ:
		case ART_CONTENT_OFF_REQ:
		{
			TContentOnOffReq* pContentStruct = (TContentOnOffReq*)pMplProtocol->GetData();
			APIU32 contentMode = pContentStruct->bunIsOnOff;
			TRACEINTO << "CTaskMediaChannel::ActivateEventMessage contentMode: " << contentMode;

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
		case VIDEO_ENCODER_UPDATE_PARAM_REQ:
		{
			ENCODER_PARAM_S* pEncoderParamsStruct = (ENCODER_PARAM_S*)pMplProtocol->getpData();
			//((CVideoChannel*)this)->OldUpdateVideoParam(pEncoderParamsStruct);
			return;
		}
	}
	
	
	//start Tx or Rx
	if (m_bRtpUpdatePort && m_bOpenUdpPort && (0 == m_firstConnection))
	{
		m_firstConnection = 1;
		
		
		// Start Media Tx for Video Out will start when Video Param arrive !!! - after TB_MSG_OPEN_PORT_REQ arrives!! 
		if (GetTaskName() == "TaskVideoChannel" && m_channelDirection == MEDIA_DIRECTION_OUT)
		{
			if (m_bVideoOutParam)			
				StartMediaTx();
			else
				return;
		}
		
		if ((GetTaskName() == "TaskVideoChannel") || (GetTaskName() == "TaskAudioChannel"))
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
//////////////////////////////////////////

void CTaskMediaChannel::DumpEventMessageList()
{
	string str = "CTaskMediaChannel::DumpEventMessageList:" + ChannelData();
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
	TRACEINTO << str;
}


/////////////////////////////////////////////////////////////////////////////
// received message from EPConnection
/////////////////////////////////////////////////////////////////////////////
void CTaskMediaChannel::OnListEventMessageAll(CSegment* pParam)
{
	TRACEINTO << "CTaskMediaChannel::OnListEventMessageAll";

	char  sOpCode[256];
	OPCODE opCode;
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	
	*pParam	>> sOpCode
			>> opCode;
	pMplProtocol->DeSerialize(*pParam);
	
	TRACEINTO << "CTaskMediaChannel::OnListEventMessageAll sOpCode:" << sOpCode;
			
	const string sOpCodeStr = sOpCode;
	
	ListEventMessage(sOpCodeStr, opCode, pMplProtocol);
	
	POBJDELETE(pMplProtocol);
}


void CTaskMediaChannel::OnActivateEventMessageAll(CSegment* pParam)
{
	TRACEINTO << "CTaskMediaChannel::OnActivateEventMessageAll";

	char  sOpCode[256];
	OPCODE opCode;
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	
	*pParam	>> sOpCode
			>> opCode;
	pMplProtocol->DeSerialize(*pParam);
	
	TRACEINTO << "CTaskMediaChannel::OnActivateEventMessageAll sOpCode:" << sOpCode;
			
	const string sOpCodeStr = sOpCode;
	
	ActivateEventMessage(sOpCodeStr, opCode, pMplProtocol);
	
	POBJDELETE(pMplProtocol);
}


//Media Channel Sockets - Tx
/////////////////////////////

void CTaskMediaChannel::CreateTxSocket()
{
	TRACEINTO << "CTaskMediaChannel::CreateTxSocket " << " TaskName: " << GetTaskName();
	
	m_pUdpSocketOut = new CUdpSocket();
	
	if (m_pUdpSocketOut != NULL)
	{
		TRACEINTO << "CTaskMediaChannel::CreateTxSocket done " << " TaskName: " << GetTaskName();
	}
	else
	{
		TRACEINTO << "MM ERROR CTaskMediaChannel::CreateTxSocket m_pUdpSocketOut is NULL " << " TaskName: " << GetTaskName();
		return;
	}
	
	
	
	/*int fDescriptor = m_pUdpSocketOut->CreateUdpSocket();
	
	if (fDescriptor == STATUS_ERROR)
	{
		TRACEINTO << "MM ERROR  CTaskMediaChannel::CreateTxSocket - CreateUdpSocket() ERROR " << " TaskName: " << GetTaskName();
		return;
	}*/
}

//////////////////////////////////////////

void CTaskMediaChannel::InitTxSocket()
{
	return;

#if 0
	TRACEINTO << "CTaskMediaChannel::InitTxSocket " << " TaskName: " << GetTaskName();
	
	int retVal = STATUS_ERROR;
	if (m_pUdpSocketOut != NULL)
	{
		retVal = m_pUdpSocketOut->InitUdpSocket();
		if (retVal == STATUS_ERROR)
		{
			TRACEINTO << "MM ERROR CTaskMediaChannel::InitTxSocket. InitUdpSocket Failed " << " TaskName: " << GetTaskName();
			return;
		}
	}
	else
	{
		TRACEINTO << "MM ERROR CTaskMediaChannel::InitTxSocket m_pUdpSocketOut is NULL " << " TaskName: " << GetTaskName();
		return;
	}
	
	TRACEINTO << "CTaskMediaChannel::InitTxSocket done";
	return;
#endif
}


//////////////////////////////////////////
void CTaskMediaChannel::SendTxSocket()
{
	//TRACEINTO << "CTaskMediaChannel::SendTxSocket";
	
	const char* buffer = "##########   TEST UDP TX   ###########";
	
	SendTxSocket(buffer);
}


void CTaskMediaChannel::SendTxSocket(const char* buffer)
{
	//TRACEINTO << "CTaskMediaChannel::SendTxSocket";
	
	if (m_pUdpSocketOut != NULL)
	{
		m_pUdpSocketOut->SendTo(buffer, (struct sockaddr*)&m_remote_addr, strlen(buffer));
		
		//TRACEINTO << "CTaskMediaChannel::SendTxSocket - done";
	}
	else
	{
		TRACEINTO << "MM ERROR CTaskMediaChannel::SendTxSocket - m_pUdpSocketOut is NULL " << " TaskName: " << GetTaskName();
	}
}

void CTaskMediaChannel::SendTxSocket(const BYTE* buffer, DWORD size)
{
	//TRACEINTO << "CTaskMediaChannel::SendTxSocket-BYTE";
	
	if (m_pUdpSocketOut != NULL)
	{
		m_pUdpSocketOut->SendTo((const char*)buffer, (struct sockaddr*)&m_remote_addr, size);
		
		//TRACEINTO << "CTaskMediaChannel::SendTxSocket - done";
	}
	else
	{
		TRACEINTO << "MM ERROR CTaskMediaChannel::SendTxSocket - m_pUdpSocketOut is NULL " << " TaskName: " << GetTaskName();
	}
}

//////////////////////////////////////////

void CTaskMediaChannel::CloseTxSocket()
{
	TRACEINTO << "CTaskMediaChannel::CloseTxSocket " << " TaskName: " << GetTaskName();
	
	if (m_pUdpSocketOut != NULL)
	{
		/*int retVal = STATUS_ERROR;
		retVal = m_pUdpSocketOut->CloseUdpSocket();
		
		if (retVal == STATUS_ERROR)
		{
			TRACEINTO << "MM ERROR CTaskMediaChannel::CloseTxSocket. CloseUdpSocket Failed " << " TaskName: " << GetTaskName();
			return;
		}*/
		
		POBJDELETE(m_pUdpSocketOut);
		
		
		TRACEINTO << "CTaskMediaChannel::CloseTxSocket - done " << " TaskName: " << GetTaskName();
	}
	else
	{
		TRACEINTO << "MM ERROR CTaskMediaChannel::CloseTxSocket - m_pUdpSocketOut is NULL " << " TaskName: " << GetTaskName();
	}
}




//Media Operations
///////////////////////

////////////////////////////////////////////////////////


void CTaskMediaChannel::StartMediaTx()
{
	DumpEventMessageList();
	
	// get media name and load to repository (if not exists) and set local pointer to data
	int status = SetupMediaFile();
	if( STATUS_OK != status)
	{
		TRACEINTO << "MM ERROR CTaskMediaChannel::StartMediaTx - Error in SetupMediaFile. TaskName: " << GetTaskName();
		return;
	}
	
	//socket init
	CreateTxSocket();
	InitTxSocket();
	
	
	//media processing
	PrepareMediaFrame();
	StartTimer (TIMER_SEND_MEDIA, 1);
	
	m_bIsChannelOpen = TRUE;
}

////////////////////////////////////////////////////////


void CTaskMediaChannel::StopMediaTx()
{
	DeleteTimer(TIMER_SEND_MEDIA);
	
	m_readInd = 0;
	
	if (m_mediaRepositoryElement != NULL)
		m_mediaRepositoryElement->DecRefCounter();
}

////////////////////////////////////////////////////////

void CTaskMediaChannel::OnTimerSendMedia(CSegment* pParam)
{
	//TRACEINTO << "CTaskMediaChannel::OnTimerSendMedia";	
	
	// start timer for next transmission
	if (m_timeToNextTransmission == 0)
	{
		//TRACEINTO << "CTaskMediaChannel::OnTimerSendMedia m_timeToNextTransmission==0";
		m_timeToNextTransmission = 3; //3 is good for video, audio-?
	}
	
	StartTimer (TIMER_SEND_MEDIA, m_timeToNextTransmission);
	
	// send all packets of current frame
	SendMedia();

	
	PrepareMediaFrame();
}


////////////////////////////////////////////////////////


void CTaskMediaChannel::PrepareMediaFrame()
{
	BYTE tempCmHeader[sizeof(TMediaHeaderIpNetwork)];	// 40 bytes
	BYTE tempRtpHeader[sizeof(TRtpHeader)];	// 12 bytes
	
	DWORD bigEndTimeStamp = 0;
	DWORD litEndTimeStamp = 0;
	
	int packetIndexInFrame = 0;
	int increaseIndexParam = 0;		// for packet padding (size should be in multiples of 4Byte)
	
	BYTE packetMarker = 0;
	DWORD savedLastUsedTS = m_lastUsedTS;
	WORD savedPacketSeqNumber = m_packetSeqNumber;
	
	if (m_readInd == 0)
	{
		//TRACEINTO << "CTaskMediaChannel::PrepareMediaFrame - start of media buffer, m_readInd=0 TaskName: " << GetTaskName();
		if (m_packetSeqNumber > 0)
		{
			m_iterationDeltaTS = m_eofFactorTS + (m_lastUsedTS - m_firstBufferTS);
		}
	}
	
	do
	{
		// get CM header (size: 40 Bytes)
		if (m_readInd > m_fileSize)
		{
			TRACEINTO << "CTaskMediaChannel::PrepareMediaFrame - m_readInd = " << m_readInd << " is bigger than m_fileSize = " << m_fileSize;
			m_readInd = 0;
		}
		
		memcpy(tempCmHeader, &m_mediaFileBuffer[m_readInd], sizeof(TMediaHeaderIpNetwork));

		if (packetIndexInFrame == MAX_PACKETS_PER_FRAME)
		{
			TRACEINTO << "CTaskMediaChannel::PrepareMediaFrame - packetIndexInFrame = " << MAX_PACKETS_PER_FRAME;
			return;
		}
		m_payloadSize[packetIndexInFrame] = ((TMediaHeaderIpNetwork*)(tempCmHeader))->tCommonHeader.nPayloadSize;
		m_readInd = m_readInd + sizeof(TMediaHeaderIpNetwork);	// increase readIndex to point after the CM header 
		
		if (m_payloadSize[packetIndexInFrame] > MAX_PACKET_SIZE)
		{
			TRACEINTO << "CTaskMediaChannel::PrepareMediaFrame - payloadSize = " << m_payloadSize[packetIndexInFrame] << " is > " << MAX_PACKET_SIZE;
			return;
		}
	
		// get current packet (size: payloadSize)
		memcpy(m_packetsArr[packetIndexInFrame], &m_mediaFileBuffer[m_readInd], m_payloadSize[packetIndexInFrame]);
		increaseIndexParam = 0;
		int sizeMod4 = m_payloadSize[packetIndexInFrame] % 4;	// packet size should be in multiples of 4Bytes
		if (0 != sizeMod4)		// if not divided by 4B without remainder - round off up
			increaseIndexParam = 4 - sizeMod4;
		
		m_readInd = m_readInd + m_payloadSize[packetIndexInFrame] + increaseIndexParam;	// increase readIndex to point after first packet
					
		//counts the number of packets in the current frame
		m_numOfPacketsForTrans++;

		// get time stamp from buffer
		memcpy(tempRtpHeader, &m_packetsArr[packetIndexInFrame][0], sizeof(TRtpHeader));
		bigEndTimeStamp = ((TRtpHeader*)(tempRtpHeader))->unTimeStamp;
		
		// convert DWORD endian-ness
		litEndTimeStamp = (bigEndTimeStamp>>24) | (((bigEndTimeStamp>>16) & 0xFF)<<8) | (((bigEndTimeStamp>>8) & 0xFF)<<16) | ((bigEndTimeStamp & 0xFF)<<24);
		if (packetIndexInFrame == 0)
			m_currentFrameTS = litEndTimeStamp;
		
		
		/////////////////////////////////////////////////////
		//Payload type - set payload type from signaling
		BYTE payloadTypeMarker = ((TRtpHeader*)(tempRtpHeader))->markerPlusPayloadType;
		packetMarker = payloadTypeMarker & 0x80;
		payloadTypeMarker = (m_payloadType & 0xFF) | packetMarker;
		memcpy(&m_packetsArr[packetIndexInFrame][1], &payloadTypeMarker, sizeof(BYTE));
		/////////////////////////////////////////////////////		
		

		/////////////////////////////////////////////////////
		//Set Sequence number
		//convert seq number into big endian
		WORD convertedSeqNumber = (m_packetSeqNumber >> 8) | ((m_packetSeqNumber & 0xFF) << 8);
		memcpy(&m_packetsArr[packetIndexInFrame][2], &convertedSeqNumber, sizeof(WORD));
		/////////////////////////////////////////////////////
				
		
		/////////////////////////////////////////////////////
		//Time stamp	
		// calculate the new timestamp
		DWORD litEndTmpTS = litEndTimeStamp + m_iterationDeltaTS;
		
		//happens only once at beginning of buffer transmission
		if (m_firstBufferTS == 0)
			m_firstBufferTS = litEndTmpTS;
		
		//save the last used TS
		m_lastUsedTS = litEndTmpTS;
		
		//convert litEndTmpTS into big endian
		DWORD bigEndTmpTS = (litEndTmpTS>>24) | (((litEndTmpTS>>16) & 0xFF)<<8) | (((litEndTmpTS>>8) & 0xFF)<<16) | ((litEndTmpTS & 0xFF)<<24);
		memcpy(&m_packetsArr[packetIndexInFrame][4], &bigEndTmpTS, sizeof(DWORD));
		
		/////////////////////////////////////////////////////
		
				
		/////////////////////////////////////////////////////
		// preparation for next packet
		packetIndexInFrame++;
		m_packetSeqNumber++;
		
	
		//check if end of file and packet is not marker -> drop frame
		if (GetTaskName() == "TaskVideoChannel")
		{			
			if (m_readInd >= m_fileSize && packetMarker==0 && (litEndTimeStamp==m_currentFrameTS))
			{
				TRACEINTO << "MM ERROR CTaskMediaChannel::PrepareMediaFrame - last frame is without marked packet. m_readInd = " << m_readInd << " m_fileSize = " << m_fileSize << " task: " << GetTaskName();				
				
				//reset parameters
				bigEndTimeStamp = 0;
				litEndTimeStamp = 0;				
				packetIndexInFrame = 0;
				increaseIndexParam = 0;
				packetMarker = 0;
				m_lastUsedTS = savedLastUsedTS;
				m_iterationDeltaTS = (savedLastUsedTS - m_firstBufferTS);
				m_packetSeqNumber = savedPacketSeqNumber;
				m_readInd = 0;
				m_currentFrameTS = 0; //just to make the condition TRUE ....check what happens with 'continue'//			
			}
		}
			
		
					
	} while	((litEndTimeStamp == m_currentFrameTS) && (m_readInd < m_fileSize));
		

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	// move m_readInd back to start of last packet that was read (this is the first packet for next transmission) and decrease packetSeqNumber
	if (litEndTimeStamp != m_currentFrameTS)
	{
		m_readInd = m_readInd - (sizeof(TMediaHeaderIpNetwork) + m_payloadSize[packetIndexInFrame-1] + increaseIndexParam);
		m_packetSeqNumber--;
	}
	
	//////////////////////////////////////////////////////////////////////////////////////
	//buffer m_readInd is over file size --> loop again over the file or prepare the next dtmf
	if (m_readInd >= m_fileSize)
	{
		TRACEINTO << "CTaskMediaChannel::PrepareMediaFrame - m_readInd >= m_fileSize" << " TaskName: " << GetTaskName();
		
		m_readInd = 0;
	
		//check if this is the end of a dtmf transmission
		if (m_bIsDtmfSession == TRUE)
		{
			((CTaskAudioChannel*)this)->PrepareNextDtmf();
		}
	}
	
	m_timeToNextTransmission = GetTimeToNextTransmission(litEndTimeStamp);
}

////////////////////////////////////////////////////////

void CTaskMediaChannel::StartMediaRx()
{
	// Create Rx Socket Task
	m_pUdpListenSocketTaskApi = new CTaskApi;
	COsQueue& rcvQ = GetRcvMbx();
	
	TaskEntryPoint udpRxSocketTaskEntryPoint = GetMediaRxTaskEntryPoint();
	m_pUdpListenSocketTaskApi->Create(udpRxSocketTaskEntryPoint, rcvQ);

	char szLocalIP[IP_ADDRESS_LEN];
	::SystemDWORDToIpString(ntohl(m_local_addr.sin_addr.s_addr), szLocalIP);
	TRACEINTO << "CTaskMediaChannel::StartMediaRx family:" << m_local_addr.sin_family << " ip: " << szLocalIP << " port: " << ntohs(m_local_addr.sin_port) << " TaskName: " << GetTaskName();
			
	CSegment* pMsg = new CSegment;
	*pMsg	<< (DWORD)m_local_addr.sin_family
			<< (DWORD)m_local_addr.sin_port
			<< (DWORD)m_local_addr.sin_addr.s_addr;			
			
	m_pUdpListenSocketTaskApi->SendMsg(pMsg, SET_PARAM_UDP_SOCKET_MSG);
	
	//writing file
	/*CSegment* pMsgWrite = new CSegment;
	string fileName = "File";
	fileName += GetTaskName();
	*pMsgWrite	<< fileName.c_str();
	m_pUdpListenSocketTaskApi->SendMsg(pMsgWrite, START_WRITE_STEAM_UDP_SOCKET_MSG);*/
	//writing file
	
	
	SystemSleep(500);
	CSegment* pMsg1 = new CSegment;
	m_pUdpListenSocketTaskApi->SendMsg(pMsg1, RECV_DATA_FROM_UDP_SOCKET_MSG);
	
	m_bIsChannelOpen = TRUE;
}

////////////////////////////////////////////////////////

void CTaskMediaChannel::PrepareMediaOutAddressing()
{
	if (m_taRemoteAddress.ipVersion == eIpVersion4)
	{
		char szRemoteIP[IP_ADDRESS_LEN];
		::SystemDWORDToIpString(m_taRemoteAddress.addr.v4.ip, szRemoteIP);
		TRACEINTO << "CTaskMediaChannel::PrepareMediaOutAddressing remoteIP:" << szRemoteIP << " remotePort: " << m_taRemoteAddress.port << " TaskName: " << GetTaskName();
		
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
		TRACEINTO << "MM ERROR CTaskMediaChannel::PrepareMediaOutAddressing - Local address is NOT ip ver 4." << " TaskName: " << GetTaskName();;
	}
}

//////////////////////////////////////////

void CTaskMediaChannel::PrepareMediaInAddressing()
{
	if (m_taLocalAddress.ipVersion == eIpVersion4)
	{
		char szLocalIP[IP_ADDRESS_LEN];
		::SystemDWORDToIpString(m_taLocalAddress.addr.v4.ip, szLocalIP);
		TRACEINTO << "CTaskMediaChannel::PrepareMediaInAddressing localIp:" << szLocalIP << " localPort: " << m_taLocalAddress.port << " TaskName: " << GetTaskName();

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
		TRACEINTO << "MM ERROR CTaskMediaChannel::PrepareMediaInAddressing - Remote address is NOT ip ver 4. " << " TaskName: " << GetTaskName();
	}
}

//////////////////////////////////////////

void CTaskMediaChannel::SaveChannelParams(CMplMcmsProtocol* pMplProtocol)
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
	
	m_bRtpUpdatePort = TRUE;
}


//////////////////////////////////////////

int CTaskMediaChannel::GetTimeToNextTransmission(DWORD timeStamp)
{
	return 0;	
}


//////////////////////////////////////////

void CTaskMediaChannel::CloseChannel()
{
	if (!m_bIsChannelOpen)
	{
		TRACEINTO << "CTaskMediaChannel::CloseChannel channel is signed as closed " << m_channelDirection << " TaskName: " << GetTaskName();
		return;
	}
	
	if (m_channelDirection == MEDIA_DIRECTION_OUT)
	{
		TRACEINTO << "CTaskMediaChannel::CloseChannel Tx start " << " TaskName: " << GetTaskName();
		
		DeleteTimer(TIMER_SEND_MEDIA);
		
		CloseTxSocket();
		
		m_bIsChannelOpen = FALSE;
		
		TRACEINTO << "CTaskMediaChannel::CloseChannel Tx end  " << " TaskName: " << GetTaskName();
	}
	else if (m_channelDirection == MEDIA_DIRECTION_IN)
	{
		TRACEINTO << "CTaskMediaChannel::CloseChannel Rx start " << " TaskName: " << GetTaskName();
		
		if (m_pUdpListenSocketTaskApi)
		{
			//writing file
			/*CSegment* pMsgWrite = new CSegment;
			m_pUdpListenSocketTaskApi->SendMsg(pMsgWrite, STOP_WRITE_STEAM_UDP_SOCKET_MSG);*/
			//stop writing file
			
			
			CSegment* pMsg = new CSegment;
			m_pUdpListenSocketTaskApi->SendMsg(pMsg, CLOSE_UDP_SOCKET_MSG);	
			
			//after Rx socket is closed -> destroy task
			m_pUdpListenSocketTaskApi->Destroy();
			POBJDELETE(m_pUdpListenSocketTaskApi);
			
			m_bIsChannelOpen = FALSE;
		}
		else
		{
			TRACEINTO << "MM ERROR CTaskMediaChannel::CloseChannel m_pUdpListenSocketTaskApi is NULL ? " << " TaskName: " << GetTaskName();
		}

		
		TRACEINTO << "CTaskMediaChannel::CloseChannel Rx end " << " TaskName: " << GetTaskName();
	}
}


//////////////////////////////////////////
//CTaskVideoChannel
//////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//  task creation function
/////////////////////////////////////////////////////////////////////////////

void VideoOutChannelEntryPoint(void* appParam)
{
	CTaskVideoChannel* pTaskVideoOutChannel = new CTaskVideoChannel(MEDIA_DIRECTION_OUT);
	pTaskVideoOutChannel->Create(*(CSegment*)appParam);
}

void VideoInChannelEntryPoint(void* appParam)
{
	CTaskVideoChannel* pTaskVideoInChannel = new CTaskVideoChannel(MEDIA_DIRECTION_IN);
	pTaskVideoInChannel->Create(*(CSegment*)appParam);
}


PBEGIN_MESSAGE_MAP(CTaskVideoChannel)
	
	ONEVENT( VIDEO_OUT_PARAM_EVENT_MESSAGE,			ANYCASE,   CTaskVideoChannel::OnVideoOutParamMessageAll)
	ONEVENT( TIMER_ENABLE_INTRA,					ANYCASE,   CTaskVideoChannel::OnTimerEnableIntra)
	ONEVENT( VIDEO_OUT_UPDATE_PARAM_EVENT_MESSAGE,	ANYCASE,   CTaskVideoChannel::OnVideoOutUpdateParamMessageAll)
	
PEND_MESSAGE_MAP(CTaskVideoChannel, CTaskMediaChannel);


CTaskVideoChannel::CTaskVideoChannel(INT32  channelDirection) : CTaskMediaChannel(channelDirection)
{	 
	m_framePosition = 0;
	m_eVideoOutProtocol = E_VIDEO_PROTOCOL_DUMMY;
	m_bAllowIntraNow = TRUE;
	m_eofFactorTS = 3600;
}

CTaskVideoChannel::~CTaskVideoChannel()
{
	if (m_channelDirection == MEDIA_DIRECTION_OUT)
	{
		TRACEINTO << "CTaskVideoChannel::~CTaskVideoChannel() Out";
	}
	else if (m_channelDirection == MEDIA_DIRECTION_IN)
	{
		TRACEINTO << "CTaskVideoChannel::~CTaskVideoChannel() In";
	}
}

//////////////////////////////////////////

string CTaskVideoChannel::ChannelData()
{
	string str = "";
	str = CTaskMediaChannel::ChannelData();
	
	//add relevant data for Video Channel
	//str += "\some video parameter: ";
	//str += GetVideoParameterStr();
	
	return str.c_str();
}

/////////////////////////////////////////////////////////////////////////////

void  CTaskVideoChannel::SelfKill()
{
	TRACEINTO << "CTaskVideoChannel::SelfKill";
	CTaskMediaChannel::SelfKill();
}

//////////////////////////////////////////

int CTaskVideoChannel::SetupMediaFile()
{
	string fullFileName = ::GetMediaMngrCfg()->GetInstallationVideoFileReadPath();
	fullFileName += "/";
	fullFileName += m_strFileName.substr(0, 4); //temp for changing the video movie   //vid.vid
	fullFileName += "/";															   //vid.vid
	fullFileName += m_strFileName;
	
	TRACEINTO << "CTaskVideoChannel::SetupMediaFile - video full file name: " << fullFileName;
	
	//loading the video buffer from repository
	m_mediaRepositoryElement = ::GetMediaRepository()->GetVideoDB()->GetMediaElement(fullFileName);
	if (m_mediaRepositoryElement == NULL)
	{
		TRACEINTO << "MM ERROR CTaskVideoChannel::SetupMediaFile - m_mediaRepositoryElement is NULL";
		return STATUS_ERROR;
	}
	
	m_mediaFileBuffer = m_mediaRepositoryElement->GetDataBuffer();
	m_fileSize = m_mediaRepositoryElement->GetSize();
	
	return STATUS_OK;
}


//////////////////////////////////////////

int CTaskVideoChannel::GetTimeToNextTransmission(DWORD timeStamp/*, TBD - frameRate */ )
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


//////////////////////////////////////////

void CTaskVideoChannel::SetFramePosition(DWORD timeStamp)
{
	if (FRAME_POSITION_THIRD == m_framePosition) 
		m_framePosition = FRAME_POSITION_FIRST;
	else
		m_framePosition++;
}


//////////////////////////////////////////

void CTaskVideoChannel::SendMedia()
{
	//TRACEINTO << "CTaskVideoChannel::SendMedia - m_numOfPacketsForTrans: " << m_numOfPacketsForTrans;
	for (int i = 0; i < m_numOfPacketsForTrans-1; i++)	// last packet is for next transmission
	{	
		//TRACEINTO << "CTaskVideoChannel::SendMedia - index: " << i << "m_packetsArr: " << m_packetsArr[i] << "m_payloadSize: " << m_payloadSize[i];
		SendTxSocket(m_packetsArr[i], m_payloadSize[i]);
	}	
	
	m_numOfPacketsForTrans = 0;
}

//////////////////////////////////////////

void CTaskVideoChannel::RestartMediaTx()
{
	TRACEINTO << "CTaskVideoChannel::RestartMediaTx";
	
	if (m_bAllowIntraNow == TRUE)
	{
		TRACEINTO << "CTaskVideoChannel::RestartMediaTx invoke intra";
		m_readInd = 0;
		m_bAllowIntraNow = FALSE;
		StartTimer (TIMER_ENABLE_INTRA, 500);
	}
}

////////////////////////////////////////////////////////

void CTaskVideoChannel::OnTimerEnableIntra(CSegment* pParam)
{
	TRACEINTO << "CTaskVideoChannel::OnTimerEnableIntra";	
	
	m_bAllowIntraNow = TRUE;
}

////////////////////////////////////////////////////////

void CTaskVideoChannel::OnVideoOutParamMessageAll(CSegment* pParam)
{
	TRACEINTO << "CTaskVideoChannel::OnVideoOutParamMessageAll";

	DWORD protocol, bitrate, res, framerate;
	*pParam	>> protocol;
	
	m_eVideoOutProtocol = (EVideoProtocol)protocol;
	
	*pParam	>> bitrate
			>> res
			>> framerate;
	
	
	m_tCurrentVideoParam.nVideoBitRate = bitrate;
	m_tCurrentVideoParam.eVideoResolution = (EMMVideoResolution)res;
	m_tCurrentVideoParam.eVideoFrameRate = (EMMVideoFrameRate)framerate;
	

	TRACEINTO << "CTaskVideoChannel::OnVideoOutParamMessageAll res=" << EMMVideoResolutionNames[m_tCurrentVideoParam.eVideoResolution]
	                                                    << " framerate=" << EShortMMVideoFrameRateNames[m_tCurrentVideoParam.eVideoFrameRate]
	                                                                                                    << " bitrate=" << m_tCurrentVideoParam.nVideoBitRate;
	
	
	//build video out file name
	////////////////////////////

	//get bitrate in string
	string bitratestr = GetBitrateStr(bitrate);
	
	m_strFileName = GetMediaPrefix();
	
	if (m_eVideoOutProtocol == E_VIDEO_PROTOCOL_H264)
		m_strFileName += "_PTC264";
	else if (m_eVideoOutProtocol==E_VIDEO_PROTOCOL_H263)
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
	
	TRACEINTO << "CTaskVideoChannel::OnVideoOutParamMessageAll file name: " << m_strFileName;
	
	//m_strFileName = "vid.vid";
	//m_strFileName = "V003_PTC264_RES4CIF_FR25_BR704.vid";
	
	
	m_bVideoOutParam = TRUE;
	
	if (m_bRtpUpdatePort && m_bOpenUdpPort)
		StartMediaTx();
}



////////////////////////////////////////////////////////

void CTaskVideoChannel::OnVideoOutUpdateParamMessageAll(CSegment* pParam)
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
	
	if (m_eVideoOutProtocol == E_VIDEO_PROTOCOL_H264)
		m_strFileName += "_PTC264";
	else if (m_eVideoOutProtocol==E_VIDEO_PROTOCOL_H263)
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
	
	TRACEINTO << "CTaskVideoChannel::OnVideoOutUpdateParamMessageAll file name: " << m_strFileName;

	int status = SetupMediaFile();
	if( STATUS_OK != status)
	{
		TRACEINTO << "MM ERROR CTaskVideoChannel::OnVideoOutUpdateParamMessageAll - Error in SetupMediaFile.";
		return;
	}
	
	// reset media buffer index
	m_readInd = 0;
	
	// call prepare media frame to be transmited in the next timer - only in the FIRST dtmf session
	PrepareMediaFrame();
}

////////////////////////////////////////////////////////


string CTaskVideoChannel::GetBitrateStr(DWORD bitrate)
{
	string bitratestr = "";
	
	if (bitrate < 192000)
		bitratestr = VIDEO_64_KBPS;
	else if (bitrate >= 192000 && bitrate < 256000)
		bitratestr = VIDEO_192_KBPS;
	else if (bitrate >= 256000 && bitrate < 384000)
		bitratestr = VIDEO_320_KBPS;
	else if (bitrate >= 384000 && bitrate < 512000)
		bitratestr = VIDEO_448_KBPS;
	else if (bitrate >= 512000 && bitrate < 768000)
		bitratestr = VIDEO_704_KBPS;
	else if (bitrate >= 768000 && bitrate < 1024000)
		bitratestr = VIDEO_960_KBPS;
	else if (bitrate >= 1024000 && bitrate < 1472000)
		bitratestr = VIDEO_1408_KBPS;
	else if (bitrate >= 1472000 && bitrate < 1920000)
		bitratestr = VIDEO_1856_KBPS;
	
	return bitratestr;
}

/////////////////////////////////////////////////////////////////////////////

void CTaskVideoChannel::UpdateVideoParam(DWORD contentBitrate, DWORD contentMode)
{
	DWORD newVideoBitrate = 0;
	// content mode ON --> save video data
	if (contentMode)
	{
		m_tSaveContentVideoParam.nVideoBitRate = m_tCurrentVideoParam.nVideoBitRate;
		m_tSaveContentVideoParam.eVideoResolution = m_tCurrentVideoParam.eVideoResolution;
		m_tSaveContentVideoParam.eVideoFrameRate = m_tCurrentVideoParam.eVideoFrameRate;
		
		//update current video param
		m_tCurrentVideoParam.nVideoBitRate = m_tCurrentVideoParam.nVideoBitRate - contentBitrate;
		
		TRACEINTO << "CTaskVideoChannel::UpdateVideoParam new Video Bitrate=" << m_tCurrentVideoParam.nVideoBitRate;
	}
	else //content mode OFF --> retrieve saved video params
 	{
		m_tCurrentVideoParam.nVideoBitRate = m_tSaveContentVideoParam.nVideoBitRate;
		m_tCurrentVideoParam.eVideoResolution = m_tSaveContentVideoParam.eVideoResolution;
		m_tCurrentVideoParam.eVideoFrameRate = m_tSaveContentVideoParam.eVideoFrameRate;
	}
}

/////////////////////////////////////////////////////////////////////////////

/*
void CTaskVideoChannel::OldUpdateVideoParam(ENCODER_PARAM_S* pEncoderParamsStruct)
{
	if (pEncoderParamsStruct == NULL)
	{
		TRACEINTO << "MM ERROR CTaskVideoChannel::OldUpdateVideoParam pEncoderParamsStruct == NULL";
		return;
	}
	
	//ONLY bitrate has changed
	
	if (pEncoderParamsStruct->nProtocol != m_eVideoOutProtocol)
	{
		TRACEINTO << "MM ERROR CTaskVideoChannel::OldUpdateVideoParam pEncoderParamsStruct->nProtocol!=m_eVideoOutProtocol,"
									<<" new video protocol: " << EVideoProtocolNames[pEncoderParamsStruct->nProtocol];
		return;
	}
	
	TRACEINTO << "CTaskVideoChannel::OldUpdateVideoParam pEncoderParamsStruct->nPnBitRaterotocol: " << pEncoderParamsStruct->nBitRate;		

	m_tCurrentVideoParam.nVideoBitRate = pEncoderParamsStruct->nBitRate;
	
	///1. build new file name
	//string videoName = m_strFileName.substr(0, 4);
	string videoName = "V002";
	string fullFileName = ::GetMediaMngrCfg()->GetVideoFileReadPath();
	fullFileName += "/";
	fullFileName += videoName;
	fullFileName += "/";
	
		
	m_strFileName = videoName;
	
	if (m_eVideoOutProtocol == E_VIDEO_PROTOCOL_H264)
		m_strFileName += "_PTC264";
	else if (m_eVideoOutProtocol == E_VIDEO_PROTOCOL_H263)
		m_strFileName += "_PTC263";
	else
		m_strFileName += "_PTCUNKNOWN";
	
	m_strFileName += "_RES";
	m_strFileName += EShortMMVideoResolutionNames[m_tCurrentVideoParam.eVideoResolution];
	m_strFileName += "_FR";
	m_strFileName += EShortMMVideoFrameRateNames[m_tCurrentVideoParam.eVideoFrameRate];
	m_strFileName += "_BR";
	m_strFileName += GetBitrateStr(m_tCurrentVideoParam.nVideoBitRate); //with new bitrate
	m_strFileName += ".vid";
	
	fullFileName += m_strFileName;
	
		
	TRACEINTO << "CTaskVideoChannel::OldUpdateVideoParam file name: " << m_strFileName;
	
	
	//2. stop current video - decrement to repository
	m_mediaRepositoryElement->DecRefCounter();
	
	
	//3. get new media element
	m_mediaRepositoryElement = ::GetMediaRepository()->GetVideoDB()->GetMediaElement(fullFileName);
	if (m_mediaRepositoryElement == NULL)
	{
		TRACEINTO << "MM ERROR CTaskVideoChannel::OldUpdateVideoParam - m_mediaRepositoryElement is NULL";
		return;
	}
	
	//4. PrepareMediaFrame	
	m_mediaFileBuffer = m_mediaRepositoryElement->GetDataBuffer();
	m_fileSize = m_mediaRepositoryElement->GetSize();
	m_readInd = 0;
	
	PrepareMediaFrame();
}
*/



/////////////////////////////////////////////////////////////////////////////

int videoCounter = 0;

string CTaskVideoChannel::GetMediaPrefix()
{
	videoCounter++;
	if (videoCounter == 5)
		videoCounter = 1;
	
	string video = "";
	
	switch (videoCounter)
	{
		case 1:
			video = "V001";
			break;
		case 2:
			video = "V002";
			break;
		case 3:
			video = "V003";
			break;
		case 4:
			video = "V004";
			break;
	}
	
	return video;
}



//////////////////////////////////////////
//CTaskAudioChannel
//////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//  task creation function
/////////////////////////////////////////////////////////////////////////////

void AudioOutChannelEntryPoint(void* appParam)
{
	CTaskAudioChannel* pTaskAudioOutChannel = new CTaskAudioChannel(MEDIA_DIRECTION_OUT);
	pTaskAudioOutChannel->Create(*(CSegment*)appParam);
}

void AudioInChannelEntryPoint(void* appParam)
{
	CTaskAudioChannel* pTaskAudioInChannel = new CTaskAudioChannel(MEDIA_DIRECTION_IN);
	pTaskAudioInChannel->Create(*(CSegment*)appParam);
}



PBEGIN_MESSAGE_MAP(CTaskAudioChannel)

PEND_MESSAGE_MAP(CTaskAudioChannel, CTaskMediaChannel);

CTaskAudioChannel::CTaskAudioChannel(INT32 channelDirection) : CTaskMediaChannel(channelDirection)
{
	m_numOfDtmf = 0;
	m_currDtmfIndex = 0;
	
	m_saveMediaFileBuffer = NULL;
	m_saveFileSize = 0;
	m_saveReadInd = 0;
	
	m_eofFactorTS = 160;
}

CTaskAudioChannel::~CTaskAudioChannel()
{
	if (m_channelDirection == MEDIA_DIRECTION_OUT)
	{
		TRACEINTO << "CTaskAudioChannel::~CTaskAudioChannel() Out";
	}
	else if (m_channelDirection == MEDIA_DIRECTION_IN)
	{
		TRACEINTO << "CTaskAudioChannel::~CTaskAudioChannel() In";
	}
}

/////////////////////////////////////////////////////////////////////////////

string CTaskAudioChannel::ChannelData()
{
	string str = "";
	str = CTaskMediaChannel::ChannelData();
	
	//add relevant data for Audio Channel
	//str += "\some audio parameter: ";
	//str += GetAudioParameterStr();
	
	return str.c_str();
}

/////////////////////////////////////////////////////////////////////////////

void  CTaskAudioChannel::SelfKill()
{
	TRACEINTO << "CTaskAudioChannel::SelfKill";
	CTaskMediaChannel::SelfKill();
}

////////////////////////////////////////////////////////

int CTaskAudioChannel::SetupMediaFile()
{
	string fullFileName = ::GetMediaMngrCfg()->GetInstallationAudioFileReadPath();
	fullFileName += "/";
	m_strFileName = GetMediaPrefix();
	m_strFileName += "_PTC";	
	
	switch (m_capTypeCode)
	{
		//G711
		case eG711Ulaw64kCapCode:
		case eG711Alaw64kCapCode:
		{
			m_strFileName += "G711_BR64";
			break;
		}
		
		//G722
		case eG722_48kCapCode:
		{
			m_strFileName += "G722_BR48";
			break;
		}
		case eG722_56kCapCode:
		{
			m_strFileName += "G722_BR56";
			break;
		}
		case eG722_64kCapCode:
		{
			m_strFileName += "G722_BR64";
			break;
		}


		//G729
		case eG729CapCode:
		case eG729AnnexACapCode:
		case eG729wAnnexBCapCode:
		case eG729AnnexAwAnnexBCapCode:
		{	
			m_strFileName += "G729_BR8";
			break;
		}
			
		//G722.1
		/*case eG7221_16kCapCode:
		{	
			m_strFileName += "G722.1_BR16";
			break;
		}*/
		case eG7221_24kCapCode:
		{	
			m_strFileName += "G722.1_BR24";
			break;
		}
		case eG7221_32kCapCode:
		{	
			m_strFileName += "G722.1_BR32";
			break;
		}		


		//Siren14
		case eSiren14_24kCapCode:
		{			
			m_strFileName += "SIREN14_BR24";
			break;
		}
		case eSiren14_32kCapCode:
		{			
			m_strFileName += "SIREN14_BR32";
			break;
		}
		case eSiren14_48kCapCode:
		{			
			m_strFileName += "SIREN14_BR48";
			break;
		}

			
		//G722.1.C
		case eG7221C_24kCapCode:
		{
			m_strFileName += "G722.1.C_BR24";
			break;
		}
		case eG7221C_32kCapCode:
		{
			m_strFileName += "G722.1.C_BR32";
			break;
		}
		case eG7221C_48kCapCode:
		{
			m_strFileName += "G722.1.C_BR48";
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

		default:
			
			TRACEINTO << "MM ERROR CTaskAudioChannel::SetupMediaFile - MM does not support this Audio CODEC: "
					  << GetCapTypeCodeStr() << ".  Selecting the default CODEC";
			
			m_strFileName = "G711_BR64";
			
	}	
	
	m_strFileName += ".aud";
	
	//YYYYY
	//m_strFileName = "MyAudioFile0.txt";	
	
	fullFileName += m_strFileName.substr(0, 4); //temp for changing the audio
	fullFileName += "/";
	fullFileName += m_strFileName;
	
	
	TRACEINTO << "CTaskAudioChannel::SetupMediaFile - audio full file name: " << fullFileName;

	//loading the audio buffer from repository
	m_mediaRepositoryElement = ::GetMediaRepository()->GetAudioDB()->GetMediaElement(fullFileName);
	if (m_mediaRepositoryElement == NULL)
	{
		TRACEINTO << "MM ERROR CTaskAudioChannel::SetupMediaFile - m_mediaRepositoryElement is NULL";
		return STATUS_ERROR;
	}
	
	m_mediaFileBuffer = m_mediaRepositoryElement->GetDataBuffer();
	m_fileSize = m_mediaRepositoryElement->GetSize();
	
	return STATUS_OK;
}


//////////////////////////////////////////

int CTaskAudioChannel::GetTimeToNextTransmission(DWORD timeStamp)
{
	DWORD deltaTS = timeStamp - m_currentFrameTS;
	//TRACEINTO << "CTaskAudioChannel::GetTimeToNextTransmission@ - deltaTS: " << deltaTS;
	
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
		case eG7221_16kCapCode:
		{	
			transmitFactor = 16;
			break;
		}

		//Siren14
		case eSiren14_48kCapCode:
		case eSiren14_32kCapCode:
		case eSiren14_24kCapCode:
		//G722.1.C
		case eG7221C_48kCapCode:
		case eG7221C_32kCapCode:
		case eG7221C_24kCapCode:
		case eG7221C_CapCode:
		{			
			transmitFactor = 32;
			break;
		}
		

		default:
			
			transmitFactor = 8;
	}	
	
	//TRACEINTO << "CTaskAudioChannel::GetTimeToNextTransmission@@ - transmitFactor: " << transmitFactor;
	
	DWORD timeToSleep = deltaTS/transmitFactor;
	
	//TRACEINTO << "CTaskAudioChannel::GetTimeToNextTransmission@@ - time to sleep: " << timeToSleep;
	
	
	DWORD currentFraction = timeToSleep % 10;
	
	if (currentFraction != 0)
	{
		m_fraction += currentFraction;
		//TRACEINTO << "CTaskAudioChannel::GetTimeToNextTransmission@@ - currentFraction: " << currentFraction;
		//TRACEINTO << "CTaskAudioChannel::GetTimeToNextTransmission@@ - m_fraction: " << m_fraction;
		
		if (m_fraction >= 10)
		{
			timeToSleep += 10;
			m_fraction -= 10;
		}
	}	
	
	//TRACEINTO << "CTaskAudioChannel::GetTimeToNextTransmission@@ - timeToSleep: " << timeToSleep;
	//TRACEINTO << "CTaskAudioChannel::GetTimeToNextTransmission@@ - m_fraction: " << m_fraction;
	
	
	timeToSleep /= 10; // becuase TIMER is in 1/100 sec
	
	//TRACEINTO << "$$Audio - timeToSleep= " << timeToSleep;
	
	return timeToSleep;
}


//////////////////////////////////////////

void CTaskAudioChannel::SendMedia()
{
	SendTxSocket(m_packetsArr[0], m_payloadSize[0]);
}

//////////////////////////////////////////


int audioCounter = 0;

string CTaskAudioChannel::GetMediaPrefix()
{
	audioCounter++;
	if (audioCounter == 5)
		audioCounter = 1;
	
	string audio = "";
	
	switch (audioCounter)
	{
		case 1:
			audio = "A001";
			break;
		case 2:
			audio = "A002";
			break;
		case 3:
			audio = "A003";
			break;
		case 4:
			audio = "A004";
			break;
	}
	
	return audio;
}



///////
//DTMF
//////

void CTaskAudioChannel::SendDTMF(SPlayToneStruct* tPlayToneStruct)
{
	TRACEINTO << "CTaskAudioChannel::SendDTMF";
	
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
					TRACEINTO << "MM ERROR CTaskAudioChannel::SendDTMF dtmf overflow, ignored: " << EAudioToneNames[tPlayToneStruct->tone[i].tTone]; 
			}
		}
		
		TRACEINTO << "CTaskAudioChannel::SendDTMF after adding dtmf to current buffer";
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
			
			TRACEINTO << "CTaskAudioChannel::SendDTMF after re-ordering dtmf buffer";
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
				TRACEINTO << "MM ERROR CTaskAudioChannel::SendDTMF dtmf overflow, ignored: " << EAudioToneNames[tPlayToneStruct->tone[i].tTone];
		}
	}
	
	PrintDtmfBuffer();
	
	
	TRACEINTO << "CTaskAudioChannel::SendDTMF m_numOfDtmf=" << m_numOfDtmf;
			
	
	m_currDtmfIndex = 0;
	
	CDtmfElement* dtmfElement = ::GetMediaRepository()->GetDtmfDB()->GetDtmfElement(GetCapTypeCode(), m_dtmfBuffer[m_currDtmfIndex]);
	if (dtmfElement == NULL)
	{
		TRACEINTO << "MM ERROR CTaskAudioChannel::SendDTMF dtmfElement is NULL at m_dtmfBuffer index=" << m_currDtmfIndex;
		return;
	}
	
	if (dtmfElement->GetDtmfBufferSize() == 0)
	{
		TRACEINTO << "MM ERROR CTaskAudioChannel::SendDTMF dtmfElement buffer is empty at m_dtmfBuffer index=" << m_currDtmfIndex;
		return;
	}
	
	//save original params
	m_saveMediaFileBuffer = m_mediaFileBuffer;
	m_saveFileSize = m_fileSize;
	m_saveReadInd = m_readInd;

		
	//entering DTMF session
	SetDtmfSession(TRUE);

	TRACEINTO << "CTaskAudioChannel::SendDTMF first tone: " << EAudioToneNames[m_dtmfBuffer[m_currDtmfIndex]];
	
	m_mediaFileBuffer = dtmfElement->GetDtmfBuffer();
	m_fileSize = dtmfElement->GetDtmfBufferSize();
	m_readInd = 0;
	
	// call prepare dtmf frame, to be transmited in the next timer - only in the FIRST dtmf session
	PrepareMediaFrame();
}



void CTaskAudioChannel::PrepareNextDtmf()
{
	TRACEINTO << "CTaskAudioChannel::PrepareNextDtmf";
	
	//increase dtmf current index
	m_currDtmfIndex++;
	
	//check if no more dtmf to transmit
	if (m_currDtmfIndex == m_numOfDtmf)
	{
		TRACEINTO << "CTaskAudioChannel::PrepareNextDtmf - No more dtmf to transmit. Exiting dtmf mode";
		
		//exiting DTMF session
		SetDtmfSession(FALSE);
		
		return;
	}
	
	//get the next dtmf to transmit
	CDtmfElement* dtmfElement = ::GetMediaRepository()->GetDtmfDB()->GetDtmfElement(GetCapTypeCode(), m_dtmfBuffer[m_currDtmfIndex]);
	if (dtmfElement == NULL)
	{
		TRACEINTO << "MM ERROR CTaskAudioChannel::PrepareNextDtmf dtmfElement is NULL at m_dtmfBuffer index=" << m_currDtmfIndex
				<< " Exiting DTMF session.";
		
		//exiting DTMF session
		SetDtmfSession(FALSE);
		
		return;
	}
	
	if (dtmfElement->GetDtmfBufferSize() == 0)
	{
		TRACEINTO << "MM ERROR CTaskAudioChannel::PrepareNextDtmf dtmfElement buffer is empty at m_dtmfBuffer index=" << m_currDtmfIndex
				<< " Exiting DTMF session.";
		
		//exiting DTMF session
		SetDtmfSession(FALSE);
		
		return;
	}
	
	TRACEINTO << "CTaskAudioChannel::PrepareNextDtmf tone: " << EAudioToneNames[m_dtmfBuffer[m_currDtmfIndex]];
	
	//set the params for next dtmf
	m_mediaFileBuffer = dtmfElement->GetDtmfBuffer();
	m_fileSize = dtmfElement->GetDtmfBufferSize();
	m_readInd = 0;
}

void CTaskAudioChannel::SetDtmfSession(BOOL isDtmfSession)
{
	if (isDtmfSession == TRUE)
	{
		TRACEINTO << "CTaskAudioChannel::SetDtmfSession - entering dtmf session";
		m_bIsDtmfSession = TRUE;
	}
	else
	{
		TRACEINTO << "CTaskAudioChannel::SetDtmfSession - exiting dtmf session";		
		m_bIsDtmfSession = FALSE;
		
		//restore saved param
		m_mediaFileBuffer = m_saveMediaFileBuffer;
		m_fileSize = m_saveFileSize;
		m_readInd = m_saveReadInd;
		
		//initialize dtmf array
		for (int i=0; i<MAX_DTMF_LEN; i++)
			m_dtmfBuffer[i] = E_AUDIO_TONE_DUMMY;
		
		m_numOfDtmf = 0;
		m_currDtmfIndex = 0;
	}
}

void CTaskAudioChannel::PrintDtmfBuffer()
{
	char tmp[MAX_DTMF_LEN];
	int i = 0;
	for (;i < m_numOfDtmf; i++)
		tmp[i] = ::GetTone(m_dtmfBuffer[i]);
	
	tmp[i] = '\0';
	
	TRACEINTO << "CTaskAudioChannel::PrintDtmfBuffer current dtmf buffer={" << tmp << "}";	
}


//////////////////////////////////////////
//CTaskContentChannel
//////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//  task creation function
/////////////////////////////////////////////////////////////////////////////

void ContentOutChannelEntryPoint(void* appParam)
{
	CTaskContentChannel* pTaskContentOutChannel = new CTaskContentChannel(MEDIA_DIRECTION_OUT);
	pTaskContentOutChannel->Create(*(CSegment*)appParam);
}

void ContentInChannelEntryPoint(void* appParam)
{
	CTaskContentChannel* pTaskContentInChannel = new CTaskContentChannel(MEDIA_DIRECTION_IN);
	pTaskContentInChannel->Create(*(CSegment*)appParam);
}

PBEGIN_MESSAGE_MAP(CTaskContentChannel)
	
PEND_MESSAGE_MAP(CTaskContentChannel, CTaskVideoChannel);

CTaskContentChannel::CTaskContentChannel(INT32 channelDirection) : CTaskVideoChannel(channelDirection)
{	 
}

CTaskContentChannel::~CTaskContentChannel()
{
	if (m_channelDirection == MEDIA_DIRECTION_OUT)
	{
		TRACEINTO << "CTaskContentChannel::~CTaskContentChannel() Out";
	}
	else if (m_channelDirection == MEDIA_DIRECTION_IN)
	{
		TRACEINTO << "CTaskContentChannel::~CTaskContentChannel() In";
	}
}

//////////////////////////////////////////


string CTaskContentChannel::ChannelData()
{
	string str = "";
	str = CTaskMediaChannel::ChannelData();
	
	//add relevant data for Content Channel
	//str += "\some content parameter: ";
	//str += GetContentParameterStr();
	
	return str.c_str();
}

/////////////////////////////////////////////////////////////////////////////

void  CTaskContentChannel::SelfKill()
{
	TRACEINTO << "CTaskContentChannel::SelfKill";
	CTaskMediaChannel::SelfKill();
}

/////////////////////////////////////////////////////////////////////////////


int CTaskContentChannel::SetupMediaFile()
{
	string fullFileName = ::GetMediaMngrCfg()->GetInstallationContentFileReadPath();
	
	fullFileName += "/";
	
	m_strFileName = GetMediaPrefix();
	m_strFileName += ".";
	m_strFileName += "content64.cont";
	
	fullFileName += m_strFileName;	
	
	TRACEINTO << "CTaskContentChannel::SetupMediaFile - content file name: " << fullFileName;
	
	//loading the content buffer from repository
	m_mediaRepositoryElement = ::GetMediaRepository()->GetContentDB()->GetMediaElement(fullFileName);
	if (m_mediaRepositoryElement == NULL)
	{
		TRACEINTO << "MM ERROR CTaskContentChannel::SetupMediaFile - m_mediaRepositoryElement is NULL";
		return STATUS_ERROR;
	}
	
	m_mediaFileBuffer = m_mediaRepositoryElement->GetDataBuffer();
	m_fileSize = m_mediaRepositoryElement->GetSize();
	
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

void CTaskContentChannel::SendMedia()
{
	CTaskVideoChannel::SendMedia();
}

/////////////////////////////////////////////////////////////////////////////

int contentCounter = 0;

string CTaskContentChannel::GetMediaPrefix()
{
	contentCounter++;
	if (contentCounter == 3)
		contentCounter = 1;
	
	string content = "";
	
	switch (contentCounter)
	{
		case 1:
			content = "C001";
			break;
		case 2:
			content = "C002";
			break;
	}
	
	return content;
}

/////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////
//CTaskFeccChannel
//////////////////////////////////////////

void FeccOutChannelEntryPoint(void* appParam)
{
	CTaskFeccChannel* pTaskFeccOutChannel = new CTaskFeccChannel(MEDIA_DIRECTION_OUT);
	pTaskFeccOutChannel->Create(*(CSegment*)appParam);
}

void FeccInChannelEntryPoint(void* appParam)
{
	CTaskFeccChannel* pTaskFeccInChannel = new CTaskFeccChannel(MEDIA_DIRECTION_IN);
	pTaskFeccInChannel->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
//  task creation function
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CTaskFeccChannel)
	
PEND_MESSAGE_MAP(CTaskFeccChannel, CTaskMediaChannel);

CTaskFeccChannel::CTaskFeccChannel(INT32 channelDirection) : CTaskMediaChannel(channelDirection)
{	 
}

CTaskFeccChannel::~CTaskFeccChannel()
{
	if (m_channelDirection == MEDIA_DIRECTION_OUT)
	{
		TRACEINTO << "CTaskFeccChannel::~CTaskFeccChannel() Out";
	}
	else if (m_channelDirection == MEDIA_DIRECTION_IN)
	{
		TRACEINTO << "CTaskFeccChannel::~CTaskFeccChannel() In";
	}
}

//////////////////////////////////////////

string CTaskFeccChannel::ChannelData()
{
	string str = "";
	str = CTaskMediaChannel::ChannelData();
	
	//add relevant data for Fecc Channel
	//str += "\some fecc parameter: ";
	//str += GetFeccParameterStr();
	
	return str.c_str();
}

/////////////////////////////////////////////////////////////////////////////

void  CTaskFeccChannel::SelfKill()
{
	TRACEINTO << "CTaskFeccChannel::SelfKill";
	CTaskMediaChannel::SelfKill();
}

/////////////////////////////////////////////////////////////////////////////
/*
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

*/

