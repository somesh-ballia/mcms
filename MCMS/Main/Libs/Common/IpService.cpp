#include <string>
#include <iostream>
#include <stdlib.h>

#include "IpService.h"
#include "DataTypes.h"
#include "IPServiceDynamicList.h"
#include "DynIPSProperties.h"
#include "DefinesGeneral.h"
#include "Segment.h"
#include "StatusesGeneral.h"
#include "SystemFunctions.h"
#include "TraceStream.h"
#include "ConfigManagerApi.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "FaultsDefines.h"
#include "SystemInterface.h"

std::string GetHostNameFromService(const CIPService* pService)
{
	std::string hostname = "";

	if (    (pService!=NULL) &&
			(pService->GetSpanByIdx(0)!=NULL) &&
			(pService->GetSpanByIdx(0)->GetSpanHostName()!=NULL) &&
			(pService->GetSpanByIdx(0)->GetSpanHostName().GetString()!=NULL)
    )
	{
			hostname = pService->GetSpanByIdx(0)->GetSpanHostName().GetString();
	}
	return hostname;
}

CIceStandardParams* GetIceParamsFromService(CIPService* pService)
{
	CIceStandardParams* pIceStandardParams = NULL;
	if (    (pService!=NULL) &&
			(pService->GetpSipAdvanced()!=NULL) &&
			(pService->GetpSipAdvanced()->GetpIceStandardParams()!=NULL)
	    )
	{
		pIceStandardParams = pService->GetpSipAdvanced()->GetpIceStandardParams();
	}
	return pIceStandardParams;
}

// IsJitcAndNetSeparation: extern function that affects IpService configuration
bool IsJitcAndNetSeparation()
{
     bool isJitcAndNetSeparation = false;

    CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
    if (pSysConfig)
    {
	    BOOL isJITCMode = NO;
	    BOOL isManagmentSep = NO;

	    pSysConfig->GetBOOLDataByKey("ULTRA_SECURE_MODE", isJITCMode);
	    pSysConfig->GetBOOLDataByKey(CFG_KEY_SEPARATE_NETWORK, isManagmentSep);

	    if(isJITCMode && isManagmentSep)
	    {
	    	isJitcAndNetSeparation = true;
	    }
    }

    return isJitcAndNetSeparation;
}

////////////////////////////////////////////////////////////////////////////
std::string GetIpServiceTmpFileName()
{
	BOOL isV35JitcSupport = NO;
	BOOL isMultipleServicesSupport = NO;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, isV35JitcSupport);
	sysConfig->GetBOOLDataByKey(CFG_KEY_MULTIPLE_SERVICES, isMultipleServicesSupport);

	if (isMultipleServicesSupport)
	{
		FTRACESTR(eLevelInfoNormal) << "std::string GetIpServiceTmpFileName() - Multiple Services";
		return IP_MULTIPLE_SERVICES_TWO_SPANS_LIST_TMP_PATH;
	}
	else if (isV35JitcSupport)
	{
		FTRACESTR(eLevelInfoNormal) << "std::string GetIpServiceTmpFileName() - V35 JITC";
		return IP_SERVICES_JITC_V35_LIST_TMP_PATH;
	}

	FTRACESTR(eLevelInfoNormal) << "std::string GetIpServiceTmpFileName() - No special mode";
	return IP_SERVICE_LIST_TMP_PATH;
}

////////////////////////////////////////////////////////////////////////////
// ConfigDnsInOS: extern function for configuring DNS (for resolving)
////////////////////////////////////////////////////////////////////////////
STATUS ConfigDnsInOS(CIPService *pService, const string theCaller)
{
	STATUS retStatus = STATUS_OK;

	// ===== 1. get data
	CIpDns curDns( *(pService->GetpDns()) );

    int ip_server1 = curDns.GetIPv4Address(0),
        ip_server2 = curDns.GetIPv4Address(1),
        ip_server3 = curDns.GetIPv4Address(2);

    if(ip_server1 == -1) ip_server1 = 0;
    if(ip_server2 == -1) ip_server2 = 0;
    if(ip_server3 == -1) ip_server3 = 0;

	char domainName[NAME_LEN],
		 ip_server1Str[IPV6_ADDRESS_LEN],
         ip_server2Str[IPV6_ADDRESS_LEN],
         ip_server3Str[IPV6_ADDRESS_LEN];

    memset(ip_server1Str, 0, IPV6_ADDRESS_LEN);
    memset(ip_server2Str, 0, IPV6_ADDRESS_LEN);
    memset(ip_server3Str, 0, IPV6_ADDRESS_LEN);

	if(eIpType_IpV4 != pService->GetIpType())
	{

		FTRACESTR(eLevelInfoNormal) <<"\nConfigDnsInOS ip_server1 = "<<ip_server1;
		// try to get ipv6
		if(0 == ip_server1)
		{
			curDns.GetIPv6Address(0, ip_server1Str);
		}

		if(0 == ip_server2)
		{
			curDns.GetIPv6Address(1, ip_server2Str);
		}

		if(0 == ip_server3)
		{
			curDns.GetIPv6Address(2, ip_server3Str);
		}
	}

    memset(domainName, 0, NAME_LEN);
    memcpy(domainName, curDns.GetDomainName().GetString(), NAME_LEN-1);

    if ( 0 != ip_server1)
        SystemDWORDToIpString(ip_server1, ip_server1Str);

    FTRACESTR(eLevelInfoNormal) <<"\nConfigDnsInOS ip_server1Str = "<<ip_server1Str;

    if ( 0 != ip_server2)
        SystemDWORDToIpString(ip_server2, ip_server2Str);

    if ( 0 != ip_server3)
        SystemDWORDToIpString(ip_server3, ip_server3Str);


	// ===== 2. call DnsConfig method [checking IsTarget is done in ConfiguratorManager]
	CConfigManagerApi api;
	retStatus = api.ConfigureDnsServers(domainName, ip_server1Str, ip_server2Str, ip_server3Str);

	// ===== 3. print to log
	FTRACESTR(eLevelInfoNormal)
    	<<"\nConfigDnsInOS (caller: " << theCaller << ")"
    	<< "\ndomain name - " << domainName
    	<< "\nip_server1  - "  << ip_server1Str
    	<< "\nip_server2  - "  << ip_server2Str
        << "\nip_server3  - "  << ip_server3Str
    	<< "\nConfiguration status: " << CProcessBase::GetProcess()->GetStatusAsString(retStatus);

	return retStatus;
}


////////////////////////////////////////////////////////////////////////////
// ConfigMoreDnsInOS: extern function for configuring DNS (for resolving)
////////////////////////////////////////////////////////////////////////////
STATUS ConfigMoreDnsInOS(const char *pStrDomain, DWORD dnsAddress, const string theCaller)
{
	STATUS retStatus = STATUS_OK;
	char ip_server1Str[IPV6_ADDRESS_LEN];
	memset(ip_server1Str, 0, IPV6_ADDRESS_LEN);

    SystemDWORDToIpString(dnsAddress, ip_server1Str);

	// ===== 2. call DnsConfig method [checking IsTarget is done in ConfiguratorManager]
	CConfigManagerApi api;
	retStatus = api.ConfigureMoreDnsServers(pStrDomain, ip_server1Str);

	// ===== 3. print to log
	FTRACESTR(eLevelInfoNormal)
    	<<"\nConfigMoreDnsInOS (caller: " << theCaller << ")"
    	<< "\ndomain name - " << pStrDomain
    	<< "\nip_server1  - "  << ip_server1Str
    	<< "\nConfiguration status: " << CProcessBase::GetProcess()->GetStatusAsString(retStatus);

	return retStatus;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class CIPServiceList                                                       //
////////////////////////////////////////////////////////////////////////////////////////////////


CIPServiceList::CIPServiceList(CIPServiceFullList *xmlWrapper)
{
	m_XMLWrapper = xmlWrapper;

    m_numb_of_serv		= 0;
    m_ind_serv			= 0;
    m_numb_of_lan_serv	= 0;
    m_updateCounter 	= 0;
	m_bChanged			= FALSE;
	m_IsServiceAdded 	= false;

    memset( m_defaultServiceName,    '\0', NET_SERVICE_PROVIDER_NAME_LEN );
    memset( m_defaultSIPServiceName, '\0', NET_SERVICE_PROVIDER_NAME_LEN );

	for( int i=0 ; i < MAX_SERV_PROVIDERS_IN_LIST; i++ )
    {
		m_pH323Service[i] = NULL;
    }

	for ( int i=0; i<MAX_SPAN_NUMBER_IN_SERVICE; i++)
	{
		m_OccupiedSpans[i] = FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////////
CIPServiceList::CIPServiceList( const CIPServiceList &other )
:CSerializeObject(other)
{
    for( int i=0 ; i < MAX_SERV_PROVIDERS_IN_LIST; i++ )
    {
        if( other.m_pH323Service[i] == NULL)
            m_pH323Service[i] = NULL;
        else
            m_pH323Service[i]= new CIPService( *other.m_pH323Service[i] );
    }
    m_numb_of_serv = other.m_numb_of_serv;
    m_ind_serv	 = other.m_ind_serv;

    strncpy( m_defaultServiceName, other.m_defaultServiceName, NET_SERVICE_PROVIDER_NAME_LEN );
	strncpy( m_defaultSIPServiceName, other.m_defaultSIPServiceName, NET_SERVICE_PROVIDER_NAME_LEN );

    m_updateCounter 	= other.m_updateCounter;
    m_numb_of_lan_serv 	= other.m_numb_of_lan_serv;
	m_bChanged			= other.m_bChanged;
	m_IsServiceAdded 	= other.m_IsServiceAdded;

	for (int i=0; i<MAX_SPAN_NUMBER_IN_SERVICE; i++)
		m_OccupiedSpans[i] = other.m_OccupiedSpans[i];
}

/////////////////////////////////////////////////////////////////////////////
CIPServiceList&  CIPServiceList::operator=( const CIPServiceList& other )
{
	if(this == &other)
	{
		return *this;
	}

    for( int i=0 ; i < MAX_SERV_PROVIDERS_IN_LIST; i++ )
    {
    	POBJDELETE(m_pH323Service[i]);
        m_pH323Service[i] = (NULL == other.m_pH323Service[i]
        					?
        					NULL : new CIPService(*other.m_pH323Service[i]));
    }

    m_numb_of_serv 		= other.m_numb_of_serv;
    m_ind_serv	 		= other.m_ind_serv;
    m_updateCounter 	= other.m_updateCounter;
    m_numb_of_lan_serv 	= other.m_numb_of_lan_serv;
	m_bChanged			= other.m_bChanged;
	m_IsServiceAdded    = other.m_IsServiceAdded;
	m_XMLWrapper 		= other.m_XMLWrapper;

    strncpy(m_defaultServiceName, other.m_defaultServiceName, NET_SERVICE_PROVIDER_NAME_LEN);
    strncpy(m_defaultSIPServiceName, other.m_defaultSIPServiceName, NET_SERVICE_PROVIDER_NAME_LEN);

	for (int i=0; i<MAX_SPAN_NUMBER_IN_SERVICE; i++)
		m_OccupiedSpans[i] = other.m_OccupiedSpans[i];

    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CIPServiceList::~CIPServiceList()
{
    for( int i=0; i<MAX_SERV_PROVIDERS_IN_LIST; i++ )
        PDELETE( m_pH323Service[i] );
}

/////////////////////////////////////////////////////////////////////////////
/*void CIPServiceList::ChangeDefaultServiceFromSipToH323(const CSmallString& defaultServiceName)
{

    if( m_defaultServiceName == defaultServiceName )
        m_defaultServiceName[0]='\0';

    if( m_defaultServiceName[0]=='\0' &&  m_numb_of_serv )
    {
        const char* dfltSrvName = GetH323DefaultName();
        if (strcmp(dfltSrvName,"")!=0)
            strncpy( m_defaultServiceName, m_pH323Service[0]->GetName(),NET_SERVICE_PROVIDER_NAME_LEN);
    }
	UpdateCounters();
}*/

/////////////////////////////////////////////////////////////////////////////
void    CIPServiceList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pIpSrvListNode;

	if(!pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("IP_SERVICE_LIST");
		pIpSrvListNode = pFatherNode;
	}
	else
	{
		pIpSrvListNode = pFatherNode->AddChildNode("IP_SERVICE_LIST");
	}

	DWORD bChanged = InsertUpdateCntChanged(pIpSrvListNode, UPDATE_CNT_BEGIN_END);
	if(FALSE == bChanged)
	{
		return;
	}

	pIpSrvListNode->AddChildNode("DEFAULT_NAME", m_defaultServiceName);
	pIpSrvListNode->AddChildNode("DEFAULT_SIP_NAME", m_defaultSIPServiceName);

	for (int i=0; i<m_numb_of_serv; i++)
	{
		m_pH323Service[i]->SerializeXml(pIpSrvListNode, UPDATE_CNT_BEGIN_END);
	}


}
/////////////////////////////////////////////////////////////////////////////////
void CIPServiceList::SerializeXml(CXMLDOMElement* pFatherNode,DWORD objToken)const
{
	CXMLDOMElement* pH323SrvListNode = pFatherNode->AddChildNode("IP_SERVICE_LIST");

	WORD bChanged = InsertUpdateCntChanged(pH323SrvListNode, objToken);
	if(TRUE == bChanged)
	{
		pH323SrvListNode->AddChildNode("DEFAULT_NAME", m_defaultServiceName);
		pH323SrvListNode->AddChildNode("DEFAULT_SIP_NAME", m_defaultSIPServiceName);

		for (int i=0; i<m_numb_of_serv; i++)
		{
			m_pH323Service[i]->SerializeXml(pH323SrvListNode, UPDATE_CNT_BEGIN_END, true/*isToEMA*/);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// schema file name:  obj_ip_srv_list.xsd
int CIPServiceList::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	TRACESTR(eLevelInfoNormal) << "\nCIPServiceList::DeSerializeXml ";
	int nStatus = STATUS_OK;
	CXMLDOMElement *pH323SrvNode;
	m_bChanged=TRUE;

	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"CHANGED",&m_bChanged,_BOOL);

	m_numb_of_serv = 0;

	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"DEFAULT_NAME",m_defaultServiceName,NET_SERVICE_PROVIDER_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"DEFAULT_SIP_NAME",m_defaultSIPServiceName,NET_SERVICE_PROVIDER_NAME_LENGTH);

	GET_FIRST_CHILD_NODE(pActionNode,"IP_SERVICE",pH323SrvNode);

	while (pH323SrvNode && m_numb_of_serv < MAX_SERV_PROVIDERS_IN_LIST)
	{
		CIPService H323Srv;
		nStatus = H323Srv.DeSerializeXml(pH323SrvNode, pszError, NULL);

		if(nStatus != STATUS_OK)
		{
			TRACESTR(eLevelInfoNormal) << "\nCIPServiceList::DeSerializeXml, bad status: " << nStatus;
			return nStatus;
		}
		else
			TRACESTR(eLevelInfoNormal) << "\nCIPServiceList::DeSerializeXml, good status: " << nStatus;

		Add(H323Srv, false);

		GET_NEXT_CHILD_NODE(pActionNode,"IP_SERVICE",pH323SrvNode);
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
WORD CIPServiceList::GetServiceNumber() const
{
    return m_numb_of_serv;
}

////////////////////////////////////////////////////////////////////////////
void CIPServiceList::SetServiceNumber( const WORD num )
{
    m_numb_of_serv = num;
}

/////////////////////////////////////////////////////////////////////////////
WORD CIPServiceList::GetNumberOfLANServices() const
{
    return m_numb_of_lan_serv;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CIPServiceList::GetChanged() const
{
  return m_bChanged;
}
/////////////////////////////////////////////////////////////////////////////
DWORD  CIPServiceList::GetUpdateCounter() const
{
	return m_updateCounter;

}

/////////////////////////////////////////////////////////////////////////////
void CIPServiceList::SetUpdateCounter(DWORD cnt)
{
	m_updateCounter = cnt;
}

////////////////////////////////////////////////////////////////////////////
void CIPServiceList::SetNumberOfLANServices( const WORD num )
{
    m_numb_of_lan_serv = num;
}
/////////////////////////////////////////////////////////////////////////////
void CIPServiceList::UpdateCounters()
{
	IncreaseUpdateCounter();
	if(NULL != m_XMLWrapper)
	{
		m_XMLWrapper->IncreaseUpdateCounter();
	}
}

/////////////////////////////////////////////////////////////////////////////
int CIPServiceList::Add(CIPService *pNewService, bool isSaveToDisk)
{
	TRACESTR(eLevelInfoNormal) << "\nCIPServiceList::Add";

	if(m_numb_of_serv >= MAX_SERV_PROVIDERS_IN_LIST )
	{
        return STATUS_NUMBER_OF_H323_SERVICES_EXCEEDED;
	}

	if(FindService( *pNewService ) != NOT_FIND )
    {
        return STATUS_H323_SERVICE_NAME_EXISTS;
    }

    BOOL isNewServiceIdShouldBeGiven = FALSE;

    if (IsServiceIdInRange(pNewService->GetId())==FALSE || FindService(pNewService->GetId()) != NOT_FIND)
    	isNewServiceIdShouldBeGiven = TRUE;

    if (isNewServiceIdShouldBeGiven)
    {
    	int FreeServiceId = GetFirstFreeServiceId();
    	if (FreeServiceId == NOT_FIND)
    		return STATUS_NUMBER_OF_H323_SERVICES_EXCEEDED;
    	TRACESTR(eLevelInfoNormal) << "\nCIPServiceList::Add - new Id should be given to the service. The ID is: "<<FreeServiceId;
    	pNewService->SetId(FreeServiceId);
    }

    m_pH323Service[m_numb_of_serv] = pNewService;

    TRACESTR(eLevelInfoNormal) << "\nCIPServiceList::Add - m_pH323Service[m_numb_of_serv] is: "<<m_pH323Service[m_numb_of_serv]<<" and pNewService is: "<<pNewService;

    UpdateOccupiedSpan(pNewService);

    m_numb_of_serv++;

  	int H323serviceType = 0;
	int	SIPserviceType 	= 0;
    if('\0' == m_defaultServiceName[0])
    {
        H323serviceType = IsH323Type();
        if (H323serviceType != NOT_FIND && H323serviceType < MAX_SERV_PROVIDERS_IN_LIST)
        {
            strncpy(m_defaultServiceName, m_pH323Service[H323serviceType]->GetName(), sizeof(m_defaultServiceName)-1);
            m_defaultServiceName[sizeof(m_defaultServiceName)-1] = '\0';
        }
    }

    if('\0' == m_defaultSIPServiceName[0])
    {
        SIPserviceType = IsSIPType();
        if ((NOT_FIND != SIPserviceType) && (SIPserviceType < MAX_SERV_PROVIDERS_IN_LIST))
        {
            strncpy(m_defaultSIPServiceName, m_pH323Service[SIPserviceType]->GetName(), sizeof(m_defaultSIPServiceName)-1);
            m_defaultSIPServiceName[sizeof(m_defaultSIPServiceName)-1] = '\0';
        }
		//else
		//	m_defaultServiceName[0]='\0'; //Fix BRIDGE-2851 remove line not needed
    }

    if (SERVICE_H323_LAN == pNewService->GetServiceType())
    {
        m_numb_of_lan_serv++;
    }

	m_IsServiceAdded = true;
	UpdateCounters();
	if(isSaveToDisk)
		WriteXmlFile(GetIpServiceTmpFileName().c_str());

    return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
void CIPServiceList::UpdateOccupiedSpan(CIPService *pNewService)
{
	TRACESTR(eLevelInfoNormal) << "\nCIPServiceList::UpdateOccupiedSpan(CIPService *pNewService)";
    CIPSpan* pSpan = pNewService->GetFirstSpan();	//for signaling - ignore the first span

    int pos = 0;

    if (pSpan)
    {
    	pSpan = pNewService->GetNextSpan();
    	pos++;

		while (pSpan)
		{
			DWORD ipV4 = pSpan->GetIPv4Address();
			char ipv6_string[IPV6_ADDRESS_LEN+1] = {0};
			pSpan->GetIPv6Address(0,ipv6_string);
			if (ipV4 != 0 || (0 != strlen(ipv6_string)  && (0 != strncmp(ipv6_string, "::", IPV6_ADDRESS_LEN))&& 0 != strncmp(ipv6_string, "::/64", IPV6_ADDRESS_LEN)))
			{
				m_OccupiedSpans[pos] = TRUE;		//mark span as occupied
				TRACESTR(eLevelInfoNormal) << "\nCIPServiceList::UpdateOccupiedSpan(CIPService *pNewService) - pos = "<<pos<<" changed to TRUE";
			}



			pSpan = pNewService->GetNextSpan();
			pos++;
		}
    }
    UpdateEnableDisableFlag();
}
/////////////////////////////////////////////////////////////////////////////
void CIPServiceList::UpdateOccupiedSpan(CIPService *pNewService, CIPService *pOldService)
{
	TRACESTR(eLevelInfoNormal) << "\nCIPServiceList::UpdateOccupiedSpan(CIPService *pNewService, CIPService *pOldService)";
    CIPSpan* pSpan = pNewService->GetFirstSpan();	//for signaling - ignore the first span
    CIPSpan* pOldSpan  = pOldService->GetFirstSpan();

    int pos = 0;

    if (pSpan)
    {
     	pSpan = pNewService->GetNextSpan();
    	pOldSpan = pOldService->GetNextSpan();
    	pos++;

		while (pSpan)
		{
			BOOL bChanged = TRUE;
			DWORD ipV4 = pSpan->GetIPv4Address();

			if (pOldSpan)
			{
				DWORD OldIpV4 = pOldSpan->GetIPv4Address();
				if (ipV4 == OldIpV4)	//span has not been updated
					bChanged = FALSE;
			}

			if (bChanged)		//span has been updated
			{
				if (ipV4 !=0)
					m_OccupiedSpans[pos] = TRUE;		//mark span as occupied
				else
					m_OccupiedSpans[pos] = FALSE;
			}

			pSpan = pNewService->GetNextSpan();
			pOldSpan = pOldService->GetNextSpan();
			pos++;
		}
    }

    UpdateEnableDisableFlag();
}
/////////////////////////////////////////////////////////////////////////////
void CIPServiceList::UpdateDeletedOccupiedSpan(CIPService *pDeletedService)
{
	TRACESTR(eLevelInfoNormal) << "\nCIPServiceList::UpdateDeletedOccupiedSpan";
    CIPSpan* pSpan = pDeletedService->GetFirstSpan();	//for signaling - ignore the first span

    int pos = 0;

    if (pSpan)
    {
    	pSpan = pDeletedService->GetNextSpan();
    	pos++;

		while (pSpan)
		{
			DWORD ipV4 = pSpan->GetIPv4Address();
			if (ipV4 != 0)
				m_OccupiedSpans[pos] = FALSE;		//mark span as no longer occupied

			pSpan = pDeletedService->GetNextSpan();
			pos++;
		}
    }

    UpdateEnableDisableFlag();
}
/////////////////////////////////////////////////////////////////////////////
void CIPServiceList::UpdateEnableDisableFlag()
{
	std::string msg = "\nCIPServiceList::UpdateEnableDisableFlag()";

	for (int i=0; i<MAX_SPAN_NUMBER_IN_SERVICE; i++)
	{
		char pos[5];
		snprintf(pos, sizeof(pos), "%d", i);

		char occupied[5];
		snprintf(occupied, sizeof(occupied), "%d", m_OccupiedSpans[i]);

		msg += "\nm_OccupiedSpans[";
		msg += pos;
		msg += "] is ";
		msg += occupied;
	}

	TRACESTR(eLevelInfoNormal) << msg;

	for( int i=0 ; i < m_numb_of_serv; i++ )
    {
		CIPSpan* pSpan = m_pH323Service[i]->GetFirstSpan(); 	//for signaling - ignore the first span
		int pos = 0;

	    if (pSpan)
	    {
	    	pSpan = m_pH323Service[i]->GetNextSpan();
	    	pos++;

			while (pSpan)
			{
				//DWORD ipV4 = pSpan->GetIPv4Address();
				//if ip is not defined - the field is according to the occupied array
				//if (ipV4 == 0)
					pSpan->SetIsSpanEnable(m_OccupiedSpans[pos]);
				//else	// the field is occupied - must be enable
				//	pSpan->SetIsSpanEnable(TRUE);

				pSpan = m_pH323Service[i]->GetNextSpan();
				pos++;
			}
	    }
    }
}

/////////////////////////////////////////////////////////////////////////////
int CIPServiceList::GetFirstFreeServiceId()
{
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	int max_num_of_services = MAX_NUMBER_OF_SERVICES_IN_RMX_2000_AND_1500;
	if (eProductTypeRMX4000 == curProductType)
		max_num_of_services = MAX_NUMBER_OF_SERVICES_IN_RMX_4000;

	for (int i=0; i<max_num_of_services; i++)
	{
		if (FindService(i+1) == NOT_FIND)
			return (i+1);
	}

	TRACESTR(eLevelInfoNormal) << "\nCIPServiceList::GetFirstFreeServiceId - no free service id";

	return NOT_FIND;	//no free service id
}
/////////////////////////////////////////////////////////////////////////////
BOOL CIPServiceList::IsServiceIdInRange(int id)
{
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	int max_num_of_services = MAX_NUMBER_OF_SERVICES_IN_RMX_2000_AND_1500;
	if (eProductTypeRMX4000 == curProductType)
		max_num_of_services = MAX_NUMBER_OF_SERVICES_IN_RMX_4000;

	if (id > 0 && id < max_num_of_services)
		return TRUE;
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
int CIPServiceList::Add(const CIPService& other, bool isSaveToDisk)
{
	CIPService* pNewService = new CIPService(other);
	STATUS status = Add(pNewService, isSaveToDisk);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
int CIPServiceList::AddOnlyMem( const CIPService& other)
{
    if(m_numb_of_serv >= MAX_SERV_PROVIDERS_IN_LIST )
        return  STATUS_NUMBER_OF_H323_SERVICES_EXCEEDED;

    if(FindService( other ) != NOT_FIND )
        return STATUS_H323_SERVICE_NAME_EXISTS;

    if ( NULL != m_pH323Service[m_numb_of_serv] )
    {
    	POBJDELETE(m_pH323Service[m_numb_of_serv]);
  	   	m_numb_of_serv--;
    }

    CIPService* pNewService = new CIPService(other);
    m_pH323Service[m_numb_of_serv] = pNewService;
	m_numb_of_serv++;

    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CIPServiceList::Update( const CIPService& other )
{
	int	SIPserviceType;
    int ind = FindService( other );
    if(ind == NOT_FIND || ind >= MAX_SERV_PROVIDERS_IN_LIST )
    {
        return STATUS_H323_SERVICE_NAME_NOT_EXISTS;
    }

    // the id should be the same as before
    DWORD id = m_pH323Service[ind]->GetId();
    *m_pH323Service[ind] = other;
    m_pH323Service[ind]->SetId(id);

    CServiceConfig *pServiceConfig =  m_pH323Service[ind]->GetServiceConfig();
    CSysMap * m_Map=pServiceConfig->GetMap();
	CSysMap::iterator iTer = m_Map->begin();
	CSysMap::iterator iEnd = m_Map->end();
	while(iEnd != iTer)
	{
		const CCfgData *cfgData = iTer->second;
		TRACESTR(eLevelInfoNormal) << "CIPServiceList::Update - param name" << cfgData->GetData();

		iTer++;
	}

	// resetting default services
	if ( !strcmp ( m_defaultServiceName, other.GetName()) && other.GetIPProtocolType() == eIPProtocolType_SIP )
		m_defaultServiceName[0]='\0';
	if ( !strcmp ( m_defaultSIPServiceName, other.GetName()) && other.GetIPProtocolType() == eIPProtocolType_H323 )
		m_defaultSIPServiceName[0]='\0';

	// updating new default services


	if(m_defaultServiceName[0]=='\0' &&  m_numb_of_serv )
    {
        int H323serviceType = IsH323Type();
        if ((H323serviceType != NOT_FIND) && (H323serviceType < MAX_SERV_PROVIDERS_IN_LIST))
        {
            strncpy( m_defaultServiceName,m_pH323Service[H323serviceType]->GetName(), sizeof(m_defaultServiceName) - 1);
            m_defaultServiceName[sizeof(m_defaultServiceName) - 1] = '\0';
        }
		else
			m_defaultServiceName[0]='\0';
    }



    if(m_defaultSIPServiceName[0]=='\0' &&  m_numb_of_serv )
    {
        SIPserviceType = IsSIPType();
        if (SIPserviceType != NOT_FIND)
        {
            if(SIPserviceType < MAX_SERV_PROVIDERS_IN_LIST)
            {
                strncpy( m_defaultSIPServiceName,
			    		 m_pH323Service[SIPserviceType]->GetName(),
				    	 sizeof(m_defaultSIPServiceName) - 1);
            }
            else
                PASSERTMSG(1, "SIPserviceType exceed MAX_SERV_PROVIDERS_IN_LIST");

            m_defaultSIPServiceName[sizeof(m_defaultSIPServiceName) - 1] = '\0';
        }
		else
			m_defaultSIPServiceName[0]='\0';
    }



	UpdateCounters();

	WriteXmlFile(GetIpServiceTmpFileName().c_str());
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CIPServiceList::UpdateOnlyMem( const CIPService& other )
{
    int status = STATUS_OK;

    int ind = FindService( other );
    if(ind == NOT_FIND || ind >= MAX_SERV_PROVIDERS_IN_LIST )
        return STATUS_H323_SERVICE_NAME_NOT_EXISTS;

    CIPService* pNewService = new CIPService(other);

	delete m_pH323Service[ind];
    m_pH323Service[ind] = pNewService;

	UpdateCounters();
    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CIPServiceList::UpdateDynamic(DWORD serviceId, CServiceInfo	&param)
{
	CIPService *service = GetService(serviceId);
	if(NULL == service)
	{
		return STATUS_H323_SERVICE_NAME_NOT_EXISTS;
	}
	service->GetDynamicProperties()->SetInfo(param);
	m_XMLWrapper->IncreaseUpdateCounter();

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CIPServiceList::UpdateDynamic(DWORD serviceId, CCSIpInfo &param)
{
	CIPService *service = GetService(serviceId);
	if(NULL == service)
	{
		return STATUS_H323_SERVICE_NAME_NOT_EXISTS;
	}
	service->GetDynamicProperties()->SetInfo(param);
	m_XMLWrapper->IncreaseUpdateCounter();

	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CIPServiceList::UpdateDynamic(DWORD serviceId, CSipInfo &param)
{
	CIPService *service = GetService(serviceId);
	if(NULL == service)
	{
		return STATUS_H323_SERVICE_NAME_NOT_EXISTS;
	}
	service->GetDynamicProperties()->SetInfo(param);
	m_XMLWrapper->IncreaseUpdateCounter();

	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CIPServiceList::UpdateDynamic(DWORD serviceId, CH323Info &param)
{
	CIPService *service = GetService(serviceId);
	if(NULL == service)
	{
		return STATUS_H323_SERVICE_NAME_NOT_EXISTS;
	}
	service->GetDynamicProperties()->SetInfo(param);
	m_XMLWrapper->IncreaseUpdateCounter();

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CIPServiceList::UpdateDynamic(DWORD serviceId, CCardIpAddress	&param)
{
	CIPService *service = GetService(serviceId);
	if(NULL == service)
	{
		return STATUS_H323_SERVICE_NAME_NOT_EXISTS;
	}
	service->GetDynamicProperties()->SetInfo(param);
	m_XMLWrapper->IncreaseUpdateCounter();

	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CIPServiceList::UpdateDynamic(DWORD serviceId, CIceInfo	&param)
{
	CIPService *service = GetService(serviceId);
	if(NULL == service)
	{
		return STATUS_H323_SERVICE_NAME_NOT_EXISTS;
	}
	service->GetDynamicProperties()->SetInfo(param);
	m_XMLWrapper->IncreaseUpdateCounter();

	return STATUS_OK;

}
/////////////////////////////////////////////////////////////////////////////
int CIPServiceList::Cancel( const char* name )
{
    int H323serviceType;
	int SIPserviceType;
    int ind = FindService( name );
    if(ind == NOT_FIND || ind >= MAX_SERV_PROVIDERS_IN_LIST )
        return STATUS_H323_SERVICE_NAME_NOT_EXISTS;

    if(!strcmp( m_defaultServiceName, name ) )
        m_defaultServiceName[0]='\0';

    if(!strcmp( m_defaultSIPServiceName, name ) )
         m_defaultSIPServiceName[0]='\0';


    if (m_pH323Service[ind]->GetServiceType() == SERVICE_H323_LAN)
        m_numb_of_lan_serv--;

    PDELETE( m_pH323Service[ind] );

	int i;
	PASSERTMSG(m_numb_of_serv > MAX_SERV_PROVIDERS_IN_LIST, "m_numb_of_serv exceed the MAX_SERV_PROVIDERS_IN_LIST");

	if (m_numb_of_serv <= MAX_SERV_PROVIDERS_IN_LIST )
	{
		for( i=0; i<(int)m_numb_of_serv; i++ )
		{
			if(m_pH323Service[i] == NULL )
				break;
		}
		for( int j=i; j<(int)m_numb_of_serv-1; j++ )
		{
			m_pH323Service[j] = m_pH323Service[j+1] ;
		}
		m_pH323Service[m_numb_of_serv-1] = NULL;
		m_numb_of_serv--;
	}



	if(m_defaultServiceName[0]=='\0' &&  m_numb_of_serv )
    {
        H323serviceType = IsH323Type();
        if ((H323serviceType != NOT_FIND) && (H323serviceType < MAX_SERV_PROVIDERS_IN_LIST))
        {
            strncpy( m_defaultServiceName,
				     m_pH323Service[H323serviceType]->GetName(),
					 sizeof(m_defaultServiceName) - 1);
            m_defaultServiceName[sizeof(m_defaultServiceName) - 1] = '\0';
        }
    }


  if(m_defaultSIPServiceName[0]=='\0' &&  m_numb_of_serv )
    {
        SIPserviceType = IsSIPType();
        if ((SIPserviceType != NOT_FIND) && (SIPserviceType < MAX_SERV_PROVIDERS_IN_LIST))
        {
            strncpy( m_defaultSIPServiceName,
					 m_pH323Service[SIPserviceType]->GetName(),
					 sizeof(m_defaultSIPServiceName) - 1);
            m_defaultSIPServiceName[sizeof(m_defaultSIPServiceName) - 1] ='\0';
        }
    }


    UpdateCounters();

	WriteXmlFile(GetIpServiceTmpFileName().c_str());
    return  STATUS_OK;
}


DWORD CIPServiceList::GetServiceIdByName(const char* name)
{
	for(int i = 0; i < m_numb_of_serv; i++)
	{
		if ( m_pH323Service[i] != NULL )
		{
			const char* service_name = m_pH323Service[i]->GetName();
			TRACESTR(eLevelInfoNormal) << "CIPServiceList::GetServiceIdByName service name in list " << service_name;
			TRACESTR(eLevelInfoNormal) << "CIPServiceList::GetServiceIdByName service name to find " << name;
			WORD len= strlen (service_name);
			if (strncmp(name,service_name,len)==0)
			{
				TRACESTR(eLevelInfoNormal) << "CIPServiceList::GetServiceIdByName service name is equal ";
				return m_pH323Service[i]->GetId();
			}

		}
	}
	return NOT_FIND;
}

void CIPServiceList::HandleServicesCfgFiles()
{
	for(int i = 0; i < m_numb_of_serv; i++)
	{
		if ( m_pH323Service[i] != NULL )
		{
			CServiceConfig  *pServiceConfig = m_pH323Service[i]->GetServiceConfig();
			eCfgParamType fileType = eCfgParamUser;
			pServiceConfig->LoadFromFile(fileType);
			CSysConfig::SwitchCfgFiles(pServiceConfig->GetFileName(eCfgParamUser), pServiceConfig->GetTmpFileName(eCfgParamUser));

		}
	}


}


/////////////////////////////////////////////////////////////////////////////
int CIPServiceList::IsH323Type()
{
	for(int i = 0; i < m_numb_of_serv; i++)
	{
		if ( m_pH323Service[i] != NULL )
        {
			eIPProtocolType type = m_pH323Service[i]->GetIPProtocolType();
			if (type == eIPProtocolType_H323 ||
				type == eIPProtocolType_SIP_H323)
			{
				return i;
			}
		}
    }

    return NOT_FIND;
}
/////////////////////////////////////////////////////////////////////////////
int CIPServiceList::IsSIPType()
{
	for(int i = 0; i < m_numb_of_serv; i++)
	{
		if ( m_pH323Service[i] != NULL )
        {
			eIPProtocolType type = m_pH323Service[i]->GetIPProtocolType();
			if (type == eIPProtocolType_SIP ||
				type == eIPProtocolType_SIP_H323)
			{
				return i;
			}
		}
    }

    return NOT_FIND;
}

/////////////////////////////////////////////////////////////////////////////
int CIPServiceList::FindService( DWORD id )const
{
	for( int i=0; i<(int)m_numb_of_serv; i++ )
    {
        if ( m_pH323Service[i] != NULL )
            if ( m_pH323Service[i]->GetId() == id )
                return i;
    }
    return NOT_FIND;
}

/////////////////////////////////////////////////////////////////////////////
int CIPServiceList::FindService(const char* name)const
{
    for( int i=0; i<(int)m_numb_of_serv; i++ )
    {
        if ( m_pH323Service[i] != NULL )
            if ( !strcmp( m_pH323Service[i]->GetName(), name ) )
                return i;
    }
    return NOT_FIND;
}

/////////////////////////////////////////////////////////////////////////////
int CIPServiceList::FindService(const CIPService& other)const
{
    for( int i=0; i<(int)m_numb_of_serv; i++ )
    {
        if ( m_pH323Service[i] != NULL )
            if ( !strcmp( m_pH323Service[i]->GetName(), other.GetName() ) )
                return i;
    }
    return NOT_FIND;
}

/////////////////////////////////////////////////////////////////////////////
CIPService* CIPServiceList::GetService(const CH323Alias* prefix)
{
    for( int i=0; i<(int)m_numb_of_serv; i++ )
    {
        if ( m_pH323Service[i] != NULL )
            if ( !strcmp( m_pH323Service[i]->GetDialInPrefix()->GetAliasName(), prefix->GetAliasName() ) )
                return m_pH323Service[i];
    }
    return NULL;
}
/////////////////////////////////////////////////////////////////////////////
const char* CIPServiceList::FindServiceAndGetStringWithoutPrefix( const char* prefixPlusString, WORD prefixType)
{
    for( int i=0; i<(int)m_numb_of_serv; i++ )
    {
        if ( m_pH323Service[i] != NULL )
        {
            const  CH323Alias* pServicePrefix = m_pH323Service[i]->GetDialInPrefix();

            if (pServicePrefix)
            {
                const char* servicePrefix = pServicePrefix->GetAliasName();

                if (servicePrefix)
                {
                    WORD len = strlen(servicePrefix);

                    if (len && (len <= strlen(prefixPlusString)) ){
                        if ( !strncmp(prefixPlusString, servicePrefix, len) )
                            return &(prefixPlusString[len]);
                    }
                }
            }
        }
    }
    return NULL;

}

/////////////////////////////////////////////////////////////////////////////
CIPService*  CIPServiceList::GetService( const char* name )
{
    for( int i=0; i<(int)m_numb_of_serv; i++ )
    {
        if ( m_pH323Service[i] != NULL )
            if (!strcmp( m_pH323Service[i]->GetName(), name ))
                return m_pH323Service[i];
    }
    return NULL;		  // STATUS_H323_SERVICE_NAME_NOT_EXISTS
}

////////////////////////////////////////////////////////////////////////////
CIPService*  CIPServiceList::GetService( const DWORD id )
{
	CIPService *service=NULL;
	int index = FindService(id);
	if(index == NOT_FIND || index >= MAX_SERV_PROVIDERS_IN_LIST )
		service= NULL;
	else
		service = m_pH323Service[index];

	return service;
}

/////////////////////////////////////////////////////////////////////////////
CIPService*  CIPServiceList::GetService( const WORD line , const BYTE serviceTypeName)
{
    for (int i=0;i<(int)m_numb_of_serv; i++)
    {
        if ( m_pH323Service[i] != NULL )
		{
                return m_pH323Service[i];
		}
    }
    return NULL;		 // STATUS_H323_SERVICE_NOT_EXISTS (or type is different)
}

/////////////////////////////////////////////////////////////////////////////
CIPService*  CIPServiceList::GetFirstService()
{
    m_ind_serv=1;
    return m_pH323Service[0];
}

/////////////////////////////////////////////////////////////////////////////
CIPService*  CIPServiceList::GetNextService()
{
    if(m_ind_serv >= m_numb_of_serv ) return NULL;

    PASSERTSTREAM_AND_RETURN_VALUE(m_ind_serv >= MAX_SERV_PROVIDERS_IN_LIST,
        "m_ind_serv has invalid value " << m_ind_serv, NULL);

    return m_pH323Service[m_ind_serv++];
}

/////////////////////////////////////////////////////////////////////////////
const char* CIPServiceList::GetH323DefaultName() const
{
    return m_defaultServiceName;
}

/////////////////////////////////////////////////////////////////////////////
void CIPServiceList::SetH323DefaultName( const char* name )
{
    int h323Service = IsH323Type();
    if (h323Service!=NOT_FIND)
    {
        strncpy( m_defaultServiceName, name, sizeof(m_defaultServiceName) - 1);
        m_defaultServiceName[sizeof(m_defaultServiceName) - 1]= '\0';
    }

	UpdateCounters();

	WriteXmlFile(GetIpServiceTmpFileName().c_str());
}

/////////////////////////////////////////////////////////////////////////////
const char* CIPServiceList::GetSIPDefaultName() const
{
    return m_defaultSIPServiceName;

}

/////////////////////////////////////////////////////////////////////////////
void CIPServiceList::SetSIPDefaultName( const char* name )
{
    int sipService = IsSIPType();
    if (sipService!=NOT_FIND)
    {
        strncpy( m_defaultSIPServiceName, name, sizeof(m_defaultSIPServiceName) - 1);
        m_defaultSIPServiceName[sizeof(m_defaultSIPServiceName) - 1]='\0';
    }

	UpdateCounters();
}

/////////////////////////////////////////////////////////////////////////////
void CIPServiceList::SetMaxNumOfParties(DWORD num)
{
	CIPService *service = GetFirstService();

	int original_num = num;
	while(NULL != service)
    {
    	num = original_num;

    	bool result = service->IsUserDefinePorts();
    	if(false == result)		//not fix port - take the number from the license
    	{
    		//Example: For RMX2000, when max parties in licensing is 720, we create each Signaling with 400 ports(multiple number of active cards for this CS)
    		//The CS gets from us 800 TCP ports for each card.
    		if (num > (service->GetMaxNumOfCalls()))
    		{
				num = service->GetMaxNumOfCalls();
				TRACESTR(eLevelInfoNormal) << "\nCIPServiceList::SetMaxNumOfParties - For service "<<service->GetName()<<" the num of calls was: "<<original_num<<" and now it is: "<<num;
    		}

    		service->DefineTcpPortRange(num);
    	}
    	service = GetNextService();
    }
}

/////////////////////////////////////////////////////////////////////////////
void CIPServiceList::SetUdpPortRange(WORD firstPort, WORD numOfPorts, DWORD serviceId)
{
    CIPService *pService = GetService(serviceId);
    if(NULL != pService)
    {
    		pService->SetUdpPortRange(firstPort, numOfPorts);
    }
}

/////////////////////////////////////////////////////////////////////////////
void CIPServiceList::SetUdpPortRange(WORD firstPort, WORD numOfPorts)
{
	CIPService *service = GetFirstService();
    while(NULL != service)
    {
    		service->SetUdpPortRange(firstPort, numOfPorts);
    	service = GetNextService();
    }
}
/////////////////////////////////////////////////////////////////////////////
void CIPServiceList::CalcMaxNumOfPorts()
{
	CIPService *pService = GetFirstService();

	while (pService)
	{
		pService->CalculateMaxNumOfCalls();
		pService = GetNextService();
	}
}
/////////////////////////////////////////////////////////////////////////////
BOOL CIPServiceList::IsOccupiedSpan(int pos)
{
	return m_OccupiedSpans[pos];
}

/////////////////////////////////////////////////////////////////////////////
BOOL CIPServiceList::IsV35InUsed()
{
	CIPService* pIpService = GetFirstService();

	while (pIpService)
	{
		if (pIpService->GetIsV35GwEnabled())
			return TRUE;
		pIpService = GetNextService();
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CIPServiceList::UpdateServiceListDefaults()
{
	CIPService* pIpService = GetFirstService();

	while (pIpService)
	{
        TRACESTR(eLevelInfoNormal) << "CIPServiceList::UpdateServiceListDefaults service name:" << pIpService->GetName();
		pIpService->UpdateServiceDefaults();
		pIpService = GetNextService();
	}
}

/////////////////////////////////////////////////////////////////////////////
BOOL CIPServiceList::CompareServiceListValues()
{
	BOOL bRaiseAA = FALSE;

	CIPService* pIpService = GetFirstService();

	while (pIpService)
	{
        TRACESTR(eLevelInfoNormal) << "CIPServiceList::CompareServiceListValues serviceName:" << pIpService->GetName();
		if (pIpService->CompareServiceValues())
			bRaiseAA = TRUE;
		pIpService = GetNextService();
	}

	return bRaiseAA;
}
//
//
/////////////////////////////////////////////////////////////////////////////
CBaseSipServer::CBaseSipServer()
:m_name()
{
    m_status = eServerStatusOff;
    m_port   = 5060;
}

/////////////////////////////////////////////////////////////////////////////
CBaseSipServer::CBaseSipServer(const CBaseSipServer& other)
:CPObject(other), m_name(other.m_name)
{
    m_status = other.m_status;
    m_port   = other.m_port;
}

/////////////////////////////////////////////////////////////////////////////
CBaseSipServer& CBaseSipServer::operator=(const CBaseSipServer& other)
{
    m_status = other.m_status;
	m_name   = other.m_name;
    m_port   = other.m_port;
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
bool CBaseSipServer::operator==(const CBaseSipServer &rHnd)const
{
	if(this == &rHnd)
		return true;

	if ( m_status != rHnd.m_status )
		return false;

	if ( m_name != rHnd.m_name )
		return false;

	if ( m_port != rHnd.m_port )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CBaseSipServer::operator!=(const CBaseSipServer& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
CBaseSipServer::~CBaseSipServer()
{

}

/////////////////////////////////////////////////////////////////////////////
void CBaseSipServer::SerializeXml(CXMLDOMElement* pFatherNode)
{
	pFatherNode->AddChildNode("SERVER_STATUS", (WORD)m_status, SERVER_STATUS_ENUM);
	pFatherNode->AddChildNode("NAME", m_name);
	pFatherNode->AddChildNode("PORT", m_port);
}

/////////////////////////////////////////////////////////////////////////////
int CBaseSipServer::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"SERVER_STATUS",(WORD*)(&m_status),SERVER_STATUS_ENUM);

    GET_VALIDATE_ASCII_CHILD(pActionNode,"NAME",m_name,ONE_LINE_BUFFER_LENGTH);

	GET_VALIDATE_CHILD(pActionNode,"PORT",&m_port,_0_TO_DWORD);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
eServerStatus CBaseSipServer::GetStatus() const
{
    return  m_status;
}

/////////////////////////////////////////////////////////////////////////////
void CBaseSipServer::SetStatus(const eServerStatus status)
{
    m_status = status;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CBaseSipServer::GetPort() const
{
    return  m_port;
}

/////////////////////////////////////////////////////////////////////////////
void CBaseSipServer::SetPort(const DWORD port)
{
    m_port = port;
}


/////////////////////////////////////////////////////////////////////////////
DWORD CBaseSipServer::GetIpAddress() const
{
	return m_name.IpToDWORD();
}

/////////////////////////////////////////////////////////////////////////////
const CSmallString& CBaseSipServer::GetName() const
{
    return m_name;
}

/////////////////////////////////////////////////////////////////////////////
void CBaseSipServer::SetName(const CSmallString& name)
{
    m_name = name;
}

/////////////////////////////////////////////////////////////////////////////
ESTATUS CBaseSipServer::TestValidity(BYTE isDns)
{
    DWORD status = STATUS_OK;

	if (m_status != eServerStatusOff &&
		m_status != eServerStatusSpecify &&
		m_status != eServerStatusAuto)
	{
		status = STATUS_ILLEGAL;
	}

	if (status == STATUS_OK &&	m_status == eServerStatusSpecify)
	{
		if (m_name.IsValid(isDns) == FALSE)
			status = STATUS_ILLEGAL;

		if (status == STATUS_OK && (m_port == 0 || m_port == 0xfffffff))
			status = STATUS_ILLEGAL;
	}

	return status;
}


/////////////////////////////////////////////////////////////////////////////
// Class CSipServer
/////////////////////////////////////////////////////////////////////////////
CSipServer::CSipServer()
:m_domain()
{

}

/////////////////////////////////////////////////////////////////////////////
CSipServer::CSipServer(const CSipServer& other):
CBaseSipServer(other), m_domain(other.m_domain)
{
}

/////////////////////////////////////////////////////////////////////////////
CSipServer& CSipServer::operator=(const CSipServer& other)
{
	this->CBaseSipServer::operator=((CBaseSipServer&)other);
	m_domain = other.m_domain;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
bool CSipServer::operator==(const CSipServer &rHnd)const
{
	if(this == &rHnd)
		return true;

	this->CBaseSipServer::operator==((CBaseSipServer&)rHnd);

	if ( m_domain != rHnd.m_domain )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CSipServer::operator!=(const CSipServer& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
CSipServer::~CSipServer()
{
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CSipServer::SerializeXml( CXMLDOMElement *pParentNode )
{
	CXMLDOMElement* pNode;
	pNode = pParentNode->AddChildNode("BASE_PARAM");
	CBaseSipServer::SerializeXml( pNode );
	pParentNode->AddChildNode("DOMAIN_NAME", m_domain );

}

/////////////////////////////////////////////////////////////////////////////
int CSipServer::DeSerializeXml( CXMLDOMElement *pParentNode,char *pszError )
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pChildNode;
	GET_CHILD_NODE(pParentNode, "BASE_PARAM", pChildNode);
	if (pChildNode)
	{
		nStatus = CBaseSipServer::DeSerializeXml( pChildNode, pszError );

		if (nStatus!=STATUS_OK)
			return nStatus;

	}

    GET_VALIDATE_ASCII_CHILD(pParentNode,"DOMAIN_NAME",m_domain,ONE_LINE_BUFFER_LENGTH);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const CSmallString& CSipServer::GetDomainName() const
{
	return m_domain;
}

/////////////////////////////////////////////////////////////////////////////
void  CSipServer::SetDomainName(const CSmallString& domain)
{
	m_domain = domain;
}

/////////////////////////////////////////////////////////////////////////////
ESTATUS CSipServer::TestValidity(BYTE isDns)
{
    DWORD status = STATUS_OK;

	status = CBaseSipServer::TestValidity(isDns);

	if (status == STATUS_OK &&	m_status == eServerStatusSpecify)
	{
		// m_domain can be a string even if dns is off => we send absolute
		// TRUE to IsValid
		if ( m_domain.IsValid(TRUE) == FALSE)
			status = STATUS_ILLEGAL;
	}

	return status;
}

/////////////////////////////////////////////////////////////////////////////
//sip class
CSip::CSip()
{
    m_pProxy                        = new CBaseSipServer;
    m_pAltProxy                     = new CBaseSipServer;
    m_pRegistrar                    = new CSipServer;
    m_pAltRegistrar                 = new CSipServer;

	m_TransportType                 = eTransportTypeUdp;

    m_ConfigureSIPServersMode       = eConfSipServerAuto;

    m_AcceptMeetMe                  = YES;
    m_AcceptAdHoc                   = YES;
    m_AcceptFactory                 = YES;
	m_RefreshStatus                 = YES;

//    m_eRegistrationMode             = eRegistrationMode_Redirect;
    m_RefreshRegistrationTout       = 0;
    m_SipServerType                 = eSipServer_generic;
}

/////////////////////////////////////////////////////////////////////////////
CSip::CSip(const CSip& other)
:CPObject(other)
{

    if (other.m_pProxy == NULL)
        m_pProxy = NULL;
    else
        m_pProxy = new CBaseSipServer(*other.m_pProxy);

   	if (other.m_pAltProxy == NULL)
        m_pAltProxy = NULL;
    else
        m_pAltProxy = new CBaseSipServer(*other.m_pAltProxy);

    if (other.m_pRegistrar == NULL)
        m_pRegistrar = NULL;
    else
        m_pRegistrar = new CSipServer(*other.m_pRegistrar);

    if (other.m_pAltRegistrar == NULL)
        m_pAltRegistrar = NULL;
    else
        m_pAltRegistrar = new CSipServer(*other.m_pAltRegistrar);

    //version 2
	m_TransportType                 = other.m_TransportType;

	m_ConfigureSIPServersMode       = other.m_ConfigureSIPServersMode;

    m_AcceptMeetMe                  = other.m_AcceptMeetMe;
    m_AcceptAdHoc                   = other.m_AcceptAdHoc;
    m_AcceptFactory                 = other.m_AcceptFactory;
	m_RefreshStatus                 = other.m_RefreshStatus;

//    m_eRegistrationMode             = other.m_eRegistrationMode;
    m_RefreshRegistrationTout       = other.m_RefreshRegistrationTout;
    m_SipServerType                 = other.m_SipServerType;
}

/////////////////////////////////////////////////////////////////////////////
CSip::~CSip()
{
    POBJDELETE(m_pProxy);
    POBJDELETE(m_pAltProxy);
    POBJDELETE(m_pRegistrar);
    POBJDELETE(m_pAltRegistrar);
}

/////////////////////////////////////////////////////////////////////////////
CSip& CSip::operator=(const CSip& other)
{
	if(this == &other){
	    return *this;
	}

    POBJDELETE(m_pProxy);
    POBJDELETE(m_pAltProxy);
    POBJDELETE(m_pRegistrar);
    POBJDELETE(m_pAltRegistrar);

    if (other.m_pProxy == NULL)
        m_pProxy = NULL;
    else
        m_pProxy = new CBaseSipServer(*other.m_pProxy);

   	if (other.m_pAltProxy == NULL)
        m_pAltProxy = NULL;
    else
        m_pAltProxy = new CBaseSipServer(*other.m_pAltProxy);

    if (other.m_pRegistrar == NULL)
        m_pRegistrar = NULL;
    else
        m_pRegistrar = new CSipServer(*other.m_pRegistrar);

    if (other.m_pAltRegistrar == NULL)
        m_pAltRegistrar = NULL;
    else
        m_pAltRegistrar = new CSipServer(*other.m_pAltRegistrar);


	//version 2
    m_TransportType                 = other.m_TransportType;

	m_ConfigureSIPServersMode       = other.m_ConfigureSIPServersMode;

    m_AcceptMeetMe                  = other.m_AcceptMeetMe;
    m_AcceptAdHoc                   = other.m_AcceptAdHoc;
    m_AcceptFactory                 = other.m_AcceptFactory;
    m_RefreshStatus                 = other.m_RefreshStatus;

//    m_eRegistrationMode             = other.m_eRegistrationMode;
    m_RefreshRegistrationTout       = other.m_RefreshRegistrationTout;
    m_SipServerType                 = other.m_SipServerType;

    return *this;
}

/////////////////////////////////////////////////////////////////////////////
bool CSip::operator==(const CSip &rHnd)const
{
	if(this == &rHnd)
		return true;

	if ( *m_pProxy != *(rHnd.m_pProxy) )
		return false;

	if ( *m_pAltProxy != *(rHnd.m_pAltProxy) )
		return false;

	if ( *m_pRegistrar != *(rHnd.m_pRegistrar) )
		return false;

	if ( *m_pAltRegistrar != *(rHnd.m_pAltRegistrar) )
		return false;

	if ( m_TransportType != rHnd.m_TransportType )
		return false;

	if ( m_ConfigureSIPServersMode != rHnd.m_ConfigureSIPServersMode )
		return false;

	if ( m_AcceptMeetMe != rHnd.m_AcceptMeetMe )
		return false;

	if ( m_AcceptAdHoc != rHnd.m_AcceptAdHoc )
		return false;

	if ( m_AcceptFactory != rHnd.m_AcceptFactory )
		return false;

	if ( m_RefreshStatus != rHnd.m_RefreshStatus )
		return false;

	if ( m_RefreshRegistrationTout != rHnd.m_RefreshRegistrationTout )
		return false;

    if ( m_SipServerType != rHnd.m_SipServerType )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CSip::operator!=(const CSip& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
void CSip::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pSipNode = pFatherNode->AddChildNode("SIP");

	CXMLDOMElement* pTempNode = pSipNode->AddChildNode("OUTBOUND_PROXY");
	m_pProxy->SerializeXml(pTempNode);
	pTempNode = pSipNode->AddChildNode("PREFERRED_SIP_SERVER");
 	m_pRegistrar->SerializeXml(pTempNode);
	pTempNode = pSipNode->AddChildNode("ALTERNATE_SIP_SERVER");
 	m_pAltRegistrar->SerializeXml(pTempNode);

	pSipNode->AddChildNode("TRANSPORT_TYPE", m_TransportType, TRANSPORT_TYPE_ENUM);
	pSipNode->AddChildNode("CONFIGURATION_SIP_SERVERS_MODE", m_ConfigureSIPServersMode, CONFIGURE_SIP_SERVERS_ENUM);
 	pSipNode->AddChildNode("ACCEPT_MEET_ME", m_AcceptMeetMe, _BOOL);
	pSipNode->AddChildNode("ACCEPT_ADHOC", m_AcceptAdHoc, _BOOL);
	pSipNode->AddChildNode("ACCEPT_FACTORY", m_AcceptFactory, _BOOL);
	pSipNode->AddChildNode("REFRESH_REGISTRATION_TOUT", m_RefreshRegistrationTout);
	pSipNode->AddChildNode("SIP_SERVER_TYPE", m_SipServerType, SIP_SERVER_TYPE_ENUM);
}

/////////////////////////////////////////////////////////////////////////////
int  CSip::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{

	TRACESTR(eLevelDebug) << "\nCSip::SerializeXml  ";
	int nStatus = STATUS_OK;

	CXMLDOMElement *pTempNode;

	GET_CHILD_NODE(pActionNode, "OUTBOUND_PROXY", pTempNode);
	if (pTempNode)
	{
		CBaseSipServer* pOutboundProxy = new CBaseSipServer;
		nStatus = pOutboundProxy->DeSerializeXml(pTempNode, pszError);

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pOutboundProxy);
			return nStatus;
		}
		POBJDELETE(m_pProxy);
		m_pProxy = pOutboundProxy;
	}

	GET_CHILD_NODE(pActionNode, "PREFERRED_SIP_SERVER", pTempNode);
	if (pTempNode)
	{
		CSipServer* pPreferedRegistrar = new CSipServer;
		nStatus = pPreferedRegistrar->DeSerializeXml(pTempNode, pszError);

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pPreferedRegistrar);
			return nStatus;
		}
		POBJDELETE(m_pRegistrar);
		m_pRegistrar = pPreferedRegistrar;
	}

	GET_CHILD_NODE(pActionNode, "ALTERNATE_SIP_SERVER", pTempNode);
	if (pTempNode)
	{
		CSipServer* pAlternateRegistrar = new CSipServer;
		nStatus = pAlternateRegistrar->DeSerializeXml(pTempNode, pszError);

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pAlternateRegistrar);
			return nStatus;
		}
		POBJDELETE(m_pAltRegistrar);
		m_pAltRegistrar = pAlternateRegistrar;
	}

	BYTE tmp = 0;
	GET_VALIDATE_CHILD(pActionNode,"TRANSPORT_TYPE",&tmp,TRANSPORT_TYPE_ENUM);
	m_TransportType = (enTransportType)tmp;

	GET_VALIDATE_CHILD(pActionNode,"CONFIGURATION_SIP_SERVERS_MODE",&tmp,CONFIGURE_SIP_SERVERS_ENUM);
	m_ConfigureSIPServersMode = (eConfigurationSipServerMode)tmp;

	GET_VALIDATE_CHILD(pActionNode,"ACCEPT_MEET_ME",&m_AcceptMeetMe,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"ACCEPT_ADHOC",&m_AcceptAdHoc,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"ACCEPT_FACTORY",&m_AcceptFactory,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"REFRESH_REGISTRATION_TOUT",&m_RefreshRegistrationTout,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"SIP_SERVER_TYPE",&tmp, SIP_SERVER_TYPE_ENUM);
	m_SipServerType = (eSipServerType)tmp;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CSip::SetProxy(const CBaseSipServer& proxy)
{
    *m_pProxy = proxy;
}

/////////////////////////////////////////////////////////////////////////////
void CSip::SetAltProxy(const CBaseSipServer& proxy)
{
    *m_pAltProxy = proxy;
}

/////////////////////////////////////////////////////////////////////////////
void CSip::SetRegistrar(const CSipServer& registrar)
{
    *m_pRegistrar = registrar;
}

/////////////////////////////////////////////////////////////////////////////
void CSip::SetAltRegistrar(const CSipServer& registrar)
{
    *m_pAltRegistrar = registrar;
}

/////////////////////////////////////////////////////////////////////////////
void CSip::SetConfigurationOfSIPServers(const eConfigurationSipServerMode mode)
{
    m_ConfigureSIPServersMode = mode;
}

////////////////////////////////////////////////////////////////////////////*/
enTransportType CSip::GetTransportType() const
{
	return m_TransportType;
}

/////////////////////////////////////////////////////////////////////////////
void CSip::SetTransportType(const enTransportType type)
{
	m_TransportType = type;
}

/////////////////////////////////////////////////////////////////////////////
WORD CSip::GetAcceptMeetMe() const
{
    return m_AcceptMeetMe;
}

/////////////////////////////////////////////////////////////////////////////
void CSip::SetAcceptMeetMe(const WORD Accept_MeetMe)
{
    m_AcceptMeetMe = Accept_MeetMe;
}

/////////////////////////////////////////////////////////////////////////////
WORD CSip::GetAcceptAdHoc() const
{
    return m_AcceptAdHoc;
}

/////////////////////////////////////////////////////////////////////////////
void CSip::SetAcceptAdHoc(const WORD acceptAdHoc)
{
    m_AcceptAdHoc = acceptAdHoc;
}

/////////////////////////////////////////////////////////////////////////////
WORD CSip::GetAcceptFactory() const
{
    return m_AcceptFactory;
}

/////////////////////////////////////////////////////////////////////////////
void CSip::SetAcceptFactory(const WORD Accept_Factory)
{
    m_AcceptFactory = Accept_Factory;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CSip::GetRefreshRegistrationStatus() const
{
    return m_RefreshStatus;
}

/////////////////////////////////////////////////////////////////////////////
void CSip::SetRefreshRegistrationStatus(const WORD Refresh_status)
{
    m_RefreshStatus = Refresh_status;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CSip::GetRefreshRegistrationTout() const
{
    return m_RefreshRegistrationTout;
}

/////////////////////////////////////////////////////////////////////////////
void CSip::SetRefreshRegistrationTout(const DWORD Refresh_Registration_Tout)
{
    m_RefreshRegistrationTout = Refresh_Registration_Tout;
}

/////////////////////////////////////////////////////////////////////////////
ESTATUS CSip::TestValidity(BYTE isDhcpEnabled, BYTE isDns)
{
    DWORD status = STATUS_OK;

    if (status == STATUS_OK &&
		m_ConfigureSIPServersMode != eConfSipServerAuto &&
        m_ConfigureSIPServersMode != eConfSipServerManually)
	{
        status = STATUS_UNKNOWN_CONFIGURATION_FOR_SIP_SERVERS;
	}

	if (status == STATUS_OK &&
		!isDhcpEnabled &&
		!isDns &&
		m_ConfigureSIPServersMode == eConfSipServerAuto)
	{
		status = STATUS_INCONSISTENT_PARAMETERS;
	}

	if (m_ConfigureSIPServersMode == eConfSipServerManually)
	{
		///PROXY///
		if (status == STATUS_OK)
			status = m_pProxy->TestValidity(isDns);
		///ALT-PROXY///
		if (status == STATUS_OK &&
			m_pAltProxy->GetStatus() != eServerStatusSpecify &&
			m_pAltProxy->GetStatus() != eServerStatusOff )
			status = STATUS_INCONSISTENT_PARAMETERS;

		if (status == STATUS_OK &&
			m_pAltProxy->GetStatus() == eServerStatusSpecify)
			status = m_pAltProxy->TestValidity(isDns);
		///REGISTRAR///
		if (status == STATUS_OK &&
			m_pRegistrar->GetStatus() != eServerStatusSpecify &&
			m_pRegistrar->GetStatus() != eServerStatusOff )
			status = STATUS_INCONSISTENT_PARAMETERS;

		if (status == STATUS_OK &&
			m_pRegistrar->GetStatus() == eServerStatusSpecify)
			status = m_pRegistrar->TestValidity(isDns);
		///ALT_REGISTAR///
		if (status == STATUS_OK &&
			m_pAltRegistrar->GetStatus() != eServerStatusSpecify &&
			m_pAltRegistrar->GetStatus() != eServerStatusOff )
			status = STATUS_INCONSISTENT_PARAMETERS;

		if (status == STATUS_OK &&
			m_pAltRegistrar->GetStatus() == eServerStatusSpecify)
			status = m_pAltRegistrar->TestValidity(isDns);
	}

    if (status == STATUS_OK &&
		m_TransportType != PROTOCOL_TRASPORT_UDP &&
		m_TransportType != PROTOCOL_TRASPORT_TCP   )
		status = STATUS_INCONSISTENT_PARAMETERS;

    if (status == STATUS_OK &&
		m_AcceptMeetMe != NO &&
        m_AcceptMeetMe != YES  )
        status = STATUS_UNKNOWN_SIP_ACCEPT_MEET_ME_STATUS;

    if (status == STATUS_OK &&
		m_AcceptAdHoc != NO &&
        m_AcceptAdHoc != YES  )
        status = STATUS_UNKNOWN_SIP_ACCEPT_AD_HOK_STATUS;

    if (status == STATUS_OK &&
		m_AcceptFactory != NO &&
        m_AcceptFactory != YES  )
        status = STATUS_UNKNOWN_SIP_ACCEPT_FACTORY_STATUS;

    if (status == STATUS_OK &&
		m_RefreshStatus != YES &&
        m_RefreshStatus != NO   )
        status = STATUS_UNKNOWN_SIP_REFRESH_STATUS;

	if (status == STATUS_OK &&
		m_RefreshRegistrationTout == 0xFFFFFFFF)
		status = STATUS_INCONSISTENT_PARAMETERS;

    return status;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
//					Class  CIPSpan
///////////////////////////////////////////////////////////////////////////////////////////////////
CIPSpan::CIPSpan()
{
    m_lineNumber    = 0;
    m_numb_of_names = 0;

    m_IPv4Address.ip	= 0;
    for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++ )
    {
    	memset(m_IPv6AaddressArray[i].ip, 0, IPV6_ADDRESS_BYTES_LEN);
    	m_IPv6AaddressArray[i].scopeId	= eIPv6AddressScope_global;
    	m_IPv6MaskArray[i]				= DEFAULT_IPV6_SUBNET_MASK;
    }

    for (WORD i=0;i<MAX_ALIAS_NAMES_NUM;i++)
        m_h323alias[i]=NULL;

    m_RASport        = 6023;
    m_callSignalPort = 1720;
    m_speed          = ePortSpeed_Auto;

    m_uniVersion          = DEFAULT_UNI_PROTOCOL_VER;
    m_maxNTUSize          = DEFAULT_MAX_NTU_SIZE;
    m_IlmiEnabled         = ILMI_ENABLE;
//    m_pCardATMIlmiAddress = new CAtmAddr;

    m_ind_name   = 0;
    m_pPortRange = new CCommH323PortRange;

    //SIP
    m_SIP_Name[0]  = '\0';
    m_eIPSpanType  = eIPSpanType_URI;
    m_Host_Name[0] = '\0';
	m_pNat         = new CIpNat;

	m_bIsSpanEnabled = TRUE;

	m_Interface = "";
}

////////////////////////////////////////////////////////////////////////////
CIPSpan::CIPSpan( const CIPSpan &other)
:CPObject(other)
{
	m_IPv4Address.ip	= other.m_IPv4Address.ip;
    for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++ )
    {
    	memcpy( &(m_IPv6AaddressArray[i]), &(other.m_IPv6AaddressArray[i]), sizeof(ipAddressV6If) );
    	m_IPv6MaskArray[i] = other.m_IPv6MaskArray[i];
    }

    m_lineNumber = other.m_lineNumber;

    for (WORD i=0;i<MAX_ALIAS_NAMES_NUM;i++)
	{
        if (other.m_h323alias[i]==NULL)
            m_h323alias[i]=NULL;
        else
            m_h323alias[i]= new CH323Alias(*other.m_h323alias[i]);
    }

    m_numb_of_names  = other.m_numb_of_names;
    m_RASport        = other.m_RASport;
    m_callSignalPort = other.m_callSignalPort;

    m_uniVersion  = other.m_uniVersion;
    m_maxNTUSize  = other.m_maxNTUSize;
    m_IlmiEnabled = other.m_IlmiEnabled;

/*
    if (other.m_pCardATMIlmiAddress == NULL)
        m_pCardATMIlmiAddress = NULL ;
    else
        m_pCardATMIlmiAddress = new CAtmAddr(*other.m_pCardATMIlmiAddress);
*/
    m_speed      = other.m_speed;
    if (other.m_pPortRange)
      m_pPortRange = new CCommH323PortRange(*other.m_pPortRange);
    else
      m_pPortRange = NULL;

    //SIP
    m_SIP_Name    = other.m_SIP_Name;
    m_eIPSpanType = other.m_eIPSpanType;
    m_Host_Name   = other.m_Host_Name;
    if (other.m_pNat)
    	m_pNat        = new CIpNat(*other.m_pNat);
    else
    	m_pNat = NULL;

    m_bIsSpanEnabled = other.m_bIsSpanEnabled;

    m_Interface = other.m_Interface;
}

////////////////////////////////////////////////////////////////////////////
CIPSpan& CIPSpan::operator=(const CIPSpan& other)
{
	if(this == &other){
	    return *this;
	}

	m_IPv4Address.ip	= other.m_IPv4Address.ip;
    for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++ )
    {
    	memcpy( &(m_IPv6AaddressArray[i]), &(other.m_IPv6AaddressArray[i]), sizeof(ipAddressV6If) );
    	m_IPv6MaskArray[i] = other.m_IPv6MaskArray[i];
    }

    m_lineNumber = other.m_lineNumber;

    for (WORD i=0;i<MAX_ALIAS_NAMES_NUM;i++)
    {
        PDELETE(m_h323alias[i]);
        if (other.m_h323alias[i]==NULL)
            m_h323alias[i]=NULL;
        else
            m_h323alias[i]= new CH323Alias(*other.m_h323alias[i]);
    }

    m_numb_of_names  = other.m_numb_of_names;
    m_RASport        = other.m_RASport;
    m_callSignalPort = other.m_callSignalPort;

    m_uniVersion  = other.m_uniVersion;
    m_maxNTUSize  = other.m_maxNTUSize;
    m_IlmiEnabled = other.m_IlmiEnabled;
    m_speed = other.m_speed;

    if(m_pPortRange != NULL)
        PDELETE(m_pPortRange);
    m_pPortRange = new CCommH323PortRange(*other.m_pPortRange);

    //SIP
    m_SIP_Name    = other.m_SIP_Name;
    m_eIPSpanType = other.m_eIPSpanType;
    m_Host_Name   = other.m_Host_Name;
	m_pNat        = new CIpNat(*other.m_pNat);

	m_bIsSpanEnabled = other.m_bIsSpanEnabled;

	m_Interface = other.m_Interface;

    return *this;
}


////////////////////////////////////////////////////////////////////////////
bool CIPSpan::operator==(const CIPSpan& other) const
{
	if(this == &other)
		return true;

	char szIP[IPV6_ADDRESS_LEN];
	eIPv6AddressScope curV6AddressScope = eIPv6AddressScope_other;
	FTRACEINTO << "BRIDGE-8284 compare  CIPSpan compare ";

    if (m_IPv4Address.ip != other.m_IPv4Address.ip)
        return false;

    FTRACESTR(eLevelInfoNormal) << "CIPSpan::operator== - check Ipv6 Ip";

    for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++ )
    {

    	if (memcmp ( &(m_IPv6AaddressArray[i]), &(other.m_IPv6AaddressArray[i]), sizeof(ipAddressV6If)) != 0)
    	{
    		FTRACESTR(eLevelInfoNormal) << "CIPSpan::operator== difference in ipv6 at i " << i;
    		::ipV6ToString(m_IPv6AaddressArray[i].ip, szIP, FALSE);
    		std::string sIpv6(szIP);
    		FTRACESTR(eLevelInfoNormal) << "CIPSpan::operator== - m_IPv6AaddressArray[i].ip = " << sIpv6.c_str();

    		 ::ipV6ToString(other.m_IPv6AaddressArray[i].ip, szIP, FALSE);
    		 std::string s2Ipv6(szIP);
    		 FTRACESTR(eLevelInfoNormal) << "CIPSpan::operator== -other.m_IPv6AaddressArray[i] = " << s2Ipv6.c_str();

    		 FTRACESTR(eLevelInfoNormal) << "CIPSpan::operator== - check Ipv6 scopeId";
    		 curV6AddressScope = ConvertenScopeIdToeIpv6AddressScope((enScopeId)m_IPv6AaddressArray[i].scopeId);
    		 const char* curV6AddressScopeStr = GetIPv6AddressScopeStr(curV6AddressScope);
    		 FTRACESTR(eLevelInfoNormal) << "CIPSpan::operator==m_IPv6AaddressArray[i].scopeId "<<curV6AddressScopeStr;
    		 curV6AddressScope = ConvertenScopeIdToeIpv6AddressScope((enScopeId)other.m_IPv6AaddressArray[i].scopeId);
    		 curV6AddressScopeStr = GetIPv6AddressScopeStr(curV6AddressScope);
    		 FTRACESTR(eLevelInfoNormal) << "CIpDns::operator==rHnd.m_IPv6AaddressArray[i].scopeId "<<curV6AddressScopeStr;
    		 if((sIpv6 !="::") || (s2Ipv6!="::"))
    				 return false;
    	}
    	if (m_IPv6MaskArray[i] != other.m_IPv6MaskArray[i])
    	{
    		FTRACESTR(eLevelInfoNormal) << "mask are not equal" ;
            return false;
    	}
    }
    FTRACESTR(eLevelInfoNormal) << " BRIDGE-8284 compare line number" ;
    if (m_lineNumber != other.m_lineNumber)
        return false;
    FTRACESTR(eLevelInfoNormal) << " BRIDGE-8284 compare number names" ;
    if (m_numb_of_names  != other.m_numb_of_names)
        return false;
    FTRACESTR(eLevelInfoNormal) << " BRIDGE-8284 compare alias names" ;

    for (WORD i=0;i<m_numb_of_names;i++)
    {
        if (*m_h323alias[i] != *(other.m_h323alias[i]) )
            return false;
    }

    FTRACESTR(eLevelInfoNormal) << " BRIDGE-8284 compare m_RASport" ;

    if (m_RASport        != other.m_RASport)
        return false;
    FTRACESTR(eLevelInfoNormal) << " BRIDGE-8284 compare m_callSignalPort" ;
    if (m_callSignalPort != other.m_callSignalPort)
        return false;
    FTRACESTR(eLevelInfoNormal) << " BRIDGE-8284 compare m_uniVersion" ;
    if (m_uniVersion  != other.m_uniVersion)
        return false;
    FTRACESTR(eLevelInfoNormal) << " BRIDGE-8284 compare m_maxNTUSize" ;
    if (m_maxNTUSize  != other.m_maxNTUSize)
        return false;
    FTRACESTR(eLevelInfoNormal) << " BRIDGE-8284 compare m_IlmiEnabled" ;
    if (m_IlmiEnabled != other.m_IlmiEnabled)
        return false;
    FTRACESTR(eLevelInfoNormal) << " BRIDGE-8284 compare m_speed" ;
    if (m_speed != other.m_speed)
        return false;
    FTRACESTR(eLevelInfoNormal) << " BRIDGE-8284 compare m_pPortRange" ;
    if (*m_pPortRange != *(other.m_pPortRange) )
		return false;

    FTRACESTR(eLevelInfoNormal) << " BRIDGE-8284 compare m_SIP_Name" ;
    //SIP
    if (m_SIP_Name    != other.m_SIP_Name)
        return false;

    if (m_eIPSpanType != other.m_eIPSpanType)
        return false;

    if (m_Host_Name   != other.m_Host_Name)
        return false;
    FTRACESTR(eLevelInfoNormal) << " BRIDGE-8284 compare m_pNat" ;
	if (*m_pNat != *(other.m_pNat) )
        return false;

	if (m_bIsSpanEnabled != other.m_bIsSpanEnabled)
		return false;
	FTRACESTR(eLevelInfoNormal) << " BRIDGE-8284 compare m_Interface" ;
	if( m_Interface != other.m_Interface )
		return false;

    return true;

}

////////////////////////////////////////////////////////////////////////////
bool CIPSpan::operator!=(const CIPSpan& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
CIPSpan::~CIPSpan()
{
    for (WORD i=0;i<MAX_ALIAS_NAMES_NUM;i++)
        PDELETE(m_h323alias[i]);
//    PDELETE(m_pCardATMIlmiAddress);
    PDELETE(m_pPortRange);
	PDELETE(m_pNat);
}

/////////////////////////////////////////////////////////////////////////////
void CIPSpan::SetIPSpanType(const eIPSpanType IPSpanType)
{
    m_eIPSpanType = IPSpanType;
}

/////////////////////////////////////////////////////////////////////////////
CIpNat* CIPSpan::GetIpNat()
{
	return m_pNat;
}

/////////////////////////////////////////////////////////////////////////////
void CIPSpan::SetNat(const CIpNat& nat)
{
	*m_pNat = nat;
}


/////////////////////////////////////////////////////////////////////////////
//schema: obj_ip_span.xsd
void CIPSpan::SerializeXml(CXMLDOMElement* pFatherNode)
{
	int i=0;

	CXMLDOMElement* pH323SpanNode = pFatherNode->AddChildNode("IP_SPAN");

	pH323SpanNode->AddChildNode("LINE_NUMBER", m_lineNumber);
	pH323SpanNode->AddChildNode("IP", m_IPv4Address.ip, IP_ADDRESS);

	CXMLDOMElement* pAliasNode = pH323SpanNode->AddChildNode("ALIAS_LIST");
	for (i=0;i<m_numb_of_names;i++)
	{
		if(m_h323alias[i]) //&& m_h323alias[i]->GetAliasType())
		        m_h323alias[i]->SerializeXml(pAliasNode);
	}
	pH323SpanNode->AddChildNode("RAS_PORT", m_RASport);
	pH323SpanNode->AddChildNode("CALL_SIGNAL_PORT", m_callSignalPort);
	pH323SpanNode->AddChildNode("SPEED", m_speed, SPEED_MODE_ENUM);
	m_pPortRange->SerializeXml(pH323SpanNode);

	pH323SpanNode->AddChildNode("HOST_NAME", m_Host_Name);

	m_pNat->SerializeXml(pH323SpanNode);

//	pH323SpanNode->AddChildNode("IP_V6_SUBNET_MASK",		m_ipV6SubnetMask);

    // ===== IPv6 params
	CXMLDOMElement* pIPv6ListNode = pH323SpanNode->AddChildNode("IP_V6_LIST");
	char curV6Mask[IPV6_ADDRESS_LEN];
	eIPv6AddressScope curV6AddressScope = eIPv6AddressScope_other;

	for (i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		CXMLDOMElement* pIPv6AddressNode = pIPv6ListNode->AddChildNode("IP_V6_ADDRESS");

		memset(curV6Mask, 0, IPV6_ADDRESS_LEN);
		GetIPv6SubnetMaskStr(i, curV6Mask);

		pIPv6AddressNode->AddIPv6ChildNode("IP_V6", m_IPv6AaddressArray[i].ip, curV6Mask);

		char szIP[IPV6_ADDRESS_LEN];
		memset(szIP, 0, IPV6_ADDRESS_LEN);
		::ipV6ToString(m_IPv6AaddressArray[i].ip, szIP, FALSE);

		curV6AddressScope = GetIPv6AddressScope(szIP);
		const char* curV6AddressScopeStr = GetIPv6AddressScopeStr(curV6AddressScope);
		pIPv6AddressNode->AddChildNode("IP_V6_ADDRESS_SCOPE", curV6AddressScopeStr, IP_V6_ADDRESS_SCOPE_ENUM);
	}

	pH323SpanNode->AddChildNode("IS_SPAN_ENABLE", m_bIsSpanEnabled, _BOOL);
	pH323SpanNode->AddChildNode("NETWORK_INTERFACE", m_Interface);

}

/////////////////////////////////////////////////////////////////////////////
//schema: obj_h323_span.xsd
int  CIPSpan::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"LINE_NUMBER",&m_lineNumber,_0_TO_WORD);

	CXMLDOMElement *pTempNode;

	GET_VALIDATE_CHILD(pActionNode,"IP", &m_IPv4Address.ip, IP_ADDRESS);

	CXMLDOMElement *pAliasListNode;

	GET_CHILD_NODE(pActionNode, "ALIAS_LIST", pAliasListNode);

	if (pAliasListNode)
	{

		m_numb_of_names=0;
		GET_FIRST_CHILD_NODE(pAliasListNode,"ALIAS",pTempNode);

		while (pTempNode && m_numb_of_names < MAX_ALIAS_NAMES_NUM)
		{
			CH323Alias* pH323Alias = new CH323Alias;
			nStatus = pH323Alias->DeSerializeXml(pTempNode, pszError);
			if (nStatus != STATUS_OK)
			{
				POBJDELETE(pH323Alias);
				return nStatus;
			}
			if (pH323Alias->m_aliasName[0] == '\0' || FindAlias(*pH323Alias) != NOT_FIND)
			{
				POBJDELETE(pH323Alias);
			}
			else
			{
				POBJDELETE(m_h323alias[m_numb_of_names]);
				m_h323alias[m_numb_of_names] = pH323Alias;
				m_numb_of_names++;
			}
			GET_NEXT_CHILD_NODE(pAliasListNode,"ALIAS",pTempNode);
		}
	}

	GET_VALIDATE_CHILD(pActionNode,"RAS_PORT",&m_RASport,_0_TO_WORD);
	GET_VALIDATE_CHILD(pActionNode,"CALL_SIGNAL_PORT",&m_callSignalPort,_0_TO_WORD);
	GET_VALIDATE_CHILD(pActionNode,"SPEED",&m_speed,SPEED_MODE_ENUM);

	GET_CHILD_NODE(pActionNode, "PORT_RANGE", pTempNode);

	if (pTempNode)
	{
		CCommH323PortRange* pH323PortRange = new CCommH323PortRange;
		nStatus = pH323PortRange->DeSerializeXml(pTempNode, pszError);
		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pH323PortRange);
			return nStatus;
		}
		POBJDELETE(m_pPortRange);
		m_pPortRange = pH323PortRange;
	}

    GET_VALIDATE_ASCII_CHILD(pActionNode,"HOST_NAME",m_Host_Name,ONE_LINE_BUFFER_LENGTH);

	GET_CHILD_NODE(pActionNode, "NAT", pTempNode);

	if (pTempNode)
	{
		CIpNat* pNat = new CIpNat;
		nStatus = pNat->DeSerializeXml(pTempNode, pszError);
		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pNat);
			return nStatus;
		}
		POBJDELETE(m_pNat);
		m_pNat = pNat;
	}

//	GET_VALIDATE_CHILD(pActionNode,"IP_V6_SUBNET_MASK",			&m_ipV6SubnetMask,_0_TO_DWORD);

    CXMLDOMElement *pIPv6ListNode;
    GET_CHILD_NODE(pActionNode, "IP_V6_LIST", pIPv6ListNode);
    if (pIPv6ListNode)
    {
        GET_FIRST_CHILD_NODE(pIPv6ListNode,"IP_V6_ADDRESS",pTempNode);

        char curFullAddress[IPV6_ADDRESS_LEN];
        char tmpAddr[IPV6_ADDRESS_LEN];
        char tmpMask[IPV6_ADDRESS_LEN];
        char curV6AddressScopeStr[ONE_LINE_BUFFER_LEN];

        int i=0;
        while ( pTempNode && (i<NUM_OF_IPV6_ADDRESSES) )
        {
            memset(curFullAddress,	0, IPV6_ADDRESS_LEN);
            memset(tmpAddr,			0, IPV6_ADDRESS_LEN);
            memset(tmpMask,			0, IPV6_ADDRESS_LEN);

            GET_VALIDATE_ASCII_CHILD(pTempNode,"IP_V6",curFullAddress,_0_TO_IPV6_ADDRESS_LENGTH);

            SplitIPv6AddressAndMask(curFullAddress, tmpAddr, tmpMask);

            SetIPv6Address(i, tmpAddr);
//                   m_IPv6AaddressArray[i].scopeId = ::getScopeId(tmpAddr); // done inside SetIPv6Address() method
            SetIPv6SubnetMask(i, tmpMask);

                  //   memset(curV6AddressScopeStr, 0, ONE_LINE_BUFFER_LEN);
                  //   GET_VALIDATE_CHILD(pIPv6AddressNode,"IP_V6_ADDRESS_SCOPE", curV6AddressScopeStr, IP_V6_ADDRESS_SCOPE_ENUM);

            i++;
            GET_NEXT_CHILD_NODE(pIPv6ListNode,"IP_V6_ADDRESS",pTempNode);
        }
    }

    GET_VALIDATE_CHILD(pActionNode,"IS_SPAN_ENABLE",&m_bIsSpanEnabled,_BOOL);
    GET_VALIDATE_CHILD(pActionNode,"NETWORK_INTERFACE", m_Interface, ONE_LINE_BUFFER_LENGTH);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CIPSpan::GetLineNumber () const
{
    return m_lineNumber;
}

/////////////////////////////////////////////////////////////////////////////
void  CIPSpan::SetLineNumber(const WORD  lineNumber)
{
    m_lineNumber=lineNumber;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CIPSpan::GetIPv4Address () const
{
    return m_IPv4Address.ip;
}

/////////////////////////////////////////////////////////////////////////////
void  CIPSpan::SetIPv4Address(const DWORD  ipV4Address)
{
	m_IPv4Address.ip = ipV4Address;
}

/////////////////////////////////////////////////////////////////////////////
void CIPSpan::GetIPv6Address(int idx, char* retStr, BOOL isBrackets/*=FALSE*/) const
{
	if ( (0 <= idx) && (NUM_OF_IPV6_ADDRESSES > idx) )
	{
		::ipV6ToString(m_IPv6AaddressArray[idx].ip, retStr , isBrackets);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CIPSpan::GetFullIPv6Address(int idx, char* retStr, BOOL isBrackets/*=FALSE*/) const
{
	if ( (0 <= idx) && (NUM_OF_IPV6_ADDRESSES > idx) )
	{
		::ipV6AndSubnetMaskToString(m_IPv6AaddressArray[idx].ip,
									m_IPv6MaskArray[idx],
									retStr,
									isBrackets);
	}
}

/////////////////////////////////////////////////////////////////////////////
const APIU8* CIPSpan::GetIPv6AddressByteArray(int idx) const
{
	if (GetIsIpV6Null(idx) )
	{
		return NULL;
	}	
	return (const APIU8* )m_IPv6AaddressArray[idx].ip;
	
}

/////////////////////////////////////////////////////////////////////////////
std::string CIPSpan::GetIPv6Address(int idx /*= 0*/, BOOL isBrackets/*=FALSE*/) const
{
    char ipv6_string[IPV6_ADDRESS_LEN+1] = {0};
    GetIPv6Address(idx,ipv6_string,isBrackets);
    return std::string(ipv6_string);
}

/////////////////////////////////////////////////////////////////////////////
std::string CIPSpan::GetFullIPv6Address(int idx, BOOL isBrackets /*=FALSE*/) const
{
    char ipv6_string[IPV6_ADDRESS_LEN+1] = {0};
    GetFullIPv6Address(idx,ipv6_string,isBrackets);
    return std::string(ipv6_string);
}


/////////////////////////////////////////////////////////////////////////////
void CIPSpan::SetIPv6Address(int idx, const char* ipV6Address)
{
	if ( (0 <= idx) && (NUM_OF_IPV6_ADDRESSES > idx) )
	{
		// ===== 1. address
		mcTransportAddress tmpIPv6Addr;
		memset(&tmpIPv6Addr, 0, sizeof(mcTransportAddress));
		::stringToIpV6( &tmpIPv6Addr, (char*)ipV6Address );
		memcpy( &(m_IPv6AaddressArray[idx].ip), &(tmpIPv6Addr.addr.v6.ip), IPV6_ADDRESS_BYTES_LEN );

		// ===== 2. scopeId
		m_IPv6AaddressArray[idx].scopeId = ::getScopeId( (char*)ipV6Address );
	}
}

/////////////////////////////////////////////////////////////////////////////
DWORD CIPSpan::GetIPv6SubnetMask(int idx) const
{
	DWORD retMask = 0;

	if ( (0 <= idx) && (NUM_OF_IPV6_ADDRESSES > idx) )
	{
		retMask = m_IPv6MaskArray[idx];
	}

	return retMask;
}

/////////////////////////////////////////////////////////////////////////////
void CIPSpan::GetIPv6SubnetMaskStr(int idx, char *pOutMask) const
{
	if ( (0 <= idx) && (NUM_OF_IPV6_ADDRESSES > idx) )
	{
		sprintf(pOutMask, "%d", m_IPv6MaskArray[idx]);
	}
	else
	{
		pOutMask[0] = '\0';
	}
}

/////////////////////////////////////////////////////////////////////////////
void CIPSpan::SetIPv6SubnetMask(int idx, const DWORD subnetMask)
{
	if ( (0 <= idx) && (NUM_OF_IPV6_ADDRESSES > idx) )
	{
		m_IPv6MaskArray[idx] = subnetMask;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CIPSpan::SetIPv6SubnetMask(int idx, const char *pMask)
{
	if ( (0 <= idx) && (NUM_OF_IPV6_ADDRESSES > idx) )
	{
		sscanf(pMask, "%d", &m_IPv6MaskArray[idx]);
	}
}

/////////////////////////////////////////////////////////////////////////////
bool CIPSpan::GetIsIpV4Null() const
{
	bool ret = IsIpNull(m_IPv4Address);

	return ret;
}

/////////////////////////////////////////////////////////////////////////////
bool CIPSpan::GetIsIpV6Null(int idx) const
{
	bool ret = true;

	if ( (0 <= idx) && (NUM_OF_IPV6_ADDRESSES > idx) )
	{
		ret = IsIpNull( m_IPv6AaddressArray[idx] );
	}

	return ret;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CIPSpan::GetRASport () const
{
    return m_RASport;
}

/////////////////////////////////////////////////////////////////////////////
void  CIPSpan::SetRASport(const WORD  RASport)
{
    m_RASport=RASport;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CIPSpan::GetIsSpanEnable()
{
	return m_bIsSpanEnabled;
}

/////////////////////////////////////////////////////////////////////////////
void CIPSpan::SetIsSpanEnable(BOOL is_span_enabled)
{
	m_bIsSpanEnabled = is_span_enabled;
}

/////////////////////////////////////////////////////////////////////////////
std::string CIPSpan::GetInterface()
{
	return m_Interface;
}

/////////////////////////////////////////////////////////////////////////////
void CIPSpan::SetInterface(std::string interface)
{
	m_Interface =  interface;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CIPSpan::GetCallSignalPort () const
{
    return m_callSignalPort;
}

/////////////////////////////////////////////////////////////////////////////
void  CIPSpan::SetCallSignalPort(const WORD  callSignalPort)
{
    m_callSignalPort=callSignalPort;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CIPSpan::GetSpeed() const
{
    return m_speed;
}

/////////////////////////////////////////////////////////////////////////////
void  CIPSpan::SetSpeed( const WORD speed )
{
    m_speed = speed;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CIPSpan::GetAliasNamesNumber() const
{
    return m_numb_of_names;
}

/////////////////////////////////////////////////////////////////////////////
void  CIPSpan::SetAliasNamesNumber( const WORD num )
{
    m_numb_of_names = num;
}

/////////////////////////////////////////////////////////////////////////////
int CIPSpan::ReplaceAliasList(CH323Alias* otherAliasList[], int listLen)
{
	if( (NULL == otherAliasList) || (0 == listLen) )
	{
		return STATUS_ILLEGAL;
	}

	for(int i = 0 ; i < m_numb_of_names ; ++i)
	{
		PDELETE(m_h323alias[i]);
	}
	m_numb_of_names = 0;


	int len = MIN_(MAX_ALIAS_NAMES_NUM, listLen);
	for(int i = 0 ; i < len ; ++i)
	{
		if( NULL == (CH323Alias*)otherAliasList[i] )
		{
			m_h323alias[i] = NULL;
		}
		else
		{
			m_h323alias[i] = new CH323Alias(*(otherAliasList[i]));
			++m_numb_of_names;
		}
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int  CIPSpan::AddAlias(const CH323Alias& other)
{
    if(m_numb_of_names >= MAX_ALIAS_NAMES_NUM )
        return  STATUS_ILLEGAL;

    if(FindAlias( other ) != NOT_FIND )
        return STATUS_ILLEGAL;

    m_h323alias[m_numb_of_names] = new CH323Alias( other );
    m_numb_of_names++;

    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int  CIPSpan::UpdateAlias(const CH323Alias& other)
{
    int ind = FindAlias( other );
    if(ind==NOT_FIND || ind >= MAX_ALIAS_NAMES_NUM)
    	return STATUS_ILLEGAL;

    PDELETE(m_h323alias[ind]);
    m_h323alias[ind] = new CH323Alias( other );

    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int  CIPSpan::CancelAlias(int ind)
{
    if(ind >= 0 && ind < MAX_ALIAS_NAMES_NUM)
    {
    	PDELETE( m_h323alias[ind] );
    }
    int i;
    for(i=0; i<(int)m_numb_of_names; i++ )
    {
        if(m_h323alias[i]==NULL )
            break;
    }
    for( int j=i; j<(int)m_numb_of_names-1; j++ )
    {
        m_h323alias[j] = m_h323alias[j+1] ;
    }
    m_h323alias[m_numb_of_names-1] = NULL;
    m_numb_of_names--;

    return  STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int  CIPSpan::CancelAlias(const char* aliasName)
{
    int ind = FindAlias( aliasName );
    if(ind==NOT_FIND ) return STATUS_ILLEGAL;

    return CancelAlias(ind);
 }


/////////////////////////////////////////////////////////////////////////////
int CIPSpan::CancelAliasList()
{
    for (WORD i=0;i<MAX_ALIAS_NAMES_NUM;i++)
    {
        PDELETE(m_h323alias[i]);
    }

    m_numb_of_names = 0;

    return  STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CIPSpan::FindAlias(const CH323Alias& other)
{
    for( int i=0; i<(int)m_numb_of_names; i++ )
    {
        if (m_h323alias[i])
            if ( !strcmp(m_h323alias[i]->GetAliasName(),other.GetAliasName()) )
                return i;
    }
    return NOT_FIND;
}

/////////////////////////////////////////////////////////////////////////////
int CIPSpan::FindAlias(const char* aliasName)
{
    for( int i=0; i<(int)m_numb_of_names; i++ )
    {
        if (m_h323alias[i])
            if ( !strcmp(m_h323alias[i]->GetAliasName(),aliasName) )
                return i;
    }

    return NOT_FIND;
}

/////////////////////////////////////////////////////////////////////////////
//SIP
const CSmallString& CIPSpan::GetSIPName() const
{
    return m_SIP_Name;
}

/////////////////////////////////////////////////////////////////////////////
void CIPSpan::SetSIPName(const  CSmallString& SIP_Name)
{
    m_SIP_Name = SIP_Name;
}

/////////////////////////////////////////////////////////////////////////////
const CSmallString& CIPSpan::GetSpanHostName() const
{
    return m_Host_Name;
}

/////////////////////////////////////////////////////////////////////////////
void CIPSpan::SetSpanHostName(const  CSmallString& Host_Name)
{
    m_Host_Name = Host_Name;
}

/////////////////////////////////////////////////////////////////////////////
CH323Alias** CIPSpan::GetAliasList()
{
	return m_h323alias;
}

/////////////////////////////////////////////////////////////////////////////
CH323Alias* CIPSpan::GetFirstAlias()
{
    m_ind_name=1;
    return m_h323alias[0];
}

/////////////////////////////////////////////////////////////////////////////
CH323Alias* CIPSpan::GetNextAlias()
{
    if(m_ind_name>=m_numb_of_names ) return NULL;
    return m_h323alias[m_ind_name++];
}

/////////////////////////////////////////////////////////////////////////////
CH323Alias* CIPSpan::GetCurrentAlias(const char* aliasName)
{
    for( int i=0; i<(int)m_numb_of_names; i++ )
    {
        if(m_h323alias[i] )
            if(!strcmp(m_h323alias[i]->GetAliasName(),aliasName) )
                return m_h323alias[i];
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////
CH323Alias* CIPSpan::GetAlias(WORD ind)
{
    if (ind >= MAX_ALIAS_NAMES_NUM || ind >= m_numb_of_names)
        return NULL;

    return m_h323alias[ind];
}

///////////////////////////////////////////////////////////////////////////////////
void  CIPSpan::SetMaxNTUSize(const WORD maxNTUSize)
{
    m_maxNTUSize=maxNTUSize;
}

////////////////////////////////////////////////////////////////////////////////
WORD  CIPSpan::GetMaxNTUSize()const
{
    return m_maxNTUSize;
}
/////////////////////////////////////////////////////////////////////////////
CCommH323PortRange* CIPSpan::GetPortRange() const
{
    return m_pPortRange;
}
/////////////////////////////////////////////////////////////////////////////

void CIPSpan::SetPortRange(const CCommH323PortRange& PortRange)
{
    *m_pPortRange = PortRange;
}
/////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////
//			se CQualityOfService
/////////////////////////////////////////////////////////////////////////////////////////////////
CQualityOfService::CQualityOfService()
:	m_eIpStatus(eQoS_disable), m_bIpIsDiffServ(0x0), m_bIpValueTOS(0x08),
m_bIpPrecedenceAudio(0x04), m_bIpPrecedenceVideo(0x04),
m_eAtmStatus(eQoS_disable),
m_bAtmPrecedenceAudio(H323_ATM_QOS_UBR), m_bAtmPrecedenceVideo(H323_ATM_QOS_UBR)
{
}

/////////////////////////////////////////////////////////////////////////////
CQualityOfService::CQualityOfService(const CQualityOfService& other)
:CPObject(other)
{
    *this = other;
}

/////////////////////////////////////////////////////////////////////////////
CQualityOfService& CQualityOfService::operator= ( const CQualityOfService& other )
{
    if(this == &other)
    {
        return *this;
    }

    m_eIpStatus             = other.m_eIpStatus;
    m_bIpIsDiffServ         = other.m_bIpIsDiffServ;
    m_bIpValueTOS           = other.m_bIpValueTOS;
    m_bIpPrecedenceAudio    = other.m_bIpPrecedenceAudio;
    m_bIpPrecedenceVideo    = other.m_bIpPrecedenceVideo;
    m_eAtmStatus            = other.m_eAtmStatus;
    m_bAtmPrecedenceAudio   = other.m_bAtmPrecedenceAudio;
    m_bAtmPrecedenceVideo   = other.m_bAtmPrecedenceVideo;

    TestValidity();

    return *this;
}

/////////////////////////////////////////////////////////////////////////////
bool CQualityOfService::operator==(const CQualityOfService &rHnd)const
{
	if(this == &rHnd)
		return true;

	if ( m_eIpStatus != rHnd.m_eIpStatus )
		return false;

	if ( m_bIpIsDiffServ != rHnd.m_bIpIsDiffServ )
		return false;

	if ( m_bIpValueTOS != rHnd.m_bIpValueTOS )
		return false;

	if ( m_bIpPrecedenceAudio != rHnd.m_bIpPrecedenceAudio )
		return false;

	if ( m_bIpPrecedenceVideo != rHnd.m_bIpPrecedenceVideo )
		return false;

	if ( m_eAtmStatus != rHnd.m_eAtmStatus )
		return false;

	if ( m_bAtmPrecedenceAudio != rHnd.m_bAtmPrecedenceAudio )
		return false;

	if ( m_bAtmPrecedenceVideo != rHnd.m_bAtmPrecedenceVideo )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CQualityOfService::operator!=(const CQualityOfService& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
CQualityOfService::~CQualityOfService()
{
}

/////////////////////////////////////////////////////////////////////////////
void  CQualityOfService::SetIpStatus(const int status)
{
	if((status < eQoS_unknown) && (status >= eQoS_disable) )
        m_eIpStatus = (eStatusQoS)status;
    else
        m_eIpStatus = eQoS_unknown;
}

/////////////////////////////////////////////////////////////////////////////
void  CQualityOfService::SetIsDiffServ(const BYTE isDiffServ)
{
    m_bIpIsDiffServ = (isDiffServ) ? YES : NO;
}

/////////////////////////////////////////////////////////////////////////////
void  CQualityOfService::SetIpValueTOS(const BYTE tos)
{
    if(tos == 0x0  ||  tos == 0x8 )
        m_bIpValueTOS = tos;
    else
        m_bIpValueTOS = 0x0;
}

/////////////////////////////////////////////////////////////////////////////
void  CQualityOfService::SetIpPrecedenceAudio(const BYTE aud)
{
    m_bIpPrecedenceAudio = (aud <= MAX_PREDECEDENCE_AUDIO
                            ?
                            aud : 0x0);
}

/////////////////////////////////////////////////////////////////////////////
void  CQualityOfService::SetIpPrecedenceVideo(const BYTE vid)
{
    m_bIpPrecedenceVideo = (vid <= MAX_PREDECEDENCE_VIDEO
                            ?
                            vid : 0x0);
}

/////////////////////////////////////////////////////////////////////////////
void  CQualityOfService::SetAtmStatus(const int status)
{
    if(status >= eQoS_disable  &&  status <= eQoS_unknown )
        m_eAtmStatus = (eStatusQoS)status;
    else
        m_eAtmStatus = eQoS_unknown;
}

/////////////////////////////////////////////////////////////////////////////
void  CQualityOfService::SetAtmPrecedenceAudio(const BYTE aud)
{
    if(aud >= H323_ATM_QOS_UBR  &&  aud <= H323_ATM_QOS_CBR )
        m_bAtmPrecedenceAudio = aud;
    else
        m_bAtmPrecedenceAudio = H323_ATM_QOS_UBR;
}

/////////////////////////////////////////////////////////////////////////////
void  CQualityOfService::SetAtmPrecedenceVideo(const BYTE vid)
{
    if(vid >= H323_ATM_QOS_UBR  &&  vid <= H323_ATM_QOS_CBR )
        m_bAtmPrecedenceVideo = vid;
    else
        m_bAtmPrecedenceVideo = H323_ATM_QOS_UBR;
}

/////////////////////////////////////////////////////////////////////////////
void  CQualityOfService::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pQOSNode = pFatherNode->AddChildNode("IP_QOS");

	pQOSNode->AddChildNode("QOS_ACTION", m_eIpStatus,QOS_ACTION_ENUM);
	pQOSNode->AddChildNode("QOS_DIFF_SERV", m_bIpIsDiffServ,QOS_DIFF_SERV_ENUM);
	pQOSNode->AddChildNode("QOS_IP_AUDIO", m_bIpPrecedenceAudio, _QOS_IP_AUDIO_RANGE_DECIMAL);
	pQOSNode->AddChildNode("QOS_IP_VIDEO", m_bIpPrecedenceVideo, _QOS_IP_VIDEO_RANGE_DECIMAL);
	pQOSNode->AddChildNode("QOS_TOS", m_bIpValueTOS,QOS_TOS_ENUM);
}
/////////////////////////////////////////////////////////////////////////////
int	  CQualityOfService::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	BYTE IPStatus = (BYTE)eQoS_unknown;
	GET_VALIDATE_CHILD(pActionNode,"QOS_ACTION", &IPStatus,QOS_ACTION_ENUM);
	m_eIpStatus = (CQualityOfService::eStatusQoS)IPStatus;

    GET_VALIDATE_CHILD(pActionNode,"QOS_DIFF_SERV", &m_bIpIsDiffServ,QOS_DIFF_SERV_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"QOS_IP_AUDIO", &m_bIpPrecedenceAudio, _QOS_IP_AUDIO_RANGE_DECIMAL);
	GET_VALIDATE_CHILD(pActionNode,"QOS_IP_VIDEO", &m_bIpPrecedenceVideo, _QOS_IP_VIDEO_RANGE_DECIMAL);
	GET_VALIDATE_CHILD(pActionNode,"QOS_TOS", &m_bIpValueTOS,QOS_TOS_ENUM);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CQualityOfService::TestValidity()
{
    // IP
    if(m_bIpValueTOS != 0x8  &&  m_bIpValueTOS != 0x0 ) // only two valid values - 0x0 and 0x8
        m_bIpValueTOS = 0x0;

    if(m_bIpPrecedenceAudio > MAX_PREDECEDENCE_AUDIO )
        m_bIpPrecedenceAudio = MAX_PREDECEDENCE_AUDIO;   // maximal valid value for audio priority

    if(m_bIpPrecedenceVideo > MAX_PREDECEDENCE_VIDEO )
        m_bIpPrecedenceVideo = MAX_PREDECEDENCE_VIDEO;   // maximal valid value for audio priority

    // ATM
    if(m_bAtmPrecedenceAudio > H323_ATM_QOS_CBR )
        m_bAtmPrecedenceAudio = H323_ATM_QOS_CBR;   // maximal valid value for audio priority

    if(m_bAtmPrecedenceVideo > H323_ATM_QOS_CBR )
        m_bAtmPrecedenceVideo = H323_ATM_QOS_CBR;   // maximal valid value for audio priority
}

/////////////////////////////////////////////////////////////////////////////
BYTE CQualityOfService::GetIpMediaQoS( const eMediaTypes media, const char* pServiceProviderName ) const
{
    BYTE  retVal = 0x0;
    BYTE  bMediaPrecedence = 0x0;

    switch( m_eIpStatus )
    {
    case eQoS_enable  : {
#ifdef __HIGHC__
        if(m_bIpIsDiffServ ) {
            // get media Qos from SYSTEM.CFG
            switch( media ) {
            case eMediaTypeAudio : {
             //   retVal = ::GetpSystemCfg()->GetDiffServAudioValue();
                break;
                                   }
            case eMediaTypeVideo : {
              //  retVal = ::GetpSystemCfg()->GetDiffServVideoValue();
                break;
                                   }
            case eMediaTypeRtcp  : {
              //  retVal = ::GetpSystemCfg()->GetDiffServRtcpValue();
                break;
                                   }
            case eMediaTypeUnknown :
            default : {
                break;
                      }
            }
            //retVal = (media == eMediaTypeRtcp) ? DIFFSERV_RTCP_VAL : DIFFSERV_MEDIA_VAL;
        } else {
            switch( media ) {
            case eMediaTypeAudio : {
                bMediaPrecedence = m_bIpPrecedenceAudio; break;
                                   }
            case eMediaTypeVideo : {
                bMediaPrecedence = m_bIpPrecedenceVideo; break;
                                   }
            case eMediaTypeRtcp  : {
                bMediaPrecedence = 0x7; break; // not configurable
                                   }
            case eMediaTypeUnknown :
            default : {
                bMediaPrecedence = 0x0; break;
                      }
            }
            // creating of QoS byte:
            // three major bits - media precedence,
            // next four bits - TOS value, other one - must be 0
            retVal = (BYTE)( ((bMediaPrecedence << 5) | (m_bIpValueTOS << 1)) );
        }
        break;
                        }
    case eQoS_service : {
        retVal = ( media == eMediaTypeRtcp ) ? 0xE0 : 0x0;


       // CIPService* pH323Serv = (CIPService*)::GetpIPservList()->GetFirstService();
        while( pH323Serv != NULL ) {

            // find service with name
            if(strcmp(pServiceProviderName,pH323Serv->GetName() ) == 0 ) {

                // service found, get Qos from service
                CQualityOfService* pServQos = pH323Serv->GetpQualityOfService();

                if(pServQos->m_eIpStatus != eQoS_enable )
                    retVal = 0x0;
                else if(pServQos->m_bIpIsDiffServ ) {
                    // get media Qos from SYSTEM.CFG
                    switch( media ) {
                    case eMediaTypeAudio : {
                        retVal = ::GetpSystemCfg()->GetDiffServAudioValue();
                        break;
                                           }
                    case eMediaTypeVideo : {
                        retVal = ::GetpSystemCfg()->GetDiffServVideoValue();
                        break;
                                           }
                    case eMediaTypeRtcp  : {
                        retVal = ::GetpSystemCfg()->GetDiffServRtcpValue();
                        break;
                                           }
                    case eMediaTypeUnknown :
                    default : {
                        break;
                              }
                    }
                    //retVal = (media == eMediaTypeRtcp) ? DIFFSERV_RTCP_VAL : DIFFSERV_MEDIA_VAL;
                } else {
                    // get media precedence from service
                    if(media == eMediaTypeAudio )
                        bMediaPrecedence = pServQos->m_bIpPrecedenceAudio;
                    else if(media == eMediaTypeVideo )
                        bMediaPrecedence = pServQos->m_bIpPrecedenceVideo;
                    else if(media == eMediaTypeRtcp )
                        bMediaPrecedence = 0x7;
                    // creating of QoS byte:
                    // three major bits - media precedence,
                    // next four bits - TOS value, other one - must be 0
                    retVal = (BYTE)( ((bMediaPrecedence << 5) | (pServQos->m_bIpValueTOS << 1)) );
                }
                break; // exit from while
            }
            // get next service
           // pH323Serv = ::GetpIPservList()->GetNextService();
        }
#endif
        break;
                        }
    case eQoS_disable :
    case eQoS_unknown :
    default           : {
        break;
                        }
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CQualityOfService::GetAtmMediaQoS( const eMediaTypes media, const char* pServiceProviderName ) const
{
    BYTE  retVal = H323_ATM_QOS_UBR;
    BYTE  bMediaPrecedence = 0x0;

    switch( m_eAtmStatus )
    {
    case eQoS_enable  : {
        switch( media ) {
        case eMediaTypeAudio : {
            retVal = m_bAtmPrecedenceAudio; break;
                               }
        case eMediaTypeVideo : {
            retVal = m_bAtmPrecedenceVideo; break;
                               }
        case eMediaTypeRtcp :
        case eMediaTypeUnknown :
        default : {
            retVal = H323_ATM_QOS_UBR; break;
                  }
        }
        break;
                        }

    case eQoS_disable :
    case eQoS_unknown :
    default           : {
        break;
                        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////
void CQualityOfService::SetQosIp(const BYTE status,
								 const BYTE isDiffService,
								 const BYTE tosValue,
                                 const BYTE audioPriority,
								 const BYTE videoPriority)
{
    SetIpStatus(status);
    SetIsDiffServ(isDiffService);
    SetIpValueTOS(tosValue);
    SetIpPrecedenceAudio(audioPriority);
    SetIpPrecedenceVideo(videoPriority);
}

////////////////////////////////////////////////////////////////////////////
void CQualityOfService::SetQosAtm(const BYTE status,
                                  const BYTE audioPriority,
								  const BYTE videoPriority)
{
    SetAtmStatus(status);
    SetAtmPrecedenceAudio(audioPriority);
    SetAtmPrecedenceVideo(videoPriority);
}

////////////////////////////////////////////////////////////////////////////
//#ifdef __HIGHC__
void CQualityOfService::GetAllQoS(const char* pServiceProviderName, CSegment& seg) const
{
	WORD audioQos = GetIpAudioQoS(pServiceProviderName);
	WORD videoQos = GetIpVideoQoS(pServiceProviderName);
	WORD rtcpQos = GetIpVideoQoS(pServiceProviderName);
	WORD atmAudioQos = GetAtmAudioQoS(pServiceProviderName);
	WORD atmVideoQos = GetAtmVideoQoS(pServiceProviderName);

    seg << audioQos
        << videoQos
        << rtcpQos
        << atmAudioQos
        << atmVideoQos;
}
//#endif




/////////////////////////////////////////////////////////////////////////////////////////////////
//						   CIPService                                                     //
////////////////////////////////////////////////////////////////////////////////////////////////
CIPService::CIPService()
    :m_GatekeeperName(),
	 m_AltGatekeeperName()
{
    SetDefaults();

	static int cnt = 0;
	cnt++;
	m_Id = cnt;
	sprintf(m_serviceName, "CUCU_LULU - %d", m_Id);
	TRACESTR(eLevelDebug) << "CIPService::CIPService -id:"<< m_Id;
}

/////////////////////////////////////////////////////////////////////////////
CIPService::CIPService(const CIPService &other):
CSerializeObject(other),
m_GatekeeperName(other.m_GatekeeperName),
m_AltGatekeeperName(other.m_AltGatekeeperName)
{
    SetDefaults();
    *this = other;
    return;
}

/////////////////////////////////////////////////////////////////////////////
CIPService::~CIPService()
{
	PDELETE(m_DynIPSProperties);
    int i;
    for ( i=0;i<MAX_ROUTERS_IN_H323_SERVICE;i++)
        PDELETE(m_pRouter[i]);

    for ( i=0; i<MAX_SPAN_NUMBER_IN_SERVICE; i++ )
        PDELETE( m_pSpan[i] );



    POBJDELETE (m_pQualityOfService);
    POBJDELETE (m_pServiceConfig);
    //sip

    POBJDELETE(m_pDns);
    POBJDELETE(m_pSecurity);
    POBJDELETE(m_pSip);

    POBJDELETE(m_pSipAdvanced);
    POBJDELETE(m_pManagementSecurity);
    POBJDELETE(m_pWhiteList);
}

////////////////////////////////////////////////////////////////////////////
CIPService&  CIPService::operator=(const CIPService& other)
{
	if(this == &other)
	{
		return *this;
	}
    CSerializeObject::operator=(other);

    m_Id = other.m_Id;

    WORD i;
    strncpy(m_serviceName, other.m_serviceName, NET_SERVICE_PROVIDER_NAME_LEN);
    m_serviceTypeName		= other.m_serviceTypeName;
    m_netIPaddress			= other.m_netIPaddress;
    m_netMask				= other.m_netMask;

    memcpy( &(m_defaultGatewayIPv4), &(other.m_defaultGatewayIPv4), sizeof(ipAddressV4If) );
    memcpy( &(m_defaultGatewayIPv6), &(other.m_defaultGatewayIPv6), sizeof(ipAddressV6If) );
    m_defaultGatewayMaskIPv6 = other.m_defaultGatewayMaskIPv6;

    m_numb_of_routers		 = other.m_numb_of_routers;
    for( i=0; i<MAX_ROUTERS_IN_H323_SERVICE; i++ )
    {
    	POBJDELETE( m_pRouter[i] );

        if(other.m_pRouter[i] == NULL)
        	m_pRouter[i] = NULL;
        else
            m_pRouter[i]= new CH323Router( *other.m_pRouter[i] );
    }

    m_DHCPServer			  = other.m_DHCPServer;
    m_gatekeeper			  = other.m_gatekeeper;
//    m_externalGatekeeperAddr  = other.m_externalGatekeeperAddr;
    m_GatekeeperDiskoveryPort = other.m_GatekeeperDiskoveryPort;

    m_numb_of_span			= other.m_numb_of_span;
    for(i=0; i<MAX_SPAN_NUMBER_IN_SERVICE; i++ )
        PDELETE( m_pSpan[i] );
    for( i=0; i<MAX_SPAN_NUMBER_IN_SERVICE; i++ )
    {
        if(other.m_pSpan[i] == NULL )
            m_pSpan[i] = NULL;
        else
            m_pSpan[i] = new CIPSpan( *other.m_pSpan[i] );
    }

    m_dialIn_prefix.m_aliasType = other.m_dialIn_prefix.m_aliasType;
    strncpy(m_dialIn_prefix.m_aliasName, other.m_dialIn_prefix.m_aliasName, ALIAS_NAME_LEN);

    m_ind_span          = 0;//other.m_indSpan;
    m_ind_router        = 0;//other.m_indRouter;

    m_ATMEnableAAL5    = other.m_ATMEnableAAL5 ;
    m_qualityOfService = other.m_qualityOfService; // for compatibility with previous versions

    m_useDefaultARPServerIPOA = other.m_useDefaultARPServerIPOA;


    m_useDiscoveredLECS = other.m_useDiscoveredLECS;


    m_useLECSForLES = other.m_useLECSForLES;

    m_useAutoElanName = other.m_useAutoElanName ;
    strncpy(m_elanName, other.m_elanName, ELAN_NAME_SIZE);

    m_forwarding= other.m_forwarding;
    m_gatekeeperMode= other.m_gatekeeperMode;
    m_IsRRQPolling= other.m_IsRRQPolling;
    m_RRQPollingInterval= other.m_RRQPollingInterval;
    m_pseudoGKListenPort = other.m_pseudoGKListenPort;

    *m_DynIPSProperties = *other.m_DynIPSProperties;

    //sip
    m_eProtocolType     = other.m_eProtocolType;
    m_GatekeeperName    = other.m_GatekeeperName;
    m_AltGatekeeperName = other.m_AltGatekeeperName;


	*m_pDns      = *other.m_pDns;
    *m_pSecurity = *other.m_pSecurity;
    *m_pSip      = *other.m_pSip;
    *m_pQualityOfService = *other.m_pQualityOfService;
    *m_pServiceConfig = *other.m_pServiceConfig;
	m_VpnIp      = other.m_VpnIp;
    m_AutoRegisterSpanHostName = other.m_AutoRegisterSpanHostName;

	m_ind_span = 0;
	m_ind_router = 0;

	m_updateCounter		= other.m_updateCounter;
	m_bChanged			= other.m_bChanged;

	m_eIpServiceType    = other.m_eIpServiceType;

    m_IsRegAsGW 		= other.m_IsRegAsGW;

	m_bSecured			= other.m_bSecured;
    m_Vlan = other.m_Vlan;

    for(i = 0 ; i < MAX_NUM_OF_PORTS_SPEED ; i++)
    {
        m_PortSpeedVector[i] = other.m_PortSpeedVector[i];
    }

    *m_pManagementSecurity = *other.m_pManagementSecurity;

    m_ipType			= other.m_ipType;
	m_ipv6ConfigType	= other.m_ipv6ConfigType;

	m_bPermanentNetwork  = other.m_bPermanentNetwork;

	*m_pSipAdvanced		= *other.m_pSipAdvanced;

	m_MaxNumOfCalls = other.m_MaxNumOfCalls;

	//m_pServiceConfig = other.m_pServiceConfig;

	m_isV35GwEnabled	= other.m_isV35GwEnabled;
	m_V35GwIpAddress	= other.m_V35GwIpAddress;
	m_V35GwUsername		= other.m_V35GwUsername;
	m_V35GwPassword_dec	= other.m_V35GwPassword_dec;
	m_V35GwPassword_enc	= other.m_V35GwPassword_enc;
	m_V35GwPort         = other.m_V35GwPort;
	m_V35GwAlias        = other.m_V35GwAlias;
	*m_pWhiteList   		= *other.m_pWhiteList;
    return *this;
}
////////////////////////////////////////////////////////
bool CIPService::compareManagment(const CIPService &rHnd) const
{
	if(this == &rHnd)
		return true;
	TRACESTR(eLevelInfoNormal) << "CIPService::compareManagment - check IP";

//IP
	TRACESTR(eLevelInfoNormal) << "CIPService::compareManagment - check service name";
	if( strcmp(m_serviceName, rHnd.m_serviceName) != 0 )
		return false;
	TRACESTR(eLevelInfoNormal) << "CIPService::compareManagment - check service Mask";
	if ( m_netMask != rHnd.m_netMask )
		return false;
	TRACESTR(eLevelInfoNormal) << "CIPService::compareManagment - check service Ip Type";
 	if ( m_ipType != rHnd.m_ipType )
 		return false;
 	TRACESTR(eLevelInfoNormal) << "CIPService::compareManagment - check service IpV6 type config";
 	if ( m_ipv6ConfigType != rHnd.m_ipv6ConfigType )
 		return false;


	 int i=0;
	 TRACESTR(eLevelInfoNormal) << "CIPService::compareManagment - check service spans";
	 //check only cntrl and shelf ip's
	 int cntrlAndShelfRange=2;
    for(i=0; i<cntrlAndShelfRange; i++ )
    {
    	if ( m_pSpan[i]->GetIPv4Address() != rHnd.m_pSpan[i]->GetIPv4Address() )
    		return false;
    	if(m_pSpan[i]->GetSpanHostName() != rHnd.m_pSpan[i]->GetSpanHostName())
    		return false;
    	for(int j=0;j < NUM_OF_IPV6_ADDRESSES;j++)
    	{
    		if ( m_pSpan[i]->GetIPv6Address(j) != rHnd.m_pSpan[i]->GetIPv6Address(j) )
    		    		return false;
    	}
    }
    TRACESTR(eLevelInfoNormal) << "CIPService::compareManagment - check DNS";
// DNS
	if ( *m_pDns != *(rHnd.m_pDns) )
		return false;
	TRACESTR(eLevelInfoNormal) << "CIPService::compareManagment - check Routers";
//Routers
	if (memcmp (&(m_defaultGatewayIPv4), &(rHnd.m_defaultGatewayIPv4), sizeof(ipAddressV4If)) != 0)
			return false;

	if (memcmp (&(m_defaultGatewayIPv6), &(rHnd.m_defaultGatewayIPv6), sizeof(ipAddressV6If)) != 0)
			return false;

	if ( m_defaultGatewayMaskIPv6 != rHnd.m_defaultGatewayMaskIPv6 )
			return false;
	for (i=0; i<m_numb_of_routers; i++)
		{
			if (*m_pRouter[i] != *(rHnd.m_pRouter[i]) )
				return false;
		}
	TRACESTR(eLevelInfoNormal) << "CIPService::compareManagment - check Lan Ports";
//LAN Ports
	for(i=0 ; i < MAX_NUM_OF_PORTS_SPEED ; i++)
	{
		if ( m_PortSpeedVector[i] != rHnd.m_PortSpeedVector[i] )
			return false;
	}
	TRACESTR(eLevelInfoNormal) << "CIPService::compareManagment - check Security";
//Security
	if ( m_bSecured != rHnd.m_bSecured )
		return false;

 	if ( *m_pManagementSecurity != *(rHnd.m_pManagementSecurity) )
 			return false;
//WhiteList
 	TRACESTR(eLevelInfoNormal) << "CIPService::compareManagment - check WhiteList";
 	if ( *m_pWhiteList != *(rHnd.m_pWhiteList) )
 	 			return false;


 	return true;

}
////////////////////////////////////////////////////////////////////////////
bool CIPService::operator==(const CIPService &rHnd)const
{
	int i=0;

	if(this == &rHnd)
		return true;

	if( strcmp(m_serviceName, rHnd.m_serviceName) != 0 )
		return false;

	if ( m_netIPaddress != rHnd.m_netIPaddress || m_netMask != rHnd.m_netMask )
		return false;

	if (memcmp (&(m_defaultGatewayIPv4), &(rHnd.m_defaultGatewayIPv4), sizeof(ipAddressV4If)) != 0)
		return false;

	if (memcmp (&(m_defaultGatewayIPv6), &(rHnd.m_defaultGatewayIPv6), sizeof(ipAddressV6If)) != 0)
		return false;

	if ( m_defaultGatewayMaskIPv6 != rHnd.m_defaultGatewayMaskIPv6 )
		return false;

	if ( m_numb_of_routers != rHnd.m_numb_of_routers )
		return false;

    if ( m_DHCPServer != rHnd.m_DHCPServer )
		return false;

    if ( m_gatekeeper != rHnd.m_gatekeeper )
		return false;

    if ( m_GatekeeperDiskoveryPort != rHnd.m_GatekeeperDiskoveryPort )
		return false;

    if ( m_dialIn_prefix != rHnd.m_dialIn_prefix )
		return false;

	if( m_serviceTypeName != rHnd.m_serviceTypeName )
		return false;

    if ( m_ATMEnableAAL5 != rHnd.m_ATMEnableAAL5 )
		return false;

    if ( m_qualityOfService != rHnd.m_qualityOfService )
		return false;

    if ( m_useDefaultARPServerIPOA != rHnd.m_useDefaultARPServerIPOA )
		return false;

    if ( m_useDiscoveredLECS != rHnd.m_useDiscoveredLECS )
		return false;

    if ( m_useLECSForLES != rHnd.m_useLECSForLES )
		return false;

    if ( m_useAutoElanName != rHnd.m_useAutoElanName )
		return false;

    if( strcmp(m_elanName, rHnd.m_elanName) != 0 )
		return false;

    if ( m_forwarding != rHnd.m_forwarding )
		return false;

    if ( m_gatekeeperMode != rHnd.m_gatekeeperMode )
		return false;

    if ( m_IsRRQPolling != rHnd.m_IsRRQPolling )
		return false;

    if ( m_RRQPollingInterval != rHnd.m_RRQPollingInterval )
		return false;

    if ( m_pseudoGKListenPort != rHnd.m_pseudoGKListenPort )
		return false;

    if ( m_eProtocolType != rHnd.m_eProtocolType )
		return false;

    if ( m_GatekeeperName != rHnd.m_GatekeeperName )
		return false;

	if ( m_AltGatekeeperName != rHnd.m_AltGatekeeperName )
		return false;

	if ( m_VpnIp != rHnd.m_VpnIp )
		return false;

	if ( m_AutoRegisterSpanHostName != rHnd.m_AutoRegisterSpanHostName )
		return false;

	if ( m_Id != rHnd.m_Id )
		return false;

	if ( m_IsRegAsGW != rHnd.m_IsRegAsGW )
		return false;

	if ( m_eIpServiceType != rHnd.m_eIpServiceType )
		return false;

	if ( m_Vlan != rHnd.m_Vlan )
		return false;

	if ( *m_pDns != *(rHnd.m_pDns) )
		return false;
	TRACESTR(eLevelInfoNormal)<< "BRIDGE-8284 check security ";

	if ( *m_pSecurity != *(rHnd.m_pSecurity) )
		return false;

	TRACESTR(eLevelInfoNormal)<< "BRIDGE-8284 check SIP ";
	if ( *m_pSip != *(rHnd.m_pSip) )
		return false;

	TRACESTR(eLevelInfoNormal)<< "BRIDGE-8284 check m_DynIPSProperties ";

	if ( *m_DynIPSProperties != *(rHnd.m_DynIPSProperties) )
		return false;
	TRACESTR(eLevelInfoNormal)<< "BRIDGE-8284 check routers ";
	for (i=0; i<m_numb_of_routers; i++)
	{
		if (*m_pRouter[i] != *(rHnd.m_pRouter[i]) )
			return false;
	}

	TRACESTR(eLevelInfoNormal)<< "BRIDGE-8284 check number of spans ";
    if ( m_numb_of_span != rHnd.m_numb_of_span )
		return false;
    TRACESTR(eLevelInfoNormal)<< "BRIDGE-8284 check  spans it self ";
    for(i=0; i<m_numb_of_span; i++ )
    {
    	if ( *(m_pSpan[i]) != *(rHnd.m_pSpan[i]) )
    		return false;
    }
    TRACESTR(eLevelInfoNormal)<< "BRIDGE-8284 check  Quality of service";
 	if ( *m_pQualityOfService != *(rHnd.m_pQualityOfService) )
		return false;
 	TRACESTR(eLevelInfoNormal)<< "BRIDGE-8284 check  port speed vector";
	for(i=0 ; i < MAX_NUM_OF_PORTS_SPEED ; i++)
	{
		if ( m_PortSpeedVector[i] != rHnd.m_PortSpeedVector[i] )
			return false;
	}

	TRACESTR(eLevelInfoNormal)<< "BRIDGE-8284 check  Managment security";
	if ( *m_pManagementSecurity != *(rHnd.m_pManagementSecurity) )
			return false;


	if ( m_ipType != rHnd.m_ipType )
		return false;

	if ( m_ipv6ConfigType != rHnd.m_ipv6ConfigType )
		return false;

	if ( m_bSecured != rHnd.m_bSecured )
		return false;

	if ( m_bPermanentNetwork != rHnd.m_bPermanentNetwork )
		return false;

	if ( m_isV35GwEnabled != rHnd.m_isV35GwEnabled )
		return false;

	if ( m_V35GwIpAddress != rHnd.m_V35GwIpAddress )
		return false;

	if ( m_V35GwUsername != rHnd.m_V35GwUsername )
		return false;

	if ( m_V35GwPassword_dec != rHnd.m_V35GwPassword_dec )
		return false;

	if ( m_V35GwPassword_enc != rHnd.m_V35GwPassword_enc )
		return false;

	if ( m_MaxNumOfCalls != rHnd.m_MaxNumOfCalls)
		return false;
    if(m_V35GwPort != rHnd.m_V35GwPort)
    	return false;
    if(m_V35GwAlias != rHnd.m_V35GwAlias)
    	TRACESTR(eLevelInfoNormal)<< "BRIDGE-8284 check  whitelist";
	if ( *m_pWhiteList != *(rHnd.m_pWhiteList) )
	 	 			return false;
	return true;
}
////////////////////////////////////////////////////////////////////////////
bool CIPService::operator!=(const CIPService& other)
{
	return (!(*this == other));
}
////////////////////////////////////////////////////////////////////////////
void CIPService::SetDefaults()
{
    m_serviceTypeName   = SERVICE_H323_LAN;
    m_netIPaddress      = 0;
    m_netMask           = 0;

    m_defaultGatewayIPv4.ip = 0;
	memset(m_defaultGatewayIPv6.ip, 0, IPV6_ADDRESS_BYTES_LEN);
	m_defaultGatewayIPv6.scopeId = eScopeIdOther;
	m_defaultGatewayMaskIPv6 = 64;

    // 03/08/06: MAX_ROUTERS_IN_H323_SERVICE are created on default
    m_numb_of_routers   = MAX_ROUTERS_IN_H323_SERVICE;

	WORD i;
    for(i=0; i<MAX_ROUTERS_IN_H323_SERVICE; i++ )
    {
    	//06.08.06: ipService will contain 5 routers by default
    	//m_pRouter[i] = NULL;
        m_pRouter[i] = new CH323Router();
    }

    m_DHCPServer			  = FALSE;
    m_gatekeeper			  = GATEKEEPER_EXTERNAL;
//    m_externalGatekeeperAddr  = 0;
    m_GatekeeperDiskoveryPort = 1719;

    m_numb_of_span			  = 0;
    for(i=0; i<MAX_SPAN_NUMBER_IN_SERVICE; i++ )
        m_pSpan[i] = NULL;

    m_dialIn_prefix.m_aliasType = PARTY_H323_ALIAS_E164_TYPE;
    m_dialIn_prefix.m_aliasName[0] = '\0';

    m_ATMEnableAAL5    = ATM_AAL5_DISABLE ;
    m_qualityOfService = H323_ATM_QOS_UBR; // only for compatibility with previous versions

    m_useDefaultARPServerIPOA = USE_DEFAULT_ARP_SERVER;
    m_useDiscoveredLECS       = USE_DISCOVERED_LECS;
     m_useLECSForLES           = USE_LECS_FOR_LES;

    m_useAutoElanName = USE_AUTO_ELAN_NAME ;
    m_elanName[0] ='\0';

    m_DynIPSProperties = new CDynIPSProperties;

    m_ind_span          = 0;
    m_ind_router        = 0;
    m_pQualityOfService = new CQualityOfService;
    m_pServiceConfig    = new CServiceConfig;

    m_forwarding         = TRUE;
    m_gatekeeperMode     = GK_MODE_BOARD_HUNTING;
    m_IsRRQPolling       = FALSE;
    m_RRQPollingInterval = 120;
    m_pseudoGKListenPort = 1719;

    //sip parameters
    m_eProtocolType = eIPProtocolType_H323;

    m_pDns          = new CIpDns;
    m_pSecurity     = new CIPSecurity;
    m_pSip          = new CSip;

	m_VpnIp         = 0;
    m_AutoRegisterSpanHostName = FALSE;

	m_updateCounter		= 0;
	m_bChanged = FALSE;

	m_eIpServiceType = eIpServiceType_Signaling;

	m_IsRegAsGW 	= FALSE;
	m_bSecured		= FALSE;

    for(int i = 0 ; i < MAX_NUM_OF_PORTS_SPEED ; i++)
    {
        m_PortSpeedVector[i].SetNum(i);
        m_PortSpeedVector[i].SetSpeed(ePortSpeed_Auto);
    }

    m_pManagementSecurity = new CManagementSecurity;

    m_ipType			= eIpType_IpV4;
	m_ipv6ConfigType	= eV6Configuration_Auto;

	m_bPermanentNetwork = TRUE;

	m_pSipAdvanced = new CSipAdvanced();

	m_isV35GwEnabled	= FALSE;
	m_V35GwIpAddress	= 0;
	m_V35GwUsername		= "";
	m_V35GwPassword_dec	= "";
	m_V35GwPassword_enc	= "";
	m_MaxNumOfCalls = 0;
	m_V35GwPort     ="";
	m_V35GwAlias    ="";
	m_pWhiteList = new CWhiteList();
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SerializeXml(CXMLDOMElement *&pFatherNode, DWORD objToken, bool isToEMA/*=false*/) const
{

	TRACESTR(eLevelInfoNormal) << "CIPService::SerializeXml";

    CXMLDOMElement* pH323SrvNode = NULL;

    if(NULL == pFatherNode)
    {
        pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("IP_SERVICE_LIST");
        pH323SrvNode = pFatherNode;
    }
    else
    {
        pH323SrvNode = pFatherNode->AddChildNode("IP_SERVICE");
	}

	BYTE bChanged = InsertUpdateCntChanged(pH323SrvNode, objToken);

	if (!bChanged)
	{
		return;
	}

	pH323SrvNode->AddChildNode("NAME", m_serviceName);

	pH323SrvNode->AddChildNode("IP", m_netIPaddress,IP_ADDRESS);
	pH323SrvNode->AddChildNode("MASK", m_netMask,IP_ADDRESS);
	pH323SrvNode->AddChildNode("DEFAULT_ROUTER", m_defaultGatewayIPv4.ip, IP_ADDRESS);

	CXMLDOMElement* pRouterNode = pH323SrvNode->AddChildNode("ROUTER_LIST");
	for (int i = 0; i < m_numb_of_routers; i++)
	{
		m_pRouter[i]->SerializeXml(pRouterNode);
	}

	pH323SrvNode->AddChildNode("DHCP_SERVER", m_DHCPServer,_BOOL);
	pH323SrvNode->AddChildNode("GATEKEEPER_TYPE", m_gatekeeper, GATEKEEPER_ENUM);

	CXMLDOMElement* pGateKeeperNode = pH323SrvNode->AddChildNode("GATEKEEPER");
//	pGateKeeperNode->AddChildNode("EXTERNAL_GATEKEEPER_ADDRESS", m_externalGatekeeperAddr,IP_ADDRESS);
	pGateKeeperNode->AddChildNode("GATEKEEPER_DISCOVERY_PORT", m_GatekeeperDiskoveryPort);
	pGateKeeperNode->AddChildNode("GATE_KEEPER_MODE", m_gatekeeperMode, GATE_KEEPER_MODE_ENUM);
	m_dialIn_prefix.SerializeXml(pGateKeeperNode);
	pGateKeeperNode->AddChildNode("RRQ_POLLING", m_IsRRQPolling, _BOOL);
	pGateKeeperNode->AddChildNode("RRQ_POLLING_INTERVAL", m_RRQPollingInterval);

	CXMLDOMElement* pSpanNode = pH323SrvNode->AddChildNode("IP_SPAN_LIST");
	for (int i = 0; i < m_numb_of_span; i++)
	{
		m_pSpan[i]->SerializeXml(pSpanNode);
	}
	if (m_pQualityOfService)
		m_pQualityOfService->SerializeXml(pH323SrvNode);

	pH323SrvNode->AddChildNode("FORWARDING", m_forwarding, _BOOL);
	pH323SrvNode->AddChildNode("PROTOCOL_TYPE", m_eProtocolType, PROTOCOL_TYPE_ENUM);
	if (m_pDns)
		m_pDns->SerializeXml(pH323SrvNode);
	if (m_pSecurity)
		m_pSecurity->SerializeXml(pH323SrvNode,isToEMA);
	if (m_pSip)
		m_pSip->SerializeXml(pH323SrvNode);
	pH323SrvNode->AddChildNode("GATEKEEPER_NAME",m_GatekeeperName.GetString());
	pH323SrvNode->AddChildNode("ALT_GATEKEEPER_NAME",m_AltGatekeeperName.GetString());
	pH323SrvNode->AddChildNode("VPN_IP", m_VpnIp,IP_ADDRESS);
	pH323SrvNode->AddChildNode("AUTO_REGISTRATION_SPAN_HOST_NAME", m_AutoRegisterSpanHostName, _BOOL);
	pH323SrvNode->AddChildNode("IP_SERVICE_TYPE", m_eIpServiceType, IP_SERVICE_TYPE_ENUM);

	pH323SrvNode->AddChildNode("IS_REGISTER_AS_GATEWAY"	, m_IsRegAsGW		, _BOOL);

	m_Vlan.SerializeXml(pH323SrvNode);

    CXMLDOMElement* pPortSpeedListNode = pH323SrvNode->AddChildNode("PORT_SPEED_LIST");
    for(int i = 0 ; i < MAX_NUM_OF_PORTS_SPEED ; i++)
    {
        m_PortSpeedVector[i].SerializeXml(pPortSpeedListNode);
    }

	pH323SrvNode->AddChildNode("IS_SECURED", m_bSecured, _BOOL);
	pH323SrvNode->AddChildNode("PERMANENT_NETWORK", m_bPermanentNetwork, _BOOL);

    pH323SrvNode->AddChildNode("IP_TYPE", m_ipType, IP_TYPE_ENUM);
    pH323SrvNode->AddChildNode("IP_V6_CONFIGURATION_TYPE",	m_ipv6ConfigType, IP_V6_CONFIG_TYPE_ENUM);

	char v6Mask[IPV6_ADDRESS_LEN];
	memset(v6Mask, 0, IPV6_ADDRESS_LEN);
	GetDefaultGatewayMaskIPv6Str(v6Mask);
	pH323SrvNode->AddIPv6ChildNode("DEFAULT_ROUTER_IP_V6", m_defaultGatewayIPv6.ip, v6Mask);

	if (m_pSipAdvanced)
		m_pSipAdvanced->SerializeXml(pH323SrvNode);

    pH323SrvNode->AddChildNode("SERVICE_ID", m_Id); // TODO: ???

    if(m_pServiceConfig)
    {
    	DWORD paramsnum=m_pServiceConfig->GetNumOfParams();
    	TRACESTR(eLevelInfoNormal) << "CIPService::SerializeXml -number of params" << paramsnum << "service name:" << m_serviceName << "id:" << m_Id;


    	m_pServiceConfig->SetIsServiceCFG(true);

    	pH323SrvNode->AddChildNode("CFG_TYPE", "user");
    	m_pServiceConfig->SerializeXml(pH323SrvNode);
    }

	if (m_pManagementSecurity)
		m_pManagementSecurity->SerializeXml(pH323SrvNode);

	CXMLDOMElement* pV35GwNode = pH323SrvNode->AddChildNode("V35_GATEWAY");

	pV35GwNode->AddChildNode("V35_GATEWAY_ENABLED",		m_isV35GwEnabled,	_BOOL);
	pV35GwNode->AddChildNode("V35_GATEWAY_IP",			m_V35GwIpAddress,	IP_ADDRESS);
	pV35GwNode->AddChildNode("V35_GATEWAY_USER_NAME",	m_V35GwUsername);
	pV35GwNode->AddChildNode("V35_GATEWAY_PORT",	m_V35GwPort);
	pV35GwNode->AddChildNode("V35_GATEWAY_ALIAS",	m_V35GwAlias);
	if (true == isToEMA)
	{
		pV35GwNode->AddChildNode("V35_GATEWAY_PASSWORD", m_V35GwPassword_dec);

//		TRACESTR(eLevelInfoNormal) << "\nCIPService::SerializeXml"
//							   << "\nPassword: " << m_V35GwPassword_dec;
	}
	else // to file
	{
		// not writing the enc passsword, since it may contain characters that harm XML parsing
		pV35GwNode->AddChildNode("V35_GATEWAY_PASSWORD", ""/*m_V35GwPassword_enc*/);
	}
	if(m_pWhiteList)
		m_pWhiteList->SerializeXml(pH323SrvNode);
}

/////////////////////////////////////////////////////////////////////////////
void   CIPService::SerializeXml(CXMLDOMElement *&pFatherNode) const
{
	SerializeXml(pFatherNode, UPDATE_CNT_BEGIN_END);
}

/////////////////////////////////////////////////////////////////////////////
int CIPService::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	TRACESTR(eLevelInfoNormal) << "CIPService::DeSerializeXml";
	int nStatus = STATUS_OK;
	char* ParentNodeName;
	CXMLDOMElement *pIpServiceNode;
	CXMLDOMElement *pTempNode=NULL;


	pActionNode->get_nodeName(&ParentNodeName);
	if(!strcmp(ParentNodeName, "IP_SERVICE"))
 		pIpServiceNode=pActionNode;
	else
		GET_MANDATORY_CHILD_NODE(pActionNode, "IP_SERVICE", pIpServiceNode);


	m_bChanged=TRUE;

	GET_VALIDATE_CHILD(pIpServiceNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
//	GET_VALIDATE_CHILD(pIpServiceNode,"CHANGED",&m_bChanged,_BOOL);

//	if (m_bChanged)
//	{
	GET_VALIDATE_CHILD(pIpServiceNode,"NAME",m_serviceName,NET_SERVICE_PROVIDER_NAME_LENGTH);
	GET_VALIDATE_CHILD(pIpServiceNode,"IP",&m_netIPaddress,IP_ADDRESS);
		GET_VALIDATE_CHILD(pIpServiceNode,"MASK",&m_netMask,IP_ADDRESS);
		GET_VALIDATE_CHILD(pIpServiceNode,"DEFAULT_ROUTER",&m_defaultGatewayIPv4.ip,IP_ADDRESS);

	CXMLDOMElement *pRouterListNode;

	GET_CHILD_NODE(pIpServiceNode, "ROUTER_LIST", pRouterListNode);

	if (pRouterListNode)
	{

		GET_FIRST_CHILD_NODE(pRouterListNode,"ROUTER",pTempNode);
		m_numb_of_routers = 0;
		while (pTempNode && m_numb_of_routers < MAX_ROUTERS_IN_H323_SERVICE)
		{
			CH323Router* pH323Router = new CH323Router;
			nStatus = pH323Router->DeSerializeXml(pTempNode, pszError);

			if (nStatus != STATUS_OK)
			{
				POBJDELETE(pH323Router);
				return nStatus;
			}
			POBJDELETE(m_pRouter[m_numb_of_routers]);
			m_pRouter[m_numb_of_routers] = pH323Router;
			m_numb_of_routers++;
			GET_NEXT_CHILD_NODE(pRouterListNode,"ROUTER",pTempNode);
		}
	}

	GET_VALIDATE_CHILD(pIpServiceNode,"DHCP_SERVER",&m_DHCPServer,_BOOL);
	GET_VALIDATE_CHILD(pIpServiceNode,"GATEKEEPER_TYPE",&m_gatekeeper,GATEKEEPER_ENUM);



	CXMLDOMElement *pGateKeeperNode;

	GET_CHILD_NODE(pIpServiceNode, "GATEKEEPER", pGateKeeperNode);

	if (pGateKeeperNode)
	{
//		GET_VALIDATE_CHILD(pGateKeeperNode,"EXTERNAL_GATEKEEPER_ADDRESS",&m_externalGatekeeperAddr,IP_ADDRESS);
		GET_VALIDATE_CHILD(pGateKeeperNode,"GATEKEEPER_DISCOVERY_PORT",&m_GatekeeperDiskoveryPort,_0_TO_WORD);	//ask danny
		GET_VALIDATE_CHILD(pGateKeeperNode,"GATE_KEEPER_MODE",&m_gatekeeperMode,GATE_KEEPER_MODE_ENUM);
		GET_CHILD_NODE(pGateKeeperNode, "ALIAS", pTempNode);

		if (pTempNode)
		{
			nStatus = m_dialIn_prefix.DeSerializeXml(pTempNode, pszError);
			if (nStatus!=STATUS_OK)
				return nStatus;
		}

		GET_VALIDATE_CHILD(pGateKeeperNode,"RRQ_POLLING",&m_IsRRQPolling,_BOOL);
		GET_VALIDATE_CHILD(pGateKeeperNode,"RRQ_POLLING_INTERVAL",&m_RRQPollingInterval,_0_TO_WORD);
	}


	CXMLDOMElement *pSpanListNode;

	GET_CHILD_NODE(pIpServiceNode, "IP_SPAN_LIST", pSpanListNode);

	if (pSpanListNode)
	{
		GET_FIRST_CHILD_NODE(pSpanListNode,"IP_SPAN",pTempNode);

		/**************************************************************
		* 21.3.10 VNGFE 2622 added by Rachel Cohen
		* The problem is That Ema sending 10 IP_spans instead of
		* 9 (one for cs and 8 for media cards each media has 2 ip_span)
		* we decided to take just the first 9 ip_spans and drop the
		* rest .and send asset in case we have more than 9.
		**************************************************************/

		m_numb_of_span = 0;
		while (pTempNode && m_numb_of_span < MAX_IP_SPAN) //MAX_SPAN_NUMBER_IN_SERVICE
		{
			CIPSpan* pH323Span = new CIPSpan;
			nStatus = pH323Span->DeSerializeXml(pTempNode, pszError);

			if (nStatus != STATUS_OK)
			{
				POBJDELETE(pH323Span);
				return nStatus;
			}
			POBJDELETE(m_pSpan[m_numb_of_span]);
			m_pSpan[m_numb_of_span] = pH323Span;
			m_numb_of_span++;
			GET_NEXT_CHILD_NODE(pSpanListNode,"IP_SPAN",pTempNode);

		}
		PASSERTMSG(pTempNode && m_numb_of_span == MAX_IP_SPAN, "Ip service list includes too many ip spans");
	}

	GET_CHILD_NODE(pIpServiceNode, "IP_QOS", pTempNode);

	if (pTempNode)
	{
		CQualityOfService* pQualityOfService = new CQualityOfService;
		nStatus = pQualityOfService->DeSerializeXml(pTempNode, pszError);
		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pQualityOfService);
			return nStatus;
		}

		POBJDELETE(m_pQualityOfService);
		m_pQualityOfService = pQualityOfService;
	}

	GET_VALIDATE_CHILD(pIpServiceNode,"FORWARDING",&m_forwarding,_BOOL);

	BYTE tmp = (BYTE)eIPProtocolType_None;
	GET_VALIDATE_CHILD(pIpServiceNode,"PROTOCOL_TYPE", &tmp,PROTOCOL_TYPE_ENUM);
	m_eProtocolType = eIPProtocolType(tmp);

	GET_CHILD_NODE(pIpServiceNode, "DNS", pTempNode);
	if (pTempNode)
	{
		CIpDns* pDNS = new CIpDns;
		nStatus = pDNS->DeSerializeXml(pTempNode, pszError);

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pDNS);
			return nStatus;
		}
		POBJDELETE(m_pDns);
		m_pDns = pDNS;
	}

	GET_CHILD_NODE(pIpServiceNode, "SECURITY", pTempNode);
	if (pTempNode)
	{
		CIPSecurity* pSecurity = new CIPSecurity;
		nStatus = pSecurity->DeSerializeXml(pTempNode, pszError);

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pSecurity);
			return nStatus;
		}
		POBJDELETE(m_pSecurity);
		m_pSecurity = pSecurity;
	}

	GET_CHILD_NODE(pIpServiceNode, "SIP", pTempNode);
	if (pTempNode)
	{
		CSip* pSip = new CSip;
		nStatus = pSip->DeSerializeXml(pTempNode, pszError);

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pSip);
			return nStatus;
		}
		POBJDELETE(m_pSip);
		m_pSip = pSip;
	}

	if (IsTarget())
	{
		GET_VALIDATE_ASCII_CHILD(pIpServiceNode, "GATEKEEPER_NAME", m_GatekeeperName, _0_TO_MAX_GK_NAME_ENUM);
	}
	else // for nightTest Scripts/TestUnicodeCutUtf8Strings.py
	{
		GET_VALIDATE_CHILD(pIpServiceNode, "GATEKEEPER_NAME", m_GatekeeperName, _0_TO_MAX_GK_NAME_ENUM);
	}

    GET_VALIDATE_ASCII_CHILD(pIpServiceNode, "ALT_GATEKEEPER_NAME", m_AltGatekeeperName, _0_TO_MAX_GK_NAME_ENUM);


	GET_VALIDATE_CHILD(pIpServiceNode,"VPN_IP",&m_VpnIp,IP_ADDRESS);

	GET_VALIDATE_CHILD(pIpServiceNode,"AUTO_REGISTRATION_SPAN_HOST_NAME",&m_AutoRegisterSpanHostName,_BOOL);//judith

	BYTE tmpType = (BYTE)eIpServiceType_Signaling;
	GET_VALIDATE_CHILD(pIpServiceNode,"IP_SERVICE_TYPE", &tmpType,IP_SERVICE_TYPE_ENUM);
	m_eIpServiceType = (eIpServiceType)tmpType;

	GET_VALIDATE_CHILD(pIpServiceNode,"IS_REGISTER_AS_GATEWAY", &m_IsRegAsGW, _BOOL);

	m_Vlan.DeSerializeXml(pIpServiceNode, pszError, action);

    CXMLDOMElement *pPortSpeedListNode = NULL;
	GET_CHILD_NODE(pIpServiceNode, "PORT_SPEED_LIST", pPortSpeedListNode);
	if (NULL != pPortSpeedListNode)
	{
        CXMLDOMElement *pPortSpeedNode = NULL;
		GET_FIRST_CHILD_NODE(pPortSpeedListNode, "PORT_SPEED", pPortSpeedNode);
        for(DWORD i = 0 ; i < MAX_NUM_OF_PORTS_SPEED && NULL != pPortSpeedNode ; i++)
        {
            m_PortSpeedVector[i].DeSerializeXml(pPortSpeedNode, pszError, action);

            GET_NEXT_CHILD_NODE(pPortSpeedListNode, "PORT_SPEED", pPortSpeedNode);
        }
	}

	GET_VALIDATE_CHILD(pIpServiceNode,"IP_TYPE", (WORD*)(&m_ipType), IP_TYPE_ENUM);
	GET_VALIDATE_CHILD(pIpServiceNode,"IP_V6_CONFIGURATION_TYPE",	(WORD*)(&m_ipv6ConfigType), IP_V6_CONFIG_TYPE_ENUM);

	// ===== IPv6 address
	char fullAddress[IPV6_ADDRESS_LEN];
	char tmpDefGwIPv6[IPV6_ADDRESS_LEN];
	char tmpMask[IPV6_ADDRESS_LEN];
	memset(fullAddress,	0, IPV6_ADDRESS_LEN);
	memset(tmpDefGwIPv6, 0, IPV6_ADDRESS_LEN);
	memset(tmpMask,	0, IPV6_ADDRESS_LEN);

	GET_VALIDATE_CHILD(pIpServiceNode,"DEFAULT_ROUTER_IP_V6", fullAddress, _0_TO_IPV6_ADDRESS_LENGTH);
	SplitIPv6AddressAndMask(fullAddress, tmpDefGwIPv6, tmpMask);
	SetDefaultGatewayIPv6(tmpDefGwIPv6);
	SetDefaultGatewayMaskIPv6(tmpMask);

	// ===== IPv6 ScopeId
//	m_defaultGatewayIPv6.scopeId = ::getScopeId(tmpDefGwIPv6); // done inside SetDefaultGatewayIPv6() method



	GET_VALIDATE_CHILD(pIpServiceNode,"IS_SECURED", &m_bSecured,_BOOL);

	GET_VALIDATE_CHILD(pIpServiceNode,"PERMANENT_NETWORK", &m_bPermanentNetwork,_BOOL);

/*----------------------------------------------------------------
	Dynamic properties of ip service.
----------------------------------------------------------------*/

	CH323Info &h323Info = m_DynIPSProperties->GetH323Info();
//	h323Info.SetNumGk(1);

	DWORD gkIp = GetExternalGatekeeperAddr();
	CProxyDataContent &primaryGK = h323Info.GetPrimaryGk();
	primaryGK.SetIPv4Address(gkIp);
	primaryGK.SetName(m_GatekeeperName.GetString());

	DWORD altGkIp = GetAltGatekeeperAddr();
	CProxyDataContent &altGK = h323Info.GetAltGk();
	altGK.SetIPv4Address(altGkIp);
	altGK.SetName(m_AltGatekeeperName.GetString());

	CGKInfo &gkInfo =  m_DynIPSProperties->GetGKInfo();
	gkInfo.SetPrimaryGkIp(gkIp);
	gkInfo.SetAltGkIp(altGkIp);

	m_Vlan.DeSerializeXml(pIpServiceNode, pszError, action);

	GET_CHILD_NODE(pIpServiceNode, "SIP_ADVANCED", pTempNode);
	if (pTempNode)
	{

		CSipAdvanced* pSipAdvanced = new CSipAdvanced;
		nStatus = pSipAdvanced->DeSerializeXml(pTempNode, pszError);

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pSipAdvanced);
			return nStatus;
		}
		POBJDELETE(m_pSipAdvanced);
		m_pSipAdvanced = pSipAdvanced;
	}

	GET_VALIDATE_CHILD(pIpServiceNode, "SERVICE_ID", &m_Id, _0_TO_WORD);

	GET_CHILD_NODE(pIpServiceNode, "SERVICE_CFG", pTempNode);
	if (pTempNode)
	{
		CServiceConfig* pServiceConfig = new CServiceConfig;
		pServiceConfig->SetId(m_Id);
		pServiceConfig->SetIsServiceCFG(true);
		nStatus = pServiceConfig->DeSerializeXml(pTempNode, pszError,NULL);
		if (nStatus != STATUS_OK)
		{
			POBJDELETE(m_pServiceConfig);
			return nStatus;
		}
		POBJDELETE(m_pServiceConfig);
		m_pServiceConfig = pServiceConfig;
	}

	GET_CHILD_NODE(pIpServiceNode, "MANAGEMENT_SECURITY", pTempNode);
	if (pTempNode)
	{
		CManagementSecurity* pManagementSecurity = new CManagementSecurity;
		nStatus = pManagementSecurity->DeSerializeXml(pTempNode, pszError, action);

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pManagementSecurity);
			return nStatus;
		}
		POBJDELETE(m_pManagementSecurity);
		m_pManagementSecurity = pManagementSecurity;
	}


	CXMLDOMElement *pV35GwNode;
	GET_CHILD_NODE(pIpServiceNode, "V35_GATEWAY", pV35GwNode);

	if (pV35GwNode)
	{
		GET_VALIDATE_CHILD(pV35GwNode,"V35_GATEWAY_ENABLED",	&m_isV35GwEnabled,	_BOOL);
		GET_VALIDATE_CHILD(pV35GwNode,"V35_GATEWAY_IP",			&m_V35GwIpAddress,	IP_ADDRESS);
		GET_VALIDATE_CHILD(pV35GwNode,"V35_GATEWAY_USER_NAME",	m_V35GwUsername,	ONE_LINE_BUFFER_LENGTH);
		GET_VALIDATE_CHILD(pV35GwNode,"V35_GATEWAY_PORT",	m_V35GwPort,ONE_LINE_BUFFER_LENGTH);
		GET_VALIDATE_CHILD(pV35GwNode,"V35_GATEWAY_ALIAS",	m_V35GwAlias,ONE_LINE_BUFFER_LENGTH);
		GET_VALIDATE_CHILD(pV35GwNode,"V35_GATEWAY_PASSWORD",	m_V35GwPassword_dec,ONE_LINE_BUFFER_LENGTH);
//		TRACESTR(eLevelInfoNormal) << "\nCIPService::DeSerializeXml"
//							   << "\nPassword: " << m_V35GwPassword_dec;
	}

	GET_CHILD_NODE(pIpServiceNode, "IP_PERMISSIONS", pTempNode);
	if (pTempNode)
	{
		CWhiteList* pWhiteList = new CWhiteList;
		nStatus = pWhiteList->DeSerializeXml(pTempNode, pszError);
		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pWhiteList);
			return nStatus;
		}
		POBJDELETE(m_pWhiteList);
		m_pWhiteList = pWhiteList;
	}
	COstrStream msg;
	m_pWhiteList->Dump(msg);
	TRACESTR(eLevelInfoNormal) << "CIPService::DeSerializeXml - Dump WhiteList\n"<< msg.str().c_str();
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
bool CIPService::IsUserDefinePorts()
{
	CIPSpan* pSpan = GetFirstSpan();
	if(NULL == pSpan)
	{
		PASSERTMSG(1, "No Spans In Ip Service");
		return false;
	}

	CCommH323PortRange* portRange = pSpan->GetPortRange();
	BOOL res = portRange->IsEnabledPortRange();
	return TRUE == res;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CIPService::SetUdpPortRange(WORD firstPort, WORD numOfPorts)
{
    CIPSpan* pSpan = GetFirstSpan();
	while(NULL != pSpan)
	{
		CCommH323PortRange* portRange = pSpan->GetPortRange();
		portRange->SetUdpPortRange(firstPort, numOfPorts);

		pSpan = GetNextSpan();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CIPService::DefineTcpPortRange(DWORD maxPartiesNum)
{
	TRACESTR(eLevelInfoNormal) << "\nCIPService::DefineTcpPortRange - "<<maxPartiesNum;
	CIPSpan* pSpan = GetFirstSpan();
	while(NULL != pSpan)
	{
		CCommH323PortRange* portRange = pSpan->GetPortRange();
		portRange->SetDefaultTcpPorts(maxPartiesNum);

		pSpan = GetNextSpan();
	}
}

/////////////////////////////////////////////////////////////////////////////
eIpType  CIPService::GetIpType () const
{
    return m_ipType;
}

/////////////////////////////////////////////////////////////////////////////
void  CIPService::SetIpType(const eIpType  ipType)
{
    m_ipType = ipType;
}

/////////////////////////////////////////////////////////////////////////////
eV6ConfigurationType CIPService::GetIpV6ConfigurationType() const
{
	return m_ipv6ConfigType;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetIpV6ConfigurationType(const eV6ConfigurationType configType)
{
	m_ipv6ConfigType = configType;
}

////////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CIPService::GetChanged() const
{
  return m_bChanged;
}
/////////////////////////////////////////////////////////////////////////////
DWORD  CIPService::GetUpdateCounter() const
{
  return m_updateCounter;
}
//////////////////////////////////////////////////////////////////////
CDynIPSProperties *CIPService::GetDynamicProperties()
{
	return m_DynIPSProperties;
}
//////////////////////////////////////////////////////////////////////
const char* CIPService::GetName() const
{
    return m_serviceName;
}

/////////////////////////////////////////////////////////////////////////////
void  CIPService::SetName( const char* name )
{

    strncpy(m_serviceName, name, sizeof(m_serviceName) - 1);
    m_serviceName[sizeof(m_serviceName) - 1] = '\0';
}
/////////////////////////////////////////////////////////////////////////////
void  CIPService::SetServiceType(const unsigned char serviceTypeName)
{
    m_serviceTypeName = serviceTypeName;
}

/////////////////////////////////////////////////////////////////////////////
unsigned char CIPService::GetServiceType() const
{
    return m_serviceTypeName;
}

/////////////////////////////////////////////////////////////////////
void  CIPService::SetNetIPaddress(DWORD netIPaddress)
{
    m_netIPaddress = netIPaddress;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CIPService::GetNetIPaddress()
{
    return m_netIPaddress;
}

/////////////////////////////////////////////////////////////////////////////
void  CIPService::SetNetMask(DWORD netMask)
{
    m_netMask = netMask;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CIPService::GetNetMask() const
{
    return m_netMask;
}

/////////////////////////////////////////////////////////////////////////////
void   CIPService::SetDefaultGatewayIPv4(const DWORD defaultGateway)
{
    m_defaultGatewayIPv4.ip = defaultGateway;

}

/////////////////////////////////////////////////////////////////////////////
DWORD  CIPService::GetDefaultGatewayIPv4() const
{
	return m_defaultGatewayIPv4.ip;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetDefaultGatewayIPv6(const char* defaultGateway)
{
	// ===== 1. address
	mcTransportAddress tmpIPv6Addr;
	memset(&tmpIPv6Addr, 0, sizeof(mcTransportAddress));
	::stringToIpV6( &tmpIPv6Addr, (char*)defaultGateway );
	memcpy( &(m_defaultGatewayIPv6.ip), &(tmpIPv6Addr.addr.v6.ip), IPV6_ADDRESS_BYTES_LEN );

	// ===== 2. scopeId
	m_defaultGatewayIPv6.scopeId = ::getScopeId( (char*)defaultGateway );

	TRACESTR(eLevelInfoNormal) << "\nCIPService::SetDefaultGatewayIPv6 - " << defaultGateway;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::GetDefaultGatewayIPv6(char* retStr, BOOL isBrackets/*=FALSE*/) const
{
	::ipV6ToString(m_defaultGatewayIPv6.ip, retStr , isBrackets);
}

void CIPService::GetDefaultGatewayFullIPv6(char* retStr, BOOL isBrackets/*=FALSE*/) const
{

	::ipV6AndSubnetMaskToString(m_defaultGatewayIPv6.ip,
								m_defaultGatewayMaskIPv6,
								retStr,
								isBrackets);
}

/////////////////////////////////////////////////////////////////////////////
DWORD CIPService::GetDefaultGatewayMaskIPv6() const
{
	return m_defaultGatewayMaskIPv6;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::GetDefaultGatewayMaskIPv6Str(char *pOutMask) const
{
	sprintf(pOutMask, "%d", m_defaultGatewayMaskIPv6);
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetDefaultGatewayMaskIPv6(const DWORD subnetMask)
{
	m_defaultGatewayMaskIPv6 = subnetMask;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetDefaultGatewayMaskIPv6(const char *pMask)
{
	sscanf(pMask, "%d", &m_defaultGatewayMaskIPv6);
}

/////////////////////////////////////////////////////////////////////////////
void  CIPService::SetDHCPServer(WORD DHCPServer)
{
    m_DHCPServer = DHCPServer;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CIPService::GetDHCPServer()const
{
    return m_DHCPServer;
}

BOOL CIPService::IsContainGK()const
{
	BOOL res = (GATEKEEPER_EXTERNAL == GetGatekeeper() ? TRUE : FALSE);
	return res;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetGatekeeper(BYTE gatekeeper)
{
    m_gatekeeper = gatekeeper;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CIPService::GetGatekeeper()const
{
    return m_gatekeeper;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CIPService::GetExternalGatekeeperAddr()const
{
		return m_GatekeeperName.IpToDWORD();
}

/////////////////////////////////////////////////////////////////////////////
DWORD CIPService::GetAltGatekeeperAddr()const
{
	DWORD ip = m_AltGatekeeperName.IpToDWORD();
	return ip;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetGatekeeperDiskoveryPort(WORD GatekeeperDiskoveryPort)
{
    m_GatekeeperDiskoveryPort = GatekeeperDiskoveryPort;
}

/////////////////////////////////////////////////////////////////////////////
WORD CIPService::GetGatekeeperDiskoveryPort()
{
    return m_GatekeeperDiskoveryPort;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CIPService::GetRoutersNumber () const
{
    return m_numb_of_routers;
}

////////////////////////////////////////////////////
void  CIPService::SetRoutersNumber(const WORD num)
{
    m_numb_of_routers=num;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CIPService::GetIsRegAsGW()
{
	return m_IsRegAsGW;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetIsRegAsGW(BOOL val)
{
	m_IsRegAsGW = val;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CIPService::GetIsSecured()const
{
	return m_bSecured;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetIsSecured(BOOL val)
{
	m_bSecured = val;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CIPService::GetIsPermanentNetworkOpen()const
{
	return m_bPermanentNetwork;
}
/////////////////////////////////////////////////////////////////////////////
void CIPService::SetIsPermanentNetworkOpen(BOOL val)
{
	m_bPermanentNetwork = val;
}
/////////////////////////////////////////////////////////////////////////////
int	CIPService::AddRouter(  const CH323Router &other  , bool isCheckSpanValidity/*=true*/, bool isShouldNotExist/*=true*/)
{
    if(m_numb_of_routers>=MAX_ROUTERS_IN_H323_SERVICE )
        return  STATUS_NUMBER_OF_ROUTERS_IN_SERVICE_EXCEEDED;

    if(true == isShouldNotExist)
    {
    	if(FindRouter( other ) != NOT_FIND )
    	    return STATUS_ROUTER_EXISTS;
    }

//     int status = IsValidRouter(&other);
//     if (status!=STATUS_OK)
//         return status;

 	if(true == isCheckSpanValidity)
 	{
	    STATUS status = IsValidRouterWithSpans(&other);
	    if (status!=STATUS_OK)
	        return status;
 	}

    m_pRouter[m_numb_of_routers] = new CH323Router( other );
    m_numb_of_routers++;

    return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
int	CIPService::UpdateRouter( const CH323Router &other )
{
    int ind = FindRouter( other );
    if(ind==NOT_FIND || ind >= MAX_ROUTERS_IN_H323_SERVICE)
    	return STATUS_ROUTER_NOT_EXISTS;

    int status = IsValidRouter(&other);
    if (status!=STATUS_OK)
        return status;

    status = IsValidRouterWithSpans(&other);
    if (status!=STATUS_OK)
        return status;

    delete m_pRouter[ind];
    m_pRouter[ind] = new CH323Router( other );

    return  STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int	CIPService::FindRouter(  const CH323Router &other  )
{
    for( int i=0; i<(int)m_numb_of_routers; i++ )
    {
        if(m_pRouter[i]->GetRouterIP()==other.GetRouterIP() &&
            m_pRouter[i]->GetRemoteIP()==other.GetRemoteIP() &&
            m_pRouter[i]->GetRemoteFlag()==other.GetRemoteFlag())
            return i;
    }
    return NOT_FIND;
}

/////////////////////////////////////////////////////////////////////////////
int	CIPService::CancelRouter( WORD ind )
{
    if(m_numb_of_routers > MAX_ROUTERS_IN_H323_SERVICE )
        return  STATUS_NUMBER_OF_ROUTERS_IN_SERVICE_EXCEEDED;

    if ((ind >= GetRoutersNumber ()) || (ind >= MAX_ROUTERS_IN_H323_SERVICE))
        return STATUS_ILLEGAL;

    PDELETE( m_pRouter[ind] );
	int i=0;
    for(i=0; i<(int)m_numb_of_routers; i++ )
    {
        if(m_pRouter[i] == NULL )
            break;
    }
    for( int j=i; j<(int)m_numb_of_routers-1; j++ )
    {
        m_pRouter[j] = m_pRouter[j+1] ;
    }
    m_pRouter[m_numb_of_routers-1] =  NULL;
    m_numb_of_routers--;

    return  STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::RemoveAllRouters( )
{
    for( int i=0; i<(int)m_numb_of_routers; i++ )
        PDELETE(m_pRouter[i]);

    m_numb_of_routers=0;
    m_ind_router=0;
}

/////////////////////////////////////////////////////////////////////////////
CH323Router* CIPService::GetFirstRouter()
{
    m_ind_router=1;
    return m_pRouter[0];
}

/////////////////////////////////////////////////////////////////////////////
CH323Router*  CIPService::GetNextRouter()
{
    if(m_ind_router>=m_numb_of_routers )
    	return NULL;

    return m_pRouter[m_ind_router++];
}

/////////////////////////////////////////////////////////////////////////////
CH323Router* CIPService::GetFirstRouter(int& nPos)
{
    CH323Router* pH323Router = CIPService::GetFirstRouter();
    nPos=m_ind_router;

    return pH323Router;
}

/////////////////////////////////////////////////////////////////////////////
CH323Router* CIPService::GetNextRouter(int ind,int& nPos)
{
    m_ind_router = ind;
    CH323Router*  pH323Router = CIPService::GetNextRouter();
    nPos = m_ind_router;

    return pH323Router;
}

/////////////////////////////////////////////////////////////////////////////
CH323Router*  CIPService::GetCurrentRouter(WORD ind) const
{
    if (ind >= GetRoutersNumber () || ind >= MAX_ROUTERS_IN_H323_SERVICE)
        return NULL;

    return m_pRouter[ind];
}

/////////////////////////////////////////////////////////////////////////////
WORD  CIPService::GetSpansNumber () const
{
    return m_numb_of_span;
}

//////////////////////////////////////////////////////////////////////////////
void  CIPService::SetSpansNumber(const WORD num)
{
    m_numb_of_span=num;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CIPService::GetVlanId()
{
	return m_Vlan.GetId();
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetVlanId(DWORD val)
{
	m_Vlan.SetId(val);
}

/////////////////////////////////////////////////////////////////////////////
int CIPService::AddSpan( const CIPSpan&  other )
{
    if(m_numb_of_span>=MAX_SPAN_NUMBER_IN_SERVICE )
	{
        return  STATUS_NUMBER_OF_SPANS_IN_SERVICE_EXCEEDED;
    }

    if(FindSpan( other ) != NOT_FIND )
	{
        return STATUS_SPAN_EXISTS;
    }

    int status = IsValidSpan(&other);
    if (status != STATUS_OK)
	{
       return status;
	}

	if ( !GetDHCPServer())
	{
		status = IsValidSpanWithRouters(&other);
		if (status != STATUS_OK)
		{
			return status;
		}
	}

    m_pSpan[m_numb_of_span] = new CIPSpan( other );
    m_numb_of_span++;

    return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
int CIPService::AddSpan_NoCheck( const CIPSpan&  other )
{
    if(m_numb_of_span>=MAX_SPAN_NUMBER_IN_SERVICE )
	{
        return  STATUS_NUMBER_OF_SPANS_IN_SERVICE_EXCEEDED;
    }

    m_pSpan[m_numb_of_span] = new CIPSpan( other );
    m_numb_of_span++;

    return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
int CIPService::UpdateSpan( const CIPSpan&  other )
{
    int ind = FindSpan( other );
    if(ind==NOT_FIND || ind >= MAX_SPAN_NUMBER_IN_SERVICE)
    	return STATUS_SPAN_NOT_EXISTS;

    int status = IsValidSpan(&other);
    if (status != STATUS_OK)
        return status;


	if ( !GetDHCPServer())
	{
		status = IsValidSpanWithRouters(&other);
		if (status != STATUS_OK)
			return status;
	}

    delete m_pSpan[ind];
    m_pSpan[ind] = new CIPSpan( other );

    return  STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::ReplaceSpan_NoCheck( const WORD idx, const CIPSpan&  other )
{
	if ( idx < MAX_SPAN_NUMBER_IN_SERVICE )
	{
		if ( NULL != m_pSpan[idx] )
		{
			POBJDELETE( m_pSpan[idx] );
			if (m_numb_of_span > 0)
			{
				m_numb_of_span--;
			}
		}

		m_pSpan[idx] = new CIPSpan( other );
		m_numb_of_span++;
	}
}

/////////////////////////////////////////////////////////////////////////////
int CIPService::CancelSpan(const WORD line)
{
    int ind = FindSpan( line );
    if(ind==NOT_FIND || ind >= MAX_SPAN_NUMBER_IN_SERVICE)
    	return STATUS_SPAN_NOT_EXISTS;

    PDELETE( m_pSpan[ind] );
    int i=0;
    for(i=0; i<(int)m_numb_of_span; i++ )
    {
        if(m_pSpan[i]==NULL )
            break;
    }
    for( int j=i; j<(int)m_numb_of_span-1; j++ )
    {
        m_pSpan[j] = m_pSpan[j+1] ;
    }
    if (m_numb_of_span > 0)
    {
		m_pSpan[m_numb_of_span-1] = NULL;
		m_numb_of_span--;
    }
    return  STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CIPService::CancelSpanByIndex(const int ind)
{
    if(ind==NOT_FIND || ind >= MAX_SPAN_NUMBER_IN_SERVICE)
    	return STATUS_SPAN_NOT_EXISTS;

    PDELETE( m_pSpan[ind] );
    int i=0;
    for(i=0; i<(int)m_numb_of_span; i++ )
    {
        if(m_pSpan[i]==NULL )
            break;
    }
    for( int j=i; j<(int)m_numb_of_span-1; j++ )
    {
        m_pSpan[j] = m_pSpan[j+1] ;
    }
    if (m_numb_of_span > 0 && m_numb_of_span <= MAX_SPAN_NUMBER_IN_SERVICE)
    {
		m_pSpan[m_numb_of_span-1] = NULL;
		m_numb_of_span--;
    }
    return  STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
void CIPService::RemoveZeroSpansFromService()
{
	int status = STATUS_OK;
	for(int i=0; i<GetSpansNumber(); i++ )
	{
		if (GetSpanByIdx(i) != NULL && GetSpanByIdx(i)->GetLineNumber() == 0  )
		{
			char	ipv6Adress[IPV6_ADDRESS_LEN];
			memset(ipv6Adress,	0, IPV6_ADDRESS_LEN);
			GetSpanByIdx(i)->GetIPv6Address(0, ipv6Adress);
			if(GetSpanByIdx(i)->GetIPv4Address() == 0 && ((strcmp(ipv6Adress, "::/64" ) == 0) || (strcmp(ipv6Adress, "::" ) == 0)))
			{
                PASSERT_AND_RETURN(i >= MAX_SPAN_NUMBER_IN_SERVICE);
				status = CancelSpanByIndex(i);
				if (status == STATUS_OK) i--;
			}
		}

	}
}
/////////////////////////////////////////////////////////////////////////////
int CIPService::FindSpan( const CIPSpan&  other )
{

    for( int i=0; i<(int)m_numb_of_span; i++ )
    {
        if (m_pSpan[i])
            if(m_pSpan[i]->GetLineNumber()==other.GetLineNumber() )
                return i;
    }
    return NOT_FIND;
}

/////////////////////////////////////////////////////////////////////////////
int CIPService::FindSpan(const WORD line)
{
    for (int i=0;i<(int)m_numb_of_span;i++)
    {
        if (m_pSpan[i])
            if(m_pSpan[i]->GetLineNumber()==line )
                return i;
    }
    return NOT_FIND;
}

/////////////////////////////////////////////////////////////////////////////
CIPSpan*  CIPService::GetFirstSpan()
{
    m_ind_span=1;
    return m_pSpan[0];
}

/////////////////////////////////////////////////////////////////////////////
CIPSpan*  CIPService::GetNextSpan()
{
    if(m_ind_span>=m_numb_of_span ) return NULL;

    PASSERTSTREAM_AND_RETURN_VALUE(m_ind_span >= MAX_SPAN_NUMBER_IN_SERVICE,
        "m_ind_span has invalid value " << m_ind_span, NULL);

    return m_pSpan[m_ind_span++];
}

/////////////////////////////////////////////////////////////////////////////
CIPSpan* CIPService::GetSpanByIdx(const WORD idx) const
{
	if ( (idx < m_numb_of_span) && idx < MAX_SPAN_NUMBER_IN_SERVICE && (m_pSpan[idx]) )
	    return m_pSpan[idx];

    return NULL;
}

/////////////////////////////////////////////////////////////////////////////
CIPSpan* CIPService::GetCurrentSpan( const WORD line ) const
{

    for( int i=0; i<(int)m_numb_of_span; i++ )
    {
        if(m_pSpan[i] )
            if(m_pSpan[i]->GetLineNumber()==line )
                return m_pSpan[i];
    }
    return NULL;		  // STATUS_SPAN_NOT_EXISTS
}

/////////////////////////////////////////////////////////////////////////////
WORD CIPService::CalcSpanPosAccordingToBoardAndSubBoardId(DWORD board_id, DWORD sub_board_id)
{
	if (sub_board_id==1)
		return board_id;
	if (sub_board_id==2)
		return board_id+1;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::ClearIPv6Addresses()
{
	ClearSignalingIpv6Addresses();
	ClearMediaIpv6Addresses();
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::ClearSignalingIpv6Addresses()
{
    CIPSpan* pSpan = GetFirstSpan(); // params of Signaling are stored in span 0
    if (pSpan)
    {
		char clearedAddress[IPV6_ADDRESS_LEN];
		memset(clearedAddress,	0, IPV6_ADDRESS_LEN);

		for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
		{
			pSpan->SetIPv6Address(i, clearedAddress);
			pSpan->SetIPv6SubnetMask(i, DEFAULT_IPV6_SUBNET_MASK);
		}
    }
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::ClearMediaIpv6Addresses()
{
    CIPSpan* pSpan = GetFirstSpan();
    if (pSpan)
    {
    	// params of Signaling are stored in span 0; params of MPMs are stored in spans 1-...
    	pSpan = GetNextSpan();
    }

	while(pSpan)
	{
		char clearedAddress[IPV6_ADDRESS_LEN];
		memset(clearedAddress,	0, IPV6_ADDRESS_LEN);

		for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
		{
			pSpan->SetIPv6Address(i, clearedAddress);
			pSpan->SetIPv6SubnetMask(i, DEFAULT_IPV6_SUBNET_MASK);
		}

		pSpan = GetNextSpan();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetIpParamsFromOtherService(const CIPService *pOther, int spanIdx, eIpType ipTypeToSet)
{
	if (eIpType_IpV4 == ipTypeToSet)
	{
		SetIPv4ParamsFromOtherService(pOther, spanIdx);
	}
	else if (eIpType_IpV6 == ipTypeToSet)
	{
		SetIPv6ParamsFromOtherService(pOther, spanIdx);
	}
	else if (eIpType_Both == ipTypeToSet)
	{
		SetIPv4ParamsFromOtherService(pOther, spanIdx);
		SetIPv6ParamsFromOtherService(pOther, spanIdx);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetIPv4ParamsFromOtherService(const CIPService *pOther, int spanIdx)
{
	CIPSpan* pSpan = GetSpanByIdx(spanIdx);
	if (!pSpan)
	{
		TRACESTR(eLevelInfoNormal) << "\nCIPService::SetIPv4ParamsFromOtherService - no span of idx " << spanIdx;
		return;
	}

	CIPSpan* pOtherSpan = pOther->GetSpanByIdx(spanIdx);
	if (!pOtherSpan)
	{
		TRACESTR(eLevelInfoNormal) << "\nCIPService::SetIPv4ParamsFromOtherService - no span of idx " << spanIdx << " in the other service";
		return;
	}


	// ===== 1. set address
	pSpan->SetIPv4Address( pOtherSpan->GetIPv4Address() );

	// ===== 2. set mask
	SetNetMask( pOther->GetNetMask() );

	// ===== 3. set defGW
	SetDefaultGatewayIPv4( pOther->GetDefaultGatewayIPv4() );

	// ===== 4. set routers list
	RemoveAllRouters();
	CH323Router *pCurOtherRouter = NULL;
	int numOfRouters = pOther->GetRoutersNumber();
	for (int i=0; (i < numOfRouters) && (i < MAX_ROUTERS_IN_H323_SERVICE); i++)
	{
		pCurOtherRouter = pOther->GetCurrentRouter(i);
		if (pCurOtherRouter)
		{
			AddRouter(*pCurOtherRouter);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetIPv6ParamsFromOtherService(const CIPService *pOther, int spanIdx)
{
	CIPSpan* pSpan = GetSpanByIdx(spanIdx);
	if (!pSpan)
	{
		TRACESTR(eLevelInfoNormal) << "\nCIPService::SetIPv6ParamsFromOtherService - no span of idx " << spanIdx << " in current service";
		return;
	}

	CIPSpan* pOtherSpan = pOther->GetSpanByIdx(spanIdx);
	if (!pOtherSpan)
	{
		TRACESTR(eLevelInfoNormal) << "\nCIPService::SetIPv6ParamsFromOtherService - no span of idx " << spanIdx << " in the other service";
		return;
	}

	// ===== 1. set addresses & masks
	char	curAddress[IPV6_ADDRESS_LEN];
	DWORD	curSubMask=0;

	for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		memset(curAddress,	0, IPV6_ADDRESS_LEN);
		pOtherSpan->GetIPv6Address(i, curAddress);
		pSpan->SetIPv6Address(i, curAddress);

		pSpan->SetIPv6SubnetMask(i, pOtherSpan->GetIPv6SubnetMask(i));
	}

	// ===== 2. set defGW
	if (eV6Configuration_Auto == m_ipv6ConfigType)		//update default gw ipv6 only in auto mode
	{
		char curDefGW[IPV6_ADDRESS_LEN];
		pOther->GetDefaultGatewayIPv6(curDefGW);
		SetDefaultGatewayIPv6(curDefGW);
		SetDefaultGatewayMaskIPv6(pOther->GetDefaultGatewayMaskIPv6());
	}
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetIpV6Params( eIpType ipType, eV6ConfigurationType v6ConfigType,
								string ipv6Add_0, string ipv6Add_1, string ipv6Add_2, string ipv6_defGw,
								DWORD ipv6Mask_0, DWORD ipv6Mask_1, DWORD ipv6Mask_2, DWORD ipv6Mask_defGw, BOOL bForceDefGwUpdate )
{
	bool isJitcAndNetSeparation = ::IsJitcAndNetSeparation();

	SetIpType(ipType);
	SetIpV6ConfigurationType(v6ConfigType);

	if (false == isJitcAndNetSeparation)
	{
		if (bForceDefGwUpdate || eV6Configuration_Auto == v6ConfigType)
		{
			TRACESTR(eLevelInfoNormal) << "CIPService::SetIpV6Params - set default ipv6 router to be = "<<ipv6_defGw.c_str()<<" with mask = "<<ipv6Mask_defGw;
			SetDefaultGatewayIPv6(ipv6_defGw.c_str());
			SetDefaultGatewayMaskIPv6(ipv6Mask_defGw);
		}

		// in Auto, Mngmnt and CS share the same address (but not in RMX4000 adn RMX1500 where every entity has its own NIC)
		eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
		if ( (eProductTypeRMX4000 != curProductType) && (eProductTypeRMX1500 != curProductType) && 
			 (eProductTypeNinja != curProductType) && (eProductTypeGesher != curProductType) && 
			(eV6Configuration_Auto == v6ConfigType) )
		{
			GetFirstSpan()->SetIPv6Address(0, ipv6Add_0.c_str());
			GetFirstSpan()->SetIPv6Address(1, ipv6Add_1.c_str());
			GetFirstSpan()->SetIPv6Address(2, ipv6Add_2.c_str());

			GetFirstSpan()->SetIPv6SubnetMask(0, ipv6Mask_0);
			GetFirstSpan()->SetIPv6SubnetMask(1, ipv6Mask_1);
			GetFirstSpan()->SetIPv6SubnetMask(2, ipv6Mask_2);
		}
	}

	else // isJitcAndNetSeparation == true
	{
		if (eV6Configuration_Auto == v6ConfigType)
		{
			//Read Signaling ipv6 address
			SystemPipedCommand("/sbin/ifconfig eth0.2198 | grep Scope:Global | awk '{ print $3 }'", ipv6Add_0);

			int lenToCopy = IPV6_ADDRESS_LEN - 1;

			int singleAddressLen = 0;
			const char *pIpv6Start	= ipv6Add_0.c_str();
			const char *pIpvEndl	= strchr(ipv6Add_0.c_str(), '/'); // search for /
			if (pIpvEndl)
			{
				singleAddressLen = pIpvEndl - pIpv6Start; // num of characters before the /
			}

			if ( (0 <= singleAddressLen) && (singleAddressLen <= lenToCopy) )
			{
				lenToCopy = singleAddressLen;
			}

			char Ipv6Address[IPV6_ADDRESS_LEN]="";
			memset(Ipv6Address, 0, IPV6_ADDRESS_LEN);
			strncpy(Ipv6Address, ipv6Add_0.c_str(), sizeof(Ipv6Address) - 1);
			Ipv6Address[sizeof(Ipv6Address) - 1] = '\0';

			TRACESTR(eLevelInfoNormal) << "CIPService::SetIpV6Params etho.2198 Scope:Global: " << ipv6Add_0 << " , after parsing:" << Ipv6Address;

			GetFirstSpan()->SetIPv6Address(0, Ipv6Address);
			GetFirstSpan()->SetIPv6SubnetMask(0, DEFAULT_IPV6_SUBNET_MASK);

			GetFirstSpan()->SetIPv6Address(1, "");
			GetFirstSpan()->SetIPv6Address(2, "");
			GetFirstSpan()->SetIPv6SubnetMask(1, DEFAULT_IPV6_SUBNET_MASK);
			GetFirstSpan()->SetIPv6SubnetMask(2, DEFAULT_IPV6_SUBNET_MASK);

			//Read Signaling default gw
			SystemPipedCommand("echo -n `route -A inet6 | grep eth0.2198 | grep UGDA | awk '{ print $2 }'`", ipv6_defGw);
			TRACESTR(eLevelInfoNormal) << "CIPService::SetIpV6Params etho.2198 default gw: " << ipv6_defGw;

			SetDefaultGatewayIPv6(ipv6_defGw.c_str());
			SetDefaultGatewayMaskIPv6(DEFAULT_IPV6_SUBNET_MASK);

		} // end if IPv6Auto
	} // end if Separation == true
}

/////////////////////////////////////////////////////////////////////////////
WORD   CIPService::IsValid(DWORD ip_address1, DWORD ip_address2)
{
    WORD result = FALSE;
    DWORD netMask = GetNetMask();

    if ( (ip_address1&netMask) == (ip_address2&netMask) )
        result = TRUE;
    return result;
}

/////////////////////////////////////////////////////////////////////////////
STATUS  CIPService::IsValidRouter(const CH323Router* pCurRouter)
{
    if (!pCurRouter)
        return STATUS_ILLEGAL;

    CH323Router*  pFirstRouter = m_pRouter[0];

    if (pFirstRouter){
        DWORD firstRouterIP = pFirstRouter->GetRouterIP();
        DWORD curRouterIP = pCurRouter->GetRouterIP();

        if (!IsValid( firstRouterIP, curRouterIP ))
            return STATUS_IP_ROUTER_IS_NOT_ON_THE_SAME_SUBNET_AS_THE_OTHER_ROUTERS;
    }

    return STATUS_OK;
}




/////////////////////////////////////////////////////////////////////////////
STATUS   CIPService::IsValidSpan(const CIPSpan* pCurSpan)
{
    ESTATUS status = STATUS_OK;
    if (!pCurSpan)
        return STATUS_ILLEGAL;

    if (GetGatekeeper()==GATEKEEPER_INTERNAL ||
        GetGatekeeper()==GATEKEEPER_EXTERNAL)
        if (pCurSpan->GetAliasNamesNumber()==0)
            return STATUS_EMPTY_ALIAS_NAMES_LIST_IN_SPAN;

        CIPSpan* pTempSpan = new CIPSpan(*pCurSpan);//to overcome span's const
        CH323Alias* pCurAlias =  pTempSpan->GetFirstAlias();
        while( pCurAlias && (status == STATUS_OK) )
        {
            status = pCurAlias->TestValidity();
            pCurAlias =  pTempSpan->GetNextAlias();
        }
        PDELETE(pTempSpan);

        if(status != STATUS_OK )
            return status;

        if (!GetDHCPServer()) // span IP can't be configured as 0.0.0.0
		{

			CIPSpan*  pFirstSpan = m_pSpan[0];
			if (pFirstSpan){
				DWORD firstSpanIP = pFirstSpan->GetIPv4Address();
				DWORD curSpanIP = pCurSpan->GetIPv4Address();

				//check ip validaty
				if (curSpanIP == 0 || curSpanIP == 0xFFFFFFFF )
					return STATUS_IP_ADDRESS_IN_SPAN_NOT_VALID;

				if (!IsValid( firstSpanIP, curSpanIP ))
					return STATUS_IP_SPAN_IS_NOT_ON_THE_SAME_SUBNET_AS_THE_OTHER_SPANS;
			}
		 }

        if(pCurSpan->GetPortRange()){
            status = pCurSpan->GetPortRange()->TestValidity();
            if(status != STATUS_OK )
                return status;
        }

        return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS   CIPService::IsValidRouterWithSpans(const CH323Router* pCurRouter)
{
    if (!pCurRouter)
        return STATUS_ILLEGAL;

    CIPSpan*  pFirstSpan = m_pSpan[0];
    if (pFirstSpan){
        DWORD curRouterIP = pCurRouter->GetRouterIP();
        DWORD firstSpanIP = pFirstSpan->GetIPv4Address();

        if (!IsValid( curRouterIP, firstSpanIP ))
            return STATUS_IP_ROUTER_IS_NOT_ON_THE_SAME_SUBNET_AS_THE_SPANS;
    }

    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS   CIPService::IsValidSpanWithRouters(const CIPSpan* pCurSpan)
{
    if (!pCurSpan)
        return STATUS_ILLEGAL;

    CH323Router*  pFirstRouter = m_pRouter[0];
    if (pFirstRouter){
        DWORD firstRouterIP = pFirstRouter->GetRouterIP();
        DWORD curSpanIP = pCurSpan->GetIPv4Address();

        if (!IsValid( firstRouterIP, curSpanIP ))
            return STATUS_IP_SPAN_IS_NOT_ON_THE_SAME_SUBNET_AS_THE_ROUTERS;
    }
    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS  CIPService::IsValidDefaultRouter()
{
    DWORD defRouterIP = GetDefaultGatewayIPv4();

    if (defRouterIP){
        CH323Router*  pFirstRouter = m_pRouter[0];

        if (pFirstRouter){
            DWORD firstRouterIP = pFirstRouter->GetRouterIP();
            if (!IsValid( firstRouterIP, defRouterIP ))
                return STATUS_IP_ROUTER_IS_NOT_ON_THE_SAME_SUBNET_AS_THE_OTHER_ROUTERS;
        }

        CIPSpan*  pFirstSpan = m_pSpan[0];
        if (pFirstSpan){
            DWORD firstSpanIP = pFirstSpan->GetIPv4Address();

            if (!IsValid( defRouterIP, firstSpanIP ))
                return STATUS_IP_ROUTER_IS_NOT_ON_THE_SAME_SUBNET_AS_THE_SPANS;
        }
    }

    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void  CIPService::SetDialInPrefix(const char* prefix, WORD prefix_type)
{
    strncpy(m_dialIn_prefix.m_aliasName, prefix, sizeof(m_dialIn_prefix.m_aliasName) - 1);
    m_dialIn_prefix.m_aliasName[sizeof(m_dialIn_prefix.m_aliasName) - 1] ='\0';
    m_dialIn_prefix.m_aliasType = prefix_type;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetDialInPrefix(const CH323Alias& newPrefix)
{
    strncpy(m_dialIn_prefix.m_aliasName, newPrefix.m_aliasName, sizeof(m_dialIn_prefix.m_aliasName) - 1);
    m_dialIn_prefix.m_aliasName[sizeof(m_dialIn_prefix.m_aliasName) - 1] = '\0';
    m_dialIn_prefix.m_aliasType = newPrefix.m_aliasType;
}

/////////////////////////////////////////////////////////////////////////////
const  CH323Alias*  CIPService::GetDialInPrefix() const
{
    return &m_dialIn_prefix;
}

/////////////////////////////////////////////////////////////////////////////
void   CIPService::SetATMEnableAAL5(const unsigned char ATMEnableAAL5)
{
    m_ATMEnableAAL5=ATMEnableAAL5;
}
///////////////////////////////////////////////////////////////////////////////
unsigned char  CIPService::GetATMEnableAAL5()const
{
    return m_ATMEnableAAL5;
}
/////////////////////////////////////////////////////////////////////////////
void CIPService::SetQualityOfService(const CQualityOfService& qos)
{
    POBJDELETE(m_pQualityOfService);
	m_pQualityOfService = new CQualityOfService(qos);
}
////////////////////////////////////////////////////////////////////////////////
void  CIPService::SetQualityOfService(const unsigned char qualityOfService)
{
    m_qualityOfService=qualityOfService;
}
////////////////////////////////////////////////////////////////////////////////
unsigned char  CIPService::GetQualityOfService()const
{
    return m_qualityOfService;
}

////////////////////////////////////////////////////////////////////////////////////
const char  * CIPService::GetH323AuthenticationPassword() const
{
   return  m_pSecurity->GetIPAuthentication()->GetSecondHTTPDigestAuthenticationElements()->GetPassword().GetString();
}

////////////////////////////////////////////////////////////////////////////////////
const char * CIPService::GetH323AuthenticationUserName() const
{
	 return m_pSecurity->GetIPAuthentication()->GetSecondHTTPDigestAuthenticationElements()->GetUserName().GetString();
}

////////////////////////////////////////////////////////////////////////////////////
eAuthenticationProtocol CIPService::GetH323AuthenticationProtocol()
{
 	 return m_pSecurity->GetIPAuthentication()->GetSecondHTTPDigestAuthenticationElements()->GetAuthenticationProtocol();
}

////////////////////////////////////////////////////////////////////////////////////
BOOL CIPService::GetH323AuthenticationEnable()
{
  	 return m_pSecurity->GetIPAuthentication()->GetSecondHTTPDigestAuthenticationElements()->GetAuthenticationEnable();
}

////////////////////////////////////////////////////////////////////////////////////
void CIPService::SetH323AuthenticationPassword(const char * password)
{
	CSmallString tmpPassword(password);
	m_pSecurity->GetIPAuthentication()->GetSecondHTTPDigestAuthenticationElements()->SetPassword(tmpPassword);
}

////////////////////////////////////////////////////////////////////////////////////
void CIPService::SetH323AuthenticationUserName(const char * username)
{
	CSmallString tmpUsername(username);
	m_pSecurity->GetIPAuthentication()->GetSecondHTTPDigestAuthenticationElements()->SetUserName(tmpUsername);
}

////////////////////////////////////////////////////////////////////////////////////
void CIPService::SetH323AuthenticationProtocol(eAuthenticationProtocol protocol)
{
	m_pSecurity->GetIPAuthentication()->GetSecondHTTPDigestAuthenticationElements()->SetAuthenticationProtocol(protocol);
}

////////////////////////////////////////////////////////////////////////////////////
void CIPService::SetH323AuthenticationEnable(BOOL authenticationMode)
{
	m_pSecurity->GetIPAuthentication()->GetSecondHTTPDigestAuthenticationElements()->SetAuthenticationEnable(authenticationMode);
}

////////////////////////////////////////////////////////////////////////////////////
void CIPService::SetQosAtm(const BYTE status,
                           const BYTE audioPriority,
						   const BYTE videoPriority)
{
    m_pQualityOfService->SetQosAtm(status,audioPriority,videoPriority);
}

////////////////////////////////////////////////////////////////////////////
BYTE CIPService::ServiceTypeToSpanType (const BYTE serviceType)
{
    switch (serviceType)
    {
    case SERVICE_H323_LAN: return SPAN_H323_LAN;

    case SERVICE_H323_IPOATM:return SPAN_H323_IPOATM;

    case SERVICE_H323_LANEMU:return SPAN_H323_LANEMU;
    }
    return SPAN_H323_LAN;
}

//////////////////////////////////////////////////////////////////////////////
void CIPService::SetVpnIp(DWORD vpnIp)
{
	m_VpnIp = vpnIp;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetDns(const CIpDns& pDNS)
{
    POBJDELETE(m_pDns);
    m_pDns = new CIpDns(pDNS);
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetSecurity(const CIPSecurity& security)
{
    POBJDELETE(m_pSecurity);
    m_pSecurity = new CIPSecurity(security);
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetIPProtocolType(const eIPProtocolType ProtocolType)
{
    m_eProtocolType = ProtocolType;
}

/////////////////////////////////////////////////////////////////////////////
CSip* CIPService::GetSip() const
{
	return m_pSip;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetSip(const CSip& sip)
{
    POBJDELETE(m_pSip);
    m_pSip = new CSip(sip);
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetSipAdvanced(const CSipAdvanced& pSipAdvanced)
{
	POBJDELETE(m_pSipAdvanced);
	m_pSipAdvanced = new CSipAdvanced(pSipAdvanced);
}

/////////////////////////////////////////////////////////////////////////////
CManagementSecurity* CIPService::GetManagementSecurity() const
{
	return m_pManagementSecurity;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetManagementSecurity(const CManagementSecurity& ManagementSecurity)
{
    POBJDELETE(m_pManagementSecurity);
    m_pManagementSecurity = new CManagementSecurity(ManagementSecurity);
}


/////////////////////////////////////////////////////////////////////////////
void CIPService::SetGatekeeperName(const CSmallString& GatekeeperName)
{
   m_GatekeeperName = GatekeeperName;
}

/////////////////////////////////////////////////////////////////////////////
const CSmallString & CIPService::GetGatekeeperName()
{
    return m_GatekeeperName;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetAltGatekeeperName(const CSmallString& AltGatekeeperName)
{
    m_AltGatekeeperName = AltGatekeeperName;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CIPService::ConvertToIpParamsStruct(IP_PARAMS_S &ipParamsStruct)
{
	TRACESTR(eLevelInfoNormal) << "\nCIPService::ConvertToIpParamsStruct";

	int i=0, j=0, listLength=0;

	// ===== 1. NetworkParameters attributes
	NETWORK_PARAMS_S curNetParamStruct;
	memset(&curNetParamStruct, 0, sizeof(NETWORK_PARAMS_S));

	curNetParamStruct.subnetMask     = GetNetMask();
	curNetParamStruct.defaultGateway = GetDefaultGatewayIPv4();

	GetDefaultGatewayFullIPv6( (char*)(curNetParamStruct.defaultGatewayIPv6));

	curNetParamStruct.isDhcpInUse    = (BYTE)(GetDHCPServer());

	listLength = GetRoutersNumber();
	for (i=0; (i < listLength) && (i < MAX_ROUTERS_IN_H323_SERVICE); i++)
	{
		if ( NULL != GetCurrentRouter(i) )
		{
			CH323Router curRouter( *(GetCurrentRouter(i)) );
			curNetParamStruct.ipRouter[i].routerIp = curRouter.GetRouterIP();
			curNetParamStruct.ipRouter[i].remoteIp = curRouter.GetRemoteIP();
			curNetParamStruct.ipRouter[i].remoteFlag = (DWORD)(curRouter.GetRemoteFlag());
			curNetParamStruct.ipRouter[i].subnetMask = (DWORD)(curRouter.GetSubnetMask());
		}
		else
		{
			curNetParamStruct.ipRouter[i].routerIp =0;
		}
	}

	ipParamsStruct.networkParams = curNetParamStruct;

	// ===== 2. Spans attributes
	listLength = GetSpansNumber();
    for (i=0; (i < listLength) && (i < MAX_NUM_OF_PQS); i++)
    {
 	   	IP_INTERFACE_S curIpInterfaceStruct;
 	   	memset(&curIpInterfaceStruct, 0, sizeof(IP_INTERFACE_S));

 	   	char curIPv6Addr[IPV6_ADDRESS_LEN];

 	   	if (0 == i) // the following parameters are taken from the 1st interface (Management)
 	   	{
 	   		curIpInterfaceStruct.isSecured = GetIsSecured();
 	   		curIpInterfaceStruct.isPermanentOpen = GetIsPermanentNetworkOpen();
 	   	}

 		if ( NULL != GetSpanByIdx(i) )
		{
	    	CIPSpan curSpan( *(GetSpanByIdx(i)) );

	    	curIpInterfaceStruct.ipType = (APIU32)(GetIpType());
	    	curIpInterfaceStruct.iPv4.iPv4Address = curSpan.GetIPv4Address();

	    	for(int addressIdx=0; addressIdx<NUM_OF_IPV6_ADDRESSES; addressIdx++)
	    	{
				memset(curIPv6Addr, 0, IPV6_ADDRESS_LEN);

				curSpan.GetFullIPv6Address(addressIdx, curIPv6Addr);

				memcpy( curIpInterfaceStruct.iPv6s[addressIdx].iPv6Address, curIPv6Addr, IPV6_ADDRESS_LEN );
		    	curIpInterfaceStruct.iPv6s[addressIdx].configurationType = (DWORD)(GetIpV6ConfigurationType());
	//			curIpInterfaceStruct.iPv6.ipV6SubnetMask = curSpan.GetIpV6SubnetMask();
	    	}

 			int numOfAliases = curSpan.GetAliasNamesNumber();
 		   	for (j=0; (j < numOfAliases) && (j < MAX_ALIAS_NAMES_NUM); j++)
    		{
    			if ( NULL != curSpan.GetAlias(i) )
    			{
	    			CH323Alias curAlias( *(curSpan.GetAlias(i)) );

	    			memset(curIpInterfaceStruct.aliasesList[j].aliasContent, 0, ALIAS_NAME_LEN);
				    memcpy(curIpInterfaceStruct.aliasesList[j].aliasContent, curAlias.GetAliasName(), ALIAS_NAME_LEN );
    			}
    		}
		}
		else
		{
			curIpInterfaceStruct.iPv4.iPv4Address = 0;
		}

    	ipParamsStruct.interfacesList[i] = curIpInterfaceStruct;
    }


	// ===== 3. DNS attributes
	DNS_CONFIGURATION_S curDnsConfigStruct;// = m_ipParamsStruct.dnsConfig;
	memset(&curDnsConfigStruct, 0, sizeof(DNS_CONFIGURATION_S));

	if ( NULL != GetpDns() )
	{
		CIpDns curDns( *(GetpDns()) );

		memset(curDnsConfigStruct.hostName,   0, NAME_LEN);
		memset(curDnsConfigStruct.domainName, 0, NAME_LEN);

		CIPSpan *firstSpan = GetFirstSpan();
		if(NULL != firstSpan)
		{
			strncpy((char*)(curDnsConfigStruct.hostName), firstSpan->GetSpanHostName().GetString(), NAME_LEN);
		}
	    strncpy((char*)(curDnsConfigStruct.domainName), curDns.GetDomainName().GetString(), NAME_LEN);

		curDnsConfigStruct.dnsServerStatus = (DWORD)(curDns.GetStatus());
		curDnsConfigStruct.isRegister      = (WORD)(curDns.GetRegisterDNSAutomatically());

    	char curIPv6Addr[IPV6_ADDRESS_LEN];
	    for (i=0; i < NUM_OF_DNS_SERVERS; i++)
	    {
	    	if ( -1 != curDns.GetIPv4Address(i) )
	    		curDnsConfigStruct.ipV4AddressList[i] = curDns.GetIPv4Address(i);

	    	memset(curIPv6Addr, 0, IPV6_ADDRESS_LEN);
	    	curDns.GetIPv6Address(i, curIPv6Addr);
			memcpy( curDnsConfigStruct.ipV6AddressList[i], curIPv6Addr, IPV6_ADDRESS_LEN );
	    }
	}
	else
	{
		memset(&curDnsConfigStruct, 0, sizeof(DNS_CONFIGURATION_S));
	}

	ipParamsStruct.dnsConfig = curDnsConfigStruct;


    // ===== 4. PORT_SPEED attributes
    memset(&ipParamsStruct.portSpeedList, 0, sizeof(ipParamsStruct.portSpeedList));
    CPortSpeed *pPortRange = GetPortSpeedVector();
    if(NULL != pPortRange)
    {
        for(int i = 0 ; i < MAX_NUM_OF_PORTS_SPEED ; i++)
        {
            ipParamsStruct.portSpeedList[i].portNum = pPortRange[i].GetNum();
            ipParamsStruct.portSpeedList[i].portSpeed = pPortRange[i].GetSpeed();
        }
    }

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
const CSmallString& CIPService::GetAltGatekeeperName()
{
    return m_AltGatekeeperName;
}

/////////////////////////////////////////////////////////////////////////////
WORD CIPService::IsAutoRegisterSpanHostName()
{
     return m_AutoRegisterSpanHostName;
}

/////////////////////////////////////////////////////////////////////////////
void CIPService::SetAutoRegisterSpanHostName(const WORD autoRegisterSpanHostName)
{
    m_AutoRegisterSpanHostName = autoRegisterSpanHostName ? TRUE : FALSE;
}
//////////////////////////////////////////////////////////////////////
eIpServiceType CIPService::GetIpServiceType () const
{
	return m_eIpServiceType;
}

//////////////////////////////////////////////////////////////////////
void CIPService::SetIpServiceType (const eIpServiceType serviceType)
{
	m_eIpServiceType = serviceType;
}

//////////////////////////////////////////////////////////////////////
void CIPService::SetGatekeeperMode(WORD mode)
{
    m_gatekeeperMode = mode;
}

//////////////////////////////////////////////////////////////////////
WORD CIPService::GetGatekeeperMode() const
{
    return m_gatekeeperMode;
}

//////////////////////////////////////////////////////////////////////
void CIPService::SetForwarding(WORD yesNo)
{
    m_forwarding = yesNo;
}

////////////////////////////////////////////////////////////////////////
WORD CIPService::IsForwarding() const
{
    return m_forwarding;
}


//////////////////////////////////////////////////////////////////////
void CIPService::SetRRQPolling(WORD yesNo)
{
    m_IsRRQPolling = yesNo;
}

//////////////////////////////////////////////////////////////////////
WORD CIPService::IsRRQPolling() const
{
    return m_IsRRQPolling;
}

//////////////////////////////////////////////////////////////////////
WORD CIPService::GetRRQPollingInterval() const
{
    return m_RRQPollingInterval;
}

//////////////////////////////////////////////////////////////////////
void CIPService::SetRRQPollingInterval(WORD seconds)
{
    m_RRQPollingInterval = seconds;
}

//////////////////////////////////////////////////////////////////////
void CIPService::SetPseudoGKListenPort(WORD portNum)
{
    m_pseudoGKListenPort = portNum;
}

//////////////////////////////////////////////////////////////////////
WORD CIPService::GetPseudoGKListenPort() const
{
    return m_pseudoGKListenPort;
}

//////////////////////////////////////////////////////////////////////
WORD CIPService::SpanChangeRequiresCardReset(const CIPSpan& newSpan) const
{
	CIPSpan* pSpan = NULL;
	for ( int i = 0; i < MAX_SPAN_NUMBER_IN_SERVICE ; i++)
		 if(newSpan.GetLineNumber() == m_pSpan[i]->GetLineNumber() )
		 {
			 pSpan = m_pSpan[i];
			 break;
		 }

	if(pSpan)
	{
	  if((pSpan->GetIPv4Address() != newSpan.GetIPv4Address()) ||
		 !( pSpan->GetPortRange()->IsEqual(*newSpan.GetPortRange()) ) )
			return TRUE;
	}
       return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
  WORD CIPService::ServiceChangeRequiresCardReset(const CIPService& newService) const
{
    // Cases in which hard reset is required
    if(m_netIPaddress            != newService.m_netIPaddress            ||
        m_netMask                 != newService.m_netMask                 ||
        m_defaultGatewayIPv4.ip   != newService.m_defaultGatewayIPv4.ip   ||
        memcmp(m_defaultGatewayIPv6.ip, newService.m_defaultGatewayIPv6.ip, IPV6_ADDRESS_BYTES_LEN)  ||
        m_defaultGatewayIPv6.scopeId != newService.m_defaultGatewayIPv6.scopeId  ||
        m_numb_of_routers         != newService.m_numb_of_routers         ||
        m_serviceTypeName         != newService.m_serviceTypeName         ||
        m_ATMEnableAAL5           != newService.m_ATMEnableAAL5           ||
        m_qualityOfService        != newService.m_qualityOfService        ||
        m_useDefaultARPServerIPOA != newService.m_useDefaultARPServerIPOA ||
        m_useDiscoveredLECS       != newService.m_useDiscoveredLECS       ||
        m_useLECSForLES           != newService.m_useLECSForLES           ||
        m_useAutoElanName         != newService.m_useAutoElanName		  ||
		m_eProtocolType			  != newService.m_eProtocolType			  ||
		m_AutoRegisterSpanHostName != newService.m_AutoRegisterSpanHostName )
        return TRUE;

	if ( m_pDns != NULL && m_pDns->DnsChangeRequiresCardReset(*(newService.m_pDns)))
		return TRUE;

	if ( m_eProtocolType != eIPProtocolType_H323 && m_pSecurity != NULL && ( ! ( *m_pSecurity == ( *newService.m_pSecurity)) ))
		return TRUE;


    for(int i = 0; i < m_numb_of_routers; i++)
    {
        if(m_pRouter[i] && newService.m_pRouter[i] )
        {
            if(m_pRouter[i]->GetRemoteFlag() != newService.m_pRouter[i]->GetRemoteFlag() ||
                m_pRouter[i]->GetRouterIP()  != newService.m_pRouter[i]->GetRouterIP()   ||
                m_pRouter[i]->GetRemoteIP()  != newService.m_pRouter[i]->GetRemoteIP()	)
                return TRUE;
        }
    }
    if(strcmp(m_elanName, newService.m_elanName) )
        return TRUE;

                //cases in which we just have to reset gatekeeper registration
                if(m_gatekeeper                != newService.m_gatekeeper                ||
                    m_GatekeeperDiskoveryPort   != newService.m_GatekeeperDiskoveryPort   ||
                    m_forwarding                != newService.m_forwarding                ||
                    m_gatekeeperMode            != newService.m_gatekeeperMode            ||
                    m_IsRRQPolling              != newService.m_IsRRQPolling              ||
                    m_RRQPollingInterval        != newService.m_RRQPollingInterval        ||
                    m_pseudoGKListenPort        != newService.m_pseudoGKListenPort        ||
                    m_dialIn_prefix.m_aliasName != newService.m_dialIn_prefix.m_aliasName ||
					m_GatekeeperName			!= newService.m_GatekeeperName			  ||
					m_AltGatekeeperName			!= newService.m_AltGatekeeperName		  )
                    return FALSE;

                return TRUE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
int CIPService::GetNumOfOccupiedSpans()
{
	int NumOfOccupiedSpans = 0;

	CIPSpan* pSpan = GetFirstSpan();	//for signaling - ignore the first span

	if (pSpan)
	{
		pSpan = GetNextSpan();

		while (pSpan)
		{
			DWORD ipV4 = pSpan->GetIPv4Address();
			char ipv6_string[IPV6_ADDRESS_LEN+1] = {0};
			pSpan->GetIPv6Address(0,ipv6_string);
			if (ipV4 != 0 || (0 != strlen(ipv6_string)  && 0 != strncmp(ipv6_string, "::", IPV6_ADDRESS_LEN)))
				NumOfOccupiedSpans++;

			pSpan = GetNextSpan();
		}
	}

	return NumOfOccupiedSpans;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CIPService::CalculateMaxNumOfCalls()
{
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();

	//In RMX1500 we have the 2 services on one card, and they share the card resources.
	//Therefore, the CS will define the maximum call per each service. (800 for each CS).

	if (eProductTypeRMX1500 == curProductType)
	{
		m_MaxNumOfCalls = 800;
	}
	//Judith: according to a meeting with Assaf, a card can't have more than 400 calls(max), therefore the calculation is:
	//every place the service appear, multiple the CS capabilities by 400. (23.12.2010)

	/*
	 * 12 Dec 2012, Rafi Fellert, JIRA issue #46
	 * For softMCUMfw we set m_MaxNumOfCalls to 600*GetNumOfOccupiedSpans()
	 */
	else if (eProductTypeSoftMCUMfw == curProductType)
	{
		m_MaxNumOfCalls = 1000*GetNumOfOccupiedSpans();
		if (m_MaxNumOfCalls > 1200 || 0 >= m_MaxNumOfCalls)
			m_MaxNumOfCalls = 1200;
	}
	else
	{
		m_MaxNumOfCalls = 400*GetNumOfOccupiedSpans();
		if (eProductTypeRMX2000 == curProductType || eProductFamilySoftMcu == curProductFamily)
		{
			if (m_MaxNumOfCalls > 800)
				m_MaxNumOfCalls = 800; //max abilities in RMX2000
		}
		else if (eProductTypeRMX4000 == curProductType)
		{
			if (m_MaxNumOfCalls > 1600)
				m_MaxNumOfCalls = 1600; //max abilities in RMX4000
		}
	}

	TRACESTR(eLevelInfoNormal) << "\nCIPService::CalculateNumOfCalls - For service "<<GetName()<<" the max number of calls is: "<<m_MaxNumOfCalls;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CIPService::GetMaxNumOfCalls()
{
	return m_MaxNumOfCalls;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CIPService::GetServiceDefaultType(const char* defaultH323service, const char* defaultSipService)
{
	BYTE isH323Default = FALSE;
	BYTE isSIPDefault = FALSE;

	if (strcmp(GetName(), defaultH323service)==0)
		isH323Default = TRUE;
	if (strcmp(GetName(), defaultSipService)==0)
		isSIPDefault = TRUE;

	if (isH323Default && isSIPDefault)
		return DEFAULT_SERVICE_BOTH;
	if (isH323Default)
		return DEFAULT_SERVICE_H323;
	if (isSIPDefault)
		return DEFAULT_SERVICE_SIP;

	return DEFAULT_SERVICE_NONE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CIPService::SetIsV35GwEnabled(BOOL bIsEnabled)
{
	m_isV35GwEnabled = bIsEnabled;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CIPService::GetIsV35GwEnabled()
{
	return m_isV35GwEnabled;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CIPService::SetV35GwIpAddress(DWORD ipAddress)
{
	m_V35GwIpAddress = ipAddress;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CIPService::GetV35GwIpAddress()
{
	return m_V35GwIpAddress;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CIPService::SetV35GwUsername(std::string sUsername)
{
	m_V35GwUsername = sUsername;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
const char* CIPService::GetV35GwUsername()
{
	return m_V35GwUsername.c_str();
}
void CIPService::SetV35GwPort(std::string sGwPort)
{
	m_V35GwPort = sGwPort;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
const char* CIPService::GetV35GwPort()
{
	return m_V35GwPort.c_str();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CIPService::SetV35GwAlias(std::string sGwAlias)
{
	m_V35GwAlias = sGwAlias;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
const char* CIPService::GetV35GwAlias()
{
	return m_V35GwAlias.c_str();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CIPService::SetV35GwPassword_dec(std::string sPassword)
{
	m_V35GwPassword_dec = sPassword;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
const char* CIPService::GetV35GwPassword_dec()
{
	return m_V35GwPassword_dec.c_str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CIPService::SetV35GwPassword_enc(std::string sPassword)
{
	m_V35GwPassword_enc = sPassword;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
const char* CIPService::GetV35GwPassword_enc()
{
	return m_V35GwPassword_enc.c_str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
int CIPService::GetV35GwPassword_dec_Length()
{
	return m_V35GwPassword_dec.length();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
int CIPService::GetV35GwPassword_enc_Length()
{
	return m_V35GwPassword_enc.length();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CIPService::IsV35ValidPortDefinition()
{
	CIPSpan* pSpan = GetFirstSpan();
	pSpan = GetNextSpan();

	WORD index = 1;
	while(pSpan)
	{
		if(0 != pSpan->GetIPv4Address())
		{
			TRACESTR(eLevelInfoNormal) << "\nCIPService::IsV35ValidPortDefinition - index: " << index << ", ip: " << pSpan->GetIPv4Address();
			//index 2,4,6,8 ==> board_id=1,2,3,4 ; sub_board_id=2
			//index 1,3,5,7 ==> board_id=1,2,3,4 ; sub_board_id=1
			div_t div_result = div(index, 2);
			if(div_result.rem == 1)				// This mean sub_board_id is one. We can't define ip address in V35 service in sub board ID 1
				return FALSE;
		}
		index++;
		pSpan = GetNextSpan();
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CIPService::UpdateServiceDefaults()
{
	TRACESTR(eLevelInfoNormal) << "CIPService::UpdateServiceDefaults";

	UpdateSystemMonitorOnlyFields();

	//Read ip address from first interface
	DWORD dword_ipAddress;
	string interface;
    string strIpV6_global;
    
	GetFirstAvailableIpAddressAndInterface(dword_ipAddress, interface, strIpV6_global);

	CIPSpan* pSpan = GetFirstSpan();
	pSpan->SetIPv4Address(dword_ipAddress);	//CS IP configuration
	pSpan->SetInterface(interface);

	pSpan = GetNextSpan();

	if(pSpan)
	{
		pSpan->SetIPv4Address(dword_ipAddress);	//Media IP configuration
		pSpan->SetInterface(interface);
	}
	else
		PASSERTMSG(1, "GetNextSpan return NULL");

	const eProductType curProductType 	= CProcessBase::GetProcess()->GetProductType();

	// in MFW there is only SIP supported
	if (eProductTypeSoftMCUMfw == curProductType && eIPProtocolType_SIP != m_eProtocolType)
		m_eProtocolType = eIPProtocolType_SIP;

	//Change Fixed port range
	pSpan = GetFirstSpan();
	while (pSpan)
	{
		CCommH323PortRange* portRange 		= pSpan->GetPortRange();
		/*
		 * 	12 Dec 2012, Rafi Fellert, JIRA issue #46
		 *  for softMCUMfw set the default TCP ports to 600.
		 */
		if (eProductTypeSoftMCUMfw == curProductType)
		{
			portRange->SetDefaultTcpPorts(1000);
			portRange->SetUdpPortRange(40000,10000);
		}
		else
		{
			portRange->SetDefaultTcpPorts(80);
		}

		pSpan = GetNextSpan();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CIPService::CompareServiceValues()
{
	BOOL bRaiseAA = FALSE;

    ///TODO::change to the first available interface netmask
	UpdateSystemMonitorOnlyFields();

    //Get CS Interface Name and IpAddress, the first IP Span
    char ipAddress[20];
	CIPSpan* pSpan = GetFirstSpan();
	SystemDWORDToIpString(pSpan->GetIPv4Address(), ipAddress);
	std::string str_ip_address = ipAddress;
	std::string interface = pSpan->GetInterface();
	DWORD dword_ipAddress = GetInterfaceIpAddress(interface, str_ip_address);     

	// for multiple network interface in MFW feature, Signal/media may use IPv6
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
    if (dword_ipAddress == 0 && eProductTypeSoftMCUMfw != curProductType)	//interface doesn't exist
		bRaiseAA = TRUE;

	if (dword_ipAddress != pSpan->GetIPv4Address())
	{
		pSpan->SetIPv4Address(dword_ipAddress);	//Update CS IP Address
	}
    
	if (interface != pSpan->GetInterface())
	{
		pSpan->SetInterface(interface);
	}

    //Get Media Interface Name and IpAddress, the second IP Span
    pSpan = GetNextSpan();
    if (pSpan !=NULL)
    {
    	SystemDWORDToIpString(pSpan->GetIPv4Address(), ipAddress);
    	str_ip_address = ipAddress;
    	interface = pSpan->GetInterface();
    	dword_ipAddress = GetInterfaceIpAddress(interface, str_ip_address);

    	if (dword_ipAddress == 0 && eProductTypeSoftMCUMfw != curProductType)
    		bRaiseAA = TRUE;

    	if (dword_ipAddress != pSpan->GetIPv4Address())
    	{
    		pSpan->SetIPv4Address(dword_ipAddress);	//Update Media IP Address
    	}

    	if (interface != pSpan->GetInterface())
    	{
    		pSpan->SetInterface(interface);
    	}
    }
    
	return bRaiseAA;
}

//1. get the first available interface, follow is the criteria: 
//   a. The interface should be UP and RUNNING
//   b. The interface should have a valid IP address (IPV4 preffered, IPV6 as second priority)
//   c. The interface should be an Ethernet interface, unless no such interface exists 
//   d. The interface should not be the loopback (lo) interface
void CIPService::GetFirstAvailableIpAddressAndInterface(DWORD& dword_ipAddress, string& interface, string& strIpV6_Global)
{
    TRACESTR(eLevelInfoNormal) << "CIPService::GetFirstAvailableIpAddressAndInterface in.";
    CSystemInterfaceList stCSystemInterfaceList;
    CSystemInterface* pFirstInterface = stCSystemInterfaceList.GetFirstAvailableInterface();

    if (NULL == pFirstInterface)
    {
        dword_ipAddress = 0;
        interface = "";
        TRACESTR(eLevelInfoNormal) << "CIPService::GetFirstAvailableIpAddressAndInterface no available interface.";
        return;
    }

    //MFW: IBM request to configure just eth-1
    //If DEFAULT_NETWORK_INTERFACE exist and contain a valid ip, use it.
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (curProductType == eProductTypeSoftMCUMfw)
	{
        string ip_address;
        string default_eth;
        CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();	
        sysConfig->GetDataByKey("DEFAULT_NETWORK_INTERFACE", default_eth);
        TRACESTR(eLevelInfoNormal) << "CIPService::GetFirstAvailableIpAddressAndInterface default_eth:" << default_eth;

        if ("" != default_eth)
        {
            CSystemInterface* pDefaultInterface = stCSystemInterfaceList.GetSystemInterfaceByName(default_eth);

            if (NULL != pDefaultInterface)
            {
                interface = pDefaultInterface->GetSystemInterfaceName();
                dword_ipAddress = pDefaultInterface->GetSystemInterfaceIp();
                strIpV6_Global  = pDefaultInterface->GetSystemInterfaceIpv6_global();
                if ((0 != dword_ipAddress) 
                    || (0 != strlen(strIpV6_Global.c_str())  && 0 == strncmp(strIpV6_Global.c_str(), "::", IPV6_ADDRESS_LEN)))
                {
                    TRACESTR(eLevelInfoNormal) << "CIPService::GetFirstAvailableIpAddressAndInterface MFW: use default interface:" << default_eth
                        << " ip_address:" << ip_address 
                        << " dword_ipAddress:" << dword_ipAddress
                        << " strIpV6_Global: " << strIpV6_Global;
                    return;
                }
            }
        }
	}

    interface = pFirstInterface->GetSystemInterfaceName();
    dword_ipAddress = pFirstInterface->GetSystemInterfaceIp();
    strIpV6_Global  = pFirstInterface->GetSystemInterfaceIpv6_global();
    
    TRACESTR(eLevelInfoNormal) << "CIPService::GetFirstAvailableIpAddressAndInterface first interface, nic_name:" << interface
        << " nic_type:"    << pFirstInterface->GetSystemInterfaceType()
        << " IpV4Dword:"   << dword_ipAddress
        << " IpV6Address:" << strIpV6_Global;

    return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CIPService::GetInterfaceIpAddress(std::string& interface, std::string ip_address)
{
    TRACESTR(eLevelInfoNormal) << "CIPService::GetInterfaceIpAddress interface:" << interface << " ip_address:" << ip_address;
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (curProductType==eProductTypeSoftMCUMfw || curProductType==eProductTypeSoftMCU || curProductType==eProductTypeEdgeAxis)
	{
		DWORD dword_ipAddress = 0;
        string strIpV6_global;
		char ipAddressStr[IP_ADDRESS_LEN] = {0};

        //MFW, EdgeAxis: no need re-get the interface, just return.
        if (curProductType == eProductTypeSoftMCUMfw || curProductType == eProductTypeEdgeAxis)
        {
            dword_ipAddress = SystemIpStringToDWORD(ip_address.c_str());
            TRACESTR(eLevelInfoNormal) << "CIPService::GetInterfaceIpAddress dword_ipAddress:" << dword_ipAddress << " interface:" << interface;
            return dword_ipAddress;
        }
        
		GetFirstAvailableIpAddressAndInterface(dword_ipAddress, interface, strIpV6_global);
		if(dword_ipAddress!=0)
		    SystemDWORDToIpString(dword_ipAddress,ipAddressStr);
        TRACESTR(eLevelInfoNormal) << "CIPService::GetInterfaceIpAddress dword_ipAddress:" << dword_ipAddress;

        return dword_ipAddress;
	}

	//if the interface is empty (maybe the first time we are here after upgrade) - see if the ip address exist in the system
	if (interface=="")
	{
		std::string command = "echo -n `/sbin/ifconfig | grep "+ip_address+" -B1 | grep 'eth[0-9]' | cut -d' ' -f1`";
		SystemPipedCommand(command.c_str(), interface);

		if (interface=="")
			return 0;
	}
	else
	{
		std::string command = "echo -n `/sbin/ifconfig | grep -A1 " + interface + " | grep 'inet addr' | cut -d':' -f2 | cut -d' ' -f1`";

		SystemPipedCommand(command.c_str(), ip_address);
		if (ip_address == "")	//interface doesn't exist
			return 0;
	}

	return SystemIpStringToDWORD(ip_address.c_str());

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CIPService::UpdateSystemMonitorOnlyFields()
{
	string ipv4_defGw;

	//Read default gw
	SystemPipedCommand("echo -n `/sbin/ip route show | grep default | cut -d' ' -f3`", ipv4_defGw);
	DWORD dword_defaultGateway = SystemIpStringToDWORD(ipv4_defGw.c_str());
	SetDefaultGatewayIPv4(dword_defaultGateway);
	TRACESTR(eLevelInfoNormal) << "CIPService::UpdateSystemMonitorOnlyFields default gw: " << ipv4_defGw;

	//Read net mask from first interface
	string net_mask;
	//SystemPipedCommand("echo -n `/sbin/ifconfig | grep -A1 'eth[0-9]' | grep Mask | cut -d':' -f4`", net_mask);
    SystemPipedCommand("echo -n `/sbin/ifconfig | grep -v lo | grep 'Mask' | cut -d':' -f4 | awk 'NR==1'`", net_mask);
    DWORD dword_netMask = SystemIpStringToDWORD(net_mask.c_str());
	SetNetMask(dword_netMask);
	TRACESTR(eLevelInfoNormal) << "CIPService::UpdateSystemMonitorOnlyFields net mask: " << net_mask;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// CCommH323PortRange Class
////////////////////////////////////////////////////////////////////////////////////////////////////
CCommH323PortRange::CCommH323PortRange()
{
    m_dynamicPortAllocation = TRUE;
    m_signallingFirstPort   = 0;
    m_signallingNumPorts    = 0;
    m_controlFirstPort      = 0;
    m_controlNumPorts       = 0;
    m_audioFirstPort        = 0;
    m_audioNumPorts         = 0;
    m_videoFirstPort        = 0;
    m_videoNumPorts         = 0;
    m_contentFirstPort      = 0;
    m_contentNumPorts       = 0;
	m_feccFirstPort			= 0;
	m_feccNumPorts			= 0;
    m_numIntendedCalls      = 0;
    m_enablePortRange       = FALSE;

    m_TcpFirstPort 			= 0;
    m_TcpNumPorts 			= 0;
    m_UdpFirstPort 			= 0;
    m_UdpNumPorts 			= 0;
}

CCommH323PortRange::CCommH323PortRange(const CCommH323PortRange& other)
:CPObject(other)
{
    m_dynamicPortAllocation = other.m_dynamicPortAllocation;
    m_signallingFirstPort   = other.m_signallingFirstPort;
    m_signallingNumPorts    = other.m_signallingNumPorts;
    m_controlFirstPort      = other.m_controlFirstPort;
    m_controlNumPorts       = other.m_controlNumPorts;
    m_audioFirstPort        = other.m_audioFirstPort;
    m_audioNumPorts         = other.m_audioNumPorts;
    m_videoFirstPort        = other.m_videoFirstPort;
    m_videoNumPorts         = other.m_videoNumPorts;
    m_contentFirstPort      = other.m_contentFirstPort;
    m_contentNumPorts       = other.m_contentNumPorts;
	m_feccFirstPort			= other.m_feccFirstPort;
	m_feccNumPorts			= other.m_feccNumPorts;
    m_numIntendedCalls      = other.m_numIntendedCalls;
    m_enablePortRange       = other.m_enablePortRange;

    m_TcpFirstPort 			= other.m_TcpFirstPort;
    m_TcpNumPorts 			= other.m_TcpNumPorts;
    m_UdpFirstPort 			= other.m_UdpFirstPort;
    m_UdpNumPorts 			= other.m_UdpNumPorts;
}

/////////////////////////////////////////////////////////////////////////////
CCommH323PortRange& CCommH323PortRange::operator=(const CCommH323PortRange& other)
{
    m_dynamicPortAllocation = other.m_dynamicPortAllocation;
    m_signallingFirstPort   = other.m_signallingFirstPort;
    m_signallingNumPorts    = other.m_signallingNumPorts;
    m_controlFirstPort      = other.m_controlFirstPort;
    m_controlNumPorts       = other.m_controlNumPorts;
    m_audioFirstPort        = other.m_audioFirstPort;
    m_audioNumPorts         = other.m_audioNumPorts;
    m_videoFirstPort        = other.m_videoFirstPort;
    m_videoNumPorts         = other.m_videoNumPorts;
    m_contentFirstPort      = other.m_contentFirstPort;
    m_contentNumPorts       = other.m_contentNumPorts;
	m_feccFirstPort			= other.m_feccFirstPort;
	m_feccNumPorts			= other.m_feccNumPorts;
    m_numIntendedCalls      = other.m_numIntendedCalls;
    m_enablePortRange       = other.m_enablePortRange;

    m_TcpFirstPort 			= other.m_TcpFirstPort;
    m_TcpNumPorts 			= other.m_TcpNumPorts;
    m_UdpFirstPort 			= other.m_UdpFirstPort;
    m_UdpNumPorts 			= other.m_UdpNumPorts;

    return *this;
}

//////////////////////////////////////////////////////////////////////
CCommH323PortRange::~CCommH323PortRange()
{
}

//////////////////////////////////////////////////////////////////////
bool CCommH323PortRange::operator==(const CCommH323PortRange &rHnd)const
{
	if(this == &rHnd)
		return true;
	FTRACESTR(eLevelInfoNormal) << " BRIDGE-8284 compare PortRange" ;
	if ( m_dynamicPortAllocation != rHnd.m_dynamicPortAllocation )
		return false;

	if ( m_signallingFirstPort != rHnd.m_signallingFirstPort )
		return false;

	if ( m_signallingNumPorts != rHnd.m_signallingNumPorts )
		return false;

	if ( m_controlFirstPort != rHnd.m_controlFirstPort )
		return false;

	if ( m_controlNumPorts != rHnd.m_controlNumPorts )
		return false;

	if ( m_audioFirstPort != rHnd.m_audioFirstPort )
		return false;

	if ( m_audioNumPorts != rHnd.m_audioNumPorts )
		return false;

	if ( m_videoFirstPort != rHnd.m_videoFirstPort )
		return false;

	if ( m_videoNumPorts != rHnd.m_videoNumPorts )
		return false;

	if ( m_contentFirstPort != rHnd.m_contentFirstPort )
		return false;

	if ( m_contentNumPorts != rHnd.m_contentNumPorts )
		return false;

	if ( m_feccFirstPort != rHnd.m_feccFirstPort )
		return false;

	if ( m_feccNumPorts != rHnd.m_feccNumPorts )
		return false;

	if ( m_numIntendedCalls != rHnd.m_numIntendedCalls )
		return false;

	if ( m_enablePortRange != rHnd.m_enablePortRange )
		return false;

	if ( m_TcpFirstPort != rHnd.m_TcpFirstPort )
		return false;

	if ( m_TcpNumPorts != rHnd.m_TcpNumPorts )
		return false;
	//BRIDGE-8284
	if(m_enablePortRange || rHnd.m_enablePortRange )
	{

		if ( m_UdpFirstPort != rHnd.m_UdpFirstPort )
			return false;

		if ( m_UdpNumPorts != rHnd.m_UdpNumPorts )
			return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CCommH323PortRange::operator!=(const CCommH323PortRange& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
void CCommH323PortRange::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pH323PortRangeNode = pFatherNode->AddChildNode("PORT_RANGE");

	pH323PortRangeNode->AddChildNode("IS_PORT_RANGE"			, m_enablePortRange			, _BOOL);
//	pH323PortRangeNode->AddChildNode("DYNAMIC_PORT_ALLOCTION"	, m_dynamicPortAllocation	, _BOOL);

	WORD tcpLastPort = m_TcpFirstPort + m_TcpNumPorts;
	pH323PortRangeNode->AddChildNode("TCP_FIRST_PORT"			, m_TcpFirstPort			, _0_TO_WORD);
	pH323PortRangeNode->AddChildNode("TCP_LAST_PORT"			, tcpLastPort				, _0_TO_WORD);

	WORD udpLastPort = m_UdpFirstPort + m_UdpNumPorts;
	pH323PortRangeNode->AddChildNode("UDP_FIRST_PORT"			, m_UdpFirstPort			, _0_TO_WORD);
	pH323PortRangeNode->AddChildNode("UDP_LAST_PORT"			, udpLastPort				, _0_TO_WORD);
}

//////////////////////////////////////////////////////////////////////
int	CCommH323PortRange::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	GET_VALIDATE_CHILD(pActionNode,"IS_PORT_RANGE"			, &m_enablePortRange		,_BOOL);
//	GET_VALIDATE_CHILD(pActionNode,"DYNAMIC_PORT_ALLOCTION"	, &m_dynamicPortAllocation	,_BOOL);

	GET_VALIDATE_CHILD(pActionNode,"TCP_FIRST_PORT"			, &m_TcpFirstPort	,_0_TO_WORD);
	WORD tcpLastPort = 0;
	GET_VALIDATE_CHILD(pActionNode,"TCP_LAST_PORT"			, &tcpLastPort		,_0_TO_WORD);
	m_TcpNumPorts = tcpLastPort - m_TcpFirstPort;

	GET_VALIDATE_CHILD(pActionNode,"UDP_FIRST_PORT"			, &m_UdpFirstPort	,_0_TO_WORD);
	WORD udpLastPort = 0;
	GET_VALIDATE_CHILD(pActionNode,"UDP_LAST_PORT"			, &udpLastPort		,_0_TO_WORD);
	m_UdpNumPorts = udpLastPort - m_UdpFirstPort;

    if(FALSE == m_enablePortRange) // the port range is not fixed by user
    {
        SetDefaultUdpPorts();
    }
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
void CCommH323PortRange::SetDynamicPortAllocation(WORD yesNo)
{
    m_dynamicPortAllocation = yesNo;
}

//////////////////////////////////////////////////////////////////////
WORD CCommH323PortRange::IsDynamicPortAllocation() const
{
    return m_dynamicPortAllocation;
}

//////////////////////////////////////////////////////////////////////
void CCommH323PortRange::SetSignallingPortRange(WORD firstPort, WORD numPorts)
{
    m_signallingFirstPort = firstPort;
    m_signallingNumPorts  = numPorts;
}

//////////////////////////////////////////////////////////////////////
void CCommH323PortRange::SetControlPortRange(WORD firstPort, WORD numPorts)
{
    m_controlFirstPort = firstPort;
    m_controlNumPorts  = numPorts;
}

//////////////////////////////////////////////////////////////////////
void CCommH323PortRange::SetAudioPortRange(WORD firstPort, WORD numPorts)
{

    m_audioFirstPort = firstPort;
    m_audioNumPorts  = numPorts;
}

//////////////////////////////////////////////////////////////////////
void CCommH323PortRange::SetVideoPortRange(WORD firstPort, WORD numPorts)
{
    m_videoFirstPort = firstPort;
    m_videoNumPorts  = numPorts;
}

//////////////////////////////////////////////////////////////////////
void CCommH323PortRange::SetContentPortRange(WORD firstPort, WORD numPorts)
{
    m_contentFirstPort = firstPort;
    m_contentNumPorts  = numPorts;
}

//////////////////////////////////////////////////////////////////////
void CCommH323PortRange::SetFeccPortRange(WORD firstPort, WORD numPorts)
{
    m_feccFirstPort = firstPort;
    m_feccNumPorts  = numPorts;
}

//////////////////////////////////////////////////////////////////////
WORD CCommH323PortRange::GetSignallingFirstPort() const
{
    return	m_signallingFirstPort ;
}

//////////////////////////////////////////////////////////////////////
WORD CCommH323PortRange::GetVideoFirstPort() const
{
    return m_videoFirstPort;
}

//////////////////////////////////////////////////////////////////////
WORD CCommH323PortRange::GetControlFirstPort() const
{
    return m_controlFirstPort;
}

//////////////////////////////////////////////////////////////////////
WORD CCommH323PortRange::GetFeccFirstPort() const
{
    return m_feccFirstPort;
}

//////////////////////////////////////////////////////////////////////
WORD CCommH323PortRange::GetContentFirstPort() const
{
    return m_contentFirstPort;
}


//////////////////////////////////////////////////////////////////////
WORD CCommH323PortRange::GetAudioFirstPort() const
{
    return m_audioFirstPort;
}

//////////////////////////////////////////////////////////////////////
WORD CCommH323PortRange::GetSignallingNumberOfPorts() const
{
    return m_signallingNumPorts;
}
//////////////////////////////////////////////////////////////////////

WORD CCommH323PortRange::GetVideoNumberOfPorts() const
{
    return m_videoNumPorts;
}
//////////////////////////////////////////////////////////////////////

WORD CCommH323PortRange::GetControlNumberOfPorts() const
{
    return m_controlNumPorts;
}
//////////////////////////////////////////////////////////////////////

WORD CCommH323PortRange::GetContentNumberOfPorts() const
{
    return m_contentNumPorts;
}
//////////////////////////////////////////////////////////////////////

WORD CCommH323PortRange::GetAudioNumberOfPorts() const
{
    return m_audioNumPorts;
}

//////////////////////////////////////////////////////////////////////
WORD CCommH323PortRange::GetFeccNumberOfPorts() const
{
    return m_feccNumPorts;
}


//////////////////////////////////////////////////////////////////////
void CCommH323PortRange::SetNumIntendedCallsOnCard(WORD number)
{
    m_numIntendedCalls = number;
}
//////////////////////////////////////////////////////////////////////

WORD CCommH323PortRange::GetNumIntendedCallsOnCard() const
{
    return m_numIntendedCalls;
}
//////////////////////////////////////////////////////////////////////

void CCommH323PortRange::SetEnablePortRange(WORD yesNo)
{
    m_enablePortRange = yesNo;
}
//////////////////////////////////////////////////////////////////////

WORD CCommH323PortRange::IsEnabledPortRange() const
{
    return m_enablePortRange;
}

//////////////////////////////////////////////////////////////////////
#define IN_RANGE(a,startRange,Rangesize) ( (a)>=(startRange)&& (a)<=((startRange)+(Rangesize)) )
ESTATUS CCommH323PortRange::TestValidity() const
{
    if(IsEnabledPortRange() )
    {

        //make sure values are in range 1025 - 62999
        if(m_signallingFirstPort < 1025 ||
            m_controlFirstPort	  < 1025 ||
            m_audioFirstPort	  < 1025 ||
            m_videoFirstPort	  < 1025 ||
            m_contentFirstPort	  < 1025 ||
			m_feccFirstPort	      < 1025 )
            return STATUS_ILLEGAL_PORT_NUMBER_BELOW_1025;

        if(m_signallingFirstPort +  m_signallingNumPorts > 62999 ||
            m_controlFirstPort    +  m_controlNumPorts	  > 62999 ||
            m_audioFirstPort      +	 m_audioNumPorts      > 62999 ||
            m_videoFirstPort      +	 m_videoNumPorts      > 62999 ||
            m_contentFirstPort	  +  m_contentNumPorts    > 62999 ||
			m_feccFirstPort       +	 m_feccNumPorts       > 62999 )
            return STATUS_ILLEGAL_PORT_NUMBER_ABOVE_62999;

        //Ports 1503, 1719, 1720 are resreved
        if(	IN_RANGE(1503, m_signallingFirstPort,  m_signallingNumPorts) ||
            IN_RANGE(1503, m_controlFirstPort,m_controlNumPorts) ||
            IN_RANGE(1503, m_audioFirstPort,m_audioNumPorts) ||
            IN_RANGE(1503, m_videoFirstPort,m_videoNumPorts) ||
            IN_RANGE(1503, m_contentFirstPort,m_contentNumPorts) ||
			IN_RANGE(1503, m_feccFirstPort,m_feccNumPorts) )
            return STATUS_ILLEGAL_RESERVED_PORT_IN_RANGE;

        if(IN_RANGE(1719, m_signallingFirstPort,  m_signallingNumPorts) ||
            IN_RANGE(1719, m_controlFirstPort,m_controlNumPorts) ||
            IN_RANGE(1719, m_audioFirstPort,m_audioNumPorts) ||
            IN_RANGE(1719, m_videoFirstPort,m_videoNumPorts) ||
            IN_RANGE(1719, m_contentFirstPort,m_contentNumPorts) ||
			IN_RANGE(1719, m_feccFirstPort,m_feccNumPorts) )
            return STATUS_ILLEGAL_RESERVED_PORT_IN_RANGE;

        if(IN_RANGE(1720, m_signallingFirstPort,  m_signallingNumPorts) ||
            IN_RANGE(1720, m_controlFirstPort,m_controlNumPorts) ||
            IN_RANGE(1720, m_audioFirstPort,m_audioNumPorts) ||
            IN_RANGE(1720, m_videoFirstPort,m_videoNumPorts) ||
            IN_RANGE(1720, m_contentFirstPort,m_contentNumPorts) ||
			IN_RANGE(1720, m_feccFirstPort,m_feccNumPorts) )
            return STATUS_ILLEGAL_RESERVED_PORT_IN_RANGE;

        // AUDIO, video content and FECC first port range must be even
        if(m_audioFirstPort % 2 != 0 )
            return  STATUS_ILLEGAL_AUDIO_PORT_ALLOCATION;
        if(m_videoFirstPort % 2 != 0 )
            return  STATUS_ILLEGAL_VIDEO_PORT_ALLOCATION;
        if(m_feccFirstPort % 2 != 0 )
            return  STATUS_ILLEGAL_FECC_PORT_ALLOCATION;
        if(m_contentFirstPort % 2 != 0 )
            return  STATUS_ILLEGAL_CONTENT_PORT_ALLOCATION;


        if(!m_dynamicPortAllocation &&
            (   m_signallingNumPorts < NUM_SIGNALLING_PORTS_PER_PARTY ||
            m_controlNumPorts	  < NUM_CONTROL_PORTS_PER_PARTY ||
            m_audioNumPorts      < NUM_AUDIO_PORTS_PER_PARTY ||
            m_videoNumPorts      < NUM_VIDEO_PORTS_PER_PARTY ||
            m_contentNumPorts    < NUM_CONTENT_PORTS_PER_PARTY ||
			m_feccNumPorts       < NUM_FECC_PORTS_PER_PARTY ) )
            return STATUS_THE_NUMBER_OF_PORTS_ALLOCATED_IS_BELOW_THE_MINIMUM;
    }

    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
WORD CCommH323PortRange::IsEqual(CCommH323PortRange& other)
{
	if((m_enablePortRange       == other.m_enablePortRange) &&
		(m_signallingNumPorts    == other.m_signallingNumPorts) &&
		(m_controlNumPorts       == other.m_controlNumPorts) &&
		(m_audioNumPorts         == other.m_audioNumPorts) &&
		(m_videoNumPorts         == other.m_videoNumPorts) &&
		(m_contentNumPorts       == other.m_contentNumPorts) &&
		(m_feccNumPorts          == other.m_feccNumPorts) &&
		(m_signallingFirstPort   == other.m_signallingFirstPort) &&
		(m_controlFirstPort      == other.m_controlFirstPort) &&
		(m_audioFirstPort        == other.m_audioFirstPort) &&
		(m_videoFirstPort        == other.m_videoFirstPort) &&
		(m_contentFirstPort      == other.m_contentFirstPort) &&
		(m_feccFirstPort         == other.m_feccFirstPort) &&
		(m_dynamicPortAllocation == other.m_dynamicPortAllocation) &&
		(m_numIntendedCalls      == other.m_numIntendedCalls) )
		return TRUE;
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
void CCommH323PortRange::SetDefaultUdpPorts()
{
	m_UdpFirstPort = 0;
    m_UdpNumPorts  = 0;
}

//////////////////////////////////////////////////////////////////////
void CCommH323PortRange::SetDefaultTcpPorts(DWORD maxPartiesNum)
{
	m_TcpFirstPort = FIRST_TCP_PORT;
    m_TcpNumPorts  = maxPartiesNum * 2;
}

//////////////////////////////////////////////////////////////////////
WORD CCommH323PortRange::GetTcpNumberOfPorts() const
{
	return m_TcpNumPorts;
}

//////////////////////////////////////////////////////////////////////
WORD CCommH323PortRange::GetTcpFirstPort() const
{
	return m_TcpFirstPort;
}

//////////////////////////////////////////////////////////////////////
void CCommH323PortRange::SetUdpPortRange(WORD firstPort, WORD numPorts)
{
	m_UdpFirstPort = firstPort;
    m_UdpNumPorts = numPorts;
}

//////////////////////////////////////////////////////////////////////
WORD CCommH323PortRange::GetUdpNumberOfPorts() const
{
	return m_UdpNumPorts;
}

//////////////////////////////////////////////////////////////////////
WORD CCommH323PortRange::GetUdpFirstPort() const
{
	return m_UdpFirstPort;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//									Class CIpNat
//////////////////////////////////////////////////////////////////////////////////////////////////
CIpNat::CIpNat()
{
	m_status            = eServerStatusOff;
    m_ExternalIpAddress = 0;
}

////////////////////////////////////////////////////////////////////////////
CIpNat::CIpNat( const  CIpNat &other)
:CPObject(other)
{
    m_status            = other.m_status;
    m_ExternalIpAddress = other.m_ExternalIpAddress;
}
/////////////////////////////////////////////////////////////////////////////

CIpNat& CIpNat::operator=(const CIpNat& other)
{
    m_status            = other.m_status;
    m_ExternalIpAddress = other.m_ExternalIpAddress;
    return *this;
}
////////////////////////////////////////////////////////////////////////////
bool CIpNat::operator==(const CIpNat &rHnd)const
{
	if(this == &rHnd)
		return true;

	if ( m_status != rHnd.m_status )
		return false;

	if ( m_ExternalIpAddress != rHnd.m_ExternalIpAddress )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CIpNat::operator!=(const CIpNat& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
CIpNat::~CIpNat()
{
}

/////////////////////////////////////////////////////////////////////////////
void CIpNat::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pNatNode = pFatherNode->AddChildNode("NAT");

	pNatNode->AddChildNode("NAT_TRAVERSAL_EXTERNAL_ADDRESS_STATUS", m_status, SERVER_STATUS_ENUM);
	pNatNode->AddChildNode("NAT_TRAVERSAL_IP_ADDRESS", m_ExternalIpAddress,IP_ADDRESS);
}
/////////////////////////////////////////////////////////////////////////////
int  CIpNat::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	BYTE tmp =0;
	GET_VALIDATE_CHILD(pActionNode,"NAT_TRAVERSAL_EXTERNAL_ADDRESS_STATUS",&tmp,SERVER_STATUS_ENUM);
	m_status = (eServerStatus)tmp;
	GET_VALIDATE_CHILD(pActionNode,"NAT_TRAVERSAL_IP_ADDRESS",&m_ExternalIpAddress,IP_ADDRESS);

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
eServerStatus CIpNat::GetStatus() const
{
    return m_status;
}

/////////////////////////////////////////////////////////////////////////////
void  CIpNat::SetStatus(const eServerStatus status)
{
    m_status = status;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CIpNat::GetExternalIpAddress() const
{
    return m_ExternalIpAddress;
}

/////////////////////////////////////////////////////////////////////////////
void  CIpNat::SetExternalIpAddress(const DWORD externalIpAddress)
{
    m_ExternalIpAddress = externalIpAddress;
}


ESTATUS CIpNat::TestValidity()
{
    DWORD status = STATUS_OK;

	if (m_status != eServerStatusSpecify &&
		m_status != eServerStatusAuto &&
		m_status != eServerStatusOff)
	{
		status = STATUS_UNKNOWN_NAT_STATUS;
	}

	if (status == STATUS_OK && m_status == eServerStatusSpecify)
	{
		if(m_ExternalIpAddress == 0 ||
			m_ExternalIpAddress == 0xFFFFFFFF)
		{
			status = STATUS_INVALID_NAT_IP_ADDRESS;
		}
	}
    return status;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//				class CIpDns
////////////////////////////////////////////////////////////////////////////////////////////////
CIpDns::CIpDns()
:m_domain_name(),
 m_host_Service_Name(),
 m_prefix()
{
    m_status                          = eServerStatusOff;
    ind_of_numb_of_dns_servers        = 0;

    for (int i=0; i<NUM_OF_DNS_SERVERS;i++)
    {
        m_DNS_serversIPv4address[i].ip		= 0;

        memset(m_DNS_serversIPv6address[i].ip, 0, IPV6_ADDRESS_BYTES_LEN);
    	//m_DNS_serversIPv6address[i].scopeId	= eScopeIdOther;//eScopeIdSite;	//Judith - for empty other there is no scope.
        m_DNS_serversIPv6address[i].scopeId	= eScopeIdGlobal;
    	m_DNS_serversIPv6MaskArray[i]		= 64;
    }

	m_register_DNS_name_automatically = FALSE;
    m_accept_calls_via_DNS            = FALSE;
    m_enable                          = FALSE;
}

//////////////////////////////////////////////////////////////////////////
CIpDns::CIpDns( const  CIpDns &other)
:CPObject(other),
m_domain_name(other.m_domain_name),
 m_host_Service_Name(other.m_host_Service_Name),
 m_prefix(other.m_prefix)
{
    m_status                   = other.m_status;
	ind_of_numb_of_dns_servers = 0;

    for (int i=0; i<NUM_OF_DNS_SERVERS;i++)
    {
        memcpy( &(m_DNS_serversIPv4address[i]), &(other.m_DNS_serversIPv4address[i]), sizeof(ipAddressV4If) );
    	memcpy( &(m_DNS_serversIPv6address[i]), &(other.m_DNS_serversIPv6address[i]), sizeof(ipAddressV6If) );
    	m_DNS_serversIPv6MaskArray[i] = other.m_DNS_serversIPv6MaskArray[i];
    }

    m_register_DNS_name_automatically = other.m_register_DNS_name_automatically;
    m_accept_calls_via_DNS            = other.m_accept_calls_via_DNS;
    m_enable                          = other.m_enable;
}
/////////////////////////////////////////////////////////////////////////////

CIpDns& CIpDns::operator=(const CIpDns& other)
{
    m_status = other.m_status;

    for (int i=0; i<NUM_OF_DNS_SERVERS;i++)
    {
       memcpy( &(m_DNS_serversIPv4address[i]), &(other.m_DNS_serversIPv4address[i]), sizeof(ipAddressV4If) );
       memcpy( &(m_DNS_serversIPv6address[i]), &(other.m_DNS_serversIPv6address[i]), sizeof(ipAddressV6If) );
       m_DNS_serversIPv6MaskArray[i] = other.m_DNS_serversIPv6MaskArray[i];
    }

    m_domain_name                     = other.m_domain_name;
    m_host_Service_Name               = other.m_host_Service_Name;
    m_prefix                          = other.m_prefix;
    m_register_DNS_name_automatically = other.m_register_DNS_name_automatically;
    m_accept_calls_via_DNS            = other.m_accept_calls_via_DNS;
    m_enable                          = other.m_enable;
	ind_of_numb_of_dns_servers        = 0;

    return *this;
}
////////////////////////////////////////////////////////////////////////////
bool CIpDns::operator==(const CIpDns &rHnd)const
{
	if(this == &rHnd)
		return true;

	if (m_status != rHnd.m_status)
		return false;
	char szIP[IPV6_ADDRESS_LEN];
	eIPv6AddressScope curV6AddressScope = eIPv6AddressScope_other;
	for (int i=0; i<NUM_OF_DNS_SERVERS;i++)
	{
		/*FTRACESTR(eLevelInfoNormal) << "CIpDns::operator== - check Ipv4 Ip";
		::ipV4ToString(m_DNS_serversIPv4address[i].ip,szIP);
		std::string sIpv4m(szIP);
		FTRACESTR(eLevelInfoNormal) << "CIpDns::operator== -m_DNS_serversIPv4address = " << sIpv4m.c_str();
		::ipV4ToString(rHnd.m_DNS_serversIPv4address[i].ip,szIP);
		std::string sIpv4r(szIP);
		FTRACESTR(eLevelInfoNormal) << "CIpDns::operator== -rHnd.m_DNS_serversIPv4address = " << sIpv4r.c_str();*/
		if (memcmp (&m_DNS_serversIPv4address[i], &(rHnd.m_DNS_serversIPv4address[i]), sizeof(ipAddressV4If)) != 0)
			return false;

		FTRACESTR(eLevelInfoNormal) << "CIpDns::operator== - check Ipv6 Ip";
		::ipV6ToString(m_DNS_serversIPv6address[i].ip, szIP, FALSE);
		std::string sIpv6(szIP);
		FTRACESTR(eLevelInfoNormal) << "CIpDns::operator== -m_DNS_serversIPv6address[i].ip = " << sIpv6.c_str();

		::ipV6ToString(rHnd.m_DNS_serversIPv6address[i].ip, szIP, FALSE);
		std::string s2Ipv6(szIP);
		FTRACESTR(eLevelInfoNormal) << "CIpDns::operator== -rHnd.m_DNS_serversIPv6address[i] = " << s2Ipv6.c_str();
		if(sIpv6 != s2Ipv6)
			return false;
		FTRACESTR(eLevelInfoNormal) << "CIpDns::operator== - check Ipv6 scopeId";
		curV6AddressScope = ConvertenScopeIdToeIpv6AddressScope((enScopeId)m_DNS_serversIPv6address[i].scopeId);
		const char* curV6AddressScopeStr = GetIPv6AddressScopeStr(curV6AddressScope);
		FTRACESTR(eLevelInfoNormal) << "CIpDns::operator==m_DNS_serversIPv6address[i].scopeId "<<curV6AddressScopeStr;
		curV6AddressScope = ConvertenScopeIdToeIpv6AddressScope((enScopeId)rHnd.m_DNS_serversIPv6address[i].scopeId);
		curV6AddressScopeStr = GetIPv6AddressScopeStr(curV6AddressScope);
		FTRACESTR(eLevelInfoNormal) << "CIpDns::operator==rHnd.m_DNS_serversIPv6address[i].scopeId "<<curV6AddressScopeStr;
		if(m_DNS_serversIPv6address[i].scopeId != rHnd.m_DNS_serversIPv6address[i].scopeId)
			return false;
		//if (memcmp (&m_DNS_serversIPv6address[i], &(rHnd.m_DNS_serversIPv6address[i]), sizeof(ipAddressV6If)) != 0)
		//	return false;

		if (m_DNS_serversIPv6MaskArray[i] != rHnd.m_DNS_serversIPv6MaskArray[i])
			return false;
	}

	if ( m_domain_name != rHnd.m_domain_name )
		return false;

	if ( m_host_Service_Name != rHnd.m_host_Service_Name )
		return false;

	if ( m_prefix != rHnd.m_prefix )
		return false;

	if ( m_register_DNS_name_automatically != rHnd.m_register_DNS_name_automatically )
		return false;

	if ( m_accept_calls_via_DNS != rHnd.m_accept_calls_via_DNS )
		return false;

	if ( m_enable != rHnd.m_enable )
		return false;

	if ( ind_of_numb_of_dns_servers != rHnd.ind_of_numb_of_dns_servers )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CIpDns::operator!=(const CIpDns& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
CIpDns::~CIpDns()
{
}

/////////////////////////////////////////////////////////////////////////////
void CIpDns::SerializeXml(CXMLDOMElement* pFatherNode)
{
	int i=0;

	CXMLDOMElement* pDnsNode = pFatherNode->AddChildNode("DNS");
	pDnsNode->AddChildNode("DNS_STATUS", m_status, SERVER_STATUS_ENUM);
	pDnsNode->AddChildNode("IP_PREFIX", m_prefix);
	pDnsNode->AddChildNode("REGISTER_DNS_NAME_AUTOMATICALLY", m_register_DNS_name_automatically, _BOOL);
	CXMLDOMElement* pIpList = pDnsNode->AddChildNode("SERVERS_IP_LIST");
	for (i=0; i<NUM_OF_DNS_SERVERS;i++)
	{
		pIpList->AddChildNode("IP", m_DNS_serversIPv4address[i].ip, IP_ADDRESS);
	}
	pDnsNode->AddChildNode("DOMAIN_NAME", m_domain_name);
	pDnsNode->AddChildNode("ACCEPT_CALLS_VIA_DNS", m_accept_calls_via_DNS);
	pDnsNode->AddChildNode("ENABLE", m_enable, _BOOL);

	char curV6Mask[IPV6_ADDRESS_LEN], szIP[IPV6_ADDRESS_LEN];
	eIPv6AddressScope curV6AddressScope = eIPv6AddressScope_other;

	CXMLDOMElement* pServersIPv6ListNode = pDnsNode->AddChildNode("SERVERS_IP_V6_LIST");

	for (i=0; i<NUM_OF_DNS_SERVERS; i++)
	{
		CXMLDOMElement* pIPv6AddressNode = pServersIPv6ListNode->AddChildNode("IP_V6_ADDRESS");

		memset(curV6Mask, 0, IPV6_ADDRESS_LEN);
		GetIPv6SubnetMaskStr(i, curV6Mask);

		pIPv6AddressNode->AddIPv6ChildNode("IP_V6", m_DNS_serversIPv6address[i].ip, curV6Mask);

		memset(szIP, 0, IPV6_ADDRESS_LEN);
		::ipV6ToString(m_DNS_serversIPv6address[i].ip, szIP, FALSE);
		//curV6AddressScope = GetIPv6AddressScope(szIP);
		curV6AddressScope = ConvertenScopeIdToeIpv6AddressScope((enScopeId)m_DNS_serversIPv6address[i].scopeId);
		const char* curV6AddressScopeStr = GetIPv6AddressScopeStr(curV6AddressScope);
		pIPv6AddressNode->AddChildNode("IP_V6_ADDRESS_SCOPE", curV6AddressScopeStr, IP_V6_ADDRESS_SCOPE_ENUM);
	}
}

/////////////////////////////////////////////////////////////////////////////
int  CIpDns::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int i=0;

	int nStatus = STATUS_OK;
	CXMLDOMElement *pTempNode;

	BYTE tmp =0;
	GET_VALIDATE_CHILD(pActionNode,"DNS_STATUS",&tmp,SERVER_STATUS_ENUM);
	m_status = (eServerStatus)tmp;

	GET_VALIDATE_CHILD(pActionNode,"IP_PREFIX",m_prefix,ONE_LINE_BUFFER_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"REGISTER_DNS_NAME_AUTOMATICALLY",&m_register_DNS_name_automatically, _BOOL);

	CXMLDOMElement *pIpList;

	GET_CHILD_NODE(pActionNode, "SERVERS_IP_LIST", pIpList);
	if (pIpList)
	{
		GET_FIRST_CHILD_NODE(pIpList,"IP",pTempNode);

		i=0;
		while(pTempNode && i < NUM_OF_DNS_SERVERS)
		{

			GET_VALIDATE(pTempNode,&(m_DNS_serversIPv4address[i].ip),IP_ADDRESS);
			i++;
			GET_NEXT_CHILD_NODE(pIpList,"IP",pTempNode);
		}
	}

    GET_VALIDATE_ASCII_CHILD(pActionNode,"DOMAIN_NAME",m_domain_name,ONE_LINE_BUFFER_LENGTH);

	GET_VALIDATE_CHILD(pActionNode,"ACCEPT_CALLS_VIA_DNS",&m_accept_calls_via_DNS,_0_TO_WORD);
	GET_VALIDATE_CHILD(pActionNode,"ENABLE",&m_enable,_BOOL);

	CXMLDOMElement *pIPv6ListNode;
	GET_CHILD_NODE(pActionNode, "SERVERS_IP_V6_LIST", pIPv6ListNode);
	if (pIPv6ListNode)
	{
		GET_FIRST_CHILD_NODE(pIPv6ListNode,"IP_V6_ADDRESS",pTempNode);

        char curFullAddress[IPV6_ADDRESS_LEN];
        char tmpAddr[IPV6_ADDRESS_LEN];
        char tmpMask[IPV6_ADDRESS_LEN];
        char curV6AddressScopeStr[ONE_LINE_BUFFER_LEN];

		i=0;
		while ( pTempNode && (i<NUM_OF_DNS_SERVERS) )
		{
            memset(curFullAddress,	0, IPV6_ADDRESS_LEN);
            memset(tmpAddr,			0, IPV6_ADDRESS_LEN);
            memset(tmpMask,			0, IPV6_ADDRESS_LEN);

            GET_VALIDATE_ASCII_CHILD(pTempNode,"IP_V6",curFullAddress,_0_TO_IPV6_ADDRESS_LENGTH);
            FTRACESTR(eLevelDebug) << "CIpDns::DeSerializeXml - Ipv6 - "<<curFullAddress;
            //printf("** CIpDns::DeSerializeXml - curFullAddress -- %s \n",curFullAddress);
            SplitIPv6AddressAndMask(curFullAddress, tmpAddr, tmpMask);
            //printf("** CIpDns::DeSerializeXml - tmpAddr %s tmpMask -- %s \n",tmpAddr,tmpMask);
            FTRACESTR(eLevelDebug) << "CIpDns::DeSerializeXml - After split - Ip - "<< tmpAddr << " mask -" <<tmpMask;
            SetIPv6Address(i, tmpAddr);
            //	m_IPv6AaddressArray[i].scopeId = ::getScopeId(tmpAddr); // done inside SetIPv6Address() method
            SetIPv6SubnetMask(i, tmpMask);
            //   memset(curV6AddressScopeStr, 0, ONE_LINE_BUFFER_LEN);
            //   GET_VALIDATE_CHILD(pIPv6AddressNode,"IP_V6_ADDRESS_SCOPE", curV6AddressScopeStr, IP_V6_ADDRESS_SCOPE_ENUM);

            i++;
            GET_NEXT_CHILD_NODE(pIPv6ListNode,"IP_V6_ADDRESS",pTempNode);
 		}
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CIpDns::IsEnable() const
{
    return m_enable;
}

/////////////////////////////////////////////////////////////////////////////
void CIpDns::SetEnable(const WORD Enable_DNS)
{
    m_enable = Enable_DNS ? TRUE : FALSE;
}

/////////////////////////////////////////////////////////////////////////////
eServerStatus CIpDns::GetStatus() const
{
//	if (!IsEnable())
//		return eServerStatusOff;
    return m_status;
}

/////////////////////////////////////////////////////////////////////////////
void CIpDns::SetStatus(const eServerStatus DNS_status)
{
    m_status = DNS_status;
}

/////////////////////////////////////////////////////////////////////////////
int CIpDns::GetIPv4Address(int idx) const
{
   if (idx >= 0 && idx < NUM_OF_DNS_SERVERS)
        return m_DNS_serversIPv4address[idx].ip;
   else
       return -1;
}

/////////////////////////////////////////////////////////////////////////////
void CIpDns::SetIPv4Address(int idx ,const DWORD IPaddress)
{
    if (idx >= 0 && idx < NUM_OF_DNS_SERVERS)
        m_DNS_serversIPv4address[idx].ip = IPaddress;
}

/////////////////////////////////////////////////////////////////////////////
void CIpDns::GetIPv6Address(int idx, char* retStr, BOOL isBrackets/*=FALSE*/)
{
	if ( (0 <= idx) && (NUM_OF_DNS_SERVERS > idx) )
	{
		::ipV6ToString(m_DNS_serversIPv6address[idx].ip, retStr, isBrackets);

		if ( !strcmp("::", retStr) || !strcmp("[::]", retStr) )
		{
			retStr[0] = '\0';
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CIpDns::SetIPv6Address(int idx, const char* ipV6Address)
{
	if ( (0 <= idx) && (NUM_OF_DNS_SERVERS > idx) )
	{
		// ===== 1. address
		mcTransportAddress tmpIPv6Addr;
		memset(&tmpIPv6Addr, 0, sizeof(mcTransportAddress));
		::stringToIpV6( &tmpIPv6Addr, (char*)ipV6Address );
		memcpy( &(m_DNS_serversIPv6address[idx].ip), &(tmpIPv6Addr.addr.v6.ip), IPV6_ADDRESS_BYTES_LEN );

		// ===== 2. scopeId
		m_DNS_serversIPv6address[idx].scopeId = ::getScopeId( (char*)ipV6Address );
	}
}

/////////////////////////////////////////////////////////////////////////////
DWORD CIpDns::GetIPv6SubnetMask(int idx) const
{
	DWORD retMask = 0;

	if ( (0 <= idx) && (NUM_OF_DNS_SERVERS > idx) )
	{
		retMask = m_DNS_serversIPv6MaskArray[idx];
	}

	return retMask;
}

/////////////////////////////////////////////////////////////////////////////
void CIpDns::GetIPv6SubnetMaskStr(int idx, char *pOutMask) const
{
	if ( (0 <= idx) && (NUM_OF_DNS_SERVERS > idx) )
	{
		sprintf(pOutMask, "%d", m_DNS_serversIPv6MaskArray[idx]);
	}
	else
	{
		pOutMask[0] = '\0';
	}
}

/////////////////////////////////////////////////////////////////////////////
void CIpDns::SetIPv6SubnetMask(int idx, const DWORD subnetMask)
{
	if ( (0 <= idx) && (NUM_OF_DNS_SERVERS > idx) )
	{
		m_DNS_serversIPv6MaskArray[idx] = subnetMask;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CIpDns::SetIPv6SubnetMask(int idx, const char *pMask)
{
	if ( (0 <= idx) && (NUM_OF_DNS_SERVERS > idx) )
	{
		if(sscanf(pMask, "%d", &m_DNS_serversIPv6MaskArray[idx]) != 1)
		{
			FTRACESTR(eLevelInfoNormal) << "CIpDns::SetIPv6SubnetMask - failed  setting default value 64";
			m_DNS_serversIPv6MaskArray[idx] = 64;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
const CSmallString& CIpDns::GetDomainName() const
{
    return m_domain_name;
}

/////////////////////////////////////////////////////////////////////////////
void CIpDns::SetDomainName(const CSmallString& domain_Name)
{
    m_domain_name = domain_Name;
}

/////////////////////////////////////////////////////////////////////////////
const CSmallString& CIpDns::GetHostServiceName() const
{
    return m_host_Service_Name;
}

/////////////////////////////////////////////////////////////////////////////
void CIpDns::SetHostServiceName(const CSmallString& DNS_Host_Service_Name)
{
    m_host_Service_Name = DNS_Host_Service_Name;
}

/////////////////////////////////////////////////////////////////////////////
const CSmallString& CIpDns::GetPrefixName() const
{
    return m_prefix;
}

/////////////////////////////////////////////////////////////////////////////
void CIpDns::SetPrefixName(const CSmallString& DNS_Prefix)
{
    m_prefix = DNS_Prefix;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CIpDns::GetRegisterDNSAutomatically() const
{
    return m_register_DNS_name_automatically;
}

/////////////////////////////////////////////////////////////////////////////
void CIpDns::SetRegisterDNSAutomatically(const BOOL register_DNS_name_automatically)
{
    m_register_DNS_name_automatically = register_DNS_name_automatically ? TRUE : FALSE;
}

/////////////////////////////////////////////////////////////////////////////
WORD CIpDns::GetAcceptCallsViaDNS() const
{
    return m_accept_calls_via_DNS;
}

/////////////////////////////////////////////////////////////////////////////
void CIpDns::SetAcceptCallsViaDNS(const WORD accept_calls_via_DNS)
{
    m_accept_calls_via_DNS = accept_calls_via_DNS ? TRUE : FALSE;
}

/////////////////////////////////////////////////////////////////////////////
ESTATUS CIpDns::TestValidity(BYTE isDhcpEnabled)
{

    DWORD status = STATUS_OK;

    if (m_status != eServerStatusOff &&
		m_status != eServerStatusSpecify &&
		m_status != eServerStatusAuto)
	{
        status = STATUS_UNKNOWN_DNS_STATUS;
	}

	if (status   == STATUS_OK   &&
		m_status == eServerStatusAuto &&
		!isDhcpEnabled)
	{
		status = STATUS_UNKNOWN_DNS_STATUS;
	}

	if (m_status == eServerStatusSpecify)
	{
		if (status == STATUS_OK)
		{
			int count_servers = 0;
			for(int i=0;i<NUM_OF_DNS_SERVERS;i++)
			{

				if( (m_DNS_serversIPv4address[i].ip != 0)  ||
				    (m_DNS_serversIPv4address[i].ip != 0xFFFFFFFF) )
				{
					count_servers++;
				}

				if (count_servers == 0)
					status = STATUS_INVALID_DNS_SERVER_ADDRESS;
			}
		}

		if (!isDhcpEnabled)
		{
			if ( status == STATUS_OK && !m_domain_name.IsEmpty() && !m_domain_name.IsValid(TRUE) )
				status = STATUS_UNKNOWN_DNS_STATUS;

			// Removed because m_host_Service_Name was removed
//			if ( status == STATUS_OK && !m_host_Service_Name.IsValid(TRUE) )
//				status = STATUS_UNKNOWN_DNS_STATUS;
		}
		else
		{
			if ( status == STATUS_OK && !m_domain_name.IsEmpty() && !m_domain_name.IsValid(TRUE) )
				status = STATUS_UNKNOWN_DNS_STATUS;

			// Removed because m_host_Service_Name was removed
//			if ( status == STATUS_OK && !m_host_Service_Name.IsEmpty() && !m_host_Service_Name.IsValid(TRUE) )
//				status = STATUS_UNKNOWN_DNS_STATUS;
		}

//		Removed this validation check because m_prefix type was change	from HostName to CSmallString (since API_IP_SERVICE_CORRECTIONS)
//		if (status == STATUS_OK && m_prefix.IsValid(TRUE))
//			status = STATUS_UNKNOWN_DNS_STATUS;

	}

    if (status  == STATUS_OK &&
		m_register_DNS_name_automatically != TRUE &&
		m_register_DNS_name_automatically != FALSE )
	{
        status = STATUS_UNKNOWN_DNS_STATUS;
	}

    if (status == STATUS_OK &&
		m_accept_calls_via_DNS != TRUE &&
		m_accept_calls_via_DNS != FALSE )
	{
        status = STATUS_UNKNOWN_DNS_STATUS;
	}

    if (status   == STATUS_OK &&
		m_enable != TRUE &&
		m_enable != 1 && // TEMP BYPASS TILL OPER WILL BE FIXED
		m_enable != FALSE )
	{
        status = STATUS_UNKNOWN_DNS_STATUS;
	}

    return status;
}

WORD CIpDns::DnsChangeRequiresCardReset(const CIpDns& newDns) const
{
	for ( WORD i = 0; i < NUM_OF_DNS_SERVERS ; i++ )
	{
		if ( m_DNS_serversIPv4address[i].ip != newDns.m_DNS_serversIPv4address[i].ip )
			return TRUE;
	}

	if ( m_status	   != newDns.m_status ||
		 m_domain_name != newDns.m_domain_name )
		 return	TRUE;


	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//     class CIPAuthenticationElement
/////////////////////////////////////////////////////////////////////////////////////////////////

//CIPAuthenticationElement
CIPAuthenticationElement::CIPAuthenticationElement()
{
    m_user[0]       = '\0';
    m_password[0]   = '\0';
    m_domain[0]     = '\0';
    m_serverName[0] = '\0';
    m_keyIP         = 0;
    m_isAuthenticationEnabled = 0;
    m_authenticationProtocol=eNone;
    m_password_enc[0] = '\0';
    m_password_dec[0] = '\0';
}

//////////////////////////////////////////////////////////////////////////
CIPAuthenticationElement::CIPAuthenticationElement( const  CIPAuthenticationElement &other)
:CPObject(other)
{
    m_user       = other.m_user;
    m_password   = other.m_password;
    m_domain     = other.m_domain;
    m_serverName = other.m_serverName;
    m_keyIP      = other.m_keyIP;

    m_isAuthenticationEnabled = other.m_isAuthenticationEnabled;
    m_authenticationProtocol  = other.m_authenticationProtocol;
    m_password_enc = other.m_password_enc;
    m_password_dec = other.m_password_dec;
}
/////////////////////////////////////////////////////////////////////////////

CIPAuthenticationElement& CIPAuthenticationElement::operator=(const CIPAuthenticationElement& other)
{
    m_user       = other.m_user;
    m_password   = other.m_password;
    m_domain     = other.m_domain;
    m_serverName = other.m_serverName;
    m_keyIP      = other.m_keyIP;

    m_isAuthenticationEnabled = other.m_isAuthenticationEnabled;
    m_authenticationProtocol  = other.m_authenticationProtocol;
    m_password_enc = other.m_password_enc;
    m_password_dec = other.m_password_dec;

    return *this;
}

////////////////////////////////////////////////////////////////////////////
WORD operator==(const CIPAuthenticationElement& lhs,const CIPAuthenticationElement& rhs)
{
	return	( lhs.m_user		== rhs.m_user			&&
			  lhs.m_password	== rhs.m_password		&&
			  lhs.m_domain		== rhs.m_domain			&&
			  lhs.m_serverName	== rhs.m_serverName		&&
			  lhs.m_keyIP		== rhs.m_keyIP			&&
			  lhs.m_isAuthenticationEnabled		== rhs.m_isAuthenticationEnabled			);
			//  lhs.m_authenticationProtocol		== rhs.m_authenticationProtocol			    );
}

////////////////////////////////////////////////////////////////////////////
bool CIPAuthenticationElement::operator!=(const CIPAuthenticationElement& other)const
{
	return (!(*this == other));
}

////////////////////////////////////////////////////////////////////////////
CIPAuthenticationElement::~CIPAuthenticationElement()
{
}
////////////////////////////////////////////////////////////////////////////
void CIPAuthenticationElement::SerializeXml(CXMLDOMElement* pFatherNode,bool isToEma)
{
//  CXMLDOMElement* pAuthenticationElementsNode = pFatherNode->AddChildNode("AUTHENTICATION_ELEMENTS");


    pFatherNode->AddChildNode("USER", m_user);

//    FTRACESTR(eLevelInfoNormal) << "CIPAuthenticationElement::SerializeXml m_user"<< m_user.GetString();
//    FTRACESTR(eLevelInfoNormal) << "CIPAuthenticationElement::SerializeXml m_password"<< m_password.GetString();
//    FTRACESTR(eLevelInfoNormal) << "CIPAuthenticationElement::SerializeXml m_password_enc"<< m_password_enc.c_str();

    if (true == isToEma || m_authenticationProtocol== eNone)
    {
        pFatherNode->AddChildNode("PASSWORD", m_password);

        //FTRACESTR(eLevelInfoNormal) << "CIPAuthenticationElement::SerializeXml m_password"<< m_password.GetString();

    }
    else // to file
    {
        // writing the enc passsword
        pFatherNode->AddChildNode("PASSWORD", m_password_enc);
    }

    pFatherNode->AddChildNode("DOMAIN", m_domain);
    pFatherNode->AddChildNode("SERVER_NAME", m_serverName);

    pFatherNode->AddChildNode("KEYS_IP", m_keyIP,IP_ADDRESS);
    pFatherNode->AddChildNode("ENABLE", m_isAuthenticationEnabled,_BOOL);
    if (true == isToEma ) // EMA is not familiar with this field as it was removed in API , do not send to EMA
    {
        pFatherNode->AddChildNode("AUTHENTICATION_PROTOCOL", m_authenticationProtocol,AUTHENTICATION_PROTOCOL_ENUM);
    }
}


/////////////////////////////////////////////////////////////////////////////
int CIPAuthenticationElement::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"USER",m_user,ONE_LINE_BUFFER_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"PASSWORD",m_password,ONE_LINE_BUFFER_LENGTH);



	GET_VALIDATE_CHILD(pActionNode,"DOMAIN",m_domain,ONE_LINE_BUFFER_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"SERVER_NAME",m_serverName,ONE_LINE_BUFFER_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"KEYS_IP",&m_keyIP,IP_ADDRESS);

	CXMLDOMElement *pChild = NULL;

	GET_CHILD_NODE(pActionNode, "ENABLE", pChild);
	if ( (pChild == NULL ) && (m_user.GetStringLength()!=0))

    {
    	FTRACESTR(eLevelInfoNormal) << "CIPAuthenticationElement::DeSerializeXml child node ENABLE was not exist we add it";
    	SetAuthenticationEnable(true);
    	SetAuthenticationProtocol(eNone);
    }
    else
    {
     GET_VALIDATE_CHILD(pActionNode,"ENABLE",&m_isAuthenticationEnabled,_BOOL);
     GET_CHILD_NODE(pActionNode, "AUTHENTICATION_PROTOCOL", pChild);
     if(pChild)
    	 GET_VALIDATE_CHILD(pActionNode,"AUTHENTICATION_PROTOCOL",(WORD*)&m_authenticationProtocol,AUTHENTICATION_PROTOCOL_ENUM);
    }
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
const CSmallString& CIPAuthenticationElement::GetUserName() const
{
    return m_user;
}

/////////////////////////////////////////////////////////////////////////////
void CIPAuthenticationElement::SetUserName(const CSmallString& user)
{
    m_user = user;
}

/////////////////////////////////////////////////////////////////////////////
const CSmallString & CIPAuthenticationElement::GetPassword() const
{
    return m_password;
}

/////////////////////////////////////////////////////////////////////////////
void CIPAuthenticationElement::SetPassword(const CSmallString& password)
{
    m_password = password;
}

/////////////////////////////////////////////////////////////////////////////
const std::string& CIPAuthenticationElement::GetPassword_enc() const
{
    return m_password_enc;
}

/////////////////////////////////////////////////////////////////////////////
void CIPAuthenticationElement::SetPassword_enc(const std::string& password)
{
	m_password_enc = password;
}

/////////////////////////////////////////////////////////////////////////////
const std::string& CIPAuthenticationElement::GetPassword_dec() const
{
    return m_password_dec;
}

/////////////////////////////////////////////////////////////////////////////
void CIPAuthenticationElement::SetPassword_dec(const std::string& password)
{
	m_password_dec = password;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CIPAuthenticationElement::GetAuthenticationEnable() const
{
    return m_isAuthenticationEnabled;
}

/////////////////////////////////////////////////////////////////////////////
void CIPAuthenticationElement::SetAuthenticationEnable(const BOOL mode)
{
	m_isAuthenticationEnabled = mode;
}

/////////////////////////////////////////////////////////////////////////////
eAuthenticationProtocol CIPAuthenticationElement::GetAuthenticationProtocol() const
{
    return m_authenticationProtocol;
}

/////////////////////////////////////////////////////////////////////////////
void CIPAuthenticationElement::SetAuthenticationProtocol(const eAuthenticationProtocol type)
{
	m_authenticationProtocol = type;
}

/////////////////////////////////////////////////////////////////////////////
const CSmallString& CIPAuthenticationElement::GetDomainName() const
{
    return m_domain;
}

/////////////////////////////////////////////////////////////////////////////
void CIPAuthenticationElement::SetDomainName(const CSmallString& domain)
{
    m_domain = domain;
}

/////////////////////////////////////////////////////////////////////////////
const CSmallString& CIPAuthenticationElement::GetServerName() const
{
    return m_serverName;
}

/////////////////////////////////////////////////////////////////////////////
void CIPAuthenticationElement::SetServerName(const CSmallString& server)
{
    m_serverName = server;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CIPAuthenticationElement::GetKeyIP() const
{
    return m_keyIP;
}

/////////////////////////////////////////////////////////////////////////////
void CIPAuthenticationElement::SetKeyIP(const DWORD key_IP)
{
    m_keyIP = key_IP;
}

/////////////////////////////////////////////////////////////////////////////
ESTATUS CIPAuthenticationElement::TestValidity()
{

    DWORD status = STATUS_OK;

    if(status == STATUS_OK )
    {
        if(m_user.Find(";"))
            status = STATUS_AUTHENTICATION_SERVER_NAME_CANNOT_INCLUDE_SEMICOLON;

        if(m_user.Find("'") ||
            m_user.Find("\"") )
            status = STATUS_AUTHENTICATION_SERVER_NAME_CANNOT_INCLUDE_QUOTATION_MARK;
    }

    if(status == STATUS_OK )
    {
        if(m_password.Find(";"))
            status = STATUS_AUTHENTICATION_SERVER_PASS_NAME_CANNOT_INCLUDE_SEMICOLON;
        if(m_password.Find("'") ||
            m_password.Find("\""))
            status = STATUS_AUTHENTICATION_SERVER_PASS_NAME_CANNOT_INCLUDE_QUOTATION_MARK;
    }

    if(status == STATUS_OK  && !m_domain.IsValid(TRUE))
		status = STATUS_INCONSISTENT_PARAMETERS;

    return status;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//		 Class CIPAuthentication
///////////////////////////////////////////////////////////////////////////////////////////////
CIPAuthentication::CIPAuthentication()
{
    m_of_ind_calls = NUM_OF_AUTHENTICATION_ELEMENTS_PARAMS;
    m_ind_of_numb_of_KerberosElements = 0;
    m_ind_of_numb_of_HTTPDigest       = 0;
    m_numHTTPDigestElements           = 2;
    m_numKerberosElements             = 0;

    for (int i=0;i<NUM_OF_AUTHENTICATION_ELEMENTS;i++)
    {
        m_pKerberos[i]   = new CIPAuthenticationElement;
        m_pHTTPDigest[i] = new CIPAuthenticationElement;
    }
}

//////////////////////////////////////////////////////////////////////////
CIPAuthentication::CIPAuthentication( const  CIPAuthentication &other)
:CPObject(other)
{
    m_numHTTPDigestElements = other.m_numHTTPDigestElements;
    m_numKerberosElements   = other.m_numKerberosElements;
    for (int i=0; i<NUM_OF_AUTHENTICATION_ELEMENTS;i++)
    {
        m_pKerberos[i]   = new CIPAuthenticationElement(*other.m_pKerberos[i]);
        m_pHTTPDigest[i] = new CIPAuthenticationElement(*other.m_pHTTPDigest[i]);
    }
}

/////////////////////////////////////////////////////////////////////////////
CIPAuthentication& CIPAuthentication::operator=(const CIPAuthentication& other)
{
    m_numHTTPDigestElements = other.m_numHTTPDigestElements;
    m_numKerberosElements   = other.m_numKerberosElements;
    int i;
	for (i=0; i<NUM_OF_AUTHENTICATION_ELEMENTS; i++)
    {
        POBJDELETE(m_pKerberos[i]);
        POBJDELETE(m_pHTTPDigest[i]);
    }
    for (i=0; i<NUM_OF_AUTHENTICATION_ELEMENTS; i++)
    {
        if (other.m_pKerberos[i] == NULL)
            m_pKerberos[i] = NULL;
        else
            m_pKerberos[i] = new CIPAuthenticationElement(*other.m_pKerberos[i]);

        if (other.m_pHTTPDigest[i] == NULL)
            m_pHTTPDigest[i] = NULL;
        else
            m_pHTTPDigest[i] = new CIPAuthenticationElement(*other.m_pHTTPDigest[i]);
    }


    return *this;
}

////////////////////////////////////////////////////////////////////////////
WORD operator==(const CIPAuthentication& lhs,const CIPAuthentication& rhs)
{

///  BRIDGE-8284  ema sends 2 lists even thu nothing changes due to binding
	if ( lhs.m_numHTTPDigestElements != rhs.m_numHTTPDigestElements)
	{
		if((rhs.m_numHTTPDigestElements == 0) && (lhs.m_numHTTPDigestElements==2))
		{
				if((lhs.m_pHTTPDigest[0]->GetAuthenticationEnable() !=FALSE)||(lhs.m_pHTTPDigest[1]->GetAuthenticationEnable() !=FALSE))
					return FALSE;
		}
		else
			return FALSE;
	}


	if ( lhs.m_numKerberosElements != rhs.m_numKerberosElements)
		return FALSE;


	WORD i;
	for (i = 0 ; i < lhs.m_numHTTPDigestElements ; i++)
	{
		if ( ! ( *lhs.m_pHTTPDigest[i] == *rhs.m_pHTTPDigest[i]) )
		{

			return FALSE;
		}
	}

	for ( WORD j = 0 ; j < lhs.m_numKerberosElements ; j++)
	{
		if ( !( *lhs.m_pKerberos[i] == *rhs.m_pKerberos[i] ) )
		{
			FTRACEINTO<< "BRIDGE-8284 m_pKerberos  CIPAuthenticationElement i = " << i;
			return FALSE;
		}
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
bool CIPAuthentication::operator!=(const CIPAuthentication& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
CIPAuthentication::~CIPAuthentication()
{
    for (int i=0; i<NUM_OF_AUTHENTICATION_ELEMENTS; i++)
    {
        POBJDELETE(m_pKerberos[i]);
        POBJDELETE(m_pHTTPDigest[i]);
    }
}

/////////////////////////////////////////////////////////////////////////////
void CIPAuthentication::SerializeXml(CXMLDOMElement* pFatherNode,bool isToEma)
{
	CXMLDOMElement* pAuthenticationNode = pFatherNode->AddChildNode("AUTHENTICATION");

	CXMLDOMElement* pListNode = NULL;
	CXMLDOMElement* pTempNode = NULL;

	if (m_numHTTPDigestElements>0)
		pListNode = pAuthenticationNode->AddChildNode("HTTP_DIGEST_LIST");

	for (DWORD i=0; i<m_numHTTPDigestElements;i++)
	{
		pTempNode = pListNode->AddChildNode("HTTP_DIGEST");
		m_pHTTPDigest[i]->SerializeXml(pTempNode, isToEma);
	}
}

/////////////////////////////////////////////////////////////////////////////
int CIPAuthentication::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pTempNode;

	CXMLDOMElement *pListNode;


	GET_CHILD_NODE(pActionNode, "HTTP_DIGEST_LIST", pListNode);

	if (pListNode)
	{

		GET_FIRST_CHILD_NODE(pListNode,"HTTP_DIGEST",pTempNode);
		m_numHTTPDigestElements = 0;
		while (pTempNode && m_numHTTPDigestElements < NUM_OF_AUTHENTICATION_ELEMENTS)
		{
			CIPAuthenticationElement* pHTTPDigest = new CIPAuthenticationElement;
			nStatus = pHTTPDigest->DeSerializeXml(pTempNode, pszError);

			if (nStatus != STATUS_OK)
			{
				POBJDELETE(pHTTPDigest);
				return nStatus;
			}
			POBJDELETE(m_pHTTPDigest[m_numHTTPDigestElements]);
			m_pHTTPDigest[m_numHTTPDigestElements] = pHTTPDigest;
			m_numHTTPDigestElements++;
			GET_NEXT_CHILD_NODE(pListNode,"HTTP_DIGEST",pTempNode);
		}
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
DWORD CIPAuthentication::GetHTTPDigestAuthenticationNumber() const
{
    return m_numHTTPDigestElements;
}

/////////////////////////////////////////////////////////////////////////////
void CIPAuthentication::SetHTTPDigestAuthenticationNumber(const DWORD numHTTPDigestElements)
{
    m_numHTTPDigestElements = numHTTPDigestElements;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CIPAuthentication::GetKerberosAuthenticationNumber() const
{
    return m_numKerberosElements;
}

/////////////////////////////////////////////////////////////////////////////
void CIPAuthentication::SetKerberosAuthenticationNumber(const DWORD numKerberosElements)
{
    m_numKerberosElements = numKerberosElements;
}

/////////////////////////////////////////////////////////////////////////////
CIPAuthenticationElement* CIPAuthentication::GetFirstKerberosAuthenticationElements()
{
    m_ind_of_numb_of_KerberosElements = 1;
    return m_pKerberos[0];
}

/////////////////////////////////////////////////////////////////////////////
CIPAuthenticationElement* CIPAuthentication::GetNextKerberosAuthenticationElements()
{
    if (m_ind_of_numb_of_KerberosElements>= NUM_OF_AUTHENTICATION_ELEMENTS)
        return NULL;
    return  m_pKerberos[m_ind_of_numb_of_KerberosElements++];
}

/////////////////////////////////////////////////////////////////////////////
CIPAuthenticationElement* CIPAuthentication::GetFirstHTTPDigestAuthenticationElements()
{
    m_ind_of_numb_of_HTTPDigest = 1;
    return m_pHTTPDigest[0];
}

/////////////////////////////////////////////////////////////////////////////
CIPAuthenticationElement* CIPAuthentication::GetSecondHTTPDigestAuthenticationElements()
{
   // m_ind_of_numb_of_HTTPDigest = 1;
    return m_pHTTPDigest[1];
}

/////////////////////////////////////////////////////////////////////////////
CIPAuthenticationElement* CIPAuthentication::GetNextHTTPDigestAuthenticationElements()
{
    if (m_ind_of_numb_of_HTTPDigest>= NUM_OF_AUTHENTICATION_ELEMENTS)
        return NULL;
    return  m_pHTTPDigest[m_ind_of_numb_of_HTTPDigest++];
}

/////////////////////////////////////////////////////////////////////////////

CIPAuthenticationElement* CIPAuthentication::GetHTTPDigestAuthenticationElement(WORD index)
{
    if (index >= NUM_OF_AUTHENTICATION_ELEMENTS)
        return NULL;
    return  m_pHTTPDigest[index];
}
/////////////////////////////////////////////////////////////////////////////
CIPAuthenticationElement* CIPAuthentication::GetKerberosAuthenticationElement(WORD index)
{
    if (index >= NUM_OF_AUTHENTICATION_ELEMENTS)
        return NULL;
    return  m_pKerberos[index];
}

/////////////////////////////////////////////////////////////////////////////
ESTATUS CIPAuthentication::TestValidity()
{
    DWORD status = STATUS_OK;

    if(status == STATUS_OK )
    {


		if(m_numHTTPDigestElements == 0xFFFFFFFF)
		//instead of:
		//if(m_numHTTPDigestElements < 0  || m_numHTTPDigestElements == 0xFFFFFFFF)
            return STATUS_INVALID_AUTHENTICATION_DIGEST_ELEMENT_NUMBER;
		//instead of:
        //if(m_numKerberosElements < 0  || m_numKerberosElements == 0xFFFFFFFF)
		if(m_numKerberosElements == 0xFFFFFFFF)
            return STATUS_INVALID_AUTHENTICATION_KERBEROS_ELEMENT_NUMBER;
    }

	DWORD i;
    for(i=0;i<m_numKerberosElements;i++)
    {
        if(m_pKerberos[i]!=NULL)
            status = m_pKerberos[i]->TestValidity();
		if (status != STATUS_OK)
			return status;
	}

	for(i=0;i<m_numHTTPDigestElements;i++)
	{
        if(m_pHTTPDigest[i]!=NULL)
            status = m_pHTTPDigest[i]->TestValidity();
		if (status != STATUS_OK)
			return status;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////////////
void CIPAuthentication::SetpKerberosIPAuthenticationElements(const CIPAuthenticationElement& pKerberosAuthenticationElements)
{
    POBJDELETE(m_pKerberos[m_ind_of_numb_of_KerberosElements - 1]);
	m_pKerberos[m_ind_of_numb_of_KerberosElements - 1] =
		new CIPAuthenticationElement(pKerberosAuthenticationElements);
}

/////////////////////////////////////////////////////////////////////////////
void CIPAuthentication::SetpHTTPDigestIPAuthenticationElements(const CIPAuthenticationElement& pHTTPDigestAuthenticationElements)
{
    POBJDELETE(m_pHTTPDigest[m_ind_of_numb_of_HTTPDigest - 1]);
	m_pHTTPDigest[m_ind_of_numb_of_HTTPDigest - 1] =
		new CIPAuthenticationElement(pHTTPDigestAuthenticationElements);
}





//////////////////////////////////////////////////////////////////////////////////////////////////
//					CIPSecurity
/////////////////////////////////////////////////////////////////////////////////////////////////
CIPSecurity::CIPSecurity()
{
    m_pAuthentication = new CIPAuthentication;
}

/////////////////////////////////////////////////////////////////////////////
CIPSecurity::CIPSecurity(const CIPSecurity& other)
:CPObject(other)
{
    if (other.m_pAuthentication == NULL)
        m_pAuthentication = NULL;
    else
        m_pAuthentication = new CIPAuthentication(*other.m_pAuthentication);
}

/////////////////////////////////////////////////////////////////////////////
CIPSecurity& CIPSecurity::operator=(const CIPSecurity& other)
{
	if(this == &other){
	    return *this;
	}

    POBJDELETE(m_pAuthentication);
    if (other.m_pAuthentication == NULL)
        m_pAuthentication = NULL;
    else
        m_pAuthentication = new CIPAuthentication(*other.m_pAuthentication);

    return *this;
}

/////////////////////////////////////////////////////////////////////////////
WORD operator==(const CIPSecurity& lhs,const CIPSecurity& rhs)
{
	return ( *lhs.m_pAuthentication == *rhs.m_pAuthentication);

}
/////////////////////////////////////////////////////////////////////////////
bool CIPSecurity::operator!=(const CIPSecurity& other)const
{
	return (!(*this == other));
}

/////////////////////////////////////////////////////////////////////////////
CIPSecurity::~CIPSecurity()
{
    POBJDELETE(m_pAuthentication);
}

/////////////////////////////////////////////////////////////////////////////
void CIPSecurity::SerializeXml(CXMLDOMElement* pFatherNode,bool isToEma)
{
	CXMLDOMElement* pSecurityNode = pFatherNode->AddChildNode("SECURITY");

	m_pAuthentication->SerializeXml(pSecurityNode,isToEma);
}

/////////////////////////////////////////////////////////////////////////////
int CIPSecurity::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement *pTempNode;

	GET_CHILD_NODE(pActionNode, "AUTHENTICATION", pTempNode);
	if (pTempNode)
	{
		CIPAuthentication* pAuthentication = new CIPAuthentication;
		nStatus = pAuthentication->DeSerializeXml(pTempNode, pszError);

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pAuthentication);
			return nStatus;
		}
		POBJDELETE(m_pAuthentication);
		m_pAuthentication = pAuthentication;
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
ESTATUS CIPSecurity::TestValidity()
{
    DWORD status = STATUS_OK;
    if (m_pAuthentication!=NULL)
        status = m_pAuthentication->TestValidity();
    return status;
}

/////////////////////////////////////////////////////////////////////////////
void CIPSecurity::SetpIPAuthentication(const CIPAuthentication& pAuthentication)
{
    POBJDELETE(m_pAuthentication);
    m_pAuthentication = new CIPAuthentication(pAuthentication);
}




///////////////////////////////////////////////////////////////////////////////////////////////
//						   CH323Router
//////////////////////////////////////////////////////////////////////////////////////////////
CH323Router::CH323Router()
{
    m_routerIP 		= 0;
    m_remoteIP 		= 0;
    m_remoteFlag 	= H323_REMOTE_NETWORK;
    m_subnetMask	= 0;

//    03/08/06: DannyB - set subnetMask default to 0
//	const char * defaultsubnetMask = "255.255.255.0";
 //   m_subnetMask	= SystemIpStringToDWORD(defaultsubnetMask);
}

////////////////////////////////////////////////////////////////////////////
CH323Router::CH323Router( const CH323Router &other)
:CPObject(other)
{
    *this = other;
}

////////////////////////////////////////////////////////////////////////////
CH323Router& CH323Router::operator=(const CH323Router& other)
{
	if(this != &other)
	{
	    m_routerIP 		= other.m_routerIP;
	    m_remoteIP 		= other.m_remoteIP;
	    m_subnetMask	= other.m_subnetMask;
	    m_remoteFlag 	= other.m_remoteFlag;
	}
    return *this;
}

////////////////////////////////////////////////////////////////////////////
bool CH323Router::operator==(const CH323Router &rHnd)const
{
	int i=0;

	if(this == &rHnd)
		return true;

	if ( m_routerIP   != rHnd.m_routerIP   ||
		 m_remoteIP	  != rHnd.m_remoteIP   ||
		 m_subnetMask != rHnd.m_subnetMask ||
		 m_remoteFlag != rHnd.m_remoteFlag)
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////
bool CH323Router::operator!=(const CH323Router& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
CH323Router::~CH323Router()
{
}

/////////////////////////////////////////////////////////////////////////////
void CH323Router::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pH323RouterNode = pFatherNode->AddChildNode("ROUTER");

	pH323RouterNode->AddChildNode("IP", m_routerIP,IP_ADDRESS);
	pH323RouterNode->AddChildNode("REMOTE_IP", m_remoteIP,IP_ADDRESS);
	pH323RouterNode->AddChildNode("SUBNET_MASK", m_subnetMask,IP_ADDRESS);
	pH323RouterNode->AddChildNode("REMOTE_FLAG", m_remoteFlag, REMOTE_FLAG_ENUM);
}

/////////////////////////////////////////////////////////////////////////////
int	 CH323Router::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"IP",&m_routerIP,IP_ADDRESS);
	GET_VALIDATE_CHILD(pActionNode,"REMOTE_IP",&m_remoteIP,IP_ADDRESS);
	GET_VALIDATE_CHILD(pActionNode,"SUBNET_MASK",&m_subnetMask,IP_ADDRESS);
	GET_VALIDATE_CHILD(pActionNode,"REMOTE_FLAG",&m_remoteFlag,REMOTE_FLAG_ENUM);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CH323Router::GetRouterIP () const
{
    return m_routerIP;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Router::SetRouterIP(const DWORD  routerIP)
{
    m_routerIP = routerIP;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CH323Router::GetRemoteIP () const
{
    return m_remoteIP;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Router::SetRemoteIP(const DWORD  remoteIP)
{
    m_remoteIP = remoteIP;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CH323Router::GetSubnetMask() const
{
	return m_subnetMask;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Router::SetSubnetMask(const DWORD  subnetMask)
{
    m_subnetMask = subnetMask;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CH323Router::GetRemoteFlag () const
{
    return m_remoteFlag;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Router::SetRemoteFlag(const BYTE  remoteFlag)
{
    m_remoteFlag = remoteFlag;
}

/////////////////////////////////////////////////////////////////////////////
bool CH323Router::IsDefault()const
{
	bool res = (0 == m_routerIP);
	return res;
}

/////////////////////////////////////////////////////////////////////////////



//#ifdef __HIGHC__


//////////////////////////////////////////////////////////////////////////////////////////////////
//						  class HostName                                                       //
////////////////////////////////////////////////////////////////////////////////////////////////



BYTE HostName::IsValid(BYTE isDns) const
{

	// try to convert the string using the w.x.y.z scheme
	if (IpToDWORD() != 0)
		return TRUE;
	else
	{
		if (!isDns) // if it's not w.x.y.z and the dns is off we don't try the hname
			return FALSE;
	}

	enum eLastCharType	{
		eNone,  // in the beginning of the string
        eDot,   // '.'
        eAlpha, // 'a'-'z','A'-'Z'
        eDigit, // '0'-'9'
        eHyphen // '_' | '-'
	} last_char = eNone; // after stores the type of the last character we scaned

	const char * temp = m_pString;

	// empty strings are not valid host name
	if (!*temp)
		return FALSE;

	// test host name validation (partly based on RFC 810)
	while (*temp)
	{
		if (*temp >= '0' && *temp <= '9')
		{
			// only alphas allowed in the beginning of the string or after a dot
			// (except in a valid w.x.y.z ip address eliminated in the first line of this function)

			// we do allow it
			//if (last_char == eDot || last_char == eNone)
			//	return FALSE;
			last_char = eDigit;
		}
		else
		{
			if ( (*temp >= 'a' && *temp <= 'z' ) || (*temp >= 'A' && *temp <= 'Z' ) )
				last_char = eAlpha;
			else
			{
				if (*temp == '.')
				{
					// dot is illegal on the beginning of the string,
					// after a dot ot after hyphen
					// * we allow digit in the end of the name
					if (last_char == eDot || last_char == eHyphen )
						return FALSE;
					last_char = eDot;
				}
				else
				{
					if (*temp == '_' || *temp == '-')
					{
						if (last_char == eDot || last_char == eNone)
							return FALSE;
						last_char = eHyphen;
					}
					else
					{
						// host name can't include characters other
						// then 'a'-'z','A'-'Z','0'-'9','-','_'
						return FALSE;
					}
				}
			}
		} // else
		temp++;
	} // while

	if (last_char != eAlpha && last_char != eDigit) // * we allow digit in the end of the hname
		return FALSE;

	return TRUE;
}


// return 0 if failed to parse ip string (w.x.y.z) to DWORD (0 <= w,x,y,z < 256)
// otherwise return the DWORD IP value
DWORD HostName::IpToDWORD() const

{
	const char * temp  = m_pString;
	DWORD  result      = 0;
	BYTE * b           = ((BYTE *) &result) + 3;
	BYTE   after_dot   = TRUE;  // are we just after a dot or in the first char
	int    digits      = 0;     // how many digits we already found in the current number
	int    dots        = 0;     // how many dots we already found

	while (*temp)
	{
		if (*temp >= '0' && *temp <= '9')
		{
			if (*b == 0 && !after_dot)
				return 0; // leading zeros are not allowed

			after_dot = FALSE;
			DWORD current_num = (*b) * 10 + (*temp - '0');

			if (current_num > 255)
				return 0; // each number should be in the range of (0-255)

			*b = (BYTE)current_num;
			digits++;
		}
		else
		{
			if (*temp == '.')
			{
				if (after_dot)
					return 0; // '.' can't start the string, or b just after another

				if (dots >= 3)
					return 0; // more than 3 dots are not allowed

				dots++;
				b--;
				after_dot = TRUE;
			}
			else
				return 0; // ip string should contain only ['0'..'9','.'] chars
		}

		temp++;
	} // while (*temp)

	if (dots != 3) // 3 dots are mandatory
		return 0;

	if (after_dot) // ip string can't end with dot
		return 0;

	return result; // parsing is OK
}

//////////////////////////////////////////////////////////////////////////////////
HostName::HostName(const char*const str)
:CSmallString(str)
{
}

//////////////////////////////////////////////////////////////////////////////////
HostName::HostName(const DWORD address)
{
	if ( !address )
		*this = "";
	else
	{
		unsigned char *a = (unsigned char*)&address;
		char buff[20];

		sprintf( buff, "%d.%d.%d.%d",*(a+3),*(a+2), *(a+1),*a);
		*this = buff;
	}
}
//////////////////////////////////////////////////////////////////////////////////
// copy constructor
HostName::HostName(const CObjString & s1)
:CSmallString(s1)
{
}



//////////////////////////////////////////////////////////////////////////////////////////
//			Class CVlan
//////////////////////////////////////////////////////////////////////////////////////////////////////
CVlan::CVlan()
{
	m_IsSupport = FALSE;
	m_Priority = 0;
	m_Id = 0;
}

//////////////////////////////////////////////////////////////////////////////////
CVlan::CVlan( const CVlan &other )
:CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////////////////
CVlan&  CVlan::operator=( const CVlan& other )
{
	if(this != &other)
	{
		m_IsSupport = other.m_IsSupport;
		m_Priority 	= other.m_Priority;
		m_Id 		= other.m_Id;
	}

	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
bool CVlan::operator==(const CVlan &rHnd)const
{
	if(this == &rHnd)
		return true;

	if ( m_IsSupport != rHnd.m_IsSupport )
		return false;

	if ( m_Priority != rHnd.m_Priority )
		return false;

	if ( m_Id != rHnd.m_Id )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CVlan::operator!=(const CVlan& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
CSerializeObject* CVlan::Clone()
{
	CSerializeObject *clone = new CVlan(*this);
	return clone;
}

//////////////////////////////////////////////////////////////////////////////////
void CVlan::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pChildNode = NULL;
	if(NULL == pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("VLAN");
		pChildNode = pFatherNode;
	}
	else
	{
		pChildNode = pFatherNode->AddChildNode("VLAN");
	}

	pChildNode->AddChildNode("IS_SUPPORTED"	, m_IsSupport	, _BOOL);
	pChildNode->AddChildNode("ID"			, m_Id			, _0_TO_DWORD);
	pChildNode->AddChildNode("PRIORITY"		, m_Priority	, _0_TO_MAX_VLAN_PRIORITY);
}

//////////////////////////////////////////////////////////////////////////////////
int  CVlan::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	CXMLDOMElement *pVlanNode = NULL;
	GET_CHILD_NODE(pActionNode, "VLAN", pVlanNode);
	if(NULL == pVlanNode)
	{
		return nStatus;
	}

	GET_VALIDATE_CHILD(pVlanNode, "IS_SUPPORTED", &m_IsSupport	, _BOOL);
	GET_VALIDATE_CHILD(pVlanNode, "ID"			, &m_Id			, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pVlanNode, "PRIORITY"	, &m_Priority	, _0_TO_MAX_VLAN_PRIORITY);

	return nStatus;
}

BOOL CVlan::GetIsSupport()const
{
	return m_IsSupport;
}

//////////////////////////////////////////////////////////////////////////////////
void CVlan::SetIsSupport(BOOL val)
{
	m_IsSupport = val;
}

//////////////////////////////////////////////////////////////////////////////////
DWORD CVlan::GetPriority()const
{
	return m_Priority;
}

//////////////////////////////////////////////////////////////////////////////////
void CVlan::SetPriority(DWORD val)
{
	m_Priority = val;
}

//////////////////////////////////////////////////////////////////////////////////
DWORD CVlan::GetId()const
{
	return m_Id;
}

//////////////////////////////////////////////////////////////////////////////////
void CVlan::SetId(DWORD val)
{
	m_Id = val;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class  CPortSpeed                                                     //
////////////////////////////////////////////////////////////////////////////////////////////////
CPortSpeed::CPortSpeed()
{
    m_Num = 0;
    m_Speed = ePortSpeed_Auto;
}

/////////////////////////////////////////////////////////////////////////////
CPortSpeed::CPortSpeed(DWORD num, ePortSpeedType speed)
{
    m_Num = num;
    m_Speed = speed;
}

/////////////////////////////////////////////////////////////////////////////
CPortSpeed::~CPortSpeed()
{
}

/////////////////////////////////////////////////////////////////////////////
bool CPortSpeed::operator==(const CPortSpeed &rHnd)const
{
	if(this == &rHnd)
		return true;

	if ( m_Num != rHnd.m_Num )
		return false;

	if ( m_Speed != rHnd.m_Speed )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CPortSpeed::operator!=(const CPortSpeed& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
void CPortSpeed::SerializeXml(CXMLDOMElement*& pFatherNode)const
{
    CXMLDOMElement *pChildNode = pFatherNode->AddChildNode("PORT_SPEED");

	pChildNode->AddChildNode("SPEED", m_Speed, SPEED_MODE_ENUM);
    pChildNode->AddChildNode("PORT", m_Num);
}

/////////////////////////////////////////////////////////////////////////////
int CPortSpeed::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    STATUS nStatus = STATUS_OK;

    DWORD tmp = 0;
    GET_VALIDATE_CHILD(pActionNode, "SPEED", &tmp, SPEED_MODE_ENUM);
    m_Speed = (ePortSpeedType)tmp;
	GET_VALIDATE_CHILD(pActionNode, "PORT", &m_Num, _0_TO_DWORD);

    return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
void CPortSpeed::SetNum(DWORD num)
{
    m_Num = num;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CPortSpeed::GetNum()const
{
    return m_Num;
}

/////////////////////////////////////////////////////////////////////////////
void CPortSpeed::SetSpeed(ePortSpeedType speed)
{
    m_Speed = (0 <= speed && speed < NUM_OF_PORT_SPEED_TYPES
               ?
               speed : ePortSpeed_Auto);
}

/////////////////////////////////////////////////////////////////////////////
ePortSpeedType CPortSpeed::GetSpeed()const
{
    return m_Speed;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class  CSipAdvanced                                                     //
////////////////////////////////////////////////////////////////////////////////////////////////
CSipAdvanced::CSipAdvanced()
{
	m_sipAdvancedUserName = "";
	m_iceEnvironment = eIceEnvironment_None;
	m_pIceStandardParams = new CIceStandardParams;
}
/////////////////////////////////////////////////////////////////////////////

CSipAdvanced::CSipAdvanced(const CSipAdvanced& other)
:CSerializeObject(other)
{
	*this = other;
}
/////////////////////////////////////////////////////////////////////////////
CSipAdvanced& CSipAdvanced::operator=(const CSipAdvanced& other)
{
	if(this != &other)
	{
		POBJDELETE(m_pIceStandardParams);

		m_sipAdvancedUserName = other.m_sipAdvancedUserName;
		m_iceEnvironment = other.m_iceEnvironment;

	    if (other.m_pIceStandardParams == NULL)
	    	m_pIceStandardParams = NULL;
	    else
	    	m_pIceStandardParams = new CIceStandardParams(*other.m_pIceStandardParams);
	}
	return *this;
}
/////////////////////////////////////////////////////////////////////////////
bool CSipAdvanced::operator==(const CSipAdvanced& other)const
{

	if(this == &other)
		return true;

	if (m_sipAdvancedUserName != other.m_sipAdvancedUserName)
		return false;

	if (m_iceEnvironment != other.m_iceEnvironment)
		return false;

	if (*m_pIceStandardParams != *other.m_pIceStandardParams)
		return false;

	return true;
}
/////////////////////////////////////////////////////////////////////////////
bool CSipAdvanced::operator!=(const CSipAdvanced& other)const
{
	bool bRes = CSipAdvanced::operator==(other);
	return !bRes;
}
/////////////////////////////////////////////////////////////////////////////
CSipAdvanced::~CSipAdvanced()
{
	POBJDELETE(m_pIceStandardParams);
}
/////////////////////////////////////////////////////////////////////////////
void CSipAdvanced::SetSipAdvancedUserName(const char* user_name)
{
	m_sipAdvancedUserName = user_name;
}
/////////////////////////////////////////////////////////////////////////////
void CSipAdvanced::SerializeXml(CXMLDOMElement*& pFatherNode)const
{
	CXMLDOMElement *pChildNode = NULL;
	if(NULL == pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("SIP_ADVANCED");
		pChildNode = pFatherNode;
	}
	else
	{
		pChildNode = pFatherNode->AddChildNode("SIP_ADVANCED");
	}

    pChildNode->AddChildNode("SIP_ADVANCED_USER_NAME", m_sipAdvancedUserName);
    pChildNode->AddChildNode("ICE_ENVIRONMENT", m_iceEnvironment, ICE_ENVIRONMENT_ENUM);

	if (m_pIceStandardParams)
		m_pIceStandardParams->SerializeXml(pChildNode);
}

/////////////////////////////////////////////////////////////////////////////
int CSipAdvanced::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    STATUS nStatus = STATUS_OK;

    TRACESTR(eLevelInfoNormal) << "CSipAdvanced::DeSerializeXml - !!!";

    GET_VALIDATE_CHILD(pActionNode,"SIP_ADVANCED_USER_NAME",m_sipAdvancedUserName,ONE_LINE_BUFFER_LENGTH);
    DWORD tmp = 0;
    GET_VALIDATE_CHILD(pActionNode, "ICE_ENVIRONMENT", &tmp, ICE_ENVIRONMENT_ENUM);
    m_iceEnvironment = (eIceEnvironmentType)tmp;

    if (m_iceEnvironment==eIceEnvironment_ms && m_sipAdvancedUserName=="")
    	return STATUS_EMPTY_USER_NAME_IN_ICE_ENVIRONMENT;

	CXMLDOMElement *pTempNode = NULL;

	GET_CHILD_NODE(pActionNode, "ICE_STANDARD_PARAMS", pTempNode);
	if (pTempNode)
	{
		CIceStandardParams* pIceStandardParams = new CIceStandardParams;
		nStatus = pIceStandardParams->DeSerializeXml(pTempNode, pszError);

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pIceStandardParams);
			return nStatus;
		}
		POBJDELETE(m_pIceStandardParams);
		m_pIceStandardParams = pIceStandardParams;

		TRACESTR(eLevelInfoNormal)
		<< " STUN Host Name " << m_pIceStandardParams->GetSTUNServerHostName()
		<< " TURN Host Name " << m_pIceStandardParams->GetTURNServerHostName()
		<< " STUN IP " << m_pIceStandardParams->GetSTUNServerIp()
		<< " TURN IP " << m_pIceStandardParams->GetTURNServerIp()
		<< " STUN PORT " <<	m_pIceStandardParams->GetSTUNServerPort()
		<< " TURN PORT " <<	m_pIceStandardParams->GetTURNServerPort() ;
	}

    return nStatus;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
const char* CSipAdvanced::GetSipAdvancedUserName()const
{
	return m_sipAdvancedUserName.c_str();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
eIceEnvironmentType CSipAdvanced::GetIceEnvironment()const
{
	return m_iceEnvironment;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
CIceStandardParams* CSipAdvanced::GetpIceStandardParams()const
{
	return m_pIceStandardParams;
}

void CSipAdvanced::SetIceEnvironment(eIceEnvironmentType enviromentType)
{
	m_iceEnvironment = enviromentType;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//					Class  CIceStandardParams                                                     //
////////////////////////////////////////////////////////////////////////////////////////////////
CIceStandardParams::CIceStandardParams()
{
	 m_IsServerPassword = FALSE;
	 m_PasswordServerIp = 0;
	 m_PasswordServerHostName[0] = '\0';
	 m_PasswordServerPort = 0;

	 m_PasswordServerUserName[0] = '\0';
	 m_PasswordServerPassword[0] = '\0';

	 m_STUNServerIp   = 0;
	 m_STUNServerHostName[0] = '\0';
	 m_STUNServerPort = 0;

	 m_TURNServerIp   = 0;
	 m_TURNServerHostName[0] = '\0';
	 m_TURNServerPort = 0;

}

/////////////////////////////////////////////////////////////////////////////
CIceStandardParams::CIceStandardParams(const CIceStandardParams& other)
:CSerializeObject(other)
{
	*this = other;
}
/////////////////////////////////////////////////////////////////////////////
CIceStandardParams& CIceStandardParams::operator=(const CIceStandardParams& other)
{
	if(this != &other)
	{

		m_IsServerPassword = other.m_IsServerPassword;
		m_PasswordServerIp = other.m_PasswordServerIp;
		m_PasswordServerHostName = other.m_PasswordServerHostName;

		m_PasswordServerPort = other.m_PasswordServerPort;
		m_PasswordServerUserName = other.m_PasswordServerUserName;
		m_PasswordServerPassword = other.m_PasswordServerPassword;

		m_STUNServerIp = other.m_STUNServerIp;
		m_STUNServerHostName = other.m_STUNServerHostName;

		m_TURNServerIp = other.m_TURNServerIp;
		m_TURNServerHostName = other.m_TURNServerHostName;

		m_STUNServerPort = other.m_STUNServerPort;
		m_TURNServerPort = other.m_TURNServerPort;

	}

	return *this;
}
/////////////////////////////////////////////////////////////////////////////
bool CIceStandardParams::operator==(const CIceStandardParams& other)const
{
	if(this == &other)
		return true;

	if(m_IsServerPassword != other.m_IsServerPassword)
		return false;

	if(m_PasswordServerIp != other.m_PasswordServerIp)
		return false;

	if(m_PasswordServerHostName != other.m_PasswordServerHostName)
		return false;

	if(m_PasswordServerPort != other.m_PasswordServerPort)
		return false;

	if(m_STUNServerIp != other.m_STUNServerIp)
		return false;

	if(m_STUNServerHostName != other.m_STUNServerHostName)
		return false;

	if(m_TURNServerIp != other.m_TURNServerIp)
		return false;

	if(m_TURNServerHostName != other.m_TURNServerHostName)
		return false;

	if( m_PasswordServerUserName != other.m_PasswordServerUserName)
		return false;

	if( m_PasswordServerPassword != other.m_PasswordServerPassword)
		return false;

	if( m_STUNServerPort != other.m_STUNServerPort)
		return false;

	if( m_TURNServerPort != other.m_TURNServerPort)
		return false;

	return true;
}
/////////////////////////////////////////////////////////////////////////////
bool CIceStandardParams::operator!=(const CIceStandardParams& other)const
{
	bool bRes = CIceStandardParams::operator==(other);
	return !bRes;
}
/////////////////////////////////////////////////////////////////////////////
CIceStandardParams::~CIceStandardParams()
{
}

/////////////////////////////////////////////////////////////////////////////
void CIceStandardParams::SerializeXml(CXMLDOMElement*& pFatherNode)const
{
	TRACESTR(eLevelInfoNormal) << "\nCIceStandardParams::SerializeXml  ";

	CXMLDOMElement *pChildNode = NULL;
	if(NULL == pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("ICE_STANDARD_PARAMS");
		pChildNode = pFatherNode;
	}
	else
	{
		pChildNode = pFatherNode->AddChildNode("ICE_STANDARD_PARAMS");
	}

	pChildNode->AddChildNode("IS_PASSWORD_SERVER", m_IsServerPassword, _BOOL);


	//pChildNode->AddChildNode("PASSWORD_SERVER_IP", m_PasswordServerIp, IP_ADDRESS);
	pChildNode->AddChildNode("PASSWORD_SERVER_IP", m_PasswordServerHostName);
	pChildNode->AddChildNode("PASSWORD_SERVER_PORT", m_PasswordServerPort, _0_TO_WORD);

	pChildNode->AddChildNode("PASSWORD_SERVER_USER_NAME", m_PasswordServerUserName);
	pChildNode->AddChildNode("PASSWORD_SERVER_PASSWORD", m_PasswordServerPassword);

	TRACESTR(eLevelInfoNormal) << " STUN_SERVER_IP "    << m_STUNServerHostName;

	//pChildNode->AddChildNode("STUN_SERVER_IP", m_STUNServerIp, IP_ADDRESS);
	pChildNode->AddChildNode("STUN_SERVER_IP", m_STUNServerHostName);
	pChildNode->AddChildNode("STUN_SERVER_PORT", m_STUNServerPort, _0_TO_WORD);

	TRACESTR(eLevelInfoNormal) << " STUN_SERVER_IP "    << m_STUNServerHostName;

	//pChildNode->AddChildNode("TURN_SERVER_IP", m_TURNServerIp, IP_ADDRESS);
	pChildNode->AddChildNode("TURN_SERVER_IP", m_TURNServerHostName);
	pChildNode->AddChildNode("TURN_SERVER_PORT", m_TURNServerPort, _0_TO_WORD);

}

/////////////////////////////////////////////////////////////////////////////
int CIceStandardParams::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{

	TRACESTR(eLevelInfoNormal) << "\nCIceStandardParams::DeSerializeXml  ";

    STATUS nStatus = STATUS_OK;

    CXMLDOMElement *pTempNode = NULL;

    GET_CHILD_NODE(pActionNode, "IS_PASSWORD_SERVER", pTempNode);

    if (pTempNode != NULL)
    {

    	GET_VALIDATE_CHILD(pActionNode,"IS_PASSWORD_SERVER",&m_IsServerPassword,_BOOL);
    	//GET_VALIDATE_CHILD(pActionNode,"PASSWORD_SERVER_IP",&m_PasswordServerIp,IP_ADDRESS);
    	GET_VALIDATE_CHILD(pActionNode,"PASSWORD_SERVER_IP",m_PasswordServerHostName,ONE_LINE_BUFFER_LENGTH);

      	GET_VALIDATE_CHILD(pActionNode,"PASSWORD_SERVER_PORT",&m_PasswordServerPort,_0_TO_WORD);

    	GET_VALIDATE_CHILD(pActionNode,"PASSWORD_SERVER_USER_NAME",m_PasswordServerUserName,ONE_LINE_BUFFER_LENGTH);
    	GET_VALIDATE_CHILD(pActionNode,"PASSWORD_SERVER_PASSWORD",m_PasswordServerPassword,ONE_LINE_BUFFER_LENGTH);

    }

    //GET_VALIDATE_CHILD(pActionNode,"STUN_SERVER_IP",&m_STUNServerIp,IP_ADDRESS);
    GET_VALIDATE_CHILD(pActionNode,"STUN_SERVER_IP",m_STUNServerHostName,ONE_LINE_BUFFER_LENGTH);
    GET_VALIDATE_CHILD(pActionNode,"STUN_SERVER_PORT",&m_STUNServerPort,_0_TO_WORD);

    //GET_VALIDATE_CHILD(pActionNode,"TURN_SERVER_IP",&m_TURNServerIp,IP_ADDRESS);
    GET_VALIDATE_CHILD(pActionNode,"TURN_SERVER_IP",m_TURNServerHostName,ONE_LINE_BUFFER_LENGTH);
    GET_VALIDATE_CHILD(pActionNode,"TURN_SERVER_PORT",&m_TURNServerPort,_0_TO_WORD);

    if (m_STUNServerPort == 0)
    {
    	TRACESTR(eLevelInfoNormal) << "STUN port set as 0 by user change to default " <<
    			DEFAULT_ICE_STUN_TURN_SERVER_PORT;
    	SetSTUNServerPort(DEFAULT_ICE_STUN_TURN_SERVER_PORT);
    }
    if (m_TURNServerPort == 0)
    {
    	TRACESTR(eLevelInfoNormal) << "TURN port set as 0 by user change to default " <<
    	    			DEFAULT_ICE_STUN_TURN_SERVER_PORT;
    	SetTURNServerPort(DEFAULT_ICE_STUN_TURN_SERVER_PORT);
    }

    TRACESTR(eLevelInfoNormal) << " STUN_SERVER_IP "    << m_STUNServerHostName << "\n"
    						   << " STUN_SERVER_PORT " << m_STUNServerPort << "\n"
    						   << " TURN_SERVER_IP "   << m_TURNServerHostName << "\n"
    						   << " TURN_SERVER_PORT " << m_TURNServerPort;

    return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CIceStandardParams::GetIsServerPassword()const
{
	return m_IsServerPassword;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CIceStandardParams::GetPasswordServerIp()const
{
	return m_PasswordServerIp;
}
/////////////////////////////////////////////////////////////////////////////
const char* CIceStandardParams::GetPasswordServerHostName()const
{
	return m_PasswordServerHostName.c_str();; // IPv4/Ipv6/Hostname
}
/////////////////////////////////////////////////////////////////////////////
WORD CIceStandardParams::GetPasswordServerPort()const
{
	return m_PasswordServerPort;
}
/////////////////////////////////////////////////////////////////////////////
const char* CIceStandardParams::GetPasswordServerUserName()const
{
	return m_PasswordServerUserName.c_str();
}
/////////////////////////////////////////////////////////////////////////////
const char* CIceStandardParams::GetPasswordServerPassword()const
{
	return m_PasswordServerPassword.c_str();
}
/////////////////////////////////////////////////////////////////////////////
DWORD CIceStandardParams::GetSTUNServerIp()const
{
	return m_STUNServerIp;
}
const char* CIceStandardParams::GetSTUNServerHostName()const
{
	return m_STUNServerHostName.c_str();; // IPv4/IPv6/Hostname
}
/////////////////////////////////////////////////////////////////////////////
DWORD CIceStandardParams::GetTURNServerIp()const
{
	return m_TURNServerIp;
}
/////////////////////////////////////////////////////////////////////////////
const char* CIceStandardParams::GetTURNServerHostName()const
{
	return m_TURNServerHostName.c_str();; // IPv4/IPv6/Hostname
}
/////////////////////////////////////////////////////////////////////////////
WORD CIceStandardParams::GetSTUNServerPort()const
{
	return m_STUNServerPort;
}
WORD CIceStandardParams::GetTURNServerPort()const
{
	return m_TURNServerPort;
}
void CIceStandardParams::SetSTUNServerPort(WORD value)
{
	m_STUNServerPort = value;
}
void CIceStandardParams::SetTURNServerPort(WORD value)
{
	m_TURNServerPort = value;
}

/////////////////////////////////////////////////////////////////////////////
CManagementSecurity::CManagementSecurity()
{
	m_isOCSPEnabled = FALSE;
	m_isRequestPeerCertificate = FALSE;
	m_ocspGlobalResponderURI = "";
	m_isAlwaysUseGlobalOCSPResponder = FALSE;

	m_isUseResponderOcspURI =FALSE;
	//change default value to false BRIDGE-4866
	m_isIncompleteRevocation =FALSE;
	m_isSkipValidateOcspCert =TRUE;
	m_revocationMethodType =eNoneMethod;
}

/////////////////////////////////////////////////////////////////////////////
CManagementSecurity::~CManagementSecurity()
{

}

/////////////////////////////////////////////////////////////////////////////
void CManagementSecurity::SerializeXml( CXMLDOMElement*& pParentNode ) const
{
	CXMLDOMElement* pManagementSecurityNode = pParentNode->AddChildNode("MANAGEMENT_SECURITY");

	pManagementSecurityNode->AddChildNode("REQUEST_PEER_CERTIFICATE", m_isRequestPeerCertificate, _BOOL);
	pManagementSecurityNode->AddChildNode("ENABLE_OCSP", m_isOCSPEnabled, _BOOL);
	pManagementSecurityNode->AddChildNode("OCSP_GLOBAL_RESPONDER_URI", m_ocspGlobalResponderURI);
	pManagementSecurityNode->AddChildNode("ALWAYS_USE_GLOBAL_OCSP_RESPONDER", m_isAlwaysUseGlobalOCSPResponder, _BOOL);
	pManagementSecurityNode->AddChildNode("USE_RESPONDER_OCSP_URI", m_isUseResponderOcspURI, _BOOL);
	pManagementSecurityNode->AddChildNode("ALLOW_INCOMPLETE_REV_CHECK",m_isIncompleteRevocation, _BOOL);
	pManagementSecurityNode->AddChildNode("SKIP_VALIDATE_OCSP_CERT", m_isSkipValidateOcspCert, _BOOL);
	pManagementSecurityNode->AddChildNode("REVOCATION_METHOD", m_revocationMethodType, REVOCATION_METHOD_ENUM);
}

/////////////////////////////////////////////////////////////////////////////
int CManagementSecurity::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode, "REQUEST_PEER_CERTIFICATE", &m_isRequestPeerCertificate, _BOOL);
	GET_VALIDATE_CHILD(pActionNode, "ENABLE_OCSP", &m_isOCSPEnabled, _BOOL);
	GET_VALIDATE_ASCII_CHILD(pActionNode,"OCSP_GLOBAL_RESPONDER_URI", m_ocspGlobalResponderURI, ONE_LINE_BUFFER_LENGTH);
	GET_VALIDATE_CHILD(pActionNode, "ALWAYS_USE_GLOBAL_OCSP_RESPONDER", &m_isAlwaysUseGlobalOCSPResponder, _BOOL);

	GET_VALIDATE_CHILD(pActionNode, "USE_RESPONDER_OCSP_URI", &m_isUseResponderOcspURI, _BOOL);
	GET_VALIDATE_CHILD(pActionNode, "ALLOW_INCOMPLETE_REV_CHECK", &m_isIncompleteRevocation, _BOOL);
	GET_VALIDATE_CHILD(pActionNode, "SKIP_VALIDATE_OCSP_CERT", &m_isSkipValidateOcspCert, _BOOL);
	GET_VALIDATE_CHILD(pActionNode, "REVOCATION_METHOD", &m_revocationMethodType, REVOCATION_METHOD_ENUM);
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
void CManagementSecurity::Serialize(WORD format,CSegment& rSeg)
{
	rSeg << (BYTE) m_isRequestPeerCertificate;
	rSeg << (BYTE)m_isOCSPEnabled;
	rSeg << m_ocspGlobalResponderURI;
	rSeg <<(BYTE) m_isAlwaysUseGlobalOCSPResponder;
	rSeg <<(BYTE)m_isUseResponderOcspURI;
	rSeg <<(BYTE)m_isIncompleteRevocation;
	rSeg <<(BYTE)m_isSkipValidateOcspCert;
	rSeg << m_revocationMethodType;
}
void CManagementSecurity::DeSerialize(WORD format,CSegment& rSeg)
{
	    rSeg >> ((BYTE&) m_isRequestPeerCertificate);
		rSeg >> ((BYTE&) m_isOCSPEnabled);
		rSeg >> m_ocspGlobalResponderURI;
		rSeg >>((BYTE&) m_isAlwaysUseGlobalOCSPResponder);
		rSeg >>((BYTE&)m_isUseResponderOcspURI);
		rSeg >>((BYTE&)m_isIncompleteRevocation);
		rSeg >>((BYTE&)m_isSkipValidateOcspCert);
		rSeg >>  m_revocationMethodType;
}
/////////////////////////////////////////////////////////////////////////////
bool CManagementSecurity::operator==(const CManagementSecurity &rHnd)const
{
	if(this == &rHnd)
		return true;

	if ( m_isOCSPEnabled != rHnd.m_isOCSPEnabled )
		return false;

	if ( m_isRequestPeerCertificate != rHnd.m_isRequestPeerCertificate )
		return false;

	if ( m_ocspGlobalResponderURI != rHnd.m_ocspGlobalResponderURI )
		return false;

	if ( m_isAlwaysUseGlobalOCSPResponder != rHnd.m_isAlwaysUseGlobalOCSPResponder )
			return false;

	if ( m_isUseResponderOcspURI != rHnd.m_isUseResponderOcspURI )
				return false;
	if ( m_isIncompleteRevocation != rHnd.m_isIncompleteRevocation )
				return false;
	if ( m_isSkipValidateOcspCert != rHnd.m_isSkipValidateOcspCert )
				return false;
	if ( m_revocationMethodType != rHnd.m_revocationMethodType )
				return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CManagementSecurity::operator!=(const CManagementSecurity& other)const
{
	return (!(*this == other));
}

/////////////////////////////////////////////////////////////////////////////
BOOL CManagementSecurity::IsRequestPeerCertificate()
{
	return m_isRequestPeerCertificate;
}

/////////////////////////////////////////////////////////////////////////////
std::string	CManagementSecurity::GetOCSPGlobalResponderURI()
{
	return m_ocspGlobalResponderURI;
}


/////////////////////////////////////////////////////////////////////////////
void CManagementSecurity::SetIsRequestPeerCertificate(BOOL isEnabled)
{
	m_isRequestPeerCertificate = isEnabled;
}

/////////////////////////////////////////////////////////////////////////////
void CManagementSecurity::GetOCSPGlobalResponderURI(std::string URI)
{
	m_ocspGlobalResponderURI = URI;
}

/////////////////////////////////////////////////////////////////////////////
BOOL	CManagementSecurity::GetIsUseResponderOcspURI()
{
	return m_isUseResponderOcspURI;
}
/////////////////////////////////////////////////////////////////////////////
BOOL	CManagementSecurity::GetIsIncompleteRevocation()
{
	return m_isIncompleteRevocation;
}
/////////////////////////////////////////////////////////////////////////////
BOOL	CManagementSecurity::GetIsSkipValidateOcspCert()
{
	return m_isSkipValidateOcspCert;
}
/////////////////////////////////////////////////////////////////////////////
BYTE 	CManagementSecurity::GetRevocationMethodType()
{
	return m_revocationMethodType;
}
/////////////////////////////////////////////////////////////////////////////
void	CManagementSecurity::SetIsUseResponderOcspURI(BOOL isUseResponderOcspURI)
{
	m_isUseResponderOcspURI = isUseResponderOcspURI;
}
/////////////////////////////////////////////////////////////////////////////
void	CManagementSecurity::SetIsIncompleteRevocation(BOOL isIncompleteRevocation)
{
	m_isIncompleteRevocation = m_isIncompleteRevocation;
}
/////////////////////////////////////////////////////////////////////////////
void	CManagementSecurity::SetIsSkipValidateOcspCert(BOOL isSkipValidateOcspCert)
{
	m_isSkipValidateOcspCert = isSkipValidateOcspCert;
}

void	CManagementSecurity::SetRevocationMethodType(BYTE revocationMethodType)
{
	m_revocationMethodType = revocationMethodType;
}

