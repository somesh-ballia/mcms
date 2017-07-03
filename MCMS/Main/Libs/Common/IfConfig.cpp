// AcIfConfig
// Ami Noy

// Pre-definitions:
// -------------

#undef  SRC
#define SRC         "ACIFCONF"





// Include files:
// --------------
#include <sys/param.h>
#include <string.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/route.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <stdlib.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <netinet/in.h>


#include "IfConfig.h"
#include "IfStructs.h"
#include "StatusesGeneral.h"
#include "InternalProcessStatuses.h"
#include "SystemFunctions.h"

//#include <AcPrintLog.h>
//#include <AcErrors.h>


// Macros:
// -------

// Compile time flags:
// -------------------


// Structures:
// -----------

// Global variables:
// -----------------

// External routines:
// ------------------

// External variables:
// -------------------
extern int errno;

// Static variables:
// -----------------
static int sfd = 0;

// Run time variables
// ------------------

// Forward declarations:
// ---------------------

// Static routines:
// -----------------
//---------------------------------------------------------------------------
//
//  Function name:  SetIpv4Address
//
//  Description:
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//---------------------------------------------------------------------------

static int SetIpv4Address(struct ifreq *pIfr, DWORD ipAddress)
{
	int status;

	struct sockaddr_in	*pSin;

	pSin = (struct sockaddr_in *) &pIfr->ifr_addr;

	memset(
		pSin,
		0,
		sizeof(struct sockaddr));

	pSin->sin_family = AF_INET;
	pSin->sin_addr.s_addr = ntohl(ipAddress);

// ---- start of debug --------
#ifdef __IfDebug__
	pSin->sin_addr.s_addr = 0;// to delete interface - just for debug

	status = ioctl(sfd, SIOCSIFADDR, pIfr);

	if (status < 0) {
		AcPrintLog("errno = %d", PERR, errno);
		return E_SERVICE_IF_IOCTL_FAIL;
	}
	pSin->sin_addr.s_addr = htonl(0xac16c0dc);
#endif // __IfDebug__
// ----- end of debug --------

	status = ioctl(sfd, SIOCSIFADDR, pIfr);

	if (status < 0) {

//		AcPrintLog("errno = %d", PERR, errno);

		if (errno == 13)//permission denied - must be administartor
			return 0;

//		return E_SERVICE_IF_IOCTL_FAIL;
		return errno;
	}

	return 0;

}// SetIpv4Address

//---------------------------------------------------------------------------
//
//  Function name:  GetIpv4Address
//
//  Description:
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//---------------------------------------------------------------------------
static int GetIpv4Address(struct ifreq *pIfr, DWORD *ipAddress)
{
	int status;
	struct sockaddr_in	*pSin;
	pSin = (struct sockaddr_in *) &pIfr->ifr_addr;

	status = ioctl(sfd, SIOCGIFADDR, pIfr);

	if(status < 0)
		return errno;

	if(AF_INET == pSin->sin_family)
		*ipAddress = htonl(pSin->sin_addr.s_addr);

	return 0;
}

//---------------------------------------------------------------------------
//
//  Function name:  SetIpv4SubnetMask
//
//  Description:
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//---------------------------------------------------------------------------
static int SetIpv4SubnetMask(struct ifreq *pIfr, DWORD subnetMask)
{
	int status;

	struct sockaddr_in	*pSin;

	pSin = (struct sockaddr_in *) &pIfr->ifr_netmask;

	memset(
		pSin,
		0,
		sizeof(struct sockaddr));

	pSin->sin_family = AF_INET;
	pSin->sin_addr.s_addr = htonl(subnetMask/*0xffffff00*/);

	status = ioctl(sfd, SIOCSIFNETMASK, pIfr);

	if (status < 0) {

//		AcPrintLog("errno = %d", PERR, errno);

		if (errno == 13)//permission denied - must be administartor
			return 0;

//		return E_SERVICE_IF_IOCTL_FAIL;
		return errno;
	}

	return 0;

}// SetIpv4SubnetMask

//---------------------------------------------------------------------------
//
//  Function name:  SetIpv4Broadcast
//
//  Description:
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//---------------------------------------------------------------------------
static int SetIpv4Broadcast(struct ifreq *pIfr, DWORD broadcast)
{
	int status;

	struct sockaddr_in	*pSin;

	pSin = (struct sockaddr_in *) &pIfr->ifr_broadaddr;

	memset(
		pSin,
		0,
		sizeof(struct sockaddr));

	pSin->sin_family = AF_INET;
	pSin->sin_addr.s_addr = htonl(broadcast/*0xffffff00*/);

	status = ioctl(sfd, SIOCSIFBRDADDR, pIfr);

	if (status < 0) {


		if (errno == 13)//permission denied - must be administartor
			return 0;

		return errno;
	}

	return 0;
}

//---------------------------------------------------------------------------
//
//  Function name:  AddDefaultGw
//
//  Description: Add default gw to dev
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//---------------------------------------------------------------------------
int AddDefaultGw(struct ifreq *pIfr, DWORD gw)
{
	int status;
	struct rtentry route;
	struct sockaddr_in *pGwSin = (struct sockaddr_in *)(&(route.rt_gateway));
	struct sockaddr_in *pDstSin = (struct sockaddr_in *)(&(route.rt_dst));
	struct sockaddr_in *pMaskSin = (struct sockaddr_in *)(&(route.rt_genmask));

	memset(&route, 0, sizeof(struct rtentry));

	pGwSin->sin_family = AF_INET;
	pDstSin->sin_family = AF_INET;
	pMaskSin->sin_family = AF_INET;

	pGwSin->sin_addr.s_addr = htonl(gw/*0xffffff00*/);

	route.rt_flags = RTF_UP | RTF_GATEWAY;
	route.rt_dev = pIfr->ifr_name;

	status = ioctl(sfd, SIOCADDRT, &route);

	if (status < 0) {

		if (errno == 13)//permission denied - must be administartor
			return 0;

		//if entry exists in route (FILE EXISTS), then this failure is actually OK since it is configured.
		if(EEXIST == errno)
			return 0;

		return errno;
	}

	return 0;
}



//---------------------------------------------------------------------------
//
//  Function name:  AddIpRoute
//
//  Description: Add route entry
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//---------------------------------------------------------------------------
int	AddIpRoute(DWORD target_ip,
				DWORD submask,
				DWORD gateway,
				DWORD type,
				char* dev)
{
	int status;
	struct rtentry route;

	struct sockaddr_in *pGwSin = (struct sockaddr_in *)(&(route.rt_gateway));
	struct sockaddr_in *pDstSin = (struct sockaddr_in *)(&(route.rt_dst));
	struct sockaddr_in *pMaskSin = (struct sockaddr_in *)(&(route.rt_genmask));

	memset(&route, 0, sizeof(struct rtentry));

	pGwSin->sin_family = AF_INET;
	pDstSin->sin_family = AF_INET;
	pMaskSin->sin_family = AF_INET;

	pGwSin->sin_addr.s_addr = htonl(gateway/*0xffffff00*/);
	pDstSin->sin_addr.s_addr = htonl(target_ip/*0xffffff00*/);
	pMaskSin->sin_addr.s_addr = htonl(submask/*0xffffff00*/);

	route.rt_flags = RTF_UP;

	if(router_host == type)
    	route.rt_flags |= RTF_HOST;

	route.rt_dev = dev;

	status = ioctl(sfd, SIOCADDRT, &route);

	if (status < 0) {

		if (errno == 13)//permission denied - must be administartor
			return 0;

		//if entry exists in route (FILE EXISTS), then this failure is actually OK since it is configured.
		if(EEXIST == errno)
			return 0;

		return errno;
	}

	return 0;
}

//---------------------------------------------------------------------------
//
//  Function name:  SetIpv4Mtu
//
//  Description:
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//---------------------------------------------------------------------------
static int SetIpv4Mtu(struct ifreq *pIfr, int mtu)
{
	int status;

	memset(
		&pIfr->ifr_ifru,
		0,
		sizeof(struct ifreq) - IFNAMSIZ);

	pIfr->ifr_mtu = mtu;

	status = ioctl(sfd, SIOCSIFMTU, pIfr);

	if (status < 0) {
//		AcPrintLog("errno = %d", PERR, errno);
//		return E_SERVICE_IF_IOCTL_FAIL;
		return errno;
	}

	return 0;

}// SetIpv4Mtu


// Global routines
//----------------
//---------------------------------------------------------------------------
//
//  Function name:  AcDelNi
//
//  Description: Delete network interface
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//  Error bit mask:
//	---------------
//	MSB																							LSB
//	15	14	13	12	11	10	9	8	7	6	5	4		3				2			1			0
//  NA	NA	NA	NA	NA	NA	NA	NA	NA	NA	NA	route	default gw		broadcast	submask		IpAddress
//
//---------------------------------------------------------------------------
WORD AcAddNi(
	char 	*pIfName,	// IN
	DWORD	ipAddress,	// IN
	DWORD	subnetMask, // IN
	DWORD	broadcast,  // IN
	DWORD   default_gw, // IN
	std::list<CRouter> &routerList, // IN
	char 	*route_dev,
	DWORD   &ret) // IN
{
	STATUS status = STATUS_OK;
	ret = 0x0000;

	struct ifreq ifr;

	memset (&ifr, 0, sizeof (ifr));
  	strncpy (ifr.ifr_name, pIfName, sizeof(ifr.ifr_name) - 1);

  	ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';

	if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		status = STATUS_FAIL;
		ret = 0xFFFF;
	}
	if(STATUS_OK == status)
	{
		/*DWORD current_ipAddress = 0;
		status = GetIpv4Address(&ifr, &current_ipAddress);
		//Updtae IP address only if needed
		if(STATUS_OK == status)
		{
			if(current_ipAddress != ipAddress)*/
				status = SetIpv4Address(&ifr, ipAddress);
			/*else
				ret = 0xee00;
		}
		else
			ret=0xef00;*/

		if(STATUS_OK == status)
		{
			status = SetIpv4SubnetMask(&ifr, subnetMask);
			if(STATUS_OK == status)
			{
				status = SetIpv4Broadcast(&ifr, broadcast);
				if(STATUS_OK == status)
				{
					if (default_gw != 0 && default_gw != 0xFFFFFFFF)
					{
						status = AddDefaultGw(&ifr, default_gw);
						if(STATUS_OK != status)
							ret = 0x0008;
					}
				}
				else
					ret = 0x0004;
			}
			else
				ret = 0x0002;
		}
		else
			ret = 0x0001;
	}


	if (status == STATUS_OK)
    {
		for (std::list<CRouter>::const_iterator itr = routerList.begin(); itr != routerList.end(); itr++)
	    {
	    	if (itr->m_targetIP != "255.255.255.255")
	    	{
		        mcTransportAddress targetAddr, subMask, gatewayAddr;
		        stringToIp(&targetAddr, (char*)itr->m_targetIP.c_str());
		        stringToIp(&subMask, (char*)itr->m_subNetmask.c_str());
		        stringToIp(&gatewayAddr, (char*)itr->m_gateway.c_str());
		        status = AddIpRoute(targetAddr.addr.v4.ip, subMask.addr.v4.ip, gatewayAddr.addr.v4.ip, itr->m_type, route_dev);
		        if(STATUS_OK != status)
		        	ret = 0x0010;
	    	}
	    }
    }

	close(sfd);

	return status;

}// AcAddNi



//---------------------------------------------------------------------------
//
//  Function name:  AcDelNi
//
//  Description: Delete network interface
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//---------------------------------------------------------------------------
int AcDelNi(
	char 	*pIfName,	// IN
	DWORD	ipAddress,	// IN
	DWORD	subnetMask, // IN
	int		mtu)		// IN
{
	int status;

	struct ifreq ifr;

	memset (&ifr, 0, sizeof (ifr));
  	strncpy (ifr.ifr_name, pIfName, sizeof(ifr.ifr_name) - 1);
  	ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';

	if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {

//		AcPrintLog("Open socket fail, errno = %d", PERR, errno);
//		return E_IPC_SOCKET_ERROR;
		return STATUS_INTERNAL_ERROR;
	}

	if (ipAddress != 0) {

//		AcPrintLog("Can't delete interface, ip is: %x", PERR, ipAddress);
//		return E_SERVICE_IF_BAD_IP_ADDRESS;
		return STATUS_INTERNAL_ERROR;
	}

	if ((status = SetIpv4Address(&ifr, ipAddress))) {

//		AcPrintLog("SetIpv4Address fail, service name: %s, ip address = %x, status = %d",
//			 PERR,
//			 pIfName,
//			 ipAddress,
//			 status);

		return status;
	}

	close(sfd);

	return 0;

}// AcDelNi


/////////////////////////////////////////////////////////////////////////////
int IsNiExist(const char *pIfName,	// IN
			  bool * result)  		// OUT
{
	*result = false;
	struct if_nameindex *niArray = if_nameindex();
	if(NULL == niArray)
	{
		perror("if_nameindex FAILED");
		return IFCONFIG_NAMEINDEX_FAIL;
	}

	struct if_nameindex *iter = NULL;
	for(iter = niArray ; iter->if_index != 0 ; iter++)
	{
		int res = strcmp(iter->if_name, pIfName);
		if(0 == res)
		{
			*result = true;
			break;
		}
	}
	if_freenameindex(niArray);
	return IFCONFIG_OK;
}

/////////////////////////////////////////////////////////////////////////////
int GetNiIpV4Addr(const char *pIfName,   // IN
				DWORD *ppIpAddr)       // OUT
{
	struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  if (pIfName)
  {
    strncpy(ifr.ifr_name, pIfName, sizeof(ifr.ifr_name)-1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';
  }

	int sfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sfd < 0)
	{
		perror("GetNiIpV4Addr: socket failed");
		return IFCONFIG_SOCKET_FAIL;
	}

	int status = ioctl(sfd, SIOCGIFADDR, &ifr);
	DWORD errnoCode = errno;
	close(sfd);

	if(status < 0)
	{
		if(ENODEV == errnoCode)
		{
			return IFCONFIG_NI_NOTEXIST;
		}

		perror("GetNiIpV4Addr: ioctl failed");
		return IFCONFIG_IOCTL_FAIL;
	}

	struct sockaddr_in	*pSin = (struct sockaddr_in *) &(ifr.ifr_addr);
	*ppIpAddr = pSin->sin_addr.s_addr;
	*ppIpAddr = ntohl(*ppIpAddr);

	return IFCONFIG_OK;
}

/////////////////////////////////////////////////////////////////////////////
int GetNiIpV6Addr(const char *pIfName,   // IN
				  char 	*pIPv6Addr,      // OUT
				  char 	*sRetGetIdx,     // OUT
				  char 	*sRetGetAdd)     // OUT
{
	struct in6_ifreq ifr6;
	memset (&ifr6, 0, sizeof(in6_ifreq));
	/*struct in6_ifreq
	{
		struct in6_addr	ifr6_addr;
		__u32			ifr6_prefixlen;
		int				ifr6_ifindex;
	};*/

	int sfd = socket(AF_INET6, SOCK_DGRAM, 0);
	if(sfd < 0)
	{
		perror("GetNiV6IpAddr: socket failed");
		return IFCONFIG_SOCKET_FAIL;
	}


	int status = STATUS_OK;

	struct ifreq ifr;
	strncpy (ifr.ifr_name, pIfName, sizeof(ifr.ifr_name) - 1);
	ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';

	//Retrieve the interface index of the interface into  ifr_ifindex.
	DWORD errnoCode = 0;
	status = ioctl(sfd, SIOGIFINDEX, &ifr);

	sprintf(sRetGetIdx, " - SIOGIFINDEX status: %d, name: %s, idx: %d", status, pIfName, ifr.ifr_ifindex);

	if(STATUS_OK == status)
	{
		ifr6.ifr6_ifindex = ifr.ifr_ifindex;
		ifr6.ifr6_prefixlen = 0;

		status = ioctl(sfd, SIOCGIFADDR, &ifr6);
		errnoCode = errno;

		sprintf(sRetGetAdd, " - SIOCGIFADDR status: %d", status);
	}
	close(sfd);


	if(status < 0)
	{
		if(ENODEV == errnoCode)
		{
			return IFCONFIG_NI_NOTEXIST;
		}

		perror("GetNiIpAddr: ioctl failed");
		return IFCONFIG_IOCTL_FAIL;
	}


	struct sockaddr_in6 sa6;
	sa6.sin6_family = AF_INET6;
	sa6.sin6_port = 0;
	memcpy((char *) &sa6.sin6_addr, (char *) &ifr6.ifr6_addr, sizeof(struct in6_addr));

	inet_ntop(AF_INET6, sa6.sin6_addr.s6_addr, pIPv6Addr, IPV6_ADDRESS_LEN);


	return IFCONFIG_OK;
}






//---------------------------------------------------------------------------
//
//  Function name:  AcDelNi
//
//  Description: Delete network interface
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//  Error bit mask:
//	---------------
//	MSB																							LSB
//	15	14	13	12	11	10	9	8	7	6	5	4		3				2			1			0
//  NA	NA	NA	NA	NA	NA	NA	NA	NA	NA	NA	NA		default gw		NA			index(eth)	IPv6Address
//
//---------------------------------------------------------------------------
DWORD AcAddNi6(
	char 	*pIfName,	// IN
	char 	*pIPv6Addr, // IN
	char 	*pIPv6Prefix,// IN
	char	*pDefaultGw,
	char	*pDefaultGwPrefix)  // IN
{
	 int status = STATUS_OK;
	 int fd;
	 DWORD	ret = 0;

	struct ifreq ifr;
	struct sockaddr_in6 sa6;
	/*struct in6_addr
	  {
	    union
	    {
			uint8_t		u6_addr8[16];
			uint16_t 	u6_addr16[8];
			uint32_t 	u6_addr32[4];
	    } in6_u;
	#define s6_addr			in6_u.u6_addr8
	#define s6_addr16		in6_u.u6_addr16
	#define s6_addr32		in6_u.u6_addr32
	  };
	*/
	struct in6_ifreq ifr6;
	/*struct in6_ifreq
	{
		struct in6_addr	ifr6_addr;
		__u32			ifr6_prefixlen;
		int				ifr6_ifindex;
	};*/


	DWORD prefix_len, gwPrefix_len;

	memset (&ifr, 0, sizeof (struct ifreq));
	memset (&ifr6, 0, sizeof (struct in6_ifreq));

	if ((fd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
	{
		status = STATUS_FAIL;
		ret = 0xFFFF;
	}
	if(STATUS_OK == status)
	{
		if (pIPv6Prefix)
			prefix_len = atol(pIPv6Prefix);
		else
			prefix_len = 0;

		if (pDefaultGwPrefix)
			gwPrefix_len = atol(pDefaultGwPrefix);
		else
			gwPrefix_len = 0;

		//Create a network address structure (from IP string to sockaddr struct)
		sa6.sin6_family = AF_INET6;
	    sa6.sin6_port = 0;
	    inet_pton(AF_INET6, pIPv6Addr, sa6.sin6_addr.s6_addr);

		memcpy((char *) &ifr6.ifr6_addr, (char *) &sa6.sin6_addr, sizeof(struct in6_addr));

		strncpy (ifr.ifr_name, pIfName, sizeof(ifr.ifr_name) - 1);
		ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';
		//Retrieve the interface index of the interface into  ifr_ifindex.
		status = ioctl(fd, SIOGIFINDEX, &ifr);
		if(STATUS_OK == status)
		{
			ifr6.ifr6_ifindex = ifr.ifr_ifindex;
			ifr6.ifr6_prefixlen = prefix_len;
			//Set IPv6 address
			status = ioctl(fd, SIOCSIFADDR, &ifr6);
			if(STATUS_OK != status)
			{
				ret = 0x0001;
			}
			else
			{
				status = AddIPv6DefaultGw(fd, pDefaultGw, ifr.ifr_ifindex, gwPrefix_len);
				if(STATUS_OK != status)
				{
					//ret = 0x0008;
					ret = STATUS_FAILED_CFG_IPV6_DEF_GW;
				}
			} // end if ioctl(fd, SIOCSIFADDR, &ifr6) == ok
		} // end if ioctl(fd, SIOGIFINDEX, &ifr) == ok
		
		else // ioctl(fd, SIOGIFINDEX, &ifr) != ok
		{
			ret = 0x0002;
		}
	}

	close(fd);
	return ret;

}// AcAddNi6

//---------------------------------------------------------------------------
//
//  Function name:  AddIPv6DefaultGw
//
//  Description: Add default gw to dev
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//---------------------------------------------------------------------------
int AddIPv6DefaultGw(int fd, char *default_gw, int index, DWORD prefix_len)
{
	int status = STATUS_OK;
	int ret;
	int sock;
	struct in6_rtmsg rtm;
	/*struct in6_rtmsg
	  {
	    struct in6_addr rtmsg_dst;
	    struct in6_addr rtmsg_src;
	    struct in6_addr rtmsg_gateway;
	    u_int32_t rtmsg_type;
	    u_int16_t rtmsg_dst_len;
	    u_int16_t rtmsg_src_len;
	    u_int32_t rtmsg_metric;
	    unsigned long int rtmsg_info;
	    u_int32_t rtmsg_flags;
	    int rtmsg_ifindex;
	  };*/

	if(*default_gw == '\0')
		return 0;

	struct sockaddr_in6 gate6;

	memset (&rtm, 0, sizeof (struct in6_rtmsg));

	rtm.rtmsg_flags = RTF_UP | RTF_GATEWAY;
	rtm.rtmsg_metric = 1;

//	memcpy (&rtm.rtmsg_dst, &ifr6.ifr6_ifindex, sizeof (struct in6_addr));
	rtm.rtmsg_dst_len = prefix_len;

	//Create a network address structure (from IP string to sockaddr struct)
	gate6.sin6_family = AF_INET6;
	gate6.sin6_port = 0;
	inet_pton(AF_INET6, default_gw, gate6.sin6_addr.s6_addr);

	memcpy (&rtm.rtmsg_gateway, (char*)&gate6.sin6_addr, sizeof (struct in6_addr));

	rtm.rtmsg_ifindex = index;

	status = ioctl(fd, SIOCADDRT, &rtm);

	if (status < 0) {

		if (errno == 13)//permission denied - must be administartor
			return 0;

		//if entry exists in route (FILE EXISTS), then this failure is actually OK since it is configured.
		if(EEXIST == errno)
			return 0;

		return errno;
	}

	return 0;
}

