//+========================================================================+
//                    SipProxyMsPresence.cpp	                     	   |
//            Copyright 2010 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SipProxyMsPresence.cpp                                  	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Shmuel                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+


#include "SipProxyMsPresence.h"
#include "OpcodesMcmsInternal.h"
#include "IpCsOpcodes.h"

extern 	void IncreasePresentedConfNumber();
extern WORD GetPresentedConfNumber();

// start message map -------------------------------------------------------
PBEGIN_MESSAGE_MAP(CSipProxyMsPresence)

	ONEVENT(SIP_START_PRESENCE,					IDLE,								CSipProxyMsPresence::OnStartPresenceIdle)
	ONEVENT(SUBSCRIBE_OK,						sPRESENCE_SUBSCRIBE_CONTACTS,		CSipProxyMsPresence::OnSubscribeOKSubscirbeContacts)
	ONEVENT(SUBSCRIBE_OK,						sPRESENCE_SUBSCRIBE_ALC,			CSipProxyMsPresence::OnSubscribeOKSubscirbeAlc)
	ONEVENT(SUBSCRIBE_OK,						sPRESENCE_SUBSCRIBE_SELF,			CSipProxyMsPresence::OnSubscribeOKSubscirbeSelf)
	ONEVENT(SERVICE_OK,							sPRESENCE_SERVICE_SET_URI,			CSipProxyMsPresence::OnServiceOKServiceSetUri)
	ONEVENT(SERVICE_OK,							sPRESENCE_SERVICE_SET_ACE,			CSipProxyMsPresence::OnServiceOKServiceSetAce)
	ONEVENT(SERVICE_OK,							sPRESENCE_SERVICE_SET_RICH,			CSipProxyMsPresence::OnServiceOKServiceSetRich)
	ONEVENT(SERVICE_OK,							sPRESENCE_SERVICE_SET_MEMBER,		CSipProxyMsPresence::OnServiceOKServiceSetMember)
	ONEVENT(SIP_END_PRESENCE,					ANYCASE,							CSipProxyMsPresence::OnEndPresence)
	ONEVENT(SERVICE_OK,							sPRESENCE_SERVICE_UNSET_RICH,		CSipProxyMsPresence::OnServiceOKServiceUnsetRich)
	ONEVENT(SERVICE_OK,							sPRESENCE_SERVICE_ONLINE_RICH,		CSipProxyMsPresence::OnServiceOKOnlineServiceRich)
	ONEVENT(SERVICE_OK,							sPRESENCE_SERVICE_BUSY_RICH,		CSipProxyMsPresence::OnServiceOKBusyServiceRich)

	// error handling
	ONEVENT(SUBSCRIBE_FAILED,					sPRESENCE_SUBSCRIBE_CONTACTS,		CSipProxyMsPresence::OnSubscribeFailedSubscirbeContacts)
	ONEVENT(SUBSCRIBE_FAILED,					sPRESENCE_SUBSCRIBE_ALC,			CSipProxyMsPresence::OnSubscribeFailedSubscirbeAlc)
	ONEVENT(SUBSCRIBE_FAILED,					sPRESENCE_SUBSCRIBE_SELF,			CSipProxyMsPresence::OnSubscribeFailedSubscirbeSelf)
	ONEVENT(SERVICE_FAILED,						sPRESENCE_SERVICE_SET_URI,			CSipProxyMsPresence::OnServiceFailedServiceSetUri)
	ONEVENT(SERVICE_FAILED,						sPRESENCE_SERVICE_SET_ACE,			CSipProxyMsPresence::OnServiceFailedServiceSetAce)
	ONEVENT(SERVICE_FAILED,						sPRESENCE_SERVICE_SET_RICH,			CSipProxyMsPresence::OnServiceFailedServiceSetRich)
	ONEVENT(SERVICE_FAILED,						sPRESENCE_SERVICE_SET_MEMBER,		CSipProxyMsPresence::OnServiceFailedServiceSetMember)
	ONEVENT(SERVICE_FAILED,						sPRESENCE_SERVICE_UNSET_RICH,		CSipProxyMsPresence::OnServiceFailedServiceUnsetRich)
	ONEVENT(PRESENCE_RESPONSE_TOUT,				ANYCASE,							CSipProxyMsPresence::OnPresenceResponseTimeOut)
	ONEVENT(SERVICE_FAILED,						sPRESENCE_SERVICE_ONLINE_RICH,		CSipProxyMsPresence::OnServiceFailedOnlineServiceRich)
	ONEVENT(SERVICE_FAILED,						sPRESENCE_SERVICE_BUSY_RICH,		CSipProxyMsPresence::OnServiceFailedBusyServiceRich)

PEND_MESSAGE_MAP(CSipProxyMsPresence,CSipSubscriberMngr);

// end message map -------------------------------------------------------

////////////////////////////////////////////////////////////////////////////
CSipProxyMsPresence::CSipProxyMsPresence()
{
	m_confName 		= NULL;
	m_domainName 	= NULL;
	m_confUri 		= NULL;
	m_aclDeltaNum 	= NULL;
	m_richPresenceVersionsList 	= NULL;
	m_ePresenceStatus = ePresenceUnknown;
	m_IsNeedToReSubscribe = TRUE;
	m_lastPresenceState = SIP_PRESENCE_OFFLINE;
}
////////////////////////////////////////////////////////////////////////////
CSipProxyMsPresence::~CSipProxyMsPresence()
{
	PDELETEA(m_confName);
	PDELETEA(m_domainName);
	PDELETEA(m_confUri);
	PDELETEA(m_aclDeltaNum);
	PDELETEA(m_richPresenceVersionsList);
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::StartPresence(const char* pConfName, const char* pDomainName, DWORD serviceId, BYTE isLyncServerVersion)
{
	CSegment* seg = new CSegment;

	*seg << pConfName;
	*seg << pDomainName;
	*seg << serviceId;
	*seg << isLyncServerVersion;

	DispatchEvent(SIP_START_PRESENCE, seg);

	PDELETE(seg);
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::EndPresence(const char* pConfName, const char* pDomainName, DWORD serviceId, BYTE isLyncServerVersion)
{
	CSegment* seg = new CSegment;

	*seg << pConfName;
	*seg << pDomainName;
	*seg << serviceId;
	*seg << isLyncServerVersion;

	DispatchEvent(SIP_END_PRESENCE, seg);

	PDELETE(seg);
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::ChangePresenceState(BYTE presenceState)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsPresence::ChangePresenceState");
	CSipServiceStruct SipService;
	if(presenceState == SIP_PRESENCE_ONLINE) // online
	{
		m_state = sPRESENCE_SERVICE_ONLINE_RICH;
		m_lastPresenceState = SIP_PRESENCE_ONLINE;
		SipService.SetSubOpcode(ServiceSetPresenceOnlineRich);
	}
	else if (presenceState == SIP_PRESENCE_BUSY) //busy
	{
		m_state = sPRESENCE_SERVICE_BUSY_RICH;
		m_lastPresenceState = SIP_PRESENCE_BUSY;
		SipService.SetSubOpcode(ServiceSetPresenceBusyRich);
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CSipProxyMsPresence::ChangePresenceState- incorrect presence state");
		return;
	}

	SipService.SetHeaderField(kContentType, "application/msrtc-category-publish+xml");
	SipService.SetHeaderField(kRichPresence, m_richPresenceVersionsList);
	FillGerenalServiceParams(SipService);
	SendService(SipService);
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnStartPresenceIdle(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsPresence::OnStartPresenceIDLE");

	m_ePresenceStatus = ePresenceInProgress;

	ALLOCBUFFER(confName, H243_NAME_LEN);
	ALLOCBUFFER(domainName, H243_NAME_LEN);
	DWORD serviceId = 0;

	*pParam >> confName;
	*pParam >> domainName;
	*pParam >> serviceId;
	*pParam >> m_IsLyncServerVersion;

	PDELETEA(m_confName);
	m_confName = new char[MaxAddressListSize];
	memset(m_confName, 0, MaxAddressListSize);
	strncpy(m_confName, confName, H243_NAME_LEN);

	PDELETEA(m_domainName);
	m_domainName = new char[MaxAddressListSize];
	memset(m_domainName, 0, MaxAddressListSize);
	strncpy(m_domainName, domainName, H243_NAME_LEN);

	PDELETEA(m_confUri);
	m_confUri = new char[MaxAddressListSize];
	strncpy(m_confUri, m_confName, H243_NAME_LEN);
	m_confUri[H243_NAME_LEN] = '\0';
	strncat(m_confUri, "@", 1);
	strncat(m_confUri, m_domainName, H243_NAME_LEN);

	m_serviceId = serviceId;

	DEALLOCBUFFER(confName);
	DEALLOCBUFFER(domainName);

	//SendSubscribeContacts();
	if(m_IsLyncServerVersion == TRUE)
		SendSelfSubscribe();
	else
		SendSubscribeContacts();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnEndPresence(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsPresence::OnEndPresence");

	m_ePresenceStatus = ePresenceInProgress;

	ALLOCBUFFER(confName, H243_NAME_LEN);
	ALLOCBUFFER(domainName, H243_NAME_LEN);
	DWORD serviceId = 0;

	*pParam >> confName;
	*pParam >> domainName;
	*pParam >> serviceId;
	*pParam >> m_IsLyncServerVersion;

	PDELETEA(m_confName);
	m_confName = new char[MaxAddressListSize];
	memset(m_confName, 0, MaxAddressListSize);
	strncpy(m_confName, confName, H243_NAME_LEN);

	PDELETEA(m_domainName);
	m_domainName = new char[MaxAddressListSize];
	memset(m_domainName, 0, MaxAddressListSize);
	strncpy(m_domainName, domainName, H243_NAME_LEN);

	PDELETEA(m_confUri);
	m_confUri = new char[MaxAddressListSize];
	strncpy(m_confUri, m_confName, H243_NAME_LEN);
	m_confUri[H243_NAME_LEN] = '\0';
	strncat(m_confUri, "@", 1);
	strncat(m_confUri, m_domainName, H243_NAME_LEN);

	m_serviceId = serviceId;

	DEALLOCBUFFER(confName);
	DEALLOCBUFFER(domainName);

	if(m_IsLyncServerVersion == TRUE)
		SendServiceUnsetRichPresence();

}

////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnSubscribeOKSubscirbeContacts(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsPresence::OnSubscribeOKSubscirbeContacts");
	DeleteTimer(PRESENCE_RESPONSE_TOUT);
	if (HandleSubscribeOk(pParam, SubscribeOCSPresenceRoamingContacts))
		SendSubscribeACL();
	else
		PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnSubscribeOKSubscirbeAlc(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsPresence::OnSubscribeOKSubscirbeAlc");
	DeleteTimer(PRESENCE_RESPONSE_TOUT);
	if (HandleSubscribeOk(pParam, SubscribeOCSPresenceAcl))
		SendServiceSetUri();
	else
		PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnSubscribeOKSubscirbeSelf(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsPresence::OnSubscribeOKSubscirbeSelf");
	DeleteTimer(PRESENCE_RESPONSE_TOUT);
	if (HandleSubscribeOk(pParam, SubscribeLyncRoamingSelf))
	{
		UINT32 sameEnterpriseMember = 0;
		UINT32 federatedMember = 0;
		int readedCategoryNum = 0;

		readedCategoryNum = sscanf(m_richPresenceVersionsList,"%u,%u;",
							&sameEnterpriseMember,&federatedMember);

		if(!sameEnterpriseMember && !federatedMember)
			SendServiceSetMembers();
		else
		{
			if(m_lastPresenceState == SIP_PRESENCE_OFFLINE)
			{
				SendServiceSetRichPresence();
			}
			else
			{
				ChangePresenceState(m_lastPresenceState);
			}
		}
	}
	else
		PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnServiceOKServiceSetUri(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsPresence::OnServiceOKServiceSetUri");
	DeleteTimer(PRESENCE_RESPONSE_TOUT);
	if (HandleServiceOk(pParam, ServiceSetPresenceUri))
		SendServiceSetAce();
	else
		PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnServiceOKServiceSetAce(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsPresence::OnServiceOKServiceSetAce - Presence flow completed");
	DeleteTimer(PRESENCE_RESPONSE_TOUT);
	if (HandleServiceOk(pParam, ServiceSetPresenceAce))
	{
		m_ePresenceStatus = ePresenceCompleted;
		m_IsNeedToReSubscribe = FALSE;
		WORD currentPresentedConfNumber = ::GetPresentedConfNumber();
		PTRACE2INT(eLevelInfoNormal,"CSipProxyMsPresence::OnServiceOKServiceSetAce. Presented conf number is", currentPresentedConfNumber);
		::IncreasePresentedConfNumber();
	}
	else
		PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnServiceOKServiceSetRich(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsPresence::OnServiceOKServiceSetRich");
	DeleteTimer(PRESENCE_RESPONSE_TOUT);
	if (HandleServiceOk(pParam, ServiceSetPresenceRich))
	{
		m_ePresenceStatus = ePresenceCompleted;
		m_IsNeedToReSubscribe = FALSE;
		WORD currentPresentedConfNumber = ::GetPresentedConfNumber();
		PTRACE2INT(eLevelError,"CSipProxyMsPresence::OnServiceOKServiceSetRich. Presented conf number is", currentPresentedConfNumber);
		::IncreasePresentedConfNumber();
		m_lastPresenceState=SIP_PRESENCE_ONLINE;
	}
	else
		PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnServiceOKServiceUnsetRich(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsPresence::OnServiceOKServiceUnsetRich");
	DeleteTimer(PRESENCE_RESPONSE_TOUT);
	if (HandleServiceOk(pParam, ServiceUnsetPresenceRich))
		m_ePresenceStatus = ePresenceCompleted;
	else
		PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnServiceOKServiceSetMember(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsPresence::OnServiceOKServiceSetMember");
	DeleteTimer(PRESENCE_RESPONSE_TOUT);
	if (HandleServiceOk(pParam, ServiceSetMembers))
	{
		if(m_lastPresenceState == SIP_PRESENCE_OFFLINE)
		{
			SendServiceSetRichPresence();
		}
		else
		{
			ChangePresenceState(m_lastPresenceState);
		}
	}
	else
		PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnServiceOKOnlineServiceRich(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsPresence::OnServiceOKOnlineServiceRich");
	DeleteTimer(PRESENCE_RESPONSE_TOUT);
	if (!HandleServiceOk(pParam, ServiceSetPresenceOnlineRich))
		PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnServiceOKBusyServiceRich(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsPresence::OnServiceOKBusyServiceRich");
	DeleteTimer(PRESENCE_RESPONSE_TOUT);
	if (!HandleServiceOk(pParam, ServiceSetPresenceBusyRich))
		PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnSubscribeFailedSubscirbeContacts(CSegment* pParam)
{
	PTRACE(eLevelError,"CSipProxyMsPresence::OnSubscribeFailedSubscirbeContacts");
	PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnSubscribeFailedSubscirbeAlc(CSegment* pParam)
{
	PTRACE(eLevelError,"CSipProxyMsPresence::OnSubscribeFailedSubscirbeAlc");
	PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnSubscribeFailedSubscirbeSelf(CSegment* pParam)
{
	PTRACE(eLevelError,"CSipProxyMsPresence::OnSubscribeFailedSubscirbeSelf");
	PresenceFlowFailed();
}
void CSipProxyMsPresence::OnServiceFailedServiceSetUri(CSegment* pParam)
{
	PTRACE(eLevelError,"CSipProxyMsPresence::OnServiceFailedServiceSetUri");
	PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnServiceFailedServiceSetAce(CSegment* pParam)
{
	PTRACE(eLevelError,"CSipProxyMsPresence::OnServiceFailedServiceSetAce");
	PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnServiceFailedServiceSetRich(CSegment* pParam)
{
	PTRACE(eLevelError,"CSipProxyMsPresence::OnServiceFailedServiceSetRich");
	PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnServiceFailedServiceSetMember(CSegment* pParam)
{
	PTRACE(eLevelError,"CSipProxyMsPresence::OnServiceFailedServiceSetMember");
	PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnServiceFailedServiceUnsetRich(CSegment* pParam)
{
	PTRACE(eLevelError,"CSipProxyMsPresence::OnServiceFailedServiceUnsetRich");
	PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnPresenceResponseTimeOut(CSegment* pParam)
{
	PTRACE(eLevelError,"CSipProxyMsPresence::OnPresenceResponseTimeOut");
	PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnServiceFailedOnlineServiceRich(CSegment* pParam)
{
	PTRACE(eLevelError,"CSipProxyMsPresence::OnServiceFailedOnlineServiceRich");
	PresenceFlowFailed();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::OnServiceFailedBusyServiceRich(CSegment* pParam)
{
	PTRACE(eLevelError,"CSipProxyMsPresence::OnServiceFailedBusyServiceRich");
	PresenceFlowFailed();
}

////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::PresenceFlowFailed()
{
	PTRACE(eLevelError,"CSipProxyMsPresence::PresenceFlowFailed");
	DeleteTimer(PRESENCE_RESPONSE_TOUT);
	m_ePresenceStatus = ePresenceFailed;
	m_state = IDLE;
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::SendSubscribeContacts()
{
	m_state = sPRESENCE_SUBSCRIBE_CONTACTS;

	CSipSubscribeStruct SipSubscribe;
	SipSubscribe.SetSubOpcode(SubscribeOCSPresenceRoamingContacts);
	SipSubscribe.SetHeaderField(kEvent, "vnd-microsoft-roaming-contacts");
	SipSubscribe.SetHeaderField(kAccept, "application/vnd-microsoft-roaming-contacts+xml");

	FillGerenalSubscribeParams(SipSubscribe);
	SendSubscribe(SipSubscribe);
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::SendSubscribeACL()
{
	m_state = sPRESENCE_SUBSCRIBE_ALC;

	CSipSubscribeStruct SipSubscribe;
	SipSubscribe.SetSubOpcode(SubscribeOCSPresenceAcl);
	SipSubscribe.SetHeaderField(kEvent, "vnd-microsoft-roaming-ACL");
	SipSubscribe.SetHeaderField(kAccept, "application/vnd-microsoft-roaming-acls+xml");

	FillGerenalSubscribeParams(SipSubscribe);
	SendSubscribe(SipSubscribe);
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::SendSelfSubscribeAndSaveLastPresenceState(BYTE presenceState)
{
	m_lastPresenceState = presenceState;
	SendSelfSubscribe();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::SendSelfSubscribe()
{
	m_state = sPRESENCE_SUBSCRIBE_SELF;
	CSipSubscribeStruct SipSubscribe;
	SipSubscribe.SetSubOpcode(SubscribeLyncRoamingSelf);
	SipSubscribe.SetHeaderField(kEvent, "vnd-microsoft-roaming-self");
	SipSubscribe.SetHeaderField(kAccept, "application/vnd-microsoft-roaming-self+xml");
	FillGerenalSubscribeParams(SipSubscribe);
	SendSubscribe(SipSubscribe);
}
void CSipProxyMsPresence::FillGerenalSubscribeParams(CSipSubscribeStruct &SipSubscribe)
{
	SipSubscribe.SetTransportType(m_TransportType);

	PTRACE2INT(eLevelInfoNormal,"CSipProxyMsPresence::FillGerenalSubscribeParams: MS_IPV6: ProxyVersion:", (DWORD)m_proxyAddress.ipVersion);
	if (m_proxyAddress.ipVersion == eIpVersion4)
		SipSubscribe.SetProxyIpV4(m_proxyAddress.addr.v4.ip);
	else
		SipSubscribe.SetProxyIpV6((char *)m_proxyAddress.addr.v6.ip);

	SipSubscribe.SetProxyPort(m_proxyAddress.port);

	SipSubscribe.SetHeaderField(kFromDisplay, m_confName);
	SipSubscribe.SetHeaderField(kFrom, m_confUri);
	SipSubscribe.SetHeaderField(kToDisplay, m_confName);
	SipSubscribe.SetHeaderField(kTo, m_confUri);
	SipSubscribe.SetHeaderField(kContactDisplay,m_confName);
	SipSubscribe.SetHeaderField(kContact, m_LocalUriIP);

	SipSubscribe.SetHeaderField(kSupported, "ms-piggyback-first-notify");
	SipSubscribe.SetId(m_id);
	SipSubscribe.SetExpires(0); // In order to terminate the subscribe session immediately after the response from the MS server.
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::SendSubscribe(CSipSubscribeStruct &SipSubscribe)
{
	COstrStream msg;
	SipSubscribe.Dump(msg);
	PTRACE2(eLevelInfoNormal, "CSipProxyMsPresence::SendSubscribe:\n ", msg.str().c_str());

	mcReqSubscribe* pMsg = SipSubscribe.BuildSubscribeReq();

	if (pMsg)
	{
		int size = sizeof(mcReqSubscribeBase) + pMsg->sipHeaders.headersListLength;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)pMsg, size);
	SendMsgToCS(m_serviceId, SIP_CS_PROXY_SUBSCRIBE_REQ, pSeg);

	PDELETE(pSeg);
	PDELETEA(pMsg);

		StartTimer(PRESENCE_RESPONSE_TOUT, PresenceResponseTimeOut);
	}
	else
	{
		PTRACE(eLevelError,"CSipProxyMsPresence::SendSubscribe - pMsg is NULL could not send SUBSCRIBE msg to CS");
		m_state = IDLE;
		DBGPASSERT_AND_RETURN(1);
	}
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::SendServiceSetUri()
{
	m_state = sPRESENCE_SERVICE_SET_URI;

	CSipServiceStruct SipService;
	SipService.SetSubOpcode(ServiceSetPresenceUri);
	SipService.SetHeaderField(kContentType, "application/SOAP+xml");

	FillGerenalServiceParams(SipService);
	SendService(SipService);

}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::SendServiceSetAce()
{
	m_state = sPRESENCE_SERVICE_SET_ACE;

	CSipServiceStruct SipService;
	SipService.SetSubOpcode(ServiceSetPresenceAce);
	SipService.SetHeaderField(kContentType, "application/SOAP+xml");
	SipService.SetHeaderField(kAclListDelta, m_aclDeltaNum);

	FillGerenalServiceParams(SipService);
	SendService(SipService);
}
void CSipProxyMsPresence::SendServiceSetRichPresence()
{
	m_state = sPRESENCE_SERVICE_SET_RICH;
	CSipServiceStruct SipService;
	SipService.SetSubOpcode(ServiceSetPresenceRich);
	SipService.SetHeaderField(kContentType, "application/msrtc-category-publish+xml");
	SipService.SetHeaderField(kRichPresence, m_richPresenceVersionsList);
	FillGerenalServiceParams(SipService);
	SendService(SipService);
}

void CSipProxyMsPresence::SendServiceUnsetRichPresence()
{
	m_state = sPRESENCE_SERVICE_UNSET_RICH;
	CSipServiceStruct SipService;
	SipService.SetSubOpcode(ServiceUnsetPresenceRich);
	SipService.SetHeaderField(kContentType, "application/msrtc-category-publish+xml");
	SipService.SetHeaderField(kRichPresence, m_richPresenceVersionsList);
	FillGerenalServiceParams(SipService);
	SendService(SipService);
}

void CSipProxyMsPresence::SendServiceSetMembers()
{
	m_state = sPRESENCE_SERVICE_SET_MEMBER;
	CSipServiceStruct SipService;
	SipService.SetSubOpcode(ServiceSetMembers);
	SipService.SetHeaderField(kContentType, "application/msrtc-setcontainermembers+xml");
	SipService.SetHeaderField(kRichPresence, m_richPresenceVersionsList);
	FillGerenalServiceParams(SipService);
	SendService(SipService);
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::FillGerenalServiceParams(CSipServiceStruct &SipService)
{
	SipService.SetTransportType(m_TransportType);

	PTRACE2INT(eLevelError, "CSipProxyMsPresence::FillGerenalServiceParams: MS_IPV6: ProxyVersion:",(DWORD)m_proxyAddress.ipVersion);
	if (m_proxyAddress.ipVersion == eIpVersion4)
		SipService.SetProxyIpV4(m_proxyAddress.addr.v4.ip);
	else
		SipService.SetProxyIpV6((char *)m_proxyAddress.addr.v6.ip);

	SipService.SetProxyPort(m_proxyAddress.port);
	SipService.SetHeaderField(kFromDisplay, m_confName);
	SipService.SetHeaderField(kFrom, m_confUri);
	SipService.SetHeaderField(kToDisplay, m_confName);
	SipService.SetHeaderField(kTo, m_confUri);
	SipService.SetHeaderField(kContactDisplay,m_confName);
	SipService.SetHeaderField(kContact, m_LocalUriIP);

	SipService.SetId(m_id);
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsPresence::SendService(CSipServiceStruct &SipService)
{
	COstrStream msg;
	SipService.Dump(msg);
	PTRACE2(eLevelInfoNormal, "CSipProxyMsPresence::SendService:\n ", msg.str().c_str());

	mcReqService* pMsg = SipService.BuildServiceReq();

	if (pMsg) 
	{
		int size = sizeof(mcReqServiceBase) + pMsg->sipHeaders.headersListLength;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)pMsg, size);
	SendMsgToCS(m_serviceId, SIP_CS_PROXY_SERVICE_REQ, pSeg);

	PDELETE(pSeg);
	PDELETEA(pMsg);

		StartTimer(PRESENCE_RESPONSE_TOUT, PresenceResponseTimeOut);
	}
	else
	{
		PTRACE(eLevelError,"CSipProxyMsPresence::SendService - pMsg is NULL could not send SERVICE msg to CS");
		m_state = IDLE;
		DBGPASSERT_AND_RETURN(2);
	}
}
////////////////////////////////////////////////////////////////////////////
BYTE CSipProxyMsPresence::HandleSubscribeOk(CSegment* pParam, WORD expectedSubOpcode)
{
	mcIndSubscribeResp * pSubscribeResponseMsg = (mcIndSubscribeResp *)pParam->GetPtr(1);
	DWORD expires = pSubscribeResponseMsg->expires;
	WORD subOpcode = pSubscribeResponseMsg->subOpcode;

	if (subOpcode != expectedSubOpcode)
	{
		PTRACE2INT(eLevelError, "CSipProxyMsPresence::HandleSubscribeOk : invalid subOpcode =",subOpcode);
		return FALSE;
	}
	if(expires)
	{
		PTRACE2INT(eLevelError, "CSipProxyMsPresence::HandleSubscribeOk : expires=",expires);
		return FALSE;
	}

	CSipHeaderList * pHeaders = new CSipHeaderList(pSubscribeResponseMsg->sipHeaders);
	const CSipHeader* pSupportedHeader = pHeaders->GetNextHeader(kSupported);
	if (!pSupportedHeader)
	{
		PTRACE(eLevelInfoNormal, "CSipProxyMsPresence::HandleSubscribeOk, Server doesn't support ms-piggyback");
		POBJDELETE(pHeaders); //B.S. klocwork 2577
		return FALSE;
	}

	BYTE bRetValue = TRUE;

	const char* pSupported = pSupportedHeader->GetHeaderStr();
	if(strncmp(OCS_PROV_SUPPORTED,pSupported,H243_NAME_LEN) != 0)
	{
		PTRACE(eLevelInfoNormal, "CSipProxyMsPresence::HandleSubscribeOk, Server doesn't support ms-piggyback");
		bRetValue = FALSE;
	}

	if (bRetValue && (subOpcode == SubscribeOCSPresenceAcl))
		bRetValue = ExtractAclDeltaNumber(pHeaders);
	else if (bRetValue && (subOpcode == SubscribeLyncRoamingSelf))
		bRetValue = ExtractPresenceVersionsList(pHeaders);

	POBJDELETE(pHeaders);
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////
BYTE CSipProxyMsPresence::HandleServiceOk(CSegment* pParam, WORD expectedSubOpcode)
{
	mcIndServiceResp * pServiceResponseMsg = (mcIndServiceResp *)pParam->GetPtr(1);
	CSipHeaderList * pHeaders = new CSipHeaderList(pServiceResponseMsg->sipHeaders);
	BYTE bRetValue = TRUE;

	WORD subOpcode = pServiceResponseMsg->subOpcode;
	bRetValue = ExtractPresenceVersionsList(pHeaders);

	PDELETE(pHeaders);

	if (subOpcode != expectedSubOpcode)
	{
		PTRACE2INT(eLevelError, "CSipProxyMsPresence::HandleServiceOk : invalid subOpcode =",subOpcode);
		return FALSE;
	}

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////
BYTE CSipProxyMsPresence::ExtractAclDeltaNumber(CSipHeaderList * pHeaders)
{
	BYTE bRetValue = FALSE;

	const CSipHeader* pAclDeltaHdr = pHeaders->GetNextHeader(kAclListDelta);
	if(pAclDeltaHdr)
	{
		const char* pAclDelta = pAclDeltaHdr->GetHeaderStr();
		if(pAclDelta)
		{
			bRetValue = TRUE;
			PDELETEA(m_aclDeltaNum);
			m_aclDeltaNum = new char[MaxLengthOfAclDeltaNum+1];
			memset(m_aclDeltaNum, 0, MaxLengthOfAclDeltaNum+1);
			strncpy(m_aclDeltaNum, pAclDelta, MaxLengthOfAclDeltaNum);
		}
	}
	if (!bRetValue)
		PTRACE(eLevelInfoNormal, "CSipProxyMsPresence::ExtractAclDeltaNumber : Failed to extract Acl Delta number");

	return bRetValue;
}
////////////////////////////////////////////////////////////////////////////
BYTE CSipProxyMsPresence::ExtractPresenceVersionsList(CSipHeaderList * pHeaders)
{
	BYTE bRetValue = FALSE;
	const CSipHeader* pRichPresenceHdr = pHeaders->GetNextHeader(kRichPresence);
	if(pRichPresenceHdr)
	{
		const char* pRichPresence = pRichPresenceHdr->GetHeaderStr();
		if(pRichPresence)
		{
			bRetValue = TRUE;
			PDELETEA(m_richPresenceVersionsList);
			m_richPresenceVersionsList = new char[MaxLengthOfRichPresenceVerList+1];
			memset(m_richPresenceVersionsList, 0, MaxLengthOfRichPresenceVerList+1);
			strncpy(m_richPresenceVersionsList, pRichPresence, MaxLengthOfRichPresenceVerList);
		}
	}
	if (!bRetValue)
		PTRACE(eLevelInfoNormal, "CSipProxyMsPresence::ExtractPresenceVersions : Failed to extract rich presence versions");
	return bRetValue;
}
