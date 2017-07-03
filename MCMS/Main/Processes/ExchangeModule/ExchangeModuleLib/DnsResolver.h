//============================================================================
// Name        : DnsResolver.h
// Author      : Vasily
// Version     :
// Copyright   : Copyright (c) Polycom Inc 2010
// Description : DNS resolver
//============================================================================


#if !defined(__DnsResolver_h__)
#define __DnsResolver_h__

#include <string>
#include "DataTypes.h"

#define STATUS_HOST_NOT_FOUND								 100000
#define STATUS_TRY_AGAIN									 100001
#define STATUS_NO_DATA										 100002


STATUS	SystemDNSNameResolve(const char* name, DWORD &ip, DWORD &ttl);
STATUS  SystemDNSNameResolve2(const char* name, DWORD &Ip, DWORD &ttl);
//STATUS  SystemDNSNameResolve3(const char* name, ipAddressStruct *pAddrList, int listSize, DWORD &ResTtl);

//STATUS	SystemDNS_SRVQuery(const char* domain, DWORD transportType, char* hostName,DWORD& resIp, DWORD& resTtl,
//						WORD& resPort, WORD& resPriority, WORD& resWeight);


class CDnsResolver
{
public:

	const static int MIN_RESOLV_INTERVAL_IN_SEC;
	enum EDnsResolverStatus { eStatusOk = 0, eStatusHostNotFound, eStatusTryAgain, eStatusNoData, eStatusUnknown /*must be last*/ };

	CDnsResolver();
	virtual ~CDnsResolver();

	EDnsResolverStatus Resolv(const char* pszDnsName);

	EDnsResolverStatus GetResolvStatus() const { return m_status; }
	std::string GetIp() const { return m_strIp; }

	static const char* StatusAsString(const EDnsResolverStatus status);

protected:
	EDnsResolverStatus  m_status;
	std::string m_strDnsName;
	std::string m_strIp;
	time_t m_resolvTime;
};

#endif /*__DnsResolver_h__*/

















