//+========================================================================+
//                         SipNetSetup.h									   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SipNetSetup.h	                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Atara	                                                   |
//-------------------------------------------------------------------------|
// Who   | Date       | Description                                        |
//-------------------------------------------------------------------------|
// Atara |  09/07/03  | Sip net setup                                      |
//+========================================================================+

#ifndef _SIPNETSETUP
#define _SIPNETSETUP

#include "IpNetSetup.h"
#include "SipDefinitions.h"
#include "ConfPartySharedDefines.h"
#include "IpAddress.h"

//#include <SipDefs.h>

class CSipNetSetup : public CIpNetSetup
{
CLASS_TYPE_1(CSipNetSetup,CIpNetSetup )
public:
	// Constructors
	CSipNetSetup();
	virtual ~CSipNetSetup();

	// Initializations

    // Operations
	virtual const char*  NameOf() const;
	virtual CIpNetSetup& operator=(const CIpNetSetup& other);
	virtual void copy(const CNetSetup * rhs);
	virtual void Serialize(WORD format,CSegment& seg);
	virtual void DeSerialize(WORD format,CSegment& seg);

	WORD GetRemoteSipAddressType() const {return m_remoteSipAddressType;}
	const char* GetRemoteSipAddress() const {return m_strRemoteSipAddress;}
	const char* GetRemoteDisplayName() const {return m_strRemoteDisplayName;}
	const char* GetRemoteUserAgent() const {return m_strRemoteUserAgent;}
	WORD GetLocalSipAddressType() const {return m_localSipAddressType;}
	const char* GetLocalSipAddress() const {return m_strLocalSipAddress;}
	const char* GetOriginalToDmaSipAddress() const {return m_strOriginalToHeaderFromDMA;}
	const char* GetLocalHost() const {return m_strLocalHost;}
	void CopyLocalUriToBuffer(char* buf,int bufSize);
	void SetRemoteSipAddressType(WORD type) {m_remoteSipAddressType = type;}
	void SetRemoteSipAddress(const char* strAddress);
	void SetRemoteDisplayName(const char* strDisplay);
	void SetRemoteUserAgent(const char * UserAgent);
	void SetLocalSipAddressType(WORD type) {m_localSipAddressType = type;}
	void SetLocalSipAddress(const char* strAddress);

	void SetLocalSipAddress(const char* strName,int nameLen,const char* strIp);
	void SetLocalHost(const char* strHost);
	virtual void  SetSrcPartyAddress(const char* strAddress);
	virtual void  SetDestPartyAddress(const char* strAddress);
	void SetDestUserName(const char* pDestUser);
	const char* GetDestUserName();
	void SetCallId(const char* strCallId);
	const char* GetCallId() const {return m_strInviteCallId;}
	WORD GetRemoteSignallingPort() const {return m_remoteSignalingPort;}
	void SetRemoteSignallingPort(WORD port) {m_remoteSignalingPort = port;}
	void SetRemoteVideoPartyType(eVideoPartyType eRemoteVideoPartyType) {m_eRemoteVideoPartyType = eRemoteVideoPartyType;}
	eVideoPartyType GetRemoteVideoPartyType() const {return m_eRemoteVideoPartyType;}
	int SetIpVersionAccordingToUri(DWORD serviceIpType = eIpType_IpV4);
	BYTE isUriWithDomainInIpFormat();
	void SetPerferedIpV6ScopeAddr(enScopeId ePerferedIpV6ScopeAddr) {m_ePerferedIpV6ScopeAddr = ePerferedIpV6ScopeAddr;}
	enScopeId GetPerferedIpV6ScopeAddr() const {return m_ePerferedIpV6ScopeAddr;}
	ipAddressStruct* GetIpDnsProxyAddress(enIpVersion ipPerferedVersion);
	void SetIpDnsProxyAddressArray(ipAddressStruct* pIpAddressDnsArr);
	enIpVersion GetSipLocalMediaType() { return m_eLocalMediaIpType;};
	void SetSipLocalMediaType(enIpVersion eLocalMediaIpType);
	void  SetAlternativeTaDestPartyAddr(const char* strAddress);
	const char* GetAlternativeTaDestPartyAddr() {return m_alternativeTaDestPartyAddr;}

	BOOL GetEnableSipICE();
	void SetEnableSipICE(BOOL isEnableSipICE );
	const char* GetRemoteSipContact() const {return m_strRemoteSipContact;}
	void SetRemoteSipContact(const char* strAddress);

	void  SetToPartyAddress(const char* strAddress);
	const char * GetToPartyAddress() const {return m_strToPartyAddress;}

	void SetMsConversationId(const sipSdpAndHeadersSt* pSdpAndHeaders);
	void SetMsConversationId(char* msConvId);
	const char * GetMsConversationId() const {return m_msConversationId;}


	/////
	void SetOriginalToFromDmaStr(const sipSdpAndHeadersSt* pSdpAndHeaders);
	void SetOriginalToDmaSipAddress(const char * strAddress);

	BOOL GetIsMsConfInvite() {return m_bIsMsConfInvite;}
	void SetIsMsConfInvite(BOOL isMsConfInvite ){m_bIsMsConfInvite = isMsConfInvite;}


	void  SetSIPConfIdAsGUID(const char* confId) { memcpy(m_conferenceId, confId, MaxConferenceIdSize);	}
	const char* GetSIPConfIdAsGUID()			{ return m_conferenceId;		}

	// TIP
//	void SetIsTipCall(BOOL);
//	BOOL GetIsTipCall();

protected:
	WORD m_remoteSipAddressType;
	char m_strRemoteSipAddress[IP_STRING_LEN];
	WORD m_remoteSignalingPort;
	char m_strRemoteDisplayName[MaxDisplaySize];
	char m_strRemoteUserAgent[MaxUserAgentSize];
	WORD m_localSipAddressType;
	char m_strLocalSipAddress[IP_STRING_LEN];   // user@ip
	char m_strLocalHost[IP_STRING_LEN];
	char m_strDestUserName[MaxAddressListSize];
	char m_strInviteCallId[MaxLengthOfSipCallId];
	char m_strOriginalToHeaderFromDMA[IP_STRING_LEN];   // user@ip as DMA go from EP
	eVideoPartyType m_eRemoteVideoPartyType;



	enScopeId  m_ePerferedIpV6ScopeAddr;
	ipAddressStruct m_dnsProxyAddr[TOTAL_NUM_OF_IP_ADDRESSES];
	enIpVersion		m_eLocalMediaIpType;
	char m_alternativeTaDestPartyAddr[IP_STRING_LEN];
	BYTE m_EnableSipICE;
	char m_strRemoteSipContact[IP_STRING_LEN];

	char  m_strToPartyAddress[MaxAddressListSize];

	char  m_msConversationId[MS_CONVERSATION_ID_LEN];

	BOOL m_bIsMsConfInvite;
	char  m_conferenceId[MaxConferenceIdSize];
	//BOOL m_bIsTipCall;
};


#endif //_SIPNETSETUP
