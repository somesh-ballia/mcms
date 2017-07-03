/*
 * SvcEventPackage.cpp
 *
 *  Created on: Aug 29, 2012
 *      Author: bguelfand
 */

#include "SvcEventPackage.h"
#include "SystemFunctions.h"
#include "StatusesGeneral.h"
#include "Observer.h"
#include "zlibengn.h"
#include "IpServiceListManager.h"
#include "IpCommonDefinitions.h"
#include "IpCsOpcodes.h"
#include "MplMcmsProtocolTracer.h"
#include "TraceStream.h"
#include "Conf.h"
#include "IpService.h"

#include <stdlib.h>
#include <arpa/inet.h>
#include "SIPConfEventPKG.h"
#include "psosxml.h"
#include "ApiStatuses.h"
#include "SystemFunctions.h"
#include "StlUtils.h"
#include "Utils.h"

#include "TraceStream.h"
extern CIpServiceListManager* GetIpServiceListMngr();

#define CONF_INFO_NAME	"conference-info"
#define UPDATE_STATE	if(eNoChange == m_state) m_state = ePartialData;
#define	XML_MAX_USERS	8

#define TIMER_SUBSCRIBE_TOUT	100
#define TIMER_CONNECT_REFER		101
#define TIMER_NOTIFY_TOUT		102
#define TIMER_NOTIFY_DELAY		103

#define TIMER_NEXT_NOTIFICATION	105


#define	REFER_NOTIFY_TOUT		1*SECOND
#define SIP_CX_NOTIFY_TOUT		5*SECOND
#define LOAD_TOUT				10*SECOND

#define NEXT_NOTIFICATION_TOUT	10
#define ACK_NOTIFICATION_TOUT	2*SECOND

#define BUF_SIZE	512
#define SIZE_TO_ZIP	2048


// CSvcEventPackageManager states
const WORD   CONNECT        = 1;
const WORD	 TERMINATION	= 2;
const WORD	 EQ_STATE		= 3;

// CSvcSubscriber states
const WORD	 SENT_FULL	= 2;
const WORD	 SENT_LAST	= 3;
const WORD	 WAIT_ACK_FULL	= 4;
const WORD	 WAIT_ACK_LAST	= 5;
const WORD	 SUBSCRIBER_DISCONNECTED	= 6;

static CCloudInfo glbCloudInfo;

enum{
	eUnknownEvent = 0,
	eConferenceEvent,
	eReferEvent,
	eSipCx
}typedef eSubscribeEventType;


// ************************************************************************************
//
//	CSvcEventPackageDispatcher
//
// ************************************************************************************

///////////////////////////////////////////////////////////////////
CSvcEventPackageDispatcher::CSvcEventPackageDispatcher(CConf* pConf, COsQueue* pRcvMbx, CMplMcmsProtocol* pProtocol, CConfPartyManagerLocalApi* pConfPartyManagerApi)
			: m_terminating(FALSE), m_pMplMcmsProtocol(pProtocol)
{
	m_pSvcConfPackage = new CSvcEventPackageManager(pConf, pRcvMbx, pProtocol, pConfPartyManagerApi);
}

///////////////////////////////////////////////////////////////////
CSvcEventPackageDispatcher::~CSvcEventPackageDispatcher()
{
	POBJDELETE(m_pSvcConfPackage);
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageDispatcher::SetIsEntryQueue(bool bEQ)
{
	m_pSvcConfPackage->SetIsEntryQueue(bEQ);
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageDispatcher::Subscribe(mcIndSubscribe* pSubscribeInd, DWORD callIndex, WORD srcUnitId, DWORD serviceId)
{
	CSipHeaderList* pTemp = new CSipHeaderList(pSubscribeInd->sipHeaders);
	eSubscribeEventType eventType = eUnknownEvent;

	char* pUserUri = NULL, *pDestUri = NULL, *pToTagStr = NULL;
	char* pEventName = NULL;
	char* pAcceptName = NULL;

	//**** Check event type to handle request according to it ****
	const CSipHeader* pEvent = pTemp->GetNextHeader(kEvent);
	if (pEvent)
	{
		PTRACE(eLevelInfoNormal, "CSvcEventPackageDispatcher::Subscribe - pEvent");
		pEventName = (char*)pEvent->GetHeaderStr();
		if (pEventName)
		{
			PTRACE2(eLevelInfoNormal, "CSvcEventPackageDispatcher::Subscribe - pEventName = ", pEventName);
			const char* pToStr = NULL;
			const CSipHeader* pTo = pTemp->GetNextHeader(kTo);
			if (pTo)
			{
				pToStr = pTo->GetHeaderStr();
				PTRACE2(eLevelInfoNormal, "CSvcEventPackageDispatcher::Subscribe - pToStr = ", pToStr);
				if (pToStr)
				{
					if (!strncmp(pEventName, "conference", H243_NAME_LEN_OLD))
					{
						PTRACE(eLevelInfoNormal, "CSvcEventPackageDispatcher::Subscribe - ConferenceEvent");
						eventType = eConferenceEvent;
						m_pSvcConfPackage->SetConfName(pToStr);
					}
				}
			}
		}
	}
	else
		PTRACE(eLevelError, "CSvcEventPackageDispatcher::Subscribe - pEvent = NULL");

	//**** if conference is terminating, reject request ****
	if (m_terminating)
	{
		switch (eventType)
		{
			case (eConferenceEvent):
				SendSipSubscribeResponse(pSubscribeInd, SipCodesNotAcceptable, callIndex, serviceId);
				break;
			default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;
		}
	}
	else
	{
		switch (eventType)
		{
			case (eConferenceEvent):
				PTRACE(eLevelInfoNormal, "CSvcEventPackageDispatcher::Subscribe - m_pSvcConfPackage->Subscribe()");
				m_pSvcConfPackage->Subscribe(pSubscribeInd, callIndex, srcUnitId, serviceId);
				break;
			default:
				SendSipSubscribeResponse(pSubscribeInd, SipCodesBadRequest, callIndex, serviceId);
				break;
		}
	}

	POBJDELETE(pTemp);
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageDispatcher::SendSipSubscribeResponse(mcIndSubscribe* pSubscribeInd, enSipCodes sipCode, DWORD callIndex, WORD srcUnitId, DWORD serviceId)
{
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageDispatcher::OnIndNotifyResp(mcIndNotifyResp* pIndNotifyResp)
{
	m_pSvcConfPackage->OnIndNotifyResp(pIndNotifyResp);
}

///////////////////////////////////////////////////////////////////
CSvcSubscriber* CSvcEventPackageDispatcher::FindParty(char* from, DWORD CSeq, char* callId)
{
	PTRACE2(eLevelInfoNormal,"CSvcEventPackageDispatcher::FindParty named = ", from);
	CSvcSubscriber* pSubscriber = NULL;
	pSubscriber = m_pSvcConfPackage->FindParty(from);
	return pSubscriber;
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageDispatcher::Dump()
{
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageDispatcher::TerminateConf()
{
	m_terminating = TRUE;
	m_pSvcConfPackage->TerminateConf();
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageDispatcher::LoadManagerAccept()
{
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageDispatcher::HandleObserverUpdate(CSegment *pSeg, WORD type)
{
	void* pSubscriber = NULL;
	WORD event = 0;
	DWORD val = 0;
	DWORD partyId = 0;

	BYTE bCheckCX = FALSE;

	*pSeg >> (DWORD&)pSubscriber >> event >> val;

	switch (type)
	{
		case SIP_CX_PACKAGE:
		{
			break;
		}
		case SIP_EVENT_PACKAGE:
		{
			m_pSvcConfPackage->ObserverUpdate(pSubscriber, event, val);
			break;
		}
		case (SIP_REFER):
		{
			break;
		}
		default:
		{
			PTRACE(eLevelError, "CSvcEventPackageDispatcher::HandleObserverUpdate, invalid pointer");
			DBGPASSERT(1);
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////
CSvcSubscriber* CSvcEventPackageDispatcher::FindPartyInSIPCX(const char* pFromStr)
{
	return NULL;
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageDispatcher::ResetNotifyTimer()
{
	m_pSvcConfPackage->ResetNotifyTimer();
}

bool CSvcEventPackageDispatcher::DisconnectSubscriberByRsrcID(DWORD dwPartyRsrcID, CSegment& seg)
{
	return m_pSvcConfPackage->DisconnectSubscriberByRsrcID(dwPartyRsrcID, seg);
}

bool CSvcEventPackageDispatcher::ConnectSubscriberByRsrcID(DWORD dwPartyRsrcID)
{
	return m_pSvcConfPackage->ConnectSubscriberByRsrcID(dwPartyRsrcID);
}

bool CSvcEventPackageDispatcher::DeleteSubscriberByRsrcID(DWORD dwPartyRsrcID)
{
	return m_pSvcConfPackage->DeleteSubscriberByRsrcID(dwPartyRsrcID);
}

bool CSvcEventPackageDispatcher::UnchainSubscriberByRsrcID(DWORD dwPartyRsrcID, CSegment& seg)
{
	return m_pSvcConfPackage->UnchainSubscriberByRsrcID(dwPartyRsrcID, seg);
}

bool CSvcEventPackageDispatcher::UnchainSubscriberByMonitorID(DWORD dwPartyMonitorId, CSegment& seg)
{
	return m_pSvcConfPackage->UnchainSubscriberByMonitorID(dwPartyMonitorId, seg);
}

bool CSvcEventPackageDispatcher::ChainSubscriber(CSegment& seg,DWORD dwNewPartyMonitorId)
{
	return m_pSvcConfPackage->ChainSubscriber(seg, dwNewPartyMonitorId);
}
void CSvcEventPackageDispatcher::PartyLeaveIVR(DWORD dwPartyRsrcID)
{
	m_pSvcConfPackage->PartyLeaveIVR(dwPartyRsrcID);
}

// ************************************************************************************
//
//	CSvcEventPackageManager
//
// ************************************************************************************

// start message map -------------------------------------------
PBEGIN_MESSAGE_MAP(CSvcEventPackageManager)

	ONEVENT(TIMER_SUBSCRIBE_TOUT	,CONNECT	,CSvcEventPackageManager::OnSubscribeTout )
	ONEVENT(TIMER_SUBSCRIBE_TOUT	,EQ_STATE	,CSvcEventPackageManager::NullActionFunction )
	ONEVENT(TIMER_SUBSCRIBE_TOUT	,ANYCASE	,CSvcEventPackageManager::NullActionFunction )
	ONEVENT(TIMER_NOTIFY_TOUT		,CONNECT	,CSvcEventPackageManager::OnNotifyTimerTout )
	ONEVENT(TIMER_NOTIFY_TOUT		,ANYCASE	,CSvcEventPackageManager::NullActionFunction )
	ONEVENT(TIMER_NOTIFY_DELAY		,CONNECT	,CSvcEventPackageManager::OnNotifyDelay )
	ONEVENT(TIMER_NOTIFY_DELAY		,ANYCASE	,CSvcEventPackageManager::NullActionFunction )


PEND_MESSAGE_MAP(CSvcEventPackageManager,CStateMachine);
// end   message map -------------------------------------------

///////////////////////////////////////////////////////////////////
CSvcEventPackageManager::CSvcEventPackageManager(CConf* pConf, COsQueue* pRcvMbx, CMplMcmsProtocol* pMplMcmsProtocol, CConfPartyManagerLocalApi* pConfPartyManagerApi, CConfApi* pConfApi)
{
	m_state = CONNECT;
	m_pConf = pConf;
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	if (pCommConf->GetEntryQ())
		m_state = EQ_STATE;
	m_SubTimer = FALSE;
	m_NotifyTimer = FALSE;
	m_LoadTimer = FALSE;
	m_pNotifyMsg = NULL;
	m_pEventConfInfo = NULL;
	m_pXMLStr = NULL;
	m_version = 0;
	m_pConfName = new char[H243_NAME_LEN];
	*m_pConfName = '\0';
	m_pMplMcmsProtocol = pMplMcmsProtocol;
	m_pConfPartyManagerApi = pConfPartyManagerApi;
	m_pConfApi = pConfApi;

//BG
	TRACEINTO << "\n\tpConf = " << pConf << "\n\tConfRsrcID = " << pConf->GetConfId() << "\n\tIsEntryQueue = " << (m_state == EQ_STATE);
	strncpy(m_pConfName, pConf->GetName(), H243_NAME_LEN);
	m_pEventConfInfo = new CSvcConfInfoType(pConf, "xcon:1001@polycom.co.il", "");

	char *XML = NULL;
	DWORD length = 0;
	CXMLDOMElement *pElement = new CXMLDOMElement;
	m_pEventConfInfo->SerializeXml(pElement, TRUE, TRUE);

	CObjString *pStr = NULL;
	pElement->DumpDataAsStringWithAttribute(&XML, &length);
	pStr = new CObjString(XML, length + 20);
	PTRACE2(eLevelInfoNormal, "CSvcEventPackageManager::CSvcEventPackageManager - m_pEventConfInfo XML:\n", pStr->GetString());
	PDELETE(pElement);
	PDELETEA(XML);
	POBJDELETE(pStr);

	if (pRcvMbx)
	{
		m_pRcvMbx = pRcvMbx;
	}
	else
		m_pRcvMbx = NULL;
	//if no mock object is inserted
	if (!pMplMcmsProtocol)
		m_runInTdd = FALSE;
	else
		m_runInTdd = TRUE;

	pCommConf->AttachObserver(m_pRcvMbx, CONF_ACTIVE, SIP_EVENT_PACKAGE);
	pCommConf->AttachObserver(m_pRcvMbx, PARTY_ADDED, SIP_EVENT_PACKAGE);
	pCommConf->AttachObserver(m_pRcvMbx, PARTY_DELETED, SIP_EVENT_PACKAGE);
}

///////////////////////////////////////////////////////////////////
CSvcEventPackageManager::~CSvcEventPackageManager()
{
	DeleteAllTimers();
	PDELETEA(m_pConfName);
	POBJDELETE(m_pXMLStr);
	PDELETEA(m_pNotifyMsg);

	std::vector<CSvcSubscriber *>::iterator it = m_EventSubscribersList.begin();
	for (; it != m_EventSubscribersList.end(); ++it)
		POBJDELETE(*it);

	POBJDELETE(m_pEventConfInfo);
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode)
{
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::SetIsEntryQueue(bool bEQ)
{
	if (bEQ)
	{
		m_state = EQ_STATE;
		TRACEINTO << "EntreQueue conference\n\tConfRsrcID = " << m_pConf->GetConfId();
	}
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::Subscribe(mcIndSubscribe* pSubscribeInd, DWORD callIndex, WORD srcUnitId, DWORD serviceId)
{
	WORD boardId = 1;
	enSipCodes SipStatus = SipCodesBadRequest;
	BYTE bValidReq = TRUE, acceptVerified = TRUE, bLooseMcuApi = FALSE, bSendNotify = FALSE, bReferWithBye = FALSE;
	BYTE bNotifyWasSent = FALSE, bIsNewSubscriber = FALSE;
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	TRACECOND_AND_RETURN(!IsValidPObjectPtr(pCommConf), "CSvcEventPackageManager::Subscribe - pCommConf == NULL");

	CSvcSubscriber* pSubscriber = NULL;

	std::auto_ptr<CSipHeaderList> pTemp(new CSipHeaderList(pSubscribeInd->sipHeaders));

	bValidReq = CheckRequestValidity(pSubscribeInd, &SipStatus);

	const CSipHeader* pFrom = pTemp->GetNextHeader(kFrom);
	const char *pFromStr = (pFrom) ? pFrom->GetHeaderStr() : NULL;
	PASSERT_AND_RETURN(!pFromStr);

	const CSipHeader* pFromTag = pTemp->GetNextHeader(kFromTag);
	const char* pFromTagStr = (pFromTag) ? pFromTag->GetHeaderStr() : NULL;

	const CSipHeader* pTo = pTemp->GetNextHeader(kTo);
	const char *pToStr = (pTo) ? pTo->GetHeaderStr() : NULL;

	const CSipHeader* pToTag = pTemp->GetNextHeader(kToTag);
	const char *pToTagStr = (pToTag) ? pToTag->GetHeaderStr() : NULL;

	const CSipHeader* pCallId = pTemp->GetNextHeader(kCallId);
	const char *pCallIdStr = (pCallId) ? pCallId->GetHeaderStr() : NULL;

	const CSipHeader* pContact = pTemp->GetNextHeader(kContact);
	const char *pContactStr = (pContact) ? pContact->GetHeaderStr() : NULL;

	if (pSubscribeInd->expires > 3600)
		pSubscribeInd->expires = 3600;

	PartyRsrcID partyRsrcID = -1;
	PartyMonitorID partyMonitorID = -1;

	if (bValidReq)
	{
		PTRACE(eLevelInfoNormal, "CSvcEventPackageManager::Subscribe - bValidReq = true");
		//if party exists
		pSubscriber = FindParty(pFromStr);
		if (IsValidPObjectPtr(pSubscriber))
		{
			//unsubscribe
			if (0 == pSubscribeInd->expires)
			{
				PTRACE2(eLevelInfoNormal, "CSvcEventPackageManager::Subscribe, unsubscribing party = ", pFromStr);
				RemoveFromVector(pSubscriber);
				SipStatus = SipCodesOk;
				POBJDELETE(pSubscriber);
				TRACESTR (eLevelInfoNormal) << "CSvcEventPackageManager::Subscribe, num of subscribers =  " << m_EventSubscribersList.size();
			}
			//refresh expires Tout.
			else
			{
				char s[10];
				snprintf(s, sizeof(s), "%ld", pSubscribeInd->expires);
				PTRACE2(eLevelInfoNormal, "CSvcEventPackageManager::Subscribe, refreshing subscription, expires = ", s);
				pSubscriber->Refresh(pSubscribeInd->transportAddress, pCallIdStr, pSubscribeInd->expires);
				SipStatus = SipCodesOk;
				bSendNotify = TRUE;
				bIsNewSubscriber = TRUE;
				pSubscriber->SetNotificationVersion(0);
				partyRsrcID = pSubscriber->GetPartyRsrcID();
			}
		}
		//new subscriber
		else
		{
			//unsubscribe is ok even if party is not found.
			if (0 == pSubscribeInd->expires)
			{
				PTRACE(eLevelInfoNormal, "CSvcEventPackageManager::Subscribe, unsubscribing party is not found.");
				SipStatus = SipCodesOk;
			}
			else
			{
				PTRACE2(eLevelInfoNormal, "CSvcEventPackageManager::Subscribe - new subscriber, m_pConfName = ", m_pConfName);

				CConfParty* pConfParty = pCommConf->GetFirstParty();
				while (pConfParty)
				{
					const char* pSipUri = pConfParty->GetSipPartyAddress();
					if (strncmp(pSipUri, pFromStr, IP_STRING_LEN) == 0)
					{
						partyMonitorID = pConfParty->GetPartyId();
						CPartyCntl* pPartyCtrl = m_pConf->GetPartyCntl(pConfParty->GetName());
						if (pPartyCtrl)
						{
							partyRsrcID = pPartyCtrl->GetPartyId();
							m_pEventConfInfo->AddParty(pConfParty, partyRsrcID, TRUE);
							break;
						}
					}
					pConfParty = pCommConf->GetNextParty();
				}

				PASSERTSTREAM_AND_RETURN(!pConfParty, "ConfId:" << m_pConf->GetConfId() << ", FromStr:" << pFromStr << " - party not found");

				if (pConfParty->GetPartyMediaType() != eSvcPartyType)
				{
					PTRACE(eLevelInfoNormal, "CSvcEventPackageManager::Subscribe - pConfParty->GetPartyMediaType() != eSvcPartyType");
					return;
				}

				PASSERT_AND_RETURN(!pFromStr);
				PASSERT_AND_RETURN(!pFromTagStr);
				PASSERT_AND_RETURN(!pToStr);
				PASSERT_AND_RETURN(!pToTagStr);
				PASSERT_AND_RETURN(!pCallIdStr);
				PASSERT_AND_RETURN(!pContactStr);

				TRACEINTO << "New subscriber party is added:" << "\n\t PartyName = " << pConfParty->GetName() << "\n\t PartyRsrcID = " << partyRsrcID << "\n\t PartyMonitorID = " << partyMonitorID << "\n\t pFromStr = " << pFromStr << "\n\t pFromTagStr = " << pFromTagStr << "\n\t pToStr = " << pToStr << "\n\t pToTagStr = " << pToTagStr << "\n\t pCallIdStr = " << pCallIdStr << "\n\t pContactStr = " << pContactStr << "\n\t transportAddress.transAddr = " << Utils::IpAddressToString(pSubscribeInd->transportAddress.transAddr, true).c_str() << "\n\t transportAddress.transAddr.port = " << pSubscribeInd->transportAddress.transAddr.port << "\n\t transportAddress.transAddr.transportType = " << pSubscribeInd->transportAddress.transAddr.transportType << "\n\t expires = " << pSubscribeInd->expires
				    << "\n\t serviceId = " << serviceId;

				pSubscriber = new CSvcSubscriber(this, partyRsrcID, partyMonitorID, boardId, pFromStr, pFromTagStr, pToStr, pToTagStr, pContactStr, pCallIdStr, pSubscribeInd->transportAddress, pSubscribeInd->expires, srcUnitId, callIndex, eParticipant, serviceId);
				m_EventSubscribersList.push_back(pSubscriber);
				SipStatus = SipCodesOk;
				bSendNotify = TRUE;
				bIsNewSubscriber = TRUE;
				pSubscriber->SetNotificationVersion(0);
			}
		}
		//recalculate timer in case of: new subscribe, refresh, unsubscribe
		if (m_SubTimer)
			DeleteTimer(TIMER_SUBSCRIBE_TOUT);
		WORD duration = CalcTimer();
		if (duration > 0)
		{
			if (!m_runInTdd)
				StartTimer(TIMER_SUBSCRIBE_TOUT, SECOND * duration);
			m_SubTimer = TRUE;
		}
	}
	else
		PTRACE(eLevelInfoNormal, "CSvcEventPackageManager::Subscribe, Invalid request is rejected.");

	if (SipCodesOk != SipStatus)
		pSubscribeInd->expires = 0;
	SendSipSubscribeResponse(pSubscribeInd, SipStatus, callIndex, srcUnitId, serviceId);

	if (bSendNotify && SipCodesOk == SipStatus && m_state != EQ_STATE)
	{
		CSvcUserType* pUser = m_pEventConfInfo->FindUserByRsrcID(partyRsrcID);
		if (pUser && !pUser->IsInIVR())
		{
			pSubscriber->StartFullNotification();
		}
	}
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::OnIndNotifyResp(mcIndNotifyResp* pIndNotifyResp)
{
	CSvcSubscriber* pSubscriber = FindSubscriberByRsrcID((DWORD)pIndNotifyResp->id);
	if (pSubscriber)
		pSubscriber->OnIndNotifyResp((DWORD)pIndNotifyResp->status);
	else
		PTRACE2INT(eLevelInfoNormal, "CSvcEventPackageManager::OnIndNotifyResp - Subscriber not found!!!\\n\tPartyRsrcID = ", (DWORD )pIndNotifyResp->id);
}

///////////////////////////////////////////////////////////////////
int CSvcEventPackageManager::NotifySubscriberFull(CSvcSubscriber* pSubscriber, long iLastUser, int iNotificationVersion, int iXmlMaxUsers)
{
	std::string sFullStr("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");
	long iNewLastUser = m_pEventConfInfo->SerializeXmlFull(&sFullStr, iLastUser, iNotificationVersion, iXmlMaxUsers);
	TRACEINTO << "CSvcEventPackageManager::NotifySubscriberFull\nPrevLastUser = " << iLastUser << "\nNewLastUser = " << iNewLastUser << "\niXmlMaxUsers = " << iXmlMaxUsers;
	if (iNewLastUser != EMPTY_LIST)
		Notify(pSubscriber, (char*)(sFullStr.c_str()), "active");
	return iNewLastUser;
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::SendSipSubscribeResponse(mcIndSubscribe* pSubscribeInd, enSipCodes sipCode, DWORD callIndex, WORD srcUnitId, DWORD cs_Id)
{
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::SendSipNotify(DWORD dwPartyRsrcID, mcReqNotify* pNotifyMsg, WORD srcUnitId, DWORD callIndex, DWORD cs_Id)
{
	DWORD size = sizeof(mcReqNotifyBase) + pNotifyMsg->sipContentAndHeaders.lenOfDynamicSection;
	CSegment* pSeg = new CSegment;
	*pSeg << callIndex << srcUnitId;
	*pSeg << size;
	pSeg->Put((BYTE*)pNotifyMsg, size);
	SendMsgToCS(dwPartyRsrcID, cs_Id, SIP_CS_PROXY_NOTIFY_REQ, pSeg);

	POBJDELETE(pSeg);
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::OnNotifyDelay(CSegment* pParam)
{
	TRACEINTO_GLA << "Queue size = " << m_queueCS.size();
	for (int i = 0; i < 5; ++i)
	{
		if (m_queueCS.size() > 0)
		{
			NOTIFY_MSG_TO_CS_STRUCT msg = m_queueCS.front();
			SendMsgToCS(msg.dwPartyRsrcID, msg.dwCsID, SIP_CS_PROXY_NOTIFY_REQ, msg.pSegment);
			m_queueCS.pop();

			POBJDELETE(msg.pSegment);
		}
		else
		{
			break;
		}
	}
	if (m_queueCS.size() > 0)
		StartTimer(TIMER_NOTIFY_DELAY, NEXT_NOTIFICATION_TOUT);
	else
		DeleteTimer(TIMER_NOTIFY_DELAY);
}

/////////////////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::SendMsgToCS(DWORD dwPartyRsrcID, DWORD cs_Id, OPCODE opcode, CSegment* pseg1)
{
	CMplMcmsProtocol *pMplMcmsProtocol = NULL;
	DWORD callIndex = 0;
	WORD srcUnitId = 0;
	//if TDD
	if (m_pMplMcmsProtocol)
		pMplMcmsProtocol = m_pMplMcmsProtocol;
	else
		pMplMcmsProtocol = new CMplMcmsProtocol;

	if (!pMplMcmsProtocol)
	{
		PASSERT(101);
		return;
	}
	pMplMcmsProtocol->AddCommonHeader(opcode);
	pMplMcmsProtocol->AddMessageDescriptionHeader();
	pMplMcmsProtocol->AddPortDescriptionHeader(dwPartyRsrcID, m_pConf->GetConfId());
	if (pseg1)
	{
		*pseg1 >> callIndex >> srcUnitId;
		DWORD size = 0;
		*pseg1 >> size;
		BYTE* pMessage = new BYTE[size];
		pseg1->Get(pMessage, size);

		pMplMcmsProtocol->AddData(size, (const char*)pMessage);
		PDELETEA(pMessage);
	}
	pMplMcmsProtocol->AddCSHeader(cs_Id, 0, srcUnitId, callIndex);
	pMplMcmsProtocol->AddPayload_len(CS_API_TYPE);
	CMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("CSvcEventPackageManager::SendMsgToCS ", CS_API_TYPE);
	pMplMcmsProtocol->SendMsgToCSApiCommandDispatcher();

	if (!m_pMplMcmsProtocol)
		PDELETE(pMplMcmsProtocol);
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::RemoveFromVector(CSvcSubscriber* pSubscriber)
{
	CSvcSubscriber* pTempSubscriber = NULL;
	std::vector<CSvcSubscriber *>::iterator itr = m_EventSubscribersList.begin();
	while (itr != m_EventSubscribersList.end())
	{
		pTempSubscriber = (*itr);
		if (!strncmp(pSubscriber->GetCallId(), pTempSubscriber->GetCallId(), MaxAddressListSize))
		{
			m_EventSubscribersList.erase(itr);
			TRACEINTO << "Remove subscriber:\n\tPartyRsrcID = " << pSubscriber->GetPartyRsrcID() << "\n\tCall-ID = " << pSubscriber->GetCallId();
			break;
		}
		++itr;
	}
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::OnNotifyTimerTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSvcEventPackageManager::OnNotifyTimerTout");
	DeleteTimer(TIMER_NOTIFY_TOUT);
	m_NotifyTimer = FALSE;

	if (m_pEventConfInfo && m_pEventConfInfo->WasUpdated())
	{
		m_pXMLStr = BuildNotifyContent(FALSE);

		if (m_pXMLStr)
		{
			PTRACE(eLevelInfoNormal, "CSvcEventPackageManager::OnNotifyTimerTout - XML exists");
			CSvcSubscriber* pSubscriber = NULL;
			std::vector<CSvcSubscriber *>::iterator itr = m_EventSubscribersList.begin();
			std::string sXml(m_pXMLStr->GetString());
			while (itr != m_EventSubscribersList.end())
			{
				pSubscriber = (*itr);
				if (IsValidPObjectPtr(pSubscriber))
				{
					bool bSendNow = pSubscriber->AddTimerNotification(sXml);
					if (bSendNow)
					{
						SetNotificationVersion(sXml, pSubscriber->GetNotificationVersion());
						Notify(pSubscriber, (char*)sXml.c_str(), "active", true);
					}
				}
				++itr;
			}
		}

		POBJDELETE(m_pXMLStr);
	}
}

///////////////////////////////////////////////////////////////////
const char* CSvcEventPackageManager::EventToString(const WORD event)
{
	switch (event)
	{
		case (PARTYSTATE):
			return "PARTYSTATE";
		case (CHAIROWNER):
			return "CHAIROWNER";
		case (DISCAUSE):
			return "DISCAUSE";
		case (MUTE_STATE):
			return "MUTE_STATE";
		case (CONTACT_INFO):
			return "CONTACT_INFO";
		case (CONF_LOCK):
			return "CONF_LOCK";
		case (CONF_ACTIVE):
			return "CONF_ACTIVE";
		case (AUDIOSRC):
			return "AUDIOSRC";
		case (PARTY_ADDED):
			return "PARTY_ADDED";
		case (PARTY_DELETED):
			return "PARTY_DELETED";
		case (MEDIA):
			return "MEDIA";
		case (MEDIA_REMOVE):
			return "MEDIA_REMOVE";
		default:
			return "UNKNOWN_EVENT";
	}
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::AttachEvents(CCommConf *pCommConf)
{
	PTRACE(eLevelInfoNormal, "CSvcEventPackageManager::AttachEvents");

	CConfParty* pConfParty = pCommConf->GetFirstParty();
	while (IsValidPObjectPtr(pConfParty))
		pConfParty = pCommConf->GetNextParty();
}

///////////////////////////////////////////////////////////////////
void   CSvcEventPackageManager::DetachEvents(CCommConf *pCommConf)
{
	TRACEINTO;
	//Conf events
	pCommConf->DetachObserver(m_pRcvMbx);
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::ObserverUpdate(void* pSubscriber, WORD event, DWORD val)
{
	if (m_state == EQ_STATE && event != PARTY_DELETED)
	{
		TRACEINTO << "\n\tConfRsrcID = " << m_pConf->GetConfId() << "\n\tIsEntryQueue = " << (m_state == EQ_STATE);
		return;
	}

	DWORD partyId = (DWORD)pSubscriber;

	TRACEINTO << "\nPartyId = " << partyId << "\nevent = " << event << "," << EventToString(event) << "\nval = " << val;

	if (!m_pEventConfInfo)
		return;

	bool bUrgent = false;
	CSvcUserType* pUserUrgentUpdated = NULL;

	CConfParty *pConfParty = NULL;
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();

	switch (event)
	{
		case MEDIA:
		case MEDIA_REMOVE:
		{
			if (IsValidPObjectPtr(pCommConf))
				pConfParty = pCommConf->GetCurrentParty(partyId);
			else
				TRACESTR(eLevelError) << "CSvcEventPackageManager::ObserverUpdate - case: " << EventToString(event) << "\nConference not found in DB.";

			if (IsValidPObjectPtr(pConfParty))
			{
				CPartyCntl* pPartyCtrl = m_pConf->GetPartyCntl(pConfParty->GetName());
				DWORD dwPartyRsrcID = 0;

				if (IsValidPObjectPtr(pPartyCtrl))
				{
					dwPartyRsrcID = pPartyCtrl->GetPartyId();
					m_setAddedParticipantsRsrcIDs.insert(dwPartyRsrcID);
				}

				bool bChanged = m_pEventConfInfo->UpdateMedia(pConfParty, dwPartyRsrcID, bUrgent, &pUserUrgentUpdated);

				TRACEINTO << "CSvcEventPackageManager::ObserverUpdate - case: " << EventToString(event) << "\nbUrgent = " << (bUrgent ? "true" : "false") << ",  bChanged = " << (bChanged ? "true" : "false");

				pConfParty->SetMediaListUpdated();

				if (!bChanged)
					return;
			}
			else
				TRACESTR(eLevelError) << "CSvcEventPackageManager::ObserverUpdate - case: " << EventToString(event) << "\nParticipant not found in DB.";

			break;
		}

		case (PARTYSTATE):
		{
			if (IsValidPObjectPtr(pCommConf))
				pConfParty = pCommConf->GetCurrentParty(partyId);
			else
				PTRACE(eLevelError, "CSvcEventPackageManager::ObserverUpdate, conf not found in DB.");

			if (IsValidPObjectPtr(pConfParty))
			{
				m_pEventConfInfo->SetEndPointStatus(partyId, pConfParty->GetPartyState());
				if (PARTY_DISCONNECTED == pConfParty->GetPartyState() && !strcmp(m_pEventConfInfo->GetActiveSpeaker(), pConfParty->GetName()))
				{
					m_pEventConfInfo->SetActiveSpeaker(partyId, "");
				}
			}
			else
				PTRACE(eLevelError, "CSvcEventPackageManager::ObserverUpdate, party not found in DB.");

			break;
		}

		case (CONTACT_INFO):
		{
			if (IsValidPObjectPtr(pCommConf))
				pConfParty = pCommConf->GetCurrentParty(partyId);
			else
				PTRACE(eLevelError, "CSvcEventPackageManager::ObserverUpdate, conf not found in DB.");

			if (IsValidPObjectPtr(pConfParty))
			{
				PTRACE(eLevelInfoNormal, "CSvcEventPackageManager::ObserverUpdate, CONTACT_INFO");
				if (0 == val)
				{
					m_pEventConfInfo->SetUserContactInfo(partyId, pConfParty->GetUserDefinedInfo(0));
				}
			}
			else
				PTRACE(eLevelError, "CSvcEventPackageManager::ObserverUpdate, party not found in DB.");

			break;
		}

		case (MUTE_STATE):
		{
			if (IsValidPObjectPtr(pCommConf))
				pConfParty = pCommConf->GetCurrentParty(partyId);

			if (IsValidPObjectPtr(pConfParty))
			{
				PTRACE(eLevelInfoNormal, "CSvcEventPackageManager::ObserverUpdate, MUTE_STATE");
				//Since Istanbul expects all media types to be muted all together
				//we mark all muted once one is received.
				BYTE onOff = FALSE;
				BYTE mutedViaFocus = FALSE;
				if (eAudio == val)
				{
					onOff = pConfParty->IsAudioMutedByParty();
					m_pEventConfInfo->MutePartyMedia(partyId, eAudio, onOff);

					mutedViaFocus = pConfParty->IsAudioMutedByOperator();
					mutedViaFocus = mutedViaFocus | pConfParty->IsAudioMutedByMCU();
					m_pEventConfInfo->MuteViaFocus(partyId, eAudio, mutedViaFocus);

					if ((true == onOff || true == mutedViaFocus) && !strcmp(m_pEventConfInfo->GetActiveSpeaker(), pConfParty->GetName()))
					{
						m_pEventConfInfo->SetActiveSpeaker(partyId, "");
					}
				}
				if (eVideoT == val)
				{
					onOff = pConfParty->IsVideoMutedByParty();
					m_pEventConfInfo->MutePartyMedia(partyId, eVideoT, onOff);

					mutedViaFocus = pConfParty->IsVideoMutedByOperator();
					mutedViaFocus = mutedViaFocus | pConfParty->IsVideoMutedByMCU();
					m_pEventConfInfo->MuteViaFocus(partyId, eVideoT, mutedViaFocus);
				}
			}
			break;
		}

		case (CONF_ACTIVE):
		{
			m_pEventConfInfo->SetActive(val);
			break;
		}

		case (AUDIOSRC):
		{
			if (IsValidPObjectPtr(pCommConf))
				pConfParty = pCommConf->GetCurrentParty(val);

			if (IsValidPObjectPtr(pConfParty))
			{
				PTRACE2(eLevelInfoNormal, "CSvcEventPackageManager::ObserverUpdate, SetActiveSpeaker = ", pConfParty->GetName());
				m_pEventConfInfo->SetActiveSpeaker(pConfParty->GetPartyId(), pConfParty->GetName());
			}
			break;
		}

		case (PARTY_ADDED):
		{
			pConfParty = pCommConf->GetCurrentParty(val);

			if (IsValidPObjectPtr(pConfParty))
			{
				BOOL bPcasWebFlow = FALSE;
				if (0 == strncmp(m_pConfName, "RAS200I_web_", 12))
					bPcasWebFlow = TRUE;

				BOOL bIsWebDialInParty = FALSE;
				if (bPcasWebFlow && DIAL_IN == pConfParty->GetConnectionType())
					bIsWebDialInParty = TRUE;

				CPartyCntl* pPartyCtrl = m_pConf->GetPartyCntl(pConfParty->GetName());
				DWORD dwPartyRsrcID = 0;
				if (IsValidPObjectPtr(pPartyCtrl))
				{
					dwPartyRsrcID = pPartyCtrl->GetPartyId();
					m_setAddedParticipantsRsrcIDs.insert(dwPartyRsrcID);
				}
				m_pEventConfInfo->AddParty(pConfParty, dwPartyRsrcID, bIsWebDialInParty);

				partyId = pConfParty->GetPartyId();
			}
			else
				PTRACE(eLevelError, "CSvcEventPackageManager::ObserverUpdate, party not found.");

			break;
		}

		case (PARTY_DELETED):
		{
			m_pEventConfInfo->DelParty(val/*pConfParty->GetPartyId()*/, m_EventSubscribersList.size());
			CSvcSubscriber* pSubscriber = FindSubscriberByMonitorID(val);
			if (IsValidPObjectPtr(pSubscriber))
			{
				DWORD dwPartyRsrcID = pSubscriber->GetPartyRsrcID();
				m_setAddedParticipantsRsrcIDs.erase(dwPartyRsrcID);
				TRACEINTO << "Remove subscriber:\n\tPartyRsrcID = " << pSubscriber->GetPartyRsrcID() << "\n\tCall-ID = " << pSubscriber->GetCallId();
				RemoveFromVector(pSubscriber);
				POBJDELETE(pSubscriber);
			}
			break;
		}

		default:
			PTRACE(eLevelError, "CSvcEventPackageManager::ObserverUpdate - Invalid event received.");
			PASSERT_AND_RETURN(event);
			break;
	}

	if (bUrgent && pUserUrgentUpdated)
	{
		m_pXMLStr = BuildUrgentNotifyContent(pUserUrgentUpdated);
		if (m_pXMLStr)
		{
			long iUserID = m_pEventConfInfo->FindUserID(pUserUrgentUpdated);
			std::string sXml(m_pXMLStr->GetString());
			for (std::vector<CSvcSubscriber*>::iterator it = m_EventSubscribersList.begin(); it != m_EventSubscribersList.end(); ++it)
			{
				bool bSendNow = (*it)->AddUrgentNotification(iUserID, sXml);
				if (bSendNow)
				{
					SetNotificationVersion(sXml, (*it)->GetNotificationVersion());
					PTRACE2(eLevelInfoNormal, "CSvcEventPackageManager::ObserverUpdate, XML = ", sXml.c_str());
					Notify(*it, (char*)sXml.c_str(), "active");
				}
			}
		}
		POBJDELETE(m_pXMLStr);
	}

	if (!bUrgent)
	{
		if (!m_runInTdd && !m_NotifyTimer)
			StartTimer(TIMER_NOTIFY_TOUT, SIP_CX_NOTIFY_TOUT);
		m_NotifyTimer = TRUE;
	}
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::SetNotificationVersion(std::string& sXml, int iVersion)
{
	int i = sXml.find("conference-info");
	if (i > 0)
	{
		i = sXml.find("version=\"", i);
		if (i > 0)
		{
			i = sXml.find('\"', i);
			if (i > 0)
			{
				int j = sXml.find('\"', i + 1);
				if (j > i)
				{
					char buff[20] = "";
					sprintf(buff, "%u", iVersion);
					sXml.replace(i + 1, j - i - 1, buff);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////
CObjString* CSvcEventPackageManager::BuildNotifyContent(BYTE bFull, BYTE bIsNewSubscriber)
{
	PTRACE2(eLevelInfoNormal, "CSvcEventPackageManager::BuildNotifyContent - bFull = ", bFull ? "true" : "false");

	CObjString *pContentStr = NULL;
	std::string sFullStr("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");
	char *XML = NULL, *pValue = NULL;
	DWORD length = 0;
	CXMLDOMElement *pElement = new CXMLDOMElement;

	m_pEventConfInfo->SerializeXml(pElement, bFull, bIsNewSubscriber);

	//if there was no change in data -> no xml will be created!!!
	pElement->get_tagName(&pValue);
	if (NULL != pValue)
	{
		pElement->DumpDataAsStringWithAttribute(&XML, &length);
		pContentStr = new CObjString(XML, length + 20);

		PDELETE(pElement);
		PDELETEA(XML);
	}
	else
		pContentStr = new CSmallString("");

	sFullStr += pContentStr->GetString();
	PTRACE2(eLevelInfoNormal, "CSvcEventPackageManager::BuildNotifyContent - Notification XML:\n", sFullStr.c_str());
	POBJDELETE(pContentStr);
	pContentStr = new CObjString(sFullStr.c_str());
	return pContentStr;
}

///////////////////////////////////////////////////////////////////
CObjString* CSvcEventPackageManager::BuildUrgentNotifyContent(CSvcUserType* pUserUrgentUpdated)
{
	TRACEINTO << "CSvcEventPackageManager::BuildUrgentNotifyContent - pUserUrgentUpdated = " << (long)pUserUrgentUpdated;

	CObjString *pContentStr = NULL;
	std::string sFullStr("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");
	char *XML = NULL, *pValue = NULL;
	DWORD length = 0;
	CXMLDOMElement *pElement = new CXMLDOMElement;

	m_pEventConfInfo->SerializeUrgentXml(pElement, pUserUrgentUpdated);

	//if there was no change in data -> no xml will be created!!!
	pElement->get_tagName(&pValue);
	if (NULL != pValue)
	{
		pElement->DumpDataAsStringWithAttribute(&XML, &length);
		pContentStr = new CObjString(XML, length + 20);

		PDELETE(pElement);
		PDELETEA(XML);
	}
	else
		pContentStr = new CSmallString("");

	sFullStr += pContentStr->GetString();
	PTRACE2(eLevelInfoNormal, "CSvcEventPackageManager::BuildUrgentNotifyContent - Notification XML:\n", sFullStr.c_str());
	POBJDELETE(pContentStr);
	pContentStr = new CObjString(sFullStr.c_str());
	return pContentStr;
}

///////////////////////////////////////////////////////////////////
BYTE CSvcEventPackageManager::CheckRequestValidity(mcIndSubscribe* pSubscribeInd, enSipCodes *SipStatus)
{
	BYTE bValidReq = TRUE, acceptVerified = TRUE;
	const char *pFromStr = NULL, *pReferToStr = NULL, *pCallIdStr = NULL;
	char* pAcceptName = NULL;
	CSipHeaderList * pTemp = new CSipHeaderList(pSubscribeInd->sipHeaders);

	//verify 'From' is not empty
	const CSipHeader* pFrom = pTemp->GetNextHeader(kFrom);
	if (pFrom)
		pFromStr = pFrom->GetHeaderStr();
	else
		bValidReq = FALSE;

	//verify 'Call-Id' is not empty
	const CSipHeader* pCallId = pTemp->GetNextHeader(kCallId);
	if (pCallId)
		pCallIdStr = pCallId->GetHeaderStr();
	else
		bValidReq = FALSE;

	//verify 'Accept' is valid
	const CSipHeader* pAccept = pTemp->GetNextHeader(kAccept);
	while (pAccept)
	{
		pAcceptName = (char*)pAccept->GetHeaderStr();
		PTRACE2(eLevelInfoNormal, "CSvcEventPackageManager::CheckRequestValidity - Accept: ", pAcceptName);

		if (pAcceptName)
		{
			//verify allow is either empty or holds "application/conference-info+xml"
			if (strncmp(pAcceptName, "application/conference-info+xml", H243_NAME_LEN) == 0)
				acceptVerified = TRUE;
			else
				acceptVerified = FALSE;
		}
		pAccept = pTemp->GetNextHeader(kAccept);
	}
	if (!acceptVerified)
	{
		*SipStatus = SipCodesNotAcceptable;
		bValidReq = FALSE;
	}

	//verify ip is not empty
	if (pSubscribeInd->transportAddress.transAddr.addr.v4.ip == 0)
		bValidReq = FALSE;

	//verify transpot type is valid
	if (eTransportTypeUdp != pSubscribeInd->transportAddress.transAddr.transportType && eTransportTypeTcp != pSubscribeInd->transportAddress.transAddr.transportType && eTransportTypeTls != pSubscribeInd->transportAddress.transAddr.transportType)
		bValidReq = FALSE;

	POBJDELETE(pTemp);

	return bValidReq;
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::FillSipNotifyStruct(CSipNotifyStruct* pSipNotifyStruct, CSvcSubscriber* pSubscriber)
{
	pSipNotifyStruct->SetHeaderField(kFrom, pSubscriber->GetTo());
	pSipNotifyStruct->SetHeaderField(kFromTag, pSubscriber->GetToTag());
	pSipNotifyStruct->SetHeaderField(kTo, pSubscriber->GetFrom());
	pSipNotifyStruct->SetHeaderField(kReqLine, pSubscriber->GetContact());
	pSipNotifyStruct->SetHeaderField(kToTag, pSubscriber->GetFromTag());
	pSipNotifyStruct->SetHeaderField(kCallId, pSubscriber->GetCallId());
	pSipNotifyStruct->SetHeaderField(kContentType, "application/conference-info+xml");
	pSipNotifyStruct->SetHeaderField(kEvent, "conference");
}

///////////////////////////////////////////////////////////////////
//Just do Notify
BYTE CSvcEventPackageManager::Notify(CSvcSubscriber* pSubscriber, char* content, char* state, BYTE isDistribute)
{
	BYTE result = FALSE;
	std::string sLog("CSvcEventPackageManager::Notify\n\tExact time:\t");
	sLog += GetTimeOfDay() + std::string("\n\tPartyRsrcID:\t") + CStlUtils::ValueToString(pSubscriber->GetPartyRsrcID()) + std::string("\n\tTo:\t") + pSubscriber->GetTo() + std::string("\n\tFrom:\t") + pSubscriber->GetFrom() + std::string("\n\tContent:\n") + content;
	PTRACE(eLevelInfoNormal, sLog.c_str());

	CSipNotifyStruct sipNotify;
	FillSipNotifyStruct(&sipNotify, pSubscriber);
	sipNotify.SetContent(content);

	char SubState[H243_NAME_LEN] = { 0 };
	strcpy_safe(SubState, state);

	if (strncmp(state, "terminated", 10))
	{
		int len = strlen(SubState);
		if (len < H243_NAME_LEN - 16)
		{
			CStructTm curTime;
			STATUS timeStatus = SystemGetTime(curTime);
			char temp[15];
			snprintf(temp, sizeof(temp), ";expires=%d", pSubscriber->GetExpireTime() - curTime);
			strncat(SubState, temp, 15);
		}
	}

	sipNotify.SetHeaderField(kSubscrpState, SubState);

	DWORD dwCSeq = pSubscriber->GetCSeq();
	sipNotify.SetCSeq(dwCSeq);
	pSubscriber->SetCSeq(dwCSeq + 1);
	m_pNotifyMsg = sipNotify.BuildNotifyReq(*(pSubscriber->GetTransportAddress()));
	if (m_pNotifyMsg)
	{
		DWORD dwCallIndex = pSubscriber->GetCallIndex();
		SendSipNotify(pSubscriber->GetPartyRsrcID(), m_pNotifyMsg, pSubscriber->GetSrcUnitId(), dwCallIndex, pSubscriber->GetCsId());
		pSubscriber->SetNotificationVersion(pSubscriber->GetNotificationVersion() + 1);
		PDELETEA(m_pNotifyMsg);
		result = TRUE; //Notify was sent ok.
	}
	return result;
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::NotifyFull(CSvcSubscriber* pSubscriber)
{
	CMedString sLog;
	sLog << "CSvcEventPackageManager::NotifyFull\n\tTo:\t" << pSubscriber->GetTo() << "\n\tFrom:\t" << pSubscriber->GetFrom();
	PTRACE(eLevelInfoNormal, sLog.GetString());
	CSipNotifyStruct sipNotify;
	FillSipNotifyStruct(&sipNotify, pSubscriber);

	char pszSubState[H243_NAME_LEN] = { 0 };
	strcpy_safe(pszSubState, "active");
	int len = strlen(pszSubState);
	if (len < H243_NAME_LEN - 16)
	{
		CStructTm curTime;
		STATUS timeStatus = SystemGetTime(curTime);
		char temp[15];
		snprintf(temp, sizeof(temp), ";expires=%d", pSubscriber->GetExpireTime() - curTime);
		strncat(pszSubState, temp, 15);
	}

	sipNotify.SetHeaderField(kSubscrpState, pszSubState);

	std::list<std::string> listXml;
	std::list<std::string>::iterator itend = listXml.end();
	for (std::list<std::string>::iterator it = listXml.begin(); it != itend; ++it)
	{
		std::string sXmlStr("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");
		sXmlStr += *it;
		sipNotify.SetContent(sXmlStr.c_str());

		DWORD dwCSeq = pSubscriber->GetCSeq();
		sipNotify.SetCSeq(dwCSeq);
		pSubscriber->SetCSeq(dwCSeq + 1);

		m_pNotifyMsg = sipNotify.BuildNotifyReq(*(pSubscriber->GetTransportAddress()));
		if (m_pNotifyMsg)
		{
			SendSipNotify(pSubscriber->GetPartyRsrcID(), m_pNotifyMsg, pSubscriber->GetSrcUnitId(), 0, pSubscriber->GetCsId());
			pSubscriber->SetNotificationVersion(pSubscriber->GetNotificationVersion() + 1);
			PDELETEA(m_pNotifyMsg);
		}
	}
}

///////////////////////////////////////////////////////////////////
CSvcSubscriber* CSvcEventPackageManager::FindParty(const char* from)
{
	PTRACE2(eLevelInfoNormal, "CSvcEventPackageManager::FindParty named = ", from);
	CSvcSubscriber* pSubscriber = NULL;
	if (from)
	{
		std::vector<CSvcSubscriber *>::iterator itr = m_EventSubscribersList.begin();
		while (itr != m_EventSubscribersList.end())
		{
			pSubscriber = (*itr);
			if (!strncmp(from, pSubscriber->GetFrom(), MaxAddressListSize))
				break;
			pSubscriber = NULL;
			++itr;
		}
	}
	return pSubscriber;
}

///////////////////////////////////////////////////////////////////
CSvcSubscriber* CSvcEventPackageManager::FindSubscriberByRsrcID(DWORD dwPartyRsrcID)
{
	CSvcSubscriber* pSubscriber = NULL;
	if (dwPartyRsrcID > 0)
	{
		std::vector<CSvcSubscriber *>::iterator it = m_EventSubscribersList.begin();
		for (; it != m_EventSubscribersList.end(); ++it)
		{
			if ((*it)->GetPartyRsrcID() == dwPartyRsrcID)
			{
				pSubscriber = (*it);
				break;
			}
		}
	}
	TRACEINTO << "\n\tPartyRsrcID: " << dwPartyRsrcID << "\n\tpSubscriber: " << pSubscriber;
	return pSubscriber;
}

///////////////////////////////////////////////////////////////////
CSvcSubscriber* CSvcEventPackageManager::FindSubscriberByMonitorID(DWORD dwPartyMonitorId)
{
	TRACEINTO << "\n\tPartyMonitorId = " << dwPartyMonitorId;
	CSvcSubscriber* pSubscriber = NULL;
	std::vector<CSvcSubscriber *>::iterator it = m_EventSubscribersList.begin();
	for (; it != m_EventSubscribersList.end(); ++it)
	{
		if ((*it)->GetPartyMonitorID() == dwPartyMonitorId)
		{
			pSubscriber = (*it);
			break;
		}
	}
	return pSubscriber;
}

///////////////////////////////////////////////////////////////////
bool CSvcEventPackageManager::DisconnectSubscriberByRsrcID(DWORD dwPartyRsrcID, CSegment& seg)
{
	TRACEINTO << "\n\tPartyRsrcID = " << dwPartyRsrcID << "\n\tConfRsrcID = " << m_pConf->GetConfId();
	CSvcSubscriber* pSubscriber = FindSubscriberByRsrcID(dwPartyRsrcID);
	if (pSubscriber == NULL)
	{
		TRACEINTO << "Subscriber not found!!!\n\tPartyRsrcID = " << dwPartyRsrcID << "\n\tConfRsrcID = " << m_pConf->GetConfId();
		return false;
	}
	pSubscriber->Disconnect();
	pSubscriber->Serialize(seg);
	return true;
}

///////////////////////////////////////////////////////////////////
bool CSvcEventPackageManager::ConnectSubscriberByRsrcID(DWORD dwPartyRsrcID)
{
	TRACEINTO << "\n\tPartyRsrcID = " << dwPartyRsrcID << "\n\tConfRsrcID = " << m_pConf->GetConfId();
	CSvcSubscriber* pSubscriber = FindSubscriberByRsrcID(dwPartyRsrcID);
	if (pSubscriber == NULL)
	{
		TRACEINTO << "Subscriber not found!!!\n\tPartyRsrcID = " << dwPartyRsrcID << "\n\tConfRsrcID = " << m_pConf->GetConfId();
		return false;
	}
	pSubscriber->Connect();
	if (pSubscriber->GetState() == IDLE)
	{
		std::set<DWORD>::iterator it = m_setAddedParticipantsRsrcIDs.find(dwPartyRsrcID);
		if (it == m_setAddedParticipantsRsrcIDs.end())
		{
			m_setConnectedSuscribersRsrcIDs.insert(dwPartyRsrcID);
		}
		else
		{
			CSvcUserType* pUser = m_pEventConfInfo->FindUserByRsrcID(dwPartyRsrcID);
			if (pUser && !pUser->IsInIVR())
			{
				pSubscriber->StartFullNotification();
			}
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////
bool CSvcEventPackageManager::DeleteSubscriberByRsrcID(DWORD dwPartyRsrcID)
{
	TRACEINTO << "\n\tPartyRsrcID = " << dwPartyRsrcID << "\n\tConfRsrcID = " << m_pConf->GetConfId();
	CSvcSubscriber* pSubscriber = FindSubscriberByRsrcID(dwPartyRsrcID);
	if (pSubscriber == NULL)
	{
		TRACEINTO << "Subscriber not found!!!\n\tPartyRsrcID = " << dwPartyRsrcID << "\n\tConfRsrcID = " << m_pConf->GetConfId();
		return false;
	}
	RemoveFromVector(pSubscriber);
	POBJDELETE(pSubscriber);
	return true;
}

///////////////////////////////////////////////////////////////////
bool CSvcEventPackageManager::UnchainSubscriberByRsrcID(DWORD dwPartyRsrcID, CSegment& seg)
{
	TRACEINTO << "\n\tPartyRsrcID = " << dwPartyRsrcID << "\n\tConfRsrcID = " << m_pConf->GetConfId();
	CSvcSubscriber* pSubscriber = FindSubscriberByRsrcID(dwPartyRsrcID);
	if (pSubscriber == NULL)
	{
		TRACEINTO << "Subscriber not found!!!\n\tPartyRsrcID = " << dwPartyRsrcID << "\n\tConfRsrcID = " << m_pConf->GetConfId();
		return false;
	}
	pSubscriber->Serialize(seg);
	RemoveFromVector(pSubscriber);
	POBJDELETE(pSubscriber);
	return true;
}

///////////////////////////////////////////////////////////////////
bool CSvcEventPackageManager::UnchainSubscriberByMonitorID(DWORD dwPartyMonitorId, CSegment& seg)
{
	TRACEINTO << "\n\tPartyMonitorID = " << dwPartyMonitorId << "\n\tConfRsrcID = " << m_pConf->GetConfId();
	CSvcSubscriber* pSubscriber = FindSubscriberByMonitorID(dwPartyMonitorId);
	if (pSubscriber == NULL)
	{
		TRACEINTO << "Subscriber not found!!!\n\tPartyMonitorID = " << dwPartyMonitorId << "\n\tConfRsrcID = " << m_pConf->GetConfId();
		return false;
	}
	pSubscriber->Serialize(seg);
	RemoveFromVector(pSubscriber);
	POBJDELETE(pSubscriber);
	return true;
}

///////////////////////////////////////////////////////////////////
bool CSvcEventPackageManager::ChainSubscriber(CSegment& seg, DWORD dwNewPartyMonitorId)
{
	CSvcSubscriber* pSubscriber = new CSvcSubscriber();
	m_EventSubscribersList.push_back(pSubscriber);
	pSubscriber->DeSerialize(seg);
	pSubscriber->SetPartyMonitorID(dwNewPartyMonitorId);
	pSubscriber->SetEventPackageManager(this);
	TRACEINTO << "\n\tPartyRsrcID = " << pSubscriber->GetPartyRsrcID() << "\n\tNewPartyMonitorId = " << pSubscriber->GetPartyMonitorID() << "\n\tConfRsrcID = " << m_pConf->GetConfId();

	if (pSubscriber->GetState() == IDLE)
	{
		CSvcUserType* pUser = m_pEventConfInfo->FindUserByMonitorID(dwNewPartyMonitorId);
		if (pUser && !pUser->IsInIVR())
		{
			pSubscriber->SetNotificationVersion(0);
			pSubscriber->StartFullNotification();
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::PartyLeaveIVR(DWORD dwPartyRsrcID)
{
	TRACEINTO << "\n\tPartyRsrcID = " << dwPartyRsrcID << "\n\tm_state = " << m_state;
	if (m_state != EQ_STATE)
	{
		CSvcSubscriber* pSubscriber = FindSubscriberByRsrcID(dwPartyRsrcID);
		if (IsValidPObjectPtr(pSubscriber))
			pSubscriber->StartFullNotification();

		CSvcUserType* pUser = m_pEventConfInfo->FindUserByRsrcID(dwPartyRsrcID);
		if (pUser == NULL)
		{
			CPartyCntl* pPartCntl = m_pConf->GetPartyCntl(dwPartyRsrcID);
			if (pPartCntl)
			{
				DWORD dwPartyMonitorID = pPartCntl->GetMonitorPartyId();
				pUser = m_pEventConfInfo->FindUserByMonitorID(dwPartyMonitorID);
				if (pUser)
					pUser->SetUserRsrcId(dwPartyRsrcID);
				TRACESTRFUNC(eLevelDebug) << "\n\tPartyRsrcID = " << dwPartyRsrcID << "\n\tPartyMonitorID = " << dwPartyMonitorID;
			}
		}
		if (pUser)
		{
			m_pEventConfInfo->PartyLeaveIVR(dwPartyRsrcID);
			TRACEINTO << "PartyId:" << dwPartyRsrcID << ", PartyMonitorId:" << pUser->GetUserMonitorId() << ", IsInIVR:" << (pUser->IsInIVR() ? "true" : "false");
			m_pXMLStr = BuildUrgentNotifyContent(pUser);
			if (m_pXMLStr)
			{
				long iUserID = m_pEventConfInfo->FindUserID(pUser);
				std::string sXml(m_pXMLStr->GetString());
				for (std::vector<CSvcSubscriber*>::iterator it = m_EventSubscribersList.begin(); it != m_EventSubscribersList.end(); ++it)
				{
					if ((*it)->GetPartyRsrcID() != dwPartyRsrcID)
					{
						bool bSendNow = (*it)->AddUrgentNotification(iUserID, sXml);
						TRACESTRFUNC(eLevelDebug) << "\n\tPartyRsrcID = " << dwPartyRsrcID << "\n\tbSendNow = " << (bSendNow ? "true" : "false");
						if (bSendNow)
						{
							SetNotificationVersion(sXml, (*it)->GetNotificationVersion());
							Notify(*it, (char*)sXml.c_str(), "active");
						}
					}
				}
			}
			POBJDELETE(m_pXMLStr);
		}
	}
}

///////////////////////////////////////////////////////////////////
//find shortest expires between all subscribers
WORD CSvcEventPackageManager::CalcTimer()
{
	PTRACE(eLevelInfoNormal, "CSvcEventPackageManager::CalcTimer");
	CSvcSubscriber* pSubscriber = NULL;
	CStructTm expiresTime;
	DWORD expires, result = 0xffffffff;

	CStructTm curTime;
	STATUS timeStatus = SystemGetTime(curTime);
	std::vector<CSvcSubscriber *>::iterator itr = m_EventSubscribersList.begin();
	while (itr != m_EventSubscribersList.end())
	{
		pSubscriber = (*itr);
		if (IsValidPObjectPtr(pSubscriber))
		{
			expiresTime = pSubscriber->GetExpireTime();
			expires = expiresTime - curTime;
			if (expires < result)
				result = expires;
		}
		++itr;
	}
	if (0xffffffff == result)
		result = 0;
	char s[10];
	snprintf(s, sizeof(s), "%d", result);
	PTRACE2(eLevelInfoNormal, "CSvcEventPackageManager::CalcTimer = ", s);
	return result;
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::TerminateConf()
{
	CSvcSubscriber* pSubscriber = NULL;

	DeleteTimer(TIMER_SUBSCRIBE_TOUT);
	if (m_NotifyTimer)
	{
		DeleteTimer(TIMER_NOTIFY_TOUT);
		m_NotifyTimer = false;
	}
	m_state = TERMINATION;

	//find all subscribers
	std::vector<CSvcSubscriber *>::iterator itr = m_EventSubscribersList.begin();
	while (itr != m_EventSubscribersList.end())
	{
		pSubscriber = (*itr);
		if (IsValidPObjectPtr(pSubscriber))
		{
			BYTE bLooseLoadMngr = FALSE;
			PTRACE2(eLevelInfoNormal, "CSvcEventPackageManager::TerminateConf party = ", pSubscriber->GetFrom());
			Notify(pSubscriber, "", "terminated;reason=noresource");
		}
		m_EventSubscribersList.erase(itr);
		POBJDELETE(pSubscriber);
		itr = m_EventSubscribersList.begin();
	}

	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	if (IsValidPObjectPtr(pCommConf))
	{
		DetachEvents(pCommConf);
		TRACEINTO << "DetatachEvents";
	}
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::Dump(COstrStream& msg) const
{
	CSvcSubscriber* pSubscriber = NULL;
	char* result = NULL;

	if (msg)
	{
		msg << "CSvcEventPackageManager::Dump type = conference\n";
		msg << "-----------------------------------\n";
	}
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::SetConfName(const char* pConfName)
{
	ALLOCBUFFER(confUri, H243_NAME_LEN);
	if (pConfName)
	{
		strncpy(confUri, pConfName, H243_NAME_LEN);
		char *temp = (char*)strstr(pConfName, "@");
		if (temp)
			*temp = '\0';
		strncpy(m_pConfName, pConfName, H243_NAME_LEN);
	}
	if (!m_pEventConfInfo)
	{
		CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();

		if (IsValidPObjectPtr(pCommConf))
		{
			PTRACE2(eLevelInfoNormal, "CSvcEventPackageManager::SetConfName conf name is : ", pConfName);
			m_pEventConfInfo = new CSvcConfInfoType(pCommConf, confUri, pCommConf->GetConfContactInfo(0)/*, m_version*/);
		}
	}
	DEALLOCBUFFER(confUri);
}

///////////////////////////////////////////////////////////////////
const char* CSvcEventPackageManager::GetConfName()
{
	return m_pConfName;
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::OnSubscribeTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSvcEventPackageManager::OnSubscribeTout");
	CSvcSubscriber* pSubscriber = NULL;
	//calc current time
	CStructTm curTime;
	STATUS timeStatus = SystemGetTime(curTime);

	//find all subscribers with expired subscription
	std::vector<CSvcSubscriber *>::iterator itr = m_EventSubscribersList.begin();
	while (itr != m_EventSubscribersList.end())
	{
		pSubscriber = (*itr);
		if (IsValidPObjectPtr(pSubscriber))
		{
			DWORD duration = pSubscriber->GetExpireTime() - curTime;
			if (duration < 5)
			{
				PTRACE2(eLevelInfoNormal, "CSvcEventPackageManager::OnSubscribeTout party = ", pSubscriber->GetFrom());
				Notify(pSubscriber, "", "terminated;reason=noresource");

				m_EventSubscribersList.erase(itr);
				POBJDELETE(pSubscriber);
				itr = m_EventSubscribersList.begin();
			}
			else
				++itr;
		}
	}

	WORD duration = CalcTimer();
	if (duration > 0)
	{
		if (!m_runInTdd)
			StartTimer(TIMER_SUBSCRIBE_TOUT, SECOND * duration);
		m_SubTimer = TRUE;
	}
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::ResetNotifyTimer()
{
	PTRACE(eLevelError, "CSvcEventPackageManager::ResetNotifyTimer");
	m_NotifyTimer = FALSE;
}

///////////////////////////////////////////////////////////////////
void CSvcEventPackageManager::ExpireTime()
{
	CSvcSubscriber* pSubscriber = (CSvcSubscriber*)m_EventSubscribersList[0];
	if (pSubscriber)
	{
		CStructTm curTime;
		STATUS timeStatus = SystemGetTime(curTime);
		pSubscriber->SetExpireTime(&curTime);
	}
}


// ************************************************************************************
//
//	CSvcSubscriber
//
// ************************************************************************************
// start message map -------------------------------------------
PBEGIN_MESSAGE_MAP(CSvcSubscriber)

// conf events
	ONEVENT(TIMER_NEXT_NOTIFICATION, IDLE, CSvcSubscriber::OnNextNotification)
	ONEVENT(TIMER_NEXT_NOTIFICATION, SENT_FULL, CSvcSubscriber::OnNextNotification)
	ONEVENT(TIMER_NEXT_NOTIFICATION, SENT_LAST, CSvcSubscriber::OnLastNotificationSent)
	ONEVENT(TIMER_NEXT_NOTIFICATION, WAIT_ACK_FULL, CSvcSubscriber::OnWaitAckFullNotification)
	ONEVENT(TIMER_NEXT_NOTIFICATION, WAIT_ACK_LAST, CSvcSubscriber::OnWaitAckLastNotificationSent)
	ONEVENT(SIP_CS_SIG_NOTIFY_RESPONSE_IND, WAIT_ACK_FULL, CSvcSubscriber::OnAckLastNotificationSent)
	ONEVENT(SIP_CS_SIG_NOTIFY_RESPONSE_IND, WAIT_ACK_LAST, CSvcSubscriber::OnAckLastNotificationSent)
	ONEVENT(SIP_CS_SIG_NOTIFY_RESPONSE_IND, ANYCASE, CSvcSubscriber::OnAckNotificationSent)

PEND_MESSAGE_MAP(CSvcSubscriber,CStateMachine)
;
// end   message map -------------------------------------------

/////////////////////////////////////////////////////////////////////////////////////////
CSvcSubscriber::CSvcSubscriber() :
		m_dwPartyRsrcID(0), m_dwPartyMonitorID(0), m_serviceID(0), m_srcUnitId(0), m_csId(1), m_callIndex(0), m_role(eParticipant), m_notifyVersionCounter(2), m_iLastUser(0)
{
	m_state = IDLE;

	m_pFromUri = new char[MaxAddressListSize];
	*m_pFromUri = '\0';
	m_pToUri = new char[MaxAddressListSize];
	*m_pToUri = '\0';
	m_pCallId = new char[MaxAddressListSize];
	*m_pCallId = '\0';
	m_pFromTag = new char[MaxAddressListSize];
	*m_pFromTag = '\0';
	m_pToTag = new char[MaxAddressListSize];
	*m_pToTag = '\0';
	m_pContact = new char[MaxAddressListSize];
	*m_pContact = '\0';

	m_pCSvcEventPackageManager = NULL;
	m_iNotificationVersion = 0;
	m_CSeq = 1;

	m_iXmlMaxUsers = XML_MAX_USERS;
	m_bFullNotificationSent = false;
}

/////////////////////////////////////////////////////////////////////////////////////////
CSvcSubscriber::CSvcSubscriber(CSvcEventPackageManager* pCSvcEventPackageManager, DWORD dwPartyRsrcID, DWORD dwPartyMonitorID, WORD boardId, const char* pFrom, const char* pFromTag, const char* pTo, const char* pToTag, const char* pContact, const char* pCallId, mcXmlTransportAddress transportAddress, WORD expires, WORD srcUnitId, DWORD callIndex, eRoleType role, DWORD cs_Id) :
		m_dwPartyRsrcID(dwPartyRsrcID), m_dwPartyMonitorID(dwPartyMonitorID), m_transportAddress(transportAddress), m_serviceID(boardId), m_csId(cs_Id), m_role(role), m_notifyVersionCounter(2), m_bFullNotificationSent(false), m_iLastUser(0)
{
	m_state = IDLE;

	m_pCSvcEventPackageManager = pCSvcEventPackageManager;
	m_pFromUri = new char[MaxAddressListSize];
	strcpy_safe(m_pFromUri, MaxAddressListSize, pFrom);
	m_pFromTag = new char[MaxAddressListSize];
	strcpy_safe(m_pFromTag, MaxAddressListSize, pFromTag);
	m_pToUri = new char[MaxAddressListSize];
	strcpy_safe(m_pToUri, MaxAddressListSize, pTo);
	m_pToTag = new char[MaxAddressListSize];
	strcpy_safe(m_pToTag, MaxAddressListSize, pToTag);
	m_pContact = new char[MaxAddressListSize];
	strcpy_safe(m_pContact, MaxAddressListSize, pContact);
	m_pCallId = new char[MaxAddressListSize];
	strcpy_safe(m_pCallId, MaxAddressListSize, pCallId);

	m_srcUnitId = srcUnitId;
	m_callIndex = callIndex;

	m_iNotificationVersion = 0;
	m_CSeq = 1;

	m_iXmlMaxUsers = XML_MAX_USERS;

	if (m_pCSvcEventPackageManager->GetState() == EQ_STATE)
		m_state = EQ_STATE;

	CStructTm curTime;
	STATUS timeStatus = SystemGetTime(curTime);	//add expires to cur time
	m_ExpiresTime = curTime;
	m_ExpiresTime.m_hour += expires / 3600;
	m_ExpiresTime.m_min += (expires % 3600) / 60;
	m_ExpiresTime.m_sec += (expires % 3600) % 60;

}

///////////////////////////////////////////////////////////////////
CSvcSubscriber::~CSvcSubscriber()
{
	PDELETEA(m_pFromUri);
	PDELETEA(m_pFromTag);
	PDELETEA(m_pToUri);
	PDELETEA(m_pToTag);
	PDELETEA(m_pCallId);
	PDELETEA(m_pContact);
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode)
{
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::Refresh(mcXmlTransportAddress transportAddress, const char* pCallId, WORD expires)
{
	TRACEINTO << "CSvcSubscriber::Refresh for party " << m_pFromUri << "\n\tPartyRsrcID:\t" << m_dwPartyRsrcID;

	m_transportAddress = transportAddress;
	if (pCallId)
		strcpy_safe(m_pCallId, MaxAddressListSize, pCallId);

	CStructTm curTime;
	STATUS timeStatus = SystemGetTime(curTime);
	//add expires to cur time
	m_ExpiresTime = curTime;
	//add expires to cur time
	m_ExpiresTime.m_hour += expires / 3600;
	m_ExpiresTime.m_min += (expires % 3600) / 60;
	m_ExpiresTime.m_sec += (expires % 3600) % 60;
}

///////////////////////////////////////////////////////////////////
WORD CSvcSubscriber::GetBoardId()
{
	return m_serviceID;
}

///////////////////////////////////////////////////////////////////
char* CSvcSubscriber::GetFrom()
{
	return m_pFromUri;
}

///////////////////////////////////////////////////////////////////
char* CSvcSubscriber::GetFromTag()
{
	return m_pFromTag;
}

///////////////////////////////////////////////////////////////////
char* CSvcSubscriber::GetTo()
{
	return m_pToUri;
}

///////////////////////////////////////////////////////////////////
char* CSvcSubscriber::GetToTag()
{
	return m_pToTag;
}

///////////////////////////////////////////////////////////////////
char* CSvcSubscriber::GetContact()
{
	return m_pContact;
}

///////////////////////////////////////////////////////////////////
DWORD CSvcSubscriber::GetIp()
{
	return m_transportAddress.transAddr.addr.v4.ip;	//  m_fromIp;
}

///////////////////////////////////////////////////////////////////
DWORD CSvcSubscriber::GetCsId()
{
	return m_csId;
}

///////////////////////////////////////////////////////////////////
WORD CSvcSubscriber::GetPort()
{
	return m_transportAddress.transAddr.port;
}

///////////////////////////////////////////////////////////////////
WORD CSvcSubscriber::GetTransport()
{
	return m_transportAddress.transAddr.transportType;
}

///////////////////////////////////////////////////////////////////
char* CSvcSubscriber::GetCallId()
{
	return m_pCallId;
}

///////////////////////////////////////////////////////////////////
eRoleType CSvcSubscriber::GetRole()
{
	return m_role;
}

///////////////////////////////////////////////////////////////////
WORD CSvcSubscriber::GetSrcUnitId()
{
	return m_srcUnitId;
}

///////////////////////////////////////////////////////////////////
DWORD CSvcSubscriber::GetCallIndex()
{
	return m_callIndex;
}

///////////////////////////////////////////////////////////////////
CStructTm CSvcSubscriber::GetExpireTime()
{
	return m_ExpiresTime;
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::SetExpireTime(CStructTm* newTime)
{
	m_ExpiresTime = *newTime;
}

///////////////////////////////////////////////////////////////////
int CSvcSubscriber::GetAndIncrementNotifyVersionCounter()
{
	return m_notifyVersionCounter++;
}

///////////////////////////////////////////////////////////////////
int CSvcSubscriber::GetNotificationVersion()
{
	return m_iNotificationVersion;
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::SetNotificationVersion(int iVersion)
{
	m_iNotificationVersion = iVersion;
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::StartFullNotification()
{
	if (m_state == EQ_STATE)
	{
		TRACEINTO << "CSvcSubscriber::StartFullNotification - Entry Queue, no need to send notifications";
		return;
	}
	if (m_state == SUBSCRIBER_DISCONNECTED)
	{
		TRACEINTO << "CSvcSubscriber::StartFullNotification - Subscriber disconnected";
		return;
	}
	DeleteTimer(TIMER_NEXT_NOTIFICATION);

	WORD state = m_state;
	m_iNotificationVersion = 0;
	m_state = WAIT_ACK_FULL; // SENT_FULL;
	m_iLastUser = m_pCSvcEventPackageManager->NotifySubscriberFull(this, 0, m_iNotificationVersion, m_iXmlMaxUsers);
	m_bFullNotificationSent = true;
	if (m_iLastUser == 0)
		m_state = WAIT_ACK_LAST; // SENT_LAST;
	TRACEINTO << "CSvcSubscriber::StartFullNotification\n\tm_iLastUser = " << m_iLastUser << ",  m_iNotificationVersion = " << m_iNotificationVersion << "\n\tPartyRsrcID:\t" << m_dwPartyRsrcID << "\n\tOld state = " << state << "\n\tNew state = " << m_state;

	StartTimer(TIMER_NEXT_NOTIFICATION, NEXT_NOTIFICATION_TOUT);
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::OnNextNotification(CSegment* pParam)
{
	TRACEINTO << "\n\tPartyRsrcID = " << m_dwPartyRsrcID;
	DeleteTimer(TIMER_NEXT_NOTIFICATION);
	WORD state = m_state;
	m_iLastUser = m_pCSvcEventPackageManager->NotifySubscriberFull(this, m_iLastUser, m_iNotificationVersion, m_iXmlMaxUsers);
	if (m_iLastUser == EMPTY_LIST)
	{
		m_iLastUser = 0;
		m_state = WAIT_ACK_LAST; // SENT_LAST; // SENDING_LAST;
		OnLastNotificationSent(NULL);
	}
	else
	{
		if (m_iLastUser == 0)
			m_state = WAIT_ACK_LAST; //SENT_LAST; // SENDING_LAST;
		else
			m_state = WAIT_ACK_FULL; //SENT_FULL; // SENDING_FULL;
		StartTimer(TIMER_NEXT_NOTIFICATION, NEXT_NOTIFICATION_TOUT);
	}
	TRACEINTO << " - m_iLastUser = " << m_iLastUser << ",  m_iNotificationVersion = " << m_iNotificationVersion << "\n\tPartyRsrcID:\t" << m_dwPartyRsrcID << "\n\tOld state = " << state << "\n\tNew state = " << m_state;
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::OnLastNotificationSent(CSegment* pParam)
{
	WORD state = m_state;
	DeleteTimer(TIMER_NEXT_NOTIFICATION);
	if (!m_listUrgentNotificationsQueue.empty())
	{
		std::string sXml = m_listUrgentNotificationsQueue.front();
		m_pCSvcEventPackageManager->SetNotificationVersion(sXml, GetNotificationVersion());
		m_pCSvcEventPackageManager->Notify(this, (char*)(sXml.c_str()), "active");
		TRACEINTO << " after notify from m_listUrgentNotificationsQueue - m_iNotificationVersion = " << m_iNotificationVersion << "\n\tstate = " << m_state << "\n\tPartyRsrcID:\t" << m_dwPartyRsrcID;
		m_listUrgentNotificationsQueue.pop_front();
		StartTimer(TIMER_NEXT_NOTIFICATION, NEXT_NOTIFICATION_TOUT);
	}
	else if (!m_listTimerNotificationsQueue.empty())
	{
		std::string sXml = m_listTimerNotificationsQueue.front();
		m_pCSvcEventPackageManager->SetNotificationVersion(sXml, GetNotificationVersion());
		m_pCSvcEventPackageManager->Notify(this, (char*)(sXml.c_str()), "active");
		TRACEINTO << " after notify from m_listTimerNotificationsQueue - m_iNotificationVersion = " << m_iNotificationVersion << "\n\tstate = " << m_state << "\n\tPartyRsrcID:\t" << m_dwPartyRsrcID;
		m_listTimerNotificationsQueue.pop_front();
		StartTimer(TIMER_NEXT_NOTIFICATION, NEXT_NOTIFICATION_TOUT);
	}
	else
	{
		m_state = IDLE;
		TRACEINTO << " - m_listUrgentNotificationsQueue and m_listTimerNotificationsQueue are empty" << "\n\tPartyRsrcID:\t" << m_dwPartyRsrcID << "\n\tOld state = " << state << "\n\tNew state = " << m_state;
	}
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::OnWaitAckFullNotification(CSegment* pParam)
{
	TRACESTRFUNC(eLevelDebug) << "\n\tstate = " << m_state << "\n\tPartyRsrcID:\t" << m_dwPartyRsrcID;
	StartTimer(TIMER_NEXT_NOTIFICATION, NEXT_NOTIFICATION_TOUT);
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::OnWaitAckLastNotificationSent(CSegment* pParam)
{
	TRACESTRFUNC(eLevelDebug) << "\n\tstate = " << m_state << "\n\tPartyRsrcID:\t" << m_dwPartyRsrcID;
	StartTimer(TIMER_NEXT_NOTIFICATION, NEXT_NOTIFICATION_TOUT);
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::OnAckLastNotificationSent(CSegment* pParam)
{
	WORD state = m_state;
	m_state = SENT_LAST;
	TRACEINTO << "\n\tOld state = " << state << "\n\tNew state = " << m_state;
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::OnAckNotificationSent(CSegment* pParam)
{
	WORD state = m_state;
	m_state = IDLE;
	TRACEINTO << "\n\tPartyRsrcID:\t" << m_dwPartyRsrcID << "\n\tOld state = " << state << "\n\tNew state = " << m_state;
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::OnIndNotifyResp(DWORD status)
{
	if (status == SipCodesUnauthorized)
	{
		++m_CSeq;
		TRACEINTO << "Additional increment CSeq because DMA proxy\n\tCSeq = " << m_CSeq << "\n\tPartyRsrcID = " << m_dwPartyRsrcID;
	}
	WORD state = m_state;
	if (m_state == WAIT_ACK_FULL)
		m_state = SENT_FULL;
	else if (m_state == WAIT_ACK_LAST)
		m_state = SENT_LAST;
	else if (m_state == SENT_LAST)
		OnLastNotificationSent(NULL);
	else
	{
		DeleteTimer(TIMER_NEXT_NOTIFICATION);
		m_state = IDLE;
	}
	TRACEINTO << "\n\tExact time:\t" << GetTimeOfDay() << "\n\tFrom:\t" << GetFrom() << "\n\tPartyRsrcID:\t" << m_dwPartyRsrcID << "\n\tstatus:\t" << status << "\n\tOld state = " << state << "\n\tNew state = " << m_state;
}

///////////////////////////////////////////////////////////////////
bool CSvcSubscriber::AddUrgentNotification(long iUserID, std::string& sXml)
{
	TRACEINTO << "\n\tPartyRsrcID:\t" << m_dwPartyRsrcID << "\n\tm_state = " << m_state << "\n\tm_iLastUser = " << m_iLastUser << "\n\tiUserID = " << iUserID << "\n\tFull notification sent = " << m_bFullNotificationSent;
	if (!m_bFullNotificationSent)
		return false;
	if (m_state == SUBSCRIBER_DISCONNECTED)
		return false;
	if (m_state != IDLE)
	{
		if (m_state == SENT_LAST || m_state == WAIT_ACK_LAST || ((m_state == SENT_FULL || m_state == WAIT_ACK_FULL) && iUserID <= m_iLastUser))
			m_listUrgentNotificationsQueue.push_back(sXml);
		return false;
	}
	m_state = SENT_LAST;
	return true;
}

///////////////////////////////////////////////////////////////////
bool CSvcSubscriber::AddTimerNotification(std::list<long> listUserID, std::string& sXml)
{
	if (m_state != IDLE)
		return false;
	m_state = SENT_LAST;
	return true;
}

///////////////////////////////////////////////////////////////////
bool CSvcSubscriber::AddTimerNotification(std::string& sXml)
{
	if (!m_bFullNotificationSent)
	{
		TRACEINTO << "\n\tPartyRsrcID:\t" << m_dwPartyRsrcID << "\n\tm_state = " << m_state << "\n\tFull notification sent = " << m_bFullNotificationSent;
		return false;
	}
	if (m_state != IDLE)
	{
		m_listTimerNotificationsQueue.push_back(sXml);
		return false;
	}
	m_state = SENT_LAST;
	return true;
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::Disconnect()
{
	m_state = SUBSCRIBER_DISCONNECTED;
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::Connect()
{
	if (m_state == SUBSCRIBER_DISCONNECTED)
	{
		m_state = IDLE;
	}
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::Serialize(CSegment &seg)
{
	seg << (void*)m_pCSvcEventPackageManager << m_pFromUri << m_pFromTag << m_pToUri << m_pToTag << m_pContact << m_pCallId << m_dwPartyRsrcID << m_dwPartyMonitorID
	    << (DWORD)m_iXmlMaxUsers << m_serviceID << m_srcUnitId << m_csId << m_callIndex << (WORD)m_role << (WORD)(m_ExpiresTime.m_year) << (WORD)(m_ExpiresTime.m_mon) << (WORD)(m_ExpiresTime.m_day) << (WORD)(m_ExpiresTime.m_hour) << (WORD)(m_ExpiresTime.m_min) << (WORD)(m_ExpiresTime.m_sec) << (DWORD)m_notifyVersionCounter << (DWORD)m_iNotificationVersion << (DWORD)m_iLastUser << m_state;
	seg.Put((byte*)(&m_transportAddress), sizeof(mcXmlTransportAddress));
}

///////////////////////////////////////////////////////////////////
void CSvcSubscriber::DeSerialize(CSegment &seg)
{
	void* pVoid = NULL;
	char buff[512] = "";
	WORD word = 0;
	DWORD dword = 0;
	seg >> pVoid;
	m_pCSvcEventPackageManager = (CSvcEventPackageManager*)pVoid;
	seg >> buff;
	strcpy_safe(m_pFromUri, MaxAddressListSize, buff);
	seg >> buff;
	strcpy_safe(m_pFromTag, MaxAddressListSize, buff);
	seg >> buff;
	strcpy_safe(m_pToUri, MaxAddressListSize, buff);
	seg >> buff;
	strcpy_safe(m_pToTag, MaxAddressListSize, buff);
	seg >> buff;
	strcpy_safe(m_pContact, MaxAddressListSize, buff);
	seg >> buff;
	strcpy_safe(m_pCallId, MaxAddressListSize, buff);

	seg >> m_dwPartyRsrcID >> m_dwPartyMonitorID;
	seg >> dword;
	m_iXmlMaxUsers = (int)dword;
	seg >> m_serviceID >> m_srcUnitId >> m_csId >> m_callIndex;
	seg >> word;
	m_role = (eRoleType)word;
	seg >> word;
	m_ExpiresTime.m_year = (int)word;
	seg >> word;
	m_ExpiresTime.m_mon = (int)word;
	seg >> word;
	m_ExpiresTime.m_day = (int)word;
	seg >> word;
	m_ExpiresTime.m_hour = (int)word;
	seg >> word;
	m_ExpiresTime.m_min = (int)word;
	seg >> word;
	m_ExpiresTime.m_sec = (int)word;
	seg >> dword;
	m_notifyVersionCounter = (int)dword;
	seg >> dword;
	m_iNotificationVersion = (int)dword;
	seg >> dword;
	m_iLastUser = (int)dword;
	seg >> m_state;
	seg.Get((byte*)(&m_transportAddress), sizeof(mcXmlTransportAddress));
}

///////////////////////////////////////////////////////////////////
std::string CSvcSubscriber::ToString()
{
	std::stringstream ss(std::stringstream::in | std::stringstream::out);
	ss << "m_pCSvcEventPackageManager = " << (DWORD)m_pCSvcEventPackageManager << "\n";
	ss << "m_pFromUri = " << m_pFromUri << "\n";
	ss << "m_pFromTag = " << m_pFromTag << "\n";
	ss << "m_pToUri = " << m_pToUri << "\n";
	ss << "m_pToTag = " << m_pToTag << "\n";
	ss << "m_pContact = " << m_pContact << "\n";
	ss << "m_pCallId = " << m_pCallId << "\n";
	ss << "m_transportType = " << m_transportAddress.transAddr.transportType << "\n"; //TCP UDP
	ss << "m_iXmlMaxUsers = " << m_iXmlMaxUsers << "\n";
	ss << "m_serviceID = " << m_serviceID << "\n";
	ss << "m_serviceID = " << m_serviceID << "\n";
	ss << "m_csId = " << m_csId << "\n";
	ss << "m_callIndex = " << m_callIndex << "\n";
	ss << "m_callIndex = " << m_callIndex << "\n";
	char buff[30] = "";
	snprintf(buff, ARRAYSIZE(buff), "%02d.%02d.%02d %02d:%02d:%02d", m_ExpiresTime.m_day, m_ExpiresTime.m_mon, (m_ExpiresTime.m_year - 100) % 100, m_ExpiresTime.m_hour, m_ExpiresTime.m_min, m_ExpiresTime.m_sec);

	ss << "m_ExpiresTime = " << buff << "\n";
	ss << "m_notifyVersionCounter = " << m_notifyVersionCounter << "\n";
	ss << "m_iNotificationVersion = " << m_iNotificationVersion << "\n";
	ss << "m_iLastUser = " << m_iLastUser;

	ss << "m_state = " << m_state;

	return ss.str();
}

///////////////////////////////////////////////////////////////////
WORD operator==(const CSvcSubscriber& first, const CSvcSubscriber& second)
{
	WORD rval = 0;
	if (!strncmp(first.m_pFromUri, second.m_pFromUri, MaxAddressListSize) && !strncmp(first.m_pToUri, second.m_pToUri, MaxAddressListSize) && !strncmp(first.m_pCallId, second.m_pCallId, MaxAddressListSize))
		rval = 1;

	return rval;
}


// ************************************************************************************
//
//	CSvcBasicConfPackageType
//
// ************************************************************************************

///////////////////////////////////////////////////////////////////
CSvcBasicConfPackageType::CSvcBasicConfPackageType()
{
	m_sNameSpacePrefix = "";
	m_elementName[0] = '\0';
}

///////////////////////////////////////////////////////////////////
CSvcBasicConfPackageType::CSvcBasicConfPackageType(char* pName)
{
	m_sNameSpacePrefix = "";
	if (pName)
	{
		strncpy(m_elementName, pName, sizeof(m_elementName) - 1);
		m_elementName[sizeof(m_elementName) - 1] = 0;
	}
}

///////////////////////////////////////////////////////////////////
CSvcBasicConfPackageType::~CSvcBasicConfPackageType()
{
}

///////////////////////////////////////////////////////////////////
std::string CSvcBasicConfPackageType::GetTagWithNsPrefix(const char* pszTag)
{
	std::string s = m_sNameSpacePrefix + std::string(pszTag);
	return s;
}

///////////////////////////////////////////////////////////////////
std::string CSvcBasicConfPackageType::GetTagWithNsPrefix(const char* pszNamespace, const char* pszTag)
{
	std::string s(pszNamespace);
	if (s.size() > 0)
		s += std::string(":");
	s += std::string(pszTag);
	return s;
}

///////////////////////////////////////////////////////////////////
const char* CSvcBasicConfPackageType::GetElementName()
{
	return m_elementName;
}

// ************************************************************************************
//
//	CSvcConfPackageType
//
// ************************************************************************************

///////////////////////////////////////////////////////////////////
CSvcConfPackageType::CSvcConfPackageType() :
		m_state(eFullData), m_bNew(true)
{
}

///////////////////////////////////////////////////////////////////
CSvcConfPackageType::CSvcConfPackageType(char* pName) :
		CSvcBasicConfPackageType(pName), m_state(eFullData), m_bNew(true)
{
}

///////////////////////////////////////////////////////////////////
CSvcConfPackageType::~CSvcConfPackageType()
{
}

///////////////////////////////////////////////////////////////////
eTypeState CSvcConfPackageType::GetState()
{
	return m_state;
}

///////////////////////////////////////////////////////////////////
void CSvcConfPackageType::SerializeXml(CXMLDOMElement* pFatherNode)
{
	SerializeXml(pFatherNode, TRUE);
}

///////////////////////////////////////////////////////////////////
const char* CSvcConfPackageType::GetStateByString()
{
	switch (m_state)
	{
		case (eNoChange):
			return "no-change";

		case (ePartialData):
			return "partial";

		case (eFullData):
			return "full";

		case (eDelletedData):
			return "deleted";

		default:
			return "unknown";
	}
}

///////////////////////////////////////////////////////////////////
void CSvcConfPackageType::SerializeState(CXMLDOMElement* pFatherNode, BYTE bFull)
{
	CXMLDOMAttribute* pAttribute = new CXMLDOMAttribute();
	pAttribute->set_nodeName("state");

	//if requested to serialize full data
	if (bFull)
	{
		pAttribute->SetValueForElement("full");
	}
	else
	{
		if (eFullData == m_state)
			pAttribute->SetValueForElement("full");
		else
			pAttribute->SetValueForElement("partial");
	}
	pFatherNode->AddAttribute(pAttribute);

	if (!bFull || eFullData == m_state)
		m_state = eNoChange;
}

// ************************************************************************************
//
//	CSvcConfInfoType
//
// ************************************************************************************

///////////////////////////////////////////////////////////////////
CSvcConfInfoType::CSvcConfInfoType() :
		m_entityAtr(""), m_version(0)
{
	strcpy_safe(m_elementName, CONF_INFO_NAME);

	m_pConfDescription = new CSvcConfDescriptionType;
	m_pHostInfo = new CSvcHostInfoType;
	m_pConfState = new CSvcConfStateType;
	m_pFloorInfo = new CSvcFloorInfoType;

	m_pConfExtension = new CSvcConfExtension;
	m_pConf = NULL;
	m_iUserCounter = 0;
}

///////////////////////////////////////////////////////////////////
CSvcConfInfoType::CSvcConfInfoType(CCommConf* pCommConf, const char* confUri, const char* contactInfo) :
		m_entityAtr(confUri), m_version(0)
{
	strcpy_safe(m_elementName, CONF_INFO_NAME);

	m_pConfDescription = new CSvcConfDescriptionType;
	m_pHostInfo = new CSvcHostInfoType;
	m_pConfState = new CSvcConfStateType;
	m_pFloorInfo = new CSvcFloorInfoType(pCommConf->GetMonitorConfId());

	m_pConfExtension = new CSvcConfExtension(contactInfo, pCommConf->GetMonitorConfId());
	m_pConf = NULL;
	m_iUserCounter = 0;
	SetupUsersList(pCommConf);
}

///////////////////////////////////////////////////////////////////
CSvcConfInfoType::CSvcConfInfoType(CConf* pConf, const char* confUri, const char* contactInfo) :
		m_entityAtr(confUri), m_version(0)
{
	m_pConf = pConf;

	strcpy_safe(m_elementName, CONF_INFO_NAME);

	CCommConf* pCommConf = (CCommConf*)(pConf->GetCommConf());
	CStructTm dtStartConf = *(pCommConf->GetStartTime());
	m_pConfDescription = new CSvcConfDescriptionType("http://www.polycom.co.il", dtStartConf);
	m_pHostInfo = new CSvcHostInfoType;
	m_pConfState = new CSvcConfStateType;
	m_pFloorInfo = new CSvcFloorInfoType(pConf->GetMonitorConfId());

	m_pConfExtension = new CSvcConfExtension(contactInfo, pConf->GetMonitorConfId());

	m_iUserCounter = 0;
	SetupUsersList(pCommConf);
}

///////////////////////////////////////////////////////////////////
CSvcConfInfoType::~CSvcConfInfoType()
{
	CSvcUserType* pTempUser = NULL;

	SvcUsersMap::iterator itend = m_mapUsers.end();
	for (SvcUsersMap::iterator it = m_mapUsers.begin(); it != itend; ++it)
		POBJDELETE(it->second);

	POBJDELETE(m_pConfDescription);
	POBJDELETE(m_pHostInfo);
	POBJDELETE(m_pConfState);
	POBJDELETE(m_pFloorInfo);

	POBJDELETE(m_pConfExtension);
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::SerializeUrgentXml(CXMLDOMElement* pFatherNode, CSvcUserType* pUserUrgentUpdated)
{
	pFatherNode->set_nodeName(m_elementName);

	pFatherNode->AddAttribute("xmlns", "urn:ietf:params:xml:ns:conference-info");
	pFatherNode->AddAttribute("xmlns:xcon", "urn:ietf:params:xml:ns:xcon-conference-info");
	pFatherNode->AddAttribute("xmlns:mrc", "urn:polycom:mrc:xml:ns:conference-info");

	CXMLDOMAttribute* pEntityAttribute = new CXMLDOMAttribute();
	pEntityAttribute->set_nodeName("entity");
	pEntityAttribute->SetValueForElement(m_entityAtr.GetString());
	pFatherNode->AddAttribute(pEntityAttribute);

	SerializeState(pFatherNode, false);

	CXMLDOMAttribute* pVersionAttribute = new CXMLDOMAttribute();
	pVersionAttribute->set_nodeName("version");
	char version[11] = "";
	snprintf(version, 10, "%d", m_version);

	pVersionAttribute->SetValueForElement(version);
	pFatherNode->AddAttribute(pVersionAttribute);

	m_pConfState->SerializeXml(pFatherNode, m_mapUsers, false);

	CXMLDOMElement* pUsersNode = NULL;
	pUsersNode = pFatherNode->AddChildNode("users");
	CXMLDOMAttribute* pStateAttribute = new CXMLDOMAttribute();
	pStateAttribute->set_nodeName("state");
	pStateAttribute->SetValueForElement("partial");
	pUsersNode->AddAttribute(pStateAttribute);

	SvcUsersMap::iterator it = m_mapUsers.begin();
	if (0 == m_mapUsers.size())
	{
		TRACEINTO << "No users to add to XML";
	}
	else
	{
		TRACEINTO << "From:", pUserUrgentUpdated->m_sFrom.c_str();
		pUserUrgentUpdated->SerializeXml(pUsersNode, pUserUrgentUpdated->IsNew(), false, true);
	}
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull)
{
	SerializeXml(pFatherNode, bFull, FALSE);
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull, BYTE bIsNewSubscriber)
{
	if (bFull || eNoChange != m_state)
	{
		if (eFullData == m_state)
			bFull = TRUE;

		pFatherNode->set_nodeName(m_elementName);

		pFatherNode->AddAttribute("xmlns", "urn:ietf:params:xml:ns:conference-info");
		pFatherNode->AddAttribute("xmlns:xcon", "urn:ietf:params:xml:ns:xcon-conference-info");
		pFatherNode->AddAttribute("xmlns:mrc", "urn:polycom:mrc:xml:ns:conference-info");

		CXMLDOMAttribute* pEntityAttribute = new CXMLDOMAttribute();
		pEntityAttribute->set_nodeName("entity");
		pEntityAttribute->SetValueForElement(m_entityAtr.GetString());
		pFatherNode->AddAttribute(pEntityAttribute);

		SerializeState(pFatherNode, bFull);

		CXMLDOMAttribute* pVersionAttribute = new CXMLDOMAttribute();
		pVersionAttribute->set_nodeName("version");
		char version[11] = "";
		snprintf(version, 10, "%d", m_version);

		pVersionAttribute->SetValueForElement(version);
		pFatherNode->AddAttribute(pVersionAttribute);

		m_pConfDescription->SerializeXml(pFatherNode, bFull);
		m_pHostInfo->SerializeXml(pFatherNode, bFull);
		m_pConfState->SerializeXml(pFatherNode, m_mapUsers, bFull);
		m_pFloorInfo->SerializeXml(pFatherNode, bFull);

		CXMLDOMElement* pUsersNode = NULL;
		pUsersNode = pFatherNode->AddChildNode("users");
		CXMLDOMAttribute* pStateAttribute = new CXMLDOMAttribute();
		pStateAttribute->set_nodeName("state");
		pStateAttribute->SetValueForElement(bFull ? "full" : "partial");
		pUsersNode->AddAttribute(pStateAttribute);

		pUsersNode->AddChildNode(GetTagWithNsPrefix("xcon", "join-handling").c_str(), "allow");
		pUsersNode->AddChildNode(GetTagWithNsPrefix("xcon", "user-admission-policy").c_str(), "anonymous");

		SvcUsersMap::iterator it = m_mapUsers.begin();
		if (0 == m_mapUsers.size())
		{
			TRACEINTO << "No users to add to XML";
		}
		else
		{
			TRACEINTO << "UsersCount:" << m_mapUsers.size() << ", IsFull:" << (bFull ? "true" : "false");
			CSvcUserType* pUser = NULL;
			BYTE wasUpdated = false;
			for (; it != m_mapUsers.end(); ++it)
			{
				pUser = it->second;
				if (!pUser->IsInIVR())
				{
					if (pUser->WasUpdated() || bFull)
					{
						TRACEINTO << "From:" << pUser->m_sFrom.c_str();
						pUser->SerializeXml(pUsersNode, bFull, bIsNewSubscriber, false);
					}
				}
			}
		}
	}
	DelDelletedParties();
}

///////////////////////////////////////////////////////////////////
int CSvcConfInfoType::SerializeXmlFull(std::list<std::string>* pListXml, int nMaxUsersInXml)
{
	TRACEINTO << "UsersCount:" << m_mapUsers.size();
	pListXml->clear();
	SvcUsersMap::iterator itUser = m_mapUsers.begin();
	char pszVersion[11] = "";

	for (int i = 0; true; ++i)
	{
		char* pszXml = NULL;
		DWORD dwXmlLen = 0;
		CXMLDOMElement *pRootElement = new CXMLDOMElement;
		pRootElement->set_nodeName(m_elementName);

		pRootElement->AddAttribute("xmlns", "urn:ietf:params:xml:ns:conference-info");
		pRootElement->AddAttribute("xmlns:xcon", "urn:ietf:params:xml:ns:xcon-conference-info");
		pRootElement->AddAttribute("xmlns:mrc", "urn:polycom:mrc:xml:ns:conference-info");

		pRootElement->AddAttribute("entity", m_entityAtr.GetString());
		snprintf(pszVersion, 10, "%d", i);
		pRootElement->AddAttribute("version", pszVersion);
		pRootElement->AddAttribute("state", i == 0 ? "full" : "partial");

		if (i == 0)
		{
			m_pConfDescription->SerializeXml(pRootElement, TRUE);
			m_pHostInfo->SerializeXml(pRootElement, TRUE);
			m_pConfState->SerializeXml(pRootElement, m_mapUsers, TRUE);
		}
		m_pFloorInfo->SerializeXml(pRootElement, TRUE);

		CXMLDOMElement* pUsersNode = pRootElement->AddChildNode("users");
		pUsersNode->AddAttribute("state", "full");
		pUsersNode->AddChildNode(GetTagWithNsPrefix("xcon", "join-handling").c_str(), "allow");
		pUsersNode->AddChildNode(GetTagWithNsPrefix("xcon", "user-admission-policy").c_str(), "anonymous");

		if (0 == m_mapUsers.size())
		{
			TRACEINTO << "No users to add to XML";
		}
		else
		{
			CSvcUserType* pUser = NULL;
			int n = (i == 0 ? 1 : 0);
			for (; itUser != m_mapUsers.end() && n < nMaxUsersInXml; ++itUser)
			{
				pUser = itUser->second;
				if (pUser->GetState() != eDelletedData)
				{
					pUser->SerializeXml(pUsersNode, TRUE, TRUE, false);
					++n;
				}
			}
		}
		pRootElement->DumpDataAsStringWithAttribute(&pszXml, &dwXmlLen);
		pListXml->push_back(std::string(pszXml, dwXmlLen));
		PDELETE(pRootElement);
		PDELETEA(pszXml);
		if (itUser == m_mapUsers.end())
			break;
	}

	return pListXml->size();
}

///////////////////////////////////////////////////////////////////
long CSvcConfInfoType::SerializeXmlFull(std::string* psFullStr, long iLastUser, int iNotificationVersion, int iXmlMaxUsers)
{
	TRACEINTO << "UsersCount:" << m_mapUsers.size() << ", LastUser:" << iLastUser << ", XmlMaxUsers:" << iXmlMaxUsers;
	SvcUsersMap::iterator itUser = m_mapUsers.upper_bound(iLastUser);
	if (itUser == m_mapUsers.end())
	{
		TRACEINTO << "No users to serialize";
		return EMPTY_LIST;
	}
	char pszVersion[11] = "";
	snprintf(pszVersion, 10, "%d", iNotificationVersion);
	char* pszXml = NULL;
	DWORD dwXmlLen = 0;
	CXMLDOMElement *pRootElement = new CXMLDOMElement;
	pRootElement->set_nodeName(m_elementName);

	pRootElement->AddAttribute("xmlns", "urn:ietf:params:xml:ns:conference-info");
	pRootElement->AddAttribute("xmlns:xcon", "urn:ietf:params:xml:ns:xcon-conference-info");
	pRootElement->AddAttribute("xmlns:mrc", "urn:polycom:mrc:xml:ns:conference-info");
	pRootElement->AddAttribute("entity", m_entityAtr.GetString());
	pRootElement->AddAttribute("version", pszVersion);
	pRootElement->AddAttribute("state", iLastUser == 0 ? "full" : "partial");

	if (iLastUser == 0)
	{
		m_pConfDescription->SerializeXml(pRootElement, TRUE);
		m_pHostInfo->SerializeXml(pRootElement, TRUE);
		m_pConfState->SerializeXml(pRootElement, m_mapUsers, TRUE);
		m_pFloorInfo->SerializeXml(pRootElement, TRUE);
	}
	CXMLDOMElement* pUsersNode = pRootElement->AddChildNode("users");
	pUsersNode->AddAttribute("state", iLastUser == 0 ? "full" : "partial");
	if (iLastUser == 0)
	{
		pUsersNode->AddChildNode(GetTagWithNsPrefix("xcon", "join-handling").c_str(), "allow");
		pUsersNode->AddChildNode(GetTagWithNsPrefix("xcon", "user-admission-policy").c_str(), "anonymous");
	}
	for (int n = 0; itUser != m_mapUsers.end(); ++itUser)
	{
		CSvcUserType* pUser = itUser->second;
		if (pUser->GetState() != eDelletedData)
			pUser->SerializeXmlFull(pUsersNode);

		if (++n >= iXmlMaxUsers)
			break;
	}

	pRootElement->DumpDataAsStringWithAttribute(&pszXml, &dwXmlLen);
	psFullStr->append(std::string(pszXml, dwXmlLen));
	PDELETE(pRootElement);
	PDELETEA(pszXml);

	if (itUser == m_mapUsers.end())
		return 0;
	return itUser->first;
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::SetupUsersList(CCommConf* pCommConf)
{
	CConfParty* pConfParty = pCommConf->GetFirstParty();
	while (pConfParty)
	{
		BOOL bPcasWebFlow = FALSE;
		if (0 == strncmp(pCommConf->GetName(), "RAS200I_web_", 12))
			bPcasWebFlow = TRUE;

		BOOL bIsWebDialInParty = FALSE;
		if (bPcasWebFlow && DIAL_IN == pConfParty->GetConnectionType())
			bIsWebDialInParty = TRUE;

		CPartyCntl* pPartyCtrl = m_pConf->GetPartyCntl(pConfParty->GetName());
		PartyRsrcID partyId = 0;
		if (IsValidPObjectPtr(pPartyCtrl))
			partyId = pPartyCtrl->GetPartyId();

		AddParty(pConfParty, partyId, bIsWebDialInParty);
		pConfParty = pCommConf->GetNextParty();
	}
}

///////////////////////////////////////////////////////////////////
CSvcUserType* CSvcConfInfoType::FindUserByMonitorID(PartyMonitorID partyId)
{
	SvcUsersMap::iterator itend = m_mapUsers.end();
	for (SvcUsersMap::iterator it = m_mapUsers.begin(); it != itend; ++it)
	{
		CSvcUserType* pUser = it->second;
		if (partyId == pUser->GetUserMonitorId())
			return pUser;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////
CSvcUserType* CSvcConfInfoType::FindUserByRsrcID(PartyRsrcID partyId)
{
	SvcUsersMap::iterator itend = m_mapUsers.end();
	for (SvcUsersMap::iterator it = m_mapUsers.begin(); it != itend; ++it)
	{
		CSvcUserType* pUser = it->second;
		if (partyId == pUser->GetUserRsrcId())
			return pUser;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////
long CSvcConfInfoType::FindUserID(CSvcUserType* pUser)
{
	SvcUsersMap::iterator itend = m_mapUsers.end();
	for (SvcUsersMap::iterator it = m_mapUsers.begin(); it != itend; ++it)
	{
		if (pUser == it->second)
			return it->first;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::SetConfContactInfo(const char* contactInfo)
{
	if (strcmp(contactInfo, GetConfContactInfo()))
		m_pConfExtension->SetConfContactInfo(contactInfo);
}

///////////////////////////////////////////////////////////////////
CSvcUserType* CSvcConfInfoType::AddParty(CConfParty* pConfParty, PartyRsrcID partyId, BOOL bIsWebDialInParty)
{
	CSvcUserType* pUser = NULL;
	if (IsValidPObjectPtr(pConfParty))
	{
		if (FindUserByMonitorID(pConfParty->GetPartyId()) == NULL)
		{
			pUser = new CSvcUserType(pConfParty, partyId, bIsWebDialInParty);
			m_mapUsers[++m_iUserCounter] = pUser;
			TRACEINTO << "PartyName:" << pConfParty->GetName() << ", UserCounter:" << m_iUserCounter;
			UPDATE_STATE;
		}
	}
	return pUser;
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::AddParty(CConfParty* pConfParty, const std::string& sFrom, const std::string& sFromTag, const std::string& sTo, BOOL bIsWebDialInParty)
{
	if (IsValidPObjectPtr(pConfParty))
	{
		//verify not already in DB
		if (FindUserByMonitorID(pConfParty->GetPartyId()))
		{
			TRACEINTO << "PartyName:" << pConfParty->GetName() << " - Failed, already in list";
			return;
		}
		TRACEINTO << "PartyName:" << pConfParty->GetName();
		CSvcUserType* pUser = new CSvcUserType(pConfParty, sFrom, sFromTag, sTo, bIsWebDialInParty);
		m_mapUsers[++m_iUserCounter] = pUser;
		UPDATE_STATE;
	}
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::DelParty(PartyMonitorID partyId, int numSubscribers)
{
	TRACEINTO << "PartyMonitorId:" << partyId;

	SvcUsersMap::iterator itend = m_mapUsers.end();
	for (SvcUsersMap::iterator it = m_mapUsers.begin(); it != itend; ++it)
	{
		CSvcUserType* pUser = it->second;
		if (partyId == pUser->GetUserMonitorId())
		{
			if (eFullData == pUser->GetState())
			{
				TRACEINTO << "From:" << it->second->m_sFrom.c_str();
				m_mapUsers.erase(it);
				POBJDELETE(pUser);
				break;
			}

			pUser->MarkForDelete();
			TRACEINTO << "PartyMonitorId:" << partyId << ", UriAttribute:" << pUser->GetUriAttribute() << ", PartyStatus:" << pUser->GetPartyStatus();
			UPDATE_STATE;
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::DelDisconnectedParties()
{
	int len = m_mapUsers.size();

	char a[30];
	*a = 0;
	sprintf(a, "%d", len);
	PTRACE2(eLevelInfoNormal, "CSvcConfInfoType::DelDisconnectedParties, length=", a);

	CSvcUserType* pUser = NULL;
	SvcUsersMap::iterator it = m_mapUsers.begin();
	while (it != m_mapUsers.end())
	{
		pUser = it->second;
		if (IsValidPObjectPtr(pUser))
		{
			char s[100] = "";
			snprintf(s, sizeof(s), "%s, %s", pUser->GetUriAttribute(), pUser->GetPartyStatus());

			if (pUser->IsMarkedForDelete() && pUser->CanDelParty())
			{
				PTRACE2(eLevelInfoNormal, "CSvcConfInfoType::DelDisconnectedParties, deleting: ", s);
				POBJDELETE(pUser);
				m_mapUsers.erase(it++);
				UPDATE_STATE;
			}
			else
			{
				++it;
				PTRACE2(eLevelInfoNormal, "CSvcConfInfoType::DelDisconnectedParties, not deleting: ", s);
			}
		}
		else
			PTRACE(eLevelInfoNormal, "CSvcConfInfoType::DelDisconnectedParties, Invalid pointer.");
	}
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::DelDelletedParties()
{
	int len = m_mapUsers.size();

	char a[30];
	*a = 0;
	sprintf(a, "%d", len);
	PTRACE2(eLevelInfoNormal, "CSvcConfInfoType::DelDelletedParties, length=", a);

	CSvcUserType* pUser = NULL;
	SvcUsersMap::iterator it = m_mapUsers.begin();
	while (it != m_mapUsers.end())
	{
		pUser = it->second;
		if (IsValidPObjectPtr(pUser))
		{
			if (pUser->GetState() == eDelletedData && pUser->IsMarkedForDelete())
			{
				PTRACE2(eLevelInfoNormal, "CSvcConfInfoType::DelDelletedParties, sFrom = ", it->second->m_sFrom.c_str());
				POBJDELETE(pUser);
				m_mapUsers.erase(it++);
			}
			else
			{
				++it;
			}
		}
		else
			PTRACE(eLevelInfoNormal, "CSvcConfInfoType::DelDelletedParties, Invalid pointer.");
	}
}

///////////////////////////////////////////////////////////////////
bool CSvcConfInfoType::UpdateMedia(CConfParty* pConfParty, PartyRsrcID partyId, bool& bUrgent, CSvcUserType** ppUserUrgentUpdated)
{
	bool bChanged = false;
	bool bNew = false;

	*ppUserUrgentUpdated = NULL;

	CSvcUserType* pUser = FindUserByMonitorID(pConfParty->GetPartyId());

	if (pUser == NULL)
	{
		BOOL bPcasWebFlow = FALSE;
		if (0 == strncmp(m_pConf->GetName(), "RAS200I_web_", 12))
			bPcasWebFlow = TRUE;
		BOOL bIsWebDialInParty = FALSE;
		if (bPcasWebFlow && DIAL_IN == pConfParty->GetConnectionType())
			bIsWebDialInParty = TRUE;

		pUser = AddParty(pConfParty, partyId, bIsWebDialInParty);
		bNew = true;
	}
	else
		pUser->SetUserRsrcId(partyId);

	if (IsValidPObjectPtr(pUser))
	{
		bChanged = pUser->UpdateMedia(pConfParty, bUrgent);

		if (bUrgent && (bChanged || bNew))
			*ppUserUrgentUpdated = pUser;

		if ((!bUrgent && bChanged) || bNew)
			UPDATE_STATE;

		if (pUser->IsInIVR())
		{
			TRACEINTO << "PartyMonitorID:" << pUser->GetUserMonitorId() << ", PartyId:" << pUser->GetUserRsrcId();
			return false;
		}
	}
	else
		PTRACE2INT(eLevelError, "CSvcConfInfoType::UpdateMedia, user not found. "
				"\n\tPartyMonitorID = ", pConfParty->GetPartyId());

	return bChanged || bNew;
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::PartyLeaveIVR(PartyRsrcID partyId)
{
	CSvcUserType* pUser = FindUserByRsrcID(partyId);
	if (pUser)
		pUser->LeaveIVR();
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::SetEndPointStatus(PartyMonitorID partyId, DWORD status)
{
	CSvcUserType* pUser = FindUserByMonitorID(partyId);
	if (IsValidPObjectPtr(pUser))
	{
		BYTE isChanged = pUser->SetEndPointStatus(status);
		if (isChanged)
			UPDATE_STATE;
	}
	else
	{
		TRACEINTO << "PartyMonitorId:" << partyId << " - Failed, not found";
	}
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::SetUserContactInfo(PartyMonitorID partyId, const char* userContactInfo)
{
	CSvcUserType* pUser = FindUserByMonitorID(partyId);
	if (IsValidPObjectPtr(pUser))
	{
		pUser->SetUserContactInfo(userContactInfo);
	}
	else
	{
		TRACEINTO << "PartyMonitorId:" << partyId << " - Failed, not found";
	}
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::SetActiveSpeaker(PartyMonitorID partyId, const char* speaker)
{
	CSvcUserType* pUser = FindUserByMonitorID(partyId);
	if (IsValidPObjectPtr(pUser))
	{
		SvcUsersMap::iterator itend = m_mapUsers.end();
		for (SvcUsersMap::iterator it = m_mapUsers.begin(); it != itend; ++it)
		{
			it->second->SetActiveSpeaker(pUser == it->second);
			UPDATE_STATE;
		}
	}

	if (IsValidPObjectPtr(pUser) && strcmp(m_pConfExtension->GetActiveSpeaker(), speaker))
	{
		if (strcmp("", speaker))
		{
			m_pConfExtension->SetActiveSpeaker(speaker, pUser->GetUriAttribute());
			m_pConfExtension->SetSpeakerContactInfo(pUser->GetUserContactInfo());
			m_pConfExtension->SetSpeakerPartyId(partyId);
			UPDATE_STATE;
		}
		else
		{
			m_pConfExtension->SetActiveSpeaker(speaker, speaker);
			m_pConfExtension->SetSpeakerContactInfo(speaker);
			m_pConfExtension->SetSpeakerPartyId(partyId);
		}
	}
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::SetUriAttribute(PartyMonitorID partyId, char* uri)
{
	if (uri)
	{
		CSvcUserType* pUser = FindUserByMonitorID(partyId);
		if (IsValidPObjectPtr(pUser))
		{
			pUser->SetUriAttribute(uri);
			UPDATE_STATE;
		}
	}
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::MutePartyMedia(PartyMonitorID partyId, eMediaContentType mediaType, BYTE onOff)
{
	CSvcUserType* pUser = FindUserByMonitorID(partyId);
	if (IsValidPObjectPtr(pUser) && pUser->IsMediaMuted(mediaType) != onOff)
	{
		pUser->MuteMedia(mediaType, onOff);
		UPDATE_STATE;
	}
}

///////////////////////////////////////////////////////////////////
void CSvcConfInfoType::MuteViaFocus(PartyMonitorID partyId, eMediaContentType mediaType, BYTE mutedViaFocus)
{
	CSvcUserType* pUser = FindUserByMonitorID(partyId);
	if (IsValidPObjectPtr(pUser) && pUser->IsMutedViaFocus(mediaType) != mutedViaFocus)
	{
		pUser->MuteViaFocus(mutedViaFocus, mediaType);
		UPDATE_STATE;
	}
}

// ************************************************************************************
//
//	CSvcConfExtension
//
// ************************************************************************************
///////////////////////////////////////////////////////////////////
CSvcConfExtension::CSvcConfExtension() :
		CSvcConfPackageType("ex-data"), m_bActive(true), m_confContactInfo("")
{
	m_pActiveSpeaker = new CSvcActiveSpeaker();
	m_confId = 0;
}

///////////////////////////////////////////////////////////////////
CSvcConfExtension::CSvcConfExtension(const char* contactInfo, DWORD confId) :
		CSvcConfPackageType("ex-data"), m_bActive(true), m_confContactInfo(contactInfo), m_confId(confId)
{
	m_pActiveSpeaker = new CSvcActiveSpeaker();
}

///////////////////////////////////////////////////////////////////
CSvcConfExtension::~CSvcConfExtension()
{
	POBJDELETE(m_pActiveSpeaker);
}

///////////////////////////////////////////////////////////////////
void CSvcConfExtension::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull)
{
	CXMLDOMElement* pFeatureNode = NULL;

	pFeatureNode = pFatherNode->AddChildNode(GetTagWithNsPrefix(m_elementName).c_str());

	pFeatureNode->AddChildNode(GetTagWithNsPrefix("conf-id").c_str(), m_confId);
	pFeatureNode->AddChildNode(GetTagWithNsPrefix("contact-info").c_str(), m_confContactInfo);
	if (0xFFFF != GetSpeakerPartyId())
		m_pActiveSpeaker->SerializeXml(pFeatureNode, bFull);
}

///////////////////////////////////////////////////////////////////
void CSvcConfExtension::SetSpeakerContactInfo(const char* userContactInfo)
{
	if (userContactInfo)
		m_pActiveSpeaker->SetSpeakerContactInfo(userContactInfo);
}

///////////////////////////////////////////////////////////////////
void CSvcConfExtension::SetConfContactInfo(const char* contactInfo)
{
	if (contactInfo)
		m_confContactInfo = contactInfo;
}

// ************************************************************************************
//
//	CSvcActiveSpeaker
//
// ************************************************************************************
///////////////////////////////////////////////////////////////////
CSvcActiveSpeaker::CSvcActiveSpeaker() :
		CSvcConfPackageType("active-speaker"), m_speakerUri(""), m_speakerContactInfo(""), m_partyId(0xFFFF)
{

}

///////////////////////////////////////////////////////////////////
CSvcActiveSpeaker::~CSvcActiveSpeaker()
{
}

///////////////////////////////////////////////////////////////////
void CSvcActiveSpeaker::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull)
{
	CXMLDOMElement* pFeatureNode = NULL;
	/*****/
	CXMLDOMElement* pActiveSpeakerNode = NULL;
	/*****/
	if (bFull || eNoChange != m_state)
	{

		pFeatureNode = pFatherNode->AddChildNode(GetTagWithNsPrefix(m_elementName).c_str());

		CXMLDOMAttribute* pUriAttribute = new CXMLDOMAttribute();
		pUriAttribute->set_nodeName("entity");
		pUriAttribute->SetValueForElement(GetSpeakerUri());
		pFeatureNode->AddAttribute(pUriAttribute);

		pFeatureNode->AddChildNode(GetTagWithNsPrefix("party-id").c_str(), m_partyId);
		pFeatureNode->AddChildNode(GetTagWithNsPrefix("contact-info").c_str(), GetSpeakerContactInfo());

	}
	if (!bFull)
		m_state = eNoChange;
}

///////////////////////////////////////////////////////////////////
void CSvcActiveSpeaker::SetActiveSpeaker(const char* activeSpeaker, const char* speakerUri)
{
	if (activeSpeaker)
	{
		if (speakerUri)
		{
			m_speakerUri = speakerUri;
		}
		m_activeSpeaker = activeSpeaker;
		if (strcmp("", activeSpeaker))
			UPDATE_STATE;
	}
}

///////////////////////////////////////////////////////////////////
const char* CSvcActiveSpeaker::GetActiveSpeaker() const
{
	return m_activeSpeaker.GetString();
}

///////////////////////////////////////////////////////////////////
const char* CSvcActiveSpeaker::GetSpeakerUri() const
{
	return m_speakerUri.GetString();
}

///////////////////////////////////////////////////////////////////
void CSvcActiveSpeaker::SetSpeakerContactInfo(const char* userContactInfo)
{
	if (userContactInfo)
		m_speakerContactInfo = userContactInfo;
}

///////////////////////////////////////////////////////////////////
const char* CSvcActiveSpeaker::GetSpeakerContactInfo() const
{
	return m_speakerContactInfo.GetString();
}

///////////////////////////////////////////////////////////////////
void CSvcActiveSpeaker::SetSpeakerPartyId(DWORD partyId)
{
	m_partyId = partyId;
}

///////////////////////////////////////////////////////////////////
DWORD CSvcActiveSpeaker::GetSpeakerPartyId()
{
	return m_partyId;
}

///////////////////////////////////////////////////////////////////
CSvcUserType::CSvcUserType() :
		CSvcConfPackageType("user"), m_bInIVR(true), m_bIsWebDialInParty(FALSE), m_dwUserMonitorId(0), m_dwUserRsrcId(0), m_bIsAudioOnly(FALSE), m_uriAttribute(""), m_userContactInfo(""), m_markedforDelete(FALSE), m_longTime(0)
{
	m_pEndPoint = new CSvcEndPointType();
	m_pUserActions = NULL;
	m_pPartyExtension = new CSvcPartyExtension();
	m_pConfParty = NULL;
}

///////////////////////////////////////////////////////////////////
CSvcUserType::CSvcUserType(CConfParty* pConfParty, DWORD dwUserRsrcId, BOOL bIsWebDialInParty) :
		CSvcConfPackageType("user"), m_bInIVR(true), m_bIsWebDialInParty(bIsWebDialInParty), m_markedforDelete(FALSE)
{
	m_pUserActions = NULL;

	m_dwUserMonitorId = pConfParty->GetPartyId();
	m_dwUserRsrcId = dwUserRsrcId;
	m_bIsAudioOnly = pConfParty->GetVoice();
	bool bIsPSTN = NO;

	if (pConfParty->GetUserDefinedInfo(0))
	{
		m_userContactInfo = pConfParty->GetUserDefinedInfo(0);
	}
	m_pConfParty = pConfParty;

	//SIP party
	if (SIP_INTERFACE_TYPE == pConfParty->GetNetInterfaceType())
	{
		const char* pSipUri = pConfParty->GetSipPartyAddress();
		PTRACE2(eLevelInfoNormal, "CSvcUserType::CSvcUserType - pSipUri = ", pSipUri);
		m_sFrom = pSipUri;
		char pszIpAddress[IPV6_ADDRESS_LEN];
		memset(pszIpAddress, '\0', IPV6_ADDRESS_LEN);
		ipToString(pConfParty->GetIpAddress(), pszIpAddress, 1);
		m_sTo = pszIpAddress; //"10.227.2.129";
		union asd
		{
			unsigned char v[4];
			int z;
			asd()
			{
				memset(this, 0, sizeof(*this));
			}
		} ccc;

		char *pStr = (char*)strchr(pSipUri, '@');
		char buff[20] = "";
		//sscanf assign 4 bytes, so need to use with int
		int a, b, c, d;
		if (pStr)
		{
			sscanf(pStr, "@%d.%d.%d.%d", &a, &b, &c, &d);
			ccc.v[0] = (unsigned char)a;
			ccc.v[1] = (unsigned char)b;
			ccc.v[2] = (unsigned char)c;
			ccc.v[3] = (unsigned char)d;

			sprintf(buff, "%u", ccc.z);
		}
		else if (pSipUri)
		{
			sscanf(pSipUri, "%d.%d.%d.%d", &a, &b, &c, &d);
			ccc.v[0] = (unsigned char)a;
			ccc.v[1] = (unsigned char)b;
			ccc.v[2] = (unsigned char)c;
			ccc.v[3] = (unsigned char)d;
			sprintf(buff, "%u", ccc.z);
			char buffHdx[50] = "";
			sprintf(buffHdx, "hdx-%u@", pConfParty->GetPartyId());
			m_sFrom = std::string(buffHdx) + m_sFrom;
		}
		else
		{
			sprintf(buff, "%u", 123456);
			char buffHdx[50] = "";
			sprintf(buffHdx, "hdx-%u@123456", pConfParty->GetPartyId());
			m_sFrom = std::string(buffHdx) + m_sFrom;
		}

		CStructTm dtCurrent;
		SystemGetTime(dtCurrent);
		m_longTime = dtCurrent.GetAbsTime(1);
		char buff50[50] = "";
		sprintf(buff50, "%8X-%04u-5678-aaaa-", static_cast<unsigned int>(m_longTime), pConfParty->GetPartyId());
		m_sFromTag = buff50;
		m_sFromTag += buff;

		if (*pSipUri != '\0')
		{
			m_uriAttribute << "sip:" << pSipUri;
		}
		else
		{
			char Ip[16];
			Ip[0] = '\0';
			m_uriAttribute << "sip:" << pConfParty->GetName();
			PTRACE2(eLevelInfoNormal, "CSvcUserType::CSvcUserType - m_uriAttribute = ", m_uriAttribute.GetString());
		}
		PTRACE2(eLevelInfoNormal, "CSvcUserType::CSvcUserType - m_uriAttribute = ", m_uriAttribute.GetString());
	}
	else //not SIP party
	{
		CStructTm dtCurrent;
		SystemGetTime(dtCurrent);
		m_longTime = dtCurrent.GetAbsTime(1);
		char buff50[50] = "";
		sprintf(buff50, "%8X-%04u-5678-aaaa-123456789", static_cast<unsigned int>(m_longTime), pConfParty->GetPartyId());
		m_sFromTag = buff50;
		sprintf(buff50, "-%04u", pConfParty->GetPartyId());
		m_sFrom = std::string(pConfParty->GetName()) + std::string(buff50);

		if (H323_INTERFACE_TYPE == pConfParty->GetNetInterfaceType())
		{
			const char* pAlias = pConfParty->GetH323PartyAlias();
			if (pAlias[0] != '\0')
			{
				m_uriAttribute << "sip:" << pAlias;
			}
			else
			{
				char Ip[16];
				Ip[0] = '\0';
				m_uriAttribute << "sip:" << pConfParty->GetName();
			}
		}
		else if (ISDN_INTERFACE_TYPE == pConfParty->GetNetInterfaceType())
		{
			if (DIAL_OUT == pConfParty->GetConnectionType())
				m_uriAttribute << "sip:" << pConfParty->GetPhoneNumber();
			else
				m_uriAttribute << "sip:" << pConfParty->GetActualPartyPhoneNumber(0)->phone_number;

		}
	}
	PTRACE2(eLevelInfoNormal, "CSvcUserType::CSvcUserType, pUserpSipUri = ", m_uriAttribute.GetString());
	// add the service's sip host name
	if (m_uriAttribute.Find("@") == NO)
	{
		char Ip[16];
		Ip[0] = '\0';

		if (ISDN_INTERFACE_TYPE != pConfParty->GetNetInterfaceType())
		{
			m_uriAttribute << "@" << Ip;
		}

		if (H323_INTERFACE_TYPE == pConfParty->GetNetInterfaceType())
		{
			m_uriAttribute << ";user=h323";
		}
		else if (ISDN_INTERFACE_TYPE == pConfParty->GetNetInterfaceType())
		{
			const char * serv_name = pConfParty->GetServiceProviderName();
			CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
			CSmallString hostName = "";

			if (strlen(serv_name) != 0)
			{
				CConfIpParameters* pServiceParams = NULL;
				pServiceParams = pIpServiceListManager->GetRelevantService(serv_name, pConfParty->GetNetInterfaceType());

				CSmallString registrarDomainName = "";
				CSmallString ProxyName = "";

				hostName = pServiceParams->GetRegistrarDomainName();
				if (hostName.IsEmpty())
					hostName = pServiceParams->GetSipProxyName();

				registrarDomainName = pServiceParams->GetRegistrarDomainName();
				ProxyName = pServiceParams->GetSipProxyName();
			}
			m_uriAttribute << "@" << hostName.GetString() << ";user=phone";
		}
	}
	m_pPartyExtension = new CSvcPartyExtension(m_uriAttribute.GetString(), m_userContactInfo.GetString(), m_dwUserMonitorId); //exten

	m_pEndPoint = new CSvcEndPointType(m_uriAttribute.GetString(), m_sFrom, m_sTo, pConfParty /*,m_bIsAudioOnly*/);
	SetEndPointStatus(pConfParty->GetPartyState());
}

///////////////////////////////////////////////////////////////////
CSvcUserType::~CSvcUserType()
{
	PTRACE2(eLevelInfoNormal, "CSvcUserType::~CSvcUserType - m_sFrom = ", m_sFrom.c_str());
	POBJDELETE(m_pEndPoint);
	POBJDELETE(m_pPartyExtension); //exten
}

///////////////////////////////////////////////////////////////////
CSvcUserType::CSvcUserType(CConfParty* pConfParty, std::string sFrom, std::string sFromTag, std::string sTo, BOOL bIsWebDialInParty) :
		CSvcConfPackageType("user"), m_bInIVR(true), m_bIsWebDialInParty(bIsWebDialInParty), m_markedforDelete(FALSE)
{
	m_sFrom = sFrom;
	m_sFromTag = sFromTag;
	m_sTo = sTo;
	m_dwUserMonitorId = pConfParty->GetPartyId();
	m_bIsAudioOnly = pConfParty->GetVoice();
	bool bIsPSTN = NO;

	if (pConfParty->GetUserDefinedInfo(0))
	{
		m_userContactInfo = pConfParty->GetUserDefinedInfo(0);
	}
	m_pConfParty = pConfParty;
	//SIP party
	if (SIP_INTERFACE_TYPE == pConfParty->GetNetInterfaceType())
	{

		const char* pSipUri = pConfParty->GetSipPartyAddress();
		PTRACE2(eLevelInfoNormal, "CSvcUserType::CSvcUserType - pSipUri = ", pSipUri);

		if (*pSipUri != '\0')
		{
			m_uriAttribute << "sip:" << pSipUri;
		}
		else
		{
			char Ip[16];
			Ip[0] = '\0';
			m_uriAttribute << "sip:" << pConfParty->GetName();
			PTRACE2(eLevelInfoNormal, "CSvcUserType::CSvcUserType - m_uriAttribute = ", m_uriAttribute.GetString());
		}
		PTRACE2(eLevelInfoNormal, "CSvcUserType::CSvcUserType - m_uriAttribute = ", m_uriAttribute.GetString());
	}
	else //not SIP party
	{
		if (H323_INTERFACE_TYPE == pConfParty->GetNetInterfaceType())
		{
			const char* pAlias = pConfParty->GetH323PartyAlias();
			if (pAlias[0] != '\0')
			{
				m_uriAttribute << "sip:" << pAlias;
			}
			else
			{
				char Ip[16];
				Ip[0] = '\0';
				m_uriAttribute << "sip:" << pConfParty->GetName();
			}
		}
		else if (ISDN_INTERFACE_TYPE == pConfParty->GetNetInterfaceType())
		{
			if (DIAL_OUT == pConfParty->GetConnectionType())
				m_uriAttribute << "sip:" << pConfParty->GetPhoneNumber();
			else
				m_uriAttribute << "sip:" << pConfParty->GetActualPartyPhoneNumber(0)->phone_number;
		}
	}
	TRACEINTO << "CSvcUserType::CSvcUserType, pUserpSipUri = " << m_uriAttribute.GetString();
	// add the service's sip host name
	if (m_uriAttribute.Find("@") == NO)
	{
		char Ip[16];
		Ip[0] = '\0';

		if (ISDN_INTERFACE_TYPE != pConfParty->GetNetInterfaceType())
		{
			m_uriAttribute << "@" << Ip;
		}

		if (H323_INTERFACE_TYPE == pConfParty->GetNetInterfaceType())
		{
			m_uriAttribute << ";user=h323";
		}
		else if (ISDN_INTERFACE_TYPE == pConfParty->GetNetInterfaceType())
		{
			const char * serv_name = pConfParty->GetServiceProviderName();
			CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
			CSmallString hostName = "";

			if (strlen(serv_name) != 0)
			{
				CConfIpParameters* pServiceParams = NULL;
				pServiceParams = pIpServiceListManager->GetRelevantService(serv_name, pConfParty->GetNetInterfaceType());

				CSmallString registrarDomainName = "";
				CSmallString ProxyName = "";

				hostName = pServiceParams->GetRegistrarDomainName();
				if (hostName.IsEmpty())
					hostName = pServiceParams->GetSipProxyName();

				registrarDomainName = pServiceParams->GetRegistrarDomainName();
				ProxyName = pServiceParams->GetSipProxyName();
			}
			m_uriAttribute << "@" << hostName.GetString() << ";user=phone";
		}
	}
	m_pPartyExtension = new CSvcPartyExtension(m_uriAttribute.GetString(), m_userContactInfo.GetString(), m_dwUserMonitorId); //exten

	m_pEndPoint = new CSvcEndPointType(m_uriAttribute.GetString(), sFrom, sTo, pConfParty /*,m_bIsAudioOnly*/);
	SetEndPointStatus(pConfParty->GetPartyState());
}

///////////////////////////////////////////////////////////////////
void CSvcUserType::MarkForDelete()
{
	m_markedforDelete = TRUE;
	m_state = eDelletedData;
	m_pPartyExtension->SetUserActionType(eDeleted);
}

///////////////////////////////////////////////////////////////////
BYTE CSvcUserType::IsMarkedForDelete()
{
	return m_markedforDelete;
}

///////////////////////////////////////////////////////////////////
BYTE CSvcUserType::CanDelParty()
{
	BYTE result = FALSE;

	if (!m_markedforDelete)
	{
		switch (m_pEndPoint->GetEndPointStatus())
		{

			case (e_Pending):
			{
				result = TRUE;
				break;
			}
			case (e_OnHold):
			case (e_Connected):
			case (e_Disconnecting):
			{
				result = FALSE;
				break;
			}
			default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;
		}
	}
	return result;
}

///////////////////////////////////////////////////////////////////
void CSvcUserType::SetUserContactInfo(const char* userContactInfo)
{
	if (userContactInfo)
	{
		m_pPartyExtension->SetUserContactInfo(userContactInfo);
		m_userContactInfo = userContactInfo;
		UPDATE_STATE;
	}
}

///////////////////////////////////////////////////////////////////
void CSvcUserType::SetMediaStatus(eMediaStatusType mediaType)
{
	m_pEndPoint->SetMediaStatus(mediaType);
	m_pPartyExtension->SetUserActionType(eMuteChanged); //exten
}

///////////////////////////////////////////////////////////////////
BYTE CSvcUserType::SetEndPointStatus(DWORD status)
{
	BYTE isChanged = m_pEndPoint->SetEndPointStatus(status);

	if ((PARTY_DELETED_BY_OPERATOR == status) || (PARTY_IDLE == status && eNoChange != /*m_pUserActions->GetUserActionTypeState(eAdded)*/m_pPartyExtension->GetUserActionTypeState(eAdded)))
		return false;

	if (isChanged)
	{
		UPDATE_STATE;
		m_pPartyExtension->SetUserActionType(eConnectionChanged);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////
void CSvcUserType::MuteViaFocus(BYTE mutedViaFocus, eMediaContentType mediaType)
{
	m_pEndPoint->MuteViaFocus(mutedViaFocus, mediaType);
	m_pEndPoint->MuteMedia(mediaType, mutedViaFocus);
	m_pPartyExtension->SetUserActionType(eMuteChanged);
	UPDATE_STATE;
}

///////////////////////////////////////////////////////////////////
BYTE CSvcUserType::IsMutedViaFocus(eMediaContentType mediaType)
{
	return m_pEndPoint->IsMutedViaFocus(mediaType);
}

///////////////////////////////////////////////////////////////////
void CSvcUserType::SetUriAttribute(char* uri)
{
	if (uri)
	{
		m_uriAttribute = uri;
		UPDATE_STATE;
	}
}

///////////////////////////////////////////////////////////////////
void CSvcUserType::SetActiveSpeaker(bool bSpeaker)
{
	if (m_pEndPoint && m_pEndPoint->IsActiveSpeaker() != bSpeaker)
	{
		m_pEndPoint->SetActiveSpeaker(bSpeaker);
		UPDATE_STATE;
	}
}

///////////////////////////////////////////////////////////////////
char* CSvcUserType::GetPartyStatus() const
{
	return m_pEndPoint->GetEndPointStatusByString();
}

///////////////////////////////////////////////////////////////////
void CSvcUserType::MuteMedia(eMediaContentType mediaType, BYTE onOff)
{
	m_pEndPoint->MuteMedia(mediaType, onOff);
	m_pPartyExtension->SetUserActionType(eMuteChanged);
	UPDATE_STATE;
}

///////////////////////////////////////////////////////////////////
void CSvcUserType::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull)
{
	SerializeXml(pFatherNode, bFull, FALSE, false);
}

///////////////////////////////////////////////////////////////////
void CSvcUserType::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull, BYTE bIsNewSubscriber, bool bUrgent)
{
	CXMLDOMElement* pFeatureNode = NULL;

	CXMLDOMElement* pExDataNode = NULL;

	if (!WasUpdated() && !bIsNewSubscriber && !bUrgent)
	{
		PTRACE(eLevelInfoNormal, "CSvcUserType::SerializeXml, not was updated");
		return;
	}

	if (bIsNewSubscriber && (e_Disconnected == m_pEndPoint->GetEndPointStatus()))
	{
		PTRACE(eLevelInfoNormal, "CSvcUserType::SerializeXml, not serializing data about disconnected party");
		return;
	}

	if (eDelletedData == m_state)
	{
		PTRACE2(eLevelInfoNormal, "CSvcUserType::SerializeXml, deleted user data, user = ", m_uriAttribute.GetString());
		pFeatureNode = pFatherNode->AddChildNode(m_elementName);

		CXMLDOMAttribute* pUriAttribute = new CXMLDOMAttribute();
		pUriAttribute->set_nodeName("entity");
		std::string sTmp = "xcon-userid:";
		sTmp += m_sFromTag;
		pUriAttribute->SetValueForElement(sTmp.c_str());
		pFeatureNode->AddAttribute(pUriAttribute);

		CXMLDOMAttribute* pStateAttribute = new CXMLDOMAttribute();
		pStateAttribute->set_nodeName("state");
		pStateAttribute->SetValueForElement("deleted");
		pFeatureNode->AddAttribute(pStateAttribute);
		SetOld();
		return;
	}

	//Full data
	if (bFull || bIsNewSubscriber || eFullData == m_state || IsNew())
	{
		PTRACE2(eLevelInfoNormal, "CSvcUserType::SerializeXml, Full user data, user = ", m_uriAttribute.GetString());
		pFeatureNode = pFatherNode->AddChildNode(m_elementName);

		CXMLDOMAttribute* pUriAttribute = new CXMLDOMAttribute();
		pUriAttribute->set_nodeName("entity");
		std::string sTmp = "xcon-userid:";
		sTmp += m_sFromTag;
		pUriAttribute->SetValueForElement(sTmp.c_str());
		pFeatureNode->AddAttribute(pUriAttribute);

		std::string sName = (const_cast<CConfParty*>(m_pConfParty))->GetVisualPartyName();
		if (sName.empty())
			sName = (const_cast<CConfParty*>(m_pConfParty))->GetName();
		pFeatureNode->AddChildNode("display-text", sName.c_str());
		SerializeState(pFeatureNode, bFull);

		CXMLDOMElement* pRoles = pFeatureNode->AddChildNode("roles");
		pRoles->AddChildNode("entry", "participant");

		m_pEndPoint->SerializeXml(pFeatureNode, bFull || IsNew(), bIsNewSubscriber, m_bIsAudioOnly, false);
	}
	else if (ePartialData == m_state || bUrgent) //Only delta
	{
		PTRACE2(eLevelInfoNormal, "CSvcUserType::SerializeXml, Partial user data, user = ", m_uriAttribute.GetString());
		pFeatureNode = pFatherNode->AddChildNode(m_elementName);

		CXMLDOMAttribute* pUriAttribute = new CXMLDOMAttribute();
		pUriAttribute->set_nodeName("entity");
		std::string sTmp = "xcon-userid:";
		sTmp += m_sFromTag;
		pUriAttribute->SetValueForElement(sTmp.c_str());
		pFeatureNode->AddAttribute(pUriAttribute);
		SerializeState(pFeatureNode, bFull);

		m_pEndPoint->SerializeXml(pFeatureNode, bFull, false, m_bIsAudioOnly, bUrgent);
	}
	SetOld();
}

///////////////////////////////////////////////////////////////////
void CSvcUserType::SerializeXmlFull(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pFeatureNode = NULL;

	PTRACE2(eLevelInfoNormal, "CSvcUserType::SerializeXmlFull, user = ", m_uriAttribute.GetString());
	pFeatureNode = pFatherNode->AddChildNode(m_elementName);

	CXMLDOMAttribute* pUriAttribute = new CXMLDOMAttribute();
	pUriAttribute->set_nodeName("entity");
	std::string sTmp = "xcon-userid:";
	sTmp += m_sFromTag;
	pUriAttribute->SetValueForElement(sTmp.c_str());
	pFeatureNode->AddAttribute(pUriAttribute);

	std::string sName = (const_cast<CConfParty*>(m_pConfParty))->GetVisualPartyName();
	if (sName.empty())
		sName = (const_cast<CConfParty*>(m_pConfParty))->GetName();
	pFeatureNode->AddChildNode("display-text", sName.c_str());
	SerializeState(pFeatureNode, TRUE);

	CXMLDOMElement* pRoles = pFeatureNode->AddChildNode("roles");
	pRoles->AddChildNode("entry", "participant");

	m_pEndPoint->SerializeXmlFull(pFeatureNode);
}

///////////////////////////////////////////////////////////////////
bool CSvcUserType::UpdateMedia(CConfParty* pConfParty, bool& bUrgent)
{
	bool bChanged = m_pEndPoint->UpdateMedia(pConfParty, bUrgent);
	if (bChanged && !bUrgent)
		UPDATE_STATE;
	return bChanged;
}


// ************************************************************************************
//
//	CSvcPartyExtension
//
// ************************************************************************************

CSvcPartyExtension::CSvcPartyExtension() :
		CSvcConfPackageType("ex-data"), m_partyId(0), m_userContactInfo("")
{
	m_pUserActions = new CSvcUserActions();

}

///////////////////////////////////////////////////////////////////
CSvcPartyExtension::CSvcPartyExtension(const char* uriAttribute, const char* userContactInfo, DWORD partyId) :
		CSvcConfPackageType("ex-data"), m_partyId(partyId), m_userContactInfo(userContactInfo)
{
	m_pUserActions = new CSvcUserActions(uriAttribute);
	m_pUserActions->SetUserActionType(eAdded);
}

///////////////////////////////////////////////////////////////////
CSvcPartyExtension::~CSvcPartyExtension()
{
	POBJDELETE(m_pUserActions);
}

///////////////////////////////////////////////////////////////////
void CSvcPartyExtension::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull, WORD bIsAudioOnly)
{
}

///////////////////////////////////////////////////////////////////
void CSvcPartyExtension::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull)
{
	CXMLDOMElement* pFeatureNode = NULL;

	PTRACE(eLevelInfoNormal, "CSvcPartyExtension::SerializeXml");

	pFeatureNode = pFatherNode->AddChildNode(GetTagWithNsPrefix(m_elementName).c_str());
	m_pUserActions->SerializeXml(pFeatureNode, bFull);

	pFeatureNode->AddChildNode(GetTagWithNsPrefix("contact-info").c_str(), m_userContactInfo);
	pFeatureNode->AddChildNode(GetTagWithNsPrefix("party-id").c_str(), m_partyId);
}

///////////////////////////////////////////////////////////////////
eTypeState CSvcPartyExtension::GetUserActionTypeState(eActionType actionType)
{
	return m_pUserActions->GetUserActionTypeState(actionType);
}

///////////////////////////////////////////////////////////////////
void CSvcPartyExtension::SetUserActionType(eActionType type)
{
	m_pUserActions->SetUserActionType(type);
}

///////////////////////////////////////////////////////////////////
const char* CSvcPartyExtension::GetUserContactInfo() const
{
	return m_userContactInfo.GetString();
}

///////////////////////////////////////////////////////////////////
void CSvcPartyExtension::SetUserContactInfo(const char* userContactInfo)
{
	if (userContactInfo)
	{
		m_userContactInfo = userContactInfo;
	}
}

///////////////////////////////////////////////////////////////////
DWORD CSvcPartyExtension::GetPartyId()
{
	return m_partyId;
}

// ************************************************************************************
//
//	CSvcUserActions
//
// ************************************************************************************
CSvcUserActions::CSvcUserActions() :
		CSvcConfPackageType("user-actions"), m_userEntity("")
{
	for (int i = 0; i < NUM_OF_ACTIONS; i++)
		m_pActionStream[i] = NULL;
}

///////////////////////////////////////////////////////////////////
CSvcUserActions::CSvcUserActions(const char* userEntity) :
		CSvcConfPackageType("user-actions"), m_userEntity(userEntity)
{
	for (int i = 0; i < NUM_OF_ACTIONS; i++)
		m_pActionStream[i] = NULL;

	m_pActionStream[0] = new CSvcActionType(eConnectionChanged);
	m_pActionStream[1] = new CSvcActionType(eMuteChanged);
	m_pActionStream[2] = new CSvcActionType(eAdded);
	m_pActionStream[3] = new CSvcActionType(eDeleted);
}

///////////////////////////////////////////////////////////////////
CSvcUserActions::~CSvcUserActions()
{
	for (int i = 0; i < NUM_OF_ACTIONS; i++)
		if (m_pActionStream[i])
			POBJDELETE(m_pActionStream[i]);
}

///////////////////////////////////////////////////////////////////
void CSvcUserActions::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull)
{
	CXMLDOMElement* pFeatureNode = NULL;
	if (eNoChange != m_state)
	{
		pFeatureNode = pFatherNode->AddChildNode(GetTagWithNsPrefix(m_elementName).c_str());

		for (int i = 0; i < NUM_OF_ACTIONS; i++)
		{
			if (m_pActionStream[i])
			{
				m_pActionStream[i]->SerializeXml(pFeatureNode, bFull);
			}
		}
	}
	if (!bFull)
		m_state = eNoChange;
}

///////////////////////////////////////////////////////////////////
void CSvcUserActions::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull, WORD bIsAudioOnly)
{
}

///////////////////////////////////////////////////////////////////
void CSvcUserActions::SetUserActionType(eActionType actionType)
{
	for (int i = 0; i < NUM_OF_ACTIONS; i++)
	{
		if (m_pActionStream[i]->GetActionType() == actionType)
		{
			m_pActionStream[i]->SetActionTypeStatus(actionType);
		}
	}
	UPDATE_STATE;
}

///////////////////////////////////////////////////////////////////
eTypeState CSvcUserActions::GetUserActionTypeState(eActionType actionType)
{
	switch (actionType)
	{
		case (eConnectionChanged):
			return m_pActionStream[0]->GetState();

		case (eMuteChanged):
			return m_pActionStream[1]->GetState();

		case (eAdded):
			return m_pActionStream[2]->GetState();

		case (eDeleted):
			return m_pActionStream[3]->GetState();

		default:
			return eNoChange;

	}
}
// ************************************************************************************
//
//	CSvcActionType
//
// ************************************************************************************
///////////////////////////////////////////////////////////////////
CSvcActionType::CSvcActionType() :
		CSvcConfPackageType("action"), m_ActionType(eActionNoChanged)
{
	m_state = eNoChange;
}

///////////////////////////////////////////////////////////////////
CSvcActionType::CSvcActionType(eActionType actionType) :
		CSvcConfPackageType("action"), m_ActionType(actionType)
{
	m_state = eNoChange;
}

///////////////////////////////////////////////////////////////////
CSvcActionType::~CSvcActionType()
{

}

///////////////////////////////////////////////////////////////////
void CSvcActionType::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull)
{
	CXMLDOMElement* pFeatureNode = NULL;
	if (eNoChange != m_state)
	{
		pFeatureNode = pFatherNode->AddChildNode(GetTagWithNsPrefix(m_elementName).c_str(), GetActionTypeAsString());
	}
	if (!bFull)
		m_state = eNoChange;
}

///////////////////////////////////////////////////////////////////
eActionType CSvcActionType::GetActionType()
{
	return m_ActionType;
}

///////////////////////////////////////////////////////////////////
const char* CSvcActionType::GetActionTypeAsString()
{
	switch (m_ActionType)
	{
		case (eConnectionChanged):
			return "connection-changed";

		case (eMuteChanged):
			return "mute-changed";

		case (eSpeakerChanged):
			return "speaker-changed";

		case (eActionNoChanged):
			return "action-no-changed";

		case (eAdded):
			return "added";

		case (eDeleted):
			return "deleted";

		default:
			return "action-no-changed";
	}
}

///////////////////////////////////////////////////////////////////
void CSvcActionType::SetActionType(eActionType actionType)
{
	m_ActionType = actionType;
	UPDATE_STATE;
}

///////////////////////////////////////////////////////////////////
void CSvcActionType::SetActionTypeStatus(eActionType actionType)
{
	if (actionType == m_ActionType)
	{
		UPDATE_STATE;
	}
}

// ************************************************************************************
//
//	CSvcEndPointType
//
// ************************************************************************************

///////////////////////////////////////////////////////////////////
CSvcEndPointType::CSvcEndPointType() :
		CSvcConfPackageType("endpoint"), m_endpointEntity(""), m_EndPointStatus(e_Pending), m_audioMutedViaFocus(false), m_videoMutedViaFocus(false)
{
	m_bActiveSpeaker = false;
	m_pConfParty = NULL;
}

///////////////////////////////////////////////////////////////////
CSvcEndPointType::CSvcEndPointType(const char* endpointUri, DWORD dwPartyID) :
		CSvcConfPackageType("endpoint"), m_endpointEntity(endpointUri), m_EndPointStatus(e_Pending), m_audioMutedViaFocus(false), m_videoMutedViaFocus(false)
{
	m_bActiveSpeaker = false;
	m_pConfParty = NULL;
}

///////////////////////////////////////////////////////////////////
CSvcEndPointType::CSvcEndPointType(const char* endpointUri, std::string sFrom, std::string sTo, const CConfParty* pConfParty) :
		CSvcConfPackageType("endpoint"), m_endpointEntity(endpointUri), m_EndPointStatus(e_Pending), m_audioMutedViaFocus(false), m_videoMutedViaFocus(false)
{
	m_bActiveSpeaker = false;

	m_sFrom = sFrom;
	m_sTo = sTo;
	int i = sFrom.find('@');
	std::string sName;
	if (i > 0)
		sName = sFrom.substr(0, i + 1);
	m_sEntity = "sip:";
	m_sEntity = m_sEntity + m_sFrom;

	m_pConfParty = pConfParty;
	char buff[120] = { 0 };
	CStructTm dtCurrent;
	SystemGetTime(dtCurrent);
	snprintf(buff, sizeof(buff), "%04u-%02u-%02uT%02u:%02u:%02u", dtCurrent.m_year, dtCurrent.m_mon, dtCurrent.m_day, dtCurrent.m_hour, dtCurrent.m_min, dtCurrent.m_sec);
	m_sTime = buff;

	const CMediaList* pMediaList = pConfParty->GetMediaList();
	if (IsValidPObjectPtr(pMediaList))
	{
		m_mediaList.SetMedia(pMediaList);
		TRACEINTO << m_mediaList.ToString().c_str();
	}
	m_pConfParty = pConfParty;
}

///////////////////////////////////////////////////////////////////
CSvcEndPointType::~CSvcEndPointType()
{
}

///////////////////////////////////////////////////////////////////
const char* CSvcEndPointType::GetEndpointUri()
{
	return m_endpointEntity.GetString();
}

///////////////////////////////////////////////////////////////////
void CSvcEndPointType::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull)
{
}

///////////////////////////////////////////////////////////////////
void CSvcEndPointType::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull, bool bIsNewSubscriber, WORD bIsAudioOnly, bool bUrgent)
{
	CXMLDOMElement* pFeatureNode = NULL;
	//Full data
	if (bFull || bIsNewSubscriber || IsNew())
	{
		TRACEINTO << "CSvcEndPointType::SerializeXml, Full endpoint data:\n\tuser = " << m_endpointEntity.GetString() << "\n\tbFull				= " << (bFull ? "true" : "false") << "\n\tbIsNewSubscriber	= " << (bFull ? "true" : "false") << "\n\tm_state				= " << m_state << "\n\tbIsNew				= " << (IsNew() ? "true" : "false");
		pFeatureNode = pFatherNode->AddChildNode(m_elementName);

		CXMLDOMAttribute* pAttribute = new CXMLDOMAttribute();
		pAttribute->set_nodeName("entity");
		pAttribute->SetValueForElement(m_sEntity.c_str());
		pFeatureNode->AddAttribute(pAttribute);

		pAttribute = new CXMLDOMAttribute();
		pAttribute->set_nodeName("state");
		pAttribute->SetValueForElement("full");
		pFeatureNode->AddAttribute(pAttribute);

		std::string sName = (const_cast<CConfParty*>(m_pConfParty))->GetVisualPartyName();
		if (sName.empty())
			sName = (const_cast<CConfParty*>(m_pConfParty))->GetName();
		pFeatureNode->AddChildNode("display-text", sName.c_str());
		pFeatureNode->AddChildNode("status", GetEndPointStatusByString());

		CXMLDOMElement* pJoinInfo = pFeatureNode->AddChildNode("joining-info");
		pJoinInfo->AddChildNode("when", m_sTime.c_str());
		pJoinInfo->AddChildNode("reason", "&lt;reason&gt;Reason: SIP;text=&quot;Ad-hoc Invitation&quot;&lt;/reason&gt;");
		pJoinInfo->AddChildNode("by", m_sEntity.c_str());

		pFeatureNode->AddChildNode(GetTagWithNsPrefix("mrc", "active-speaker").c_str(), m_bActiveSpeaker ? "true" : "false");

		m_mediaList.SerializeXml(pFeatureNode, m_sNameSpacePrefix.c_str(), true, bIsNewSubscriber);

		CXMLDOMElement* pCallInfo = pFeatureNode->AddChildNode("call-info");
		CXMLDOMElement* pSip = pCallInfo->AddChildNode("sip");
		pSip->AddChildNode("display-text", "full info");
	}
	else
	{
		//Only delta
		{
			PTRACE2(eLevelInfoNormal, "CSvcEndPointType::SerializeXml, Partial endpoint data, user = ", m_endpointEntity.GetString());
			pFeatureNode = pFatherNode->AddChildNode(m_elementName);

			CXMLDOMAttribute* pAttribute = new CXMLDOMAttribute();
			pAttribute->set_nodeName("entity");
			pAttribute->SetValueForElement(m_sEntity.c_str());
			pFeatureNode->AddAttribute(pAttribute);

			pAttribute = new CXMLDOMAttribute();
			pAttribute->set_nodeName("state");
			pAttribute->SetValueForElement("partial");
			pFeatureNode->AddAttribute(pAttribute);

			if (!bUrgent)
			{
				pFeatureNode->AddChildNode("status", GetEndPointStatusByString());
			}
			m_mediaList.SerializeXml(pFeatureNode, m_sNameSpacePrefix.c_str(), false, bIsNewSubscriber);
		}
	}
	SetOld();
	if (!bIsNewSubscriber)
	{
		m_state = eNoChange;
		m_mediaList.SetMediaUpdated();
	}
}

///////////////////////////////////////////////////////////////////
void CSvcEndPointType::SerializeXmlFull(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pFeatureNode = NULL;

	TRACEINTO << "CSvcEndPointType::SerializeXmlFull";
	pFeatureNode = pFatherNode->AddChildNode(m_elementName);
	pFeatureNode->AddAttribute("entity", m_sEntity.c_str());
	pFeatureNode->AddAttribute("state", "full");

	std::string sName = (const_cast<CConfParty*>(m_pConfParty))->GetVisualPartyName();
	if (sName.empty())
		sName = (const_cast<CConfParty*>(m_pConfParty))->GetName();
	pFeatureNode->AddChildNode("display-text", sName.c_str());
	pFeatureNode->AddChildNode("status", GetEndPointStatusByString());

	CXMLDOMElement* pJoinInfo = pFeatureNode->AddChildNode("joining-info");
	pJoinInfo->AddChildNode("when", m_sTime.c_str());
	pJoinInfo->AddChildNode("reason", "&lt;reason&gt;Reason: SIP;text=&quot;Ad-hoc Invitation&quot;&lt;/reason&gt;");
	pJoinInfo->AddChildNode("by", m_sEntity.c_str());

	pFeatureNode->AddChildNode(GetTagWithNsPrefix("mrc", "active-speaker").c_str(), m_bActiveSpeaker ? "true" : "false");
	m_mediaList.SerializeXml(pFeatureNode, m_sNameSpacePrefix.c_str(), true, true);

	CXMLDOMElement* pCallInfo = pFeatureNode->AddChildNode("call-info");
	CXMLDOMElement* pSip = pCallInfo->AddChildNode("sip");
	pSip->AddChildNode("display-text", "full info");
}

///////////////////////////////////////////////////////////////////
eEndPointStatusType CSvcEndPointType::GetEndPointStatus()
{
	return m_EndPointStatus;
}

///////////////////////////////////////////////////////////////////
BYTE CSvcEndPointType::SetEndPointStatus(DWORD status)
{
	eEndPointStatusType previousStatus = m_EndPointStatus;
	switch (status)
	{
		case (PARTY_IDLE):
		case (PARTY_STAND_BY):
		case (PARTY_WAITING_FOR_DIAL_IN):
			m_EndPointStatus = e_Pending;
			break;
		case (PARTY_CONNECTED):
		case (PARTY_CONNECTED_PARTIALY):
		case (PARTY_SECONDARY):
		case (PARTY_CONNECTED_WITH_PROBLEM):
			m_EndPointStatus = e_Connected;
			break;
		case (PARTY_DISCONNECTING):
			m_EndPointStatus = e_Disconnecting;
			break;
		case (PARTY_DISCONNECTED):
			m_EndPointStatus = e_Disconnected;
			break;
	}

	if (previousStatus != m_EndPointStatus)
	{
		UPDATE_STATE;
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////
void CSvcEndPointType::MuteViaFocus(BYTE mutedViaFocus, eMediaContentType mediaType)
{
	if (true == mutedViaFocus)
	{
		m_EndPointStatus = e_MutedViaFocus;
		if (eAudio == mediaType)
			m_audioMutedViaFocus = true;
		else
			m_videoMutedViaFocus = true;
	}
	else
	{
		m_EndPointStatus = e_Connected;
		if (eAudio == mediaType)
			m_audioMutedViaFocus = false;
		else
			m_videoMutedViaFocus = false;

	}
	UPDATE_STATE;
}

///////////////////////////////////////////////////////////////////
BYTE CSvcEndPointType::IsMutedViaFocus(eMediaContentType mediaType)
{
	if (e_MutedViaFocus == m_EndPointStatus)
		if ((eAudio == mediaType && true == m_audioMutedViaFocus) || (eVideoT == mediaType && true == m_videoMutedViaFocus))
			return true;

	return false;
}

///////////////////////////////////////////////////////////////////
char* CSvcEndPointType::GetEndPointStatusByString() const
{
	switch (m_EndPointStatus)
	{
		case (e_Pending):
			return "pending";

		case (e_DialingOut):
			return "dialing-out";

		case (e_DialingIn):
			return "dialing-in";

		case (e_OnHold):
			return "on-hold";

		case (e_Connected):
			return "connected";

		case (e_Disconnecting):
			return "disconnecting";

		case (e_Disconnected):
			return "disconnected";

		case (e_Alerting):
			return "alerting";

		case (e_MutedViaFocus):
			return "muted-via-focus";

		default:
			return "unknown";
	}
}

///////////////////////////////////////////////////////////////////
void CSvcEndPointType::SetMediaStatus(eMediaStatusType mediaStatus)
{
}

///////////////////////////////////////////////////////////////////
void CSvcEndPointType::MuteMedia(eMediaContentType mediaType, BYTE onOff)
{
}

///////////////////////////////////////////////////////////////////
BYTE CSvcEndPointType::IsMuted(eMediaContentType mediaType)
{
	BYTE result = FALSE;
	return result;
}

///////////////////////////////////////////////////////////////////
void CSvcEndPointType::SetActiveSpeaker(bool bSpeaker)
{
	if (m_bActiveSpeaker != bSpeaker)
	{
		m_bActiveSpeaker = bSpeaker;
		UPDATE_STATE;
	}
}

///////////////////////////////////////////////////////////////////
bool CSvcEndPointType::UpdateMedia(CConfParty* pConfParty, bool& bUrgent)
{
	bool bChanged = false;
	const CMediaList* pMediaList = pConfParty->GetMediaList();

	if (IsValidPObjectPtr(pMediaList))
	{
		PTRACE2(eLevelDebug, "CSvcEndPointType::UpdateMedia merge from - ", pMediaList->ToString().c_str());
		PTRACE2(eLevelDebug, "CSvcEndPointType::UpdateMedia merge to - ", m_mediaList.ToString().c_str());

		bUrgent = pMediaList->IsUrgent();
		bChanged = m_mediaList.Merge(pMediaList);

		if (bChanged && !bUrgent)
			UPDATE_STATE;

		PTRACE2(eLevelDebug, "CSvcEndPointType::UpdateMedia after merge - ", m_mediaList.ToString().c_str());
	}

	return bChanged;
}

// ************************************************************************************
//
//	CSvcConfDescriptionType
//
// ************************************************************************************
///////////////////////////////////////////////////////////////////
CSvcConfDescriptionType::CSvcConfDescriptionType() :
		CSvcConfPackageType("conference-description")
{
	m_sConfStartDateTime = "DTSTART:20120308T070318";
	m_sUri = "http://www.polycom.co.il";
}

///////////////////////////////////////////////////////////////////
CSvcConfDescriptionType::CSvcConfDescriptionType(const char* pszUri, CStructTm& stConfStartDateTime) :
		CSvcConfPackageType("conference-description")
{
	m_sUri = std::string(pszUri);
	char buff[100] = "";
	sprintf(buff, "DTSTART:%04u%02u%02uT%02u%02u%02u", stConfStartDateTime.m_year, stConfStartDateTime.m_mon, stConfStartDateTime.m_day, stConfStartDateTime.m_hour, stConfStartDateTime.m_min, stConfStartDateTime.m_sec);
	m_sConfStartDateTime = buff;
}

///////////////////////////////////////////////////////////////////
CSvcConfDescriptionType::~CSvcConfDescriptionType()
{
}

///////////////////////////////////////////////////////////////////
void CSvcConfDescriptionType::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull)
{
	CXMLDOMElement* pConfDesc = NULL;

	if (bFull)
	{
		pConfDesc = pFatherNode->AddChildNode(m_elementName);
		pConfDesc->AddChildNode("keywords");

		CXMLDOMElement* pConfTime = pConfDesc->AddChildNode(GetTagWithNsPrefix("xcon", "conference-time").c_str());
		CXMLDOMElement* pEntry = pConfTime->AddChildNode(GetTagWithNsPrefix("xcon", "entry").c_str());
		pEntry->AddChildNode(GetTagWithNsPrefix("xcon", "base").c_str(), m_sConfStartDateTime.c_str());

		CXMLDOMElement* pConfUris = pConfDesc->AddChildNode("conf-uris");
		CXMLDOMAttribute* pAttribute = new CXMLDOMAttribute();
		pAttribute->set_nodeName("state");
		pAttribute->SetValueForElement(bFull ? "full" : "partial");
		pConfUris->AddAttribute(pAttribute);
		pEntry = pConfUris->AddChildNode("entry");
		pEntry->AddChildNode("uri", m_sUri.c_str());
		pEntry->AddChildNode("purpose", "participation");

		CXMLDOMElement* pServiceUris = pConfDesc->AddChildNode("service-uris");
		pAttribute = new CXMLDOMAttribute();
		pAttribute->set_nodeName("state");
		pAttribute->SetValueForElement(bFull ? "full" : "partial");
		pServiceUris->AddAttribute(pAttribute);
		pEntry = pServiceUris->AddChildNode("entry");
		pEntry->AddChildNode("uri", m_sUri.c_str());
		pEntry->AddChildNode("purpose", "event");

		pConfDesc->AddChildNode("available-media");
	}
}

// ************************************************************************************
//
//	CSvcHostInfoType
//
// ************************************************************************************
CSvcHostInfoType::CSvcHostInfoType() :
		CSvcConfPackageType("host-info")
{
	m_sDisplayText = "Polycom Hag Purim Sameah";
	m_sWebPage = "http://www.polycom.co.il";
}

///////////////////////////////////////////////////////////////////
CSvcHostInfoType::CSvcHostInfoType(const char* pszDisplayText, const char* pszWebPage) :
		CSvcConfPackageType("host-info")
{
	m_sDisplayText = std::string(pszDisplayText);
	m_sWebPage = std::string(pszWebPage);
}

///////////////////////////////////////////////////////////////////
CSvcHostInfoType::~CSvcHostInfoType()
{
}

///////////////////////////////////////////////////////////////////
void CSvcHostInfoType::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull)
{
	CXMLDOMElement* pHostInfo = NULL;

	if (bFull)
	{
		pHostInfo = pFatherNode->AddChildNode(m_elementName);
		pHostInfo->AddChildNode("display-text", m_sDisplayText.c_str());
		pHostInfo->AddChildNode("web-page", m_sWebPage.c_str());
	}
}

// ************************************************************************************
//
//	CSvcConfStateType
//
// ************************************************************************************
CSvcConfStateType::CSvcConfStateType() :
		CSvcConfPackageType("conference-state")
{
}

///////////////////////////////////////////////////////////////////
CSvcConfStateType::~CSvcConfStateType()
{
}

///////////////////////////////////////////////////////////////////
void CSvcConfStateType::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull)
{
}

///////////////////////////////////////////////////////////////////
void CSvcConfStateType::SerializeXml(CXMLDOMElement* pFatherNode, SvcUsersMap & mapUsers, BYTE bFull)
{
	CXMLDOMElement* pConfState = NULL;

	int nUsersCount = 0;
	SvcUsersMap::iterator it = mapUsers.begin();
	for (; it != mapUsers.end(); ++it)
	{
		CSvcUserType* pUser = it->second;
		if (pUser->GetState() != eDelletedData)
			++nUsersCount;
	}

//	if(bFull /*|| eNoChange != m_state*/)
	{
		pConfState = pFatherNode->AddChildNode(m_elementName);
		pConfState->AddChildNode(GetTagWithNsPrefix("xcon", "allow-conference-event-subscription").c_str(), "true");

		char buff[20];
		snprintf(buff, sizeof(buff), "%d", nUsersCount);
		pConfState->AddChildNode("user-count", buff);

		pConfState->AddChildNode("active", "true");
		pConfState->AddChildNode("locked", "false");
	}
}

///////////////////////////////////////////////////////////////////
void CSvcConfStateType::SerializeXml(CXMLDOMElement* pFatherNode, int nUsersCount, BYTE bFull)
{
	CXMLDOMElement* pConfState = NULL;

//	if(bFull /*|| eNoChange != m_state*/)
	{
		pConfState = pFatherNode->AddChildNode(GetTagWithNsPrefix(m_elementName).c_str());
		pConfState->AddChildNode(GetTagWithNsPrefix("xcon", "allow-conference-event-subscription").c_str(), "true");

		char buff[20];
		snprintf(buff, sizeof(buff), "%d", nUsersCount);
		pConfState->AddChildNode("user-count", buff);

		pConfState->AddChildNode("active", "true");
		pConfState->AddChildNode("locked", "false");
	}
}
// ************************************************************************************
//
//	CSvcFloorInfoType
//
// ************************************************************************************
CSvcFloorInfoType::CSvcFloorInfoType() :
		CSvcConfPackageType("floor-information")
{
	m_dwConfID = 1001;
}

///////////////////////////////////////////////////////////////////
CSvcFloorInfoType::CSvcFloorInfoType(DWORD dwConfID) :
		CSvcConfPackageType("floor-information")
{
	m_dwConfID = dwConfID;
}

///////////////////////////////////////////////////////////////////
CSvcFloorInfoType::~CSvcFloorInfoType()
{

}

///////////////////////////////////////////////////////////////////
void CSvcFloorInfoType::SerializeXml(CXMLDOMElement* pFatherNode, BYTE bFull)
{
	CXMLDOMElement* pFloorNode = pFatherNode->AddChildNode(GetTagWithNsPrefix("xcon", m_elementName).c_str());

	char buff[20];
	snprintf(buff, sizeof(buff), "%d", m_dwConfID);
	pFloorNode->AddChildNode(GetTagWithNsPrefix("xcon", "conference-ID").c_str(), buff);

	pFloorNode->AddChildNode(GetTagWithNsPrefix("xcon", "allow-floor-events").c_str(), "true");
	pFloorNode->AddChildNode(GetTagWithNsPrefix("xcon", "floor-request-handling").c_str(), "confirm");

	CXMLDOMElement* pPolicyNode = pFloorNode->AddChildNode(GetTagWithNsPrefix("xcon", "conference-floor-policy"));
	CXMLDOMElement* pPolicyFloorNode = pPolicyNode->AddChildNode(GetTagWithNsPrefix("xcon", "floor").c_str());
	CXMLDOMAttribute* pAttribute = new CXMLDOMAttribute();
	pAttribute->set_nodeName("id");
	pAttribute->SetValueForElement("0");
	pPolicyFloorNode->AddAttribute(pAttribute);
}
