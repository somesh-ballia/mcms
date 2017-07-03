// CommServiceService.cpp: implementation of the CCommServiceService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Communication with CS Module.
//								Service type(it sends/receives buffer of chars)
//========   ==============   =====================================================================


#include "CommServiceService.h"
#include "IpCsOpcodes.h"
#include "Segment.h"
#include "CSMngrProcess.h"
#include "IpService.h"
#include "CSKeys.h"
#include "DynIPSProperties.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "DefinesGeneral.h"
#include "DefinesIpService.h"
#include "CSMngrStatuses.h"
#include "TraceStream.h"
#include "WrappersCS.h"
#include "ConfigManagerApi.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "MultipleServicesFunctions.h"
//-S--PLCM_DNS-------------------------//
#include "OpcodesMcmsInternal.h"
//-E--PLCM_DNS-------------------------//

#define COMMON_REQ_BUF_SIZE 512

extern char* IpV6ConfigurationTypeToString(APIU32 v6Type, bool caps = false);
extern char* IpTypeToString(APIU32 ipType, bool caps = false);
extern bool  IsJitcAndNetSeparation();

static CCSMngrProcess *pCSMngrProcess = NULL;



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCommServiceService::CCommServiceService()
{
	pCSMngrProcess = (CCSMngrProcess*)(CCSMngrProcess::GetProcess());
	m_LastInfoType = eInfoTypeInvalid;
	m_IsErrorSectionReceived = false;
	m_csId = 0;
	m_type = eSectionBegin;

	m_SectionMethodArray[eSERVICE_NAME			] = &CCommServiceService::SendServiceName;
	m_SectionMethodArray[ePORT_RANGE			] = &CCommServiceService::SendPortRange;
	m_SectionMethodArray[eIP_INTERFACE			] = &CCommServiceService::SendIpInterface;
	m_SectionMethodArray[eSUPPORTED_PROTOCOLS	] = &CCommServiceService::SendSupportedProtocols;
	m_SectionMethodArray[eDNS_CFG				] = &CCommServiceService::SendDNSCfg;
	m_SectionMethodArray[eDNS_CALLS				] = &CCommServiceService::SendDNSCalls;
	m_SectionMethodArray[eDHCP					] = &CCommServiceService::SendDHCP;
	m_SectionMethodArray[eDNS_NAME				] = &CCommServiceService::SendDNSName;
	m_SectionMethodArray[eAUTH_ENTRY			] = &CCommServiceService::SendAuthenticationEntry;
	m_SectionMethodArray[eSIP_SPAN_TYPE			] = &CCommServiceService::SendSIPSpanType;
	m_SectionMethodArray[eSIP_TRANSPORT_TYPE	] = &CCommServiceService::SendSIPTransportType;

	m_InfoMethodMap["SERVICE_INFO"	] = &CCommServiceService::UpdatedServiceInfo;
	m_InfoMethodMap["IP_INFO"		] = &CCommServiceService::UpdatedIpInfo;
	m_InfoMethodMap["SIP_INFO"		] = &CCommServiceService::UpdatedSipInfo;
	m_InfoMethodMap["H323_INFO"		] = &CCommServiceService::UpdatedH323Info;

	m_ReceiveMethodMap[CS_NEW_SERVICE_INIT_IND	] = &CCommServiceService::ReceiveNewServiceInitInd;
	m_ReceiveMethodMap[CS_COMMON_PARAM_IND		] = &CCommServiceService::ReceiveCommonParamInd;
	m_ReceiveMethodMap[CS_END_SERVICE_INIT_IND	] = &CCommServiceService::ReceiveEndServiceInitInd;
	m_ReceiveMethodMap[CS_DEL_SERVICE_REQ		] = &CCommServiceService::ReceiveDeleteServiceInd;
}

//////////////////////////////////////////////////////////////////////
CCommServiceService::~CCommServiceService()
{

}

//////////////////////////////////////////////////////////////////////
eInfoType CCommServiceService::GetLastInfoType()
{
	return m_LastInfoType;
}

//////////////////////////////////////////////////////////////////////
WORD CCommServiceService::GetLastUpdateServiceId()
{
	return m_LastInfoServiceId;
}

//////////////////////////////////////////////////////////////////////
const char * CCommServiceService::GetServiceName()
{
	return m_IpService->GetName();
}
STATUS CCommServiceService::StartNewServiceConfiguration()
{
	CIPServiceList *list = m_pCSProcess->GetIpServiceListDynamic();
	
	CIPService *service = list->GetService(m_csId);

	if(NULL == service)
	{
        return STATUS_NO_SERVICE;
	}

	SetIpService(service);

	STATUS status = SendNewServiceInitReq();
	
	char str_csId[2];
	snprintf(str_csId, sizeof(str_csId), "%d",m_csId);

	std::string action = "to send New Service Init Request to CS Module. csId = ";
	action += str_csId; 
	CCSMngrProcess::TraceToLogger("CCommServiceService::StartNewServiceConfiguration", action.c_str(), status);

	return status;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SendNewServiceInitReq()
{
	const char * serviceName;
	DWORD		 serviceId;

	serviceName = m_IpService->GetName();
	serviceId	= m_IpService->GetId();

	STATUS res = SupportedNewService(serviceName, serviceId);

	return res;
}


//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SendCommonParam(WORD isFirst)
{
	STATUS status = STATUS_FAILED_TO_SEND_COMMON_PARAMS;
	while(STATUS_OK != status &&
          STATUS_END_COMMON != status &&
          STATUS_END_CONFIG != status)
	{
		status = SendCommonParamReq(isFirst);
		isFirst = FALSE;
		if(STATUS_END_COMMON != status)
		{
			char str_csId[2];
			snprintf(str_csId, sizeof(str_csId), "%d",m_csId);

			std::string action = "to send Common param to CS Module. csId = ";
			action += str_csId; 
			CCSMngrProcess::TraceToLogger("CCommServiceService::SendCommonParam", action.c_str(), status);
		}
	}

	return status;
}


//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SendEndServiceInitReq()
{
	std::string serviceName = m_IpService->GetName();

	STATUS res = SupportedEndServiceInitReq(serviceName.c_str());

	return res;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SendCommonParamReq(WORD isFirst)
{
//	static eCommonParamSectionType type = eSectionBegin;
	m_type = (YES == isFirst ? eSectionBegin : m_type);

	CCommService::MoveNext((int&)m_type);
	
	if(eSectionEnd == m_type)
	{
		return STATUS_END_COMMON;
	}

	if(eSectionEnd < m_type)
	{
		char str_csId[2];
		snprintf(str_csId, sizeof(str_csId), "%d",m_csId);
		std::string assert = "illegal flow, the type must be inside eCommonParamSectionType. csId = ";
		assert += str_csId;
		PASSERTMSG(1, assert.c_str());
		return STATUS_END_CONFIG;
	}

	STATUS res = SendGenericCommonParam(m_type);

	return res;
}


//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SendDelServiceReq(CIPService *service)
{
	STATUS res = SupportedDelService(service->GetName());

	return res;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::Receive(OPCODE opcode,CSegment *pSeg)
{
	STATUS res = STATUS_FAILED_TO_RECEIVE_MSG_FROM_CS;

	BYTE *data = pSeg->GetPtr();
	DWORD dataLen = *((DWORD*)data);

	static const DWORD DWORDLen = sizeof(DWORD);
	data += DWORDLen;

	m_IsErrorSectionReceived = false;

//	TRACEINTO << CCSBufferIndWrapper((char*)data, dataLen);

	ReceiveMethodType method = m_ReceiveMethodMap[opcode];
	if(NULL == method)
	{
		PASSERT(1);
	}
	else
	{
		res = (this->*method)(data);
	}

	return res;
}

/////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::ReceiveNewServiceInitInd(BYTE *commonParam)
{
	char 		rcvSectionName[STR_LEN];
	int			inBufSize		= 0;
	int         numOfKeys       = 0;
	int			inSecNameSize	= STR_LEN;

	cfgBufHndl  commonRequestHandle;
	commonRequestHandle.bInit = CfgFalse;

	int res = CfgUnpackBuf(
		&commonRequestHandle,
		(char*) commonParam,
		rcvSectionName,
		&inSecNameSize,
		&inBufSize,
		&numOfKeys);

	STATUS status = STATUS_OK;
	if(0 != res)
	{
		char str_csId[2];
		snprintf(str_csId, sizeof(str_csId), "%d",m_csId);
		
		std::string assert = "FAILED to unpack a message from CSModule. csId = ";
		assert += str_csId;
		PASSERTMSG(1, assert.c_str());
		status = STATUS_FAILED_TO_UNPACK_MSG_FROM_CS;
	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_OK_NEW_SERVICE))
    {
        SectionOKNewService(commonRequestHandle);
    }
	else if(0 == strcmp(rcvSectionName, CS_KEY_ERROR_NEW_SERVICE))
    {
		SectionError(commonRequestHandle);
    }
	else
	{
		status = STATUS_INVALID_SECTION_NAME;
	}

	return status;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::ReceiveCommonParamInd(BYTE *commonParam)
{
	char 		rcvSectionName [STR_LEN];
	int			inBufSize		= 0;
	int         numOfKeys       = 0;
	int			inSecNameSize	= STR_LEN;
	STATUS status = STATUS_OK;

	cfgBufHndl  commonRequestHandle;
	commonRequestHandle.bInit = CfgFalse;

	int res = CfgUnpackBuf(
		&commonRequestHandle,
		(char*) commonParam,
		rcvSectionName,
		&inSecNameSize,
		&inBufSize,
		&numOfKeys);

	if(0 != res)
	{
		char str_csId[2];
		snprintf(str_csId, sizeof(str_csId), "%d",m_csId);
		
		std::string assert = "FAILED to unpack a message from CSModule. csId = ";
		assert += str_csId;
		PASSERTMSG(1, assert.c_str());
		status = STATUS_FAILED_TO_UNPACK_MSG_FROM_CS;
	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_OK_SECTION))
	{
		SectionOKCommonParam(commonRequestHandle);
	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_ERROR_SECTION))
	{
		SectionError(commonRequestHandle);
	}
	else
	{
		UpdatedInfoMethodType method = m_InfoMethodMap[rcvSectionName];
		if(NULL != method)
		{
			m_LastInfoType = eInfoTypeInvalid;
			status = (this->*method)(commonRequestHandle);
		}
		else
		{
			status = STATUS_INFO_MESSAGE_FAIL;
		}
	}

	return status;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::ReceiveEndServiceInitInd(BYTE *commonParam)
{
	int			inBufSize		= 0;
	int         numOfKeys       = 0;
	int			inSecNameSize	= STR_LEN;
	char 		rcvSectionName [STR_LEN];
	STATUS 		status = STATUS_OK;

	cfgBufHndl  commonRequestHandle;
	commonRequestHandle.bInit = CfgFalse;

	int res = CfgUnpackBuf(
		&commonRequestHandle,
		(char*) commonParam,
		rcvSectionName,
		&inSecNameSize,
		&inBufSize,
		&numOfKeys);

	if(0 != res)
	{
		char str_csId[2];
		snprintf(str_csId, sizeof(str_csId), "%d",m_csId);
		
		std::string assert = "FAILED to unpack a message from CSModule. csId = ";
		assert += str_csId;
		PASSERTMSG(1, assert.c_str());
		status = STATUS_FAILED_TO_UNPACK_MSG_FROM_CS;
	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_OK_END_SERVICE))
    {
        SectionOKEndService(commonRequestHandle);
    }
	else if(0 == strcmp(rcvSectionName, CS_KEY_ERROR_END_SERVICE))
    {
        SectionError(commonRequestHandle);
    }
	else
	{
		status = STATUS_INVALID_SECTION_NAME;
	}

	return status;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::ReceiveDeleteServiceInd(BYTE *commonParam)
{
	int	inBufSize	  = 0;
	int numOfKeys     = 0;
	int inSecNameSize = 0;
	STATUS status = STATUS_OK;
	char rcvSectionName [STR_LEN];

	cfgBufHndl  commonRequestHandle;
	commonRequestHandle.bInit = CfgFalse;

	int res = CfgUnpackBuf(
		&commonRequestHandle,
		(char*) commonParam,
		rcvSectionName,
		&inSecNameSize,
		&inBufSize,
		&numOfKeys);

	if( 0 != res )
	{
		char str_csId[2];
		snprintf(str_csId, sizeof(str_csId), "%d",m_csId);
		
		std::string assert = "FAILED to unpack a message from CSModule. csId = ";
		assert += str_csId;
		
		PASSERTMSG(1, assert.c_str());
		status = STATUS_FAILED_TO_UNPACK_MSG_FROM_CS;
	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_OK_DEL_SERVICE))
    {
        status = SectionOKDelService(commonRequestHandle);
    }
	else if(0 == strcmp(rcvSectionName, CS_KEY_ERROR_DEL_SERVICE))
    {
        status = SectionError(commonRequestHandle);
    }
	else
	{
		status = STATUS_INVALID_SECTION_NAME;
	}

	return status;
}


//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SectionOKNewService(cfgBufHndl &request)
{
	const int NumOfParams	= 1;
	const int ParamLen		= 256;

	int   keyLen	[NumOfParams] = {ParamLen};
	int	  dataSize	[NumOfParams] = {ParamLen};

	ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
	ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

	STATUS status = CfgGetParamsContinue(
		&request,
		NumOfParams,
		request.pCurBuf,
		keyName[0],	&keyLen[0], dataName[0], &dataSize[0], cfString
		);

	if (STATUS_OK != status											||
		0 != strncmp(keyName[0], CS_KEY_SERVICE_NAME      , keyLen[0]))
	{
		TRACEINTO << "CCommServiceService::SectionOKNewService - bad params from csId" << m_csId;
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "CCommServiceService::SectionOKNewService - Ok. csId = "<<m_csId;
	}	

	DEALLOCBUFFERS(keyName, NumOfParams);
	DEALLOCBUFFERS(dataName, NumOfParams);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SectionOKCommonParam(cfgBufHndl &request)
{
	const int NumOfParams	= 1;
	const int ParamLen		= 256;

	int   keyLen	[NumOfParams] = {ParamLen};
	int	  dataSize	[NumOfParams] = {ParamLen};

	ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
	ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

	STATUS status = CfgGetParamsContinue(
		&request,
		NumOfParams,
		request.pCurBuf,
		keyName[0],	&keyLen[0], dataName[0], &dataSize[0], cfString
		);

	char *result = NULL;
	if (STATUS_OK != status		||
		0 != strncmp(keyName[0], CS_KEY_SECTION_NAME      , keyLen[0]))
	{
		result = " : bad params";
	}
	else
	{
		result = " : OK";

	}
	TRACEINTO << "CCommServiceService::SectionOKCommonParam for csId = "<<m_csId<<" - " << dataName[0] << result;

	DEALLOCBUFFERS(keyName, NumOfParams);
	DEALLOCBUFFERS(dataName, NumOfParams);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SectionOKEndService(cfgBufHndl &request)
{
	const int NumOfParams	= 1;
	const int ParamLen		= 256;

	int   keyLen	[NumOfParams] = {ParamLen};
	int	  dataSize	[NumOfParams] = {ParamLen};

	ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
	ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

	STATUS status = CfgGetParamsContinue(
		&request,
		NumOfParams,
		request.pCurBuf,
		keyName[0],	&keyLen[0], dataName[0], &dataSize[0], cfString
		);

	if (STATUS_OK != status											||
		0 != strncmp(keyName[0], CS_KEY_SERVICE_NAME      , keyLen[0]))
	{
		TRACEINTO << "CCommServiceService::SectionOKEndService - bad params for csId: "<<m_csId;
	}
	else
	{
		TRACEINTO << "CCommServiceService::SectionOKEndService - Ok for csId: "<<m_csId;
	}

	DEALLOCBUFFERS(keyName, NumOfParams);
	DEALLOCBUFFERS(dataName, NumOfParams);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SectionError(cfgBufHndl &request)
{
	m_IsErrorSectionReceived = true;

	const int NumOfParams	= 4;
	const int ParamLen		= 256;

	int   keyLen	[NumOfParams] = {ParamLen, ParamLen, ParamLen, ParamLen};
	int	  dataSize	[NumOfParams] = {ParamLen, ParamLen, ParamLen, ParamLen};

	ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
	ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

	STATUS status = CfgGetParamsContinue(
		&request,
		NumOfParams,
		request.pCurBuf,
		keyName[0],	&keyLen[0], dataName[0], &dataSize[0], cfString,
		keyName[1],	&keyLen[1], dataName[1], &dataSize[1], cfString,
		keyName[2],	&keyLen[2], dataName[2], &dataSize[2], cfString,
		keyName[3],	&keyLen[3], dataName[3], &dataSize[3], cfString
		);

	if (STATUS_OK != status												||
		0 != strncmp(keyName[0], CS_KEY_SECTION_NAME 	, keyLen[0])	||
		0 != strncmp(keyName[1], CS_KEY_KEY		      	, keyLen[0])	||
		0 != strncmp(keyName[2], CS_KEY_DATA      		, keyLen[0])	||
		0 != strncmp(keyName[3], CS_KEY_REASON      	, keyLen[0]))
	{
		TRACEINTO << "CCommServiceService::SectionError - bad params for csId: "<<m_csId;
	}
	else
	{
        for(int i = 0 ; i < NumOfParams ; i++)
        {
            pCSMngrProcess->TestStringValidity(dataName[i], ParamLen, __PRETTY_FUNCTION__);
        }

		m_LastErrorSection.Set(dataName[0], dataName[1], dataName[2], dataName[3]);
		TRACEINTO << "CCommServiceService::SectionError - Ok for csId: "<<m_csId;
	}

	DEALLOCBUFFERS(keyName, NumOfParams);
	DEALLOCBUFFERS(dataName, NumOfParams);

	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SendGenericCommonParam(eCommonParamSectionType type)
{
	SectionMethodType method = m_SectionMethodArray[type];
	STATUS res = (this->*method)();

	return res;
}







////////////////////////////////////////////////////////////////////////
// 1 (NEW)
STATUS CCommServiceService::SendServiceName()
{
	std::string  serviceName	= m_IpService->GetName();
	DWORD		 serviceId		= m_IpService->GetId();

	STATUS res = SupportedServiceName(serviceName.c_str(), serviceId);

	return res;
}

////////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SendPortRange()
{
	CIPSpan *csSpan = m_IpService->GetFirstSpan();
	if(NULL == csSpan)
	{
		TRACEINTO << "CCommServiceService::SendPortRange - No SPANs in ip service . At least one must be. csId = "<<m_csId;
		return STATUS_NO_SPANS_IN_SERVICE;
	}

	CCommH323PortRange *portRange = csSpan->GetPortRange();
	WORD tcpFirstPort = portRange->GetTcpFirstPort();
	WORD tcpNumOfPorts = portRange->GetTcpNumberOfPorts();
	TRACEINTO << "CCommServiceService::SendPortRange tcpNumOfPorts =  " << tcpNumOfPorts;
	STATUS res = SupportedPortRange(tcpFirstPort, tcpNumOfPorts);

	return res;
}

////////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SendSIPTransportType()
{
	DWORD theType = eUnknownTransportType;

	if ( (eIPProtocolType_SIP      == m_IpService->GetIPProtocolType()) ||
	     (eIPProtocolType_SIP_H323 == m_IpService->GetIPProtocolType()) )
	{
		CSip *pSip = m_IpService->GetpSip();
		if(NULL == pSip)
		{
			TRACEINTO << "CCommServiceService::SendSIPTransportType - No SIP structure in ip service. csId = "<<m_csId;
			return STATUS_NO_SIP_IN_SERVICE;
		}

		theType = (DWORD)(pSip->GetTransportType());
	}

	else // not a SIP service
	{
		theType = eUnknownTransportType;
		TRACEINTO << "CCommServiceService::SendSIPTransportType - Not a SIP ip service. 'UnknownTransportType' is sent to CS. csId = "<<m_csId;
	}

	STATUS res = SupportedTransportType(theType);

	return res;
}

////////////////////////////////////////////////////////////////////////
// 2 (NEW)
STATUS CCommServiceService::SendIpInterface()
{
	if (NULL == m_IpService)
	{
		TRACEINTO << "ip service is NULL. csId = "<<m_csId;
		return STATUS_NO_SERVICE;
	}

	CIPSpan *csSpan = m_IpService->GetFirstSpan();
	if(NULL == csSpan)
	{
		TRACEINTO << "CCommServiceService::SendIpInterface - No SPANs in ip service . At least one must be. csId = "<<m_csId;
		return STATUS_NO_SPANS_IN_SERVICE;
	}

	eConfigInterfaceType ifType = pCSMngrProcess->GetSignalingInterfaceType(m_csId);

	 eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	 std::string ifName;

	 eIpType ipType = m_IpService->GetIpType();
	 ifName = GetLogicalInterfaceName(ifType,ipType);
     if (eProductTypeSoftMCU == curProductType || eProductTypeSoftMCUMfw == curProductType || eProductTypeEdgeAxis == curProductType) 
     {
	    ifName = csSpan->GetInterface();
     }

	BOOL ifState				= YES;

// from EMA
	const CVlan &pVlan 			= m_IpService->GetVlan();
	BOOL vlan_support			= pVlan.GetIsSupport();
	DWORD vlan_priority			= pVlan.GetPriority();
	DWORD vlan_id				= 0; //pVlan.GetId();

	DWORD ipv4DefaultGateway	= m_IpService->GetDefaultGatewayIPv4();
	DWORD ipv4SubnetMask		= m_IpService->GetNetMask();

	DWORD ipv4Address 		 	= csSpan->GetIPv4Address();
	DWORD ipv4Mtu				= 1500;
	DWORD ipv6_6to4_relay_address = 0;
	DWORD ipv4InternalCardAddress = 0;


	char ipv6_address[IPV6_ADDRESS_LEN];
	memset(ipv6_address, 0, IPV6_ADDRESS_LEN);
	csSpan->GetIPv6Address(0, ipv6_address);


	TRACEINTO << "\nCCommServiceService::SendIpInterface - ipv6_address: "<< ipv6_address << " for csId: " << m_csId
			  << "\nCCommServiceService::SendIpInterface - ifName = "<< ifName
	          << "\nCCommServiceService::SendIpInterface - service name: " << m_IpService->GetName();

	char subNetBuf[IPV6_ADDRESS_LEN];
	memset(subNetBuf, 0, IPV6_ADDRESS_LEN);
	csSpan->GetIPv6SubnetMaskStr(0, subNetBuf);

	string subnetStr = subNetBuf;
	char *ipv6_subnet_mask		= (char*)(subnetStr.c_str());

	eV6ConfigurationType ipV6ConfigType = m_IpService->GetIpV6ConfigurationType(); //AUTO, DHCP, MANUAL
	char *ipv6_config_type		= ::IpV6ConfigurationTypeToString(ipV6ConfigType, true);

	char ip_interface_type[10] = "";
	strncpy(ip_interface_type, ::IpTypeToString(ipType, true), sizeof(ip_interface_type) - 1);
	ip_interface_type[sizeof(ip_interface_type) - 1] = '\0';

	DWORD multi_services_ext_ipv4Address = 0;
	char multi_services_ext_ipv6_address[IPV6_ADDRESS_LEN];
	memset(multi_services_ext_ipv6_address, 0, IPV6_ADDRESS_LEN);

	BOOL isV35JitcSupport = NO;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, isV35JitcSupport);

	//For V35 service, under V35_ULTRA_SECURED_SUPPORT, simulate IPv4 type only - RVGW does not support IPv6
	if(m_IpService->GetIsV35GwEnabled() && isV35JitcSupport)
	{
		strncpy(ip_interface_type, "IPV4", 10);
	}

	BOOL isMultipleServices = FALSE;
	if (pCSMngrProcess->GetIsMultipleServices() == eTRUE)
		isMultipleServices = TRUE;

	//BOOL isMultipleServices = pCSMngrProcess->GetIsMultipleServices();
	//for multiple service begin
	//isMultipleServices = false;
	//end

	TRACEINTO << "\nCCommServiceService::SendIpInterface - isMultipleServices: " << isMultipleServices;
	TRACEINTO << "\nCCommServiceService::SendIpInterface - isServiceV35Enabled: " << m_IpService->GetIsV35GwEnabled();
	if(( isMultipleServices || (m_IpService->GetIsV35GwEnabled() && isV35JitcSupport))
			&& eProductTypeGesher != curProductType && eProductTypeNinja != curProductType)
	{
		DWORD boardIdUsedByCS =0 , subBoardIdUsedByCS =0 ;
		DWORD		 serviceId		= m_IpService->GetId();

		if(m_IpService->GetIsV35GwEnabled())
		{
			pCSMngrProcess->GetSignalingMasterBoardId(m_csId, boardIdUsedByCS, subBoardIdUsedByCS);
		}
		else
		{
			boardIdUsedByCS    = pCSMngrProcess->GetCSIpConfigMasterBoardId(serviceId);
			subBoardIdUsedByCS = pCSMngrProcess->GetCSIpConfigMasterPqId(serviceId);
		}
		if(boardIdUsedByCS)
		{
			vlan_id = CalcMSvlanId(boardIdUsedByCS, subBoardIdUsedByCS);
			ipv4Address	= GetVlanInternalIpv4Address(vlan_id);
			multi_services_ext_ipv4Address = csSpan->GetIPv4Address();
			ipv4SubnetMask = SystemIpStringToDWORD(MS_VLAN_SUBNET_MASK);
			ipv4InternalCardAddress = GetVlanCardInternalIpv4Address(vlan_id); //_M_S_

			TRACEINTO << "\nCCommServiceService::SendIpInterface - multiple data: boardIdUsedByCS=" << boardIdUsedByCS << " ipv4="<< ipv4Address<< " vlan="<<vlan_id;
			//TODO - Ori
			//multi_services_ext_ipv6_address = ????
		}
		else
			TRACEINTO << "\nCCommServiceService::SendIpInterface - multiple data: illegal board ID";
	}
	if( isMultipleServices && (eProductTypeGesher == curProductType || eProductTypeNinja == curProductType))
	{
		multi_services_ext_ipv4Address = ipv4Address;
	}

	char ipDnsAddress[IPV6_ADDRESS_LEN];
	memset(ipDnsAddress, 0, IPV6_ADDRESS_LEN);
	CIpDns *dns = m_IpService->GetpDns();
	int ipAdd0_dns1 = dns->GetIPv4Address(0);


	//network separation
	if( !isMultipleServices )
	{
		ipv4InternalCardAddress = 0;
		//get dns from IP
		if(dns->GetStatus() == eServerStatusSpecify)
		{
			//check if ipv4 or ipv6
			if(ipAdd0_dns1 != 0 )
			{
				SystemDWORDToIpString(ipAdd0_dns1, ipDnsAddress);
			}
			else
			{
				dns->GetIPv6Address(0,ipDnsAddress);
			}
		}
		else
		{
			ipDnsAddress[0] = '\0';
		}

	}
	//multiple service
	else
	{
        //ipDnsAddress[0] = '\0';

        //Original code is only above one line and set the dnsIp to 0
        //I don't know why, but it doesn't work on Multipe-Service
        TRACEINTO << "\nCCommServiceService::SendIpInterface Multiple Service DNSStatus: " << dns->GetStatus()
                  << " IpServiceName:" << m_IpService->GetName() << " ifName:"<< ifName << " ipAdd0_dns1:" << ipAdd0_dns1; 
		if(dns->GetStatus() == eServerStatusSpecify)
		{
			if(ipAdd0_dns1 != 0 )
			{
				SystemDWORDToIpString(ipAdd0_dns1, ipDnsAddress);
			}
			else
			{
				dns->GetIPv6Address(0,ipDnsAddress);
			}
		}
		else
		{
			ipDnsAddress[0] = '\0';
		}	
        TRACEINTO << "\nCCommServiceService::SendIpInterface ipDnsAddress: " << ipDnsAddress;
	}

	STATUS res = SupportedIpInterface(
		ifName.c_str(),
		ifState,
		vlan_support,
		vlan_priority,
		vlan_id,
		ipv4Address,
		ipv4DefaultGateway,
		ipv4SubnetMask,
		ipv4Mtu,
		ipv6_6to4_relay_address,
		ipv6_address,
		ipv6_subnet_mask,
		ipv6_config_type,
		ip_interface_type,
		(isMultipleServices || (m_IpService->GetIsV35GwEnabled() && isV35JitcSupport) ) ,
		multi_services_ext_ipv4Address,
		multi_services_ext_ipv6_address,
		ipv4InternalCardAddress,//_M_S_
		ipDnsAddress);

	return res;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CCommServiceService::GetVlanInternalIpv4Address(const DWORD vlanId)
{
	std::string internalIpv4Address = "";

	switch(vlanId)
	{
		case(2012):
		{
			internalIpv4Address = VLAN_2012_CS_ADDRESS;
			break;
		}
		case(2013):
		{
			internalIpv4Address = VLAN_2013_CS_ADDRESS;
			break;
		}
		case(2022):
		{
			internalIpv4Address = VLAN_2022_CS_ADDRESS;
			break;
		}
		case(2023):
		{
			internalIpv4Address = VLAN_2023_CS_ADDRESS;
			break;
		}
		case(2032):
		{
			internalIpv4Address = VLAN_2032_CS_ADDRESS;
			break;
		}
		case(2033):
		{
			internalIpv4Address = VLAN_2033_CS_ADDRESS;
			break;
		}
		case(2042):
		{
			internalIpv4Address = VLAN_2042_CS_ADDRESS;
			break;
		}
		case(2043):
		{
			internalIpv4Address = VLAN_2043_CS_ADDRESS;
			break;
		}

	}

	DWORD ipv4 = SystemIpStringToDWORD(internalIpv4Address.c_str());

	TRACESTR(eLevelInfoNormal) << "CCommServiceService::GetVlanInternalIpv4Address - vlanId: " << vlanId;
	TRACESTR(eLevelInfoNormal) << "CCommServiceService::GetVlanInternalIpv4Address - internalIpv4Address: " << internalIpv4Address;
	TRACESTR(eLevelInfoNormal) << "CCommServiceService::GetVlanInternalIpv4Address - ipv4: " << ipv4;

	return ipv4;
}


////////////////////////////////////////////////////////////////////////
// 3
STATUS CCommServiceService::SendSupportedProtocols()
{
    WORD H323 = FALSE;
	WORD SIP  = FALSE;

    switch(m_IpService->GetIPProtocolType())
    {
    case eIPProtocolType_SIP:
        {
            SIP  = TRUE;
            H323 = FALSE;
            break;
        }
    case eIPProtocolType_H323:
        {
            SIP  = FALSE;
            H323 = TRUE;
            break;
        }
    case eIPProtocolType_SIP_H323:
        {
            SIP  = TRUE;
            H323 = TRUE;
            break;
        }
    default :
        {
			H323 = TRUE;
			SIP  = TRUE;
			TRACEINTO << "CCommServiceService::SendSupportedProtocols - illegal protocol type. csId = "<<m_csId;
            return STATUS_ILLEGAL_IP_PROTOCOL;
            break;
        }
    }

    STATUS res = SupportedProtocolsRequest(H323,SIP);

	return res;
}

////////////////////////////////////////////////////////////////////////
// 4
STATUS CCommServiceService::SendDHCP()
{
	WORD isSipFromDhcp  = NO;
	WORD isGkFromDhcp   = NO;
	WORD isDhcpEnabled = m_IpService->GetDHCPServer();
	if (isDhcpEnabled)
	{
		if (m_IpService->GetIPProtocolType() != eIPProtocolType_H323 &&
			m_IpService->GetpSip()->GetConfigurationOfSIPServers() ==
			eConfSipServerAuto)
		{
			isSipFromDhcp = YES;
		}

		if (m_IpService->GetIPProtocolType() != eIPProtocolType_SIP &&
			m_IpService->GetGatekeeper() == GATEKEEPER_INTERNAL)
		{
			isGkFromDhcp = YES;
		}
	}

    STATUS status = SupportedDHCP( isDhcpEnabled,
								   isSipFromDhcp,
								   isGkFromDhcp);

	return status;
}

////////////////////////////////////////////////////////////////////////
// 6
STATUS CCommServiceService::SendDNSCfg()
{
	CIpDns *pDns = m_IpService->GetpDns();
	if(NULL == pDns)
	{
		TRACEINTO << "CCommServiceService::SendDNSCfg - DNS does not exist. csId = "<<m_csId;
		return STATUS_NO_DNS_IN_SERVICE;
	}

	DWORD isUseDns      = FALSE;
	DWORD isGetFromDHCP = FALSE;

	switch (pDns->GetStatus())
	{
	case eServerStatusOff:
		{
			isUseDns      = FALSE;
			isGetFromDHCP = FALSE;
			break;
		}
	case eServerStatusSpecify:
		{
			isUseDns      = TRUE;
			isGetFromDHCP = FALSE;
			break;
		}
	case eServerStatusAuto:
		{
			isUseDns      = TRUE;
			if (m_IpService->GetDHCPServer())
			{
				isGetFromDHCP = TRUE;
			}
			else
			{
				// DHCP should not be off if DNS is in auto configuration !
				isGetFromDHCP = FALSE;
				TRACEINTO << "CCommServiceService::SendDNSCfg - DHCP cannot be off if DNS is in auto configuration. csId = "<<m_csId;
				return STATUS_NOT_VALID_DHCP;
			}
			break;
		}
	default:
		{
			TRACEINTO << "CCommServiceService::SendDNSCfg - illegal Dns status. csId = "<<m_csId;
			return STATUS_ILLEGAL_DNS_STATUS;
			break;
		}
	}

    STATUS status = SupportedDNSCfg(isUseDns,isGetFromDHCP);

	return status;
}


////////////////////////////////////////////////////////////////////////
// 7
STATUS CCommServiceService::SendDNSCalls()
{
	CIpDns *pDns = m_IpService->GetpDns();
	if(NULL == pDns)
	{
		TRACEINTO << "CCommServiceService::SendDNSCalls - DNS does not exist. csId = "<<m_csId;
		return STATUS_NO_DNS_IN_SERVICE;
	}

	WORD isAcceptDNSCalls 	= pDns->GetAcceptCallsViaDNS();
	WORD isRegisterToDNS 	= m_IpService->IsAutoRegisterSpanHostName();
	std::string hostName 	= m_IpService->GetName();
	std::string domainName 	= pDns->GetDomainName().GetString();

	if (!domainName.empty())
	{
		hostName = hostName + ".";
		hostName = hostName + domainName;
	}

    STATUS status = SupportedDNSCalls(isAcceptDNSCalls,isRegisterToDNS,hostName.c_str());

	return status;
}

////////////////////////////////////////////////////////////////////////
// 8
STATUS CCommServiceService::SendDNSName()
{
	CIpDns *pDns = m_IpService->GetpDns();
	if(NULL == pDns)
	{
		TRACEINTO << "CCommServiceService::SendDNSName - DNS does not exist. csId = "<<m_csId;
		return STATUS_NO_DNS_IN_SERVICE;
	}

	WORD isGetFromDHCP = (pDns->GetStatus() == eServerStatusAuto ? TRUE : FALSE);
	std::string domainName = (FALSE == isGetFromDHCP ? pDns->GetDomainName().GetString() : "Polycom");

	CIPSpan* csSpan = m_IpService->GetFirstSpan();
	if(NULL == csSpan)
	{
		TRACEINTO << "CCommServiceService::SendDNSName - No SPANs in ip service . At least one must be. csId = "<<m_csId;
		return STATUS_NO_SPANS_IN_SERVICE;
	}

	std::string hostName = (FALSE == isGetFromDHCP ? GetHostNameFromService(m_IpService) : "Polycom");

	// can't send empty fields, CS Module fails.
	if(hostName.empty())
	{
		TRACEINTO << "CCommServiceService::SendDNSName - Host name does not exist. csId = "<<m_csId;
		return STATUS_NO_HOST_NAME;
	}

	if(domainName.empty())
	{
		TRACEINTO << "CCommServiceService::SendDNSName - Domain name does not exist. csId = "<<m_csId;
		return STATUS_NO_DOMAIN_NAME;
	}

    STATUS res = SupportedDNSName(	isGetFromDHCP,
					  				domainName.c_str(),
					  				hostName.c_str());

	return res;
}

////////////////////////////////////////////////////////////////////////
// 9
STATUS CCommServiceService::SendAuthenticationEntry()
{
	CIPSecurity *pSecurity = m_IpService->GetpSecurity();
	if(NULL == pSecurity)
	{
		TRACEINTO << "CCommServiceService::SendAuthenticationEntry - Security does not exist. csId = "<<m_csId;
		return STATUS_NO_SEQURITY_IN_SERVICE;
	}

	CIPAuthentication* pAuth = pSecurity->GetIPAuthentication();
	if(NULL == pAuth)
	{
		TRACEINTO << "CCommServiceService::SendAuthenticationEntry - Authentication does not exist. csId = "<<m_csId;
		return STATUS_NO_AUT_IN_SERVICE;
	}

	DWORD kerberosItemNumber 	= pAuth->GetKerberosAuthenticationNumber();
	DWORD HTTPDigestItemNumber 	= pAuth->GetHTTPDigestAuthenticationNumber();

    char *protocolName 					= NULL;
	CIPAuthenticationElement *ipAutElem = NULL;

	if (kerberosItemNumber)
	{
		protocolName = "KERBEROS";

		ipAutElem = pAuth->GetKerberosAuthenticationElement(pAuth->GetKerberosAuthenticationNumber() - kerberosItemNumber);
		kerberosItemNumber--;
		pAuth->SetKerberosAuthenticationNumber(kerberosItemNumber);
	}
    else if (HTTPDigestItemNumber)
	{
		protocolName = "DIGEST";

		ipAutElem = pAuth->GetHTTPDigestAuthenticationElement(pAuth->GetHTTPDigestAuthenticationNumber() - HTTPDigestItemNumber);
		HTTPDigestItemNumber--;
		pAuth->SetHTTPDigestAuthenticationNumber(HTTPDigestItemNumber);
	}

	if(NULL == ipAutElem)
	{
		TRACEINTO << "CCommServiceService::SendAuthenticationEntry - authentication element does not exists. csId = "<<m_csId;
		return STATUS_NO_AUT_ELEMENT_IN_SERVICE;
	}

	if(!ipAutElem->GetAuthenticationEnable())
	{
		TRACEINTO << "CCommServiceService::SendAuthenticationEntry - authentication element is not enabled. csId = "<<m_csId;
		return STATUS_NO_AUT_ELEMENT_IN_SERVICE;
	}

	const char *action 				= "ADD";
	const char * userName   		= ipAutElem->GetUserName().GetString();
	const char * password   		= ipAutElem->GetPassword().GetString();
	const char * domainName 		= ipAutElem->GetDomainName().GetString();
	const char * serverName 		= ipAutElem->GetServerName().GetString();
	const DWORD  serverIpAddress 	= ipAutElem->GetKeyIP();

	if(NULL == protocolName)
	{
		TRACEINTO << "CCommServiceService::SendAuthenticationEntry - Protocol name does not exist. csId = "<<m_csId;
		return STATUS_NO_PROTOCOL_NAME;
	}
	if(NULL == userName)
	{
		TRACEINTO << "CCommServiceService::SendAuthenticationEntry - User name does not exist. csId = "<<m_csId;
		return STATUS_NO_USER_NAME;
	}
	if(NULL == password)
	{
		TRACEINTO << "CCommServiceService::SendAuthenticationEntry - Password does not exist. csId = "<<m_csId;
		return STATUS_NO_PASSWORD;
	}
	if(NULL == domainName)
	{
		TRACEINTO << "CCommServiceService::SendAuthenticationEntry - Domain name does not exist. csId = "<<m_csId;
		return STATUS_NO_DOMAIN_NAME;
	}
	if(NULL == serverName)
	{
		TRACEINTO << "CCommServiceService::SendAuthenticationEntry - DNS prefix name does not exist. csId = "<<m_csId;
		return STATUS_NO_SERVER_NAME;
	}

    STATUS status = SupportedAuthenticationEntry( action,
												  protocolName,
												  userName,
												  password,
												  domainName,
												  serverName,
												  serverIpAddress);

	return status;
}

////////////////////////////////////////////////////////////////////////
//8
STATUS CCommServiceService::SendSIPSpanType()
{
	std::string name = m_IpService->GetpDns()->GetPrefixName().GetString();
	char *type = "URI";

	if(name.empty())
	{
		TRACEINTO << "CCommServiceService::SendSIPSpanType - DNS prefix name does not exist. csId = "<<m_csId;
		return STATUS_NO_DNS_PREFIX_NAME;
	}

    STATUS res = SupportedSIPSpanType(type,name.c_str());

	return res;
}


//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SendToCsApi(OPCODE opcode, const int dataLen, const char *pData)
{
	if(opcode != CS_NEW_SERVICE_INIT_REQ &&
			opcode != CS_NEW_SERVICE_INIT_IND &&
			opcode != CS_COMMON_PARAM_REQ) //VNGR-10970 - prevent passwords being sent into the log.
	{
		TRACEINTO << CCSBufferIndWrapper(pData, dataLen);
	}

	const int fullDataLen	= dataLen + sizeof(DWORD);

	char *pFullData = new char [fullDataLen];

	*((DWORD*)pFullData) = dataLen;
	memcpy(pFullData + sizeof(DWORD), pData, dataLen);

	STATUS res = CCommCsModuleService::SendToCsApi(opcode, eServiceMngr, fullDataLen, pFullData, m_csId);

	PDELETEA(pFullData);

	return res;
}


//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SectionOKDelService(cfgBufHndl &request)
{
	char serviceName[STR_LEN];

	const int NumOfParams	= 1;
	const int ParamLen		= 256;
	int   keyLen[NumOfParams] = {ParamLen};
	int dataSize;

	ALLOCBUFFERS(keyName,ParamLen,NumOfParams);

	STATUS status = CfgGetParamsContinue(
		&request,
		NumOfParams,
		request.pCurBuf,
		keyName[0],	&keyLen[0], serviceName, &dataSize, cfString
		);

	if (STATUS_OK != status											||
		0 != strncmp(keyName[0], CS_KEY_SERVICE_NAME      , keyLen[0]))
	{
		std::string assert = "CCommServiceService::SectionOKDelService - bad params. csId = " + m_csId;
		PASSERTMSG(1,assert.c_str());
	}
	else
	{
		// update the relevant service;
	}

	DEALLOCBUFFERS(keyName, NumOfParams);

	return STATUS_OK;
}







/////////////////////////////////////////////////////////////////////////////
// 0
STATUS CCommServiceService::SupportedNewService(const char *serviceName, DWORD serviceId)
{
	char fastBuf[COMMON_REQ_BUF_SIZE];

	int allocBufSize;

	int res = CfgSetParams(
		2,
		fastBuf,
		COMMON_REQ_BUF_SIZE,
		&allocBufSize,
		CS_KEY_NEW_SERVICE,
		CS_KEY_SERVICE_NAME,		serviceName,				cfString,
		CS_KEY_SERVICE_ID,	    	serviceId,			    	cfINT32
		);

	if(0 != res)
	{
		TRACEINTO << "CCommServiceService::SupportedNewService: - failed with status: "<<res<<" for csId: "<<m_csId;
		return STATUS_FAILED_TO_SEND_MSG_TO_CS;
	}

	STATUS status = SendToCsApi(CS_NEW_SERVICE_INIT_REQ, allocBufSize, fastBuf);

	return status;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SupportedPortRange(WORD tcpFirstPort, WORD tcpNumOfPorts)
{
	char fastBuf[COMMON_REQ_BUF_SIZE];
	memset(fastBuf, 0, COMMON_REQ_BUF_SIZE);
	int allocBufSize;

	int res = CfgSetParams(
		2,
		fastBuf,
		COMMON_REQ_BUF_SIZE,
		&allocBufSize,
		CS_KEY_PORT_RANGE,
		CS_KEY_TCP_FIRST_PORT,	    tcpFirstPort,	cfUINT16,
		CS_KEY_TCP_NUM_PORTS,	    tcpNumOfPorts,	cfUINT16
		);

	if(0 != res)
	{
		TRACEINTO << "CCommServiceService::SupportedPortRange: - failed with status: "<<res<<" for csId: "<<m_csId;
		return STATUS_FAILED_TO_SEND_MSG_TO_CS;
	}

	STATUS status = SendToCsApi(CS_COMMON_PARAM_REQ, allocBufSize, fastBuf);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SupportedTransportType(DWORD theType)
{
	char fastBuf[COMMON_REQ_BUF_SIZE];
	int allocBufSize;

	int res = CfgSetParams(
		1,
		fastBuf,
		COMMON_REQ_BUF_SIZE,
		&allocBufSize,
		CS_KEY_SIP_TRANSPORT_TYPE,
		CS_KEY_TYPE, theType, cfINT32
		);

	if(0 != res)
	{
		TRACEINTO << "CCommServiceService::SupportedTransportType: - failed with status: "<<res<<" for csId: "<< m_csId;
		return STATUS_FAILED_TO_SEND_MSG_TO_CS;
	}

	STATUS status = SendToCsApi(CS_COMMON_PARAM_REQ, allocBufSize, fastBuf);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
// 1 ???????????????????????????????????????????????????????????????????????????????
STATUS CCommServiceService::SupportedServiceName(const char *serviceName, DWORD serviceId)
{
	char fastBuf[COMMON_REQ_BUF_SIZE];

	int allocBufSize;

	int res = CfgSetParams(
		2,
		fastBuf,
		COMMON_REQ_BUF_SIZE,
		&allocBufSize,
		CS_KEY_SERVICE_NAME,
		CS_KEY_SERVICE_NAME,		serviceName,				cfString,
		CS_KEY_SERVICE_ID,	    	serviceId,			    	cfINT32
		);

	if(0 != res)
	{
		TRACEINTO << "CCommServiceService::SupportedServiceName: - failed with status: "<<res<<" for csId: "<<m_csId;
		return STATUS_FAILED_TO_SEND_MSG_TO_CS;
	}

	STATUS status = SendToCsApi(CS_COMMON_PARAM_REQ, allocBufSize, fastBuf);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SupportedIpInterface(
				const char *ifName,
				BOOL ifState,
				BOOL vlan_support,
				DWORD vlan_priority,
				DWORD vlan_id,
				DWORD ipv4Address,
				DWORD ipv4DefaultGateway,
				DWORD ipv4SubnetMask,
				DWORD ipv4Mtu,
				DWORD ipv6_6to4_relay_ad,
				char *ipv6_address,
				char *ipv6_subnet_mask,
				char *ipv6_config_type,
				char *ip_interface_type,
				BOOL multiple_service,
				DWORD multi_services_ext_ipv4Address,
				char *multi_services_ext_ipv6_address,
				DWORD ipv4InternalCardAddress, //_M_S_
				char * ipDnsAddress)
{
	char fastBuf[COMMON_REQ_BUF_SIZE];

	int allocBufSize;

	int res = CfgSetParams(
		19,
		fastBuf,
		COMMON_REQ_BUF_SIZE,
		&allocBufSize,
		CS_KEY_IP_INTERFACE,
		CS_KEY_IF_NAME,						ifName,								cfString,
		CS_KEY_IF_STATE,	    			ifState,							cfBOOL,

		CS_KEY_VLAN_SUPPORT,				vlan_support,						cfBOOL,
		CS_KEY_VLAN_PRIORITY,				vlan_priority,						cfINT32,
		CS_KEY_VLAN_ID,						vlan_id,							cfINT32,

		CS_KEY_IPv4_ADDRESS,				ipv4Address,						cfIpAddress,
		CS_KEY_IPv4_DEFAULT_GATEWAY,		ipv4DefaultGateway,					cfIpAddress,
		CS_KEY_IPv4_SUBNET_MASK,			ipv4SubnetMask,						cfIpAddress,
		CS_KEY_IPv4_MTU,					ipv4Mtu,							cfIpAddress,

		CS_KEY_IPv6_6to4_RELAY_ADDRESS,		ipv6_6to4_relay_ad,					cfIpAddress,
		CS_KEY_IPv6_ADDRESS,				ipv6_address,						cfString,
		CS_KEY_IPv6_SUBNET_MASK,			ipv6_subnet_mask,					cfString,
		CS_KEY_IPv6_CONFIG_TYPE,			ipv6_config_type,   				cfString,

		CS_KEY_IP_INTERFACE_TYPE,			ip_interface_type,					cfString,

		CS_KEY_MULTI_SRV_FLAG,              multiple_service,   				cfBOOL,
		CS_KEY_MULTI_SRV_IPv4_ADDRESS_EXT,  multi_services_ext_ipv4Address,		cfIpAddress,
		CS_KEY_MULTI_SRV_IPv6_ADDRESS_EXT,  multi_services_ext_ipv6_address,	cfString,
		CS_KEY_IPv4_CARD_ADDRESS,			ipv4InternalCardAddress,			cfIpAddress, //_M_S_
		CS_KEY_IP_DNS_ADDRESS_STRING,  		ipDnsAddress,						cfString

		);

	if(0 != res)
	{
		TRACEINTO << "CCommServiceService::SupportedServiceName: - failed with status: "<< res<<" for csId: "<<m_csId;
		return STATUS_FAILED_TO_SEND_MSG_TO_CS;
	}

	char ipAddressStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString(ipv4Address, ipAddressStr);
	char ipv4DefaultGatewayStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString(ipv4DefaultGateway, ipv4DefaultGatewayStr);
	char ipv4extipv4AddressStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString(multi_services_ext_ipv4Address, ipv4extipv4AddressStr);
	char internalCardIpAddressStr[IP_ADDRESS_LEN];	//_M_S
	SystemDWORDToIpString(ipv4InternalCardAddress, internalCardIpAddressStr);


	CLargeString str = "\nCCommServiceService::SupportedIpInterface";
	str << "\ncsId: " << m_csId
		<< "\nInterface name: "	<< ifName
	    << "\nInterfaceState: " << ( (TRUE == ifState) ? "yes" : "no" )
	    << "\nvlan_support: "   << ( (TRUE == vlan_support) ? "yes" : "no" )
	    << "\nvlan_priority: "  << vlan_priority
	    << "\nvlan_id: "		<< vlan_id
	    << "\nipv4Address: "	<< ipv4Address << "(" << ipAddressStr << ")"
	    << "\nipv4DefaultGateway: " << ipv4DefaultGateway << "(" << ipv4DefaultGatewayStr << ")"
	    << "\nipv4SubnetMask: " << ipv4SubnetMask
	    << "\nipv4Mtu: " 		<< ipv4Mtu
	    << "\nipv6_6to4_relay_ad: " << ipv6_6to4_relay_ad
	    << "\nipv6_address: " 		<< ipv6_address
	    << "\nipv6_subnet_mask: " 	<< ipv6_subnet_mask
	    << "\nipv6_config_type: " 	<<  ipv6_config_type
	    << "\nip_interface_type: " 	<< ip_interface_type
	    << "\nmultiple_service: " 	<< multiple_service
	    << "\nmulti_services_ext_ipv4Address: " << multi_services_ext_ipv4Address << "(" << ipv4extipv4AddressStr << ")"
	    << "\nmulti_services_ext_ipv6_address: " << multi_services_ext_ipv6_address
		<< "\nipv4InternalCardAddress: " << internalCardIpAddressStr
		<< "\nipDnsAddress: " << ipDnsAddress;

	TRACESTR(eLevelInfoNormal) << str.GetString();
		
	STATUS status = SendToCsApi(CS_COMMON_PARAM_REQ, allocBufSize, fastBuf);

	BOOL isMultipleServices = FALSE;
	if (pCSMngrProcess->GetIsMultipleServices() == eTRUE)
		isMultipleServices = TRUE;

	CIpDns *dns = m_IpService->GetpDns();
	//if destination is dns of signaling same port
	if(IsJitcAndNetSeparation() && !isMultipleServices && dns->GetStatus() == eServerStatusSpecify)
	{

		//getting route table
		eConfigInterfaceNum  ifNum;
		DWORD tmpIfType;
		DWORD tmpIpType;
		eConfigInterfaceType ifType = pCSMngrProcess->GetSignalingInterfaceType(m_csId);

		std::string route_table = "";
		eIpType ipType = m_IpService->GetIpType();

		ifNum = GetInterfaceNum(ifType,ipType);

		route_table = GetRouteTableName(ifNum);

		//end getting route table


		CConfigManagerApi configuratorApi;
		DWORD retIP = 0;
		DWORD retVal;
		STATUS respStatus = configuratorApi.AddIpandNatRuleForDNSPerService(ipDnsAddress, route_table,  10000,  isIpV4Str(ipDnsAddress), ipAddressStr,  retVal);




	}
//-S--PLCM_DNS-------------------------//
	std::string sIpv4 = ipAddressStr, sIpv6 = ipv6_address;
	CSegment* pParam = new CSegment;
	*pParam << m_csId
			<< sIpv4
			<< sIpv6;
	CManagerApi dnsAgentMngr(eProcessDNSAgent);
	dnsAgentMngr.SendMsg(pParam,CSMNGR_TO_DNSAGENT_CS_CONFIGURED);
//-E--PLCM_DNS-------------------------//

	CSegment* pParamRrsr = new CSegment;
			*pParamRrsr << m_csId << ifName;
		CManagerApi resourceMngr(eProcessResource);
		resourceMngr.SendMsg(pParamRrsr,CSMNGR_TO_RESOURSE_INTERFACE_UPDATE);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
//1
STATUS CCommServiceService::SupportedProtocolsRequest(WORD isH323, WORD isSip)
{
	char fastBuf[COMMON_REQ_BUF_SIZE];
	int allocBufSize;

	int res = CfgSetParams(
		2,
		fastBuf,
		COMMON_REQ_BUF_SIZE,
		&allocBufSize,
		CS_KEY_SUPPORTED_PROTOCOLS,
		CS_KEY_H323,			isH323,					cfBOOL,
		CS_KEY_SIP,	    		isSip,			    	cfBOOL
		);

	if(0 != res)
	{
		TRACEINTO << "CCommServiceService::SupportedProtocolsRequest: - failed with status: "<<res<<" .csId: "<<m_csId;
		return STATUS_FAILED_TO_SEND_MSG_TO_CS;
	}

	STATUS status = SendToCsApi(CS_COMMON_PARAM_REQ, allocBufSize, fastBuf);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
// 4
STATUS  CCommServiceService::SupportedDNSCfg(WORD isUSED, WORD isGetFromDHCP)
{
	char fastBuf[COMMON_REQ_BUF_SIZE];
	int allocBufSize;

	int res = CfgSetParams(
		2,
		fastBuf,
		COMMON_REQ_BUF_SIZE,
		&allocBufSize,
		CS_KEY_DNS_CFG,
		CS_KEY_USE,			    isUSED,					cfBOOL,
		CS_KEY_GET_FROM_DHCP,	isGetFromDHCP,			cfBOOL
		);

	if(0 != res)
	{
		TRACEINTO << "CCommServiceService::SupportedDNSCfg: - failed with status: "<< res<<" .csId: "<<m_csId;
		return STATUS_FAILED_TO_SEND_MSG_TO_CS;
	}

	STATUS status = SendToCsApi(CS_COMMON_PARAM_REQ, allocBufSize, fastBuf);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
// 5
STATUS  CCommServiceService::SupportedDNSCalls(WORD isAcceptDNSCalls,WORD isRegisterToDNS,const char* HostName)
{
	char fastBuf[COMMON_REQ_BUF_SIZE];
	int allocBufSize;

	int res = CfgSetParams(
		3,
		fastBuf,
		COMMON_REQ_BUF_SIZE,
		&allocBufSize,
		CS_KEY_DNS_CALLS,
		CS_KEY_ACCEPT_DNS_CALLS,			isAcceptDNSCalls,		cfBOOL,
		CS_KEY_REGISTER_TO_DNS,	    	isRegisterToDNS,		cfBOOL,
        CS_KEY_HOST_NAME,                 HostName,               cfString
		);

	if(0 != res)
	{
		TRACEINTO << "CCommServiceService::SupportedDNSCalls: - failed with status: "<<res<<" .csId = "<<m_csId;
		return STATUS_FAILED_TO_SEND_MSG_TO_CS;
	}

	STATUS status = SendToCsApi(CS_COMMON_PARAM_REQ, allocBufSize, fastBuf);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
// 6
STATUS
CCommServiceService::SupportedDNSName
(WORD isGetFromDHCP,const char* domainName,const char* hostName)
{
	char fastBuf[COMMON_REQ_BUF_SIZE];
	int allocBufSize;

	int res = CfgSetParams(
		3,
		fastBuf,
		COMMON_REQ_BUF_SIZE,
		&allocBufSize,
		CS_KEY_DNS_NAME,
		CS_KEY_GET_FROM_DHCP,	isGetFromDHCP,	    cfBOOL,
		CS_KEY_DOMAIN_NAME,	    domainName,			cfString,
        CS_KEY_HOST_NAME,       hostName,           cfString
		);

	if(0 != res)
	{
		TRACEINTO << "CCommServiceService::isGetFromDHCP: - failed with status: "<<res<<" .csId = "<<m_csId;
		return STATUS_FAILED_TO_SEND_MSG_TO_CS;
	}

	STATUS status = SendToCsApi(CS_COMMON_PARAM_REQ, allocBufSize, fastBuf);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
//7
STATUS
CCommServiceService::SupportedAuthenticationEntry(
									const char* action,const char* protocolName,
									const char* userName,const char* password,
									const char* domainName,const char* serverName,
									DWORD serverIpAddress)
{
	char fastBuf[COMMON_REQ_BUF_SIZE];
	int allocBufSize;

	int res = CfgSetParams(
		7,
		fastBuf,
		COMMON_REQ_BUF_SIZE,
		&allocBufSize,
		CS_KEY_AUTHENTICATION_ENTRY,
		CS_KEY_ACTION,			action,				cfString,
		CS_KEY_PROTOCOL,	    	protocolName,		cfString,
        CS_KEY_USER,	    	    userName,	    	cfString,
        CS_KEY_PASSWORD,	    	password,	    	cfString,
        CS_KEY_DOMAIN_NAME,	    domainName,		    cfString,
        CS_KEY_SERVER_NAME,	    serverName,		    cfString,
        CS_KEY_SERVER_IP_ADDRESS,serverIpAddress,    cfIpAddress
		);

	if(0 != res)
	{
		TRACEINTO << "CCommServiceService::SupportedAuthenticationEntry: - failed with status: "<< res <<" .csId = "<<m_csId;
		return STATUS_FAILED_TO_SEND_MSG_TO_CS;
	}

	STATUS status = SendToCsApi(CS_COMMON_PARAM_REQ, allocBufSize, fastBuf);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
//8
STATUS CCommServiceService::SupportedSIPSpanType(const char* spanType, const char* name)
{
	char fastBuf[COMMON_REQ_BUF_SIZE];
	int allocBufSize;

	int res = CfgSetParams(
		2,
		fastBuf,
		COMMON_REQ_BUF_SIZE,
		&allocBufSize,
		CS_KEY_SIP_SPAN_TYPE,
		CS_KEY_TYPE,			spanType,					cfString,
		CS_KEY_NAME,	    	name,   			    	cfString
		);

	if(0 != res)
	{
		TRACEINTO << "CCommServiceService::SupportedSIPSpanType: - failed with status: "<< res<< ". csId = "<<m_csId;
		return STATUS_FAILED_TO_SEND_MSG_TO_CS;
	}

	STATUS status = SendToCsApi(CS_COMMON_PARAM_REQ, allocBufSize, fastBuf);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
//2
STATUS CCommServiceService::SupportedDHCP(WORD isDHCP,
							 WORD isSipFromDhcp,
							 WORD isGkFromDhcp)
{
    char fastBuf[COMMON_REQ_BUF_SIZE];
	int allocBufSize;

    	int res = CfgSetParams(
		3,
		fastBuf,
		COMMON_REQ_BUF_SIZE,
		&allocBufSize,
		CS_KEY_DHCP,
		CS_KEY_DHCP_ENABLED,             isDHCP,        cfBOOL,
		CS_KEY_GET_SIP_PROXY_FROM_DHCP,  isSipFromDhcp, cfBOOL,
		CS_KEY_GET_GK_IP_FROM_DHCP,      isGkFromDhcp,  cfBOOL);

	if(0 != res)
	{
		TRACEINTO << "CCommServiceService::SupportedDHCP: - failed with status: "<<res<<" .csId = "<<m_csId;
		return STATUS_FAILED_TO_SEND_MSG_TO_CS;
	}

	STATUS status = SendToCsApi(CS_COMMON_PARAM_REQ, allocBufSize, fastBuf);

	return status;
}


//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SupportedEndServiceInitReq(const char *serviceName)
{
	char fastBuf[COMMON_REQ_BUF_SIZE];
	int allocBufSize;

    int res = CfgSetParams(
		1,
		fastBuf,
		COMMON_REQ_BUF_SIZE,
		&allocBufSize,
		CS_KEY_END_SERVICE,
		CS_KEY_SERVICE_NAME, serviceName, cfString);

	if(0 != res)
	{
		TRACEINTO << "CCommServiceService::SupportedEndServiceInitReq: - failed with status: "<<res<<" .csId = "<<m_csId;
		return STATUS_FAILED_TO_SEND_MSG_TO_CS;
	}

	STATUS status = SendToCsApi(CS_END_SERVICE_INIT_REQ, allocBufSize, fastBuf);

	return status;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SupportedDelService(const char *serviceName)
{
	char fastBuf[COMMON_REQ_BUF_SIZE];
	int allocBufSize;

    	int res = CfgSetParams(
		1,
		fastBuf,
		COMMON_REQ_BUF_SIZE,
		&allocBufSize,
		CS_KEY_DEL_SERVICE,
		CS_KEY_SERVICE_NAME, serviceName, cfString);

	if(0 != res)
	{
		TRACEINTO << "CCommServiceService::SupportedDelService: - failed with status: "<< res<<" csId = "<<m_csId;
		return STATUS_FAILED_TO_SEND_MSG_TO_CS;
	}

	STATUS status = SendToCsApi(CS_DEL_SERVICE_REQ, allocBufSize, fastBuf);

	return status;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::UpdatedServiceInfo(cfgBufHndl &commonRequestHandle)
{
	TRACEINTO << "CCommServiceService::UpdatedServiceInfo";

	CServiceInfo serviceInfo;

	const int NumOfParams	= 3;
	const int ParamLen		= 256;

	int   keyLen	[NumOfParams] = {ParamLen,ParamLen,ParamLen};
	int	  dataSize	[NumOfParams] = {ParamLen,ParamLen,ParamLen};

	ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
	ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

	STATUS status = STATUS_INFO_MESSAGE_OK;

	int res = CfgGetParamsContinue(
		&commonRequestHandle,
		NumOfParams,
		commonRequestHandle.pCurBuf,
		keyName[0],	&keyLen[0], dataName[0]				,	&dataSize[0], cfString,
		keyName[1],	&keyLen[1], &(serviceInfo.m_Id)		,   &dataSize[1], cfINT32,
		keyName[2],	&keyLen[2], &(serviceInfo.m_Status)	,	&dataSize[2], cfINT32
		);

	serviceInfo.m_Name = dataName[0];

	if (0 != res													||
		0 != strncmp(keyName[0], CS_KEY_SERVICE_NAME , keyLen[0])	||
		0 != strncmp(keyName[1], CS_KEY_SERVICE_ID   , keyLen[1])	||
		0 != strncmp(keyName[2], CS_KEY_STATUS		 , keyLen[2]))
	{
		char str_csId[2];
		snprintf(str_csId, sizeof(str_csId), "%d",m_csId);
		
		std::string assert = "CCommServiceService::UpdatedServiceInfo: bad parameters. csId = ";
		assert += str_csId;
		PASSERTMSG(1,assert.c_str());
		status = STATUS_INFO_MESSAGE_FAIL;
	}
	else
	{
        pCSMngrProcess->TestStringValidity(dataName[0], ParamLen, __PRETTY_FUNCTION__);

		status = pCSMngrProcess->GetIpServiceListDynamic()->UpdateDynamic(serviceInfo.m_Id, serviceInfo);
		if(STATUS_OK != status)
		{
			TRACEINTO	<< "CCommServiceService::UpdatedServiceInfo: unknown service : "
						<< serviceInfo.m_Id << ". csId = "<<m_csId;

			status = STATUS_INFO_MESSAGE_FAIL;
		}
		else
		{
			m_LastInfoType = eServiceInfo;
			m_LastInfoServiceId = serviceInfo.m_Id;
			status = STATUS_INFO_MESSAGE_OK;
		}
	}

	DEALLOCBUFFERS(keyName ,NumOfParams);
	DEALLOCBUFFERS(dataName,NumOfParams);

	return status;
}




//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::UpdatedIpInfo(cfgBufHndl &requestHandle)
{
	CCSIpInfo csIpInfo;

	const int NumOfParams	= 12;
	const int ParamLen		= 256;

	int   keyLen	[NumOfParams] = {ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,
									 ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen};
	int	  dataSize	[NumOfParams] = {ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,
									 ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen};

	ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
	ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

	DWORD serviceId 			= 0;
	DWORD ipAddress 			= 0;
	DWORD subnetMask 			= 0;
	DWORD defaultGW 			= 0;
	DWORD dhcpStatusType		= 0;
	DWORD dhcpServerIp 			= 0;
	DWORD dhcpServerListLenght 	= 0;
	DWORD natIp 				= 0;
	char buff[256];

	const DWORD numDnsIps = 3;
	DWORD dnsIps [numDnsIps] = {0, 0, 0};



	STATUS status = STATUS_INFO_MESSAGE_OK;

	int res = CfgGetParamsContinue(
		&requestHandle,
		NumOfParams,
		requestHandle.pCurBuf,
		keyName[0],	&keyLen[0], &serviceId,   			&dataSize[0]  ,	cfINT32,
		keyName[1],	&keyLen[1], &ipAddress,   			&dataSize[1]  ,	cfIpAddress,
		keyName[2],	&keyLen[2], &subnetMask,   			&dataSize[2]  ,	cfIpAddress,
		keyName[3],	&keyLen[3], &defaultGW,   			&dataSize[3]  ,	cfIpAddress,
		keyName[4],	&keyLen[4], &dhcpStatusType,   		&dataSize[4]  ,	cfINT32,
		keyName[5],	&keyLen[5], &dhcpServerIp,   		&dataSize[5]  ,	cfIpAddress,

		keyName[6],	&keyLen[6], &dhcpServerListLenght,  &dataSize[6]  ,	cfINT32,
		keyName[7],	&keyLen[7], &(dnsIps[0]),   		&dataSize[7]  ,	cfIpAddress,
		keyName[8],	&keyLen[8], &(dnsIps[1]),   		&dataSize[8]  ,	cfIpAddress,
		keyName[9],	&keyLen[9], &(dnsIps[2]),   		&dataSize[9]  ,	cfIpAddress,

		keyName[10],&keyLen[10], buff,					&dataSize[10] , cfString,
		keyName[11],&keyLen[11],&natIp,   				&dataSize[11] ,	cfIpAddress
		);

	CIpInfo &ipInfo = csIpInfo.GetIpInfo();

	ipInfo.SetIPv4Addr(ipAddress);
	ipInfo.SetSubneMask(subnetMask);
	ipInfo.SetDefaultGW(defaultGW);
	ipInfo.SetNatIp(natIp);
	ipInfo.SetDhcpServerIp(dhcpServerIp);
	ipInfo.SetDhcpStatusType(static_cast<eDHCPState>(dhcpStatusType));
	ipInfo.SetDomainName(buff);

	CDnsServerList &dnsServerList = ipInfo.GetDnsServerList();
	dnsServerList.SetLength(dhcpServerListLenght);
	dnsServerList.SetDnsIPv4s(dnsIps, numDnsIps);

	if (0 != res														||
		0 != strncmp(keyName[0], CS_KEY_SERVICE_ID			, keyLen[0])||
		0 != strncmp(keyName[1], CS_KEY_IP_ADDRESS			, keyLen[1])||
		0 != strncmp(keyName[2], CS_KEY_SUBNET_MASK			, keyLen[2])||
		0 != strncmp(keyName[3], CS_KEY_DEFAULT_GATEWAY		, keyLen[3])||
		0 != strncmp(keyName[4], CS_KEY_DHCP_STATE			, keyLen[4])||
		0 != strncmp(keyName[5], CS_KEY_DHCP_SERVER_IP		, keyLen[5])||
		0 != strncmp(keyName[6], CS_KEY_NUM_OF_DNS_SERVERS	, keyLen[6])||
		0 != strncmp(keyName[7], CS_KEY_DNS_SERVER_1		, keyLen[7])||
		0 != strncmp(keyName[8], CS_KEY_DNS_SERVER_2		, keyLen[8])||
		0 != strncmp(keyName[9], CS_KEY_DNS_SERVER_3		, keyLen[9])||
		0 != strncmp(keyName[10],CS_KEY_DOMAIN_NAME			, keyLen[10])||
		0 != strncmp(keyName[11],CS_KEY_NAT_IP              , keyLen[11]))
	{
		char str_csId[2];
		snprintf(str_csId, sizeof(str_csId), "%d",m_csId);
		
		std::string assert = "CCommServiceService::UpdatedIpInfo: bad parameters. csId = ";
		assert += str_csId;
		PASSERTMSG(1,assert.c_str());
		status = STATUS_INFO_MESSAGE_FAIL;
	}
	else if (dhcpServerListLenght > MAX_NUM_DNS)
	{
		status = STATUS_INFO_MESSAGE_FAIL;
	}
	else
	{
        pCSMngrProcess->TestStringValidity(dataName[10], ParamLen, __PRETTY_FUNCTION__);

		status = pCSMngrProcess->GetIpServiceListDynamic()->UpdateDynamic(serviceId, csIpInfo);
		if(STATUS_OK != status)
		{
			TRACEINTO	<< "CCommServiceService::UpdatedIpInfo: unknown service : "
						<< serviceId <<". csId = "<<m_csId;

			status = STATUS_INFO_MESSAGE_FAIL;
		}
		else
		{
			m_LastInfoType = eIpInfo;
			m_LastInfoServiceId = serviceId;
			status = STATUS_INFO_MESSAGE_OK;
		}
	}

	DEALLOCBUFFERS(keyName ,NumOfParams);
	DEALLOCBUFFERS(dataName,NumOfParams);

	return status;
}


//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::UpdatedSipInfo(cfgBufHndl &requestHandle)
{
	CSipInfo sipInfo;

	const int NumOfParams	= 6;
	const int ParamLen		= 256;

	int   keyLen	[NumOfParams] = {ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen};
	int	  dataSize	[NumOfParams] = {ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen};

	ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
	ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

	DWORD serviceId 	= 0;
	DWORD numProxy 		= 0;
	DWORD proxyIp 		= 0;
	DWORD altIp 		= 0;
	char namePrimaryProxy	[ParamLen];
	char nameAltProxy		[ParamLen];

	STATUS status = STATUS_INFO_MESSAGE_OK;

	int res = CfgGetParamsContinue(
		&requestHandle,
		  NumOfParams,
          requestHandle.pCurBuf,
		  keyName[0], &keyLen[0], &serviceId		,   &dataSize[0]	, cfINT32,
          keyName[1], &keyLen[1], &numProxy			,   &dataSize[1]	, cfINT32,
          keyName[2], &keyLen[2], &proxyIp			,   &dataSize[2]	, cfIpAddress,
          keyName[3], &keyLen[3], namePrimaryProxy	,	&dataSize[3]	, cfString,
          keyName[4], &keyLen[4], &altIp			,   &dataSize[4]	, cfIpAddress,
          keyName[5], &keyLen[5], nameAltProxy		,	&dataSize[5]	, cfString
		  );

	sipInfo.SetNumProxy(numProxy);

	CProxyDataContent &primaryProxy = sipInfo.GetPrimaryProxy();
	primaryProxy.SetName(namePrimaryProxy);
	primaryProxy.SetIPv4Address(proxyIp);

	CProxyDataContent &altProxy = sipInfo.GetAltProxy();
	altProxy.SetName(nameAltProxy);
	altProxy.SetIPv4Address(altIp);


	if (0 != res										||
		0 != strncmp(keyName[0], CS_KEY_SERVICE_ID		, keyLen[0])||
		0 != strncmp(keyName[1], CS_KEY_NUM_OF_PROXYS   , keyLen[1])||
		0 != strncmp(keyName[2], CS_KEY_SIP_PROXY_1_IP  , keyLen[2])||
		0 != strncmp(keyName[3], CS_KEY_SIP_PROXY_1_NAME, keyLen[3])||
		0 != strncmp(keyName[4], CS_KEY_SIP_PROXY_2_IP   , keyLen[4])||
		0 != strncmp(keyName[5], CS_KEY_SIP_PROXY_2_NAME , keyLen[5]))
	{
		char str_csId[2];
		snprintf(str_csId, sizeof(str_csId), "%d",m_csId);
		std::string assert = "CCommServiceService::UpdatedSipInfo: bad parameters. csId = ";
		assert += str_csId;
		PASSERTMSG(1,assert.c_str());
		status = STATUS_INFO_MESSAGE_FAIL;
	}
	else
	{
        pCSMngrProcess->TestStringValidity(dataName[3], ParamLen, __PRETTY_FUNCTION__);
        pCSMngrProcess->TestStringValidity(dataName[5], ParamLen, __PRETTY_FUNCTION__);

		status = pCSMngrProcess->GetIpServiceListDynamic()->UpdateDynamic(serviceId, sipInfo);
		if(STATUS_OK != status)
		{
			TRACEINTO	<< "CCommServiceService::UpdatedSipInfo: unknown service : "
						<< serviceId <<". csId = " << m_csId;

			status = STATUS_INFO_MESSAGE_FAIL;
		}
		else
		{
			m_LastInfoType = eSipInfo;
			m_LastInfoServiceId = serviceId;
			status = STATUS_INFO_MESSAGE_OK;
		}
	}

	DEALLOCBUFFERS(keyName ,NumOfParams);
	DEALLOCBUFFERS(dataName,NumOfParams);

	return status;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::UpdatedH323Info(cfgBufHndl &requestHandle)
{
	CH323Info h323Info;

	const int NumOfParams	= 6;
	const int ParamLen		= 256;

	int   keyLen	[NumOfParams] = {ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen};
	int	  dataSize	[NumOfParams] = {ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen};

	ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
	ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

	DWORD serviceId 	= 0;
	DWORD numGK 		= 0;
	DWORD primaryIp		= 0;
	DWORD altIp 		= 0;
	char namePrimaryGK	[ParamLen];
	char nameAltGK		[ParamLen];


	STATUS status = STATUS_INFO_MESSAGE_OK;

	int res = CfgGetParamsContinue(
		&requestHandle,
		  NumOfParams,
          requestHandle.pCurBuf,
		  keyName[0], &keyLen[0], &serviceId	,   &dataSize[0]	, cfINT32,
          keyName[1], &keyLen[1], &numGK		,   &dataSize[1]	, cfINT32,
          keyName[2], &keyLen[2], &primaryIp	,   &dataSize[2]	, cfIpAddress,
          keyName[3], &keyLen[3], namePrimaryGK	,	&dataSize[3]	, cfString,
          keyName[4], &keyLen[4], &altIp		,   &dataSize[4]	, cfIpAddress,
          keyName[5], &keyLen[5], nameAltGK		,	&dataSize[5]	, cfString
		  );

	h323Info.SetNumGk(numGK);

	CProxyDataContent &primaryGK = h323Info.GetPrimaryGk();
	primaryGK.SetIPv4Address(primaryIp);
	primaryGK.SetName(namePrimaryGK);

	CProxyDataContent &altGK = h323Info.GetAltGk();
	altGK.SetIPv4Address(altIp);
	altGK.SetName(nameAltGK);

	if (0 != res												||
		0 != strncmp(keyName[0], CS_KEY_SERVICE_ID	, keyLen[0])||
		0 != strncmp(keyName[1], CS_KEY_NUM_OF_GKS  , keyLen[1])||
		0 != strncmp(keyName[2], CS_KEY_GK_1_IP		, keyLen[2])||
		0 != strncmp(keyName[3], CS_KEY_GK_1_NAME	, keyLen[3])||
		0 != strncmp(keyName[4], CS_KEY_GK_2_IP		, keyLen[4])||
		0 != strncmp(keyName[5], CS_KEY_GK_2_NAME	, keyLen[5]))
	{
		char str_csId[2];
		snprintf(str_csId, sizeof(str_csId), "%d",m_csId);
		std::string assert = "CCommServiceService::UpdatedH323Info: bad parameters. csId = ";
		assert += str_csId;
		PASSERTMSG(1,assert.c_str());
		status = STATUS_INFO_MESSAGE_FAIL;
	}
	else
	{
        pCSMngrProcess->TestStringValidity(dataName[3], ParamLen, __PRETTY_FUNCTION__);
        pCSMngrProcess->TestStringValidity(dataName[5], ParamLen, __PRETTY_FUNCTION__);

		status = pCSMngrProcess->GetIpServiceListDynamic()->UpdateDynamic(serviceId, h323Info);
		if(STATUS_OK != status)
		{
			TRACEINTO	<< "CCommServiceService::UpdatedH323Info: unknown service : "
						<< serviceId << ". csId = "<<m_csId;

			status = STATUS_INFO_MESSAGE_FAIL;
		}
		else
		{
			m_LastInfoType = eH323Info;
			m_LastInfoServiceId = serviceId;
			status = STATUS_INFO_MESSAGE_OK;
		}
	}

	DEALLOCBUFFERS(keyName ,NumOfParams);
	DEALLOCBUFFERS(dataName,NumOfParams);

	return status;
}

//////////////////////////////////////////////////////////////////////
CIPService* CCommServiceService::GetService(const char * serviceName)
{
	CIPServiceList  *ipServiceList	= GetIpServiceList();
	int				index			= ipServiceList->FindService(serviceName);

	PASSERT_AND_RETURN_VALUE(index >= MAX_SERV_PROVIDERS_IN_LIST, NULL);

	CIPService *service	= (NOT_FIND != index ? ipServiceList->m_pH323Service[index] : NULL);

	return service;
}

//////////////////////////////////////////////////////////////////////
CIPService* CCommServiceService::GetService(int serviceId)
{
	CIPServiceList  *ipServiceList	= GetIpServiceList();
	int				index			= ipServiceList->FindService(serviceId);

	PASSERT_AND_RETURN_VALUE(index >= MAX_SERV_PROVIDERS_IN_LIST, NULL);

	CIPService *service	= (NOT_FIND != index ? ipServiceList->m_pH323Service[index] : NULL);

	return service;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommServiceService::SendPingToCs(const char* destination, const ePingIpType ipType)
{
    int reqLen = sizeof(CS_Ping_req_S);
    char * pPingRequestBuff = new char[reqLen];
    CS_Ping_req_S * pPingRequest = (CS_Ping_req_S *)pPingRequestBuff;
    pPingRequest->ipType = ipType;
    strncpy(pPingRequest->destination, destination, sizeof(pPingRequest->destination) - 1);
    pPingRequest->destination[sizeof(pPingRequest->destination) - 1] = '\0';
    
    STATUS status = CCommCsModuleService::SendToCsApi(CS_PING_REQ, eServiceMngr, reqLen, pPingRequestBuff, m_csId);

	delete[] pPingRequestBuff;

	return status;
}

//////////////////////////////////////////////////////////////////////
void CCommServiceService::SetCsId(WORD id)
{
	m_csId = id;
}

//////////////////////////////////////////////////////////////////////
