// CommConfService.cpp: implementation of the CCommConfService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Communication with Conf-Party
//========   ==============   =====================================================================


#include "CommConfService.h"
#include "TaskApi.h"
#include "CSMngrProcess.h"
#include "IpService.h"
#include "Segment.h"
#include "StatusesGeneral.h"
#include "WrappersConfParty.h"
#include "TraceStream.h"
#include "OpcodesMcmsInternal.h"
#include "RvCommonDefs.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCommConfService::CCommConfService()
{

}

//////////////////////////////////////////////////////////////////////
CCommConfService::~CCommConfService()
{

}

////////////////////////////////////////////
STATUS CCommConfService::SendServiceCfgList(CSegment *pSeg)
{
	STATUS res = SendToMcmsProcess(eProcessConfParty, CS_SERVICE_CFG_UPDATE_IND, pSeg);
	return res;

}

//////////////////////////////////////////////////////////////////////
STATUS CCommConfService::SendIpServiceParamInd(CIPService *service)
{
	CONF_IP_PARAMS_S param;
	memset(&param, 0, sizeof(CONF_IP_PARAMS_S));
	FillParam(param, service);
	
	TRACEINTO << CConfIpParamWrapper(param);

	CSegment *pSeg = new CSegment;
	pSeg->Put((BYTE*)&param, sizeof(CONF_IP_PARAMS_S));

	STATUS res = SendToMcmsProcess(eProcessConfParty, CS_CONF_IP_SERVICE_PARAM_IND, pSeg);

	return res;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommConfService::SendIpServiceParamEndInd()
{
	STATUS res = SendToMcmsProcess(eProcessConfParty, CS_CONF_IP_SERVICE_PARAM_END_IND, NULL);

	return res;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommConfService::SendDelIpService(CIPService *service)
{
	STATUS res = SendDelIpServiceToMcmsProcess(eProcessConfParty, CS_CONF_DELETE_IP_SERVICE_IND, service);

	return res;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommConfService::SendDefaultService()
{
	CIPServiceList *list = m_pCSProcess->GetIpServiceListStatic();

	DEFAULT_IP_SERVICE_S param;

	strncpy(param.defaultSIPServiceName, list->GetSIPDefaultName(), sizeof(param.defaultSIPServiceName)-1);
	param.defaultSIPServiceName[sizeof(param.defaultSIPServiceName)-1] = '\0';

  strncpy(param.defaultH323ServiceName, list->GetH323DefaultName(), sizeof(param.defaultH323ServiceName)-1);
  param.defaultH323ServiceName[sizeof(param.defaultH323ServiceName)-1] = '\0';

	CSegment *pSeg = new CSegment;
	pSeg->Put((BYTE*)&param, sizeof(DEFAULT_IP_SERVICE_S));

	STATUS res = SendToMcmsProcess(eProcessConfParty, CS_CONF_DEFAULT_SERVICE_IND, pSeg);

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void CCommConfService::SendReleaseMsgReq() const
{
	CSegment *pSeg = new CSegment;
	*pSeg << (BYTE)FALSE;
	*pSeg << (BYTE)eConfBlockReason_CSMngr_Position_1;
	CTaskApi api(eProcessConfParty, eManager);
	api.SendMsg(pSeg, CONF_BLOCK_IND); 
}

//////////////////////////////////////////////////////////////////////
void CCommConfService::FillParam(CONF_IP_PARAMS_S &param, CIPService *service)const 
{	
	SetGeneralParams(param, service);
	
	CQualityOfService *pQos = service->GetpQualityOfService();
	if(NULL != pQos)
		SetQos(param.qos, pQos);

	CIPSpan *span = service->GetFirstSpan();
	if(NULL != span)
		SetSpan( param, span, service->GetIpType(),service->GetIpV6ConfigurationType() );

	CSip *sip = service->GetpSip();
	if(NULL != sip)
		SetSip(param.sip, sip, service->GetpSipAdvanced());
	
	CIpDns *pDns = service->GetpDns();
	if(NULL != pDns)
		SetDns(param, pDns);
	
	const  CH323Alias *pDialIn = service->GetDialInPrefix();
	if(NULL != pDialIn)
		SetAlias(param.dialIn, pDialIn);
}

//////////////////////////////////////////////////////////////////////
void CCommConfService::SetGeneralParams(CONF_IP_PARAMS_S &param, CIPService *service)const
{	
	param.service_id 		= service->GetId();
	param.service_protocol_type 	= service->GetIPProtocolType();
  strncpy(param.service_name, service->GetName(), sizeof(param.service_name)-1);
  param.service_name[sizeof(param.service_name)-1] = '\0';

	CIPServiceList *list = m_pCSProcess->GetIpServiceListStatic();
  strncpy(param.default_service_name, list->GetH323DefaultName(), sizeof(param.default_service_name)-1);
  param.default_service_name[sizeof(param.default_service_name)-1] = '\0';

    // Primary gatekeeper
    param.gk_ip.ipVersion = cmTransportTypeIP;

    CDynIPSProperties *dynService = service->GetDynamicProperties();

	CH323Info &gkInfo = dynService->GetCSGKInfo();

	string sGkName = "";
	if (!(service->GetGatekeeperName().IsEmpty()))
		sGkName = service->GetGatekeeperName().GetString();//primaryGK.GetName(); == TO BE CHANGED ===

	char*  cGkName = (char*)(sGkName.c_str());

	if ( false == sGkName.empty() )
	{
        param.is_gk_external = service->IsContainGK();
		if ( TRUE == isIpV6Str(cGkName) )
		{
			mcTransportAddress tmpIPv6Addr;
			memset(&tmpIPv6Addr, 0, sizeof(mcTransportAddress));
			::stringToIpV6( &tmpIPv6Addr, cGkName );

			param.gk_ip.ipVersion = eIpVersion6;
			memcpy( &(param.gk_ip.addr.v6.ip), &(tmpIPv6Addr.addr.v6.ip), IPV6_ADDRESS_BYTES_LEN );
			param.gk_ip.addr.v6.scopeId = (APIU32)(getScopeId(cGkName));
		}
		else if ( TRUE == isIpV4Str(cGkName) )
		{
			param.gk_ip.ipVersion = eIpVersion4;
			param.gk_ip.addr.v4.ip = SystemIpStringToDWORD(cGkName);
		}

		else
		{
		  strncpy(param.gkName, cGkName, sizeof(param.gkName)-1);
		  param.gkName[sizeof(param.gkName)-1] = '\0';
		}
	}

    param.isAvfOn = (GK_MODE_PSEUDO_AVAYA_GK == service->GetGatekeeperMode()
                    ?
                    TRUE : FALSE);
    
    param.service_default_type = service->GetServiceDefaultType(list->GetH323DefaultName(), list->GetSIPDefaultName());
}

//////////////////////////////////////////////////////////////////////
void CCommConfService::SetQos(QOS_S &qos, const CQualityOfService *pQos)const 
{
	qos.m_IsDefault = FALSE;

	qos.m_eIpStatus 			= pQos->GetIpStatus();
	qos.m_bIpIsDiffServ 		= pQos->GetIpDiffServ();
	qos.m_bIpValueTOS 			= pQos->GetIpTos();
	qos.m_bIpPrecedenceAudio 	= pQos->GetIpAudPrec();
	qos.m_bIpPrecedenceVideo 	= pQos->GetIpVidPrec();
	qos.m_eAtmStatus 			= pQos->GetAtmStatus();
	qos.m_bAtmPrecedenceAudio 	= pQos->GetAtmAudPrec();
	qos.m_bAtmPrecedenceVideo 	= pQos->GetAtmVidPrec();
}
//////////////////////////////////////////////////////////////////////
void CCommConfService::SetSpan(CONF_IP_PARAMS_S &param, CIPSpan *span, eIpType theType,eV6ConfigurationType v6configType)const 
{
	// ===== 1. IPv4
	param.service_ip_protocol_types = theType;
	param.cs_ipV4.v4.ip = span->GetIPv4Address();
/*	mcTransportAddress ttt;
	memset(&ttt,0,sizeof(mcTransportAddress));
	ttt.addr.v4.ip = param.cs_ipV4.v4.ip;
	char aa[64];
	memset (aa,'\0',64);
	ipToString(ttt,aa,1);
	TRACEINTO << "CCommConfService::SetSpan " <<  param.cs_ipV4.v4.ip << " Stringed " << aa << "\n";
	ALLOCBUFFER(s2, 200);
	sprintf(s2,"Address is 0x %x",param.cs_ipV4.v4.ip);
	PTRACE2(eLevelInfoNormal,"CCommConfService::SetSpan: ",s2);
	DEALLOCBUFFER(s2);*/
	BOOL ipv6Empty = FALSE;
	if(eIpType_Both == theType)
	{
		if(0==span->GetIPv4Address())
		{
			ipv6Empty = TRUE;
		}
	}
	// ===== 2. IPv6
	char curIPv6Addr[IPV6_ADDRESS_LEN];
	mcTransportAddress tempTransAdd;
	for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		memset(curIPv6Addr,		0, IPV6_ADDRESS_LEN);
		memset(&tempTransAdd,	0, sizeof(mcTransportAddress));
		if(ipv6Empty)
		{
			strncpy(curIPv6Addr ,"::",IPV6_ADDRESS_LEN );
										 										
		}
		else
		{
			span->GetIPv6Address(i, curIPv6Addr);
		}
		::stringToIp(&tempTransAdd, curIPv6Addr);
		memcpy(&param.cs_ipV6Array[i].v6.ip, &tempTransAdd.addr, IPV6_ADDRESS_BYTES_LEN);
	
		param.cs_ipV6Array[i].v6.scopeId = ::getScopeId(curIPv6Addr);

}


	// ===== 2. Other params
	int i = 0;
	const  CH323Alias *pAlias = span->GetFirstAlias();
	while(NULL != pAlias)
	{
		SetAlias(param.aliases[i], pAlias);

		i++;
		pAlias = span->GetNextAlias();
	}		
}

//////////////////////////////////////////////////////////////////////
void CCommConfService::SetSip(SIP_S &param, const CSip *sip, const CSipAdvanced* pSipAdvanced)const
{
	const CBaseSipServer *proxy = sip->GetpProxy();
	if(NULL != proxy)
	{
		SetBaseSipServer(param.proxy, proxy);
	}

	const CBaseSipServer *altProxy = sip->GetpAltProxy();
	if(NULL != altProxy)
	{
		SetBaseSipServer(param.altProxy, altProxy);
	}

	const CSipServer *registrar = sip->GetpRegistrar();
	if(NULL != registrar)
	{
		SetSipServer(param.registrar, registrar);
	}
	
	const CSipServer *altRegistrar = sip->GetpAltRegistrar();
	if(NULL != altRegistrar)
	{
		SetSipServer(param.altRegistrar, altRegistrar);
	}
	
	// all general SIP parameters
	param.transportType					= sip->GetTransportType();
	param.configureSIPServersMode		= sip->GetConfigurationOfSIPServers();
	param.IceType						= pSipAdvanced->GetIceEnvironment();
	param.SipServerType					= sip->GetSipServerType();

}

//////////////////////////////////////////////////////////////////////
void CCommConfService::SetBaseSipServer(BASE_SIP_SERVER_S &param, const CBaseSipServer *baseSipServer)const 
{
	param.status = baseSipServer->GetStatus();
	param.port = baseSipServer->GetPort();
	memcpy(param.hostName, baseSipServer->GetName().GetString(), ONE_LINE_BUFFER_LEN);
}

//////////////////////////////////////////////////////////////////////
void CCommConfService::SetSipServer(SIP_SERVER_S &param, const CSipServer *sipServer)const 
{
	SetBaseSipServer(param.baseSipServer, sipServer);
	
	memcpy(
		param.domainName, 
		sipServer->GetDomainName().GetString(), 
		ONE_LINE_BUFFER_LEN
		);				
}

//////////////////////////////////////////////////////////////////////
void CCommConfService::SetDns(CONF_IP_PARAMS_S &param, const CIpDns *pDns)const 
{
	strncpy(param.domain_name, pDns->GetDomainName().GetString(), sizeof(param.domain_name) - 1);
	param.domain_name[sizeof(param.domain_name) - 1 ] = '\0';
}

//////////////////////////////////////////////////////////////////////
void CCommConfService::SetAlias(ALIAS_S &param, const CH323Alias *alias)const
{
	strcpy((char *)param.aliasContent, alias->GetAliasName());
	param.aliasType = alias->GetAliasType();
}








