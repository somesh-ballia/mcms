//+========================================================================+
//                GideonSimLogicalModule.cpp                               |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimLogicalModule.cpp                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

// GideonSimLogicalModule.cpp:
//
/////////////////////////////////////////////////////////////////////////////
#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif


#include "Macros.h"
#include "Trace.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "InternalProcessStatuses.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsShelfMngr.h"
#include "OpcodesMcmsCardMngrMaintenance.h"
#include "OpcodesMcmsCardMngrRecording.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "OpcodesMcmsAudio.h"
#include "OpcodesMcmsAudioCntl.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsRtmIsdnMaintenance.h"
#include "OpcodesMcmsNetQ931.h"  // for NET_ opcodes (until implemented by other sides)
#include "OpcodesMcmsCardMngrBFCP.h"
#include "CardsStructs.h"
#include "IpMfaOpcodes.h"
#include "IpRtpReq.h"
#include "IpRtpInd.h"
#include "IpCmReq.h"
#include "IpCmInd.h"
#include "AcIndicationStructs.h"
#include "VideoStructs.h"
#include "IvrApiStructures.h"
#include "IVRPlayMessage.h"
#include "AudRequestStructs.h"
#include "AudIndicationStructs.h"
#include "McuMngrStructs.h"
#include "Q931Structs.h"

#include "SystemFunctions.h"
#include "StateMachine.h"
#include "TaskApi.h"
#include "MplMcmsProtocol.h"
#include "MplMcmsProtocolTracer.h"

#include "GideonSim.h"
#include "GideonSimConfig.h"
#include "GideonSimLogicalModule.h"
#include "GideonSimLogicalUnit.h"
#include "GideonSimCardAckStatusList.h"
#include "ObjString.h"
#include "VideoApiDefinitions.h"
#include "ArtDefinitions.h"

#include "Bonding.h"
#include "OpcodesMcmsBonding.h"
#include "muxint.h"
#include "OpcodesMcmsMux.h"
#include "AcRequestStructs.h"
//#include "../../../McmIncld/MPL/Card/PhysicalPortAudioCntl/AcDefinitions.h"
#include "AcDefinitions.h"
#include "OpcodesMcmsPCM.h"
#include "IpCommonUtilTrace.h"
#include "BFCPH239Translator.h"
#include "OpcodesMrmcCardMngrMrc.h"
#include "MrcStructs.h"
#ifndef __DISABLE_ICE__
#include "Lock.hxx"
#endif //__DISABLE_ICE__


#include "GideonSimLogicalParams.h"
#include "OpcodesMrmcCardMngrMrc.h"
#include "MrcStructs.h"

/////////////////////////////////////////////////////////////////////////////
//
//   GideonSimLogicalModule - base class (abstract) for all logical modules
//
/////////////////////////////////////////////////////////////////////////////
//               Action table
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CGideonSimLogicalModule)


PEND_MESSAGE_MAP(CGideonSimLogicalModule,CStateMachine)


/////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CGideonSimLogicalModule::CGideonSimLogicalModule(CTaskApp* pTask,WORD boardId,WORD subBoardId)
			: CStateMachine(pTask),
              m_wBoardId(boardId),
              m_wSubBoardId(subBoardId)
{
//	InitTimer(pTask->GetRcvMbx());
	m_pTaskApi = new CTaskApi;
	m_pTaskApi->CreateOnlyApi(pTask->GetRcvMbx(),this,pTask->GetLocalQueue());
	/*if (this is CGideonSimSwitchLogical)
	{
		m_version_burn_rate = 50;
		m_IPMC_burn_rate = 50;
	}
	else
	{*/
		m_version_burn_rate = 20;
		m_IPMC_burn_rate = 150;
	//}

		m_stop_software_upgrade = FALSE;
		m_stop_ipmc_upgrade = FALSE;
		m_pause_software_upgrade = FALSE;
		m_pause_ipmc_upgrade = FALSE;

		m_pTranslator = (BFCPH239Translator *) new char[sizeof(BFCPH239Translator)];

		if (m_pTranslator)
		{
			if (statusOK != InitializeTranslatorDefaults(m_pTranslator, 1, kBFCPPriorityNormal,1, 1, kBFCPFloorCtrlServer))
			{
				PTRACE(eLevelInfoNormal, "CGideonSimLogicalModule::CGideonSimLogicalModule - fail to init m_pTranslator");

				PDELETEA(m_pTranslator);
			}
		}

		m_bfcpTransportType = eUnknownTransportType;
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimLogicalModule::~CGideonSimLogicalModule()
{
//	DestroyTimer();
	DeleteAllTimers();

	if( CPObject::IsValidPObjectPtr(m_pTaskApi) )  {
		m_pTaskApi->DestroyOnlyApi();
		POBJDELETE(m_pTaskApi);
	}

	if (m_pTranslator)
		PDELETEA(m_pTranslator);
}

/////////////////////////////////////////////////////////////////////////////
void* CGideonSimLogicalModule::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimLogicalModule::Startup()
{
	DispatchEvent(CM_STARTUP,NULL);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimLogicalModule::ProcessMcmsMsg(CSegment* pMsg)
{
	DispatchEvent(CM_MCMS_MSG,pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimLogicalModule::Ack( CMplMcmsProtocol& rMplProt, const STATUS status ) const
{
	const CGideonSimCardAckStatus*  pAckStatus = ::GetCardAckStatusList()->GetAckStatus(rMplProt.getCommonHeaderOpcode());
	if( pAckStatus != NULL  &&  pAckStatus->GetIsSendAck() == FALSE )
	{
		TRACEINTO	<< " CGideonSimLogicalModule::Ack - ACK for opcode <"
					<< (int)rMplProt.getCommonHeaderOpcode()
					<< "> will not be sent";
		return;
	}
	TRACEINTO	<< " !@# CGideonSimLogicalModule::Ack - ACK for opcode <"
				<< (int)rMplProt.getCommonHeaderOpcode()
				<< "> will be sent";

	ACK_IND_S  rAckStruct;
	memset(&rAckStruct,0,sizeof(ACK_IND_S));
	rAckStruct.ack_base.ack_opcode = rMplProt.getCommonHeaderOpcode();
	rAckStruct.ack_base.status = status;
	rAckStruct.ack_base.ack_seq_num = rMplProt.getMsgDescriptionHeaderRequest_id();

	if( pAckStatus != NULL )
		rAckStruct.ack_base.status = pAckStatus->GetStatus();

	switch( rMplProt.getCommonHeaderOpcode() )
	{
		case H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:
		{
			TUpdatePortOpenRtpChannelReq* pStruct = (TUpdatePortOpenRtpChannelReq*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->unChannelDirection;
			rAckStruct.media_type      = pStruct->unChannelType;
			break;
		}
		case H320_RTP_UPDATE_CHANNEL_REQ:
		{
			TUpdateRtpChannelReq* pStruct = (TUpdateRtpChannelReq*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->unChannelDirection;
			rAckStruct.media_type      = pStruct->unChannelType;
			break;
		}
		case H320_RTP_UPDATE_CHANNEL_RATE_REQ:
		{
			TUpdateRtpChannelRateReq* pStruct = (TUpdateRtpChannelRateReq*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->unChannelDirection;
			rAckStruct.media_type      = pStruct->unChannelType;
			break;
		}
		case CONF_PARTY_MRMP_OPEN_CHANNEL_REQ: /// to change
		{
			MrmpOpenChannelRequestStruct* pStruct = (MrmpOpenChannelRequestStruct*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->m_channelDirection;
			rAckStruct.media_type      = pStruct->m_channelType;
			rAckStruct.channelHandle      = 17532;
			break;
		}
		case CONF_PARTY_MRMP_UPDATE_CHANNEL_REQ: /// to change
		{
			MrmpOpenChannelRequestStruct* pStruct = (MrmpOpenChannelRequestStruct*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->m_channelDirection;
			rAckStruct.media_type      = pStruct->m_channelType;
			rAckStruct.channelHandle      = 17532;
			break;
		}
/*		case CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ: /// to change
		{
			MrmpCloseChannelRequestMessage* pStruct = (MrmpCloseChannelRequestMessage*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->m_channelDirection;
			rAckStruct.media_type      = pStruct->m_channelType;
			rAckStruct.channelHandle      = 17532;
			break;
		}*/

	}

	rMplProt.AddCommonHeader(ACK_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	rMplProt.AddData(sizeof(ACK_IND_S),(char*)(&rAckStruct));

	SendToCmForMplApi(rMplProt);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimLogicalModule::AckIp( CMplMcmsProtocol& rMplProt, const STATUS status )
{
	const CGideonSimCardAckStatus*  pAckStatus = ::GetCardAckStatusList()->GetAckStatus(rMplProt.getCommonHeaderOpcode());
	if( pAckStatus != NULL  &&  pAckStatus->GetIsSendAck() == FALSE )
	{
		TRACEINTO	<< " CGideonSimLogicalModule::AckIp - ACK for opcode <"
					<< (int)rMplProt.getCommonHeaderOpcode()
					<< "> will not be sent";
		return;
	}

	ACK_IND_S  rAckStruct;
	memset(&rAckStruct,0,sizeof(ACK_IND_S));
	rAckStruct.ack_base.ack_opcode  = rMplProt.getCommonHeaderOpcode();
	rAckStruct.ack_base.status      = status;
	rAckStruct.ack_base.ack_seq_num = rMplProt.getMsgDescriptionHeaderRequest_id();

	if( pAckStatus != NULL )
		rAckStruct.ack_base.status  = pAckStatus->GetStatus();

	rAckStruct.media_direction = 0;
	rAckStruct.media_type      = 0;

	switch( rMplProt.getCommonHeaderOpcode() )
	{
		case H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:
		case SIP_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:
		{
			TUpdatePortOpenRtpChannelReq* pStruct = (TUpdatePortOpenRtpChannelReq*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->unChannelDirection;
			rAckStruct.media_type      = pStruct->unChannelType;
			break;
		}
		case H323_RTP_UPDATE_CHANNEL_REQ:
		case SIP_RTP_UPDATE_CHANNEL_REQ:
		{
			TUpdateRtpChannelReq* pStruct = (TUpdateRtpChannelReq*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->unChannelDirection;
			rAckStruct.media_type      = pStruct->unChannelType;
			break;
		}
		case H323_RTP_STREAM_ON_REQ:
		case SIP_RTP_STREAM_ON_REQ:
		{
			TStreamOnReq* pStruct = (TStreamOnReq*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->unChannelDirection;
			rAckStruct.media_type      = pStruct->unChannelType;
			break;
		}
		case H323_RTP_STREAM_OFF_REQ:
		case SIP_RTP_STREAM_OFF_REQ:
		{
			TStreamOffReq* pStruct = (TStreamOffReq*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->unChannelDirection;
			rAckStruct.media_type      = pStruct->unChannelType;
			break;
		}
		case CONFPARTY_CM_OPEN_UDP_PORT_REQ:
		case CONFPARTY_CM_UPDATE_UDP_ADDR_REQ:
		case SIP_CM_OPEN_UDP_PORT_REQ:
		case SIP_CM_UPDATE_UDP_ADDR_REQ:
		{
			TOpenUdpPortOrUpdateUdpAddrMessageStruct* pStruct = (TOpenUdpPortOrUpdateUdpAddrMessageStruct*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->tCmOpenUdpPortOrUpdateUdpAddr.channelDirection;
			rAckStruct.media_type      = pStruct->tCmOpenUdpPortOrUpdateUdpAddr.channelType;

			if (((rAckStruct.media_type == kIpVideoChnlType) || (rAckStruct.media_type == kIpContentChnlType))
			   && (rAckStruct.media_direction == cmCapTransmit))
				SendVideoRefreshInd(rMplProt, rAckStruct.media_type, rAckStruct.media_direction, RTCP_PLI); // Send intra request (for testing)
			TCmRtcpMsg packetLossStruct;
			(packetLossStruct.tCmRtcpMsgInfo).uMediaType = rAckStruct.media_type;
		//	packetLossStruct.media_direction = rAckStruct.media_direction;
			(packetLossStruct.tCmRtcpMsgInfo).uMsgType = RTCP_INTRA_RTV;//RTCP_PLI;
			CMplMcmsProtocol oMplProtocol;
			oMplProtocol.AddPortDescriptionHeader( rMplProt.getPortDescriptionHeaderParty_id(), 0xffffffff, rMplProt.getPortDescriptionHeaderConnection_id());
			FillMplProtocol( &oMplProtocol, IP_CM_RTCP_MSG_IND,(BYTE*)(&packetLossStruct),sizeof(TCmRtcpMsg));
			SendToCmForMplApi(oMplProtocol);

			if(rMplProt.getCommonHeaderOpcode() == CONFPARTY_CM_OPEN_UDP_PORT_REQ)
				SendBfcpTcpTransportInd(rMplProt);

			break;
		}
		case CONFPARTY_CM_CLOSE_UDP_PORT_REQ:
		case SIP_CM_CLOSE_UDP_PORT_REQ:
		{
			TCloseUdpPortMessageStruct* pStruct = (TCloseUdpPortMessageStruct*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->tCmCloseUdpPort.channelDirection;
			rAckStruct.media_type      = pStruct->tCmCloseUdpPort.channelType;
			break;
		}
		case H323_RTP_PARTY_MONITORING_REQ:
		case RTP_PARTY_VIDEO_CHANNELS_STATISTICS_REQ: //CDR_MCCF
		{
			rAckStruct.media_direction = 1;
			rAckStruct.media_type      = 1;
			break;
		}
		case ART_CONTENT_ON_REQ:
		case ART_CONTENT_OFF_REQ:
		{
			TContentOnOffReq* pStruct = (TContentOnOffReq*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->unChannelDirection;
			rAckStruct.media_type      = kIpContentChnlType;
			break;
		}
		case ART_EVACUATE_REQ:
		{
			TEvacuateReq* pStruct = (TEvacuateReq*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->unChannelDirection;
			rAckStruct.media_type      = pStruct->unChannelType;
			break;
		}

		case IP_CM_STOP_PREVIEW_CHANNEL:
		{
			mcReqCmCloseUdpPort* pStruct = (mcReqCmCloseUdpPort*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->channelDirection;
			rAckStruct.media_type      = pStruct->channelType;
			break;
		}
		case IP_CM_START_PREVIEW_CHANNEL:
		{
			mcReqCmStartPreviewChannel* pStruct = (mcReqCmStartPreviewChannel*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->channelDirection;
			rAckStruct.media_type      = pStruct->channelType;
			break;
		}
		case IP_CM_RTCP_VIDEO_PREFERENCE_REQ:
		{
			rAckStruct.media_direction = 2;
			rAckStruct.media_type      = kIpVideoChnlType;
			break;
		}
		case H323_RTP_LPR_MODE_CHANGE_REQ:
		{
			TLprModeChangeReq* pStruct = (TLprModeChangeReq*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->unChannelDirection;
			rAckStruct.media_type      = pStruct->unChannelType;
			break;
		}
		case H323_RTP_LPR_MODE_RESET_REQ:
		{
			TLprModeChangeReq* pStruct = (TLprModeChangeReq*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->unChannelDirection;
			rAckStruct.media_type      = pStruct->unChannelType;
			break;
		}
		case CONF_PARTY_MRMP_OPEN_CHANNEL_REQ:
		{
			MrmpOpenChannelRequestStruct* pStruct = (MrmpOpenChannelRequestStruct*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->m_channelDirection;
			rAckStruct.media_type      = pStruct->m_channelType;
			rAckStruct.channelHandle   = 10000 + rAckStruct.media_type + rAckStruct.media_direction;
			ForwardMRMInfo2Endpoints(rMplProt, rAckStruct.channelHandle);
			break;
		}
		case CONF_PARTY_MRMP_UPDATE_CHANNEL_REQ:
		{
			MrmpOpenChannelRequestStruct* pStruct = (MrmpOpenChannelRequestStruct*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->m_channelDirection;
			rAckStruct.media_type      = pStruct->m_channelType;
			rAckStruct.channelHandle   = 10000 + rAckStruct.media_type + rAckStruct.media_direction;
			break;
		}
		case CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ:
		{
			MrmpCloseChannelRequestStruct* pStruct = (MrmpCloseChannelRequestStruct*)rMplProt.GetData();
			rAckStruct.media_direction = pStruct->tMrmpCloseChannelRequestMessage.m_channelDirection;
			rAckStruct.media_type      = pStruct->tMrmpCloseChannelRequestMessage.m_channelType;
			break;
		}
		case IP_CM_DTLS_CLOSE_REQ:
		{
			TDtlsCloseStruct *pStruct = (TDtlsCloseStruct *) rMplProt.GetData();
			rAckStruct.media_direction = pStruct->tCmDtlsClose.channelDirection;
			rAckStruct.media_type      = pStruct->tCmDtlsClose.channelType;
			break;
		}
		default:
		{
			rAckStruct.ack_base.status  = STATUS_FAIL;
			break;
		}
	}

	rMplProt.AddCommonHeader(ACK_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	rMplProt.AddData(sizeof(ACK_IND_S),(char*)(&rAckStruct));

	SendToCmForMplApi(rMplProt);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimLogicalModule::SendToCmForMplApi(CMplMcmsProtocol& rMplProtocol) const
{
#ifndef __DISABLE_ICE__
	resip::Lock lock(m_TaskApiMutex);
#endif	//__DISABLE_ICE__
	if( CPObject::IsValidPObjectPtr(m_pTaskApi) ) {

		CSegment* pParamSeg = new CSegment;
		rMplProtocol.Serialize(*pParamSeg);

		m_pTaskApi->SendLocalMessage(pParamSeg,FORWARD_MSG_MPLAPI);

		CMplMcmsProtocolTracer(rMplProtocol).TraceMplMcmsProtocol("GIDEON_SIM_SEND_TO_MPL_API");
		TraceMplMcms(&rMplProtocol);
	}
}

//////////////////////////////////////////////////////////////////////
void CGideonSimLogicalModule::TraceMplMcms(const CMplMcmsProtocol* pMplProt) const
{
	static const CProcessBase * process = CProcessBase::GetProcess();
	const std::string &str = process->GetOpcodeAsString(pMplProt->getCommonHeaderOpcode());
	const char* pszOpcodeStr = str.c_str();

	ALLOCBUFFER(pszMessage,512);
	sprintf(pszMessage,"Req: <%d>; HEADER: Board<%d>, SubBoard<%d>, Unit<%d>; OPCODE: <%s>",
		(int)pMplProt->getMsgDescriptionHeaderRequest_id(),
		(int)pMplProt->getPhysicalInfoHeaderBoard_id(),
		(int)pMplProt->getPhysicalInfoHeaderSub_board_id(),
		(int)pMplProt->getPhysicalInfoHeaderUnit_id(),
		pszOpcodeStr);
    LOGGER_TRACE2(eLevelInfoHigh,"GIDEON_SIM_SEND_TO_MPL_API - ",pszMessage);
    DEALLOCBUFFER(pszMessage);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimLogicalModule::FillMplProtocol( CMplMcmsProtocol* pMplProt,
		const DWORD opcode,const BYTE* pData,const DWORD nDataLen ) const
{
	DWORD  payloadLen   =  sizeof(COMMON_HEADER_S)
						+  sizeof(MESSAGE_DESCRIPTION_HEADER_S)
						+  sizeof(PHYSICAL_INFO_HEADER_S)
						+  nDataLen;
	DWORD payloadOffset =  sizeof(COMMON_HEADER_S);

	pMplProt->AddCommonHeader(opcode,MPL_PROTOCOL_VERSION_NUM,0,
				(BYTE)eMpl,(BYTE)eMcms,SystemGetTickCount().GetIntegerPartForTrace(),
				payloadLen, payloadOffset,
				(DWORD)eHeaderMsgDesc,sizeof(MESSAGE_DESCRIPTION_HEADER_S));

	pMplProt->AddMessageDescriptionHeader(123/*requestId*/,(DWORD)eMpl/*entity_type*/,
				SystemGetTickCount().GetIntegerPartForTrace(),eHeaderPhysical,sizeof(PHYSICAL_INFO_HEADER_S));

// 	pMplProt->AddPortDescriptionHeader(0/*partyId*/,0/*confId*/ /*,connId,lrt1,lrt2,0,0,0,0*/);

	pMplProt->AddPhysicalHeader(0/*box_id*/,(BYTE)m_wBoardId,(BYTE)m_wSubBoardId);

	pMplProt->AddData(nDataLen,(const char*)pData);
}
/////////////////////////////////////////////////////////////////////////////
void CGideonSimLogicalModule::FillMplProtocol( CMplMcmsProtocol* pMplProt,
		const DWORD opcode ) const
{
	DWORD  payloadLen   =  sizeof(COMMON_HEADER_S)
						+  sizeof(MESSAGE_DESCRIPTION_HEADER_S)
						+  sizeof(PHYSICAL_INFO_HEADER_S);
						//+  nDataLen;
	DWORD payloadOffset =  sizeof(COMMON_HEADER_S);

	pMplProt->AddCommonHeader(opcode,MPL_PROTOCOL_VERSION_NUM,0,
				(BYTE)eMpl,(BYTE)eMcms,SystemGetTickCount().GetIntegerPartForTrace(),
				payloadLen, payloadOffset,
				(DWORD)eHeaderMsgDesc,sizeof(MESSAGE_DESCRIPTION_HEADER_S));

	pMplProt->AddMessageDescriptionHeader(123/*requestId*/,(DWORD)eMpl/*entity_type*/,
				SystemGetTickCount().GetIntegerPartForTrace(),eHeaderPhysical,sizeof(PHYSICAL_INFO_HEADER_S));

// 	pMplProt->AddPortDescriptionHeader(0/*partyId*/,0/*confId*/ /*,connId,lrt1,lrt2,0,0,0,0*/);

	pMplProt->AddPhysicalHeader(0/*box_id*/,(BYTE)m_wBoardId,(BYTE)m_wSubBoardId);

	//pMplProt->AddData(nDataLen,(const char*)pData);
}

void CGideonSimLogicalModule::StartVersionUpgrade()
{
	m_software_upgrade_done = -1;
	StartTimer(SOFTWARE_UPGRADE_TIMER, m_version_burn_rate);

	CMplMcmsProtocol*  pMplProt = new CMplMcmsProtocol;

	pMplProt->AddCommonHeader(CM_UPGRADE_NEW_VERSION_READY_ACK_IND,
				MPL_PROTOCOL_VERSION_NUM,
				0,
				(BYTE)eMpl,
				(BYTE)eMcms);

	pMplProt->AddPhysicalHeader(0,m_wBoardId,m_wSubBoardId);
	SendToCmForMplApi(*pMplProt);
	POBJDELETE(pMplProt);

}

void CGideonSimLogicalModule::SetBurnAction(eBurnActionTypes burnActionType,eBurnTypes burnType)
{
	if (burnActionType == eStopBurn && burnType == eVersionBurnType)
		m_stop_software_upgrade = TRUE;
	else if (burnActionType == eStopBurn && burnType == eIpmcBurnType)
		m_stop_ipmc_upgrade = TRUE;
	else if (burnActionType == ePauseBurn && burnType == eVersionBurnType)
		m_pause_software_upgrade = TRUE;
	else if (burnActionType == ePauseBurn && burnType == eIpmcBurnType)
		m_pause_ipmc_upgrade = TRUE;
	else if (burnActionType == eResumeBurn && burnType == eVersionBurnType)
		m_pause_software_upgrade = FALSE;
	else if (burnActionType == eResumeBurn && burnType == eIpmcBurnType)
		m_pause_ipmc_upgrade = FALSE;
	else if (burnActionType == eStartBurn && burnType == eVersionBurnType)
		StartVersionUpgrade();
	else if (burnActionType == eStartBurn && burnType == eIpmcBurnType)
	{
		m_ipmc_software_upgrade_done = 0;
		StartTimer(SOFTWARE_IPMC_UPGRADE_TIMER,m_IPMC_burn_rate);
	}
}


void CGideonSimLogicalModule::OnTimerSoftwareUpgrade(CSegment* pMsg)
{
	TRACEINTO	<< " CGideonSimLogicalModule::OnTimerSoftwareUpgrade:burn rate: "
				<< m_version_burn_rate;

	if (m_stop_software_upgrade)
	{
		TRACEINTO	<< " CGideonSimLogicalModule::OnTimerSoftwareUpgrade:burn stopped"
					<< ";m_software_upgrade_done:" << m_software_upgrade_done;
		return;
	}

	if (m_version_burn_rate == 0)//no need to install version
		m_software_upgrade_done = 100;

	if (m_software_upgrade_done < 100)
	  {
		if (!m_pause_software_upgrade)
		{
			m_software_upgrade_done++;
		}
		else
		{
			TRACEINTO	<< " CGideonSimLogicalModule::OnTimerSoftwareUpgrade:burn paused"
						<< ";m_software_upgrade_done:" << m_software_upgrade_done;

		}

	    UPGRADE_PROGRESS_IND_S rStruct;
	    rStruct.progress_precents = m_software_upgrade_done;
	    CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	    FillMplProtocol( pMplProtocol,CM_UPGRADE_PROGRESS_IND,
			(BYTE*)(&rStruct),sizeof(UPGRADE_PROGRESS_IND_S));
	    SendToCmForMplApi(*pMplProtocol);
	    POBJDELETE(pMplProtocol);

	  }

	if (m_software_upgrade_done < 100)
	  {
	    StartTimer(SOFTWARE_UPGRADE_TIMER,m_version_burn_rate);
	  }
	else
	  {
	     UPGRADE_IPMC_IND_S rStruct;
	     if (m_IPMC_burn_rate == 0)//no need to burn ipmc version
	    	 rStruct.require_ipmc_upgrade = FALSE;
	     else
	    	 rStruct.require_ipmc_upgrade = TRUE;//need to move to burn ipmc version

	     CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	     FillMplProtocol( pMplProtocol,CM_UPGRADE_IPMC_IND,
			      (BYTE*)(&rStruct),sizeof(UPGRADE_IPMC_IND_S));
	     SendToCmForMplApi(*pMplProtocol);
	     POBJDELETE(pMplProtocol);

	     m_software_upgrade_done = -1;
	  }

}

///////////////////////////////////////////////////////////////////////////////
void CGideonSimLogicalModule::OnTimerIpmcSoftwareUpgrade(CSegment* pMsg)
{
	TRACEINTO	<< " CGideonSimLogicalModule::OnTimerIpmcSoftwareUpgrade:burn rate: "
					<< m_IPMC_burn_rate;

	if (m_stop_ipmc_upgrade)
		return;


	if (m_ipmc_software_upgrade_done < 100)
	  {
		if (!m_pause_ipmc_upgrade)
			m_ipmc_software_upgrade_done++;

	    UPGRADE_PROGRESS_IND_S rStruct;
	    rStruct.progress_precents = m_ipmc_software_upgrade_done;
	    CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	    FillMplProtocol( pMplProtocol,CM_UPGRADE_IPMC_PROGRESS_IND,
			(BYTE*)(&rStruct),sizeof(UPGRADE_PROGRESS_IND_S));
	    SendToCmForMplApi(*pMplProtocol);
	    POBJDELETE(pMplProtocol);

	  }



	if (m_ipmc_software_upgrade_done < 100)
	{
	  StartTimer(SOFTWARE_IPMC_UPGRADE_TIMER,m_IPMC_burn_rate);
	}
	else
	  {
	    //	     CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	    //	     pMplProtocol->AddCommonHeader(CM_UPGRADE_IPMC_ALMOST_COMPLETE_IND,
	    //				       MPL_PROTOCOL_VERSION_NUM,
	    //				       0,(BYTE)eMpl,(BYTE)eMcms);

	    //	     pMplProtocol->AddPhysicalHeader(0,m_wBoardId,m_wSubBoardId);

	    //	     SendToCmForMplApi(*pMplProtocol);
	    //	     POBJDELETE(pMplProtocol);

	     m_ipmc_software_upgrade_done = -1;
	  }

}
///////////////////////////////////////////////////////////////////////////////
void CGideonSimLogicalModule::SendVideoRefreshInd(CMplMcmsProtocol& rMplProt, APIU32 mediaType, APIU32 mediaDirection, APIU32 msgType, APIU32 tipPosition) const
{
	TCmRtcpMsg packetLossStruct;
	(packetLossStruct.tCmRtcpMsgInfo).uMediaType = mediaType;
	(packetLossStruct.tCmRtcpMsgInfo).uMsgType = msgType;
	(packetLossStruct.tCmRtcpMsgInfo).uTipPosition = tipPosition;
	CMplMcmsProtocol oMplProtocol;
	oMplProtocol.AddPortDescriptionHeader(rMplProt.getPortDescriptionHeaderParty_id(), rMplProt.getPortDescriptionHeaderConf_id(), rMplProt.getPortDescriptionHeaderConnection_id());
	FillMplProtocol( &oMplProtocol, IP_CM_RTCP_MSG_IND,(BYTE*)(&packetLossStruct),sizeof(TCmRtcpMsg));
	SendToCmForMplApi(oMplProtocol);
}

void CGideonSimLogicalModule::SendBfcpTcpTransportInd(CMplMcmsProtocol& rMplProt)
{
	PTRACE(eLevelInfoNormal,"CGideonSimLogicalModule::SendBfcpTcpTransportInd");

	const TOpenUdpPortOrUpdateUdpAddrMessageStruct *pTbConnectStruct = (const TOpenUdpPortOrUpdateUdpAddrMessageStruct *)rMplProt.getpData();
	const mcReqCmOpenUdpPortOrUpdateUdpAddr& pUdpSt  = pTbConnectStruct->tCmOpenUdpPortOrUpdateUdpAddr;

	TRACEINTO << "CGideonSimLogicalModule::SendBfcpTcpTransportInd type=" << pUdpSt.channelType
			  << " direction=" << pUdpSt.channelDirection << " conn=" << pUdpSt.connMode;

	if(pUdpSt.channelType == kBfcpChnlType)
	{
		if(pUdpSt.connMode == eTcpActive || pUdpSt.connMode == eTcpPassive)
			m_bfcpTransportType = eTransportTypeUdp;
		else
			m_bfcpTransportType = eTransportTypeTcp;

		PTRACE2INT(eLevelInfoNormal,"CGideonSimLogicalModule::SendBfcpTcpTransportInd set bfcp transport=", m_bfcpTransportType);
	}

	if(pUdpSt.channelType == kBfcpChnlType && pUdpSt.channelDirection == cmCapTransmit &&
			(pUdpSt.connMode == eTcpActive || pUdpSt.connMode == eTcpPassive))
	{
		PTRACE(eLevelInfoNormal,"CGideonSimLogicalModule::SendBfcpTcpTransportInd for BFCP TCP transmit open port request");

		mcIndBfcpTransport* pTransport = new mcIndBfcpTransport; AUTO_DELETE(pTransport);
		DWORD dataLen = sizeof(mcIndBfcpTransport);
		memset(pTransport, 0, dataLen);

		pTransport->status = bfcp_msg_status_connected;

		CMplMcmsProtocol oMplProtocol;
		oMplProtocol.AddPortDescriptionHeader(rMplProt.getPortDescriptionHeaderParty_id(), rMplProt.getPortDescriptionHeaderConf_id(), rMplProt.getPortDescriptionHeaderConnection_id());
		FillMplProtocol( &oMplProtocol, IP_CM_BFCP_TCP_TRANSPORT_IND,(BYTE*)(pTransport),sizeof(mcIndBfcpTransport));
		SendToCmForMplApi(oMplProtocol);
	}
}

///////////////////////////////////////////////////////////////////////////////
void CGideonSimLogicalModule::SendDtlsEndInd(CMplMcmsProtocol& rMplProt)
{
    PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::SendDtlsEndInd");

    APIU32  status          = STATUS_OK;
    APIS32  opCode          = 0;

    size_t size = sizeof(mcIndCmDtlsEnd);

    mcIndCmDtlsEnd *pMsg = new mcIndCmDtlsEnd; AUTO_DELETE(pMsg);

	CMplMcmsProtocol oMplProtocol;
	oMplProtocol.AddPortDescriptionHeader(rMplProt.getPortDescriptionHeaderParty_id(), rMplProt.getPortDescriptionHeaderConf_id(), rMplProt.getPortDescriptionHeaderConnection_id());

	TDtlsStartStruct* pStruct = (TDtlsStartStruct*)rMplProt.GetData();

	pMsg->status		= STATUS_OK;
	pMsg->channelType 	= pStruct->tCmDtlsStart.channelType;
	pMsg->channelDirection = pStruct->tCmDtlsStart.channelDirection;

	memcpy(&pMsg->sdesCap, &pStruct->tCmDtlsStart.sdesCap, sizeof(sdesCapSt));
	char* key = pMsg->sdesCap.keyParamsList[0].keyInfo.keySalt;
	memset(key,0,MAX_BASE64_KEY_SALT_LEN);
	strcpy(key,"newDtlsKeyFromCM");


	PTRACE2INT(eLevelInfoNormal,"CGideonSimBarakLogical::SendDtlsEndInd, is SRTP enable:", pMsg->sdesCap.bIsSrtpInUse);

	FillMplProtocol( &oMplProtocol, IP_CM_DTLS_END_IND,(BYTE*)(pMsg),size);
	SendToCmForMplApi(oMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimLogicalModule::ForwardMRMInfo2Endpoints(CMplMcmsProtocol& rMplProt, DWORD channelId) const
{
	PTRACE(eLevelInfoNormal,"CGideonSimLogicalModule::ForwardMRMInfo2Endpoints - SEND TO E.P.");

	CSegment msg;

	WORD unitId = rMplProt.getPhysicalInfoHeaderUnit_id();
	WORD portId = rMplProt.getPhysicalInfoHeaderPort_id();

	msg << (DWORD)rMplProt.getOpcode()
		<< (WORD)m_wBoardId
		<< (WORD)m_wSubBoardId
		<< (WORD)unitId
		<< (WORD)portId;


	msg << (DWORD)rMplProt.getPortDescriptionHeaderConf_id()
		<< (DWORD)rMplProt.getPortDescriptionHeaderParty_id()
		<< (DWORD)rMplProt.getPortDescriptionHeaderConnection_id()
		<< (DWORD)channelId;

	::SendMRMMessageToEndpointsSimApp(msg);
}

