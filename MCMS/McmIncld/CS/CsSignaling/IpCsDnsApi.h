// IpCsDnsApi.h
// Ami Noy

#ifndef IPCSDNSAPI_H_
#define IPCSDNSAPI_H_

#include <MplMcmsStructs.h>
#include <IpCommonDefinitions.h>
#include <IpCsSizeDefinitions.h>
#include <CsHeader.h>

// DNS Request
//-------------
typedef struct {
	int  	transactionId;
	char 	hostName[DnsQueryNameSize];
}  DNS_CS_RESOLVE_REQ_ST;

typedef struct {
	COMMON_HEADER_S 				commonHeader;
	MESSAGE_DESCRIPTION_HEADER_S	msgDescHeader;
	CENTRAL_SIGNALING_HEADER_S		csHeader;	
	DNS_CS_RESOLVE_REQ_ST			dnsCsResolveReq;
} McReqDnsResolve;

typedef struct {
	int  	transactionId;
	int 	transportType;
	char 	domainName[DnsQueryNameSize];
}  DNS_CS_SERVICE_REQ_ST;

typedef struct {
	COMMON_HEADER_S 				commonHeader;
	MESSAGE_DESCRIPTION_HEADER_S	msgDescHeader;
	CENTRAL_SIGNALING_HEADER_S		csHeader;	
	DNS_CS_SERVICE_REQ_ST			dnsCsServiceReq;
} McReqDnsService;

// DNS Indication
//---------------
typedef enum {
	typeDnsResolve,
	typeDnsService,
	numOfDnsTblTypes,
} tblDnsTypes;

typedef struct {
	int		ipType;	
	char 	ipAddress[MaxIpAddressStringSize];
} ipAddressElemSt;

typedef struct {
	xmlDynamicHeader	xmlHeader;
	ipAddressElemSt		elem;
} xmlIpAddressElemSt;
	
typedef struct {
	int 				status;
	int 				transactionId;
	char 				hostName[DnsQueryNameSize];
	char				errReason[DnsErrDescSize];
	
	int						nAddresses;
	xmlDynamicProperties	xmlDynamicProps;
	char					ipAddrList[1];

} DNS_CS_RESOLVE_IND_ST;

typedef struct {
	COMMON_HEADER_S 				commonHeader;	
	MESSAGE_DESCRIPTION_HEADER_S	msgDescHeader;
	CENTRAL_SIGNALING_HEADER_S		csHeader;	
	DNS_CS_RESOLVE_IND_ST 			dnsCsResolveInd;
} McIndDnsResolve;

typedef struct {
	int 				status;
	int 				transactionId;
	
	char 				domainName[DnsQueryNameSize];
	char 				hostName[DnsQueryNameSize];
	char				errReason[DnsErrDescSize];
	
	UINT32				ttl;
	INT16 				port;
	INT16				priority;
	INT32				weight;
	
	int						nAddresses;
	xmlDynamicProperties	xmlDynamicProps;
	char					ipAddrList[1];	

} DNS_CS_SERVICE_IND_ST;

typedef struct {
	COMMON_HEADER_S 				commonHeader;	
	MESSAGE_DESCRIPTION_HEADER_S	msgDescHeader;
	CENTRAL_SIGNALING_HEADER_S		csHeader;	
	DNS_CS_SERVICE_IND_ST 			dnsCsServiceInd;
} McIndDnsService;

#endif /*IPCSDNSAPI_H_*/
