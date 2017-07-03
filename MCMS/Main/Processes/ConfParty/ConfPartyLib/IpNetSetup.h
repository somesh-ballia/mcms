//+========================================================================+
//                         IpNetS.h										   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IpNetS.h		                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Atara	                                                   |
//-------------------------------------------------------------------------|
// Who   | Date       | Description                                        |
//-------------------------------------------------------------------------|
// Atara |  09/07/03  | Ip net setup                                       |
//+========================================================================+

#ifndef _IPNETSETUP
#define _IPNETSETUP

#include "NetSetup.h"
#include "IPUtils.h"
#include "SystemFunctions.h"


class CIpNetSetup : public CNetSetup
{
CLASS_TYPE_1(CIpNetSetup,CNetSetup )
public:
	// Constructors
	CIpNetSetup(); 
	virtual ~CIpNetSetup();
	virtual const char* NameOf() const { return "CIpNetSetup";}
	
	// Initializations  
	
    // Operations
	virtual CIpNetSetup& operator=(const CIpNetSetup& other);
	virtual void copy(const CNetSetup * rhs); 
	virtual void Serialize(WORD format,CSegment& seg);
	virtual void DeSerialize(WORD format,CSegment& seg);

	DWORD GetConnectionId() const {return m_connectionId;}
	DWORD GetCallIndex() const {return m_callIndex;}
	DWORD GetConfId() const {return m_monitorConfId;}
	const char * GetLocalDisplayName() const {return m_strLocalDisplayName;}
	const char * GetSrcPartyAddress() const {return m_strSrcPartyAddress;}
	const char * GetDestPartyAddress() const {return m_strDestPartyAddress;}
 	BYTE  GetEndpointType() const {return m_endpointType;}
	DWORD GetMaxRate() const {return m_maxRate;}
	DWORD GetMinRate() const {return m_minRate;}
	DWORD GetRemoteSetupRate() const {return m_remoteSetupRate;}
	DWORD GetSubServiceId() { return m_subServiceId;}
	BOOL  IsItPrimaryNetwork() { return (GetSubServiceId() ? FALSE: TRUE);}

	DWORD GetCsServiceid() const {return m_csServiceId;}
	// IpV6
	void  SetTaSrcPartyAddr(mcTransportAddress* taSrcPartyAddr);
	const mcTransportAddress* GetTaSrcPartyAddr() const;
	void  SetTaDestPartyAddr(mcTransportAddress* taDestPartyAddr);
	const mcTransportAddress* GetTaDestPartyAddr() const;
	void  SetIpVersion(enIpVersion ipVersion);
	enIpVersion GetIpVersion();
	void  SetTaSrcPartyAddr(const mcTransportAddress* taSrcPartyAddr);
	void  SetTaDestPartyAddr(const mcTransportAddress* taDestPartyAddr);
	virtual void  SetDestPartyAliases(const char* destPartyAlias) {}
    
	void  SetConnectionId(WORD connectionId) {m_connectionId = connectionId;}
	void  SetCallIndex(DWORD index) {m_callIndex = index;}
	void  SetConfId(DWORD confId) {m_monitorConfId = confId;}
	void  SetSubServiceId(DWORD subServiceId) { m_subServiceId = subServiceId;};
	
	void  SetLocalDisplayName(const char * strDisplayName = AccordVendorId); 
	void  SetEndpointType(BYTE endpointType) {m_endpointType = endpointType;}
	void  SetMaxRate(DWORD rate) {m_maxRate = rate;}
	void  SetMinRate(DWORD rate) {m_minRate = rate;}
	void  SetRemoteSetupRate(DWORD remoteSetupRate) {m_remoteSetupRate = remoteSetupRate;}
	void  SetCsServiceid(DWORD csServiceId) {m_csServiceId = csServiceId;}

	virtual void  SetSrcPartyAddress(const char* strAddress)  = 0; 
	virtual void  SetDestPartyAddress(const char* strAddress) = 0; 
	
	void SetSrcUnitId(APIU16 srcUnitId) { m_srcUnitId = srcUnitId; }
	APIU16 GetSrcUnitId() { return m_srcUnitId;}
    void  SetPredefiendIvrStr(const char * predefiendIvrStr);
    const char * GetPredefiendIvrStr();
    void  SetInitialEncryptionValue(BYTE initialEncryptionValue) {m_initialEncryptionValue = initialEncryptionValue;}
    BYTE GetInitialEncryptionValue() {return m_initialEncryptionValue;}

protected:

	WORD  m_connectionId;
	DWORD m_callIndex;
	DWORD m_monitorConfId;
	char  m_strLocalDisplayName[MaxDisplaySize];
	char  m_strSrcPartyAddress[MaxAddressListSize];
	char  m_strDestPartyAddress[MaxAddressListSize];
	BYTE  m_endpointType;
	DWORD m_maxRate;
	DWORD m_minRate;
	DWORD m_remoteSetupRate;
	// IpV6
	mcTransportAddress	m_taSrcPartyAddr;
	mcTransportAddress	m_taDestPartyAddr;
	enIpVersion			m_ipVersion;
	DWORD m_srcUnitId;
	DWORD m_subServiceId;
    char m_predefinedIvrStr[ALIAS_NAME_LEN];
	
	DWORD m_csServiceId;
	BYTE m_initialEncryptionValue; //YES,NO,AUTO
};



#endif //_IPNETSETUP
