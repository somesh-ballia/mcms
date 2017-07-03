//+========================================================================+
//                         IpNetS.cpp									   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IpNetS.cpp	                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Atara	                                                   |
//-------------------------------------------------------------------------|
// Who   | Date       | Description                                        |
//-------------------------------------------------------------------------|
// Atara |  09/07/03  | Ip net setup                                       |
//+========================================================================+

#include "IpNetSetup.h"
#include "Segment.h"

//#ifndef _MLANCFG
//#include <mlancfg.h>
//#endif
//
//#ifdef WINNT
//#include "pna.h" //FOR htonl function
//#endif


/////////////////////////////////////////////////////////////////////////////
CIpNetSetup::CIpNetSetup()
{
	m_connectionId			= 0;
	m_callIndex				= 0;
	m_monitorConfId			= 0xFFFFFFFF;
	memset(m_strLocalDisplayName,0,MaxDisplaySize);
	memset(m_strSrcPartyAddress,0,MaxAddressListSize);
	memset(m_strDestPartyAddress,0,MaxAddressListSize);
	m_endpointType			= 0;	
	m_maxRate				= 0;
	m_minRate				= 0xFFFFFFFF;
	m_remoteSetupRate       = 0;
	m_srcUnitId				= 0;
	m_subServiceId			= 0;
	// IpV6
	m_ipVersion = eIpVersion4;
	memset(&m_taSrcPartyAddr,0,sizeof(mcTransportAddress));
	memset(&m_taDestPartyAddr,0,sizeof(mcTransportAddress));
	memset(&m_predefinedIvrStr, 0, ALIAS_NAME_LEN);
	m_csServiceId           = 0;
	m_initialEncryptionValue= AUTO;
	
}

/////////////////////////////////////////////////////////////////////////////
CIpNetSetup::~CIpNetSetup()
{
}

/////////////////////////////////////////////////////////////////////////////
void CIpNetSetup::copy(const CNetSetup * rhs)
{
	*this = *((CIpNetSetup *)rhs);
}
/////////////////////////////////////////////////////////////////////////////
void CIpNetSetup::SetLocalDisplayName(const char * strDisplayName)
{
	strncpy(m_strLocalDisplayName,strDisplayName,sizeof(m_strLocalDisplayName) - 1);
	m_strLocalDisplayName[sizeof(m_strLocalDisplayName) - 1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CIpNetSetup& CIpNetSetup::operator=(const CIpNetSetup& other)
{
	if(this != &other) 
	{
		m_connectionId		= other.m_connectionId;
		m_callIndex			= other.m_callIndex;
		m_monitorConfId		= other.m_monitorConfId;
		SetLocalDisplayName(other.m_strLocalDisplayName);
		SetSrcPartyAddress(other.m_strSrcPartyAddress);
		SetDestPartyAddress(other.m_strDestPartyAddress);
		m_endpointType		= other.m_endpointType; 
		m_maxRate			= other.m_maxRate;
		m_minRate			= other.m_minRate;
		m_remoteSetupRate   = other.m_remoteSetupRate;
		m_srcUnitId			= other.m_srcUnitId;
		m_subServiceId		= other.m_subServiceId;
		
		m_ipVersion			= other.m_ipVersion;
		// IpV6
		memset(&m_taSrcPartyAddr,0,sizeof(mcTransportAddress));
		memcpy(&m_taSrcPartyAddr, &other.m_taSrcPartyAddr, sizeof(mcTransportAddress));
	
		memset(&m_taDestPartyAddr,0,sizeof(mcTransportAddress));
		memcpy(&m_taDestPartyAddr, &other.m_taDestPartyAddr, sizeof(mcTransportAddress));
		memcpy(&m_predefinedIvrStr, &other.m_predefinedIvrStr, ALIAS_NAME_LEN);
		m_csServiceId = other.m_csServiceId;
		m_initialEncryptionValue = other.m_initialEncryptionValue;
		
	}
	return *this;	
}

/////////////////////////////////////////////////////////////////////////////
void CIpNetSetup::Serialize(WORD format,CSegment& seg)
{
	if (format == NATIVE)
	{
		seg << m_connectionId;
		seg << m_callIndex;
		seg << m_monitorConfId;
		
		DWORD len = strlen(m_strLocalDisplayName);
		seg << len;
		seg.Put((BYTE *)m_strLocalDisplayName,len);

		len = strlen(m_strSrcPartyAddress);
		seg << len;
		seg.Put((BYTE *)m_strSrcPartyAddress,len);
		
		len = strlen(m_strDestPartyAddress);
		seg << len;
		seg.Put((BYTE *)m_strDestPartyAddress,len);
		
		seg << m_endpointType;
		seg	<< m_maxRate;
		seg << m_minRate;
		seg << m_remoteSetupRate; 	
		seg << m_srcUnitId;
		seg << m_subServiceId;
		
		// IpV6
		DWORD tmp = (DWORD)m_ipVersion;
		seg << tmp;
		seg << (DWORD)m_taSrcPartyAddr.ipVersion;
		seg << (DWORD)m_taSrcPartyAddr.port;
		seg << (DWORD)m_taSrcPartyAddr.distribution;
		seg << (DWORD)m_taSrcPartyAddr.transportType;
		if ((enIpVersion)m_taSrcPartyAddr.ipVersion == eIpVersion4)
			seg << (DWORD)m_taSrcPartyAddr.addr.v4.ip;
		else
		{
			seg << (DWORD)m_taSrcPartyAddr.addr.v6.scopeId;
			char szIP[64];
			::ipToString(m_taSrcPartyAddr, szIP,0); // With Brackets
			seg << szIP;
		}	
		seg << (DWORD)m_taDestPartyAddr.ipVersion;
		seg << (DWORD)m_taDestPartyAddr.port;
		seg << (DWORD)m_taDestPartyAddr.distribution;
		seg << (DWORD)m_taDestPartyAddr.transportType;
		if ((enIpVersion)m_taDestPartyAddr.ipVersion == eIpVersion4)
			seg << (DWORD)m_taDestPartyAddr.addr.v4.ip;
		else
		{
			seg << (DWORD)m_taDestPartyAddr.addr.v6.scopeId;
			char szIP[64];
			::ipToString(m_taDestPartyAddr, szIP,0); // With Brackets
			seg << szIP;
		}
        len = strlen(m_predefinedIvrStr);
		seg << len;
		seg.Put((BYTE *)m_predefinedIvrStr,len);	
		
		seg << m_csServiceId;
		seg << (DWORD)m_initialEncryptionValue;

	}
}

/////////////////////////////////////////////////////////////////////////////
void CIpNetSetup::DeSerialize(WORD format,CSegment& seg)
{
	if (format == NATIVE)
	{
		DWORD len;
		seg >> m_connectionId;
		seg >> m_callIndex;
		seg >> m_monitorConfId;

		seg >> len;
		seg.Get((BYTE *)m_strLocalDisplayName,len);
		m_strLocalDisplayName[len] = 0;

		seg >> len;
		seg.Get((BYTE *)m_strSrcPartyAddress,len);
		m_strSrcPartyAddress[len] = 0;

		seg >> len;
		seg.Get((BYTE *)m_strDestPartyAddress,len);
		m_strDestPartyAddress[len] = 0;
		
		seg >> m_endpointType;
		seg >> m_maxRate;
		seg >> m_minRate;
		seg >> m_remoteSetupRate; 	
		seg >> m_srcUnitId;		
		seg >> m_subServiceId;
	
		// IpV6
		// IpV6
		DWORD tmp = 0;
		seg >> tmp;

		m_ipVersion = (enIpVersion)tmp;
		seg >> m_taSrcPartyAddr.ipVersion;
		seg >> m_taSrcPartyAddr.port;
		seg >> m_taSrcPartyAddr.distribution;
		seg >> m_taSrcPartyAddr.transportType;
		if ((enIpVersion)m_taSrcPartyAddr.ipVersion == eIpVersion4)
			seg >> m_taSrcPartyAddr.addr.v4.ip;
		else
		{
			seg >> m_taSrcPartyAddr.addr.v6.scopeId ;
			char szIP[64];
			memset(szIP,'\0',64);
			seg >> szIP;
			::stringToIp(&m_taSrcPartyAddr, szIP); // With Brackets
		}
		
		seg >> m_taDestPartyAddr.ipVersion;
		seg >> m_taDestPartyAddr.port;
		seg >> m_taDestPartyAddr.distribution;
		seg >> m_taDestPartyAddr.transportType;
		if ((enIpVersion)m_taDestPartyAddr.ipVersion == eIpVersion4)
			seg >> m_taDestPartyAddr.addr.v4.ip;
		else
		{
			seg >> m_taDestPartyAddr.addr.v6.scopeId ;
			char szIP[64];
			memset(szIP,'\0',64);
			seg >> szIP;
			::stringToIp(&m_taDestPartyAddr, szIP); // With Brackets
		}
        
		seg >> len;
		seg.Get((BYTE *)m_predefinedIvrStr,len);
		m_predefinedIvrStr[len] = 0;
	
		seg >> m_csServiceId;
		seg >> tmp;
		m_initialEncryptionValue = (BYTE)tmp;
		
	}
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void  CIpNetSetup::SetTaSrcPartyAddr(mcTransportAddress* taSrcPartyAddr)
{
	memset(&m_taSrcPartyAddr,0,sizeof(mcTransportAddress));
	memcpy(&m_taSrcPartyAddr, taSrcPartyAddr, sizeof(mcTransportAddress));
	if (taSrcPartyAddr->ipVersion == eIpVersion6)
	{
		enScopeId scopeId = eScopeIdOther;
		char tempName[64];
		memset (&tempName,'\0',IPV6_ADDRESS_LEN);
		::ipToString(*taSrcPartyAddr,tempName,1);
		scopeId  = ::getScopeId(tempName);
		m_taSrcPartyAddr.addr.v6.scopeId = scopeId;
		
	}
    char  strSrcPartyAddress[MaxAddressListSize];
    ::ipToString(*taSrcPartyAddr, strSrcPartyAddress, FALSE);
	SetSrcPartyAddress(strSrcPartyAddress);
    
}

/////////////////////////////////////////////////////////////////////////////
const mcTransportAddress* CIpNetSetup::GetTaSrcPartyAddr() const
{
	return &m_taSrcPartyAddr;
}

/////////////////////////////////////////////////////////////////////////////
void  CIpNetSetup::SetTaDestPartyAddr(mcTransportAddress* taDestPartyAddr)
{
	memset(&m_taDestPartyAddr,0,sizeof(mcTransportAddress));
	memcpy(&m_taDestPartyAddr, taDestPartyAddr, sizeof(mcTransportAddress));
	
	if (taDestPartyAddr->ipVersion == eIpVersion6)
	{
		enScopeId scopeId = eScopeIdOther;
		char tempName[64];
		memset (&tempName,'\0',IPV6_ADDRESS_LEN);
		::ipToString(*taDestPartyAddr,tempName,1);
		scopeId  = ::getScopeId(tempName);
		m_taDestPartyAddr.addr.v6.scopeId = scopeId;
		
	}
    char  strDestPartyAddress[MaxAddressListSize];
    ::ipToString(*taDestPartyAddr, strDestPartyAddress, FALSE);
    SetDestPartyAddress(strDestPartyAddress);
}

/////////////////////////////////////////////////////////////////////////////
void  CIpNetSetup::SetTaSrcPartyAddr(const mcTransportAddress* taSrcPartyAddr)
{
	memset(&m_taSrcPartyAddr,0,sizeof(mcTransportAddress));
	memcpy(&m_taSrcPartyAddr, taSrcPartyAddr, sizeof(mcTransportAddress));
	if (taSrcPartyAddr->ipVersion == eIpVersion6)
	{
		enScopeId scopeId = eScopeIdOther;
		char tempName[64];
		memset (&tempName,'\0',IPV6_ADDRESS_LEN);
		::ipToString(*taSrcPartyAddr,tempName,1);
		scopeId  = ::getScopeId(tempName);
		m_taSrcPartyAddr.addr.v6.scopeId = scopeId;
		
	}
    char  strSrcPartyAddress[MaxAddressListSize];
    ::ipToString(*taSrcPartyAddr, strSrcPartyAddress, FALSE);
    SetSrcPartyAddress(strSrcPartyAddress);
}

/////////////////////////////////////////////////////////////////////////////
void  CIpNetSetup::SetTaDestPartyAddr(const mcTransportAddress* taDestPartyAddr)
{
	memset(&m_taDestPartyAddr,0,sizeof(mcTransportAddress));
	memcpy(&m_taDestPartyAddr, taDestPartyAddr, sizeof(mcTransportAddress));
	if (taDestPartyAddr->ipVersion == eIpVersion6)
	{
		enScopeId scopeId = eScopeIdOther;
		char tempName[64];
		memset (&tempName,'\0',IPV6_ADDRESS_LEN);
		::ipToString(*taDestPartyAddr,tempName,1);
		scopeId  = ::getScopeId(tempName);
		m_taDestPartyAddr.addr.v6.scopeId = scopeId;
		
	}
    char  strDestPartyAddress[MaxAddressListSize];
    ::ipToString(*taDestPartyAddr, strDestPartyAddress, FALSE);
    SetDestPartyAddress( strDestPartyAddress);
}
/////////////////////////////////////////////////////////////////////////////
const mcTransportAddress* CIpNetSetup::GetTaDestPartyAddr() const
{
	return &m_taDestPartyAddr;
}
/////////////////////////////////////////////////////////////////////////////
void  CIpNetSetup::SetIpVersion(enIpVersion ipVersion)
{
	m_ipVersion = ipVersion;
}
/////////////////////////////////////////////////////////////////////////////
enIpVersion CIpNetSetup::GetIpVersion()
{
	return m_ipVersion;
}

/////////////////////////////////////////////////////////////////////////////
void  CIpNetSetup::SetPredefiendIvrStr(const char * predefiendIvrStr)
{
    memset(&m_predefinedIvrStr, 0, ALIAS_NAME_LEN);
    strcat(m_predefinedIvrStr, "#");
    if(strlen(predefiendIvrStr) < ALIAS_NAME_LEN - 1)
        strncat(m_predefinedIvrStr, predefiendIvrStr, ALIAS_NAME_LEN-2);
    else
       PASSERTMSG(1, "predefinedIvrStr exceed ALIAS_NAME_LEN - 1");  		
		
}
/////////////////////////////////////////////////////////////////////////////
const char *  CIpNetSetup::GetPredefiendIvrStr()
{
    return m_predefinedIvrStr; 
}
