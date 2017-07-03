#ifndef SYSTEMQUERY_H_
#define SYSTEMQUERY_H_

#include "DataTypes.h"
//-S- PLCM_DNS ------------------------------//
#include "PlcmDNS_Packet.h"
//-E- PLCM_DNS ------------------------------//


STATUS	SystemDNSNameResolve(const char* name, DWORD &ip, DWORD &ttl);
STATUS  SystemDNSNameResolve2(const char* name, DWORD &Ip, DWORD &ttl);
STATUS  SystemDNSNameResolve3(const char* name, ipAddressStruct *pAddrList, int listSize, DWORD &ResTtl);

STATUS	SystemDNS_SRVQuery(const char* domain, DWORD transportType, char* hostName,DWORD& resIp, DWORD& resTtl, 
						WORD& resPort, WORD& resPriority, WORD& resWeight);

//-S- PLCM_DNS ------------------------------//
int     plcmMakeDnsQuery(char				*	par_HostName
					   , char				*	par_DomainName
					   , DNS_REQ_RES_TYPE		par_eReqType
					   , unsigned short			par_wReqId
					   , unsigned char		*	par_pOutBuff
					   , int					par_nOutBuffLen);
//-E- PLCM_DNS ------------------------------//
#endif /*SYSTEMQUERY_H_*/


