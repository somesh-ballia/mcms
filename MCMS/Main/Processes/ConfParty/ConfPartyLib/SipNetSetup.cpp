//+========================================================================+
//                         SipNetSetup.cpp									   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SipNetSetup.cpp	                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Atara	                                                   |
//-------------------------------------------------------------------------|
// Who   | Date       | Description                                        |
//-------------------------------------------------------------------------|
// Atara |  09/07/03  | Sip net setup                                      |
//+========================================================================+

#include "SipNetSetup.h"
#include "SipHeadersList.h"
#include "SipUtils.h"

/////////////////////////////////////////////////////////////////////////////
CSipNetSetup::CSipNetSetup()
{
	m_remoteSipAddressType = PARTY_SIP_SIPURI_ID_TYPE;
	memset(m_strRemoteSipAddress,0,IP_STRING_LEN);
	memset(m_strRemoteDisplayName,0,MaxDisplaySize);
	memset(m_strRemoteUserAgent,0,MaxUserAgentSize);
	m_localSipAddressType = PARTY_SIP_SIPURI_ID_TYPE;
	memset(m_strLocalSipAddress,0,IP_STRING_LEN);
	memset(m_strOriginalToHeaderFromDMA,0,IP_STRING_LEN);
	memset(m_strLocalHost,0,IP_STRING_LEN);
	memset(m_strDestUserName,0,MaxAddressListSize);
	memset(m_strInviteCallId, 0, MaxLengthOfSipCallId);
	m_remoteSignalingPort = 5060;
	m_eRemoteVideoPartyType = eVideo_party_type_none;
	m_ePerferedIpV6ScopeAddr = eScopeIdOther;
	memset(&m_dnsProxyAddr, 0, (sizeof(ipAddressStruct)*TOTAL_NUM_OF_IP_ADDRESSES));
	m_eLocalMediaIpType = eIpVersion4;
	memset(m_alternativeTaDestPartyAddr,0,IP_STRING_LEN);
	m_EnableSipICE = FALSE;
	memset(m_strRemoteSipContact,0,IP_STRING_LEN);

	memset(m_strToPartyAddress,0,MaxAddressListSize);
	m_bIsMsConfInvite = FALSE;

	memset(m_msConversationId,0,MS_CONVERSATION_ID_LEN);



}

/////////////////////////////////////////////////////////////////////////////
CSipNetSetup::~CSipNetSetup()
{
}
///////////////////////////////////////////////////////////////////
const char*  CSipNetSetup::NameOf() const
{
	return "CSipNetSetup";
}


/////////////////////////////////////////////////////////////////////////////
CIpNetSetup& CSipNetSetup::operator=(const CIpNetSetup& other)
{
	if (this != &other)
	{
		CIpNetSetup::operator=(other);

		if (strcmp(other.NameOf() ,"CSipNetSetup") == 0)
		{
			CSipNetSetup & rOtherSipNetSetup = (CSipNetSetup &)other;
			m_remoteSipAddressType = rOtherSipNetSetup.m_remoteSipAddressType;
			strncpy(m_strRemoteSipAddress,rOtherSipNetSetup.m_strRemoteSipAddress,IP_STRING_LEN);
			strncpy(m_strRemoteDisplayName,rOtherSipNetSetup.m_strRemoteDisplayName,MaxDisplaySize);
			strncpy(m_strRemoteUserAgent,rOtherSipNetSetup.m_strRemoteUserAgent,MaxUserAgentSize);
			m_localSipAddressType = rOtherSipNetSetup.m_localSipAddressType;
			strncpy(m_strLocalSipAddress,rOtherSipNetSetup.m_strLocalSipAddress,IP_STRING_LEN);
			strncpy(m_strOriginalToHeaderFromDMA,rOtherSipNetSetup.m_strOriginalToHeaderFromDMA,IP_STRING_LEN);
			strncpy(m_strLocalHost,rOtherSipNetSetup.m_strLocalHost,IP_STRING_LEN);
			strncpy(m_strDestUserName, rOtherSipNetSetup.m_strDestUserName, MaxAddressListSize);
			strncpy(m_strInviteCallId, rOtherSipNetSetup.m_strInviteCallId, MaxLengthOfSipCallId);
			m_remoteSignalingPort = rOtherSipNetSetup.m_remoteSignalingPort;
			m_eRemoteVideoPartyType = rOtherSipNetSetup.m_eRemoteVideoPartyType;
			m_ePerferedIpV6ScopeAddr = rOtherSipNetSetup.m_ePerferedIpV6ScopeAddr;
			memcpy(&m_dnsProxyAddr,rOtherSipNetSetup.m_dnsProxyAddr,(sizeof(ipAddressStruct)*TOTAL_NUM_OF_IP_ADDRESSES));
			m_eLocalMediaIpType = rOtherSipNetSetup.m_eLocalMediaIpType;
			strncpy(m_strRemoteSipContact,rOtherSipNetSetup.m_strRemoteSipContact,IP_STRING_LEN);
			SetToPartyAddress(rOtherSipNetSetup.m_strToPartyAddress);

			m_bIsMsConfInvite = rOtherSipNetSetup.m_bIsMsConfInvite;

			strncpy(m_msConversationId,rOtherSipNetSetup.m_msConversationId,MS_CONVERSATION_ID_LEN);
		}
		else
			DBGPASSERT(YES);
	}
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::copy(const CNetSetup * rhs)
{
	*this = *((CSipNetSetup *)rhs);
}

/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::Serialize(WORD format,CSegment& seg)
{
	if (format == NATIVE)
	{
		CIpNetSetup::Serialize(format,seg);

		seg << m_remoteSipAddressType;

		DWORD len = strlen(m_strRemoteSipAddress);

		if(len >= IP_STRING_LEN)//VNGR-24921
        {
            PTRACE2INT(eLevelInfoNormal,"CSipNetSetup::Serialize - possible memory corruption length of m_strRemoteSipAddress = ",len);
            len = IP_STRING_LEN - 1;
        }

		seg << len;
		seg.Put((BYTE *)m_strRemoteSipAddress,len);

		seg << m_remoteSignalingPort;

		len = strlen(m_strRemoteDisplayName);
		seg << len;
		seg.Put((BYTE *)m_strRemoteDisplayName,len);

		len = strlen(m_strRemoteUserAgent);
		seg << len;
		seg.Put((BYTE *)m_strRemoteUserAgent,len);

		seg << m_EnableSipICE;

		seg << m_localSipAddressType;

		len = strlen(m_strLocalSipAddress);
		seg << len;
		seg.Put((BYTE *)m_strLocalSipAddress,len);


		len = strlen(m_strOriginalToHeaderFromDMA);
		seg << len;
		seg.Put((BYTE *)m_strOriginalToHeaderFromDMA,len);



		len = strlen(m_strLocalHost);
		seg << len;
		seg.Put((BYTE *)m_strLocalHost,len);

		len = strlen(m_strDestUserName);
		seg << len;
		seg.Put((BYTE *)m_strDestUserName,len);

		len = strlen(m_strInviteCallId);
		seg << len;
		seg.Put((BYTE *)m_strInviteCallId,len);

		len = strlen(m_strRemoteSipContact);

		if(len >= IP_STRING_LEN)//VNGR-24921
        {
            PTRACE2INT(eLevelInfoNormal,"CSipNetSetup::Serialize - possible memory corruption length of m_strRemoteSipContact = ",len);
            len = IP_STRING_LEN - 1;
        }

		seg << len;
		seg.Put((BYTE *)m_strRemoteSipContact,len);

		seg << (BYTE)m_eRemoteVideoPartyType;

		seg << (BYTE)m_ePerferedIpV6ScopeAddr;

		seg << (BYTE)m_eLocalMediaIpType;

		BYTE numOfValidProxyDnsIpAddresses = 0;
		char szIP[64];
		for (int i = 0 ; i < TOTAL_NUM_OF_IP_ADDRESSES; i++)
		{
			if (!(::IsIpNull(&(m_dnsProxyAddr[i]))))
			{
				numOfValidProxyDnsIpAddresses++;
			}
		}
		seg << numOfValidProxyDnsIpAddresses;
		if (numOfValidProxyDnsIpAddresses > 0)
		{
			for (int i = 0 ; i < numOfValidProxyDnsIpAddresses; i++)
			{
				seg << m_dnsProxyAddr[i].ipVersion;
				if (m_dnsProxyAddr[i].ipVersion == eIpVersion6)
					seg << m_dnsProxyAddr[i].addr.v6.scopeId;
				memset(szIP,'\0',64);
				CIpAddressPtr proxyIp = CIpAddress::CreateIpAddress(m_dnsProxyAddr[i]);
				memcpy(szIP,proxyIp->ToString(1),64);
				seg << szIP;
			}

		}

		len = strlen(m_strToPartyAddress);
		seg << len;
		seg.Put((BYTE *)m_strToPartyAddress,len);

		len = strlen(m_msConversationId);
		seg << len;
		seg.Put((BYTE *)m_msConversationId,len);

		seg << m_bIsMsConfInvite;



		//		seg << m_bIsTipCall;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::DeSerialize(WORD format,CSegment& seg)
{
	if (format == NATIVE)
	{
		DWORD len;
		CIpNetSetup::DeSerialize(format,seg);
		BYTE tempRemoteVideoPartyType = 0;
		BYTE tempPerferedScopeId = 0;
		BYTE tempLocalIptype = 0;
		BYTE numOfValidProxyDnsIpAddresses = 0;

		seg >> m_remoteSipAddressType;
		seg >> len;

		if(len >= IP_STRING_LEN)//VNGR-24921
        {
            PTRACE2INT(eLevelInfoNormal,"CSipNetSetup::DeSerialize - possible memory corruption length of m_strRemoteSipAddress = ",len);
            len = IP_STRING_LEN - 1;
        }

		seg.Get((BYTE *)m_strRemoteSipAddress,len);
		m_strRemoteSipAddress[len] = 0;
		seg >> m_remoteSignalingPort;
		seg >> len;
		seg.Get((BYTE *)m_strRemoteDisplayName,len);
		m_strRemoteDisplayName[len] = 0;

		seg >> len;
		seg.Get((BYTE *)m_strRemoteUserAgent,len);
		m_strRemoteUserAgent[len] = 0;
		seg >> m_EnableSipICE;
		seg >> m_localSipAddressType;
		seg >> len;
		seg.Get((BYTE *)m_strLocalSipAddress,len);
		m_strLocalSipAddress[len] = 0;

		seg >> len;
		seg.Get((BYTE *)m_strOriginalToHeaderFromDMA,len);
		m_strOriginalToHeaderFromDMA[len] = 0;

		seg >> len;
		seg.Get((BYTE *)m_strLocalHost,len);
		m_strLocalHost[len] = 0;
		seg >> len;
		seg.Get((BYTE *)m_strDestUserName,len);
		m_strDestUserName[len] = 0;
		seg >> len;
		seg.Get((BYTE *)m_strInviteCallId,len);
		m_strInviteCallId[len] = 0;
		seg >> len;
		if(len >= IP_STRING_LEN)//VNGR-24921
        {
            PTRACE2INT(eLevelInfoNormal,"CSipNetSetup::DeSerialize - possible memory corruption length of m_strRemoteSipContact = ",len);
            len = IP_STRING_LEN - 1;
        }
		seg.Get((BYTE *)m_strRemoteSipContact,len);
		m_strRemoteSipContact[len] = 0;
		seg >> tempRemoteVideoPartyType;
		m_eRemoteVideoPartyType = (eVideoPartyType)tempRemoteVideoPartyType;
		seg >> tempPerferedScopeId;
		m_ePerferedIpV6ScopeAddr = (enScopeId)tempPerferedScopeId;
		seg >> tempLocalIptype;
		m_eLocalMediaIpType = (enIpVersion)tempLocalIptype;
		seg >> numOfValidProxyDnsIpAddresses;
		char szIP[64];
		for (int i = 0 ; i < numOfValidProxyDnsIpAddresses; i++)
		{
			seg >> m_dnsProxyAddr[i].ipVersion;
			if (m_dnsProxyAddr[i].ipVersion == eIpVersion6)
				seg >> m_dnsProxyAddr[i].addr.v6.scopeId;
			memset(szIP,'\0',64);
			seg >> szIP;
			::stringToIp(&(m_dnsProxyAddr[i]),szIP);
		}

		seg >> len;
		seg.Get((BYTE *)m_strToPartyAddress,len);
		m_strToPartyAddress[len] = 0;


		seg >> len;
		seg.Get((BYTE *)m_msConversationId,len);
		m_msConversationId[len] = 0;

		seg >> m_bIsMsConfInvite;
		//seg >> m_bIsTipCall;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetRemoteSipAddress(const char * strAddress)
{
    char tmpstrAddress[IP_STRING_LEN] = "";

    char*		strHostIp = NULL;
	char* strAt	= (char*)strstr(strAddress,"@");
	strHostIp	= strAt ? (strAt+1) : NULL;
	BYTE bIsUriWithIp =FALSE;
	int	hostLen = 0;
	if (strHostIp && isIpV6Str(strHostIp))
	{
        mcTransportAddress trAddr;
		::stringToIp(&trAddr,strHostIp);
        char	strTransportIp[IPV6_ADDRESS_LEN];
        int maxStringLenForKW = min((int)sizeof(tmpstrAddress) - 1, (int)((DWORD)strAt-(DWORD)strAddress + 1));
        strncpy (tmpstrAddress, strAddress, maxStringLenForKW); //+1 for the @
        tmpstrAddress[maxStringLenForKW] = 0;
        // add the God damm bracets with this 1 parameter
        ::ipToString(trAddr, strTransportIp, 1);
        strcat (tmpstrAddress, strTransportIp);
        strncpy(m_strRemoteSipAddress, tmpstrAddress,IP_STRING_LEN - 1);
    }
    else
        strncpy(m_strRemoteSipAddress,strAddress,IP_STRING_LEN - 1);

    m_strRemoteSipAddress[IP_STRING_LEN - 1] = '\0';
    PTRACE2 (eLevelInfoNormal, "CSipNetSetup::SetRemoteSipAddress - m_strRemoteSipAddres ", m_strRemoteSipAddress);
}
////////////////////////////////////////////////////////////////
int CSipNetSetup::SetIpVersionAccordingToUri(DWORD serviceIpType)
{
	char*		strHostIp = NULL;
	char* strAt	= strstr(m_strRemoteSipAddress,"@");
	strHostIp	= strAt ? (strAt+1) : NULL;
	BYTE bIsUriWithIp =FALSE;
	int	hostLen = 0;
	int scopeNum = -1;
	if (strHostIp)
	{
		mcTransportAddress trAddr;
		memset(&trAddr,0,sizeof(mcTransportAddress));
		::stringToIp(&trAddr,strHostIp);
		BYTE isIpAddrValid = ::isApiTaNull(&trAddr);
		if (isIpAddrValid != TRUE)
	    {
			isIpAddrValid = ::isIpTaNonValid(&trAddr);
			if (isIpAddrValid != TRUE)
			bIsUriWithIp = TRUE;
		}
		hostLen	= strHostIp ? strlen(strHostIp) : NO;
		if(bIsUriWithIp)
		{
			char	strTransportIp[IPV6_ADDRESS_LEN];
			int maxStringSizeForKW = min(IPV6_ADDRESS_LEN - 1, hostLen);
			strncpy(strTransportIp, strHostIp, maxStringSizeForKW);
			strTransportIp[maxStringSizeForKW] = 0;
			memset(&trAddr,0,sizeof(mcTransportAddress));
			::stringToIp(&trAddr,strTransportIp);
			if (trAddr.ipVersion == eIpVersion4)
			{
				PTRACE(eLevelInfoNormal,"CSipNetSetup::SetIpVersionAccordingToUri -ipv4");
				m_ipVersion = eIpVersion4;

			}
			else
			{
				m_ipVersion = eIpVersion6;
				scopeNum = trAddr.addr.v6.scopeId;
				scopeNum= ::getScopeId(strTransportIp);
				PTRACE2INT(eLevelInfoNormal,"CSipNetSetup::SetIpVersionAccordingToUri and set source ip according to the scope of ip in uri  -ipv6 - scope id is " ,scopeNum );

			}

		}
		else
		{
			PTRACE(eLevelInfoNormal,"CSipNetSetup::SetIpVersionAccordingToUri -ipv4 - because the host is not with ip -wait to dns");
			if(serviceIpType == eIpType_IpV6)
			{
				m_ipVersion = eIpVersion6;
				scopeNum = eScopeIdGlobal;
			} else
				m_ipVersion = eIpVersion4;
		}

	}
	else
	{
		PTRACE(eLevelInfoNormal,"CSipNetSetup::SetIpVersionAccordingToUri -ipv4 - no host is found!!");
		m_ipVersion = eIpVersion4;
	}
	return scopeNum;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CSipNetSetup::isUriWithDomainInIpFormat()
{
	char*		strHostIp = NULL;
	char* strAt	= strstr(m_strRemoteSipAddress,"@");
	strHostIp	= strAt ? (strAt+1) : NULL;
	BYTE bIsUriWithIp =FALSE;
	int	hostLen = 0;
	int scopeNum = -1;
	if (strHostIp)
	{

		mcTransportAddress trAddr;
		memset(&trAddr,0,sizeof(mcTransportAddress));
		::stringToIp(&trAddr,strHostIp);
		BYTE isIpAddrValid = ::isApiTaNull(&trAddr);
		if (isIpAddrValid != TRUE)
		 {
			isIpAddrValid = ::isIpTaNonValid(&trAddr);
			if (isIpAddrValid != TRUE)
                bIsUriWithIp = TRUE;
		 }
	}

	else
	{
		PTRACE(eLevelInfoNormal,"CSipNetSetup::isUriWithDomainInIpFormat - no host is found!!");

	}
	return bIsUriWithIp;


}
/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetRemoteDisplayName(const char * strDisplay)
{
	if (strDisplay && strcmp(strDisplay, ""))
	{
		strncpy(m_strRemoteDisplayName,strDisplay,MaxDisplaySize - 1);
		m_strRemoteDisplayName[MaxDisplaySize - 1] = 0;
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetRemoteUserAgent(const char * UserAgent)
{
	if (UserAgent && strcmp(UserAgent, ""))
	{
		strncpy(m_strRemoteUserAgent,UserAgent,MaxUserAgentSize - 1);
		m_strRemoteUserAgent[MaxUserAgentSize - 1] = 0;
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetLocalSipAddress(const char * strAddress)
{
	strncpy(m_strLocalSipAddress,strAddress,IP_STRING_LEN - 1);
	m_strLocalSipAddress[IP_STRING_LEN - 1] = 0;
}
////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetOriginalToDmaSipAddress(const char * strAddress)
{
	strncpy(m_strOriginalToHeaderFromDMA,strAddress,IP_STRING_LEN - 1);
	m_strOriginalToHeaderFromDMA[IP_STRING_LEN - 1] = 0;
}
/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetLocalSipAddress(const char* strName,int nameLen,const char* strIp)
{
	nameLen = min(nameLen, (int) strlen(strName));
	if (IP_STRING_LEN > nameLen + 1 + strlen(strIp))
	{
		strncpy(m_strLocalSipAddress,strName,nameLen);
		m_strLocalSipAddress[nameLen] = '@';
		m_strLocalSipAddress[nameLen + 1] = '\0';
		strcat(m_strLocalSipAddress,strIp);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetLocalHost(const char* strHost)
{
	strncpy(m_strLocalHost,strHost,IP_STRING_LEN - 1);
	m_strLocalHost[IP_STRING_LEN - 1] = 0;
}

/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetSrcPartyAddress(const char * strAddress)
{
	strncpy(m_strSrcPartyAddress,strAddress,MaxAddressListSize - 1);
	m_strSrcPartyAddress[MaxAddressListSize - 1] = 0;
}
/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetDestPartyAddress(const char * strAddress)
{
	strncpy(m_strDestPartyAddress,strAddress,MaxAddressListSize - 1);
	m_strDestPartyAddress[MaxAddressListSize - 1] = 0;
}

/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetDestUserName(const char* pDestUser)
{
	strncpy(m_strDestUserName, pDestUser, MaxAddressListSize - 1);
	m_strDestUserName[MaxAddressListSize - 1] = 0;
}

/////////////////////////////////////////////////////////////////////////////
const char* CSipNetSetup::GetDestUserName()
{
	return m_strDestUserName;
}
/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::CopyLocalUriToBuffer(char* buf, int bufSize)
{
	memset(buf, 0, bufSize);
	const char* endName = strstr(m_strLocalSipAddress,"@");
	if (endName && strlen(m_strLocalHost))
	{
		int nameLen = (endName - m_strLocalSipAddress);
		if((nameLen + strlen(m_strLocalHost) + 2) <= (unsigned int)bufSize)//"name@dns"
		{
			strncpy(buf,m_strLocalSipAddress,nameLen); // copy name
			buf[bufSize - 1] = '\0';
			strncat(buf,"@",1);
			strncat(buf,m_strLocalHost,strlen(m_strLocalHost));
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetCallId(const char* strCallId)
{
	strncpy(m_strInviteCallId, strCallId, MaxLengthOfSipCallId - 1);
	m_strInviteCallId[MaxLengthOfSipCallId - 1] = 0;
}

/////////////////////////////////////////////////////////////////////////////
ipAddressStruct* CSipNetSetup::GetIpDnsProxyAddress(enIpVersion ipPerferedVersion)
{
	ipAddressStruct* pProxyAddr = NULL;
	for (int i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES; i++)
	{
		if (m_dnsProxyAddr[i].ipVersion == (DWORD)ipPerferedVersion)
		{
			if (ipPerferedVersion == (DWORD)eIpVersion4 && !(::IsIpNull(&m_dnsProxyAddr[i])))
				return &(m_dnsProxyAddr[i]);
			else if (ipPerferedVersion == (DWORD)eIpVersion6)
			{
				if (m_dnsProxyAddr[i].addr.v6.scopeId == (DWORD)m_ePerferedIpV6ScopeAddr)
					return &(m_dnsProxyAddr[i]);
				else
				{
					for (int j = i; j < TOTAL_NUM_OF_IP_ADDRESSES; j++)
					{
						if (m_dnsProxyAddr[j].ipVersion == (DWORD)ipPerferedVersion && m_dnsProxyAddr[j].addr.v6.scopeId == (DWORD)m_ePerferedIpV6ScopeAddr)
							return &(m_dnsProxyAddr[j]);
					}
				}
				return &(m_dnsProxyAddr[i]);
			}
		}
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetIpDnsProxyAddressArray(ipAddressStruct* pIpAddressDnsArr)
{
	memset(&m_dnsProxyAddr, 0, (sizeof(ipAddressStruct)*TOTAL_NUM_OF_IP_ADDRESSES));
	// Set the Array
	memcpy(m_dnsProxyAddr,pIpAddressDnsArr, (sizeof(ipAddressStruct)*TOTAL_NUM_OF_IP_ADDRESSES));

}

/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetSipLocalMediaType(enIpVersion eLocalMediaIpType)
{
	m_eLocalMediaIpType = eLocalMediaIpType;
}

/////////////////////////////////////////////////////////////////////////////
void  CSipNetSetup::SetAlternativeTaDestPartyAddr(const char* strAltToAddr)
{
	char *maddrStart, *strTmp;
	char strAddrTmp[IP_STRING_LEN];
	memset(m_alternativeTaDestPartyAddr,0,IP_STRING_LEN);

	strncpy(strAddrTmp, strAltToAddr, IP_STRING_LEN - 1);
	strAddrTmp[IP_STRING_LEN - 1] = 0;

	maddrStart = strstr(strAddrTmp, "maddr=");
	if( maddrStart )	//if maddr exist in the message
	{
	    maddrStart += strlen("maddr=");
	    strTmp = maddrStart;
	    while( *strTmp != ';' )
	        strTmp++;

	    *strTmp = '\0';
	    strncpy(m_alternativeTaDestPartyAddr, maddrStart, IP_STRING_LEN - 1);
	    m_alternativeTaDestPartyAddr[IP_STRING_LEN - 1] = 0;
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetEnableSipICE(BOOL isEnableSipICE )
{
	m_EnableSipICE = isEnableSipICE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CSipNetSetup::GetEnableSipICE()
{
	return m_EnableSipICE ;
}

/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetRemoteSipContact(const char * strAddress)
{
	strncpy(m_strRemoteSipContact,strAddress,IP_STRING_LEN - 1);
	m_strRemoteSipContact[IP_STRING_LEN - 1] = 0;
}
//// TIP -----------------------------------------------------------------------
//void CSipNetSetup::SetIsTipCall(BOOL isTipCall )
//{
//	m_bIsTipCall = isTipCall;
//}
///////////////////////////////////////////////////////////////////////////////
//BOOL CSipNetSetup::GetIsTipCall()
//{
//	return m_bIsTipCall ;
//}

/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetToPartyAddress(const char * strAddress)
{
	strncpy(m_strToPartyAddress,strAddress,MaxAddressListSize - 1);
	m_strToPartyAddress[MaxAddressListSize - 1] = 0;
}
/////////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetMsConversationId(const sipSdpAndHeadersSt* pSdpAndHeaders)
{
	if (pSdpAndHeaders->sipHeadersLength)
	{
		BYTE* pStart = (BYTE*) &(pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset]);
		sipMessageHeaders* pHeaders = (sipMessageHeaders*) pStart;
		char msConvId[MS_CONVERSATION_ID_LEN];
		msConvId[0] = '\0';
		::SipGetHeaderValue(pHeaders, kMsConversationId, msConvId, MS_CONVERSATION_ID_LEN);
		SetMsConversationId(msConvId);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetMsConversationId(char* msConvId)
{
	strncpy(m_msConversationId, msConvId, MS_CONVERSATION_ID_LEN - 1);
	m_msConversationId[MS_CONVERSATION_ID_LEN-1] = '\0';

	PTRACE2(eLevelInfoNormal,"CSipNetSetup::SetMsConversationId - ",m_msConversationId);
}
///////////////////////////////////////////////////////////////////////////////////////////
void CSipNetSetup::SetOriginalToFromDmaStr(const sipSdpAndHeadersSt* pSdpAndHeaders)
{
	if (pSdpAndHeaders && pSdpAndHeaders->sipHeadersLength)
	{
		BYTE* pStart = (BYTE*) &(pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset]);
		sipMessageHeaders* pHeaders = (sipMessageHeaders*) pStart;
		const char* pPlcmHeaderStr = NULL;
		if(pHeaders)
		{
			CSipHeaderList headerList(*pHeaders);
			const CSipHeader* pPlcmHeader = headerList.GetNextPrivateOrProprietyHeader(kProprietyHeader, strlen(PLCM_ORGINAL_TO_HEADER), PLCM_ORGINAL_TO_HEADER);

			if (pPlcmHeader)
				pPlcmHeaderStr = pPlcmHeader->GetHeaderStr();
			if (pPlcmHeaderStr)
			{

				pPlcmHeaderStr += sizeof(PLCM_ORGINAL_TO_HEADER); 	// (skipping header, semicolon & space)
				if(pPlcmHeaderStr && pPlcmHeaderStr[0])
				{
					PTRACE2(eLevelInfoNormal,"CSipNetSetup::SetOriginalToFromDmaStr pPlcmHeaderStr - ",pPlcmHeaderStr);
					const char* pReadPtr1 = strstr(pPlcmHeaderStr,":");
					const char* pOriginalToStr = NULL;
					if(pReadPtr1)
					{
						strncpy(m_strOriginalToHeaderFromDMA,pReadPtr1 +1,IP_STRING_LEN-1);
						m_strOriginalToHeaderFromDMA[IP_STRING_LEN-1] = '\0';
					}
					PTRACE2(eLevelInfoNormal,"CSipNetSetup::SetOriginalToFromDmaStr m_strOriginalToHeaderFromDMA - ",m_strOriginalToHeaderFromDMA);

				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

