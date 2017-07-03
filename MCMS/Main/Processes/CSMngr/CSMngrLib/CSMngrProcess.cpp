// CSMngrProcess.cpp: implementation of the CCSMngrProcess class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner
//========   ==============   =====================================================================

#include <string>
#include <stdlib.h>
using namespace std;

#include "CSMngrProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "DynIPSProperties.h"
#include "IPServiceDynamicList.h"
#include "SIPProxyStructs.h"
#include "StatusesGeneral.h"
#include "DefinesGeneral.h"
#include "CSMngrStatuses.h"
#include "ApiStatuses.h"
#include "CsStructs.h"
#include "OpcodesMcmsCommon.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "TraceStream.h"
#include "CommCsModuleService.h"
#include "IpCsOpcodes.h"
#include "ConfigManagerApi.h"
#include "CommConfService.h"
#include "CommSnmpService.h"
#include "ICEApiDefinitions.h"
#include "MultipleServicesFunctions.h"
#include "SystemInterface.h"
#include "Request.h"
//for Gesher/Ninja
#include "ConfigHelper.h"

#define DEFAULT_NUM_OF_PARTY 0xFFFFFFFF

extern char* IpTypeToString(APIU32 ipType, bool caps = false);
extern bool	 IsJitcAndNetSeparation();
extern std::string GetIpServiceTmpFileName();

//////////////////////////////////////////////////////////////////////
extern void CSMngrManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CCSMngrProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CCSMngrProcess::GetManagerEntryPoint()
{
	return CSMngrManagerEntryPoint;
}

//////////////////////////////////////////////////////////////////////
CCSMngrProcess::CCSMngrProcess()
{
	m_XMLWraper					= new CIPServiceFullList;
	m_pIpServiceListDynamic		= new CIPServiceList(m_XMLWraper);
	m_pIpServiceListStatic 		= new CIPServiceList(m_XMLWraper);
	m_LicensingMaxNumOfParty 	= DEFAULT_NUM_OF_PARTY;

	m_sysIpType					= eIpType_IpV4;
	m_sysIPv6ConfigType			= eV6Configuration_Auto;
	m_MngmntAddress_IPv4		= 0;

	for(int i = 0; i < MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES ; ++i)
    {
    	m_MngmntAddress_IPv6[i] = "";
    }

	m_MngmntDefaultGatewayIPv6 = "";
	m_MngmntDefaultGatewayMaskIPv6 = 0;

    for (int i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
    {
    	m_pSignalingTasksMbxs_List[i] = NULL;
    	m_SignalingTasksState_List[i] = eSignalingTaskOk;
    }

    for (int i=0; i<= MAX_NUM_OF_IP_SERVICES; i++)
    {
    	m_MasterBoardIdPerService[i].boardId = 0;
        m_MasterBoardIdPerService[i].pqId = 0;
    }
    m_pCurrentPing = NULL;
    m_isMultipleServices = eMSNotConfigure;

    m_pSysInterfaceList = new CSystemInterfaceList;
    m_IpTypeReceivedStatus = FALSE;
    m_dnsStatus = FALSE;
}

//////////////////////////////////////////////////////////////////////
CCSMngrProcess::~CCSMngrProcess()
{
	POBJDELETE(m_XMLWraper);
	POBJDELETE(m_pIpServiceListDynamic);
	POBJDELETE(m_pIpServiceListStatic);
	POBJDELETE (m_pCurrentPing);
	POBJDELETE(m_pSysInterfaceList);
}

///////////////////////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::SetUpProcess()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::AddNewIpService(CIPService &ipService)
{
    FixPortRange(ipService);

	m_pIpServiceListStatic->Add(ipService, true);

//	FixPortRange(ipService);
	m_pIpServiceListDynamic->Add(ipService, false);
}

//////////////////////////////////////////////////////////////////////
void CCSMngrProcess::TraceToLogger(const char *location, const char *action, STATUS status)
{
	static CCSMngrProcess *process = (CCSMngrProcess*)CProcessBase::GetProcess();
	process->TraceMessage(location, action, status);
}

//////////////////////////////////////////////////////////////////////
void CCSMngrProcess::TraceMessage(const char *location, const char *action, STATUS status)
{
	if(STATUS_OK == status)
	{
		return;
	}

	string buff = "Location: ";
	buff += location;
	buff += "\n";
	buff += "Action	:";
	buff += action;
	buff += "\n";
	buff += "Status	:";
	buff += GetStatusAsString(status);

	PTRACE(eLevelInfoNormal, buff.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::AddExtraStringsToMap()
{
	CProcessBase::AddExtraStringsToMap();

	// GK -> CSMngr -> EMA

	for(eGKConnectionState en = eGKStateInvalid ; en < NumOfGKConnectionStates ; en = (eGKConnectionState)(en + 1))
	{
		CStringsMaps::AddItem(GK_CONNECTION_STATE_ENUM, en, GKConnectionStateNames[en]);
	}

	for(eGkFaultsAndServiceStatus en = eGkFaultsAndServiceStatusOk ; en < NumOfGkFaultsAndServiceStatus ; en = (eGkFaultsAndServiceStatus)(en + 1))
	{
		CStringsMaps::AddItem(GK_SERVICE_STATUS_ENUM, en, GkFaultsAndServiceStatusNames[en]);
	}

	for(eDynamicGkRole en = eGKActive ; en < NumOfGKRoles ; en = (eDynamicGkRole)(en + 1))
	{
		CStringsMaps::AddItem(GK_DYNAMIC_ROLE_ENUM, en, DynamicGkRoleNames[en]);
	}

	CStringsMaps::AddItem(IP_SERVICE_STATE_ENUM, eServiceStateConfig, "Configuration");
	CStringsMaps::AddItem(IP_SERVICE_STATE_ENUM, eServiceStateOk	, "Ok");
	CStringsMaps::AddItem(IP_SERVICE_STATE_ENUM, eServiceStateFailed, "Failed");

    CStringsMaps::AddItem(SERVICE_ROLE_ENUM, eServerTypeNotAvailable, "not_available");
    CStringsMaps::AddItem(SERVICE_ROLE_ENUM, eServerTypePrimary, "Primary_Server");
    CStringsMaps::AddItem(SERVICE_ROLE_ENUM, eServerTypeAlternate, "Alternate_Server");

    CStringsMaps::AddItem(SIP_SERVICE_STATUS_ENUM, eSipServerStatusNotAvailable, "not_available");
    CStringsMaps::AddItem(SIP_SERVICE_STATUS_ENUM, eSipServerStatusTypeOk, "ok");
    CStringsMaps::AddItem(SIP_SERVICE_STATUS_ENUM, eSipServerStatusTypeFail, "fail");


    //Ping
    CStringsMaps::AddItem(PING_STATUS_ENUM, ePingStatus_ok, "ok");
    CStringsMaps::AddItem(PING_STATUS_ENUM, ePingStatus_fail, "fail");

    CStringsMaps::AddItem(PING_IP_TYPE_ENUM, ePingIpType_IPv4, "IPv4");
    CStringsMaps::AddItem(PING_IP_TYPE_ENUM, ePingIpType_IPv6, "IPv6");

//	duplicated
//		CStringsMaps::AddMinMaxItem(NET_SERVICE_PROVIDER_NAME_LENGTH,0,NET_SERVICE_PROVIDER_NAME_LEN - 1);
	//ice
	CStringsMaps::AddItem(ICE_SERVER_ROLE_ENUM, eIceServerRoleStunPwdServer, "ice_server_role_stun_password_server");
	CStringsMaps::AddItem(ICE_SERVER_ROLE_ENUM, eIceServerRoleStunSrvUdp, "ice_server_role_stun_server_udp");
	CStringsMaps::AddItem(ICE_SERVER_ROLE_ENUM, eIceServerRoleStunSrvTcp, "ice_server_role_stun_server_tcp");
	CStringsMaps::AddItem(ICE_SERVER_ROLE_ENUM, eIceServerRoleRelaySrvUdp, "ice_server_role_relay_server_udp");
	CStringsMaps::AddItem(ICE_SERVER_ROLE_ENUM, eIceServerRoleRelaySrvTcp, "ice_server_role_relay_server_tcp");
	CStringsMaps::AddItem(ICE_SERVER_ROLE_ENUM, eIceServerRoleNotAvailable, "ice_server_role_not_available");

	CStringsMaps::AddItem(ICE_SERVER_STATUS_ENUM, eIceServerUnavailble, "ice_server_status_not_available");
	CStringsMaps::AddItem(ICE_SERVER_STATUS_ENUM, eIceInitOk, "ice_server_status_connection_ok");
	CStringsMaps::AddItem(ICE_SERVER_STATUS_ENUM, eIceInitServerFail, "ice_server_status_connection_failed");
	CStringsMaps::AddItem(ICE_SERVER_STATUS_ENUM, eIceStunPassServerAuthenticationFailure, "ice_server_status_stun_user_password_failed");
	CStringsMaps::AddItem(ICE_SERVER_STATUS_ENUM, eIceStunPassServerConnectionFailure, "ice_server_status_stun_password_connection_failure");
	CStringsMaps::AddItem(ICE_SERVER_STATUS_ENUM, eIceTurnServerDnsResolveFailure, "ice_turn_server_dns_resolve_failure");
	CStringsMaps::AddItem(ICE_SERVER_STATUS_ENUM, eIceTurnServerUnreachable, "ice_turn_server_unreachable");
	CStringsMaps::AddItem(ICE_SERVER_STATUS_ENUM, eIceTurnServerAuthorizationFailure, "ice_turn_server_authorization_failure");
	CStringsMaps::AddItem(ICE_SERVER_STATUS_ENUM, eIceUnknownProblem, "ice_server_status_unknown_failure");

	CStringsMaps::AddItem(ICE_FIREWALL_DETECTION_ENUM, eFwTypeUnknown, "ice_firewall_detection_unknown");
	CStringsMaps::AddItem(ICE_FIREWALL_DETECTION_ENUM, eFwTypeUdp, "ice_firewall_detection_udp_enabled");
	CStringsMaps::AddItem(ICE_FIREWALL_DETECTION_ENUM, eFwTypeTcpOnly, "ice_firewall_detection_tcp_enabled");
	CStringsMaps::AddItem(ICE_FIREWALL_DETECTION_ENUM, eFwTypeProxy, "ice_firewall_detection_tcp_proxy");
	CStringsMaps::AddItem(ICE_FIREWALL_DETECTION_ENUM, eFwTypeBlocked, "ice_firewall_detection_block");
	CStringsMaps::AddItem(ICE_FIREWALL_DETECTION_ENUM, eFwTypeNone, "ice_firewall_detection_none");
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::SetLicensingMaxNumOfParties(DWORD num)
{
	/*
	 * 12 Dec 2012, Rafi Fellert, JIRA issue #46
	 * For softMCUMfw m_LicensingMaxNumOfParty needs to be 1200. for other products it must be 800.
	 */
	//const eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

//	if (eProductTypeSoftMCUMfw == curProductType)
//	{
//		m_LicensingMaxNumOfParty = 1200;
//	}
//	else
//	{
		m_LicensingMaxNumOfParty = num;
//	}

	if (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
	{
		m_pIpServiceListDynamic->CalcMaxNumOfPorts(); //BRIDGE-7468
		m_pIpServiceListStatic->CalcMaxNumOfPorts(); //BRIDGE-11316
	}
    m_pIpServiceListDynamic->SetMaxNumOfParties(m_LicensingMaxNumOfParty);
    m_pIpServiceListStatic->SetMaxNumOfParties(m_LicensingMaxNumOfParty);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
bool CCSMngrProcess::IsLicensingReceived()const
{
	return DEFAULT_NUM_OF_PARTY != m_LicensingMaxNumOfParty;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::AddExtraOpcodesStrings()
{
//	AddOpcodeString(CSAPI_MSG, "CSAPI_MSG");
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::AddExtraStatusesStrings()
{
	AddStatusString(STATUS_END_CONFIG, "Configuration complete");
	AddStatusString(STATUS_END_COMMON, "STATUS_END_COMMON");
	AddStatusString(STATUS_SYSTEM_CONFIG_FILE_NOT_EXISTS, "System Configuration file not found");
	AddStatusString(STATUS_SECTION_NOT_FOUND, "STATUS_SECTION_NOT_FOUND");
	AddStatusString(STATUS_INFO_MESSAGE_OK, "STATUS_INFO_MESSAGE_OK");
	AddStatusString(STATUS_INFO_MESSAGE_FAIL, "STATUS_INFO_MESSAGE_FAIL");
	AddStatusString(STATUS_NO_DATA_COME_FROM_CS_MODULE, "No data received from signaling host module");
	AddStatusString(STATUS_CORRUPTED_IP_SERVICE, "IP Network Service is corrupt");

	AddStatusString(STATUS_FAILED_TO_SEND_MSG_TO_CS, "Failed to send message to signaling host module");
	AddStatusString(STATUS_FAILED_TO_SEND_CONFIG_PARAMS, "Failed to send configuration parameters");
	AddStatusString(STATUS_FAILED_TO_SEND_COMMON_PARAMS, "STATUS_FAILED_TO_SEND_COMMON_PARAMS");
	AddStatusString(STATUS_FAILED_TO_RECEIVE_MSG_FROM_CS, "Failed to receive message from signaling host module");
	AddStatusString(STATUS_FAILED_TO_UNPACK_MSG_FROM_CS, "STATUS_FAILED_TO_UNPACK_MSG_FROM_CS");
	AddStatusString(STATUS_INVALID_SECTION_NAME, "Invalid Section name");
	AddStatusString(STATUS_NO_SPANS_IN_SERVICE, "No Spans defined in Network Service");
	AddStatusString(STATUS_NO_NAT_IN_SERVICE, "No NAT defined in IP Network Service ");
	AddStatusString(STATUS_NO_DNS_IN_SERVICE, "No DNS defined in IP Network Service ");
	AddStatusString(STATUS_ILLEGAL_IP_PROTOCOL, "Invalid IP protocol");
	AddStatusString(STATUS_ILLEGAL_SERVICE_STATUS, "STATUS_ILLEGAL_SERVICE_STATUS");
	AddStatusString(STATUS_NOT_VALID_DHCP, "No DHCP defined in IP Network Service");
	AddStatusString(STATUS_ILLEGAL_DNS_STATUS, "STATUS_ILLEGAL_DNS_STATUS");
	AddStatusString(STATUS_NO_DOMAIN_NAME, " No domain name defined in IP Network Service ");
	AddStatusString(STATUS_NO_HOST_NAME, " No Host name defined in IP Network Service ");

	AddStatusString(STATUS_NO_AUT_IN_SERVICE, "STATUS_NO_AUT_IN_SERVICE");
	AddStatusString(STATUS_NO_AUT_ELEMENT_IN_SERVICE, "STATUS_NO_AUT_ELEMENT_IN_SERVICE");
	AddStatusString(STATUS_NO_SEQURITY_IN_SERVICE, "No Security parameters defined in IP Network Service ");
	AddStatusString(STATUS_NO_PROTOCOL_NAME, "Protocol name not defined ");
	AddStatusString(STATUS_NO_USER_NAME, "User name not found");
	AddStatusString(STATUS_NO_PASSWORD, "Password not found");
	AddStatusString(STATUS_NO_SERVER_NAME, "Service name not found");
	AddStatusString(STATUS_NO_DNS_PREFIX_NAME, "No DNS prefix name defined in IP Network Service");

	AddStatusString(STATUS_NO_SERVICE, "Service not found");
	AddStatusString(STATUS_NO_SIP_IN_SERVICE, "SIP not enabled in IP Network Service");
	AddStatusString(STATUS_PORT_NOT_READY, "STATUS_PORT_NOT_READY");
	AddStatusString(STATUS_NO_WEB_RTC_ICE_IN_SERVICE, "WebRTC ICE not enabled in IP Network Service");


}

// //////////////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrProcess::FixPortRange(CIPService &ipService)
{
	if(ipService.IsUserDefinePorts() || IsLicensingReceived())	//if the user defined fixed ports
	{
		ipService.CalculateMaxNumOfCalls();

        //VNGR-21109, the max number of calls should be the smaller one between maxNumOfParty and MaxNumOfCalls in ipService
        WORD maxNumOfCalls = (m_LicensingMaxNumOfParty < ipService.GetMaxNumOfCalls())? m_LicensingMaxNumOfParty: ipService.GetMaxNumOfCalls();
		WORD NumOfCalls = 0;

		if (ipService.IsUserDefinePorts())
		{
			CIPSpan *firstSpan = ipService.GetFirstSpan();

			//get defined ports from the user
			CCommH323PortRange *portRange = NULL;

			if (firstSpan)
			{
				portRange = firstSpan->GetPortRange();
				if (portRange)
					NumOfCalls = portRange->GetTcpNumberOfPorts()/2;

				if( eProductTypeSoftMCUMfw == GetProductType() )
				{
					WORD udpFirstPort = -1;
					if(portRange){
						portRange->GetUdpFirstPort();
					}

					if( udpFirstPort < MFW_UDP_FIRST_PORT_MIN || udpFirstPort > MFW_UDP_FIRST_PORT_MAX )
						return STATUS_ILLEGAL_FIRST_UDP_PORT_VALUE;
				}
			}


			if (NumOfCalls > maxNumOfCalls)		//TCP ports is double than IP Service max number of ports
            {
                ipService.DefineTcpPortRange(maxNumOfCalls);
				return STATUS_MAX_NUM_OF_PORTS_EXCEEDED;
            }


		}
		else
		{
			ipService.DefineTcpPortRange(maxNumOfCalls);
		}
	}
	//BRIDGE-13992  when its not fixed ports and we have no license we will show no error
	// and let user config ip service
	/*else
	{
		return STATUS_NO_NUM_OF_PORTS;
	}*/

	return STATUS_OK;
}

// //////////////////////////////////////////////////////////////////////////////////////
// bool CCSMngrProcess::FixTcpPortRange(CIPServiceList &ipServiceList)
// {
// 	CIPService *currentService = ipServiceList.GetFirstService();
// 	while(NULL != currentService)
// 	{
// 		FixPortRange(*currentService);

//     	currentService = ipServiceList.GetNextService();
// 	}

// 	return true;
// }


////////////////////////////////////////////////////////////////////////////
BOOL CCSMngrProcess::GetIsDebugMode() const
{
	BOOL isDebugMode = FALSE;
	GetSysConfig()->GetBOOLDataByKey(CFG_KEY_DEBUG_MODE, isDebugMode);
	return isDebugMode;
}


////////////////////////////////////////////////////////////////////////////
DWORD CCSMngrProcess::GetGkIp()const
{
    DWORD ip = 0;

    CIPServiceList *pServiceListDynamic = GetIpServiceListDynamic();
    if(NULL == pServiceListDynamic)
    {
        PASSERTMSG(TRUE, "No Dynamic service list");
        return ip;
    }

    CIPService *pNewIpServiceDynamic = pServiceListDynamic->GetFirstService();
    if(NULL == pNewIpServiceDynamic)
    {
        PASSERTMSG(TRUE, "No Dynamic service");
        return ip;
    }

    CDynIPSProperties *dynService = pNewIpServiceDynamic->GetDynamicProperties();
    if(NULL == dynService)
    {
        PASSERTMSG(TRUE, "No Dynamic properties in service");
        return ip;
    }

    CH323Info &gkInfo = dynService->GetCSGKInfo();
    CProxyDataContent &primaryGK = gkInfo.GetPrimaryGk();
    ip = primaryGK.GetIPv4Address();

    return ip;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCSMngrProcess::GetCsIp()const
{
    DWORD ip = 0;

    CIPServiceList *pServiceListDynamic = GetIpServiceListDynamic();
    if(NULL == pServiceListDynamic)
    {
        PASSERTMSG(TRUE, "No Dynamic service list");
        return ip;
    }

    CIPService *pNewIpServiceDynamic = pServiceListDynamic->GetFirstService();
    if(NULL == pNewIpServiceDynamic)
    {
        PASSERTMSG(TRUE, "No Dynamic service");
        return ip;
    }

    CIPSpan* pSpan = pNewIpServiceDynamic->GetSpanByIdx(0);
    if(NULL == pSpan)
    {
        PASSERTMSG(TRUE, "No CS span in the IP service");
        return ip;
    }

    ip = pSpan->GetIPv4Address();

    return ip;
}

///////////////////////////////////////////////////////////////////////
bool CCSMngrProcess::IsFailoverBlockTransaction_SlaveMode(string sAction)
{
	bool isBlocked = false;

	if ( ("NEW_IP_SERVICE"				== sAction) ||
		 ("DEL_IP_SERVICE"				== sAction) ||
		 ("UPDATE_IP_SERVICE"			== sAction) ||
		 ("SET_DEFAULT_H323_SERVICE"	== sAction) ||
		 ("SET_DEFAULT_SIP_SERVICE"		== sAction)  )
	{
		isBlocked = true;
	}

	return isBlocked;
}
////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::SetSysIpType(eIpType theType)
{
	m_sysIpType = theType;
}

////////////////////////////////////////////////////////////////////////////
eIpType	 CCSMngrProcess::GetSysIpType()
{
	return m_sysIpType;
}

////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::SetSysIPv6ConfigType(eV6ConfigurationType theType)
{
	m_sysIPv6ConfigType = theType;
}

////////////////////////////////////////////////////////////////////////////
eV6ConfigurationType CCSMngrProcess::GetSysIPv6ConfigType()
{
	return m_sysIPv6ConfigType;
}

////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::SetMngmntAddress_IPv4(DWORD theAddress)
{
	m_MngmntAddress_IPv4 = theAddress;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCSMngrProcess::GetMngmntAddress_IPv4()
{
	return m_MngmntAddress_IPv4;
}

////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::SetMngmntAddress_IPv6(int idx, string theAddress)
{
	if ( (0 <= idx) && (MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES > idx) )
	{
		m_MngmntAddress_IPv6[idx] = theAddress;
	}
}

////////////////////////////////////////////////////////////////////////////
string CCSMngrProcess::GetMngmntAddress_IPv6(int idx)
{
	string retStr = "";

	if ( (0 <= idx) && (MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES > idx) )
	{
		retStr = m_MngmntAddress_IPv6[idx];
	}

	return retStr;
}

/////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::SetMngmntDefaultGatewayIPv6(const string Address)
{
	m_MngmntDefaultGatewayIPv6 = Address;
}

/////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::SetMngmntDefaultGatewayMaskIPv6(const DWORD mask)
{
	m_MngmntDefaultGatewayMaskIPv6 = mask;
}

/////////////////////////////////////////////////////////////////////////////
const string CCSMngrProcess::GetMngmntDefaultGatewayIPv6()
{
	return m_MngmntDefaultGatewayIPv6;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CCSMngrProcess::GetMngmntDefaultGatewayMaskIPv6()
{
	return m_MngmntDefaultGatewayMaskIPv6;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrProcess::ValidateDuplicateSpanDefinition(CIPServiceList *pIpServiceList, CIPService *pNewService, CIPService *pOldService)
{
    CIPSpan* pSpan = pNewService->GetFirstSpan();	//for signaling - ignore the first span

    int pos = 0;

    if (pSpan)
    {
    	pSpan = pNewService->GetNextSpan();
    	pos++;

		while (pSpan)
		{
			DWORD ipV4 = pSpan->GetIPv4Address();
			if (ipV4 != 0)
			{
				if (pIpServiceList->IsOccupiedSpan(pos) == TRUE)	//if the span is already occupied
				{
					if (pOldService)//if it is update service
					{
						CIPSpan* pSpan = pOldService->GetSpanByIdx(pos);
						if(pSpan)
						{
							DWORD oldIpv4 = pSpan->GetIPv4Address();
							if (oldIpv4 == 0)
								return STATUS_PORT_IS_ALREADY_OCCUPIED_BY_ANOTHER_SERVICE;
						}
						else
							PASSERT_AND_RETURN_VALUE(pSpan==NULL, STATUS_FAIL);
					}
					else
						return STATUS_PORT_IS_ALREADY_OCCUPIED_BY_ANOTHER_SERVICE;
				}
			}

			pSpan = pNewService->GetNextSpan();
			pos++;
		}
    }
    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrProcess::ValidateDuplicateIpAddrCS(CIPService *pService, CObjString &errorMsg)
{
	eIpType					curIpType	= GetSysIpType();
	eV6ConfigurationType	curCfgType	= GetSysIPv6ConfigType();
	eProductType prodType = CProcessBase::GetProcess()->GetProductType();

	if ( (FALSE == IsTarget() && ( eProductTypeGesher != prodType ) && ( eProductTypeNinja != prodType ))	||
		 (NULL == pService)		||
		 ((eIpType_IpV6 == curIpType) && (eV6Configuration_Auto == curCfgType)) ||  // in Auto mode, Mngmnt and CS share the same address
		 (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator) )
	{
		return STATUS_OK;
	}


	string retStr		= "\nCCSMngrProcess::ValidateDuplicateIpAddrCS FAILED : ";
	string dplctdAdd	= "";

	// ===== 1. compare addresses
	CIPSpan *pSpan = pService->GetFirstSpan();
	if(NULL == pSpan)
	{
		char* ipType = ::IpTypeToString(curIpType);
		if(ipType)
			retStr = retStr + "No SPANs in ip service . At least one must be (ipType: " + ipType + ")";
		else
			retStr = retStr + "No SPANs in ip service . ipType is NULL";

		PTRACE( eLevelInfoNormal, retStr.c_str() );

		return STATUS_SERVICE_NO_SPAN;
	}

	//   ----- 1a. IPv4 address
	STATUS duplicateStat = STATUS_OK;

	if ( (eIpType_IpV4 == curIpType) || (eIpType_Both == curIpType) )
	{
		const DWORD csIpAddrV4	= pSpan->GetIPv4Address();
		const DWORD dMngmntIpV4	= GetMngmntAddress_IPv4();

		if(csIpAddrV4 == dMngmntIpV4)
		{
			duplicateStat = STATUS_FAIL;

			char iPv4Str[IP_ADDRESS_LEN];
			SystemDWORDToIpString(csIpAddrV4, iPv4Str);
			dplctdAdd = iPv4Str;
		}
	}

	//   ----- ab. IPv6 address
	if ( (STATUS_OK == duplicateStat)									&&
		 ((eIpType_IpV6 == curIpType) || (eIpType_Both == curIpType))	&&
		 (eV6Configuration_Auto != curCfgType) ) // in Auto mode, Mngmnt and CS share the same address
	{
		const string csIpAddrV6 = pSpan->GetIPv6Address(0);

		for (int i=0; i<MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES; i++)
		{
			if ( csIpAddrV6 == GetMngmntAddress_IPv6(i) && csIpAddrV6!="::")
			{
				duplicateStat = STATUS_FAIL;
				dplctdAdd = csIpAddrV6;

				break;
			}
		}
	}

	// ===== 2. report validation results
	if (STATUS_OK != duplicateStat)
	{
		retStr = retStr + "Duplicate IP (Central Signaling and Management Network Service) (ipType: " + ::IpTypeToString(curIpType) + ")";
		retStr = retStr + "\nDuplicated address: " + dplctdAdd;
//		errorMsg = "Duplicate IP (Central Signaling and Management Network Service)";

		PTRACE( eLevelInfoNormal, retStr.c_str() );
		return STATUS_SERVICE_DUPLICATE_IP_CS_MNGMNT;
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrProcess::ValidateV35GwAlreadyConfigured(string servName)
{
	   CIPServiceList *pIpServListStatic = GetIpServiceListStatic();
		if (!pIpServListStatic)
		{
			return STATUS_FAIL;
		}

		BOOL	curIsEnabled	= FALSE;
		string	curServName		= "";

		CIPService *pService = pIpServListStatic->GetFirstService();
		while (pService)
		{
			curIsEnabled	= pService->GetIsV35GwEnabled();
			curServName		= pService->GetName();

			if ( (TRUE == curIsEnabled) && (servName != curServName) ) // another service already configured V35GW
			{
				return STATUS_V35_GATEWAY_IS_ALREADY_CONFIGURED_IN_ANOTHER_SERVICE;
			}

			pService = pIpServListStatic->GetNextService();
		} // end while (pService)

	  return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrProcess::ValidateSwMcuFields(CIPService *pUpdatedService)
{
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrProcess::ValidateSwMcuFields";

	CIPServiceList *pIpServListStatic = GetIpServiceListStatic();
	if (!pIpServListStatic)
	{
		return STATUS_FAIL;
	}

	BOOL	curIsEnabled	= FALSE;
	string	curServName		= "";

	CIPService *pService = pIpServListStatic->GetFirstService();
	while (pService)
	{
		if (strcmp(pUpdatedService->GetName(),pService->GetName())==0)
		{
			STATUS status = ValidateRouterDetails(pService, pUpdatedService);

			if (status!=STATUS_OK)
				return status;

			status = ValidateDnsDetails(pService, pUpdatedService);
			if (status!=STATUS_OK)
				return status;

			status = ValidateSubnetMaskDetails(pService->GetNetMask(), pUpdatedService->GetNetMask());
			if (status!=STATUS_OK)
				return status;

			status = ValidateV35Details(pService, pUpdatedService);
			if (status!=STATUS_OK)
				return status;

			CIPSpan* ip_span = pService->GetFirstSpan();
			CIPSpan* updated_ip_span = pUpdatedService->GetFirstSpan();

            string strCsInterface = updated_ip_span->GetInterface();

			//check that the CS ip addresses and interface configured exist in the system
			status = ValidateIpAddressAndInterface(ip_span->GetIPv4Address(), updated_ip_span->GetIPv4Address(),
					ip_span->GetInterface(), updated_ip_span->GetInterface());
            
			if (status!=STATUS_OK)
				return status;

			//check that the Media ip addresses and interface configured exist in the system
			ip_span = pService->GetNextSpan();
			updated_ip_span = pUpdatedService->GetNextSpan();

            //EdgeAxis
            string strMdInterface = updated_ip_span->GetInterface();
            eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
            if (curProductType == eProductTypeEdgeAxis && strCsInterface != strMdInterface)
            {
                status = STATUS_SIGNALING_AND_MEDIA_MUST_BE_SAME_IN_EDGE;
      	        TRACESTR(eLevelInfoNormal) << "CCSMngrProcess::ValidateSwMcuFields Signaling and Media interface must be same in Edge. CsInterface:" << strCsInterface << " MdInterface:" << strMdInterface;  
                return status;
            }
        
			while (updated_ip_span && ip_span)
			{
				status = ValidateIpAddressAndInterface(ip_span->GetIPv4Address(), updated_ip_span->GetIPv4Address(),
						ip_span->GetInterface(), updated_ip_span->GetInterface());

				if (status!=STATUS_OK)
					return status;

				ip_span = pService->GetNextSpan();
				updated_ip_span = pUpdatedService->GetNextSpan();
			}



			return STATUS_OK;
		}

		pService = pIpServListStatic->GetNextService();
	} // end while (pService)

	return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::SetMFWIpServiceExtDiscription(STATUS nStatus, std::string strServiceName, CRequest *pRequest, CIPSpan *pIpSpan)
{
	std::string strExtDiscription = "The configuration of the " + strServiceName + " service is invalid. Interface " + pIpSpan->GetInterface();

		switch(nStatus)
		{
		case STATUS_IP_ADDRESS_MISMATCHES_SYSTEM:
			strExtDiscription += " with address IpV4:" + m_pSysInterfaceList->GetValidatedResult() + " does not exist or is invalid.";
			break;
		case STATUS_MUST_ASSIGN_INTERFACE_IN_MFW:
			strExtDiscription += " does not exist or is invalid.";
			break;
		case STATUS_IPV6_GLOBAL_ADDRESS_MISMATCHES_SYSTEM:
		case STATUS_IPV6_SITE_ADDRESS_MISMATCHES_SYSTEM:
		case STATUS_IPV6_LINK_ADDRESS_MISMATCHES_SYSTEM:
			strExtDiscription += "with address Ipv6: " + m_pSysInterfaceList->GetValidatedResult() + " does not exist or is invalid.";
			break;
		default:
			strExtDiscription = "";
			break;
		}
		pRequest->SetExDescription(strExtDiscription.c_str());
}

/////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrProcess::ValidateMFWFields(CIPService* pUpdateService, CRequest* pRequest)
{
	TRACESTR(eLevelInfoNormal)<< "CCSMngrProcess::ValidateMFWFields";
	CIPServiceList *pIpServListStatic = GetIpServiceListStatic();
	if (!pIpServListStatic)
	{
		return STATUS_FAIL;
	}

	CIPService *pService = pIpServListStatic->GetFirstService();
	while (pService)
	{
		if (strcmp(pUpdateService->GetName(),pService->GetName())==0)
		{
			STATUS status = ValidateRouterDetails(pService, pUpdateService);

			if (status!=STATUS_OK)
				return status;

			status = ValidateDnsDetails(pService, pUpdateService);
			if (status!=STATUS_OK)
				return status;

			status = ValidateV35Details(pService, pUpdateService);
			if (status!=STATUS_OK)
				return status;
			eIpType ipType = pUpdateService->GetIpType();
			CIPSpan* updated_ip_span = pUpdateService->GetFirstSpan();

			//check that the CS ip addresses and interface configured exist in the system
			status = m_pSysInterfaceList->ValidateMFWSPANField(updated_ip_span, ipType);
			if (status!=STATUS_OK)
			{
				SetMFWIpServiceExtDiscription(status, "Signal", pRequest, updated_ip_span);
				return status;
			}
			//check that the Media ip addresses and interface configured exist in the system
			updated_ip_span = pUpdateService->GetNextSpan();
            PASSERT_AND_RETURN_VALUE(NULL == updated_ip_span, STATUS_FAIL);
			status = m_pSysInterfaceList->ValidateMFWSPANField(updated_ip_span, ipType);
			if (status!=STATUS_OK)
			{
				SetMFWIpServiceExtDiscription(status, "Media", pRequest, updated_ip_span);
				return status;
			}

			return STATUS_OK;
		}

		pService = pIpServListStatic->GetNextService();
	} // end while (pService)
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrProcess::ValidateV35Details(CIPService *pService, CIPService *pUpdatedService)
{
	if (pService->GetIsV35GwEnabled() != pUpdatedService->GetIsV35GwEnabled())
		return STATUS_V35_CANT_BE_CHANGED_IN_SMCU;

	if (pService->GetV35GwIpAddress() != pUpdatedService->GetV35GwIpAddress())
		return STATUS_V35_CANT_BE_CHANGED_IN_SMCU;

	if (pService->GetV35GwUsername() != pUpdatedService->GetV35GwUsername())
		return STATUS_V35_CANT_BE_CHANGED_IN_SMCU;

	if (pService->GetV35GwPassword_dec() != pUpdatedService->GetV35GwPassword_dec())
		return STATUS_V35_CANT_BE_CHANGED_IN_SMCU;

	if (pService->GetV35GwPassword_enc() != pUpdatedService->GetV35GwPassword_enc())
		return STATUS_V35_CANT_BE_CHANGED_IN_SMCU;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrProcess::ValidateRouterDetails(CIPService *pService, CIPService *pUpdatedService)
{
	//check default GW
	if (pService->GetDefaultGatewayIPv4()!=pUpdatedService->GetDefaultGatewayIPv4())
		return STATUS_ROUTER_CANT_BE_CHANGED_IN_SMCU;

	if (pService->GetRoutersNumber()!=pUpdatedService->GetRoutersNumber())
		return STATUS_ROUTER_CANT_BE_CHANGED_IN_SMCU;

	//check routers list
	CH323Router *router = pService->GetFirstRouter();
	CH323Router *pUpdatedRouter = pUpdatedService->GetFirstRouter();

	while(NULL != router && pUpdatedRouter != NULL)
	{
		if (*router != *pUpdatedRouter)
			return STATUS_ROUTER_CANT_BE_CHANGED_IN_SMCU;

		router = pService->GetNextRouter();
		pUpdatedRouter = pUpdatedService->GetNextRouter();
	}

	//they both suppose to be NULL when the while loop is over
	if (router!=NULL || pUpdatedRouter!=NULL)
		return STATUS_ROUTER_CANT_BE_CHANGED_IN_SMCU;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrProcess::ValidateDnsDetails(CIPService *pService, CIPService *pUpdatedService)
{
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	TRACESTR(eLevelInfoNormal) << "\nCCSMngrProcess::ValidateDnsDetails: "
                           << " old: " << (pService->GetpDns())->GetHostServiceName()
	                       << " new: " << (pUpdatedService->GetpDns())->GetHostServiceName();
	//check DNS
	if (*(pService->GetpDns())!=*(pUpdatedService->GetpDns()) && curProductType != eProductTypeEdgeAxis)
		return STATUS_DNS_CANT_BE_CHANGED_IN_SMCU;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrProcess::ValidateSubnetMaskDetails(DWORD netMask, DWORD updatedNetmask)
{
	//check Subnet mask
	if (netMask!=updatedNetmask)
		return STATUS_SUBNET_MASK_CANT_BE_CHANGED_IN_SMCU;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrProcess::ValidateIpAddressAndInterface(DWORD ipv4Address, DWORD UpdatedIpv4Address, string interface, string UpdatedInterface)
{
	//check that the CS ip addresses and interface configured exist in the system
	if (ipv4Address!=UpdatedIpv4Address || interface!=UpdatedInterface)
	{
		//the ip address and interface were removed
		if (UpdatedIpv4Address==0 && UpdatedInterface=="")
			return STATUS_OK;

		//read the interface ip address
		string ip_address;
		string cmd = "echo -n `/sbin/ifconfig | grep -A1 ";
		cmd += UpdatedInterface.c_str();
		cmd += " | grep 'inet addr' | cut -d':' -f2 | cut -d' ' -f1`";
		SystemPipedCommand(cmd.c_str(), ip_address);

		if (ip_address=="")	//if no interface defined on this system
			return STATUS_INTERFACE_IS_NOT_CONFIGURED_IN_THE_SYSTEM;

		DWORD dword_ipAddress = 0;
		dword_ipAddress = SystemIpStringToDWORD(ip_address.c_str());
		if (dword_ipAddress != UpdatedIpv4Address)
			return STATUS_IP_ADDRESS_IS_DIFFERENT_FROM_THE_ONE_CONFIGURED_IN_THE_SYSTEM;
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::SetPing (CPingData* newPingData)
{
    m_pCurrentPing = newPingData;
}

/////////////////////////////////////////////////////////////////////////////
CPingData * CCSMngrProcess::GetPing ()
{
	return m_pCurrentPing;
}

/////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::DeletePing()
{
	POBJDELETE(m_pCurrentPing);
}

/////////////////////////////////////////////////////////////////////////////
// static
WORD CCSMngrProcess::GetMaxNumOfCSTasks(void)
{
/*	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	int max_num_of_services = MAX_NUMBER_OF_SERVICES_IN_RMX_2000_AND_1500;
	if (eProductTypeRMX4000 == curProductType)
		max_num_of_services = MAX_NUMBER_OF_SERVICES_IN_RMX_4000;

	return max_num_of_services;*/
	//judith - at the moment, return the maximum CS's in Rmx4000

	return MAX_NUMBER_OF_SERVICES_IN_RMX_4000;
}

/////////////////////////////////////////////////////////////////////////////
// static
WORD CCSMngrProcess::CheckAndFixCSID(WORD csID, OPCODE opcode)
{
    CProcessBase* process = CProcessBase::GetProcess();
    FPASSERT_AND_RETURN_VALUE(!process, 1);

    // return CD_ID=1 in a case of assertion
    FPASSERTSTREAM_AND_RETURN_VALUE(
        0 == csID || csID > GetMaxNumOfCSTasks(),
        "Arrived illegal CS ID: " << csID
            << ", Opcode: " << process->GetOpcodeAsString(opcode)
            << " (" << opcode << ")",
        1);

    return csID;
}

/////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::AddToSignalingTasksList(COsQueue* mbx, WORD index)
{
    PASSERTSTREAM_AND_RETURN(
         GetMaxNumOfCSTasks() <= index,
        "Unable to add signaling task: illegal index: " << index);

	m_pSignalingTasksMbxs_List[index] = mbx;
	m_SignalingTasksState_List[index] = eSignalingTaskOk;

	TRACESTR(eLevelInfoNormal) << "\nCCardsProcess::AddToSignalingTasksList: "
                           << m_pSignalingTasksMbxs_List[index]
	                       << "\ncsId " << index;
}

//////////////////////////////////////////////////////////////////////
/*void CCSMngrProcess::RemoveFromSignalingTasksList(WORD csId)
{
	if ( GetMaxNumOfCSTasks() <= csId )
	{
		char buff[TEN_LINE_BUFFER_LEN];
		sprintf(buff, "CCSMngrProcess::RemoveFromSignalingTasksList - Illegal: csId: %d", csId);
		PASSERTMSG(1, buff);
	}

	TRACESTR(eLevelInfoNormal) << "\nCCSMngrProcess::RemoveFromSignalingTasksList: " << m_pSignalingTasksMbxs_List[csId]
	                       << "\n(csId " << csId;
    TurnSignalingTaskToZombie(m_pSignalingTasksMbxs_List[csId]);
	m_pSignalingTasksMbxs_List[csId] = NULL;
	m_SignalingTasksState_List[csId] = eSignalingTaskOk;

}*/
//////////////////////////////////////////////////////////////////////
void CCSMngrProcess::TurnSignalingTaskToZombie(COsQueue* mbx)
{
	if (NULL == mbx)
	{
		TRACESTR(eLevelInfoNormal)<< "\nCCSMngrProcess::TurnSignalingTaskToZombie: no mbx";
		return;
	}

	BOOL isFound = NO;

	int i=0;
	for (i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
	{
		if ( (mbx != NULL) && (mbx == m_pSignalingTasksMbxs_List[i]) )
		{
			isFound=YES;
			m_SignalingTasksState_List[i] = eSignalingTaskZombie;

				TRACESTR(eLevelInfoNormal)<< "\nCCSMngrProcess::TurnSignalingTaskToZombie"
				                      << "\nMbx for csId " << i << " becomes Zombie";
		}
	}

	if (NO == isFound)
		TRACESTR(eLevelInfoNormal)<< "\nCCSMngrProcess::TurnSignalingTaskToZombie: no mbx found";
}
//////////////////////////////////////////////////////////////////////
/*void CCSMngrProcess::RemoveFromSignalingTasksList(COsQueue* mbx)
{
	if (NULL == mbx)
	{
		TRACESTR(eLevelInfoNormal)<< "\nCCSMngrProcess::RemoveFromSignalingTasksList: no mbx";
		return;
	}

	BOOL isFound = NO;

	int i=0;
	for (i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
	{
		if ( (mbx != NULL) && (mbx == m_pSignalingTasksMbxs_List[i]) )
		{
			isFound=YES;
			m_pSignalingTasksMbxs_List[i] = NULL;
			m_SignalingTasksState_List[i] = eSignalingTaskOk;

	 		TRACESTR(eLevelInfoNormal)<< "\nCCSMngrProcess::RemoveFromSignalingTasksList"
			                      << "\nMbx for csId " << i << " is deleted from SignalingTasksMbxs_List";

		} // end record found

		if (YES == isFound)
			break;
	} // end loop (i)

	if (NO == isFound)
	{
		TRACESTR(eLevelInfoNormal)<< "\nCCSMngrProcess::RemoveFromSignalingTasksList: no mbx found";
	}
}*/
//////////////////////////////////////////////////////////////////////
COsQueue* CCSMngrProcess::GetSignalingMbx(WORD index)
{
    PASSERTSTREAM_AND_RETURN_VALUE(
        GetMaxNumOfCSTasks() <= index,
        "Unable to get signaling task mailbox: illegal index: " << index,
        NULL);

	return m_pSignalingTasksMbxs_List[index];
}
//////////////////////////////////////////////////////////////////////
eSignalingTaskStateType CCSMngrProcess::GetSignalingTaskState(WORD index)
{
    PASSERTSTREAM_AND_RETURN_VALUE(
        GetMaxNumOfCSTasks() <= index,
        "Unable to get signaling task state: illegal index: " << index,
        NumOfSignalingTaskTypes);

	return m_SignalingTasksState_List[index];
}

//////////////////////////////////////////////////////////////////////
void CCSMngrProcess::RetrieveIPv6AddressesInAutoMode(eIpType ipType)
{
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrProcess::RetrieveIPv6AddressesInAutoMode  Begin ...";

    // ===== 1. retrieve the address
    std::string retStr_ipV6;
    IpV6AddressMaskS ipV6_S[MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES];

	eConfigInterfaceType ifType = eSignalingNetwork;
	if(IsJitcAndNetSeparation())
		ifType = eSeparatedSignalingNetwork;
	//TODO
	else if(m_isMultipleServices == eTRUE)
		ifType = eSeparatedSignalingNetwork_1_1; // eth0.2012 //GetSignalingNetwork();

	//TODO - Judith. Run on all the VLAN and for each one, get the IPV6 and update it in the right service
	std::string ifName = GetDeviceName(ifType);

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if ((eProductTypeGesher == curProductType) || (eProductTypeNinja == curProductType))
	{		
		ifName = GetLogicalInterfaceName(ifType, ipType);
	}
    else if (eProductTypeSoftMCUMfw == curProductType || eProductTypeEdgeAxis == curProductType)
    {
        CIPServiceList *pIpServListStatic = GetIpServiceListStatic();
        pIpServListStatic->ReadXmlFile(IP_SERVICE_LIST_PATH, eNoActiveAlarm,
			eRenameFile);
        CIPService* pIpService1 = pIpServListStatic->GetFirstService();
        CIPSpan* pSpan1 = pIpService1->GetFirstSpan();
        CIPSpan* pSpan2 = pIpService1->GetNextSpan();
        PASSERT_AND_RETURN(NULL == pSpan2);
        TRACESTR(eLevelInfoNormal) << "CCSMngrManager::RetrieveIPv6AddressesInAutoMode, no need retrieve, "
            << " CS_ipv4: " << pSpan1->GetIPv4Address()
            << " CS_ipv6: " << pSpan1->GetFullIPv6Address(0)
            << " Media_ipv4: " << pSpan2->GetIPv4Address()
            << " Media_ipv6: " << pSpan2->GetFullIPv6Address(0);

        return;
    }

	RetrieveIpAddressConfigured_IpV6(ipV6_S, retStr_ipV6, ifName);
	TRACEINTO << "ifName:     " << ifName.c_str()
		      << "\nifType:     " << ipType
	          << "\nRetStrIPv6: " << retStr_ipV6.c_str()
	          << "\nipV6_S[0].address: " << ipV6_S[0].address
	          << "\nipV6_S[0].mask:    " << ipV6_S[0].mask
			  << "\nipV6_S[1].address: " << ipV6_S[1].address
	          << "\nipV6_S[1].mask:    " << ipV6_S[1].mask
			  << "\nipV6_S[2].address: " << ipV6_S[2].address
	          << "\nipV6_S[2].mask:    " << ipV6_S[2].mask;

	// ===== 2. update the services
	CIPServiceList *pIpServListStatic	= GetIpServiceListStatic();
	CIPServiceList *pIpServListDynamic	= GetIpServiceListDynamic();

	CIPService *pService = pIpServListStatic->GetFirstService();
	
	if (pService)
	{
		bool isJitcAndNetSeparation = ::IsJitcAndNetSeparation();
		
		while (pService)
		{
			// BRIDGE-10731: RMX IPv6 router address for signaling was empty 
			// when it was set to 'auto' configure IPv6 address
			// Update IPv6 route address for RMX2000 in case of JitcAndNetSeparation plus Auto
			if (isJitcAndNetSeparation &&
				(pService->GetIpV6ConfigurationType() == eV6Configuration_Auto))
			{
				string ipv6_route;
				string ipv6_defGw_multiple;
				char ipv6_defGw_single[IPV6_ADDRESS_LEN];

				// trace current ipv6 route
				SystemPipedCommand("/sbin/route -A inet6", ipv6_route);
				TRACEINTO << ipv6_route;

				//Read Signaling default gw
				SystemPipedCommand("/sbin/route -A inet6 | grep eth0.2198 | grep U | grep G | awk '{ print $2 }'", ipv6_defGw_multiple);

				int lenToCopy = ipv6_defGw_multiple.length();

				int singleAddressLen = 0;
				const char *pDefGwStart	= ipv6_defGw_multiple.c_str();
				const char *pDefGwEndl	= strchr(ipv6_defGw_multiple.c_str(), '\n'); // search for '\n'

				if (pDefGwEndl)
				{
					singleAddressLen = pDefGwEndl - pDefGwStart; // num of characters before the '\n'

					if ((0 <= singleAddressLen) && (singleAddressLen <= lenToCopy))
					{
						lenToCopy = singleAddressLen;
					}
				}

				if (lenToCopy >= IPV6_ADDRESS_LEN)
				{
					lenToCopy = IPV6_ADDRESS_LEN - 1;
				}

				memset(ipv6_defGw_single, 0, IPV6_ADDRESS_LEN);
				strncpy(ipv6_defGw_single, ipv6_defGw_multiple.c_str(), lenToCopy); // copy only a single address
				ipv6_defGw_single[IPV6_ADDRESS_LEN - 1] = '\0';

				TRACEINTO << "etho.2198 default gw: " << ipv6_defGw_single;

				pService->SetDefaultGatewayIPv6(ipv6_defGw_single);						
			}
			
			if (!pService->GetIsV35GwEnabled())
			{
				CIPSpan *pSpan = pService->GetFirstSpan();
				
				if (pSpan)
				{
					pSpan->SetIPv6Address(0, ipV6_S[0].address); // set the address as the 1st IPv6 address of the Signaling
					pSpan->SetIPv6SubnetMask(0, ipV6_S[0].mask);

					if ((eProductTypeGesher == curProductType) || (eProductTypeNinja == curProductType))
					{
						if ('\0' != ipV6_S[1].address[0])
						{
							
							pSpan->SetIPv6Address(1, ipV6_S[1].address); // set the address as the 1st IPv6 address of the Signaling
							pSpan->SetIPv6SubnetMask(1, ipV6_S[1].mask);
						}

						if ('\0' != ipV6_S[2].address[0])
						{
							
							pSpan->SetIPv6Address(2, ipV6_S[2].address); // set the address as the 1st IPv6 address of the Signaling
							pSpan->SetIPv6SubnetMask(2, ipV6_S[2].mask);
						}
					}
					
					if (eProductTypeSoftMCU == curProductType)
					{
						//in case of SMCU we need to update the second span as well. ResourceManager looks at the second span.
						pSpan=pService->GetNextSpan();
						
						if(pSpan)
						{
							pSpan->SetIPv6Address(0, ipV6_S[0].address); // set the address as the 1st IPv6 address of the Signaling
							pSpan->SetIPv6SubnetMask(0, ipV6_S[0].mask);
						}
					}
				}
			}
			
			pService = pIpServListStatic->GetNextService();
		} // end if service exists

		std::string ipServiceFileName, ipServiceFileNameTmp;

		GetIpServiceFileNames(ipServiceFileName, ipServiceFileNameTmp);

		pIpServListStatic->UpdateCounters();
		pIpServListStatic->WriteXmlFile(ipServiceFileNameTmp.c_str());

		*pIpServListDynamic = *pIpServListStatic;
		pIpServListDynamic->UpdateCounters();
		pIpServListDynamic->WriteXmlFile(ipServiceFileName.c_str());
	}

	TRACESTR(eLevelInfoNormal) << "\nCCSMngrProcess::RetrieveIPv6AddressesInAutoMode  Done ...";
}

//////////////////////////////////////////////////////////////////////
void CCSMngrProcess::SetIsMultipleServices(eCsMultipleServiceMode isMultipleServices)
{
	m_isMultipleServices = isMultipleServices;
}

//////////////////////////////////////////////////////////////////////
eCsMultipleServiceMode CCSMngrProcess::GetIsMultipleServices()
{
	return m_isMultipleServices;
}

//////////////////////////////////////////////////////////////////////
BYTE CCSMngrProcess::GetIsV35JITCSupport()
{
	BOOL isV35JitcSupport = NO;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, isV35JitcSupport);
	return isV35JitcSupport;
}
//////////////////////////////////////////////////////////////////////
void CCSMngrProcess::GetSignalingMasterBoardId(const DWORD csId, DWORD& board_id, DWORD& sub_board_id)
{
	CIPService*  pService = m_pIpServiceListStatic->GetService(csId);
	if(IsValidPObjectPtr(pService))
	{
		TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::GetSignalingMasterBoardId service name: " << pService->GetName();

		eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

		WORD index = 1;
		CIPSpan* pSpan = pService->GetFirstSpan();
		pSpan = pService->GetNextSpan();
		while(pSpan)
		{
			//TODO - Add test if span's card is 'alive'
			if(/*pSpan->GetIsSpanEnable() && */(0 != pSpan->GetIPv4Address()))
			{
				TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::GetSignalingMasterBoardId - found the ip, ip=" << pSpan->GetIPv4Address();
				break;
			}
			//if span is not valid
			else
			{
				index++;
				pSpan = pService->GetNextSpan();
			}
		}

		if(!pSpan)
		{
			TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::GetSignalingMasterBoardId - span wasn't found!!";
			index = 0;
		}


		//index 2,4,6,8 ==> board_id=1,2,3,4 ; sub_board_id=2
		//index 1,3,5,7 ==> board_id=1,2,3,4 ; sub_board_id=1
		div_t div_result = div(index, 2);
		board_id = div_result.quot + div_result.rem;
		if(div_result.rem == 0)

			sub_board_id = 2;
		else
			sub_board_id = 1;



		TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::GetSignalingMasterBoardId board_id = " << board_id << " and sub_board_id = " << sub_board_id;
	}
	else
		PASSERTMSG(pService==NULL, "CCSMngrManager::GetSignalingMasterBoardId - service not found");
}

//////////////////////////////////////////////////////////////////////
void CCSMngrProcess::SetIpv6Params(DWORD ipv4Add,
									string ipv6Add_0,
									string ipv6Add_1,
									string ipv6Add_2,
									string ipv6_defGw,
									DWORD ipv6Mask_0,
									DWORD ipv6Mask_1,
									DWORD ipv6Mask_2,
									DWORD ipv6Mask_defGw,
									CCommConfService *pCommConfService,
									CCommSnmpService *pCommSnmpService,
									BOOL bForceDefGwUpdate)
{

	// ===== 1. update static list
	CIPService *pService = m_pIpServiceListStatic->GetFirstService();
	while (pService)
	{
		pService->SetIpV6Params( m_sysIpType, m_sysIPv6ConfigType,
								 ipv6Add_0, ipv6Add_1, ipv6Add_2, ipv6_defGw,
								 ipv6Mask_0, ipv6Mask_1, ipv6Mask_2, ipv6Mask_defGw, bForceDefGwUpdate);

		m_pIpServiceListStatic->UpdateCounters();
		m_pIpServiceListStatic->WriteXmlFile(GetIpServiceTmpFileName().c_str());

		pService = m_pIpServiceListStatic->GetNextService();
	}


	// ===== 2. update dynamic list
	CIPService *pServiceD = m_pIpServiceListDynamic->GetFirstService();
	while (pServiceD)
	{
		pServiceD->SetIpV6Params( m_sysIpType, m_sysIPv6ConfigType,
								  ipv6Add_0, ipv6Add_1, ipv6Add_2, ipv6_defGw,
								  ipv6Mask_0, ipv6Mask_1, ipv6Mask_2, ipv6Mask_defGw, bForceDefGwUpdate);

		m_pIpServiceListDynamic->UpdateCounters();

		// ===== 3. in IPv6, the addresses might be changed afterwards; other processes should then be also notified
		if ( ((eIpType_IpV6 == m_sysIpType) || (eIpType_Both == m_sysIpType))
 			// && (eV6Configuration_Auto == pFirstService->GetIpV6ConfigurationType())
 			  )
		{
			STATUS status = STATUS_OK;

			status = pCommConfService->SendIpServiceParamInd(pServiceD);
			CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "sending params to ConfParty", status);

			status = pCommSnmpService->SendIpServiceParamInd(pServiceD);
			CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "sending params to SNMP", status);
		}

		pServiceD = m_pIpServiceListDynamic->GetNextService();
	}
}

////////////////////////////////////////////////////////////////////////////////////
eConfigInterfaceType CCSMngrProcess::GetSignalingInterfaceType(const DWORD csId)
{
	BOOL isV35Service = FALSE;
	CIPService*  pService = m_pIpServiceListStatic->GetService(csId);
	//BOOL isLanRedundancy = GetSystemCfgFlagInt<BOOL>(CFG_KEY_LAN_REDUNDANCY);
	BOOL isLanRedundancy = CProcessBase::GetProcess()->GetLanRedundancy();
	eProductType curProductType=CProcessBase::GetProcess()->GetProductType();

	if(IsValidPObjectPtr(pService))
		isV35Service = pService->GetIsV35GwEnabled();


	BOOL isMultipleServices = FALSE;
	if ( GetIsMultipleServices() == eTRUE )
		isMultipleServices = TRUE;



	eConfigInterfaceType ifType = eSignalingNetwork;
	if( IsJitcAndNetSeparation() && !isV35Service)
		ifType = eSeparatedSignalingNetwork;
	else if( isMultipleServices || ( GetIsV35JITCSupport() && isV35Service) )
	{
		//DWORD boardIdUsedByCS, subBoardIdUsedByCS;
		//GetSignalingMasterBoardId(csId, boardIdUsedByCS, subBoardIdUsedByCS);
		DWORD boardIdUsedByCS =0 , subBoardIdUsedByCS =0;
		CIPServiceList *pIpServListDynamic = GetIpServiceListDynamic();
		CIPService *pService = pIpServListDynamic->GetService(csId);
		if(pService)
		{
			DWORD serviceId = pService->GetId();

			if(isV35Service)
			{
					GetSignalingMasterBoardId(serviceId, boardIdUsedByCS, subBoardIdUsedByCS);
			}
			else
			{
				boardIdUsedByCS = GetCSIpConfigMasterBoardId(serviceId);
				subBoardIdUsedByCS = GetCSIpConfigMasterPqId(serviceId);
			}
			TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::GetSignalingInterfaceType boardIdUsedByCS = " << boardIdUsedByCS << " and subBoardIdUsedByCS = " << subBoardIdUsedByCS;

			if(boardIdUsedByCS)
			{
				ifType = GetSignalingNetworkType(boardIdUsedByCS, subBoardIdUsedByCS);
			}
		}
	}
	if (isLanRedundancy && (eProductTypeGesher == curProductType|| eProductTypeNinja == curProductType))
	{
		ifType = eLanRedundancySignalingNetwork;
	}

	return ifType;
}

////////////////////////////////////////////////////////////////////////////////////
DWORD CCSMngrProcess::GetServiceIdFromDynamicList(DWORD board_id, DWORD sub_board_id)
{
	CIPService *pServiceD = m_pIpServiceListDynamic->GetFirstService();
	while (pServiceD)
	{
		WORD pos = pServiceD->CalcSpanPosAccordingToBoardAndSubBoardId(board_id, sub_board_id);

		CIPSpan* pSpan = pServiceD->GetSpanByIdx(pos);

		if(pSpan)
		{
			if (0 != pSpan->GetIPv4Address())
			{
				TRACESTR(eLevelInfoNormal) << "\nCCSMngrProcess::GetServiceIdFromDynamicList - found the service id "<<pServiceD->GetId()<<" for board_id = "<<board_id<<" sub board id = "<<sub_board_id;
				return pServiceD->GetId();
			}
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::GetServiceIdFromDynamicList - still looking for the service";
		}

		pServiceD = m_pIpServiceListDynamic->GetNextService();
	}
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrProcess::GetServiceIdFromDynamicList - couldn't find the service id for board_id = "<<board_id<<" sub board id = "<<sub_board_id;
	return -1;
}

////////////////////////////////////////////////////////////////////////////////////
BYTE CCSMngrProcess::GetIpServiceFileNames(std::string &ipServiceFileName, std::string &ipServiceFileNameTmp)
{
	BOOL isV35JitcSupport = NO;
	BOOL isMultipleServicesSupport = NO;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, isV35JitcSupport);
	sysConfig->GetBOOLDataByKey(CFG_KEY_MULTIPLE_SERVICES, isMultipleServicesSupport);
	BYTE bSystemMode;

	if (isMultipleServicesSupport)
	{
		ipServiceFileName = IP_MULTIPLE_SERVICES_TWO_SPANS_LIST_PATH;
		ipServiceFileNameTmp = IP_MULTIPLE_SERVICES_TWO_SPANS_LIST_TMP_PATH;
		FTRACESTR(eLevelInfoNormal) << "std::string CCSMngrProcess::GetIpServiceFileName() - Multiple Services";
		bSystemMode = eSystemMode_Multiple_services;
	}
	else if (isV35JitcSupport)
	{
		ipServiceFileName = IP_SERVICES_JITC_V35_LIST_PATH;
		ipServiceFileNameTmp = IP_SERVICES_JITC_V35_LIST_TMP_PATH;
		FTRACESTR(eLevelInfoNormal) << "std::string CCSMngrProcess::GetIpServiceFileName() - V35 JITC";
		bSystemMode = eSystemMode_Jitc_v35;
	}
	else
	{
		ipServiceFileName = IP_SERVICE_LIST_PATH;
		ipServiceFileNameTmp = IP_SERVICE_LIST_TMP_PATH;
		bSystemMode = eSystemMode_None;
		FTRACESTR(eLevelInfoNormal) << "std::string CCSMngrProcess::GetIpServiceFileName() - No special mode";
	}
	return bSystemMode;
}


////////////////////////////////////////////////////////////////////////////

void CCSMngrProcess::SetCSIpConfigMasterBoardId(int serviceId,DWORD boardId)
{
	if(serviceId <= MAX_NUM_OF_IP_SERVICES)
		m_MasterBoardIdPerService[serviceId].boardId = boardId;
}

////////////////////////////////////////////////////////////////////////////
 DWORD CCSMngrProcess::GetCSIpConfigMasterBoardId(int serviceId) const
{
	DWORD   boardId= 0;
	if(serviceId <= MAX_NUM_OF_IP_SERVICES)
	{
		return m_MasterBoardIdPerService[serviceId].boardId;
	}
	return boardId;
}

////////////////////////////////////////////////////////////////////////////

void CCSMngrProcess::SetCSIpConfigMasterPqId(int serviceId,DWORD pqId)
{
	if(serviceId <= MAX_NUM_OF_IP_SERVICES)
		m_MasterBoardIdPerService[serviceId].pqId = pqId;
}

////////////////////////////////////////////////////////////////////////////
 DWORD CCSMngrProcess::GetCSIpConfigMasterPqId(int serviceId) const
{
	DWORD   pqId= 0;
	if(serviceId <= MAX_NUM_OF_IP_SERVICES)
	{
		return m_MasterBoardIdPerService[serviceId].pqId;
	}
	return pqId;
}

CSystemInterfaceList* CCSMngrProcess::GetSystemInterfaceList()
{
    TRACESTR(eLevelInfoNormal) << "CCSMngrProcess::GetSystemInterfaceList Clear and Get Interface";
    m_pSysInterfaceList->CleanUpList();
    ReadSystemInterfaceList();
    return m_pSysInterfaceList;
}

void CCSMngrProcess::ReadSystemInterfaceList()
{

	//adding default value for non-using of ip address
	//m_pSysInterfaceList->Add(0, 0, "", "::/64", "::/64", "::/64");
    TRACESTR(eLevelInfoNormal) << "CCSMngrProcess::ReadSystemInterfaceList";

	m_pSysInterfaceList->GetInterfacesFromSystem();
	m_pSysInterfaceList->Dump();
}

/////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::SetMngmntDnsIpV4Address(char ipAddressStr[IP_ADDRESS_LEN])
{
	strncpy(m_ipv4AddressStr,ipAddressStr,strlen(ipAddressStr));
	m_ipv4AddressStr[strlen(ipAddressStr)] = '\0';
}
/////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::SetMngmntDnsIpV6Address(char ipAddressStr[IPV6_ADDRESS_LEN])
{
	strncpy(m_ipv6AddressStr,ipAddressStr,strlen(ipAddressStr));
	m_ipv6AddressStr[strlen(ipAddressStr)] = '\0';

}
/////////////////////////////////////////////////////////////////////////////
void CCSMngrProcess::SetMngmntDnsStatus(BOOL dnsStatus)
{
	 m_dnsStatus = dnsStatus;
}
