// SipProxyManager.cpp: implementation of the CSipProxyServiceManager class.
//
//////////////////////////////////////////////////////////////////////


#include "SipProxyServiceManager.h"
#include "IpCsOpcodes.h"
#include "MplMcmsProtocol.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "Trace.h"
#include "SIPProxyStructs.h"
#include  "StatusesGeneral.h"
#include  "SipProxyApi.h"
#include  "DataTypes.h"
#include  "NStream.h"
#include  "CsCommonStructs.h"
#include  "SystemFunctions.h"
#include "SipProxyProcess.h"
#include "FaultsDefines.h"
#include "ConfPartyManagerApi.h"
#include <algorithm>
#include "ApiStatuses.h"
#include "InternalProcessStatuses.h"
#include "SipProxyMplMcmsProtocolTracer.h"
#include "SysConfig.h"
#include "ICEApiDefinitions.h"
#include "IceCmReq.h"
#include "IceCmInd.h"
#include <arpa/inet.h>

#include "TraceStream.h"
#include "ServiceConfigList.h"
#include "DefinesIpService.h"//carmit45
#include "SysConfigKeys.h"
#include "SipProxyGlobals.h"

#include "ConfigHelper.h"
#include "SysConfig.h"



// definitions
#define CONF_NAME_EMPTY					1
#define ADD_CONF_ALREADY_EXISTS				2
#define IP_CARD_NOT_FOUND			3
#define ADD_CONF_DB_IS_FULL					4
#define ADD_CONF_CNTL_CREATE_ERROR			5
#define ADD_CONF_START_REGISTRATION_ERROR	6
#define DEL_CONF_DOES_NOT_EXIST				7
#define SERVICE_SET_DONT_REG_ONGOING		8
#define SERVICE_SET_DONT_REG_MEETING_ROOM	9
#define SERVICE_SET_DONT_REG_ENTRY_QUEUE	10
#define SERVICE_SET_DONT_REG_SIP_FACTORY	11
#define DONT_REG_ONGOING_COPY_OF_EQ			12
#define SERVICE_SET_DONT_REG_GW_PROFILE		13
#define PRESENTED_CONF_DOES_NOT_EXIST		14

#define USER_DOMAIN_NAME_LEN				256

#define TIMER_RETRY_PROXY			100
#define TIMER_RETRY_BUSY_PROXY		101
#define TIMER_RETRY_ASK_CONFS_DB	102

#define	RetryTimeOut			90

#define	RetryAskConfsDbTimeOut	40

#define MAX_CALLS_PER_CARD		30

#define RETRY_AFTER_THRESHOLD	60


// SIP Proxy Manager states
const WORD	 SET_UP		= 1;
const WORD   CONNECT    = 2;

// temporary as a sample
const WORD   DUMMY          = 1;
const WORD   DUMMY_ENTRY     = 0;
////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CSipProxyServiceManager)
	//ONEVENT(CSAPI_MSG							,ANYCASE	,CSipProxyServiceManager::OnCSApiMsg)
	ONEVENT(SIP_CS_PROXY_TRANSPORT_ERROR_IND	,ANYCASE	,CSipProxyServiceManager::HandleTransportError)
	ONEVENT(SIP_CS_PROXY_BAD_STATUS_IND			,ANYCASE	,CSipProxyServiceManager::NullActionFunction)
	ONEVENT(SIP_CS_PROXY_REGISTER_RESPONSE_IND	,CONNECT	,CSipProxyServiceManager::HandleRegisterResponse)
	ONEVENT(SIP_CS_PROXY_SUBSCRIBE_RESPONSE_IND	,CONNECT	,CSipProxyServiceManager::HandleSubscribeResponse)
	ONEVENT(SIP_CS_PROXY_NOTIFY_IND				,CONNECT	,CSipProxyServiceManager::HandleNotifyInd)
	ONEVENT(SIP_CS_SIG_NOTIFY_RESPONSE_IND	,CONNECT	,CSipProxyServiceManager::HandleNotifyResponse)
	ONEVENT(SIP_CS_PROXY_SERVICE_RESPONSE_IND	,CONNECT	,CSipProxyServiceManager::HandleServiceResponse)
	ONEVENT(SIP_CS_SIG_BAD_STATUS_IND			,CONNECT	,CSipProxyServiceManager::HandleRegisterFailure)
	ONEVENT(SIP_CS_SIG_TRACE_INFO_IND			,CONNECT    ,CSipProxyServiceManager::HandleRegisterFailure)
	ONEVENT(SIP_CS_PROXY_TRACE_INFO_IND			,CONNECT    ,CSipProxyServiceManager::HandleRegisterFailure)

	ONEVENT(CS_PROXY_IP_SERVICE_PARAM_IND		,CONNECT	,CSipProxyServiceManager::OnIpServiceParamIndConnect)
	ONEVENT(CS_PROXY_DELETE_IP_SERVICE_IND		,CONNECT	,CSipProxyServiceManager::OnIpServiceParamDel)

	//ONEVENT(DUMMY				,CONNECT	,CSipProxyServiceManager::OnCardEndStartup	)
	//ONEVENT(ON_CARD_END_STARTUP	,CONNECT    ,CSipProxyServiceManager::OnCardEndStartup	)
	ONEVENT(START_CONF_REG		,CONNECT    ,CSipProxyServiceManager::OnAddConf	)
	ONEVENT(START_CONF_REG		,SET_UP    ,CSipProxyServiceManager::NullActionFunction	)
	ONEVENT(TERMINATE_CONF_REG	,CONNECT    ,CSipProxyServiceManager::OnDelConf	)
	ONEVENT(TERMINATE_CONF_REG	,SET_UP     ,CSipProxyServiceManager::NullActionFunction	)



	ONEVENT(SIP_PROXY_CHANGE_PRESENCE_STATE		,CONNECT    ,CSipProxyServiceManager::OnChangePresenceState	)
	ONEVENT(SIP_PROXY_CHANGE_PRESENCE_STATE		,SET_UP    ,CSipProxyServiceManager::NullActionFunction	)






	ONEVENT(KILLONECONF				,CONNECT    ,CSipProxyServiceManager::OnKillOneConf)
	//ONEVENT(H323_ON_CARD_CRASHED	,CONNECT	,CSipProxyServiceManager::OnCardCrash)
	ONEVENT(TIMER_RETRY_PROXY		,CONNECT    ,CSipProxyServiceManager::OnTimerRetryProxy)
	ONEVENT(TIMER_RETRY_BUSY_PROXY	,CONNECT    ,CSipProxyServiceManager::OnTimerRetryBusyProxy)
	ONEVENT(TIMER_RETRY_ASK_CONFS_DB,CONNECT    ,CSipProxyServiceManager::OnTimerRetryAskConfsDb)
	ONEVENT(DNS_RESOLVE_IND			,CONNECT    ,CSipProxyServiceManager::OnDNSResolveInd)
	ONEVENT(DNS_SERVICE_IND			,CONNECT    ,CSipProxyServiceManager::OnDNSServiceInd)
//	ONEVENT(LOADACCEPTMSGACK		,CONNECT    ,CSipProxyServiceManager::OnLoadMngrAck)
	ONEVENT(REGISTRAR_STATUS		,CONNECT    ,CSipProxyServiceManager::OnRegistrarStatus)
	ONEVENT(CS_PROXY_IP_SERVICE_PARAM_IND		,SET_UP		,CSipProxyServiceManager::OnIpServiceParamIndSetup)
	ONEVENT(CS_PROXY_IP_SERVICE_PARAM_END_IND	,SET_UP		,CSipProxyServiceManager::OnIpServiceParamEnd)
	ONEVENT(CS_PROXY_CS_UP_IND					,SET_UP		,CSipProxyServiceManager::OnSC_upInd)
	ONEVENT(CS_PROXY_CS_UP_IND					,CONNECT	,CSipProxyServiceManager::NullActionFunction)

	ONEVENT(SIPPROXY_MCUMNGR_CONFIGURATION_IND	,SET_UP		,CSipProxyServiceManager::OnMcuMngrConfigSetup)
	ONEVENT(SIPPROXY_MCUMNGR_CONFIGURATION_IND	,CONNECT	,CSipProxyServiceManager::OnMcuMngrConfigCONNECT)


	ONEVENT(SIP_CS_PROXY_SERVICE_RESPONSE_IND	,CONNECT	,CSipProxyServiceManager::HandleServiceResponse)
	ONEVENT(CS_PROXY_IP_SERVICE_PARAM_END_IND	,CONNECT	,CSipProxyServiceManager::NullActionFunction)

	ONEVENT(UPDATE_ICE_END      ,CONNECT	,CSipProxyServiceManager::OnCSEndIceInd)

	ONEVENT(SIPPROXY_CONFIG_CS_IND	,SET_UP		,CSipProxyServiceManager::OnMediaCardSetupEnd)
	ONEVENT(SIPPROXY_CONFIG_CS_IND	,CONNECT	,CSipProxyServiceManager::NullActionFunction)

	ONEVENT(MCUMNGR_TO_SIPPROXY_MULTIPLE_SERVICES_IND	,ANYCASE	,CSipProxyServiceManager::OnMultipleServicesInd)
	ONEVENT(DIALOG_RECOVERY_IND,ANYCASE  ,CSipProxyServiceManager::OnDialogRecoveryMessageInd)

	ONEVENT(SIP_CS_PROXY_CRLF_ERR_IND   ,ANYCASE  ,CSipProxyServiceManager::OnMsKeepAliveToutErrInd)
//	ONEVENT(XML_REQUEST			,IDLE		,CSipProxyServiceManager::HandlePostRequest )

PEND_MESSAGE_MAP(CSipProxyServiceManager,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

//BEGIN_SET_TRANSACTION_FACTORY(CSipProxyServiceManager)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CSipProxyServiceManager::HandleOperLogin)
//END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
/*BEGIN_TERMINAL_COMMANDS(CSipProxyServiceManager)
	ONCOMMAND("dump_sip_proxy",CSipProxyServiceManager::HandleTerminalDumpSipProxy,"Dump Sip proxy data.")
END_TERMINAL_COMMANDS*/

extern void SipProxyMonitorEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void SipProxyServiceManagerEntryPoint(void* appParam)
{
	CSipProxyServiceManager * pSipProxyManager = new CSipProxyServiceManager;
	pSipProxyManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CSipProxyServiceManager::GetMonitorEntryPoint()
{
	return SipProxyMonitorEntryPoint;
}


/**
 * This is a utility function which finds the sipConfType.
 */
eSipRegistrationConfType SipProxyServiceManagerGetConfType(COneConfRegistration *confReg)
{
	eSipRegistrationConfType sipConfType;

	if(confReg->m_bIsOngoing)
		sipConfType = eSipRegistrationConfTypeOngoing;
	else if(confReg->m_bIsMR)
		sipConfType = eSipRegistrationConfTypeMR;
	else if(confReg->m_bIsEQ)
		sipConfType = eSipRegistrationConfTypeEQ;
	else if(confReg->m_bIsFactory)
		sipConfType = eSipRegistrationConfTypeFactory;
	else if(confReg->m_bIsGWProfile)
		sipConfType = eSipRegistrationConfTypeGWProfile;
	else
		sipConfType = eSipRegistrationConfTypeIncorrect;

	return sipConfType;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CSipProxyServiceManager::CSipProxyServiceManager() : m_conferencesCounter(0), m_UseMainProxy(TRUE)
							, m_timerRetry(0), m_SCisUp(FALSE), m_serviceEndWasReceived(FALSE)
                            , m_mediaCardSetupEndWasReceived(FALSE), m_pMockMplProtocol(NULL), m_bSystemMultipleServices(NO)
{
	int i = 0;
	m_state = SET_UP;
	//::SetpConfRegDB((COneConfRegistration**)m_pConfRegDB);*************
	for (i = 0; i < MAX_CONF_REGISTRATIONS; i++)
		m_pConfRegDB[i] = 0;

	m_confList = new CConfList;
	m_servicesList = new CServicesList;
	m_numOfBlastRegRequests = 0;
	m_firstRegisterTicksTime = 0;
	m_lastRegisterTicksTime = 0;
	SetBusyExpiredTime(0);
	m_BusyProxyVector.clear();
	memset(m_Dummy_name, 0, sizeof(m_Dummy_name));

	m_pProxyService = NULL;
	m_pICEserviceId = 0;
	//printf("CSipProxyManager::CSipProxyServiceManager constructor\n");
}

/////////////////////////////////////////////////////////////////////////////
CSipProxyServiceManager::CSipProxyServiceManager(CMplMcmsProtocol* pMock): m_conferencesCounter(0), m_UseMainProxy(TRUE)
							, m_timerRetry(0), m_SCisUp(FALSE), m_serviceEndWasReceived(FALSE), m_DNSConfigWasReceived(FALSE)
							, m_DNSStatus(FALSE), m_mediaCardSetupEndWasReceived(FALSE), m_pMockMplProtocol(pMock), m_bSystemMultipleServices(NO)
{
	int i = 0;
	m_state = SET_UP;
	//::SetpConfRegDB((COneConfRegistration**)m_pConfRegDB);*************
	for (i = 0; i < MAX_CONF_REGISTRATIONS; i++)
		m_pConfRegDB[i] = 0;

	m_confList = new CConfList;
	m_servicesList = new CServicesList;
	m_numOfBlastRegRequests = 0;
	m_firstRegisterTicksTime = 0;
	m_lastRegisterTicksTime = 0;
	SetBusyExpiredTime(0);
	m_BusyProxyVector.clear();
  memset(m_Dummy_name, 0, sizeof(m_Dummy_name));
}

/////////////////////////////////////////////////////////////////////////////
CSipProxyServiceManager::~CSipProxyServiceManager()
{
	if(m_timerRetry)
		DeleteTimer(TIMER_RETRY_PROXY);

	if (IsValidTimer(TIMER_RETRY_BUSY_PROXY))
		DeleteTimer(TIMER_RETRY_BUSY_PROXY);
	if (IsValidTimer(TIMER_RETRY_ASK_CONFS_DB))
		DeleteTimer(TIMER_RETRY_ASK_CONFS_DB);

	POBJDELETE(m_confList);
	POBJDELETE(m_servicesList);
	for (int i = 0; i < MAX_CONF_REGISTRATIONS; i++)
		if (m_pConfRegDB[i])
			POBJDELETE(m_pConfRegDB[i]);

	DeleteBusyServerList();

	POBJDELETE(m_pProxyService);


}

//////////////////////////////////////////////////////////////////////
void CSipProxyServiceManager::DeleteBusyServerList()
{
	// delete busy server vector:
	StBusyServerParam* pBusyServer = NULL;
	std::vector< StBusyServerParam* >::iterator itr =  m_BusyProxyVector.begin();
	while (itr != m_BusyProxyVector.end())
	{
		pBusyServer = (*itr);
		m_BusyProxyVector.erase(itr);
		POBJDELETE(pBusyServer);
		itr = m_BusyProxyVector.begin();
	}
	m_BusyProxyVector.clear();
}
//////////////////////////////////////////////////////////////////////
//STATUS CSipProxyServiceManager::HandleTerminalPing(CSegment * seg,std::ostream& answer)
//{
//	PTRACE(eLevelError,"pong to logger");
//	answer << "pong to console";
//	return STATUS_OK;
//}

/////////////////////////////////////////////////////////////////////
/*void CSipProxyServiceManager::DeclareStartupConditions()
{
	CActiveAlarm aa1(FAULT_GENERAL_SUBJECT,
					 AA_NO_IP_SERVICE_PARAMS,
					 MAJOR_ERROR_LEVEL,
					 "No IP service was received from CSMngr",
					 false,
					 false);
 	AddStartupCondition(aa1);
}*/

/////////////////////////////////////////////////////////////////////
void CSipProxyServiceManager::ManagerStartupActionsPoint()
{

	McuMngrConfigSetup();

}

/////////////////////////////////////////////////////////////////////////////
void*  CSipProxyServiceManager::GetMessageMap()
{
  return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::RequestIPServicesFromCsManager()
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::RequestIPServicesFromCsManager");
	CSegment*  pRetParam = new CSegment;

	const COsQueue* pCsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

	STATUS res = pCsMbx->Send(pRetParam,CS_PROXY_IP_SERVICE_PARAM_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::AskMcuMngrForConfigurationStatus()
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::AskMcuMngrForConfigurationStatus");
	CSegment*  pRetParam = new CSegment;

	const COsQueue* pCsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessMcuMngr, eManager);

	STATUS res = pCsMbx->Send(pRetParam, SIPPROXY_MCUMNGR_CONFIGURATION_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnIpServiceParamIndSetup(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnIpServiceParamIndSetup ");

	CSipProxyIpParams* pService = new CSipProxyIpParams;
	pService->Deserialize(pSeg);
	COstrStream msg;
	pService->Dump(msg);
	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnIpServiceParamIndSetup:\n ", msg.str().c_str());



	if (pService->GetIpType() == eIpType_IpV4)
	{
		for (int i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES; i++)
		{
			if (pService->GetServiceIpAddress(i).ipVersion == eIpVersion6)
				pService->SetServiceIpAddressToNull(i,0);
		}
	}
	else if (pService->GetIpType() == eIpType_IpV6)
	{
		for (int i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES; i++)
		{
			if (pService->GetServiceIpAddress(i).ipVersion == eIpVersion4)
				pService->SetServiceIpAddressToNull(i,1);

		}
	}

  for (int i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES; i++)
  {
    ipAddressStruct ipAddrSt = pService->GetServiceIpAddress(i);
    if (!IsIpNull(&ipAddrSt))
    {
      char a[200];
      ipToString(ipAddrSt, a, 0);
      BuildDummyName(a);
      break;
    }
  }
  PTRACE2(eLevelInfoHigh, "CSipProxyServiceManager::OnIpServiceParamIndSetup dummy is: ", m_Dummy_name);

	COstrStream msg1;
	pService->Dump(msg1);
	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnIpServiceParamIndSetup after:\n ", msg1.str().c_str());
	// In case the service is of one IP type only  - ProxyManager will remove from CSIp (AddrList) addresses the opposite
	// Address -
	//If IpV6 only - Remove IpV4 CS address and vice versa

	//create service -- rons// IpV6 - Temp untill Ema fixes its code
/*
	SIP_PROXY_IP_PARAMS_S newServiceIpParams;


	newServiceIpParams.IpType = eIpType_IpV6;
	for(int i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&newServiceIpParams.pAddrList[i], 0, sizeof(ipAddressStruct));
	}

	newServiceIpParams.AlternateProxyAddress.addr.v4.ip = 0;
	newServiceIpParams.AlternateProxyAddress.port = 0;
	newServiceIpParams.Dhcp = NO;
	newServiceIpParams.DNSStatus = eServerStatusOff;//eServerStatusSpecify,eServerStatusOff
//	newServiceIpParams.IpAddress.addr.v4.ip = SystemIpStringToDWORD("172.22.172.141", eHost);
//	newServiceIpParams.IpAddress.addr.v4.ip = SystemIpStringToDWORD("172.22.192.54", eHost);
//	newServiceIpParams.IpAddress.ipVersion  = eIpVersion4;

	mcTransportAddress tmpIpAddr;
	::stringToIp(&tmpIpAddr,"2001:db8:0:1:215:60ff:fe09:b17a");
//	memcpy(newServiceIpParams.IpAddressIpV6.addr.v6.ip, tmpIpAddr.addr.v6.ip,IPV6_ADDRESS_BYTES_LEN);
//	newServiceIpParams.IpAddressIpV6.ipVersion  = eIpVersion6;

	memcpy(newServiceIpParams.pAddrList[0].addr.v6.ip, tmpIpAddr.addr.v6.ip,IPV6_ADDRESS_BYTES_LEN);
	newServiceIpParams.pAddrList[0].ipVersion  = eIpVersion6;

//	newServiceIpParams.OutboundProxyAddress.addr.v4.ip = SystemIpStringToDWORD("172.22.188.150", eHost);
//	ipV6ToString(mcTransportAddress *ipAddress, char *sIpAddress);pService
    mcTransportAddress tmpProxyAddr;
    tmpProxyAddr.port = 5060;//1720;
    tmpProxyAddr.ipVersion = eIpVersion6;
    tmpProxyAddr.distribution = 0;
    tmpProxyAddr.transportType = eTransportTypeTcp;//eTransportTypeUdp,eTransportTypeTcp
    tmpProxyAddr.addr.v6.scopeId = 0;
    ::stringToIp(&tmpProxyAddr,"2001:dbb:0:1:213:21ff:feb5:b0a7");//2001:dbb:0:1:213:21ff:feb5:b0a7,2001:dbb:0:1:201:2ff:fedf:a030


//	memcpy(newServiceIpParams.OutboundProxyAddress.addr.v6.ip, "2001:dbb:0:1:201:2ff:fedf:a030",IPV6_ADDRESS_BYTES_LEN);
	newServiceIpParams.OutboundProxyAddress.addr.v4.ip = 0;
	memcpy(newServiceIpParams.OutboundProxyAddress.addr.v6.ip, tmpProxyAddr.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);

	newServiceIpParams.OutboundProxyAddress.ipVersion  = eIpVersion6;
//	newServiceIpParams.OutboundProxyAddress.ipVersion  = eIpVersion4;
	newServiceIpParams.OutboundProxyAddress.port = 5060;
	strcpy(newServiceIpParams.pAltProxyHostName, "");
	strcpy(newServiceIpParams.pAltProxyName, "");
	strcpy(newServiceIpParams.pOutboundProxyName, "");


//	strcpy(newServiceIpParams.pProxyHostName, "ip.co.il");

	char ProxyHostNameAddr[128];
	ipV6ToString(tmpProxyAddr.addr.v6.ip, ProxyHostNameAddr, TRUE);
	strncpy(newServiceIpParams.pProxyHostName, ProxyHostNameAddr, H243_NAME_LEN);
	FTRACESTR(eLevelInfoNormal) << "CSipProxyServiceManager::OnIpServiceParamIndSetup tmpHostNameAddr = " << ProxyHostNameAddr;


	strcpy(newServiceIpParams.pProxyName, "");
//	newServiceIpParams.ProxyAddress.addr.v4.ip  = SystemIpStringToDWORD("172.22.188.150", eHost);
//	newServiceIpParams.ProxyAddress.ipVersion  = eIpVersion4;
	newServiceIpParams.ProxyAddress.addr.v4.ip = 0;
	memcpy(newServiceIpParams.ProxyAddress.addr.v6.ip, tmpProxyAddr.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN); //rons -for DNS
	newServiceIpParams.ProxyAddress.ipVersion  = eIpVersion6; //rons -for DNS

	newServiceIpParams.ProxyAddress.port = 5060;
	newServiceIpParams.refreshTout = 3600;
	newServiceIpParams.RegistrationFlags = 0xFF;
	newServiceIpParams.serversConfig = eConfSipServerManually;
	newServiceIpParams.ServiceId = 3;
	strcpy(newServiceIpParams.ServiceName, "Dummy LCS");
	newServiceIpParams.transportType = eTransportTypeTcp;//eTransportTypeUdp,eTransportTypeTcp

	CSipProxyIpParams*  pServiceParams = new CSipProxyIpParams(newServiceIpParams);
	//create service -- rons
	COstrStream msg;
	pServiceParams->Dump(msg);
	PTRACE2(eLevelInfoNormpServiceal, "CSipProxyServiceManager::OnIpServiceParamIndSetup:\n ", msg.str().c_str());
*/
	//insert(service) -- rons
//	m_servicesList->Insert(pServiceParams);
	//insert(service) -- rons
	m_ServiceIceType = pService->GetICEType();
	m_pProxyService = new CSipProxyIpParams;

	*m_pProxyService = *pService;

	m_pICEserviceId = pService->GetServiceId();

	PTRACE2INT(eLevelInfoHigh, "CSipProxyServiceManager::OnIpServiceParamIndSetup #1 m_pProxyService->GetICEType: ", m_pProxyService->GetICEType());
	PTRACE2INT(eLevelInfoHigh, "CSipProxyServiceManager::OnIpServiceParamIndSetup #1 m_ServiceIceType: ", m_ServiceIceType);
	PTRACE2INT(eLevelInfoHigh, "CSipProxyServiceManager::OnIpServiceParamIndSetup #1 m_pICEserviceId: ", m_pICEserviceId);

	if(m_servicesList->Insert(pService)==STATUS_OK)
	{
		m_pICEserviceId = pService->GetServiceId();
		RemoveActiveAlarmByErrorCode(AA_NO_IP_SERVICE_PARAMS);
	} 
	else 
	{
		//m_pProxyService = new CSipProxyIpParams;
		//m_pProxyService = pService;
	//	m_pICEserviceId = 1;
		/*if return STATUS_FAIL, need to free the memory*/
		POBJDELETE(pService);

	//	PTRACE2INT(eLevelInfoHigh, "CSipProxyServiceManager::OnIpServiceParamIndSetup #2 m_pProxyService->GetICEType: ", m_pProxyService->GetICEType());

	}

	PTRACE2INT(eLevelInfoHigh, "CSipProxyServiceManager::OnIpServiceParamIndSetup #3 m_pProxyService->GetICEType: ", m_pProxyService->GetICEType());
	PTRACE2INT(eLevelInfoHigh, "CSipProxyServiceManager::OnIpServiceParamIndSetup #3 m_ServiceIceType ", m_ServiceIceType);

}
/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnIpServiceParamIndSetup2(CSegment* pSeg) //JUST TO CHECK THAT IPV4 IS STILL WORKING
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnIpServiceParamIndSetup2 ");
/*
	CSipProxyIpParams* pService = new CSipProxyIpParams;
	pService->Deserialize(pSeg);
	COstrStream msg;
	pService->Dump(msg);
	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnIpServiceParamIndSetup2:\n ", msg.str().c_str());
*/
	//create service -- rons// IpV6 - Temp untill Ema fixes its code

	SIP_PROXY_IP_PARAMS_S newServiceIpParams;
	memset(&newServiceIpParams, 0, sizeof(newServiceIpParams));


/*
    mcTransportAddress tmpAlternateProxyAddr;
    tmpAlternateProxyAddr.port = 5060;//1720;
    tmpAlternateProxyAddr.ipVersion = eIpVersion6;
    tmpAlternateProxyAddr.distribution = 0;
    tmpAlternateProxyAddr.transportType = 0;
    tmpAlternateProxyAddr.addr.v6.scopeId = 0;
    ::stringToIp(&tmpAlternateProxyAddr,"2001:dbb:0:1:201:2ff:fedf:a030");
	memcpy(newServiceIpParams.AlternateProxyAddress.addr.v6.ip, tmpAlternateProxyAddr.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
	newServiceIpParams.AlternateProxyAddress.ipVersion  = eIpVersion6;
*/
	newServiceIpParams.IpType = eIpType_IpV4;
	for(int i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&newServiceIpParams.pAddrList[i], 0, sizeof(ipAddressStruct));
	}

	newServiceIpParams.AlternateProxyAddress.addr.v4.ip = 0;
	newServiceIpParams.AlternateProxyAddress.port = 0;
	newServiceIpParams.Dhcp = NO;
	newServiceIpParams.DNSStatus = eServerStatusSpecify;//eServerStatusOff;
//	newServiceIpParams.IpAddress.addr.v4.ip = SystemIpStringToDWORD("172.22.172.141", eHost);
//	newServiceIpParams.IpAddress.addr.v4.ip = SystemIpStringToDWORD("172.22.192.54", eHost);
//	newServiceIpParams.IpAddress.ipVersion  = eIpVersion4;

	mcTransportAddress tmpIpAddr;
	::stringToIp(&tmpIpAddr,"172.22.192.54");// my pizza(central signaling) ip
//	newServiceIpParams.IpAddressIpV4.addr.v4.ip =  tmpIpAddr.addr.v4.ip;
//	newServiceIpParams.IpAddressIpV4.ipVersion  = eIpVersion4;
	newServiceIpParams.pAddrList[0].addr.v4.ip =  tmpIpAddr.addr.v4.ip;
	newServiceIpParams.pAddrList[0].ipVersion  = eIpVersion4;

//	newServiceIpParams.OutboundProxyAddress.addr.v4.ip = SystemIpStringToDWORD("172.22.184.84", eHost);
//	ipV6ToString(mcTransportAddress *ipAddress, char *sIpAddress);
    mcTransportAddress tmpProxyAddr;

    tmpProxyAddr.addr.v4.ip = 0;//rons

    tmpProxyAddr.port = 5060;//1720;
    tmpProxyAddr.ipVersion = eIpVersion4;
    tmpProxyAddr.distribution = 0;
    tmpProxyAddr.transportType = eTransportTypeTcp;//eTransportTypeUdp,eTransportTypeTcp
 //   tmpProxyAddr.addr.v6.scopeId = 0;
 //   ::stringToIp(&tmpProxyAddr,"172.22.188.150");//2001:dbb:0:1:213:21ff:feb5:b0a7,2001:dbb:0:1:201:2ff:fedf:a030,172.22.184.84


//	memcpy(newServiceIpParams.OutboundProxyAddress.addr.v6.ip, "2001:dbb:0:1:201:2ff:fedf:a030",IPV6_ADDRESS_BYTES_LEN);
//	newServiceIpParams.OutboundProxyAddress.addr.v4.ip = tmpProxyAddr.addr.v4.ip;
	newServiceIpParams.OutboundProxyAddress.ipVersion  = eIpVersion4;
//	newServiceIpParams.OutboundProxyAddress.ipVersion  = eIpVersion4;
	newServiceIpParams.OutboundProxyAddress.port = 5060;
	strcpy(newServiceIpParams.pAltProxyHostName, "");
	strcpy(newServiceIpParams.pAltProxyName, "");
//	strcpy(newServiceIpParams.pOutboundProxyName, "sqa-lcs2005");
	strcpy(newServiceIpParams.pOutboundProxyName, "sagi222");


//	strcpy(newServiceIpParams.pProxyHostName, "sqa1.il");
	strcpy(newServiceIpParams.pProxyHostName, "israel.polycom.com");


	char ProxyHostNameAddr[16];
	SystemDWORDToIpString(tmpProxyAddr.addr.v4.ip, ProxyHostNameAddr);
	strncpy(newServiceIpParams.pProxyHostName, ProxyHostNameAddr, H243_NAME_LEN);
//	strncpy(newServiceIpParams.pProxyHostName, "sqa-lcs2005", H243_NAME_LEN);
//	strncpy(newServiceIpParams.pProxyName, "sqa1.il", H243_NAME_LEN);
//	strncpy(newServiceIpParams.pOutboundProxyName, "sqa1.il", H243_NAME_LEN);

	FTRACESTR(eLevelInfoNormal) << "CSipProxyServiceManager::OnIpServiceParamIndSetup tmpHostNameAddr = " << ProxyHostNameAddr;


//	strcpy(newServiceIpParams.pProxyName, "sqa-lcs2005");
	strcpy(newServiceIpParams.pProxyName, "sagi222");
//	newServiceIpParams.ProxyAddress.addr.v4.ip  = SystemIpStringToDWORD("172.22.188.150", eHost);
//	newServiceIpParams.ProxyAddress.ipVersion  = eIpVersion4;
	newServiceIpParams.ProxyAddress.addr.v4.ip = tmpProxyAddr.addr.v4.ip;//rons
	newServiceIpParams.ProxyAddress.ipVersion  = eIpVersion4;

	newServiceIpParams.ProxyAddress.port = 5060;
	newServiceIpParams.refreshTout = 3600;
	newServiceIpParams.RegistrationFlags = 0xFF;
	newServiceIpParams.serversConfig = eConfSipServerManually;
	newServiceIpParams.ServiceId = 3;
	strcpy(newServiceIpParams.ServiceName, "Dummy LCS");
	newServiceIpParams.transportType = eTransportTypeTcp;//eTransportTypeUdp,eTransportTypeTcp

	CSipProxyIpParams*  pServiceParams = new CSipProxyIpParams(newServiceIpParams);
	
	COstrStream msg;
	pServiceParams->Dump(msg);
	PTRACE2(eLevelInfoHigh, "CSipProxyServiceManager::OnIpServiceParamIndSetup2:\n ", msg.str().c_str());

	if ( m_servicesList->Insert(pServiceParams) == STATUS_OK)
	{
		RemoveActiveAlarmByErrorCode(AA_NO_IP_SERVICE_PARAMS);
	}
	else
	{
		POBJDELETE(pServiceParams);
	}

}//JUST TO CHECK THAT IPV4 IS STILL WORKING
/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnIpServiceParamIndSetup3(CSegment* pSeg) //JUST TO CHECK THAT IPV4 + IPV6 (BOTH) IS WORKING
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnIpServiceParamIndSetup3 ");
/*
	CSipProxyIpParams* pService = new CSipProxyIpParams;
	pService->Deserialize(pSeg);
	COstrStream msg;
	pService->Dump(msg);

	PTRACE2(eLevelInfoNormal, "CSipProxyManager::OnIpServiceParamIndSetup3:\n ", msg.str().c_str());
*/
	//create service -- rons// IpV6 - Temp untill Ema fixes its code

	SIP_PROXY_IP_PARAMS_S newServiceIpParams;
	memset(&newServiceIpParams, 0, sizeof(newServiceIpParams));


	newServiceIpParams.IpType = eIpType_Both;

	newServiceIpParams.AlternateProxyAddress.addr.v4.ip = 0;
	newServiceIpParams.AlternateProxyAddress.port = 0;
	newServiceIpParams.Dhcp = NO;
	newServiceIpParams.DNSStatus = eServerStatusOff;//eServerStatusSpecify;
//	newServiceIpParams.IpAddress.addr.v4.ip = SystemIpStringToDWORD("172.22.172.141", eHost);
//	newServiceIpParams.IpAddress.addr.v4.ip = SystemIpStringToDWORD("172.22.192.54", eHost);
//	newServiceIpParams.IpAddress.ipVersion  = eIpVersion4;

	for(int i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&newServiceIpParams.pAddrList[i], 0, sizeof(ipAddressStruct));
	}
	mcTransportAddress tmpIpAddrIpV6;
	::stringToIp(&tmpIpAddrIpV6,"2001:db8:0:1:215:60ff:fe09:b17a");
//	memcpy(newServiceIpParams.IpAddressIpV6.addr.v6.ip, tmpIpAddrIpV6.addr.v6.ip,IPV6_ADDRESS_BYTES_LEN);
//	newServiceIpParams.IpAddressIpV6.ipVersion  = eIpVersion6;
	memcpy(newServiceIpParams.pAddrList[0].addr.v6.ip, tmpIpAddrIpV6.addr.v6.ip,IPV6_ADDRESS_BYTES_LEN);
	newServiceIpParams.pAddrList[0].ipVersion  = eIpVersion6;
	newServiceIpParams.pAddrList[0].addr.v6.scopeId  = eScopeIdGlobal;

	mcTransportAddress tmpIpAddrIpV4;
	::stringToIp(&tmpIpAddrIpV4,"172.22.192.54");
//	newServiceIpParams.IpAddressIpV4.addr.v4.ip = tmpIpAddrIpV4.addr.v4.ip;
//	newServiceIpParams.IpAddressIpV4.ipVersion  = eIpVersion4;

	newServiceIpParams.pAddrList[1].addr.v4.ip = tmpIpAddrIpV4.addr.v4.ip;
	newServiceIpParams.pAddrList[1].ipVersion  = eIpVersion4;

	memset(&newServiceIpParams.pAddrList[2], 0, sizeof(ipAddressStruct));


//	newServiceIpParams.OutboundProxyAddress.addr.v4.ip = SystemIpStringToDWORD("172.22.188.150", eHost);
//	ipV6ToString(mcTransportAddress *ipAddress, char *sIpAddress);
    mcTransportAddress tmpProxyAddr;
    tmpProxyAddr.port = 5060;//1720;
    tmpProxyAddr.ipVersion = eIpVersion6;
    tmpProxyAddr.distribution = 0;
    tmpProxyAddr.transportType = eTransportTypeTcp;//eTransportTypeUdp,eTransportTypeTcp
    tmpProxyAddr.addr.v6.scopeId = 0;
    ::stringToIp(&tmpProxyAddr,"2001:dbb:0:1:213:21ff:feb5:b0a7");//2001:dbb:0:1:213:21ff:feb5:b0a7,2001:dbb:0:1:201:2ff:fedf:a030
//    ::stringToIp(&tmpProxyAddr,"2001:dbb:0:1:201:2ff:fedf:a030");


//	memcpy(newServiceIpParams.OutboundProxyAddress.addr.v6.ip, "2001:dbb:0:1:201:2ff:fedf:a030",IPV6_ADDRESS_BYTES_LEN);
	memcpy(newServiceIpParams.OutboundProxyAddress.addr.v6.ip, tmpProxyAddr.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
	newServiceIpParams.OutboundProxyAddress.ipVersion  = eIpVersion6;
//	newServiceIpParams.OutboundProxyAddress.ipVersion  = eIpVersion4;
	newServiceIpParams.OutboundProxyAddress.port = 5060;
	strcpy(newServiceIpParams.pAltProxyHostName, "");
	strcpy(newServiceIpParams.pAltProxyName, "");
	strcpy(newServiceIpParams.pOutboundProxyName, "");


//	strcpy(newServiceIpParams.pProxyHostName, "ip.co.il");

	char ProxyHostNameAddr[128];
	ipV6ToString(tmpProxyAddr.addr.v6.ip, ProxyHostNameAddr, TRUE);
	const int maxLenForKlocWork = sizeof(newServiceIpParams.pProxyHostName) - 1;
	strncpy(newServiceIpParams.pProxyHostName, ProxyHostNameAddr, maxLenForKlocWork);
	newServiceIpParams.pProxyHostName[maxLenForKlocWork] = 0;

	FTRACESTR(eLevelInfoNormal) << "CSipProxyServiceManager::OnIpServiceParamIndSetup3 tmpHostNameAddr = " << ProxyHostNameAddr;


	strcpy(newServiceIpParams.pProxyName, "");
//	newServiceIpParams.ProxyAddress.addr.v4.ip  = SystemIpStringToDWORD("172.22.188.150", eHost);
//	newServiceIpParams.ProxyAddress.ipVersion  = eIpVersion4;
	memcpy(newServiceIpParams.ProxyAddress.addr.v6.ip, tmpProxyAddr.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
	newServiceIpParams.ProxyAddress.ipVersion  = eIpVersion6;

	newServiceIpParams.ProxyAddress.port = 5060;
	newServiceIpParams.refreshTout = 3600;
	newServiceIpParams.RegistrationFlags = 0xFF;
	newServiceIpParams.serversConfig = eConfSipServerManually;
	newServiceIpParams.ServiceId = 3;
	strcpy(newServiceIpParams.ServiceName, "Dummy LCS");
	newServiceIpParams.transportType = eTransportTypeTcp;//eTransportTypeUdp,eTransportTypeTcp

	CSipProxyIpParams*  pServiceParams = new CSipProxyIpParams(newServiceIpParams);

	COstrStream msg;
	pServiceParams->Dump(msg);
	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnIpServiceParamIndSetup3:\n ", msg.str().c_str());

	if(m_servicesList->Insert(pServiceParams)==STATUS_OK) 
	{
	     RemoveActiveAlarmByErrorCode(AA_NO_IP_SERVICE_PARAMS);
	}
	else 
	{
		/*if failure, need to free the memory*/
		POBJDELETE(pServiceParams);
	}

}//JUST TO CHECK THAT IPV4 + IPV6 (BOTH) IS WORKING


void CSipProxyServiceManager::UpdateCardsMngrForLocalIceInit(DWORD serviceId)
{
	PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::UpdateCardsMngrForLocalIceInit");
	eIpType ipType = m_servicesList->GetIpType(serviceId);

	// ICE_INIT_REQ_S
	ICE_SERVER_TYPES_S	*pParams = new ICE_SERVER_TYPES_S;
	memset(pParams,0,sizeof(ICE_SERVER_TYPES_S));

	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pParams->forced_MS_version = (int)(GetSystemCfgFlagHex<DWORD>("MS_ICE_VERSION"));

	if (pParams->forced_MS_version <= MS_TURN_ICE1)
	{
		if (ipType == eIpType_IpV6)
		{
			pParams->forced_MS_version = MS_TURN_ICE2_SHA256_IPv6;
			PTRACE2INT(eLevelInfoNormal, "CStandIceUser::UpdateCM - setting MS_ICE_VERSION according to IP Type IPv6 ", pParams->forced_MS_version);
		}
		else
		{
			pParams->forced_MS_version = MS_TURN_ICE2;
			PTRACE2INT(eLevelInfoNormal, "CStandIceUser::UpdateCM - setting MS_ICE_VERSION according to IP Type IPv4 ", pParams->forced_MS_version);
		}
	}
	else if (pParams->forced_MS_version > MS_TURN_ICE2_SHA256_IPv6)
	{
		PTRACE2INT(eLevelInfoNormal, "CStandIceUser::UpdateCM - user set invalid value for MS_ICE_VERSION setting to ", MS_TURN_ICE2_SHA256_IPv6);
		pParams->forced_MS_version = MS_TURN_ICE2_SHA256_IPv6;
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal, "CStandIceUser::UpdateCM - user set MS_ICE_VERSION to ", pParams->forced_MS_version);
	}

	pParams->forced_MS_version = htonl(pParams->forced_MS_version);

	pParams->ice_env = eIceEnvMs;
	pParams->req_id = 0;

	pParams->service_id = serviceId; //_M_S_


	pParams->isEnableBWPolicyCheck = 0;
	PTRACE2INT(eLevelInfoNormal, "CSipProxyMsSubscriber::UpdateCardsMngr - pParams->forced_MS_version ", pParams->forced_MS_version);

	CSegment*  pSegment = new CSegment;
	pSegment->Put( (BYTE*)pParams, sizeof(ICE_SERVER_TYPES_S) );
	delete pParams;

	CManagerApi apiCards(eProcessCards);
	apiCards.SendMsg(pSegment, SIPPROXY_TO_CARDS_ICE_INIT_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnIpServiceParamIndConnect(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnIpServiceParamIndConnect ");

	CSipProxyIpParams* pService = new CSipProxyIpParams;
	pService->Deserialize(pSeg);
	COstrStream msg;
	pService->Dump(msg);
	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnIpServiceParamIndConnect:\n ", msg.str().c_str());

	// Checks whether the received service changes an existing service params and handle the changes if needed
	HandleChangesInServiceIfNeeded(pService);

	if (m_servicesList->Insert(pService) == STATUS_OK)
	{
		if(pService->GetIsMsICE())
			CreateICEUserInDB(pService->GetServiceId(),TRUE);
		else if(pService->GetSipServerType() == eSipServer_ms) // ms server but ice is not configured
		{
			// init ice
			UpdateCardsMngrForLocalIceInit(pService->GetServiceId());

			CreateDummyConfIfDBisEmpty(pService->GetServiceId());
		}
		else
			CreateDummyConfIfDBisEmpty(pService->GetServiceId());

		if (IsValidTimer(TIMER_RETRY_ASK_CONFS_DB))
			DeleteTimer(TIMER_RETRY_ASK_CONFS_DB);
		CConfPartyManagerApi ConfPartyApi;
		ConfPartyApi.SipProxyDBRequest();

		RemoveActiveAlarmByErrorCode(AA_NO_IP_SERVICE_PARAMS);
	}
	else 
	{
		/*If failure, need to free the memory*/
		POBJDELETE(pService);
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnIpServiceParamDel(CSegment* pSeg)
{
	Del_Ip_Service_S *param = (Del_Ip_Service_S*)pSeg->GetPtr();
	char s[16];
	*s = '\0';
	sprintf(s, "%d", param->service_id);
	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnIpServiceParamDelSetup serviceId:", s);
	m_servicesList->Remove(param->service_id);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnSC_upInd(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnSC_upInd");
	m_SCisUp = TRUE;

	if(m_pProxyService)
	{
		//eIceEnvironmentType Type = m_pProxyService->GetICEType();
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::OnSC_upInd Type: ",m_pProxyService->GetICEType());
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::OnSC_upInd m_pICEserviceId: ", m_pICEserviceId);
	}
	else
		PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnSC_upInd - m_pProxyService not Valid");

	BYTE bIsMultipleServices =  GetMultipleServices();
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if((YES == bIsMultipleServices && m_mediaCardSetupEndWasReceived) || (NO == bIsMultipleServices)
			|| eProductTypeGesher==curProductType || eProductTypeNinja == curProductType)
	{
		if(m_serviceEndWasReceived && m_DNSConfigWasReceived)
		{
			PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnSC_upInd Enter to SipProxyConnect ");
			m_state = CONNECT;
			SipProxyConnect();
		}
	}


}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnIpServiceParamEnd(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnIpServiceParamEnd");
	RemoveActiveAlarmByErrorCode(AA_NO_IP_SERVICE_PARAMS);

	if(m_pProxyService)
	{
		//eIceEnvironmentType Type = m_pProxyService->GetICEType();
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::OnIpServiceParamEnd Type: ", m_pProxyService->GetICEType());
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::OnIpServiceParamEnd m_pICEserviceId: ", m_pICEserviceId);

	}
	else
		PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnIpServiceParamEnd - m_pProxyService not Valid");

	m_serviceEndWasReceived = TRUE;
	BYTE bIsMultipleServices =  GetMultipleServices();
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if((YES == bIsMultipleServices && m_mediaCardSetupEndWasReceived) || (NO == bIsMultipleServices)
			|| eProductTypeGesher==curProductType || eProductTypeNinja == curProductType)
	{
		if(m_SCisUp && m_DNSConfigWasReceived)
		{
			PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnIpServiceParamEnd Enter to sipConnect  ");
			m_state = CONNECT;
			SipProxyConnect();
		}
	}
}
/////////////////////////////////////////////////////////////////////////
void 	CSipProxyServiceManager::McuMngrConfigSetup()
{
	BYTE bIsMultipleServices =  GetMultipleServices();
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if((YES == bIsMultipleServices && m_mediaCardSetupEndWasReceived) || (NO == bIsMultipleServices)
			|| eProductTypeGesher==curProductType || eProductTypeNinja == curProductType)
		{
			if(m_SCisUp && m_serviceEndWasReceived)
			{
				m_state = CONNECT;
				SipProxyConnect();
			}
		}
}
/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnMcuMngrConfigSetup(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnMcuMngrConfigSetup");
	m_DNSConfigWasReceived = TRUE;

	WORD flag = (WORD)eDnsConfigurationFailure;
	*pSeg >> flag;

	if(eDnsConfigurationSuccess == flag)
		m_DNSStatus = TRUE;
	else
		m_DNSStatus = FALSE;

	McuMngrConfigSetup();
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnMcuMngrConfigCONNECT(CSegment* pSeg)
{
	WORD flag = (WORD)eDnsConfigurationFailure;
	*pSeg >> flag;

	if(eDnsConfigurationSuccess == flag)
		m_DNSStatus = TRUE;
	else
		m_DNSStatus = FALSE;

	UpdateDNSStatusAtAllEntries();
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnMediaCardSetupEnd(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnMediaCardSetupEnd");

	m_mediaCardSetupEndWasReceived = TRUE;

	if(m_SCisUp && m_serviceEndWasReceived && m_DNSConfigWasReceived)
	{
		m_state = CONNECT;
		SipProxyConnect();
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::SipProxyConnect()
{
	int errorFound = 0, unreg = 0, unregOne = 0, indInDB = -1, indInConfs = -1, i = 0, change = 0, gChange = 0;
	char strHostName[H243_NAME_LEN]; // B.S. klocwork 2582 instead of allocbuff

	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::SipProxyConnect");
	PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::SipProxyConnect ", m_DNSStatus);

	CSipProxyIpParams* pService = m_servicesList->GetFirstService();
	DWORD serviceId = 0;
	if(pService)
		serviceId = pService->GetServiceId();
	else
	{

		if((m_ServiceIceType == eIceEnvironment_Standard) && (m_pProxyService))
		{
			eIceEnvironmentType Type = m_pProxyService->GetICEType();
			PTRACE2INT(eLevelInfoHigh, "CSipProxyManager::SipProxyConnect Type: ", Type);
			PTRACE2INT(eLevelInfoHigh, "CSipProxyManager::SipProxyConnect m_pICEserviceId: ", m_pICEserviceId);


			CreateICEUserInDB(m_pICEserviceId, FALSE);
			PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::SipProxyConnect - Start Ice standard session");

		} else
		{
			PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::SipProxyConnect - no service defined");
		}
		return;
	}

	for(i = 0; i < MAX_CONF_REGISTRATIONS; i++)
	{
		if(m_pConfRegDB[i] && !m_pConfRegDB[i]->m_remove
			&& CONF_UNREGISTERING != m_pConfRegDB[i]->m_status
			&& CONF_UNREGISTERING_DNS_RESOLVING != m_pConfRegDB[i]->m_status)
		{
			if(serviceId == m_pConfRegDB[i]->m_serviceId || m_pConfRegDB[i]->m_serviceId == (DWORD)-1)
				continue;
			else
				InsertConfToDB( m_pConfRegDB[i]->m_confID, m_pConfRegDB[i]->m_pConfName
				, (WORD)-1, m_pConfRegDB[i]->m_expires, m_UseMainProxy, FALSE
				, m_pConfRegDB[i]->m_bIsOngoing, m_pConfRegDB[i]->m_bIsMR, m_pConfRegDB[i]->m_bIsEQ
				, m_pConfRegDB[i]->m_bIsFactory, m_pConfRegDB[i]->m_bIsGWProfile,m_pConfRegDB[i]->m_sipServerType, m_Dummy_name);
		}
	}

	for (i = 0; i < MAX_CONF_REGISTRATIONS; i++)
	{
		//find unregistered or those registered on the same card before reset.
		if (m_pConfRegDB[i] && m_pConfRegDB[i]->m_serviceId == (DWORD)-1)
		{
			if(CONF_IDLE == m_pConfRegDB[i]->m_status && !m_pConfRegDB[i]->m_remove)
			{
				//if entry is from a crashed card
		//		if(m_pConfRegDB[i]->m_serviceId != (DWORD)-1 && m_pConfRegDB[i]->m_proxyAddress.addr.v4.ip != 0)
				if( m_pConfRegDB[i]->m_serviceId != (DWORD)-1 && !isApiTaNull(&m_pConfRegDB[i]->m_proxyAddress) ) {

						//if service updated, and entry was assigned for a proxy
						//if only transport type changed (TCP / UDP), no need to unregister
						if((unreg || unregOne)
							&& (!isApiTaNull(&m_pConfRegDB[i]->m_proxyAddress) || m_pConfRegDB[i]->m_pHostName[0]!='\0')
							&& (change != 0x1000))
						{
							//add a new entry with new data for new register
							indInDB = InsertConfToDB( m_pConfRegDB[i]->m_confID, m_pConfRegDB[i]->m_pConfName
								, serviceId, m_pConfRegDB[i]->m_expires, m_UseMainProxy, FALSE
								, m_pConfRegDB[i]->m_bIsOngoing, m_pConfRegDB[i]->m_bIsMR, m_pConfRegDB[i]->m_bIsEQ
								, m_pConfRegDB[i]->m_bIsFactory, m_pConfRegDB[i]->m_bIsGWProfile,m_pConfRegDB[i]->m_sipServerType, m_Dummy_name);

							// (create a new conf state-machine (insert to list))
							indInConfs = m_confList->CreateConfCntrl( m_pConfRegDB[indInDB], m_pRcvMbx, m_pMockMplProtocol);
						}
						//if card start-up
						else
						{
							memset(strHostName, 0, H243_NAME_LEN); // B.S. klocwork 2582 instead of allocbuff
							m_pConfRegDB[i]->m_serviceId = serviceId;
							eIpType ipType = m_servicesList->GetIpType(serviceId);
							m_pConfRegDB[i]->m_ipType = ipType;
							//if (m_servicesList->GetIpVersion(serviceId) == eIpVersion4)
							/*if (eIpType_Both == ipType || eIpType_IpV4 == ipType)
							{
								m_pConfRegDB[i]->m_serviceAddressIpV4.addr.v4.ip = m_servicesList->GetIpV4(serviceId);
								m_pConfRegDB[i]->m_serviceAddressIpV4.ipVersion = eIpVersion4;
							}
							//else if(m_servicesList->GetIpVersion(serviceId) == eIpVersion6)
							if (eIpType_Both == ipType || eIpType_IpV6 == ipType)
							{
								memcpy(m_pConfRegDB[i]->m_serviceAddressIpV6.addr.v6.ip, m_servicesList->GetIpV6(serviceId), IPV6_ADDRESS_BYTES_LEN);
								m_pConfRegDB[i]->m_serviceAddressIpV6.addr.v6.scopeId = m_servicesList->GetIpV6scopeid(serviceId);
								m_pConfRegDB[i]->m_serviceAddressIpV6.ipVersion = eIpVersion6;
							}*/
							for(int index=0; index<TOTAL_NUM_OF_IP_ADDRESSES; index++)
							{
								ipAddressStruct ipAddrSt = m_servicesList->GetServiceIpAddress(serviceId,index);
								m_pConfRegDB[i]->SetServiceAddresses(ipAddrSt,index);
							}

							m_pConfRegDB[i]->m_transportType = m_servicesList->GetTransportType(serviceId);
							m_pConfRegDB[i]->m_serversConfig = m_servicesList->GetServersConfig(serviceId);
							if (m_servicesList->GetOutboundProxyIpVersion(serviceId) == eIpVersion4)
							{
								m_pConfRegDB[i]->m_outboundProxyAddress.addr.v4.ip = m_servicesList->GetOutboundProxyIpV4(serviceId);
								m_pConfRegDB[i]->m_outboundProxyAddress.ipVersion = eIpVersion4;
							}
							else if (m_servicesList->GetOutboundProxyIpVersion(serviceId) == eIpVersion6)
							{
								char * pOutboundProxyIpV6 = m_servicesList->GetOutboundProxyIpV6(serviceId);

								if (pOutboundProxyIpV6==NULL)
									PTRACE(eLevelError, "GetOutboundProxyIpV6 returned NULL");
								else
									memcpy(m_pConfRegDB[i]->m_outboundProxyAddress.addr.v6.ip, pOutboundProxyIpV6, IPV6_ADDRESS_BYTES_LEN);
								m_pConfRegDB[i]->m_outboundProxyAddress.addr.v6.scopeId = m_servicesList->GetOutboundProxyV6scopeid(serviceId);
								m_pConfRegDB[i]->m_outboundProxyAddress.ipVersion = eIpVersion6;
							}
							m_pConfRegDB[i]->m_outboundProxyAddress.port = m_servicesList->GetOutboundProxyPort(serviceId);

							char * pOutboundProxyName = m_servicesList->GetOutboundProxyName(serviceId);
							if (pOutboundProxyName==NULL)
								PTRACE(eLevelError, "GetOutboundProxyName returned NULL");
							else
								strncpy(m_pConfRegDB[i]->m_poutboundProxyName, pOutboundProxyName, H243_NAME_LEN);

							if(m_UseMainProxy)
							{
								if (m_servicesList->GetProxyIpVersion(serviceId) == eIpVersion4)
								{
									m_pConfRegDB[i]->m_proxyAddress.addr.v4.ip = m_servicesList->GetProxyIpV4(serviceId);
									m_pConfRegDB[i]->m_proxyAddress.ipVersion = eIpVersion4;
								}
								else if (m_servicesList->GetProxyIpVersion(serviceId) == eIpVersion6)
								{
									char * pProxyIpV6 = m_servicesList->GetProxyIpV6(serviceId);
									if (pProxyIpV6==NULL)
										PTRACE(eLevelError, "GetProxyIpV6 returned NULL");
									else
										memcpy(m_pConfRegDB[i]->m_proxyAddress.addr.v6.ip, pProxyIpV6, IPV6_ADDRESS_BYTES_LEN);

									m_pConfRegDB[i]->m_proxyAddress.addr.v6.scopeId = m_servicesList->GetProxyV6scopeid(serviceId);
									m_pConfRegDB[i]->m_proxyAddress.ipVersion = eIpVersion6;
								}
								m_pConfRegDB[i]->m_proxyAddress.port = m_servicesList->GetProxyPort(serviceId);

								char * pProxyName = m_servicesList->GetProxyName(serviceId);
								if (pProxyName==NULL)
									PTRACE(eLevelError, "GetProxyName returned NULL");
								else
									strncpy(m_pConfRegDB[i]->m_pProxyName, pProxyName, H243_NAME_LEN);

								char *pRegHostName = m_servicesList->GetRegHostName(serviceId);
								if (pRegHostName==NULL)
									PTRACE(eLevelError, "GetRegHostName returned NULL");
								else
								{
									strncpy(strHostName, pRegHostName, H243_NAME_LEN -1);
									strHostName[H243_NAME_LEN -1]='\0';
								}
							}
							else
							{
								if (m_servicesList->GetAlternateProxyIpVersion(serviceId) == eIpVersion4)
								{
									m_pConfRegDB[i]->m_proxyAddress.addr.v4.ip = m_servicesList->GetAlternateProxyIpV4(serviceId);
									m_pConfRegDB[i]->m_proxyAddress.ipVersion = eIpVersion4;
								}
								else if (m_servicesList->GetAlternateProxyIpVersion(serviceId) == eIpVersion6)
								{
									char * pAlternateProxyIpV6 = m_servicesList->GetAlternateProxyIpV6(serviceId);
									if (pAlternateProxyIpV6==NULL)
										PTRACE(eLevelError, "GetAlternateProxyIpV6 returned NULL");
									else
										memcpy(m_pConfRegDB[i]->m_proxyAddress.addr.v6.ip, pAlternateProxyIpV6, IPV6_ADDRESS_BYTES_LEN);
									m_pConfRegDB[i]->m_proxyAddress.addr.v6.scopeId = m_servicesList->GetAlternateProxyV6scopeid(serviceId);
									m_pConfRegDB[i]->m_proxyAddress.ipVersion = eIpVersion6;
								}
								m_pConfRegDB[i]->m_proxyAddress.port = m_servicesList->GetAlternateProxyPort(serviceId);
								if(m_servicesList->GetAltProxyName(serviceId))
								{
									char *pAltProxyName = m_servicesList->GetAltProxyName(serviceId);
									if (pAltProxyName==NULL)
										PTRACE(eLevelError, "pAltProxyName returned NULL");
									else
										strncpy(m_pConfRegDB[i]->m_pProxyName, pAltProxyName, H243_NAME_LEN);
								}
								if(m_servicesList->GetAltRegHostName(serviceId))
								{
									char *pAltRegHostName = m_servicesList->GetAltRegHostName(serviceId);
									if (pAltRegHostName==NULL)
										PTRACE(eLevelError, "GetAltRegHostName returned NULL");
									else
									{
										strncpy(strHostName, pAltRegHostName, H243_NAME_LEN -1);
										strHostName[H243_NAME_LEN -1]='\0';
									}
								}
							}

							 // B.S. klocwork 2582
							strncpy(m_pConfRegDB[i]->m_pHostName,strHostName, H243_NAME_LEN);
							m_pConfRegDB[i]->m_DNSStatus = m_DNSStatus;

							DWORD refreshTout = 3600;
							if(m_servicesList->find(serviceId))
								refreshTout = m_servicesList->GetRefreshTout(serviceId);

							m_pConfRegDB[i]->m_expires = refreshTout;

							// (create a new conf state-machine (insert to list))
							indInConfs = m_confList->CreateConfCntrl( m_pConfRegDB[i], m_pRcvMbx, m_pMockMplProtocol);
						}
			    }

						if (-1 == indInConfs) {
							RemoveFromConfDB( i );
							errorFound = ADD_CONF_CNTL_CREATE_ERROR;
						}

						//if entry is from a crashed card
						if(unregOne)
						{
							m_confList->Unregister(i);
							//m_confList->DeleteConfCntrl(i);
							//RemoveFromConfDB(i);

							unregOne = 0;
						}

						if(unreg)
						{
							if(m_pConfRegDB[i] &&
								(!isApiTaNull(&m_pConfRegDB[i]->m_proxyAddress) || m_pConfRegDB[i]->m_pHostName[0]!='\0'))
								if(CONF_IDLE == m_pConfRegDB[i]->m_status)
									m_confList->Unregister(i);
						}

						//so we can re-register with new transport type
						if(change == 0x1000)
							m_confList->ResetConfCntl(indInConfs);

						// (start registration in the Proxy Registrar Server)
						if (!errorFound)
							errorFound = StartRegister(indInConfs);
			}
		}
	}//end for
	CreateICEUserInDB(serviceId,TRUE);

	CreateDummyConfIfDBisEmpty(serviceId);
	if (IsValidTimer(TIMER_RETRY_ASK_CONFS_DB))
		DeleteTimer(TIMER_RETRY_ASK_CONFS_DB);

	//printf("CSipProxyServiceManager::SipProxyConnect service %d\n",m_ServiceId );
	CConfPartyManagerApi ConfPartyApi;
	ConfPartyApi.SipProxyDBRequest();
}


/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::Dump(COstrStream& msg) const
{
	int i = 0, non = 1;

	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "CSipProxyServiceManager::Dump\n";
	msg << "---------------------------------------------------------------------------\n";

	DWORD j = 0;

	msg << "------------------------------------------------------------------------------------------\n";
	msg << "\tConf\t| Status\t\t| OutboundProxyIp | RegistrarIp | RegistrarPort | \n";
	msg << "------------------------------------------------------------------------------------------\n";
	for (i = 0; i < MAX_CONF_REGISTRATIONS; i++)
	{
		if (m_pConfRegDB[i])/* && m_pConfRegDB[i]->m_serviceId == j) */
		{
			non = 0;
			msg << i << " : " << m_pConfRegDB[i]->m_pConfName << "\t| ";
			switch(m_pConfRegDB[i]->m_status)
			{
			case(1):
				{
					msg << "Idle\t\t\t| ";
					break;
				}
			case(2):
				{
					msg << "Registering\t\t| ";
					break;
				}
			case(3):
				{
					msg << "Registered\t\t| ";
					break;
				}
			case(4):
				{
					msg << "Unregistering\t| ";
					break;
				}
			default:
				{
					msg << m_pConfRegDB[i]->m_status <<"\t| ";
					break;
				}
			}
			if(eIpVersion4 == m_pConfRegDB[i]->m_outboundProxyAddress.ipVersion)
			{
				char OutproxyIp[16];
				SystemDWORDToIpString(m_pConfRegDB[i]->m_outboundProxyAddress.addr.v4.ip, OutproxyIp);
				msg << OutproxyIp;
				msg <<"\t|";
			}
			else if(eIpVersion6 == m_pConfRegDB[i]->m_outboundProxyAddress.ipVersion)
			{
				msg << m_pConfRegDB[i]->m_outboundProxyAddress.addr.v6.ip;
				msg <<"\t|";

			}
			if(eIpVersion4 == m_pConfRegDB[i]->m_proxyAddress.ipVersion)
			{
				char proxyIp[16];
				SystemDWORDToIpString(m_pConfRegDB[i]->m_proxyAddress.addr.v4.ip, proxyIp);
				msg << proxyIp << "\t|";
			}
			else if(eIpVersion6 == m_pConfRegDB[i]->m_proxyAddress.ipVersion)
			{
				msg << m_pConfRegDB[i]->m_proxyAddress.addr.v6.ip;
				msg <<"\t|";
			}
			if(m_pConfRegDB[i]->m_remove)
				msg << "\t (marked as garbage)";

			msg << m_pConfRegDB[i]->m_proxyAddress.port;
			if(m_pConfRegDB[i]->m_bIsFromOldService)
				msg << "\t (From old service)";

			msg <<"\t\t |\n";

		}
	} //end for

	if(non == 1)
		msg<<"No Registrations exist.\n";
	else
		non = 1;
	msg << "\n";

	/*		if(-1 == j)
				break;*/
	//	}

//	PTRACE(eLevelInfoNormal,msg.str());
}

//////////////////////////////////////////////////////////////////////
/*void CSipProxyServiceManager::OnCSApiMsg(CSegment *pSeg)
{
	CMplMcmsProtocol* mplMcmsProtocol = new CMplMcmsProtocol;
	mplMcmsProtocol->DeSerialize(*pSeg, CS_API_TYPE);
	CSipProxyMplMcmsProtocolTracer(*mplMcmsProtocol).TraceMplMcmsProtocol("***CSipProxyServiceManager::OnCSApiMsg",CS_API_TYPE);

	OPCODE opcode = mplMcmsProtocol->getOpcode();

	POBJDELETE(mplMcmsProtocol);

	pSeg->ResetRead();
	DispatchEvent(opcode, pSeg);
}*/

/////////////////////////////////////////////////////////////////////////////
// Checks whether the received service changes an existing service params and handle the changes if needed.
void  CSipProxyServiceManager::HandleChangesInServiceIfNeeded(CSipProxyIpParams* pReceivedService)
{
	BYTE bUnregisterAll = FALSE;
	CSipProxyIpParams* pExistingService = NULL;
	DWORD serviceId = 0;
	mcTransportAddress newAdd, oldAdd;
	if(pReceivedService)
		serviceId = pReceivedService->GetServiceId();

	pExistingService = m_servicesList->find(serviceId);

	// check if it is new server - only add the new server.
	if (pExistingService == NULL)
	{
		PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::HandleChangesInServiceIfNeeded : New service. Not need to handle changes in an existing service");
		return;
	}

	COstrStream msg;
	if (m_UseMainProxy)
		msg << "Use Primary server";
	else
		msg << "Use Alternate server";
	msg << "\nExisiting service:\n";
	pExistingService->Dump(msg);
	msg << "New service:\n";
	if(pReceivedService)
		pReceivedService->Dump(msg);
	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::HandleChangesInServiceIfNeeded:\n", msg.str().c_str());

	// sip server is set to off - unregister all. The service will be removed and we won't request conf db from ConfParty.
	if (pReceivedService && pReceivedService->GetServersConfig() == eConfSipServerAuto)
	{
		PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::HandleChangesInServiceIfNeeded - Sip server is set to off");
		bUnregisterAll = TRUE;
	}

	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::HandleChangesInServiceIfNeeded - Temporary: unregister all always!");
	bUnregisterAll = FALSE;

	if( pReceivedService )
	{
		// Changing in outbound parameters - unregister all. The ReRgister will be done by requsting the confs db from ConfParty after update the service
		newAdd = pReceivedService->GetOutboundProxyAddress();
		oldAdd = pExistingService->GetOutboundProxyAddress();
		if ((!(::isIpAddressEqual(&newAdd, &oldAdd)))
			|| (pReceivedService->GetOutboundProxyPort() != pExistingService->GetOutboundProxyPort()))
		{
			PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::HandleChangesInServiceIfNeeded - Changes in outbound proxy");
			bUnregisterAll = TRUE;
		}

		// Changing in Primary registrar parameters when using the primary registrar - unregister all. The ReRgister will be done by requsting the confs db from ConfParty after update the service
		newAdd = pReceivedService->GetProxyAddress();
		oldAdd = pExistingService->GetProxyAddress();
		if ((m_UseMainProxy)
			&& ((!(::isIpAddressEqual(&newAdd, &oldAdd)))
				|| (pReceivedService->GetProxyPort() != pExistingService->GetProxyPort())
				|| (strncmp(pReceivedService->GetProxyHostName(), pExistingService->GetProxyHostName(), H243_NAME_LEN))))
		{
			PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::HandleChangesInServiceIfNeeded - Changes in primary registrar when using primary registrar");
			bUnregisterAll = TRUE;
		}

		// Changing in Alternate registrar parameters when using the alternate registrar - unregister all. The ReRgister will be done by requsting the confs db from ConfParty after update the service
		newAdd = pReceivedService->GetAltProxyAddress();
		oldAdd = pExistingService->GetAltProxyAddress();
		if ((!m_UseMainProxy)
			&& ((!(::isIpAddressEqual(&newAdd, &oldAdd)))
				|| (pReceivedService->GetAltProxyPort() != pExistingService->GetAltProxyPort())
				|| (strncmp(pReceivedService->GetAltProxyHostName(), pExistingService->GetAltProxyHostName(), H243_NAME_LEN))))
		{
			PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::HandleChangesInServiceIfNeeded - Changes in alternate registrar when using alternate registrar");
			bUnregisterAll = TRUE;
		}
	}

	if (bUnregisterAll)
	{
		// unregister and delete all
		UnregisterAndDeleteAll(serviceId);
	}
	else if(pReceivedService)// check other parameters
	{
		// Changing in registration flags - unregister the conference types that shound not be registered. The Rgister of new conferences type will be done by requsting the confs db from ConfParty after update the service
		BYTE unregisterOnGoing = (!pReceivedService->IsRegOnGoing() && pExistingService->IsRegOnGoing()) ? TRUE : FALSE;
		BYTE unregisterMR = (!pReceivedService->IsRegMeetingRooms() && pExistingService->IsRegMeetingRooms()) ? TRUE : FALSE;
		BYTE unregisterEQ = (!pReceivedService->IsRegEQs() && pExistingService->IsRegEQs()) ? TRUE : FALSE;
		BYTE unregisterFactory = (!pReceivedService->IsRegFactories() && pExistingService->IsRegFactories()) ? TRUE : FALSE;
		BYTE unregisterGW = (!pReceivedService->IsRegGWProfiles() && pExistingService->IsRegGWProfiles()) ? TRUE : FALSE;
		BYTE unregisterICEUser = (!pReceivedService->GetIsMsICE() && pExistingService->GetIsMsICE()) ? TRUE : FALSE;
		if (unregisterOnGoing || unregisterMR || unregisterEQ || unregisterFactory || unregisterGW || unregisterICEUser)
		{
			PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::HandleChangesInServiceIfNeeded - Changes in conference registration flags");
			UnregisterAndDeleteAccordingToConfTypes(serviceId, unregisterOnGoing, unregisterMR, unregisterEQ, unregisterFactory, FALSE, unregisterGW, unregisterICEUser);
		}

		// Changing in refresh registeration time out - update the expired time of all conferences that are not registered
		if (pReceivedService->GetRefreshTout() != pExistingService->GetRefreshTout())
		{
			PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::HandleChangesInServiceIfNeeded - Changes in refresh registration time");
			UpdateExpiredTimeForIdleEntries(serviceId, pReceivedService->GetRefreshTout());
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::UnregisterAndDeleteAll(DWORD serviceId)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::UnregisterAndDeleteAll");
	UnregisterAndDeleteAccordingToConfTypes(serviceId, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::UnregisterAndDeleteAccordingToConfTypes(DWORD serviceId, BYTE delOngoings, BYTE delMRs, BYTE delEQs, BYTE delFactories, BYTE delDiscovery, BYTE delGWProfiles,BYTE delIceUser)
{
	CSmallString str;
	str << "Unregister:";
	if (delOngoings)
		str << " Ongoings";
	if (delMRs)
			str << " MRs";
	if (delEQs)
			str << " EQs";
	if (delFactories)
			str << " Factories";
	if (delFactories)
			str << " Discovery";
	if (delGWProfiles)
			str << " GWProfiles";
	if(delIceUser)
			str << " IceUser";

	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::UnregisterAndDeleteAccordingToConfTypes : ", str.GetString());

	for(int i=0; i<MAX_CONF_REGISTRATIONS; i++)
	{
		if(m_pConfRegDB[i] && (m_pConfRegDB[i]->m_serviceId == serviceId))
		{
			if ((m_pConfRegDB[i]->m_bIsOngoing && delOngoings)
				|| (m_pConfRegDB[i]->m_bIsMR && delMRs)
				|| (m_pConfRegDB[i]->m_bIsEQ && delEQs)
				|| (m_pConfRegDB[i]->m_bIsFactory && delFactories)
				|| (m_pConfRegDB[i]->m_bIsDiscovery && delDiscovery)
				|| (m_pConfRegDB[i]->m_bIsGWProfile && delGWProfiles)
				|| ((m_pConfRegDB[i]->m_bIsMSIceUser || m_pConfRegDB[i]->m_bIsStandIceUser) && delIceUser))
			{
				if(CONF_IDLE == m_pConfRegDB[i]->m_status)
				{
					PTRACE(eLevelInfoNormal,"CSipProxyManager::UnregisterAndDeleteAccordingToConfTypes  Delete ConfDFromDB");
					DeleteConfCntlAndDB(i);
				}
				else
				{

					PTRACE(eLevelInfoNormal,"CSipProxyManager::UnregisterAndDeleteAccordingToConfTypes  Remove conf");
					m_pConfRegDB[i]->m_bIsFromOldService = TRUE;
					m_confList->Unregister(i);
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::UpdateExpiredTimeForIdleEntries(DWORD serviceId, DWORD newExpires)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::UpdateExpiredTimeForIdleEntries");
	for(int i=0; i<MAX_CONF_REGISTRATIONS; i++)
	{
		if(m_pConfRegDB[i] && (m_pConfRegDB[i]->m_serviceId == serviceId) && (m_pConfRegDB[i]->m_status == CONF_IDLE))
			m_pConfRegDB[i]->m_expires = newExpires;
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::HandleTransportError(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::HandleTransportError Ind\n ");
	for (int i = 0; i < MAX_CONF_REGISTRATIONS; i++)
	{
		if(m_pConfRegDB[i])
		{
			m_confList->SetTransportErrorState(i);
			m_confList->SendCrlfMessageOnTransportError(i);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::HandleRegisterResponse(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::HandleRegisterResponse ");
	DWORD Id = 0xffffffff, dbId = 0xffffffff, status = STATUS_ILLEGAL;
	WORD RegDBStatus = 0, cardId = 0;
	CSegment* pSeg = new CSegment;

	CMplMcmsProtocol  mplMcmsProtocol;
	mplMcmsProtocol.DeSerialize(*pParam, CS_API_TYPE);

	WORD src_unit_id = mplMcmsProtocol.getCentralSignalingHeaderSrcUnitId();
	int	serviceId= mplMcmsProtocol.getCentralSignalingHeaderCsId();
	int	data_len= mplMcmsProtocol.getDataLen();

	mcIndRegisterResp * pRegResponseMsg = (mcIndRegisterResp *)new char[data_len];
	memcpy(pRegResponseMsg, mplMcmsProtocol.GetData(), data_len);

	CSipHeaderList * pHeaders = new CSipHeaderList(pRegResponseMsg->sipHeaders);
	//PASSERT_AND_RETURN(NULL == pHeaders);
	if(NULL == pHeaders)//B.S. klocwork 2593 and 2585
	{
		PASSERT(1);
		POBJDELETE(pSeg);
		PDELETEA(pRegResponseMsg);
		return;
	}
	//Check if this is Microsoft enviroment
    const CSipHeader* pMSDiagnosticsHdr = pHeaders->GetNextHeader(kMsDiagnostics);
    const char* pMSdiagnosticStr = NULL;
    if (pMSDiagnosticsHdr)
        pMSdiagnosticStr = pMSDiagnosticsHdr->GetHeaderStr();

//    CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
    BOOL bSipRegisterOnlyOnce = FALSE;
	BOOL bSipRegisterEnableActiveAlarm = TRUE;
//	pSysConfig->GetBOOLDataByKey("SIP_REGISTER_ONLY_ONCE", bSipRegisterOnlyOnce);
	if( pServiceSysCfg != NULL )
	{
	    pServiceSysCfg->GetBOOLDataByKey(serviceId, "SIP_REGISTER_ONLY_ONCE", bSipRegisterOnlyOnce);
		pServiceSysCfg->GetBOOLDataByKey(serviceId, "ENABLE_SIP_SERVER_OR_REGISTRAR_ERROR_ACTIVE_ALARM", bSipRegisterEnableActiveAlarm);
	}
    if (bSipRegisterOnlyOnce)
        PTRACE (eLevelInfoNormal, "CSipProxyServiceManager::HandleRegisterResponse : SIP_REGISTER_ONLY_ONCE is set to YES");
	if (bSipRegisterEnableActiveAlarm)
		PTRACE (eLevelInfoNormal, "CSipProxyServiceManager::HandleRegisterResponse : ENABLE_SIP_SERVER_OR_REGISTRAR_ERROR_ACTIVE_ALARM is set to YES");

	status = pRegResponseMsg->status;
	Id = pRegResponseMsg->id;
	dbId = Id >> 16;
	if(m_pConfRegDB[dbId])
		RegDBStatus = m_pConfRegDB[dbId]->m_status;

	if(STATUS_OK == status && 0 != RegDBStatus)
	{
		eSipRegistrationConfType sipConfType= eSipRegistrationConfTypeIncorrect;

		PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse : status = STATUS_OK.");
		*pSeg << Id
			  << src_unit_id;
		pSeg->Put((BYTE*)((mcIndRegisterResp *)pRegResponseMsg), data_len) ;
		m_confList->HandleEvent(pSeg, 0, REGISTERING_OK);
		RemoveActiveAlarmByErrorCode(AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_SIP_REGISTRAR);

		//Unregister incase there is no ICE in the system - will remove ICE user from DB
		if(m_pConfRegDB[dbId]->m_bIsMSIceUser && 0 == pRegResponseMsg->expires)
		{
			PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse - DeleteConfCntlAndDB");
			DeleteConfCntlAndDB(dbId);
		}
		else if (m_pConfRegDB[dbId]->m_bIsFromOldService != TRUE)
		{
			sipConfType = SipProxyServiceManagerGetConfType(m_pConfRegDB[dbId]);
			SetRegistrarStatus(m_pConfRegDB[dbId]->m_serviceId , m_pConfRegDB[dbId]->m_pProxyName, status, m_pConfRegDB[dbId]->m_proxyAddress, m_pConfRegDB[dbId]->m_confID,(WORD)sipConfType,pRegResponseMsg->expires, FALSE);
		}

		/* in case of DUMMY_ENTRY set timer for next registration */
		if ((sipConfType == eSipRegistrationConfTypeIncorrect) && (CONF_UNREGISTERING == RegDBStatus) && (m_pConfRegDB[dbId] != NULL)) {

			sipConfType = SipProxyServiceManagerGetConfType(m_pConfRegDB[dbId]);
			PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse - dummy_tester registration response");
			SetRegistrarStatus(m_pConfRegDB[dbId]->m_serviceId , m_pConfRegDB[dbId]->m_pProxyName, status, m_pConfRegDB[dbId]->m_proxyAddress, m_pConfRegDB[dbId]->m_confID,(WORD)sipConfType,pRegResponseMsg->expires, FALSE);

			// Check if need to start timer / replace old timer
			m_pConfRegDB[dbId]->m_expires = pRegResponseMsg->expires;
			TICKS newExpiredTime = (SystemGetTickCount().GetIntegerPartForTrace()) + m_pConfRegDB[dbId]->m_expires*SECOND;
			if(newExpiredTime > GetBusyExpiredTime()) {

				if(IsValidTimer(TIMER_RETRY_PROXY)) {
					DeleteTimer(TIMER_RETRY_PROXY);
					PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse - DeleteTimer");
				}
				PTRACE2INT(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse - Start dummy_tester registration timer=", pRegResponseMsg->expires);
				StartTimer(TIMER_RETRY_PROXY, pRegResponseMsg->expires*SECOND);
				SetBusyExpiredTime(newExpiredTime);
			}
		}

	} else {
		PTRACE2INT(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse : Registration failed, status = ",status);

		//Get Retry-After header value if exist
		DWORD retryAfterVal = 0;
		if (pHeaders)
		{
			const CSipHeader* pRetryAfterHdr = pHeaders->GetNextHeader(kRetryAfter);
		    if (pRetryAfterHdr)
		    {
		    	char cHeaderValue[256] = {0};
		    	strncpy(cHeaderValue, pRetryAfterHdr->GetHeaderStr(), sizeof(cHeaderValue)-1);
		    	cHeaderValue[sizeof(cHeaderValue)-1] = '\0';
		    	retryAfterVal = atoi(cHeaderValue);
		    	if (retryAfterVal)
		    	{
		    		PTRACE2INT(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse : Retry After = ",retryAfterVal);
		       	}
		    }

		    const CSipHeader* pWarningHdr = pHeaders->GetNextHeader(kWarning);

		    if (pWarningHdr)
			{
				char cHeaderValue[256] = {0};
				char *pStr;

			  strncpy(cHeaderValue, pWarningHdr->GetHeaderStr(), sizeof(cHeaderValue)-1);
			  cHeaderValue[sizeof(cHeaderValue)-1] = '\0';

				pStr = strstr(cHeaderValue, "cucm");

				if (pStr)
				{
					PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse, warning: cucm server");

					CConfPartyManagerApi ConfPartyApi;

					ConfPartyApi.SipProxyServerTypeRequest(serviceId, eSipServer_CiscoCucm);
				}
			}
		}

		if(0 != RegDBStatus)
		{
			if((status >= 300 && status <= 599))
			{
                int temp = Id >> 16;
                //In MS enviroment we don't set the active alarm for failed registration
                BYTE bSetActiveAlarm = TRUE;

				DWORD proxyStatus = STATUS_OK;

				//if(408 == status || 477  == status)

				/**
				 * VNGR-20390 - dummy_registration 404 response code shows active alarm + Failure in signaling monitor.
				 * update failed registration in 2 cases.
				 * 	1. Error is received and conf-id is not dummy_tester.
				 *  2. 408 Error is received and conf-id is dummy_tester.
				 *
				 *  408 means that we received timeout, this means that dummy_tester
				 *  which is used as keep-alive mechanism failed to receive an answer.
				 *  in such a case we want an error to appear, as there's no connectivity to proxy.
				 */

				
				if(status == 408 || status == 477 || status == 407 || status==401) // BRIDGE-7115
				{
					proxyStatus = STATUS_FAIL;
					if(bSipRegisterOnlyOnce)
						bSetActiveAlarm = FALSE;
					else
						bSetActiveAlarm = TRUE;
				}
				else
				{
					if(m_pConfRegDB[temp]->m_confID == DUMMY_ENTRY)
						bSetActiveAlarm = FALSE;
					else
					{
						proxyStatus = STATUS_FAIL;
						if(bSipRegisterOnlyOnce)
							bSetActiveAlarm = FALSE;
						else
							bSetActiveAlarm = bSipRegisterEnableActiveAlarm;
					}
				}


				if (m_pConfRegDB[temp]->m_bIsFromOldService != TRUE)
				{
					eSipRegistrationConfType sipConfType= eSipRegistrationConfTypeIncorrect;
					sipConfType = SipProxyServiceManagerGetConfType(m_pConfRegDB[dbId]);

					SetRegistrarStatus(m_pConfRegDB[temp]->m_serviceId , m_pConfRegDB[temp]->m_pProxyName,
					                       proxyStatus, m_pConfRegDB[temp]->m_proxyAddress, m_pConfRegDB[temp]->m_confID,(WORD)sipConfType,pRegResponseMsg->expires, bSetActiveAlarm);
				}
				*pSeg << Id;
				pSeg->Put((BYTE*)((mcIndRegisterResp *)pRegResponseMsg), data_len);
				m_confList->HandleEvent(pSeg, 0, REGISTERING_FAILED);

				int dummyInDB = FindConfInDB( 0, m_Dummy_name, m_pConfRegDB[temp]->m_serviceId );
                {//---Sasha Sh. Disable/Enable dummy_tester1  REGISTER---------------------------------------------//
                    BOOL bIsDisableDummyRegistration = 0;
                    std::string key = "DISABLE_DUMMY_REGISTRATION";
                    CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key, bIsDisableDummyRegistration);
                    if(true == bIsDisableDummyRegistration)
                    {
                        PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::CreateDummyConf - Flag:DISABLE.");
                        dummyInDB = -1;
                    }
                }//-----------------------------------------------------------------------------------------------//
				if( (pRegResponseMsg->expires != 0 || (0 == pRegResponseMsg->expires && dummyInDB == temp))
                                    && !m_pConfRegDB[temp]->m_bIsMSIceUser )
				{
					PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse - verify1");
					//if failed registration is not marked for removal or not from old service params
					if(m_pConfRegDB[temp] && !m_pConfRegDB[temp]->m_remove && !m_pConfRegDB[temp]->m_bIsFromOldService && !m_pConfRegDB[temp]->m_bIsDiscovery)
					{
						//4XX
						if(status >= 400 && status <= 499)
						{
							switch(status)
							{
								case(400):
								case(404):
								case(408):
								case(409):
								case(411):
								case(414):
								case(421):
								case(477):
								case(486):
								{
									//RegConfOnAlternate(temp);
									MoveAllRegsToAlternate(m_pConfRegDB[temp]->m_serviceId, temp);
									break;
								}
								case(401):
								case(402):
								case(403):
								case(405):
								case(406):
								case(407):
								case(410):
								case(488):
								{
									MoveAllRegsToAlternate(m_pConfRegDB[temp]->m_serviceId, temp);
									break;
								}
								case(413):
								case(415):
								case(420):
								case(481):
								case(482):
								case(483):
								case(484):
								case(485):
								case(487):
								{
									PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse : Registration failed, status is unexpected for REGISTER request.");
									PASSERT(1);
									//RegConfOnAlternate(temp);
									MoveAllRegsToAlternate(m_pConfRegDB[temp]->m_serviceId, temp);
									break;
								}
								case(423):
								{
									m_pConfRegDB[temp]->m_expires = 3600;
									StartRegister(temp);
									break;
								}
								case(480):
								{
									if (retryAfterVal)
									{
										PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse : 480 received-SetServerBusyStartRetryTimer.");
										SetServerBusyStartRetryTimer(temp, retryAfterVal);  // first set server busy in all confs that use the busy server
									}
									else
									{
										PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse : 480 received. retryAfterVal is empty.");
										SetServerBusyStartRetryTimer(temp, RetryTimeOut);  // first set server busy in all confs that use the busy server
									}
								}
								default:
								{
									//RegConfOnAlternate(temp);
									MoveAllRegsToAlternate(m_pConfRegDB[temp]->m_serviceId, temp);
									break;
								}
							}
						}
						else //5XX server failure and 3XX
						{

							if (retryAfterVal)
							{
								SetServerBusyStartRetryTimer(temp, retryAfterVal);  // first set server busy in all confs that use the busy server
								if (retryAfterVal > RETRY_AFTER_THRESHOLD) // if retry after value is large, move conferences to alternate.
									MoveAllRegsToAlternate(m_pConfRegDB[temp]->m_serviceId, temp);
								//RegConfOnAlternate(temp, FALSE); // no need to start retry timer
							}
							else if(504 == status)
								//RegConfOnAlternate(temp);
								MoveAllRegsToAlternate(m_pConfRegDB[temp]->m_serviceId, temp);
							else
							{
								MoveAllRegsToAlternate(m_pConfRegDB[temp]->m_serviceId, temp);
							}
						}
					}
				}
			}
			else
				DBGPASSERT(1);
		}
	}

	//set card properties
	if(m_pConfRegDB[dbId])
		cardId = m_pConfRegDB[dbId]->m_serviceId;

	Id = Id >> 16;

	//if received OK for Unregister request
	if(Id<MAX_CONF_REGISTRATIONS)// && pRegResponseMsg->expires == 0)
		if(m_pConfRegDB[Id] && CONF_UNREGISTERING == RegDBStatus)
		{
			PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse, received OK for unregister" );
			//if registration is moved between cards, re-register it (this time in the dest card)
			if(m_pConfRegDB[Id]->m_move == TRUE)
			{
				PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse - verify2");
				m_pConfRegDB[Id]->m_move = FALSE;

				eIpType ipType = m_servicesList->GetIpType(m_pConfRegDB[Id]->m_serviceId);
				m_pConfRegDB[Id]->m_ipType = ipType;
				/*
				//if(m_servicesList->GetIpVersion(m_pConfRegDB[Id]->m_serviceId) == eIpVersion4)
				if (eIpType_Both == ipType || eIpType_IpV4 == ipType)
				{
					m_pConfRegDB[Id]->m_serviceAddressIpV4.addr.v4.ip = m_servicesList->GetIpV4(m_pConfRegDB[Id]->m_serviceId);
					m_pConfRegDB[Id]->m_serviceAddressIpV4.ipVersion = eIpVersion4;

				}
				//else if(m_servicesList->GetIpVersion(m_pConfRegDB[Id]->m_serviceId) == eIpVersion6)
				if (eIpType_Both == ipType || eIpType_IpV6 == ipType)
				{
					memcpy(m_pConfRegDB[Id]->m_serviceAddressIpV6.addr.v6.ip, m_servicesList->GetIpV6(m_pConfRegDB[Id]->m_serviceId), IPV6_ADDRESS_BYTES_LEN);
					m_pConfRegDB[Id]->m_serviceAddressIpV6.addr.v6.scopeId = m_servicesList->GetIpV6scopeid(m_pConfRegDB[Id]->m_serviceId);
					m_pConfRegDB[Id]->m_serviceAddressIpV6.ipVersion = eIpVersion6;
				}
				*/
				for(int index=0; index<TOTAL_NUM_OF_IP_ADDRESSES; index++)
				{
					ipAddressStruct ipAddrSt = m_servicesList->GetServiceIpAddress(m_pConfRegDB[Id]->m_serviceId,index);
					m_pConfRegDB[Id]->SetServiceAddresses(ipAddrSt,index);
				}

				StartRegister(Id);
			}
			//else - unregistration is finished - remove it.
			else
			{
				PTRACE2INT(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse - m_pConfRegDB[Id]->m_move=", m_pConfRegDB[Id]->m_move);
				if(!m_pConfRegDB[Id]->m_bIsDiscovery) {
					PTRACE2INT(eLevelInfoNormal,"CSipProxyServiceManager::HandleRegisterResponse - m_pConfRegDB[Id]->m_bIsDiscovery=", m_pConfRegDB[Id]->m_bIsDiscovery);
					DeleteConfCntlAndDB(Id);
				}

			}
		}
    if (pMSdiagnosticStr && strcmp(pMSdiagnosticStr, "")) //||bSipRegisterOnlyOnce
    {
        if(m_pConfRegDB[Id])
        {
            if(CONF_IDLE != m_pConfRegDB[Id]->m_status && m_pConfRegDB[Id]->m_bIsDiscovery)
            {
                m_confList->StopReg(Id);
                m_pConfRegDB[Id]->m_status = CONF_IDLE;
                m_confList->ResetConfCntl(Id);
                int indInDB = FindConfInDB( 0, m_pConfRegDB[Id]->m_pConfName, m_pConfRegDB[Id]->m_serviceId );
                if (-1 != indInDB)
                    DeleteConfCntlAndDB(indInDB);
            }
        }

    }
    else
        CreateDummyConfIfDBisEmpty(cardId);

	POBJDELETE(pHeaders);
	POBJDELETE(pSeg);
	PDELETEA(pRegResponseMsg);

}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::HandleRegisterFailure(CSegment* pParam)
{
	/*CSegment* pSeg = new CSegment;

	CMplMcmsProtocol  mplMcmsProtocol;
	mplMcmsProtocol.DeSerialize(*pParam, CS_API_TYPE);************/


//	mcIndBadStatus msg;
/*	int size1 = sizeof(tmDsIndSipBadStatus);
	memcpy(&(msg.header.pmHeader), Ptr, size1 - sizeof(serial323Header)); */

	/*tmDsIndSipBadStatus * pRegResponseMsg = NULL;
	int totalLength	= sizeof(tmDsIndSipRegisterRespBase) + msg.sipHeaders.headersListLength;
	pRegResponseMsg = (tmDsIndSipRegisterResp *)new char[totalLength];
	memcpy(&((pRegResponseMsg->header).pmHeader), Ptr, totalLength - sizeof(serial323Header));
*/

}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::HandleSubscribeResponse(CSegment* pParam)
{
	DWORD Id = 0xffffffff, status = STATUS_ILLEGAL;
	CSegment* pSeg = new CSegment;

	CMplMcmsProtocol  mplMcmsProtocol;
	mplMcmsProtocol.DeSerialize(*pParam, CS_API_TYPE);
    int	data_len= mplMcmsProtocol.getDataLen();

	mcIndSubscribeResp * pSubscribeResponseMsg = (mcIndSubscribeResp *)new char[data_len];
	memcpy(pSubscribeResponseMsg, mplMcmsProtocol.GetData(), data_len);

	status = pSubscribeResponseMsg->status;
	Id = pSubscribeResponseMsg->id;

	CSipHeaderList * pTemp = new CSipHeaderList(pSubscribeResponseMsg->sipHeaders);
	COstrStream msg;
	pTemp->Dump(msg);
	msg << "Id=" << pSubscribeResponseMsg->id << "\n";
	msg << "status=" << pSubscribeResponseMsg->status << "\n";
	msg << "expires=" << pSubscribeResponseMsg->expires << "\n";
	msg << "sub opcode=" << pSubscribeResponseMsg->subOpcode << "\n";
	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::HandleSubscribeResponse Ind:\n ", msg.str().c_str());
	POBJDELETE(pTemp);

	OPCODE subscribeStatus;
	if(STATUS_OK == status)
	{
		PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleSubscribeResponse : status = STATUS_OK.");
		subscribeStatus = SUBSCRIBING_OK;
	}
	else
	{
		char s[16];
		sprintf(s,"%i", status);
		PTRACE2(eLevelInfoNormal,"CSipProxyServiceManager::HandleSubscribeResponse : Subscribing failed, status = ",s);
		subscribeStatus = SUBSCRIBING_FAILED;
	}
	*pSeg << Id;
	pSeg->Put((BYTE*)((mcIndSubscribeResp *)pSubscribeResponseMsg), data_len) ;
	m_confList->HandleEvent(pSeg, 0, subscribeStatus);

	PDELETEA(pSubscribeResponseMsg);
	PDELETE(pSeg);

}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::HandleNotifyInd(CSegment* pParam)
{
	DWORD Id,i;
	DWORD dbId = 0xffffffff;
	CSegment* pSeg = new CSegment;

	PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleNotifyInd");

	CMplMcmsProtocol  mplMcmsProtocol;
	mplMcmsProtocol.DeSerialize(*pParam, CS_API_TYPE);
        int data_len = mplMcmsProtocol.getDataLen();

	mcIndNotify * pNotifyIndMsg = (mcIndNotify *)new char[data_len];
	memcpy(pNotifyIndMsg, mplMcmsProtocol.GetData(), data_len);

	//Need to change Api with CS Manger (to send id) - patch for V7.0
	for (i=0; i < MAX_CONF_REGISTRATIONS; i++) {
		if (m_pConfRegDB[i])
		{
			if(m_pConfRegDB[i]->m_bIsMSIceUser)
			{
				dbId = i;
			}
		}
	}

	//COstrStream msg;
	//msg << "CallId=" << pNotifyIndMsg->callId << "\n";
	//msg << "Message=" << pNotifyIndMsg->message << "\n";
	//PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::HandleNotifyInd Ind:\n ", msg.str().c_str());

	//Id = m_confList->FindIdByCallId(pNotifyIndMsg->callId);
	if(dbId != 0xFFFFFFFF)
	{
		*pSeg << dbId;
		pSeg->Put((BYTE*)((mcIndNotify *)pNotifyIndMsg), data_len) ;
		m_confList->HandleEvent(pSeg, 0, NOTIFY_IND);
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::HandleNotifyInd, Could not match Call-Id.");
		POBJDELETE(pSeg);
	}
	PDELETEA(pNotifyIndMsg);

}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::HandleNotifyResponse(CSegment* pParam)
{
	DWORD Id = 0xffffffff, status = STATUS_ILLEGAL;
//	CSegment* pSeg = new CSegment;

	CMplMcmsProtocol  mplMcmsProtocol;
	mplMcmsProtocol.DeSerialize(*pParam, CS_API_TYPE);
    int	data_len= mplMcmsProtocol.getDataLen();

	mcIndNotifyResp * pNotifyResponseMsg = (mcIndNotifyResp *)new char[data_len];
	memcpy(pNotifyResponseMsg, mplMcmsProtocol.GetData(), data_len);

	status = pNotifyResponseMsg->status;
	Id = pNotifyResponseMsg->id;

//	CSipHeaderList * pTemp = new CSipHeaderList(pSubscribeResponseMsg->sipHeaders);
	COstrStream msg;
//	pTemp->Dump(msg);
	msg << "Id=" << pNotifyResponseMsg->id << "\n";
	msg << "status=" << pNotifyResponseMsg->status << "\n";
//	msg << "expires=" << pSubscrpNotifyResponseMsgibeResponseMsg->expires << "\n";
//	msg << "sub opcode=" << pSubscribeResponseMsg->subOpcode << "\n";
	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::HandleNotifyResponse Ind:\n ", msg.str().c_str());
//	POBJDELETE(pTemp);

//	OPCODE notifyStatus;
//	if(STATUS_OK == status)
//	{
//		PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleSubscribeResponse : status = STATUS_OK.");
//		notifyStatus = SUBSCRIBING_OK;
//	}
//	else
//	{
//		char s[16];
//		sprintf(s,"%i", status);
//		PTRACE2(eLevelInfoNormal,"CSipProxyServiceManager::HandleSubscribeResponse : Subscribing failed, status = ",s);
//		notifyStatus = SUBSCRIBING_FAILED;
//	}
//	*pSeg << Id;
//	pSeg->Put((BYTE*)((mcIndNotifyResp*)pNotifyResponseMsg), data_len) ;
//	m_confList->HandleEvent(pSeg, 0, notifyStatus);

	PDELETEA(pNotifyResponseMsg);
//	PDELETE(pSeg);

}

/////////////////////////////////////////////////////////////////////////////
/*void  CSipProxyServiceManager::HandleSubscribeIndication(CSegment* pHdlcParam)
{
	WORD status = SipCodesBadRequest;
	CSegment &newSeg = CSegment();
	newSeg.Create(pHdlcParam->GetWrtOffset() - pHdlcParam->GetRdOffset());
	*pHdlcParam >> newSeg;

	PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleSubscribeIndication");

	// The function GetPtr returns the pointer Ptr always 2 bytes above the the common Header/
	// To prevent alighment problem causing from the structure 'serial323Header'
	// we forward the Ptr 2 bytes ahead.
	unsigned char *Ptr = newSeg.GetPtr();
	Ptr = Ptr + 2 * sizeof(short);

	tmDsIndSipSubscribe msg;
	int size1 = sizeof(tmDsIndSipSubscribe);
	memcpy(&(msg.header.pmHeader), Ptr, size1 - sizeof(serial323Header));

	tmDsIndSipSubscribe* pSubscribeMsg = NULL;
	int totalLength	= sizeof(tmDsIndSipSubscribeBase) + msg.sipHeaders.headersListLength;
	pSubscribeMsg = (tmDsIndSipSubscribe *)new char[totalLength];
	memcpy(&((pSubscribeMsg->header).pmHeader), Ptr, totalLength - sizeof(serial323Header));

	CSipHeaderList * pTemp = new CSipHeaderList(pSubscribeMsg->sipHeaders);

	const CSipHeader* pTo = pTemp->GetNextHeader(kTo);
	if(pTo)
	{
		const char* pToStr = pTo->GetHeaderStr();
		char *pConfName = new char [H243_NAME_LEN];

		if(strcmp(pToStr, ""))
		{
			char *temp = strstr(pToStr, "@");
			if(temp)
			{
				strncpy(pConfName, pToStr, temp - pToStr);
				pConfName[temp-pToStr] = '\0';
			}
		}

		PTRACE2(eLevelInfoNormal,"CSipProxyServiceManager::HandleSubscribeIndication To:",pConfName);

		if(strcmp(pConfName, ""))
		{
			CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(pConfName);
			//conf found, handle it
			if(IsValidPObjectPtr(pCommConf))
			{
				//verify passwords are matching only in case of subscribe (not for unsubscribe)
				if(0 == pSubscribeMsg->expires || CheckPasswords(pCommConf, pTemp))
				{
					CSegment* pSeg = new CSegment;
					pSeg->Put((BYTE*)((mcReqSipBase *)pSubscribeMsg), totalLength);

					CConfApi *confApi = new CConfApi;
					confApi->CreateOnlyApi(*(pCommConf->GetRcvMbx()));
					confApi->SipSubscribeIndication(pSeg, (pHdlcEvent->m_pRsrcDesc)->GetBoardId());
					confApi->DestroyOnlyApi();
					POBJDELETE(confApi);
					status = SipCodesOk;
				}
				else
					status = SipCodesForbidden;
			}
			else
			{
				//look for conf under 'Meeting rooms'
				CCommRes* pCommRes = ::GetpMeetingRoomDB()->GetCurrentRsrv(pConfName);
				if(IsValidPObjectPtr(pCommRes) && pCommRes->IsMeetingRoom())
				{
					if(CheckPasswords(pCommRes, pTemp))
					{
						COsQueue McuRcvMbx;
						PASSERT_AND_RETURN( McuRcvMbx.Ident( "MMGR",0,NOWAIT ) );
						CMcuApi* pMcuApi = new CMcuApi;
						pMcuApi->CreateOnlyApi( McuRcvMbx );
						pMcuApi->AwakeMeetingRoom(pCommRes->GetConferenceId());
						pMcuApi->DestroyOnlyApi();
						POBJDELETE(pMcuApi);

						SystemSleep(200);

						CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(pConfName);
						//conf found, handle it
						if(IsValidPObjectPtr(pCommConf))
						{
							CSegment* pSeg = new CSegment;
							pSeg->Put((BYTE*)((mcReqSipBase *)pSubscribeMsg), totalLength);

							CConfApi *confApi = new CConfApi;
							confApi->CreateOnlyApi(*(pCommConf->GetRcvMbx()));
							confApi->SipSubscribeIndication(pSeg, (pHdlcEvent->m_pRsrcDesc)->GetBoardId());
							confApi->DestroyOnlyApi();
							POBJDELETE(confApi);
						}

						status = SipCodesOk;
					}
					else
						status = SipCodesForbidden;
				}
				else
					status = SipCodesNotFound;
			}
		}
		PDELETEA(pConfName);
	}
	//Reject Subscribe request only if error was found
	if(SipCodesOk != status)
	{
		CMntnH323Api* pCardApi = (CMntnH323Api*)( ::GetpRsrcTbl()->GetRsrcMngrPtr((pHdlcEvent->m_pRsrcDesc)->GetBoardId()) );
		if(pCardApi)
			pCardApi->SipSubscribeResponse(pSubscribeMsg->header.pmHeader, status, 0);

	}
	POBJDELETE(pTemp);
	PDELETE(pSubscribeMsg);
}*/

/////////////////////////////////////////////////////////////////////////////
/*void  CSipProxyServiceManager::HandleReferIndication(CSegment* pHdlcParam)
{
	CSegment &newSeg = CSegment();
	newSeg.Create(pHdlcParam->GetWrtOffset() - pHdlcParam->GetRdOffset());
	*pHdlcParam >> newSeg;

	PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleReferIndication");

	// The function GetPtr returns the pointer Ptr always 2 bytes above the the common Header/
	// To prevent alighment problem causing from the structure 'serial323Header'
	// we forward the Ptr 2 bytes ahead.
	unsigned char *Ptr = newSeg.GetPtr();
	Ptr = Ptr + 2 * sizeof(short);

	tmDsIndSipRefer msg;
	int size1 = sizeof(tmDsIndSipRefer);
	memcpy(&(msg.header.pmHeader), Ptr, size1 - sizeof(serial323Header));

	tmDsIndSipRefer* pSubscribeMsg = NULL;
	int totalLength	= sizeof(tmDsIndSipReferBase) + msg.sipHeaders.headersListLength;
	pSubscribeMsg = (tmDsIndSipRefer *)new char[totalLength];
	memcpy(&((pSubscribeMsg->header).pmHeader), Ptr, totalLength - sizeof(serial323Header));

	CSipHeaderList * pTemp = new CSipHeaderList(pSubscribeMsg->sipHeaders);

	const CSipHeader* pTo = pTemp->GetNextHeader(kTo);
	const char* pToStr = pTo->GetHeaderStr();
	char *pConfName = new char [H243_NAME_LEN];

	if(strcmp(pToStr, ""))
	{
		char *temp = strstr(pToStr, "@");
		if(temp)
		{
			strncpy(pConfName, pToStr, temp - pToStr);
			pConfName[temp-pToStr] = '\0';
		}
	}
	PTRACE2(eLevelInfoNormal,"CSipProxyServiceManager::HandleReferIndication To:",pConfName);

	BYTE bRejectRequest = TRUE;

	//verify conf name is not empty
	if(strcmp(pConfName, ""))
	{
		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(pConfName);
		//conf found, handle it
		if(IsValidPObjectPtr(pCommConf))
		{
				bRejectRequest = FALSE;

				CSegment* pSeg = new CSegment;
				pSeg->Put((BYTE*)((mcReqSipBase *)pSubscribeMsg), totalLength);

				CConfApi *confApi = new CConfApi;
				confApi->CreateOnlyApi(*(pCommConf->GetRcvMbx()));
				confApi->SipReferIndication(pSeg, (pHdlcEvent->m_pRsrcDesc)->GetBoardId());
				confApi->DestroyOnlyApi();
				POBJDELETE(confApi);
		}
		else
			PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::HandleReferIndication party is not found in conf");
	}

	if(bRejectRequest)
	{
		CMntnH323Api* pCardApi = (CMntnH323Api*)( ::GetpRsrcTbl()->GetRsrcMngrPtr((pHdlcEvent->m_pRsrcDesc)->GetBoardId()) );
		if(pCardApi)
			pCardApi->SipReferResponse(pSubscribeMsg->header.pmHeader, SipCodesNotFound, 0);
	}
	POBJDELETE(pTemp);
	PDELETE(pSubscribeMsg);
	PDELETEA(pConfName);
}*/
/////////////////////////////////////////////////////////////////////////////
/*void  CSipProxyServiceManager::OnCardDataIndication(CSegment* pHdlcParam)
{
	CSegment &newSeg = CSegment();
	newSeg.Create(pHdlcParam->GetWrtOffset() - pHdlcParam->GetRdOffset());
	*pHdlcParam >> newSeg;
	ALLOCBUFFER(cLog, 256);

	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::HandleCardDataIndication");

	// The function GetPtr returns the pointer Ptr always 2 bytes above the the common Header/
	// To prevent alighment problem causing from the structure 'serial323Header'
	// we forward the Ptr 2 bytes ahead.
	unsigned char *Ptr = newSeg.GetPtr();
	Ptr = Ptr + 2 * sizeof(short);

	tmDsIndSipCardData indCardData;
	int size1 = sizeof(tmDsIndSipCardData);
	memcpy(&(indCardData.header.pmHeader), Ptr, size1 - sizeof(serial323Header));

		//from enCardDataCodes
	switch(indCardData.cardDataCodes)
	{
		case SipNortel:
			{
				if ( strstr(indCardData.cardValueDetailed, SipNortelValueNames[0]) != NULL )
				{
					SipSetNortelMode(NO);
				}
				else if (strstr(indCardData.cardValueDetailed, SipNortelValueNames[1]) != NULL)
				{
					SipSetNortelMode(YES);
				}
				else
				{
					sprintf(cLog, "SipNortel passed out");
				}
			}
			break;

		default:
				sprintf(cLog, "Unknown");
			break;
	}

	sprintf(cLog, "%s .cardDataCode [%d] with cardValueDetailed [%s]",
								cLog,
								indCardData.cardDataCodes,
								indCardData.cardValueDetailed);

	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::HandleCardDataIndication ", cLog);
	DEALLOCBUFFER(cLog);
}*/

/////////////////////////////////////////////////////////////////////////////
/*void  CSipProxyServiceManager::OnCardEndStartup(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnCardEndStartup");
	WORD cardID, oldCardId;
	BYTE spanType, newCard = FALSE, bAddedConfs = FALSE;
	int errorFound = 0, unreg = 0, unregOne = 0, indInDB = -1, indInConfs = -1, ret = 1, i = 0, change = 0, gChange = 0;

	*pParam >> cardID
			>> spanType;

	if (cardID >= MAX_CARDS_IN_MCU) {	// error
		PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnCardEndStartup illegal card-ID");
		PASSERT(1);
		return;
	}

	// 1. delete old card and mark confs registered on it as unregistered
	if (m_servicesList->IsExists(cardID))
	{
		indInDB = FindConfInDB( 0, "_dummy_tester1", cardID );
		if (-1 != indInDB)
		{
			m_confList->DeleteConfCntrl(indInDB);
			RemoveFromConfDB(indInDB);
			indInDB = -1;
		}

		//check if there was a service data update
		gChange = AreCardParamsChanged(cardID, spanType);
		if(gChange)
		{
			if(m_servicesList->IsMasterCard(cardID) || (m_servicesList->GetCardPolicy(cardID) != eRegistrationMode_Redirect))
			{
				//if only transport type changed (TCP / UDP), no need to unregister
				//same if changed marking for conf type registrations
				if(gChange != 0x0010 && gChange != 0x0040)
					unreg = 1;

				if(m_timerRetry)
				{
					DeleteTimer(TIMER_RETRY_PROXY);
					m_timerRetry = 0;
				}
				for (int i = 0; i < MAX_CONF_REGISTRATIONS; i++)
				{
					//check per conf if it's registration data has changed in service, if so - it has to be updated, else left alone.
					if(m_pConfRegDB[i]  && m_pConfRegDB[i]->m_cardID == cardID
						&& AreCardParamsChanged(cardID, spanType, i))// && m_pConfRegDB[i]->m_status == CONF_UNREGISTERING)
					{
						//m_pConfRegDB[i]->m_cardID = -1;
						m_pConfRegDB[i]->m_status = CONF_IDLE;

					}
				}
			}
			m_servicesList->Remove(cardID);
			ret = m_servicesList->Insert(cardID, spanType);
		}
	}

	// 2. insert the new card to the list
	else
	{
		ret = m_servicesList->Insert(cardID, spanType);
		//if(ret == 0 && m_masterCardId == (WORD)-1)
			//m_masterCardId = cardID;

		if(ret == 0)
		{
			newCard = TRUE;

			if((m_servicesList->GetCardPolicy(cardID) != eRegistrationMode_Redirect) || m_servicesList->IsMasterCard(cardID))
			{
				for(i = 0; i < MAX_CONF_REGISTRATIONS; i++)
				{
					if(m_pConfRegDB[i] && !m_pConfRegDB[i]->m_remove
						&& CONF_UNREGISTERING != m_pConfRegDB[i]->m_status
						&& CONF_UNREGISTERING_DNS_RESOLVING != m_pConfRegDB[i]->m_status)
					{
						if(m_pConfRegDB[i]->m_cardID == cardID || m_pConfRegDB[i]->m_cardID == (WORD)-1)
							continue;
						else
							InsertConfToDB( m_pConfRegDB[i]->m_confID, m_pConfRegDB[i]->m_pConfName
							, (WORD)-1, m_pConfRegDB[i]->m_expires, m_UseMainProxy );
					}
				}
			}
			if(m_servicesList->GetCardPolicy(cardID) == eRegistrationMode_Redirect)
				SetCardAutoRedirectData(cardID);*/
/*		}
	}

	//3.Register conferences via new card
	if((m_servicesList->GetCardPolicy(cardID) != eRegistrationMode_Redirect) || m_servicesList->IsMasterCard(cardID))
	{
		for (i = 0; i < MAX_CONF_REGISTRATIONS; i++)
		{
			//find unregistered or those registered on the same card before reset.
			if (m_pConfRegDB[i])// && m_pConfRegDB[i]->m_cardID == (WORD)-1)
			{
				if(CONF_IDLE == m_pConfRegDB[i]->m_status && !m_pConfRegDB[i]->m_remove)
				{
					change = AreCardParamsChanged(cardID, spanType, i);

					//if entry is from a crashed card
					if(m_pConfRegDB[i]->m_cardID != (WORD)-1 && m_pConfRegDB[i]->m_proxyIP != 0)
						if(!ret && IsOldMasterCard(m_pConfRegDB[i]->m_cardID))
							if( change || !IsOldMasterCard(cardID))
								unregOne = 1;

					//if service updated, and entry was assigned for a proxy
					//if only transport type changed (TCP / UDP), no need to unregister
					if((unreg || unregOne)
						&& (m_pConfRegDB[i]->m_proxyIP != 0 || m_pConfRegDB[i]->m_pHostName[0]!='\0')
						&& (change != 0x1000))
					{
						//add a new entry with new data for new register
						indInDB = InsertConfToDB( m_pConfRegDB[i]->m_confID, m_pConfRegDB[i]->m_pConfName
							, cardID, m_pConfRegDB[i]->m_expires, m_UseMainProxy );

						// (create a new conf state-machine (insert to list))
						indInConfs = m_confList->CreateConfCntrl( m_pConfRegDB[indInDB], m_pRcvMbx, m_servicesList->GetCardPolicy(cardID) );
					}
					//if card start-up
					else
					{
						ALLOCBUFFER(strHostName, H243_NAME_LEN);
						oldCardId = m_pConfRegDB[i]->m_cardID;
						m_pConfRegDB[i]->m_cardID = cardID;
						m_pConfRegDB[i]->m_cardIp = m_servicesList->GetCardIp(cardID);
						m_pConfRegDB[i]->m_transportType = m_servicesList->GetTransportType(cardID);
						m_pConfRegDB[i]->m_serversConfig = m_servicesList->GetServersConfig(cardID);
						m_pConfRegDB[i]->m_outboundProxyIP = m_servicesList->GetOutboundProxyIp(cardID);
						m_pConfRegDB[i]->m_outboundProxyPort = m_servicesList->GetOutboundProxyPort(cardID);
						if(m_servicesList->GetOutboundProxyName(cardID))
							strncpy(m_pConfRegDB[i]->m_poutboundProxyName, m_servicesList->GetOutboundProxyName(cardID), H243_NAME_LEN);
						if(m_UseMainProxy)
						{
							m_pConfRegDB[i]->m_proxyIP = m_servicesList->GetProxyIp(cardID);
							m_pConfRegDB[i]->m_proxyPort = m_servicesList->GetProxyPort(cardID);
							if(m_servicesList->GetProxyName(cardID))
								strncpy(m_pConfRegDB[i]->m_pProxyName, m_servicesList->GetProxyName(cardID), H243_NAME_LEN);
							if(m_servicesList->GetRegHostName(cardID))
								strncpy(strHostName, m_servicesList->GetRegHostName(cardID), H243_NAME_LEN);
						}
						else
						{
							m_pConfRegDB[i]->m_proxyIP = m_servicesList->GetAlternateProxyIp(cardID);
							m_pConfRegDB[i]->m_proxyPort = m_servicesList->GetAlternateProxyPort(cardID);
							if(m_servicesList->GetAltProxyName(cardID))
								strncpy(m_pConfRegDB[i]->m_pProxyName, m_servicesList->GetAltProxyName(cardID), H243_NAME_LEN);
							if(m_servicesList->GetAltRegHostName(cardID))
								strncpy(strHostName, m_servicesList->GetAltRegHostName(cardID), H243_NAME_LEN);
						}

						if (strHostName)
							strncpy(m_pConfRegDB[i]->m_pHostName,strHostName, H243_NAME_LEN);
						DEALLOCBUFFER(strHostName);
						m_pConfRegDB[i]->m_DNSStatus = m_DNSStatus;

						DWORD refreshTout = 3600;
						if(m_servicesList->GetServiceName(cardID))
						{
	 ************			CIPService* pIpServ = (CIPService*)::GetpIPservList()->GetCurrentService(m_servicesList->GetServiceName(cardID));
							if(pIpServ)
							{
								CSip* pSip = pIpServ->GetpSip();
								if(pSip)
								{
									if(pSip->GetRefreshRegistrationStatus())
										refreshTout = pSip->GetRefreshRegistrationTout();
								}
							}*/
/*						}
						m_pConfRegDB[i]->m_expires = refreshTout;

						// (create a new conf state-machine (insert to list))
						if((WORD)-1 == oldCardId && oldCardId != cardID)
							indInConfs = m_confList->CreateConfCntrl( m_pConfRegDB[i], m_pRcvMbx, m_servicesList->GetCardPolicy(cardID) );
						else
							indInConfs = i;
					}

					if (-1 == indInConfs) {
						RemoveFromConfDB( i );
						errorFound = ADD_CONF_CNTL_CREATE_ERROR;
					}

					//if entry is from a crashed card
					if(unregOne)
					{
						m_confList->Unregister(i);
						//m_confList->DeleteConfCntrl(i);
						//RemoveFromConfDB(i);

						unregOne = 0;
					}

					if(unreg)
					{
						if(m_pConfRegDB[i] &&
							(m_pConfRegDB[i]->m_proxyIP != 0 || m_pConfRegDB[i]->m_pHostName[0]!='\0'))
							if(CONF_IDLE == m_pConfRegDB[i]->m_status)
								m_confList->Unregister(i);
					}

					//so we can re-register with new transport type
					if(change == 0x1000)
						m_confList->ResetConfCntl(indInConfs);

					// (start registration in the Proxy Registrar Server)
					if (!errorFound) {
						if (-1 == m_confList->StartRegister(indInConfs)) {
							m_confList->DeleteConfCntrl( indInConfs );
							RemoveFromConfDB( indInConfs );
							errorFound = ADD_CONF_START_REGISTRATION_ERROR;
						}
					}
				}
			}
		}//end for
	}
	else
	{
		if(unreg)
		{
			for (i = 0; i < MAX_CONF_REGISTRATIONS; i++)
			{
				if(m_pConfRegDB[i] && cardID == m_pConfRegDB[i]->m_cardID &&
					(m_pConfRegDB[i]->m_proxyIP != 0 || m_pConfRegDB[i]->m_pHostName[0]!='\0'))// && m_pConfRegDB[i]->m_cardID == (WORD)-1)
					if(CONF_IDLE == m_pConfRegDB[i]->m_status)
					{
						//m_confList->StopReg(i);
						m_confList->Unregister(i);
					}
			}
		}
	}

	m_oldMasterCardId[cardID] = (WORD)-1;

	if(newCard || (gChange & 0x0020) || (gChange & 0x0040) || (gChange & 0x0080))
		bAddedConfs = CheckAllConfDBS(cardID); ******* */

	//check if card has no conferences, then create one dummy.
/*	if(!bAddedConfs)
	{
		for (i = 0; i < MAX_CONF_REGISTRATIONS; i++)
		{
			if(m_pConfRegDB[i] && cardID == m_pConfRegDB[i]->m_cardID)
				break;
		}
		if(MAX_CONF_REGISTRATIONS == i && 0 == ret)
		{
			CreateDummyConf(cardID);
		}
	}
}*/

/////////////////////////////////////////////////////////////////////////////
/*void  CSipProxyServiceManager::OnCardCrash(CSegment* pParam)
{
	WORD cardID, rCardId = (WORD)-1;
	*pParam >> cardID;
	if (cardID > MAX_CARDS_IN_MCU) {	// error
		PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnCardCrash illegal card-ID");
		PASSERT(1);
		return;
	}

	char s[8];
    sprintf(s,"%i", cardID);
	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnCardCrash BoardId=", s);

	// delete card, and all conferences registered via its IP
	if (m_servicesList->IsExists(cardID))
	{
		//find replacing card
		rCardId = m_servicesList->GetCardID_NextService(cardID);

		//if crashed card is the master
		if((m_servicesList->GetCardPolicy(cardID) != eRegistrationMode_Redirect) || m_servicesList->IsMasterCard(cardID))
		{
			if ((WORD)-1 == rCardId)
			{
				PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnCardCrash No other IP card to unregister from.");

				//m_oldMasterCardId[cardID] = cardID;
				m_servicesList->SetMasterCard(cardID, FALSE);

				for (int i = 0; i < MAX_CONF_REGISTRATIONS; i++)
				{
					if(m_pConfRegDB[i] && m_pConfRegDB[i]->m_cardID == cardID)
					{
						m_pConfRegDB[i]->m_status = CONF_IDLE;
						if(!m_pConfRegDB[i]->m_remove)
						{
							m_confList->StopReg(i);
							m_confList->ResetConfCntl(i);
							m_confList->DeleteConfCntrl(i);
							RemoveFromConfDB(i);
						}
					}
				}
			}
			else
			{
				m_oldMasterCardId[rCardId] = cardID;
				m_servicesList->SetMasterCard(cardID, FALSE);
				m_servicesList->SetMasterCard(rCardId, TRUE);

				for (int i = 0; i < MAX_CONF_REGISTRATIONS; i++)
				{
					if(m_pConfRegDB[i] && m_pConfRegDB[i]->m_cardID == cardID)
					{
						m_confList->StopReg(i);
						m_pConfRegDB[i]->m_status = CONF_IDLE;
						//delete only 'dummy'
						if(m_pConfRegDB[i]->m_bIsDiscovery)
						{
							m_confList->ResetConfCntl(i);
							m_confList->DeleteConfCntrl(i);
							RemoveFromConfDB(i);
						}
					}
				}

				if(m_servicesList->GetCardPolicy(cardID) == eRegistrationMode_Redirect)
					MoveRegistrationsBetweenCards();
				else
					RemoveRegistrationsUsingOtherCard();
			}
		}
		// remove card from list
		m_servicesList->Remove(cardID);
		SetRegistrarStatus(cardID, 0);

		if(m_servicesList->GetCardPolicy(cardID) == eRegistrationMode_Redirect)
			SetCardAutoRedirectData(rCardId);*/
/*	}
	else
		PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnCardCrash card not found in DB.");
}*/

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnAddConf(CSegment* pParam)
{
	//SystemSleep(20);

	DWORD confID;
	ALLOCBUFFER(confName, H243_NAME_LEN);
	ALLOCBUFFER(userDomainName, USER_DOMAIN_NAME_LEN);
	ALLOCBUFFER(id, 50);
	ALLOCBUFFER(str, 80);

	int errorFound = 0, indInConfs = -1, indInDB = -1;
	DWORD  durationTime = 0;
	WORD	wRegOngoing = 0, wRegMR = 0, wRegEQandFactory = 0, confType = 0;
	BYTE	bFoundSipCard = FALSE, isOngoing = FALSE, isMR = FALSE, isEQ = FALSE, isFactory = FALSE, isGW = FALSE;
	WORD	sipServerType = 0;

	*pParam >> confID;		// check conf ID validity
	*pParam >> confName;
	*pParam >> isOngoing;
	*pParam >> isMR;
	*pParam >> isEQ;
	*pParam >> isFactory;
	*pParam >> isGW;
	*pParam >> durationTime;

	//do not register the system defined t1 cas meeting room
	if(strncmp(confName, "##T1CAS$$", 9))
	{

		PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnAddConf, confName=", confName);

		if (confName[0] == 0){// || userDomainName[0] == 0) {	// check name validity
			errorFound = CONF_NAME_EMPTY;
		}

	//	if(strcspn(confName, BAD_CHARACTERS_FOR_SIP_URI) != strlen(confName))
		if(!IsGoodConfName(confName))
			errorFound = STATUS_ILLEGAL_RESERVATION_NAME;

		// find IP service to register via it (use its IP address)
		int serviceID = -1;
		if (!errorFound)
		{

			CSipProxyIpParams* pService = m_servicesList->GetFirstService();
			if(pService)
			{
				serviceID = pService->GetServiceId();
				sipServerType = pService->GetSipServerType();
			}
			while(-1 != serviceID)
			{
				sprintf(id, "%d", serviceID);
				PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnAddConf, next service=", id);

				//if conf is ongoing, but service marks not to register on-going
				/*if(!m_servicesList->IsRegOnGoing(serviceID) && isOngoing)
					errorFound = SERVICE_SET_DONT_REG_ONGOING;

				//if conf is a meeting room, not ongoing, but service marks not to reg meeting rooms
				if(!m_servicesList->IsRegMRs(serviceID) && !isOngoing && isMR && !isEQ)
					errorFound = SERVICE_SET_DONT_REG_MEETING_ROOM;

				//if conf is an entry queue, not ongoing, but service marks not to reg entry queues
				if(!m_servicesList->IsRegEQs(serviceID) && !isOngoing && isEQ)
					errorFound = SERVICE_SET_DONT_REG_ENTRY_QUEUE;

				//if conf is an entry queue, not ongoing, but service marks not to reg entry queues
				if(!m_servicesList->IsRegFactories(serviceID) && !isOngoing && isFactory)
					errorFound = SERVICE_SET_DONT_REG_SIP_FACTORY;

				//if conf is a GW profile, not ongoing, but service marks not to reg GW profiles
				if(!m_servicesList->IsRegGWProfiles(serviceID) && !isOngoing && isGW)
					errorFound = SERVICE_SET_DONT_REG_GW_PROFILE;


				//if ongoing conf is a copy of an entry queue
				if(isOngoing && isEQ)
					errorFound = DONT_REG_ONGOING_COPY_OF_EQ;*/

				// (check if this conference already exists in the DB)
				if(!errorFound)
				{
					int confIdInDb = FindConfInDB( confID, confName, serviceID );
					if (-1 != confIdInDb)
					{
						errorFound = ADD_CONF_ALREADY_EXISTS;
						PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnAddConf, ADD_CONF_ALREADY_EXISTS");
						DWORD status;

						if(m_pConfRegDB[confIdInDb]->m_status == CONF_REGISTERED)
							status = STATUS_OK;
						else
							status = STATUS_FAIL;

						eSipRegistrationConfType sipConfType = eSipRegistrationConfTypeIncorrect;
						sipConfType = SipProxyServiceManagerGetConfType(m_pConfRegDB[confIdInDb]);

						SetConfRegistrationStatus(serviceID, confID, (WORD)sipConfType, status,1);
						// do something...
						break;
					}
				}
					// (insert the conference into the confs DB)
					indInDB = -1;
					if (!errorFound)
					{
						indInDB = InsertConfToDB( confID, confName, serviceID, durationTime, m_UseMainProxy, FALSE, isOngoing, isMR, isEQ, isFactory, isGW, sipServerType, m_Dummy_name);
						PTRACE2INT(eLevelInfoNormal,"CSipProxyServiceManager::OnAddConf indInDB=", indInDB);
						if (-1 == indInDB) {
							errorFound = ADD_CONF_DB_IS_FULL;
							if (IsConfFromOldServiceExist())
							{
								PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnAddConf, DB is full but there is conf from old service, retry after timer");
								if(IsValidTimer(TIMER_RETRY_ASK_CONFS_DB))
									DeleteTimer(TIMER_RETRY_ASK_CONFS_DB);
								StartTimer(TIMER_RETRY_ASK_CONFS_DB, RetryAskConfsDbTimeOut*SECOND);
							}
							else
							{
								AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
														SIP_REGISTRATION_LIMIT_REACHED,
														SYSTEM_MESSAGE,
														"SIP registrations limit reached",
														true,
														true
													);
							}
						}
						else
							RemoveActiveAlarmByErrorCode(SIP_REGISTRATION_LIMIT_REACHED);
					}

					// (create a new conf state-machine (insert to list))
					indInConfs = -1;
					if (!errorFound)
					{
						indInConfs = m_confList->CreateConfCntrl( m_pConfRegDB[indInDB], m_pRcvMbx, m_pMockMplProtocol);
						if (-1 == indInConfs) {
							RemoveFromConfDB( indInDB );
							errorFound = ADD_CONF_CNTL_CREATE_ERROR;
						}
					}
					if (!errorFound)
					{
						if (IsUseBusyServer(indInDB) && IsValidTimer(TIMER_RETRY_BUSY_PROXY))
							m_confList->SetBusyServer(indInConfs, TRUE);
					}
					// (start registration in the Proxy Registrar Server)
					if (!errorFound)
						errorFound = StartRegister(indInConfs);

					if(errorFound)
					{
//						CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
						CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
						BOOL bSipRegisterOnlyOnce = FALSE;
//						pSysConfig->GetBOOLDataByKey("SIP_REGISTER_ONLY_ONCE", bSipRegisterOnlyOnce);
						if( pServiceSysCfg != NULL )
						    pServiceSysCfg->GetBOOLDataByKey(serviceID, "SIP_REGISTER_ONLY_ONCE", bSipRegisterOnlyOnce);
						if(!(bSipRegisterOnlyOnce && indInDB != -1 && m_pConfRegDB[indInDB]->m_bIsDiscovery)){
							sprintf(str, "%d, error = %d", serviceID, errorFound);
							PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnAddConf, failed to reg on service=", str);
						}
					}
					//if successfull, delete dummy if dummy exists
					else
					{
						indInDB = FindConfInDB( 0, m_Dummy_name, serviceID );
                        {//---Sasha Sh. Disable/Enable dummy_tester1  REGISTER---------------------------------------------//
                            BOOL bIsDisableDummyRegistration = 0;
                            std::string key = "DISABLE_DUMMY_REGISTRATION";
                            CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key, bIsDisableDummyRegistration);
                            if(true == bIsDisableDummyRegistration)
                            {
                                PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::CreateDummyConf - Flag:DISABLE.");
                                indInDB = -1;
                            }
                        }//-----------------------------------------------------------------------------------------------//
						if (-1 != indInDB)
							DeleteConfCntlAndDB(indInDB);
					}


					errorFound = 0;
					bFoundSipCard = FALSE;

                    pService = m_servicesList->GetNextService(serviceID);
                    DBGPASSERT(pService && (int)(pService->GetServiceId()) == serviceID);
                    if(pService && (int)(pService->GetServiceId()) != serviceID)
						serviceID = pService->GetServiceId();
					else
						serviceID = -1;
			}//end while

		}
		else
		{
			sprintf(str, "%d, error = %d", serviceID, errorFound);
			PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnAddConf, failed to reg on service=", str);
		}
	}
	DEALLOCBUFFER(confName);
	DEALLOCBUFFER(userDomainName);
	DEALLOCBUFFER(id);
	DEALLOCBUFFER(str);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnDelConf(CSegment* pParam)
{
	DWORD confID;
	ALLOCBUFFER(confName, H243_NAME_LEN);
	confName[0]='\0';
	ALLOCBUFFER(userDomainName, USER_DOMAIN_NAME_LEN);
	userDomainName[0]='\0';
	ALLOCBUFFER(id, 12);
	int errorFound = 0, indInDB = 0, i = 0;

	*pParam >> confID;		// check conf ID validity
	*pParam >> confName;

	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnDelConf, confName=", confName);
	if (confName[0] == 0){ // || userDomainName[0] == 0) {	// check name validity
		errorFound = CONF_NAME_EMPTY;
	}

	// check if this conference already exists in the DB
	// find IP service to cancel registeration
	// remove the conference from the confs DB
	// delete the conf state-machine (remove to list)
	// start unregistering in the proxy

	// find IP card to unregister via it (use its IP address)
	int serviceID = -1;
	if (!errorFound)
	{
		CSipProxyIpParams* pService = m_servicesList->GetFirstService();
		if(pService)
			serviceID = pService->GetServiceId();

		if (-1 == serviceID)
		{
			errorFound = IP_CARD_NOT_FOUND;
			// (insert the conference into the confs DB)
			//even if no card exist, we need a copy in the DB
			indInDB = FindConfInDB( confID, confName, serviceID );
			if (-1 == indInDB)
				errorFound = DEL_CONF_DOES_NOT_EXIST;
			else
				RemoveFromConfDB(indInDB);
		}

		while(-1 != serviceID)
		{
			sprintf(id, "%d", serviceID);
			PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnDelConf, next service=", id);
			// (check if this conference already exists in the DB)
			indInDB = 0;
			indInDB = FindConfInDB( confID, confName, serviceID );
			if (-1 == indInDB) {
				errorFound = DEL_CONF_DOES_NOT_EXIST;
				// do something...
			}
			if (!errorFound)
			{
				if(IsExistOtherRegWithSameName(indInDB, serviceID))
				{
					m_confList->ChangePresence(indInDB,SIP_PRESENCE_ONLINE);//change presence back to online

					DeleteConfCntlAndDB(indInDB);
				}
				else
					// (start unregistration in the Proxy Registrar Server)
					m_confList->Unregister(indInDB);
			}
			errorFound = 0;

			pService = m_servicesList->GetNextService(serviceID);
            DBGPASSERT(pService && (int)(pService->GetServiceId()) == serviceID);

			if(pService && (int)(pService->GetServiceId()) != serviceID)
				serviceID = pService->GetServiceId();
			else
				serviceID = -1;
		}
	}

	//What about a rout to delete conf from list once registration is removed?
	DEALLOCBUFFER(confName);
	DEALLOCBUFFER(userDomainName);
	DEALLOCBUFFER(id);
}

/////////////////////////////////////////////////////////////////////////////
void	CSipProxyServiceManager::OnKillOneConf(CSegment* pParam)
{
	WORD indInDB = (WORD)-1, serviceId = 0;
	ALLOCBUFFER(confName, H243_NAME_LEN);
	confName[0]='\0';
	ALLOCBUFFER(userDomainName, USER_DOMAIN_NAME_LEN);
	userDomainName[0]='\0';
	int errorFound = 0, i = 0;

	*pParam >> indInDB;

	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnKillOneConf.confName=", confName);
	if ((WORD)-1 != indInDB && MAX_CONF_REGISTRATIONS > indInDB)
		if(m_pConfRegDB[indInDB])
		{
			serviceId = m_pConfRegDB[indInDB]->m_serviceId;
			DeleteConfCntlAndDB(indInDB);
			CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
			BOOL bSipRegisterOnlyOnce = FALSE;
			pSysConfig->GetBOOLDataByKey("SIP_REGISTER_ONLY_ONCE", bSipRegisterOnlyOnce);
			if (!bSipRegisterOnlyOnce)
				CreateDummyConfIfDBisEmpty(serviceId);
		}


	DEALLOCBUFFER(confName);
	DEALLOCBUFFER(userDomainName);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnTimerRetryAskConfsDb(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnTimerRetryAskConfsDb");
	CConfPartyManagerApi ConfPartyApi;
	ConfPartyApi.SipProxyDBRequest();
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnTimerRetryProxy(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnTimerRetryProxy, retry registration on proxy.");
	m_timerRetry = 0;
	int i = 0;

	for(i=0; i<MAX_CONF_REGISTRATIONS; i++) {
		if(m_pConfRegDB[i])
			if(CONF_IDLE == m_pConfRegDB[i]->m_status)
			{
				if(m_servicesList->find(m_pConfRegDB[i]->m_serviceId))
					StartRegister(i);
			}
	}
	/**
	 * DB is empty need to send register for dummy_tester.
	 */
	if(MAX_CONF_REGISTRATIONS == i) {
		StartRegister(0);
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnTimerRetryBusyProxy(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnTimerRetryBusyProxy");
	// currently we use one timer for idle all servers.
	DeleteBusyServerList();
	SetBusyExpiredTime(0);
	for(int i=0; i<MAX_CONF_REGISTRATIONS; i++)
	{
		if(m_pConfRegDB[i] && (m_servicesList->find(m_pConfRegDB[i]->m_serviceId)))
		{
			m_confList->SetBusyServer(i, FALSE);
			if(CONF_IDLE == m_pConfRegDB[i]->m_status)
			{
				StartRegister(i);
			}
			if((CONF_REGISTERED == m_pConfRegDB[i]->m_status)
			  && m_pConfRegDB[i]->m_remove)
			{
				m_confList->Unregister(i);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSipProxyServiceManager::IsUseBusyServer(DWORD Id)
{
	if(!m_pConfRegDB[Id])
	{
		DBGPASSERT(Id? Id: (DWORD)-1);
		return FALSE;
	}

	StBusyServerParam* pStruct = NULL;
	BusyServerVec::iterator itr = m_BusyProxyVector.begin();
	while(itr != m_BusyProxyVector.end())
	{
		pStruct = *itr;
		if(pStruct)
		{
			if (!(::isApiTaNull(&m_pConfRegDB[Id]->m_proxyAddress)) && !(::isApiTaNull(&pStruct->m_proxyAddress))) // proxy ip is different from 0 and both side's proxy ip is equal,
			{
				if (::isIpAddressEqual(&m_pConfRegDB[Id]->m_proxyAddress, &pStruct->m_proxyAddress))
					return TRUE;
			}
			else // one of the proxy ip is missing, check by proxy name
			{
				if (strcmp(m_pConfRegDB[Id]->m_pProxyName,"") && !strcmp(m_pConfRegDB[Id]->m_pProxyName,pStruct->m_pProxyName))
					return TRUE;
			}
		}

		itr++;
	}
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnDNSResolveInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnDNSResolveInd.");
	WORD serviceId = 0, tmpService = 0;
	ALLOCBUFFER(pDomainName, DnsQueryNameSize);//H243_NAME_LEN);
	ALLOCBUFFER(pServiceName, DnsQueryNameSize);//H243_NAME_LEN);
	pDomainName[0]='\0';
	pServiceName[0]='\0';
	CSipProxyIpParams* pService;

	*pParam >> serviceId
			>> pDomainName;

	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnDNSResolveInd. pDomainName:", pDomainName);
	pParam->ResetRead();

	tmpService = serviceId;
	while(tmpService != (WORD)-1)
	{

		for (int i = 0; i < MAX_CONF_REGISTRATIONS; i++)
		{
			
			if(m_pConfRegDB[i] != NULL)
				if( (tmpService == m_pConfRegDB[i]->m_serviceId) 								                                &&
					((eConfSipServerManually == m_pConfRegDB[i]->m_serversConfig) || (m_pConfRegDB[i] ->m_bIsStandIceUser))		&&
					//in standard ICE there is no sip server continue without it
					( ((isApiTaNull(&m_pConfRegDB[i]->m_proxyAddress) && !strncmp(pDomainName, m_pConfRegDB[i]->m_pProxyName, DnsQueryNameSize)) 			       ||
					  (isApiTaNull(&m_pConfRegDB[i]->m_outboundProxyAddress) && !strncmp(pDomainName, m_pConfRegDB[i]->m_poutboundProxyName, DnsQueryNameSize)))   ||
					  (m_pConfRegDB[i] ->m_bIsStandIceUser) ) )
				{
					PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnDNSResolveInd. calling :HandleEventByIndex");
					pParam->ResetRead();
					m_confList->HandleEventByIndex(DNS_RESOLVE_IND, i, pParam);
				}
		}

		pService = m_servicesList->GetNextService(tmpService);
		if(pService)
			tmpService = pService->GetServiceId();
		//if returned to the original card
		if(tmpService == serviceId)
			break;
	}
	DEALLOCBUFFER(pDomainName);
	DEALLOCBUFFER(pServiceName);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnDNSServiceInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnDNSServiceInd.");
	WORD serviceId = 0, tmpService = 0;
	ALLOCBUFFER(pDomainName, DnsQueryNameSize);//H243_NAME_LEN);
	pDomainName[0]='\0';

	*pParam >> serviceId
			>> pDomainName;

	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnDNSServiceInd. pDomainName:", pDomainName);

	pParam->ResetRead();

	tmpService = serviceId;
	while(tmpService != (WORD)-1)
	{
		for (int i = 0; i < MAX_CONF_REGISTRATIONS; i++)
		{
			if(m_pConfRegDB[i] != NULL)
				if(tmpService == m_pConfRegDB[i]->m_serviceId
					&& eConfSipServerAuto == m_pConfRegDB[i]->m_serversConfig
					&& (isApiTaNull(&m_pConfRegDB[i]->m_proxyAddress) && isApiTaNull(&m_pConfRegDB[i]->m_outboundProxyAddress)
						&& !strncmp(pDomainName, m_pConfRegDB[i]->m_pHostName, DnsQueryNameSize)))//H243_NAME_LEN)))

				{
					m_confList->HandleEventByIndex(DNS_SERVICE_IND, i, pParam);
					pParam->ResetRead();
				}
		}
		CSipProxyIpParams* pService = m_servicesList->GetNextService(tmpService);
		if(pService)
			tmpService = pService->GetServiceId();
		//if returned to the original card
		if(tmpService == serviceId)
			break;
	}
	DEALLOCBUFFER(pDomainName);
}

/////////////////////////////////////////////////////////////////////////////
/*void  CSipProxyServiceManager::OnLoadMngrAck(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnLoadMngrAck.");

	CSegment* pSeg = new CSegment;
	DWORD Id = 0;
	WORD index = 0;

	*pParam >> Id;

	index = (WORD)Id;

	*pSeg << (DWORD)0
		  << (WORD)LOADACCEPTMSGACK;

	m_confList->HandleEventByIndex(index, pSeg);
}*/

/////////////////////////////////////////////////////////////////////////////
int  CSipProxyServiceManager::FindConfInDB( DWORD confID, char *confName, int serviceId )
{
	for (int i = 0; i < MAX_CONF_REGISTRATIONS; i++) {
		if(m_pConfRegDB[i] != NULL)
		{
			TRACEINTO << "ConfName:" << m_pConfRegDB[i]->m_pConfName << ", ConfId:" << m_pConfRegDB[i]->m_confID;
			//if (m_pConfRegDB[i]->m_confID == confID)
				if (0 == strcmp(confName, m_pConfRegDB[i]->m_pConfName))
					if((DWORD)serviceId == m_pConfRegDB[i]->m_serviceId)
						if (m_pConfRegDB[i]->m_bIsFromOldService == FALSE)
							return i;	// index of the conf in DB
		}
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CSipProxyServiceManager::IsExistOtherRegWithSameName(int index, int serviceId)
{
	BYTE result = FALSE;
	for (int i = 0; i < MAX_CONF_REGISTRATIONS; i++)
	{
		if(i != index && m_pConfRegDB[i] != NULL)
			if (m_pConfRegDB[i]->m_confID != m_pConfRegDB[index]->m_confID)
				if (!strncmp(m_pConfRegDB[index]->m_pConfName, m_pConfRegDB[i]->m_pConfName, H243_NAME_LEN))
					if((DWORD)serviceId == m_pConfRegDB[i]->m_serviceId)
						if (m_pConfRegDB[i]->m_bIsFromOldService == FALSE)
						{
							result = TRUE;
							break;
						}
	}
	return result;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CSipProxyServiceManager::IsConfFromOldServiceExist()
{
	for (int i = 0; i < MAX_CONF_REGISTRATIONS; i++)
		if((m_pConfRegDB[i] != NULL) && (m_pConfRegDB[i]->m_bIsFromOldService == TRUE))
			return TRUE;
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
int CSipProxyServiceManager::InsertIceConfToDB(DWORD confID, char *confName, WORD serviceId, DWORD expires,
        BYTE useMainProxy, BOOL bIsDiscovery, BYTE bIsOngoing, BYTE bIsMR, BYTE bIsEQ,
        BYTE bIsFactory, BYTE bIsGW, WORD sipServerType, char *dummyName, BYTE IsMSICEUser,BYTE IsStandICEUser)
{
	int i=1;
	DWORD refreshTout = 3600;

	for (; i < MAX_CONF_REGISTRATIONS; i++)
	{
		if (!m_pConfRegDB[i])
		{
			m_pConfRegDB[i] = new COneConfRegistration;
			m_pConfRegDB[i]->m_status = CONF_IDLE;
			m_pConfRegDB[i]->m_remove = FALSE;
			m_pConfRegDB[i]->m_bIsFromOldService = FALSE;
			m_pConfRegDB[i]->m_confID = confID;
			m_pConfRegDB[i]->m_serviceId = m_pICEserviceId;
			m_pConfRegDB[i]->m_bIsDiscovery = bIsDiscovery;
			m_pConfRegDB[i]->m_bIsOngoing = bIsOngoing;
			m_pConfRegDB[i]->m_bIsMR = bIsMR;
			m_pConfRegDB[i]->m_bIsEQ = bIsEQ;
			m_pConfRegDB[i]->m_bIsFactory = bIsFactory;
			m_pConfRegDB[i]->m_bIsGWProfile = bIsGW;
			m_pConfRegDB[i]->m_bIsMSIceUser = IsMSICEUser;
			m_pConfRegDB[i]->m_bIsStandIceUser = IsStandICEUser;
			m_pConfRegDB[i]->m_sipServerType = 	sipServerType;

			PTRACE2INT(eLevelInfoNormal,"CSipProxyServiceManager::InsertIceConfToDB discovery: ",bIsDiscovery);

			eIpType ipType = m_pProxyService->GetIpType();
			m_pConfRegDB[i]->m_ipType = ipType;

			for(int index=0; index<TOTAL_NUM_OF_IP_ADDRESSES; index++)
			{
				ipAddressStruct ipAddrSt = m_pProxyService->GetServiceIpAddress(index);
				m_pConfRegDB[i]->SetServiceAddresses(ipAddrSt,index);
			}

			m_pConfRegDB[i]->m_transportType = m_pProxyService->GetTransportType();
			m_pConfRegDB[i]->m_serversConfig = m_pProxyService->GetServersConfig();
			if(m_pProxyService->GetOutboundProxyIpVersion() == eIpVersion4)
			{
				m_pConfRegDB[i]->m_outboundProxyAddress.addr.v4.ip = m_pProxyService->GetOutboundProxyIpV4();
				m_pConfRegDB[i]->m_outboundProxyAddress.ipVersion = eIpVersion4;
			}
			else if(m_pProxyService->GetOutboundProxyIpVersion() == eIpVersion6)
			{
				char * pOutboundProxyIpV6 = m_pProxyService->GetOutboundProxyIpV6();
				if (pOutboundProxyIpV6==NULL)
					PTRACE(eLevelError, "GetOutboundProxyIpV6 returned NULL");
				else
					memcpy(m_pConfRegDB[i]->m_outboundProxyAddress.addr.v6.ip, pOutboundProxyIpV6, IPV6_ADDRESS_BYTES_LEN);

				m_pConfRegDB[i]->m_outboundProxyAddress.addr.v6.scopeId = m_pProxyService->GetOutboundProxyV6scopeId();
				m_pConfRegDB[i]->m_outboundProxyAddress.ipVersion = eIpVersion6;
			}

			m_pConfRegDB[i]->m_outboundProxyAddress.port = m_pProxyService->GetOutboundProxyPort();

			char * pOutboundProxyName = m_pProxyService->GetOutboundProxyName();
			if (pOutboundProxyName==NULL)
				PTRACE(eLevelError, "GetOutboundProxyName returned NULL");
			else
				strncpy(m_pConfRegDB[i]->m_poutboundProxyName, pOutboundProxyName, H243_NAME_LEN);

			m_pConfRegDB[i]->m_DNSStatus = m_DNSStatus;

			ALLOCBUFFER(strHostName, H243_NAME_LEN);

			if(useMainProxy)
			{
				if(m_pProxyService->GetProxyIpVersion() == eIpVersion4)
				{
					m_pConfRegDB[i]->m_proxyAddress.addr.v4.ip = m_pProxyService->GetProxyIpV4();
					m_pConfRegDB[i]->m_proxyAddress.ipVersion = eIpVersion4;
				}
				else if(m_pProxyService->GetProxyIpVersion() == eIpVersion6)
				{
					char * pProxyIpV6 = m_pProxyService->GetProxyIpV6();
					if (pProxyIpV6==NULL)
						PTRACE(eLevelError, "GetProxyIpV6 returned NULL");
					else
						memcpy(m_pConfRegDB[i]->m_proxyAddress.addr.v6.ip, pProxyIpV6, IPV6_ADDRESS_BYTES_LEN);

					m_pConfRegDB[i]->m_proxyAddress.addr.v6.scopeId = m_pProxyService->GetProxyV6scopeId();
					m_pConfRegDB[i]->m_proxyAddress.ipVersion = eIpVersion6;
				}
				m_pConfRegDB[i]->m_proxyAddress.port = m_pProxyService->GetProxyPort();

				char * pProxyName = m_pProxyService->GetProxyName();
				if (pProxyName==NULL)
					PTRACE(eLevelError, "GetProxyName returned NULL");
				else
					strncpy(m_pConfRegDB[i]->m_pProxyName, pProxyName, H243_NAME_LEN);

				char * pRegHostName = m_pProxyService->GetProxyHostName();
				if (pRegHostName==NULL)
					PTRACE(eLevelError, "GetProxyHostName returned NULL");
				else
					strncpy(strHostName, pRegHostName, H243_NAME_LEN);
			}
			else
			{
				if(m_pProxyService->GetAltProxyIpVersion() == eIpVersion4)
				{
					m_pConfRegDB[i]->m_proxyAddress.addr.v4.ip = m_pProxyService->GetAltProxyIpV4();
					m_pConfRegDB[i]->m_proxyAddress.ipVersion = eIpVersion4;
				}
				else if(m_pProxyService->GetAltProxyIpVersion() == eIpVersion6)
				{
					char * pAlternateProxyIpV6 = m_pProxyService->GetAltProxyIpV6();
					if (pAlternateProxyIpV6==NULL)
						PTRACE(eLevelError, "GetAlternateProxyIpV6 returned NULL");
					else
						memcpy(m_pConfRegDB[i]->m_proxyAddress.addr.v6.ip, pAlternateProxyIpV6, IPV6_ADDRESS_BYTES_LEN);
					m_pConfRegDB[i]->m_proxyAddress.addr.v6.scopeId = m_pProxyService->GetAltProxyV6scopeId();
					m_pConfRegDB[i]->m_proxyAddress.ipVersion = eIpVersion6;
				}
				m_pConfRegDB[i]->m_proxyAddress.port = m_pProxyService->GetAltProxyPort();
				char * pAltProxyName = m_pProxyService->GetAltProxyName();
				if (pAltProxyName==NULL)
					PTRACE(eLevelError, "GetAltProxyName returned NULL");
				else
					strncpy(m_pConfRegDB[i]->m_pProxyName, pAltProxyName, H243_NAME_LEN);

				//YossiG fix Klocwork NPD issue
				char * pAltRegHostName = m_pProxyService->GetAltProxyHostName();
				if (pAltRegHostName==NULL)
					PTRACE(eLevelError, "GetAltRegHostName returned NULL");
				else
					strncpy(strHostName, pAltRegHostName, H243_NAME_LEN);
			}

			strncpy(m_pConfRegDB[i]->m_pHostName,strHostName, H243_NAME_LEN);
			DEALLOCBUFFER(strHostName);
			m_pConfRegDB[i]->m_move = FALSE;
			strncpy( m_pConfRegDB[i]->m_pConfName, confName, H243_NAME_LEN );
			            strncpy( m_pConfRegDB[i]->m_dummy_name, dummyName, H243_NAME_LEN -1); // B.S. klocwork 2583
			            m_pConfRegDB[i]->m_dummy_name[H243_NAME_LEN -1]='\0'; // B.S. klocwork 2583
			//m_pConfRegDB[i]->m_expires = expires;
			if(m_pProxyService->GetServiceName())
				refreshTout = m_pProxyService->GetRefreshTout();

			m_pConfRegDB[i]->m_expires = refreshTout;
			m_conferencesCounter++;

			return i;
		}
	}

	PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::InsertIceConfToDB  Not Found empty place");
		return -1;	// not found an empty place
}

/////////////////////////////////////////////////////////////////////////////
int  CSipProxyServiceManager::InsertConfToDB( DWORD confID, char *confName, WORD serviceId, DWORD expires,
						            BYTE useMainProxy, BOOL bIsDiscovery, BYTE bIsOngoing, BYTE bIsMR, BYTE bIsEQ,
						            BYTE bIsFactory, BYTE bIsGW, WORD sipServerType, char *dummyName, BYTE IsMSICEUser,BYTE IsStandICEUser)
{
	DWORD refreshTout = 3600;
	int i;
	if(strcmp(confName, dummyName) == 0 || IsMSICEUser)//||IsStandICEUser)
		i=DUMMY_ENTRY;
	else
		i=1;

    {//---Sasha Sh. Disable/Enable dummy_tester1  REGISTER---------------------------------------------//
        BOOL bIsDisableDummyRegistration = 0;
        std::string key = "DISABLE_DUMMY_REGISTRATION";
        CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key, bIsDisableDummyRegistration);
        if(true == bIsDisableDummyRegistration)
        {
            PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::InsertConfToDB - Flag:DISABLE.");
            i=1;
        }
    }//------------------------------------------------------------------------------------------------//
	for (; i < MAX_CONF_REGISTRATIONS; i++) {
		if (!m_pConfRegDB[i]) {
			m_pConfRegDB[i] = new COneConfRegistration;
			m_pConfRegDB[i]->m_status = CONF_IDLE;
			m_pConfRegDB[i]->m_remove = FALSE;
			m_pConfRegDB[i]->m_bIsFromOldService = FALSE;
			m_pConfRegDB[i]->m_confID = confID;
			m_pConfRegDB[i]->m_serviceId = serviceId;
			m_pConfRegDB[i]->m_bIsDiscovery = bIsDiscovery;
			m_pConfRegDB[i]->m_bIsOngoing = bIsOngoing;
			m_pConfRegDB[i]->m_bIsMR = bIsMR;
			m_pConfRegDB[i]->m_bIsEQ = bIsEQ;
			m_pConfRegDB[i]->m_bIsFactory = bIsFactory;
			m_pConfRegDB[i]->m_bIsGWProfile = bIsGW;
			m_pConfRegDB[i]->m_bIsMSIceUser = IsMSICEUser;
			m_pConfRegDB[i]->m_bIsStandIceUser = IsStandICEUser;
			m_pConfRegDB[i]->m_sipServerType = 	sipServerType;

			PTRACE2INT(eLevelInfoNormal,"CSipProxyServiceManager::InsertConfToDB discovery: ",bIsDiscovery);

			eIpType ipType;

			//if(IsMSICEUser || IsStandICEUser) //MS_IPV6
			//	ipType = eIpType_IpV4;
			//else
				ipType = m_servicesList->GetIpType(serviceId);

			m_pConfRegDB[i]->m_ipType = ipType;

			TRACEINTO << "MS_IPV6: ipType:" << ipType << ", serviceId:" << serviceId << ", IsMSICEUser:" << IsMSICEUser << ", IsStandICEUser:" << IsStandICEUser;
			/*
			//if(m_servicesList->GetIpVersion(serviceId) == eIpVersion4)
			if (eIpType_Both == ipType || eIpType_IpV4 == ipType)
			{
				m_pConfRegDB[i]->m_serviceAddressIpV4.addr.v4.ip = m_servicesList->GetIpV4(serviceId);
				m_pConfRegDB[i]->m_serviceAddressIpV4.ipVersion = eIpVersion4;

			}
			//else if (m_servicesList->GetIpVersion(serviceId) == eIpVersion6)
			if (eIpType_Both == ipType || eIpType_IpV6 == ipType)
			{
				memcpy(m_pConfRegDB[i]->m_serviceAddressIpV6.addr.v6.ip, m_servicesList->GetIpV6(serviceId), IPV6_ADDRESS_BYTES_LEN);
				m_pConfRegDB[i]->m_serviceAddressIpV6.addr.v6.scopeId = m_servicesList->GetIpV6scopeid(serviceId);
				m_pConfRegDB[i]->m_serviceAddressIpV6.ipVersion = eIpVersion6;
			}
			*/
			for(int index=0; index<TOTAL_NUM_OF_IP_ADDRESSES; index++)
			{
				ipAddressStruct ipAddrSt = m_servicesList->GetServiceIpAddress(serviceId,index);
				m_pConfRegDB[i]->SetServiceAddresses(ipAddrSt,index);
			}

			m_pConfRegDB[i]->m_transportType = m_servicesList->GetTransportType(serviceId);
			m_pConfRegDB[i]->m_serversConfig = m_servicesList->GetServersConfig(serviceId);
			if(m_servicesList->GetOutboundProxyIpVersion(serviceId) == eIpVersion4)
			{
				m_pConfRegDB[i]->m_outboundProxyAddress.addr.v4.ip = m_servicesList->GetOutboundProxyIpV4(serviceId);
				m_pConfRegDB[i]->m_outboundProxyAddress.ipVersion = eIpVersion4;
			}
			else if(m_servicesList->GetOutboundProxyIpVersion(serviceId) == eIpVersion6)
			{
				char * pOutboundProxyIpV6 = m_servicesList->GetOutboundProxyIpV6(serviceId);
				if (pOutboundProxyIpV6==NULL)
					PTRACE(eLevelError, "GetOutboundProxyIpV6 returned NULL");
				else
					memcpy(m_pConfRegDB[i]->m_outboundProxyAddress.addr.v6.ip, pOutboundProxyIpV6, IPV6_ADDRESS_BYTES_LEN);
				m_pConfRegDB[i]->m_outboundProxyAddress.addr.v6.scopeId = m_servicesList->GetOutboundProxyV6scopeid(serviceId);
				m_pConfRegDB[i]->m_outboundProxyAddress.ipVersion = eIpVersion6;
			}
			m_pConfRegDB[i]->m_outboundProxyAddress.port = m_servicesList->GetOutboundProxyPort(serviceId);

			char * pOutboundProxyName = m_servicesList->GetOutboundProxyName(serviceId);
			if (pOutboundProxyName==NULL)
				PTRACE(eLevelError, "GetOutboundProxyName returned NULL");
			else
				strncpy(m_pConfRegDB[i]->m_poutboundProxyName, pOutboundProxyName, H243_NAME_LEN);

			m_pConfRegDB[i]->m_DNSStatus = m_DNSStatus;

			ALLOCBUFFER(strHostName, H243_NAME_LEN);

			if(useMainProxy)
			{
				if(m_servicesList->GetProxyIpVersion(serviceId) == eIpVersion4)
				{
					m_pConfRegDB[i]->m_proxyAddress.addr.v4.ip = m_servicesList->GetProxyIpV4(serviceId);
					m_pConfRegDB[i]->m_proxyAddress.ipVersion = eIpVersion4;
				}
				else if(m_servicesList->GetProxyIpVersion(serviceId) == eIpVersion6)
				{
					char * pProxyIpV6 = m_servicesList->GetProxyIpV6(serviceId);
					if (pProxyIpV6==NULL)
						PTRACE(eLevelError, "GetProxyIpV6 returned NULL");
					else
						memcpy(m_pConfRegDB[i]->m_proxyAddress.addr.v6.ip, pProxyIpV6, IPV6_ADDRESS_BYTES_LEN);
					m_pConfRegDB[i]->m_proxyAddress.addr.v6.scopeId = m_servicesList->GetProxyV6scopeid(serviceId);
					m_pConfRegDB[i]->m_proxyAddress.ipVersion = eIpVersion6;
				}
				m_pConfRegDB[i]->m_proxyAddress.port = m_servicesList->GetProxyPort(serviceId);

				char * pProxyName = m_servicesList->GetProxyName(serviceId);
				if (pProxyName==NULL)
					PTRACE(eLevelError, "GetProxyName returned NULL");
				else
					strncpy(m_pConfRegDB[i]->m_pProxyName, pProxyName, H243_NAME_LEN);

				char * pRegHostName = m_servicesList->GetRegHostName(serviceId);
				if (pRegHostName==NULL)
					PTRACE(eLevelError, "GetRegHostName returned NULL");
				else
					strncpy(strHostName, pRegHostName, H243_NAME_LEN);
			}
			else
			{
				if(m_servicesList->GetAlternateProxyIpVersion(serviceId) == eIpVersion4)
				{
					m_pConfRegDB[i]->m_proxyAddress.addr.v4.ip = m_servicesList->GetAlternateProxyIpV4(serviceId);
					m_pConfRegDB[i]->m_proxyAddress.ipVersion = eIpVersion4;
				}
				else if(m_servicesList->GetAlternateProxyIpVersion(serviceId) == eIpVersion6)
				{
					char * pAlternateProxyIpV6 = m_servicesList->GetAlternateProxyIpV6(serviceId);
					if (pAlternateProxyIpV6==NULL)
						PTRACE(eLevelError, "GetAlternateProxyIpV6 returned NULL");
					else
						memcpy(m_pConfRegDB[i]->m_proxyAddress.addr.v6.ip, pAlternateProxyIpV6, IPV6_ADDRESS_BYTES_LEN);
					m_pConfRegDB[i]->m_proxyAddress.addr.v6.scopeId = m_servicesList->GetAlternateProxyV6scopeid(serviceId);
					m_pConfRegDB[i]->m_proxyAddress.ipVersion = eIpVersion6;
				}
				m_pConfRegDB[i]->m_proxyAddress.port = m_servicesList->GetAlternateProxyPort(serviceId);
				char * pAltProxyName = m_servicesList->GetAltProxyName(serviceId);
				if (pAltProxyName==NULL)
					PTRACE(eLevelError, "GetAltProxyName returned NULL");
				else
					strncpy(m_pConfRegDB[i]->m_pProxyName, pAltProxyName, H243_NAME_LEN);

				//YossiG fix Klocwork NPD issue
				char * pAltRegHostName = m_servicesList->GetAltRegHostName(serviceId);
				if (pAltRegHostName==NULL)
					PTRACE(eLevelError, "GetAltRegHostName returned NULL");
				else
					strncpy(strHostName, pAltRegHostName, H243_NAME_LEN);
			}

			strncpy(m_pConfRegDB[i]->m_pHostName,strHostName, H243_NAME_LEN);
			DEALLOCBUFFER(strHostName);
			m_pConfRegDB[i]->m_move = FALSE;
			strncpy( m_pConfRegDB[i]->m_pConfName, confName, H243_NAME_LEN );
			strncpy( m_pConfRegDB[i]->m_dummy_name, dummyName, H243_NAME_LEN - 1); // B.S. klocwork 2584
			m_pConfRegDB[i]->m_dummy_name[H243_NAME_LEN -1]='\0'; // B.S. klocwork 2584
			//m_pConfRegDB[i]->m_expires = expires;
			if(m_servicesList->GetServiceName(serviceId))
				refreshTout = m_servicesList->GetRefreshTout(serviceId);

			m_pConfRegDB[i]->m_expires = refreshTout;
			m_conferencesCounter++;

			return i;
		}
	}

	PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::InsertConfToDB  Not Found empty place");
	return -1;	// not found an empty place
}

/////////////////////////////////////////////////////////////////////////////
int  CSipProxyServiceManager::RemoveFromConfDB( int indInDB )
{
	if(indInDB < MAX_CONF_REGISTRATIONS)
	{
		if (m_pConfRegDB[indInDB]) {
			DWORD serviceId = m_pConfRegDB[indInDB]->m_serviceId;
			BYTE bCreateDummy = TRUE;
			if(strcmp(m_pConfRegDB[indInDB]->m_pConfName, m_Dummy_name) == 0)
				bCreateDummy = FALSE;
			POBJDELETE(m_pConfRegDB[indInDB]);
			m_conferencesCounter--;

			//if last conf - create dummy
			if(0 == m_conferencesCounter && bCreateDummy)
			{
				CreateDummyConf(serviceId);
			}
			return 0;
		}
	}
	return -1;	// conf not found
}


///////////////////////////////////////////////////////////////////////////////
//void  CSipProxyServiceManager::RegConfOnAlternate(DWORD Id, BYTE bStartTimer)
//{
//	DWORD serviceId = 0;
//	int indInDB = 0;
//	BYTE useBusyServer;
//
//	if(!m_pConfRegDB[Id])
//		return;
//
//	serviceId = m_pConfRegDB[Id]->m_serviceId;
//	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::RegConfOnAlternate.");
//
//	//if no alternate proxy is defined, terminate
//	if(Id >= MAX_CONF_REGISTRATIONS ||
//		(0 == m_servicesList->GetAlternateProxyIp(serviceId) && *(m_servicesList->GetAltProxyName(serviceId)) == '\0') ||
//		m_servicesList->GetAlternateProxyIp(serviceId) == m_servicesList->GetProxyIp(serviceId))
//	{
//		PTRACE(eLevelError, "CSipProxyServiceManager::RegConfOnAlternate, No Alternate Proxy Defined.");
//		m_pConfRegDB[Id]->m_status = CONF_IDLE;
//		if(bStartTimer && !m_timerRetry)
//		{
//			StartTimer(TIMER_RETRY_PROXY, RetryTimeOut*SECOND);
//			m_timerRetry = 1;
//		}
//		return;
//	}
//
//	//if tried to unregister and failed, no need to deal with alternate
//	if(m_pConfRegDB[Id] && CONF_UNREGISTERING != m_pConfRegDB[Id]->m_status)
//	{
//		PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::RegConfOnAlternate, confName = ", m_pConfRegDB[Id]->m_pConfName);
//
//		//if first time it registers
//		if(CONF_REGISTERING == m_pConfRegDB[Id]->m_status)
//		{
//			if(m_pConfRegDB[Id]->m_proxyIP == m_servicesList->GetProxyIp(serviceId))
//			{
//				PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::RegConfOnAlternate, Trying Alternate Proxy.");
//				m_pConfRegDB[Id]->m_proxyIP = m_servicesList->GetAlternateProxyIp(serviceId);
//				m_pConfRegDB[Id]->m_proxyPort = m_servicesList->GetAlternateProxyPort(serviceId);
//				if(m_servicesList->GetAltProxyName(serviceId))
//					strncpy(m_pConfRegDB[Id]->m_pProxyName, m_servicesList->GetAltProxyName(serviceId), H243_NAME_LEN);
//				if(m_servicesList->GetAltRegHostName(serviceId))
//					strncpy(m_pConfRegDB[Id]->m_pHostName, m_servicesList->GetAltRegHostName(serviceId), H243_NAME_LEN);
//				m_pConfRegDB[Id]->m_status = CONF_IDLE;
//			}
//			//couldn't even register on alternate
//			else
//			{
//				PTRACE(eLevelError, "CSipProxyServiceManager::RegConfOnAlternate, Both Proxies did not respond.");
//				m_pConfRegDB[Id]->m_proxyIP = m_servicesList->GetProxyIp(serviceId);
//				m_pConfRegDB[Id]->m_proxyPort = m_servicesList->GetProxyPort(serviceId);
//				if(m_servicesList->GetProxyName(serviceId))
//					strncpy(m_pConfRegDB[Id]->m_pProxyName, m_servicesList->GetProxyName(serviceId), H243_NAME_LEN);
//				if(m_servicesList->GetRegHostName(serviceId))
//					strncpy(m_pConfRegDB[Id]->m_pHostName, m_servicesList->GetRegHostName(serviceId), H243_NAME_LEN);
//				m_pConfRegDB[Id]->m_status = CONF_IDLE;
//				if(bStartTimer && !m_timerRetry)
//				{
//					StartTimer(TIMER_RETRY_PROXY, RetryTimeOut*SECOND);
//					m_timerRetry = 1;
//				}
//				useBusyServer = IsUseBusyServer(Id);
//				m_confList->SetBusyServer(Id, useBusyServer);
//				return;
//			}
//		}
//		else
//		{
//			//if conf was already registered
//			if(CONF_REGISTERED == m_pConfRegDB[Id]->m_status)
//			{
//				if(m_pConfRegDB[Id]->m_proxyIP == m_servicesList->GetProxyIp(serviceId))
//				{
//					PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::RegConfOnAlternate, Trying Alternate Proxy.");
//					m_pConfRegDB[Id]->m_proxyIP = m_servicesList->GetAlternateProxyIp(serviceId);
//					m_pConfRegDB[Id]->m_proxyPort = m_servicesList->GetAlternateProxyPort(serviceId);
//					if(m_servicesList->GetAltProxyName(serviceId))
//						strncpy(m_pConfRegDB[Id]->m_pProxyName, m_servicesList->GetAltProxyName(serviceId), H243_NAME_LEN);
//					if(m_servicesList->GetAltRegHostName(serviceId))
//						strncpy(m_pConfRegDB[Id]->m_pHostName, m_servicesList->GetAltRegHostName(serviceId), H243_NAME_LEN);
//					m_pConfRegDB[Id]->m_status = CONF_IDLE;
//				}
//				else
//					if(m_pConfRegDB[Id]->m_proxyIP == m_servicesList->GetAlternateProxyIp(serviceId))
//					{
//						PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::RegConfOnAlternate, Trying Main Proxy.");
//						m_pConfRegDB[Id]->m_proxyIP = m_servicesList->GetProxyIp(serviceId);
//						m_pConfRegDB[Id]->m_proxyPort = m_servicesList->GetProxyPort(serviceId);
//						if(m_servicesList->GetProxyName(serviceId))
//							strncpy(m_pConfRegDB[Id]->m_pProxyName, m_servicesList->GetProxyName(serviceId), H243_NAME_LEN);
//						if(m_servicesList->GetRegHostName(serviceId))
//							strncpy(m_pConfRegDB[Id]->m_pHostName, m_servicesList->GetRegHostName(serviceId), H243_NAME_LEN);
//						m_pConfRegDB[Id]->m_status = CONF_IDLE;
//					}
//			}
//		}
//
//		useBusyServer = IsUseBusyServer(Id);
//		m_confList->SetBusyServer(Id, useBusyServer);
//
//		if(StartRegister( Id ))
//		{
//			indInDB = FindConfInDB( 0, "_dummy_tester1", serviceId );
//			if (-1 != indInDB)
//				DeleteConfCntlAndDB(indInDB);
//		}
//	}
//}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::MoveAllRegsToAlternate(DWORD serviceId, int index)
{
	int i=0, indInDB=0, indInConfs=0;
	char *pAltProxyName = m_servicesList->GetAltProxyName(serviceId);

	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::MoveAllRegsToAlternate.");

	//if no alternate proxy is defined, terminate
       mcTransportAddress	proxyAlternatIp =m_servicesList->GetAlternateProxyAddress(serviceId) ;
       mcTransportAddress	proxyIp =m_servicesList->GetProxyAddress(serviceId) ;

	if((true == ::isApiTaNull(&proxyAlternatIp) && pAltProxyName && *(pAltProxyName) == '\0') ||
	TRUE == ::isIpAddressEqual(&proxyAlternatIp, &proxyIp))
	{
		PTRACE(eLevelError, "CSipProxyServiceManager::MoveAllRegsToAlternate, No Alternate Proxy Defined.");
		if(m_pConfRegDB[index] && serviceId == m_pConfRegDB[index]->m_serviceId)
		{
			if(CONF_IDLE != m_pConfRegDB[index]->m_status)
			{
				m_confList->StopReg(index);
				m_pConfRegDB[index]->m_status = CONF_IDLE;
				m_confList->ResetConfCntl(index);
			}
		}
//		for(i=0; i<MAX_CONF_REGISTRATIONS; i++)
//			if(m_pConfRegDB[i] && serviceId == m_pConfRegDB[i]->m_serviceId)
//			{
//				if(CONF_IDLE != m_pConfRegDB[i]->m_status)
//				{
//					m_confList->StopReg(i);
//					m_pConfRegDB[i]->m_status = CONF_IDLE;
//					m_confList->ResetConfCntl(i);
//				}
//				/*else
//				{
//					m_confList->DeleteConfCntrl(i);
//					RemoveFromConfDB(i);
//				}*/
//			}

		return;
	}

	//if till now accessed the alternate, check if any conf could contact it
	if(!m_UseMainProxy)
	{
		for(i=0; i<MAX_CONF_REGISTRATIONS; i++)
		{
			if(m_pConfRegDB[i] && serviceId == m_pConfRegDB[i]->m_serviceId)
			{
				if(CONF_REGISTERED == m_pConfRegDB[i]->m_status
					|| CONF_UNREGISTERING == m_pConfRegDB[i]->m_status)
					break;
			}
		}
		//no reg on alternate, don't even try to get back to main proxy
		if(i == MAX_CONF_REGISTRATIONS)
		{
			PTRACE(eLevelError, "CSipProxyServiceManager::MoveAllRegsToAlternate, Both Proxies did not respond.");
			for(i=0; i<MAX_CONF_REGISTRATIONS; i++)
				if(m_pConfRegDB[i] && serviceId == m_pConfRegDB[i]->m_serviceId)
					m_pConfRegDB[i]->m_status = CONF_IDLE;

			if(!m_timerRetry)
			{
				StartTimer(TIMER_RETRY_PROXY, RetryTimeOut*SECOND);
				m_timerRetry = 1;
			}
			return;
		}
	}

	m_UseMainProxy = !m_UseMainProxy;

	for(i=0; i<MAX_CONF_REGISTRATIONS; i++)
	{
		if(m_pConfRegDB[i] && serviceId == m_pConfRegDB[i]->m_serviceId && !m_pConfRegDB[i]->m_bIsDiscovery
			&& !m_pConfRegDB[i]->m_remove && !m_pConfRegDB[i]->m_bIsFromOldService)
		{
                        mcTransportAddress	proxyIpDb =m_pConfRegDB[i]->m_proxyAddress;
			if( (m_UseMainProxy && !isIpAddressEqual(&proxyIpDb, &proxyIp ) )
				|| (!m_UseMainProxy && !isIpAddressEqual(&proxyIpDb, &proxyAlternatIp)) )
			{

				if(CONF_IDLE == m_pConfRegDB[i]->m_status)
					DeleteConfCntlAndDB(i);
				else
				{
					m_pConfRegDB[i]->m_status = CONF_IDLE;
					indInDB = InsertConfToDB( m_pConfRegDB[i]->m_confID, m_pConfRegDB[i]->m_pConfName
						, serviceId, m_pConfRegDB[i]->m_expires, m_UseMainProxy, FALSE
						, m_pConfRegDB[i]->m_bIsOngoing, m_pConfRegDB[i]->m_bIsMR, m_pConfRegDB[i]->m_bIsEQ
						, m_pConfRegDB[i]->m_bIsFactory,m_pConfRegDB[i]->m_bIsGWProfile,m_pConfRegDB[i]->m_sipServerType, m_Dummy_name);
					// (create a new conf state-machine (insert to list))
					if (indInDB != -1)
					{
						indInConfs = m_confList->CreateConfCntrl( m_pConfRegDB[indInDB], m_pRcvMbx, m_pMockMplProtocol);
					}
					else
					{
						PASSERT(1);
					}

					if (-1 == indInConfs)
						RemoveFromConfDB( i );

					// (start registration in the Proxy Registrar Server)
					else
					{
						if (StartRegister(indInConfs))
						{
							indInDB = FindConfInDB( 0, m_Dummy_name, serviceId );
							if (-1 != indInDB)
								DeleteConfCntlAndDB(indInDB);
						}
					}

					//m_confList->StopReg(i);
					m_confList->Unregister(i);
					//m_confList->DeleteConfCntrl(i);
					//RemoveFromConfDB(i);
				}
			}
		}
	}

	// update busy server:
	for(i=0; i<MAX_CONF_REGISTRATIONS; i++)
	{
		if(m_pConfRegDB[i] && serviceId == m_pConfRegDB[i]->m_serviceId)
		{
			BYTE useBusyServer = IsUseBusyServer(i);
			m_confList->SetBusyServer(i, useBusyServer);
		}
	}

}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::SetServerBusyStartRetryTimer(DWORD Id, DWORD retryAfterVal)
{
	COstrStream msg;
	msg << "Id=" << Id << " Retry After=" << retryAfterVal;
	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::SetServerBusyStartRetryTimer : ", msg.str().c_str());

	if(!m_pConfRegDB[Id])
	{
		DBGPASSERT(Id? Id: (DWORD)-1);
		return;
	}

	// Add server to busy server list
	AddServerToBusyServersList(Id);

	// Set busy server to all confs that use busy server
	for(int i=0; i<MAX_CONF_REGISTRATIONS; i++)
	{
		if(m_pConfRegDB[i]
		  && (m_servicesList->find(m_pConfRegDB[i]->m_serviceId))
		  && (IsUseBusyServer(i))) // Set busy only confs that use busy proxy
		{
			m_confList->SetBusyServer(i, TRUE);
		}
	}

	// Check if need to start timer / replace old timer
	TICKS newExpiredTime = (SystemGetTickCount().GetIntegerPartForTrace()) + retryAfterVal*SECOND;
	if(newExpiredTime > GetBusyExpiredTime())
	{
		if(IsValidTimer(TIMER_RETRY_BUSY_PROXY))
			DeleteTimer(TIMER_RETRY_BUSY_PROXY);
		StartTimer(TIMER_RETRY_BUSY_PROXY, retryAfterVal*SECOND);
		SetBusyExpiredTime(newExpiredTime);
	}

}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::AddServerToBusyServersList(DWORD Id)
{
	if (m_pConfRegDB[Id])
	{
		StBusyServerParam* pStruct = new StBusyServerParam;
		memset(pStruct, 0, sizeof(StBusyServerParam));
		memcpy(&(pStruct->m_proxyAddress), &(m_pConfRegDB[Id]->m_proxyAddress), sizeof(mcTransportAddress));

		if (m_pConfRegDB[Id]->m_pProxyName)
		{
			strncpy(pStruct->m_pProxyName, m_pConfRegDB[Id]->m_pProxyName, H243_NAME_LEN - 1);
			pStruct -> m_pProxyName[H243_NAME_LEN - 1] = 0;
		}

		// Dump:
		COstrStream msg;
		msg << "Proxy Ip=";
		if(eIpVersion4 == pStruct->m_proxyAddress.ipVersion)
		{
			char strIp[128];
			SystemDWORDToIpString(pStruct->m_proxyAddress.addr.v4.ip, strIp);
			msg << strIp;
		}
		else if(eIpVersion6 == pStruct->m_proxyAddress.ipVersion)
		{
			msg << pStruct->m_proxyAddress.addr.v6.ip;
		}
		msg << " Proxy Name=" << pStruct->m_pProxyName;
		PTRACE2(eLevelInfoNormal, "COneConf::AddServerToBusyServersList : ", msg.str().c_str());

		m_BusyProxyVector.push_back(pStruct);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Registrations on oldMaster are unregistered from the new card,
/*void  CSipProxyServiceManager::RemoveRegistrationsUsingOtherCard()
{
	int errorFound = 0, indInDB = -1, indInConfs = -1, i = 0;
	WORD tempMasterCard = (WORD)-1, tempOldMasterCard = (WORD)-1;
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::RemoveRegistrationsUsingOtherCard.");

	//check if any card has an 'old' master card. otherwise, no need to move regs
	for (i = 0; i < MAX_CARDS_IN_MCU; i++)
		if(m_oldMasterCardId[i] != (WORD)-1)
			break;
	if(i == MAX_CARDS_IN_MCU)
		return;

	for (i = 0; i < MAX_CONF_REGISTRATIONS; i++)
	{
		//find registrations on old master
		if (m_pConfRegDB[i] && !m_pConfRegDB[i]->m_remove && IsOldMasterCard(m_pConfRegDB[i]->m_cardID))
		{
			tempOldMasterCard = m_pConfRegDB[i]->m_cardID;

			if(CONF_IDLE == m_pConfRegDB[i]->m_status)
			{
				tempMasterCard = m_servicesList->GetServiceMasterCard(m_pConfRegDB[i]->m_cardID);
				if((WORD)-1 != tempMasterCard)
				{
					m_pConfRegDB[i]->m_cardID = tempMasterCard;
					m_confList->Unregister(i);
				}
			}
			m_oldMasterCardId[tempMasterCard] = (WORD)-1;
		}
	}
}*/


/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnRegistrarStatus(CSegment* pParam)
{
	WORD	status = STATUS_OK;
	WORD 	serviceId = (WORD)-1;
	DWORD	confID = (DWORD)-1;
	WORD 	confType = (WORD)-1;
//	DWORD	proxyIp = 0;
	DWORD	expire = 0;
	char	proxyName[H243_NAME_LEN]="";

	*pParam >> status
			>> serviceId
			>> confID
			>> confType
			>> expire
			>> proxyName;

	mcTransportAddress	proxyIp;
	memset(&proxyIp, 0, sizeof(mcTransportAddress));
	pParam->Get((BYTE *)&proxyIp, sizeof(mcTransportAddress));

	SetRegistrarStatus(serviceId, proxyName, status, proxyIp, confID, confType,expire);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::SetRegistrarStatus(WORD serviceID, char* proxyName, DWORD status, mcTransportAddress proxyIp, DWORD confId, WORD confType,DWORD expire, BYTE bSetActiveAlarm)
{
	WORD line = 0xFFFF;
	eServerType role = eServerTypeNotAvailable;

	char s[H243_NAME_LEN*2];
	if(eIpVersion4 == proxyIp.ipVersion)
    	snprintf(s, sizeof(s), "%d, name=%s, ip=%d, confType=%u, confId=%u", status, proxyName, proxyIp.addr.v4.ip,confType,confId);
    if(eIpVersion6 == proxyIp.ipVersion)
    {
    	char proxyIpStr[IPV6_ADDRESS_LEN];
    	ipV6ToString(proxyIp.addr.v6.ip, proxyIpStr, TRUE);
    	snprintf(s, sizeof(s), "%d, name=%s, ip=%s, confType=%u, confId=%u", status, proxyName, proxyIpStr,confType,confId);
    }
	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::SetRegistrarStatus, status = ", s);

	BOOL	bIsAnyRegisteredConf   = FALSE;
	BOOL	bIsAnyFailedConf	 = FALSE;
	BOOL	bIsConfDummy	= FALSE;
	for(int index=0; index< MAX_CONF_REGISTRATIONS; index++)
	{
		if(!m_pConfRegDB[index])
			continue;
		if(m_pConfRegDB[index]->m_confID == confId)
			continue;
		if(m_pConfRegDB[index]->m_confID == DUMMY_ENTRY)
		{
			if(eSipServerStatusTypeOk== m_pConfRegDB[index]->m_lastRegStatus)
			{
				PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::SetRegistrarStatus - there's dummy conf successfully connected wtih Proxy!");
				bIsAnyRegisteredConf  = TRUE;
			}
			bIsConfDummy= TRUE;
			continue;
		}
		if(eSipServerStatusTypeOk== m_pConfRegDB[index]->m_lastRegStatus)
		{
			PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::SetRegistrarStatus - there's other conf successfully connected wtih Proxy!");
			bIsAnyRegisteredConf  = TRUE;
		}
		if(eSipServerStatusTypeFail == m_pConfRegDB[index]->m_lastRegStatus)
		{
			PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::SetRegistrarStatus - there's other conf failed to connect wtih Proxy!");
			bIsAnyFailedConf  = TRUE;
		}
	}


	//if present status is what we need, return;
	eSipServerStatusType serverStatus;
	if(STATUS_OK == status)
		serverStatus = eSipServerStatusTypeOk;
	else
		serverStatus = eSipServerStatusTypeFail;

	if((DWORD)serverStatus == m_servicesList->GetRegistrarStatus(serviceID))
	{
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::SetRegistrarStatus 1:serviceID =",serviceID);
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::SetRegistrarStatus 1:confId = ",confId);
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::SetRegistrarStatus 1: confType = ",confType);
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::SetRegistrarStatus 1:registartionStatus = ",status);
		SetConfRegistrationStatus(serviceID, confId, confType, status,expire);

		if(eSipServerStatusTypeFail == serverStatus || bIsAnyFailedConf)
		{
			// there's some Conf Reg failed, set AA_REGISTER_FAILED_FOR_SOME_CONF
			if (bSetActiveAlarm)
			{
				AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
										AA_REGISTER_FAILED_FOR_SOME_CONF,
										SYSTEM_MESSAGE,
										"Registration failed for some Conference",
										true,
										true);
			}

		}
		else
		{
			//there's no Conf Reg Failed -- Clear AA_REGISTER_FAILED_FOR_SOME_CONF, 			
			RemoveActiveAlarmByErrorCode(AA_REGISTER_FAILED_FOR_SOME_CONF);
		}		
		return;
	}

	CSipProxyIpParams* pService = m_servicesList->find(serviceID);
	if (pService)
	{
		WORD numRegistrars = 0;
//		DWORD altRegistrarIp = 0, registrarIp = 0, outProxyIp = 0;
		mcTransportAddress altRegistrarIp, registrarIp, outProxyIp;
		memset(&altRegistrarIp, 0, sizeof(mcTransportAddress));
		memset(&registrarIp, 0, sizeof(mcTransportAddress));
		memset(&outProxyIp, 0, sizeof(mcTransportAddress));

		ALLOCBUFFER(outProxyName, H243_NAME_LEN);
		ALLOCBUFFER(registrarName, H243_NAME_LEN);
		ALLOCBUFFER(altRegistrarName, H243_NAME_LEN);
		outProxyName[0] = '\0';
		registrarName[0] = '\0';
		altRegistrarName[0] = '\0';

		if(eConfSipServerManually == pService->GetServersConfig())
		{
			//outProxyIp = pService->GetOutboundProxyIpV4();
			outProxyIp = pService->GetOutboundProxyAddress();
			strncpy(outProxyName, pService->GetOutboundProxyName(), H243_NAME_LEN);

			registrarIp = pService->GetProxyAddress();
			//if(0 != registrarIp)
			if(!isApiTaNull(&registrarIp))
				numRegistrars = 1;
			strncpy(registrarName, pService->GetProxyName(), H243_NAME_LEN);

			strncpy(altRegistrarName, pService->GetAltProxyName(), H243_NAME_LEN);
			//altRegistrarIp = pService->GetAltProxyIpV4();
			altRegistrarIp = pService->GetAltProxyAddress();
			//if(0 != altRegistrarIp)
			if(!isApiTaNull(&altRegistrarIp))
				numRegistrars = 2;

			//if proxy is primary
			if(!strncmp(proxyName, registrarName, H243_NAME_LEN))
				role = eServerTypePrimary;
			else
				if(!strncmp(proxyName, altRegistrarName, H243_NAME_LEN))
					role = eServerTypeAlternate;
		}
		/*else
		{
		outProxyIp = pService->Geto
		strncpy(outProxyName, cardSip.GetPrimaryProxyName().GetString(), H243_NAME_LEN);
		registrarIp = cardSip.GetPrimaryRegistrarIP();
		strncpy(registrarName, cardSip.GetPrimaryProxyName().GetString(), H243_NAME_LEN);
		numRegistrars = 1;
		}

		  cardSip.SetServersParamsFromDNS(
		  1,
		  sipService->GetpProxy()->GetStatus(),
		  proxyIp,
		  proxyName,
		  sipService->GetpAltProxy()->GetStatus(),
		  sipService->GetpAltProxy()->GetIpAddress(),
		  sipService->GetpAltProxy()->GetName(),
		  numRegistrars,
		  sipService->GetpRegistrar()->GetStatus(),
		  registrarIp,
		  registrarName,
		  sipService->GetpAltRegistrar()->GetStatus(),
		  altRegistrarIp,
		  altRegistrarName);

			if(m_UseMainProxy)
			cardSip.SetPrimaryRegistrarStatus(status);
			else
			cardSip.SetAltRegistrarStatus(status);
		cardSip.SetPrimaryProxyStatus(status);*/

		DEALLOCBUFFER(outProxyName);
		DEALLOCBUFFER(registrarName);
		DEALLOCBUFFER(altRegistrarName);

		
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::SetRegistrarStatus 2:serviceID =",serviceID);
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::SetRegistrarStatus 2:confId = ",confId);
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::SetRegistrarStatus 2: confType = ",confType);
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::SetRegistrarStatus 2:registartionStatus = ",status);

		SetConfRegistrationStatus(serviceID, confId, confType, status,expire);

		if(eSipServerStatusTypeOk== serverStatus||bIsAnyRegisteredConf)
		{
			DWORD  tmpStatus = STATUS_OK;
			m_servicesList->SetRegistrarStatus(serviceID, role, tmpStatus, proxyIp);
			//there's some Conf Succ, clear AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_SIP_REGISTRAR
			RemoveActiveAlarmByErrorCode(AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_SIP_REGISTRAR);
		}
		else
		{		
			m_servicesList->SetRegistrarStatus(serviceID, role, status, proxyIp);
			//there's no Conf Reg Succ  -- Proxy may be not OK, set AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_SIP_REGISTRAR
			if (bSetActiveAlarm)
			{
				AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
										AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_SIP_REGISTRAR,
										SYSTEM_MESSAGE,
										"Can not establish connection with SIP registrar",
										true,
										true);
			}

		}
		
		//This Alarm is triggerred by non-dummy Conf
		if(!bIsConfDummy)
		{		
			if(eSipServerStatusTypeFail == serverStatus || bIsAnyFailedConf)
			{
				// there's some Conf Reg failed, set AA_REGISTER_FAILED_FOR_SOME_CONF
				if (bSetActiveAlarm)
				{
					AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
											AA_REGISTER_FAILED_FOR_SOME_CONF,
											SYSTEM_MESSAGE,
											"Registration failed for some Conference",
											true,
											true);
				}

			}
			else
			{
				//there's no Conf Reg Failed -- Clear AA_REGISTER_FAILED_FOR_SOME_CONF, 			
				RemoveActiveAlarmByErrorCode(AA_REGISTER_FAILED_FOR_SOME_CONF);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::CreateDummyConf(WORD serviceId)
{
    //--- Sasha.  Flag:DISABLE_DUMMY_REGISTRATION
    {//---Sasha Sh. Disable/Enable dummy_tester1  REGISTER---------------------------------------------//
        BOOL bIsDisableDummyRegistration = 0;
        std::string key = "DISABLE_DUMMY_REGISTRATION";
        CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key, bIsDisableDummyRegistration);
        if(true == bIsDisableDummyRegistration)
        {
            PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::CreateDummyConf - Flag:DISABLE.");
            return;
        }
    }//------------------------------------------------------------------------------------------------//
	// just to be on the safe side...
	if (!m_servicesList->find(serviceId)) {	// error
		return;
	}

	int indInDB = -1, errorFound = 0, indInConfs = -1;
	ALLOCBUFFER(str, 50);

	char s[4];
    snprintf(s, sizeof(s), "%d", serviceId);
	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::CreateDummyConf, serviceId = ", s);

	indInDB = InsertConfToDB( 0, m_Dummy_name, serviceId, 0, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, m_Dummy_name, 0 );
	if (-1 == indInDB)
	{
			errorFound = ADD_CONF_DB_IS_FULL;
			AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
									SIP_REGISTRATION_LIMIT_REACHED,
									SYSTEM_MESSAGE,
									"SIP registrations limit reached",
									true,
									true
								);
	}
	else
		RemoveActiveAlarmByErrorCode(SIP_REGISTRATION_LIMIT_REACHED);

	// (create a new conf state-machine (insert to list))
	indInConfs = -1;
	if (!errorFound)
	{
		indInConfs = m_confList->CreateConfCntrl( m_pConfRegDB[indInDB], m_pRcvMbx, m_pMockMplProtocol);
		if (-1 == indInConfs) {
			// RemoveFromConfDB( indInConfs );
			errorFound = ADD_CONF_CNTL_CREATE_ERROR;
		}
	}

	// (start registration in the Proxy Registrar Server)
	if (!errorFound)
	{
		if (-1 == m_confList->Unregister(indInConfs)) {
			DeleteConfCntlAndDB( indInConfs );
			errorFound = ADD_CONF_START_REGISTRATION_ERROR;
		}
	}

	if(errorFound)
	{

		snprintf(str, sizeof(str), "%d, error = %d", serviceId, errorFound);
		PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::CreateDummyConf, failed to reg on service=", str);

	}
	DEALLOCBUFFER(str);
}



/////////////////////////////////////////////////////////////////////////////
/*BYTE  CSipProxyServiceManager::CheckPasswords(CCommRes* pCommRes, CSipHeaderList* pTemp)
{
	BYTE bParty = FALSE;
	const CSipHeader* pAuth = pTemp->GetNextHeader(kAuthorization);
	if(pAuth)
	{
		const char* pAuthStr = pAuth->GetHeaderStr();
		if (pAuthStr)
		{
			char* partyStr = strstr(pAuthStr,"Participant");
			if (partyStr)
			{
				char temp[CONFERENCE_ENTRY_PASSWORD_LEN] = "";
				char* passStart = strstr(partyStr,"\"") + 1;
				char* passEnd	= strstr(passStart+1,"\"");
				int len = passEnd-passStart;
				strncpy(temp, passStart, len);
				if(!strcmp(pCommRes->GetEntryPassword(), ""))
					bParty = TRUE;
				else
					if (!strncmp(temp, pCommRes->GetEntryPassword(), CONFERENCE_ENTRY_PASSWORD_LEN))
						bParty = TRUE;
			}
		}
	}
	return bParty;
}*/

/////////////////////////////////////////////////////////////////////////////
/*STATUS CSipProxyServiceManager::HandleTerminalDumpSipProxy(CTerminalCommand & command, std::ostream& answer)
{
	COstrStream msg;
	Dump(msg);
	answer << msg.str().c_str();

    return STATUS_OK;
}*/

/////////////////////////////////////////////////////////////////////////////
int CSipProxyServiceManager::StartRegister(int index)
{
	PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::StartRegister");

	//_M_S_ - PATCH for multiple services
	DWORD delay = 0;
	if(!m_bSystemMultipleServices)
		delay = CalculateDelay();

	int errorFound = 0;
	if (-1 == m_confList->StartRegister(index, delay))
	{
		DeleteConfCntlAndDB(index);
		errorFound = ADD_CONF_START_REGISTRATION_ERROR;
	}
	return errorFound;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CSipProxyServiceManager::CalculateDelay()
{
	DWORD retDelay = 0;         			// retrun delay value in TICKS.
	TICKS currentTicksTime = 0; 			// current time in Ticks.
	DWORD computedRegisterDelay = 0;		// The computed needed delay.
	TICKS currentGapInTicks = 0; 			// gap between current time and the last register request time.
	TICKS totalGapFromFirstRequest = 0; 	// gap between current time and the first register request time in the current blast session.
	DWORD supposedRegisterRequestTime = 0;	// The supposed time after computing the needed delay.
	DWORD absGap = 0;						// absolute gap between current time and the last register request time.
	DWORD systemRegisterDelay = 300;			// System configuration for delay between register requests.

//	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
//	sysConfig->GetDWORDDataByKey("SIP_REGISTER_DELAY_MILLI_SEC", systemRegisterDelay);
	if( pServiceSysCfg != NULL )
	    pServiceSysCfg->GetDWORDDataByKey(m_ServiceId, "SIP_REGISTER_DELAY_MILLI_SEC", systemRegisterDelay);
	systemRegisterDelay = systemRegisterDelay/10;	// From miliseconds to Ticks

	if (m_firstRegisterTicksTime == 0) // First register request
	{	// No need for delay
		m_firstRegisterTicksTime = m_lastRegisterTicksTime = SystemGetTickCount().GetIntegerPartForTrace();
		m_numOfBlastRegRequests = 1;
	}
	else // Start computing delay algorithm
	{
		// Step 2: Compute current gap and check we need to add delay
		currentTicksTime = SystemGetTickCount().GetIntegerPartForTrace();

		//Get the gap between the last register request and this new request
		currentGapInTicks = currentTicksTime - m_lastRegisterTicksTime;

		//Get the Gap between the first register request and this new request
		totalGapFromFirstRequest = currentTicksTime - m_firstRegisterTicksTime;

		//Number of blast requests * system Cfg delay - total gap from first request in this blast session
		if ((m_numOfBlastRegRequests*systemRegisterDelay) > totalGapFromFirstRequest.GetIntegerPartForTrace())
			computedRegisterDelay = (m_numOfBlastRegRequests*systemRegisterDelay) - totalGapFromFirstRequest.GetIntegerPartForTrace();
		else
			computedRegisterDelay = 0;

		//Compute the time of the new register request
		supposedRegisterRequestTime = currentTicksTime.GetIntegerPartForTrace() + computedRegisterDelay;

		//Compute the gap
		absGap = abs((int)(m_lastRegisterTicksTime.GetIntegerPartForTrace() - currentTicksTime.GetIntegerPartForTrace()));
		if ( (currentGapInTicks < systemRegisterDelay) || (absGap <  systemRegisterDelay) )
		{
			// Should set a delay
			retDelay = computedRegisterDelay;
			m_lastRegisterTicksTime = supposedRegisterRequestTime;
			m_numOfBlastRegRequests++;
		}
		else
		{
			// Restart the whole procedure with the last register request as the first request for this session
			m_numOfBlastRegRequests = 1;
		  	m_firstRegisterTicksTime = m_lastRegisterTicksTime = currentTicksTime;
		}
	}
	PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::CalculateDelay : delay=", retDelay);
	return retDelay;
}

/////////////////////////////////////////////////////////////////////////////
void CSipProxyServiceManager::DeleteConfCntlAndDB(int index)
{
	m_confList->DeleteConfCntrl(index);
	RemoveFromConfDB(index);
}

/////////////////////////////////////////////////////////////////////////////
void CSipProxyServiceManager::CreateDummyConfIfDBisEmpty(WORD serviceId)
{
	int i = 0;
	for (i = 0; i < MAX_CONF_REGISTRATIONS; i++)
	{
		if(m_pConfRegDB[i] && serviceId == m_pConfRegDB[i]->m_serviceId && FALSE == m_pConfRegDB[i]->m_bIsFromOldService)
			break;
	}
	if(MAX_CONF_REGISTRATIONS == i)
	{
		PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::CreateDummyConfIfDBisEmpty - DB is empty , create DummyConf ");
		CreateDummyConf(serviceId);
	}
}

//===========================================================================
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// COneConfRegistration
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//===========================================================================


//===========================================================================
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CConfList
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//===========================================================================

/////////////////////////////////////////////////////////////////////////////
CConfList::CConfList()
{
	for (int i = 0; i < MAX_CONF_REGISTRATIONS; i++)
		m_pConfList[i] = 0;
}

/////////////////////////////////////////////////////////////////////////////
CConfList::~CConfList()
{
	for (int i = 0; i < MAX_CONF_REGISTRATIONS; i++)
		POBJDELETE( m_pConfList[i] );
}

/////////////////////////////////////////////////////////////////////////////
void CConfList::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	DWORD Id = 0;
	*pMsg >> Id;
	pMsg->ResetRead();

	WORD localId = (WORD)(Id >> 16);
	if(localId<MAX_CONF_REGISTRATIONS)
	{
		if(m_pConfList[localId])
			m_pConfList[localId]->HandleEvent(pMsg, msgLen, opCode);
		else
			DBGPASSERT(Id? Id: (DWORD)-1);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CConfList::HandleEventByIndex(OPCODE opcode, DWORD Id, CSegment* pMsg)
{
	if(Id<MAX_CONF_REGISTRATIONS)
	{
		if(m_pConfList[Id])
			m_pConfList[Id]->HandleEventByIndex(opcode, pMsg);
		else
			DBGPASSERT(Id? Id: (DWORD)-1);
	}
}

/////////////////////////////////////////////////////////////////////////////
DWORD CConfList::FindIdByCallId(char* callId)
{
	DWORD result = 0xFFFFFFFF;
	for (int i = 0; i < MAX_CONF_REGISTRATIONS && m_pConfList[i]; i++)
	{
		if(!strncmp(m_pConfList[i]->GetCallId(), callId, MaxLengthOfSipCallId))
		{
			PTRACE(eLevelInfoNormal, "CConfList::FindIdByCallId, Found matching Id.");
			result = m_pConfList[i]->GetId();
			break;
		}
	}
	return result;
}


/////////////////////////////////////////////////////////////////////////////
int CConfList::CreateConfCntrl( COneConfRegistration *data, COsQueue* pRcvMbx, CMplMcmsProtocol* pMockMplProtocol,CSipProxyIpParams* pService)
{
	unsigned int i;
	if(strcmp(data->m_pConfName, data->m_dummy_name) == 0 || data->m_bIsMSIceUser) // should add also dtandard ICE user??
		i=DUMMY_ENTRY;
	else
		i=1;
    {//---Sasha Sh. Disable/Enable dummy_tester1  REGISTER---------------------------------------------//
        BOOL bIsDisableDummyRegistration = 0;
        std::string key = "DISABLE_DUMMY_REGISTRATION";
        CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key, bIsDisableDummyRegistration);
        if(true == bIsDisableDummyRegistration)
        {
            PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::CreateDummyConf - Flag:DISABLE.");
            i=1;
        }
    }//------------------------------------------------------------------------------------------------//
//	{
//		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
//		BOOL bSipRegisterOnlyOnce = FALSE;
//		pSysConfig->GetBOOLDataByKey("SIP_REGISTER_ONLY_ONCE", bSipRegisterOnlyOnce);
//		    if (bSipRegisterOnlyOnce)
//		    {
//		    	//PTRACE (eLevelInfoNormal, "CConfList::CreateConfCntrl : SIP_REGISTER_ONLY_ONCE is set to YES -not register conf");
//		    	return -1;	// error
//		    }
//		i=1;
//	}


	for (; i < MAX_CONF_REGISTRATIONS; i++) {
		if (0 == m_pConfList[i])
		{
			PTRACE2INT(eLevelInfoNormal,"CConfList::CreateConfCntrl free index: ",i);
			if (data->m_bIsMSIceUser)
				m_pConfList[i] = new CMsIceUser;
			else if (data->m_bIsStandIceUser)
				m_pConfList[i] = new CStandIceUser;

			else
			{
				BOOL bMsEnviroment = FALSE;
				if(data->m_sipServerType == eSipServer_ms)
					bMsEnviroment = TRUE;

				if (bMsEnviroment)
					m_pConfList[i] = new CMsOneConf;
				else
					m_pConfList[i] = new COneConf;
			}
			int retValue = 0;

			if (data->m_bIsStandIceUser)
			{
				if(pService)
				{
					retValue = ((CStandIceUser*)m_pConfList[i])->Create( data, i, pRcvMbx, pMockMplProtocol,pService);
				}
			}
			else
				retValue = m_pConfList[i]->Create( data, i, pRcvMbx, pMockMplProtocol);

			if (-1 == retValue)
			{
				POBJDELETE(m_pConfList[i]);
				return -1;	// error
			}

			return i;	// index in list
		}
		else	PTRACE2INT(eLevelInfoNormal,"CConfList::CreateConfCntrl m_pConfList[i] is occupied:",i);
	}

	return -1;	// error
}

/////////////////////////////////////////////////////////////////////////////
int CConfList::DeleteConfCntrl( int indInConfs )
{
	char s[5];
	snprintf(s, sizeof(s), "%d", indInConfs);
	PTRACE2(eLevelInfoNormal, "CConfList::DeleteConfCntrl, ind = ", s);

	if (indInConfs >= MAX_CONF_REGISTRATIONS)
		return -1;
	if (!m_pConfList[indInConfs])
		return -1;

	POBJDELETE(m_pConfList[indInConfs]);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int CConfList::StartRegister( int indInConfs, DWORD delay )
{
	if (indInConfs >= MAX_CONF_REGISTRATIONS)
		return -1;
	if (!m_pConfList[indInConfs])
		return -1;

	m_pConfList[indInConfs]->StartRegister(delay);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int CConfList::Unregister( int indInConfs )
{
	if (!m_pConfList[indInConfs])
		return -1;
	m_pConfList[indInConfs]->Unregister();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int  CConfList::StopReg(int indInConfs )
{
	if (!m_pConfList[indInConfs])
		return -1;
	m_pConfList[indInConfs]->StopReg();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int  CConfList::ResetConfCntl( int indInConfs )
{
	if (!m_pConfList[indInConfs])
		return -1;
	m_pConfList[indInConfs]->ResetConfCntl();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int  CConfList::SetBusyServer(int indInConfs, BYTE busyServer)
{
	if (indInConfs >= MAX_CONF_REGISTRATIONS)
			return -1;
	if (!m_pConfList[indInConfs])
			return -1;
	m_pConfList[indInConfs]->SetBusyServer(busyServer);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int CConfList::ChangePresence( int indInConfs ,BYTE presenceState)
{
	if (!m_pConfList[indInConfs])
		return -1;
	m_pConfList[indInConfs]->ChangePresence(presenceState);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int CConfList::SetTransportErrorState(int indInConfs)
{
	PTRACE(eLevelInfoNormal, "CConfList::SetTransportErrorState ");
	if (!m_pConfList[indInConfs])
		return -1;
	m_pConfList[indInConfs]->SetTransportErrorState();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int CConfList::DialogRecoveryMessageInd(int indInConfs, DWORD partyId, DWORD connId)
{
  if (!m_pConfList[indInConfs] || !IsValidPObjectPtr(m_pConfList[indInConfs]) )
    return -1;
  m_pConfList[indInConfs]->AddPartyToRegisterWaitingPartiesVector(partyId, connId);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
int CConfList::MsKeepAliveToutErrInd(int indInConfs, DWORD errCode)
{
  if (!m_pConfList[indInConfs])
    return -1;
  m_pConfList[indInConfs]->OnMsKeepAliveToutErrInd(errCode);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
int CConfList::SendCrlfMessageOnTransportError(int indInConfs)
{
	PTRACE(eLevelInfoNormal, "CConfList::SetTransportErrorState ");
	if (!m_pConfList[indInConfs])
		return -1;
	m_pConfList[indInConfs]->SendCrlfMessageOnTransportError();
	return 0;
}

//===========================================================================
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CServicesList
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//===========================================================================

/////////////////////////////////////////////////////////////////////////////
CServicesList::CServicesList()
{
}


/////////////////////////////////////////////////////////////////////////////
CServicesList::~CServicesList()
{
	CSipProxyIpParams*  pSipProxyIpParams = NULL;
	std::vector< CSipProxyIpParams * >::iterator itr = m_services.begin();
	while(itr != m_services.end())
	{
		pSipProxyIpParams = (*itr);
		m_services.erase(itr);
		POBJDELETE(pSipProxyIpParams);
		itr = m_services.begin();
	}
}

/////////////////////////////////////////////////////////////////////////////
CSipProxyIpParams*  CServicesList::GetFirstService()
{
	CSipProxyIpParams* pSipProxyIpParams = NULL;
	if(m_services.size())
		pSipProxyIpParams = *(m_services.begin());
	return pSipProxyIpParams;
}

/////////////////////////////////////////////////////////////////////////////
CSipProxyIpParams*  CServicesList::GetNextService(int serviceID)
{
	CSipProxyIpParams*  pSipProxyIpParams = NULL;
	std::vector< CSipProxyIpParams * >::iterator itr =  m_services.begin();
	while (itr != m_services.end())
	{
		pSipProxyIpParams = (CSipProxyIpParams*)(*itr);
		if ( (DWORD)serviceID == pSipProxyIpParams->GetServiceId() )
		{
			itr++;
			if(itr == m_services.end())
				break;
			else
			{
				pSipProxyIpParams = (CSipProxyIpParams*)(*itr);
				return pSipProxyIpParams;
			}
		}
		itr++;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
CSipProxyIpParams*  CServicesList::find(int serviceID)
{
	CSipProxyIpParams*  pSipProxyIpParams = NULL;
	std::vector< CSipProxyIpParams * >::iterator itr =  m_services.begin();
	while (itr != m_services.end())
	{
		pSipProxyIpParams = (CSipProxyIpParams*)(*itr);
		if ( (DWORD)serviceID == pSipProxyIpParams->GetServiceId() )
		{
			return pSipProxyIpParams;
		}
		itr++;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
int   CServicesList::Insert( CSipProxyIpParams* pService )
{
    // Since we can receive service that its mode is off, we should check the service mode.
    if (pService->GetServersConfig() == eConfSipServerAuto/*=off*/) // if sip server is off don't insert it.
	{
		PTRACE(eLevelInfoNormal, "CServicesList::Insert - service isn't inserted. Sip server is off");
		return STATUS_FAIL;
	}
	if (find(pService->GetServiceId()))
        Remove(pService->GetServiceId());
	
	PTRACE2(eLevelInfoNormal, "CServicesList::Insert, serviceName: ", pService->GetServiceName());
	m_services.push_back(pService);
	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
void   CServicesList::Remove( int serviceID )
{
	PTRACE(eLevelInfoNormal, "CServicesList::Remove");
	CSipProxyIpParams*  pSipProxyIpParams = NULL;
	std::vector< CSipProxyIpParams * >::iterator itr;
	for(itr = m_services.begin(); itr != m_services.end(); itr++)
	{
		pSipProxyIpParams = (CSipProxyIpParams*)(*itr);
		if ( (DWORD)serviceID == pSipProxyIpParams->GetServiceId() )
		{
			m_services.erase(itr);
            if (IsValidPObjectPtr(pSipProxyIpParams))
                POBJDELETE(pSipProxyIpParams);
			break;
		}
		itr++;
	}
}

/////////////////////////////////////////////////////////////////////////////
eIpType CServicesList::GetIpType(int serviceID)
{
	eIpType result = eIpType_None;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetIpType();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
/*
DWORD CServicesList::GetIpV4(int serviceID)
{
	DWORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetIpV4();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
char* CServicesList::GetIpV6(int serviceID)
{
	char* result = NULL;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetIpV6();
		//memcpy( result, pService->GetIpV6(), IPV6_ADDRESS_BYTES_LEN );
	return result;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CServicesList::GetIpV6scopeid(int serviceID)
{
	DWORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetIpV6scopeId();
	return result;
}
*/
/*
/////////////////////////////////////////////////////////////////////////////
DWORD CServicesList::GetIpVersion(int serviceID)
{
	DWORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetIpVersion();
	return result;
}
*/
/////////////////////////////////////////////////////////////////////////////
ipAddressStruct	CServicesList::GetServiceIpAddress(int serviceID, WORD index)
{
	ipAddressStruct result;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		//memcpy(&result, &pService->GetIpAddress(), sizeof(mcTransportAddress));
		result = pService->GetServiceIpAddress(index);
	else
	{
		PASSERTSTREAM(TRUE, "Service for id " << serviceID << " is not found.");
		memset(&result, 0xFF, sizeof(result));
	}
		
	return result;
}
/////////////////////////////////////////////////////////////////////////////
/*
mcTransportAddress	CServicesList::GetIpAddressIpV4(int serviceID)
{
	mcTransportAddress result;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		//memcpy(&result, &pService->GetIpAddress(), sizeof(mcTransportAddress));
		result = pService->GetIpAddressIpV4();
	return result;
}
/////////////////////////////////////////////////////////////////////////////
mcTransportAddress	CServicesList::GetIpAddressIpV6(int serviceID)
{
	mcTransportAddress result;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		//memcpy(&result, &pService->GetIpAddress(), sizeof(mcTransportAddress));
		result = pService->GetIpAddressIpV6();
	return result;
}
*/
/////////////////////////////////////////////////////////////////////////////
DWORD CServicesList::GetOutboundProxyIpV4(int serviceID)
{
	DWORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetOutboundProxyIpV4();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
char* CServicesList::GetOutboundProxyIpV6(int serviceID)
{
	char* result = NULL;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetOutboundProxyIpV6();
		//memcpy( result, pService->GetOutboundProxyIpV6(), IPV6_ADDRESS_BYTES_LEN );
	return result;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CServicesList::GetOutboundProxyIpVersion(int serviceID)
{
	DWORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetOutboundProxyIpVersion();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CServicesList::GetOutboundProxyV6scopeid(int serviceID)
{
	DWORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetOutboundProxyV6scopeId();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
WORD CServicesList::GetOutboundProxyPort(int serviceID)
{
	WORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetOutboundProxyPort();
	return result;
}
/////////////////////////////////////////////////////////////////////////////
mcTransportAddress	CServicesList::GetOutboundProxyAddress(int serviceID)
{
	mcTransportAddress result;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		//memcpy(&result, &pService->GetOutboundProxyAddress(), sizeof(mcTransportAddress));
		result = pService->GetOutboundProxyAddress();
	else
	{
		PASSERTSTREAM(TRUE, "Service for id " << serviceID << " is not found.");
		memset(&result, 0xFF, sizeof(result));
	}
	
	return result;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CServicesList::GetProxyIpV4(int serviceID)
{
	DWORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetProxyIpV4();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
char* CServicesList::GetProxyIpV6(int serviceID)
{
	char* result = NULL;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetProxyIpV6();
		//memcpy( result, pService->GetProxyIpV6(), IPV6_ADDRESS_BYTES_LEN );
	return result;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CServicesList::GetProxyV6scopeid(int serviceID)
{
	DWORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetProxyV6scopeId();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
WORD CServicesList::GetProxyPort(int serviceID)
{
	WORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetProxyPort();
	return result;
}
/////////////////////////////////////////////////////////////////////////////
mcTransportAddress	CServicesList::GetProxyAddress(int serviceID)
{
	mcTransportAddress result;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
	//	memcpy(&result, &pService->GetProxyAddress(), sizeof(mcTransportAddress));
		result = pService->GetProxyAddress();
	else
	{
		PASSERTSTREAM(TRUE, "Service for id " << serviceID << " is not found.");
		memset(&result, 0xFF, sizeof(result));
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CServicesList::GetProxyIpVersion(int serviceID)
{
	DWORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetProxyIpVersion();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CServicesList::GetAlternateProxyIpV4(int serviceID)
{
	DWORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetAltProxyIpV4();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
char* CServicesList::GetAlternateProxyIpV6(int serviceID)
{
	char* result = NULL;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetAltProxyIpV6();
		//memcpy( result, pService->GetAltProxyIpV6(), IPV6_ADDRESS_BYTES_LEN );
	return result;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CServicesList::GetAlternateProxyV6scopeid(int serviceID)
{
	DWORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetAltProxyV6scopeId();
	return result;
}
/////////////////////////////////////////////////////////////////////////////
mcTransportAddress	CServicesList::GetAlternateProxyAddress(int serviceID)
{
	mcTransportAddress result;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		//memcpy(&result, &pService->GetAltProxyAddress(), sizeof(mcTransportAddress));
		result = pService->GetAltProxyAddress();
	else
	{
		PASSERTSTREAM(TRUE, "Service for id " << serviceID << " is not found.");
		memset(&result, 0xFF, sizeof(result));
	}
	
	return result;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CServicesList::GetAlternateProxyIpVersion(int serviceID)
{
	DWORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetAltProxyIpVersion();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
WORD CServicesList::GetAlternateProxyPort(int serviceID)
{
	WORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetAltProxyPort();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
char* CServicesList::GetProxyName(int serviceID)
{
	char* result = NULL;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetProxyName();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
char* CServicesList::GetOutboundProxyName(int serviceID)
{
	char* result = NULL;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetOutboundProxyName();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
char* CServicesList::GetAltProxyName(int serviceID)
{
	char* result = NULL;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetAltProxyName();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
char* CServicesList::GetRegHostName(int serviceID)
{
	char* result = NULL;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetProxyHostName();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
char* CServicesList::GetAltRegHostName(int serviceID)
{
	char* result = NULL;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetAltProxyHostName();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
enTransportType CServicesList::GetTransportType(int serviceID)
{
	enTransportType result = eTransportTypeUdp;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetTransportType();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
WORD CServicesList::GetRefreshTout(int serviceID)
{
	WORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetRefreshTout();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CServicesList::GetServersConfig(int serviceID)
{
	DWORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetServersConfig();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
WORD CServicesList::GetDNSStatus(int serviceID)
{
	WORD result = 0;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetDNSStatus();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CServicesList::IsDHCP(int serviceID)
{
	BYTE result = FALSE;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->IsDHCP();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CServicesList::IsRegOnGoing(int serviceID)
{
	BYTE result = FALSE;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->IsRegOnGoing();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CServicesList::IsRegMRs(int serviceID)
{
	BYTE result = FALSE;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->IsRegMeetingRooms();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CServicesList::IsRegEQs(int serviceID)
{
	BYTE result = FALSE;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->IsRegEQs();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CServicesList::IsRegFactories(int serviceID)
{
	BYTE result = FALSE;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->IsRegFactories();
	return result;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CServicesList::IsRegGWProfiles(int serviceID)
{
	BYTE result = FALSE;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->IsRegGWProfiles();
	return result;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CServicesList::GetRegistrarStatus(int serviceID)
{
	DWORD result = FALSE;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetOnGoingRegistrarStatus();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
void CServicesList::SetRegistrarStatus(int serviceID, eServerType role, DWORD status, mcTransportAddress proxyIp)
{
	PTRACE(eLevelInfoNormal, "CServicesList::SetRegistrarStatus");

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
	{
		eSipServerStatusType serverStatus;
		if(STATUS_OK == status)
			serverStatus = eSipServerStatusTypeOk;
		else
			serverStatus = eSipServerStatusTypeFail;

		if(eServerTypePrimary == role)
		{
			pService->SetOnGoingRegistrarStatus(serverStatus);
			pService->SetOnGoingRegistrarIp(proxyIp);
		}
		else if(eServerTypeAlternate == role)
		{
			pService->SetOnGoingAltRegistrarStatus(serverStatus);
			pService->SetOnGoingAltRegistrarIp(proxyIp);
		}

		SIP_PROXY_STATUS_PARAMS_S data;
        memset(&data, 0, sizeof(SIP_PROXY_STATUS_PARAMS_S));
		data.ServiceId = serviceID;

		data.ProxyList[0].Role = eServerTypePrimary;
		strncpy(data.ProxyList[0].Name, pService->GetProxyName(), H243_NAME_LEN - 1);
		data.ProxyList[0].Name[H243_NAME_LEN - 1] = 0;

		if(eIpVersion4 == proxyIp.ipVersion)
		{
			data.ProxyList[0].IpV4.v4.ip = pService->GetOnGoingRegistrarV4Ip();
		}
		else
		{
			data.ProxyList[0].IpV4.v4.ip = 0;
		}

		memcpy(data.ProxyList[0].IpV6.v6.ip, pService->GetOnGoingRegistrarV6Ip(), IPV6_ADDRESS_BYTES_LEN);
		data.ProxyList[0].Status = pService->GetOnGoingRegistrarStatus();

		data.ProxyList[1].Role = eServerTypeAlternate;
		const int maxLenForKlocWork = sizeof(data.ProxyList[1].Name) - 1;
		strncpy(data.ProxyList[1].Name, pService->GetAltProxyName(), maxLenForKlocWork);
		data.ProxyList[1].Name[maxLenForKlocWork] = 0;
		if(eIpVersion4 == proxyIp.ipVersion)
		{
			data.ProxyList[1].IpV4.v4.ip = pService->GetOnGoingRegistrarV4Ip();
		}
		else
		{
			data.ProxyList[1].IpV4.v4.ip = 0;
		}
		memcpy(data.ProxyList[1].IpV6.v6.ip, pService->GetOnGoingAltRegistrarV6Ip(), IPV6_ADDRESS_BYTES_LEN);
		data.ProxyList[1].Status = pService->GetOnGoingAltRegistrarStatus();

		CSegment*  pReqParam = new CSegment;
		pReqParam->Put((BYTE*)&data, sizeof(SIP_PROXY_STATUS_PARAMS_S));

		const COsQueue* pCsMbx =
				CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

		STATUS res = pCsMbx->Send(pReqParam, CS_PROXY_REGISTRAR_STATUS_UPDATE_IND);
	}
}

/////////////////////////////////////////////////////////////////////////////
char* CServicesList::GetServiceName(int serviceID)
{
	char* result = NULL;

	CSipProxyIpParams* pService = find(serviceID);
	if(pService)
		result = pService->GetServiceName();
	return result;
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::UpdateDNSStatusAtAllEntries()
{
	for (int i = 0; i < MAX_CONF_REGISTRATIONS; i++)
	{
		if (m_pConfRegDB[i])
			m_pConfRegDB[i]->m_DNSStatus = m_DNSStatus;
	}
}





void CSipProxyServiceManager::CreateTaskName()
{
	char buff[256];
	snprintf(buff, sizeof(buff), "CSipProxySrvcMngr (ServiceId %d)", m_ServiceId);
	m_TaskName = buff;
}

void CSipProxyServiceManager::SetServiceId(DWORD id)
{
	m_ServiceId = id;
	CreateTaskName();
}

//////////////////////////////////////////////////////////////////////
DWORD CSipProxyServiceManager::GetServiceId()
{
	return m_ServiceId;
}
/////////////////////////////////////////////////////////////////////////
void CSipProxyServiceManager::SetMngmntDNSStatus(BYTE dNSConfigWasReceived,WORD dNSStatus)
{
	m_DNSConfigWasReceived = dNSConfigWasReceived;
	if(eDnsConfigurationSuccess == dNSStatus)
			m_DNSStatus = TRUE;
		else
			m_DNSStatus = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CSipProxyServiceManager::CreateICEUserInDB(WORD serviceId, BOOL IsProxyEnable)
{
	//CSipProxyIpParams* pService = NULL;
	//pService = m_servicesList->find(serviceId);

	PTRACE2INT(eLevelInfoNormal, "CSipProxyManager::CreateICEUserInDB IsProxyEnable:",IsProxyEnable);


	if(!m_pProxyService)
	//if(!pService)
		return;

	BOOL bIsEnableGenericICE = 0;
	std::string key = "ENABLE_STANDART_ICE";
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key, bIsEnableGenericICE);

	PTRACE2INT(eLevelInfoNormal, "CSipProxyManager::CreateICEUserInDB #1 bIsEnableGenericICE: ", bIsEnableGenericICE);

	eIceEnvironmentType Type = m_pProxyService->GetICEType();
	PTRACE2INT(eLevelInfoNormal, "CSipProxyManager::CreateICEUserInDB Type ", Type);
	PTRACE2INT(eLevelInfoNormal, "CSipProxyManager::CreateICEUserInDB #1 m_pProxyService->GetIsStandartICE(): ", m_pProxyService->GetIsStandartICE());
	PTRACE2INT(eLevelInfoNormal, "CSipProxyManager::CreateICEUserInDB #1 m_ServiceIceType: ", m_ServiceIceType);

//	if(pService->GetIsMsICE()||(pService->GetIsStandartICE()&& bIsEnableGenericICE ))
	if(m_pProxyService->GetIsMsICE()||m_pProxyService->GetIsStandartICE())
	{
		PTRACE(eLevelInfoNormal, "CSipProxyManager::CreateICEUserInDB - Enable ICE");
		CreateNewICEUser(IsProxyEnable);//serviceId);
	}
	else
		PTRACE(eLevelInfoNormal, "CSipProxyManager::CreateICEUserInDB - Disable ICE");
}
/////////////////////////////////////////////////////////////////////////////
void CSipProxyServiceManager::CreateNewICEUser(BOOL IsProxyEnable)//WORD serviceId)
{
//	CSipProxyIpParams* pService = NULL;
//	pService = m_servicesList->find(serviceId);
	PTRACE2INT(eLevelInfoNormal, "CSipProxyManager::CreateNewICEUser IsProxyEnable:",IsProxyEnable);



	char* UserName = m_pProxyService->GetUserName();
//	char* UserName = pService->GetUserName();
	int indInDB = -1, errorFound = 0, indInConfs = -1;

	if(m_pProxyService->GetIsMsICE())
	//if(pService->GetIsMsICE())
	{
		for (int i = 0; i < MAX_CONF_REGISTRATIONS; i++)
		{
			//if(m_pConfRegDB[i] && serviceId == m_pConfRegDB[i]->m_serviceId && FALSE == m_pConfRegDB[i]->m_bIsFromOldService)
			if(m_pConfRegDB[i] && m_pICEserviceId == m_pConfRegDB[i]->m_serviceId && FALSE == m_pConfRegDB[i]->m_bIsFromOldService)
				break;

		}
	}
	else //We should create dummy conf.
		if(IsProxyEnable)
		{
			PTRACE(eLevelInfoNormal, "CSipProxyManager::CreateNewICEUser, Create Dummy Conf");
			CreateDummyConf(m_pICEserviceId);
		}

	int confIdInDb = -1;
	confIdInDb = FindConfInDB( 0, UserName, m_pICEserviceId );
	// confIdInDb = FindConfInDB( 0, UserName, serviceId );
	if (-1 != confIdInDb)
	{
		errorFound = ADD_CONF_ALREADY_EXISTS;
		PTRACE(eLevelInfoNormal, "CSipProxyManager::CreateNewICEUser, USER_ALREADY_EXISTS");

		return;
	}

	//if(pService)
	if(m_pProxyService)
	{	//Add to DB only the MS conf not the standart conf
		if(m_ServiceIceType == eIceEnvironment_ms)
		{
			indInDB = InsertConfToDB( 0, UserName, m_pICEserviceId, 0, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,eSipServer_ms, m_Dummy_name, TRUE,FALSE);
		}
		else if(m_ServiceIceType == eIceEnvironment_Standard)
			indInDB = InsertIceConfToDB( 0, UserName, m_pICEserviceId, 0, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,eSipServer_generic, m_Dummy_name,FALSE,TRUE);

			PTRACE2INT(eLevelInfoNormal, "CSipProxyManager::CreateNewICEUser - indInDB: ",indInDB);

			if (-1 == indInDB)
			{
				errorFound = ADD_CONF_DB_IS_FULL;
				AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,SIP_REGISTRATION_LIMIT_REACHED,
						SYSTEM_MESSAGE,"SIP registrations limit reached",true,true);
			}
			else
				RemoveActiveAlarmByErrorCode(SIP_REGISTRATION_LIMIT_REACHED);

		// (create a new conf state-machine (insert to list))
		indInConfs = -1;
		if (!errorFound)
		{
			indInConfs = m_confList->CreateConfCntrl( m_pConfRegDB[indInDB], m_pRcvMbx, m_pMockMplProtocol,m_pProxyService);
		//	indInConfs = m_confList->CreateConfCntrl( m_pConfRegDB[indInDB], m_pRcvMbx, m_pMockMplProtocol,pService);
			if (-1 == indInConfs)
			{
				// RemoveFromConfDB( indInConfs );
				errorFound = ADD_CONF_CNTL_CREATE_ERROR;
			}
		}

		// (start registration in the Proxy Registrar Server)

		if ((!errorFound)&& (m_pProxyService->GetIsMsICE()))// Register only MS ICE user.
			errorFound = StartRegister(indInConfs);

		if(errorFound)
		{
			ALLOCBUFFER(str, 50);
			snprintf(str, sizeof(str), "%d, error = %d", m_pICEserviceId, errorFound);
			PTRACE2(eLevelInfoNormal, "CSipProxyManager::CreateNewICEUser, failed to reg on service=", str);
			DEALLOCBUFFER(str);

		}
	}

}
///////////////////////////////////////////////////////////////////////////
//pRelayHostName: in format "ipv4;" or "ipv6;" or "ipv4;ipv6;"
// return value editedHostName:  in format "ipv4" or "ipv6" or "ipv4;ipv6;"
void CSipProxyServiceManager::GetRelayHostNameAcordingToIPTypeConfiguration(const char* pRelayHostName, char* editedHostName, size_t editedHostNameLen)
{
	PASSERT_AND_RETURN(pRelayHostName == NULL || editedHostName == NULL);
	//TRACEINTO << "pRelayHostName:" << pRelayHostName;
	//eIpType configurationIpType = eIpType_IpV4;
	eIpType ipType = m_servicesList->GetIpType(m_ServiceId); // ???
	//eIpType ipType = eIpType_IpV6;
	BOOL throwIPv4 = (ipType == eIpType_IpV6) ? TRUE : FALSE;
	BOOL throwIPv6 = (ipType == eIpType_IpV4) ? TRUE : FALSE;
	TRACEINTO << "pRelayHostName:" << pRelayHostName << ", ipType:" << ipType << ", throwIPv4:" << (DWORD)throwIPv4 << ", throwIPv6:" << (DWORD)throwIPv6;
	string delim = ";";
	string addr;
	string fullHostName(pRelayHostName);
	size_t nextDelimIndex = fullHostName.find(delim);
	size_t nextAddrIndex = 0;

	while (nextDelimIndex != string::npos)
	{
		addr = fullHostName.substr(nextAddrIndex,nextDelimIndex - nextAddrIndex);
		TRACEINTO << "addr:" << addr;
		nextAddrIndex = nextDelimIndex + 1;
		nextDelimIndex = fullHostName.find(delim,nextAddrIndex);

		// todo: if(isvalidipv4())
		if((string::npos != addr.find(".")) && !throwIPv4)
		{
			// ipv4
			strncpy(editedHostName, addr.c_str(), editedHostNameLen - 1 );
			editedHostName[editedHostNameLen-1] = '\0';
			strncat(editedHostName, ";", 1);
			editedHostName[editedHostNameLen-1] = '\0';
			TRACEINTO << "editedHostName 1:" << editedHostName;
		} else if((string::npos != addr.find(":")) && !throwIPv6)// todo: if(isvalidipv6())
		{
			// ipv6
			size_t len = strlen(editedHostName);
			strncat(editedHostName, addr.c_str(), editedHostNameLen - len - 2 );
			strncat(editedHostName, ";", 1);
			editedHostName[editedHostNameLen-1] = '\0';
			TRACEINTO << "editedHostName 2:" << editedHostName;
		}

	}
	TRACEINTO << "editedHostName:" << editedHostName;
}
/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::HandleServiceResponse(CSegment* pParam)
{
	DWORD Id = 0xffffffff, status = STATUS_ILLEGAL;


	CMplMcmsProtocol  mplMcmsProtocol;
	mplMcmsProtocol.DeSerialize(*pParam, CS_API_TYPE);
    int	data_len= mplMcmsProtocol.getDataLen();

	mcIndServiceResp * pServiceResponseMsg = (mcIndServiceResp *)new char[data_len];
	PASSERT_AND_RETURN(NULL == pServiceResponseMsg);
	memcpy(pServiceResponseMsg, mplMcmsProtocol.GetData(), data_len);


	status = pServiceResponseMsg->status;
	Id = pServiceResponseMsg->id;

	OPCODE ServiceStatus;
	if(STATUS_OK == status)
	{
		PTRACE(eLevelInfoNormal,"CSipProxyManager::HandleServiceResponse : status = STATUS_OK.");
		ServiceStatus = SERVICE_IND_OK;
	}
	else
	{
		char s[16];
		sprintf(s,"%i", status);
		PTRACE2(eLevelInfoNormal,"CSipProxyManager::HandleServiceResponse : Subscribing failed, status = ",s);
		ServiceStatus = SERVICE_IND_FAILED;
	}

	// build new headers
	size_t originalRelayHostNameLen = 0;
	size_t editedRelayHostNameLen = 0;
	char editedHostName[128*2]; // ??? len
	memset(editedHostName, '\0', sizeof(editedHostName) );

	//  old headers list object
	CSipHeaderList * pHeaders = new CSipHeaderList(pServiceResponseMsg->sipHeaders);

	// new headers list object
	CSipHeaderList headersNew(pServiceResponseMsg->sipHeaders.numOfHeaders * 2 , 0);

	if(pHeaders)
	{
		for(int i=0; i<pHeaders->GetNumOfElem(); i++) // for each header in old headers list
		{
			// take header data
			const CSipHeader* pCurHeader = (*pHeaders)[i];
			if(pCurHeader)
			{
				enHeaderField header = pCurHeader->GetHeaderField();
				unsigned int headerLen = (NULL != pCurHeader) ? pCurHeader->GetHeaderStrLen() : 0;
				char headerStr[headerLen + 1];
				memset(headerStr, '\0', headerLen + 1);
				strncpy(headerStr, pCurHeader->GetHeaderStr(), headerLen);



				TRACEINTO << " adding header:" << header << ", headerStr:" << headerStr;
				headersNew.AddHeader(header, headerLen, headerStr);

			} else
			{
				TRACEINTO << " (*pHeaders)[" << i << "]:" << " is NULL" ;
			}
		}
	}

	TRACEINTO << " dbg editedHostName:" << editedHostName;

	int	edited_data_len = data_len ;
	TRACEINTO << "dbg edited_data_len: " << edited_data_len << ", editedRelayHostNameLen:" << editedRelayHostNameLen << ", originalRelayHostNameLen:" <<originalRelayHostNameLen;
	mcIndServiceResp * pServiceResponseMsgEdited = (mcIndServiceResp *)new char[edited_data_len];
	pServiceResponseMsgEdited->status = pServiceResponseMsg->status;
	pServiceResponseMsgEdited->id = pServiceResponseMsg->id;
	pServiceResponseMsgEdited->subOpcode = pServiceResponseMsg->subOpcode;

	headersNew.BuildMessage((sipMessageHeaders*)(&pServiceResponseMsgEdited->sipHeaders));

	CSipHeaderList * pTemp = new CSipHeaderList(pServiceResponseMsgEdited->sipHeaders);
	COstrStream msg2;
	pTemp->Dump(msg2);
	msg2 << "Id=" << pServiceResponseMsgEdited->id << "\n";
	msg2 << "status=" << pServiceResponseMsgEdited->status << "\n";
	msg2 << "sub opcode=" << pServiceResponseMsgEdited->subOpcode << "\n";

	PTRACE2(eLevelInfoNormal, "dbg CSipProxyManager::HandleServiceResponse Ind:\n ", msg2.str().c_str());

	edited_data_len = sizeof(mcIndServiceResp) + pServiceResponseMsgEdited->sipHeaders.headersListLength;
	CSegment* pSeg = new CSegment;
	*pSeg << Id;
	pSeg->Put((BYTE*)((mcIndServiceResp *)pServiceResponseMsgEdited),  edited_data_len) ;
	m_confList->HandleEvent(pSeg, 0, ServiceStatus);

	PDELETE(pSeg);
	PDELETE(pTemp);
	PDELETEA(pServiceResponseMsgEdited);
	PDELETE(pHeaders);
	PDELETEA(pServiceResponseMsg);
}


void CSipProxyServiceManager::OnFakeIceEndInit(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::OnFakeIceEndInit ");
	WORD NumOfCards = 0;
	BYTE atLeastOneBoardSucceed=FALSE;
	DWORD ice_init_status = STATUS_OK;

	CProcessBase *pProcess = CProcessBase::GetProcess();

	*pParam >>NumOfCards;
	PTRACE2INT(eLevelInfoNormal,"CSipProxyServiceManager::OnFakeIceEndInit - NumOfCards: ",NumOfCards);

	ICE_INIT_IND_S* ice_init_array[NumOfCards];

	for(int i=0;i<NumOfCards;i++)
	{
		ice_init_array[i] = new ICE_INIT_IND_S;
		if(ice_init_array[i])
		{
			pParam->Get( (BYTE*)ice_init_array[i], sizeof(ICE_INIT_IND_S) );

			PTRACE2(eLevelInfoNormal,"CSipProxyServiceManager::OnFakeIceEndInit Status:  ", ((ice_init_array[i]->status) == STATUS_OK) ? "SUCCESS":"FAIL");

			if(ice_init_array[i]->status == STATUS_OK)
			{
				atLeastOneBoardSucceed = TRUE;
				PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::OnFakeIceEndInit - atLeastOneBoardSucceed ");
			}
			else
			{
				PTRACE(eLevelError,"CSipProxyServiceManager::OnFakeIceEndInit response Ack with status fail- status: ");
			}

			delete ice_init_array[i];
			ice_init_array[i] = NULL;
		}
	}

	if(atLeastOneBoardSucceed )
	{
		ice_init_status = STATUS_OK;
		PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::OnFakeIceEndInit - ICE initiation OK!! ");
		CSegment *pSeg = new CSegment;
		*pSeg << (BYTE)ice_init_status;
		*pSeg << (BYTE)NO;
		*pSeg << (DWORD)0;//m_UcMaxVideoRateAllowed
		*pSeg << (DWORD)eIceEnvironment_ms;

		CTaskApi api(eProcessConfParty, eManager);
		api.SendMsg(pSeg, SIP_PROXY_TO_CONF_END_INIT_ICE);
	}
	else
	{
		ice_init_status = STATUS_FAIL;
		PTRACE(eLevelError,"CSipProxyServiceManager::OnFakeIceEndInit - ICE initiation Failed!!");
	}
}
/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnCSEndIceInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::OnCardsMngrIceInitResponse.");

	DWORD id = 0;
	*pParam >> id;

	WORD localId = (WORD)(id >> 16);
	//pParam->ResetRead();


	if(m_ServiceIceType != eIceEnvironment_None)
		m_confList->HandleEventByIndex(END_ICE_INIT, localId, pParam);
	else
	{
		OnFakeIceEndInit(pParam);
	}
}




/////////////////////////////////////////////////////////////////////////////
/*int  CConfList::ResolveHostName(DWORD indInConfs,char* HostName)
{
	if (indInConfs >= MAX_CONF_REGISTRATIONS)
				return -1;
	if (!m_pConfList[indInConfs])
				return -1;
	m_pConfList[indInConfs]->ResolveHostName(HostName);
}
*/
////////////////////////////////////////////////////////////////////////////
void CSipProxyServiceManager::NullActionFunction(CSegment* pParam)
{
	CSmallString str;
	str << "Opcode " << m_log[m_current_log_entry].eventStr << " arrived at state " << m_state << "\n";
	PTRACE2(eLevelInfoNormal, "CSipProxyManager::NullActionFunction: ---- ! ---- ",str.GetString());
}

/////////////////////////////////////////////////////////////////////////////
void CSipProxyServiceManager::SetConfRegistrationStatus(WORD serviceID, DWORD confId, WORD confType, DWORD status,DWORD	expire)
{
	PTRACE(eLevelInfoNormal, "CSipProxyServiceManager::SetConfRegistrationStatus");
	CSipProxyIpParams* pService = m_servicesList->find(serviceID);
	if(pService)
	{
		if(expire == 0)
		{
			PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::SetConfRegistrationStatus. Expire = 0. Status will not send");
			return;
		}
		eSipRegistrationStatusType registartionStatus;
		if(STATUS_OK == status)
			registartionStatus = eSipRegistrationStatusTypeRegistered;
		else
			registartionStatus = eSipRegistrationStatusTypeFailed;

		if(confId == DUMMY_ENTRY && confType == eSipRegistrationConfTypeIncorrect)
			return;
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::SetConfRegistrationStatus:serviceID =",serviceID);
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::SetConfRegistrationStatus:confId = ",confId);
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::SetConfRegistrationStatus: confType = ",confType);
		PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::SetConfRegistrationStatus:registartionStatus = ",registartionStatus);

		CSegment* pSeg = new CSegment;
		*pSeg << (WORD)serviceID
			  << (DWORD)confId
			  << (WORD)confType
			  << (DWORD)registartionStatus;

		CTaskApi api(eProcessConfParty, eManager);
		api.SendMsg(pSeg, CS_PROXY_CONF_STATUS_UPDATE_IND);
	}
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSipProxyServiceManager::GetMultipleServices()
{
    BOOL isMultipleServicesCfgFlag = NO;

    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_MULTIPLE_SERVICES, isMultipleServicesCfgFlag);
	if (YES == m_bSystemMultipleServices  && isMultipleServicesCfgFlag)
	{
		return YES;
	}

	return NO;
}

/////////////////////////////////////////////////////////////////////////////
void CSipProxyServiceManager::OnMultipleServicesInd(CSegment* pParam)
{
	*pParam >> m_bSystemMultipleServices;
	TRACESTR(eLevelInfoNormal) << "\nCSipProxyServiceManager::OnMultipleServicesInd"
						   << "\nMultiple services: : " << (YES ==  m_bSystemMultipleServices? "YES" : "NO");
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyServiceManager::OnChangePresenceState(CSegment* pParam)
{
	DWORD confID;
	char confName[H243_NAME_LEN];
	char id[12];
	BYTE  presenceState = 0xFF;
	int errorFound = 0, indInDB = 0, i = 0;

	*pParam >> confID;		// check conf ID validity
	*pParam >> confName;
	*pParam >> presenceState;

	PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::OnChangePresenceState:confID =",confID);
	PTRACE2INT(eLevelInfoNormal, "CSipProxyServiceManager::OnChangePresenceState:presenceState =",presenceState);
	PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnChangePresenceState, confName=", confName);


	if (confName[0] == 0){ // || userDomainName[0] == 0) {	// check name validity
		errorFound = CONF_NAME_EMPTY;
	}

	// find IP card to unregister via it (use its IP address)
	int serviceID = -1;
	if (!errorFound)
	{
		CSipProxyIpParams* pService = m_servicesList->GetFirstService();
		if(pService)
			serviceID = pService->GetServiceId();

		if (-1 == serviceID)
		{
			errorFound = IP_CARD_NOT_FOUND;
			// (insert the conference into the confs DB)
			//even if no card exist, we need a copy in the DB

		}

		while(-1 != serviceID)
		{
			sprintf(id, "%d", serviceID);
			PTRACE2(eLevelInfoNormal, "CSipProxyServiceManager::OnChangePresenceState, next service=", id);
			// (check if this conference already exists in the DB)
			indInDB = 0;
			indInDB = FindConfInDB( confID, confName, serviceID );
			if (-1 == indInDB) {
				errorFound = PRESENTED_CONF_DOES_NOT_EXIST;
				// do something...
			}
			if (!errorFound && presenceState!= 0xff)
			{
				m_confList->ChangePresence(indInDB,presenceState);
			}
			errorFound = 0;

			pService = m_servicesList->GetNextService(serviceID);
            DBGPASSERT(pService && (int)(pService->GetServiceId()) == serviceID);

			if(pService && (int)(pService->GetServiceId()) != serviceID)
				serviceID = pService->GetServiceId();
			else
				serviceID = -1;
		}
	}

}

void CSipProxyServiceManager::OnDialogRecoveryMessageInd(CSegment *pSeg)
{
  DWORD confID, connId, partyId;
  char confName[H243_NAME_LEN];
  int errorFound = 0, indInDB = 0, i = 0;
  DWORD serviceId = 0;

  *pSeg >> confID;    // check conf ID validity
  *pSeg >> confName;
  *pSeg >> partyId;
  *pSeg >> connId;

  CSipProxyIpParams* pService = m_servicesList->GetFirstService();
  if(pService)
    serviceId = pService->GetServiceId();

  TRACEINTO << "ConfName:" << confName << ", ConfId:" << confID << ", ServiceId:" << serviceId;

  indInDB = FindConfInDB( confID, confName, serviceId );
  errorFound = m_confList->DialogRecoveryMessageInd(indInDB, partyId, connId);
  if(errorFound)
  {
    CSegment *pSeg = new CSegment;
    *pSeg << (DWORD)0;

    SendReqToConfParty(DIALOG_RECOVERY_REQ, connId, partyId, pSeg);
  }
}

void CSipProxyServiceManager::OnMsKeepAliveToutErrInd(CSegment *pSeg)
{
   DWORD Id,i;
   DWORD dbId = 0xffffffff;

   PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::OnMsKeepAliveToutErrInd");

   CMplMcmsProtocol  mplMcmsProtocol;
   mplMcmsProtocol.DeSerialize(*pSeg, CS_API_TYPE);
   int data_len = mplMcmsProtocol.getDataLen();

   mcIndProxyCrlfError * pIndMsg = (mcIndProxyCrlfError *)new char[data_len];
   memcpy(pIndMsg, mplMcmsProtocol.GetData(), data_len);

   Id = pIndMsg->nConfId;
   WORD localId = (WORD)(Id >> 16);
   for (int i = 0; i < MAX_CONF_REGISTRATIONS; i++)
   {
	   if(m_pConfRegDB[i])
		   m_confList->MsKeepAliveToutErrInd(i, pIndMsg->eCrlfSendingErrorCode);
   }
   PDELETEA(pIndMsg);

}

void CSipProxyServiceManager::BuildDummyName(char *IpStr)
{
  strcpy(m_Dummy_name, "RMX_");
  if(NULL != IpStr)
  {
      strncat(m_Dummy_name, IpStr, (H243_NAME_LEN-sizeof("RMX_")-1));
      {//--- BRIDGE-5174  -- Start
          //See RFC-2396
          //reserved    = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | ","
          //All reserved symbols must be changed to "_"
          unsigned int dwLen = strlen(m_Dummy_name);
 
          for(unsigned int nInd = 0; nInd < dwLen; nInd++)
          {
              if(   (';' == m_Dummy_name[nInd])
                  ||('/' == m_Dummy_name[nInd])   
                  ||('?' == m_Dummy_name[nInd])   
                  ||(':' == m_Dummy_name[nInd])   
                  ||('@' == m_Dummy_name[nInd])   
                  ||('&' == m_Dummy_name[nInd])   
                  ||('=' == m_Dummy_name[nInd])   
                  ||('+' == m_Dummy_name[nInd])   
                  ||('$' == m_Dummy_name[nInd])   
                  ||(',' == m_Dummy_name[nInd])   
                 )
              {
                m_Dummy_name[nInd] = '_';
              }
          }
      }//--- BRIDGE-5174  -- End
  }
}


