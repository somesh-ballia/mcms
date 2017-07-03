// CommGKService.cpp: implementation of the CCommGKService class.
//
//
//Date         Updated By         Description
//
//3/10/05	  Yuri Ratner		Communication with Gate Keaper
//========   ==============   =====================================================================

#include "CommGKService.h"
#include "H323Alias.h"
#include "WrappersGK.h"
#include "TraceStream.h"
#include "Segment.h"
#include "H323Alias.h"
#include "RvCommonDefs.h"
#include "CSMngrProcess.h"
#include "CSMngrProcess.h"
#include "IPServiceDynamicList.h"
#include "OpcodesMcmsInternal.h"
#include "InternalProcessStatuses.h"
#include "CSMngrStatuses.h"
#include "DynIPSProperties.h"
#include "IpService.h"
#include "GkTaskApi.h"



void AssertMessage(DWORD lineNumber, WORD serviceId)
{
	CLargeString buff;
    buff << "Line : " << lineNumber
         << "; No Service with id : " << serviceId;
	FPASSERTMSG(TRUE, buff.GetString());
}
#define ASSERT_NO_SERVICE(serviceId) ( AssertMessage(__LINE__, (serviceId) ) )








CCommGKService::CCommGKService()
{
}

CCommGKService::~CCommGKService()
{
}

////////////////////////////////////////////
STATUS CCommGKService::SendServiceCfgList(CSegment *pSeg)
{
	STATUS res = SendToMcmsProcess(eProcessGatekeeper, CS_SERVICE_CFG_UPDATE_IND, pSeg);
	return res;

}

STATUS CCommGKService::SendIpServiceParamInd(CIPService *service)
{
	bool res = IsServiceReady(service);
	//printf("CCommGKService::SendIpServiceParamInd service %d\n",service->GetId() );
	if (false == res)
	{
		PTRACE(eLevelInfoNormal, "CCommGKService::SendIpServiceParamInd - IsServiceReady==false");
		// no data from CS Module was accepted for this service.
		return STATUS_NO_DATA_COME_FROM_CS_MODULE;
	}

	GkManagerServiceParamsIndStruct param;
	memset(&param, 0, sizeof(GkManagerServiceParamsIndStruct));
	res = FillParams(param, service);
	if(!res)
	{
		PTRACE(eLevelInfoNormal, "CCommGKService::SendIpServiceParamInd - FillParams==!ok");
		return STATUS_CORRUPTED_IP_SERVICE;
	}

	TRACEINTO << CGkManagerServiceParamsIndStructWrapper(param);

	CSegment *pSeg = new CSegment;
	pSeg->Put((BYTE*)&param,sizeof(GkManagerServiceParamsIndStruct));

	//STATUS status = SendToMcmsProcess(eProcessGatekeeper, CS_GKMNGR_IP_SERVICE_PARAM_IND, pSeg);
	CGatekeeperTaskApi api(service->GetId());
	STATUS status = api.SendMsg(pSeg, CS_GKMNGR_IP_SERVICE_PARAM_IND);


	return status;
}


STATUS CCommGKService::SendUpdateIpServiceParamInd(CIPService *service)
{
	bool res = IsServiceReady(service);
	//printf("CCommGKService::SendIpServiceParamInd service %d\n",service->GetId() );
	if (false == res)
	{
		PTRACE(eLevelInfoNormal, "CCommGKService::SendUpdateIpServiceParamInd - IsServiceReady==false");
		// no data from CS Module was accepted for this service.
		return STATUS_NO_DATA_COME_FROM_CS_MODULE;
	}

	GkManagerServiceParamsIndStruct param;
	memset(&param, 0, sizeof(GkManagerServiceParamsIndStruct));
	res = FillParams(param, service);
	if(!res)
	{
		PTRACE(eLevelInfoNormal, "CCommGKService::SendUpdateIpServiceParamInd - FillParams==!ok");
		return STATUS_CORRUPTED_IP_SERVICE;
	}

	TRACEINTO << CGkManagerServiceParamsIndStructWrapper(param);

	CSegment *pSeg = new CSegment;
	pSeg->Put((BYTE*)&param,sizeof(GkManagerServiceParamsIndStruct));

	//STATUS status = SendToMcmsProcess(eProcessGatekeeper, CS_GKMNGR_IP_SERVICE_PARAM_IND, pSeg);
	CGatekeeperTaskApi api(service->GetId());
	STATUS status = api.SendMsg(pSeg, CS_GKMNGR_IP_SERVICE_UPDATE_PARAM_IND);


	return status;
}

STATUS CCommGKService::SendIpServiceParamEndInd()
{
	return STATUS_OK;
}

STATUS CCommGKService::SendDelIpService(CIPService *service)
{
	STATUS res = SendDelIpServiceToMcmsProcess(eProcessGatekeeper, CS_GKMNGR_DELETE_IP_SERVICE_IND, service);

	return res;
}

STATUS CCommGKService::ReceiveUpdatePropertiesReq(const GkManagerUpdateServicePropertiesReqStruct &param)
{
	CIPService *pServiceDynamic = GetDynamicIpServiceIncListCnt(param.serviceId);
	if(NULL == pServiceDynamic)
	{
		ASSERT_NO_SERVICE(param.serviceId);
		return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
	}
	TRACEINTO << CGkManagerUpdateServicePropertiesReqStructWrapper(param);

	CDynIPSProperties *pDynIpService = pServiceDynamic->GetDynamicProperties();
	CGKInfo &refGKInfo = pDynIpService->GetGKInfo();
	refGKInfo.SetGKInfo(param);

    STATUS status = UpdateQOS(param);

	return status;
}

STATUS CCommGKService::UpdateQOS(const GkManagerUpdateServicePropertiesReqStruct &param)
{
    CIPService *pServiceDynamic = GetDynamicIpServiceIncListCnt(param.serviceId);
	if(NULL == pServiceDynamic)
	{
		ASSERT_NO_SERVICE(param.serviceId);
		return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
	}

    CQualityOfService *pQosDynamic = pServiceDynamic->GetpQualityOfService();
    if(NULL == pQosDynamic)
    {
        PASSERTMSG(TRUE, "NULL == pQos");
        return STATUS_FAIL;
    }

    bool isQosIsReal = (0 == param.ipPrecedenceAudio && 0 == param.ipPrecedenceVideo
                        ?
                        false : true);
    if(isQosIsReal)
    {
        pQosDynamic->SetIpPrecedenceAudio(param.ipPrecedenceAudio);
        pQosDynamic->SetIpPrecedenceVideo(param.ipPrecedenceVideo);
    }
    else
    {
        CIPServiceList *list = m_pCSProcess->GetIpServiceListStatic();
        CIPService *pServiceStatic = list->GetService(param.serviceId);
        if(NULL == pServiceStatic)
        {
            ASSERT_NO_SERVICE(param.serviceId);
            return STATUS_FAIL;
        }

        CQualityOfService *pQosStatic = pServiceStatic->GetpQualityOfService();
        *pQosDynamic = *pQosStatic;
    }

    return STATUS_OK;
}

STATUS CCommGKService::ReceiveIpInPropertiesReq(const SetGkIPInPropertiesReqStruct &param)
{
	CIPService *service = GetDynamicIpServiceIncListCnt(param.serviceId);
	if(NULL == service)
	{
		ASSERT_NO_SERVICE(param.serviceId);
		return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
	}
	TRACEINTO << CSetGkIPInPropertiesReqStructWrapper(param);

	CDynIPSProperties *pDynIpService = service->GetDynamicProperties();
	CGKInfo &refGKInfo = pDynIpService->GetGKInfo();
	refGKInfo.SetGKInfo(param);

	return STATUS_OK;
}

STATUS CCommGKService::ReceiveIdInPropertiesReq(const SetGkIdInPropertiesReqStruct &param)
{
	CIPService *service = GetDynamicIpServiceIncListCnt(param.serviceId);
	if(NULL == service)
	{
		ASSERT_NO_SERVICE(param.serviceId);
		return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
	}
	TRACEINTO << CSetGkIdInPropertiesReqStructWrapper(param);

	CDynIPSProperties *pDynIpService = service->GetDynamicProperties();
	CGKInfo &refGKInfo = pDynIpService->GetGKInfo();
	refGKInfo.SetGKInfo(param);

	return STATUS_OK;
}

STATUS CCommGKService::ReceiveNameInPropertiesReq(const SetGkNameInPropertiesReqStruct &param)
{
	CIPService *service = GetDynamicIpServiceIncListCnt(param.serviceId);
	if(NULL == service)
	{
		ASSERT_NO_SERVICE(param.serviceId);
		return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
	}

	TRACEINTO << CSetGkNameInPropertiesReqStructWrapper(param);

	CDynIPSProperties *pDynIpService = service->GetDynamicProperties();
	CGKInfo &refGKInfo = pDynIpService->GetGKInfo();
	refGKInfo.SetGKInfo(param);

	return STATUS_OK;
}

STATUS CCommGKService::ReceiveClearPropertiesReq(const ClearGkParamsFromPropertiesReqStruct &param)
{
	CIPService *service = GetDynamicIpServiceIncListCnt(param.serviceId);
	if(NULL == service)
	{
		ASSERT_NO_SERVICE(param.serviceId);
		return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
	}
	TRACEINTO << CClearGkParamsFromPropertiesReqStructWrapper(param);

	CDynIPSProperties *pDynIpService = service->GetDynamicProperties();
	CGKInfo &refGKInfo = pDynIpService->GetGKInfo();
	refGKInfo.SetGKInfo(param);

	return STATUS_OK;
}

bool CCommGKService::FillParams(GkManagerServiceParamsIndStruct &param, CIPService *service)
{
	param.serviceId = service->GetId();
	param.service_ip_protocol_types = service->GetIpType();
	strncpy(param.serviceName, service->GetName(), sizeof(param.serviceName)-1);
	param.serviceName[sizeof(param.serviceName)-1] = '\0';

	param.bIsGkInService = service->IsContainGK();
	param.rrqPolingInterval = service->GetRRQPollingInterval();

	const CH323Alias *aliasDialIn = service->GetDialInPrefix();
	if(NULL == aliasDialIn)
	{
		PASSERTMSG(1, "No DialIn Prefix In Service");
		return false;
	}

    const char *prefixName = aliasDialIn->GetAliasName();
    if(NULL != prefixName)
    {
        strncpy(param.prefixName, prefixName, sizeof(param.prefixName) - 1);
        param.prefixName[sizeof(param.prefixName) - 1] = '\0';
    }

	param.bIsRegAsGw = service->GetIsRegAsGW();
	param.bIsAvf = (GK_MODE_PSEUDO_AVAYA_GK == service->GetGatekeeperMode()
                    ?
                    TRUE : FALSE);

	CIPSpan *span = service->GetFirstSpan();
	if(NULL == span)
	{
		PASSERTMSG(1, "No Spans In Service");
		return false;
	}

	CH323Alias *alias = span->GetFirstAlias();
	int i = 0;
	while(NULL != alias && i < MAX_ALIAS_NAMES_NUM)
	{
		const char *aliasName = alias->GetAliasName();
		if('\0' != aliasName[0])
		{
			param.aliases[i].aliasType = alias->GetAliasType();
			strcpy((char*)param.aliases[i].aliasContent, alias->GetAliasName());
			i++;
		}

		alias = span->GetNextAlias();
	}

	// ===== I. first csIp (csIp[0]) contains IPv4 (if exists)
	i=0;
	if (service->GetIpType() != eIpType_IpV6 && !span->GetIsIpV4Null() )
	{
		param.csIp[0].ipVersion = eIpVersion4;
		param.csIp[0].addr.v4.ip = span->GetIPv4Address();
		i++;
	}

	if(service->GetIpType() != eIpType_IpV4)
	{
		// ===== II. other csIps contain IPv6 (if exist)
		char curIPv6Addr[IPV6_ADDRESS_LEN];
		mcTransportAddress tempTransAdd;
		for (int j=0; j<NUM_OF_IPV6_ADDRESSES; j++)
		{
			if ( !span->GetIsIpV6Null(j) )
			{
				// ---- a. ip version
				param.csIp[i].ipVersion = eIpVersion6;

				// ---- b. ip params
				memset(curIPv6Addr,		0, IPV6_ADDRESS_LEN);
				memset(&tempTransAdd,	0, sizeof(mcTransportAddress));
				span->GetIPv6Address(j, curIPv6Addr);
				::stringToIp(&tempTransAdd, curIPv6Addr);

				// --------- b1. ip address
				memcpy(&param.csIp[i].addr.v6.ip, &tempTransAdd.addr, IPV6_ADDRESS_BYTES_LEN);

				// --------- b2. scope id
				param.csIp[i].addr.v6.scopeId = ::getScopeId(curIPv6Addr);

				i++;
			}
		}
	}

	param.gkIp.ipVersion = cmTransportTypeIP;

	CDynIPSProperties *dynService = service->GetDynamicProperties();
	CH323Info &gkInfo = dynService->GetCSGKInfo();

	// ===== 1. Primary gatekeeper
	CProxyDataContent &primaryGK = gkInfo.GetPrimaryGk();
	string sGkName = service->GetGatekeeperName().GetString();//primaryGK.GetName(); == TO BE CHANGED ===
	char*  cGkName = (char*)(sGkName.c_str());

	TRACEINTO << "\n" << __FUNCTION__ << ": cGkName = " << cGkName << endl;

	if ( false == sGkName.empty() )
	{
		if ( TRUE == isIpV6Str(cGkName) )
		{
			mcTransportAddress tmpIPv6Addr;
			memset(&tmpIPv6Addr, 0, sizeof(mcTransportAddress));
			::stringToIpV6( &tmpIPv6Addr, cGkName );

			param.gkIp.ipVersion = eIpVersion6;
			memcpy( &(param.gkIp.addr.v6.ip), &(tmpIPv6Addr.addr.v6.ip), IPV6_ADDRESS_BYTES_LEN );
			param.gkIp.addr.v6.scopeId = (APIU32)(getScopeId(cGkName));
		}
		else if ( TRUE == isIpV4Str(cGkName) )
		{
			param.gkIp.ipVersion = eIpVersion4;
			param.gkIp.addr.v4.ip = SystemIpStringToDWORD(cGkName);
		}
		else
		{
		  strncpy(param.gkName, cGkName, sizeof(param.gkName)-1);
		  param.gkName[sizeof(param.gkName)-1] = '\0';
		}
	}

	// ===== 2. Alternate gatekeeper
	CProxyDataContent &altGk = gkInfo.GetAltGk();
	string sAltGkName = altGk.GetName();
	char*  cAlGkName  = (char*)(sAltGkName.c_str());
	if ( false == sAltGkName.empty() )
	{
		if ( TRUE == isIpV6Str(cAlGkName) )
		{
			mcTransportAddress tmpIPv6Addr;
			memset(&tmpIPv6Addr, 0, sizeof(mcTransportAddress));
			::stringToIpV6( &tmpIPv6Addr, cAlGkName );

			param.alternateGkIp.ipVersion = eIpVersion6;
			memcpy( &(param.alternateGkIp.addr.v6.ip), &(tmpIPv6Addr.addr.v6.ip), IPV6_ADDRESS_BYTES_LEN );
			param.alternateGkIp.addr.v6.scopeId = (APIU32)(getScopeId(cGkName));
		}
		else if ( TRUE == isIpV4Str(cAlGkName) )
		{
			param.alternateGkIp.ipVersion = eIpVersion4;
			param.alternateGkIp.addr.v4.ip = SystemIpStringToDWORD(cAlGkName);
		}
		else if(0 != altGk.GetIPv4Address())
		{
			param.alternateGkIp.ipVersion = eIpVersion4;
			param.alternateGkIp.addr.v4.ip = altGk.GetIPv4Address();
		}
		else
		{
			strncpy(param.altGkName, cAlGkName, H243_NAME_LEN-1); //B.S.: KW 391
			param.altGkName[H243_NAME_LEN-1] = '\0';
		}
	}

	//fill authentication params
	param.authenticationParams.authenticationProtocol  =  service->GetH323AuthenticationProtocol();
	param.authenticationParams.isAuthenticationEnabled = service->GetH323AuthenticationEnable();

	const char * pUserName = service->GetH323AuthenticationUserName();
	strncpy(param.authenticationParams.user_name, pUserName, USER_NAME_LEN-1);//B.S.: KW 391
	param.authenticationParams.user_name[USER_NAME_LEN-1]='\0';

	const char * pPassword = service->GetH323AuthenticationPassword();
	strncpy(param.authenticationParams.password, pPassword, PASSWORD_LEN-1);//B.S.: KW 391
	param.authenticationParams.password[PASSWORD_LEN-1]='\0';

	return true;

/*
	CProxyDataContent &primaryGK = gkInfo.GetPrimaryGk();
	DWORD ipAddressGK = primaryGK.GetIpAddress();
	if(0 != ipAddressGK)
	{
		param.gkIp.addr.v4.ip = ipAddressGK;
	}
	else if(false == primaryGK.GetName().empty())
	{
		strncpy(param.gkName, primaryGK.GetName().c_str(), H243_NAME_LEN);
        param.gkName[H243_NAME_LEN - 1] = '\0';
	}

	DWORD ipAddressAlt = altGk.GetIpAddress();
	if(0 != ipAddressAlt)
	{
		param.alternateGkIp.addr.v4.ip = ipAddressAlt;
	}
	else if(false == altGk.GetName().empty())
	{
		strncpy(param.altGkName, altGk.GetName().c_str(), H243_NAME_LEN);
        param.altGkName[H243_NAME_LEN - 1] = '\0';
	}

	return true;
*/
/*
	if(0 != gkIp)
	{
		param.gkIp.addr.v4.ip 			= gkIp;
	}
	else
	{
		CProxyDataContent &primaryGk = gkInfo.GetPrimaryGk();
		DWORD ipAddress = primaryGk.GetIpAddress();
		if(0 != ipAddress)
		{
			param.gkIp.addr.v4.ip = ipAddress;
		}
		else
		{
			string &gkName = primaryGk.GetName();
			if(false == gkName.empty())
			{
				strcpy(param.gkName, gkName.c_str());
			}
			else
			{
				PASSERTMSG(1, "No Ip or Name for GK");
				return false;
			}
		}

		CProxyDataContent &altGk = gkInfo.GetAltGk();
		DWORD ipAddressAlt = altGk.GetIpAddress();
		if(0 != ipAddressAlt)
		{
			param.alternateGkIp.addr.v4.ip = ipAddressAlt;
		}
		else
		{
			string &altGkName = altGk.GetName();
			if(false == altGkName.empty())
			{
				strcpy(param.altGkName, altGkName.c_str());
			}
		}
	}

	return true;
*/
}

bool CCommGKService::IsServiceReady(CIPService *service)
{
	CDynIPSProperties *dynService = service->GetDynamicProperties();
	CH323Info &gkInfo = dynService->GetCSGKInfo();

	return gkInfo.GetIsUpdatedFromCsModule();
}


