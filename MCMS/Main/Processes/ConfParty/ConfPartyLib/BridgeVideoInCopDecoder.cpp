 //+========================================================================+
//                        BridgeVideoOutCopEncoder.cpp                          |
//					      Copyright 2005 Polycom                           |
//                        All Rights Reserved.                             |
//-------------------------------------------------------------------------|
// FILE:       BridgeVideoInCopDecoder.cpp                                     |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Keren                                                       |
//-------------------------------------------------------------------------|
// Who  | Date  Aug-2009  | Description                                    |
//-------------------------------------------------------------------------|
//
//+========================================================================+

#include "BridgeVideoInCopDecoder.h"

#include "VideoHardwareInterface.h"
#include "StatusesGeneral.h"
//#include "Layout.h"
//#include "LayoutHandler.h"
#include "TraceStream.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "HostCommonDefinitions.h"
#include "ConfPartyGlobals.h"
#include "H264Util.h"


//// Time-out values
//#define     VIDEO_OUT_CHANGE_LAYOUT_TOUT	8*SECOND //8 second timeout
//#define		SITE_NAME_DISPLAY_TOUT			5*SECOND
//#define		SLIDE_INTRA_TIMER	SECOND/2
//
//
//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );

LayoutType GetNewLayoutType(const BYTE oldLayoutType);

// ------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------CBridgeVideoOutCopDecoder--------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------

PBEGIN_MESSAGE_MAP(CBridgeVideoInCopDecoder)
	ONEVENT(CONNECT_VIDEO_IN,			OPENED,			    CBridgeVideoInCopDecoder::OnVideoBridgePartyConnectOPENED)
	ONEVENT(CONNECT_VIDEO_IN,			CLOSING,		    CBridgeVideoInCopDecoder::OnVideoBridgePartyConnectCLOSING)
	ONEVENT(CONNECT_VIDEO_IN,			CONNECTING,			CBridgeVideoInCopDecoder::OnVideoBridgePartyConnectCONNECTING)

	ONEVENT(DISCONNECT_VIDEO_IN,		OPENED,			    CBridgeVideoInCopDecoder::OnVideoBridgePartyDisConnectOPENED)
	ONEVENT(DISCONNECT_VIDEO_IN,		CLOSING,		    CBridgeVideoInCopDecoder::OnVideoBridgePartyDisConnectCLOSING)
	ONEVENT(DISCONNECT_VIDEO_IN,		CONNECTING,			CBridgeVideoInCopDecoder::OnVideoBridgePartyDisConnectCONNECTING)

	ONEVENT(CLOSE_VIDEO_IN,				IDLE,				CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseIDLE)
	ONEVENT(CLOSE_VIDEO_IN,				SETUP,				CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseSETUP)
	ONEVENT(CLOSE_VIDEO_IN,				CONNECTED,			CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseCONNECTED)
	ONEVENT(CLOSE_VIDEO_IN,				DISCONNECTING,		CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseDISCONNECTING)
	ONEVENT(CLOSE_VIDEO_IN,				CLOSING,			CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseCLOSING)
	ONEVENT(CLOSE_VIDEO_IN,				CONNECTING,			CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseCONNECTING)
	ONEVENT(CLOSE_VIDEO_IN,				OPENED,				CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseOPENED)

	ONEVENT(UPDATE_VIDEO_IN_PARAMS,		OPENED,			    CBridgeVideoInCopDecoder::OnVideoBridgePartyUpdateVideoParamOPENED)
	ONEVENT(UPDATE_VIDEO_IN_PARAMS,		CLOSING,		    CBridgeVideoInCopDecoder::OnVideoBridgePartyUpdateVideoParamCLOSING)
	ONEVENT(UPDATE_VIDEO_IN_PARAMS,		CONNECTING,			CBridgeVideoInCopDecoder::OnVideoBridgePartyUpdateVideoParamCONNECTING)

	ONEVENT(ACK_IND,					CLOSING,			CBridgeVideoInCopDecoder::OnMplAckCLOSING)
	ONEVENT(ACK_IND,					CONNECTING,			CBridgeVideoInCopDecoder::OnMplAckCONNECTING)

	ONEVENT(VIDEO_DECODER_SYNC_IND,		OPENED,				CBridgeVideoInCopDecoder::OnMplDecoderSyncOPENED)
	ONEVENT(VIDEO_DECODER_SYNC_IND,		CLOSING,			CBridgeVideoInCopDecoder::OnMplDecoderSyncCLOSING)
	ONEVENT(VIDEO_DECODER_SYNC_IND,		CONNECTING,			CBridgeVideoInCopDecoder::OnMplDecoderSyncCONNECTING)

	ONEVENT(VIDEO_DECODER_SYNC_TOUT, 	OPENED,				CBridgeVideoInCopDecoder::NullActionFunction)
	ONEVENT(VIDEO_DECODER_SYNC_TOUT,	CLOSING,			CBridgeVideoInCopDecoder::NullActionFunction)
	ONEVENT(VIDEO_DECODER_SYNC_TOUT,	CONNECTING,			CBridgeVideoInCopDecoder::NullActionFunction)


	ONEVENT(DECODER_RECURRENT_INTRA_REQ_TIMEOUT, CLOSING,    CBridgeVideoInCopDecoder::NullActionFunction)
    ONEVENT(DECODER_RECURRENT_INTRA_REQ_TIMEOUT, CONNECTING, CBridgeVideoInCopDecoder::NullActionFunction)




PEND_MESSAGE_MAP(CBridgeVideoInCopDecoder,CBridgePartyVideoIn);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------constructors and initialize--------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CBridgeVideoInCopDecoder::CBridgeVideoInCopDecoder():CBridgePartyVideoIn()
{
	m_videoConfType = eVideoConfTypeCopHD108025fps;//This is the default if it eVideoConfTypeCopHD72050fps we will receive via the initparams
	m_copArtConnectionId = DUMMY_CONNECTION_ID;
	m_copArtPartyId = DUMMY_PARTY_ID;
	m_copDecoderIndex = (0xFFFF);

	VALIDATEMESSAGEMAP;
}
// ------------------------------------------------------------
CBridgeVideoInCopDecoder::~CBridgeVideoInCopDecoder()
{

}
// ------------------------------------------------------------

void CBridgeVideoInCopDecoder::Create(const CBridgePartyCntl*	pBridgePartyCntl, const CRsrcParams* pRsrcParams
								 ,const CBridgePartyMediaParams * pBridgePartyVideoInParams)
{
	CBridgePartyVideoIn::Create(pBridgePartyCntl,pRsrcParams,pBridgePartyVideoInParams);
	PTRACE2(eLevelInfoNormal,"CBridgeVideoInCopDecoder::Create : Name - ",m_pBridgePartyCntl->GetFullName());

	m_copArtConnectionId = ((CBridgePartyVideoParams*)pBridgePartyVideoInParams)->GetCopConnectionId();
	m_copArtPartyId = ((CBridgePartyVideoParams*)pBridgePartyVideoInParams)->GetCopPartyId();
	m_copDecoderIndex = ((CBridgePartyVideoParams*)pBridgePartyVideoInParams)->GetCopResourceIndex();
	m_videoConfType = ((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetVideoConfType();

}

// ------------------------------------------------------------

void CBridgeVideoInCopDecoder::Close()
{
	DispatchEvent(CLOSE_VIDEO_IN,NULL);
}

// ------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------protected functions--------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CBridgeVideoInCopDecoder::SendConnectToRtp()
{
  PTRACE2(eLevelInfoNormal,"CBridgeVideoInCopDecoder::SendConnectToRtp : Name - ",m_pBridgePartyCntl->GetFullName());
	ConnectionID decoderConnectionId, rtpConnectionId;
	decoderConnectionId = rtpConnectionId = DUMMY_CONNECTION_ID;

	DWORD decoderRsrcPartyId, rtpRsrcPartyId;
	decoderRsrcPartyId = rtpRsrcPartyId = DUMMY_PARTY_ID;

	decoderConnectionId = m_pHardwareInterface->GetConnectionId();
	decoderRsrcPartyId = m_pHardwareInterface->GetPartyRsrcId();
	rtpRsrcPartyId = m_copArtPartyId;
	rtpConnectionId = m_copArtConnectionId;

	ON(m_isConnected);
	((CVideoHardwareInterface*)m_pHardwareInterface)->SendConnect(decoderConnectionId, rtpConnectionId, decoderRsrcPartyId, rtpRsrcPartyId);
}

// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyConnectOPENED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgeVideoInCopDecoder::OnVideoBridgePartyConnectOPENED : Name - ",m_pBridgePartyCntl->GetFullName());
	m_state = CONNECTING;
	//SetClosePortAckStatus(STATUS_OK);
	SendConnectToRtp();
}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyConnectCLOSING(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgeVideoInCopDecoder::OnVideoBridgePartyConnectCLOSING : Ignored!!! : Name - ",m_pBridgePartyCntl->GetFullName());

}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyConnectCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgeVideoInCopDecoder::OnVideoBridgePartyConnectCONNECTING : Ignored!!! : Name - ",m_pBridgePartyCntl->GetFullName());

}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseIDLE(CSegment* pParam)
{
	PTRACE2INT(eLevelError,"CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseIDLE: the decoder is already closed Decoder Index - ",GetCopDecoderIndex());

	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)statOK;

	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_IN_CLOSED);

	POBJDELETE(pSeg);
}

// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseSETUP(CSegment* pParam)
{
	PTRACE2INT(eLevelError,"CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseSETUP: Decoder Index - ",GetCopDecoderIndex());
	if(!m_isPortOpened)
	{
		//Port never opened - Disconnecting while setup or because of no ack or bad status in ack
		m_state = IDLE;
		CSegment *pMsg = new CSegment;
		*pMsg << (BYTE)statOK;
		m_pBridgePartyCntl->HandleEvent(pMsg,0,VIDEO_IN_CLOSED);
		POBJDELETE(pMsg);
	}
	else
		OnVideoBridgePartyClose(pParam);
}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseCONNECTED(CSegment* pParam)
{
	PTRACE2INT(eLevelError,"CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseCONNECTED: Decoder Index - ",GetCopDecoderIndex());
	OnVideoBridgePartyClose(pParam);
}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseDISCONNECTING(CSegment* pParam)
{
	PTRACE2INT(eLevelError,"CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseDISCONNECTING: Decoder Index - ",GetCopDecoderIndex());
	OnVideoBridgePartyClose(pParam);
}


// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseCLOSING(CSegment* pParam)
{
	PTRACE2INT(eLevelError,"CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseCLOSING : Ignored!!! Decoder Index - ",GetCopDecoderIndex());
}

// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseCONNECTING(CSegment* pParam)
{
	PTRACE2INT(eLevelError,"CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseCONNECTING: Decoder Index - ",GetCopDecoderIndex());
	OnVideoBridgePartyClose(pParam);
}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseOPENED(CSegment* pParam)
{
	PTRACE2INT(eLevelError,"CBridgeVideoInCopDecoder::OnVideoBridgePartyCloseOPENED: Decoder Index - ",GetCopDecoderIndex());
	OnVideoBridgePartyClose(pParam);
}

// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyClose(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CBridgeVideoInCopDecoder::OnVideoBridgePartyClose : Decoder Index - ",GetCopDecoderIndex());

	m_state = CLOSING;

	m_pImage->SyncLost();
	//UpdateDBLocalVideoSyncState(YES);

	SendCloseDecoder();

}

// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnMplAckCLOSING(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CBridgeVideoInCopDecoder::OnMplAckCLOSING : Decoder Index - ",GetCopDecoderIndex());
	OPCODE	AckOpcode;
	DWORD  ack_seq_num;
	STATUS  status;
	*pParam >> AckOpcode >> ack_seq_num >> status;
	switch(AckOpcode){
	case	TB_MSG_CLOSE_PORT_REQ:
		{
			OnMplClosePortAck(status);
			break;
		}
	default:{
		CProcessBase * process = CProcessBase::GetProcess();
		std::string str = process->GetOpcodeAsString(AckOpcode);
		TRACEINTO << "CBridgeVideoInCopDecoder::OnMplAckCLOSING - ACK_IND Ignored! - Ack Opcode: "<< str.c_str();
		}
	}// end switch
}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnMplAckCONNECTING(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CBridgeVideoInCopDecoder::OnMplAckCONNECTING : Decoder Index - ",GetCopDecoderIndex());
	OPCODE	AckOpcode;
	DWORD  ack_seq_num;
	STATUS  status;
	*pParam >> AckOpcode >> ack_seq_num >> status;
	switch(AckOpcode){
	case	TB_MSG_CONNECT_REQ:{
			OnMplConnectAck(status);
			break;
		}
	default:{
		CProcessBase * process = CProcessBase::GetProcess();
		std::string str = process->GetOpcodeAsString(AckOpcode);
		TRACEINTO << "CBridgeVideoInCopDecoder::OnMplAckCONNECTING - ACK_IND Ignored! - Ack Opcode: "<< str.c_str();
		}
	}// end switch
}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnMplClosePortAck(STATUS status)
{
	PTRACE2INT(eLevelInfoNormal,"CBridgeVideoInCopDecoder::OnMplClosePortAck : Decoder Index - ",GetCopDecoderIndex());
	BYTE	responseStatus = statOK;

	if(status!=STATUS_OK)
	{
		PTRACE(eLevelError, "CBridgeVideoInCopDecoder::OnMplClosePortAck :  Bad Status!!!");

		//Add assert to EMA in case of NACK
		AddFaultAlarm("NACK on Close video decoder",m_pHardwareInterface->GetPartyRsrcId(),status);
		responseStatus = statVideoInOutResourceProblem;		// statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoDecoder VideoEncoder
		//In case we receive nack on close cop decoder we will send kill port.
		PTRACE(eLevelError, "CBridgeVideoInCopDecoder::OnMplClosePortAck :  send kill port!!!");
		SendKillPort();
	}
	SetClosePortAckStatus(responseStatus);

	m_state = IDLE;
	DeleteTimer(DECODER_RECURRENT_INTRA_REQ_TIMEOUT);

	CSegment* pSeg = new CSegment;
	*pSeg  << (BYTE)responseStatus;
	if(responseStatus == statVideoInOutResourceProblem)
		*pSeg  << (BYTE)eMipIn << (BYTE)eMipStatusFail << (BYTE)eMipClose;


	// Inform BridgePartyCntl
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_IN_CLOSED);
	POBJDELETE(pSeg);

}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyDisConnect(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CBridgeVideoInCopDecoder::OnVideoBridgePartyDisConnect : Decoder Index - ",GetCopDecoderIndex());
	m_state = DISCONNECTING;

	m_pImage->SyncLost();
	//UpdateDBLocalVideoSyncState(YES);

	SendDisconnectFromRtp();
}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyDisConnectDISCONNECTING(CSegment* pParam)
{
	// we will resend the disconnect to avoid chain reaction that we will never disconnect the decoder in case there was problem
	PTRACE2(eLevelError,"CBridgeVideoInCopDecoder::OnVideoBridgePartyDisConnectDISCONNECTING : Name - ",m_pBridgePartyCntl->GetFullName());
	OnVideoBridgePartyDisConnect(pParam);
}
// -----------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyDisConnectOPENED(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgeVideoInCopDecoder::OnVideoBridgePartyDisConnectOPENED : Already disconnected! : Name - ",m_pBridgePartyCntl->GetFullName());
	CSegment* pSeg = new CSegment;
	*pSeg  << (BYTE)statOK;
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_IN_DISCONNECTED);
	POBJDELETE(pSeg);
}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyDisConnectCLOSING(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgeVideoInCopDecoder::OnVideoBridgePartyDisConnectCLOSING : Already disconnected! : Name - ",m_pBridgePartyCntl->GetFullName());
	CSegment* pSeg = new CSegment;
	*pSeg  << (BYTE)statOK;
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_IN_DISCONNECTED);
	POBJDELETE(pSeg);
}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyDisConnectCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelError,"CBridgeVideoInCopDecoder::OnVideoBridgePartyDisConnectCONNECTING : Name - ",m_pBridgePartyCntl->GetFullName());
	OnVideoBridgePartyDisConnect(pParam);

}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::SendDisconnectFromRtp()
{
	ConnectionID decoderConnectionId, rtpConnectionId;
	decoderConnectionId = rtpConnectionId = DUMMY_CONNECTION_ID;

	DWORD decoderRsrcPartyId, rtpRsrcPartyId;
	decoderRsrcPartyId = rtpRsrcPartyId = DUMMY_PARTY_ID;

	decoderConnectionId = m_pHardwareInterface->GetConnectionId();
	decoderRsrcPartyId = m_pHardwareInterface->GetPartyRsrcId();
	rtpRsrcPartyId = m_copArtPartyId;
	rtpConnectionId = m_copArtConnectionId;

	OFF(m_isConnected);//KEREN?
	((CVideoHardwareInterface*)m_pHardwareInterface)->SendDisconnect(decoderConnectionId, rtpConnectionId, decoderRsrcPartyId, rtpRsrcPartyId);

}
// ------------------------------------------------------------
void  CBridgeVideoInCopDecoder::OnMplAckDISCONNECTING(CSegment* pParam)
{
	OPCODE	AckOpcode;
	DWORD  ack_seq_num;
	STATUS  status;
	*pParam >> AckOpcode >> ack_seq_num >> status;
	switch(AckOpcode){
	case	TB_MSG_DISCONNECT_REQ:
	{
		OnMplDisconnectAck(status);
		break;
	}
	default:{
		CProcessBase * process = CProcessBase::GetProcess();
		std::string str = process->GetOpcodeAsString(AckOpcode);
		TRACEINTO << "CBridgeVideoInCopDecoder::OnMplAckDISCONNECTING - ACK_IND Ignored! - Ack Opcode: "<< str.c_str() << " - Name: "<< m_pBridgePartyCntl->GetFullName();
			}
	}// end switch
}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnMplDisconnectAck(STATUS status)
{
	PTRACE2INT(eLevelInfoNormal,"CBridgeVideoInCopDecoder::OnMplDisconnectAck : Decoder Index - ",GetCopDecoderIndex());
	BYTE	responseStatus = statOK;

	if(status!=STATUS_OK)
	{
		PTRACE(eLevelError, "CBridgeVideoInCopDecoder::OnMplDisconnectAck :  Bad Status!!!");

		//Add assert to EMA in case of NACK
		AddFaultAlarm("NACK on disconnect video decoder",m_pHardwareInterface->GetPartyRsrcId(),status);
		responseStatus = statVideoInOutResourceProblem;		// statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoDecoder VideoEncoder
	}
	//SetClosePortAckStatus(responseStatus);//keren

	m_state = OPENED;

	CSegment* pSeg = new CSegment;
	*pSeg  << (BYTE)responseStatus;
	if(responseStatus == statVideoInOutResourceProblem)
		*pSeg  << (BYTE)eMipIn << (BYTE)eMipStatusFail << (BYTE)eMipDisconnect;
	// Inform BridgePartyCntl
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_IN_DISCONNECTED);
	POBJDELETE(pSeg);
}
// ------------------------------------------------------------

void  CBridgeVideoInCopDecoder::SaveAndSendUpdatedVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyVideoIn::SaveAndSendUpdatedVideoParams : Name - ",m_pBridgePartyCntl->GetFullName());

	if(!(CPObject::IsValidPObjectPtr(pBridgePartyVideoParams)))
	{
		PASSERTMSG(1, "CBridgePartyVideoIn::SaveAndSendUpdatedVideoParams : Internal Error received invalid params");
		CSegment *pMsg = new CSegment;
		*pMsg << (BYTE)statIllegal;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_IN_UPDATED);
		POBJDELETE(pMsg);
		return;
	}
	DWORD newCopArtConnectionId             = pBridgePartyVideoParams->GetCopConnectionId();
	DWORD newCopArtPartyId                  = pBridgePartyVideoParams->GetCopPartyId();

	if((newCopArtConnectionId != m_copArtConnectionId)||(newCopArtPartyId!=m_copArtPartyId))
	{
		CMedString mstr;
		mstr << "To Decoder Index: " << m_copDecoderIndex << " New ART connectionId  = " << newCopArtConnectionId << ", New ART partyId c= " << newCopArtPartyId;
		PTRACE2(eLevelInfoNormal,"CBridgeVideoInCopDecoder::SaveAndSendUpdatedVideoParams :",mstr.GetString());
		m_copArtConnectionId = newCopArtConnectionId ;
		m_copArtPartyId = newCopArtPartyId;
	}
	CBridgePartyVideoIn::SaveAndSendUpdatedVideoParams(pBridgePartyVideoParams);

}

// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyUpdateVideoParamCLOSING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgeVideoInCopDecoder::OnVideoBridgePartyUpdateVideoParamCLOSING : IGNORED!!! Name - ",m_pBridgePartyCntl->GetFullName());
}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyUpdateVideoParamOPENED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgeVideoInCopDecoder::OnVideoBridgePartyUpdateVideoParamOPENED:Name - ",m_pBridgePartyCntl->GetFullName());
	CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoParamsCONNECTED(pParam);
}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnVideoBridgePartyUpdateVideoParamCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgeVideoInCopDecoder::OnVideoBridgePartyUpdateVideoParamCONNECTING:Name - ",m_pBridgePartyCntl->GetFullName());
	CBridgePartyVideoParams* pBridgePartyVideoParams = new CBridgePartyVideoParams;
	pBridgePartyVideoParams->DeSerialize(NATIVE, *pParam);
	if(!(CPObject::IsValidPObjectPtr(pBridgePartyVideoParams)))
	{
		PASSERTMSG(1, "CBridgeVideoInCopDecoder::OnVideoBridgePartyUpdateVideoParamCONNECTING : Internal Error receive invalid params");
		CSegment *pMsg = new CSegment;
		*pMsg << (BYTE)statIllegal;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_IN_UPDATED);
		POBJDELETE(pMsg);
		POBJDELETE(pBridgePartyVideoParams);
		return;
	}

	m_pWaitingForUpdateParams = new CBridgePartyVideoParams;
	*m_pWaitingForUpdateParams = *pBridgePartyVideoParams;
	POBJDELETE(pBridgePartyVideoParams);

}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnMplDecoderSyncOPENED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgeVideoInCopDecoder::OnMplDecoderSyncOPENED ignored :Name - ",m_pBridgePartyCntl->GetFullName());

}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnMplDecoderSyncCLOSING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgeVideoInCopDecoder::OnMplDecoderSyncCLOSING ignored :Name - ",m_pBridgePartyCntl->GetFullName());

}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::OnMplDecoderSyncCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgeVideoInCopDecoder::OnMplDecoderSyncCONNECTING ignored :Name - ",m_pBridgePartyCntl->GetFullName());

}
// ------------------------------------------------------------
void CBridgeVideoInCopDecoder::SendKillPort()
{
	((CVideoHardwareInterface*)m_pHardwareInterface)->SendKillPort();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BYTE CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio()
{
	BYTE isDetectedVideoMatchCurrentResRatio = NO;

	DWORD  decoderDetectedModeWidth = DEFAULT_DECODER_DETECTED_MODE_WIDTH;
	DWORD  decoderDetectedModeHeight = DEFAULT_DECODER_DETECTED_MODE_HEIGHT;
	DWORD  detectedResolutionRatio = RESOLUTION_RATIO_0;
	DWORD  currentParamsResolutionRatio = RESOLUTION_RATIO_0;

	decoderDetectedModeWidth = m_pImage->GetDecoderDetectedModeWidth();
	decoderDetectedModeHeight = m_pImage->GetDecoderDetectedModeHeight();
	if(DEFAULT_DECODER_DETECTED_MODE_WIDTH == decoderDetectedModeWidth || DEFAULT_DECODER_DETECTED_MODE_HEIGHT == decoderDetectedModeHeight)
	{
		PTRACE2(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio = NO  DEFAULT VALUES :Name - ",m_pBridgePartyCntl->GetFullName());
		return isDetectedVideoMatchCurrentResRatio;
	}
	switch(m_videoAlg)
	{
		case H264: ///to add the RTV HP cases
		{
			DWORD detectedFS = (decoderDetectedModeWidth*decoderDetectedModeHeight)/256;
			detectedFS = GetMaxFsAsDevision(detectedFS);
			detectedResolutionRatio = ((CVideoHardwareInterface*)m_pHardwareInterface)-> TranslateToVideoResolutionRatio(H264,eVideoResolutionDummy, detectedFS, m_MBPS, m_videoConfType);
			currentParamsResolutionRatio=((CVideoHardwareInterface*)m_pHardwareInterface)->TranslateToVideoResolutionRatio(H264,eVideoResolutionDummy, m_FS, m_MBPS, m_videoConfType);
			CMedString mstr;
			mstr << "H264 Decoder: "<< m_pBridgePartyCntl->GetFullName() << " detectedFS "<< detectedFS<<" detectedResolutionRatio: " << detectedResolutionRatio << " currentParamsResolutionRatio  = " << currentParamsResolutionRatio;
			PTRACE2(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio :",mstr.GetString());
			if(detectedResolutionRatio==currentParamsResolutionRatio)
			{
				isDetectedVideoMatchCurrentResRatio =YES;
				PTRACE(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio YES:");
			}
		}
		break;
		case H263:
		{
			//in case of H263 we can receive only: QCIF CIF/SIF 4CIF/4SIF
			CMedString mstr;
			mstr << "H263 Decoder: "<< m_pBridgePartyCntl->GetFullName() << " decoderDetectedModeWidth: " << decoderDetectedModeWidth << " decoderDetectedModeHeight  = " << decoderDetectedModeHeight << " current resolution " <<m_videoResolution;
			PTRACE2(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio:",mstr.GetString());
			switch (m_videoResolution)
			{
				case eVideoResolutionQCIF:
				{
					PTRACE(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio H263 QCIF:");
					if((ResoltionWidthAndHeight[eVideoResolutionQCIF].resWidth==decoderDetectedModeWidth)&&(ResoltionWidthAndHeight[eVideoResolutionQCIF].resHeight==decoderDetectedModeHeight))
					{
						PTRACE(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio YES H263 QCIF:");
						isDetectedVideoMatchCurrentResRatio = YES;
					}

				}
				break;
				case eVideoResolutionCIF:
				case eVideoResolutionSIF:
				{
					PTRACE(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio H263 CIF/SIF:");
					if(((ResoltionWidthAndHeight[eVideoResolutionCIF].resWidth==decoderDetectedModeWidth)&&(ResoltionWidthAndHeight[eVideoResolutionCIF].resHeight==decoderDetectedModeHeight))||
					   ((ResoltionWidthAndHeight[eVideoResolutionSIF].resWidth==decoderDetectedModeWidth)&&(ResoltionWidthAndHeight[eVideoResolutionSIF].resHeight==decoderDetectedModeHeight)))
					{
							PTRACE(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio YES H263 CIF/SIF:");
							isDetectedVideoMatchCurrentResRatio = YES;
					}
				}
				break;
				case eVideoResolution4SIF:
				case eVideoResolution4CIF:
				{
					if(m_videoResolution == eVideoResolutionQCIF)
					{
						PTRACE(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio H263 4CIF/4SIF:");
						if(((ResoltionWidthAndHeight[eVideoResolution4CIF].resWidth==decoderDetectedModeWidth)&&(ResoltionWidthAndHeight[eVideoResolution4CIF].resHeight==decoderDetectedModeHeight))||
						   ((ResoltionWidthAndHeight[eVideoResolution4SIF].resWidth==decoderDetectedModeWidth)&&(ResoltionWidthAndHeight[eVideoResolution4SIF].resHeight==decoderDetectedModeHeight)))
						{
							PTRACE(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio YES H263 4CIF/4SIF:");
							isDetectedVideoMatchCurrentResRatio = YES;
						}
					}
				}
				break;
				default:
				{
					PTRACE2INT(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio unsupported H263 resolution:",m_videoResolution);

				}
			}
		}
		break;
		case H261:
		{

			//in case of H261 we can receive only: QCIF CIF
			CMedString mstr;
			mstr << "H261 Decoder: "<< m_pBridgePartyCntl->GetFullName() << " decoderDetectedModeWidth: " << decoderDetectedModeWidth << " decoderDetectedModeHeight  = " << decoderDetectedModeHeight << " current resolution " <<m_videoResolution;
			PTRACE2(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio:",mstr.GetString());
			switch (m_videoResolution)
			{
				case eVideoResolutionQCIF:
				{
					PTRACE(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio H261 QCIF:");
					if((ResoltionWidthAndHeight[eVideoResolutionQCIF].resWidth==decoderDetectedModeWidth)&&(ResoltionWidthAndHeight[eVideoResolutionQCIF].resHeight==decoderDetectedModeHeight))
					{
						PTRACE(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio YES H263 QCIF:");
						isDetectedVideoMatchCurrentResRatio = YES;
					}
				}
				break;
				case eVideoResolutionCIF:
				{
					PTRACE(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio H261 CIF:");
					if((ResoltionWidthAndHeight[eVideoResolutionCIF].resWidth==decoderDetectedModeWidth)&&(ResoltionWidthAndHeight[eVideoResolutionCIF].resHeight==decoderDetectedModeHeight))
					{
						PTRACE(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio YES H261 CIF:");
						isDetectedVideoMatchCurrentResRatio = YES;
					}
				}
				break;
				default:
				{
					PTRACE2INT(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio unsupported H261 resolution:",m_videoResolution);
				}

			}

		}
		break;
		default:
		{
			PTRACE2INT(eLevelInfoNormal,"CBridgeVideoInCopDecoder::IsDetectedVideoMatchCurrentResRatio unsupported video algorithm:",m_videoAlg);

		}
	}
	return isDetectedVideoMatchCurrentResRatio;
}
