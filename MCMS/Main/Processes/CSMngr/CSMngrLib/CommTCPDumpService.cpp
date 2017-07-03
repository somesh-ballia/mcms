// CommTCPDumpService.cpp

#include "CommTCPDumpService.h"
#include "StatusesGeneral.h"
#include "IpService.h"
#include "OpcodesMcmsInternal.h"
#include "DefinesIpServiceStrings.h"
#include "TraceStream.h"


// Static
const eProcessType CCommTCPDumpService::kProcessType = eProcessUtility;

CCommTCPDumpService::CCommTCPDumpService(void)
{}

// Virtual
const char* CCommTCPDumpService::NameOf(void) const
{
    return GetCompileType();
}

// Virtual
STATUS CCommTCPDumpService::SendIpServiceParamInd(CIPService* srv)
{
    IP_SERVICE_TCPDUMP_S prm;
    FillParams(prm, srv);

    CSegment* seg = new CSegment;
    seg->Put((BYTE*)&prm, sizeof prm);

    return SendToMcmsProcess(kProcessType,
                             CS_TCPDUMP_IP_SERVICE_PARAM_IND,
                             seg);
}

// Virtual
STATUS CCommTCPDumpService::SendIpServiceParamEndInd(void)
{
    return SendToMcmsProcess(kProcessType,
                             CS_TCPDUMP_IP_SERVICE_PARAM_END_IND,
                             NULL);
}

// Virtual
STATUS CCommTCPDumpService::SendDelIpService(CIPService* srv)
{
    return SendDelIpServiceToMcmsProcess(kProcessType,
                                         CS_TCPDUMP_DELETE_IP_SERVICE_IND,
                                         srv);
}

// Virtual
STATUS CCommTCPDumpService::SendServiceCfgList(CSegment *pSeg)
{
	/*TODO: maybe we need to send message to utility here.*/
	/*implement the virtual function avoid the memory leak:  David Liang -- 2012-05-08*/
	POBJDELETE(pSeg);
	return STATUS_OK;
}


// Private
void CCommTCPDumpService::FillParams(IP_SERVICE_TCPDUMP_S& prm, CIPService* srv)
{
    PASSERTMSG_AND_RETURN(NULL == srv, "Illegal parameter");
	TRACESTR(eLevelInfoNormal) << "\nCCommTCPDumpService::FillParams Entering...";

	//ipAddressV6If   IpV6Addr[NUM_OF_IPV6_ADDRESSES];
	string ipAddressV6[NUM_OF_IPV6_ADDRESSES] = "";
	eIPv6AddressScope ipAddrScope[NUM_OF_IPV6_ADDRESSES];
	
    memset(&prm, 0, sizeof prm);
    prm.id = srv->GetId();
    prm.type = srv->GetIpServiceType();
    prm.ipType = srv->GetIpType();
    strncpy(prm.name, srv->GetName(), ARRAYSIZE(prm.name) - 1);

	TRACESTR(eLevelInfoNormal) << "\nCCommTCPDumpService::FillParams: "
		                              << "\nprm.type: " << prm.type
		                              << "\nprm.ipType" << prm.ipType;
	
	if (prm.ipType != eIpType_IpV6)
	{
	    unsigned int i = 0;
	    for (CIPSpan* span = srv->GetFirstSpan();
	         span != NULL; span = srv->GetNextSpan(), i++)
	    {
	        PASSERT_AND_RETURN(i >= ARRAYSIZE(prm.span_ips));
	        prm.span_ips[i] = span->GetIPv4Address();
	    }
	}
	else 
	{
	
		//IPV6 Only
		unsigned int i = 0;
		CIPSpan* span = srv->GetFirstSpan();
		/*for (CIPSpan* span = srv->GetFirstSpan();
	         span != NULL; span = srv->GetNextSpan(), i++)*/
	    {
	        
			//memcpy(IpV6Addr[j].ip, &span->GetIPv6Address(j), IPV6_ADDRESS_BYTES_LEN);
	        //prm.span_ips[i] = span->GetIPv4Address();
	        //prm.ipType = eIpType_None;
			//prm.type = eIpServiceType_Signaling; //invalid value
	        string sCurIPv6Address = "";
			eIPv6AddressScope eCurIPV6AddrScope = eIPv6AddressScope_other;
			BOOL indexGlobal = NUM_OF_IPV6_ADDRESSES + 1;
			BOOL indexSite = NUM_OF_IPV6_ADDRESSES + 1;
			BOOL indexLink = NUM_OF_IPV6_ADDRESSES + 1;
	        for (int j=0; j<NUM_OF_IPV6_ADDRESSES; j++)
			{	
				ipAddressV6[j] = "";
				ipAddrScope[j] = eIPv6AddressScope_other;
				// IPv6 addressses
				sCurIPv6Address = span->GetIPv6Address(j);

				TRACESTR(eLevelInfoNormal) << "\nCCommTCPDumpService::FillParams: "
		                              << "\nIPV6 [: " << j
		                              << "] Addr :" << sCurIPv6Address.c_str();
				
				if (( "::" != sCurIPv6Address) && ( 0 != sCurIPv6Address.length()))
				{
					eCurIPV6AddrScope = GetIPv6AddressScope(sCurIPv6Address.c_str());
					ipAddressV6[j] = sCurIPv6Address;
					ipAddrScope[j] = eCurIPV6AddrScope;
					if(eCurIPV6AddrScope == eIPv6AddressScope_global)
					{
						indexGlobal = j;
					}
					else if (eCurIPV6AddrScope == eIPv6AddressScope_siteLocal)
					{
						indexSite = j;
					}
					else if (eCurIPV6AddrScope == eIPv6AddressScope_linkLocal)
					{
						indexLink = j;
					}
				}
			}

			if(indexGlobal != NUM_OF_IPV6_ADDRESSES + 1)
			{
				strncpy(((char*)prm.span_ipv6s), ipAddressV6[indexGlobal].c_str(), IPV6_ADDRESS_LEN);
				prm.span_ipv6s[IPV6_ADDRESS_LEN-1]='\0';
				prm.ipType = eIpType_IpV6;
			}
			else if (indexSite != NUM_OF_IPV6_ADDRESSES + 1)
			{
				strncpy(((char*)prm.span_ipv6s), ipAddressV6[indexSite].c_str(), IPV6_ADDRESS_LEN);
				prm.span_ipv6s[IPV6_ADDRESS_LEN-1]='\0';
				prm.ipType = eIpType_IpV6;
			}
			else if (indexLink != NUM_OF_IPV6_ADDRESSES + 1)
			{
				strncpy(((char*)prm.span_ipv6s), ipAddressV6[indexLink].c_str(), IPV6_ADDRESS_LEN);
				prm.span_ipv6s[IPV6_ADDRESS_LEN-1]='\0';
				prm.ipType = eIpType_IpV6;
			}
			

			TRACESTR(eLevelInfoNormal) << "\nCCommTCPDumpService::FillParams OK: "		                             
		                              << "\nspan_ipv6s: " << prm.span_ipv6s
		                              << "\nprm.ipType" << prm.ipType
		                              << "\nprm.type" << prm.type;
	    }
		
	}
	
}

STATUS CCommTCPDumpService::SendMngmIpService(DWORD ip, eIpType	 ipType, const char *str_Ipv6) const
{

    IP_SERVICE_TCPDUMP_S prm;
    memset(&prm, 0, sizeof prm);

    prm.id = 0;
    prm.type = eIpServiceType_Management;
    prm.ipType = ipType;
    strncpy(prm.name, MANAGEMENT_NETWORK_NAME, ARRAYSIZE(prm.name) - 1);
	if (ipType != eIpType_IpV6)
	{
		prm.span_ips[0] = ip;
	}
	else 
	{
		strncpy(prm.span_ipv6s, str_Ipv6, IPV6_ADDRESS_LEN);
		prm.span_ipv6s[IPV6_ADDRESS_LEN-1]='\0';
	}

    CSegment* seg = new CSegment;
    seg->Put((BYTE*)&prm, sizeof prm);

    return SendToMcmsProcess(kProcessType,
                             CS_TCPDUMP_IP_SERVICE_PARAM_IND,
                             seg);
}
