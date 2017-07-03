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
// FILE: SIPProxyMsSubscriber.cpp                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// Inga | 21/12/09  |                                                      |
//+========================================================================+

#include <stdlib.h>
#include <string.h>
#include "SipProxyMsSubscriber.h"
#include "SipHeadersList.h"
#include "SipUtils.h"
#include "SipProxyConfCntl.h"
#include "IpAddressDefinitions.h"
#include  "IpCsOpcodes.h"
#include "SipProxyManagerLocalApi.h"
#include "OpcodesMcmsInternal.h"
//#include "ICEApiDefinitions.h"
#include "IpPartyMonitorDefinitions.h"
#include "ManagerApi.h"
#include "ICEApiDefinitions.h"
#include "IceCmReq.h"
#include "IceCmInd.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "FaultsDefines.h"
#include "AlarmableTask.h"
#include <arpa/inet.h>


#include "ConfigHelper.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"

// definitions
#define	SubscribeTimeOut		    40
#define TIMER_START_SUBSCRIBING		300
#define NOTIFY_TIMER				301
#define MAX_SUBSCRIBE_RETRIES		5

//#define TIMER_CM_ICE_INIT	100

//Timers
//#define IceInitTimeOut		20


// start message map -------------------------------------------
PBEGIN_MESSAGE_MAP(CSipProxyMsSubscriber)

	ONEVENT(SIP_START_SUBSCRIBE,					IDLE,				CSipProxyMsSubscriber::OnStartSubscribeIDLE)
	ONEVENT(SIP_START_SUBSCRIBE,					SUBSCRIBING,		CSipProxyMsSubscriber::OnStartSubscribeIDLE)
	ONEVENT(SIP_START_SUBSCRIBE,				    SUBSCRIBED,		    CSipProxyMsSubscriber::OnStartSubscribeIDLE)
	ONEVENT(TIMER_START_SUBSCRIBING,			    SUBSCRIBING,		CSipProxyMsSubscriber::OnSubscribeTimer)


	ONEVENT(SUBSCRIBE_OK,							SUBSCRIBING,		CSipProxyMsSubscriber::OnSubscribeOKSubscribing)
	ONEVENT(SUBSCRIBE_FAILED,						SUBSCRIBING,		CSipProxyMsSubscriber::OnSubscribeFailedSubscribing)

//	ONEVENT(SIP_CS_PROXY_SUBSCRIBE_RESPONSE_IND,	SUBSCRIBING,	    CSipSubscriber::HandleSubscribeResponse)
	ONEVENT(RECEIVEDNOTIFY,							SUBSCRIBED,			CSipProxyMsSubscriber::OnNotifyReceiveSubscribed)\
	ONEVENT(NOTIFY_TIMER,							SUBSCRIBED,			CSipProxyMsSubscriber::OnNotifyTimout)

	ONEVENT(SERVICE_OK,								SUBSCRIBED,			CSipProxyMsSubscriber::OnServiceOKSubscribed)
	ONEVENT(SERVICE_FAILED,							SUBSCRIBED,			CSipProxyMsSubscriber::OnServiceFailedSubscribed)

	ONEVENT(END_INIT_ICE_IND,						SUBSCRIBED,		    CSipProxyMsSubscriber::EndIceInitInd)
//	ONEVENT(TIMER_CM_ICE_INIT,						SUBSCRIBED,		    CSipProxyMsSubscriber::CMIceInitTimeout)



	PEND_MESSAGE_MAP(CSipProxyMsSubscriber,CSipSubscriberMngr);

// end   message map -------------------------------------------



CSipProxyMsSubscriber::CSipProxyMsSubscriber()
{
	m_MrasUri = NULL;
	m_WaitForNotifyInd = FALSE;
//	m_timerServiceRefresh = 480;
	m_DomainName = NULL;
	m_UserName = NULL;
	m_IsNeedToReSubscribe = TRUE;
	m_serviceId = 0;
	m_IsEnableBWPolicyCheck = FALSE;
	m_UcMaxVideoRateAllowed = 0;


}
////////////////////////////////////////////////////////////////////////////
CSipProxyMsSubscriber::~CSipProxyMsSubscriber()
{
	PDELETEA(m_MrasUri);
	PDELETEA(m_UserName);
	PDELETEA(m_DomainName);

}

////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::StartSubscribe(char* UserName,char* HostName, DWORD serviceId)
{
	CSegment* seg = new CSegment;

	*seg << UserName
		 << HostName
		 << serviceId;

	DispatchEvent(SIP_START_SUBSCRIBE,seg);
	PDELETE(seg);
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::OnStartSubscribeIDLE(CSegment* pParam)
{

	PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnStartSubscribeIDLE ");

	m_state = SUBSCRIBING;

	char UserName[H243_NAME_LEN];
	char HostName[H243_NAME_LEN];

	DWORD serviceId = 0;

	*pParam >> UserName;
	*pParam >> HostName;
	*pParam >> serviceId;


	if(m_UserName== NULL)
	{
		PTRACE2(eLevelInfoNormal,"CSipProxyMsSubscriber::OnStartSubscribeIDLE  UserName = ", UserName);
		m_UserName = new char[MaxAddressListSize];
		memset(m_UserName, '\0', MaxAddressListSize);
		//YYYY
		strncpy(m_UserName, UserName, H243_NAME_LEN);
		//snprintf(m_UserName, H243_NAME_LEN, "%s", "rmx24027.reg10.ent");
		PTRACE2(eLevelError,"YYYY COneConf::Subscribe - m_UserName = ", m_UserName);
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CSipProxyMsSubscriber::OnStartSubscribeIDLE  m_UserName = ", m_UserName);
	}

	if(m_DomainName== NULL)
	{
		PTRACE2(eLevelInfoNormal,"CSipProxyMsSubscriber::OnStartSubscribeIDLE  HostName", HostName);
		m_DomainName = new char[MaxAddressListSize];
		memset(m_DomainName, '\0', MaxAddressListSize);
		strncpy(m_DomainName, HostName, H243_NAME_LEN);
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CSipProxyMsSubscriber::OnStartSubscribeIDLE  m_DomainName", m_DomainName);
	}

	m_serviceId = serviceId;

	BuildSubscribeMsg(m_serviceId);


}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::BuildSubscribeMsg(DWORD serviceId)
{
	DWORD expires = 0;	// In order to terminate the subscribe session immediately after the response from the MS server.

	PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::BuildSubscribeReq ");

	ALLOCBUFFER(UserUri, MaxAddressListSize);
	memset(UserUri, '\0', MaxAddressListSize);
	strncpy(UserUri, m_UserName, MaxAddressListSize-2);
	strncat(UserUri, "@", 1);

	//verify UserUri has enough buffer space to copy the domain
	if (strlen(m_DomainName) < MaxAddressListSize - strlen(UserUri) -1)
		strncat(UserUri, m_DomainName, H243_NAME_LEN);

	m_IsNeedToReSubscribe = TRUE;
	PTRACE2(eLevelInfoNormal,"CSipProxyMsSubscriber::SendServiceReq UserUri = ", UserUri);
	CSipSubscribeStruct SipSubscribe;
	SipSubscribe.SetSubOpcode(SubscribeOCSServerConfiguration);
	SipSubscribe.SetTransportType(m_TransportType);

	TRACEINTO << "MS_IPV6: ProxyVersion:" << (DWORD)m_proxyAddress.ipVersion;
	if (m_proxyAddress.ipVersion == eIpVersion4)
	{
		SipSubscribe.SetProxyIpV4(m_proxyAddress.addr.v4.ip);
		SipSubscribe.SetProxyIpVersion(eIpVersion4);
	}
	else
	{
		SipSubscribe.SetProxyIpV6((char *)m_proxyAddress.addr.v6.ip);
		SipSubscribe.SetProxyIpVersion(eIpVersion6);
	}

	SipSubscribe.SetProxyPort(m_proxyAddress.port);
	SipSubscribe.SetHeaderField(kFromDisplay, m_UserName);
	SipSubscribe.SetHeaderField(kFromDisplay, "");
	SipSubscribe.SetHeaderField(kFrom, UserUri);
	SipSubscribe.SetHeaderField(kToDisplay, m_UserName);
	SipSubscribe.SetHeaderField(kToDisplay, "");
	SipSubscribe.SetHeaderField(kTo, UserUri);
	SipSubscribe.SetHeaderField(kContactDisplay,m_UserName);
	SipSubscribe.SetHeaderField(kContact, m_LocalUriIP);
	PTRACE2(eLevelInfoNormal,"CSipProxyMsSubscriber::BuildSubscribeReq ", m_LocalUriIP);

	DEALLOCBUFFER(UserUri);

	ALLOCBUFFER(event, H243_NAME_LEN);
	strncpy(event, OCS_PROV_EVENT_V2, H243_NAME_LEN);
	SipSubscribe.SetHeaderField(kEvent, event);
	DEALLOCBUFFER(event);

	ALLOCBUFFER(accept, MaxAddressListSize);
	strncpy(accept, OCS_PROV_CONFIGURATION_PACKAGE, H243_NAME_LEN);
	SipSubscribe.SetHeaderField(kAccept, accept);
	DEALLOCBUFFER(accept);

	ALLOCBUFFER(supported, MaxAddressListSize);
	strncpy(supported, OCS_PROV_SUPPORTED, H243_NAME_LEN);
	SipSubscribe.SetHeaderField(kSupported, supported);
	DEALLOCBUFFER(supported);

	ALLOCBUFFER(contentType, MaxAddressListSize);
	strncpy(contentType, OCS_PROV_CONFIGURATION_PACKAGE, H243_NAME_LEN);
	SipSubscribe.SetHeaderField(kContentType, contentType);
	DEALLOCBUFFER(contentType);


	SipSubscribe.SetId(m_id);
	SipSubscribe.SetExpires(expires);

	COstrStream msg;
	SipSubscribe.Dump(msg);
	PTRACE2(eLevelInfoNormal, "CSipProxyMsSubscriber::BuildSubscribeReq req:\n ", msg.str().c_str());


	mcReqSubscribe* pSubMsg = SipSubscribe.BuildSubscribeReq();

	if (pSubMsg)
	{
		int size = sizeof(mcReqSubscribeBase) + pSubMsg->sipHeaders.headersListLength;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)pSubMsg, size);
	SendMsgToCS(serviceId, SIP_CS_PROXY_SUBSCRIBE_REQ, pSeg);

	DWORD tout = SubscribeTimeOut*SECOND;

	StartTimer(TIMER_START_SUBSCRIBING, tout);

		PDELETE(pSeg);
		PDELETEA(pSubMsg);
	}
	else
	{
		PTRACE(eLevelError,"CSipProxyMsPresence::SendService - pSubMsg is NULL could not send SUBSCRIBE msg to CS");
		m_state = IDLE;
		DBGPASSERT_AND_RETURN(1);
	}
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::OnSubscribeTimer(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnSubscribeTimer Failed to receive Subscribe response");
	DeleteTimer(TIMER_START_SUBSCRIBING);

	CProcessBase *pProcess = CProcessBase::GetProcess();
	pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,
								AA_FAILED_TO_SUBSCRIBE_WITH_THE_OCS,
								MAJOR_ERROR_LEVEL,
								"Failed to subscribe with the OCS",
								true,
								true);

	if(m_NumOfSubscribeRetries < 5)
	{
		PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnSubscribeTimer Failed to receive Subscribe response - Try again!!");
		m_NumOfSubscribeRetries++;

		//Try to Resubscribe again.
		BuildSubscribeMsg(m_serviceId);
	}
	else
	{
		DBGPASSERT(1);
		PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnSubscribeTimer Failed to receive Subscribe response - Not tring any more");
	}
}

////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::OnSubscribeOKSubscribing(CSegment* pParam)
{

	BOOL bIsFailure = FALSE;
	PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnSubscribeOKSubscribing ");

	DeleteTimer( TIMER_START_SUBSCRIBING);

	mcIndSubscribeResp * pSubscribeResponseMsg = (mcIndSubscribeResp *)pParam->GetPtr(1);
	DWORD expires = pSubscribeResponseMsg->expires;
	if(expires == 0)
	{
		PTRACE(eLevelInfoNormal, "CSipProxyMsSubscriber::OnSubscribeOKSubscribing, Expire... ");
		CSipHeaderList * pHeaders = new CSipHeaderList(pSubscribeResponseMsg->sipHeaders);

		const CSipHeader* pSupportedHeader = pHeaders->GetNextHeader(kSupported);
		if(pSupportedHeader)
		{
			PTRACE(eLevelInfoNormal, "CSipProxyMsSubscriber::OnSubscribeOKSubscribing, Supported Header.... ");
			const char* pSupported = pSupportedHeader->GetHeaderStr();
			if(!strncmp(OCS_PROV_SUPPORTED,pSupported,H243_NAME_LEN))
			{
				PTRACE(eLevelInfoNormal, "CSipProxyMsSubscriber::OnSubscribeOKSubscribing, Server supports ms-piggyback ! ");
				const CSipHeader* pMrasUriHeader = pHeaders->GetNextHeader(kMrasUri);

				if(pMrasUriHeader)
				{
					const char* pMrasUri = pMrasUriHeader->GetHeaderStr();
					if(strcmp(pMrasUri, ""))
					{
						PTRACE2(eLevelInfoNormal, "CSipProxyMsSubscriber::OnSubscribeOKSubscribing, MrasUri : ",pMrasUri);

						CProcessBase *pProcess = CProcessBase::GetProcess();
						pProcess->RemoveActiveAlarmFromProcess(AA_FAILED_TO_SUBSCRIBE_WITH_THE_OCS);

						if(m_MrasUri)
							PDELETEA(m_MrasUri);

						m_MrasUri = new char[MaxLengthOfMrasUri];
						memset(m_MrasUri, '\0', MaxLengthOfMrasUri);
						strncpy(m_MrasUri, pMrasUri, MaxLengthOfMrasUri-1);
//						PTRACE(eLevelInfoNormal, "CSipProxyMsSubscriber::OnSubscribeOKSubscribing, calling UpdateCardsMngr: ");
//
//						UpdateCardsMngr(m_id, NULL, NULL, m_MrasUri, 0, 0);
//
//						PTRACE(eLevelInfoNormal, "CSipProxyMsSubscriber::OnSubscribeOKSubscribing, after UpdateCardsMngr: ");

						PTRACE(eLevelInfoNormal, "CSipProxyMsSubscriber::OnSubscribeOKSubscribing, calling SendServiceReq: ");
						SendServiceReq();//YGYG
					}
					else
					{
						PTRACE(eLevelInfoNormal, "CSipProxyMsSubscriber::OnSubscribeOKSubscribing, MrasUri is empty ! - Subscribe with no m_MrasUri prbalby local env with no edge ");
						bIsFailure = FALSE;
						const char* pUserName=NULL;
						const char* pPassword=NULL;
						const char* pRelayHostName = NULL;
						WORD  port = 0;
						WORD udpPortVal = 0;
						WORD tcpPortVal = 0;
						CProcessBase *pProcess = CProcessBase::GetProcess();
						pProcess->RemoveActiveAlarmFromProcess(AA_SERVICE_REQ_FAILED);


						UpdateCardsMngr(m_id,pUserName,pPassword,pRelayHostName,tcpPortVal,udpPortVal);
						/*
						CProcessBase *pProcess = CProcessBase::GetProcess();
						pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,AA_FAILED_TO_SUBSCRIBE_WITH_THE_OCS,	MAJOR_ERROR_LEVEL,
							"Failed to subscribe with the OCS",
						     true,
							true);

						TerminateICERegistration();
						*/
						//m_WaitForNotifyInd = TRUE;
					}
				} else {
					PTRACE(eLevelInfoNormal, "CSipProxyMsSubscriber::OnSubscribeOKSubscribing: No MrasUri");
				}

				const CSipHeader* pBWPolicyHeader = pHeaders->GetNextHeader(kEnableBWPolicyCheck);
				const CSipHeader* pMaxVideoRateHeader = pHeaders->GetNextHeader(kUcMaxVideoRateAllowed);
				if(pBWPolicyHeader)
				{
					const char* pBWPolicy = pBWPolicyHeader->GetHeaderStr();
					if(strcmp(pBWPolicy, ""))
					{
						PTRACE2(eLevelInfoNormal, "CSipProxyMsSubscriber::OnSubscribeOKSubscribing, BWPolicyCheck : ",pBWPolicy);

						if(!strcmp(pBWPolicy, "true"))
							m_IsEnableBWPolicyCheck = TRUE;
						else
							m_IsEnableBWPolicyCheck = FALSE;
					}
					CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
					std::string sKey;
					std::string sCacEnable;

					sKey = "CAC_ENABLE";
					sysConfig->GetDataByKey(sKey, sCacEnable);
					if( strcmp("NO", sCacEnable.c_str()) == 0 )
						m_IsEnableBWPolicyCheck = FALSE;

				}
				if(pMaxVideoRateHeader)
				{
					const char* pMaxVideoRate = pMaxVideoRateHeader->GetHeaderStr();
					if(strcmp(pMaxVideoRate, ""))
					{
						PTRACE2(eLevelInfoNormal, "CSipProxyMsSubscriber::OnSubscribeOKSubscribing, UcMaxVideoRateAllowed : ",pMaxVideoRate);
						m_UcMaxVideoRateAllowed = ParseHeaderMaxVideoRateAllowed(pMaxVideoRate);
					}
				}

			}
			else
			{
					PTRACE(eLevelInfoNormal, "CSipProxyMsSubscriber::OnSubscribeOKSubscribing, MrasUri is empty ! - local env no edge ");
					bIsFailure = FALSE;
				    /*
					CProcessBase *pProcess = CProcessBase::GetProcess();
					pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,AA_FAILED_TO_SUBSCRIBE_WITH_THE_OCS,	MAJOR_ERROR_LEVEL,
							"Failed to subscribe with the OCS",
						     true,
							true);

					TerminateICERegistration();
					*/
					bIsFailure = FALSE;
					const char* pUserName=NULL;
					const char* pPassword=NULL;
					const char* pRelayHostName = NULL;
					WORD  port = 0;
					WORD udpPortVal = 0;
					WORD tcpPortVal = 0;
					CProcessBase *pProcess = CProcessBase::GetProcess();
					pProcess->RemoveActiveAlarmFromProcess(AA_SERVICE_REQ_FAILED);


					UpdateCardsMngr(m_id,pUserName,pPassword,pRelayHostName,tcpPortVal,udpPortVal);
						//m_WaitForNotifyInd = TRUE;
			}
		}

		POBJDELETE(pHeaders);
	}

	if(bIsFailure)
		m_state = IDLE;
	else
		m_state = SUBSCRIBED;

	if(m_WaitForNotifyInd)
	{
		DWORD tout = SubscribeTimeOut*SECOND;
		StartTimer(NOTIFY_TIMER,tout);
	}

}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::OnNotifyTimout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnNotifyTimout ");

	if(m_WaitForNotifyInd)
	{
		CProcessBase *pProcess = CProcessBase::GetProcess();
			pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,
								AA_RECEIVED_NOTIFICATION_FAILED,
								MAJOR_ERROR_LEVEL,
								"The Notify message containing the A/V Edge Server URI was not received",
								true,
								true);
	}
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::OnSubscribeFailedSubscribing(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnSubscribeFailedRegistered ");

	DWORD status = STATUS_FAIL;
	BYTE NeedToStartReSubscribe = FALSE;

	mcIndSubscribeResp * pSubscribeResponseMsg = (mcIndSubscribeResp *)pParam->GetPtr(1);
	DWORD expires = pSubscribeResponseMsg->expires;

	status = pSubscribeResponseMsg->status;

	CSipHeaderList * pHeaders = new CSipHeaderList(pSubscribeResponseMsg->sipHeaders);

	//Get Retry-After header value if exist
	DWORD retryAfterVal = 0;
	if (pHeaders)
	{
		const CSipHeader* pRetryAfterHdr = pHeaders->GetNextHeader(kRetryAfter);
	    if (pRetryAfterHdr)
	    {
	    	char cHeaderValue[256] = {0};
	    	strncpy(cHeaderValue, pRetryAfterHdr->GetHeaderStr(), sizeof(cHeaderValue)-1);
	    	cHeaderValue[sizeof(cHeaderValue)-1] = '\0';
	    	retryAfterVal = atoi(cHeaderValue);
	    	if (retryAfterVal)
	    	{
	    		PTRACE2INT(eLevelInfoNormal,"CSipProxyMsSubscriber::OnSubscribeFailedRegistered : Retry After = ",retryAfterVal);
	       	}

	    }
	    POBJDELETE(pHeaders);//B.S. klocwork 2579
	}
	CProcessBase *pProcess = CProcessBase::GetProcess();

	pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,AA_FAILED_TO_SUBSCRIBE_WITH_THE_OCS,	MAJOR_ERROR_LEVEL,
					"Failed to subscribe with the OCS",
					true,
					true);

	switch(status)
	{
		case(489):
		{
			PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnSubscribeFailedRegistered - Bad Event - Do nothing !! ");
			break;
		}
		case(481):
		{
			PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnSubscribeFailedRegistered - Not Exist - Do nothing !! ");
			break;
		}
		case(422):
		case(423):
		{
			PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnSubscribeFailedRegistered - Interval too small !! ");
			NeedToStartReSubscribe = TRUE;
			break;
		}
		case(408):
		{
			PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnSubscribeFailedRegistered - Timeout !! ");
			NeedToStartReSubscribe = TRUE;
			break;
		}
		case(403):
		{
			PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnSubscribeFailedRegistered - Forbidden - Do nothing!! ");
			break;
		}
		case(603):
		{
			PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnSubscribeFailedRegistered - Decline - Do nothing!! ");
			break;
		}

	}
	if(NeedToStartReSubscribe)
	{
		BuildSubscribeMsg(m_serviceId);
	}
	else
		if(retryAfterVal)
		{
			// TBD
		}
		else
		{
			TerminateICERegistration();
			m_state = IDLE;
		}
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::OnNotifyReceiveSubscribed(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnNotifyReceiveSubscribed ");
	CProcessBase *pProcess = CProcessBase::GetProcess();

	if(IsValidTimer(NOTIFY_TIMER))
		DeleteTimer(NOTIFY_TIMER);

	if(m_WaitForNotifyInd)
	{
		mcIndNotify * pNotifyMsg = (mcIndNotify *)pParam->GetPtr(1);

		CSipHeaderList * pHeaders = new CSipHeaderList(pNotifyMsg->sipHeaders);

		const CSipHeader* pMrasUriHeader = pHeaders->GetNextHeader(kMrasUri);

		if(pMrasUriHeader)
		{
			const char* pMrasUri = pMrasUriHeader->GetHeaderStr();
			if(pMrasUri)
			{
				PTRACE2(eLevelInfoNormal, "CSipProxyMsSubscriber::OnNotifyReceiveSubscribed, MrasUri : ",pMrasUri);

				pProcess->RemoveActiveAlarmFromProcess(AA_RECEIVED_NOTIFICATION_FAILED);

				if(m_MrasUri)
					PDELETEA(m_MrasUri);

				m_MrasUri = new char[MaxLengthOfMrasUri];
				memset(m_MrasUri, '\0', MaxLengthOfMrasUri);
				strncpy(m_MrasUri, pMrasUri, MaxLengthOfMrasUri-1);

				m_WaitForNotifyInd = FALSE;

				SendServiceReq();
			}
			else
			{
				PTRACE(eLevelInfoNormal, "CSipProxyMsSubscriber::OnNotifyReceiveSubscribed, No header of MrasUri ! ");
				pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,
										AA_RECEIVED_NOTIFICATION_FAILED,
										MAJOR_ERROR_LEVEL,
										"Received Notification does not contain URI",
										true,
										true);
			}
		}
		POBJDELETE(pHeaders);//B.S. klocwork 2580
	}
	PTRACE(eLevelInfoNormal, "CSipProxyMsSubscriber::OnSubscribeOKRegistered, Not waiting for Notify ! ");
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::SendServiceReq()
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::SendServiceReq ");

	ALLOCBUFFER(Duration, MaxAddressListSize);
	memset(Duration, '\0', MaxAddressListSize);
	strncpy(Duration, "480", H243_NAME_LEN);

	ALLOCBUFFER(UserUri, MaxAddressListSize);

	memset(UserUri, '\0', MaxAddressListSize);
	strncpy(UserUri, m_UserName, H243_NAME_LEN);

	strncat(UserUri, "@", 1);
	strncat(UserUri, m_DomainName, H243_NAME_LEN);
	PTRACE2(eLevelInfoNormal,"CSipProxyMsSubscriber::SendServiceReq m_UserName = ", m_UserName);
	PTRACE2(eLevelInfoNormal,"CSipProxyMsSubscriber::SendServiceReq UserUri = ", UserUri);
	CSipServiceStruct SipService;
	SipService.SetSubOpcode(ServiceEdgeConfiguration);
	SipService.SetTransportType(m_TransportType);

	TRACEINTO << "MS_IPV6: proxyVersion:" << (DWORD)m_proxyAddress.ipVersion;
	if (m_proxyAddress.ipVersion==eIpVersion4)
	{
		SipService.SetProxyIpV4(m_proxyAddress.addr.v4.ip);
		SipService.SetProxyIpVersion(eIpVersion4);
	}
	else //eIpType_IpV6
	{
		SipService.SetProxyIpV6((char *)m_proxyAddress.addr.v6.ip);
		SipService.SetProxyIpVersion(eIpVersion6);
	}

	PTRACE2INT(eLevelInfoNormal,"CSipProxyMsSubscriber::SendServiceReq m_proxyAddress.addr.v4.ip = ", m_proxyAddress.addr.v4.ip);


	SipService.SetProxyPort(m_proxyAddress.port);
	SipService.SetHeaderField(kFrom,UserUri);
	SipService.SetHeaderField(kFromDisplay, m_UserName);
	SipService.SetHeaderField(kFromDisplay, "");
	SipService.SetHeaderField(kTo, m_MrasUri);
	SipService.SetHeaderField(kToDisplay, m_UserName);
	SipService.SetHeaderField(kToDisplay, "");
	//YYYY SipService.SetHeaderField(kContactDisplay,m_UserName);
	SipService.SetHeaderField(kContactDisplay, "");
	//YYY SipService.SetHeaderField(kContact,m_LocalUriIP);

	std::string sContact;
	CProcessBase::GetProcess()->GetSysConfig()->GetDataByKey("SIP_CONTACT_OVERRIDE_STR", sContact);

	if(sContact.empty())
	{
		SipService.SetHeaderField(kContact,m_LocalUriIP);
	}
	else
	{
		if (strncmp(sContact.c_str(), "sip:", 4) == 0)
		{
			//strip sip: from string 256 is max gruu size
			std::string tmp = sContact.substr(4, MaxAddressListSize);
			SipService.SetHeaderField(kContact,tmp.c_str());
		}
		else
		{
			SipService.SetHeaderField(kContact,sContact.c_str());
		}
	}
	//SipService.SetHeaderField(kContact,sContact.c_str());

	//SipService.SetHeaderField(kContact, "rmx24027.reg10.ent@reg10.ent;opaque=srvr:videorouting:Tggd9yw0FFyNjnRqleNLwQAA;gruu");
	SipService.SetHeaderField(kCredDuration,Duration);


	ALLOCBUFFER(contentType, MaxAddressListSize);
	strncpy(contentType, OCS_MEDIA_RELAY_SERV_PACKAGE, H243_NAME_LEN);
	SipService.SetHeaderField(kContentType, contentType);
	DEALLOCBUFFER(contentType);

	SipService.SetId(m_id);
//	SipService.SetExpires(expires);

	mcReqService* pSubMsg = SipService.BuildServiceReq();

	if (pSubMsg)
	{
		int size = sizeof(mcReqServiceBase) + pSubMsg->sipHeaders.headersListLength;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)pSubMsg, size);
	SendMsgToCS(m_serviceId, SIP_CS_PROXY_SERVICE_REQ, pSeg);

		PDELETE(pSeg);
		PDELETEA(pSubMsg);
		DEALLOCBUFFER(UserUri);
		DEALLOCBUFFER(Duration);
	}
	else
	{
		PTRACE(eLevelError,"CSipProxyMsPresence::SendService - pSubMsg is NULL could not send SERVICE msg to CS");
		m_state = IDLE;
		DEALLOCBUFFER(UserUri);
		DEALLOCBUFFER(Duration);
		DBGPASSERT_AND_RETURN(2);
	}
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::OnServiceOKSubscribed(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnServiceOKSubscribed  - Credentials List");

	mcIndServiceResp * pServiceResponseMsg = (mcIndServiceResp *)pParam->GetPtr(1);

	CProcessBase *pProcess = CProcessBase::GetProcess();

	WORD udpPortVal = 0;
	WORD tcpPortVal = 0;
	const char* pUserName=NULL;
	const char* pPassword=NULL;
	const char* pDuration=NULL;
	const char* pRelayLocation = NULL;
	const char* pRelayHostName = NULL;
	const char* pUdpPort = NULL;
	const char* pTcpPort = NULL;
	const char *pIceUserName   = NULL;
	WORD  port = 0;
	BOOL IsSucceedToResolve = TRUE;
	BOOL bIsFailure = FALSE;

	CSipHeaderList * pHeaders = new CSipHeaderList(pServiceResponseMsg->sipHeaders);
	if(pHeaders)
	{
		const CSipHeader* pUserHeader = pHeaders->GetNextHeader(kCredUser);
		const CSipHeader* pPasswordHeader = pHeaders->GetNextHeader(kCredPass);
		const CSipHeader* pDurationHeader = pHeaders->GetNextHeader(kCredDuration);
		const CSipHeader* pRelayLocationHeader = pHeaders->GetNextHeader(kMRelayLocation);
		const CSipHeader* pRelayHostNameHeader = pHeaders->GetNextHeader(kMRelayHostName);
		const CSipHeader* pUdpPortHeader = pHeaders->GetNextHeader(kMRelayUdpPort);
		const CSipHeader* pTcpPortHeader = pHeaders->GetNextHeader(kMRelayTcpPort);

		if(pUserHeader)
		{
			pUserName = pUserHeader->GetHeaderStr();
			if(pUserName)
				PTRACE2(eLevelInfoNormal, "CSipProxyMsSubscriber::OnServiceOKSubscribed, UserName : ",pUserName);
		}

		if(pPasswordHeader)
		{
			pPassword = pPasswordHeader->GetHeaderStr();
			if(pPassword)
				PTRACE2(eLevelInfoNormal, "CSipProxyMsSubscriber::OnServiceOKSubscribed, Password : ",pPassword);

		}
		if(pRelayLocationHeader)
		{
			pRelayLocation = pRelayLocationHeader->GetHeaderStr();
			if(pRelayLocation)
				PTRACE2(eLevelInfoNormal, "CSipProxyMsSubscriber::OnServiceOKSubscribed, RelayLocation : ",pRelayLocation);
		}
		if(pRelayHostNameHeader)
		{
			pRelayHostName = pRelayHostNameHeader->GetHeaderStr();
			if(pRelayHostName)
			{
				TRACEINTO << "pRelayHostName:" << pRelayHostName;
				PTRACE2(eLevelInfoNormal, "CSipProxyMsSubscriber::OnServiceOKSubscribed, RelayHostName : ",pRelayHostName);
				if(strstr(pRelayHostName, "edge server resolve failed"))
				{
					IsSucceedToResolve = FALSE;
					PTRACE(eLevelInfoNormal, "CSipProxyMsSubscriber::OnServiceOKSubscribed, RelayHostName - edge server resolve failed ");
					pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,
													AA_SERVICE_REQ_FAILED,
													MAJOR_ERROR_LEVEL,
													"edge server resolve failed on the service request",
																						true,
																						true);

					bIsFailure = TRUE;
				}

			}
		}

		if(pUdpPortHeader)
		{
			pUdpPort = pUdpPortHeader->GetHeaderStr();
			if(pUdpPort)
			{
				udpPortVal = atoi(pUdpPort);
				if(udpPortVal)
					PTRACE2INT(eLevelInfoNormal, "CSipProxyMsSubscriber::OnServiceOKSubscribed, UdpPort : ",udpPortVal);
			}
		}
		if(pTcpPortHeader)
		{
			pTcpPort = pTcpPortHeader->GetHeaderStr();
			if(pTcpPort)
			{
				tcpPortVal = atoi(pTcpPort);
				if(tcpPortVal)
					PTRACE2INT(eLevelInfoNormal, "CSipProxyMsSubscriber::OnServiceOKSubscribed, TcpPort : ",tcpPortVal);
			}
		}

		if(pUserName && pPassword && pDurationHeader && pRelayLocationHeader && pRelayHostName && pRelayHostNameHeader && (tcpPortVal || udpPortVal) && IsSucceedToResolve)
		{
			pProcess->RemoveActiveAlarmFromProcess(AA_SERVICE_REQ_FAILED);

			UpdateCardsMngr(m_id,pUserName,pPassword,pRelayHostName,tcpPortVal,udpPortVal);
		//	StartTimer(TIMER_CM_ICE_INIT, IceInitTimeOut*SECOND);

			//CSipProxyManagerLocalApi pSipProxyMngrApi;
			//pSipProxyMngrApi.UpdateSipProxyWithCredentials(m_id,pUserName,pPassword,pRelayHostName,udpPortVal,tcpPortVal);

			//CSipProxyManagerLocalApi* pSipProxyMngrApi = (CSipProxyManagerLocalApi*)CProcessBase::GetProcess()->GetManagerApi();
			//pSipProxyMngrApi->UpdateSipProxyWithCradentials(m_id,pUserName,pPassword,pRelayHostName,udpPortVal,tcpPortVal);
		}

		POBJDELETE(pHeaders);
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CSipProxyMsSubscriber::OnServiceOKSubscribed, No headers ");

		pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,
								AA_SERVICE_REQ_FAILED,
								MAJOR_ERROR_LEVEL,
								"Received Service message does not contain the Credentials",
																	true,
																	true);
		bIsFailure = TRUE;

	}

	if(bIsFailure)
	{
		TerminateICERegistration();
	}


}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::OnServiceFailedSubscribed(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnServiceFailedSubscribed ");

	CProcessBase *pProcess = CProcessBase::GetProcess();
	pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,
					AA_SERVICE_REQ_FAILED,
					MAJOR_ERROR_LEVEL,
					"Failure response from the A/V Edge Server to the RMX Service Request",
					true,
					true);

	TerminateICERegistration();

}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::UpdateCardsMngr(DWORD id,const char* pUserName,const char* pPassword, const char* pRelayHostName,WORD tcpPort,WORD udpPort)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::UpdateCardsMngr ");

	// ICE_INIT_REQ_S
	ICE_SERVER_TYPES_S	*pParams = new ICE_SERVER_TYPES_S;
	memset(pParams,0,sizeof(ICE_SERVER_TYPES_S));

	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pParams->forced_MS_version = (int)(GetSystemCfgFlagHex<DWORD>("MS_ICE_VERSION"));

	if (pParams->forced_MS_version <= MS_TURN_ICE1)
	{
		if (m_proxyAddress.ipVersion == eIpVersion6)
		{
			pParams->forced_MS_version = MS_TURN_ICE2_SHA256_IPv6;
			PTRACE2INT(eLevelInfoNormal, "CStandIceUser::UpdateCM - setting MS_ICE_VERSION according to IP Type IPv6 ", pParams->forced_MS_version);
		}
		else
		{
			pParams->forced_MS_version = MS_TURN_ICE2;
			PTRACE2INT(eLevelInfoNormal, "CStandIceUser::UpdateCM - setting MS_ICE_VERSION according to IP Type IPv4 ", pParams->forced_MS_version);
		}
	}
	else if (pParams->forced_MS_version > MS_TURN_ICE2_SHA256_IPv6)
	{
		PTRACE2INT(eLevelInfoNormal, "CStandIceUser::UpdateCM - user set invalid value for MS_ICE_VERSION setting to ", MS_TURN_ICE2_SHA256_IPv6);
		pParams->forced_MS_version = MS_TURN_ICE2_SHA256_IPv6;
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal, "CStandIceUser::UpdateCM - user set MS_ICE_VERSION to ", pParams->forced_MS_version);
	}

	pParams->forced_MS_version = htonl(pParams->forced_MS_version);

	pParams->ice_env = eIceEnvMs;
	pParams->req_id = id;
	if (pUserName)
	{
		strncpy(pParams->authParams.user_name, pUserName, IceStrLen - 1);
		pParams->authParams.user_name[IceStrLen - 1] = 0;
	}
	if (pPassword)
	{
		strncpy(pParams->authParams.password, pPassword, IceStrLen - 1);
		pParams->authParams.password[IceStrLen - 1] = 0;
	}
	if (m_DomainName)
	{
		strncpy(pParams->authParams.realm, m_DomainName, IceStrLen - 1);
		pParams->authParams.realm[IceStrLen - 1] = 0;
	}

	pParams->service_id = m_serviceId; //_M_S_

	if (pRelayHostName)
	{
		//if we got FQDN set it without the @
		char *pAtsign  = (char *)strstr(pRelayHostName, "@");

		if (pAtsign != NULL)
		{
			PTRACE2(eLevelInfoNormal,"CSipProxyMsSubscriber::UpdateCardsMngr pAtsign ", pAtsign);
			//ICE_SERVER_PARAMS_S relay_udp_server_params
			int maxLen = min(pAtsign-pRelayHostName, IceStrLen - 1);
			strncpy(pParams->relay_udp_server_params.sIpAddr, pRelayHostName, maxLen);
			pParams->relay_udp_server_params.sIpAddr[maxLen] = 0;
			
			//ICE_SERVER_PARAMS_S relay_tcp_server_params
			strncpy(pParams->relay_tcp_server_params.sIpAddr, pRelayHostName, maxLen);
			pParams->relay_tcp_server_params.sIpAddr[maxLen] = 0;
			(pParams->relay_udp_server_params).port = 0;
			(pParams->relay_tcp_server_params).port = 0;
		}
		else
		{
			//ICE_SERVER_PARAMS_S relay_udp_server_params
			strncpy(pParams->relay_udp_server_params.sIpAddr, pRelayHostName, IceStrLen - 1);
			pParams->relay_udp_server_params.sIpAddr[IceStrLen - 1] = 0;
			(pParams->relay_udp_server_params).port = udpPort;
			//ICE_SERVER_PARAMS_S relay_tcp_server_params
			strncpy(pParams->relay_tcp_server_params.sIpAddr, pRelayHostName, IceStrLen - 1);
			pParams->relay_tcp_server_params.sIpAddr[IceStrLen - 1] = 0;
			(pParams->relay_tcp_server_params).port = tcpPort;
		}

		PTRACE2(eLevelInfoNormal,"CSipProxyMsSubscriber::UpdateCardsMngr pRelayHostName ", pRelayHostName);
	}

	pParams->isEnableBWPolicyCheck = m_IsEnableBWPolicyCheck;
	PTRACE2INT(eLevelInfoNormal, "CSipProxyMsSubscriber::UpdateCardsMngr - pParams->forced_MS_version ", ntohl(pParams->forced_MS_version));

	CSegment*  pSegment = new CSegment;
	pSegment->Put( (BYTE*)pParams, sizeof(ICE_SERVER_TYPES_S) );
	delete pParams;

	CManagerApi apiCards(eProcessCards);
	apiCards.SendMsg(pSegment, SIPPROXY_TO_CARDS_ICE_INIT_REQ);
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::EndIceInitInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::EndIceInitInd ");
	DWORD responseStatus = STATUS_OK;
    WORD NumOfCards = 0;
    DWORD id = 0;
    BYTE atLeastOneBoardSucceed=FALSE;
   // WORD server_status = eIceUnknownProblem;
    DWORD ice_init_status = STATUS_OK;

    CProcessBase *pProcess = CProcessBase::GetProcess();

    *pParam >>NumOfCards;
    PTRACE2INT(eLevelInfoNormal,"CSipProxyMsSubscriber::OnCMIceInitInd - NumOfCards: ",NumOfCards);

	WORD udp_status = eIceInitOk;
	WORD tcp_status = eIceInitOk;
	int  fwType 	= -1;

	ICE_INIT_IND_S* ice_init_array[NumOfCards];

	for(int i=0;i<NumOfCards;i++)
	{
		PTRACE2INT(eLevelInfoNormal,"CSipProxyMsSubscriber::OnCMIceInitInd  service_id: ",m_serviceId);

		ice_init_array[i] = new ICE_INIT_IND_S;
		pParam->Get( (BYTE*)ice_init_array[i], sizeof(ICE_INIT_IND_S) );

		PTRACE2INT(eLevelInfoNormal,"CSipProxyMsSubscriber::OnCMIceInitInd Status:  ",ice_init_array[i]->status);
		PTRACE2INT(eLevelInfoNormal,"CSipProxyMsSubscriber::OnCMIceInitInd Relay_udp_status:  ",ice_init_array[i]->Relay_udp_status);

		if(ice_init_array[i]->status == STATUS_OK)
		{
			udp_status = ice_init_array[i]->Relay_udp_status;
			tcp_status = ice_init_array[i]->Relay_tcp_status;
			fwType = ice_init_array[i]->fw_type;
			if(udp_status==eIceInitOk || tcp_status==eIceInitOk || fwType == eFwTypeBlocked) //Blocked is for workaround - when can't connect to EDGE we don't have to use the relay server
			{
				atLeastOneBoardSucceed = TRUE;
				PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::OnCMIceInitInd - atLeastOneBoardSucceed ");
				m_IsNeedToReSubscribe = FALSE;

				pProcess->RemoveActiveAlarmByErrorCodeUserIdFromProcess(AA_INITIALIZE_ICE_STACK_FAILURE,m_serviceId);//_M_S_
			}
			else
			{
				PTRACE2INT(eLevelInfoNormal,"CSipProxyMsSubscriber::ICE initiation Failed!! - status: ",udp_status);
				SetIceInitActiveAlarm((iceServersStatus)udp_status,m_serviceId);//_M_S_
			}
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::response Ack with status fail- status: ");

			pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,
					AA_INITIALIZE_ICE_STACK_FAILURE,
					MAJOR_ERROR_LEVEL,
					"General Status failure",
					true,
					true);

		}
		PDELETE(ice_init_array[i]);
	}
//	if(IsValidTimer(TIMER_CM_ICE_INIT))
//		DeleteTimer(TIMER_CM_ICE_INIT);

	if(atLeastOneBoardSucceed )
	{
		ice_init_status = STATUS_OK;
		PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::EndIceInitInd - ICE initiation OK!! ");
	}
	else
	{
		ice_init_status = STATUS_FAIL;
		PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::EndIceInitInd - ICE initiation Failed!!");
	}

	//DBGPASSERT(249);

	CSegment *pSeg = new CSegment;
	*pSeg << (BYTE)ice_init_status;
	*pSeg << (BYTE)m_IsEnableBWPolicyCheck;
	*pSeg << (DWORD)m_UcMaxVideoRateAllowed;
	*pSeg << (DWORD)eIceEnvironment_ms;


	CTaskApi api(eProcessConfParty, eManager);
	api.SendMsg(pSeg, SIP_PROXY_TO_CONF_END_INIT_ICE);


}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::TerminateICERegistration()
{
	CSegment *pSeg = new CSegment;
	*pSeg << (BYTE)STATUS_FAIL;
	*pSeg << (BYTE)0;
	*pSeg << (DWORD)0;
	*pSeg << (DWORD)eIceEnvironment_ms;

	CTaskApi api(eProcessConfParty, eManager);
	api.SendMsg(pSeg, SIP_PROXY_TO_CONF_END_INIT_ICE);
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::CMIceInitTimeout(CSegment* pParam)
{

	//UpdateConfPartyOnICEFail();
	//SetActiveAlarm();
}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::HandleCSICEInitResponse(CSegment* pParam,OPCODE opcode)
{
	PTRACE(eLevelInfoNormal,"CSipProxyMsSubscriber::HandleCSICEInitResponse ");
	DispatchEvent(opcode,pParam);


}
////////////////////////////////////////////////////////////////////////////
void CSipProxyMsSubscriber::SetIceInitActiveAlarm(iceServersStatus Status,int i)
{
	char messageDescription[100];
	memset (&messageDescription,0, 100);

	switch(Status)
	{
	case eIceInitServerFail:
		strncpy(messageDescription,"Ice Init Server Fail", strlen("Ice Init Server Fail"));
		break;
	case eIceStunPassServerAuthenticationFailure:
		strncpy(messageDescription,"STUN password server authentication failure", strlen("STUN password server authentication failure"));
		break;
	case eIceStunPassServerConnectionFailure:
		strncpy(messageDescription,"Failed to connect to the STUN password server", strlen("Failed to connect to the STUN password server"));
		break;
	case eIceTurnServerDnsResolveFailure:
		strncpy(messageDescription,"DNS resolution failure for the TURN server",strlen("DNS resolution failure for the TURN server"));
		break;
	case eIceTurnServerUnreachable:
		strncpy(messageDescription,"TURN server is not reachable",strlen("TURN server is not reachable"));
		break;
	case eIceTurnServerAuthorizationFailure:
		strncpy(messageDescription,"Turn Server Authorization Failure",strlen("Turn Server Authorization Failure"));
		break;
	case eIceServerUnavailble:
		strncpy(messageDescription,"Ice Server Unavailble",strlen("Ice Server Unavailble"));
		break;
	case eIceUnknownProblem	:
		strncpy(messageDescription,"Ice Unknown Problem",strlen("Ice Unknown Problem"));
		break;

	default:
		strncpy(messageDescription,"Ice Unknown Problem",strlen("Ice Unknown Problem"));
		break;
	}

	//_M_S_
	char strMessage[100];
	memset (&strMessage,0, 100);
	snprintf(strMessage,sizeof(strMessage), "%s (cs id %d)" , messageDescription, i);

	CProcessBase *pProcess = CProcessBase::GetProcess();
	pProcess->AddActiveAlarmFromProcess(FAULT_GENERAL_SUBJECT,
					AA_INITIALIZE_ICE_STACK_FAILURE,
					MAJOR_ERROR_LEVEL,
					strMessage,
					true,
					false,
					i);
}

DWORD CSipProxyMsSubscriber::ParseHeaderMaxVideoRateAllowed(const char* pRateString)
{
//	ucMaxVideoRateAllowed: Maximum video rate that the UAC endpoints (5) are to use. The maximum video rate MUST be one of the following defined values:
//	default: Maximum resolution of VGA-600K.
//	HD720P-1.5M: Maximum resolution of HD720P. High definition, with a resolution of 1280x720.
//	VGA-600K: Maximum resolution of VGA, 640x480, 25 fps.
//	CIF-250K: <33> Maximum resolution of Common Intermediate Format (CIF). CIF has a resolution of 352x288, 15 fps.
	char tempStr[128];
	char *start;
	DWORD rate = 0;

  strncpy(tempStr, pRateString, sizeof(tempStr)-1);
  tempStr[sizeof(tempStr)-1] = '\0';
	start = strstr(tempStr, "-");
	if( start )	//if '-' exist
	{
	    start++; //we don't need "-"
		PTRACE2(eLevelInfoNormal,"CSipProxyMsSubscriber::ParseHeaderMaxVideoRateAllowed: ", start);
	    if(!strncmp(start, "1.5M", strlen("1.5M")))
	    	rate = 15000;
	    else if(!strncmp(start, "600K", strlen("600K")))
	    	rate = 6000;
	    else if(!strncmp(start, "250K", strlen("250K")))
	    	rate = 2500;
	}
	return rate;
}

