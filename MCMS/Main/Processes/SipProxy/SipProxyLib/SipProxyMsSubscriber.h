//+========================================================================+
//                                                                         |
//            Copyright 2005 Poplycom Technologies Ltd.                    |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE: SipProxyMsSubscriber.h                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// Inga | 21/12/09      |                                                  |
//+========================================================================+

#ifndef SIPPROXYMSSUBSCRIBER_H_
#define SIPPROXYMSSUBSCRIBER_H_

#include "SipSubscriberMngr.h"
#include "ICEApiDefinitions.h"

// Subscriber Events

const WORD   END_INIT_ICE_IND  = 12;





class CSipProxyMsSubscriber : public CSipSubscriberMngr
{
CLASS_TYPE_1(CSipProxyMsSubscriber,CSipSubscriberMngr)
// public functions
public:

	CSipProxyMsSubscriber();
	~CSipProxyMsSubscriber();

	void StartSubscribe(char* UserName,char* HostName, DWORD serviceId);
	void OnStartSubscribeIDLE(CSegment* pParam);
	void OnSubscribeTimer(CSegment* pParam);
	void BuildSubscribeMsg(DWORD serviceId);


	void OnSubscribeOKSubscribing(CSegment* pParam);
	void OnSubscribeFailedSubscribing(CSegment* pParam);
	void OnNotifyReceiveSubscribed(CSegment* pParam);
	void OnNotifyTimout(CSegment* pParam);
	void SendServiceReq();

	void OnServiceOKSubscribed(CSegment* pParam);
	void OnServiceFailedSubscribed(CSegment* pParam);

	void UpdateCardsMngr(DWORD id,const char* pUserName,const char* pPassword, const char* pRelayHostName,WORD tcpPortVal,WORD udpPortVal);
	void EndIceInitInd(CSegment* pParam);
	void CMIceInitTimeout(CSegment* pParam);

	void HandleCSICEInitResponse(CSegment* pParam,OPCODE opcode);
	void SetIceInitActiveAlarm(iceServersStatus Status,int i);
    void TerminateICERegistration();
    BYTE GetIsNeedToReSubscribe(){return m_IsNeedToReSubscribe;}
    DWORD ParseHeaderMaxVideoRateAllowed(const char* pRateString);


protected:

	char* m_MrasUri;
	BYTE  m_WaitForNotifyInd;
	char* m_DomainName;
	char* m_UserName;
	BYTE m_IsNeedToReSubscribe;
	DWORD m_serviceId;
	BYTE m_IsEnableBWPolicyCheck;
	int m_UcMaxVideoRateAllowed;




	PDECLAR_MESSAGE_MAP
};


#endif /* SIPPROXYMSSUBSCRIBER_H_ */
