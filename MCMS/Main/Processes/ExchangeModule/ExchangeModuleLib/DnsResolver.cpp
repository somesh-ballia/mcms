//============================================================================
// Name        : DnsResolver.cpp
// Author      : Vasily
// Version     :
// Copyright   : Copyright (c) Polycom Inc 2010
// Description : DNS resolver
//============================================================================


//#include <iostream>
//#include <sstream>



#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>

#undef STATUS

#include "IpAddressDefinitions.h"
#include "StatusesGeneral.h"
#include "SystemFunctions.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "TraceStream.h"
#include "OsTask.h"
#include "DnsResolver.h"

typedef union {
        HEADER hdr;           /* defined in resolv.h */
        u_char buf[PACKETSZ]; /* defined in arpa/nameser.h */
} response_t;               /* response buffers */


typedef struct dnshost_st
{
    unsigned int        type;
    unsigned int        _class;
	unsigned int        ttl;
} dnshost_t;

typedef struct dns_srv_st
{
    unsigned int        priority;
    unsigned int        weight;
    unsigned int        port;
    unsigned int        rweight;

    char                name[256];
} dns_srv_t;


/////////////////////////////////////////////////////////////////////////////
STATUS SystemDNSNameResolve2(const char* name, DWORD &ResIp, DWORD &ResTtl)
{
	STATUS status = STATUS_OK;
	DWORD Ip = 0;
	response_t response;
	res_init();
//	_res.options |= RES_USE_INET6;//rons -IPV6 SUPPORT
	int responseLen = res_search(name,      /* the zone we care about   */
                     			ns_c_in,     /* Internet class records     */
			                    ns_t_a,      /* Look up host address*/
            			        (u_char *)&response,      /*response buffer*/
			                    sizeof(response));        /*buffer size    */
	if(0 > responseLen)
	{
		switch(h_errno){
	        case HOST_NOT_FOUND:
		      status = STATUS_HOST_NOT_FOUND;
	          break;
	        case NO_DATA:
	          status = STATUS_NO_DATA;
	          break;
	        case TRY_AGAIN:
	          status = STATUS_TRY_AGAIN;
	          break;
	        default:
	          status = STATUS_FAIL;
	          break;
	    }
        return status;                  /* and quit                   */
    }

    ns_msg handle;  /* handle for response message */
    if (ns_initparse(response.buf, responseLen, &handle) < 0)
    {
        status = STATUS_FAIL;
        return status;
    }

    ns_sect section = ns_s_an;
    int rrnum=0;  /* resource record number */
    int num_msgs = ns_msg_count(handle, section);
    ns_rr rr;   /* expanded resource record */

    int i, dup; /* misc variables */

    /*
     * Look at all the resource records in this section.
     */
    for(rrnum = 0; rrnum < num_msgs; rrnum++)
    {
        /*
         * Expand the resource record number rrnum into rr.
         */
        if (ns_parserr(&handle, section, rrnum, &rr)) {
            status = STATUS_FAIL;
        	return status;
        }

        /*
         * If the record type is host address, save the name of the
         * name server.
         */
        if (ns_rr_type(rr) == ns_t_a)
        {
            ResTtl = ns_rr_ttl(rr);
 //           size_t rdlen = ns_rr_rdlen(rr); //rons -IPV6 SUPPORT
 //           if (rdlen == (size_t)NS_IN6ADDRSZ)//rons -IPV6 SUPPORT
 //          {
 //           	const char *pIpAddressIpv6 = (const char*)ns_rr_rdata(rr);
 //           }
            const char *pIpAddress = (const char*)ns_rr_rdata(rr);
            char temp[4];
			temp[0] = pIpAddress[3];
			temp[1] = pIpAddress[2];
			temp[2] = pIpAddress[1];
			temp[3] = pIpAddress[0];

			DWORD *tmp = (DWORD*)temp;
			ResIp = *tmp;
        }
        else if (ns_rr_type(rr) == ns_t_aaaa) //rons -IpV6
        {
        	ResTtl = ns_rr_ttl(rr);
            size_t rdlen = ns_rr_rdlen(rr); //rons -IPV6 SUPPORT
           	const char *pIpAddressIpv6 = (const char*)ns_rr_rdata(rr);

        }
        else
        {
        	status = STATUS_FAIL;
        	return status;
        }
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////
//STATUS SystemDNSNameResolve3(const char* name, ipAddressStruct *pAddrList, int listSize, DWORD &ResTtl)
//{
//	struct addrinfo aiHints;
//	struct addrinfo *pAiHints = &aiHints;
//	struct addrinfo *pAiList = NULL;
//	struct addrinfo *pAi;
//
//	 struct sockaddr_in *sin;
//	 struct sockaddr_in6 *sin6;
//
//	 ipAddressStruct *pIpAddrElem = pAddrList;
//
//	 CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
//	 sysConfig->GetDWORDDataByKey("TTL", ResTtl);
//	 FTRACESTR(eLevelInfoNormal) << "SystemDNSNameResolve3 ttl = " << ResTtl;
////   ResTtl = 60;
//
//	 memset(pAiHints, 0, sizeof(aiHints));
//	// for(int i=0; i<listSize ; i++)
//	// {
//	 	//memset(pAddrList[i], 0, sizeof(ipAddressStruct));
//	// }
//
//	 // Setup the hints address info structure
//	 // which is passed to the getaddrinfo() function
//	 pAiHints->ai_flags   = AI_PASSIVE;//AI_CANONNAME;
//	 pAiHints->ai_family  = PF_UNSPEC;
//	 pAiHints->ai_socktype  = SOCK_STREAM;
//	 pAiHints->ai_protocol  = IPPROTO_TCP;
//
//	// if (ipType == enIPv4Only)
//	//  pAiHints->ai_family = AF_INET;
//	// else if (ipType == enIPv6Only)
//	//  pAiHints->ai_family = AF_INET6;
//	// else
//	 pAiHints->ai_family = AF_UNSPEC;
//
//	 STATUS status;
//	 FTRACESTR(eLevelInfoNormal) << "SystemDNSNameResolve3 name = " << name;
//     if(TRUE == IsTarget())
//     {
//
//         if ((status = getaddrinfo(name, /*"172.22.188.178"*/NULL, pAiHints, &pAiList)) != 0) {
//             FTRACESTR(eLevelInfoNormal) << "SystemDNSNameResolve3:  getaddrinfo() failed: Can't get Host " <<  name << ", status = " << status;
//             //	PTRACE2(eLevelInfoNormal, "SystemDNSNameResolve3:", s);
//             //sprintf(errBuf, "getaddrinfo() failed:[%d] Host not found", retVal);
//             return status;
//         }
//     }
//     else
//     {
//         ResTtl=0x01020304;
//         return STATUS_OK;
//
//     }
//
//
//
//	 int j=0;
//	 for (pAi = pAiList; pAi; pAi = pAi->ai_next, pIpAddrElem++/*, pAddrList->nIpAddrs++*/)
//	 {
//		 switch (pAi->ai_family)
//		 {
//			 case AF_INET://IpV4
//		     {
//		    	 sin = (struct sockaddr_in*) pAi->ai_addr;
//
//		    	 pAddrList[j].addr.v4.ip = SystemIpStringToDWORD(inet_ntoa(sin->sin_addr));
//		  		 pAddrList[j].ipVersion = eIpVersion4;
//
//		  		char addrIpV4[16];
//		  		SystemDWORDToIpString((DWORD)pAddrList[j].addr.v4.ip, addrIpV4);
//		  		FTRACESTR(eLevelInfoNormal) << "SystemDNSNameResolve3(Ipv4):  Host " <<  name << ", is on address = " << addrIpV4;
//
//				 j++;
//		  		 break;
//		     }
//			 case AF_INET6://IpV6
//			 {
//			  	 sin6 = (struct sockaddr_in6*) pAi->ai_addr;
//			  	 char tmpIPv6 [INET6_ADDRSTRLEN];
//			  	 mcTransportAddress tmpIPv6Addr;
//
//			  	 inet_ntop(AF_INET6, sin6->sin6_addr.s6_addr , &tmpIPv6[0],  INET6_ADDRSTRLEN);
//			  	 ::stringToIpV6(&tmpIPv6Addr,tmpIPv6);
//
//			  	 enScopeId scopeId = ::getScopeId(tmpIPv6);
//
//			  	 if(eScopeIdSite == scopeId || eScopeIdGlobal == scopeId)
//			  	 {
//			  		 pAddrList[j].ipVersion = eIpVersion6;
//			  		 memcpy(&pAddrList[j].addr.v6.ip, tmpIPv6Addr.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
//					 pAddrList[j].addr.v6.scopeId = scopeId;
//			  	 }
//			  	 else
//			  		FTRACESTR(eLevelInfoNormal) << "SystemDNSNameResolve3:  unexpected Scope:" <<  scopeId;
//
//			  	 char addrIpV6[128];
//			  	 ipV6ToString(pAddrList[j].addr.v6.ip, addrIpV6, TRUE);
//			  	 FTRACESTR(eLevelInfoNormal) << "SystemDNSNameResolve3(Ipv6):  Host " <<  name << ", is on address = " << addrIpV6;
//
//				 j++;
//			  	 break;
//			 }
//
//			 default:
//				 FTRACESTR(eLevelInfoNormal) << "SystemDNSNameResolve3(Ipv6):  Unexpected family type:" << pAi->ai_family;
//				 break;
//		 }
//	 }
//
//	 if (pAiList)
//	     freeaddrinfo(pAiList);
//
//	 return status;
//
//}

/////////////////////////////////////////////////////////////////////////////
STATUS SystemDNSNameResolve(const char* name, DWORD &ResIp, DWORD &ResTtl)
{
	STATUS status = STATUS_OK;
	DWORD Ip = 0;
	response_t response;

	res_init();

    int responseLen;          /* buffer length */

    u_char     *cp;       /* character pointer to parse DNS packet */
    u_char     *endOfMsg; /* need to know the end of the message */
    u_short    _class;     /* classes defined in arpa/nameser.h */
    u_short    type;      /* types defined in arpa/nameser.h */
    u_int32_t  ttl;       /* resource record time to live */
    u_short    dlen;      /* size of resource record data */

    int i, count, dup; /* misc variables */

    /*
     * Look up the NS records for the given domain name.
     * We expect the domain to be a fully qualified name, so
     * we use res_query().  If we wanted the resolver search
     * algorithm, we would have used res_search() instead.
     */
    if((responseLen =
           res_search(name,        /* the domain we care about   */
                     C_IN,        /* Internet class records     */
                     T_A,         /* Look up name server records*/
                     (u_char *)&response,      /*response buffer*/
                     sizeof(response)))        /*buffer size    */
                                        < 0)
    {  /*If negative    */
		switch(h_errno){
	        case HOST_NOT_FOUND:
		      status = STATUS_HOST_NOT_FOUND;
	          break;
	        case NO_DATA:
	          status = STATUS_NO_DATA;
	          break;
	        case TRY_AGAIN:
	          status = STATUS_TRY_AGAIN;
	          break;
	        default:
	          status = STATUS_FAIL;
	          break;
	    }
        return status;                  /* and quit                   */
    }

    /*
     * Keep track of the end of the message so we don't
     * pass it while parsing the response.  responseLen is
     * the value returned by res_query.
     */
    endOfMsg = response.buf + responseLen;

    /*
     * Set a pointer to the start of the question section,
     * which begins immediately AFTER the header.
     */
    cp = response.buf + sizeof(HEADER);

    /*
     * Skip over the whole question section.  The question
     * section is comprised of a name, a type, and a class.
     * QFIXEDSZ (defined in arpa/nameser.h) is the size of
     * the type and class portions, which is fixed.  Therefore,
     * we can skip the question section by skipping the
     * name (at the beginning) and then advancing QFIXEDSZ.
     * After this calculation, cp points to the start of the
     * answer section, which is a list of NS records.
     */

	int n = dn_skipname(cp, endOfMsg); //SkipName
    cp += n + QFIXEDSZ;

    /*
     * Create a list of name servers from the response.
     * NS records may be in the answer section and/or in the
     * authority section depending on the DNS implementation.
     * Walk through both.  The name server addresses may be in
     * the additional records section, but we will ignore them
     * since it is much easier to call gethostbyname() later
     * than to parse and store the addresses here.
     */
    count = ntohs(response.hdr.ancount) +
            ntohs(response.hdr.nscount);
    if ((--count >= 0)        /* still more records     */
        && (cp < endOfMsg))	 /* still inside the packet*/
    {

        /* Skip to the data portion of the resource record */

		u_char *tmp_cp = cp;  /* temporary version of cp */

		/* Skip the domain name; it matches the name we looked up */
		int n = dn_skipname(tmp_cp, endOfMsg); //SkipName
		tmp_cp += n;

		/*
		 * Grab the type, class, and ttl.  GETSHORT and GETLONG
		 * are macros defined in arpa/nameser.h.
		 */
		GETSHORT(type, tmp_cp);
		GETSHORT(_class, tmp_cp);
		GETLONG(ttl, tmp_cp);
		GETSHORT(dlen, tmp_cp);

        cp += tmp_cp - cp;

        if(T_A == type)
        {
			char temp[4];
			temp[0] = cp[3];
			temp[1] = cp[2];
			temp[2] = cp[1];
			temp[3] = cp[0];

			DWORD *tmp = (DWORD*)temp;
			ResIp = *tmp;
			ResTtl = ttl;
        }

        /* Advance the pointer over the resource record data */
        //cp += dlen;
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////
//STATUS SystemDNS_SRVQuery(const char* domain, DWORD transportType, char* hostName,DWORD& resIp, DWORD& resTtl,
//						WORD& resPort, WORD& resPriority, WORD& resWeight)
//{
//	STATUS status = STATUS_OK;
//
//    response_t packet;
//    int len, qdcount, ancount, an, n;
//    unsigned char *eom, *scan;
//    unsigned int type, _class, ttl;
//    dns_srv_t	srv;
//    dnshost_st 	reply;
//
//    if(domain == NULL || *domain == '\0')
//        status = STATUS_FAIL;
//    else
//    {
//    	res_init();
//    	char request[256];
//    	if(eTransportTypeTcp == transportType)
//    		sprintf(request, "_sip._tcp.%s", domain);
//    	else
//    		if(eTransportTypeUdp == transportType)
//    			sprintf(request, "_sip._udp.%s", domain);
//
//    	len = res_query(request, C_IN, T_SRV, packet.buf, PACKETSZ);
//    	if(len < 0)
//    	{
//    		switch(h_errno)
//    		{
//		        case HOST_NOT_FOUND:
//			      status = STATUS_HOST_NOT_FOUND;
//		          break;
//		        case NO_DATA:
//		          status = STATUS_NO_DATA;
//		          break;
//		        case TRY_AGAIN:
//		          status = STATUS_TRY_AGAIN;
//		          break;
//		        default:
//		          status = STATUS_FAIL;
//		          break;
//	    	}
//    	}
//    }
//    if(STATUS_OK == status)
//    {
//    	/* no. of packets, no. of answers */
//    	qdcount = ntohs(packet.hdr.qdcount);
//	    ancount = ntohs(packet.hdr.ancount);
//
//    	 /* end of the returned message */
//	     eom = (unsigned char *) (packet.buf + len);
//
//	     /* our current location */
//    	 scan = (unsigned char *) (packet.buf + sizeof(HEADER));
//
//	     /* skip over the packet records */
//    	 while(qdcount > 0 && scan < eom)
//    	 {
//        	 qdcount--;
//	         if((len = dn_expand(packet.buf, eom, scan, hostName, 256)) < 0)
//	         {
//    	         status = STATUS_FAIL;
//    	         break;
//	         }
//    	     else
//	        	 scan = (unsigned char *) (scan + len + QFIXEDSZ);
//	     }
//
//	     an = 0;
//    	 /* loop through the answer buffer and extract SRV records */
//	     while(ancount > 0 && scan < eom )
//	     {
//	     	 memset(&reply, 0, sizeof(dnshost_t));
//    	     ancount--;
//        	 len = dn_expand(packet.buf, eom, scan, hostName, 256);
//	         if(len < 0)
//	         {
//    	         status = STATUS_FAIL;
//    	         break;
//        	 }
// 			 else
// 			 {
//		         scan += len;
//
//		         /* extract the various parts of the record */
//	    	     GETSHORT(type, scan);
//	        	 GETSHORT(_class, scan);
//		         GETLONG(ttl, scan);
//	    	     GETSHORT(len, scan);
//
//		         /* skip records we're not interested in */
//		         if(T_SRV != type)
//		         {
//		             scan = (unsigned char *) (scan + len);
//		             continue;
//		         }
//
//		         reply.type = type;
//	    	     reply._class = _class;
//		         reply.ttl = ttl;
//
// 			 	 memset(&srv, 0, sizeof(dns_srv_t));
//
//	    		 if(T_SRV == type)
//	    		 {
//	    		 	GETSHORT(srv.priority, scan);
//    			  	GETSHORT(srv.weight, scan);
//     				GETSHORT(srv.port, scan);
//     				if(srv.weight != 0)
//        				srv.rweight = 1;
//    				else
//         				srv.rweight = 0;
//
//
//     				len = dn_expand(packet.buf, eom, scan, srv.name, 256);
//     				if (len < 0)
//   					{
//   						memset(&srv, 0, sizeof(dns_srv_t));
//   						break;
//   					}
//	    		 }
//	    		 else
//	    		 	scan = (unsigned char *) (scan + len);
//
//
//
//         		 /* on to the next one */
//         		 an++;
// 			 }
//     	 }
//    }
//
//    if(STATUS_OK == status && *srv.name!='\0')
//    {
//    	DWORD ip=0, ttl=0;
//    	status = SystemDNSNameResolve2(srv.name, ip, ttl);
//    	if(STATUS_OK == status && ip!=0)
//    	{
//    		strncpy(hostName, srv.name, 256);
//    		resIp = ip;
//    		resTtl = ttl;
//    		resPort = srv.port;
//    		resPriority = srv.priority;
//    		resWeight = srv.rweight;
//    	}
//    }
//    return status;
//}


const int CDnsResolver::MIN_RESOLV_INTERVAL_IN_SEC = 30;

///////////////////////////////////////////////////////////////////////////////////////////////////
CDnsResolver::CDnsResolver() : m_status(CDnsResolver::eStatusUnknown), m_resolvTime(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CDnsResolver::~CDnsResolver()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CDnsResolver::EDnsResolverStatus CDnsResolver::Resolv(const char* pszDnsName)
{
	if( pszDnsName == NULL || strlen(pszDnsName) < 1 )
		return eStatusUnknown;

	time_t now = 0;
	// if Dns name as previous
	if( m_strDnsName == pszDnsName )
	{
		// check if Min interval not finished, return previous status
		now = time((time_t*)NULL);
		if( now - m_resolvTime < MIN_RESOLV_INTERVAL_IN_SEC )
			return m_status;
	}

	m_resolvTime = now;
	m_strDnsName = pszDnsName;

	DWORD ip = 0;
	DWORD resTtl = 0;
	char addrIpV4[16];

	// resolv here
	
	CTaskApp * taskApp = COsTask::GetTaskApp();
	if(taskApp == NULL)
	{
		return eStatusUnknown;
	}

	taskApp->UnlockRelevantSemaphore();
	STATUS stat = SystemDNSNameResolve2(m_strDnsName.c_str(),ip,resTtl);
	taskApp->LockRelevantSemaphore();

    switch( stat )
    {
		case STATUS_OK:
			m_status = eStatusOk;
			break;
		case STATUS_HOST_NOT_FOUND:
			m_status = eStatusHostNotFound;
			break;
		case STATUS_TRY_AGAIN:
			m_status = eStatusTryAgain;
			break;
		case STATUS_NO_DATA:
			m_status = eStatusNoData;
			break;
    }

    if( m_status != eStatusOk )
    	return m_status;

    SystemDWORDToIpString(ip, addrIpV4);

    m_strIp = addrIpV4;

    return eStatusOk;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const char* CDnsResolver::StatusAsString(const CDnsResolver::EDnsResolverStatus status)
{
	switch( status )
	{
		case eStatusOk:
			return "eStatusOk";
		case eStatusHostNotFound:
			return "eStatusHostNotFound";
		case eStatusTryAgain:
			return "eStatusTryAgain";
		case eStatusNoData:
			return "eStatusNoData";
		case eStatusUnknown:
			return "eStatusUnknown";
	}
	return "Unknown!";
}



























