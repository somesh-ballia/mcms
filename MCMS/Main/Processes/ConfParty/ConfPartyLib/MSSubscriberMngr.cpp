/*
 * MSSubscriberMngr.cpp
 *
 *  Created on: Oct 2, 2013
 *      Author: dkrasnopolsky
 */

#include "MSSubscriberMngr.h"
#include "SipDefinitions.h" //enGeneralSubscribeOpcode=SubscribeLyncEventPackage
#include "SipUtils.h"
#include "IpCsOpcodes.h"
#include "EventPackage.h"
#include "SysConfigKeys.h"
#include "Tokenizer.h"

#define DUMPSTR(str) (((str != 0) && (str[0] != '\0')) ? str : "NA")

extern CIpServiceListManager* GetIpServiceListMngr();

PBEGIN_MESSAGE_MAP(CMSSubscriberMngr)
	ONEVENT(PARTYCONNECTTOUT       , ANYCASE          , CMSSubscriberMngr::OnTimerSubscribe)
	ONEVENT(SIP_CS_SIG_BENOTIFY_IND, sMS_DISCONNECTING, CMSSubscriberMngr::OnSipBeNotifyDisconnect)
	ONEVENT(SIP_CS_SIG_BENOTIFY_IND, ANYCASE          , CMSSubscriberMngr::OnSipBeNotify)

	ONEVENT(MSMNGRDISCONNECTTOUT 	,sMS_DISCONNECTING,		CMSSubscriberMngr::OnTimerDisconnectMngr)

PEND_MESSAGE_MAP(CMSSubscriberMngr, CMSAvMCUMngr);

////////////////////////////////////////////////////////////////////////////
//                        CMSSubscriberMngr
////////////////////////////////////////////////////////////////////////////
CMSSubscriberMngr::CMSSubscriberMngr()
{
}

//--------------------------------------------------------------------------
CMSSubscriberMngr::~CMSSubscriberMngr()
{
}

//--------------------------------------------------------------------------

CMSSubscriberMngr::CMSSubscriberMngr(const CMSSubscriberMngr &other) : CMSAvMCUMngr(other)
{

}

//--------------------------------------------------------------------------
void CMSSubscriberMngr::Create(CRsrcParams* pRsrcParams, CConf* pConf, sipSdpAndHeadersSt* MsConfReq, DWORD PartyId, CSipNetSetup* SipNetSetup, DWORD ServiceId, char* focusUri)
{
	TRACEINTO << "FocusUri:" << focusUri;

	CMSAvMCUMngr::Create(pRsrcParams, pConf, SipNetSetup, ServiceId, PartyId);

	SetFocusUri(focusUri);

	BuildToAddress(PartyId);

	SendSubscribe(SipNetSetup,FALSE);
}

//--------------------------------------------------------------------------
void CMSSubscriberMngr::SendSubscribe(CSipNetSetup* SipNetSetup, BYTE ToStopSubscribe)
{
	m_state = sMS_CONNECTING;

	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
	PASSERTSTREAM_AND_RETURN(!pServiceParams, "ServiceId:" << m_serviceId << " - IP service does not exist");

	DWORD sipProxyStatus      = pServiceParams->GetSipProxyStatus();
	DWORD outBoundProxyStatus = STATUS_OK;
	bool  isDialDirect        = false;
	bool  isUriWithIp         = false;
	int   lenHostIp           = 0;
	char* strHostIp           = NULL;

	char strTransportIp[IPV6_ADDRESS_LEN];
	char strAltTaDestPartyAddr[IPV6_ADDRESS_LEN];
	char strTaDestPartyAddr[IPV6_ADDRESS_LEN];
	char strProxyAddress[MaxLengthOfSingleUrl];
	const char* strOriginalToFromDma = SipNetSetup->GetOriginalToDmaSipAddress();
	memset(strTransportIp, 0, sizeof(strTransportIp));
	memset(strAltTaDestPartyAddr, 0, sizeof(strAltTaDestPartyAddr));
	memset(strTaDestPartyAddr, 0, sizeof(strTaDestPartyAddr));
	memset(strProxyAddress, 0, sizeof(strProxyAddress));

	const char* strDestPartyAddr = SipNetSetup->GetDestPartyAddress();
	const mcTransportAddress* pDestTaAddr = SipNetSetup->GetTaDestPartyAddr();

	ipToString(*pDestTaAddr, strTaDestPartyAddr, 1);

	strcpy_safe(strAltTaDestPartyAddr, SipNetSetup->GetAlternativeTaDestPartyAddr());

	TRACEINTO << "StrTaDestPartyAddr:" << DUMPSTR(strTaDestPartyAddr) << ", StrAltTaDestPartyAddr:" << DUMPSTR(strAltTaDestPartyAddr) << ", StrDestPartyAddr:" << DUMPSTR(strDestPartyAddr);

	if (::isApiTaNull(pDestTaAddr) == FALSE || strAltTaDestPartyAddr[0] != '\0')
	{
		isDialDirect = true;
		isUriWithIp  = false;
	}
	else
	{
		isDialDirect = false;

		// check if present @ in dial SIP URI
		char* strAt = (char*)strstr(strDestPartyAddr, "@");

		// if found @ it's dialing via proxy (if defined)
		// otherwise if only valid IP address it's direct dial

		// if found @ check what is after the @
		strHostIp = strAt ? (strAt+1) : NULL;
		lenHostIp = strHostIp ? strlen(strHostIp) : 0;

		TRACEINTO << "StrAt:" << DUMPSTR(strAt) << ", StrHostIp:" << DUMPSTR(strHostIp) << ", LenHostIp:" << lenHostIp;

		if (strHostIp)
		{
			mcTransportAddress trAddr;
			memset(&trAddr, 0, sizeof(trAddr));
			::stringToIp(&trAddr, strHostIp);
			if (!::isApiTaNull(&trAddr) && !::isIpTaNonValid(&trAddr))
				isUriWithIp = true;
		}
	}

	// if the service status is On and its not direct IP call (IP address valid or URI with IP address)
	// we try to get the outbound proxy.

	if (sipProxyStatus != eServerStatusOff && !isDialDirect)
	{
		GetOutboundSipProxy(strProxyAddress);
		strProxyAddress[MaxLengthOfSingleUrl - 1] = '\0';
		if (strProxyAddress[0] == '\0')
			outBoundProxyStatus = STATUS_FAIL;
	}

	if (outBoundProxyStatus != STATUS_OK)
	{
		PASSERTSTREAM(1, "Failed, No outbound proxy, need to close the call");
	}
	else if (sipProxyStatus == eServerStatusOff || isDialDirect || isUriWithIp || strProxyAddress[0] != '\0')
	{
		const char* strToDisplay      = m_ToAddrStr;
		const char* strToAddr         = m_ToAddrStr;
		const char* strFromDisplay    = "";
		const char* strFromAddr;
		const char* strEvent          = "conference";
		const char* strSupported      = "ms-piggyback-first-notify,ms-benotify,com.microsoft.autoextend";
		const char* strContactDisplay = "";
		const char* strAccept         = "application/conference-info+xml";
		char strContact[IP_STRING_LEN];       // user@ip
		char*		strLocalUri			= new char[IP_STRING_LEN];
		FPASSERT_AND_RETURN(!strLocalUri);
		SipNetSetup->CopyLocalUriToBuffer(strLocalUri,IP_STRING_LEN); // user@domain (if we have domain)


		if (strOriginalToFromDma && strOriginalToFromDma[0])
		{
			strFromAddr = strOriginalToFromDma;
			if (strLocalUri && strLocalUri[0])
			{
				snprintf(strContact,IP_STRING_LEN,"%s",strLocalUri);
			}
			else
			{
				snprintf(strContact,IP_STRING_LEN,"%s",SipNetSetup->GetSrcPartyAddress());
			}
		}
		else if (strLocalUri[0])
		{
			strFromAddr = strLocalUri;
			snprintf(strContact,IP_STRING_LEN,"%s",strFromAddr);
		}
		else
		{
			strFromAddr = SipNetSetup->GetSrcPartyAddress();
			snprintf(strContact,IP_STRING_LEN,"%s",strFromAddr);
		}

		TRACEINTO
			<< "\n  StrToDisplay   :" << DUMPSTR(strToDisplay)
			<< "\n  StrToAddr      :" << DUMPSTR(strToAddr)
			<< "\n  StrFromDisplay :" << DUMPSTR(strFromDisplay)
			<< "\n  StrFromAddr    :" << DUMPSTR(strFromAddr)
			<< "\n  StrContact     :" << DUMPSTR(strContact)
			<< "\n  StrDestAddr    :" << DUMPSTR(strDestPartyAddr)
			<< "\n  StrEvent       :" << DUMPSTR(strEvent);

		CSipHeaderList headerList(MIN_ALLOC_HEADERS*2, 9,
			(int)kToDisplay, strlen(strToDisplay), strToDisplay,
			(int)kTo, strlen(strToAddr), strToAddr,
			(int)kFromDisplay, strlen(strFromDisplay), strFromDisplay,
			(int)kFrom, strlen(strFromAddr), strFromAddr,
			(int)kEvent, strlen(strEvent), strEvent,
			(int)kSupported, strlen(strSupported), strSupported,
			(int)kAccept, strlen(strAccept), strAccept,
			(int)kContactDisplay, strlen(strContactDisplay), strContactDisplay,
			(int)kContact, strlen(strContact), strContact);

		size_t lenHeader    = headerList.GetTotalLen();
		size_t lenSubscribe = sizeof(mcReqSubscribe)+lenHeader;

		mcReqSubscribe* reqSubscribe = (mcReqSubscribe*)new BYTE[lenSubscribe];

		reqSubscribe->id = 0;
		reqSubscribe->expires = (ToStopSubscribe) ? 0 : -1;
		reqSubscribe->subOpcode = SubscribeLyncEventPackage;


		// remote port will NOT be use if the call is trough an outbound proxy
		reqSubscribe->transportAddr.transAddr.port = SipNetSetup->GetRemoteSignallingPort();

		// invite message struct may be filled up with either an ip address
		// or a domain name. domain name, requires a dns lookup by CS.
		bool isUserDialWithIP = true; // no dns lookup is needed
		if (isDialDirect)
		{
			TRACEINTO<<"is dial direct is true";
			if (strAltTaDestPartyAddr[0] != '\0')
			{
				mcTransportAddress trAddr;
				memset(&trAddr, 0, sizeof(trAddr));
				::stringToIp(&trAddr, strAltTaDestPartyAddr);
				if (!::isApiTaNull(&trAddr) && !::isIpTaNonValid(&trAddr))
				{
					strcpy_safe(strTransportIp, strAltTaDestPartyAddr);
				}
				else // dns lookup(by CS) is needed
				{
					isUserDialWithIP = false;
				}
			}
			else
			{
				::ipToString(*pDestTaAddr, strTransportIp, 1);
			}
			TRACEINTO << "StrTransportIp:" << strTransportIp << " - (Dial direct)";
		}
		else if (strProxyAddress[0] != '\0')
		{
			// if uri contains the proxy ip address or contains registrar domain name
			// or contains a format of "user@domain"
			if ((isUriWithIp && strcmp(strHostIp, strProxyAddress) == 0) ||
			    (isUriWithIp && strcmp(strHostIp, SipNetSetup->GetLocalHost()) == 0) || !isUriWithIp)
			{
				reqSubscribe->transportAddr.transAddr.port = 0;   // User Dial With Proxy, card will take port from proxy's service
				mcTransportAddress trAddr;
				memset(&trAddr, 0, sizeof(trAddr));
				::stringToIp(&trAddr, strProxyAddress);
				if (!::isApiTaNull(&trAddr) && !::isIpTaNonValid(&trAddr))
				{
					strcpy_safe(strTransportIp, strProxyAddress);
				}
				else // dns lookup(by CS) is needed
				{
					isUserDialWithIP = false;
					TRACEINTO<<"send outbound proxy by name";
					memcpy(reqSubscribe->domainName,strProxyAddress,MaxLengthOfSingleUrl);


				}
			}
			else if (isUriWithIp) // uri with a none proxy/registrar ip address
			{
				// direct dial
				strcpy_safe(strTransportIp, strHostIp);
			}
		}
		// no proxy is definedreqSubscribe but ip with uri exists
		else if (isUriWithIp)
		{
			strcpy_safe(strTransportIp, strHostIp);
		}
		else
			strTransportIp[0] = 0;

		if (isUserDialWithIP)
		{
			mcTransportAddress trAddr;
			memset(&trAddr, 0, sizeof(trAddr));
			::stringToIp(&trAddr, strTransportIp);
			reqSubscribe->transportAddr.transAddr.addr.v4.ip = trAddr.addr.v4.ip;
			TRACEINTO << "StrTransportIp:" << DUMPSTR(strTransportIp);
		}

		reqSubscribe->transportAddr.transAddr.transportType = eTransportTypeTls;
		reqSubscribe->transportAddr.transAddr.addr.v4.ip    = 0x0; // modified for ANAT
		reqSubscribe->transportAddr.transAddr.port          = 0;
		reqSubscribe->transportAddr.transAddr.ipVersion     = eIpVersion4;
		reqSubscribe->transportAddr.transAddr.distribution  = 0;
		reqSubscribe->transportAddr.transAddr.transportType = eTransportTypeTls;
		reqSubscribe->transportAddr.unionProps.unionType    = eIpVersion4;
		reqSubscribe->transportAddr.unionProps.unionSize    = sizeof(ipAddressIf);

		headerList.BuildMessage(&reqSubscribe->sipHeaders);

		SendSIPMsgToCS(SIP_CS_PROXY_SUBSCRIBE_REQ, reqSubscribe, lenSubscribe);

		delete[] reqSubscribe;
	}
}

//--------------------------------------------------------------------------
void CMSSubscriberMngr::OnSipBeNotify(CSegment* pSeg)
{
	if (IsValidTimer(PARTYCONNECTTOUT))
		DeleteTimer(PARTYCONNECTTOUT);

	TRACEINTO << "PartyId:" << m_PartyId;

	STATUS status = ProcessBeNotify(pSeg, m_PartyId);

	//-S- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------/
	if(STATUS_IN_PROGRESS == status)
	{
		FTRACEINTO << "BRIDGE-13277.[Add. MCMS]. " << "STATUS_IN_PROGRESS";
		return;
	}
	//-E- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------/

	PASSERTSTREAM(status != STATUS_OK, "status:" << status);

	if (status == STATUS_OK)
	{
		status = STATUS_FAIL;

		const EventPackage::Conference* pConference = EventPackage::Manager::Instance().GetConference(m_PartyId);
		PASSERT(!pConference);

		if (pConference)
		{
			const EventPackage::Uris::UrisContainer& uris = pConference->m_conferenceDescription.m_confUris.m_uris;
			EventPackage::Uris::UrisContainer::const_iterator _iuri = std::find_if(uris.begin(), uris.end(), std::bind2nd(EventPackage::Predicate::Uri_Purpose(), to_tag(EventPackage::eUriPurposeType_AudioVideo)));
			if (_iuri != uris.end())
			{
				TRACEINTO << "PartyId:" << m_PartyId << ", Uri:" << _iuri->m_entity.c_str();
				m_pTaskApi->MSSubscriberEndConnection(m_PartyId, STATUS_OK, TRUE, _iuri->m_entity.c_str());
				return;
			}

		}
	}
	TRACEINTO << "PartyId:" << m_PartyId << ", Uri:NA";
	m_pTaskApi->MSSubscriberEndConnection(m_PartyId, status, TRUE, NULL);
}
//-S- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------/
//--------------------------------------------------------------------------
//STATUS CMSSubscriberMngr::ProcessBeNotify(CSegment* pSeg, PartyRsrcID id)
//{
//	APIU32 callIndex      = 0;
//	APIU32 channelIndex   = 0;
//	APIU32 mcChannelIndex = 0;
//	APIU32 stat1          = 0;
//	APIU16 srcUnitId      = 0;
//	STATUS status 		 = STATUS_OK;
//
//
//	*pSeg >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;
//
//	FTRACEINTO
//		<< "PartyId: " << id
//		<< ", callIndex:" << callIndex
//		<< ", channelIndex:" << channelIndex
//		<< ", mcChannelIndex:" << mcChannelIndex
//		<< ", stat1:" << stat1
//		<< ", srcUnitId:" << srcUnitId;
//	mcIndBenotify* pIndBenotify = (mcIndBenotify*)pSeg->GetPtr(true);
//	FPASSERT_AND_RETURN_VALUE(!pIndBenotify, STATUS_INCONSISTENT_PARAMETERS);
//
//	sipContentAndHeaders* pSipContentAndHeaders = (sipContentAndHeaders*)&pIndBenotify->sipContentAndHeaders;
//	FPASSERT_AND_RETURN_VALUE(!pSipContentAndHeaders, STATUS_INCONSISTENT_PARAMETERS);
//
//	DWORD Expire = pIndBenotify->expires;
//
//	if(Expire)
//	{
//
//		size_t xmlLen = pSipContentAndHeaders->sipHeadersOffset;
//		size_t headerLen = pSipContentAndHeaders->lenOfDynamicSection-pSipContentAndHeaders->sipHeadersOffset;
//
//		FTRACEINTO
//			<< "xmlLen:" << xmlLen
//			<< ", headerLen:" << headerLen
//			<< ", sipHeadersOffset:" << pSipContentAndHeaders->sipHeadersOffset
//			<< ", lenOfDynamicSection:" << pSipContentAndHeaders->lenOfDynamicSection
//			<< ", Expire:" << Expire;
//
//		pSeg->ResetRead(pSeg->GetRdOffset()+sizeof(pSipContentAndHeaders->sipHeadersOffset)+sizeof(pSipContentAndHeaders->lenOfDynamicSection)+sizeof(pIndBenotify->expires));
//
//		const char* xmlBuf = (const char*)pSeg->GetPtr(true);
//
//		pSeg->ResetRead(pSeg->GetRdOffset()+xmlLen);
//
//		CSipBeNotifyStruct sipBeNotify;
//		sipBeNotify.ReadHeaders((BYTE*)pSeg->GetPtr(true), headerLen);
//
//		const char* strFrom   = sipBeNotify.GetHeaderField(kFrom);
//		const char* strTo     = sipBeNotify.GetHeaderField(kTo);
//		const char* strCallId = sipBeNotify.GetHeaderField(kCallId);
//
//		FTRACEINTO << "strFrom:" << strFrom << ", strTo:" << strTo << ", strCallId:" << strCallId << ", xml:\n" << CLexeme(xmlBuf, xmlLen);
//
//		status =  EventPackage::Manager::Instance().AddConference(id, strCallId, xmlBuf, xmlLen);
//
//	}
//	return status;
//}

typedef struct _sCOLLECT_BUFF
{
	char *			pBuff        ;
	unsigned int	dwStaticIndex;

}
COLLECT_BUFF;

#define MAX_CS_ID_INDEX 10 
#define	PART_OPC_BENOTIFY_ID_CS	 0x1000

static COLLECT_BUFF  G_CollectBuffNotify[MAX_CS_ID_INDEX]=
{
	{NULL, 0}
  , {NULL, 0}
  , {NULL, 0}
  , {NULL, 0}
  , {NULL, 0}
  , {NULL, 0}
  , {NULL, 0}
  , {NULL, 0}
  , {NULL, 0}
  , {NULL, 0}
};

STATUS CMSSubscriberMngr::ProcessBeNotify(CSegment* pSeg, PartyRsrcID id)
{
	APIU32 callIndex      = 0;
	APIU32 channelIndex   = 0;
	APIU32 mcChannelIndex = 0;
	APIU32 stat1          = 0;
	APIU16 srcUnitId      = 0;
	STATUS status 		 = STATUS_OK;
	unsigned short         wCsID = 0;
	BOOL                   bIsNeedDelete = FALSE;
	mcIndBenotify_Part   * pIndBenotifyCompleted = NULL;

	*pSeg >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

	mcIndBenotify_Part* pIndBenotify = (mcIndBenotify_Part*)pSeg->GetPtr(true);
	if (pIndBenotify)
	{
		wCsID = pIndBenotify->sipContentAndHeadersPart.sMessagePartDescr.wCsId;
		FTRACEINTO << "BENOTIFY_IND_MCMS CS" << pIndBenotify->sipContentAndHeadersPart.sMessagePartDescr.wCsId;
	}


	if((NULL != pIndBenotify)&&(MAX_CS_ID_INDEX > pIndBenotify->sipContentAndHeadersPart.sMessagePartDescr.wCsId))
	{
		if(1 == pIndBenotify->sipContentAndHeadersPart.sMessagePartDescr.wPartSeq)
		{
			if(NULL != G_CollectBuffNotify[wCsID].pBuff)
				delete [] G_CollectBuffNotify[wCsID].pBuff;
			G_CollectBuffNotify[wCsID].pBuff = NULL;
			G_CollectBuffNotify[wCsID].dwStaticIndex = 0; 
		}

		if(NULL == G_CollectBuffNotify[wCsID].pBuff)
		{
			int nTotalBuffLen = sizeof(mcIndBenotify_Part) + pIndBenotify->sipContentAndHeadersPart.sMessagePartDescr.dwFullMessLen;
			G_CollectBuffNotify[wCsID].pBuff = new char[nTotalBuffLen + 16];
			memset(G_CollectBuffNotify[wCsID].pBuff, 0, nTotalBuffLen + 16);
			G_CollectBuffNotify[wCsID].dwStaticIndex = 0;

			if(NULL == G_CollectBuffNotify[wCsID].pBuff)
			{
				FPASSERT_AND_RETURN_VALUE(!pIndBenotify, STATUS_INCONSISTENT_PARAMETERS);
			}
		}

		if(NULL != G_CollectBuffNotify[wCsID].pBuff)
		{
			mcIndBenotify_Part * pTemp = (mcIndBenotify_Part*) G_CollectBuffNotify[wCsID].pBuff;

			pTemp->expires = pIndBenotify->expires;
			pTemp->sipContentAndHeadersPart.lenOfDynamicSection = pIndBenotify->sipContentAndHeadersPart.lenOfDynamicSection;
			pTemp->sipContentAndHeadersPart.sipHeadersOffset    = pIndBenotify->sipContentAndHeadersPart.sipHeadersOffset;
			    pTemp->sipContentAndHeadersPart.sMessagePartDescr.dwFullMessLen  = pIndBenotify->sipContentAndHeadersPart.sMessagePartDescr.dwFullMessLen; 
				pTemp->sipContentAndHeadersPart.sMessagePartDescr.wPartLen       = pIndBenotify->sipContentAndHeadersPart.sMessagePartDescr.wPartLen;
				pTemp->sipContentAndHeadersPart.sMessagePartDescr.wCsId		     = pIndBenotify->sipContentAndHeadersPart.sMessagePartDescr.wCsId;		    
				pTemp->sipContentAndHeadersPart.sMessagePartDescr.wOpcMessSeq    = pIndBenotify->sipContentAndHeadersPart.sMessagePartDescr.wOpcMessSeq;   
				pTemp->sipContentAndHeadersPart.sMessagePartDescr.wPartSeq       = pIndBenotify->sipContentAndHeadersPart.sMessagePartDescr.wPartSeq;      
				pTemp->sipContentAndHeadersPart.sMessagePartDescr.wNumberOfParts = pIndBenotify->sipContentAndHeadersPart.sMessagePartDescr.wNumberOfParts;
				pTemp->sipContentAndHeadersPart.sMessagePartDescr.aReserve[0]    = pIndBenotify->sipContentAndHeadersPart.sMessagePartDescr.aReserve[0];   
				pTemp->sipContentAndHeadersPart.sMessagePartDescr.aReserve[1]    = pIndBenotify->sipContentAndHeadersPart.sMessagePartDescr.aReserve[1];   

			memcpy(&(pTemp->sipContentAndHeadersPart.contentAndHeaders[G_CollectBuffNotify[wCsID].dwStaticIndex])
			, &pIndBenotify->sipContentAndHeadersPart.contentAndHeaders[0]
				, pIndBenotify->sipContentAndHeadersPart.sMessagePartDescr.wPartLen);

			G_CollectBuffNotify[wCsID].dwStaticIndex += pIndBenotify->sipContentAndHeadersPart.lenOfDynamicSection;
			unsigned short wOpcMessSeq = pTemp->sipContentAndHeadersPart.sMessagePartDescr.wOpcMessSeq - PART_OPC_BENOTIFY_ID_CS;

			FTRACEINTO << "  BENOTIFY_IND_MCMS [CS" << pTemp->sipContentAndHeadersPart.sMessagePartDescr.wCsId
				       << "]. | BENOTIFY_IND Sq.:#" << wOpcMessSeq
				       << " ["<<pTemp->sipContentAndHeadersPart.sMessagePartDescr.wPartSeq
					   << "/"<<pTemp->sipContentAndHeadersPart.sMessagePartDescr.wNumberOfParts
					   << " | DataLen: ["<<pTemp->sipContentAndHeadersPart.sMessagePartDescr.wPartLen
					   << "/" << pTemp->sipContentAndHeadersPart.sMessagePartDescr.dwFullMessLen 
					   << "]";

			if(G_CollectBuffNotify[wCsID].dwStaticIndex < pIndBenotify->sipContentAndHeadersPart.sMessagePartDescr.dwFullMessLen)
			{
				return STATUS_IN_PROGRESS;
			}
			else
			{
				pIndBenotifyCompleted = (mcIndBenotify_Part*)G_CollectBuffNotify[wCsID].pBuff;
				bIsNeedDelete = TRUE;
			}
		}	
	}
	else
	{
		FTRACEINTO << "BENOTIFY_IND_MCMS. INCOM PARAMS - ERROR = NULL!!!!";
		FPASSERT_AND_RETURN_VALUE(!pIndBenotify, STATUS_INCONSISTENT_PARAMETERS);
	}

	if (pIndBenotifyCompleted)
	{
		sipContentAndHeadersSt_Part* pSipContentAndHeaders = (sipContentAndHeadersSt_Part*)&pIndBenotifyCompleted->sipContentAndHeadersPart;
		FPASSERT_AND_RETURN_VALUE(!pSipContentAndHeaders, STATUS_INCONSISTENT_PARAMETERS);

		DWORD Expire = pIndBenotifyCompleted->expires;

		if(Expire)
		{
			size_t xmlLen = pSipContentAndHeaders->sipHeadersOffset;
			size_t headerLen = pSipContentAndHeaders->sMessagePartDescr.dwFullMessLen - pSipContentAndHeaders->sipHeadersOffset;

			pSipContentAndHeaders->lenOfDynamicSection = pSipContentAndHeaders->sMessagePartDescr.dwFullMessLen;

			pSeg->ResetRead(pSeg->GetRdOffset()
							+sizeof(pSipContentAndHeaders->sipHeadersOffset)
							+sizeof(sPART_MESSAGE_DECR)
							+sizeof(pSipContentAndHeaders->lenOfDynamicSection)
							+sizeof(pIndBenotify->expires));

			const char* xmlBuf = (const char*)&pSipContentAndHeaders->contentAndHeaders[0];

			CSipBeNotifyStruct sipBeNotify;
			sipBeNotify.ReadHeaders((BYTE*)&(pSipContentAndHeaders->contentAndHeaders[pSipContentAndHeaders->sipHeadersOffset]), headerLen);

			const char* strFrom   = sipBeNotify.GetHeaderField(kFrom);
			const char* strTo     = sipBeNotify.GetHeaderField(kTo);
			const char* strCallId = sipBeNotify.GetHeaderField(kCallId);

			FTRACEINTO << "BENOTIFY_IND_MCMS:  strFrom:"
						 << strFrom << ", strTo:"
							  << strTo << ", strCallId:"
									<< strCallId << ", xml:\n" << CLexeme(xmlBuf, xmlLen);

			status =  EventPackage::Manager::Instance().AddConference(id, strCallId, xmlBuf, xmlLen);
		}
	}

	if(TRUE == bIsNeedDelete)
	{
		if(NULL != G_CollectBuffNotify[wCsID].pBuff)
			delete[] G_CollectBuffNotify[wCsID].pBuff;
		G_CollectBuffNotify[wCsID].pBuff = NULL;
		G_CollectBuffNotify[wCsID].dwStaticIndex = 0;
		FTRACEINTO << "BENOTIFY_IND_MCMS CS"<< wCsID << " Buffer has released.";
	}

	return status;
}
//-E- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------/

//--------------------------------------------------------------------------
void CMSSubscriberMngr::OnTimerSubscribe(CSegment* pParam)
{
	TRACEINTO << "Time is out";

	if (IsValidTimer(PARTYCONNECTTOUT))
		DeleteTimer(PARTYCONNECTTOUT);

	m_pTaskApi->MSSubscriberEndConnection(m_PartyId, STATUS_FAIL, TRUE, NULL);
}
//--------------------------------------------------------------------------
void CMSSubscriberMngr::TerminateEventPackageConnection(CSipNetSetup* SipNetSetup)
{
	PTRACE2INT(eLevelInfoNormal,"CMSSubscriberMngr::TerminateEventPackageConnection  m_state1:",m_state);
	SendSubscribe(SipNetSetup,TRUE);

	StartTimer(MSMNGRDISCONNECTTOUT,  SECOND);

	m_state = sMS_DISCONNECTING;


}
//--------------------------------------------------------------------------
void CMSSubscriberMngr::RemoveEventPackageConnection()
{
	TRACEINTO;
	m_pTaskApi->MSSubscriberEndDisconnection(m_PartyId);

	RemoveFromRsrcTbl();

	m_state = sMS_DISCONNECTED;
}

//--------------------------------------------------------------------------
void CMSSubscriberMngr::OnSipBeNotifyDisconnect(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CMSSubscriberMngr::OnSipBeNotifyDisconnect");
	//TRACEINTO;

	if (IsValidTimer(MSMNGRDISCONNECTTOUT))
		DeleteTimer(MSMNGRDISCONNECTTOUT);

	bool Isdel =  EventPackage::Manager::Instance().DelConference(m_PartyId);

	RemoveEventPackageConnection();
}
//--------------------------------------------------------------------------
void CMSSubscriberMngr::OnTimerDisconnectMngr(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CMSSubscriberMngr::OnTimerDisconnectCall");

	bool Isdel =  EventPackage::Manager::Instance().DelConference(m_PartyId);

	RemoveEventPackageConnection();
}
