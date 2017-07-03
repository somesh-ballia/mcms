///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Date         Created By         Description
//
//  12/9/2012	    Judith		 wrapper class for SystemInterfaceList and SystemInterface
//  =========   ==============   ========================================================
////////////////////////////////////////////////////////////////////////////////////////////////


#include "SystemInterface.h"
#include "StatusesGeneral.h"
#include "IpService.h"
#include "ApiStatuses.h"
//#include "CSMngrProcess.h"
#include "TraceStream.h"
#include "InitCommonStrings.h"
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include "EnumsToStrings.h"
//#include <linux/wireless.h>



//////////////////////////////////////////////////////////////////////////////////////////
//			Class CSystemInterface
//////////////////////////////////////////////////////////////////////////////////////////////////////

CSystemInterface::CSystemInterface()
{
	m_InterfaceName = "";
	m_InterfaceIp = 0;
	m_IpType = eIpType_IpV4;
	m_IPv6_global = "";
	m_IPv6_site = "";
	m_IPv6_link = "";
	m_InterfaceType = "";
	m_SubNetMask = 0;
}

////////////////////////////////////////////////////////////////////////////
CSystemInterface::CSystemInterface( const CSystemInterface &other)
:CPObject(other)
{
    *this = other;
}

////////////////////////////////////////////////////////////////////////////
CSystemInterface& CSystemInterface::operator=(const CSystemInterface& other)
{
	if(this != &other)
	{
		m_InterfaceName = other.m_InterfaceName;
		m_InterfaceIp = other.m_InterfaceIp;
		m_IpType = other.m_IpType;
		m_IPv6_global = other.m_IPv6_global;
		m_IPv6_site = other.m_IPv6_site;
		m_IPv6_link = other.m_IPv6_link;
		m_InterfaceType = other.m_InterfaceType;
		m_SubNetMask = other.m_SubNetMask;
	}

    return *this;
}

////////////////////////////////////////////////////////////////////////////
bool CSystemInterface::operator==(const CSystemInterface &rHnd)const
{
	if(this == &rHnd)
		return true;

	if ( m_InterfaceName != rHnd.m_InterfaceName )
		return false;

	if ( m_InterfaceIp != rHnd.m_InterfaceIp )
		return false;

	if ( m_IpType != rHnd.m_IpType )
		return false;

	if ( m_IPv6_global != rHnd.m_IPv6_global )
	    return false;

	if ( m_IPv6_site != rHnd.m_IPv6_site )
	    return false;

	if ( m_IPv6_link != rHnd.m_IPv6_link )
	    return false;

	if ( m_InterfaceType != rHnd.m_InterfaceType )
		return false;

	if ( m_SubNetMask != rHnd.m_SubNetMask )
		return false;
	return true;
}

////////////////////////////////////////////////////////////////////////////
bool CSystemInterface::operator!=(const CSystemInterface& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
CSystemInterface::~CSystemInterface()
{
}

/////////////////////////////////////////////////////////////////////////////
void CSystemInterface::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement *pChildNode = NULL;
	if(NULL == pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("SYSTEM_INTERFACE");
		pChildNode = pFatherNode;
	}
	else
	{
		pChildNode = pFatherNode->AddChildNode("SYSTEM_INTERFACE");
	}

	pChildNode->AddChildNode("NETWORK_INTERFACE"	, m_InterfaceName);
	pChildNode->AddChildNode("NETWORK_INTERFACE_TYPE", m_InterfaceType);
	pChildNode->AddChildNode("IP" , m_InterfaceIp	,IP_ADDRESS);
	pChildNode->AddChildNode("SUBNET_MASK_ADDRESS", m_SubNetMask, IP_ADDRESS);
	pChildNode->AddChildNode("IP_TYPE", m_IpType, IP_TYPE_ENUM);


	CXMLDOMElement *pChildNodeIpV6 = NULL;
	pChildNodeIpV6 = pChildNode->AddChildNode("IPV6_ADDRESS");
	if(pChildNodeIpV6)
	{
	   pChildNodeIpV6->AddChildNode("GLOBAL", m_IPv6_global);
	   pChildNodeIpV6->AddChildNode("SITE", m_IPv6_site);
	   pChildNodeIpV6->AddChildNode("LINK", m_IPv6_link);
	}
}

/////////////////////////////////////////////////////////////////////////////
int	 CSystemInterface::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	/*	CXMLDOMElement *pSystemInterfaceNode = NULL;
		GET_CHILD_NODE(pActionNode, "SYSTEM_INTERFACE", pSystemInterfaceNode);
		if(NULL == pSystemInterfaceNode)
		{
			return nStatus;
		}

		GET_VALIDATE_CHILD(pSystemInterfaceNode, "INETERFACE", &m_Interface	, NET_SERVICE_PROVIDER_NAME_LENGTH);
		GET_VALIDATE_CHILD(pSystemInterfaceNode, "IP"			, &m_Id			, IP_ADDRESS);*/

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CSystemInterface::SetSystemInterfaceName(std::string interface)
{
	m_InterfaceName = interface;
}

/////////////////////////////////////////////////////////////////////////////
void CSystemInterface::SetSystemInterfaceIp(DWORD interface_ip)
{
	m_InterfaceIp = interface_ip;
}

/////////////////////////////////////////////////////////////////////////////
void CSystemInterface::SetSystemIpType(DWORD ip_type)
{
	m_IpType = ip_type;
}
void CSystemInterface::SetSystemInterfaceIpv6_global(std::string strIpv6_global)
{
	m_IPv6_global = strIpv6_global;
}
void CSystemInterface::SetSystemInterfaceIpv6_site(std::string strIpv6_site)
{
	m_IPv6_site = strIpv6_site;
}
void CSystemInterface::SetSystemInterfaceIpv6_link(std::string strIpv6_link)
{
	m_IPv6_link = strIpv6_link;
}
void CSystemInterface::SetSystemInterfaceType(std::string interfaceType)
{
	m_InterfaceType = interfaceType;
}
void CSystemInterface::SetSystemSubNetMask(DWORD subnet_mask)
{
	m_SubNetMask = subnet_mask;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class CSystemInterfaceList                                                       //
////////////////////////////////////////////////////////////////////////////////////////////////


CSystemInterfaceList::CSystemInterfaceList()
{
	m_bHasGottenInterfaces = false;
}

/////////////////////////////////////////////////////////////////////////////
CSystemInterfaceList::CSystemInterfaceList( const CSystemInterfaceList &other )
:CSerializeObject(other)
{
	CleanUpList();
	std::vector<CSystemInterface*>::const_iterator itOther = other.m_vSystemInterfaces.begin();
	while( other.m_vSystemInterfaces.end() != itOther)
	{
		CSystemInterface *pInterface = new CSystemInterface(*(CSystemInterface*)*itOther);
		m_vSystemInterfaces.push_back(pInterface);
		itOther++;
	}
	m_bHasGottenInterfaces = other.m_bHasGottenInterfaces;

}

/////////////////////////////////////////////////////////////////////////////
CSystemInterfaceList&  CSystemInterfaceList::operator=( const CSystemInterfaceList& other )
{
	if(this == &other)
	{
		return *this;
	}

	CleanUpList();
	std::vector<CSystemInterface*>::const_iterator itOther = other.m_vSystemInterfaces.begin();
	while( other.m_vSystemInterfaces.end() != itOther)
	{
		CSystemInterface *pInterface = new CSystemInterface(*(CSystemInterface*)*itOther);
		m_vSystemInterfaces.push_back(pInterface);
		itOther++;
	}
	m_bHasGottenInterfaces = other.m_bHasGottenInterfaces;

    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CSystemInterfaceList::~CSystemInterfaceList()
{
    CleanUpList();
}

////////////////////////////////////////////////////////////////////////////
void CSystemInterfaceList::CleanUpList()
{
	std::vector<CSystemInterface*>::iterator it = m_vSystemInterfaces.begin();
	while ( m_vSystemInterfaces.end() != it )
	{
		CSystemInterface *pInterface = (CSystemInterface*)*it;
		PDELETE(pInterface);
		it++;
	}
	m_vSystemInterfaces.clear();
	m_bHasGottenInterfaces = false;
}

/////////////////////////////////////////////////////////////////////////////
void    CSystemInterfaceList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pSysInterfaceListNode;

	if(!pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("SYSTEM_INTERFACE_LIST");
		pSysInterfaceListNode = pFatherNode;
	}
	else
	{
		pSysInterfaceListNode = pFatherNode->AddChildNode("SYSTEM_INTERFACE_LIST");
	}

	WORD bChanged = InsertUpdateCntChanged(pSysInterfaceListNode, UPDATE_CNT_BEGIN_END);
	if(TRUE == bChanged)
	{
		std::vector<CSystemInterface*>::const_iterator it = m_vSystemInterfaces.begin();
		while ( m_vSystemInterfaces.end() != it )
		{
			CSystemInterface *pInterface = (CSystemInterface*)*it;
			pInterface->SerializeXml(pSysInterfaceListNode);
			it++;
		}

	}

}

//////////////////////////////////////////////////////////////////////////
// schema file name:  obj_system_interface_list.xsd
int CSystemInterfaceList::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;
/*	CXMLDOMElement *pSysInterfaceNode;
	m_bChanged=TRUE;

	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"CHANGED",&m_bChanged,_BOOL);

	m_number_of_interfaces = 0;

	GET_FIRST_CHILD_NODE(pActionNode,"SYSTEM_INTERFACE",pSysInterfaceNode);

	while (pSysInterfaceNode && m_number_of_interfaces < 10)
	{
		CSystemInterface systemInterface;
		nStatus = systemInterface.DeSerializeXml(pSysInterfaceNode, pszError, NULL);

		if(nStatus != STATUS_OK)
		{
			return nStatus;
		}

		Add(systemInterface, false);

		GET_NEXT_CHILD_NODE(pActionNode,"SYSTEM_INTERFACE",pH323SrvNode);
	}*/

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void  CSystemInterfaceList::Add(DWORD interface_ip, DWORD subnet_mask, std::string interface,
								std::string iPv6_gloabl_str, std::string iPv6_site_str, std::string iPv6_link_str, std::string interfaceType)
{
	TRACESTR(eLevelInfoNormal) << "CSystemInterfaceList::Add";
    
	CSystemInterface *pInterface = new CSystemInterface;

	pInterface->SetSystemInterfaceIp(interface_ip);
	pInterface->SetSystemInterfaceName(interface);
	pInterface->SetSystemIpType(eIpType_IpV4);	//at teh moment, only ipv4 is supported in soft mcu
	pInterface->SetSystemInterfaceIpv6_global(iPv6_gloabl_str);
	pInterface->SetSystemInterfaceIpv6_site(iPv6_site_str);
	pInterface->SetSystemInterfaceIpv6_link(iPv6_link_str);
	pInterface->SetSystemInterfaceType(interfaceType);
	pInterface->SetSystemSubNetMask(subnet_mask);


	m_vSystemInterfaces.push_back(pInterface);

}

//////////////////////////////////////////////////////////////////////////
CSystemInterface* CSystemInterfaceList::GetSystemInterfaceByName(std::string interface)
{
	if (interface.empty())
	{
		TRACESTR(eLevelError) << "CSystemInterfaceList::GetSystemInterfaceByName : interface is empty";
		return NULL;
	}
	CSystemInterface* pInterface = NULL;
	std::vector<CSystemInterface*>::iterator it = m_vSystemInterfaces.begin();
	while (it != m_vSystemInterfaces.end())
	{
		pInterface = (CSystemInterface*)*it;
		if(interface == pInterface->GetSystemInterfaceName())
		{
			return pInterface;
		}
		it++;
	}
	return NULL;

}
bool CSystemInterfaceList::GetInterfacesFromSystem()
{
	// get all alive NIC base information : interface name and IPv4 info
	struct ifaddrs * ifAddrStruct=NULL;
	struct ifaddrs * ifa=NULL;

	if ( getifaddrs(&ifAddrStruct) != 0 )
	{
		TRACESTR(eLevelError)<< "CSystemInterfaceList::GetInterfacesFromSystem : getifaddrs return error("<< errno
				<< "-"<< strerror(errno)<< ")";
		return false;
	}

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
	{
		if( 0 == strcmp(ifa->ifa_name, "lo"))
		{
			continue;
		}
		if ( (ifa->ifa_flags & IFF_UP) && (ifa->ifa_flags & IFF_RUNNING) )
		{
			CSystemInterface* pInterface = GetSystemInterfaceByName(ifa->ifa_name);
			if( NULL == pInterface )
			{
				pInterface = new CSystemInterface();
				pInterface->SetSystemInterfaceName(ifa->ifa_name);
				m_vSystemInterfaces.push_back(pInterface);
			}


			if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
				// is a valid IP4 Address
				DWORD ipTmp = htonl(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr);
				pInterface->SetSystemInterfaceIp(ipTmp);
				ipTmp = htonl(((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr);
				pInterface->SetSystemSubNetMask(ipTmp);
			}
		}
	}

	if (ifAddrStruct!=NULL)
		freeifaddrs(ifAddrStruct);

	//get interface type and IPv6 info
	std::vector<CSystemInterface*>::iterator it = m_vSystemInterfaces.begin();
	while( it != m_vSystemInterfaces.end() )
	{
		CSystemInterface *pInterface = (CSystemInterface*)*it;
		GetInterfaceTypeFromSystem(pInterface);
		GetInterfaceIpv6FromSystem(pInterface);
		it++;
	}
	m_bHasGottenInterfaces = true;
	return true;
}
//////////////////////////////////////////////////////////////////////////
#ifndef SIOCGIWNAME
#define SIOCGIWNAME	0x8B01
union	iwreq_data
{
	/* Config - generic */
	char		name[IFNAMSIZ];
};
struct	iwreq
{
	union
	{
		char	ifrn_name[IFNAMSIZ];	/* if name, e.g. "eth0" */
	} ifr_ifrn;

	/* Data part (defined just above) */
	union	iwreq_data	u;
};
#endif
bool CSystemInterfaceList::GetInterfaceTypeFromSystem(CSystemInterface *pInterface)
{
	if(( NULL == pInterface) || (pInterface->GetSystemInterfaceName().empty()))
	{
		TRACESTR(eLevelError)<< "CSystemInterfaceList::GetInterfaceTypeFromSystem : pInterface is NULL or interface name is empty";
		return false;
	}
	struct ifreq ifr;
	struct iwreq iwq;
	const char *ifname = pInterface->GetSystemInterfaceName().c_str();
	int skfd;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if( 0 >= skfd )
	{
		TRACESTR(eLevelError)<< "CSystemInterfaceList::GetInterfaceTypeFromSystem : socket error ( "<< errno<< "-"<< strerror(errno)<<" )";
		return false;
	}

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0)
	{
		TRACESTR(eLevelError)<< "CSystemInterfaceList::GetInterfaceTypeFromSystem : ioctl error( "<< errno<< "-"<< strerror(errno)<< " )";
		close(skfd);
		return false;
	}

	switch (ifr.ifr_hwaddr.sa_family)  // TODO: detect whether is WIFI,
	{
	case ARPHRD_ETHER:
		pInterface->SetSystemInterfaceType("Ethernet");
		strncpy(iwq.ifr_name, ifname, IFNAMSIZ - 1);
        iwq.ifr_name[IFNAMSIZ - 1] = '\0';
		if ( -1 != ioctl(skfd, SIOCGIWNAME, &iwq))
			pInterface->SetSystemInterfaceType("WIFI");
		else
			TRACESTR(eLevelInfoNormal)<< "CSystemInterfaceList::GetInterfaceTypeFromSystem : judgment WIFI error( "<< errno<< "-"<< strerror(errno)<< " )";
		break;
	case ARPHRD_PPP:
		pInterface->SetSystemInterfaceType("PPP");
		break;
	case ARPHRD_SIT:
		pInterface->SetSystemInterfaceType("Sit");
		break;
	default:
		pInterface->SetSystemInterfaceType("Other");
		break;
	}
	TRACESTR(eLevelInfoNormal)<< "CSystemInterfaceList::GetInterfaceTypeFromSystem : the type of "<< ifname<<" is "<< pInterface->GetSystemInterfaceType();
	close(skfd);
	return true;
}
/////////////////////////////////////////////////////////////////////////
#define IPV6_ADDR_LOOPBACK      0x0010U
#define IPV6_ADDR_LINKLOCAL     0x0020U
#define IPV6_ADDR_SITELOCAL     0x0040U

#define IPV6_ADDR_COMPATv4      0x0080U

#define IPV6_ADDR_SCOPE_MASK    0x00f0U
void CSystemInterfaceList::GetInterfaceIpv6FromSystem(CSystemInterface *pInterface)
{
	if(( NULL == pInterface) || (pInterface->GetSystemInterfaceName().empty()))
	{
		TRACESTR(eLevelError)<< "CSystemInterfaceList::GetInterfaceIpv6FromSystem : pInterface is NULL or interface name is empty";
		return;
	}
	char addr6[40], devname[20];
	int plen, scope, dad_status, if_idx;
	char addr6p[8][5];

	FILE *fIpv6 = fopen("/proc/net/if_inet6", "r");
	if(NULL == fIpv6)
	{
		TRACESTR(eLevelError)<< "CSystemInterfaceList::GetInterfaceIpv6FromSystem : open /proc/net/if_inet6 error ( "<< errno<< " - "<< strerror(errno)<< " )";
		return;
	}

	while ( fscanf(fIpv6, "%4s%4s%4s%4s%4s%4s%4s%4s %08x %02x %02x %02x %20s\n",
				addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4],
				addr6p[5], addr6p[6], addr6p[7], &if_idx, &plen, &scope,
				&dad_status, devname) != EOF )
	{
		if( devname == pInterface->GetSystemInterfaceName() )
		{
			snprintf(addr6, sizeof(addr6), "%s:%s:%s:%s:%s:%s:%s:%s",
								addr6p[0], addr6p[1], addr6p[2], addr6p[3],
								addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
			switch (scope & IPV6_ADDR_SCOPE_MASK)
			{
			case 0:
				TRACESTR(eLevelInfoNormal)<< "CSystemInterfaceList::GetInterfaceIpv6FromSystem : "<< pInterface->GetSystemInterfaceName()<< " get IPv6 Global address "<< addr6;
				pInterface->SetSystemInterfaceIpv6_global(addr6);
				break;
			case IPV6_ADDR_LINKLOCAL:
				TRACESTR(eLevelInfoNormal)<< "CSystemInterfaceList::GetInterfaceIpv6FromSystem : "<< pInterface->GetSystemInterfaceName()<< " get IPv6 Link address "<< addr6;
				pInterface->SetSystemInterfaceIpv6_link(addr6);
				break;
			case IPV6_ADDR_SITELOCAL:
				TRACESTR(eLevelInfoNormal)<< "CSystemInterfaceList::GetInterfaceIpv6FromSystem : "<< pInterface->GetSystemInterfaceName()<< " get IPv6 Site address "<< addr6;
				pInterface->SetSystemInterfaceIpv6_site(addr6);
				break;
			case IPV6_ADDR_COMPATv4:
			case IPV6_ADDR_LOOPBACK:
			default:
				TRACESTR(eLevelInfoNormal)<< "CSystemInterfaceList::GetInterfaceIpv6FromSystem : "<< pInterface->GetSystemInterfaceName()<< " get unknown scope ( "
				<< scope<< " ) address "<< addr6;
			}
		}

	}
	fclose(fIpv6);

}
////////////////////////////////////////////////////////////////////////
void CSystemInterfaceList::Dump()
{
	std::vector<CSystemInterface*>::iterator it = m_vSystemInterfaces.begin();

	std::string strOutput = "CSystemInterfaceList::Dump\n";
	char caIpTmp[64];

	while( it != m_vSystemInterfaces.end() )
	{
		CSystemInterface *pInterface = (CSystemInterface*)*it;
		strOutput += "\n\tName : " + pInterface->GetSystemInterfaceName();
		strOutput += "\n\tType : " + pInterface->GetSystemInterfaceType();

		memset(caIpTmp, 0, sizeof(caIpTmp));
		SystemDWORDToIpString(pInterface->GetSystemInterfaceIp(), caIpTmp);
		strOutput += "\n\tIPv4 : " + std::string(caIpTmp);
		memset(caIpTmp, 0, sizeof(caIpTmp));
		SystemDWORDToIpString(pInterface->GetSystemSubNetMask(), caIpTmp);
		strOutput += "\n\tsubnet mask : " + std::string(caIpTmp);
		strOutput += "\n\tIPv6 Global: " + pInterface->GetSystemInterfaceIpv6_global();
		strOutput += "\n\tIPv6 Site: " + pInterface->GetSystemInterfaceIpv6_site();
		strOutput += "\n\tIPv6 Link: " + pInterface->GetSystemInterfaceIpv6_link();

		it++;
	}
	TRACESTR(eLevelInfoNormal)<< strOutput;
}
///////////////////////////////////////////////////////////////////////
STATUS CSystemInterfaceList::ValidateMFWSPANField(CIPSpan* pIpSpan, eIpType  ipType)
{
	if ( IsContainSysInterfaces() )
	{
		CleanUpList();
	}
	GetInterfacesFromSystem();
	CSystemInterface* pInterface = GetSystemInterfaceByName(pIpSpan->GetInterface());
	if( NULL == pInterface )
	{
		TRACESTR(eLevelError)<< "CSystemInterfaceList::ValidateMFWFields : the assigned interface must be included in system interface list";
		return STATUS_MUST_ASSIGN_INTERFACE_IN_MFW;
	}
	//3. check the assigned IPv4 address (if it has) must equal the related interface's IPv4 address
	if ( eIpType_Both == ipType || eIpType_IpV4 == ipType )
	{
		if (  0 == pIpSpan->GetIPv4Address() || pIpSpan->GetIPv4Address() != pInterface->GetSystemInterfaceIp() )
		{
			TRACESTR(eLevelError)<< "CSystemInterfaceList::ValidateMFWFields : assigned IP address !=  interface's IP address";
			char ipMngStr[IP_ADDRESS_LEN] = {0};
			SystemDWORDToIpString(pIpSpan->GetIPv4Address(), ipMngStr);
			m_strValidatedResult = ipMngStr;
			return STATUS_IP_ADDRESS_MISMATCHES_SYSTEM;
		}
	}
	//4. check the assigned IPv6 address (if it has) must equal the related interface's IPv6 address.
	bool hasIpv6InSpan = false;
	if ( eIpType_IpV6 == ipType || eIpType_Both == ipType)
	{

		mcTransportAddress tmpIPv6Addr;
		for (int idx = 0; idx < NUM_OF_IPV6_ADDRESSES; idx++)
		{
			char ipv6_string[IPV6_ADDRESS_LEN+1] = {0};
			pIpSpan->GetIPv6Address(idx,ipv6_string);
			if( 0 == strlen(ipv6_string)  || 0 == strncmp(ipv6_string, "::", IPV6_ADDRESS_LEN) )
			{
				continue;
			}
			hasIpv6InSpan = true;
			const APIU8* pIpv6 = pIpSpan->GetIPv6AddressByteArray(idx);
            PASSERT_AND_RETURN_VALUE(!pIpv6, STATUS_FAIL);
			enScopeId eScopeId = ::getScopeId( ipv6_string );
	//		TRACESTR(eLevelInfoNormal)<< "CMcuMngrProcess::ValidateMFWFields : In span, IPv6 is "<< ipv6_string<< " and scope is "<< eScopeId;
			m_strValidatedResult = ipv6_string;
			switch (eScopeId)
			{
			case eScopeIdGlobal:
				memset(&tmpIPv6Addr, 0, sizeof(mcTransportAddress));
				::stringToIpV6( &tmpIPv6Addr, (char*)pInterface->GetSystemInterfaceIpv6_global().c_str() );
				if ( 0 != memcmp(pIpv6, tmpIPv6Addr.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN))
				{
					TRACESTR(eLevelError)<< "CSystemInterfaceList::ValidateMFWFields : assigned IPv6 global " << ipv6_string
							<< "!= interface's IPv6 global "<< pInterface->GetSystemInterfaceIpv6_global();
					return STATUS_IPV6_GLOBAL_ADDRESS_MISMATCHES_SYSTEM;
				}
				break;
			case eScopeIdSite:
				memset(&tmpIPv6Addr, 0, sizeof(mcTransportAddress));
				::stringToIpV6( &tmpIPv6Addr, (char*)pInterface->GetSystemInterfaceIpv6_site().c_str() );
				if ( 0 != memcmp(pIpv6, tmpIPv6Addr.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN))
				{
					TRACESTR(eLevelError)<< "CSystemInterfaceList::ValidateMFWFields : assigned IPv6 site "<< ipv6_string
							<<" != interface's IPv6 site "<< pInterface->GetSystemInterfaceIpv6_site();
					return STATUS_IPV6_SITE_ADDRESS_MISMATCHES_SYSTEM;
				}
				break;
			case eScopeIdLink:
				memset(&tmpIPv6Addr, 0, sizeof(mcTransportAddress));
				::stringToIpV6( &tmpIPv6Addr, (char*)pInterface->GetSystemInterfaceIpv6_link().c_str() );
				if ( 0 != memcmp(pIpv6, tmpIPv6Addr.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN))
				{
					TRACESTR(eLevelError)<< "CSystemInterfaceList::ValidateMFWFields : assigned IPv6 link "<< ipv6_string
							<< " != interface's IPv6 link"<< pInterface->GetSystemInterfaceIpv6_link();
					return STATUS_IPV6_LINK_ADDRESS_MISMATCHES_SYSTEM;
				}
				break;
			default:
				TRACESTR(eLevelInfoNormal)<< "CSystemInterfaceList::ValidateMFWFields : IPv6 - "<< ipv6_string<< " and the scope is "<< eScopeId;
				break;
			}
		}
		if(!hasIpv6InSpan)
		{
			TRACESTR(eLevelError)<< "CSystemInterfaceList::ValidateMFWFields : there is none of IPv6 address when IP_TYPE is "
					<< ::IpTypeToString(ipType, true);
			return STATUS_IPV6_GLOBAL_ADDRESS_MISMATCHES_SYSTEM;
		}
	}
	if ( (0 == pIpSpan->GetIPv4Address()) && (!hasIpv6InSpan) )
	{
		TRACESTR(eLevelError)<< "CSystemInterfaceList::ValidateMFWFields : there is none of IPv6 and IPv6 address";
		return STATUS_NONE_IPV4_IPV6_ADDRESS;
	}

	return STATUS_OK;
}

CSystemInterface* CSystemInterfaceList::GetFirstAvailableInterface()
{
    TRACESTR(eLevelInfoNormal)<< "CSystemInterfaceList::GetFirstAvailableInterface in";

    //Get all interface information in the system.
    CleanUpList();
	GetInterfacesFromSystem();
    Dump();
    
    char caIpTmp[64];
    CSystemInterface* pFirstInterface = NULL;
    std::vector<CSystemInterface*>::iterator it = m_vSystemInterfaces.begin();

	while( it != m_vSystemInterfaces.end() )
	{
		CSystemInterface *pInterface = (CSystemInterface*)*it;
        DWORD d_IpV4 = pInterface->GetSystemInterfaceIp();
        
        if (0 != d_IpV4)
        {
            pFirstInterface = pInterface;
            break;
        }
		it++;
	}

    if (NULL == pFirstInterface)
    {
        std::vector<CSystemInterface*>::iterator it = m_vSystemInterfaces.begin();
    	while( it != m_vSystemInterfaces.end() )
    	{
    		CSystemInterface *pInterface = (CSystemInterface*)*it;
            string strIpV6Gobal = pInterface->GetSystemInterfaceIpv6_global();
            
            if ("" != strIpV6Gobal)
            {
                pFirstInterface = pInterface;
                break;
            }
    		it++;
    	}
        TRACESTR(eLevelInfoNormal) << "CSystemInterfaceList::GetFirstAvailableInterface no valid IpV4 Interface, use the first valid IpV6 interface."; 
    }
 
    return pFirstInterface;
}

//////////////////////////////////////////////////////////////////////
