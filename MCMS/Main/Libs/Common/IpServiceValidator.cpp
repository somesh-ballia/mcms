// IpServiceValidator.cpp

#include "IpServiceValidator.h"

#include <ctype.h>
#include <stdlib.h>

#include "IpService.h"
#include "IfConfig.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "StringsLen.h"
#include "SystemFunctions.h"
#include "HlogApi.h"
#include "FaultsDefines.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ProcessBase.h"
#include "ObjString.h"
#include "ConfigManagerApi.h"

class CCSMngrProcess;

extern char* IpTypeToString(APIU32 ipType, bool caps = false);

CIpServiceValidator::CIpServiceValidator(CIPService & service)
:m_Service(service)
{
}

CIpServiceValidator::~CIpServiceValidator()
{
}

STATUS CIpServiceValidator::ValidateFullCS(CObjString &errorMsg)
{
  STATUS status = ValidateBase(errorMsg);
  if (STATUS_OK != status)
  {
    return status;
  }

  status = ValidateSpanList(errorMsg);
  if (STATUS_OK != status)
  {
    return status;
  }

  status = ValidateIpAddress(errorMsg);
  if (STATUS_OK != status)
  {
    return status;
  }
  
  status = ValidateIpAddressWithMask(errorMsg);
  if (STATUS_OK != status)
  {
    return status;
  }

  status = ValidateGK(errorMsg);
  if (STATUS_OK != status)
  {
    return status;
  }

  status = ValidateTCPPortRange(errorMsg);
  if (STATUS_OK != status)
  {
    return status;
  }

  status = ValidateIpAddressLocality(errorMsg);
  if (STATUS_OK != status)
  {
    return status;
  }

  status = ValidateSipServers(errorMsg);
  if (STATUS_OK != status)
  {
    return status;
  }

  return status;
}

STATUS CIpServiceValidator::ValidateFullMNGMNT(CObjString &errorMsg)
{
  STATUS status = ValidateIpAddress(errorMsg);
  if (STATUS_OK != status)
  {
    return status;
  }

  status = ValidateIpAddressWithMask(errorMsg);
  if (STATUS_OK != status)
  {
    return status;
  }

  status = ValidateIpAddressLocality(errorMsg);
  if (STATUS_OK != status)
  {
    return status;
  }

  status = ValidateDnsServers(errorMsg);
  if (STATUS_OK != status)
  {
    return status;
  }

  return status;
}

STATUS CIpServiceValidator::ValidateBase(CObjString &errorMsg)
{
	const char *serviceName = m_Service.GetName();
	if(NULL == serviceName)
	{
		PTRACE(eLevelInfoNormal, "CIpServiceValidator::ValidateBase FAILED : NULL == serviceName");
		return STATUS_SERVICE_NO_NAME;
	}

	return STATUS_OK;
}

STATUS CIpServiceValidator::ValidateGK(CObjString &errorMsg)
{
	// GK is not mandatory in ip service.
	STATUS status = STATUS_OK;

	BOOL isGKinService = m_Service.IsContainGK();
	if(FALSE == isGKinService)
	{
		return STATUS_OK;
	}

	// prefix OR at least one alias.

	const CH323Alias *prefix = m_Service.GetDialInPrefix();
	const char *prefixName = prefix->GetAliasName();
	if('\0' != *prefixName)
	{
		status = ValidateGKPrefixName(prefixName);
		if(STATUS_OK != status)
		{
			TRACEINTO << "\nCIpServiceValidator::ValidateGK FAILED : prefix is not a valid : " << prefixName;
			return status;
		}
	}

	CIPSpan *span = m_Service.GetFirstSpan();
	if(NULL == span)
	{
		PTRACE(eLevelInfoNormal, "CIpServiceValidator::ValidateGK FAILED : No Spans In Service");
		return STATUS_SERVICE_NO_SPAN;
	}

	CH323Alias *alias = span->GetFirstAlias();

	const char* aliasName;

	// must have prefix or at least one alias
	if('\0' == *prefixName)
	{
		if(alias == NULL)
		{
			PTRACE(eLevelInfoNormal, "CIpServiceValidator::ValidateGK FAILED : Alias is NULL");
			return STATUS_GK_ALIAS_EMPTY_NAME;
		}
		else
		{
			aliasName = alias->GetAliasName();
			if (*aliasName == '\0')
			{
				PTRACE(eLevelInfoNormal, "CIpServiceValidator::ValidateGK FAILED : Empty alias name");
				return STATUS_GK_ALIAS_EMPTY_NAME;
			}
		}
	}

	// check aliases validity
	while(NULL != alias)
	{
		aliasName = alias->GetAliasName();
		if('\0' != *aliasName)
		{
			//TODO: XML_API_MAX_SERVICE_ALIAS_LEN shall be replace by a new common constant definition
			//      which will represent a general max alias length on next version
			if( strlen(aliasName) > XML_API_MAX_SERVICE_ALIAS_LEN)
			{
				TRACEINTO << "\nCIpServiceValidator::ValidateGK FAILED : service alias is too long : " << aliasName;
				return STATUS_ALIAS_NAME_TOO_LONG;
			}
			status = alias->TestValidity();
			if( status != STATUS_OK)
				return status;
		}
		alias = span->GetNextAlias();
	}

    const CSmallString & gkName = m_Service.GetGatekeeperName();
    if("" == gkName || "0.0.0.0" == gkName)
    {
        errorMsg = "GK address cannot be empty or 0.0.0.0";
        TRACEINTO << "\nCIpServiceValidator::ValidateGK FAILED : GK ip is 0 : ";
        return STATUS_INVALID_SERVICE;
    }

    // Validate name compatibility with ip type
    eIpType ipType = m_Service.GetIpType();
    const char * gkNameCh = gkName.GetString();
    if((ipType == eIpType_IpV6 && ::isIpV4Str(gkNameCh)) ||
       (ipType == eIpType_IpV4 && ::isIpV6Str(gkNameCh)) )
    {
    	TRACEINTO << __FUNCTION__ << ": ip type doesn't match gk name ip type - "
				  << "ipType = " << ::IpTypeToString(ipType) << ", gkName = " << gkNameCh;
	errorMsg = "Gatekeeper's ip type does not match system ip type";
    	status = STATUS_INVALID_SERVICE;
    }

	return status;
}

STATUS CIpServiceValidator::ValidateSpanList(CObjString &errorMsg)
{
	CIPSpan *span = m_Service.GetFirstSpan();
	if(NULL == span)
	{
		return STATUS_SERVICE_NO_SPAN;
	}
	return STATUS_OK;
}

STATUS CIpServiceValidator::ValidateGKPrefixName(const char *prefixName)const
{
	if(false == CObjString::IsNumeric(prefixName))
	{
		TRACEINTO << "\nCIpServiceValidator::ValidateGKPrefixName FAILED : prefix is not a numeric : " << prefixName;
		return STATUS_GK_PREFIX_NOT_NUMERIC;
	}

	const int prefixLen = strlen(prefixName);
	if( PHONE_NUMBER_DIGITS_LEN  < prefixLen)
	{
		TRACEINTO << "\nCIpServiceValidator::ValidateGKPrefixName FAILED : prefix contains more then 10 digits : " << prefixName;
		return STATUS_GK_PREFIX_TOO_MANY_DIGITS;
	}

	return STATUS_OK;
}

STATUS CIpServiceValidator::ValidateIpAddressWithMask(CObjString &errorMsg)
{
	// Ninja&Gesher need to check the IpAddress with mask.
    eProductType prodType = CProcessBase::GetProcess()->GetProductType();
    if(FALSE == IsTarget() && eProductTypeGesher != prodType && eProductTypeNinja != prodType) // no configuration is actually done
	{
		return STATUS_OK;
	}

    eIpType curIpType = m_Service.GetIpType();
    if (eIpType_IpV6 == curIpType) // no need to validate IPv4
    {
    	return STATUS_OK;
    }

	STATUS retStatus = STATUS_OK;

	CIPSpan *pSpan = m_Service.GetFirstSpan();
	if (NULL != pSpan)
	{
		DWORD ipAddress	= pSpan->GetIPv4Address(),
		      netMask	= m_Service.GetNetMask(),
		      defGw		= m_Service.GetDefaultGatewayIPv4();

		if ( (ipAddress & netMask) != (defGw & netMask) )
		{
			char ipAddressStr[IP_ADDRESS_LEN],
			     netMaskStr[IP_ADDRESS_LEN],
			     defGwStr[IP_ADDRESS_LEN];
			SystemDWORDToIpString(ipAddress, ipAddressStr);
			SystemDWORDToIpString(netMask,   netMaskStr);
			SystemDWORDToIpString(defGw,     defGwStr);

			TRACESTR(eLevelInfoNormal) << "\nCIpServiceValidator::ValidateIpAddressWithMask FAILED : IpAddress mismatches NetMask:"
			                       << "\nIp address: " << ipAddressStr
			                       << "\nNetMask:    " << netMaskStr
			                       << "\nDeault GW:  " << defGwStr;

			retStatus = STATUS_IP_ADDRESS_MISMATCHES_NETMASK;
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCIpServiceValidator::ValidateIpAddressWithMask SUCCEEDED";
			retStatus = STATUS_OK;
		}
	}

	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCIpServiceValidator::ValidateIpAddressWithMask FAILED : No Spans In Service";
		return STATUS_SERVICE_NO_SPAN;
	}


	return retStatus;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CIpServiceValidator::ValidateTCPPortRange(CObjString &errorMsg)
{
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	
    bool isUserDefine = m_Service.IsUserDefinePorts();
    if(false == isUserDefine)
    {
    	// the port allocation will be done by CSMngr.
    	return STATUS_OK;
    }

    CIPSpan* pSpan = m_Service.GetFirstSpan();
    if(NULL == pSpan)
    {
    	return STATUS_SERVICE_NO_SPAN;
    }

    CCommH323PortRange* portRange = pSpan->GetPortRange();
    WORD numOfTcpPorts = portRange->GetTcpNumberOfPorts();
    //VNGFE-2553 - the decision was to remove this validation test
    /*    if(0 != (numOfTcpPorts % 2))
	{
		return STATUS_ODD_NUM_TCP_PORTS;
	}*/
    //VNGFE-2553 - For RMX 2000 minimum ports 4 maximum ports 1600.
    //For RMX 4000 minimum ports 8 maximum ports 3200.

    int minPortNum = 4;
    int maxPortNum = 1600;

    if (curProductType == eProductTypeRMX4000)
    {
    	minPortNum=8;
    	maxPortNum=3200;
    }

    if (curProductType == eProductTypeSoftMCUMfw)
    {
       	minPortNum = 8;
       	maxPortNum = 2000;
    }

    if(minPortNum > numOfTcpPorts || maxPortNum < numOfTcpPorts)
    {
    	return STATUS_ILLEGAL_NUM_TCP_PORTS;
    }
    return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CIpServiceValidator::ValidateIpAddressLocality(CObjString &errorMsg)
{
    eIpType curIpType = m_Service.GetIpType();
    if (eIpType_IpV6 == curIpType) // no need to validate IPv4
    {
    	return STATUS_OK;
    }

    CIPSpan* pSpan = m_Service.GetFirstSpan();
    if(NULL == pSpan)
    {
        return STATUS_SERVICE_NO_SPAN;
    }
    DWORD ipAddress = pSpan->GetIPv4Address();

     // the format should be little endian
    BYTE current[4];
    current[0] = GET_LE_IP_BYTE_1(ipAddress);
    current[1] = GET_LE_IP_BYTE_2(ipAddress);
    current[2] = GET_LE_IP_BYTE_3(ipAddress);
    current[3] = GET_LE_IP_BYTE_4(ipAddress);

    BYTE min[4] = {169, 254, 1, 0};
    BYTE max[4] = {169, 254, 254, 255};

    bool isInRange = true;
    DWORD len = sizeof(current) / sizeof(current[0]);
    for(DWORD i = 0 ; i < len ; i++)
    {
        isInRange = (min[i] <= current[i] && current[i] <= max[i]);
        if(!isInRange)
        {
            break;
        }
    }

    STATUS retStatus = STATUS_OK;

    // print to trace
	char ipAddressStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString(ipAddress, ipAddressStr);

	if (!isInRange)
    {
    	// '!isInRange' means that the address is not local (and that's good...)
    	retStatus = STATUS_OK;
    	TRACESTR(eLevelInfoNormal) << "\nCIpServiceValidator::ValidateIpAddressLocality SUCCEEDED"
    						   << "\nThe Address " << ipAddressStr
    						   << " is not within Local range (169.254.1.0 - 169.254.254.255)";
    }
    else
    {
    	retStatus = STATUS_SERVICE_LOCAL_IP_ADDRESS;
    	TRACESTR(eLevelInfoNormal) << "\nCIpServiceValidator::ValidateIpAddressLocality FAILED :"
    						   << "\nThe Address " << ipAddressStr
    						   << " is within Local range (169.254.1.0 - 169.254.254.255)";
    }

    return retStatus;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CIpServiceValidator::ValidateIpAddress(CObjString &errorMsg)
{
	STATUS retStatus = STATUS_OK;
	CIPSpan* pSpan = m_Service.GetFirstSpan();
	if(NULL == pSpan)
	  {
	        return STATUS_SERVICE_NO_SPAN;
	 }

    eIpType curIpType = m_Service.GetIpType();

    if (eIpType_IpV6 == curIpType) // no need to validate IPv4
    {
    	eV6ConfigurationType configType	= m_Service.GetIpV6ConfigurationType();
    	if(eV6Configuration_Manual == configType)
    	{
    		char ip_server1Str[IPV6_ADDRESS_LEN];
    		 memset(ip_server1Str, 0, IPV6_ADDRESS_LEN);
    		pSpan->GetIPv6Address(0,ip_server1Str);
    		mcTransportAddress tmpIPv6Addr;
    		::stringToIpV6(&tmpIPv6Addr,ip_server1Str);
    		if(isApiTaNull(&tmpIPv6Addr))
    			return STATUS_IP_ADDRESS_NOT_VALID;
    	}
    	return STATUS_OK;
    }


    DWORD ipAddress = pSpan->GetIPv4Address();



    // print to trace
	char ipAddressStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString(ipAddress, ipAddressStr);

	// if ipAdress is 0.0.0.0, return invalied IP Address.
    if (0 != ipAddress)
    {
    	retStatus = STATUS_OK;
    	TRACESTR(eLevelInfoNormal) << "\nCIpServiceValidator::ValidateIpAddress SUCCEEDED"
    						   << "\nThe Address " << ipAddressStr;
    }
    else
    {
    	retStatus = STATUS_IP_ADDRESS_NOT_VALID;
    	TRACESTR(eLevelInfoNormal) << "\nCIpServiceValidator::ValidateIpAddress FAILED :"
    						   << "\nThe Address " << ipAddressStr;
    }

    return retStatus;
}

STATUS CIpServiceValidator::ValidateSipServers(CObjString &errorMsg)
{
    TRACEINTO << __FUNCTION__;
    STATUS status = STATUS_OK;

    const CSip *pSip = m_Service.GetpSip();
    if(pSip == NULL)
    {
        PASSERTMSG(TRUE, "pSip == NULL");
        return STATUS_FAIL;
    }

    const CSipServer *pSipRegistar = pSip->GetpRegistrar();
    const CSipServer *pAltSipRegistrar = pSip->GetpAltRegistrar();

    eServerStatus sipRegistrarStatus =  pSipRegistar->GetStatus();
    eServerStatus altSipRegistrarStatus =  pAltSipRegistrar->GetStatus();
    if(eServerStatusOff == sipRegistrarStatus)
    {
        status = STATUS_OK;
    }
    else
    {
		DWORD registrationTout = pSip->GetRefreshRegistrationTout();
		bool isInRange = (300 <=  registrationTout && registrationTout <= 3600);
		status = (isInRange ? STATUS_OK : STATUS_INVALID_REGISTRATION_REFRESH_TIMEOUT);

		if(STATUS_OK == status)
		{
			eConfigurationSipServerMode curSipMode = pSip->GetConfigurationOfSIPServers();
			
			if (eConfSipServerAuto != curSipMode) // 'eConfSipServerAuto' means 'Off' (in GUI)
			{
				// Validate name compatibility with ip type
				eIpType ipType = m_Service.GetIpType();
				const CBaseSipServer* pProxy = pSip->GetpProxy();
				eServerStatus proxStatus = pProxy->GetStatus();
	
				const char * regDomName = pSipRegistar->GetDomainName().GetString();
				const char * regName 	= pSipRegistar->GetName().GetString();
	
				const char * proxName 	= pProxy->GetName().GetString();
	
				const char * altRegDomName = pAltSipRegistrar->GetDomainName().GetString();
				const char * altRegName    = pAltSipRegistrar->GetName().GetString();
	
				if(ipType == eIpType_IpV6)
				{
					if(isIpV4Str(regDomName) ||
					   isIpV4Str(regName) ||
					   (eServerStatusOff != proxStatus && isIpV4Str(proxName)) ||
	                                   ((eServerStatusOff != altSipRegistrarStatus) && (isIpV4Str(altRegDomName) || isIpV4Str(altRegName)) )
	                                  )
						status = STATUS_INVALID_SERVICE;
				}
				else if(ipType == eIpType_IpV4)
				{
					if(isIpV6Str(regDomName) ||
					   isIpV6Str(regName) ||
					   (eServerStatusOff != proxStatus && isIpV6Str(proxName)) ||
	                                   ((eServerStatusOff != altSipRegistrarStatus) && (isIpV6Str(altRegDomName) || isIpV6Str(altRegName)) )
					  )
						status = STATUS_INVALID_SERVICE;
				}
	
				if(STATUS_INVALID_SERVICE == status)
				{
					errorMsg = "The IP type of the SIP server does not match the IP type of the system";
					TRACEINTO << __FUNCTION__ << ": " << errorMsg;
				}
			} // end if (eConfSipServerAuto != curSipMode)
		} // end if (STATUS_OK == status)
    }


    	return status;
}

STATUS CIpServiceValidator::ValidateDnsServers(CObjString &errorMsg)
{
    CIpDns* pDns = m_Service.GetpDns();
    if (eServerStatusSpecify != pDns->GetStatus())
    {
        // validation is needed only in 'specify'
        return STATUS_OK;
    }

    bool isLegalIPv4AddressExists = false,
    	 isLegalIPv6AddressExists = false;

    int curAddress = 0;
    char curIPv6Addr[IPV6_ADDRESS_LEN];

    for (int i=0; i < NUM_OF_DNS_SERVERS; i++)
    {
		// IPv4
		curAddress = pDns->GetIPv4Address(i);
		if (0 != curAddress) // not empty
		{
			isLegalIPv4AddressExists = true;
		}

		// IPv6
		memset(curIPv6Addr, 0, IPV6_ADDRESS_LEN);
		pDns->GetIPv6Address(i, curIPv6Addr);
		if (strcmp(curIPv6Addr, "") && strcmp(curIPv6Addr, "::") && strcmp(curIPv6Addr, "::/64" ))  // not empty
		{
			TRACESTR(eLevelInfoNormal) << "\nCIpServiceValidator:: curIPv6Addr is not empty";
			if ( false == CObjString::IsContainsSpecialChars(curIPv6Addr) ) // does not contain illegal chars
			{
				TRACESTR(eLevelInfoNormal) << "\nCIpServiceValidator:: curIPv6Addr is not contain special chars";
				isLegalIPv6AddressExists = true;
			}
		}
    }


    // calculate results
    STATUS retStatus = STATUS_OK;
    bool isValidDns = true;
    eIpType curIpType = m_Service.GetIpType();

    if ( ((eIpType_IpV4 == curIpType) && (false == isLegalIPv4AddressExists))	||
    	 ((eIpType_IpV6 == curIpType) && (false == isLegalIPv6AddressExists))	||
    	 ((false == isLegalIPv4AddressExists) && (false == isLegalIPv6AddressExists)) )
    {
    	isValidDns = false;
    }

	if (isValidDns)
    {
        retStatus = STATUS_OK;
        TRACESTR(eLevelInfoNormal) << "\nCIpServiceValidator::ValidateDnsServers SUCCEEDED";
    }
    else
    {
        retStatus = STATUS_ILLEGAL_DNS_SERVERS_ADDRESSES;
        TRACESTR(eLevelInfoNormal) << "\nCIpServiceValidator::ValidateDnsServers FAILED"
        					   << "\nNo valid DNS server was provided";
    }

    return retStatus;
}

CIpServiceListValidator::CIpServiceListValidator(CIPServiceList & serviceList)
:m_ServiceList(serviceList)
{
}

CIpServiceListValidator::~CIpServiceListValidator()
{
}

STATUS CIpServiceListValidator::ValidateSingleCS(CIPService & service, bool isMustExist, bool isMustNotExist, CObjString &errorMsg)const
{
	STATUS status = CIpServiceValidator(service).ValidateFullCS(errorMsg);
	if(STATUS_OK != status)
	{
		return status;
	}

	const char* serviceName = service.GetName();
	const int ret = m_ServiceList.FindService(serviceName);
	if(true == isMustExist)
	{
		if(ret == NOT_FIND)
		{
			return STATUS_SERVICE_NOT_EXIST;
		}
	}
	else if(true == isMustNotExist)
	{
		if(ret != NOT_FIND)
		{
			return STATUS_SERVICE_EXIST;
		}
	}

	return STATUS_OK;
}

STATUS CIpServiceListValidator::ValidateBase(CObjString &errorMsg)
{
	ValidateMethodType method = &CIpServiceValidator::ValidateBase;
	STATUS status = ApplyValidateMethod(method, errorMsg);
	return status;
}

STATUS CIpServiceListValidator::ValidateFullCS(CObjString &errorMsg)
{
	ValidateMethodType method = &CIpServiceValidator::ValidateFullCS;
	STATUS status = ApplyValidateMethod(method, errorMsg);
	return status;
}

STATUS CIpServiceListValidator::ValidateFullMNGMNT(CObjString &errorMsg)
{
	ValidateMethodType method = &CIpServiceValidator::ValidateFullMNGMNT;
	STATUS status = ApplyValidateMethod(method, errorMsg);
	return status;
}

STATUS CIpServiceListValidator::ValidateTCPPortRange(CObjString &errorMsg)
{
    ValidateMethodType method = &CIpServiceValidator::ValidateTCPPortRange;
	STATUS status = ApplyValidateMethod(method, errorMsg);
	return status;
}

STATUS CIpServiceListValidator::ValidateIpAddressLocality(CObjString &errorMsg)
{
    ValidateMethodType method = &CIpServiceValidator::ValidateIpAddressLocality;
	STATUS status = ApplyValidateMethod(method, errorMsg);
	return status;
}

STATUS CIpServiceListValidator::ValidateIpAddress(CObjString &errorMsg)
{
    ValidateMethodType method = &CIpServiceValidator::ValidateIpAddress;
	STATUS status = ApplyValidateMethod(method, errorMsg);
	return status;
}

STATUS CIpServiceListValidator::ApplyValidateMethod(ValidateMethodType method, CObjString &errorMsg)
{
	STATUS status = STATUS_NO_SERVICES;
	CIPService *service = m_ServiceList.GetFirstService();
	while(NULL != service)
	{
		status = (CIpServiceValidator(*service).*method)(errorMsg);
		if(STATUS_OK != status)
		{
			break;
		}
		service = m_ServiceList.GetNextService();
	}
	return status;
}

int CIpServiceListValidator::RemoveSipTls(CObjString &errorMsg)
{
  int numOfTlsCorrections = 0;
  STATUS status = STATUS_NO_SERVICES;
  ValidateMethodType method = &CIpServiceValidator::ValidateSipServers;
  CIPService *service = m_ServiceList.GetFirstService();
  while (NULL != service)
  {
    status = (CIpServiceValidator(*service).*method)(errorMsg);
    if (STATUS_SIP_TLS_NOT_SUPPORTED_IN_JITC_MODE == status)
    {
      numOfTlsCorrections++;
      CSip *pSip = service->GetpSip();
      pSip->SetTransportType(eTransportTypeUdp);
      CLargeString wrongTransportTypeDescription;
      wrongTransportTypeDescription
          << "TLS value as SIP transport type is forbidden in JITC mode: "
          << "Modify transport type to UDP. " << "Service Name: "
          << service->GetName();
      CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
          SIP_FORBIDDEN_TRANSPORT_TYPE_IN_JITC_MODE, MAJOR_ERROR_LEVEL,
          wrongTransportTypeDescription.GetString(), FALSE);

    }
    service = m_ServiceList.GetNextService();
  }

  return numOfTlsCorrections;
}
