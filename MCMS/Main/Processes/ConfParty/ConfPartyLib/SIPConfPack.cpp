#ifndef __SIPCONFPACKAGE_CPP__
#define __SIPCONFPACKAGE_CPP__

//===== Include Files =====
#include "SIPConfPack.h"
#include "SystemFunctions.h"
#include "StatusesGeneral.h"
#include "SipRefer.h"
#include "Observer.h"
#include "zlibengn.h"
#include "IpCommonDefinitions.h"
#include "IpCsOpcodes.h"
#include "MplMcmsProtocolTracer.h"
#include "TraceStream.h"

#include <stdlib.h>

#define TIMER_SUBSCRIBE_TOUT	100
#define TIMER_CONNECT_REFER		101
#define TIMER_NOTIFY_TOUT		102
#define TIMER_LOAD_TOUT			103


#define	REFER_NOTIFY_TOUT		1*SECOND
#define SIP_CX_NOTIFY_TOUT		5*SECOND
#define LOAD_TOUT				10*SECOND

#define BUF_SIZE	512
#define SIZE_TO_ZIP	2048


// CSIPEventPackageManager states
const WORD   CONNECT        = 1;
const WORD	 TERMINATION	= 2;

//Board Id states
#define  NOT_USED			0
#define  WAIT_FOR_APPROVAL	1
#define	 NOTIFY_WAS_SENT	2


enum{
	eUnknownEvent = 0,
	eConferenceEvent,
	eReferEvent,
	eSipCx
}typedef eSubscribeEventType;


///////////////////////////////////////////////////////////////////
// CSIPEventPackageDispatcher
///////////////////////////////////////////////////////////////////
CSIPEventPackageDispatcher::CSIPEventPackageDispatcher(COsQueue* pRcvMbx, CMplMcmsProtocol* pProtocol, CConfPartyManagerLocalApi* pConfPartyManagerApi)
			: m_terminating(FALSE), m_pMplMcmsProtocol(pProtocol)
{
	m_pSipConfPackage = new CSIPEventPackageManager(pRcvMbx, pProtocol, pConfPartyManagerApi);
	m_pSipCXPackage = new CSIPCXPackageManager(pRcvMbx, pProtocol, pConfPartyManagerApi);
	m_pSipReferPackage = new CSIPReferEventPackageManager(this, pRcvMbx, pProtocol, pConfPartyManagerApi);
}

///////////////////////////////////////////////////////////////////
CSIPEventPackageDispatcher::~CSIPEventPackageDispatcher()
{
	//SAGI: Can't trace in this stage of the process (d'tor)
	POBJDELETE(m_pSipConfPackage);
	POBJDELETE(m_pSipCXPackage);
	POBJDELETE(m_pSipReferPackage);
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageDispatcher::Subscribe(mcIndSubscribe* pSubscribeInd, DWORD callIndex, WORD srcUnitId, DWORD serviceId)
{
	CSipHeaderList * pTemp = new CSipHeaderList(pSubscribeInd->sipHeaders);
	eSubscribeEventType eventType = eUnknownEvent;

	char *pUserUri = NULL, *pDestUri = NULL, *pToTagStr = NULL;
	char* pEventName = NULL;
	char* pAcceptName = NULL;

	//**** Check event type to handle request according to it ****
	const CSipHeader* pEvent = pTemp->GetNextHeader(kEvent);
	if (pEvent)
	{
		pEventName = (char*)pEvent->GetHeaderStr();
		if (pEventName)
		{
			const char* pToStr = NULL;
			const CSipHeader* pTo = pTemp->GetNextHeader(kTo);
			if (pTo)
			{
				pToStr = pTo->GetHeaderStr();
				if (pToStr)
				{
					if (!strncmp(pEventName, "com.microsoft.sip.cx-conference-info", H243_NAME_LEN_OLD))
					{
						eventType = eSipCx;
						m_pSipCXPackage->SetConfName(pToStr);
					}
					if (!strncmp(pEventName, "conference", H243_NAME_LEN_OLD))
					{
						PTRACE(eLevelInfoNormal, "CSIPEventPackageDispatcher::Subscribe");
						eventType = eConferenceEvent;
						m_pSipConfPackage->SetConfName(pToStr);
					}
					if (!strncmp(pEventName, "refer", H243_NAME_LEN_OLD))
					{
						eventType = eReferEvent;
						//m_pSipReferPackage->SetConfName(pToStr);
					}
				}
			}
		}
	}

	//**** if conference is terminating, reject request ****
	if (m_terminating)
	{
		switch (eventType)
		{
			case (eConferenceEvent):
			case (eSipCx):
				SendSipSubscribeResponse(pSubscribeInd, SipCodesNotAcceptable, callIndex, serviceId);
				break;
			case (eReferEvent):
				//m_pMplMcmsProtocol->SipReferResponse(pSubscribeInd->header.pmHeader, SipCodesNotAcceptable);
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
				m_pSipConfPackage->Subscribe(pSubscribeInd, callIndex, srcUnitId, serviceId);
				break;
			case (eReferEvent):
				//m_pSipReferPackage->Refer(pSubscribeInd, boardId);
				break;
			case (eSipCx):
				m_pSipCXPackage->Subscribe(pSubscribeInd, callIndex, srcUnitId, serviceId);
				break;
			default:
				SendSipSubscribeResponse(pSubscribeInd, SipCodesBadRequest, callIndex, serviceId);

		}
	}

	POBJDELETE(pTemp);
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageDispatcher::SendSipSubscribeResponse(mcIndSubscribe* pSubscribeInd, enSipCodes sipCode, DWORD callIndex, WORD srcUnitId, DWORD serviceId)
{
	mcReqSubscribeResp* pSubResp = new mcReqSubscribeResp;
	pSubResp->pCallObj = pSubscribeInd->pCallObj;
	pSubResp->status = sipCode;
	pSubResp->remoteCseq = pSubscribeInd->remoteCseq;

	if (SipCodesOk == sipCode)
		pSubResp->expires = pSubscribeInd->expires;
	else
		pSubResp->expires = 0;

	CSegment* pSeg = new CSegment;
	DWORD size = sizeof(mcReqSubscribeResp);
	*pSeg << callIndex << srcUnitId;

	*pSeg << size;
//	pSeg->Put((BYTE*)pSubResp, sizeof(mcReqSubscribeResp));
	pSeg->Put((BYTE*)pSubResp, size);

	SendMsgToCS(serviceId, SIP_CS_SIG_SUBSCRIBE_RESP_REQ, pSeg);
	PDELETE(pSubResp);
	POBJDELETE(pSeg);
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageDispatcher::SendSipReferResponse(mcIndRefer* pReferInd, enSipCodes sipCode, DWORD callIndex, WORD srcUnitId, DWORD serviceId)
{
	mcReqReferResp* pReferResp = new mcReqReferResp;
	pReferResp->pCallObj = pReferInd->pCallObj;
	pReferResp->status = sipCode;
	pReferResp->remoteCseq = pReferInd->remoteCseq;

	CSegment* pSeg = new CSegment;
	DWORD size = sizeof(mcReqReferResp);
	*pSeg << callIndex << srcUnitId;
	*pSeg << size;
	pSeg->Put((BYTE*)pReferResp, size);

	SendMsgToCS(serviceId, SIP_CS_SIG_REFER_RESP_REQ, pSeg);
	PDELETE(pReferResp);
	POBJDELETE(pSeg);
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageDispatcher::SendMsgToCS(DWORD cs_Id, OPCODE opcode, CSegment* pseg1)
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
	pMplMcmsProtocol->AddPortDescriptionHeader(0, 0);

	if (pseg1)
	{
		*pseg1 >> callIndex >> srcUnitId;
		DWORD size = 0;
		*pseg1 >> size;
		BYTE* pMessage = new BYTE[size];
		pseg1->Get(pMessage, size);

//		BYTE*  pMessage =  new BYTE[pseg1->GetWrtOffset()];
//		pseg1->Get(pMessage,pseg1->GetWrtOffset());
//		pMplMcmsProtocol->AddData(pseg1->GetWrtOffset(),(const char*)pMessage);
		pMplMcmsProtocol->AddData(size, (const char*)pMessage);

		PDELETEA(pMessage);

	}
	pMplMcmsProtocol->AddCSHeader(cs_Id, 0, srcUnitId, callIndex);
	pMplMcmsProtocol->AddPayload_len(CS_API_TYPE);
	CMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("CSIPEventPackageDispatcher::SendMsgToCS ", CS_API_TYPE);
	pMplMcmsProtocol->SendMsgToCSApiCommandDispatcher();

	if (!m_pMplMcmsProtocol)
		PDELETE(pMplMcmsProtocol);
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageDispatcher::Refer(mcIndRefer* pReferInd, DWORD callIndex, WORD srcUnitId, DWORD serviceId)
{
	BYTE bLooseMcuApi = FALSE;
	eRoleType role = eParticipant;

	CSipHeaderList* pTemp = new CSipHeaderList(pReferInd->sipHeaders);
	AUTO_DELETE(pTemp);

	DWORD ip = pReferInd->transportAddress.transAddr.addr.v4.ip;
	WORD port = pReferInd->transportAddress.transAddr.port;
	WORD transportType = pReferInd->transportAddress.transAddr.transportType;
	WORD expires = 300;
	eSubscribeEventType eventType = eUnknownEvent;

	char* pUserUri = NULL, *pDestUri = NULL;
	char* pEventName = NULL;
	char* pAcceptName = NULL;

	//**** Check event type to handle request according to it ****
	const char* pToStr = NULL;
	//only if name is not initialized, set it
	if (!strcmp(m_pSipConfPackage->GetConfName(), ""))
	{
		const CSipHeader* pTo = pTemp->GetNextHeader(kTo);
		if (pTo)
		{
			pToStr = pTo->GetHeaderStr();
			if (pToStr)
			{
				char *temp = (char*)strstr(pToStr, "@");
				if (temp)
					*temp = '\0';
			}
		}
	}
	if (pToStr && strcmp(pToStr, ""))
		m_pSipReferPackage->SetConfName(pToStr);
	else
	{
		PTRACE(eLevelInfoNormal, "CSIPEventPackageDispatcher::Refer - Bad TO element - reject");
		SendSipReferResponse(pReferInd, SipCodesBadRequest, callIndex, srcUnitId, serviceId);
		return;
	}

	const char* pFromStr = NULL;
	const CSipHeader* pFrom = pTemp->GetNextHeader(kFrom);
	BYTE bFromIsValid = TRUE;
	if (pFrom)
		pFromStr = pFrom->GetHeaderStr();
	else
		bFromIsValid = FALSE;

	if (!strcmp(pToStr, ""))
		bFromIsValid = FALSE;

	if (!bFromIsValid)
	{
		PTRACE(eLevelInfoNormal, "CSIPEventPackageDispatcher::Refer - Bad FROM element- reject");
		SendSipReferResponse(pReferInd, SipCodesBadRequest, callIndex, srcUnitId, serviceId);
		return;
	}

	CSIPSubscriber* pSubscriber = m_pSipCXPackage->FindParty(pFromStr);
	if (pSubscriber)
		role = pSubscriber->GetRole();

	//**** if conference is terminating, reject request ****
	if (m_terminating)
		SendSipReferResponse(pReferInd, SipCodesNotAcceptable, callIndex, srcUnitId, serviceId);

	else
		m_pSipReferPackage->Refer(pReferInd, callIndex, srcUnitId, role, serviceId);

	POBJDELETE(pTemp);
}

///////////////////////////////////////////////////////////////////
CSIPSubscriber* CSIPEventPackageDispatcher::FindParty(char* from, DWORD CSeq, char* callId)
{
	PTRACE2(eLevelInfoNormal, "CSIPEventPackageDispatcher::FindParty named = ", from);

	CSIPSubscriber* pSubscriber = m_pSipConfPackage->FindParty(from);
	if (!IsValidPObjectPtr(pSubscriber))
		pSubscriber = m_pSipReferPackage->FindParty(from, CSeq, callId);
	if (!IsValidPObjectPtr(pSubscriber))
		pSubscriber = m_pSipCXPackage->FindParty(from);

	return pSubscriber;
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageDispatcher::Dump()
{
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageDispatcher::TerminateConf()
{
	m_terminating = TRUE;
	m_pSipConfPackage->TerminateConf();
	m_pSipCXPackage->TerminateConf();
	m_pSipReferPackage->TerminateConf();
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageDispatcher::LoadManagerAccept()
{
	m_pSipCXPackage->OnLoadManagerAccept();
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageDispatcher::HandleObserverUpdate(CSegment *pSeg, WORD type)
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
			m_pSipCXPackage->ObserverUpdate(pSubscriber, event, val);
			break;
		}
		case SIP_EVENT_PACKAGE:
		{
			m_pSipConfPackage->ObserverUpdate(pSubscriber, event, val);
			break;
		}
		case (SIP_REFER):
		{
			if (pSubscriber)
			{
				if (IsValidPObjectPtr((CPObject*)pSubscriber))
				{
					if (m_pSipReferPackage->FindParty(((CSIPSubscriber*)pSubscriber)->GetFrom(), ((CSIPREFERSubscriber*)pSubscriber)->GetCSeq(), ((CSIPREFERSubscriber*)pSubscriber)->GetCallId()))
						m_pSipReferPackage->ObserverUpdate((CSIPSubscriber*)pSubscriber, event, val);
				}
			}
			break;
		}
		default:
		{
			PTRACE(eLevelError, "CSIPEventPackageDispatcher::HandleObserverUpdate, invalid pointer");
			DBGPASSERT(1);
		}
	}
}

///////////////////////////////////////////////////////////////////
CSIPSubscriber* CSIPEventPackageDispatcher::FindPartyInSIPCX(const char* pFromStr)
{
	PTRACE2(eLevelInfoNormal,"CSIPEventPackageDispatcher::FindPartyInSIPCX named = ", pFromStr);
	CSIPSubscriber* pSubscriber = m_pSipCXPackage->FindParty(pFromStr);
	return pSubscriber;
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageDispatcher::ResetNotifyTimer()
{
	m_pSipConfPackage->ResetNotifyTimer();
	m_pSipCXPackage->ResetNotifyTimer();
	m_pSipReferPackage->ResetNotifyTimer();
}

// start message map -------------------------------------------
PBEGIN_MESSAGE_MAP(CSIPEventPackageManager)

	// conf events
	ONEVENT(TIMER_SUBSCRIBE_TOUT	,CONNECT	,CSIPEventPackageManager::OnSubscribeTout )
	ONEVENT(TIMER_NOTIFY_TOUT		,CONNECT	,CSIPEventPackageManager::OnNotifyTimerTout )

PEND_MESSAGE_MAP(CSIPEventPackageManager,CStateMachine);
// end   message map -------------------------------------------

///////////////////////////////////////////////////////////////////
// CSIPEventPackageManager
///////////////////////////////////////////////////////////////////
CSIPEventPackageManager::CSIPEventPackageManager(COsQueue* pRcvMbx, CMplMcmsProtocol* pMplMcmsProtocol, CConfPartyManagerLocalApi* pConfPartyManagerApi, CConfApi* pConfApi)
{
	m_state = CONNECT;
	m_SubTimer = FALSE;
	m_NotifyTimer = FALSE;
	m_LoadTimer = FALSE;
	m_pNotifyMsg = NULL;
	m_pEventConfInfo = NULL;
	m_pXMLStr = NULL;
	m_CSeq = 1;
	m_version = 1;
	*m_pConfName = '\0';
	m_pMplMcmsProtocol = pMplMcmsProtocol;
	m_pConfPartyManagerApi = pConfPartyManagerApi;
	m_pConfApi = pConfApi;
	if (pRcvMbx)
		m_pRcvMbx = pRcvMbx;
	else
		m_pRcvMbx = NULL;
	if (!pMplMcmsProtocol)
		m_runInTdd = FALSE;
	else
		m_runInTdd = TRUE;
}

///////////////////////////////////////////////////////////////////
CSIPEventPackageManager::~CSIPEventPackageManager()
{
	DeleteAllTimers();
	POBJDELETE(m_pXMLStr);
	PDELETEA(m_pNotifyMsg);

	CSIPSubscriber* pSubscriber = NULL;
	//find all subscribers
	std::vector< CSIPSubscriber * >::iterator itr =  m_EventSubscribersList.begin();
	while (itr != m_EventSubscribersList.end())
	{
		pSubscriber = (*itr);
		m_EventSubscribersList.erase(itr);
		POBJDELETE(pSubscriber);
		itr = m_EventSubscribersList.begin();
	}

	POBJDELETE(m_pEventConfInfo);
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageManager::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageManager::Subscribe(mcIndSubscribe* pSubscribeInd, DWORD callIndex, WORD srcUnitId, DWORD serviceId)
{
	WORD boardId = 1;
	enSipCodes SipStatus = SipCodesBadRequest;
	BYTE bValidReq = TRUE, acceptVerified = TRUE, bLooseMcuApi = FALSE, bSendNotify = FALSE, bReferWithBye = FALSE;
	BYTE bNotifyWasSent = FALSE, bIsNewSubscriber = FALSE;
	const char *pFromStr = NULL, *pFromTagStr = NULL, *pToStr = NULL, *pToTagStr = NULL, *pCallIdStr = NULL;
	char *pAcceptName = NULL;
	CSIPSubscriber* pSubscriber = NULL;

	CSipHeaderList *pTemp = new CSipHeaderList(pSubscribeInd->sipHeaders);

	bValidReq = CheckRequestValidity(pSubscribeInd, &SipStatus);

	const CSipHeader* pFrom = pTemp->GetNextHeader(kFrom);
	if (pFrom)
		pFromStr = pFrom->GetHeaderStr();

	const CSipHeader* pFromTag = pTemp->GetNextHeader(kFromTag);
	if (pFromTag)
		pFromTagStr = pFromTag->GetHeaderStr();

	const CSipHeader* pTo = pTemp->GetNextHeader(kTo);
	if (pTo)
		pToStr = pTo->GetHeaderStr();

	const CSipHeader* pToTag = pTemp->GetNextHeader(kToTag);
	if (pToTag)
		pToTagStr = pToTag->GetHeaderStr();

	const CSipHeader* pCallId = pTemp->GetNextHeader(kCallId);
	if (pCallId)
		pCallIdStr = pCallId->GetHeaderStr();

	if (pSubscribeInd->expires > 3600)
		pSubscribeInd->expires = 3600;

	if (bValidReq)
	{
		//if party exists
		pSubscriber = FindParty(pFromStr);
		if (IsValidPObjectPtr(pSubscriber))
		{
			//unsubscribe
			if (0 == pSubscribeInd->expires)
			{
				PTRACE2(eLevelInfoNormal, "CSIPEventPackageManager::Subscribe, unsubscribing party = ", pFromStr);
				RemoveFromVector(pSubscriber);
				SipStatus = SipCodesOk;
				POBJDELETE(pSubscriber);
				TRACESTR (eLevelInfoNormal) << "CSIPEventPackageManager::Subscribe, num of subscribers =  " << m_EventSubscribersList.size();

				if (0 == m_EventSubscribersList.size())
				{
					PTRACE(eLevelInfoNormal, "CSIPEventPackageManager::Subscribe, unsubscribing last party");
					CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConfByNameOrByNumericId(m_pConfName);
					if (IsValidPObjectPtr(pCommConf))
						DetachEvents(pCommConf);
					POBJDELETE(m_pEventConfInfo);
					*m_pConfName = '\0';
					if (m_NotifyTimer)
					{
						DeleteTimer(TIMER_NOTIFY_TOUT);
						m_NotifyTimer = FALSE;
					}
				}

			}
			//refresh expires Tout.
			else
			{
				char s[20];
				memset(s, 0, 20);
				sprintf(s, "%ld", pSubscribeInd->expires);
				PTRACE2(eLevelInfoNormal, "CSIPEventPackageManager::Subscribe, refreshing subscription, expires = ", s);
				pSubscriber->Refresh(pSubscribeInd->transportAddress.transAddr.addr.v4.ip, pSubscribeInd->transportAddress.transAddr.port, pSubscribeInd->transportAddress.transAddr.transportType, pCallIdStr, pSubscribeInd->expires);
				SipStatus = SipCodesOk;
			}
		}
		//new subscriber
		else
		{
			//unsubscribe is ok even if party is not found.
			if (0 == pSubscribeInd->expires)
			{
				PTRACE(eLevelInfoNormal, "CSIPEventPackageManager::Subscribe, unsubscribing party is not found.");
				SipStatus = SipCodesOk;
				//m_pMplMcmsProtocol->SendSipSubscribeResponse(pmHeader, SipCodesOk, 0);
			}
			else
			{
				CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConfByNameOrByNumericId(m_pConfName);
				if (IsValidPObjectPtr(pCommConf))
				{

					PTRACE(eLevelInfoNormal, "CSIPEventPackageManager::Subscribe, new subscriber party is added.");
					if (pToTagStr && pCallIdStr && pToStr && pFromTagStr && pFromStr)
					{
						pSubscriber = new CSIPSubscriber(boardId, pFromStr, pFromTagStr, pToStr, pToTagStr, pCallIdStr, pSubscribeInd->transportAddress.transAddr.addr.v4.ip, pSubscribeInd->transportAddress.transAddr.port, pSubscribeInd->transportAddress.transAddr.transportType, pSubscribeInd->expires, srcUnitId, callIndex, eParticipant, serviceId);
						m_EventSubscribersList.push_back(pSubscriber);
					}
					else
						PASSERT(1);

					AttachEvents(pCommConf);
					SipStatus = SipCodesOk;
					bSendNotify = TRUE;
					bIsNewSubscriber = TRUE;
				}
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
	else if (pFromStr)
		PTRACE(eLevelInfoNormal, "CSIPEventPackageManager::Subscribe, Invalid request is rejected.");

	if (SipCodesOk != SipStatus)
		pSubscribeInd->expires = 0;

	SendSipSubscribeResponse(pSubscribeInd, SipStatus, callIndex, srcUnitId, serviceId);

	if (bSendNotify && SipCodesOk == SipStatus)
	{
		//this notify is sent only to one subscriber
		CObjString* pStr = BuildNotifyContent(TRUE, bIsNewSubscriber);
		bNotifyWasSent = Notify(pSubscriber, (char*)pStr->GetString(), "active");
		POBJDELETE(pStr);

		//Send partial notify to all others ------ *********************** ------------
	}
	POBJDELETE(pTemp);
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageManager::SendSipSubscribeResponse(mcIndSubscribe* pSubscribeInd, enSipCodes sipCode, DWORD callIndex, WORD srcUnitId, DWORD cs_Id)
{
	mcReqSubscribeResp* pSubResp = new mcReqSubscribeResp;
	pSubResp->pCallObj = pSubscribeInd->pCallObj;
	pSubResp->expires = pSubscribeInd->expires;
	pSubResp->status = sipCode;
	pSubResp->remoteCseq = pSubscribeInd->remoteCseq;

	CSegment* pSeg = new CSegment;
	*pSeg << callIndex << srcUnitId;
	DWORD size = sizeof(mcReqSubscribeResp);
	*pSeg << size;
	pSeg->Put((BYTE*)pSubResp, size);

	SendMsgToCS(cs_Id, SIP_CS_SIG_SUBSCRIBE_RESP_REQ, pSeg);
	PDELETE(pSubResp);
	POBJDELETE(pSeg);
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageManager::SendSipNotify(mcReqNotify* pNotifyMsg, WORD srcUnitId, DWORD callIndex, DWORD cs_Id)
{
	DWORD size = sizeof(mcReqNotifyBase) + pNotifyMsg->sipContentAndHeaders.lenOfDynamicSection;
	CSegment* pSeg = new CSegment;
	*pSeg << callIndex << srcUnitId;
	*pSeg << size;
	pSeg->Put((BYTE*)pNotifyMsg, size);

	SendMsgToCS(cs_Id, SIP_CS_PROXY_NOTIFY_REQ, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CSIPEventPackageManager::SendMsgToCS(DWORD cs_Id, OPCODE opcode, CSegment* pseg1)
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
	pMplMcmsProtocol->AddPortDescriptionHeader(0, 1);
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
	CMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("CSIPEventPackageDispatcher::SendMsgToCS ", CS_API_TYPE);
	pMplMcmsProtocol->SendMsgToCSApiCommandDispatcher();

	if (!m_pMplMcmsProtocol)
		PDELETE(pMplMcmsProtocol);
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageManager::RemoveFromVector(CSIPSubscriber* pSubscriber)
{
	CSIPSubscriber* pTempSubscriber = NULL;
	std::vector<CSIPSubscriber *>::iterator itr = m_EventSubscribersList.begin();
	while (itr != m_EventSubscribersList.end())
	{
		pTempSubscriber = (*itr);
		if (!strncmp(pSubscriber->GetCallId(), pTempSubscriber->GetCallId(), MaxAddressListSize))
		{
			m_EventSubscribersList.erase(itr);
			break;
		}
		++itr;
	}
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageManager::OnNotifyTimerTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSIPEventPackageManager::OnNotifyTimerTout");
	//if there is a waiting Notify message, send it
	if (m_pNotifyMsg)
	{
		if (pParam && (pParam->GetWrtOffset() > 0))
		{
			WORD srcUnitId = 0;
			DWORD callIndex = 0;
			DWORD cs_Id = 0;
			*pParam >> srcUnitId >> callIndex >> cs_Id;
			SendSipNotify(m_pNotifyMsg, srcUnitId, callIndex, cs_Id);
		}
		else
			SendSipNotify(m_pNotifyMsg);
		if (!m_runInTdd)
			StartTimer(TIMER_NOTIFY_TOUT, SIP_CX_NOTIFY_TOUT);
		m_NotifyTimer = TRUE;
		PDELETEA(m_pNotifyMsg);
	}
	//else, just mark that a new message may be sent.
	else
	{
		DeleteTimer(TIMER_NOTIFY_TOUT);
		m_NotifyTimer = FALSE;

		if (m_pEventConfInfo->WasUpdated())
		{
			m_pXMLStr = BuildNotifyContent(FALSE);

			if (m_pXMLStr)
			{
				CSIPSubscriber* pSubscriber = NULL;
				std::vector<CSIPSubscriber *>::iterator itr = m_EventSubscribersList.begin();
				while (itr != m_EventSubscribersList.end())
				{
					pSubscriber = (*itr);
					if (IsValidPObjectPtr(pSubscriber))
					{
						Notify(pSubscriber, (char*)m_pXMLStr->GetString(), "active", true);
					}
					++itr;
				}
			}
			POBJDELETE(m_pXMLStr);
		}
	}
}

///////////////////////////////////////////////////////////////////
const char* CSIPEventPackageManager::EventToString(const WORD event)
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
		default:
			return "UNKNOWN_EVENT";
	}
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageManager::AttachEvents(CCommConf *pCommConf)
{
	PTRACE(eLevelInfoNormal, "CSIPEventPackageManager::AttachEvents");
	DWORD partyId = 0;
	CConfParty* pConfParty = pCommConf->GetFirstParty();
	while (IsValidPObjectPtr(pConfParty))
	{
		partyId = pConfParty->GetPartyId();
		//Party events
		pConfParty->AttachObserver(m_pRcvMbx, PARTYSTATE, SIP_EVENT_PACKAGE, partyId);
		pConfParty->AttachObserver(m_pRcvMbx, MUTE_STATE, SIP_EVENT_PACKAGE, partyId);

		pConfParty->AttachObserver(m_pRcvMbx, CONTACT_INFO, SIP_EVENT_PACKAGE, partyId);

		pConfParty = pCommConf->GetNextParty();
	}
	//Conf events
	pCommConf->AttachObserver(m_pRcvMbx, CONF_ACTIVE, SIP_EVENT_PACKAGE);
	pCommConf->AttachObserver(m_pRcvMbx, AUDIOSRC, SIP_EVENT_PACKAGE);
	pCommConf->AttachObserver(m_pRcvMbx, PARTY_ADDED, SIP_EVENT_PACKAGE);
	pCommConf->AttachObserver(m_pRcvMbx, PARTY_DELETED, SIP_EVENT_PACKAGE);
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageManager::DetachEvents(CCommConf *pCommConf)
{
	PTRACE(eLevelInfoNormal, "CSIPEventPackageManager::DetachEvents");
	DWORD partyId = 0;
	CConfParty* pConfParty = pCommConf->GetFirstParty();
	while (IsValidPObjectPtr(pConfParty))
	{
		PTRACE2(eLevelInfoNormal, "CSIPEventPackageManager::DetachEvents, partyName = ", pConfParty->GetName());
		partyId = pConfParty->GetPartyId();
		//Party events
		pConfParty->DetachObserver(m_pRcvMbx);

		pConfParty = pCommConf->GetNextParty();
	}
	//Conf events
	pCommConf->DetachObserver(m_pRcvMbx);
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageManager::ObserverUpdate(void* pSubscriber, WORD event, DWORD val)
{
	DWORD partyId = (DWORD)pSubscriber;

	ALLOCBUFFER(s, 100);
	sprintf(s, "PartyId=%i event=%s, val=%i", partyId, EventToString(event), val);
	PTRACE2(eLevelInfoNormal, "CSIPEventPackageManager::ObserverUpdate, ", s);
	DEALLOCBUFFER(s);

	if (!m_pEventConfInfo)
		return;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConfByNameOrByNumericId(m_pConfName);
	CConfParty *pConfParty = NULL;

	switch (event)
	{
		case (PARTYSTATE):
		{
			if (IsValidPObjectPtr(pCommConf))
				pConfParty = pCommConf->GetCurrentParty(partyId);
			else
				PTRACE(eLevelWarn, "CSIPEventPackageManager::ObserverUpdate, conf not found in DB.");

			if (IsValidPObjectPtr(pConfParty))
			{
				m_pEventConfInfo->SetEndPointStatus(partyId, pConfParty->GetPartyState());
				if (PARTY_DISCONNECTED == pConfParty->GetPartyState() && !strcmp(m_pEventConfInfo->GetActiveSpeaker(), pConfParty->GetName()))
				{
					m_pEventConfInfo->SetActiveSpeaker("", partyId);
				}
			}
			else
				PTRACE(eLevelError, "CSIPEventPackageManager::ObserverUpdate, party not found in DB.");

			break;
		}

		case (CONTACT_INFO):
		{
			if (IsValidPObjectPtr(pCommConf))
				pConfParty = pCommConf->GetCurrentParty(partyId);
			else
				PTRACE(eLevelError, "CSIPEventPackageManager::ObserverUpdate, conf not found in DB.");

			if (IsValidPObjectPtr(pConfParty))
			{
				PTRACE(eLevelInfoNormal, "CSIPEventPackageManager::ObserverUpdate, CONTACT_INFO");
				if (0 == val)
				{
					m_pEventConfInfo->SetUserContactInfo(pConfParty->GetUserDefinedInfo(0), partyId);
				}
			}
			else
				PTRACE(eLevelError, "CSIPEventPackageManager::ObserverUpdate, party not found in DB.");
			break;
		}

		case (MUTE_STATE):
		{
			if (IsValidPObjectPtr(pCommConf))
				pConfParty = pCommConf->GetCurrentParty(partyId);
			if (IsValidPObjectPtr(pConfParty))
			{
				PTRACE(eLevelInfoNormal, "CSIPEventPackageManager::ObserverUpdate, MUTE_STATE");
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
						m_pEventConfInfo->SetActiveSpeaker("", partyId);
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
				PTRACE2(eLevelInfoNormal, "CSIPEventPackageManager::ObserverUpdate, SetActiveSpeaker = ", pConfParty->GetName());
				m_pEventConfInfo->SetActiveSpeaker(pConfParty->GetName(), pConfParty->GetPartyId());
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

				m_pEventConfInfo->AddParty(pConfParty, bIsWebDialInParty);

				partyId = pConfParty->GetPartyId();
				//Party events
				pConfParty->AttachObserver(m_pRcvMbx, PARTYSTATE, SIP_EVENT_PACKAGE, partyId);
				pConfParty->AttachObserver(m_pRcvMbx, MUTE_STATE, SIP_EVENT_PACKAGE, partyId);
				pConfParty->AttachObserver(m_pRcvMbx, CONTACT_INFO, SIP_EVENT_PACKAGE, partyId);
			}
			else
				PTRACE(eLevelError, "CSIPEventPackageManager::ObserverUpdate, party not found.");
			break;
		}

		case (PARTY_DELETED):
		{

			m_pEventConfInfo->DelParty(val, m_EventSubscribersList.size());
			break;
		}

		default:
			PTRACE(eLevelError, "CSIPEventPackageManager::ObserverUpdate - Invalid event received.");
			PASSERT_AND_RETURN(event);
	}

	//if we can send notify right now
	if (!m_NotifyTimer)
	{
		CSIPSubscriber* pSubscriber = (CSIPSubscriber*)m_EventSubscribersList[0];
		if (IsValidPObjectPtr(pSubscriber))
		{

			BOOL bWebDialInEmptyInfo = FALSE;
			if (IsValidPObjectPtr(pConfParty))
			{
				// 1. Connection type should be DIAL_IN
				BYTE connectionType = pConfParty->GetConnectionType();

				// 2. Conference was created by PCAS by a non-web (Same Time) flow - name format: RAS200I_xxx
				BOOL bPcasWebFlow = FALSE;
				if (0 == strncmp(m_pConfName, "RAS200I_web_", 12))
					bPcasWebFlow = TRUE;

				// 3. UserContactInfo was not recieved yet from the External DB
				BOOL bEmptyContactInfo = FALSE;
				if (0 == strcmp(pConfParty->GetUserDefinedInfo(0), ""))
					bEmptyContactInfo = TRUE;

				if (!(bPcasWebFlow && DIAL_IN == connectionType && bEmptyContactInfo))
					bWebDialInEmptyInfo = TRUE;
			}

			//in order to wait until the externalDB is update the UserContactInfo
			if (m_pEventConfInfo->WasUpdated() && bWebDialInEmptyInfo/*!(bPcasWebFlow && DIAL_IN == connectionType && bEmptyContactInfo)*/)
			{
				m_pXMLStr = BuildNotifyContent(FALSE);
				if (m_pXMLStr)
				{
					Notify(pSubscriber, (char*)m_pXMLStr->GetString(), "active");
				}
				POBJDELETE(m_pXMLStr);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////
CObjString* CSIPEventPackageManager::BuildNotifyContent(BYTE bFull, BYTE bIsNewSubscriber)
{
	CObjString *pContentStr = NULL;
	char *XML = NULL, *pValue = NULL;
	DWORD length = 0;
	CXMLDOMElement *pElement = new CXMLDOMElement;

	m_pEventConfInfo->SerializeXml(pElement, bFull, bIsNewSubscriber);

	//if there was no change in data -> no xml will be created!!!
	pElement->get_tagName(&pValue);
	if (NULL != pValue)
	{
		CObjString *pStr = NULL;

		pElement->DumpDataAsStringWithAttribute(&XML, &length);

		pStr = new CObjString(XML, length + 20);

		PDELETE(pElement);
		PDELETEA(XML);

		if (bFull)
			pContentStr = UpdateVersionInContent(pStr, m_version);
		else
			pContentStr = UpdateVersionInContent(pStr, ++m_version);

		POBJDELETE(pStr);
	}
	else
		pContentStr = new CSmallString("");

	return pContentStr;
}

///////////////////////////////////////////////////////////////////
CObjString* CSIPEventPackageManager::UpdateVersionInContent(CObjString *pStr, int version)
{
	CObjString* pResultStr = NULL;
	if (1 != version)
	{
		char* pToken = (char*)strstr(pStr->GetString(), "version");
		TRACEINTO << pToken;
		if (pToken)
		{
			char Ver[20];
			snprintf(Ver, ARRAYSIZE(Ver), "%d", version);
			std::string str = pStr->GetString();
			str.insert(pToken - pStr->GetString() + 9, Ver);

			pResultStr = new CObjString(str.c_str(), str.length());
		}
	}
	else
		pResultStr = new CObjString(pStr->GetString());
	return pResultStr;
}

///////////////////////////////////////////////////////////////////
BYTE CSIPEventPackageManager::CheckRequestValidity(mcIndSubscribe* pSubscribeInd, enSipCodes *SipStatus)
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
		if (pAcceptName)
		{
			//verify allow is either empty or holds "application/conference-info+xml"
			if (strncmp(pAcceptName, "application/conference-info+xml", H243_NAME_LEN))
			{
				//or if it holds Microsoft's allow header
				if (strncmp(pAcceptName, "application/com.microsoft.sip.cx-conference-info+xml", H243_NAME_LEN))
				{
					if (strcmp(pAcceptName, ""))
						acceptVerified = FALSE;
				}
				else
					acceptVerified = TRUE;
			}
			else
				acceptVerified = TRUE;
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
	if (eTransportTypeUdp != pSubscribeInd->transportAddress.transAddr.transportType && eTransportTypeTcp != pSubscribeInd->transportAddress.transAddr.transportType)
		bValidReq = FALSE;

	POBJDELETE(pTemp);

	return bValidReq;
}

///////////////////////////////////////////////////////////////////
//Just do Notify
BYTE CSIPEventPackageManager::Notify(CSIPSubscriber* pSubscriber, char* content, char* state, BYTE isDistribute)
{
	BYTE result = FALSE;

	TRACEINTO << "To:" << pSubscriber->GetTo() << ", From:" << pSubscriber->GetFrom() << "Content:" << content;

	CSipNotifyStruct SipNotify;
	SipNotify.SetHeaderField(kFrom, pSubscriber->GetTo());
	SipNotify.SetHeaderField(kFromTag, pSubscriber->GetToTag());
	SipNotify.SetHeaderField(kTo, pSubscriber->GetFrom());
	SipNotify.SetHeaderField(kToTag, pSubscriber->GetFromTag());
	SipNotify.SetHeaderField(kCallId, pSubscriber->GetCallId());
	SipNotify.SetHeaderField(kContentType, "application/conference-info+xml");
	SipNotify.SetContent(content);
	SipNotify.SetHeaderField(kEvent, "conference");

	char SubState[H243_NAME_LEN] = { 0 };
	strcpy_safe(SubState, state);
	if (strncmp(state, "terminated", 10))
	{
		int len = strlen(SubState);
		if (len < H243_NAME_LEN - 16)
		{
			CStructTm curTime;
			STATUS timeStatus = SystemGetTime(curTime);
			char temp[30];
			sprintf(temp, ";expires=%d", pSubscriber->GetExpireTime() - curTime);
			strncat(SubState, temp, 15);
		}
	}

	//if terminating
	else
	{
		//detach from observer pattern
		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConfByNameOrByNumericId(m_pConfName);
		if (IsValidPObjectPtr(pCommConf))
		{
			char subscribeTo[MaxAddressListSize] = { 0 };
			strcpy_safe(subscribeTo, ((CSIPSubscriber*)pSubscriber)->GetTo());
			char* temp = (char*)strstr(subscribeTo, "@");
			if (temp)
				*temp = '\0';
			CConfParty* pConfParty = pCommConf->GetCurrentParty(subscribeTo);
			if (IsValidPObjectPtr(pConfParty))
				pConfParty->DetachObserver(m_pRcvMbx, PARTYSTATE, SIP_EVENT_PACKAGE, (DWORD)pSubscriber);
		}
	}

	SipNotify.SetHeaderField(kSubscrpState, SubState);
	SipNotify.SetIp(pSubscriber->GetIp());
	SipNotify.SetPort(pSubscriber->GetPort());
	SipNotify.SetTransportType(pSubscriber->GetTransport());
	SipNotify.SetCSeq(m_CSeq++);

	m_pNotifyMsg = SipNotify.BuildNotifyReq();
	PASSERT_AND_RETURN_VALUE(NULL == m_pNotifyMsg, result);

	//Notify may be sent only once a second (top frequency)
	if ((!m_NotifyTimer) || isDistribute)
	{
		CSegment *pNotifyParams = new CSegment;
		*pNotifyParams << pSubscriber->GetSrcUnitId() << ((CSIPREFERSubscriber*)pSubscriber)->GetCallIndex() << ((CSIPREFERSubscriber*)pSubscriber)->GetBoardId();
		SendSipNotify(m_pNotifyMsg, pSubscriber->GetSrcUnitId(), 0, pSubscriber->GetBoardId());
		if (!m_runInTdd)
			StartTimer(TIMER_NOTIFY_TOUT, SIP_CX_NOTIFY_TOUT, pNotifyParams);
		m_NotifyTimer = TRUE;
		PDELETEA(m_pNotifyMsg);
		result = TRUE; //Notify was sent ok.
	}
	else
		PTRACE(eLevelInfoNormal, "CSIPEventPackageManager::Notify not sent due to Timer.");

	return result;
}

///////////////////////////////////////////////////////////////////
CSIPSubscriber* CSIPEventPackageManager::FindParty(const char* from)
{
	PTRACE2(eLevelInfoNormal, "CSIPEventPackageManager::FindConfPkgParty named = ", from);
	CSIPSubscriber* pSubscriber = NULL;
	if (from)
	{
		std::vector<CSIPSubscriber *>::iterator itr = m_EventSubscribersList.begin();
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
BYTE CSIPEventPackageManager::FindParty(void* pSubscriber)
{
	return FALSE;
}

///////////////////////////////////////////////////////////////////
//find shortest expires between all subscribers
WORD CSIPEventPackageManager::CalcTimer()
{
	PTRACE(eLevelInfoNormal, "CSIPEventPackageManager::CalcTimer");
	CSIPSubscriber* pSubscriber = NULL;
	CStructTm expiresTime;
	DWORD expires, result = 0xffffffff;

	CStructTm curTime;
	STATUS timeStatus = SystemGetTime(curTime);
	std::vector<CSIPSubscriber *>::iterator itr = m_EventSubscribersList.begin();
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
	char s[20];
	memset(s, 0, 20);
	sprintf(s, "%d", result);
	PTRACE2(eLevelInfoNormal, "CSIPEventPackageManager::CalcTimer = ", s);
	return result;
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageManager::TerminateConf()
{
	CSIPSubscriber* pSubscriber = NULL;

	DeleteTimer(TIMER_SUBSCRIBE_TOUT);
	m_state = TERMINATION;

	//find all subscribers
	std::vector<CSIPSubscriber *>::iterator itr = m_EventSubscribersList.begin();
	while (itr != m_EventSubscribersList.end())
	{
		pSubscriber = (*itr);
		if (IsValidPObjectPtr(pSubscriber))
		{
			BYTE bLooseLoadMngr = FALSE;
			PTRACE2(eLevelInfoNormal, "CSIPEventPackageManager::TerminateConf party = ", pSubscriber->GetFrom());
			Notify(pSubscriber, "", "terminated;reason=noresource");
		}
		m_EventSubscribersList.erase(itr);
		POBJDELETE(pSubscriber);
		itr = m_EventSubscribersList.begin();
	}

	PTRACE(eLevelInfoNormal, "CSIPEventPackageManager::TerminateConf, DetatachEvents");
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConfByNameOrByNumericId(m_pConfName);
	if (IsValidPObjectPtr(pCommConf))
		DetachEvents(pCommConf);
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageManager::Dump(COstrStream& msg) const
{
	CSIPSubscriber* pSubscriber = NULL;
	char* result = NULL;

	if (msg)
	{
		msg << "CSIPEventPackageManager::Dump type = conference\n";
		msg << "-----------------------------------\n";
	}
}

///////////////////////////////////////////////////////////////////
DWORD CSIPEventPackageManager::GetCSeq()
{
	return m_CSeq;
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageManager::SetConfName(const char* pConfName)
{
	PASSERT_AND_RETURN(NULL == pConfName);

	char confUri[H243_NAME_LEN];
	strcpy_safe(confUri, pConfName);

	char *temp = (char*)strstr(pConfName, "@");
	if (temp)
		*temp = '\0';
	strcpy_safe(m_pConfName, pConfName);

	if (!m_pEventConfInfo)
	{
		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConfByNameOrByNumericId(m_pConfName);
		if (IsValidPObjectPtr(pCommConf))
		{
			PTRACE2(eLevelInfoNormal, "CSIPEventPackageManager::SetConfName conf name is : ", pConfName);
			m_pEventConfInfo = new CConfInfoType(pCommConf, confUri, pCommConf->GetConfContactInfo(0)/*, m_version*/);
		}
	}
}

///////////////////////////////////////////////////////////////////
const char* CSIPEventPackageManager::GetConfName()
{
	return m_pConfName;
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageManager::OnSubscribeTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSIPEventPackageManager::OnSubscribeTout");
	CSIPSubscriber* pSubscriber = NULL;
	//calc current time
	CStructTm curTime;
	STATUS timeStatus = SystemGetTime(curTime);

	//find all subscribers with expired subscription
	std::vector<CSIPSubscriber *>::iterator itr = m_EventSubscribersList.begin();
	while (itr != m_EventSubscribersList.end())
	{
		pSubscriber = (*itr);
		if (IsValidPObjectPtr(pSubscriber))
		{
			DWORD duration = pSubscriber->GetExpireTime() - curTime;
			if (duration < 5)
			{
				PTRACE2(eLevelInfoNormal, "CSIPEventPackageManager::OnSubscribeTout party = ", pSubscriber->GetFrom());
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
void CSIPEventPackageManager::ResetNotifyTimer()
{
	PTRACE(eLevelError, "CSIPEventPackageManager::ResetNotifyTimer");
	m_NotifyTimer = FALSE;
}

///////////////////////////////////////////////////////////////////
void CSIPEventPackageManager::ExpireTime()
{
	CSIPSubscriber* pSubscriber = (CSIPSubscriber*)m_EventSubscribersList[0];
	if(pSubscriber)
	{
		CStructTm curTime;
		STATUS timeStatus = SystemGetTime(curTime);
		pSubscriber->SetExpireTime(&curTime);
	}
}


// start message map -------------------------------------------
PBEGIN_MESSAGE_MAP(CSIPCXPackageManager)

	// conf events
	ONEVENT(TIMER_SUBSCRIBE_TOUT	,CONNECT	,CSIPCXPackageManager::OnSubscribeTout	)
	ONEVENT(TIMER_NOTIFY_TOUT		,CONNECT	,CSIPCXPackageManager::OnNotifyTimerTout)
	ONEVENT(TIMER_LOAD_TOUT		,CONNECT	,CSIPCXPackageManager::OnTimeoutLoadManager	)

PEND_MESSAGE_MAP(CSIPCXPackageManager,CStateMachine);
// end   message map -------------------------------------------

///////////////////////////////////////////////////////////////////
// CSIPCXPackageManager
///////////////////////////////////////////////////////////////////
CSIPCXPackageManager::CSIPCXPackageManager(COsQueue* pRcvMbx, CMplMcmsProtocol* pMplMcmsProtocol, CConfPartyManagerLocalApi* pConfPartyManagerApi, CConfApi* pConfApi)
							:CSIPEventPackageManager(pRcvMbx, pMplMcmsProtocol, pConfPartyManagerApi, pConfApi), m_pConfInfo(NULL), m_pXMLStr(NULL)
{
	memset(m_serviceID, 0, sizeof(m_serviceID));
}

///////////////////////////////////////////////////////////////////
CSIPCXPackageManager::~CSIPCXPackageManager()
{
	POBJDELETE(m_pConfInfo);
	POBJDELETE(m_pXMLStr);
}

///////////////////////////////////////////////////////////////////
void CSIPCXPackageManager::SetConfName(const char* pConfName)
{
	if (pConfName && !strcmp(m_pConfName, ""))
	{
		strcpy_safe(m_pConfName, pConfName);
		char *temp = (char*)strstr(m_pConfName, "@");
		if (temp)
			*temp = '\0';

		if (!m_pConfInfo)
		{
			CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConfByNameOrByNumericId(m_pConfName);
			if (IsValidPObjectPtr(pCommConf))
				m_pConfInfo = new CCXConfInfoType(pCommConf, pConfName);
		}
	}
}

///////////////////////////////////////////////////////////////////
void CSIPCXPackageManager::AttachEvents(CCommConf *pCommConf)
{
	PTRACE(eLevelInfoNormal, "CSIPCXPackageManager::AttachEvents");
	DWORD partyId = 0;
	CConfParty* pConfParty = pCommConf->GetFirstParty();
	while (IsValidPObjectPtr(pConfParty))
	{
		partyId = pConfParty->GetPartyId();
		//Party events
		pConfParty->AttachObserver(m_pRcvMbx, PARTYSTATE, SIP_CX_PACKAGE, partyId);
		pConfParty->AttachObserver(m_pRcvMbx, CHAIROWNER, SIP_CX_PACKAGE, partyId);
		pConfParty->AttachObserver(m_pRcvMbx, DISCAUSE, SIP_CX_PACKAGE, partyId);
		pConfParty->AttachObserver(m_pRcvMbx, MUTE_STATE, SIP_CX_PACKAGE, partyId);

		pConfParty = pCommConf->GetNextParty();
	}
	//Conf events
	pCommConf->AttachObserver(m_pRcvMbx, CONF_LOCK, SIP_CX_PACKAGE);
	pCommConf->AttachObserver(m_pRcvMbx, CONF_ACTIVE, SIP_CX_PACKAGE);
	pCommConf->AttachObserver(m_pRcvMbx, AUDIOSRC, SIP_CX_PACKAGE);
	pCommConf->AttachObserver(m_pRcvMbx, PARTY_ADDED, SIP_CX_PACKAGE);
	pCommConf->AttachObserver(m_pRcvMbx, PARTY_DELETED, SIP_CX_PACKAGE);
}

///////////////////////////////////////////////////////////////////
void CSIPCXPackageManager::DetachEvents(CCommConf *pCommConf)
{
	PTRACE(eLevelInfoNormal, "CSIPCXPackageManager::DetachEvents");

	CConfParty* pConfParty = pCommConf->GetFirstParty();
	while (IsValidPObjectPtr(pConfParty))
	{
		//Party events
		pConfParty->DetachObserver(m_pRcvMbx);

		pConfParty = pCommConf->GetNextParty();
	}
	//Conf events
	pCommConf->DetachObserver(m_pRcvMbx);
}

///////////////////////////////////////////////////////////////////
BYTE CSIPCXPackageManager::FindPartyInInfo(DWORD partyId)
{
	BYTE result = FALSE;
	if (m_pConfInfo)
	{
		if (m_pConfInfo->FindUser(partyId))
			result = TRUE;
	}
	return result;
}

///////////////////////////////////////////////////////////////////
void CSIPCXPackageManager::HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode)
{
}

///////////////////////////////////////////////////////////////////
const char* CSIPCXPackageManager::EventToString(const WORD event)
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
		default:
			return "UNKNOWN_EVENT";
	}
}

///////////////////////////////////////////////////////////////////
void CSIPCXPackageManager::ObserverUpdate(void* pPointer, WORD event, DWORD val)
{
	DWORD partyId = (DWORD)pPointer;

	TRACEINTO << "PartyId:" << partyId << ", Event:" << EventToString(event) << ", Value:" << val;

	if (!m_pConfInfo)
		return;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConfByNameOrByNumericId(m_pConfName);
	CConfParty *pConfParty = NULL;

	switch (event)
	{
		case (PARTYSTATE):
		{
			if (IsValidPObjectPtr(pCommConf))
				pConfParty = pCommConf->GetCurrentParty(partyId);
			else
				PTRACE(eLevelWarn, "CSIPCXPackageManager::ObserverUpdate, conf not found in DB.");
			break;
		}
		case (CHAIROWNER):
		{
			if (val)
				m_pConfInfo->SetAdministrator(partyId, TRUE);
			else
				m_pConfInfo->SetAdministrator(partyId, FALSE);
			break;
		}
		case (DISCAUSE):
		{
			m_pConfInfo->SetDisconnectReason(partyId, val);
			break;
		}
		case (MUTE_STATE):
		{
			if (IsValidPObjectPtr(pCommConf))
				pConfParty = pCommConf->GetCurrentParty(partyId);
			if (IsValidPObjectPtr(pConfParty))
			{
				//Since Istanbul expects all media types to be muted all together
				//we mark all muted once one is received.
				BYTE onOff = pConfParty->IsAudioMutedByParty();
				m_pConfInfo->MutePartyMedia(partyId, eAudio, onOff);
				m_pConfInfo->MutePartyMedia(partyId, eVoip, onOff);
				m_pConfInfo->MutePartyMedia(partyId, eVideoT, onOff);
			}
			break;
		}
		case (CONF_LOCK):
		{
			m_pConfInfo->SetLocked(val);
			break;
		}
		case (CONF_ACTIVE):
		{
			m_pConfInfo->SetActive(val);
			break;
		}
		case (AUDIOSRC):
		{
			if (IsValidPObjectPtr(pCommConf))
				pConfParty = pCommConf->GetCurrentParty(val);
			if (IsValidPObjectPtr(pConfParty))
				m_pConfInfo->SetActiveSpeaker(val, TRUE);
			break;
		}
		case (PARTY_ADDED):
		{
			pConfParty = pCommConf->GetCurrentParty(val);
			if (IsValidPObjectPtr(pConfParty))
			{
				m_pConfInfo->AddParty(pConfParty);
				partyId = pConfParty->GetPartyId();
				//Party events
				pConfParty->AttachObserver(m_pRcvMbx, PARTYSTATE, SIP_CX_PACKAGE, partyId);
				pConfParty->AttachObserver(m_pRcvMbx, CHAIROWNER, SIP_CX_PACKAGE, partyId);
				pConfParty->AttachObserver(m_pRcvMbx, DISCAUSE, SIP_CX_PACKAGE, partyId);
				pConfParty->AttachObserver(m_pRcvMbx, MUTE_STATE, SIP_CX_PACKAGE, partyId);
			}
			else
				PTRACE(eLevelWarn, "CSIPCXPackageManager::ObserverUpdate, party not found.");
			break;
		}
		case (PARTY_DELETED):
		{
			m_pConfInfo->DelParty(val, m_EventSubscribersList.size());
			break;
		}
		default:
			PTRACE(eLevelError, "CSIPCXPackageManager::ObserverUpdate - Invalid event received.");
			PASSERT_AND_RETURN(1);
	}

	//if we can send notify right now
	if (!m_NotifyTimer && !m_LoadTimer)
	{
		CSIPSubscriber* pSubscriber = (CSIPSubscriber*)m_EventSubscribersList[0];
		if (IsValidPObjectPtr(pSubscriber))
		{
			if (m_pConfInfo->WasUpdated())
			{
				m_pXMLStr = BuildNotifyContent(FALSE);
				if (m_pXMLStr)
				{
					WORD boardId = pSubscriber->GetBoardId();
					if (boardId < MAX_BORAD_NUM)
					{
						m_serviceID[boardId] = WAIT_FOR_APPROVAL;
						if (!m_runInTdd)
							StartTimer(TIMER_LOAD_TOUT, LOAD_TOUT);
						m_LoadTimer = TRUE;
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////
void CSIPCXPackageManager::Subscribe(mcIndSubscribe* pSubscribeInd, DWORD callIndex, WORD srcUnitId, DWORD serviceId)
{
	WORD boardId = 1;
	enSipCodes SipStatus = SipCodesBadRequest;
	BYTE bValidReq = TRUE, acceptVerified = TRUE, bLooseMcuApi = FALSE, bSendNotify = FALSE, bReferWithBye = FALSE;
	BYTE bNotifyWasSent = FALSE, bIsNewSubscriber = FALSE, bWasUnsubscribed = FALSE;
	const char *pFromStr = NULL, *pToStr = NULL;
	char *pAcceptName = NULL;
	CMedString cardAddr;
	CSIPCxSubscriber* pSubscriber = NULL;

	CSipHeaderList *pTemp = new CSipHeaderList(pSubscribeInd->sipHeaders);

	bValidReq = CheckRequestValidity(pSubscribeInd, &SipStatus);

	const CSipHeader* pFrom = pTemp->GetNextHeader(kFrom);
	if (pFrom)
		pFromStr = pFrom->GetHeaderStr();

	const CSipHeader* pTo = pTemp->GetNextHeader(kTo);
	if (pTo)
		pToStr = pTo->GetHeaderStr();

	if (pSubscribeInd->expires > 3600)
		pSubscribeInd->expires = 3600;

	if (bValidReq)
	{
		//if party exists
		if (IsValidPObjectPtr(pSubscriber))
		{
			//check if subscribe request is not forked
			if (boardId == pSubscriber->GetBoardId())
			{
				//unsubscribe
				if (0 == pSubscribeInd->expires)
				{
					PTRACE2(eLevelInfoNormal, "CSIPCXPackageManager::Subscribe, unsubscribing party = ", pFromStr);
					RemoveFromVector(pSubscriber);
					SipStatus = SipCodesOk;
					POBJDELETE(pSubscriber);
					if (0 == m_EventSubscribersList.size())
					{
						PTRACE(eLevelInfoNormal, "CSIPCXPackageManager::Subscribe, unsubscribing last party");
						CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConfByNameOrByNumericId(m_pConfName);
						if (IsValidPObjectPtr(pCommConf))
							DetachEvents(pCommConf);
						POBJDELETE(m_pConfInfo);
						*m_pConfName = '\0';
						if (m_NotifyTimer)
						{
							DeleteTimer(TIMER_NOTIFY_TOUT);
							m_NotifyTimer = FALSE;
						}
					}
				}
				//refresh expires Tout.
				else
				{
					char s[10];
					sprintf(s, "%ld", pSubscribeInd->expires);
					PTRACE2(eLevelInfoNormal, "CSIPCXPackageManager::Subscribe, refreshing subscription, expires = ", s);
					SipStatus = SipCodesOk;
					bSendNotify = TRUE;
				}
			}
		}
		//new subscriber
		else
		{
			//unsubscribe is ok even if party is not found.
			if (0 == pSubscribeInd->expires)
			{
				PTRACE(eLevelInfoNormal, "CSIPCXPackageManager::Subscribe, unsubscribing party is not found.");
				SipStatus = SipCodesOk;
			}
			else
			{
				CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConfByNameOrByNumericId(m_pConfName);
				if (IsValidPObjectPtr(pCommConf))
				{
					STATUS RsrcStatus = STATUS_OK;

					WORD connectionId = 0;
					WORD RsrcBoardId = boardId;
					if (STATUS_OK == RsrcStatus)
					{
						PTRACE(eLevelInfoNormal, "CSIPCXPackageManager::Subscribe, new subscriber party is added.");
						m_EventSubscribersList.push_back(pSubscriber);
						AttachEvents(pCommConf);
						SipStatus = SipCodesOk;
						bSendNotify = TRUE;
						bIsNewSubscriber = TRUE;
					}
					else
					{
						char s[8];
						snprintf(s, ARRAYSIZE(s), "%d", RsrcBoardId);
						PTRACE2(eLevelWarn, "CSIPCXPackageManager::Subscribe, RscsAlloc offers to redirect watcher to board = ", s);
						//if Rsrc offers another board
						char* pRedirectIp = NULL;
						if (RsrcBoardId != boardId)
						{
							pRedirectIp = GetRedirectIp(boardId, RsrcBoardId);
							if (pRedirectIp)
							{
								cardAddr << m_pConfName << "@" << pRedirectIp;
								PDELETEA(pRedirectIp);
								SipStatus = SipCodesMovedTemp;
							}
							else
								SipStatus = SipCodesBusyHere;
						}
					}
				}
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
	else if (pFromStr)
		PTRACE(eLevelInfoNormal, "CSIPCXPackageManager::Subscribe, Invalid request is rejected.");

	if (SipCodesOk != SipStatus)
		pSubscribeInd->expires = 0;

	SendSipSubscribeResponse(pSubscribeInd, SipStatus, callIndex, serviceId);

	if (bSendNotify && SipCodesOk == SipStatus)
	{
		//this notify is sent only to one subscriber
		CObjString* pStr = BuildNotifyContent(TRUE, bIsNewSubscriber);
		bNotifyWasSent = Notify(pSubscriber, (char*)pStr->GetString(), "active");
	}

	POBJDELETE(pTemp);
}

///////////////////////////////////////////////////////////////////
CSIPCxSubscriber* CSIPCXPackageManager::FindParty2(const char* from, WORD watcherId)
{
	CSIPCxSubscriber* pSubscriber = NULL;
	if (from)
	{
		CSmallString str;
		str << from << " WatcherId=" << watcherId << "\n";
		PTRACE2(eLevelInfoNormal, "CSIPCXPackageManager::FindParty named = ", str.GetString());

		std::vector<CSIPSubscriber *>::iterator itr = m_EventSubscribersList.begin();
		while (itr != m_EventSubscribersList.end())
		{
			pSubscriber = (CSIPCxSubscriber*)(*itr);
			if (IsValidPObjectPtr(pSubscriber))
			{
				if (pSubscriber->GetFrom() && strcmp(pSubscriber->GetFrom(), ""))
				{
					if (!strncmp(from, pSubscriber->GetFrom(), IP_LIMIT_ADDRESS_CHAR_LEN))
					{
						if (watcherId == pSubscriber->GetWatcherId())
							break;
						else
							pSubscriber = NULL;
					}
				}
				else
					pSubscriber = NULL;
			}
			++itr;
		}
	}
	return pSubscriber;
}

///////////////////////////////////////////////////////////////////
DWORD CSIPCXPackageManager::MediaCheckSum(char *pBuff, int len)
{
	WORD *psh = (WORD*)pBuff;
	DWORD sum = 0;
	WORD pad = 0;

	while ((len -= 32) >= 0)
	{
		sum += psh[0];
		sum += psh[1];
		sum += psh[2];
		sum += psh[3];
		sum += psh[4];
		sum += psh[5];
		sum += psh[6];
		sum += psh[7];
		sum += psh[8];
		sum += psh[9];
		sum += psh[10];
		sum += psh[11];
		sum += psh[12];
		sum += psh[13];
		sum += psh[14];
		sum += psh[15];
		psh += 16;
	}
	len += 32;

	while ((len -= 8) >= 0)
	{
		sum += psh[0];
		sum += psh[1];
		sum += psh[2];
		sum += psh[3];
		psh += 4;
	}
	len += 8;

	while ((len -= 2) >= 0)
	{
		sum += *psh;
		psh++;
	}

	if (len == -1)
	{
		pad = *((unsigned char*)psh);
		sum += pad << 8;
	}

	return sum;
}

///////////////////////////////////////////////////////////////////
//pSubscriber will be valid only if Notify is sent to one Subscriber.
//if it is NULL -> notify should be sent to all
BYTE CSIPCXPackageManager::Notify(CSIPSubscriber* pSubscriber, char* content, char* state)
{
	PTRACE(eLevelInfoNormal, "CSIPCXPackageManager::Notify");
	BYTE bFullXML = FALSE, result = FALSE, bIsCompress = FALSE;

	//check if content has 'full' data
	//full ,ust be the whole XML state, and not the 'User' State.
	char *pMsft = (char*)strstr(content, "com.microsoft.sip.cx-conference-info");
	char *pFull = (char*)strstr(content, "state=\"full\"");
	char *pUser = (char*)strstr(content, "user uri");

	if ((pMsft < pFull && pFull < pUser) || (pFull && NULL == pUser))
		bFullXML = TRUE;

	uLong uncomprLen = 100000;

	//zip string if needed
	uLong comprLen = 100000;
	char* compr = (char*)calloc(comprLen, 1);
	ALLOCBUFFER(uncompr, uncomprLen);
	//zip string if needed
	uLong length = strlen(content);
	if (length > SIZE_TO_ZIP)
	{
		int err;
		err = compress((Byte *)compr, &comprLen, (const Bytef*)content, (uLong)length);
		if (Z_OK == err)
		{
			PASSERT(comprLen > 100000);
			void* m = realloc(compr, comprLen);
			if (m == NULL)
				PASSERT(109);
			compr = (char*)m;
			PTRACE2(eLevelInfoNormal, "CSIPCXPackageManager::Notify, the unzipped content: \n", content);
			bIsCompress = TRUE;
		}
		// the uncompress is just for the PASSERT purpose
		err = uncompress((unsigned char *)uncompr, &uncomprLen, (unsigned char *)compr, comprLen);
		DBGPASSERT(strcmp((char* )uncompr, content));

	}

	int contentLength = 0;
	CSipDistrNotifyStruct SipNotify;
	SipNotify.SetContentType("application/com.microsoft.sip.cx-conference-info+xml");
	if (bIsCompress)
	{
		SipNotify.SetContentUnzippedOrgSize(length);
		SipNotify.SetContent(compr, comprLen);
		contentLength = comprLen;

	}
	else
	{
		SipNotify.SetContentUnzippedOrgSize(0);
		contentLength = length;
		SipNotify.SetContent(content, contentLength);
	}

	free(compr);
	compr = NULL;
	DEALLOCBUFFER(uncompr);

	SipNotify.SetHeaderField(kEvent, "com.microsoft.sip.cx-conference-info");

	if (bFullXML)
		SipNotify.AddWatcherId(((CSIPCxSubscriber*)pSubscriber)->GetWatcherId());
	//add all subscribers from same board Id
	else
	{
		CSIPSubscriber* pTemp = NULL;
		std::vector<CSIPSubscriber *>::iterator itr = m_EventSubscribersList.begin();
		while (itr != m_EventSubscribersList.end())
		{
			pTemp = (CSIPSubscriber*)(*itr);
			if (pTemp && pTemp->GetBoardId() == pSubscriber->GetBoardId())
				SipNotify.AddWatcherId(((CSIPCxSubscriber*)pTemp)->GetWatcherId());
			++itr;
		}
	}

	char SubState[H243_NAME_LEN] = { 0 };
	strcpy_safe(SubState, state);

	//add expires only after 'active' and only if sending 'full' xml body.
	if (strncmp(state, "terminated", 10) && bFullXML)
	{
		int len = strlen(SubState);
		if (len < H243_NAME_LEN - 16)
		{
			CStructTm curTime;
			STATUS timeStatus = SystemGetTime(curTime);
			char temp[30];
			sprintf(temp, ";expires=%d", pSubscriber->GetExpireTime() - curTime);
			strncat(SubState, temp, 15);
		}
	}
	SipNotify.SetHeaderField(kSubscrpState, SubState);

	return result;
}

///////////////////////////////////////////////////////////////////
CObjString* CSIPCXPackageManager::BuildNotifyContent(BYTE bFull, BYTE bIsNewSubscriber)
{
	CObjString *pContentStr = NULL;
	char *XML = NULL, *pValue = NULL;
	DWORD length = 0;
	CXMLDOMElement *pElement = new CXMLDOMElement;
	m_pConfInfo->SerializeXml(pElement, bFull, bIsNewSubscriber);
	//if there was no change in data -> no xml will be created!!!
	pElement->get_tagName(&pValue);
	if (NULL != pValue)
	{
		CObjString *pStr = NULL;

		pElement->DumpDataAsStringWithAttribute(&XML, &length);

		pStr = new CObjString(XML, length + 20);

		PDELETE(pElement);
		PDELETEA(XML);

		if (bFull)
			pContentStr = UpdateVersionInContent(pStr, m_version);
		else
			pContentStr = UpdateVersionInContent(pStr, ++m_version);

		POBJDELETE(pStr);
	}
	else
		pContentStr = new CSmallString("");

	return pContentStr;
}

///////////////////////////////////////////////////////////////////
CObjString* CSIPCXPackageManager::UpdateVersionInContent(CObjString *pStr, int version)
{
	CObjString* pResultStr = NULL;
	if (1 != version)
	{
		char* pToken = (char*)strstr(pStr->GetString(), "version");
		if (pToken)
		{
			int position = pToken - pStr->GetString() + 9;
			CObjString prefixStr(pStr->GetString(), position);
			CObjString suffixStr(pToken + 10, pStr->GetStringLength() - position);

			pResultStr = new CObjString("", prefixStr.GetStringLength() + suffixStr.GetStringLength() + 20);

			char Ver[12];
			sprintf(Ver, "%d", version);

			*pResultStr << prefixStr << Ver << suffixStr;
		}
	}
	else
		pResultStr = new CObjString(pStr->GetString());
	return pResultStr;
}

///////////////////////////////////////////////////////////////////
BYTE CSIPCXPackageManager::CheckRequestValidity(mcIndSubscribe* pSubscribeInd, enSipCodes *SipStatus)
{
	BYTE bValidReq = TRUE, acceptVerified = TRUE;
	const char *pFromStr = NULL, *pReferToStr = NULL;
	char* pAcceptName = NULL;
	CSipHeaderList * pTemp = new CSipHeaderList(pSubscribeInd->sipHeaders);

	//verify 'From' is not empty
	const CSipHeader* pFrom = pTemp->GetNextHeader(kFrom);
	if (pFrom)
		pFromStr = pFrom->GetHeaderStr();
	else
		bValidReq = FALSE;

	//verify 'Accept' is valid
	const CSipHeader* pAccept = pTemp->GetNextHeader(kAccept);
	while (pAccept)
	{
		pAcceptName = (char*)pAccept->GetHeaderStr();
		if (pAcceptName)
		{
			//verify allow is either empty or holds "application/conference-info+xml"
			if (strncmp(pAcceptName, "application/com.microsoft.sip.cx-conference-info+xml", H243_NAME_LEN))
			{
				if (strcmp(pAcceptName, ""))
					acceptVerified = FALSE;
			}
			else
				acceptVerified = TRUE;
		}
		pAccept = pTemp->GetNextHeader(kAccept);
	}
	if (!acceptVerified)
	{
		*SipStatus = SipCodesNotAcceptable;
		bValidReq = FALSE;
	}

	//verify ip is not empty
	POBJDELETE(pTemp);

	return bValidReq;
}

///////////////////////////////////////////////////////////////////
void CSIPCXPackageManager::OnSubscribeTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSIPCXPackageManager::OnSubscribeTout");
	CSIPSubscriber* pSubscriber = NULL;
	//calc current time
	CStructTm curTime;
	STATUS timeStatus = SystemGetTime(curTime);

	//find all subscribers with expired subscription
	std::vector<CSIPSubscriber *>::iterator itr = m_EventSubscribersList.begin();
	while (itr != m_EventSubscribersList.end())
	{
		pSubscriber = (CSIPSubscriber*)(*itr);
		if (IsValidPObjectPtr(pSubscriber))
		{
			DWORD duration = pSubscriber->GetExpireTime() - curTime;
			if (duration < 5)
			{
				PTRACE2(eLevelInfoNormal, "CSIPCXPackageManager::OnSubscribeTout party = ", pSubscriber->GetFrom());
				Notify(pSubscriber, "", "terminated;reason=noresource");

				//if not TDD
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
void CSIPCXPackageManager::OnNotifyTimerTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSIPCXPackageManager::OnNotifyTimerTout");
	//if there is a waiting Notify message, send it
	if (m_pNotifyMsg)
	{
		PTRACE(eLevelInfoNormal, "CSIPCXPackageManager::OnNotifyTimerTout, a message waiting to be sent.");
		if (!m_runInTdd)
			StartTimer(TIMER_NOTIFY_TOUT, SIP_CX_NOTIFY_TOUT);
		m_NotifyTimer = TRUE;
		PDELETE(m_pNotifyMsg);
	}
	//else, mark that a new message may be sent.
	//and check if there is a new xml to be created and sent
	else
	{
		DeleteTimer(TIMER_NOTIFY_TOUT);
		m_NotifyTimer = FALSE;

		CSIPSubscriber* pSubscriber = (CSIPSubscriber*)m_EventSubscribersList[0];
		if (IsValidPObjectPtr(pSubscriber))
		{
			if (m_pConfInfo->WasUpdated())
			{
				m_pXMLStr = BuildNotifyContent(FALSE);
				if (m_pXMLStr)
				{
					WORD boardId = pSubscriber->GetBoardId();
					if (boardId < MAX_BORAD_NUM)
						m_serviceID[boardId] = WAIT_FOR_APPROVAL;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////
void CSIPCXPackageManager::OnLoadManagerAccept()
{
}

///////////////////////////////////////////////////////////////////
void CSIPCXPackageManager::OnTimeoutLoadManager(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSIPCXPackageManager::OnTimeoutLoadManager");

	//if there is what to send
	if (m_LoadTimer && m_pXMLStr)
	{
		DeleteTimer(TIMER_LOAD_TOUT);
		m_LoadTimer = FALSE;

		//find board id
		int i = 0, boardId = 0;

		CSIPSubscriber* pSubscriber = NULL;
		std::vector<CSIPSubscriber *>::iterator itr = m_EventSubscribersList.begin();
		while (itr != m_EventSubscribersList.end())
		{
			pSubscriber = (CSIPSubscriber*)(*itr);
			if (IsValidPObjectPtr(pSubscriber) && boardId == pSubscriber->GetBoardId())
			{
				//Send this notify to all subscribers from that Card Id.
				Notify(pSubscriber, (char*)m_pXMLStr->GetString(), "active");
				break;
			}
			++itr;
		}

		if (pSubscriber)
		{
			if (pSubscriber->GetBoardId() < MAX_BORAD_NUM)
			{
				m_serviceID[pSubscriber->GetBoardId()] = NOTIFY_WAS_SENT;
				pSubscriber = NULL;
			}
			else
				PASSERT(1);
		}

		//look for subscribers from other boards
		itr = m_EventSubscribersList.begin();
		while (itr != m_EventSubscribersList.end())
		{
			pSubscriber = (CSIPSubscriber*)(*itr);
			if (pSubscriber->GetBoardId() >= MAX_BORAD_NUM)
			{
				++itr;
				PASSERT(1);
				continue;
			}

			if (NOT_USED == m_serviceID[pSubscriber->GetBoardId()])
			{
				m_serviceID[pSubscriber->GetBoardId()] = WAIT_FOR_APPROVAL;
				if (!m_runInTdd)
					StartTimer(TIMER_LOAD_TOUT, LOAD_TOUT);
				m_LoadTimer = TRUE;
				break;
			}
			++itr;
		}
		if (itr == m_EventSubscribersList.end())
		{
			POBJDELETE(m_pXMLStr);
			//start timer to disable consecutive notify
			if (!m_runInTdd)
				StartTimer(TIMER_NOTIFY_TOUT, SIP_CX_NOTIFY_TOUT);
			m_NotifyTimer = TRUE;
		}
	}
	DeleteTimer(TIMER_LOAD_TOUT);
	m_LoadTimer = FALSE;
}

///////////////////////////////////////////////////////////////////
void CSIPCXPackageManager::TerminateConf()
{
	DeleteTimer(TIMER_SUBSCRIBE_TOUT);
	m_state = TERMINATION;

	//find all subscribers
	std::vector<CSIPSubscriber *>::iterator itr = m_EventSubscribersList.begin();
	while (itr != m_EventSubscribersList.end())
	{
		CSIPSubscriber* pSubscriber = (CSIPSubscriber*)(*itr);
		//if board id was not used yet

		if (IsValidPObjectPtr(pSubscriber))
		{
			WORD boardId = pSubscriber->GetBoardId();
			if (boardId < MAX_BORAD_NUM && NOT_USED == m_serviceID[boardId])
			{
				PTRACE2(eLevelInfoNormal, "CSIPCXPackageManager::TerminateConf party = ", pSubscriber->GetFrom());
				Notify(pSubscriber, "", "terminated;reason=noresource");
				WORD boardId = pSubscriber->GetBoardId();
				if (boardId < ARRAYSIZE(m_serviceID))
					m_serviceID[boardId] = NOTIFY_WAS_SENT;

				m_EventSubscribersList.erase(itr);
				POBJDELETE(pSubscriber);
				itr = m_EventSubscribersList.begin();
				continue;
			}
		}
		++itr;
	}
}

///////////////////////////////////////////////////////////////////
char* CSIPCXPackageManager::GetRedirectIp(WORD orgBoardId, WORD newBoardId)
{
	return 0;
}


///////////////////////////////////////////////////////////////////
// CSIPSubscriber
///////////////////////////////////////////////////////////////////
CSIPSubscriber::CSIPSubscriber() : m_fromIp(0), m_fromPort(0), m_transportType(0), m_serviceID(0), m_srcUnitId(0), m_csId(0), m_callIndex(0),
		 m_role(eParticipant),m_notifyVersionCounter(2)
{
	memset(m_pFromUri, 0, sizeof(m_pFromUri));
	memset(m_pToUri, 0, sizeof(m_pToUri));
	memset(m_pCallId, 0, sizeof(m_pCallId));
	memset(m_pFromTag, 0, sizeof(m_pFromTag));
	memset(m_pToTag, 0, sizeof(m_pToTag));
}

///////////////////////////////////////////////////////////////////
CSIPSubscriber::CSIPSubscriber(WORD boardId, const char* pFrom, const char* pFromTag, const char* pTo, const char* pToTag, const char* pCallId,
							   DWORD fromIp, WORD fromPort, WORD fromTransport, WORD expires, WORD srcUnitId, DWORD callIndex, eRoleType role, DWORD cs_Id)
		:m_fromIp(fromIp), m_fromPort(fromPort), m_transportType(fromTransport), m_serviceID(boardId), m_csId(cs_Id), m_role(role), m_notifyVersionCounter(2)
{
	strcpy_safe(m_pFromUri, pFrom);
	strcpy_safe(m_pFromTag, pFromTag);
	strcpy_safe(m_pToUri, pTo);
	strcpy_safe(m_pToTag, pToTag);
	strcpy_safe(m_pCallId, pCallId);
	m_srcUnitId = srcUnitId;
	m_callIndex = callIndex;

	CStructTm curTime;
	STATUS timeStatus = SystemGetTime(curTime);//add expires to cur time
	m_ExpiresTime = curTime;
	m_ExpiresTime.m_hour += expires/3600;
	m_ExpiresTime.m_min += (expires%3600)/60;
	m_ExpiresTime.m_sec += (expires%3600)%60;

}

///////////////////////////////////////////////////////////////////
CSIPSubscriber::~CSIPSubscriber()
{
}

///////////////////////////////////////////////////////////////////
void CSIPSubscriber::Refresh(DWORD ip, WORD port, WORD transport, const char* pCallId, WORD expires)
{
	PTRACE2(eLevelInfoNormal, "CSIPSubscriber::Refresh for party = ", m_pFromUri);

	m_fromIp = ip;
	m_fromPort = port;
	m_transportType = transport;
	strcpy_safe(m_pCallId, pCallId);

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
WORD CSIPSubscriber::GetBoardId()
{
	return m_serviceID;
}

///////////////////////////////////////////////////////////////////
char* CSIPSubscriber::GetFrom()
{
	return m_pFromUri;
}

///////////////////////////////////////////////////////////////////
char* CSIPSubscriber::GetFromTag()
{
	return m_pFromTag;
}

///////////////////////////////////////////////////////////////////
char* CSIPSubscriber::GetTo()
{
	return m_pToUri;
}

///////////////////////////////////////////////////////////////////
char* CSIPSubscriber::GetToTag()
{
	return m_pToTag;
}

///////////////////////////////////////////////////////////////////
DWORD CSIPSubscriber::GetIp()
{
	return m_fromIp;
}
///////////////////////////////////////////////////////////////////
DWORD CSIPSubscriber::GetCsId()
{
	return m_csId;
}
///////////////////////////////////////////////////////////////////
WORD CSIPSubscriber::GetPort()
{
	return m_fromPort;
}
///////////////////////////////////////////////////////////////////
WORD CSIPSubscriber::GetTransport()
{
	return m_transportType;
}

///////////////////////////////////////////////////////////////////
char* CSIPSubscriber::GetCallId()
{
	return m_pCallId;
}

///////////////////////////////////////////////////////////////////
eRoleType CSIPSubscriber::GetRole()
{
	return m_role;
}

///////////////////////////////////////////////////////////////////
WORD CSIPSubscriber::GetSrcUnitId()
{
	return m_srcUnitId;
}

///////////////////////////////////////////////////////////////////
DWORD CSIPSubscriber::GetCallIndex()
{
	return m_callIndex;
}

///////////////////////////////////////////////////////////////////
CStructTm CSIPSubscriber::GetExpireTime()
{
	return m_ExpiresTime;
}

///////////////////////////////////////////////////////////////////
void CSIPSubscriber::SetExpireTime(CStructTm* newTime)
{
	m_ExpiresTime = *newTime;
}

///////////////////////////////////////////////////////////////////
char* CSIPSubscriber::ToString()
{
	return m_pFromUri;
}

///////////////////////////////////////////////////////////////////
int CSIPSubscriber::GetAndIncrementNotifyVersionCounter()
{
	return m_notifyVersionCounter++;
}

/////////////////////////////////////////////////////////////////////////////
WORD operator==(const CSIPSubscriber& first, const CSIPSubscriber& second)
{
	WORD rval = 0;
	if (!strncmp(first.m_pFromUri, second.m_pFromUri, MaxAddressListSize) &&
			!strncmp(first.m_pToUri, second.m_pToUri, MaxAddressListSize) &&
			!strncmp(first.m_pCallId, second.m_pCallId, MaxAddressListSize))
		rval = 1;

	return rval;
}


///////////////////////////////////////////////////////////////////
// CSIPCxSubscriber
///////////////////////////////////////////////////////////////////
CSIPCxSubscriber::CSIPCxSubscriber() : m_watcherId(0), m_mcmsConnId(0)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
CSIPCxSubscriber::CSIPCxSubscriber(WORD boardId, const char* pFrom, const char* pTo, WORD expires, WORD watcherId, WORD connectionId, eRoleType role)
		:CSIPSubscriber(boardId,pFrom,"",pTo,"","",0,0,0,expires,0,0, role), m_watcherId(watcherId), m_mcmsConnId(connectionId)
{
}

///////////////////////////////////////////////////////////////////
void CSIPCxSubscriber::Refresh(WORD expires, WORD watcherId)
{
	CSIPSubscriber::Refresh(0,0,0,"",expires);
	m_watcherId = watcherId;
}

///////////////////////////////////////////////////////////////////
WORD CSIPCxSubscriber::GetWatcherId()
{
	return m_watcherId;
}

///////////////////////////////////////////////////////////////////
WORD CSIPCxSubscriber::GetConnectionId()
{
	return m_mcmsConnId;
}

#endif //__SIPCONFPACKAGE_CPP__
