// 
// ==========================================
//    Copyright (C) 2014           "POLYCOM"
// ==========================================
// FileName:               PlcmDNS_Defines.h  
// Include line recommended:
// #include "PlcmDNS_Defines.h"  //
// 
// ==========================================

#ifndef _PLCMDNS_DEFINES_H_
#define _PLCMDNS_DEFINES_H_

#include "IpAddressDefinitions.h"
#include "SipProxyTaskApi.h"


#ifndef IN
 #define IN
#endif

#ifndef OUT
 #define OUT
#endif

#define PLCM_STRING_SIZE_256				256


#define PLCM_DNS_SERVER_LISTENER_PORT		53
#define PLCM_DNS_CLIENT_PORT				40053

#define PLCM_DNS_DOMAINNAME_AMOUNT_MAX		5
#define	PLCM_DNS_MAX_ADDITIONAL_IP			3

#define PLCM_DNS_TIMEOUT_DEFAULT_SEC		5
#define PLCM_DNS_SERVICE_DEFAULT_TIME_SEC   30  // in seconds
#define PLCM_DNS_SERVICE_REQ_SHIFT_TIME_Sec 2   // 2 seconds	
#define DNS_REQ_TIMER_TOUT					101
#define DNS_REQ_RECV						110

//#define DNS_RESOLVED_MAX					20 
#define DNS_MAX_SERVICES					MAX_SERVICES_NUM + 2// see: SipProxyTaskApi.h   OR GkTaskApi.h



#define PLCM_MAX_RESOLVED_IP		PLCM_HOST_IPV4_AMOUNT + PLCM_HOST_IPV6_AMOUNT//see IpAddressDefinitions.h


//#define PLCM_DNS_HOST_NAME_SIZE		256
//#define PLCM_DNS_DOMAIN_NAME_SIZE		64
//#define PLCM_HOST_NAME_AMOUNT			100
//#define PLCM_HOST_IPV4_AMOUNT			10
//#define PLCM_HOST_IPV6_AMOUNT			10
//
//#define PLCM_DNS_RECORDS_AMOUNT			((PLCM_HOST_NAME_AMOUNT)*(PLCM_HOST_IPV4_AMOUNT + PLCM_HOST_IPV6_AMOUNT)) * 10//
//
//typedef struct _sDNS_RECORD
//{
//	unsigned int		dwRawId									;
//	char				szHostName	[PLCM_DNS_HOST_NAME_SIZE]	;
//	char				szDomainName[PLCM_DNS_DOMAIN_NAME_SIZE]	;
//	ipAddressStruct		sResolveIp								;
//	unsigned int		dwStatus								;
//	unsigned int		dwTimeStamp_Responce					;
//	unsigned int		dwTTL									;
//	unsigned int		dwTimeStamp_NextReq						;
//	ipAddressStruct		sDnsAddress								;
//	char				Reserve [28]							;
//}
//sDNS_RECORD;
/*
typedef BOOL (*DnsAuditFun) (sDNS_RECORD * par_pRecord, int par_nDataType, void * par_pData);

BOOL   DnsInsertHostByName			(sDNS_RECORD		     *  par_pDnsRecord );

BOOL   DnsDeleteHostByNameByIpType	(IN   char               *  par_szHostName 
								    ,IN   ipAddressStruct       par_sDnsAddrr  
								    ,IN   int                   par_rRequiredIpType );

BOOL   DnsDeleteHostByName			(IN   char  	         *  par_szHostName 
									,IN   ipAddressStruct       par_sDnsAddrr       );

BOOL   DnsDeleteHostByNameAllDnsByIpType
                                    (IN char                 *  par_szHostName 
									,IN int                     par_rRequiredIpType );

BOOL   DnsDeleteHostByNameAllDns    (IN   char               *  par_szHostName );

BOOL   DnsGetHostByName				(IN     char			 *  par_szHostName
									,IN     ipAddressStruct     par_sDnsAddrr
									,IN     int                 par_eRequiredIpType
									,IN OUT int              *  par_pOutBuffSize     	
									,OUT    sDNS_RECORD	     *  par_pOutBuff       );

BOOL   DnsGetNameByHost				(IN	    ipAddressStruct  *  par_IpHost
									,IN     ipAddressStruct     par_sDnsAddrr
									,IN OUT int              *  par_pOutBuffSize     	
									,OUT    char             *  par_szOutName   );

BOOL   DnsAudit						(DnsAuditFun par_pAuditFunction);
BOOL   DnsAuditBreak				(DnsAuditFun par_pAuditFunction);// First return from "par_pAuditFunction" - is break from loop


//void   fDnsAudit_Print(sDNS_RECORD * par_pRecod, int par_nDataType, void * par_pData);
*/



#endif // _PLCMDNS_DEFINES_H_



