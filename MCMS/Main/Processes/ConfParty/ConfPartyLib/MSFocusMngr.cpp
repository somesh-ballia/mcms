//+========================================================================+
//                     MSFocusMngr.cpp                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MSFocusMngr.cpp	                                       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | Sept-2013  |                                                      |
//+========================================================================+


#include "MSFocusMngr.h"
#include "EventPackage.h"
#include "IpCsOpcodes.h"


extern void DumpXMLToStream(std::ostream &ostr, sipSdpAndHeadersSt* sdpAndHeaders);
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );
extern CIpServiceListManager* GetIpServiceListMngr();


////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CMSFocusMngr)



ONEVENT(SIP_CS_CCCP_SIG_ADD_USER_INVITE_RESPONSE_IND 				,sMS_CONNECTING, 	 CMSFocusMngr::OnSipInviteResponseConnecting)
ONEVENT(ADDUSERCONNECTTOUT	  				                        ,sMS_CONNECTING,     CMSFocusMngr::OnTimerAddUserConnect)
ONEVENT(SIP_CS_SIG_PROV_RESPONSE_IND								,sMS_CONNECTING,     CMSFocusMngr::OnCsProvisunalResponseInd)
ONEVENT(SIP_CS_SIG_BYE_IND  			 				        	,sMS_CONNECTING,       CMSFocusMngr::OnSipByeIndConnecting)


ONEVENT(SIP_CS_SIG_BYE_200_OK_IND									,sMS_DISCONNECTING,		CMSFocusMngr::OnSipBye200OkInd)
//ONEVENT(SIP_CS_SIG_BYE_200_OK_IND									,sMS_CONNECTED,		    CMSFocusMngr::OnSipBye200OkInd)  // To check
ONEVENT(MSMNGRDISCONNECTTOUT 									 	,sMS_DISCONNECTING,		CMSFocusMngr::OnTimerDisconnectMngr)

ONEVENT(SIP_CS_SIG_BYE_IND  			 				        	,sMS_CONNECTED,       CMSFocusMngr::OnSipByeInd)



PEND_MESSAGE_MAP(CMSFocusMngr,CMSAvMCUMngr);
////////////////////////////////////////////////////////////////////////////////////
CMSFocusMngr::CMSFocusMngr()
{
	//m_MsAddUserInvite           = NULL;
	memset(m_strAVMCUAddress,0,MaxAddressListSize);
	m_MSAddUserResponse = NULL;

}
////////////////////////////////////////////////////////////////////////////////////
CMSFocusMngr::CMSFocusMngr(const CMSFocusMngr &other)
:CMSAvMCUMngr(other)
{
	memset(m_strAVMCUAddress,0,MaxAddressListSize);
	m_MSAddUserResponse = NULL;
}

////////////////////////////////////////////////////////////////////////////////////
CMSFocusMngr::~CMSFocusMngr()
{
	POBJDELETE(m_MSAddUserResponse);
}

//////////////////////////////////////////////////////////////////////////////////
void CMSFocusMngr::Create(CRsrcParams* RsrcParams,CConf* pConf,DWORD PartyId,CSipNetSetup* SipNetSetup,DWORD ServiceId,char* FocusUri,const char* strConfParamInfo)
{

	CMSAvMCUMngr::Create(RsrcParams ,pConf, SipNetSetup, ServiceId,PartyId);

	PTRACE(eLevelInfoNormal, "CMSFocusMngr::Create ");

    int FocusUriLen = strlen(FocusUri);
	m_FocusUri = new char[FocusUriLen+1];
	memset(m_FocusUri, '\0', FocusUriLen);
	strncpy(m_FocusUri, FocusUri, FocusUriLen);
	m_FocusUri[FocusUriLen] = '\0';


/*
   int FocusUriLen = strlen(FocusUri);
   ALLOCBUFFER(m_FocusUri,FocusUriLen);
   strncpy(m_FocusUri,FocusUri,FocusUriLen);
   m_FocusUri[FocusUriLen] = '\0';
*/
   TRACEINTO << "FocusUriLen :" << FocusUriLen;
   TRACEINTO << "m_FocusUri :" << m_FocusUri;

   TRACEINTO << "SrcPartyAddress :" << SipNetSetup->GetSrcPartyAddress();

   BuildToAddress(PartyId);
   BuildAVMCUAddress(SipNetSetup->GetSrcPartyAddress());
	BuildAddUserMsg(SipNetSetup,strConfParamInfo);



}

//////////////////////////////////////////////////////////////////////////////////
void CMSFocusMngr::BuildAddUserMsg(CSipNetSetup* SipNetSetup,const char* strConfParamInfo)
{
	PTRACE(eLevelInfoNormal,"CMSFocusMngr::BuildAddUserMsg");

	m_state = sMS_CONNECTING;
	DWORD outboundProxyIp = 0;
	char buffXml[2048];





		BYTE bMessageSent = NO;
		CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
		CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
		if (pServiceParams == NULL)
		{
			PASSERTMSG(m_pCsRsrcDesc->GetConnectionId(), "CMSFocusMngr::BuildAddUserMsg - IP Service does not exist!!!");
			return;
		}

		DWORD bOn = pServiceParams->GetSipProxyStatus();
		BYTE		bDialDirect	= 0;
		BYTE		bIsUriWithIp = 0;
		int			hostLen = 0;
		char*		strHostIp = NULL;


		DWORD bIsUseOutBoundProxy = STATUS_OK;
		// IpV6
		mcTransportAddress trAddr;
		memset(&trAddr,0,sizeof(mcTransportAddress));

		const char* strDestAddr			= SipNetSetup->GetDestPartyAddress();
		const char* strOriginalToFromDma = SipNetSetup->GetOriginalToDmaSipAddress();
		TRACEINTO << "strOriginalToFromDma " << strOriginalToFromDma;

		char		strTransportIp[IPV6_ADDRESS_LEN];
		char		strAlternativeTransportIp[IPV6_ADDRESS_LEN];
		const mcTransportAddress* pDestTaAddr = SipNetSetup->GetTaDestPartyAddr();

		char tempName11[IPV6_ADDRESS_LEN];
		memset (&tempName11,'\0',IPV6_ADDRESS_LEN);
		mcTransportAddress ttt11;
		memcpy(&ttt11,pDestTaAddr,sizeof(mcTransportAddress));
		ipToString(ttt11,tempName11,1);
		strncpy(strAlternativeTransportIp, SipNetSetup->GetAlternativeTaDestPartyAddr(), IPV6_ADDRESS_LEN - 1);
		strAlternativeTransportIp[IPV6_ADDRESS_LEN - 1] = 0;

		TRACEINTOFUNC << "m_pNetSetup->GetTaDestPartyAddr()  " << tempName11
					  << "\nm_pNetSetup->GetAlternativeTaDestPartyAddr()  " << strAlternativeTransportIp
					  << "\nstrDestAddr = " << strDestAddr;

		if (::isApiTaNull(pDestTaAddr) == FALSE || strAlternativeTransportIp[0] != '\0')
		{
			bDialDirect = 1;
			bIsUriWithIp = 0;
		}
		else
		{
		//check if present @ in dial SIP URI

			//char* strAt = (char*)strstr(strDestAddr,"@");
			char* strAt = (char*)strstr(m_ToAddrStr,"@");


			TRACEINTO << "CMSFocusMngr::BuildAddUserMsg - strAt = " << strAt;
		//if found @ it's dialing via proxy (if defined)
		//otherwise if only valid IP address it's direct dial

			bDialDirect = 0;

		//if found @ check what is after the @
			strHostIp	= strAt ? (strAt+1) : NULL;
		//valid explicit IP or not
			TRACEINTO << "CMSFocusMngr::BuildAddUserMsg - strHostIp = " << strHostIp;
			if (strHostIp)
			{
				memset(&trAddr,0,sizeof(mcTransportAddress));
				::stringToIp(&trAddr,strHostIp);
				BYTE isIpAddrValid = ::isApiTaNull(&trAddr);
				if (isIpAddrValid != TRUE)
				{
					isIpAddrValid = ::isIpTaNonValid(&trAddr);
					if (isIpAddrValid != TRUE)
						bIsUriWithIp = 1;
				}
			}
		//	strHostIp ? ::IsValidIpV4Address(strHostIp) : NO;
			hostLen	= strHostIp ? strlen(strHostIp) : NO;
		}

		// if the service status is On and its not direct IP call (IP address valid or URI with IP address)
		// we try to get the outbound proxy.
		char proxyAddress[MaxLengthOfSingleUrl];
		memset(proxyAddress,0,MaxLengthOfSingleUrl);

		if(bOn != eServerStatusOff && bDialDirect == NO)
		{
			GetOutboundSipProxy(proxyAddress);

			proxyAddress[MaxLengthOfSingleUrl - 1] = '\0';

			if (proxyAddress[0] == '\0')
				bIsUseOutBoundProxy = STATUS_FAIL;

		}

		if(bIsUseOutBoundProxy != STATUS_OK)
		{
			DBGPASSERT(STATUS_FAIL);
			 PTRACE(eLevelInfoNormal,"CMSFocusMngr::BuildAddUserMsg: No OutboundProxy - need to close call");
			//m_pPartyApi->SipPartyCallFailed(SIP_BAD_STATUS);
		}

		else if (bOn == eServerStatusOff || bDialDirect || bIsUriWithIp || proxyAddress[0] != '\0'/*m_outboundProxyIp*/)
		{
			char strContact[IP_STRING_LEN];
			const char* strLocalAddr = SipNetSetup->GetLocalSipAddress();  // user@ip
			char		strLocalUri[IP_STRING_LEN];
			char		newstrToAddr[IP_STRING_LEN];

			SipNetSetup->CopyLocalUriToBuffer(strLocalUri,IP_STRING_LEN); // user@domain (if we have domain)

			const char* strToDisplay		= NULL;
			const char* strToAddr			= NULL;
			const char* strFromDisplay		= "";//SipNetSetup->GetSrcPartyAddress();

			const char* strFromAddr;
			if(strOriginalToFromDma && strOriginalToFromDma[0])
			{
				PTRACE(eLevelInfoNormal,"CMSFocusMngr::BuildAddUserMsg: IN strOriginalToFromDma");
				strFromAddr = strOriginalToFromDma;
				if(strLocalUri[0])
				{
					snprintf(strContact,IP_STRING_LEN,"%s",strLocalUri);
				}
				else
				{
					snprintf(strContact,IP_STRING_LEN,"%s",SipNetSetup->GetSrcPartyAddress());
				}
			}
			else if(strLocalUri[0])
			{
				PTRACE(eLevelInfoNormal,"CMSFocusMngr::BuildAddUserMsg: IN strLocalUri");
				strFromAddr = strLocalUri;
				snprintf(strContact,IP_STRING_LEN,"%s",strFromAddr);
			}
			else
			{
				PTRACE(eLevelInfoNormal,"CMSFocusMngr::BuildAddUserMsg: IN GetSrcPartyAddress");
				strFromAddr = SipNetSetup->GetSrcPartyAddress();
				snprintf(strContact,IP_STRING_LEN,"%s",strFromAddr);
			}


	/*		TRACEINTOFUNC << "bDialDirect: " << (int)bDialDirect << ", strLocalUri: " << strLocalUri << ", strLocalAddr: " << strLocalAddr;

	        if (bDialDirect ||  bOn == eServerStatusOff)
	            strFromAddr = strLocalAddr;
	        else
	            strFromAddr = strLocalUri[0]? strLocalUri: strLocalAddr;
			const char* strRemoteUri		= SipNetSetup->GetRemoteSipAddress();

*/
		/*	if (alternativeAddrStr && alternativeAddrStr[0])
			{
				strToDisplay	= "";
				strToAddr		= alternativeAddrStr;
				strDestAddr 	= alternativeAddrStr;
			}

			else
			{
		*/		strToDisplay	= m_ToAddrStr;      //SipNetSetup->GetRemoteDisplayName();
				strToAddr		= m_ToAddrStr;		//strRemoteUri[0]? strRemoteUri: strDestAddr;
				SipNetSetup->SetDestPartyAddress(m_ToAddrStr);

/*
				std::string sKey;
				std::string sipURIsuffix;

				sKey = "SIP_AUTO_SUFFIX_EXTENSION";
				CProcessBase::GetProcess()->GetSysConfig()->GetDataByKey(sKey, sipURIsuffix);
			 	int sipURIsuffixLen = sipURIsuffix.size();
				if( (sipURIsuffixLen>0) && (strstr(strToAddr, sipURIsuffix.c_str()) == NULL))

				{
				   memset(newstrToAddr, 0, IP_STRING_LEN);
				   strncpy(newstrToAddr, strToAddr, IP_STRING_LEN-1);
			 	   strncat(newstrToAddr, sipURIsuffix.c_str(), IP_STRING_LEN);
			 	   strToAddr = newstrToAddr;

				}

		//	}
*/
//		if (SipNetSetup->GetRemoteSipAddressType() == PARTY_SIP_TELURL_ID_TYPE)/*do we need to add this in case of Amdocs?*/
/*			{
			    char * sipTelURIsuffix = ";user=phone";
			    if( strstr(strToAddr, sipTelURIsuffix) == NULL )
			    {
					memset(newstrToAddr, 0, IP_STRING_LEN);
					strncpy(newstrToAddr, strToAddr, IP_STRING_LEN-1);
					strncat(newstrToAddr, sipTelURIsuffix, IP_STRING_LEN-1);
					strToAddr = newstrToAddr;
			    }
			}
*/
	const char* strContactDisplay	= "";
	const char* strContentType      = "application/cccp+xml";

	//strToDisplay	= SipNetSetup->GetRemoteDisplayName();

	int toDisplayLen		= strlen(strToDisplay);
	int toAddrLen			= strlen(strToAddr);
	int fromDisplayLen		= strlen(strFromDisplay);
	int fromAddrLen			= strlen(strFromAddr);
	int contactDisplayLen	= strlen(strContactDisplay);
	int localAddrLen		= strlen(strLocalAddr);

	int contentTypeLen      = strlen(strContentType);
	int confParamLen		= strlen(strConfParamInfo);

	TRACEINTO << "CMSFocusMngr::BuildAddUserMsg - strToDisplay - " << strToDisplay << "\n strToAddr - " << strToAddr
	<< "\n strFromDisplay - " << strFromDisplay << "\n strFromAddr - " << strFromAddr << "\n strContactDisplay - " << strContactDisplay
	<< "\n strLocalAddr - " << strLocalAddr << "\n strDestAddr - " << strDestAddr << "\n";


	CSipHeaderList headerList(MIN_ALLOC_HEADERS*2,7,
	(int)kToDisplay,		toDisplayLen,		strToDisplay,
	(int)kTo,				toAddrLen,			strToAddr,
	(int)kFromDisplay,		fromDisplayLen,		strFromDisplay,
	(int)kFrom,				strlen(strFromAddr),		strFromAddr,
	(int)kContactDisplay,	contactDisplayLen,	strContactDisplay,
	(int)kContact,			strlen(strContact),	strContact,
	(int)kContentType,			contentTypeLen		,strContentType,
	(int)kProprietyHeader,	confParamLen,		strConfParamInfo);



	SetDialOutSessionTimerHeaders(headerList);
	int headersSize  = headerList.GetTotalLen();

	//Build XML Budy

	COstrStream ostr;

	std::string ConfGUID= GetHexNum(SipNetSetup->GetSIPConfIdAsGUID());
/*
	ostr <<
			"<?xml version=\"1.0\"?>"
			"<request xmlns=\"urn:ietf:params:xml:ns:cccp\""
			" xmlns:mscp=\"http://schemas.microsoft.com/rtc/2005/08/cccpextensions\""
			" C3PVersion=\"1\" to=\"" << m_FocusUri << "\" from=\"sip:boris2@isrexchlab.local\" requestId=\"0\">"
			"<addUser>"
			"<conferenceKeys confEntity=\"" << m_FocusUri << "\"/>"
			"<ci:user xmlns:ci=\"urn:ietf:params:xml:ns:conference-info\" entity=\"sip:boris2@isrexchlab.local\">"
			"<ci:roles>"
			"<ci:entry>attendee</ci:entry>"
			"</ci:roles>"
		//	"<ci:endpoint entity=\"" << ConfGUID << "\" xmlns:msci=\"http://schemas.microsoft.com/rtc/2005/08/confinfoextensions\">\n"
			"<ci:endpoint entity=\"" << "2508C0BD-EA91-4205-93B4-723FF15031B6" << "\" xmlns:msci=\"http://schemas.microsoft.com/rtc/2005/08/confinfoextensions\">"
			"<msci:clientInfo>"
			"<cis:separator xmlns:cis=\"urn:ietf:params:xml:ns:conference-info-separator\"></cis:separator>"
			"<msci2:lobby-capable xmlns:msci2=\"http://schemas.microsoft.com/rtc/2008/12/confinfoextensions\">true</msci2:lobby-capable>"
			"</msci:clientInfo>"
			"</ci:endpoint>"
			"</ci:user>"
			"</addUser>"
			"</request>";
*/


		memset(buffXml, '\0',2048);

	/*	sprintf(buffXml,
				"<?xml version=\"1.0\"?> <request xmlns=\"urn:ietf:params:xml:ns:cccp\" xmlns:mscp=\"http://schemas.microsoft.com/rtc/2005/08/cccpextensions\" C3PVersion=\"1\" to=\"%s\" from=\"sip:boris2@isrexchlab.local\" requestId=\"0\"> <addUser> <conferenceKeys confEntity=\"%s\"/> <ci:user xmlns:ci=\"urn:ietf:params:xml:ns:conference-info\" entity=\"sip:boris2@isrexchlab.local\"> <ci:roles> <ci:entry>attendee</ci:entry> </ci:roles> <ci:endpoint entity=\"{2508C0BD-EA91-4205-93B4-723FF15031B6}\" xmlns:msci=\"http://schemas.microsoft.com/rtc/2005/08/confinfoextensions\"> <msci:clientInfo> <cis:separator xmlns:cis=\"urn:ietf:params:xml:ns:conference-info-separator\"></cis:separator> <msci2:lobby-capable xmlns:msci2=\"http://schemas.microsoft.com/rtc/2008/12/confinfoextensions\">true</msci2:lobby-capable> </msci:clientInfo> </ci:endpoint> </ci:user> </addUser> </request>"
				, m_FocusUri ,  m_FocusUri);
*/


		snprintf(buffXml,
				    sizeof(buffXml),
					"<?xml version=\"1.0\"?> <request xmlns=\"urn:ietf:params:xml:ns:cccp\" xmlns:mscp=\"http://schemas.microsoft.com/rtc/2005/08/cccpextensions\" C3PVersion=\"1\" to=\"%s\" from=\"sip:boris2@isrexchlab.local\" requestId=\"0\"> <addUser> <conferenceKeys confEntity=\"%s\"/> <ci:user xmlns:ci=\"urn:ietf:params:xml:ns:conference-info\" entity=\"sip:boris2@isrexchlab.local\"> <ci:roles> <ci:entry>attendee</ci:entry> </ci:roles> <ci:endpoint entity=\"{2508C0BD-EA91-4205-93B4-723FF15031B6}\" xmlns:msci=\"http://schemas.microsoft.com/rtc/2005/08/confinfoextensions\"> <msci:clientInfo> <cis:separator xmlns:cis=\"urn:ietf:params:xml:ns:conference-info-separator\"></cis:separator> <msci2:lobby-capable xmlns:msci2=\"http://schemas.microsoft.com/rtc/2008/12/confinfoextensions\">true</msci2:lobby-capable> </msci:clientInfo> </ci:endpoint> </ci:user> </addUser> </request>"
					, m_FocusUri ,  m_FocusUri);




		PTRACE2(eLevelInfoNormal,"CMSFocusMngr::BuildAddUserMsg: buffXml",buffXml); //debug for ANAT

		//ostr << buffXml;


	// BuildXMLBudy(SipNetSetup,ostr);
	// PTRACE2(eLevelInfoNormal,"CMSFocusMngr::BuildAddUserMsg: XML Body",ostr.str().c_str()); //debug for ANAT

	int contentSize = strlen(buffXml) + 1;
	PTRACE2INT(eLevelInfoNormal,"CMSFocusMngr::BuildAddUserMsg: XML Body size: ",contentSize); //debug for ANAT

	int contentAndHeadersSize	= headersSize + contentSize;

	int totalSize			= sizeof(mcReqInvite);
	totalSize				+= contentAndHeadersSize;

	//Build Invite Msg
	mcReqInvite* pInviteMsg = (mcReqInvite *)new BYTE[totalSize];
	memset(pInviteMsg, '\0', totalSize);

	//remote port will NOT be use if the call is trough an outbound proxy
	pInviteMsg->transportAddress.transAddr.port = SipNetSetup->GetRemoteSignallingPort();

	// invite message struct may be filled up with either an ip address
	// or a domain name. domain name, requires a dns lookup by CS.
	BYTE bUserDialWithIp = TRUE; // no dns lookup is needed
	if (bDialDirect)
	{
	  if( strAlternativeTransportIp[0] != '\0' )
	   {
	       mcTransportAddress trAddr;
		   memset(&trAddr,0,sizeof(mcTransportAddress));
		   ::stringToIp(&trAddr,strAlternativeTransportIp);
		   BYTE isIpAddrValid = ::isApiTaNull(&trAddr);
		   if (isIpAddrValid != TRUE)
		     {
			               isIpAddrValid = ::isIpTaNonValid(&trAddr);
			               if (isIpAddrValid != TRUE)
			               {
			        	       strncpy(strTransportIp, strAlternativeTransportIp, 64);
			        	       strTransportIp[sizeof(strTransportIp) - 1] = '\0';
			               }
		     }
		     else
		     {       // dns lookup(by CS) is needed
			               memcpy(pInviteMsg->domainName, strAlternativeTransportIp, IPV6_ADDRESS_LEN);
			               bUserDialWithIp = FALSE;
		     }
		}
		else
		{
			        char str[64];
			        memset(str,'\0',64);
			        ::ipToString( *pDestTaAddr,str,1);
			        DWORD destLen = strlen(str);
			        strncpy(strTransportIp, str, 64);
			        strTransportIp[sizeof(strTransportIp) - 1] = '\0';
		}
		TRACEINTO << "CSipCntl::SipInviteReq - strTransportIp (Dial direct) " << strTransportIp;
	}
	else if(proxyAddress[0] != '\0')
	{
		// if uri contains the proxy ip address or contains registrar domain name
		// or contains a format of "user@domain"
		if (  (bIsUriWithIp && strcmp(strHostIp,proxyAddress)==0) ||
			  (bIsUriWithIp && strcmp(strHostIp,SipNetSetup->GetLocalHost())==0)  || !bIsUriWithIp )
					// User Dial With Proxy
			{
					pInviteMsg->bIsOutboundProxyInUse = YES;
					pInviteMsg->transportAddress.transAddr.port = 0; // card will take port from proxy's service
					mcTransportAddress trAddr;
					memset(&trAddr,0,sizeof(mcTransportAddress));
					::stringToIp(&trAddr,proxyAddress);
					if (!::isApiTaNull(&trAddr) && !::isIpTaNonValid(&trAddr))
					{
							strncpy(strTransportIp, proxyAddress, sizeof(strTransportIp) - 1);
							strTransportIp[sizeof(strTransportIp) - 1] = 0;
					}
					else
					{	// dns lookup(by CS) is needed
						memcpy(pInviteMsg->domainName,proxyAddress,MaxLengthOfSingleUrl);
						bUserDialWithIp = FALSE;
					}
			}
			else if(bIsUriWithIp) // uri with a none proxy/registrar ip address
			{
				// direct dial
		       	    strncpy(strTransportIp, strHostIp, sizeof(strTransportIp) - 1);
	    		    strTransportIp[sizeof(strTransportIp) - 1] = '\0';

				    if (hostLen >= IPV6_ADDRESS_LEN)
				    {
	                   		 PTRACE2(eLevelInfoNormal,"CSipCntl::SipInviteReq host ip is longer that 64 chars", strHostIp);
				    }
			}
		}
			// no proxy is defined but ip with uri exists
			else if(bIsUriWithIp)
			{
	            strncpy(strTransportIp, strHostIp, sizeof(strTransportIp) - 1);
	            strTransportIp[sizeof(strTransportIp) - 1] = '\0';
			}
			else
				strTransportIp[0] = 0;

			if(bUserDialWithIp)
			{
				mcTransportAddress trAddr;
				memset(&trAddr,0,sizeof(mcTransportAddress));
				::stringToIp(&trAddr,strTransportIp);
						pInviteMsg->transportAddress.transAddr.addr.v4.ip = trAddr.addr.v4.ip;

			}


			//POBJDELETE(pLanCfg);
			pInviteMsg->transportAddress.transAddr.transportType = eTransportTypeTls;//m_transportType;

			// IpV6
			//enIpVersion eIpAddrMatch = CheckForMatchBetweenPartyAndUdp(m_pNetSetup->GetIpVersion(),m_UdpAddressesParams.IpType);

			//set XML params
			//pInviteMsg->transportAddress.unionProps.unionType = eIpAddrMatch;//assuming the signaling type is equal to the media IP address type.
			pInviteMsg->transportAddress.unionProps.unionType = pInviteMsg->transportAddress.transAddr.ipVersion;  //modified for ANAT
			pInviteMsg->transportAddress.unionProps.unionSize = sizeof(ipAddressIf);

		/*	//add for CG_SoftMCU
			if (IsCallGeneratorConf())
			{
				pInviteMsg->callGeneratorParams.bIsCallGenerator=1;
				pInviteMsg->callGeneratorParams.eEndpointModel=endpointModelHDX9000;
			}
*/

		sipSdpAndHeadersSt* pSdpAndHeaders = (sipSdpAndHeadersSt*)&pInviteMsg->sipSdpAndHeaders;

		//////Inga /////
		//char XMLBuffer [contentSize]= {0};
		//strncpy(XMLBuffer,buffXml,contentSize);



		PTRACE2(eLevelInfoNormal,"CMSFocusMngr::BuildAddUserMsg: XML Body",buffXml); //debug for ANATostr.str().c_str()); //debug for ANAT

		PTRACE2INT(eLevelInfoNormal,"CMSFocusMngr::BuildAddUserMsg: XML buffer size:",strlen(buffXml)); //debug for ANATostr.str().c_str()); //debug for ANAT

			////////////

	memset(pSdpAndHeaders->capsAndHeaders,'\0', contentSize);
	memcpy(pSdpAndHeaders->capsAndHeaders,buffXml,contentSize-1);
	pSdpAndHeaders->capsAndHeaders[contentSize-1]='\0';

	PTRACE2(eLevelInfoNormal,"CMSFocusMngr::BuildAddUserMsg: XML Body2",pSdpAndHeaders->capsAndHeaders); //debug for ANATostr.str().c_str()); //debug for ANAT

	int lenOfXML = strlen(pSdpAndHeaders->capsAndHeaders)-1;

	pSdpAndHeaders->lenOfDynamicSection = contentAndHeadersSize;
	pSdpAndHeaders->sipHeadersOffset = lenOfXML;

	pSdpAndHeaders->sipMediaLinesLength = lenOfXML;
	pSdpAndHeaders->sipHeadersLength =  headersSize;

	sipMessageHeaders* pHeaders = (sipMessageHeaders*) ((char*) pSdpAndHeaders->capsAndHeaders + pSdpAndHeaders->sipHeadersOffset);

	headerList.BuildMessage(pHeaders);

	size_t size = sizeof(mcReqInvite) + pInviteMsg->sipSdpAndHeaders.lenOfDynamicSection;
	SendSIPMsgToCS(SIP_CS_CCCP_SIG_INVITE_REQ, pInviteMsg, size);


	StartTimer(ADDUSERCONNECTTOUT, GetSystemCfgFlagInt<DWORD>(CFG_KEY_SIP_MSG_TIMEOUT) * SECOND);
	PDELETEA(pInviteMsg);
		}
		else if (outboundProxyIp == 0)// get the proxy IP from DNS
		{
			PTRACE(eLevelInfoNormal,"CSipCntl::SipInviteReq: Zero IP. Get IP from DNS");
		}

}
/////////////////////////////////////////////////////////////////////////////////////////
void CMSFocusMngr::BuildXMLBudy(CSipNetSetup* SipNetSetup,std::ostream &ostr)
{

	PTRACE(eLevelInfoNormal,"CMSFocusMngr::BuildXMLBudy");
	EventPackage::Request AddUserReq;
/*
	int AVMCUAddlen = strlen(m_strAVMCUAddress);
	ALLOCBUFFER(AddUserReq.m_to,AVMCUAddlen);
	strncpy(AddUserReq.m_to,m_strAVMCUAddress,AVMCUAddlen);
	AddUserReq.m_to[AVMCUAddlen] = '\0';


	int len = strlen(m_strAVMCUAddress);
	ALLOCBUFFER(AddUserReq.m_from,len);
	strncpy(AddUserReq.m_from,SipNetSetup->GetLocalSipAddress(),len);
	AddUserReq.m_from[len] = '\0';
*/

//
	//AddUserReq.m_NSRequestDif="xmlns=\"urn:ietf:params:xml:ns:cccp\" xmlns:mscp=\"http://schemas.fabrikam.com/rtc/2005/08/cccpextensions\"";
	AddUserReq.m_to = m_FocusUri;//m_strAVMCUAddress;

	std::string from("sip:");
	from += SipNetSetup->GetSrcPartyAddress();
	AddUserReq.m_from = from;
	AddUserReq.m_requestId = "0";
	AddUserReq.m_C3PVersion = "1";



	EventPackage::AddUser AddUser;

	AddUser.m_conferenceKeys.m_confEntity = m_FocusUri;//m_strAVMCUAddress;

/*
	ALLOCBUFFER(AddUser.m_confere.nceKeys.m_confEntity,AVMCUAddlen);
	strncpy(AddUser.m_conferenceKeys.m_confEntity,m_strAVMCUAddress,AVMCUAddlen);
	AddUser.m_conferenceKeys.m_confEntity[AVMCUAddlen] = '\0';
*/

	std::string role = "attendee";
	AddUser.m_user.m_roles.m_roles.push_back(role);


	//std::string GUID = GetHexNum(SipNetSetup->GetSIPConfIdAsGUID());
	EventPackage::Endpoint Endpoint;

	Endpoint.m_entity = GetHexNum(SipNetSetup->GetSIPConfIdAsGUID());//SipNetSetup->GetSIPConfIdAsGUID();
	Endpoint.m_clientInfo.m_lobbyCapable = true;
	//"http://schemas.microsoft.com/rtc/2005/08/confinfoextensions";

	AddUser.m_user.m_entity = SipNetSetup->GetSrcPartyAddress();
	AddUser.m_user.m_endpoints.push_back(Endpoint);

	AddUserReq.m_pRequestType = &AddUser;

	ostr << AddUserReq;


	//int contentSize = pStr->GetStringLength();


}
/////////////////////////////////////////////////////////////////////////////////////////
void CMSFocusMngr::OnSipInviteResponseConnecting(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CMSFocusMngr::OnSipInviteResponseConnecting");


	APIU32 callIndex = 0;
	APIU32 channelIndex = 0;
	APIU32 mcChannelIndex = 0;
	APIU32 stat1 = 0;
	APIU16 srcUnitId = 0;
	BOOL IsMs2013Server = FALSE;


	*pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

	PTRACE2INT(eLevelInfoNormal,"CMSFocusMngr::OnSipInviteResponseConnecting:",callIndex);

	m_callIndex = callIndex;

	PTRACE2INT(eLevelInfoNormal,"CMSFocusMngr::OnSipInviteResponseConnecting:",m_callIndex);

		mcIndInviteResponse* pInviteResponseMsg = NULL;
		pInviteResponseMsg = (mcIndInviteResponse *)pParam->GetPtr(1);
		PASSERT_AND_RETURN(NULL == pInviteResponseMsg);
		DWORD status = pInviteResponseMsg->status;

		sipSdpAndHeadersSt* pSdpAndHeaders = (sipSdpAndHeadersSt *)&pInviteResponseMsg->sipSdpAndHeaders;


		if (pSdpAndHeaders)
		{
			int length = sizeof(sipSdpAndHeadersBaseSt) + pSdpAndHeaders->lenOfDynamicSection;
			sipSdpAndHeadersSt * 	LyncSdp = NULL;
			LyncSdp = (sipSdpAndHeadersSt *)new BYTE[length];
			memset(LyncSdp,'\0', length);
			memcpy(LyncSdp, pSdpAndHeaders, length-1);


			COstrStream ostr1;
			::DumpXMLToStream(ostr1,LyncSdp);
			PTRACE2(eLevelInfoNormal,"CMSFocusMngr::OnSipInviteResponseConnecting - dump XML2",ostr1.str().c_str()); //debug

			sipMessageHeaders* pHeaders = (sipMessageHeaders*) ((char*) pSdpAndHeaders->capsAndHeaders + pSdpAndHeaders->sipHeadersOffset);

			CSipHeaderList* pTemp = new CSipHeaderList(*pHeaders); AUTO_DELETE(pTemp);
			if(pHeaders)
			{
				const CSipHeader* pMsServer	= pTemp->GetNextHeader(kServer);
				if(pMsServer)
				{
					IsMs2013Server = strstr(pMsServer->GetHeaderStr(),"5") == 0 ? false : true;
					PTRACE2(eLevelInfoNormal,"CMSFocusMngr::OnSipInviteResponseConnecting, pMsServer=", pMsServer->GetHeaderStr());
				}
			}

			if(STATUS_OK == ParseAddUserXML(LyncSdp))
			{
				m_state = sMS_CONNECTED;
				SendInviteAckReq();
				//SendAck()
				//SendRespToPartyCntl
				m_pTaskApi->MSFocusEndConnection(m_PartyId,STATUS_OK,IsMs2013Server,m_ToAddrStr);
			}
			POBJDELETE(LyncSdp);

		}



}
/////////////////////////////////////////////////////////////////////////////////////////
STATUS CMSFocusMngr::ParseAddUserXML(sipSdpAndHeadersSt * LyncSdp)
{
	PTRACE(eLevelInfoNormal, "CMSOrganizerMngr::ParseXML ");
	//Need to parse here the XML
	int size = LyncSdp->sipMediaLinesLength;
	char* DynamicSection = &LyncSdp->capsAndHeaders[LyncSdp->sipMediaLinesOffset];
	char *pXMLBuffer = new char[size+1];
	PASSERTSTREAM_AND_RETURN_VALUE(!pXMLBuffer,"PartyId:" << m_PartyId,STATUS_INCONSISTENT_PARAMETERS);

	memset(pXMLBuffer,'\0', size+1);
	memcpy(pXMLBuffer,DynamicSection,size);

	PTRACE2(eLevelInfoNormal,"CMSFocusMngr::ParseXML - dump XML2",pXMLBuffer);



	m_MSAddUserResponse = new EventPackage::Response;

//	PASSERTSTREAM_AND_RETURN_VALUE(!m_MSAddUserResponse->ReadFromXmlStream(pXMLBuffer, size), "PartyId:" << m_PartyId, STATUS_INCONSISTENT_PARAMETERS);

//	TRACEINTO << "m_MSAddUserResponse: " << (*m_MSAddUserResponse);

	POBJDELETE(pXMLBuffer);

	 return STATUS_OK;

}
/////////////////////////////////////////////////////////////////////////////////////////
void CMSFocusMngr::SendInviteAckReq()
{
	PTRACE(eLevelInfoNormal, "CMSFocusMngr::SendInviteAckReq");

	PTRACE2INT(eLevelInfoNormal,"CMSFocusMngr::SendInviteAckReq callindex: ",m_callIndex);

	mcReqInviteAck* pInviteAckMsg = new mcReqInviteAck; AUTO_DELETE(pInviteAckMsg);
	size_t size = sizeof(mcReqInviteAck);
	memset(pInviteAckMsg, 0, size);
	SendSIPMsgToCS(SIP_CS_SIG_INVITE_ACK_REQ, pInviteAckMsg, size);
}
///////////////////////////////////////////////////////////////////////////
void CMSFocusMngr::OnTimerAddUserConnect(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CMSFocusMngr::OnTimerAddUserConnect");

	m_pTaskApi->MSFocusEndConnection(m_PartyId,STATUS_FAIL,FALSE);

//	m_pPartyApi->SipPartyConnectTout();
}
///////////////////////////////////////////////////////////////////////////
void CMSFocusMngr::OnCsProvisunalResponseInd(CSegment* pParam)
{
	APIU32 callIndex = 0;
	APIU32 channelIndex = 0;
	APIU32 mcChannelIndex = 0;
	APIU32 stat1 = 0;
	APIU16 srcUnitId = 0;
	CMedString str;

	*pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;
	m_callIndex = callIndex;

	PTRACE2INT(eLevelInfoNormal,"CMSFocusMngr::OnCsProvisunalResponseInd callindex: ",m_callIndex);
//	m_pCall->SetCallIndex(m_callIndex);

/*	mcIndProvResponse* pProvisunalMsg = (mcIndProvResponse *)pParam->GetPtr(1);
	str <<  " Status - " << pProvisunalMsg->status;
	if((180<= pProvisunalMsg->status) && (189 >= pProvisunalMsg->status))
	{
		SetRecevRingback(TRUE);
	}

	PTRACE2(eLevelInfoNormal,"CSipCntl::OnCsProvisunalResponseInd, ", str.GetString());
	*/
}
///////////////////////////////////////////////////////////////////////////
void CMSFocusMngr::TerminateFocusConnection()
{
	if(m_state == sMS_DISCONNECTING || m_state == sMS_DISCONNECTED)
	{
			PTRACE(eLevelInfoNormal,"CMSFocusMngr::TerminateFocusConnection - already in disconnection state - igore");
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CMSFocusMngr::TerminateFocusConnection");
		PTRACE2INT(eLevelInfoNormal,"CMSFocusMngr::TerminateFocusConnection m_state1:",m_state);
		m_state = sMS_DISCONNECTING;


		mcReqBye* pByeMsg = new mcReqBye;
		size_t size = sizeof(mcReqBye);
		memset(pByeMsg, 0, size);
		SendSIPMsgToCS(SIP_CS_SIG_BYE_REQ, pByeMsg, size);

		DWORD nRetVal;
		nRetVal = GetSystemCfgFlagInt<DWORD>(CFG_KEY_SIP_MSG_TIMEOUT);
		StartTimer(MSMNGRDISCONNECTTOUT, nRetVal  * SECOND);
		PDELETE(pByeMsg);

		PTRACE2INT(eLevelInfoNormal,"CMSFocusMngr::TerminateFocusConnection m_state2:",m_state);

	}
}

////////////////////////////////////////////////////////////////////////
void CMSFocusMngr::OnSipBye200OkInd(CSegment* pParam)
{
	APIU32 callIndex = 0;
	APIU32 channelIndex = 0;
	APIU32 mcChannelIndex = 0;
	APIU32 stat1 = 0;
	APIU16 srcUnitId = 0;
	PTRACE(eLevelInfoNormal,"CMSFocusMngr::OnSipBye200OkInd");

	*pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

	if (IsValidTimer(MSMNGRDISCONNECTTOUT))
		DeleteTimer(MSMNGRDISCONNECTTOUT);

	mcIndBye200Ok* pBye200OkMsg = (mcIndBye200Ok *)pParam->GetPtr(1);
	DWORD status = pBye200OkMsg->status;

	if (status < LOW_REJECT_VAL || status >= HIGH_REJECT_VAL) // something is wrong
	{
		DBGPASSERT(status);
	}
	else if (status) // normally cross message
	{
		PTRACE2INT(eLevelInfoNormal,"CMSFocusMngr::OnSipBye200OkInd: status - ", status);
	}

	RemoveFocuseConnection();
}


///////////////////////////////////////////////////////////////////////////
void CMSFocusMngr::RemoveFocuseConnection()
{
	PTRACE(eLevelInfoNormal,"CMSFocusMngr::RemoveFocuseConnection");

	RemoveFromRsrcTbl();

	m_state = sMS_DISCONNECTED;

	m_pTaskApi->MSFocusEndDisconnection(m_PartyId);
}

///////////////////////////////////////////////////////////////////////////////////////////////
void CMSFocusMngr::OnTimerDisconnectMngr(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CMSFocusMngr::OnTimerDisconnectCall");
	RemoveFocuseConnection();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CMSFocusMngr::OnSipByeIndConnecting(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CMSFocusMngr::OnSipByeIndConnecting");

	CMSAvMCUMngr::OnSipByeInd(pParam);

	m_pTaskApi->MSFocusEndDisconnection(m_PartyId);

}

/////////////////////////////////////////
void CMSFocusMngr::OnSipByeInd(CSegment* pParam)
{

	PTRACE(eLevelInfoNormal,"CMSFocusMngr::OnSipByeInd");

	CMSAvMCUMngr::OnSipByeInd(pParam);

	m_pTaskApi->MSFocusEndDisconnection(m_PartyId);

}

