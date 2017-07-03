#include "EPConnection.h"
#include "IpChannelParams.h"
#include "IpRtpReq.h"
#include "MplMcmsProtocol.h"
#include "HostCommonDefinitions.h"
#include "OpcodesMcmsCardMngrIpMedia.h"
#include "ProcessBase.h"
#include "IpCmReq.h"
#include "IpMfaOpcodes.h"
#include "TraceStream.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "ObjString.h"
#include "IpCommonUtilTrace.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsAudio.h"
#include "ChannelParams.h"
#include "UdpSocket.h"
#include "TaskApi.h"
#include "AudRequestStructs.h"

#include "SysConfig.h"
#include "SysConfigKeys.h"



using namespace std;

//GLOBALS

TaskEntryPoint GetTaskMediaChannelEntryPoint(int channelIndex)
{
	switch (channelIndex)
	{
		case VIDEO_OUT_CHANNEL_INDEX:
			return VideoOutChannelEntryPoint;
		case VIDEO_IN_CHANNEL_INDEX:
			return VideoInChannelEntryPoint;
		
		case AUDIO_OUT_CHANNEL_INDEX:
			return AudioOutChannelEntryPoint;
		case AUDIO_IN_CHANNEL_INDEX:
			return AudioInChannelEntryPoint;
		
		case CONTENT_OUT_CHANNEL_INDEX:
			return ContentOutChannelEntryPoint;
		case CONTENT_IN_CHANNEL_INDEX:
			return ContentInChannelEntryPoint;									
		
		case FECC_OUT_CHANNEL_INDEX:
			return FeccOutChannelEntryPoint;
		case FECC_IN_CHANNEL_INDEX:
			return FeccInChannelEntryPoint;
	}
	return NULL;
}



PBEGIN_MESSAGE_MAP(CEPConnection)
	
PEND_MESSAGE_MAP(CEPConnection, CStateMachine)


CEPConnection::CEPConnection(CTaskApp* pOwnerTask) : CStateMachine(pOwnerTask)
{
	//reset channel api array
	for (int i = 0; i < NUM_OF_MEDIA_CHANNELS; i++)
	{
		m_pMediaChannelApiArr[i] = NULL;
	}
	
	//Video
	m_eVideoProtocol = E_VIDEO_PROTOCOL_DUMMY;
	//reset Video variables
	m_tVideoParam.nVideoBitRate = 0;
	m_tVideoParam.eVideoResolution = E_MM_VIDEO_RES_DUMMY;
	m_tVideoParam.eVideoFrameRate = E_MM_VIDEO_FPS_DUMMY;
	m_tVideoParam.eVideoProfileType = E_MM_VIDEO_PROFILE_DUMMY;
	
	//reset ep media params
	m_sVideoFileNameTx 		= "";
	m_sAudioFileNameTx 		= "";
	m_sContentFileNameTx 	= "";
	m_sVideoFileNameRx 		= "";
	m_sAudioFileNameRx 		= "";
	m_sContentFileNameRx 	= "";
	
	//content
	m_contentBitrate = 0;
	
//	//Incoming channel params
//	m_tIncomingChannelParam.bReadFlag = FALSE;
//	m_tIncomingChannelParam.bCheckSeqNumber = FALSE;
//	m_tIncomingChannelParam.bIntraSeqNumber = FALSE;
//	m_tIncomingChannelParam.bCheckTimeStamp = FALSE;
//	m_tIncomingChannelParam.bIntraTimeStamp = FALSE;
//	m_tIncomingChannelParam.bDecryptMedia = FALSE;
//	m_tIncomingChannelParam.bWriteMedia = FALSE;
//	m_tIncomingChannelParam.eHeaderType = E_MM_NO_HEADER;
}

////////////////////////////////////////////////////////////////////////////////////

CEPConnection::~CEPConnection()
{
	TRACEINTO << "CEPConnection::~CEPConnection";
	
	for (int i = 0; i < NUM_OF_MEDIA_CHANNELS; i++)
	{
		if ( CPObject::IsValidPObjectPtr(m_pMediaChannelApiArr[i]))
		{
			TRACEINTO << "CEPConnection::~CEPConnection close channel: " << i;
			m_pMediaChannelApiArr[i]->Destroy();
			POBJDELETE(m_pMediaChannelApiArr[i]);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////

void CEPConnection::ExecuteEventMessage(CMplMcmsProtocol* pMplProtocol)
{
	OPCODE opCode = pMplProtocol->getOpcode();
	static const CProcessBase* process = CProcessBase::GetProcess();
	const string &opcodestr = process->GetOpcodeAsString(pMplProtocol->getCommonHeaderOpcode());
	
	
	int channelIndex = -1;
	cmCapDirection channelDirection = (cmCapDirection)0;
	kChanneltype channelType = kUnknownChnlType;
	

	switch (opCode)
	{
		case CONFPARTY_CM_OPEN_UDP_PORT_REQ:
		case SIP_CM_OPEN_UDP_PORT_REQ:
		{
			TOpenUdpPortOrUpdateUdpAddrMessageStruct* pStruct = (TOpenUdpPortOrUpdateUdpAddrMessageStruct*)pMplProtocol->GetData();
			channelDirection = (cmCapDirection)pStruct->tCmOpenUdpPortOrUpdateUdpAddr.channelDirection;
			channelType = (kChanneltype)pStruct->tCmOpenUdpPortOrUpdateUdpAddr.channelType;
			break;
		}

		case H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:
		case SIP_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:	
		{
			TUpdateRtpChannelReq* pStruct = (TUpdateRtpChannelReq*)pMplProtocol->GetData();
			channelDirection = (cmCapDirection)pStruct->unChannelDirection;
			channelType = (kChanneltype)pStruct->unChannelType;
			
			
			//save 'content out' bitrate -> for further use in content on/off
			if (channelType==kIpContentChnlType && channelDirection==cmCapTransmit)
			{
				m_contentBitrate = pStruct->tUpdateRtpSpecificChannelParams.unBitRate * 100;
				
				TRACEINTO << "CEPConnection::ExecuteEventMessage m_contentBitrate:" << m_contentBitrate;
			}
			
			break;
		}
		
		case H323_RTP_UPDATE_CHANNEL_REQ:
		case SIP_RTP_UPDATE_CHANNEL_REQ:
		{
			TUpdateRtpChannelReq* pStruct = (TUpdateRtpChannelReq*)pMplProtocol->GetData();
			channelDirection = (cmCapDirection)pStruct->unChannelDirection;
			channelType = (kChanneltype)pStruct->unChannelType;
			break;
		}
		
		
		case TB_MSG_OPEN_PORT_REQ:
		{
			eResourceTypes  resType = (eResourceTypes)pMplProtocol->getPhysicalInfoHeaderResource_type();
			const char* strResType = ::ResourceTypeToString(resType);
			
			//TRACEINTO << "CEPConnection::ExecuteEventMessage - **strResType:" << strResType;
			
			if (resType == ePhysical_video_encoder)
			{
				ENCODER_PARAM_S* pEncoderStruct = (ENCODER_PARAM_S*)pMplProtocol->GetData();
				if (pEncoderStruct == NULL)
				{
					TRACEINTO << "MM ERROR CEPConnection::ExecuteEventMessage pEncoderStruct == NULL";
					return;
				}
				
				EVideoEncoderType encoderType = (EVideoEncoderType)pEncoderStruct->nVideoEncoderType;
				
				if (encoderType == E_VIDEO_ENCODER_NORMAL || encoderType == E_VIDEO_ENCODER_MASTER_SPLIT_ENCODER)
				{
					TRACEINTO << "CEPConnection::ExecuteEventMessage pEncoderStruct->nProtocol=" << EVideoProtocolNames[pEncoderStruct->nProtocol];
									
					if (pEncoderStruct->nProtocol == E_VIDEO_PROTOCOL_H264)
					{
						m_eVideoProtocol = E_VIDEO_PROTOCOL_H264;
						SetH264VideoParam(pEncoderStruct);
					}
					else if (pEncoderStruct->nProtocol == E_VIDEO_PROTOCOL_H263)
					{
						m_eVideoProtocol = E_VIDEO_PROTOCOL_H263;
						SetH263VideoParam(pEncoderStruct);					
					}
					
					//send to video out media channel task
					CTaskApi* pTaskVideoOutChannelApi = m_pMediaChannelApiArr[VIDEO_OUT_CHANNEL_INDEX];
					if (pTaskVideoOutChannelApi != NULL)
					{
						CSegment* pMsg = new CSegment;
						*pMsg	<< (DWORD)m_eVideoProtocol
								<< (DWORD)m_tVideoParam.nVideoBitRate
								<< (DWORD)m_tVideoParam.eVideoResolution
								<< (DWORD)m_tVideoParam.eVideoFrameRate
								<< (DWORD)m_tVideoParam.eVideoProfileType;
						
						pTaskVideoOutChannelApi->SendMsg(pMsg, VIDEO_OUT_PARAM_EVENT_MESSAGE);
					}
				}
				else
				{
					return; // encoderType is not normal nor master
				}
			}

			return; // must return here - don't pass on
		}
		
		
		//closing channel
		case CONFPARTY_CM_CLOSE_UDP_PORT_REQ:
		case SIP_CM_CLOSE_UDP_PORT_REQ:
		{
			TCloseUdpPortMessageStruct* pStruct = (TCloseUdpPortMessageStruct*)pMplProtocol->GetData();
			channelDirection = (cmCapDirection)pStruct->tCmCloseUdpPort.channelDirection;
			channelType = (kChanneltype)pStruct->tCmCloseUdpPort.channelType;
			break;
		}
		
		//intra
		case VIDEO_FAST_UPDATE_REQ:
		{
			TRACEINTO << "CEPConnection::ExecuteEventMessage - opcode:" << opcodestr;
			channelDirection = (cmCapDirection)cmCapTransmit;
			channelType = (kChanneltype)kIpVideoChnlType;
			break;
		}
		
		//dtmf
		case AUDIO_PLAY_TONE_REQ:
		{
			TRACEINTO << "CEPConnection::ExecuteEventMessage - opcode:" << opcodestr;
			channelDirection = (cmCapDirection)cmCapTransmit;
			channelType = (kChanneltype)kIpAudioChnlType;
			break;
		}
		
		//content - on/off
		case ART_CONTENT_ON_REQ:
		case ART_CONTENT_OFF_REQ:
		{
			TRACEINTO << "CEPConnection::ExecuteEventMessage - opcode:" << opcodestr;
			
			TContentOnOffReq* pStruct = (TContentOnOffReq*)pMplProtocol->GetData();
			channelDirection = (cmCapDirection)pStruct->unChannelDirection;
			channelType = (kChanneltype)kIpContentChnlType;
			
			APIU32 contentMode = pStruct->bunIsOnOff;
			
			// only in content ON - first update Video-Out channel
			if (contentMode)
			{
				//send to video out media channel task
				CTaskApi* pTaskVideoOutChannelApi = m_pMediaChannelApiArr[VIDEO_OUT_CHANNEL_INDEX];
				if (pTaskVideoOutChannelApi != NULL)
				{
					CSegment* pMsg = new CSegment;
					*pMsg	<< (DWORD)m_contentBitrate
							<< (DWORD)contentMode;
					
					pTaskVideoOutChannelApi->SendMsg(pMsg, VIDEO_OUT_UPDATE_PARAM_EVENT_MESSAGE);
				}
			}
			
			break;
		}
		
		//video update param
		case VIDEO_ENCODER_UPDATE_PARAM_REQ:
		{
			TRACEINTO << "CEPConnection::ExecuteEventMessage - opcode:" << opcodestr << " Get channelDirection & Type";
			channelDirection = (cmCapDirection)cmCapTransmit;
			channelType = (kChanneltype)kIpVideoChnlType;
			break;
		}
	
	}
	
	CObjString* pContentStr = new CObjString("", 50);
	
	//ChannelType
	::GetChannelTypeName(channelType, *pContentStr);
	const string sChannelType = pContentStr->GetString();
	
	pContentStr->Clear();
	
	//ChannelDirection
	::GetChannelDirectionName(channelDirection, *pContentStr);
	const string sChannelDirection = pContentStr->GetString();
	
	POBJDELETE(pContentStr);
	
	TRACEINTO << "CEPConnection::ExecuteEventMessage - opcode:" << opcodestr << " direction:" << sChannelDirection << " (" << channelDirection << ") type:" << sChannelType << " (" << channelType << ")";
	//kIpAudioChnlType	= 1, kIpVideoChnlType	= 2,kIpFeccChnlType		= 3,kIpContentChnlType	= 4,
	
	
	//Get the Channel Index <<<==== ChannelDirection & ChannelType
	//////////////////////////////////////////////////////////////
	channelIndex = GetMediaChannelIndex(channelType, channelDirection);
	const char* strChannelAndDirection = ::MediaTypeAndDirectionToString(channelIndex);
	
	if (channelIndex == STATUS_ERROR)
	{
		TRACEINTO << "MM ERROR CEPConnection::ExecuteEventMessage - invalid channelIndex - opcode:" << opcodestr << " type & direction:" << strChannelAndDirection;
		return;
	}
	
	
	TRACEINTO << "CEPConnection::ExecuteEventMessage - channelIndex:" << channelIndex << " type&direction:" << strChannelAndDirection << " opcode:" << opcodestr;
	
	
	////////////////////////////////////////////////////////////////////////////////////	
	//get media channel task api
	CTaskApi* pTaskMediaChannelApi = m_pMediaChannelApiArr[channelIndex];
	if (pTaskMediaChannelApi == NULL)
	{	
		//create media channel task api
		m_pMediaChannelApiArr[channelIndex] = new CTaskApi;
		pTaskMediaChannelApi = m_pMediaChannelApiArr[channelIndex];
		
		COsQueue& rcvQ = m_OwnerTaskApp->GetRcvMbx();
		
		TaskEntryPoint taskMediaChannelEntryPoint = ::GetTaskMediaChannelEntryPoint(channelIndex);
			
		pTaskMediaChannelApi->Create(taskMediaChannelEntryPoint, rcvQ);
		
		TRACEINTO << "CEPConnection::ExecuteEventMessage create channel task. channelIndex: " << channelIndex;
	}
	
	////////////////////////////////////////////////////////////////////////////////////
	//List event message to Media Channel events' list
	CSegment* pListEventMsg = new CSegment;
	const char* sOpCode = opcodestr.c_str();
	*pListEventMsg	<< sOpCode
					<< opCode;
	pMplProtocol->Serialize(*pListEventMsg);
	
	pTaskMediaChannelApi->SendMsg(pListEventMsg, LIST_EVENT_MESSAGE);
	
	
	
	////////////////////////////////////////////////////////////////////////////////////
	//Activate event message on channel
	CSegment* pActivateEventMsg = new CSegment;
	sOpCode = opcodestr.c_str();
	*pActivateEventMsg	<< sOpCode
						<< opCode;
	pMplProtocol->Serialize(*pActivateEventMsg);
	
	pTaskMediaChannelApi->SendMsg(pActivateEventMsg, ACTIVATE_EVENT_MESSAGE);
	
	
	
	
	////////////////
	//SPECIAL CASES/
	////////////////
	
	////////////////////////////////////////////////////////////////////////////////////
	// after udp sockets closed -->destroy the channel task
	if (opCode == CONFPARTY_CM_CLOSE_UDP_PORT_REQ || opCode == SIP_CM_CLOSE_UDP_PORT_REQ)
	{
		TRACEINTO << "CEPConnection::ExecuteEventMessage before Destroy channelIndex: " << channelIndex;
		pTaskMediaChannelApi->Destroy();
		POBJDELETE(pTaskMediaChannelApi);
		m_pMediaChannelApiArr[channelIndex] = NULL;
		return;
	}
	////////////////////////////////////////////////////////////////////////////////////
	
	
	
	
	////////////////////////////////////////////////////////////////////////////////////
	// after content OFF sent to content out channel --> update video out param
	if (opCode == ART_CONTENT_OFF_REQ)
	{
		TRACEINTO << "CEPConnection::ExecuteEventMessage - before send content OFF to video-out channel";

		// only in content OFF - update Video-Out channel
		//send to video out media channel task
		CTaskApi* pTaskVideoOutChannelApi = m_pMediaChannelApiArr[VIDEO_OUT_CHANNEL_INDEX];
		if (pTaskVideoOutChannelApi != NULL)
		{
			CSegment* pMsg = new CSegment;
			*pMsg	<< (DWORD)m_contentBitrate
					<< (DWORD)0;
			
			pTaskVideoOutChannelApi->SendMsg(pMsg, VIDEO_OUT_UPDATE_PARAM_EVENT_MESSAGE);
		}
		
		return;		
	}
}



////////////////////////////////////////////////////////////////////////////////////
int CEPConnection::GetMediaChannelIndex(kChanneltype channelType, cmCapDirection channelDirection)
{
	int channelIndex = STATUS_ERROR;
	switch (channelType)
	{
		case kIpAudioChnlType:
			
			if (channelDirection == MEDIA_DIRECTION_IN)
			{
				channelIndex = AUDIO_IN_CHANNEL_INDEX;
			}
			else if (channelDirection == MEDIA_DIRECTION_OUT)
			{
				channelIndex = AUDIO_OUT_CHANNEL_INDEX;
			}
			
			break;
			
		case kIpVideoChnlType:		
			if (channelDirection == MEDIA_DIRECTION_IN)
			{
				channelIndex = VIDEO_IN_CHANNEL_INDEX;
			}
			else if (channelDirection == MEDIA_DIRECTION_OUT)
			{
				channelIndex = VIDEO_OUT_CHANNEL_INDEX;
			}		
			break;
		
		case kIpFeccChnlType:		
			
			if (channelDirection == MEDIA_DIRECTION_IN)
			{
				channelIndex = FECC_IN_CHANNEL_INDEX;
			}
			else if (channelDirection == MEDIA_DIRECTION_OUT)
			{
				channelIndex = FECC_OUT_CHANNEL_INDEX;
			}	
			break;
		
		case kIpContentChnlType:		
			
			if (channelDirection == MEDIA_DIRECTION_IN)
			{
				channelIndex = CONTENT_IN_CHANNEL_INDEX;
			}
			else if (channelDirection == MEDIA_DIRECTION_OUT)
			{
				channelIndex = CONTENT_OUT_CHANNEL_INDEX;
			}
			break;
		
		default:
			break;
	}
	
	return channelIndex;
}

////////////////////////////////////////////////////////////////////////////////////


void CEPConnection::SetH264VideoParam(ENCODER_PARAM_S* pEncoderStruct)
{
	//Bit Rate
	m_tVideoParam.nVideoBitRate = pEncoderStruct->nBitRate;
						
	int frameSizeMacroBlock = (pEncoderStruct->tH264VideoParams.nFS) * MACRO_BLOCK;
	TRACEINTO << "CEPConnection::SetH264VideoParam frameSizeMacroBlock=" << frameSizeMacroBlock;
	
	//Resolution
	SetH264Resolution(frameSizeMacroBlock);
	
	int numMacroBlockPerSec = (pEncoderStruct->tH264VideoParams.nMBPS) * MACRO_BLOCK_PER_SECOND;
	TRACEINTO << "CEPConnection::SetH264VideoParam numMacroBlockPerSec=" << numMacroBlockPerSec;
	
	//Frame Rate
	SetH264FrameRate(numMacroBlockPerSec);

	//High Profile
	m_tVideoParam.eVideoProfileType = (EMMVideoProfileType)(pEncoderStruct->tH264VideoParams.nProfile);
}

////////////////////////////////////////////////////////////////////////////////////

void CEPConnection::SetH264Resolution(int frameSizeMacroBlock)
{
	//////////////////////////////////////////////////////////////////////////
	// choosing the optimal resolution according to frame size MacroBlock
	//////////////////////////////////////////////////////////////////////////
	
	if (frameSizeMacroBlock <= 2*MACRO_BLOCK)
	{
		m_tVideoParam.eVideoResolution = E_MM_VIDEO_RES_CIF;
	}
	else if (frameSizeMacroBlock <= 5*MACRO_BLOCK)
	{
		m_tVideoParam.eVideoResolution = E_MM_VIDEO_RES_2CIF;
	}
	else if (frameSizeMacroBlock <= 14*MACRO_BLOCK)
	{
		m_tVideoParam.eVideoResolution = E_MM_VIDEO_RES_4CIF;
	}
	else if (frameSizeMacroBlock <= 31*MACRO_BLOCK)
	{
		m_tVideoParam.eVideoResolution = E_MM_VIDEO_RES_HD720;
	}
	else
	{
		m_tVideoParam.eVideoResolution = E_MM_VIDEO_RES_HD1080;
	}
	
	TRACEINTO << "CEPConnection::SetH264Resolution m_tVideoParam.eVideoResolution=" << EMMVideoResolutionNames[m_tVideoParam.eVideoResolution];
}

////////////////////////////////////////////////////////////////////////////////////

void CEPConnection::SetH264FrameRate(int numMacroBlockPerSec)
{
	int tmpFrameRate = 0;
	
	TRACEINTO << "CEPConnection::SetH264FrameRate numMacroBlockPerSec:" << numMacroBlockPerSec << " m_tVideoParam.eVideoResolution=" << EMMVideoResolutionNames[m_tVideoParam.eVideoResolution];
	
	if (m_tVideoParam.eVideoResolution == E_MM_VIDEO_RES_CIF)
	{
		tmpFrameRate = numMacroBlockPerSec / FRAME_SIZE_MB_RES_CIF;
		/*if (tmpFrameRate < 25)
		{
			SetH264FrameRate(numMacroBlockPerSec);
		}*/
		if (tmpFrameRate >= 25 && tmpFrameRate < 30)
		{
			m_tVideoParam.eVideoFrameRate = E_MM_VIDEO_25_FPS;
		}
		else
		{
			m_tVideoParam.eVideoFrameRate = E_MM_VIDEO_30_FPS;
		}
		
		//m_tVideoParam.eVideoFrameRate = E_MM_VIDEO_25_FPS;
		
	}
	else if (m_tVideoParam.eVideoResolution == E_MM_VIDEO_RES_2CIF)
	{
		tmpFrameRate = numMacroBlockPerSec / FRAME_SIZE_MB_RES_2CIF;
		if (tmpFrameRate < 25)
		{
			//decrease resolution
			m_tVideoParam.eVideoResolution = E_MM_VIDEO_RES_CIF;
			SetH264FrameRate(numMacroBlockPerSec);
			return;
		}
		if (tmpFrameRate >= 25 && tmpFrameRate < 30)
		{
			m_tVideoParam.eVideoFrameRate = E_MM_VIDEO_25_FPS;
		}
		else
		{
			m_tVideoParam.eVideoFrameRate = E_MM_VIDEO_30_FPS;
		}
	}
	else if (m_tVideoParam.eVideoResolution == E_MM_VIDEO_RES_4CIF)
	{
		tmpFrameRate = numMacroBlockPerSec / FRAME_SIZE_MB_RES_4CIF;
		if (tmpFrameRate < 15)
		{
			//decrease resolution
			m_tVideoParam.eVideoResolution = E_MM_VIDEO_RES_2CIF;
			SetH264FrameRate(numMacroBlockPerSec);
			return;
		}
		else if (tmpFrameRate >= 15 && tmpFrameRate < 25)
		{
			m_tVideoParam.eVideoFrameRate = E_MM_VIDEO_15_FPS;
		}
		else if (tmpFrameRate >= 25 && tmpFrameRate < 30)
		{
			m_tVideoParam.eVideoFrameRate = E_MM_VIDEO_25_FPS;
		}
		else
		{
			m_tVideoParam.eVideoFrameRate = E_MM_VIDEO_30_FPS;
		}
	}
	else if (m_tVideoParam.eVideoResolution == E_MM_VIDEO_RES_HD720)
	{
		tmpFrameRate = numMacroBlockPerSec / FRAME_SIZE_MB_RES_HD720;
		if (tmpFrameRate < 25)
		{
			//decrease resolution
			m_tVideoParam.eVideoResolution = E_MM_VIDEO_RES_4CIF;
			SetH264FrameRate(numMacroBlockPerSec);
			return;
		}
		if (tmpFrameRate >= 25 && tmpFrameRate < 30)
		{
			m_tVideoParam.eVideoFrameRate = E_MM_VIDEO_25_FPS;
		}
		else
		{
			m_tVideoParam.eVideoFrameRate = E_MM_VIDEO_30_FPS;
		}
	}
	else if (m_tVideoParam.eVideoResolution == E_MM_VIDEO_RES_HD1080)
	{
		tmpFrameRate = numMacroBlockPerSec / FRAME_SIZE_MB_RES_HD1080;
		if (tmpFrameRate < 25)
		{
			//decrease resolution
			m_tVideoParam.eVideoResolution = E_MM_VIDEO_RES_HD720;
			SetH264FrameRate(numMacroBlockPerSec);
			return;
		}
		if (tmpFrameRate >= 25 && tmpFrameRate < 30)
		{
			m_tVideoParam.eVideoFrameRate = E_MM_VIDEO_25_FPS;
		}
		else
		{
			m_tVideoParam.eVideoFrameRate = E_MM_VIDEO_30_FPS;
		}
	}
	
	TRACEINTO << "CEPConnection::SetH264FrameRate m_tVideoParam.eVideoFrameRate=" << EShortMMVideoFrameRateNames[m_tVideoParam.eVideoFrameRate];
}


////////////////////////////////////////////////////////////////////////////////////


void CEPConnection::SetH263VideoParam(ENCODER_PARAM_S* pEncoderStruct)
{
	TRACEINTO << "CEPConnection::SetH263VideoParam";
	//Bit Rate
	m_tVideoParam.nVideoBitRate = pEncoderStruct->nBitRate;
	
	//Resolution
	H263_H261_VIDEO_PARAM_S H263Video = pEncoderStruct->tH263_H261VideoParams;
	SetH263ResolutionFrameRate(H263Video);
	
	TRACEINTO << "CEPConnection::SetH263VideoParam Resolution:" << EMMVideoResolutionNames[m_tVideoParam.eVideoResolution]
						<< " FrameRate:" << EShortMMVideoFrameRateNames[m_tVideoParam.eVideoFrameRate];	
}

////////////////////////////////////////////////////////////////////////////////////

void CEPConnection::SetH263ResolutionFrameRate(H263_H261_VIDEO_PARAM_S H263Video)
{
	if (H263Video.n4CifFrameRate != E_VIDEO_FPS_DUMMY)
	{
		m_tVideoParam.eVideoResolution = E_MM_VIDEO_RES_4CIF;
		m_tVideoParam.eVideoFrameRate = (EMMVideoFrameRate)H263Video.n4CifFrameRate;
		return;		
	}
	else if (H263Video.nCifFrameRate != E_VIDEO_FPS_DUMMY)
	{
		m_tVideoParam.eVideoResolution = E_MM_VIDEO_RES_CIF;
		m_tVideoParam.eVideoFrameRate = (EMMVideoFrameRate)H263Video.nCifFrameRate;
		return;
	}
	else if (H263Video.nQcifFrameRate != E_VIDEO_FPS_DUMMY)
	{
		m_tVideoParam.eVideoResolution = E_MM_VIDEO_RES_QCIF;
		m_tVideoParam.eVideoFrameRate = (EMMVideoFrameRate)H263Video.nQcifFrameRate;
		return;
	}
	else
	{
		m_tVideoParam.eVideoResolution = E_MM_VIDEO_RES_QCIF;
		m_tVideoParam.eVideoFrameRate = E_MM_VIDEO_15_FPS;
	}
}

////////////////////////////////////////////////////////////////////////////////////


void CEPConnection::ExecuteAPIMessage(CSegment* pSeg)
{
	*pSeg >> m_sVideoFileNameTx
		  >> m_sAudioFileNameTx
		  >> m_sContentFileNameTx
		  >> m_sVideoFileNameRx
		  >> m_sAudioFileNameRx
		  >> m_sContentFileNameRx;
	
	TRACEINTO << "CEPConnection::ExecuteAPIMessage m_sVideoFileNameTx=" << m_sVideoFileNameTx
											  << " m_sAudioFileNameTx=" << m_sAudioFileNameTx
											  << " m_sContentFileNameTx=" << m_sContentFileNameTx
											  << " m_sVideoFileNameRx=" << m_sVideoFileNameRx
											  << " m_sAudioFileNameRx=" << m_sAudioFileNameRx
											  << " m_sContentFileNameRx=" << m_sContentFileNameRx;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////









////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////  class CEPConnectionObj
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CEPConnectionObj)
	
PEND_MESSAGE_MAP(CEPConnectionObj, CEPConnection)


CEPConnectionObj::CEPConnectionObj(CTaskApp* pOwnerTask) : CEPConnection(pOwnerTask)
{
	//reset channel obj array
	for (int i = 0; i < NUM_OF_MEDIA_CHANNELS; i++)
	{
		m_pMediaChannelArr[i] = NULL;
	}	
	
	// save parameters for Intra
	m_boardId 			= (DWORD)-1;
	m_subBoardId 		= (DWORD)-1;
	m_unitId 			= (DWORD)-1;
	m_conferenceId 		= (DWORD)-1;
	m_partyId 			= (DWORD)-1;
	m_connectionId 		= (DWORD)-1;

	m_bSendSilentStream = FALSE;
}


////////////////////////////////////////////////////////////////////////////////////

CEPConnectionObj::~CEPConnectionObj()
{
	TRACEINTO << GetParticipantTicket() << " CEPConnectionObj::~CEPConnectionObj";
	
	for (int i = 0; i < NUM_OF_MEDIA_CHANNELS; i++)
	{
		if ( CPObject::IsValidPObjectPtr(m_pMediaChannelArr[i]))
		{
			TRACEINTO << "CEPConnectionObj::~CEPConnectionObj close channel obj at index: " << i;
			POBJDELETE(m_pMediaChannelApiArr[i]);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////


void CEPConnectionObj::ExecuteEventMessage(CMplMcmsProtocol* pMplProtocol)
{
	TRACEINTO << GetParticipantTicket() << " CEPConnectionObj::ExecuteEventMessage";
	
	OPCODE opCode = pMplProtocol->getOpcode();
	static const CProcessBase* process = CProcessBase::GetProcess();
	const string &opcodestr = process->GetOpcodeAsString(pMplProtocol->getCommonHeaderOpcode());
	
	
	int channelIndex = -1;
	cmCapDirection channelDirection = (cmCapDirection)0;
	kChanneltype channelType = kUnknownChnlType;
	

	switch (opCode)
	{
		case CONFPARTY_CM_OPEN_UDP_PORT_REQ:
		case SIP_CM_OPEN_UDP_PORT_REQ:
		{
			TOpenUdpPortOrUpdateUdpAddrMessageStruct* pStruct = (TOpenUdpPortOrUpdateUdpAddrMessageStruct*)pMplProtocol->GetData();
			channelDirection = (cmCapDirection)pStruct->tCmOpenUdpPortOrUpdateUdpAddr.channelDirection;
			channelType = (kChanneltype)pStruct->tCmOpenUdpPortOrUpdateUdpAddr.channelType;
			break;
		}

		case H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:
		case SIP_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:	
		{
			TUpdatePortOpenRtpChannelReq* pStruct = (TUpdatePortOpenRtpChannelReq*)pMplProtocol->GetData();
			channelDirection = (cmCapDirection)pStruct->unChannelDirection;
			channelType = (kChanneltype)pStruct->unChannelType;
			
			OnRtpUpdatePortOpenChannelReq( pMplProtocol );
			break;
		}
		
		case H323_RTP_UPDATE_CHANNEL_REQ:
		case SIP_RTP_UPDATE_CHANNEL_REQ:
		{
			TUpdateRtpChannelReq* pStruct = (TUpdateRtpChannelReq*)pMplProtocol->GetData();
			channelDirection = (cmCapDirection)pStruct->unChannelDirection;
			channelType = (kChanneltype)pStruct->unChannelType;
			break;
		}
		
		
		case TB_MSG_OPEN_PORT_REQ:
		case VIDEO_ENCODER_UPDATE_PARAM_REQ:
		{
			eResourceTypes  resType = (eResourceTypes)pMplProtocol->getPhysicalInfoHeaderResource_type();
			const char* strResType = ::ResourceTypeToString(resType);
			
			//TRACEINTO << "CEPConnectionObj::ExecuteEventMessage - **strResType:" << strResType;
			
			if (resType == ePhysical_video_encoder)
			{
				ENCODER_PARAM_S* pEncoderStruct = (ENCODER_PARAM_S*)pMplProtocol->GetData();
				if (pEncoderStruct == NULL)
				{
					TRACEINTO << GetParticipantTicket() << " MM ERROR CEPConnectionObj::ExecuteEventMessage pEncoderStruct == NULL";
					return;
				}
				
				EVideoEncoderType encoderType = (EVideoEncoderType)pEncoderStruct->nVideoEncoderType;
				
				if (encoderType == E_VIDEO_ENCODER_NORMAL ||
					encoderType == E_VIDEO_ENCODER_MASTER_SPLIT_ENCODER ||
					encoderType == E_VIDEO_ENCODER_MASTER_SPLIT_ENCODER_HALF_DSP)
				{
					TRACEINTO << GetParticipantTicket() << " CEPConnectionObj::ExecuteEventMessage pEncoderStruct->nProtocol=" << EVideoProtocolNames[pEncoderStruct->nProtocol];
									
					if (pEncoderStruct->nProtocol == E_VIDEO_PROTOCOL_H264)
					{
						m_eVideoProtocol = E_VIDEO_PROTOCOL_H264;
						SetH264VideoParam(pEncoderStruct);
					}
					else if (pEncoderStruct->nProtocol == E_VIDEO_PROTOCOL_H263)
					{
						m_eVideoProtocol = E_VIDEO_PROTOCOL_H263;
						SetH263VideoParam(pEncoderStruct);					
					}
					
					//send to video out media channel obj
					CMediaChannel* pVideoOutChannel = m_pMediaChannelArr[VIDEO_OUT_CHANNEL_INDEX];
					if (pVideoOutChannel == NULL)
					{
						if (TB_MSG_OPEN_PORT_REQ == opCode)
						{
							m_pMediaChannelArr[VIDEO_OUT_CHANNEL_INDEX] = new CVideoChannel(m_OwnerTaskApp, MEDIA_DIRECTION_OUT);
							pVideoOutChannel = m_pMediaChannelArr[VIDEO_OUT_CHANNEL_INDEX];
							pVideoOutChannel->SetParticipantTicket(GetParticipantTicket());


	/*						////////////////////////////////////////////////////////////////////////////////////
							//send Incoming Channel Params to incoming channels only
							////////////////////////////////////////////////////////////////////////////////////
							if (channelDirection == MEDIA_DIRECTION_IN)
							{
								CSegment* pMsgIncomingParams = new CSegment;
								*pMsgIncomingParams	<< (BOOL)m_tIncomingChannelParam.bReadFlag
													<< (BOOL)m_tIncomingChannelParam.bCheckSeqNumber
													<< (BOOL)m_tIncomingChannelParam.bIntraSeqNumber
													<< (BOOL)m_tIncomingChannelParam.bCheckTimeStamp
													<< (BOOL)m_tIncomingChannelParam.bIntraTimeStamp
													<< (BOOL)m_tIncomingChannelParam.bDecryptMedia
													<< (BOOL)m_tIncomingChannelParam.bWriteMedia
													<< (DWORD)m_tIncomingChannelParam.eHeaderType;


								pVideoOutChannel->HandleEvent(pMsgIncomingParams, 0, CHANNEL_IN_PARAM_EVENT_MESSAGE);

								POBJDELETE(pMsgIncomingParams);
							}
							////////////////////////////////////////////////////////////////////////////////////
	*/
							
							
						}
						else
						{
							TRACEINTO << GetParticipantTicket() << " MM ERROR CEPConnectionObj::ExecuteEventMessage - VIDEO_ENCODER_UPDATE_PARAM_REQ for pVideoOutChannel == NULL";
							return;
						}
					}
					
					CSegment* pMsg = new CSegment;
					*pMsg	<< (DWORD)m_eVideoProtocol
							<< (DWORD)m_tVideoParam.nVideoBitRate
							<< (DWORD)m_tVideoParam.eVideoResolution
							<< (DWORD)m_tVideoParam.eVideoFrameRate
							<< (DWORD)m_tVideoParam.eVideoProfileType;
					
					if (TB_MSG_OPEN_PORT_REQ == opCode)		
						pVideoOutChannel->HandleEvent(pMsg, 0, VIDEO_OUT_PARAM_EVENT_MESSAGE);		
					if (VIDEO_ENCODER_UPDATE_PARAM_REQ == opCode)
						pVideoOutChannel->HandleEvent(pMsg, 0, VIDEO_OUT_ENCODER_UPDATE_PARAM_EVENT_MESSAGE);

					
					POBJDELETE(pMsg);
					
				}
				else
				{
					return; // encoderType is not normal nor master
				}
			}
			
			if (resType == ePhysical_video_decoder)
			{
				DECODER_PARAM_S* pDecoderStruct = (DECODER_PARAM_S*)pMplProtocol->GetData();
				if (pDecoderStruct == NULL)
				{
					TRACEINTO << GetParticipantTicket() << " MM ERROR CEPConnectionObj::ExecuteEventMessage pDecoderStruct == NULL";
					return;
				}
				
				TRACEINTO << GetParticipantTicket() << " CEPConnectionObj::ExecuteEventMessage pDecoderStruct->nProtocol=" << EVideoProtocolNames[pDecoderStruct->nProtocol];
												
				//send to video out media channel obj
				CMediaChannel* pVideoInChannel = m_pMediaChannelArr[VIDEO_IN_CHANNEL_INDEX];
				if (pVideoInChannel == NULL)
				{
					m_pMediaChannelArr[VIDEO_IN_CHANNEL_INDEX] = new CVideoChannel(m_OwnerTaskApp, MEDIA_DIRECTION_IN);
					pVideoInChannel = m_pMediaChannelArr[VIDEO_IN_CHANNEL_INDEX];
					pVideoInChannel->SetParticipantTicket(GetParticipantTicket());
				}	
								
				CSegment* pMsg = new CSegment;
				*pMsg	<< (DWORD)pDecoderStruct->nProtocol;
					
				pVideoInChannel->HandleEvent(pMsg, 0, VIDEO_IN_PARAM_EVENT_MESSAGE);
				
				POBJDELETE(pMsg);
			}

			return; // must return here - don't pass on
		}
		
		
		//closing channel
		case CONFPARTY_CM_CLOSE_UDP_PORT_REQ:
		case SIP_CM_CLOSE_UDP_PORT_REQ:
		{
			TCloseUdpPortMessageStruct* pStruct = (TCloseUdpPortMessageStruct*)pMplProtocol->GetData();
			channelDirection = (cmCapDirection)pStruct->tCmCloseUdpPort.channelDirection;
			channelType = (kChanneltype)pStruct->tCmCloseUdpPort.channelType;
			break;
		}
		
		//intra
		case VIDEO_FAST_UPDATE_REQ:
		{
			TRACEINTO << GetParticipantTicket() << " CEPConnectionObj::ExecuteEventMessage - opcode:" << opcodestr;
			channelDirection = (cmCapDirection)cmCapTransmit;
			channelType = (kChanneltype)kIpVideoChnlType;
			break;
		}
		
		//dtmf
		case AUDIO_PLAY_TONE_REQ:
		{
			TRACEINTO << GetParticipantTicket() << " CEPConnectionObj::ExecuteEventMessage - opcode:" << opcodestr;
			channelDirection = (cmCapDirection)cmCapTransmit;
			channelType = (kChanneltype)kIpAudioChnlType;
			break;
		}
		
		//content - on/off
		case ART_CONTENT_ON_REQ:
		case ART_CONTENT_OFF_REQ:
		{
			TRACEINTO << GetParticipantTicket() << " CEPConnectionObj::ExecuteEventMessage - opcode:" << opcodestr;
			
			TContentOnOffReq* pStruct = (TContentOnOffReq*)pMplProtocol->GetData();
			channelDirection = (cmCapDirection)pStruct->unChannelDirection;
			channelType = (kChanneltype)kIpContentChnlType;
			
			APIU32 contentMode = pStruct->bunIsOnOff;
			
			// only in content ON - first update Video-Out channel
			if (contentMode)
			{
				//send to video out media channel obj
				CMediaChannel* pVideoOutChannel = m_pMediaChannelArr[VIDEO_OUT_CHANNEL_INDEX];
				if (pVideoOutChannel != NULL)
				{
					CSegment* pMsg = new CSegment;
					*pMsg	<< (DWORD)m_contentBitrate
							<< (DWORD)contentMode;
					
					pVideoOutChannel->HandleEvent(pMsg, 0, VIDEO_OUT_UPDATE_PARAM_EVENT_MESSAGE);
					
					POBJDELETE(pMsg);
				}
			}
			
			break;
		}
		
///		//video update param
///		case VIDEO_ENCODER_UPDATE_PARAM_REQ:
///		{
///			TRACEINTO << GetParticipantTicket() << " CEPConnectionObj::ExecuteEventMessage - opcode:" << opcodestr << " Get channelDirection & Type";
///			channelDirection = (cmCapDirection)cmCapTransmit;
///			channelType = (kChanneltype)kIpVideoChnlType;
///			break;
///		}
	
	}
	
	CObjString* pContentStr = new CObjString("", 50);
	
	//ChannelType
	::GetChannelTypeName(channelType, *pContentStr);
	const string sChannelType = pContentStr->GetString();
	
	pContentStr->Clear();
	
	//ChannelDirection
	::GetChannelDirectionName(channelDirection, *pContentStr);
	const string sChannelDirection = pContentStr->GetString();
	
	POBJDELETE(pContentStr);
	
	//TRACEINTO << "CEPConnectionObj::ExecuteEventMessage - opcode:" << opcodestr << " direction:" << sChannelDirection << " (" << channelDirection << ") type:" << sChannelType << " (" << channelType << ")";
	//kIpAudioChnlType	= 1, kIpVideoChnlType	= 2,kIpFeccChnlType		= 3,kIpContentChnlType	= 4,
	
	
	//Get the Channel Index <<<==== ChannelDirection & ChannelType
	//////////////////////////////////////////////////////////////
	channelIndex = GetMediaChannelIndex(channelType, channelDirection);
	const char* strChannelAndDirection = ::MediaTypeAndDirectionToString(channelIndex);
	
	if (channelIndex == STATUS_ERROR)
	{
		TRACEINTO << GetParticipantTicket() << " MM ERROR CEPConnectionObj::ExecuteEventMessage - invalid channelIndex - opcode:" << opcodestr << " type & direction:" << strChannelAndDirection;
		return;
	}
	
	
	TRACEINTO << GetParticipantTicket() << " CEPConnectionObj::ExecuteEventMessage - channelIndex:" << channelIndex << " type&direction:" << strChannelAndDirection << " opcode:" << opcodestr;
	
	
	////////////////////////////////////////////////////////////////////////////////////	
	//get media channel obj
	CMediaChannel* pMediaChannel = m_pMediaChannelArr[channelIndex];
	if (pMediaChannel == NULL)
	{	
		//create media channel obj
		if (channelIndex==VIDEO_OUT_CHANNEL_INDEX || channelIndex==VIDEO_IN_CHANNEL_INDEX)
		{
			m_pMediaChannelArr[channelIndex] = new CVideoChannel(m_OwnerTaskApp, channelDirection);
		}
		else if (channelIndex==AUDIO_OUT_CHANNEL_INDEX || channelIndex==AUDIO_IN_CHANNEL_INDEX)
		{
			m_pMediaChannelArr[channelIndex] = new CAudioChannel(m_OwnerTaskApp, channelDirection);
			((CAudioChannel *)m_pMediaChannelArr[channelIndex])->SetSilentStream(m_bSendSilentStream);
		}
		else if (channelIndex==CONTENT_OUT_CHANNEL_INDEX || channelIndex==CONTENT_IN_CHANNEL_INDEX)
		{
			m_pMediaChannelArr[channelIndex] = new CContentChannel(m_OwnerTaskApp, channelDirection);
		}
		else if (channelIndex==FECC_OUT_CHANNEL_INDEX || channelIndex==FECC_IN_CHANNEL_INDEX)
		{
			m_pMediaChannelArr[channelIndex] = new CFeccChannel(m_OwnerTaskApp, channelDirection);
		}

		pMediaChannel = m_pMediaChannelArr[channelIndex];
		pMediaChannel->SetParticipantTicket(GetParticipantTicket());
		
		TRACEINTO << GetParticipantTicket() << " CEPConnectionObj::ExecuteEventMessage create channel obj. channelIndex: " << channelIndex;
		
		
/*		////////////////////////////////////////////////////////////////////////////////////
		//send Incoming Channel Params to incoming channels only
		////////////////////////////////////////////////////////////////////////////////////
		if (channelDirection == MEDIA_DIRECTION_IN)
		{
			CSegment* pMsgIncomingParams = new CSegment;
			*pMsgIncomingParams	<< (BYTE)m_tIncomingChannelParam.bReadFlag
								<< (BYTE)m_tIncomingChannelParam.bCheckSeqNumber
								<< (BYTE)m_tIncomingChannelParam.bIntraSeqNumber
								<< (BYTE)m_tIncomingChannelParam.bCheckTimeStamp
								<< (BYTE)m_tIncomingChannelParam.bIntraTimeStamp
								<< (BYTE)m_tIncomingChannelParam.bDecryptMedia
								<< (BYTE)m_tIncomingChannelParam.bWriteMedia
								<< (DWORD)m_tIncomingChannelParam.eHeaderType;
								
			
			pMediaChannel->HandleEvent(pMsgIncomingParams, 0, CHANNEL_IN_PARAM_EVENT_MESSAGE);
			
			POBJDELETE(pMsgIncomingParams);
		}
		////////////////////////////////////////////////////////////////////////////////////
*/	
		
	}
	
	////////////////////////////////////////////////////////////////////////////////////
	//List event message to Media Channel events' list
	CSegment* pListEventMsg = new CSegment;
	const char* sOpCode = opcodestr.c_str();
	*pListEventMsg	<< sOpCode
					<< opCode;
	pMplProtocol->Serialize(*pListEventMsg);
	
	pMediaChannel->HandleEvent(pListEventMsg, 0, LIST_EVENT_MESSAGE);
	
	POBJDELETE(pListEventMsg);
	
	
	
	////////////////////////////////////////////////////////////////////////////////////
	//Activate event message on channel
	CSegment* pActivateEventMsg = new CSegment;
	sOpCode = opcodestr.c_str();
	*pActivateEventMsg	<< sOpCode
						<< opCode;
	pMplProtocol->Serialize(*pActivateEventMsg);
	
	pMediaChannel->HandleEvent(pActivateEventMsg, 0, ACTIVATE_EVENT_MESSAGE);
	
	POBJDELETE(pActivateEventMsg);
	
	
	////////////////
	//SPECIAL CASES/
	////////////////
	
	////////////////////////////////////////////////////////////////////////////////////
	// after udp sockets closed --> delete the channel obj & set to NULL
	if (opCode == CONFPARTY_CM_CLOSE_UDP_PORT_REQ || opCode == SIP_CM_CLOSE_UDP_PORT_REQ)
	{
		TRACEINTO << GetParticipantTicket() << " CEPConnectionObj::ExecuteEventMessage before Destroy channelIndex: " << channelIndex;
		POBJDELETE(pMediaChannel);
		m_pMediaChannelArr[channelIndex] = NULL;
		return;
	}
	////////////////////////////////////////////////////////////////////////////////////
	
	
	
	////////////////////////////////////////////////////////////////////////////////////
	// after content OFF sent to content out channel --> update video out param
	if (opCode == ART_CONTENT_OFF_REQ)
	{
		TRACEINTO << GetParticipantTicket() << " CEPConnectionObj::ExecuteEventMessage - before send content OFF to video-out channel";

		// only in content OFF - update Video-Out channel
		//send to video out media channel obj
		CMediaChannel* pVideoOutChannel = m_pMediaChannelArr[VIDEO_OUT_CHANNEL_INDEX];
		if (pVideoOutChannel != NULL)
		{
			CSegment* pMsg = new CSegment;
			*pMsg	<< (DWORD)m_contentBitrate
					<< (DWORD)0;
			
			pVideoOutChannel->HandleEvent(pMsg, 0, VIDEO_OUT_UPDATE_PARAM_EVENT_MESSAGE);
			
			POBJDELETE(pMsg);
		}
		
		return;		
	}
	
	
	////////////////////////////////////////////////////////////////////////////////////
	// update video channel with parameters for Intra
	if (opCode == H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ || opCode == SIP_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ)
	{
		if ((channelDirection == cmCapReceive) && (channelType == kIpVideoChnlType))
		{
			// update Video channel
			UpdateMediaChannelParams( pMediaChannel );
		}
	}	
	
}



////////////////////////////////////////////////////////////////////////////////////
void CEPConnectionObj::UpdateMediaChannelParams( CMediaChannel* pMediaChannel )
{
	CSegment* pSeg = new CSegment;

	*pSeg	<< m_boardId
			<< m_subBoardId
			<< m_unitId
			<< m_conferenceId
			<< m_partyId
			<< m_connectionId;

	pMediaChannel->UpdateMediaChannelParams( pSeg );
	
	POBJDELETE(pSeg);
}

 			


////////////////////////////////////////////////////////////////////////////////////
void CEPConnectionObj::OnRtpUpdatePortOpenChannelReq( CMplMcmsProtocol* pMplProtocol )
{
	TUpdatePortOpenRtpChannelReq* pStruct = (TUpdatePortOpenRtpChannelReq*)pMplProtocol->GetData();
	cmCapDirection channelDirection = (cmCapDirection)pStruct->unChannelDirection;
	kChanneltype channelType = (kChanneltype)pStruct->unChannelType;
	
	//save 'content out' bitrate -> for further use in content on/off
	if (channelType==kIpContentChnlType && channelDirection==cmCapTransmit)
	{
		m_contentBitrate = pStruct->tUpdateRtpSpecificChannelParams.unBitRate * 100;
		
		TRACEINTO << GetParticipantTicket() << " CEPConnectionObj::ExecuteEventMessage m_contentBitrate:" << m_contentBitrate;
	}
	
	// for "Intra" supports (CG asks Intra from EP)
	if ((channelDirection == cmCapReceive) && (channelType == kIpVideoChnlType))
	{
		m_boardId		= (DWORD) pMplProtocol->getPhysicalInfoHeaderBoard_id();
		m_subBoardId	= (DWORD) pMplProtocol->getPhysicalInfoHeaderSub_board_id();
		m_unitId		= (DWORD) pMplProtocol->getPhysicalInfoHeaderUnit_id();
		m_conferenceId	= (DWORD) pMplProtocol->getPortDescriptionHeaderConf_id();
		m_partyId		= (DWORD) pMplProtocol->getPortDescriptionHeaderParty_id();
		m_connectionId	= (DWORD) pMplProtocol->getPortDescriptionHeaderConnection_id();

		CLargeString traceStr = "";
		if (channelType == kIpVideoChnlType)
			traceStr << "\nCEPConnection::ExecuteEventMessage: RTP_UPDATE_PORT_OPEN_CHANNEL_REQ Video\n";
		else
			traceStr << "\nCEPConnection::ExecuteEventMessage: RTP_UPDATE_PORT_OPEN_CHANNEL_REQ Audio\n";
		traceStr <<   "==========================================================================\n";
		traceStr << "\n boardId         : " << m_boardId;
		traceStr << "\n subBoardId      : " << m_subBoardId;
		traceStr << "\n unitId          : " << m_unitId;
		traceStr << "\n conferenceId    : " << m_conferenceId;
		traceStr << "\n partyId         : " << m_partyId;
		traceStr << "\n connectionId    : " << m_connectionId;

		TRACESTR(eLevelInfoNormal) << traceStr.GetString();
	}
}



////////////////////////////////////////////////////////////////////////////////////


void CEPConnectionObj::ExecuteAPIMessage(CSegment* pSeg)
{
	CEPConnection::ExecuteAPIMessage(pSeg);
}

////////////////////////////////////////////////////////////////////////////////////


void CEPConnectionObj::InvokeEPGlobalTxTimer()
{
	//invoke timer per all Tx channels
	CMediaChannel* mediaTxChannel = NULL;
	mediaTxChannel = m_pMediaChannelArr[VIDEO_OUT_CHANNEL_INDEX];
	if (mediaTxChannel != NULL)
		mediaTxChannel->OnTimerSendMedia();
	
	mediaTxChannel = m_pMediaChannelArr[AUDIO_OUT_CHANNEL_INDEX];
	if (mediaTxChannel != NULL)
		mediaTxChannel->OnTimerSendMedia();
		
	mediaTxChannel = m_pMediaChannelArr[CONTENT_OUT_CHANNEL_INDEX];
	if (mediaTxChannel != NULL)
		mediaTxChannel->OnTimerSendMedia();
		
	//mediaTxChannel = m_pMediaChannelArr[FECC_OUT_CHANNEL_INDEX];
	//if (mediaTxChannel != NULL)
	//	mediaTxChannel->OnTimerSendMedia();
	
		
	//invoke timer per all Rx channels
	CMediaChannel* mediaRxChannel = NULL;
	mediaRxChannel = m_pMediaChannelArr[VIDEO_IN_CHANNEL_INDEX];
	if (mediaRxChannel != NULL)
		mediaRxChannel->OnTimerRecvMedia();
	
	mediaRxChannel = m_pMediaChannelArr[AUDIO_IN_CHANNEL_INDEX];
	if (mediaRxChannel != NULL)
		mediaRxChannel->OnTimerRecvMedia();
		
	//mediaRxChannel = m_pMediaChannelArr[CONTENT_IN_CHANNEL_INDEX];
	//if (mediaRxChannel != NULL)
	//	mediaRxChannel->OnTimerRecvMedia();
		
	//mediaRxChannel = m_pMediaChannelArr[FECC_IN_CHANNEL_INDEX];
	//if (mediaRxChannel != NULL)
	//	mediaRxChannel->OnTimerRecvMedia();
		
		
}


////////////////////////////////////////////////////////////////////////////////////


void CEPConnectionObj::InvokeEPResetChannel(BYTE channelType)
{
	if ( CHANNEL_TYPE_AUDIO == (channelType & CHANNEL_TYPE_AUDIO))
	{
		//invoke reset video out
		CMediaChannel* pAudioTxChannel = m_pMediaChannelArr[AUDIO_OUT_CHANNEL_INDEX];
		if (pAudioTxChannel != NULL)
		{
			CSegment* pMsg = new CSegment;
			
			pAudioTxChannel->HandleEvent(pMsg, 0, AUDIO_OUT_RESET_EVENT_MESSAGE);
			
			POBJDELETE(pMsg);
		}
	}

	if ( CHANNEL_TYPE_VIDEO == (channelType & CHANNEL_TYPE_VIDEO))
	{
		//invoke reset video out
		CMediaChannel* pVideoTxChannel = m_pMediaChannelArr[VIDEO_OUT_CHANNEL_INDEX];
		if (pVideoTxChannel != NULL)
		{
			CSegment* pMsg = new CSegment;
			
			pVideoTxChannel->HandleEvent(pMsg, 0, VIDEO_OUT_RESET_EVENT_MESSAGE);
			
			POBJDELETE(pMsg);
		}
	}
}



void CEPConnectionObj::InvokeEPVideoUpdatePic()
{
	cmCapDirection channelDirection = (cmCapDirection)MEDIA_DIRECTION_IN;
	kChanneltype channelType = kIpVideoChnlType;
	int channelIndex = GetMediaChannelIndex(channelType, channelDirection);
	CMediaChannel* pMediaChannel = m_pMediaChannelArr[channelIndex];
	if (pMediaChannel != NULL)
	{	
		CSegment* pMsg = new CSegment;
		pMediaChannel->HandleEvent(pMsg, 0, VIDEO_IN_UPDATE_PIC_EVENT_MESSAGE);
		POBJDELETE(pMsg);
	}
}

////////////////////////////////////////////////////////////////////////////////////

void CEPConnectionObj::SetParticipantTicket(string participantTicket)
{
	m_participantTicket = participantTicket;
}

////////////////////////////////////////////////////////////////////////////////////

string CEPConnectionObj::GetParticipantTicket()
{
	return m_participantTicket;
}


void CEPConnectionObj::InvokeEPAudioUpdateChannel()
{
	CMediaChannel* pAudioTxChannel = m_pMediaChannelArr[AUDIO_OUT_CHANNEL_INDEX];
	if (pAudioTxChannel != NULL)
	{
		CSegment* pMsg = new CSegment;
		((CAudioChannel *)pAudioTxChannel)->SetSilentStream(m_bSendSilentStream);
		pAudioTxChannel->HandleEvent(pMsg, 0, AUDIO_OUT_UPDATE_CHANNEL_EVENT_MESSAGE);
		
		POBJDELETE(pMsg);
	}
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CEPAccess)

PEND_MESSAGE_MAP(CEPAccess, CStateMachine)


CEPAccess::CEPAccess(CTaskApp* pOwnerTask)	
		:CStateMachine(pOwnerTask)
{
	m_epRsrcId = DUMMY_PARTY_ID;
	m_index = -1;
//	m_pEPConnection = new CEPConnection(pOwnerTask);
}


CEPAccess::~CEPAccess()
{
	TRACEINTO << "CEPAccess::~CEPAccess";
	POBJDELETE(m_pEPConnection);
}

//////////////////////////////////////////////////////////////////////////////


DWORD CEPAccess::GetEpRsrcId() const
{
	return m_epRsrcId;
}

//////////////////////////////////////////////////////////////////////////////

void CEPAccess::SetEpRsrcId(const WORD epRscId)
{
	m_epRsrcId = epRscId;
}

//////////////////////////////////////////////////////////////////////////////


int CEPAccess::GetIndex() const
{
	return m_index;
}

//////////////////////////////////////////////////////////////////////////////

void CEPAccess::SetIndex(const int index)
{
	m_index = index;
}

//////////////////////////////////////////////////////////////////////////////

CEPConnection* CEPAccess::GetEPConnectionPtr()
{
	return m_pEPConnection;
}

//////////////////////////////////////////////////////////////////////////////

void CEPAccess::SetEPConnectionPtr(CEPConnection* ptr)
{
	m_pEPConnection = ptr;
}


//////////////////////////////////////////////////////////////////////////////

void CEPAccess::ExecuteEventMessage(CMplMcmsProtocol* pMplProtocol)
{
	m_pEPConnection->ExecuteEventMessage(pMplProtocol);
}


///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

DWORD CEPAccess::GetMonitorPartyId() const
{
	return m_monitorPartyId;
}

void CEPAccess::SetMonitorPartyId(const DWORD monitorPartyId)
{
	m_monitorPartyId = monitorPartyId;
}


DWORD CEPAccess::GetMonitorConfId() const
{
	return m_monitorConfId;
}


void CEPAccess::SetMonitorConfId(const DWORD monitorConfId)
{
	m_monitorConfId = monitorConfId;
}

void CEPAccess::ExecuteAPIMessage(CSegment* pSeg)
{
	m_pEPConnection->ExecuteAPIMessage(pSeg);
}




////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////  class CEPAccessObj
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CEPAccessObj)

PEND_MESSAGE_MAP(CEPAccessObj, CEPAccess)

////////////////////////////////////////////////////////////////////////////////////


CEPAccessObj::CEPAccessObj(CTaskApp* pOwnerTask) : CEPAccess(pOwnerTask)
{
	m_pEPConnection = new CEPConnectionObj(pOwnerTask);
}

////////////////////////////////////////////////////////////////////////////////////

CEPAccessObj::~CEPAccessObj()
{
	TRACEINTO << GetParticipantTicket() << " CEPAccessObj::~CEPAccessObj";
	POBJDELETE(m_pEPConnection);
}

////////////////////////////////////////////////////////////////////////////////////

void CEPAccessObj::ExecuteEventMessage(CMplMcmsProtocol* pMplProtocol)
{
	((CEPConnectionObj*)m_pEPConnection)->ExecuteEventMessage(pMplProtocol);
}

////////////////////////////////////////////////////////////////////////////////////

void CEPAccessObj::InvokeEPGlobalTxTimer()
{
	((CEPConnectionObj*)m_pEPConnection)->InvokeEPGlobalTxTimer();
}

////////////////////////////////////////////////////////////////////////////////////

void CEPAccessObj::InvokeEPResetChannel(BYTE channelType)
{
	((CEPConnectionObj*)m_pEPConnection)->InvokeEPResetChannel(channelType);
}

////////////////////////////////////////////////////////////////////////////////////

void CEPAccessObj::InvokeEPVideoUpdatePic()
{
	((CEPConnectionObj*)m_pEPConnection)->InvokeEPVideoUpdatePic();
}

//////////////////////////////////////////////////////////////////////////////

void CEPAccessObj::PrepareParticipantTicket()
{
	ALLOCBUFFER(buff, 256);
	
	sprintf(buff, "[%d, %d]", GetIndex(), GetEpRsrcId());
	
	m_participantTicket = buff;
	
	((CEPConnectionObj*)m_pEPConnection)->SetParticipantTicket(m_participantTicket);
	
	DEALLOCBUFFER(buff);
	
	TRACEINTO << "CEPAccessObj::PrepareParticipantTicket  - " << m_participantTicket;
}

//////////////////////////////////////////////////////////////////////////////

string CEPAccessObj::GetParticipantTicket()
{
	return m_participantTicket;
}

////////////////////////////////////////////////////////////////////////////////////

void CEPAccessObj::InvokeEPAudioUpdateChannel()
{
	((CEPConnectionObj*)m_pEPConnection)->InvokeEPAudioUpdateChannel();
}

