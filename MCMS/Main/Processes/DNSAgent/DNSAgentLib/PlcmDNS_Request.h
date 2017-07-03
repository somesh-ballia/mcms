// 
// ==========================================
//    Copyright (C) 2014           "POLYCOM"
// ==========================================
// FileName:                PlcmDNS_Request.h  
// Include line recommended:
// #include "PlcmDNS_Request.h"  //
// 
// ==========================================

#ifndef PLCMDNS_REQUEST_H_
#define PLCMDNS_REQUEST_H_

#include "StateMachine.h"
#include "Segment.h"


#include "PlcmDNS_Defines.h"
#include "PlcmDNS_Packet.h"   //
#include "PlcmDNS_Tools.h"    //


typedef struct _DNS_PLCM_DOMAINNAME
{
	char szDomainName[PLCM_DNS_DOMAIN_NAME_SIZE];
}
DNS_PLCM_DOMAINNAME;

typedef struct _DNS_IPADDR_TTL
{
	ipAddressStruct	sIpAddr	;
	unsigned int	dwTTL	;
}
DNS_IPADDR_TTL;


typedef enum _eDNS_REQ_RES_STATUS
{
	  DNS_REQ_RESP_STAUS_IDLE		= 0
	, DNS_REQ_HAS_SENT			
	, DNS_RES_HAS_RECV
	, DNS_RES_HAS_TIMEOUTED
	, DNS_RESP_FINISH_RESOLVED
	, DNS_RESP_FINISH_NOT_RESOLVED
}
eDNS_REQ_RES_STATUS;

typedef struct _DNS_REQ_NODE
{
	unsigned short		wDnsReqID 										;// KEY for found
	char				szHostName	[PLCM_DNS_HOST_NAME_SIZE]			;// 

	DNS_REQ_RES_TYPE	wReqType										;// eDNS_TYPE_A_IPv4 |	eDNS_TYPE_AAAA_IPv6 | eDNS_TYPE_SRV
	unsigned short		wReqClass										;

	unsigned short		wPartnerDnsReqID								;
	//----- Configured INFO -----------------------------------//
	unsigned int		dwDomainNameAmount								;
	unsigned int		dwDomainName_Current							;
	DNS_PLCM_DOMAINNAME	aDomainName[PLCM_DNS_DOMAINNAME_AMOUNT_MAX]		;

	BOOL				bIsRequiredIpv4									;	
	BOOL				bIsRequiredIpv6									;	

	unsigned int		dwDnsAddressAmount								;
	unsigned int		dwDnsAddress_Current							;
	ipAddressStruct		aDnsAddress[PLCM_DNS_MAX_ADDITIONAL_IP]			;

	//------ Additional Info ----------------------------------// 
	unsigned int		dwTimeStamp_Request								;
	unsigned int		dwTimeOut_mSec									;
	unsigned short		wAttempts										;//Must be from :0 up to 5
	unsigned short		wAttempt_Current								;
	BOOL 		        bTimeOutTimerOrdered							;
	BOOL                bIsResultIsTimeouted                            ;
	//------ Return INFO --------------------------------------// 
	unsigned short		wServiceID										;
	int					nProcessType									;// eProcessType: 
	void			*	pDNSMngrRcvMbx									;// COsQueue *

	unsigned int		dwResolvedAddrAmount							;			
	DNS_IPADDR_TTL		aResoledAddressAndTTL[PLCM_MAX_RESOLVED_IP]		;

	//------SERV INFO -----------------------------------------//  
	unsigned short      wPriority										;
	unsigned short      wWeight											;
	unsigned short      wPort											;
	eIPProtocolType		eSignalProtocolType								;
	enTransportType		eTransportProtocol								;
    //---------------------------------------------------------//

	eDNS_REQ_RES_STATUS eReqResStatus									;
}
DNS_REQ_NODE;


class cDNS_PLCM_REQ_NODE : public CStateMachine
{
	CLASS_TYPE_1(CDNSQuery, CStateMachine)
public:
	virtual const char*  NameOf() const {return "cDNS_PLCM_REQ_NODE";}
			cDNS_PLCM_REQ_NODE   ();
	virtual ~cDNS_PLCM_REQ_NODE  ();

	void vClear			 ();
	void vTimerArrived	 (CSegment* par_pParam);

	void vTimerAwake     (int par_TimerID);
	void vTimerStop      (int par_TimerID, char * par_Log);  


public:
	//------ Request DATA -------------------------------------// 
	DNS_REQ_NODE         m_sReqNode;
protected:
	PDECLAR_MESSAGE_MAP;

};
//DNS_PLCM_REQ_NODE;

//====================================================================//
typedef std::vector <cDNS_PLCM_REQ_NODE*> DNS_REQ_VECTOR;  
typedef BOOL (*DnsReqListAuditFun) (cDNS_PLCM_REQ_NODE * par_pRecord, int par_nDataType, void * par_pData);

class cDnsReqList :  public CPObject
{
	CLASS_TYPE_1(cDnsReqList, CPObject)
public: 
	virtual const char*  NameOf() const {return "cDnsReqList";}
public:

			cDnsReqList ();
	virtual ~cDnsReqList();

	BOOL				 bInsert(cDNS_PLCM_REQ_NODE * par_pNode);
	BOOL				 bDelete(cDNS_PLCM_REQ_NODE * par_pNode);

	BOOL				 bDelete(unsigned int par_wDnsReqID, char * par_szLog);

	cDNS_PLCM_REQ_NODE * Find(unsigned short par_wDnsReqID);
	void                 vUpdatePartnerId(unsigned short par_wDnsReqID, unsigned short par_wPartnerDnsReqID);
	BOOL				 bClean();
	unsigned int		 dwGetSize();
	BOOL				 DnsReqListAudit(DnsReqListAuditFun par_pAuditFunction, int par_nDataType, void * par_pData);
	BOOL				 DnsReqListAuditBreak(DnsReqListAuditFun par_pAuditFunction, int par_nDataType, void * par_pData);// First return from "par_pAuditFunction" - is break from loop

protected:
	PLc_MUTEX			m_Mutex;
	DNS_REQ_VECTOR		m_List;
};
//====================================================================//


//-S- PLCM_DNS ------------------------------//
//Return:
//  1(0x01) = A		=> eDNS_TYPE_A_IPv4
// 28(0x1c) = AAAA	=> eDNS_TYPE_AAAA_IPv6
// 33(0x21) = SRV	=> eDNS_TYPE_SRV
unsigned short plcmDNS_GetQueryType(char		*	szBuff_S
									, int				nBytesRcvd);

char * plcmDNS_szGetReqTypeName(unsigned short par_ReqType);

void plcmDNS_DnsAnswerParsing( char				*	szBuff_S
							  , int					nBytesRcvd
							  , DNS_IPADDR_TTL	*	par_aResolveAddrAndTTL
							  , int				*	par_pLen	        
							  , char             *   par_OutHostName
							  , int				     par_nOutHostNameLen     
							  , unsigned short   *   par_pOutReqId
							  , unsigned short   *   par_pOutType);

void	plcmDNS_SendResolveResult(  WORD				par_wServiceID
								  , char			*	par_szHostName
								  , int					par_eProcessType //eProcessType
								  , void			*	par_DNSMngrRcvMbx			
								  , DWORD				par_dwErrorCode
								  , ipAddressStruct *   par_pIpResolveResult
								  , int                 par_RecodsAmount);

void	plcmDNS_SendResolveResultSRV(  WORD				par_wServiceID
								  , char			*	par_szHostName
								  , int					par_eProcessType //eProcessType
								  , void			*	par_DNSMngrRcvMbx			
								  , DWORD				par_dwErrorCode
								  , ipAddressStruct *   par_pIpResolveResult
								  , int                 par_RecodsAmount
								  , unsigned int        par_dwTTL
								  , WORD				par_wPort
								  , WORD				par_wPriority
								  , WORD				par_wWeight );





//Return IPversion (see enIpVersion)
int		plcmDNS_nGetStrIpFromipAddressStruct(char * par_szOut, int par_OutLen, ipAddressStruct * par_pAddStruct);




#endif //PLCMDNS_REQUEST_H_

