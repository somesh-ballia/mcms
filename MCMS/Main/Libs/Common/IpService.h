

//+========================================================================+
//                                                             |
//          Copyright 1995 Pictel Technologies Ltd.                        |
//                      All Rights Reserved.                               |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:      NH323CFG.h                                                   |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Sergey                                                      |
//-------------------------------------------------------------------------|
// Who | Date      | Description                                           |
//-------------------------------------------------------------------------|
//     | 21.5.98   | created                                               |
//+========================================================================+

#ifndef _IpService_
#define _IpService_



#include <stdio.h>
#include "psosxml.h"
#include "SerializeObject.h"
#include "StringsMaps.h"
#include "DefinesIpService.h"
#include "InitCommonStrings.h"
#include "ObjString.h"
#include "McuMngrStructs.h"
#include "DataTypes.h"
#include "H323Alias.h"
#include "DynIPSProperties.h"
#include "InternalProcessStatuses.h"
#include "ApiStatuses.h"
#include "AllocateStructs.h"
#include "ServiceConfig.h"
#include "WhiteList.h"

#define IP_SERVICE_LIST_PATH 						"Cfg/IPServiceList.xml"
#define IP_SERVICE_LIST_TMP_PATH 					"Cfg/IPServiceListTmp.xml"

#define IP_MULTIPLE_SERVICES_LIST_PATH 				"Cfg/IPMultipleServicesList.xml"
#define IP_MULTIPLE_SERVICES_LIST_TMP_PATH 			"Cfg/IPMultipleServicesListTmp.xml"

#define IP_MULTIPLE_SERVICES_TWO_SPANS_LIST_PATH 	"Cfg/IPMultipleServicesTwoSpansList.xml"
#define IP_MULTIPLE_SERVICES_TWO_SPANS_LIST_TMP_PATH "Cfg/IPMultipleServicesTwoSpansListTmp.xml"

#define IP_SERVICES_JITC_V35_LIST_PATH 				"Cfg/IPServicesJitcV35List.xml"
#define IP_SERVICES_JITC_V35_LIST_TMP_PATH 			"Cfg/IPServicesJitcV35ListTmp.xml"

#define IP_VERSION_CFG_SOFT_MCU_SERVICES_LIST_PATH  "VersionCfg/SoftMcuDefaultIPServiceList.xml"

#define DEFAULT_SERVICE_NONE 0
#define DEFAULT_SERVICE_H323 1
#define DEFAULT_SERVICE_SIP 2
#define DEFAULT_SERVICE_BOTH 3

#define IP_MULTIPLE_SERVICES_LIST_PATH 	"Cfg/IPMultipleServicesList.xml"
#define IP_MULTIPLE_SERVICES_LIST_TMP_PATH 	"Cfg/IPMultipleServicesListTmp.xml"

#define DEFAULT_SERVICE_NONE 0
#define DEFAULT_SERVICE_H323 1
#define DEFAULT_SERVICE_SIP 2
#define DEFAULT_SERVICE_BOTH 3

// const WORD FIRST_UDP_PORT =	49152;
const WORD FIRST_TCP_PORT = 49152;
const BYTE NUM_OF_UDP_SECTIONS = 4 ;


const DWORD MAX_PREDECEDENCE_AUDIO = 63;
const DWORD MAX_PREDECEDENCE_VIDEO = 63;

const DWORD MAX_IP_SPAN = 9;


class CIPServiceFullList;
class CDynIPSProperties;
class CIpNat;
class CIPServiceList;



/*
///////////////////////////////////////////////////////////////////////////////////////////////////
//Class CAtmAddr
////////////////////////////////////////////////////////////////////////////////////////////////////

#define ATM_ADDRESS_LEN                20  // 20 bytes


class CAtmAddr : public CPObject
{
CLASS_TYPE_1(CAtmAddr, CPObject)
public:
	   //Constructors
	CAtmAddr();
	CAtmAddr( const CAtmAddr &other );
	CAtmAddr&  operator=( const CAtmAddr& other );

	virtual ~CAtmAddr() ;

      // Implementation
//	void        Serialize( WORD format, std::ostream  &m_ostr );
//    void        DeSerialize( WORD format, std::istream &m_istr );

	char*		GetAsString(char* szAddressString) const;
	int			SetFromString(char* szAddressString);

      // Attributes
	BYTE         m_uniaddr[ATM_ADDRESS_LEN];
};

*/







/////////////////////////////////////////////////////////////////////////////////////////////////////
//					Class HostName                                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////
class HostName : public CSmallString
{
	CLASS_TYPE_1(HostName, CSmallString)
public:
	HostName(const char * const str="");
	HostName(const DWORD address);
	HostName(const CObjString& stringSrc);
	DWORD IpToDWORD() const;
	BYTE  IsValid(BYTE isDns) const;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
//							CH323Router
///////////////////////////////////////////////////////////////////////////////////////////////////
class CH323Router : public CPObject
{
CLASS_TYPE_1(CH323Router, CPObject)
public:
	   //Constructors
    CH323Router();
    CH323Router( const CH323Router &other );
    CH323Router&  operator=(const CH323Router& other);
    bool operator==(const CH323Router &rHnd)const;
    bool operator!=(const CH323Router& other)const;
    
    virtual ~CH323Router() ;
    
    virtual const char* NameOf() const { return "CH323Router";}
    // Implementation
    void   SerializeXml(CXMLDOMElement* pFatherNode);
    int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

    bool   IsDefault()const;

    DWORD  GetRouterIP() const;
    void   SetRouterIP(const DWORD routerIP);
    DWORD  GetRemoteIP() const;
    void   SetRemoteIP(const DWORD remoteIP);
    BYTE   GetRemoteFlag() const;
    void   SetRemoteFlag(const BYTE remoteFlag);
    DWORD  GetSubnetMask() const;
    void   SetSubnetMask(const DWORD  subnetMask);

private:
    // Attributes
    DWORD  m_routerIP;
    DWORD  m_remoteIP;
    DWORD  m_subnetMask;
    BYTE   m_remoteFlag;
};
////////////////////////////////////////////////////////////////////////////////////////////////
//							CCommH323PortRange
///////////////////////////////////////////////////////////////////////////////////////////////

class CCommH323PortRange : public CPObject
{
CLASS_TYPE_1(CCommH323PortRange, CPObject)
public:

    CCommH323PortRange();
    virtual ~CCommH323PortRange();
    CCommH323PortRange(const CCommH323PortRange&);
    CCommH323PortRange&  operator=(const CCommH323PortRange& other);

    virtual const char* NameOf() const { return "CCommH323PortRange";}
    
    bool operator==(const CCommH323PortRange &rHnd)const;
    bool operator!=(const CCommH323PortRange& other)const;

    void  SerializeXml(CXMLDOMElement* pFatherNode);
    int   DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

    void SetSignallingPortRange(WORD firstPort, WORD numPorts);
    WORD GetSignallingNumberOfPorts() const;
    WORD GetSignallingFirstPort() const;

    void SetControlPortRange(WORD firstPort, WORD numPorts);
    WORD GetControlNumberOfPorts() const;
    WORD GetControlFirstPort() const;

    void SetAudioPortRange(WORD firstPort, WORD numPorts);
    WORD GetAudioNumberOfPorts() const;
    WORD GetAudioFirstPort() const;

    void SetVideoPortRange(WORD firstPort, WORD numPorts);
    WORD GetVideoNumberOfPorts() const;
    WORD GetVideoFirstPort() const;

    void SetContentPortRange(WORD firstPort, WORD numPorts);
    WORD GetContentNumberOfPorts() const;
    WORD GetContentFirstPort() const;
    void SetFeccPortRange(WORD firstPort, WORD numPorts);
    WORD GetFeccNumberOfPorts() const;
    WORD GetFeccFirstPort() const;

    void SetDynamicPortAllocation(WORD yesNo);
    WORD IsDynamicPortAllocation() const;
    WORD GetNumIntendedCallsOnCard() const;
    void SetNumIntendedCallsOnCard(WORD number);

    void SetEnablePortRange(WORD yesNo);
    WORD IsEnabledPortRange() const;
	WORD IsEqual(CCommH323PortRange& other);

    ESTATUS TestValidity() const;

//    void SetTcpPortRange(WORD firstPort, WORD numPorts);
    WORD GetTcpNumberOfPorts() const;
    WORD GetTcpFirstPort() const;

    void SetUdpPortRange(WORD firstPort, WORD numPorts);
    WORD GetUdpNumberOfPorts() const;
    WORD GetUdpFirstPort() const;
    void SetDefaultTcpPorts(DWORD maxPartiesNum);

private:
    void SetDefaultUdpPorts();

//	void DivideUdpPorts(WORD udpFirstPort, WORD udpNumPorts);


    WORD      m_dynamicPortAllocation;
    WORD      m_signallingFirstPort;
    WORD      m_signallingNumPorts;
    WORD      m_controlFirstPort;
    WORD      m_controlNumPorts;
    WORD      m_audioFirstPort;
    WORD      m_audioNumPorts;
    WORD      m_videoFirstPort;
    WORD      m_videoNumPorts;
    WORD      m_contentFirstPort;
    WORD      m_contentNumPorts;
	WORD	  m_feccFirstPort;
	WORD	  m_feccNumPorts;
    WORD      m_numIntendedCalls;
    // API API_NUM_H323_ENABLE_PORTRANGE
    WORD      m_enablePortRange;

    WORD      m_TcpFirstPort;
    WORD      m_TcpNumPorts;
    WORD      m_UdpFirstPort;
    WORD      m_UdpNumPorts;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//							CIPSpan
///////////////////////////////////////////////////////////////////////////////////////////////////
class CIPSpan : public CPObject
{
	CLASS_TYPE_1(CIPSpan, CPObject)
public:
    //Constructors
    CIPSpan();
    CIPSpan( const CIPSpan &other );
    CIPSpan&  operator=(const CIPSpan& other);
    bool operator==(const CIPSpan& other) const;
    bool operator!=(const CIPSpan& other)const;

    virtual ~CIPSpan() ;
    virtual const char* NameOf() const { return "CIPSpan";}
    // Implementation
    void        SerializeXml(CXMLDOMElement* pFatherNode);
    int         DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);


    WORD        GetLineNumber() const;
    void        SetLineNumber(const WORD lineNumber);

	DWORD       GetIPv4Address() const;
    void        SetIPv4Address(const DWORD ipV4Address);
	void		GetIPv6Address(int idx, char* retStr, BOOL isBrackets=FALSE) const;
	
	const APIU8*	GetIPv6AddressByteArray(int idx) const;
	
    std::string GetIPv6Address(int idx=0, BOOL isBrackets=FALSE) const;

    // return ipv6 address including subnet mask at the end
    void		GetFullIPv6Address(int idx, char* retStr, BOOL isBrackets=FALSE) const;
    std::string GetFullIPv6Address(int idx, BOOL isBrackets=FALSE) const;


	void		SetIPv6Address(int idx, const char* ipV6Address);

	DWORD		GetIPv6SubnetMask(int idx) const;
	void		GetIPv6SubnetMaskStr(int idx, char *pOutMask) const;
	void		SetIPv6SubnetMask(int idx, const DWORD subnetMask);
	void		SetIPv6SubnetMask(int idx, const char *pMask);

	bool		GetIsIpV4Null() const;
	bool		GetIsIpV6Null(int idx) const;

    WORD        GetRASport() const;
    void        SetRASport(const WORD RASport);
    WORD        GetCallSignalPort() const;
    void        SetCallSignalPort(const WORD callSignalPort);


    const       CSmallString& GetSIPName() const;
    void	    SetSIPName(const  CSmallString& SIP_Name);
    enum        eIPSpanType{ eIPSpanType_URI=0};
    void        SetIPSpanType(const eIPSpanType IPSpanType);
    eIPSpanType GetIPSpanType (){return m_eIPSpanType;}

    const       CSmallString& GetSpanHostName() const;
    void        SetSpanHostName(const CSmallString& Host_Name);

    WORD        GetAliasNamesNumber() const;
    void        SetAliasNamesNumber( const WORD num );

    int      AddAlias(const CH323Alias& other);
    int      UpdateAlias(const CH323Alias& other);
    int      CancelAlias(const char* aliasName);
    int 	 CancelAlias(int ind);
    int 	 CancelAliasList();
    int      FindAlias(const CH323Alias& other);
    int      FindAlias(const char* aliasName);
    int      ReplaceAliasList(CH323Alias* otherAliasList[], int listLen);

    CH323Alias** GetAliasList();
    CH323Alias*  GetFirstAlias();
    CH323Alias*  GetNextAlias();
    CH323Alias*  GetCurrentAlias(const char* aliasName);
    CH323Alias*  GetAlias(WORD ind);

    WORD         GetSpeed() const;
    void         SetSpeed( const WORD speed );

    /* ANNEX C - remove
    void		   SetUniProtocolVersion(const unsigned short uniProtocolVersion );
    unsigned short GetUniProtocolVersion()const;*/
    void		   SetMaxNTUSize(const unsigned short maxNTUSize);
    unsigned short GetMaxNTUSize()const;
    /*    void		   SetIlmiEnabled(const unsigned short ilmiEnabled);
    unsigned short GetIlmiEnabled()const;
    void		   SetCardATMIlmiAddress(const CAtmAddr& cardAtmIlmiAddress);
    CAtmAddr*	  GetCardATMIlmiAddress()const;
    end ANNEX C*/

    CIpNat* GetIpNat();
    void SetNat(const CIpNat&);

	CCommH323PortRange* GetPortRange() const;
    void SetPortRange(const CCommH323PortRange& PortRange);
    
    BOOL GetIsSpanEnable();
    void SetIsSpanEnable(BOOL is_span_enabled);

    void SetInterface(std::string interface);
    std::string GetInterface();

    ipv6AddressArray	m_IPv6AaddressArray;

protected:
    // Attributes
    WORD        m_lineNumber;
    WORD        m_numb_of_names;
    CH323Alias* m_h323alias[MAX_ALIAS_NAMES_NUM];
    WORD        m_RASport;
    WORD        m_callSignalPort;

    ipAddressV4If		m_IPv4Address;
    DWORD				m_IPv6MaskArray[NUM_OF_IPV6_ADDRESSES];


    // API - 20
    WORD        m_speed;

    //API - API_NUM_H323_ATM_2
    unsigned short       m_uniVersion;
    unsigned short       m_maxNTUSize;
    unsigned char        m_IlmiEnabled;
//    CAtmAddr*            m_pCardATMIlmiAddress;
    CCommH323PortRange*  m_pPortRange;

    //sip
    HostName      m_SIP_Name;
    eIPSpanType   m_eIPSpanType;
    HostName      m_Host_Name;
    CIpNat*       m_pNat;
    
    BOOL		  m_bIsSpanEnabled;

    std::string	  m_Interface;

private:
    WORD          m_ind_name;
}; // CIPSpan

///////////////////////////////////////////////////////////////////////////////////////////////////
//				class CQualityOfService
/////////////////////////////////////////////////////////////////////////////////////////////////
class CQualityOfService : public CPObject
{
CLASS_TYPE_1(CQualityOfService,CPObject)
public:
    CQualityOfService();
    CQualityOfService(const CQualityOfService& other);
    ~CQualityOfService();

    CQualityOfService& operator= (const CQualityOfService& other);
    bool operator==(const CQualityOfService &rHnd)const;
    bool operator!=(const CQualityOfService& other)const;

    // base class interface
    virtual const char* NameOf() const { return "CQualityOfService";}
    
    // methods
    void  TestValidity();
    void  SerializeXml(CXMLDOMElement* pFatherNode);
    int   DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

    // get IP fields
    void        SetQosIp(const BYTE status, const BYTE isDiffService, const BYTE tosValue,
                         const BYTE  audioPriority, const BYTE videoPriority);

    BYTE  GetIpStatus()const   { return (BYTE)m_eIpStatus; }
    BYTE  GetIpDiffServ()const { return m_bIpIsDiffServ; }
    BYTE  GetIpTos()const      { return m_bIpValueTOS; }
    BYTE  GetIpAudPrec()const  { return m_bIpPrecedenceAudio; }
    BYTE  GetIpVidPrec()const  { return m_bIpPrecedenceVideo; }
    // set IP fields
    void  SetIpStatus(const int status);
    void  SetIsDiffServ(const BYTE isDiffServ);
    void  SetIpValueTOS(const BYTE tos);
    void  SetIpPrecedenceAudio(const BYTE aud);
    void  SetIpPrecedenceVideo(const BYTE vid);


    // get ATM fields
    void  SetQosAtm(const BYTE status, const BYTE audioPriority, const BYTE videoPriority);
    BYTE  GetAtmStatus()const  { return (BYTE)m_eAtmStatus; }
    BYTE  GetAtmAudPrec()const { return m_bAtmPrecedenceAudio; }
    BYTE  GetAtmVidPrec()const { return m_bAtmPrecedenceVideo; }
    // set ATM fields
    void  SetAtmStatus(const int status);
    void  SetAtmPrecedenceAudio(const BYTE aud);
    void  SetAtmPrecedenceVideo(const BYTE vid);

//#ifdef __HIGHC__
    void GetAllQoS(const char* pServiceProviderName, CSegment& seg) const;
//#endif

    // types of status
    enum  eStatusQoS { eQoS_disable = 0, eQoS_enable, eQoS_service, eQoS_unknown/*must be last*/ };
    // types of media
    enum  eMediaTypes { eMediaTypeAudio = 0, eMediaTypeVideo, eMediaTypeRtcp, eMediaTypeUnknown/*must be last*/ };

    // WARNING : these functions NOT return fields of class,
    //         them create BYTE of QOS by the standard
    //
    // get QoS for IP call party
    BYTE  GetIpAudioQoS(const char* pService) const
    { return GetIpMediaQoS(eMediaTypeAudio,pService); }
    BYTE  GetIpVideoQoS(const char* pService) const
    { return GetIpMediaQoS(eMediaTypeVideo,pService); }
    BYTE  GetIpRtcpQoS (const char* pService) const
    { return GetIpMediaQoS(eMediaTypeRtcp,pService);  }
    // get QoS for ATM call party
    BYTE  GetAtmAudioQoS(const char* pService) const
    { return GetAtmMediaQoS(eMediaTypeAudio,pService); }
    BYTE  GetAtmVideoQoS(const char* pService) const
    { return GetAtmMediaQoS(eMediaTypeVideo,pService); }

protected:

    // utilities
    BYTE  GetIpMediaQoS  ( const eMediaTypes media, const char* pServiceProviderName ) const;
    BYTE  GetAtmMediaQoS ( const eMediaTypes media, const char* pServiceProviderName ) const;

    // IP QoS fields
    eStatusQoS  m_eIpStatus;            // may be Disable / Enable / Take From Service
    BYTE        m_bIpIsDiffServ;        // may be NO / YES
    BYTE        m_bIpValueTOS;          // may be 0x0 / 0x8 (for D value of TOS - minimize delay)
    BYTE        m_bIpPrecedenceAudio;   // may be [0..5] - priority for audio channel packets, may be every thing
    BYTE        m_bIpPrecedenceVideo;   // may be [0..5] - priority for video channel packets, may be every thing

    // ATM QoS fields
    eStatusQoS  m_eAtmStatus;           // may be Disable / Enable / Take From Service
    BYTE        m_bAtmPrecedenceAudio;  // may be [1..5] - priority for audio
    BYTE        m_bAtmPrecedenceVideo;  // may be [1..5] - priority for video
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//						class CIpNat
//////////////////////////////////////////////////////////////////////////////////////////////////
class CIpNat: public CPObject
{
CLASS_TYPE_1(CIpNat,CPObject)
public:

    //Constructors
    CIpNat();
    CIpNat(const CIpNat& other);
    CIpNat&  operator=(const CIpNat& other);
    bool operator==(const CIpNat &rHnd)const;
    bool operator!=(const CIpNat& other)const;

    ~CIpNat();
    virtual const char* NameOf() const { return "CIpNat";}
    // Implementation
    void       SerializeXml( CXMLDOMElement *pParentNode );
    int        DeSerializeXml( CXMLDOMElement *pParentNode,char *pszError );


    eServerStatus       GetStatus() const;
    void       SetStatus(const eServerStatus status);

    DWORD      GetExternalIpAddress() const;
    void       SetExternalIpAddress(const DWORD externalIpAddress);

    ESTATUS    TestValidity();
    // Attributes
protected:

    eServerStatus    m_status; // SERVER_SPECIFY
	                  // SERVER_AUTO
	                  // SERVER_OFF
    DWORD   m_ExternalIpAddress;
};


//////////////////////////////////////////////////////////////////////////////////////////////////
//         Class CIpDns
///////////////////////////////////////////////////////////////////////////////////////////////////
class CIpDns: public CPObject
{
CLASS_TYPE_1(CIpDns,CPObject)
public:
    // Constructors
    CIpDns();
    CIpDns(const CIpDns& other);
    
    CIpDns&  operator=(const CIpDns& other);
    bool operator==(const CIpDns &rHnd)const;
    bool operator!=(const CIpDns& other)const;
    
    ~CIpDns();
    virtual const char* NameOf() const { return "CIpDns";}

    // Implementation
    void       SerializeXml( CXMLDOMElement *pParentNode );
    int        DeSerializeXml( CXMLDOMElement *pParentNode,char *pszError );


    eServerStatus       GetStatus() const;
    void       SetStatus(const eServerStatus DNS_status);

    int        IsEnable() const;
    void       SetEnable(const WORD Enable_DNS);


	int			GetIPv4Address(int idx) const;
    void        SetIPv4Address(int idx ,const DWORD ipV4Address);
	void		GetIPv6Address(int idx, char* retStr, BOOL isBrackets=FALSE);
	void		SetIPv6Address(int idx, const char* ipV6Address);

	DWORD		GetIPv6SubnetMask(int idx) const;
	void		GetIPv6SubnetMaskStr(int idx, char *pOutMask) const;
	void		SetIPv6SubnetMask(int idx, const DWORD subnetMask);
	void		SetIPv6SubnetMask(int idx, const char *pMask);

    const      CSmallString& GetDomainName() const;
    void       SetDomainName(const CSmallString& domain_Name);

    const      CSmallString& GetHostServiceName() const;
    void       SetHostServiceName(const CSmallString& DNS_Host_Service_Name);

    const      CSmallString& GetPrefixName() const;
    void       SetPrefixName(const CSmallString& DNS_Prefix);

    BOOL       GetRegisterDNSAutomatically() const;
    void       SetRegisterDNSAutomatically(const BOOL register_DNS_name_automatically);

    WORD       GetAcceptCallsViaDNS() const;
    void       SetAcceptCallsViaDNS(const WORD accept_calls_via_DNS);

    ESTATUS    TestValidity(BYTE isDhcpEnabled);

    WORD	   DnsChangeRequiresCardReset(const CIpDns& newDns) const;

    void GetDnsServersIpv6(int idx,ipAddressV6If& val)
    {
    	if ( (0 <= idx) && (NUM_OF_DNS_SERVERS > idx) )
    	{
    		memcpy( &(val.ip), &(m_DNS_serversIPv6address[idx].ip), IPV6_ADDRESS_BYTES_LEN );
    		val.scopeId = m_DNS_serversIPv6address[idx].scopeId;
    	}
    };
    // Attributes
protected:
    eServerStatus         m_status; // SERVER_OFF
	                       // SERVER_SPECIFY
	                       // SERVER_AUTO

    ipAddressV4If	m_DNS_serversIPv4address[NUM_OF_DNS_SERVERS];
    ipAddressV6If	m_DNS_serversIPv6address[NUM_OF_DNS_SERVERS];
    DWORD			m_DNS_serversIPv6MaskArray[NUM_OF_DNS_SERVERS];


    HostName     m_domain_name;
    HostName     m_host_Service_Name;
    CSmallString m_prefix;
    BOOL         m_register_DNS_name_automatically; // TRUE / FALSE
    WORD         m_accept_calls_via_DNS;            // TRUE / FALSE
    WORD         m_enable;                          // TRUE / FALSE

private:
    WORD ind_of_numb_of_dns_servers;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class CIPAuthenticationElement;
WORD operator==(const CIPAuthenticationElement& lhs,const CIPAuthenticationElement& rhs);

////////////////////////////////////////////////////////////////////////////////////////////////////
//			Class   CIPAuthenticationElement
//////////////////////////////////////////////////////////////////////////////////////////////////
class CIPAuthenticationElement : public CPObject
{
CLASS_TYPE_1(CIPAuthenticationElement,CPObject)
public:
    // Constructors
    CIPAuthenticationElement();
    CIPAuthenticationElement(const CIPAuthenticationElement& other);
    CIPAuthenticationElement&  operator=(const CIPAuthenticationElement& other);
	~CIPAuthenticationElement();
    friend WORD operator==(const CIPAuthenticationElement& lhs,const CIPAuthenticationElement& rhs);
    bool operator!=(const CIPAuthenticationElement& other)const;
    
    virtual const char* NameOf() const { return "CIPAuthenticationElement";}
    // Implementation
    void     SerializeXml( CXMLDOMElement *pParentNode,bool isToEma );
    int      DeSerializeXml( CXMLDOMElement *pParentNode,char *pszError );

    const    CSmallString& GetUserName() const;
    void     SetUserName(const CSmallString& user);

    const    CSmallString& GetPassword() const;
    void     SetPassword(const  CSmallString& password);

    const    std::string& GetPassword_enc() const;
    void     SetPassword_enc(const  std::string& password);

    const    std::string& GetPassword_dec() const;
    void     SetPassword_dec(const  std::string& password);

    const    CSmallString& GetDomainName() const;
    void     SetDomainName(const CSmallString& domain);

    const    CSmallString& GetServerName() const;
    void     SetServerName(const CSmallString& server);

    DWORD    GetKeyIP() const;
    void     SetKeyIP(const DWORD key_IP);

    /////////////////////////////////////////////////////////////////////////////
    BOOL GetAuthenticationEnable() const;

    /////////////////////////////////////////////////////////////////////////////
    void SetAuthenticationEnable(BOOL mode);

    /////////////////////////////////////////////////////////////////////////////
    eAuthenticationProtocol GetAuthenticationProtocol() const;


    /////////////////////////////////////////////////////////////////////////////
    void SetAuthenticationProtocol(const eAuthenticationProtocol type);


    ESTATUS    TestValidity();

    // Attributes
protected:
	CSmallString  			m_user;
    CSmallString  			m_password;
    HostName     			m_domain;
    HostName     			m_serverName;
    DWORD         			m_keyIP;
    BOOL          			m_isAuthenticationEnabled;
    eAuthenticationProtocol m_authenticationProtocol;
    std::string  			m_password_enc;
    std::string  			m_password_dec;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
class CIPAuthentication;
WORD operator==(const CIPAuthentication& lhs,const CIPAuthentication& rhs);

//////////////////////////////////////////////////////////////////////////////////////////////////
//			Class CIPAuthentication
//////////////////////////////////////////////////////////////////////////////////////////////////

class CIPAuthentication : public CPObject
{
CLASS_TYPE_1(CIPAuthentication,CPObject)
public:
    CIPAuthentication();
    CIPAuthentication(const CIPAuthentication& other);
    CIPAuthentication&  operator=(const CIPAuthentication& other);
	friend WORD operator==(const CIPAuthentication& lhs,const CIPAuthentication& rhs);
	bool operator!=(const CIPAuthentication& other)const;
    ~CIPAuthentication();
    virtual const char* NameOf() const { return "CIPAuthentication";}

    // Implementation
    void     SerializeXml( CXMLDOMElement *pParentNode ,bool isToEma);
    int      DeSerializeXml( CXMLDOMElement *pParentNode,char *pszError );

    DWORD    GetHTTPDigestAuthenticationNumber() const;
    void     SetHTTPDigestAuthenticationNumber(const DWORD HTTPDigestElements);

    DWORD    GetKerberosAuthenticationNumber() const;
    void     SetKerberosAuthenticationNumber(const DWORD numKerberosElements);

    CIPAuthenticationElement*    GetFirstKerberosAuthenticationElements();
    CIPAuthenticationElement*    GetNextKerberosAuthenticationElements();
    void SetpKerberosIPAuthenticationElements(const CIPAuthenticationElement& pKerberosAuthenticationElements);

    CIPAuthenticationElement*    GetFirstHTTPDigestAuthenticationElements();
    CIPAuthenticationElement*    GetNextHTTPDigestAuthenticationElements();

    CIPAuthenticationElement*       GetSecondHTTPDigestAuthenticationElements();

	CIPAuthenticationElement*	 GetHTTPDigestAuthenticationElement(WORD index);
	CIPAuthenticationElement*	 GetKerberosAuthenticationElement(WORD index);

    void SetpHTTPDigestIPAuthenticationElements(const CIPAuthenticationElement& pHTTPDigestAuthenticationElements);


    ESTATUS    TestValidity();



    // Attributes
protected:
    DWORD m_numHTTPDigestElements;
    DWORD m_numKerberosElements;
    CIPAuthenticationElement* m_pKerberos[NUM_OF_AUTHENTICATION_ELEMENTS];
    CIPAuthenticationElement* m_pHTTPDigest[NUM_OF_AUTHENTICATION_ELEMENTS];
private:
    WORD  m_ind_of_numb_of_KerberosElements;
    WORD  m_ind_of_numb_of_HTTPDigest;
    WORD  m_of_ind_calls;

};

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
class CIPSecurity;
WORD operator==(const CIPSecurity& lhs,const CIPSecurity& rhs);
/////////////////////////////////////////////////////////////////////////////////////////////////
//			class CIPSecurity
/////////////////////////////////////////////////////////////////////////////////////////////////

class CIPSecurity: public CPObject
{
CLASS_TYPE_1(CIPSecurity,CPObject)
public:

    // Constructors
    CIPSecurity();
    CIPSecurity(const CIPSecurity& other);
    CIPSecurity& operator=(const CIPSecurity& other);
	friend WORD operator==(const CIPSecurity& lhs,const CIPSecurity& rhs);
	bool operator!=(const CIPSecurity& other)const;
	
    ~CIPSecurity();
    virtual const char* NameOf() const { return "CIPSecurity";}

    // Implementation
    ESTATUS    TestValidity();
    void       SerializeXml( CXMLDOMElement *pParentNode,bool isToEMA );
    int        DeSerializeXml( CXMLDOMElement *pParentNode,char *pszError );

    //Get
    CIPAuthentication*  GetIPAuthentication() { return m_pAuthentication;}
    //Set
    void SetpIPAuthentication(const CIPAuthentication& pAuthentication);

    // instances
protected:
    CIPAuthentication* m_pAuthentication;
};


/////////////////////////////////////////////////////////////////
// CBaseSipServer is used to store proxy details
//
class CBaseSipServer: public CPObject
{
CLASS_TYPE_1(CBaseSipServer,CPObject)
public:
    // Constructors
    CBaseSipServer();
    CBaseSipServer(const CBaseSipServer& other);

    CBaseSipServer& operator=(const CBaseSipServer& other);
    bool operator==(const CBaseSipServer &rHnd)const;
    bool operator!=(const CBaseSipServer& other)const;
    
    ~CBaseSipServer();
    virtual const char* NameOf() const { return "CBaseSipServer";}

    // Implementation
    virtual void       SerializeXml( CXMLDOMElement *pParentNode );
    virtual int        DeSerializeXml( CXMLDOMElement *pParentNode,char *pszError );

    eServerStatus  GetStatus() const;
    void           SetStatus(const eServerStatus status);

    const CSmallString& GetName() const;
    void                SetName(const CSmallString& name);

	DWORD      GetIpAddress() const;

    DWORD      GetPort() const;
    void       SetPort(const DWORD Port);

    ESTATUS    TestValidity(BYTE isDns);

protected:
    eServerStatus  m_status; //(eServerStatusOff/Specify/Auto)
    HostName       m_name;   // server name/ip string
    DWORD          m_port;   // server port
};

/////////////////////////////////////////////////////////////////
// CSipServer is used to store SIP Server (used to be called 'registrar') details
//
class CSipServer: public CBaseSipServer
{
CLASS_TYPE_1(CSipServer,CBaseSipServer)
public:
    // Constructors
    CSipServer();
    CSipServer(const CSipServer& other);

    CSipServer& operator=(const CSipServer& other);
    bool operator==(const CSipServer &rHnd)const;
    bool operator!=(const CSipServer& other)const;
    
    ~CSipServer();
    virtual const char* NameOf() const { return "CSipServer";}

    // Implementation
    virtual void       SerializeXml( CXMLDOMElement *pParentNode );
    virtual int        DeSerializeXml( CXMLDOMElement *pParentNode,char *pszError );

    const CSmallString& GetDomainName() const;
    void                SetDomainName(const CSmallString& domain);

    ESTATUS    TestValidity(BYTE isDns);

protected:
    HostName     m_domain;   // domain name/ip string
};

///end CSipServer
////////////////////////////////////////////////////////////////////////////
// class CSip


class CSip: public CPObject
{
CLASS_TYPE_1(CSip,CPObject)
public:
    // Constructors
    CSip();
    CSip(const CSip& other);
    
    CSip&  operator=(const CSip& other);
    bool operator==(const CSip &rHnd)const;
    bool operator!=(const CSip& other)const;
    
    ~CSip();
    virtual const char* NameOf() const { return "CSip";}
    // Implementation
    void       SerializeXml( CXMLDOMElement *pParentNode );
    int        DeSerializeXml( CXMLDOMElement *pParentNode,char *pszError );

    //Get
    const CBaseSipServer*  GetpProxy()const         { return m_pProxy;}
    const CBaseSipServer*  GetpAltProxy()const      { return m_pAltProxy;}
    const CSipServer*  GetpRegistrar() const    	{ return m_pRegistrar;}
    const CSipServer*  GetpAltRegistrar()const  	{ return m_pAltRegistrar;}


    //Set
    void SetProxy(const CBaseSipServer& pOutbondProxy);
    void SetAltProxy(const CBaseSipServer& pAlternateOutbondProxy);
    void SetRegistrar(const CSipServer& pPreferedRegistrar);
    void SetAltRegistrar(const CSipServer& pAlternateRegistrar);

    eConfigurationSipServerMode GetConfigurationOfSIPServers()const { return m_ConfigureSIPServersMode;}
    void SetConfigurationOfSIPServers(const eConfigurationSipServerMode mode);

//     WORD GetRegistrationOngoingConfrences() const;
//     void SetRegistrationOngoingConfrences(const WORD Registration_Ongoing_confrences);
//     WORD GetRegistrationMeetingRoom() const;
//     void SetRegistrationMeetingRoom(const WORD Registration_Meeting_Room);
//     WORD GetRegistrationFactories() const;
//     void SetRegistrationFactories(const WORD registrationFactories);
//     WORD GetRegistrationGwProfiles() const;
//     void SetRegistrationGwProfiles(const WORD registrationGwProfiles);

//    eRegistrationMode GetRegistrationMode(){return m_eRegistrationMode;}
//    void        SetRegistrationMode(const eRegistrationMode RegistrationMode);

//     WORD        GetRegistrationEntryQueue() const;
//     void        SetRegistrationEntryQueue(const WORD Registration_Entry_Queue);
    WORD        GetAcceptMeetMe() const;
    void        SetAcceptMeetMe(const WORD Accept_MeetMe);
    WORD        GetAcceptAdHoc() const;
    void        SetAcceptAdHoc(const WORD Accept_AdHoc);
    WORD        GetAcceptFactory() const;
    void        SetAcceptFactory(const WORD Accept_Factory);
    DWORD       GetRefreshRegistrationStatus() const;
    void        SetRefreshRegistrationStatus(const WORD Refresh_status);
    DWORD  GetRefreshRegistrationTout() const;
    void   SetRefreshRegistrationTout(const DWORD Refresh_Registration_Tout);

    enTransportType   GetTransportType() const;
    void   SetTransportType(const enTransportType type);

    ESTATUS  TestValidity(BYTE isDhcpEnabled, BYTE isDns);

    const eSipServerType& GetSipServerType() const{ return m_SipServerType;}
    void SetSipServerType(const eSipServerType& type){ m_SipServerType = type;}

protected:
    // Attributes
    CBaseSipServer*   m_pProxy;
    CBaseSipServer*   m_pAltProxy;
	CSipServer*		  m_pRegistrar;			// Changed since API_IP_SERVICE_CORRECTIONS
    CSipServer*		  m_pAltRegistrar;		// Changed since API_IP_SERVICE_CORRECTIONS
	enTransportType    m_TransportType;		// PROTOCOL_TRASPORT_UDP
											// PROTOCOL_TRASPORT_TCP
    eConfigurationSipServerMode  m_ConfigureSIPServersMode;		// CONFIGURE_SIP_SERVERS_AUTOMATICALLY
											// CONFIGURE_SIP_SERVERS_MANUALLY

//     WORD  m_RegistrationOngoingConfrences; // YES / NO
//     WORD  m_RegistrationMeetingRoom;       // YES / NO
// 	WORD  m_RegistrationEntryQueue;        // YES / NO
// 	WORD  m_RegistrationFactories;         // YES / NO
// 	WORD  m_RegistrationGwProfiles;        // YES / NO
	WORD  m_AcceptMeetMe;                  // YES / NO
    WORD  m_AcceptAdHoc;                   // YES / NO
    WORD  m_AcceptFactory;                 // YES / NO
    WORD  m_RefreshStatus;                 // YES / NO

    CSmallString  m_user;
    WORD m_EnableICE;
//    eRegistrationMode m_eRegistrationMode;                // Ori sad : not necessarily
    DWORD m_RefreshRegistrationTout;
    eSipServerType  m_SipServerType;
};

///end SIP




//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class CVlan                                                            //
////////////////////////////////////////////////////////////////////////////////////////////////
class CVlan : public CSerializeObject
{
CLASS_TYPE_1(CVlan, CSerializeObject)
public:
	CVlan();
    CVlan( const CVlan &other );
    
    CVlan&  operator=( const CVlan& other );
    bool operator==(const CVlan &rHnd)const;
    bool operator!=(const CVlan& other)const;
    
    virtual const char* NameOf() const { return "CVlan";}
    
    virtual CSerializeObject* Clone();
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    BOOL GetIsSupport()const;
    void SetIsSupport(BOOL val);

    DWORD GetPriority()const;
    void SetPriority(DWORD val);

    DWORD GetId()const;
    void SetId(DWORD val);

private:
	BOOL 	m_IsSupport;
	DWORD 	m_Priority;
	DWORD 	m_Id;
};
///end CVlan


//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class CVlan                                                            //
////////////////////////////////////////////////////////////////////////////////////////////////
/*
class CGateKeeper : public CSerializeObject
{
CLASS_TYPE_1(CGateKeeper, CSerializeObject)
public:
	CGateKeeper();
    CGateKeeper( const CGateKeeper &other );
    CGateKeeper&  operator=( const CGateKeeper& other );

    virtual CSerializeObject* Clone();
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);



public:
	DWORD m_externalGatekeeperAddr;
	WORD m_GatekeeperDiskoveryPort;
	WORD m_gatekeeperMode;
	WORD m_IsRRQPolling;
	WORD m_RRQPollingInterval;
	BOOL m_RegAsGateway;
};
///end CGateKeeper
*/


//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class  CPortSpeed                                                     //
////////////////////////////////////////////////////////////////////////////////////////////////
class CPortSpeed : public CSerializeObject
{
public:
    CPortSpeed();
    CPortSpeed(DWORD num, ePortSpeedType speed);
    virtual ~CPortSpeed();
    virtual const char* NameOf() const { return "CPortSpeed";}
    
    bool operator==(const CPortSpeed &rHnd)const;
    bool operator!=(const CPortSpeed& other)const;
    
	CSerializeObject* Clone() {return new CPortSpeed(*this);}

    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    void SetNum(DWORD num);
    DWORD GetNum()const;

    void SetSpeed(ePortSpeedType speed);
    ePortSpeedType GetSpeed()const;

private:
    DWORD m_Num;
    ePortSpeedType m_Speed;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class  CManagementSecurity                                                     //
////////////////////////////////////////////////////////////////////////////////////////////////
class CManagementSecurity : public CSerializeObject
{
CLASS_TYPE_1(CManagementSecurity, CSerializeObject)
public:
	// Constructors
	CManagementSecurity();

	~CManagementSecurity();
	virtual const char* NameOf() const { return "CManagementSecurity";}

	bool operator==(const CManagementSecurity &rHnd)const;
	bool operator!=(const CManagementSecurity& other)const;

	CSerializeObject* Clone() {return new CManagementSecurity(*this);}

    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	void Serialize(WORD format,CSegment& rSeg);
	void DeSerialize(WORD format,CSegment& rSeg);

	BOOL 	IsRequestPeerCertificate();
	std::string	GetOCSPGlobalResponderURI();

	BOOL	GetIsUseResponderOcspURI();
	BOOL	GetIsIncompleteRevocation();
	BOOL	GetIsSkipValidateOcspCert();
	BYTE 	GetRevocationMethodType();
	void 	SetIsRequestPeerCertificate(BOOL isEnabled);
	void	GetOCSPGlobalResponderURI(std::string URI);

	void	SetIsUseResponderOcspURI(BOOL isUseResponderOcspURI);
	void	SetIsIncompleteRevocation(BOOL isIncompleteRevocation);
	void	SetIsSkipValidateOcspCert(BOOL isSkipValidateOcspCert);
	void	SetRevocationMethodType(BOOL revocationMethodType);

	BOOL    IsUseResponderOcspUri(){return m_isUseResponderOcspURI;};
	BOOL    IsIncompleteRevocation(){return m_isIncompleteRevocation;};
	BOOL    IsSkipValidationOcspCert(){return m_isSkipValidateOcspCert;};
	eRevocationMethod getRevocationMethodType(){return (eRevocationMethod)m_revocationMethodType;};
private:
	BOOL	m_isRequestPeerCertificate;

	// not in used
	BOOL	m_isOCSPEnabled;
	// not in used!
	BOOL	m_isAlwaysUseGlobalOCSPResponder;
	std::string	m_ocspGlobalResponderURI;
	BOOL    m_isUseResponderOcspURI;
	BOOL    m_isIncompleteRevocation;
	BOOL    m_isSkipValidateOcspCert;
	BYTE    m_revocationMethodType;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class  CIceStandardParams                                                     //
////////////////////////////////////////////////////////////////////////////////////////////////
class CIceStandardParams : public CSerializeObject
{
CLASS_TYPE_1(CIceStandardParams, CSerializeObject)	
public:
	CIceStandardParams();
	CIceStandardParams(const CIceStandardParams& other);
	
	virtual ~CIceStandardParams();
	
	CIceStandardParams&  operator=(const CIceStandardParams& other);
	bool operator==(const CIceStandardParams& other)const;
	bool operator!=(const CIceStandardParams& other)const;

    virtual const char* NameOf() const { return "CIceStandardParams";}

    virtual CSerializeObject* Clone() { return new CIceStandardParams(*this);}
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    virtual int DeSerializeXml( CXMLDOMElement *pActionNode,char *pszError,const char* action=NULL );


    BOOL GetIsServerPassword()const;

    DWORD GetPasswordServerIp()const;
    const char* GetPasswordServerHostName()const;
    WORD GetPasswordServerPort()const;
    const char* GetPasswordServerUserName()const;
    const char* GetPasswordServerPassword()const;
    DWORD GetSTUNServerIp()const;
    const char* GetSTUNServerHostName()const;
    DWORD GetTURNServerIp()const;
    const char* GetTURNServerHostName()const;
    WORD GetSTUNServerPort()const;
    WORD GetTURNServerPort()const;
    void SetSTUNServerPort(WORD value);
    void SetTURNServerPort(WORD value);

private:

    BOOL  m_IsServerPassword;
    WORD m_PasswordServerIp; //need for backward compatibility
    std::string m_PasswordServerHostName; // IPv4/Ipv6/Hostname
    WORD  m_PasswordServerPort;
    std::string m_PasswordServerUserName;
    std::string m_PasswordServerPassword;

    WORD m_STUNServerIp; //need for backward compatibility
    std::string m_STUNServerHostName; // IPv4/IPv6/Hostname
    WORD m_TURNServerIp; //need for backward compatibility
    std::string m_TURNServerHostName; // IPv4/IPv6/Hostname
    WORD  m_STUNServerPort;
    WORD  m_TURNServerPort;

};

//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class  CSipAdvanced                                                     //
////////////////////////////////////////////////////////////////////////////////////////////////
class CSipAdvanced : public CSerializeObject
{
	CLASS_TYPE_1(CSipAdvanced, CSerializeObject)		
public:
	CSipAdvanced();
	CSipAdvanced(const CSipAdvanced& other);
	CSipAdvanced&  operator=(const CSipAdvanced& other);
	bool operator==(const CSipAdvanced& other)const;
	bool operator!=(const CSipAdvanced& other)const;
	virtual ~CSipAdvanced();
    virtual const char* NameOf() const { return "CSipAdvanced";}
    
    virtual CSerializeObject* Clone() { return new CSipAdvanced(*this);}
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action=NULL );
    
    const char* GetSipAdvancedUserName()const;
    eIceEnvironmentType GetIceEnvironment()const;
    void SetIceEnvironment(eIceEnvironmentType enviromentType);
    CIceStandardParams* GetpIceStandardParams()const;
    
    void SetSipAdvancedUserName(const char* user_name);

private:
	std::string m_sipAdvancedUserName;
	eIceEnvironmentType m_iceEnvironment;
	CIceStandardParams* m_pIceStandardParams;
};




//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class IpService                                                            //
////////////////////////////////////////////////////////////////////////////////////////////////
class CIPService : public CSerializeObject
{
CLASS_TYPE_1(CIPService, CSerializeObject)
public:
	   //Constructors
    CIPService();
    CIPService( const CIPService &other );
    CIPService&  operator=( const CIPService& other );
    bool operator==(const CIPService &rHnd)const;
    bool operator!=(const CIPService& other);
    virtual const char* NameOf() const { return "CIPService";}
    bool compareManagment(const CIPService &rHnd) const;
    virtual ~CIPService() ;

//    DWORD TestValidity();

	   // Implementation
    void   SerializeXml(CXMLDOMElement *&pFatherNode, DWORD ObjToken, bool isToEMA=false) const;
    void   SerializeXml(CXMLDOMElement *&pFatherNode) const;
    int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	CSerializeObject* Clone() {return new CIPService(*this);}

    void          SetName(const char*  name);
    const char*   GetName() const;
    void          SetServiceType(const unsigned char serviceTypeName);
    BYTE          GetServiceType ( ) const;
    void          SetNetIPaddress(DWORD netIPaddress);
    DWORD         GetNetIPaddress();

    void          SetNetMask(DWORD netMask);
    DWORD         GetNetMask() const;

    void          SetDefaultGatewayIPv4(const DWORD defaultGateway);
    DWORD         GetDefaultGatewayIPv4() const;

    void          SetDefaultGatewayIPv6(const char* defaultGateway);
    void          GetDefaultGatewayIPv6(char* retStr, BOOL isBrackets=FALSE) const;

    void          GetDefaultGatewayFullIPv6(char* retStr, BOOL isBrackets=FALSE) const;

	DWORD         GetDefaultGatewayMaskIPv6() const;
	void          GetDefaultGatewayMaskIPv6Str(char *pOutMask) const;
	void          SetDefaultGatewayMaskIPv6(const DWORD subnetMask);
	void          SetDefaultGatewayMaskIPv6(const char *pMask);

    WORD          GetRoutersNumber () const;
    void          SetRoutersNumber(const WORD num);
    int           AddRouter( const CH323Router &other , bool isCheckSpanValidity=true, bool isShouldNotExist=true);
    int           FindRouter( const CH323Router &other );
    int           UpdateRouter( const CH323Router &other );
    int           CancelRouter( WORD ind );
    void          RemoveAllRouters( );
    CH323Router*  GetFirstRouter();
    CH323Router*  GetNextRouter();
    CH323Router*  GetFirstRouter(int& nPos);
    CH323Router*  GetNextRouter(int ind,int& nPos);
    CH323Router*  GetCurrentRouter(WORD ind) const;

    void         SetDHCPServer(WORD DHCPServer);
    WORD         GetDHCPServer()const;

    BOOL 		 IsContainGK()const;
    void         SetGatekeeper(BYTE gatekeeper);
    BYTE         GetGatekeeper()const;
//    void         SetExternalGatekeeperAddr(DWORD externalGatekeeperAddr);
    DWORD        GetExternalGatekeeperAddr()const;
//    DWORD 		 GetRealExternalGatekeeperAddr()const;
    void         SetGatekeeperDiskoveryPort(WORD GatekeeperDiskoveryPort);
    WORD         GetGatekeeperDiskoveryPort();

    DWORD 		 GetAltGatekeeperAddr()const;

    WORD         GetSpansNumber () const;
    void         SetSpansNumber(const WORD num);

    BOOL 		 GetIsRegAsGW();
    void 		 SetIsRegAsGW(BOOL val);

    BOOL 		 GetIsVlanSupport();
    void 		 SetIsVlanSupport(BOOL val);

    DWORD 		 GetVlanPriority();
    void 		 SetVlanPriority(DWORD val);

	DWORD 		 GetVlanId();
    void 		 SetVlanId(DWORD val);

    int          AddSpan( const CIPSpan &other );
    int          AddSpan_NoCheck( const CIPSpan &other );

    int          FindSpan( const CIPSpan &other );
    int          FindSpan( const WORD line );
    int          UpdateSpan( const CIPSpan &other );
	void         ReplaceSpan_NoCheck( const WORD idx, const CIPSpan&  other );
    int          CancelSpan(const WORD line);
    int 		 CancelSpanByIndex(const int ind);
    void 		 RemoveZeroSpansFromService();
    CIPSpan*     GetNextSpan();
	CIPSpan*	 GetFirstSpan();
    CIPSpan*	 GetSpanByIdx(const WORD idx) const;
    CIPSpan*	 GetCurrentSpan(const WORD line) const;

    
    void         ClearIPv6Addresses();
    void         ClearSignalingIpv6Addresses();
    void         ClearMediaIpv6Addresses();
    void         SetIpParamsFromOtherService(const CIPService *pOther, int spanIdx, eIpType ipTypeToSet);
    void         SetIPv4ParamsFromOtherService(const CIPService *pOther, int spanIdx);
    void         SetIPv6ParamsFromOtherService(const CIPService *pOther, int spanIdx);
	void         SetIpV6Params( eIpType ipType, eV6ConfigurationType v6ConfigType,
								string ipv6Add_0, string ipv6Add_1, string ipv6Add_2, string ipv6_defGw,
								DWORD ipv6Mask_0, DWORD ipv6Mask_1, DWORD ipv6Mask_2, DWORD ipv6Mask_defGw, BOOL bForceDefGwUpdate );

    WORD       IsValid(DWORD routerIP, DWORD cardIP);
    STATUS       IsValidRouter(const CH323Router* pCurRouter);
    STATUS       IsValidSpan(const CIPSpan* pCurSpan);
    STATUS       IsValidRouterWithSpans(const CH323Router* pCurRouter);
    STATUS       IsValidSpanWithRouters(const CIPSpan* pCurSpan);
    STATUS       IsValidDefaultRouter();

    void           SetDialInPrefix(const char* prefix, WORD prefix_type = PARTY_H323_ALIAS_E164_TYPE);
    void           SetDialInPrefix(const CH323Alias& newPrefix);
    const CH323Alias*  GetDialInPrefix() const;

    void           SetATMEnableAAL5(const unsigned char ATMEnableALL5);
    unsigned char  GetATMEnableAAL5()const;
	void           SetQualityOfService (const CQualityOfService& qos);
    void           SetQualityOfService(const unsigned char qualityOfService);
    unsigned char  GetQualityOfService()const;

    BYTE           ServiceTypeToSpanType (const BYTE serviceType);

    // API - API_NUM_H323_QOS
    CQualityOfService*  GetpQualityOfService() { return m_pQualityOfService; }
    CServiceConfig*  GetServiceConfig() { return m_pServiceConfig; }
    // get funcs
    BYTE GetQosAtmAudio() const { return m_pQualityOfService->GetAtmAudioQoS(m_serviceName); }


    const char * GetH323AuthenticationPassword() const ;
    const char * GetH323AuthenticationUserName() const ;

    eAuthenticationProtocol GetH323AuthenticationProtocol() ;


    BOOL GetH323AuthenticationEnable();

    void SetH323AuthenticationPassword(const char *)  ;
    void SetH323AuthenticationUserName(const char *)  ;

    void SetH323AuthenticationProtocol(eAuthenticationProtocol protocol) ;


    void SetH323AuthenticationEnable(BOOL authenticationMode);

    // QoS set/get funcs
    // IP
  //  void SetQosIp (const BYTE status, const BYTE isDiffService, const BYTE tosValue,
//        const BYTE audioPriority, const BYTE videoPriority);
    // ATM
    void SetQosAtm(const BYTE status, const BYTE audioPriority, const BYTE videoPriority);

    void SetGatekeeperMode(WORD mode);
    WORD GetGatekeeperMode() const;
    void SetRRQPollingInterval(WORD seconds);
    WORD GetRRQPollingInterval () const;
    WORD IsRRQPolling () const;
    void SetRRQPolling (WORD yesNo);
    WORD IsForwarding() const;
    void SetForwarding(WORD yesNo);
    WORD GetPseudoGKListenPort() const;
    void SetPseudoGKListenPort(WORD portNum);
    WORD ServiceChangeRequiresCardReset(const CIPService& newService) const;
    WORD SpanChangeRequiresCardReset(const CIPSpan& newSpan) const;

	BYTE	GetChanged() const;
	DWORD	GetUpdateCounter() const;

	CDynIPSProperties *GetDynamicProperties();

	STATUS ConvertToIpParamsStruct(IP_PARAMS_S &ipParamsStruct);


    BOOL GetIsMepModeRequired() const;
    
    
     // API - API_NUM_SIP
	 DWORD         GetVpnIp()     { return m_VpnIp;}
     CIpDns*       GetpDns()      { return m_pDns; }
     CIPSecurity*  GetpSecurity() { return m_pSecurity;}
	 CSip*         GetpSip()      { return m_pSip;}
     const CSmallString & GetGatekeeperName();
     const CSmallString & GetAltGatekeeperName();

     //Set
     void    SetVpnIp(DWORD ip);
     void    SetDns(const CIpDns&);
     void    SetSecurity(const CIPSecurity& pSecurity);
     void    SetGatekeeperName(const CSmallString& GatekeeperName);
     void    SetAltGatekeeperName(const CSmallString& AltGatekeeperName);

	 CSip* GetSip () const;
	 void  SetSip (const CSip& sip);

     //sip
     eIPProtocolType GetIPProtocolType()const{return m_eProtocolType;}
     void SetIPProtocolType(const eIPProtocolType ProtocolType);
     WORD    IsAutoRegisterSpanHostName();
     void    SetAutoRegisterSpanHostName(const WORD m_AutoRegisterSpanHostName);

     eIpServiceType  GetIpServiceType () const;
     void            SetIpServiceType (const eIpServiceType serviceType);

    DWORD GetId() const { return m_Id; }
    void SetId(DWORD id){m_Id = id;}

	const CVlan& GetVlan() {return m_Vlan;}
	bool IsUserDefinePorts();
    void DefineTcpPortRange(DWORD maxPartiesNum);
	void SetUdpPortRange(WORD firstPort, WORD numOfPorts);

    CPortSpeed* GetPortSpeedVector(){return m_PortSpeedVector;}

	BOOL GetIsSecured()const;
	void SetIsSecured(BOOL val);

	BOOL GetIsPermanentNetworkOpen()const;
	void SetIsPermanentNetworkOpen(BOOL val);

    eIpType		GetIpType () const;
	void		SetIpType(const eIpType  ipType);

	eV6ConfigurationType GetIpV6ConfigurationType() const;
	void		SetIpV6ConfigurationType(const eV6ConfigurationType configType);

	CManagementSecurity* GetManagementSecurity() const;
	void SetManagementSecurity(const CManagementSecurity& ManagementSecurity);
	//ice
	CSipAdvanced*    GetpSipAdvanced()      { return m_pSipAdvanced;}
    void 	 SetSipAdvanced(const CSipAdvanced& pSipAdvanced);
    
    int 		GetNumOfOccupiedSpans();
    void 		CalculateMaxNumOfCalls();
    DWORD 		GetMaxNumOfCalls();
    
    WORD 		CalcSpanPosAccordingToBoardAndSubBoardId(DWORD board_id, DWORD sub_board_id);
    
    BYTE 		GetServiceDefaultType(const char* defaultH323service, const char* defaultSipService);
    
    void		SetIsV35GwEnabled(BOOL bIsEnabled);
    BOOL		GetIsV35GwEnabled();

    void		SetV35GwIpAddress(DWORD ipAddress);
    DWORD		GetV35GwIpAddress();

    void		SetV35GwUsername(std::string sUsername);
    const char*	GetV35GwUsername();

    void		SetV35GwPassword_dec(std::string sPassword);
    const char*	GetV35GwPassword_dec();

    void		SetV35GwPassword_enc(std::string sPassword);
    const char*	GetV35GwPassword_enc();

    int	GetV35GwPassword_dec_Length();
    int	GetV35GwPassword_enc_Length();

    BOOL  IsV35ValidPortDefinition();
    void 		SetV35GwPort(std::string sGwPort);
    const char* GetV35GwPort();
    const char* GetV35GwAlias();
    void 		SetV35GwAlias(std::string sGwAlias);

    void UpdateServiceDefaults();
    void UpdateSystemMonitorOnlyFields();
    BOOL CompareServiceValues();

    void GetFirstAvailableIpAddressAndInterface(DWORD& dword_ipAddress, string& interface, string& strIpV6_Global);
    DWORD GetInterfaceIpAddress(std::string& interface, std::string ip_address);
    CWhiteList* GetWhiteList(){return m_pWhiteList;}
    void SetWhiteList(CWhiteList* pWhiteList)
		{	if(pWhiteList != NULL)
			{
				*m_pWhiteList = *pWhiteList;
			}

		}
    void GetDefaultGatewayBytesIpv6(ipAddressV6If& val)
    {
    	memcpy( &(val.ip), &(m_defaultGatewayIPv6.ip), IPV6_ADDRESS_BYTES_LEN );
    	val.scopeId = m_defaultGatewayIPv6.scopeId;
    };
    void SetDefaultGatewayBytesIpv6(ipAddressV6If& val,DWORD mask)
    {
    	m_defaultGatewayIPv6 = val;
    	m_defaultGatewayMaskIPv6 = mask;
    };

private:
	void SetDefaults();


	char         m_serviceName[NET_SERVICE_PROVIDER_NAME_LEN];
     DWORD        m_netIPaddress;
     DWORD        m_netMask;
     ipAddressV4If  m_defaultGatewayIPv4;
     ipAddressV6If  m_defaultGatewayIPv6;  // API - 16
     DWORD			m_defaultGatewayMaskIPv6;



     WORD         m_numb_of_routers;

     WORD         m_DHCPServer;
     BYTE         m_gatekeeper;
//     DWORD        m_externalGatekeeperAddr;
     WORD         m_GatekeeperDiskoveryPort;


     //API - 42
     CH323Alias   m_dialIn_prefix;

     //API - API_NUM_H323_ATM
     unsigned char   m_serviceTypeName;
     unsigned char   m_ATMEnableAAL5;
     unsigned char   m_qualityOfService; // this fiels for compatibility with previous versions


     //API - API_NUM_H323_ATM_2
     unsigned char   m_useDefaultARPServerIPOA;
//     CAtmAddr*       m_pARPServerIPOA;
     unsigned char   m_useDiscoveredLECS;
//     CAtmAddr*       m_pLECS;  //LANE Configured Server
     unsigned char   m_useLECSForLES;
//     CAtmAddr*       m_pLES;  //LANE Server

     //API - API_NUM_H323_ATM_3
     unsigned char   m_useAutoElanName;
     char            m_elanName[ELAN_NAME_SIZE];

     //type

     //API - API_GK_MODES
     WORD  m_forwarding;
     WORD  m_gatekeeperMode;
     WORD  m_IsRRQPolling;
     WORD  m_RRQPollingInterval;
     WORD  m_pseudoGKListenPort;

     // API SIP
     eIPProtocolType  m_eProtocolType;    // eIPProtocolType_SIP
	                                      // eIPProtocolType_H323
	                                      // eIPProtocolType_SIP_H323
	 HostName         m_GatekeeperName;
	 HostName         m_AltGatekeeperName;

	 DWORD            m_VpnIp;
	 WORD             m_AutoRegisterSpanHostName; // TRUE / FALSE

	 WORD        m_ind_span;
	 WORD        m_ind_router;

	 BYTE  m_bChanged;
	 DWORD m_Id;

	 WORD  m_IsRegAsGW;		// TRUE / FALSE
	 eIpServiceType  m_eIpServiceType; // eIpServiceType_Signaling
		                               // eIpServiceType_Management
		                               // eIpServiceType_Control

	CVlan m_Vlan;

	CIpDns*          m_pDns;
    CIPSecurity*     m_pSecurity;
    CSip*            m_pSip;
    CWhiteList*      m_pWhiteList;
	CDynIPSProperties *m_DynIPSProperties;

	CH323Router* m_pRouter[MAX_ROUTERS_IN_H323_SERVICE];  // API - 16

	WORD         m_numb_of_span;
    CIPSpan*     m_pSpan[MAX_SPAN_NUMBER_IN_SERVICE];

    	// API - API_NUM_H323_QOS
    CQualityOfService*	m_pQualityOfService;
    CServiceConfig * m_pServiceConfig;
    CPortSpeed m_PortSpeedVector[MAX_NUM_OF_PORTS_SPEED];

    CManagementSecurity*	m_pManagementSecurity;

    eIpType		m_ipType; // eIpType_IpV4, V6...
	eV6ConfigurationType	m_ipv6ConfigType;  // Auto, Manual...
	BYTE m_bSecured;
	BYTE m_bPermanentNetwork;
	
	//Ice
	CSipAdvanced* m_pSipAdvanced;
	
	DWORD m_MaxNumOfCalls;			//number of possible calls on the ip service

	BOOL		m_isV35GwEnabled;
	DWORD		m_V35GwIpAddress;
	std::string	m_V35GwUsername;
	std::string	m_V35GwPassword_dec;
	std::string	m_V35GwPassword_enc;
	std::string m_V35GwPort;
	std::string m_V35GwAlias;
};

std::string GetHostNameFromService(const CIPService* pService);

CIceStandardParams* GetIceParamsFromService(CIPService* pService);
//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class CIPServiceList                                                       //
////////////////////////////////////////////////////////////////////////////////////////////////
class CIPServiceList : public CSerializeObject
{
CLASS_TYPE_1(CIPServiceList,CSerializeObject)
public:
	   //Constructors
    CIPServiceList(CIPServiceFullList *xmlWrapper = NULL);
    CIPServiceList( const CIPServiceList &other );
 //   CIPServiceList();
    CIPServiceList&  operator=( const CIPServiceList& other );
    
    virtual ~CIPServiceList() ;
    virtual const char* NameOf() const { return "CIPServiceList";}

 	void    SerializeXml(CXMLDOMElement* pFatherNode,DWORD ObjToken) const;
	void    SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int     DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	CSerializeObject* Clone(){return new CIPServiceList(m_XMLWrapper);}

    WORD         GetServiceNumber() const;
    void         SetServiceNumber( const WORD num );
    WORD         GetNumberOfLANServices() const;
    void         SetNumberOfLANServices( const WORD num );
    int 		 Add(CIPService *pOther, bool isSaveToDisk);
    int          Add(const CIPService &other, bool isSaveToDisk = true);
    int          AddOnlyMem( const CIPService&  other );
    int          Update( const CIPService &other );
    int          UpdateOnlyMem( const CIPService&  other );
    int          Cancel( const char* name );
    int          FindService( const CIPService &other )const;
    int          FindService( const char* name )const;
	int          FindService( DWORD id )const;
    CIPService*  GetService(const CH323Alias* prefix);// ori inbar
    const char*  FindServiceAndGetStringWithoutPrefix( const char* prefixPlusString, WORD prefixType = PARTY_H323_ALIAS_E164_TYPE);
    CIPService*  GetService( const char* name );
    CIPService*  GetService( const DWORD id );
    CIPService*  GetService( const WORD line ,const BYTE serviceTypeName);
    CIPService*  GetFirstService();
    CIPService*  GetNextService();
    void         SetH323DefaultName( const char* name );
    const char*  GetH323DefaultName() const;
    void         SetSIPDefaultName( const char* name );
    const char*  GetSIPDefaultName() const;
    int          IsSIPType();
    int          IsH323Type();
    void         ChangeDefaultServiceFromSipToH323(const CSmallString& defaultService);
    DWORD        GetUpdateCounter() const;
    BYTE         GetChanged() const;

    void SetUpdateCounter(DWORD cnt);
	void UpdateCounters();

    STATUS UpdateDynamic(DWORD serviceId, CServiceInfo	&param);
	STATUS UpdateDynamic(DWORD serviceId, CCSIpInfo		&param);
	STATUS UpdateDynamic(DWORD serviceId, CSipInfo		&param);
	STATUS UpdateDynamic(DWORD serviceId, CH323Info		&param);
	STATUS UpdateDynamic(DWORD serviceId, CCardIpAddress	&param);
	STATUS UpdateDynamic(DWORD serviceId, CIceInfo	&param);

	bool GetIsServiceAdded(){return m_IsServiceAdded;}
	
	int GetFirstFreeServiceId();
	BOOL IsServiceIdInRange(int id);

//    void SetMaxNumOfParties(DWORD num, DWORD serviceId);
    void SetMaxNumOfParties(DWORD num);
	void SetUdpPortRange(WORD firstPort, WORD numOfPorts, DWORD serviceId);
    void SetUdpPortRange(WORD firstPort, WORD numOfPorts);
    DWORD GetServiceIdByName(const char* name);

    void HandleServicesCfgFiles();
    
    void CalcMaxNumOfPorts();
    
    BOOL IsOccupiedSpan(int pos);
    void UpdateOccupiedSpan(CIPService *pNewService, CIPService *pOldService);
    void UpdateOccupiedSpan(CIPService *pNewService);
    void UpdateDeletedOccupiedSpan(CIPService *pDeletedService);
    void UpdateEnableDisableFlag();

    BOOL IsV35InUsed();

    void UpdateServiceListDefaults();
    BOOL CompareServiceListValues();

public:
	   // Attributes
    WORD           m_numb_of_serv;
    char           m_defaultServiceName[NET_SERVICE_PROVIDER_NAME_LEN];
    char           m_defaultSIPServiceName[NET_SERVICE_PROVIDER_NAME_LEN];
    CIPService*    m_pH323Service[MAX_SERV_PROVIDERS_IN_LIST];

private:
    WORD          m_ind_serv;
	WORD          m_numb_of_lan_serv;
    BYTE          m_bChanged;
	CIPServiceFullList *m_XMLWrapper;
	bool 		  m_IsServiceAdded;
	
    BYTE 		  m_OccupiedSpans[MAX_SPAN_NUMBER_IN_SERVICE];
};

class CCloudInfo
{
public:
	CCloudInfo()
	{
		m_cloudIp[0] = '\0';

		std::string fname = MCU_TMP_DIR+"/cloudIp";
		FILE *pFile = fopen(fname.c_str(), "r");
		if (pFile) {
			fscanf(pFile, "%127s", m_cloudIp);
			fclose(pFile);
		}
	}
	char* GetCloudIp(){return m_cloudIp;}

	bool IsCloud() {return m_cloudIp[0] != '\0';}

protected:
	char m_cloudIp[128];
};

/*
/////////////////////////////////////////////////////////////////////////////
// CH323ServDbFile
class CH323ServDbFile : public CCfgFile
{
CLASS_TYPE_1(CH323ServDbFile,CCfgFile)
public:
	   //Constructors
    CH323ServDbFile() {};
    virtual ~CH323ServDbFile() {};

	   // Implementation
    void   PutDefaultServNames(const char* fileName, int& status,const char* h323,const char *sip);
    char*  GetDefaultH323ServName(const char* fileName,  int& status);
    char*  GetDefaultSipServName(const char* fileName,  int& status);

	// Attributes

};
/////////////////////////////////////////////////////////////////////////////
*/
#endif // _IpService_
