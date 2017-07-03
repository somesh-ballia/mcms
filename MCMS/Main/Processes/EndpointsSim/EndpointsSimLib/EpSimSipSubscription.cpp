//////////////////////////////////////////////////////////////////////
//
// EpSimSipSubscription.cpp: SIP subscriptions / notifications classes.
//
//////////////////////////////////////////////////////////////////////

#include "Macros.h"
#include "StatusesGeneral.h"
#include "HostCommonDefinitions.h"
#include "IpCsOpcodes.h"
#include "SipCsReq.h"
#include "SipCsInd.h"

#include "Segment.h"
#include "TraceStream.h"
#include "TaskApi.h"
#include "MplMcmsProtocol.h"

#include "EndpointsSim.h"
#include "CommEndpointsSimSet.h"
#include "EpSimSipSubscription.h"
#include "ObjString.h"
#include "CSIDTaskApi.h"
#include "OpcodesMcmsInternal.h"
/////////////////////////////////////////////////////////////////////////////
//    CEpSimSipSubscriptionsMngr
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CEpSimSipSubscriptionsMngr::CEpSimSipSubscriptionsMngr()
{
	m_pCsApi = NULL;

	for( int i=0; i<MAX_SIP_SIBSCRIPTIONS; i++ )
		m_paSubscriptions[i] = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CEpSimSipSubscriptionsMngr::~CEpSimSipSubscriptionsMngr()
{
	for( int i=0; i<MAX_SIP_SIBSCRIPTIONS; i++ )
		POBJDELETE(m_paSubscriptions[i]);
}

/////////////////////////////////////////////////////////////////////////////
void CEpSimSipSubscriptionsMngr::Init(CTaskApi* pCsApi)
{
	POBJDELETE(m_pCsApi);
	m_pCsApi = pCsApi;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CEpSimSipSubscriptionsMngr::ProcessBatchRequest(CCommSetAddSubscription* pAddSubscriptionSet)
{
	int indFirstEmpty = MAX_SIP_SIBSCRIPTIONS;

	int i=0;
	for( i=0; i<MAX_SIP_SIBSCRIPTIONS; i++ )
	{
		// find matching subscriptions
		if( NULL == m_paSubscriptions[i] )
		{
			if( MAX_SIP_SIBSCRIPTIONS <= indFirstEmpty )
				indFirstEmpty = i;
			continue;
		}
		else
		{
			if( 0 == strcmp(pAddSubscriptionSet->GetConfName(),m_paSubscriptions[i]->GetConfName()) &&
				0 == strcmp(pAddSubscriptionSet->GetSubscriber(),m_paSubscriptions[i]->GetSubscriberName()) &&
				0 == strcmp(pAddSubscriptionSet->GetEvent(),m_paSubscriptions[i]->GetEvent()) )
			{
				// match found
				//   if 'expires' = 0, remove subscription
				if( 0 == pAddSubscriptionSet->GetExpires() )
				{
					PTRACE(eLevelInfoNormal,"CEpSimSipSubscriptionsMngr::ProcessBatchRequest(ADD) - subscription found, expired. Delete.");
					POBJDELETE(m_paSubscriptions[i]);
				}
				else
				{
					PTRACE(eLevelInfoNormal,"CEpSimSipSubscriptionsMngr::ProcessBatchRequest(ADD) - subscription found, only send Indication.");
					SendSipSubscribeInd(m_paSubscriptions[i],pAddSubscriptionSet->GetExpires());
				}
				break;
			}
		}
	}
	// if subscription was not found
	if( MAX_SIP_SIBSCRIPTIONS == i )
	{
		// if empty place exists
		if( MAX_SIP_SIBSCRIPTIONS > indFirstEmpty )
		{
			PTRACE(eLevelInfoNormal,"CEpSimSipSubscriptionsMngr::ProcessBatchRequest(ADD) - ADD new subscription and send indication.");
			m_paSubscriptions[indFirstEmpty] = new CEpSimSipSubscription(pAddSubscriptionSet->GetConfName(),pAddSubscriptionSet->GetSubscriber(),pAddSubscriptionSet->GetEvent());
			// send SIP indication about new subscription
			SendSipSubscribeInd(m_paSubscriptions[indFirstEmpty],pAddSubscriptionSet->GetExpires());
		}
		else
			PTRACE(eLevelInfoNormal,"CEpSimSipSubscriptionsMngr::ProcessBatchRequest(ADD) - no place to ADD new subscription.");
	}

	DumpList(eLevelError);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CEpSimSipSubscriptionsMngr::ProcessBatchRequest(CCommSetGetNotification* pGetNotificationSet)
{
	int i=0;
	for( i=0; i<MAX_SIP_SIBSCRIPTIONS; i++ )
	{
		// find matching subscriptions
		if( NULL != m_paSubscriptions[i] )
		{
			if( 0 == strcmp(pGetNotificationSet->GetConfName(),m_paSubscriptions[i]->GetConfName()) &&
				0 == strcmp(pGetNotificationSet->GetSubscriber(),m_paSubscriptions[i]->GetSubscriberName()) &&
				0 == strcmp(pGetNotificationSet->GetEvent(),m_paSubscriptions[i]->GetEvent()) )
			{
				PTRACE(eLevelInfoNormal,"CEpSimSipSubscriptionsMngr::ProcessBatchRequest(GET) - subscription found. Get notification.");
				pGetNotificationSet->SetSubscriptionNotification(m_paSubscriptions[i]->GetLastNotification());
				break;
			}
		}
	}
	if( MAX_SIP_SIBSCRIPTIONS == i )
		PTRACE(eLevelInfoNormal,"CEpSimSipSubscriptionsMngr::ProcessBatchRequest(GET) - subscription not found.");

	DumpList(eLevelError);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CEpSimSipSubscriptionsMngr::ProcessCsRequest(CMplMcmsProtocol* pMplMcmsProt)
{
	STATUS status = STATUS_FAIL;
	switch( pMplMcmsProt->getOpcode() )
	{
		case SIP_CS_PROXY_NOTIFY_REQ:
		{
			status = CsSipNotificationMsg(pMplMcmsProt);
			break;
		}
	}
	return status;
}
/*
/////////////////////////////////////////////////////////////////////////////
STATUS CEpSimSipSubscriptionsMngr::CsSipNotificationMsg(CMplMcmsProtocol* pMplMcmsProt)
{
	STATUS status = STATUS_FAIL;

	char *pszFrom = NULL, *pszTo = NULL, *pszState = NULL;
	DWORD event = 0xFFFFFFFF;

	// get notification info: from, to and event type
	mcReqNotify* pReq = (mcReqNotify*)pMplMcmsProt->GetData();
	char* pszContent = pReq->content;

	sipMessageHeaders *  pHeaders = &pReq->sipHeaders;

	sipHeaderElement* 	 pTheHeader = NULL;
	int  numOfHeaders  = pHeaders->numOfHeaders;

	char* pSList = pHeaders->headersList + numOfHeaders * sizeof(sipHeaderElement);
	for( int i=0; i<numOfHeaders; i++ )
	{
		pTheHeader = (sipHeaderElement*) (pHeaders->headersList + i * sizeof(sipHeaderElement));
		char* pString = &(pSList[pTheHeader->position]);

		if( kTo == pTheHeader->eHeaderField )
			pszTo = pString;
		else if( kFrom == pTheHeader->eHeaderField )
			pszFrom = pString;
		else if( kSubscrpState == pTheHeader->eHeaderField )
			pszState = pString;
	}

	// search for match in subscriptions array
	for( int i=0; i<MAX_SIP_SIBSCRIPTIONS; i++ )
	{
		if( NULL == m_paSubscriptions[i] )
			continue;
		if( 0 == strcmp(m_paSubscriptions[i]->GetConfName(),pszFrom) )
		{
			if( 0 == strcmp(m_paSubscriptions[i]->GetSubscriberName(),pszTo) )
			{
				if( NULL == strstr(pszState,"active") )
				{
					PTRACE(eLevelInfoNormal,"CEpSimSipSubscriptionsMngr::CsSipNotificationMsg - subscription not active. Delete.");
					POBJDELETE(m_paSubscriptions[i]);
				} else {
					m_paSubscriptions[i]->SetLastNotification(pszContent);
				}
				status =  STATUS_OK;
			}
		}
	}

	if( status !=  STATUS_OK )
		TRACESTR(eLevelError) << " CEpSimSipSubscriptionsMngr::CsSipNotificationMsg - "
			<< "Unknown subscription notification: "
			<< "From <" << pszFrom << ">, To <" << pszTo << ">, State <" << pszState << ">";

	DumpList(eLevelError);

	return status;
}
*/
/////////////////////////////////////////////////////////////////////////////
STATUS CEpSimSipSubscriptionsMngr::CsSipNotificationMsg(CMplMcmsProtocol* pMplMcmsProt)
{
	STATUS status = STATUS_FAIL;

	char *pszFrom = NULL, *pszTo = NULL, *pszState = NULL;
	DWORD event = 0xFFFFFFFF;

	// get notification info: from, to and event type
	mcReqNotify* pReq = (mcReqNotify*)pMplMcmsProt->GetData();

	sipContentAndHeadersSt*  pSip = &(pReq->sipContentAndHeaders);
	sipMessageHeaders *  pHeaders = (sipMessageHeaders *)(pSip->contentAndHeaders + pSip->sipHeadersOffset);
	sipHeaderElement* 	 pTheHeader = NULL;
	int  numOfHeaders  = pHeaders->numOfHeaders;
	char* pSList = pHeaders->headersList + numOfHeaders * sizeof(sipHeaderElement);

//	char* pszContent = pSip->contentAndHeaders;
	ALLOCBUFFER(pszContent,pSip->sipHeadersOffset+1);
	strncpy(pszContent,pSip->contentAndHeaders,pSip->sipHeadersOffset);
    pszContent[pSip->sipHeadersOffset] = '\0';

	for( int i=0; i<numOfHeaders; i++ )
	{
		pTheHeader = (sipHeaderElement*) (pHeaders->headersList + i * sizeof(sipHeaderElement));
		char* pString = &(pSList[pTheHeader->position]);

		if( kTo == pTheHeader->eHeaderField )
			pszTo = pString;
		else if( kFrom == pTheHeader->eHeaderField )
			pszFrom = pString;
		else if( kSubscrpState == pTheHeader->eHeaderField )
			pszState = pString;
	}

	// search for match in subscriptions array
	for( int i=0; i<MAX_SIP_SIBSCRIPTIONS; i++ )
	{
		if( NULL == m_paSubscriptions[i] )
			continue;
		if(pszFrom && 0 == strcmp(m_paSubscriptions[i]->GetConfName(),pszFrom) )
		{
			if(pszTo && 0 == strcmp(m_paSubscriptions[i]->GetSubscriberName(),pszTo) )
			{
				if( pszState && NULL == strstr(pszState,"active") )
				{
					PTRACE(eLevelInfoNormal,"CEpSimSipSubscriptionsMngr::CsSipNotificationMsg - subscription not active. Delete.");
					POBJDELETE(m_paSubscriptions[i]);
				} else {
					PTRACE(eLevelInfoNormal,"CEpSimSipSubscriptionsMngr::CsSipNotificationMsg - set last notification.");
					PTRACE(eLevelInfoNormal, pszContent);
					m_paSubscriptions[i]->SetLastNotification(pszContent);
				}
				status =  STATUS_OK;
			}
		}
	}

	DEALLOCBUFFER(pszContent);

	if( status !=  STATUS_OK)
	{
		if (pszState)
			TRACESTR(eLevelError) << " CEpSimSipSubscriptionsMngr::CsSipNotificationMsg - "
				<< "Unknown subscription notification: "
				<< "From <" << pszFrom << ">, To <" << pszTo << ">, State <" << pszState << ">";
		else
			TRACESTR(eLevelError) << " CEpSimSipSubscriptionsMngr::CsSipNotificationMsg - "
			<< "Unknown subscription notification: "
			<< "From <" << pszFrom << ">, To <" << pszTo << ">";
	}

	DumpList(eLevelError);

	return status;
}


/////////////////////////////////////////////////////////////////////////////
void CEpSimSipSubscriptionsMngr::DumpList(const DWORD traceLevel /*=DEBUG*/) const
{
	CLargeString  traceString;
	traceString << " CEpSimSipSubscriptionsMngr::DumpList - List of available subscriptions: \n"
				<< "   Index     ConfName      SubscriberName      Event \n"
				<< "  ---------------------------------------------------------------------\n";
	for( int i=0; i<MAX_SIP_SIBSCRIPTIONS; i++ )
	{
		if( NULL != m_paSubscriptions[i] )
			traceString << "   "
						<< i << "      "
						<< m_paSubscriptions[i]->GetConfName() << "   "
						<< m_paSubscriptions[i]->GetSubscriberName() << "   "
						<< m_paSubscriptions[i]->GetEvent() << "\n";
	}
	TRACESTR(traceLevel) << traceString.GetString();
	perror(traceString.GetString());
}

/////////////////////////////////////////////////////////////////////////////
void CEpSimSipSubscriptionsMngr::SendSipSubscribeInd(const CEpSimSipSubscription* pSubscr,const WORD expires) const // SendSipSubscribeInd(m_paSubscriptions[i])
{
	PTRACE(eLevelInfoNormal,"CEpSimSipSubscriptionsMngr::SendSipSubscribeInd - SIP_CS_SIG_SUBSCRIBE_IND ");

		// get sip headers
	BYTE* pHeadersBuffer = NULL;
	WORD  nHeadersLen = CreateSipHeadersBuffer(&pHeadersBuffer,STATUS_OK,
				pSubscr->GetConfName(),pSubscr->GetSubscriberName(),pSubscr->GetEvent());

	WORD   indLen    = sizeof(mcIndSubscribe) + nHeadersLen;
	BYTE*  pIndArray = new BYTE [indLen];
	memset(pIndArray,0,indLen);

	mcIndSubscribe*  pInd = (mcIndSubscribe*)pIndArray;

	pInd->expires = expires;

	pInd->transportAddress.transAddr.addr.v4.ip = 0x46be16ac; //172.22.190.70
	pInd->transportAddress.transAddr.port       = 0xc020;
	pInd->transportAddress.transAddr.ipVersion     = eIpVersion4;
	pInd->transportAddress.transAddr.distribution  = 0;
	pInd->transportAddress.transAddr.transportType = eTransportTypeUdp;
	pInd->transportAddress.unionProps.unionType    = eIpVersion4;
	pInd->transportAddress.unionProps.unionSize    = sizeof(ipAddressIf);

	BYTE* sipHeaders = ((BYTE*)&(pInd->sipHeaders));
	memcpy(sipHeaders,pHeadersBuffer,nHeadersLen);

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;

	pCSProt->AddPortDescriptionHeader(DUMMY_PARTY_ID,DUMMY_CONF_ID,DUMMY_CONNECTION_ID,ePhysical_art_light);
	pCSProt->AddCSHeader(5,0,eCentral_signaling);

	::FillCsProtocol(pCSProt, CCSIDTaskApi::GetCSIDFromTaskApi(m_pCsApi),
                     SIP_CS_SIG_SUBSCRIBE_IND, pIndArray, indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

	if( TRUE == CPObject::IsValidPObjectPtr(m_pCsApi) )
		m_pCsApi->SendMsg(pMsg,SEND_TO_CSAPI);
	else
		DBGPASSERT(1);

	POBJDELETE(pCSProt);
	PDELETEA(pIndArray);
}

/////////////////////////////////////////////////////////////////////////////
DWORD CEpSimSipSubscriptionsMngr::CreateSipHeadersBuffer(BYTE** ppHeadersBuffer,const DWORD rejectStatus,
			const char* pszConfName,const char* pszSubscriberName, const char* pszEvent) const
{
	const int NUM_SIP_HEADERS = 10; // change carefully!!! check indexes in aHeaderElements[] and pszArray[]

	WORD   nStringLen = 0; // len of string
	ALLOCBUFFER(pszStringList,4096);
	memset(pszStringList,0,4096);

	sipHeaderElement aHeaderElements[NUM_SIP_HEADERS];

	aHeaderElements[0].eHeaderField	= kToTag;   // 0x1;
	aHeaderElements[1].eHeaderField	= kTo;          // 0x2;
	aHeaderElements[2].eHeaderField	= kFromDisplay; // 0x4;
	aHeaderElements[3].eHeaderField	= kFrom;        // 0x5;
	aHeaderElements[4].eHeaderField	= kContactDisplay; // 0x7;
	aHeaderElements[5].eHeaderField	= kContact;     // 0x8;
	aHeaderElements[6].eHeaderField	= kReqLine;     // 0xa;
	aHeaderElements[7].eHeaderField	= kCallId;      // 0x13;
	aHeaderElements[8].eHeaderField	= kEvent;       // 0x14;
	aHeaderElements[9].eHeaderField	= kFromTag;   // 0xb;

	char* pszArray[NUM_SIP_HEADERS];

	for( int i=0; i<NUM_SIP_HEADERS; i++ )
	{
		pszArray[i] = new char[128];
		memset(pszArray[i],'\0',128);
	}

	sprintf(pszArray[0],"%s","tag=67890");  // kTo
	sprintf(pszArray[1],"%s",pszConfName);  // kTo
	sprintf(pszArray[2],"%s",pszSubscriberName); // kFromDisplay
	sprintf(pszArray[3],"%s",pszSubscriberName); // kFrom
	sprintf(pszArray[5],"%s",pszSubscriberName); // kContact
	sprintf(pszArray[6],"%s",pszConfName);  // kReqLine
	sprintf(pszArray[8],"%s",pszEvent);  // kEvent
	strcpy(pszArray[9],"tag=12345");  // kFromTag

	WORD position = 0;
	for( int i=0; i<NUM_SIP_HEADERS; i++ )
	{
		aHeaderElements[i].flags	 = 0x0;
		aHeaderElements[i].position  = position;

		strcpy( pszStringList+position, pszArray[i] );
		position += strlen(pszArray[i]) + 1;
	}
	nStringLen = position + 1;

	for( int i=0; i<NUM_SIP_HEADERS; i++ )
		DEALLOCBUFFER(pszArray[i]);

	sipMessageHeadersBase  tHeadersBase;
	tHeadersBase.numOfHeaders = NUM_SIP_HEADERS;
	tHeadersBase.headersListLength = NUM_SIP_HEADERS * sizeof(sipHeaderElement) + nStringLen;

		// create result buffer
	DWORD nBufferLen = sizeof(sipMessageHeadersBase)
					 + NUM_SIP_HEADERS * sizeof(sipHeaderElement)
					 + nStringLen;

	*ppHeadersBuffer = new BYTE[nBufferLen];
		// copy from temporary buffer to result
		//  a. sipMessageHeaderBase
	memcpy(*ppHeadersBuffer,&tHeadersBase,sizeof(sipMessageHeadersBase));
		//  b. 10 sipHeaderElements
	for( int i=0; i<NUM_SIP_HEADERS; i++ )
	{
		BYTE*  pPtr = *ppHeadersBuffer + sizeof(sipMessageHeadersBase) + i*sizeof(sipHeaderElement);
		memcpy(pPtr,&(aHeaderElements[i]),sizeof(sipHeaderElement));
	}
		//  c. string list
	BYTE*  pPtr = *ppHeadersBuffer + sizeof(sipMessageHeadersBase) + NUM_SIP_HEADERS*sizeof(sipHeaderElement);
	memcpy(pPtr,pszStringList,nStringLen);

	DEALLOCBUFFER(pszStringList);

	return nBufferLen;
}


/////////////////////////////////////////////////////////////////////////////
//    CEpSimSipSubscription
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CEpSimSipSubscription::CEpSimSipSubscription()
{
	memset(m_szConfName,0,H243_NAME_LEN);
	memset(m_szSubscriberName,0,H243_NAME_LEN);
	memset(m_szEvent,0,H243_NAME_LEN);

	m_pszSubscrLastNotif = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CEpSimSipSubscription::CEpSimSipSubscription(const char* pszConfName,const char* pszSubscriberName,const char* pszEvent)
{
	strncpy(m_szConfName,pszConfName,sizeof(m_szConfName) - 1);
	m_szConfName[sizeof(m_szConfName) - 1] = '\0';

	strncpy(m_szSubscriberName,pszSubscriberName,sizeof(m_szSubscriberName) - 1);
	m_szSubscriberName[sizeof(m_szSubscriberName) - 1] ='\0';

	strncpy(m_szEvent,pszEvent,sizeof(m_szEvent) - 1);
	m_szEvent[sizeof(m_szEvent) - 1] = '\0';

	m_pszSubscrLastNotif = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CEpSimSipSubscription::~CEpSimSipSubscription()
{
	memset(m_szConfName,0,H243_NAME_LEN);
	memset(m_szSubscriberName,0,H243_NAME_LEN);
	memset(m_szEvent,0,H243_NAME_LEN);

	PDELETEA(m_pszSubscrLastNotif);
}

/////////////////////////////////////////////////////////////////////////////
void CEpSimSipSubscription::SetLastNotification(const char* pszNotification)
{
	PDELETEA(m_pszSubscrLastNotif);

	if( NULL == pszNotification )
		return;

	DWORD len = strlen(pszNotification);
	if( len > 0 && len < 10000 )
	{
		m_pszSubscrLastNotif = new char[len+1];
		strncpy(m_pszSubscrLastNotif,pszNotification,len);
		m_pszSubscrLastNotif[len] = '\0';
	}
}












