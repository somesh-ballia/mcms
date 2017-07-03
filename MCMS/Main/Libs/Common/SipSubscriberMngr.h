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
// FILE: SipProxySubscriber.H                                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                            |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// Ori | 17/12/09      |                                                      |
//+========================================================================+


#ifndef SIPSUBSCRIBER_H_
#define SIPSUBSCRIBER_H_

#include "StateMachine.h"
#include "DefinesIpService.h"
#include "SipUtils.h"

//#include "Trace.h"

// Subscriber states
//const WORD  IDLE         = 0;
const WORD  SUBSCRIBING  = 1;
const WORD  SUBSCRIBED   = 2;
const WORD  UNSUBSCRIBNG = 3;

// Subscriber Events
const WORD   SIP_START_SUBSCRIBE  = 1;
const WORD	 SUBSCRIBE_OK		  = 2;
const WORD	 SUBSCRIBE_FAILED     = 3;
const WORD   RECEIVEDNOTIFY		  = 4;
const WORD	 SERVICE_OK           = 5;
const WORD	 SERVICE_FAILED       = 6;



class CSipSubscriberMngr : public CStateMachine
{
CLASS_TYPE_1(CSipSubscriberMngr,CStateMachine)
// public functions
public:
	CSipSubscriberMngr();
//	CSipSubscriberMngr(DWORD Id,WORD src_unit_id);
	~CSipSubscriberMngr();

//	BuildSubscribeReq();
//	HandleSubscribeResponse();
	virtual void create(COsQueue* pSipMngrRcvMbx, DWORD Id,WORD src_unit_id,WORD TransportType,mcTransportAddress proxyAddress,char* LocalUri);

	void  SendMsgToCS(DWORD cs_Id, OPCODE opcode, CSegment* pSeg);
	void HandleEvent (CSegment *pMsg,DWORD msgLen,OPCODE opCode);

	virtual void OnStartSubscribeIDLE(CSegment* pParam);
	virtual void StartSubscribe(char* UserName,char* HostName);

	void HandleSubscribeResponse(CSegment* pParam,OPCODE opcode);
	virtual void OnSubscribeOKSubscribing(CSegment* pParam);
	virtual void OnSubscribeFailedSubscribing(CSegment* pParam);
	virtual void HandleNotifyInd(CSegment* pParam);
	virtual void OnNotifyReceiveSubscribed(CSegment* pParam);
	void HandleServiceResponse(CSegment* pParam,OPCODE opcode);


	PDECLAR_MESSAGE_MAP

protected:

//	WORD	m_timerSubscribeRefresh;
//	CSipSubscribeStruct m_LocalSubscribe;

	DWORD m_id;
	WORD m_src_unit_id;
	mcTransportAddress m_proxyAddress;
	WORD  m_TransportType;
	char* m_LocalUriIP;
	WORD m_NumOfSubscribeRetries;
	COsQueue* m_pSipMngrRcvMbx;

};


#endif /* SIPSUBSCRIBER_H_ */
