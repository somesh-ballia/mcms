// IpServiceListManager.cpp: List IP services in ConfParty process.
//
//////////////////////////////////////////////////////////////////////


#include <algorithm>
#include <vector>
#include "IpServiceListManager.h"
#include "IpService.h"
#include "StatusesGeneral.h"
#include "ObjString.h"

//////////////////////////////////////////////////////////////////////
CIpServiceListManager::CIpServiceListManager()
{
	m_ipConfParamsVector = new VECTOR_OF_CONF_IP_PARAMS;
}
//////////////////////////////////////////////////////////////////////
CIpServiceListManager::~CIpServiceListManager()
{
	Clear();
	PDELETE(m_ipConfParamsVector);
}
//////////////////////////////////////////////////////////////////////
void CIpServiceListManager::Clear()
{
	VECTOR_OF_CONF_IP_PARAMS::iterator it = m_ipConfParamsVector->begin();
	for( ;  it != m_ipConfParamsVector->end() ; ++it)
		POBJDELETE(*it);
	m_ipConfParamsVector->clear();
}
//////////////////////////////////////////////////////////////////////
WORD CIpServiceListManager::numberOfIpServices()
{
	return m_ipConfParamsVector->size();
}
//////////////////////////////////////////////////////////////////////
STATUS CIpServiceListManager::insertIpService(CConfIpParameters* pConfIpParameters)
{
	 STATUS insertStatus = STATUS_OK;
	 if(!FindIpService(pConfIpParameters->GetServiceId()))
	  m_ipConfParamsVector->push_back(pConfIpParameters);
	 else
	  insertStatus = STATUS_FAIL;
	 return insertStatus;
}
//////////////////////////////////////////////////////////////////////
STATUS CIpServiceListManager::updateIpService(CConfIpParameters* pNewConfIpParameters)
{
	DWORD serviceID = pNewConfIpParameters->GetServiceId();
	CConfIpParameters* pRemovedConfIpParameters = removeIpService(serviceID);
	if (pRemovedConfIpParameters == NULL)
		PASSERTMSG(serviceID, "CIpServiceListManager::updateIpService - IP service is not found!!!");
	else
		if (CPObject::IsValidPObjectPtr(pRemovedConfIpParameters))
			POBJDELETE(pRemovedConfIpParameters);
		
	return insertIpService(pNewConfIpParameters);			 
}

/////////////////////////////////////////////////////////////////////
CConfIpParameters* CIpServiceListManager::removeIpService(DWORD serviceID)
{
	CConfIpServiceHunterByServiceID serviceHunter;
	serviceHunter.serviceID = serviceID;
	CConfIpParameters* pDeletedConfIpParameters = NULL;

	int size = m_ipConfParamsVector->size();
	VECTOR_OF_CONF_IP_PARAMS::iterator iTer = m_ipConfParamsVector->begin();
	VECTOR_OF_CONF_IP_PARAMS::iterator iEnd = m_ipConfParamsVector->end();

	VECTOR_OF_CONF_IP_PARAMS::iterator it;
	it = std::find_if(iTer, iEnd, serviceHunter);
	pDeletedConfIpParameters = *it;
	if(CPObject::IsValidPObjectPtr(pDeletedConfIpParameters))
	{
		 m_ipConfParamsVector->erase(it);
	}
	else
	{
		pDeletedConfIpParameters = NULL;
	}
	return pDeletedConfIpParameters;
}
/////////////////////////////////////////////////////////////////////////////
const char* CIpServiceListManager::FindServiceAndGetStringWithoutPrefix( const char* prefixPlusString, WORD prefixType, BOOL* pServiceMatched)
{
	
	if (pServiceMatched) *pServiceMatched = NO;

	CConfIpServiceHunterByDialinString dialinHunter;
	strncpy(dialinHunter.dialinString, prefixPlusString, sizeof(dialinHunter.dialinString) - 1);
	dialinHunter.dialinString[sizeof(dialinHunter.dialinString) - 1] ='\0';
	std::vector<CConfIpParameters*>::iterator it = std::find_if (m_ipConfParamsVector->begin(), m_ipConfParamsVector->end (), dialinHunter);
	if(it == m_ipConfParamsVector->end())
		return NULL;
	
	const char* servicePrefix = (const char*)((*it)->GetDialIn().aliasContent);
	
	if (servicePrefix)
    {
       WORD len = strlen(servicePrefix);
              
        if (len && (len <= strlen(prefixPlusString)) ){
            if ( !strncmp(prefixPlusString, servicePrefix, len) )
			{
				if (pServiceMatched) *pServiceMatched = YES;
               return &(prefixPlusString[len]);
			}
    	}
    }
    return NULL;
    
}
/////////////////////////////////////////////////////////////////////
CConfIpParameters* CIpServiceListManager::FindIpService(DWORD serviceID)
{
	CConfIpServiceHunterByServiceID serviceHunter;
	serviceHunter.serviceID = serviceID;
	std::vector<CConfIpParameters*>::iterator it = std::find_if (m_ipConfParamsVector->begin(), m_ipConfParamsVector->end (), serviceHunter);
	if(it == m_ipConfParamsVector->end())
		return NULL;
	return *it;
}
/////////////////////////////////////////////////////////////////////
CConfIpParameters* CIpServiceListManager::FindServiceByName( const char* serviceName)
{
	CConfIpServiceHunterByName serviceNameHunter;
	WORD sizeOfServiceName = sizeof(serviceNameHunter.serviceName); 
	memset(serviceNameHunter.serviceName,'\0',sizeof(serviceNameHunter.serviceName));
	strncpy(serviceNameHunter.serviceName,serviceName,sizeof(serviceNameHunter.serviceName) - 1);
	serviceNameHunter.serviceName[sizeof(serviceNameHunter.serviceName) - 1] = '\0';
	std::vector<CConfIpParameters*>::iterator it = std::find_if (m_ipConfParamsVector->begin(), m_ipConfParamsVector->end (), serviceNameHunter);
	if(it == m_ipConfParamsVector->end())
		return NULL;
	return *it;
}
////////////////////////////////////////////////////////////////////
CConfIpParameters* CIpServiceListManager::FindServiceByIPAddress( const mcTransportAddress IPAddress)
{
	CConfIpServiceHunterByIPAddress IPAddressHunter;
	IPAddressHunter.IPAddress = IPAddress;
	std::vector<CConfIpParameters*>::iterator it = std::find_if (m_ipConfParamsVector->begin(), m_ipConfParamsVector->end (), IPAddressHunter);
	if(it == m_ipConfParamsVector->end())
		return NULL;
	return *it;
}

////////////////////////////////////////////////////////////////////
CConfIpParameters* CIpServiceListManager::FindServiceBySIPDomain(const char* pDestAddr)
{
	CConfIpParameters* pResult = NULL;
	std::vector<CConfIpParameters*>::iterator it = m_ipConfParamsVector->begin();
	while(it != m_ipConfParamsVector->end())
	{
		pResult = (*it);
		if(strstr(pDestAddr, pResult->GetRegistrarDomainName().GetString()) != NULL)
			return pResult;
		it++;
	}
	return NULL;
}
////////////////////////////////////////////////////////////////////
CConfIpParameters* CIpServiceListManager::GetFirstServiceInList()
{
	CConfIpParameters* pFirstServiceInList = NULL;
	if(m_ipConfParamsVector->size() ==0)
	return pFirstServiceInList;
	
	pFirstServiceInList = *(m_ipConfParamsVector->begin());
	return pFirstServiceInList;
}
///////////////////////////////////////////////////////////////////
CConfIpParameters* CIpServiceListManager::GetRelevantService(const char* serviceName, BYTE netInterfaceType /*= H323_INTERFACE_TYPE*/)
{
	WORD serviceNameLen = 0;
	if( serviceName )
		serviceNameLen = strlen(serviceName);
	WORD isServiceisInList = FALSE;
	CConfIpParameters* pServiceParams =  NULL;
	 
	if(serviceNameLen)
	{
		pServiceParams =  FindServiceByName(serviceName);
		if(pServiceParams)
			isServiceisInList = TRUE;
	}

	if( (serviceNameLen == 0) || isServiceisInList == 0 )
	{
		if( H323_INTERFACE_TYPE == netInterfaceType || SIP_INTERFACE_TYPE == netInterfaceType)
		{
			std::vector<CConfIpParameters*>::iterator it = m_ipConfParamsVector->begin();
			while( it != m_ipConfParamsVector->end() )
			{
				BYTE isDefaultH323Sip = (*it)->GetIsDefaultH323SipService();
				if( ( DEFAULT_SERVICE_BOTH == isDefaultH323Sip ) || //default for both net interface types
					( DEFAULT_SERVICE_H323 == isDefaultH323Sip && H323_INTERFACE_TYPE == netInterfaceType) ||
					( DEFAULT_SERVICE_SIP  == isDefaultH323Sip &&  SIP_INTERFACE_TYPE == netInterfaceType) )
				{
					pServiceParams = (*it);
					break;
				}
				it++;
			}
		}
		if( pServiceParams == NULL )
			pServiceParams = GetFirstServiceInList();
	   
		if( pServiceParams == NULL )
		{
			PASSERTMSG(1,"IpServiceListManager::GetRelevantService - IP ServiceList is empty, can't configure Default service!!!");
		}
	}
	return pServiceParams;
}
///////////////////////////////////////////////////////////////////
void CIpServiceListManager::SetDefaultIpServiceType(const char* defaultH323serv, const char* defaultSIPserv)
{
	BOOL isSameDefault = FALSE;
	if( defaultH323serv && defaultSIPserv && !strcmp(defaultH323serv, defaultSIPserv) )
		isSameDefault = TRUE;

	std::vector<CConfIpParameters*>::iterator itr = m_ipConfParamsVector->begin();
	while( itr != m_ipConfParamsVector->end() )
	{
		CONF_IP_PARAMS_S* ip_service_itr = (*itr)->GetConfIpParamsStruct();
		if( !ip_service_itr )
			continue;
    	if(defaultH323serv && !strcmp( (const char*)(*itr)->GetServiceName(), defaultH323serv) )
    	{
    		BYTE service_default_type = (isSameDefault ? DEFAULT_SERVICE_BOTH : DEFAULT_SERVICE_H323);
    		ip_service_itr->service_default_type = service_default_type;
    	}
    	else if(defaultSIPserv && !strcmp( (const char*)(*itr)->GetServiceName(), defaultSIPserv) )
    	{
    		BYTE service_default_type = (isSameDefault ? DEFAULT_SERVICE_BOTH : DEFAULT_SERVICE_SIP);
       		ip_service_itr->service_default_type = service_default_type;
    	}
    	else
       		ip_service_itr->service_default_type = DEFAULT_SERVICE_NONE;

		itr++;
//    	TRACEINTO << " CIpServiceListManager::SetDefaultIpServiceType :  service id = "
//    			  << itr->GetServiceId() << ", name = " <<  itr->GetServiceName()
//    			  << " has default type = " << (WORD)ip_service_itr->service_default_type;
	}
}
