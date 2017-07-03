//+========================================================================+
//                    SipProxyMsPresence.h 		                     	   |
//            Copyright 2010 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SipProxyMsPresence.h                                  	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Shmuel                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#ifndef SIPPROXYMSPRESENCE_H_
#define SIPPROXYMSPRESENCE_H_

#include "SipSubscriberMngr.h"
#include "SipProxyTaskApi.h"

// CSipProxyMsPresence events:
const WORD  sPRESENCE_SUBSCRIBE_CONTACTS	 = 10;
const WORD  sPRESENCE_SUBSCRIBE_ALC			 = 11;
const WORD  sPRESENCE_SERVICE_SET_URI		 = 12;
const WORD  sPRESENCE_SERVICE_SET_ACE		 = 13;
const WORD  sPRESENCE_SUBSCRIBE_SELF		 = 14;
const WORD  sPRESENCE_SERVICE_SET_RICH		 = 15;
const WORD 	sPRESENCE_SERVICE_SET_MEMBER	 = 16;
const WORD 	sPRESENCE_SERVICE_UNSET_RICH	 = 17;
const WORD 	sPRESENCE_SERVICE_ONLINE_RICH	 = 18;
const WORD 	sPRESENCE_SERVICE_BUSY_RICH	 	 = 19;

const WORD  PRESENCE_RESPONSE_TOUT			 = 100;
#define	PresenceResponseTimeOut				40*SECOND

#define MaxLengthOfAclDeltaNum				16
#define MaxLengthOfRichPresenceVerList		16

typedef enum {
	ePresenceCompleted = 0,
	ePresenceInProgress,
	ePresenceFailed,
	ePresenceUnknown
} EPresenceStatus;

class CSipProxyMsPresence : public CSipSubscriberMngr
{
CLASS_TYPE_1(CSipProxyMsPresence, CSipSubscriberMngr)
public:
	CSipProxyMsPresence();
	virtual ~CSipProxyMsPresence();
	void StartPresence(const char* pConfName, const char* pDomainName, DWORD serviceId, BYTE isLyncServerVersion);
	void EndPresence(const char* pConfName, const char* pDomainName, DWORD serviceId, BYTE isLyncServerVersion);
	void ChangePresenceState(BYTE presenceState);
	void SendSelfSubscribeAndSaveLastPresenceState(BYTE presenceState);
	void OnStartPresenceIdle(CSegment* pParam);
	void OnEndPresence(CSegment* pParam);
	void OnSubscribeOKSubscirbeContacts(CSegment* pParam);
	void OnSubscribeOKSubscirbeAlc(CSegment* pParam);
	void OnSubscribeOKSubscirbeSelf(CSegment* pParam);
	void OnServiceOKServiceSetUri(CSegment* pParam);
	void OnServiceOKServiceSetAce(CSegment* pParam);
	void OnServiceOKServiceSetRich(CSegment* pParam);
	void OnServiceOKServiceUnsetRich(CSegment* pParam);
	void OnServiceOKServiceSetMember(CSegment* pParam);
	void OnServiceOKOnlineServiceRich(CSegment* pParam);
	void OnServiceOKBusyServiceRich(CSegment* pParam);
	void OnSubscribeFailedSubscirbeContacts(CSegment* pParam);
	void OnSubscribeFailedSubscirbeAlc(CSegment* pParam);
	void OnSubscribeFailedSubscirbeSelf(CSegment* pParam);
	void OnServiceFailedServiceSetUri(CSegment* pParam);
	void OnServiceFailedServiceSetAce(CSegment* pParam);
	void OnServiceFailedServiceSetRich(CSegment* pParam);
	void OnServiceFailedServiceSetMember(CSegment* pParam);
	void OnServiceFailedServiceUnsetRich(CSegment* pParam);
	void OnServiceFailedOnlineServiceRich(CSegment* pParam);
	void OnServiceFailedBusyServiceRich(CSegment* pParam);
	void FillGerenalSubscribeParams(CSipSubscribeStruct &SipSubscribe);
	void SendSubscribe(CSipSubscribeStruct &SipSubscribe);
	void SendSubscribeContacts();
	void SendSubscribeACL();
	void SendSelfSubscribe();
	void FillGerenalServiceParams(CSipServiceStruct &SipService);
	void SendService(CSipServiceStruct &SipService);
	void SendServiceSetUri();
	void SendServiceSetAce();
	void SendServiceSetRichPresence();
	void SendServiceUnsetRichPresence();
	void SendServiceSetMembers();
	BYTE HandleSubscribeOk(CSegment* pParam, WORD expectedSubOpcode);
	BYTE HandleServiceOk(CSegment* pParam, WORD expectedSubOpcode);
	BYTE ExtractAclDeltaNumber(CSipHeaderList * pHeaders);
	EPresenceStatus GetPresenceStatus() {return m_ePresenceStatus;}
	void SetPresenceStatus(EPresenceStatus ePresenceStatus) {m_ePresenceStatus = ePresenceStatus;}
	void PresenceFlowFailed();
	void OnPresenceResponseTimeOut(CSegment* pParam);
	BYTE ExtractPresenceVersionsList(CSipHeaderList * pHeaders);
	BYTE GetIsNeedToReSubscribe(){return m_IsNeedToReSubscribe;}

protected:

	char* m_confName;
	char* m_domainName;
	char* m_confUri;
	char* m_aclDeltaNum;
	char* m_richPresenceVersionsList;
	EPresenceStatus m_ePresenceStatus; // Todo: to be used for monitoring (by sending the status to SipProxyManager)
	DWORD m_serviceId;
	BYTE m_IsNeedToReSubscribe;
	BYTE m_IsLyncServerVersion;
	BYTE m_lastPresenceState;

	PDECLAR_MESSAGE_MAP
};

#endif /* SIPPROXYMSPRESENCE_H_ */
