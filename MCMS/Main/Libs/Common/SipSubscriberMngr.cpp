//+========================================================================+
//                                                                      |
//            Copyright 2005 Poplycom Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE: SipSubscriber.cpp                                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                            |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// Inga | 17/12/09      |                                                      |
//+========================================================================+


#include "SipSubscriberMngr.h"
#include "MplMcmsProtocol.h"
#include  "MplMcmsProtocolTracer.h"
#include "ApiStatuses.h"


// start message map -------------------------------------------
PBEGIN_MESSAGE_MAP(CSipSubscriberMngr)

	ONEVENT(SUBSCRIBE_OK,				SUBSCRIBING,		CSipSubscriberMngr::OnSubscribeOKSubscribing)
	ONEVENT(SUBSCRIBE_FAILED,			SUBSCRIBING,		CSipSubscriberMngr::OnSubscribeFailedSubscribing)
//	ONEVENT(SIP_START_SUBSCRIBE ,					IDLE,				CSipSubscriberMngr::OnStartSubscribeIDLE)
//	ONEVENT(SIP_CS_PROXY_SUBSCRIBE_RESPONSE_IND,	SUBSCRIBING,	    CSipSubscriberMngr::HandleSubscribeResponse)
	ONEVENT(RECEIVEDNOTIFY,				    SUBSCRIBED,			    CSipSubscriberMngr::OnNotifyReceiveSubscribed)


PEND_MESSAGE_MAP(CSipSubscriberMngr,CStateMachine);

// end   message map -------------------------------------------


CSipSubscriberMngr::CSipSubscriberMngr()
{
	m_src_unit_id = 0;
	m_id = 0;
	memset(&m_proxyAddress,0,sizeof(mcTransportAddress));
	m_TransportType = eUnknownTransportType;
//	m_timerSubscribeRefresh = 0;
	m_state = IDLE;
	m_LocalUriIP = NULL;
	m_NumOfSubscribeRetries = 0;
	m_pSipMngrRcvMbx = NULL;
}
////////////////////////////////////////////////////////////////////////////
CSipSubscriberMngr::~CSipSubscriberMngr()
{
	PDELETEA(m_LocalUriIP); 
}
////////////////////////////////////////////////////////////////////////////
void CSipSubscriberMngr::create(COsQueue* pSipMngrRcvMbx, DWORD Id,WORD src_unit_id,WORD TransportType,mcTransportAddress proxyAddress,char* LocalUri)
{
	PTRACE(eLevelInfoNormal,"CSipSubscriberMngr::create ");
	m_pSipMngrRcvMbx = pSipMngrRcvMbx;
	m_src_unit_id = src_unit_id;
	m_id = Id;
	m_TransportType = TransportType;
	memcpy(&m_proxyAddress, &proxyAddress, sizeof(mcTransportAddress));

	if(LocalUri)
	{
		m_LocalUriIP = new char[MaxAddressListSize];
		memset(m_LocalUriIP, '\0', MaxAddressListSize);
		strncpy(m_LocalUriIP, LocalUri, MaxAddressListSize);
	}


}
////////////////////////////////////////////////////////////////////////////
void CSipSubscriberMngr::StartSubscribe(char* UserName,char* HostName)
{
	PTRACE(eLevelInfoNormal,"CSipSubscriberMngr::StartSubscribe ");
	m_state = SUBSCRIBING;


}
////////////////////////////////////////////////////////////////////////////
void CSipSubscriberMngr::OnStartSubscribeIDLE(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipSubscriberMngr::OnStartSubscribeIDLE ");

}
////////////////////////////////////////////////////////////////////////////
void CSipSubscriberMngr::HandleEvent (CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	  DispatchEvent(opCode,pMsg);
}

////////////////////////////////////////////////////////////////////////////
void  CSipSubscriberMngr::SendMsgToCS(DWORD cs_Id, OPCODE opcode, CSegment* pSeg)
{
	CMplMcmsProtocol *pMplMcmsProtocol = NULL;

	pMplMcmsProtocol = new CMplMcmsProtocol;

	if(!pMplMcmsProtocol)
	{
		PASSERT(101);
		return;
	}
	pMplMcmsProtocol->AddCommonHeader(opcode);
	pMplMcmsProtocol->AddMessageDescriptionHeader();
	pMplMcmsProtocol->AddPortDescriptionHeader(0, m_id);
	pMplMcmsProtocol->AddCSHeader(cs_Id,0, m_src_unit_id);
	if(pSeg)
	{
		DWORD nMsgLen = pSeg->GetWrtOffset() - pSeg->GetRdOffset();
		BYTE* pMessage = new BYTE[nMsgLen];
		pSeg->Get(pMessage,nMsgLen);
		pMplMcmsProtocol->AddData(nMsgLen,(const char*)pMessage);
		PDELETEA(pMessage);
	}
	pMplMcmsProtocol->AddPayload_len(CS_API_TYPE);
	CMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("CSipSubscriberMngr::SendMsgToCS ",CS_API_TYPE);
	pMplMcmsProtocol->SendMsgToCSApiCommandDispatcher();


	PDELETE(pMplMcmsProtocol);
}
////////////////////////////////////////////////////////////////////////////
void CSipSubscriberMngr::HandleSubscribeResponse(CSegment* pParam,OPCODE opcode)
{
	PTRACE(eLevelInfoNormal,"CSipSubscriberMngr::HandleSubscribeResponse ");

	DispatchEvent(opcode,pParam);
}
////////////////////////////////////////////////////////////////////////////
void CSipSubscriberMngr::OnSubscribeOKSubscribing(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipSubscriberMngr::OnSubscribeOKRegistered ");

	m_state = SUBSCRIBED;

}
////////////////////////////////////////////////////////////////////////////
void CSipSubscriberMngr::OnSubscribeFailedSubscribing(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipSubscriberMngr::OnSubscribeFailedRegistered ");

}
////////////////////////////////////////////////////////////////////////////
void CSipSubscriberMngr::HandleNotifyInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipSubscriberMngr::HandleNotifyInd  ");
	DispatchEvent(RECEIVEDNOTIFY,pParam);

}
////////////////////////////////////////////////////////////////////////////
void CSipSubscriberMngr::OnNotifyReceiveSubscribed(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipSubscriberMngr::HandleNotifyInd  ");

}
////////////////////////////////////////////////////////////////////////////
void CSipSubscriberMngr::HandleServiceResponse(CSegment* pParam,OPCODE opcode)
{
	PTRACE(eLevelInfoNormal,"CSipSubscriberMngr::HandleServiceResponse ");

	DispatchEvent(opcode,pParam);
}
