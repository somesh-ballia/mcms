//+========================================================================+
//                  IpAddressDefinitions.h			                       |
//					Copyright 1995 Polycom Ltd.				               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       IpAddressDefinitions.h	                                               |
// SUBSYSTEM:  MFA/CS/ConfParty                                                |
// PROGRAMMER: Guy D,													   |
// Date : 15/5/05														   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+                        
/*

This header file contains all mutual Indication structures related to 
ConfParty + CS entities within the Carmel.
In this document we set the common structures.

*/

#ifndef __IPADDRESSDEFINITIONS_H__
#define __IPADDRESSDEFINITIONS_H__


#include "DataTypes.h"


//------------------ Definitions  ---------
#define IPV6_ADDRESS_BYTES_LEN		16

#define NUM_OF_IPV4_ADDRESSES 		(1) 
#define NUM_OF_IPV6_ADDRESSES 		(5)
#define TOTAL_NUM_OF_IP_ADDRESSES 	(NUM_OF_IPV4_ADDRESSES + NUM_OF_IPV6_ADDRESSES)


//------------------ Includes -------------

//------------------ Globals   ------------


typedef enum {
	eIpVersion4				= 0,
	eIpVersion6				= 1,
    enIpVersionMAX
}enIpVersion;

typedef enum {
	eCandidateTypeV4		= 0, // for "a=candidate"
	eCandidateTypeV6		= 1, // for "a=x-candidate-ipv6"
}eCandidateType;

typedef enum {
	eDistributionUnicast		= 0,
	eDistributionMulticast	= 1,
    enDistributionMAX
}enDistribution;


typedef enum {
	eUnknownTransportType	= 0,
	eTransportTypeUdp		= 1,
	eTransportTypeTcp		= 2,
	eTransportTypeTls		= 3,
    enTransportTypeMAX
}enTransportType;

typedef enum {
	eUnused = -1,
	eLowPriority,
	eMediumPriority,
	eHighPriority	
} enIPv6Priority;

typedef struct
{
    APIU32		ip; /* IPv4 address */
} ipAddressV4If;

typedef struct
{
    APIU8		ip[IPV6_ADDRESS_BYTES_LEN]; /* IPv6 address */
    APIU32		scopeId;
} ipAddressV6If;

union ipAddressIf
{
    ipAddressV4If v4;
    ipAddressV6If v6;
};

typedef struct
{
    union	ipAddressIf addr;	
    APIU32	ipVersion; // enIpVersion
    APIU32	port;
    APIU32	distribution; // enDistribution - Distribution type of this address. for H.245 addresses
	APIU32	transportType;	// enTransportType - TCP,UDP,TLS
} mcTransportAddress;

/* example of usage:
mcTransportAddress TA;

TA.type =           Address TYPE.
TA.addr.v6.ip =     IP v6, or-
TA.addr.v4.ip =     IP v4, and maybe-
TA.addr.v4.route =  IP v4 route.
TA.port =           Port for any IP type
TA.distribution =   Distribution for any IP type
*/

typedef enum {
	eScopeIdSite				= 0,
	eScopeIdGlobal				= 1,
	eScopeIdLink				= 3,
	eScopeIdOther				= 4
}enScopeId;

// currently, following struct is being used only by mcms and Emb
typedef struct
{
	union	ipAddressIf addr;	
	APIU32	ipVersion; // enIpVersion
} ipAddressStruct;


//-S- PLCM_DNS ------------------------------//
#define PLCM_DNS_HOST_NAME_SIZE			256
#define PLCM_DNS_DOMAIN_NAME_SIZE		64
#define PLCM_HOST_NAME_AMOUNT			100
#define PLCM_HOST_IPV4_AMOUNT			10
#define PLCM_HOST_IPV6_AMOUNT			10

#define PLCM_DNS_RECORDS_AMOUNT			((PLCM_HOST_NAME_AMOUNT)*(PLCM_HOST_IPV4_AMOUNT + PLCM_HOST_IPV6_AMOUNT)) * 10//

typedef enum _eDnsReslvSTATUS
{
	eDnsStatusResolved = 0
  , eDnsStatusNotResolved
  , eDnsStatusTimeOut

  , eDnsStatusMAX
}
eDnsReslvSTATUS;

typedef struct _sDNS_RECORD
{
	unsigned int		dwRawId									;
    unsigned int		dwLastUseTimeStamp						;
	char				szHostName	[PLCM_DNS_HOST_NAME_SIZE]	;
	char				szDomainName[PLCM_DNS_DOMAIN_NAME_SIZE]	;
	ipAddressStruct		sResolveIp								;
	eDnsReslvSTATUS		eStatus								    ;
	unsigned int		dwTimeStamp_Responce					;
	unsigned int		dwTTL									;
	unsigned int		dwTimeStamp_NextReq						;
	ipAddressStruct		sDnsAddress								;
	unsigned short      wServiceId								;
	char				Reserve [22]							;
}
sDNS_RECORD;
//-E- PLCM_DNS ------------------------------//


#endif // __IPADDRESSDEFINITIONS_H__
