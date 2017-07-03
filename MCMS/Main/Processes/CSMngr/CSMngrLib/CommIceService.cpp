// CommIceService.cpp: implementation of the CCommIceService class.
//
//
//Date         Updated By         Description
//
//24/04/14	  Boris Sirotin		Communication with Ice
//=====================================================================

#include "CommIceService.h"
#include "Segment.h"
#include "IpService.h"
#include "CsCommonStructs.h"
#include "StatusesGeneral.h"
#include "WrappersSipProxy.h"
#include "TraceStream.h"
#include "DefinesGeneral.h"
#include "CSMngrStatuses.h"
#include "SIPProxyStructs.h"
#include "OpcodesMcmsInternal.h"
#include "WrappersSipProxy.h"
#include "IceTaskApi.h"

////////////////////////////////////////////////////////////////////////////
//                        CTokenUser
////////////////////////////////////////////////////////////////////////////
CCommIceService::CCommIceService()
{
}

////////////////////////////////////////////////////////////////////////////
CCommIceService::~CCommIceService()
{
}

////////////////////////////////////////////////////////////////////////////
STATUS CCommIceService::SendServiceCfgList(CSegment *pSeg)
{
  return SendToMcmsProcess(eProcessIce, CS_SERVICE_CFG_UPDATE_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
STATUS CCommIceService::SendIpServiceParamInd(CIPService *service)
{
	STATUS status = STATUS_OK;

	bool isApplic = CheckSipProxyApplicable(service);
	bool isWebRtcIce = CheckWebRtcIceApplicable(service);
	if(false == isApplic)
	{
		return STATUS_NO_SIP_IN_SERVICE;
	}
	if (false == isWebRtcIce)
	{
		return STATUS_NO_WEB_RTC_ICE_IN_SERVICE;
	}

	SIP_PROXY_IP_PARAMS_S param;
	memset(&param, 0, sizeof(SIP_PROXY_IP_PARAMS_S));
	bool res = FillParams(param, service);
	if(!res)
	{
		return STATUS_NO_SIP_IN_SERVICE;
	}

	CSegment *pSeg = new CSegment;
	pSeg->Put((BYTE*)&param, sizeof(SIP_PROXY_IP_PARAMS_S));

	return SendToMcmsProcess(eProcessIce, CS_PROXY_IP_SERVICE_PARAM_IND, pSeg);


}

////////////////////////////////////////////////////////////////////////////
STATUS CCommIceService::SendIpServiceParamEndInd()
{
	return SendToMcmsProcess(eProcessIce, CS_PROXY_IP_SERVICE_PARAM_END_IND, NULL);
}

////////////////////////////////////////////////////////////////////////////
STATUS CCommIceService::SendDelIpService(CIPService *service)
{
	STATUS res = SendDelIpServiceToMcmsProcess(eProcessIce, CS_PROXY_DELETE_IP_SERVICE_IND, service);

	return res;
}


////////////////////////////////////////////////////////////////////////////
bool CCommIceService::FillParams(SIP_PROXY_IP_PARAMS_S &param, CIPService *service)const
{
	param.ServiceId = service->GetId();
	strncpy(param.ServiceName, service->GetName(), sizeof(param.ServiceName)-1);
	param.ServiceName[sizeof(param.ServiceName)-1] = '\0';
	param.IpType = service->GetIpType();

	const CSip *pSip = service->GetpSip();
	if(NULL == pSip)
	{
		return false;
	}

	const CSipServer *altRegistrar = pSip->GetpAltRegistrar();
	eServerStatus status = eServerStatusOff;
	if(NULL != altRegistrar)
	{
		status = altRegistrar->GetStatus();
	}

	param.Dhcp 		= service->GetDHCPServer();

	CIPSpan *pCsSpan = service->GetFirstSpan();
	if(NULL != pCsSpan)
	{
		int i=0, j=0;
		for(i=0; i<NUM_OF_IPV4_ADDRESSES ; i++)
		{
			param.pAddrList[i].addr.v4.ip = pCsSpan->GetIPv4Address();
			param.pAddrList[i].ipVersion = eIpVersion4;
		}

		for(j=0; j<NUM_OF_IPV6_ADDRESSES ; j++)
		{
			memcpy( &param.pAddrList[j+i/*following v4 addresses*/].addr.v6,
					&pCsSpan->m_IPv6AaddressArray[j],
					sizeof(ipAddressV6If) );

			param.pAddrList[j+i/*following v4 addresses*/].ipVersion = eIpVersion6;
		}
	}
	else
	{
		PASSERTMSG(1, "No Spans In IpService, At least one should be");
		return false;
	}

	CIpDns *dns = service->GetpDns();
	if(NULL != dns)
	{
		param.DNSStatus = dns->GetStatus();
	}

	const CBaseSipServer *proxy = pSip->GetpProxy();
	if(NULL != proxy)
	{
		status = proxy->GetStatus();
		if(eServerStatusOff != status)
			SetProxy(param, proxy);
	}

	const CSipServer *registrar = pSip->GetpRegistrar();
	if(NULL != registrar)
	{
		status = registrar->GetStatus();
		if(eServerStatusOff != status)
		{
        SetRegistrar(param, registrar);
        if( isApiTaNull(&param.OutboundProxyAddress) && (eServerStatusOff == param.DNSStatus) )
		      SetProxy(param, registrar);
		}
	}

	SetSipFields(param, pSip);
	
	CSipAdvanced* pSipAdvanced = service->GetpSipAdvanced();
	if (pSipAdvanced)
		SetSipAdvancedFields(param, pSipAdvanced);

	return true;
}

////////////////////////////////////////////////////////////////////////////
void CCommIceService::SetSipAdvancedFields(SIP_PROXY_IP_PARAMS_S &param, const CSipAdvanced* pSipAdvanced)const
{
  strncpy(param.userName, pSipAdvanced->GetSipAdvancedUserName(), sizeof(param.userName)-1);
  param.userName[sizeof(param.userName)-1] = '\0';

	param.IceType = pSipAdvanced->GetIceEnvironment();
	
	CIceStandardParams* pIceStandardParams = pSipAdvanced->GetpIceStandardParams();
	
	if (pIceStandardParams)
	{
		if(pIceStandardParams->GetIsServerPassword())
		{

			param.stunPassServerAddress.port = pIceStandardParams->GetPasswordServerPort();
			strncpy(param.stunPassServerHostName, pIceStandardParams->GetPasswordServerHostName(), H243_NAME_LEN-1); //kw added -1
			param.stunPassServerHostName[H243_NAME_LEN-1] = '\0';
			param.stunPassServerAddress.ipVersion = eIpVersion4;
			param.stunPassServerAddress.addr.v4.ip = pIceStandardParams->GetPasswordServerIp();

			strncpy(param.stunPassServerUserName, pIceStandardParams->GetPasswordServerUserName(), sizeof(param.stunPassServerUserName)-1);
			param.stunPassServerUserName[sizeof(param.stunPassServerUserName)-1] = '\0';

			strncpy(param.stunPassServerPassword, pIceStandardParams->GetPasswordServerPassword(), sizeof(param.stunPassServerPassword)-1);
			param.stunPassServerPassword[sizeof(param.stunPassServerPassword)-1] = '\0';
		}
		
		param.stunServerUDPAddress.port = pIceStandardParams->GetSTUNServerPort();
		strncpy(param.stunServerHostName, pIceStandardParams->GetSTUNServerHostName(), H243_NAME_LEN - 1);
		param.stunServerHostName[H243_NAME_LEN - 1] = '\0';
		param.stunServerUDPAddress.ipVersion = eIpVersion4;
		param.stunServerUDPAddress.addr.v4.ip	= pIceStandardParams->GetSTUNServerIp();

		PTRACE2INT(eLevelInfoHigh, "CCommIceService::SetSipAdvancedFields- pIceStandardParams->GetSTUNServerPort()",pIceStandardParams->GetSTUNServerPort());
		PTRACE2INT(eLevelInfoHigh, "CCommIceService::SetSipAdvancedFields- param.stunServerUDPAddress.port",param.stunServerUDPAddress.port);
		PTRACE2(eLevelInfoHigh, "CCommIceService::SetSipAdvancedFields- pIceStandardParams->GetSTUNServerHostName()",pIceStandardParams->GetSTUNServerHostName());
		PTRACE2(eLevelInfoHigh, "CCommIceService::SetSipAdvancedFields- param.stunServerHostName",param.stunServerHostName);
		PTRACE2INT(eLevelInfoHigh, "CCommIceService::SetSipAdvancedFields- pIceStandardParams->GetSTUNServerIp()",pIceStandardParams->GetSTUNServerIp());

		param.stunServerTCPAddress.port = pIceStandardParams->GetSTUNServerPort();
		param.stunServerTCPAddress.ipVersion = eIpVersion4;
		param.stunServerTCPAddress.addr.v4.ip	= pIceStandardParams->GetSTUNServerIp();

		// TURN = RELAY
		param.RelayServerUDPAddress.port = pIceStandardParams->GetTURNServerPort();
		strncpy(param.RelayServerHostName, pIceStandardParams->GetTURNServerHostName(), H243_NAME_LEN - 1);
		param.RelayServerHostName[H243_NAME_LEN - 1] = '\0';
		param.RelayServerUDPAddress.ipVersion = eIpVersion4;
		param.RelayServerUDPAddress.addr.v4.ip	= pIceStandardParams->GetTURNServerIp();

		param.RelayServerTCPAddress.port = pIceStandardParams->GetTURNServerPort();
		param.RelayServerTCPAddress.ipVersion = eIpVersion4;
		param.RelayServerTCPAddress.addr.v4.ip	= pIceStandardParams->GetTURNServerIp();

		
	}
}

////////////////////////////////////////////////////////////////////////////
void CCommIceService::SetIpV6(mcTransportAddress & outTransAddress, char *name)const
{
	mcTransportAddress tmpIPv6Addr;
	memset(&tmpIPv6Addr, 0, sizeof(mcTransportAddress));
	::stringToIpV6(&tmpIPv6Addr, name);

	outTransAddress.ipVersion = eIpVersion6;
	memcpy( &(outTransAddress.addr.v6.ip), &(tmpIPv6Addr.addr.v6.ip), IPV6_ADDRESS_BYTES_LEN);
	outTransAddress.addr.v6.scopeId = (APIU32)::getScopeId(name);
}

////////////////////////////////////////////////////////////////////////////
void CCommIceService::SetProxy(SIP_PROXY_IP_PARAMS_S &param, const CBaseSipServer *proxy)const
{
	param.OutboundProxyAddress.port			= (APIU32)proxy->GetPort();

	char *proxName = (char*)proxy->GetName().GetString();

	if(::isIpV6Str(proxName))
	{
		SetIpV6(param.OutboundProxyAddress, proxName);
	}
	else
	{
		param.OutboundProxyAddress.ipVersion = eIpVersion4;
    strncpy(param.pOutboundProxyName, proxName, sizeof(param.pOutboundProxyName)-1);
    param.pOutboundProxyName[sizeof(param.pOutboundProxyName)-1] = '\0';
		param.OutboundProxyAddress.addr.v4.ip = proxy->GetIpAddress();
	}

	char strIp[128];
	::ipV6ToString(param.OutboundProxyAddress.addr.v6.ip, strIp, TRUE);
	TRACEINTO << "CCommIceService::SetProxy - strIp:" <<  strIp << ", port:" << param.OutboundProxyAddress.port;
}

////////////////////////////////////////////////////////////////////////////
void CCommIceService::SetRegistrar(SIP_PROXY_IP_PARAMS_S &param, const CSipServer *registrar)const
{
	param.ProxyAddress.port			= (APIU32)registrar->GetPort();

	char* hostName = (char*)registrar->GetDomainName().GetString();
	if(::isIpV6Str(hostName))
	{
		mcTransportAddress TempHostName;
		memset(&TempHostName, 0, sizeof(TempHostName));
		char arr[H243_NAME_LEN];
		memset(arr, '\0', sizeof(arr));
		stringToIpV6(&TempHostName, hostName);
		ipV6ToString(TempHostName.addr.v6.ip, arr, 1);

    strncpy(param.pProxyHostName, arr, sizeof(param.pProxyHostName)-1);
    param.pProxyHostName[sizeof(param.pProxyHostName)-1] = '\0';
		TRACEINTO << "CCommIceService::SetRegistrar - ProxyHostName:" << param.pProxyHostName;
	}
	else
	{
    strncpy(param.pProxyHostName, hostName, sizeof(param.pProxyHostName)-1);
    param.pProxyHostName[sizeof(param.pProxyHostName)-1] = '\0';
	}

	char *regisName = (char*)registrar->GetName().GetString();

	if(::isIpV6Str(regisName))
	{
		SetIpV6(param.ProxyAddress, regisName);
	}
	else
	{
		param.ProxyAddress.ipVersion = eIpVersion4;
		param.ProxyAddress.addr.v4.ip = registrar->GetIpAddress();
    strncpy(param.pProxyName, regisName, sizeof(param.pProxyName)-1);
    param.pProxyName[sizeof(param.pProxyName)-1] = '\0';
	}
}

////////////////////////////////////////////////////////////////////////////
void CCommIceService::SetSipFields(SIP_PROXY_IP_PARAMS_S &param, const CSip *pSip)const
{
	param.refreshTout 	= pSip->GetRefreshRegistrationTout();
	param.serversConfig = pSip->GetConfigurationOfSIPServers();
	param.transportType = pSip->GetTransportType();

	param.RegistrationFlags = 0x00;
	param.SipServerType		= pSip->GetSipServerType();
}

////////////////////////////////////////////////////////////////////////////
bool CCommIceService::CheckSipProxyApplicable(CIPService* service) const
{
	// get protocolType - H323, Sip or both:
	eIPProtocolType protocolType = service->GetIPProtocolType();

	// only if it's Sip (or both), we send IpService
	// Note that we don't check the sip server mode (on/off). SipProxyManager is responsible for checking it.
	// It that way, we can support on->off and off->on changing without reset.
	if ((eIPProtocolType_SIP_H323 == protocolType) || (eIPProtocolType_SIP == protocolType))
	{
		return true;
	}
	// otherwise don't send IpService (only 'End' ind)
	return false;
}

////////////////////////////////////////////////////////////////////////////
bool CCommIceService::CheckWebRtcIceApplicable(CIPService* service) const
{
	// get Sip advanced
	CSipAdvanced *pSipAdvanced = service->GetpSipAdvanced() ;

	// only if it's WebRTC ICE, we send IpService
	if(NULL != pSipAdvanced)
	{
		eIceEnvironmentType  iceEnvironmentType = pSipAdvanced->GetIceEnvironment();
		if(eIceEnvironment_WebRtc == iceEnvironmentType)
		{
			return true;
		}
		return false;
	}

	return false;
}
