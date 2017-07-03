// SignalingTask.cpp

#include "SignalingTask.h"

#include "TraceStream.h"
#include "CommStartupService.h"
#include "CommServiceService.h"
#include "CommGKService.h"
#include "CommSnmpService.h"
#include "CommProxyService.h"
#include "CommConfService.h"
#include "CommTCPDumpService.h"
#include "MplMcmsProtocolTracer.h"
#include "CSMngrMplMcmsProtocolTracer.h"
#include "IpCsOpcodes.h"
#include "OpcodesMcmsInternal.h"
#include "Segment.h"
#include "CSMngrStatuses.h"
#include "ConfigManagerApi.h"
#include "FaultsDefines.h"
#include "McmsDaemonApi.h"
#include "WrappersCS.h"
#include "HlogApi.h"
#include "CsPinger.h"
#include "SysConfigKeys.h"
#include "MultipleServicesFunctions.h"
#include "AlarmStrTable.h"
#include "IpService.h"
#include "ConfigManagerApi.h"
#include "CSMngrDefines.h"
#include "Versions.h"

const WORD	STARTUP						= 1;
const WORD	NEW_SERVICE_CONFIGURATION 	= 2;
const WORD	READY						= 3;
//const WORD	WAIT_FOR_DNS_IND    		= 4;
const WORD  AFTER_STARTUP               = 4;

#define MCU_VERSION_ARRAY_LEN  40

const DWORD DEFAULT_KEEP_ALIVE_SEND_PERIOD = 10;

extern bool		IsJitcAndNetSeparation();
extern char*	CsCompTypeToString(compTypes compType);
extern char*	CsCompStatusToString(compStatuses compStatus);
extern char*	CsRecoveryReasonToString(recoveryReasons recoveryReason);

extern const char *GetUserMsgCode_CsStr(enum eUserMsgCode_CS theCode);
extern const char *GetUserMsgLocationStr(enum eUserMsgLocation theLocation);
extern const char *GetUserMsgOperationStr(enum eUserMsgOperation theOperation);
extern const char *GetUserMsgAutoRemovalStr(enum eUserMsgAutoRemoval theAutoRemoval);
extern const char *GetUserMsgProcessType_CsStr(enum eUserMsgProcessType_CS theType);

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CSignalingTask)
	ONEVENT(CS_CONFIG_PARAM_IND			,STARTUP ,  CSignalingTask::OnCsConfigParamInd)
	ONEVENT(CS_END_CONFIG_PARAM_IND		,STARTUP ,  CSignalingTask::OnCsEndConfigParamInd)
	ONEVENT(CS_END_CS_STARTUP_IND		,STARTUP ,  CSignalingTask::OnCsEndStartUpInd)

    ONEVENT(CS_NEW_SERVICE_INIT_IND		,NEW_SERVICE_CONFIGURATION ,  CSignalingTask::OnCsNewServiceInitInd)
	ONEVENT(CS_COMMON_PARAM_IND			,NEW_SERVICE_CONFIGURATION ,  CSignalingTask::OnCsCommonParamInd)
	ONEVENT(CS_COMMON_PARAM_IND			,READY                     ,  CSignalingTask::OnCsCommonParamReadyInd)

	ONEVENT(CS_END_SERVICE_INIT_IND		,NEW_SERVICE_CONFIGURATION ,  CSignalingTask::OnCsEndServiceInitInd)

	ONEVENT(CS_DEL_SERVICE_IND			,READY ,   CSignalingTask::OnCsDelServiceInd)
	ONEVENT(CS_RECONNECT_IND			,READY ,   CSignalingTask::OnCsReconnectInd)

	ONEVENT(CS_COMP_STATUS_IND			,ANYCASE,  CSignalingTask::OnCsCompStatusInd)
	ONEVENT(CS_USER_MSG_IND				,ANYCASE,  CSignalingTask::OnCsActiveAlarmInd)
	ONEVENT(CS_PING_IND					,ANYCASE,  CSignalingTask::OnCsPingInd)

	ONEVENT(CS_KEEP_ALIVE_IND			,ANYCASE,  CSignalingTask::OnCsKeepAliveInd)
	ONEVENT(KEEP_ALIVE_TIMER_SEND   	,ANYCASE,  CSignalingTask::OnTimerKeepAliveSendTimeout)
	ONEVENT(KEEP_ALIVE_TIMER_RECEIVE	,ANYCASE,  CSignalingTask::OnTimerKeepAliveReceiveTimeout)

	ONEVENT(CSMNGR_END_IP_CONFIG_IND						,ANYCASE, CSignalingTask::OnCsMngrEndIpConfigInd)
	ONEVENT(CSMNGR_PROXY_IP_SERVICE_PARAM_REQ				,ANYCASE, CSignalingTask::OnCsMngrProxyIpServiceParamReq)
	ONEVENT(CSMNGR_SNMP_INTERFACE_REQ						,ANYCASE, CSignalingTask::OnCsMngrSnmpUnterfaceReq)
	ONEVENT(CSMNGR_ICE_TYPE_IND								,ANYCASE, CSignalingTask::OnCsMngrIceTypeInd)
	ONEVENT(CSMNGR_SIP_SERVER_TYPE_IND							,ANYCASE, CSignalingTask::SendSipServerTypeToCS)
	ONEVENT(CSMNGR_ADD_REMOVE_AA_IND							,ANYCASE, CSignalingTask::AddRemoveAAInd)
    ONEVENT(CS_CHECK_CS_IP_CONFIG_TIMER	,ANYCASE		                ,CSignalingTask::OnCheckCsIpconfig )
    ONEVENT(CS_CHECK_CS_IP_CONFIG_END_TIMER,ANYCASE		                ,CSignalingTask::StartStartupConfiguration )
	ONEVENT(CS_SIGNAL_PORT_INACTIVITY_TIMER, ANYCASE,	  CSignalingTask::OnTimerSignalPortInactTimeout)
PEND_MESSAGE_MAP(CSignalingTask,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void signalingEntryPoint(void* appParam)
{
	CSignalingTask * pSignalingTask = new CSignalingTask;
	pSignalingTask->Create(*(CSegment*)appParam);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CSignalingTask::CSignalingTask()
{
	TRACESTR(eLevelInfoNormal) << "\nCSignalingTask - constructor";

	m_pProcess = (CCSMngrProcess*)CCSMngrProcess::GetProcess();

	m_csId = (DWORD)this;

	m_CSMngrMplMcmsProtocolTracer = new CCSMngrMplMcmsProtocolTracer;

	m_CommStartupService = new CCommStartupService;
	m_CommServiceService = new CCommServiceService;
	m_CommGKService		 = new CCommGKService;
	m_CommSnmpService    = new CCommSnmpService;
	m_CommProxyService	 = new CCommProxyService;
	m_CommConfService	 = new CCommConfService;
	
	m_CommTCPDumpService = new CCommTCPDumpService;
//	m_CommRcrsService 	 = new CCommRsrcService;

	m_CommStartupService->SetMplMcmsProtocolTracer(m_CSMngrMplMcmsProtocolTracer);
	m_CommServiceService->SetMplMcmsProtocolTracer(m_CSMngrMplMcmsProtocolTracer);

	m_flagIsSignalingStartupEndedReceived = false;
	m_flagIsCsMngrEndIpConfigIndReceived = false;

    m_numOfkeepAliveTimeoutReached     = 0;

	m_lastKeepAliveReqTime.InitDefaults();
	m_lastKeepAliveIndTime.InitDefaults();

    InitkeepAliveStruct();
    m_isKeepAliveFailureAlreadyTreated = NO;
    memset(&m_compStatusStructure, 0, sizeof(csCompStatusSt));

	memset(&m_keepAliveStructure,  0, sizeof(csKeepAliveSt));

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetDWORDDataByKey("KEEP_ALIVE_RECEIVE_PERIOD", m_keepAliveReceivePeriod);

	m_keepAliveSendPeriod = DEFAULT_KEEP_ALIVE_SEND_PERIOD;

	m_flagIsSignalPortUp = false;

	CreateTaskName();

	SetState(IDLE);
}
//////////////////////////////////////////////////////////////////////
CSignalingTask::~CSignalingTask()
{
	POBJDELETE(m_CommStartupService);
	POBJDELETE(m_CommServiceService);
	POBJDELETE(m_CSMngrMplMcmsProtocolTracer);
	POBJDELETE(m_CommGKService);
	POBJDELETE(m_CommSnmpService);
	POBJDELETE(m_CommProxyService);
	POBJDELETE(m_CommConfService);
	
	POBJDELETE(m_CommTCPDumpService);
//	POBJDELETE(m_CommRcrsService);
}
//////////////////////////////////////////////////////////////////////
void CSignalingTask::InitTask()
{
	PTRACE(eLevelInfoNormal, __PRETTY_FUNCTION__);
	StartStartupConfiguration();
	SetState(STARTUP);
}
//////////////////////////////////////////////////////////////////////
void CSignalingTask::SelfKill()
{
//  was put in commrnt due to leak problems
//	TRACESTR(eLevelInfoNormal) << "\nCSignalingTask::SelfKill";

	// ===== 1. update entry in array
	COsQueue& rcvMbx = GetRcvMbx();
	m_pProcess->TurnSignalingTaskToZombie(&rcvMbx);

	// ===== 2. call father's SelfKill
	CTaskApp::SelfKill();
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::CreateTaskName()
{
	char buff[256];
	sprintf(buff, "SignalingTask (csId %d)", m_csId);
	m_TaskName = buff;
}

/////////////////////////////////////////////////////////////////////
void CSignalingTask::AddFilterOpcodePoint()
{
	AddFilterOpcodeToQueue(CS_KEEP_ALIVE_IND);
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::SetCsId(WORD id)
{
	m_csId = id;
	CreateTaskName();

	m_CommStartupService->SetCsId(m_csId);
	m_CommServiceService->SetCsId(m_csId);
}

//////////////////////////////////////////////////////////////////////
WORD CSignalingTask::GetCsId()
{
	return m_csId;
}

//////////////////////////////////////////////////////////////////////
STATUS CSignalingTask::StartStartupConfiguration()
{
	PTRACE(eLevelInfoNormal, __PRETTY_FUNCTION__);

	if (m_pProcess->GetIsMultipleServices() == eMSNotConfigure)
	{
		StartTimer(CS_CHECK_CS_IP_CONFIG_END_TIMER,10*SECOND);
		return STATUS_OK;
	}
	else if( m_pProcess->GetIsMultipleServices() == eTRUE )
	{


		CIPServiceList *pIpServListDynamic = m_pProcess->GetIpServiceListDynamic();
		CIPService *pService = pIpServListDynamic->GetService(m_csId);
		if(pService)
		{
			DWORD serviceId = pService->GetId();
			DWORD boardIdUsedByCS = 0;
			//Added for Gesher/Ninja
			eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
			if (eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)//in gesher/ninja don't check whether IP is set.
			{
				DeleteTimer(CS_CHECK_CS_IP_CONFIG_END_TIMER);
			}
			else
			{
				boardIdUsedByCS = m_pProcess->GetCSIpConfigMasterBoardId(serviceId);
				if (boardIdUsedByCS == 0)
				{
					StartTimer(CS_CHECK_CS_IP_CONFIG_END_TIMER,20*SECOND);
					PTRACE(eLevelInfoNormal, "CSignalingTask::StartStartupConfiguration: CS ip has not been configured yet . no chosen boardid no vlan");
					return STATUS_OK;
				}
				else
					DeleteTimer(CS_CHECK_CS_IP_CONFIG_END_TIMER);
			}

		}
	}


	m_CommStartupService->SetIsConnected(true);

	m_CommStartupService->SetCsId(m_csId);
	m_CommServiceService->SetCsId(m_csId);

	m_CommStartupService->ReceiveNewInd(&m_CsNewIndStruct);

	STATUS status = m_CommStartupService->SendNewReq(m_pProcess->GetLicensingMaxNumOfParties());
	CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "to send New Request to CS Module", status);

	status = m_CommStartupService->SendConfigParam(true);
	CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "to send Config param to CS Module", status);

	//send ice type to cs
	CIPServiceList *list= m_pProcess->GetIpServiceListDynamic();
	CIPService *pService = list->GetService(m_csId);
	if (pService)
	{
		int sipServerType = eSipServer_generic;
		if( pService->GetSip()->GetConfigurationOfSIPServers() )
		{
			sipServerType = pService->GetSip()->GetSipServerType();
		    PTRACE(eLevelInfoNormal, "CSignalingTask::StartStartupConfiguration: sip server type");
		    SendSipServerTypeToCS(sipServerType);
		}

		CSipAdvanced *pSipAdvanced = pService->GetpSipAdvanced();
		if (pSipAdvanced)
		{
			int iceType = pSipAdvanced->GetIceEnvironment();

			PTRACE(eLevelInfoNormal, "CSignalingTask::StartStartupConfiguration: ice type");

			SendIceTypeToCS(iceType);
			SendIceGruuToCs(NULL);

		}
		SendSecurityPKIToCs(NULL);
	}

	PTRACE(eLevelInfoNormal, "CSignalingTask::StartStartupConfiguration: mcu version id");
	SendMcuVersionIdToCs();

	return status;
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::SetCsNewIndData(char* data)
{
	memcpy(&m_CsNewIndStruct, data, sizeof(CS_New_Ind_S));
}

//////////////////////////////////////////////////////////////////////
STATUS CSignalingTask::OnCsConfigParamInd(CSegment * pSeg)
{
	CS_Config_Ind_S *param = (CS_Config_Ind_S*)pSeg->GetPtr();
	m_CommStartupService->ReceiveConfigInd(param);

	// send sysConfig params to CS
	STATUS status = m_CommStartupService->SendConfigParam(false);
	if(STATUS_END_CONFIG != status)
	{
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "to send Config param to CS Module", status);
	}
	else if(STATUS_END_CONFIG == status)
	{
		// finished to send all the sysconfig parameters
		status = m_CommStartupService->SendEndConfigParamReq();
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "to send End Config params to CS Module", status);
	}
	else
	{
		string buff = "Bad Flow. status ";
		buff += m_pProcess->GetStatusAsString(status);
		PASSERTMSG(1, buff.c_str());
	}

	return status;
}

//////////////////////////////////////////////////////////////////////
STATUS CSignalingTask::OnCsEndConfigParamInd(CSegment *pSeg)
{
	CS_End_Config_Ind_S *param = (CS_End_Config_Ind_S*)pSeg->GetPtr();
	m_CommStartupService->ReceiveEndConfigInd(param);

	// cs got all the sysconfig params.sending Lan configuration
	STATUS status = m_CommStartupService->SendLanCfgReq();
	CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "to send Lan Config Request to CS Module", status);

	return status;
}

STATUS CSignalingTask::OnCsEndStartUpInd(CSegment* pSeg)
{
  CS_End_StartUp_Ind_S* param = (CS_End_StartUp_Ind_S*)pSeg->GetPtr();
  m_CommStartupService->ReceiveEndStartupInd(param);

  SetState(AFTER_STARTUP);
  SetSignalingStartupEndedReceived(true);
  TryStartSignalingServiceConfiguration();

  // Verifies only on IPv4 and both IP types, VNGR-25753
  if (IsTarget() && eIpType_IpV6 != m_pProcess->GetSysIpType())
    StartTimer(CS_CHECK_CS_IP_CONFIG_TIMER, 10* SECOND);

  return STATUS_OK;
}

void CSignalingTask::OnCheckCsIpconfig()
{
  CIPServiceList* list = m_pProcess->GetIpServiceListDynamic();
  TRACECOND_AND_RETURN(NULL == list, "Service list is empty");

  CIPService* pService = list->GetService(m_csId);
  TRACECOND_AND_RETURN(NULL == pService, "There is no service for " << m_csId);

  CIPSpan* pFirstSpan = pService->GetFirstSpan();
  TRACECOND_AND_RETURN(NULL == pFirstSpan, "There is no span for " << m_csId);

  if(!m_pProcess->GetIpTypeReceivedStatus())
  {
	  TRACEINTOFUNC << "Ip type has not yet been receive check in 2 seconds";
	  StartTimer(CS_CHECK_CS_IP_CONFIG_TIMER, 2*SECOND);
	  return;
  }
  if(eIpType_IpV6 == m_pProcess->GetSysIpType())
  {
	  TRACEINTOFUNC << "Ip type is Ipv6 only no checking is done. (exiting timer) ";
	  return ;
  }
  DWORD firstSpanIP = 0;
	if (m_pProcess->GetIsMultipleServices() == eMSNotConfigure)
	{
		StartTimer(CS_CHECK_CS_IP_CONFIG_TIMER, SECOND);
		return ;
	}
	else if (m_pProcess->GetIsMultipleServices() == eTRUE)
  {

		        DWORD boardIdUsedByCS =0 , subBoardIdUsedByCS =0;
		       // m_pProcess->GetSignalingMasterBoardId(m_csId, boardIdUsedByCS, subBoardIdUsedByCS);


				CIPServiceList *pIpServListDynamic = m_pProcess->GetIpServiceListDynamic();
				CIPService *pService = pIpServListDynamic->GetService(m_csId);
				if(pService)
				{
					DWORD serviceId = pService->GetId();

					boardIdUsedByCS = m_pProcess->GetCSIpConfigMasterBoardId(serviceId);
					subBoardIdUsedByCS = m_pProcess->GetCSIpConfigMasterPqId(serviceId);

					if(boardIdUsedByCS)
					{
						eConfigInterfaceType ifType = GetSignalingNetworkType(boardIdUsedByCS, subBoardIdUsedByCS);
						DWORD vlanId = CalcMSvlanId(boardIdUsedByCS, subBoardIdUsedByCS);
						firstSpanIP = GetVlanCSInternalIpv4Address(vlanId);
					}
				}
  }
  else
  {
    firstSpanIP = pFirstSpan->GetIPv4Address();
  }

  // Checks IP validaty
  if (firstSpanIP != 0 && firstSpanIP != 0xFFFFFFFF)
  {
    char ip_str[IP_ADDRESS_LEN];
    SystemDWORDToIpString(firstSpanIP, ip_str);

    CConfigManagerApi configuratorApi;
    DWORD retIP = 0;
    STATUS respStatus = configuratorApi.CheckCSIpConfig(firstSpanIP, retIP);
    if (retIP == 0)
    {
    	if (IsStartupFinished())
      AddCSActiveAlarmSingleton(CS_IP_NOT_CONFIGURED,
                                m_csId,
                                "Failed to configure the Signaling Host IP Address");

      TRACEWARN << "Failed to configure CS IP " << ip_str << " (" << retIP << ")";
      StartTimer(CS_CHECK_CS_IP_CONFIG_TIMER, SECOND);
    }
    else
    {
      DeleteTimer(CS_CHECK_CS_IP_CONFIG_TIMER);
      RemoveCSActiveAlarm(CS_IP_NOT_CONFIGURED, m_csId);
      TRACEINTOFUNC << "CS IP " << ip_str << " (" << retIP << ")" << "has been configured";
      
      eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
      if (IsTarget() && (eProductTypeRMX1500 == curProductType || eProductTypeRMX4000 == curProductType ))
      {
    	  TRACEINTOFUNC << "Starting  CHECK_ETH2_CS_TIMER";
		  CManagerApi api(eProcessCSMngr);
		  api.SendMsg(NULL, FIRE_CHECK_ETH2_CS);
      }      
    }
  }
}

STATUS CSignalingTask::OnCsNewServiceInitInd(CSegment *pSeg)
{
	STATUS status = STATUS_FAILED_TO_RECEIVE_MSG_FROM_CS;

	DWORD length = *((DWORD*)pSeg->GetPtr());
	if(0 != length)
	{
		status = m_CommServiceService->Receive(CS_NEW_SERVICE_INIT_IND, pSeg);
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "to receive NEW SERVICE IND from CS Module", status);
	}
	else
	{
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "empty message from CS Module", STATUS_FAILED_TO_RECEIVE_MSG_FROM_CS);
	}

	status = m_CommServiceService->SendCommonParam(YES);
	CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "to send Common param to CS Module", status);

	return status;
}

//////////////////////////////////////////////////////////////////////
// receive COMMON ind messages from CS Module, the state should be service configuration
STATUS CSignalingTask::OnCsCommonParamInd(CSegment *pSeg)
{
	TRACEINTO << "\n" << __FUNCTION__;

	DWORD length = *((DWORD*)pSeg->GetPtr());

	STATUS status = STATUS_FAILED_TO_RECEIVE_MSG_FROM_CS;
	if(0 == length)
	{
        CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "empty message from CS Module", STATUS_FAILED_TO_RECEIVE_MSG_FROM_CS);
        return status;
    }

    status = m_CommServiceService->Receive(CS_COMMON_PARAM_IND, pSeg);
    if(	STATUS_INFO_MESSAGE_OK == status || STATUS_INFO_MESSAGE_FAIL == status)
    {
        string message = "Received INFO param ind in wrong state : ";
        message += GetStateStr(GetState());
        PASSERTMSG(TRUE, message.c_str() );

        return status;
    }
    CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "to receive CS COMMON IND from CS Module", status);

    status = m_CommServiceService->SendCommonParam(NO);
	if(STATUS_END_COMMON == status)
	{
		SetNICIpV6AutoConfig();

		status = m_CommServiceService->SendEndServiceInitReq();
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "to send End Service Unit Request to CS Module", status);
	}

	return status;
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::SetNICIpV6AutoConfig()
{
	TRACEINTO << "\n" << __FUNCTION__;
	eConfigInterfaceType ifType = m_pProcess->GetSignalingInterfaceType(m_csId);


	 eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	 std::string ifName;

	eIpType ipType = m_pProcess->GetSysIpType();
	ifName = GetLogicalInterfaceName(ifType, ipType);

	CConfigManagerApi api;

	//1. Interface up
	ipType = eIpType_None;
	BOOL bIsAutoConfig = FALSE;
	CIPServiceList *pIpServListDynamic = m_pProcess->GetIpServiceListDynamic();
	CIPService *pService = pIpServListDynamic->GetService(m_csId);
	std::string defaultGatewayIPv4Str = "";
	std::string stdDefaultGatewayIPv6Str = "";
	char defaultGatewayIPv6Str[IPV6_ADDRESS_LEN] = "";
	if (pService)
	{
		pService->GetDefaultGatewayIPv6(defaultGatewayIPv6Str, FALSE);
		stdDefaultGatewayIPv6Str = defaultGatewayIPv6Str;
		//2. IPv6 Autoconfiguration
		ipType = pService->GetIpType();
		eV6ConfigurationType ipv6CfgType = pService->GetIpV6ConfigurationType();
		if(eIpType_IpV6 == ipType || eIpType_Both == ipType)
			if(eV6Configuration_Auto == ipv6CfgType)
				bIsAutoConfig = TRUE;
	}

	//Jitc NetSep
	if (IsJitcAndNetSeparation())
	{
  		ifType = eSeparatedSignalingNetwork;;
		const std::string strMac = "dummytobechangedbymacchanger";
		api.SetMacAddress(ifType,eIpType_IpV4,strMac);
	}

	api.InterfaceUp(ifName);

	api.SetNICIpV6AutoConfig(ifName, bIsAutoConfig,ifType,ipType,stdDefaultGatewayIPv6Str);
}

//////////////////////////////////////////////////////////////////////
// receive INFO messages from CS Module, the state should be ready
STATUS CSignalingTask::OnCsCommonParamReadyInd(CSegment *pSeg)
{
	TRACEINTO << "\n" << __FUNCTION__;
    DWORD length = *((DWORD*)pSeg->GetPtr());

	STATUS status = STATUS_FAILED_TO_RECEIVE_MSG_FROM_CS;
	if(0 == length)
	{
        CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "empty message from CS Module", STATUS_FAILED_TO_RECEIVE_MSG_FROM_CS);
        return status;

    }

	status = m_CommServiceService->Receive(CS_COMMON_PARAM_IND, pSeg);

    if(	STATUS_INFO_MESSAGE_OK == status || STATUS_INFO_MESSAGE_FAIL == status) // receives INFO messages
    {
        status = (STATUS_INFO_MESSAGE_OK == status ? STATUS_OK : STATUS_FAILED_TO_RECEIVE_MSG_FROM_CS);
        CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "to receive INFO from CS Module", status);

        if(STATUS_OK == status)
            TakeInfoMsgFromCS();

        return status;
    }
    else if(STATUS_OK == status) // IP Interface can be sent as update during READY state
    {
        CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "to receive Common Ind from CS Module", status);
    }
    else
    {
        string message = "Received INFO param ind in wrong state : ";
        message += GetStateStr(GetState());
        PASSERTMSG(TRUE, message.c_str() );
    }

	return status;
}

////////////////////////////////////////////////////////////////////////////
void CSignalingTask::TakeInfoMsgFromCS()
{
	WORD serviceId 		= m_CommServiceService->GetLastUpdateServiceId();
	CIPServiceList *list= m_pProcess->GetIpServiceListDynamic();
	CIPService *service = list->GetService(serviceId);

	STATUS status 		= STATUS_FAIL;
	eInfoType infoType 	= m_CommServiceService->GetLastInfoType();
	switch(infoType)
	{
		case eH323Info:
			status = m_CommGKService->SendIpServiceParamInd(service);
			CCSMngrProcess::TraceToLogger("CSignalingTask::OnCsCommonParamInd", "to send Updated Service to GateKeeper", status);

			status = m_CommSnmpService->SendIpServiceParamInd(service);
			CCSMngrProcess::TraceToLogger("CSignalingTask::OnCsCommonParamInd", "to send Updated Service to SNMP process", status);

		break;

		case eServiceInfo:
		{
			DBGPASSERT_AND_RETURN(!service);
			CDynIPSProperties *dynamicService 	= service->GetDynamicProperties();
			DBGPASSERT_AND_RETURN(!dynamicService);
			const CServiceInfo &ipInfo			= dynamicService->GetServiceInfo();
			eIpServiceState serviceState 		= ipInfo.GetStatus();

			if(eServiceStateFailed == serviceState)
			{
				AddCSActiveAlarmSingleton(CS_SERVICE_STATE_FAIL, serviceId,// serviceId as 'userId' (token)
										  "Central signaling failure detected in IP Network Service");
			}
			else
			{
				RemoveCSActiveAlarm(CS_SERVICE_STATE_FAIL, serviceId);
			}
		break;
		}

		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
}

//////////////////////////////////////////////////////////////////////
STATUS CSignalingTask::OnCsEndServiceInitInd(CSegment *pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCSignalingTask::OnCsEndServiceInitInd";

	STATUS status = STATUS_FAILED_TO_RECEIVE_MSG_FROM_CS;

	DWORD length = *((DWORD*)pSeg->GetPtr());

	if(0 != length)
	{
		status = m_CommServiceService->Receive(CS_END_SERVICE_INIT_IND, pSeg);
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "to receive CS END SERVICE from CS Module", status);

		if(STATUS_OK == status)
		{
			bool res = m_CommServiceService->IsErrorSectionReceived();

			//send response to CsMngr
//		    CSegment* pSeg = new CSegment;
//		    *pSeg << (BYTE)res;

		    string description = "";

		    if (res == true)
			{
				LastErrorSection_S errorSection = m_CommServiceService->GetLastErrorSection();
				description = errorSection.Reason;

				AddCSActiveAlarmSingleton(CS_NOT_CONFIGURED, m_csId, description);
			}
			else
				RemoveCSActiveAlarm(CS_NOT_CONFIGURED, m_csId);

//			*pSeg << description.c_str();

//		    CManagerApi api(eProcessCSMngr);
//		    api.SendMsg(pSeg, CS_END_SERVICE_INIT_IND);
		}
	}
	else
	{
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "empty message from CS Module", STATUS_FAILED_TO_RECEIVE_MSG_FROM_CS);
	}

	HandleNetworkSeparationConfigurations();
	if( IsTarget() )
		HandleMultipleNetworksConfigurations();

	// getting the dynamic service (will be used later)
    CIPServiceList *list = m_pProcess->GetIpServiceListDynamic();
    CIPService *pServiceDynamic = list->GetService(m_csId);

    if(NULL == pServiceDynamic)
    {
        PASSERTMSG(TRUE, "NULL == pServiceDynamic");
        return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
    }

	// ===== on IPv6_Auto mode, the addresses should be read from the interface
    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	eIpType					ipType		= pServiceDynamic->GetIpType();
	eV6ConfigurationType	configType	= pServiceDynamic->GetIpV6ConfigurationType();
	if ( ((eIpType_IpV6 == ipType) || (eIpType_Both == ipType)) &&
		 (eV6Configuration_Auto == configType || eProductTypeSoftMCU == curProductType || eProductTypeSoftMCUMfw == curProductType  || eProductTypeEdgeAxis == curProductType) )
	{
		m_pProcess->RetrieveIPv6AddressesInAutoMode(ipType);

		//Send TCPDUMP Para
		SendIpServicesTCDumpParams();
	}

	pServiceDynamic = list->GetService(m_csId);				//get the service after the ipv6 update

	if(NULL == pServiceDynamic)
    {
        PASSERTMSG(TRUE, "NULL == pServiceDynamic");
        return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
    }

	status = SendCsNewIndicationToProxyService();
	CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "to send Cs New Ind to SipProxy", status);


	SetState(READY);

	// ===== KeepAlive polling starts
	OnTimerKeepAliveSendTimeout();

    // send CS info to ConfParty and SNMP
    m_CommConfService->SendIpServiceParamInd(pServiceDynamic);
    m_CommSnmpService->SendIpServiceParamInd(pServiceDynamic);

    if ( false == ::IsJitcAndNetSeparation() ) // otherwise it was done in 'HandleNetworkSeparationConfigurations' method
    {
    	 if (eProductTypeRMX2000 == curProductType)
    	        status = AddRoutingTableRule(pServiceDynamic);
    	 else
    		 status = AddRoutingTableNetRule(pServiceDynamic);
    }

	CSegment* seg = new CSegment;
	*seg << m_csId;
	CManagerApi(eProcessCSMngr).SendMsg(seg, SIGNALINGTASK_TO_CSMNGR_SERVICE_UP_IND);

   	return status;
}

//////////////////////////////////////////////////////////////////////
BOOL  CSignalingTask::isCSIPV6AutoConfigforNinjaGesher(CIPService* pService)
{
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (pService)
	{
		if ((eProductTypeNinja == curProductType) || (eProductTypeGesher == curProductType))
		{
			eIpType ipType = pService->GetIpType();
			eV6ConfigurationType ipV6configType = pService->GetIpV6ConfigurationType();

			if ((ipType != eIpType_IpV4) && (ipV6configType == eV6Configuration_Auto))
			{
				return YES;
			}
		}
	}

	return NO;
}
void CSignalingTask::HandleNetworkSeparationConfigurations()
{
	TRACESTR(eLevelInfoNormal) << "\nCSignalingTask::HandleNetworkSeparationConfigurations.";

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if((eProductTypeRMX4000 == curProductType) ||
			(eProductTypeRMX1500 == curProductType) ||
			((eProductTypeRMX2000 == curProductType) && IsJitcAndNetSeparation()) || 
			((eProductTypeNinja == curProductType) || (eProductTypeGesher == curProductType)))
	{
		eConfigInterfaceType ifType = eSignalingNetwork;
		if(IsJitcAndNetSeparation())
			ifType = eSeparatedSignalingNetwork;

		CIPServiceList *pIpServListDynamic = m_pProcess->GetIpServiceListDynamic();
		CIPService *pService = pIpServListDynamic->GetService(m_csId);
		if (pService)
		{
			if(!pService->GetIsV35GwEnabled() )
			{
				BOOL bConfig = YES;
				if (((eProductTypeNinja == curProductType) || (eProductTypeGesher == curProductType)) &&
					(NO == isCSIPV6AutoConfigforNinjaGesher(pService)))
				{
					bConfig = NO;
				}

				if (YES == bConfig)
				{
					ConfigureNetwork(pService, ifType);
				}
			}
			else
			{
				TRACESTR(eLevelInfoNormal) << "\nCSignalingTask::HandleNetworkSeparationConfigurations, service " << pService->GetName() << " is a v35 service, no NS configuration needed.";
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::HandleMultipleNetworksConfigurations()
{
	BOOL isV35JitcSupport = NO;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, isV35JitcSupport);

	if((m_pProcess->GetIsMultipleServices() == eTRUE )|| isV35JitcSupport)
	{
		TRACESTR(eLevelInfoNormal) << "\nCSignalingTask::HandleMultipleNetworksConfigurations.";
		CConfigManagerApi api;

		CIPServiceList *pIpServListDynamic = m_pProcess->GetIpServiceListDynamic();
		CIPService *pService = pIpServListDynamic->GetService(m_csId);
		if(pService)
		{
			//Only V35 service under 'v35 jitc support' uses 'Multiple services' mechanisms (vlans & NAT via media card).
			//Non V35 service (Regular media service) under 'v35 jitc support' goes via vlan 2198 - network separation style.
			if(!pService->GetIsV35GwEnabled() && isV35JitcSupport)
			{
				TRACESTR(eLevelInfoNormal) << "\nCSignalingTask::HandleMultipleNetworksConfigurations, service " << pService->GetName() << " is NOT a v35 service, no MS configuration needed.";
			}
			else
			{
				TRACESTR(eLevelInfoNormal) << "\nCSignalingTask::HandleMultipleNetworksConfigurations for service=" << pService->GetName();
				DWORD serviceId = pService->GetId();

				DWORD boardIdUsedByCS =0 , subBoardIdUsedByCS =0 ;
				//m_pProcess->GetSignalingMasterBoardId(m_csId, boardIdUsedByCS, subBoardIdUsedByCS);



				CIPServiceList *pIpServListDynamic = m_pProcess->GetIpServiceListDynamic();
				CIPService *pService = pIpServListDynamic->GetService(m_csId);
				if(pService)
				{
					DWORD serviceId = pService->GetId();
					if(pService->GetIsV35GwEnabled())
					{
						m_pProcess->GetSignalingMasterBoardId(serviceId, boardIdUsedByCS, subBoardIdUsedByCS);
					}
					else
					{
						boardIdUsedByCS = m_pProcess->GetCSIpConfigMasterBoardId(serviceId);
						subBoardIdUsedByCS = m_pProcess->GetCSIpConfigMasterPqId(serviceId);
					}

				}
				if(boardIdUsedByCS)
				{
					eConfigInterfaceType ifType = GetSignalingNetworkType(boardIdUsedByCS, subBoardIdUsedByCS);
					DWORD vlanId = CalcMSvlanId(boardIdUsedByCS, subBoardIdUsedByCS);
					DWORD ipAddress	= GetVlanCSInternalIpv4Address(vlanId);
					DWORD internalGWAddress = GetVlanCardInternalIpv4Address(vlanId);

					//Get ipv4 address
					char ipStr[IP_ADDRESS_LEN]="";
					SystemDWORDToIpString(ipAddress,ipStr);

					char internalGWAddressStr[IP_ADDRESS_LEN]="";
					SystemDWORDToIpString(internalGWAddress, internalGWAddressStr);

					std::list<CRouter> routerList;
					DWORD mask = 0xffffff00;

					eIpType ipType = pService->GetIpType();
					//Cleanup
					api.DelRouteTableRule(ifType, ipType,ipStr, mask, internalGWAddressStr);
					//Add
					//api.AddDefaultGW(ifName, stIpv4Gateway);
					TRACESTR(eLevelInfoNormal) << "\nCSignalingTask::HandleMultipleNetworksConfigurations, ipv4 Add, vlan=" << vlanId;

					api.AddRouteTableRule(ifType,ipType, ipStr, mask, internalGWAddressStr,"", routerList);
				}
				else
					PASSERTMSG(TRUE, "boardIdUsedByCS == 0");
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::ConfigureNetwork(CIPService* pService, const eConfigInterfaceType ifType)
{
	if(pService)
	{
		 std::string ifName;

		 eIpType ipType = pService->GetIpType();
		 ifName = GetLogicalInterfaceName(ifType, ipType);

		TRACESTR(eLevelInfoNormal) << "\nCSignalingTask::ConfigureNetwork for service=" << pService->GetName() << " , type=" << ifName;

		CConfigManagerApi api;

		//1. Interface up
		api.InterfaceUp(ifName);

		char defaultGatewayIPv4Str[IP_ADDRESS_LEN] = "";
		char defaultGatewayIPv6Str[IPV6_ADDRESS_LEN] = "";
		BOOL bIsAutoConfig = FALSE;

		//2. IPv6 Autoconfiguration
		eV6ConfigurationType ipv6CfgType = pService->GetIpV6ConfigurationType();
		if(eV6Configuration_Auto == ipv6CfgType)
			bIsAutoConfig = TRUE;

		//api.SetNICIpV6AutoConfig(ifName, bIsAutoConfig);

		//3. Get ipv4 address
		char ipStr[IP_ADDRESS_LEN]="";
		CIPSpan* pTmpSpan = pService->GetSpanByIdx(0); // Mngmnt params are stored in the 1st span (idx==0)
		if(pTmpSpan)
		{
			DWORD ipAddress   = pTmpSpan->GetIPv4Address();
			SystemDWORDToIpString(ipAddress,ipStr);

			//4. IPv4 Default GW setup
			DWORD ipv4_gateway = pService->GetDefaultGatewayIPv4();
			pService->GetDefaultGatewayIPv6(defaultGatewayIPv6Str, FALSE);

			DWORD mask = pService->GetNetMask();

			SystemDWORDToIpString(ipv4_gateway, defaultGatewayIPv4Str);
			std::string stIpv4Gateway = defaultGatewayIPv4Str;

			std::list<CRouter> routerList;
			RetrieveRouterList(pService, routerList);

			if (stIpv4Gateway != "0.0.0.0" &&
					stIpv4Gateway != "255.255.255.255")
			{
				eIpType ipType = pService->GetIpType();
				//Cleanup
				TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::ConfigureNetwork, ipv4 cleanup";
				api.DelRouteTableRule(ifType, ipType,ipStr, mask, stIpv4Gateway);
				//Add
				//api.AddDefaultGW(ifName, stIpv4Gateway);
				TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::ConfigureNetwork, ipv4 Add";

				api.AddRouteTableRule(ifType,ipType, ipStr, mask, stIpv4Gateway,"", routerList);
			}

			ipType = pService->GetIpType();
			if(eIpType_IpV6 == ipType || eIpType_Both == ipType)
			{
				TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::ConfigureNetwork, ipv6 section";
				//5. Get IPv6 address
				std::string stIPv6Address = pTmpSpan->GetIPv6Address();
				char ipv6SubNetMask[IPV6_ADDRESS_LEN];
				pTmpSpan->GetIPv6SubnetMaskStr(0, ipv6SubNetMask);

				eIpType ipType = pService->GetIpType();
				//6. IPv6 Default GW setup
				std::string stIpv6Gateway = defaultGatewayIPv6Str;
				//Cleanup
				api.DelRouteTableRule(ifType,ipType, stIPv6Address, mask, stIpv6Gateway, TRUE);
				//Add

				api.AddRouteTableRule(ifType,ipType, stIPv6Address, mask, stIpv6Gateway,ipv6SubNetMask, routerList, TRUE);
			}
		}
		else
			PASSERTMSG(1, "CCSMngrManager::ConfigureNetwork, invalid span");

	//	SetIpv6Params();	// VNGFE-2988 - Judith: The service router is deleted. Ori thinks we don't need to erase teh IPV6 addresses anymore
	}
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::SetIpv6Params()
{
	DWORD ipv4Add			= 0;
	string ipv6Add_0		= "",
		   ipv6Add_1		= "",
		   ipv6Add_2		= "",
		   ipv6_defGw		= "";

	DWORD ipv6Mask_0		= DEFAULT_IPV6_SUBNET_MASK,
		  ipv6Mask_1		= DEFAULT_IPV6_SUBNET_MASK,
		  ipv6Mask_2		= DEFAULT_IPV6_SUBNET_MASK,
		  ipv6Mask_defGw	= DEFAULT_IPV6_SUBNET_MASK;

	m_pProcess->SetIpv6Params(ipv4Add,
				  			  ipv6Add_0, ipv6Add_1 ,ipv6Add_2, ipv6_defGw,
				  			  ipv6Mask_0, ipv6Mask_1, ipv6Mask_2, ipv6Mask_defGw,
				  			  m_CommConfService, m_CommSnmpService, TRUE);
}

////////////////////////////////////////////////////////////////////////////
void CSignalingTask::RetrieveRouterList(CIPService *pService, std::list<CRouter> & routerList)
{
	const CH323Router *currentRouter = pService->GetFirstRouter();
	while(NULL != currentRouter)
	{
		if(false == currentRouter->IsDefault())
		{
			char targetIpBuffer		[128];
			char subnetMaskIPv4Buffer	[128];
			char gatewayBuffer		[128];

			SystemDWORDToIpString(currentRouter->GetRemoteIP()	, targetIpBuffer);
			SystemDWORDToIpString(currentRouter->GetSubnetMask(), subnetMaskIPv4Buffer);
			SystemDWORDToIpString(currentRouter->GetRouterIP()	, gatewayBuffer);

			CRouter param;
			param.m_type 		= (currentRouter->GetRemoteFlag() == H323_REMOTE_NETWORK ? router_net : router_host);
			param.m_targetIP 	= targetIpBuffer;
			param.m_subNetmask 	= subnetMaskIPv4Buffer;
			param.m_gateway 	= gatewayBuffer;

			routerList.push_back(param);
		}
		currentRouter = pService->GetNextRouter();
	}
}

/////////////////////////////////////////////////////////////////////////////
STATUS CSignalingTask::AddRoutingTableRule(CIPService *pService)
{
	STATUS retStatus = STATUS_OK;
	CConfigManagerApi api;

	eConfigInterfaceType ifType = m_pProcess->GetSignalingInterfaceType(m_csId);

	char ipStr[IP_ADDRESS_LEN]="";
	CIPSpan* pTmpSpan = pService->GetSpanByIdx(0); // Mngmnt params are stored in the 1st span (idx==0)
	DWORD ipAddress = 0;
	if(pTmpSpan)
	{
		ipAddress = pTmpSpan->GetIPv4Address();
		SystemDWORDToIpString(ipAddress,ipStr);
	}


	char defaultGatewayIPv4Str[IP_ADDRESS_LEN] = "";
	char defaultGatewayIPv6Str[IPV6_ADDRESS_LEN] = "";

	DWORD ipv4_gateway = pService->GetDefaultGatewayIPv4();
	pService->GetDefaultGatewayIPv6(defaultGatewayIPv6Str, FALSE);

	SystemDWORDToIpString(ipv4_gateway, defaultGatewayIPv4Str);
	std::string stIpv4Gateway = defaultGatewayIPv4Str;

	DWORD mask = pService->GetNetMask();

	eIpType ipType = pService->GetIpType();

	std::list<CRouter> routerList;
	RetrieveRouterList(pService, routerList);

	if ( (eIpType_Both == ipType) || (eIpType_IpV4 == ipType) )
	{
		if (stIpv4Gateway != "0.0.0.0" &&
			stIpv4Gateway != "255.255.255.255")
		{


			eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
			std::string ifName;

			ifName = GetLogicalInterfaceName(ifType, ipType);

			api.AddDefaultGW(ifName, stIpv4Gateway);
			api.AddStaticIpRoutes(ifType, routerList);
		}
	}

	if (STATUS_OK == retStatus)
	{
		if ( (eIpType_Both == ipType) || (eIpType_IpV6 == ipType) )
		{
			//5. Get IPv6 address
			char ipv6SubNetMask[IPV6_ADDRESS_LEN];
			std::string stIPv6Address ;
			if(pTmpSpan)
			{
				pTmpSpan->GetIPv6SubnetMaskStr(0, ipv6SubNetMask);
				stIPv6Address = pTmpSpan->GetIPv6Address();
			}

			//6. IPv6 Default GW setup
			std::string stIpv6Gateway = defaultGatewayIPv6Str;
			ipType = pService->GetIpType();
			retStatus = api.AddRouteTableRule(ifType, ipType, stIPv6Address, mask, stIpv6Gateway,ipv6SubNetMask, routerList, TRUE);
		}
	}


	return retStatus;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CSignalingTask::AddRoutingTableNetRule(CIPService *pService)
{
	STATUS retStatus = STATUS_OK;
	CConfigManagerApi api;

	eConfigInterfaceType ifType = eSignalingNetwork;
	if(IsJitcAndNetSeparation())
		ifType = eSeparatedSignalingNetwork;

	char ipStr[IP_ADDRESS_LEN]="";
	CIPSpan* pTmpSpan = pService->GetSpanByIdx(0); // Mngmnt params are stored in the 1st span (idx==0)
	if(IsValidPObjectPtr(pTmpSpan))
	{
	DWORD ipAddress   = pTmpSpan->GetIPv4Address();
	SystemDWORDToIpString(ipAddress,ipStr);


	char defaultGatewayIPv4Str[IP_ADDRESS_LEN] = "";
	char defaultGatewayIPv6Str[IPV6_ADDRESS_LEN] = "";

	DWORD ipv4_gateway = pService->GetDefaultGatewayIPv4();
	pService->GetDefaultGatewayIPv6(defaultGatewayIPv6Str, FALSE);

	SystemDWORDToIpString(ipv4_gateway, defaultGatewayIPv4Str);
	std::string stIpv4Gateway = defaultGatewayIPv4Str;

	DWORD mask = pService->GetNetMask();

	eIpType ipType = pService->GetIpType();

	std::list<CRouter> routerList;
	RetrieveRouterList(pService, routerList);

	if ( (eIpType_Both == ipType) || (eIpType_IpV4 == ipType) )
	{
		if (stIpv4Gateway != "0.0.0.0" &&
			stIpv4Gateway != "255.255.255.255")
		{
			std::string ifName;

			ifName = GetLogicalInterfaceName(ifType, ipType);

			api.AddDefaultGW(ifName, stIpv4Gateway);
			api.AddStaticRoutes(ifType, routerList);
		}
	}

	if (STATUS_OK == retStatus)
	{
		if ( (eIpType_Both == ipType) || (eIpType_IpV6 == ipType) )
		{
			//5. Get IPv6 address
			char ipv6SubNetMask[IPV6_ADDRESS_LEN];
			pTmpSpan->GetIPv6SubnetMaskStr(0, ipv6SubNetMask);
			std::string stIPv6Address = pTmpSpan->GetIPv6Address();

			//6. IPv6 Default GW setup
			std::string stIpv6Gateway = defaultGatewayIPv6Str;
			eIpType ipType = pService->GetIpType();
			retStatus = api.AddRouteTableRule(ifType,ipType, stIPv6Address, mask, stIpv6Gateway,ipv6SubNetMask, routerList, TRUE);
		}
	}
	}
	else
		retStatus = STATUS_FAIL;

	return retStatus;
}

//////////////////////////////////////////////////////////////////////
STATUS CSignalingTask::OnCsDelServiceInd(CSegment *pSeg)
{
	STATUS status = m_CommServiceService->Receive(CS_DEL_SERVICE_REQ, pSeg);
	CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "to receive CS DELETE SERVICE from CS Module", status);

	return status;
}

//////////////////////////////////////////////////////////////////////
STATUS CSignalingTask::OnCsReconnectInd(CSegment *pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCSignalingTask::OnCsReconnectInd";

	//	CS_Reconnect_Ind_S *param = (CS_Reconnect_Ind_S*)pSeg->GetPtr();

	// ===== 1. stop previous timers etc
	DeleteTimer(KEEP_ALIVE_TIMER_SEND);
	DeleteTimer(KEEP_ALIVE_TIMER_RECEIVE);
	RemoveActiveAlarmByErrorCode(NO_CONNECTION_WITH_CS);
	m_numOfkeepAliveTimeoutReached = 0;

	// ===== 2. restart KeepAlive polling
	OnTimerKeepAliveSendTimeout();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::OnTimerKeepAliveSendTimeout()
{
	SendCsKeepAliveRequestToCs();
	StartTimer(KEEP_ALIVE_TIMER_RECEIVE, m_keepAliveReceivePeriod*SECOND);
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::SendCsKeepAliveRequestToCs()
{
    TRACESTR(eLevelInfoNormal) << "\nCSignalingTask::SendCsKeepAliveRequestToCs";
	SystemGetTime(m_lastKeepAliveReqTime);

	CMplMcmsProtocol mplProt;

	mplProt.AddCommonHeader(CS_KEEP_ALIVE_REQ, 0, 0, 0, eCentral_signaling);
	mplProt.AddMessageDescriptionHeader();
	mplProt.AddCSHeader(m_csId, 0, eServiceMngr);

	CMplMcmsProtocolTracer(mplProt).TraceMplMcmsProtocol("CSignalingTask Sends to CS Module", CS_API_TYPE);
	mplProt.SendMsgToCSApiCommandDispatcher();
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::PrintLastKeepAliveTimes()
{
	CSmallString headline;
	headline	<< "\nCSignalingTask::PrintLastKeepAliveTimes -";

	COstrStream lastReqTimeStr, lastIndTimeStr;
	m_lastKeepAliveReqTime.Serialize(lastReqTimeStr);
	m_lastKeepAliveIndTime.Serialize(lastIndTimeStr);

	string kaStr = headline.GetString();
	kaStr += "\nLast KeepAliveReq was sent at     ";
	kaStr += lastReqTimeStr.str();
	kaStr += "\nLast KeepAliveInd was received at ";
	kaStr += lastIndTimeStr.str();


// temp: not printing
	TRACESTR(eLevelInfoNormal) << kaStr.c_str();
}
//////////////////////////////////////////////////////////////////////
STATUS CSignalingTask::TryStartSignalingServiceConfiguration()
{
	//the service ip is ok and the CS finished the startup
    if ( !GetIsCsMngrEndIpConfigIndReceived() || !GetSignalingStartupEndedReceived() )
    {
        TRACEINTO << "\nCSignalingTask::TryStartSignalingServiceConfiguration : "
                  << "IP is not ready or CS didn't finish the startup, therefore can't start service configuration"
                  << "\nIsCsMngrEndIpConfigIndReceived:  " << (GetIsCsMngrEndIpConfigIndReceived()	? "yes" : "no")
                  << "\nIsSignalingStartupEndedReceived: " << (GetSignalingStartupEndedReceived()	? "yes" : "no");

        return STATUS_OK;
    }

    if(GetState() == NEW_SERVICE_CONFIGURATION)
    {
        TRACEINTO << "\nCSignalingTask::TryStartSignalingServiceConfiguration : "
                  << "state == NEW_SERVICE_CONFIGURATION, so no service configuration will be done";
        return STATUS_OK;
    }

    STATUS status = m_CommServiceService->StartNewServiceConfiguration();
	if(STATUS_OK == status)
	{
		SetState(NEW_SERVICE_CONFIGURATION);
	}
    return status;
}

//////////////////////////////////////////////////////////////////////
const char *CSignalingTask::GetStateStr(WORD state)
{
//const WORD  IDLE			= 0;        // default state
// const WORD	STARTUP						= 1;
// const WORD	NEW_SERVICE_CONFIGURATION 	= 2;
// const WORD	READY						= 3;
// const WORD	WAIT_FOR_DNS_IND    		= 4;
// const WORD  AFTER_STARTUP               = 5;

    static char * names [] =
        {
            "IDLE",
            "STARTUP",
            "NEW_SERVICE_CONFIGURATION",
            "READY",
//            "WAIT_FOR_DNS_IND",
            "AFTER_STARTUP"
        };

    const DWORD namesLen = sizeof(names) / sizeof(names[0]);
    const char *name = (state < namesLen
                        ?
                        names[state] : "Invalid state");
    return name;
}

//////////////////////////////////////////////////////////////////////
STATUS CSignalingTask::OnCsMngrEndIpConfigInd(CSegment *pSeg)
{
	TRACEINTO << "\nCSignalingTask::OnCsMngrEndIpConfigInd";
	SetIsCsMngrEndIpConfigIndReceived(TRUE);

	return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////

void CSignalingTask::SetIsCsMngrEndIpConfigIndReceived(bool val)
{
	PTRACE2INT(eLevelInfoNormal, "CSignalingTask::SetIsCsMngrEndIpConfigIndReceived, val:", val);

	m_flagIsCsMngrEndIpConfigIndReceived = val;
	TryStartSignalingServiceConfiguration();
}
//////////////////////////////////////////////////////////////////////
void CSignalingTask::OnTimerKeepAliveReceiveTimeout()
{
	if ( CS_KEEP_ALIVE_RETRIES > m_numOfkeepAliveTimeoutReached )
	{
		m_numOfkeepAliveTimeoutReached++;
	}

	else
	{
		if ( NO == m_isKeepAliveFailureAlreadyTreated )
		{
			m_isKeepAliveFailureAlreadyTreated = YES;
			TreatCsKeepAliveFailure();
                        return;
		}
	}

	// and anyway, keep sending KeepAlive requests
	OnTimerKeepAliveSendTimeout();
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::OnCsKeepAliveInd(CSegment* pSeg)
{
	SystemGetTime(m_lastKeepAliveIndTime);

	// ===== 1. print (if periodTrace==YES)
//	BOOL isPeriodTrace = FALSE;
//	pCSMngrProcess->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_PERIOD_TRACE, isPeriodTrace);
//	if (YES == isPeriodTrace)
//	{
		//TRACESTR(eLevelInfoNormal) << "CCSMngrManager::OnCsKeepAliveInd - KeepAlive indication received";
//	}

	// ===== 1. stop previous timer
	DeleteTimer(KEEP_ALIVE_TIMER_RECEIVE);
	RemoveActiveAlarmByErrorCode(NO_CONNECTION_WITH_CS);
	m_numOfkeepAliveTimeoutReached = 0;

	// ===== 2. fill attribute with data from structure received
	csKeepAliveSt *tmpKA = (csKeepAliveSt*)pSeg->GetPtr();

	// ===== 3. check results
	if ( YES == IsKeepAliveChangedFromPrevious(tmpKA) )
	{
		CheckKeepAliveResults(tmpKA);
		memcpy( &m_keepAliveStructure, tmpKA, sizeof(csKeepAliveSt) );
	}

	// ===== 4. start the timer for the next keepAlive
	StartTimer(KEEP_ALIVE_TIMER_SEND, m_keepAliveSendPeriod*SECOND);
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::TreatCsKeepAliveFailure()
{
	// ===== 1. produce a Fault (NoConnectionWithCard)
	ProduceFaultAndLogger(eFailureTypeCsNoConnection);

	PrintLastKeepAliveTimes();

	// ===== 2. ask reset from Daemon
	SendResetReqToDaemon("No connection with CS");
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::SendResetReqToDaemon(CSmallString errStr)
{
    TRACEINTO << "CSignalingTask::SendResetReqToDaemon - " << errStr.GetString();

    string desc = errStr.GetString();
    CMcmsDaemonApi api;
    //TBD - jud - add CsId to Daemon request
    STATUS res = api.SendResetReq(desc/*, m_csId*/);

}

//EXT-4321: The timer to check Signaling
void CSignalingTask::OnTimerSignalPortInactTimeout()
{
	CSegment *pMsg = new CSegment;
	DWORD eventType = eFailoverSignalPortFailure;
	*pMsg << eventType;
	CManagerApi api(eProcessFailover);
	api.SendMsg(pMsg, FAILOVER_EVENT_TRIGGER_IND);

	TRACESTR(eLevelInfoNormal) << "\nCSMngrManager::OnTimerSignalPortInactTimeout, trigger the Hot backup event!";
}


//////////////////////////////////////////////////////////////////////
//EXT-4321: to check the signaling port for Failover
void CSignalingTask::CheckSignalPortforFailover(compStatuses status)
{
	DWORD Eth_Inactivity_Duration = 30;
	CSysConfig* pSysConfig = NULL;
	std::string key = CFG_KEY_ETH_INACTIVITY_DURATION;

	if (status == emCompOk) {
		TRACESTR(eLevelInfoNormal) << "\nCSMngrManager::CheckSignalPortforFailover, Signaling port is UP";
		m_flagIsSignalPortUp = TRUE;
		if (IsValidTimer(CS_SIGNAL_PORT_INACTIVITY_TIMER)) {
			DeleteTimer(CS_SIGNAL_PORT_INACTIVITY_TIMER);
		}
	} else if (status == emCompFailed) {
		if (m_flagIsSignalPortUp == TRUE) {
			/*If signaling port is down, start a timer to check whether it is not up again in Eth_Inactivity_Duration seconds*/
			TRACESTR(eLevelInfoNormal) << "\nCSMngrManager::CheckSignalPortforFailover, Signaling port is DOWN";
			m_flagIsSignalPortUp = FALSE;
			pSysConfig = CProcessBase::GetProcess()->GetSysConfig();;
			pSysConfig->GetDWORDDataByKey(key, Eth_Inactivity_Duration);
			StartTimer(CS_SIGNAL_PORT_INACTIVITY_TIMER, Eth_Inactivity_Duration*SECOND);
		}
	}
	return;
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::CheckKeepAliveResults(csKeepAliveSt *tmpKA)
{
	TRACESTR(eLevelInfoNormal) << "===== CS KeepAlive Changed!! =====\n"
	                              << CCSKeepAliveIndWrapper(*tmpKA);

	DWORD faultId = 0;

	compTypes    curUnitType      = emNonComponent;
	compStatuses curUnitNewStatus = emCompOk;
	eProductFamily prodFamily = CProcessBase::GetProcess()->GetProductFamily();
	eProductType prodType = CProcessBase::GetProcess()->GetProductType();


	for (int i=0; i<NumOfComponents; i++)
	{
		curUnitType      = tmpKA->componentTbl[i].type;
		curUnitNewStatus = tmpKA->componentTbl[i].status;

		if ( emNonComponent == curUnitType) // component i does not exist
		{
			continue;
		}
		// EXT-4321: to check the Signaling port's status for failover
		if (emCompCSSignalPort == curUnitType)
		{
			/*BRIDGE-1495/BRIDGE-9355: For SoftMCU(except NINJA), we don't need to monitor signal port's the status here */
			if(	(eProductFamilySoftMcu == prodFamily) &&
				(eProductTypeNinja != prodType))
			{
				continue;
			}
			/*For multipleServiceMode, we will check the signaling port's status in MFA*/
			if ((m_pProcess->GetIsMultipleServices() == eTRUE) || (m_pProcess->GetIsMultipleServices() == eMSNotConfigure))
			{
				continue;
			}
			else
			{
				CheckSignalPortforFailover(curUnitNewStatus);
			}
		}
		if (emCompOk == curUnitNewStatus)
		{
			// ===== Remove old ActiveAlarm (if exists)
			RemoveCSActiveAlarm(CS_COMPONENT_FATAL, i);
		}
		else
		{
			// ===== if it's a new failure, then produce a Fault (ComponentFatal) etc.
			if ( YES == IsNewFailure(i, curUnitNewStatus) )
			{
				RemoveCSActiveAlarm(CS_COMPONENT_FATAL, i);
				ProduceFaultAndLogger(eFailureTypeCsUnitFailure, i, curUnitType, curUnitNewStatus);

				/*David Liang: For SignalPort failure, we don't reset the CS*/
				if (emCompCSSignalPort != curUnitType)
				{
					// ===== ask reset from Daemon
					CSmallString errStr = "CS Component Failure; unit type: ";
					char* csCompTypeStr = ::CsCompTypeToString(curUnitType);
					if (csCompTypeStr)
					{
						errStr << csCompTypeStr;
					}
					else
					{
						errStr << "(invalid: " << curUnitType << ")";
					}
					SendResetReqToDaemon(errStr);
				}
			} // end if(IsNewFailure)
		} // end if(curUnitNewStatus != ok)

	} // end loop over MAX_NUM_OF_UNITS

}


//////////////////////////////////////////////////////////////////////
BOOL CSignalingTask::IsKeepAliveChangedFromPrevious(csKeepAliveSt *tmpKA)
{
	DWORD isChanged = NO;

	for (int i=0; i<NumOfComponents; i++)
	{
		if ( (tmpKA->componentTbl[i].status) != (m_keepAliveStructure.componentTbl[i].status) ||
			(tmpKA->componentTbl[i].type) != (m_keepAliveStructure.componentTbl[i].type) )
		{
			isChanged = YES;
			break;
		}
	}

	return isChanged;
}

//////////////////////////////////////////////////////////////////////
DWORD CSignalingTask::ProduceFaultAndLogger(eCsFailureType failureType, int id,
                                            compTypes curCompType, compStatuses curCompStatus)
{
	// ===== 1. prepare error description
	CMedString errStr;
	DWORD errCode=0, faultId=0;

	switch (failureType)
	{
		case eFailureTypeCsUnitFailure:
		{
			char* csCompTypeStr = ::CsCompTypeToString(curCompType);
			if (csCompTypeStr)
			{
				errStr << "Central signaling component failure; unit type: " << csCompTypeStr;
			}
			else
			{
				errStr << "Central signaling component failure; unit type: (invalid: " << curCompType << ")";
			}

			errCode = CS_COMPONENT_FATAL;
			break;
		}

		case eFailureTypeCsNoConnection:
		{
			errStr << "No connection with central signaling";
			errCode = NO_CONNECTION_WITH_CS;
			break;
		}

		default:
		{
			errStr  << "Central signaling component failure - Invalid failure type"
			        << " Unit id: " << id
			        << ", Type: "   << curCompType
			        << ", Status: " << curCompStatus;

			errCode = CS_COMPONENT_FATAL;
			break;
		}
	} // end switch (failureType)


	// ===== 2. to Faults
	faultId = AddCSActiveAlarmSingleton( errCode, id, errStr.GetString());
//	faultId = AddActiveAlarm( FAULT_GENERAL_SUBJECT, errCode, MAJOR_ERROR_LEVEL, errStr.GetString(),
//	                          true, true, id /*compId as 'userId' (token)*/);

	// ===== 3. to Logger
	TRACEINTO << "CSignalingTask::ProduceUnitFailureFaultAndLogger - " << errStr.GetString();

	return faultId;
}
//////////////////////////////////////////////////////////////////////
BOOL CSignalingTask::IsNewFailure(int id, compStatuses unitStatus)
{
	BOOL isNew = YES;

	if ( m_keepAliveStructure.componentTbl[id].status == unitStatus )
		isNew = NO;

	return isNew;
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::InitkeepAliveStruct()
{
	int i=0;
	for (i=0; i<NumOfComponents; i++)
	{
		m_keepAliveStructure.componentTbl[i].status = emCompOk;
	}
}

/////////////////////////////////////////////////////////////////////
void CSignalingTask::SetState(WORD newState)
{
    TRACEINTO << "\nCSignalingTask::SetState : "
              << GetStateStr(m_state) << " -> " << GetStateStr(newState);

    m_state = newState;
}

/////////////////////////////////////////////////////////////////////
void  CSignalingTask::OnCsMngrProxyIpServiceParamReq(CSegment* pSeg)
{
	STATUS status;
	m_CommProxyService->SetIsConnected(true);

	if (m_state == READY)
	{
		status = SendCsNewIndicationToProxyService();
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__, "to send Cs New Ind to SipProxy", status);
	}
}

/////////////////////////////////////////////////////////////////////
void CSignalingTask::OnCsMngrSnmpUnterfaceReq(CSegment* pSeg)
{
	if (m_state != READY)
    {
        TRACEINTO << "\nCSignalingTask::OnCsMngrSnmpUnterfaceReq - CS is not configured yet, nothing will be sent to SNMP for csId: "<<m_csId;
        return;
    }

    CIPServiceList *pServiceListDynamic = m_pProcess->GetIpServiceListDynamic();
    if(NULL == pServiceListDynamic)
    {
        PASSERTMSG(TRUE, "No Dynamic service list");
        return;
    }

    CIPService *pNewIpServiceDynamic = pServiceListDynamic->GetService(m_csId);

    if(NULL == pNewIpServiceDynamic)
    {
        PASSERTMSG(TRUE, "No Dynamic service");
        return;
    }

    m_CommSnmpService->SendIpServiceParamInd(pNewIpServiceDynamic);
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::OnCsCompStatusInd(CSegment* pSeg)
{
	// ===== 1. deserialize the segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg, CS_API_TYPE);
	CMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("CSMNGR_RECEIVED_FROM_CS");

	// ===== 2. fill attribute with data from structure received
	csCompStatusSt *tmpCompStatus = (csCompStatusSt*)pSeg->GetPtr();
	memset(&m_compStatusStructure, 0, sizeof(csCompStatusSt));
	memcpy(&m_compStatusStructure, tmpCompStatus, sizeof(csCompStatusSt));
	POBJDELETE(pMplMcmsProtocol);

	// ===== 3. poroduce Logger and Fault
	// prepare the string
	COstrStream errStr;
	GenerateCompStatusStStr(errStr);

	// ===== 4. poroduce the Logger and the Fault
	TRACESTR(eLevelInfoNormal) << "\nCSignalingTask::OnCsCompStatusInd"
	                              << "\n" << errStr.str().c_str();

	CHlogApi::CsRecoveryStatus( errStr.str().c_str() );
}

//////////////////////////////////////////////////////////////////////
void CSignalingTask::GenerateCompStatusStStr(ostream &ostr)
{
	// ===== 1. prepare params for printing
	char *compTypeStr	= ::CsCompTypeToString(m_compStatusStructure.type),
	     *compStatusStr	= ::CsCompStatusToString(m_compStatusStructure.status),
	     *reasonStr		= ::CsRecoveryReasonToString(m_compStatusStructure.reason);

	m_pProcess->TestStringValidity( (char*)(m_compStatusStructure.unitName),
						                MAX_UNIT_NAME_SIZE,
						                "CSignalingTask::GenerateCompStatusStStr" );

	m_pProcess->TestStringValidity( (char*)(m_compStatusStructure.errorStr),
						                MaxRecoveryErrorStrSize,
						                "CSignalingTask::GenerateCompStatusStStr" );


	// ===== 2. generate the output string
	ostr << "Unit " << m_compStatusStructure.unitId;

	if (compTypeStr)
	{
        ostr << "; process type: " << compTypeStr;
	}
	else
	{
		ostr << "; (invalid process type: " << m_compStatusStructure.type << ")";
	}

	if (compStatusStr)
	{
		ostr <<  "; status: " << compStatusStr;
	}
	else
	{
		ostr << "; (invalid status: " << m_compStatusStructure.status << ")";
	}

	if (reasonStr)
	{
		ostr << "; reason: " << reasonStr;
	}
	else
	{
		ostr << "; (invalid reason: " << m_compStatusStructure.reason << ")";
	}

	ostr << "; name: " << m_compStatusStructure.unitName << ". ";
	ostr << m_compStatusStructure.errorStr;
}
//////////////////////////////////////////////////////////////////////
CSmallString CSignalingTask::GetCSSubErrorString(eUserMsgSubErrorCode1_CS errorSubCode1,eUserMsgSubErrorOCSPCode2_CS errorSubCode2)
{
	CSmallString errReasonsStr = "Reason=";
		
	if(errorSubCode1 != TLS_CERTIFICATE_VALID)
	{
		if(errorSubCode1 & TLS_UNABLE_TO_GET_ISSUER_CERT)
		{
			errReasonsStr+="unable to get issuer certificate;";							
		}
		if(errorSubCode1 & TLS_UNABLE_TO_GET_CRL)
		{
			errReasonsStr+="the CRL of a certificate could not be found;";							
		}
		if(errorSubCode1 & TLS_UNABLE_TO_DECRYPT_CERT_SIGNATURE)
		{
			errReasonsStr+="unable to decrypt certificate's signature;";							
		}
		if(errorSubCode1 & TLS_UNABLE_TO_DECRYPT_CRL_SIGNATURE)
		{
			errReasonsStr+="unable to decrypt CRL's signature;";							
		}
		if(errorSubCode1 & TLS_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY)
		{
			errReasonsStr+="unable to decode issuer public key;";							
		}
		if(errorSubCode1 & TLS_CERT_SIGNATURE_FAILURE)
		{
			errReasonsStr+="certificate signature failure;";							
		}
		if(errorSubCode1 & TLS_CRL_SIGNATURE_FAILURE)
		{
			errReasonsStr+="CRL signature failure;";							
		}
		if(errorSubCode1 & TLS_CERT_NOT_YET_VALID)
		{
			errReasonsStr+="certificate is not yet valid;";							
		}
		if(errorSubCode1 & TLS_CERT_HAS_EXPIRED)
		{
			errReasonsStr+="certificate has expired;";							
		}
		if(errorSubCode1 & TLS_CRL_NOT_YET_VALID)
		{
			errReasonsStr+="CRL is not yet valid;";							
		}
		if(errorSubCode1 & TLS_CRL_HAS_EXPIRED)
		{
			errReasonsStr+="CRL has expired;";							
		}
		if(errorSubCode1 & TLS_ERROR_IN_CERT_NOT_BEFORE_FIELD)
		{
			errReasonsStr+="format error in certificate's notBefore field;";							
		}
		if(errorSubCode1 & TLS_ERROR_IN_CERT_NOT_AFTER_FIELD)
		{
			errReasonsStr+="format error in certificate's notAfter field;";							
		}
		if(errorSubCode1 & TLS_ERROR_IN_CRL_LAST_UPDATE_FIELD)
		{
			errReasonsStr+="format error in CRL's lastUpdate field;";							
		}
		if(errorSubCode1 & TLS_ERROR_IN_CRL_NEXT_UPDATE_FIELD)
		{
			errReasonsStr+="format error in CRL's nextUpdate field;";							
		}
		if(errorSubCode1 & TLS_OUT_OF_MEM)
		{
			errReasonsStr+="out of memory;";							
		}
		if(errorSubCode1 & TLS_DEPTH_ZERO_SELF_SIGNED_CERT)
		{
			errReasonsStr+="self signed certificate;";							
		}
		if(errorSubCode1 & TLS_SELF_SIGNED_CERT_IN_CHAIN)
		{
			errReasonsStr+="self signed certificate in certificate chain;";							
		}
		if(errorSubCode1 & TLS_UNABLE_TO_GET_ISSUER_CERT_LOCALLY)
		{
			errReasonsStr+="unable to get local issuer certificate;";							
		}
		if(errorSubCode1 & TLS_UNABLE_TO_VERIFY_LEAF_SIGNATURE)
		{
			errReasonsStr+="unable to verify the first certificate;";							
		}
		if(errorSubCode1 & TLS_CERT_CHAIN_TOO_LONG)
		{
			errReasonsStr+="certificate chain too long;";							
		}
		if(errorSubCode1 & TLS_CERT_REVOKED)
		{
			errReasonsStr+="certificate revoked;";							
		}
		if(errorSubCode1 & TLS_INVALID_CA)
		{
			errReasonsStr+="invalid CA certificate;";							
		}
		if(errorSubCode1 & TLS_PATH_LENGTH_EXCEEDED)
		{
			errReasonsStr+="path length constraint exceeded;";							
		}
		if(errorSubCode1 & TLS_INVALID_PURPOSE)
		{
			errReasonsStr+="unsupported certificate purpose;";							
		}
		if(errorSubCode1 & TLS_CERT_UNTRUSTED)
		{
			errReasonsStr+="certificate not trusted;";							
		}
		if(errorSubCode1 & TLS_CERT_REJECTED)
		{
			errReasonsStr+="certificate rejected;";							
		}
		if(errorSubCode1 & TLS_SUBJECT_ISSUER_MISMATCH)
		{
			errReasonsStr+="subject issuer mismatch;";							
		}
		if(errorSubCode1 & TLS_AKID_SKID_MISMATCH)
		{
			errReasonsStr+="authority and subject key identifier mismatch;";							
		}
		if(errorSubCode1 & TLS_AKID_ISSUER_SERIAL_MISMATCH)
		{
			errReasonsStr+="authority and issuer serial number mismatch;";							
		}
		if(errorSubCode1 & TLS_KEYUSAGE_NO_CERTSIGN)
		{
			errReasonsStr+="key usage does not include certificate signing;";							
		}
	}
	if(errorSubCode2 != TLS_SPC_OCSPRESULT_CERTIFICATE_VALID)
	{
		if(errorSubCode2 & TLS_SPC_OCSPRESULT_CERTIFICATE_REVOKED)
		{
			errReasonsStr+="ocsp certificate revoked;";							
		}
		if(errorSubCode2 & TLS_SPC_OCSPRESULT_ERROR_INVALIDRESPONSE)
		{
			errReasonsStr+="ocsp invalid response;";							
		}
		if(errorSubCode2 & TLS_SPC_OCSPRESULT_ERROR_CONNECTFAILURE)
		{
			errReasonsStr+="ocsp connect failure;";							
		}
		if(errorSubCode2 & TLS_SPC_OCSPRESULT_ERROR_SIGNFAILURE)
		{
			errReasonsStr+="ocsp sign failure;";							
		}
		if(errorSubCode2 & TLS_SPC_OCSPRESULT_ERROR_BADOCSPADDRESS)
		{
			errReasonsStr+="ocsp bad address;";							
		}
		if(errorSubCode2 & TLS_SPC_OCSPRESULT_ERROR_OUTOFMEMORY)
		{
			errReasonsStr+="ocsp out of memory;";							
		}
		if(errorSubCode2 & TLS_SPC_OCSPRESULT_ERROR_UNKNOWN)
		{
			errReasonsStr+="ocsp unknown error;";							
		}
		if(errorSubCode2 & TLS_SPC_OCSPRESULT_ERROR_UNAUTHORIZED)
		{
			errReasonsStr+="ocsp unautorized;";							
		}
		if(errorSubCode2 & TLS_SPC_OCSPRESULT_ERROR_SIGREQUIRED)
		{
			errReasonsStr+="ocsp sign required;";							
		}
		if(errorSubCode2 & TLS_SPC_OCSPRESULT_ERROR_TRYLATER)
		{
			errReasonsStr+="ocsp try later;";							
		}
		if(errorSubCode2 & TLS_SPC_OCSPRESULT_ERROR_INTERNALERROR)
		{
			errReasonsStr+="ocsp internal error;";							
		}
		if(errorSubCode2 & TLS_SPC_OCSPRESULT_ERROR_MALFORMEDREQUEST)
		{
			errReasonsStr+="ocsp mal formed request;";							
		}
	}

	return errReasonsStr;
}

void CSignalingTask::OnCsActiveAlarmInd(CSegment* pSeg)
{
                CMedString messageDescription;
                CSmallString errReasonsStr = "";
                eUserMsgSubErrorCode1_CS errorSubCode1 ;
                eUserMsgSubErrorOCSPCode2_CS errorSubCode2;
                // ===== 1. fill attribute with data from structure received
                USER_MSG_S *tmpActiveAlarm = (USER_MSG_S*)pSeg->GetPtr();
                TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::OnCsActiveAlarmInd"
                                                                                                                                  << "\nMessage:  " << GetUserMsgCode_CsStr((eUserMsgCode_CS)tmpActiveAlarm->messageCode)
                                                                                                                                  << "\nLocation: " << GetUserMsgLocationStr((eUserMsgLocation)tmpActiveAlarm->location)
                                                                                                                                  << "\nOperation:  " << GetUserMsgOperationStr((eUserMsgOperation)tmpActiveAlarm->operation)
                                                                                                                                  << "\nAutoRemoval: " << GetUserMsgAutoRemovalStr((eUserMsgAutoRemoval)tmpActiveAlarm->autoRemoval)
                                                                                                                                  << "\nProcessType:  " << GetUserMsgProcessType_CsStr((eUserMsgProcessType_CS)tmpActiveAlarm->process_type)
                                                                                                                                  << "\nFuture_use1: " << tmpActiveAlarm->future_use1
                                                                                                                                  << "\nFuture_use2: " << tmpActiveAlarm->future_use2;

                memset(&m_activeAlarmStructure, 0, sizeof(USER_MSG_S));
                memcpy(&m_activeAlarmStructure, tmpActiveAlarm, sizeof(USER_MSG_S));

                errorSubCode1 = (eUserMsgSubErrorCode1_CS)tmpActiveAlarm->future_use1;
                errorSubCode2 = (eUserMsgSubErrorOCSPCode2_CS)tmpActiveAlarm->future_use2;
                // ===== 2. produce Alert or Fault
                if (tmpActiveAlarm->operation == eUSerMsgOperation_add) {
                                switch(tmpActiveAlarm->messageCode)
                                {
                                                case eUserMsgCode_Cs_SipTLS_RegistrationHandshakeFailure:
                                                                messageDescription="SIP TLS: Registration handshake failure";
                                                                break;
                                                case eUserMsgCode_Cs_SipTLS_FailedToLoadOrVerifyCertificateFiles:
                                                                messageDescription="SIP TLS: Failed to load or verify certificate files";
                                                                break;
                                                case eUserMsgCode_Cs_SipTLS_RegistrationTransportError:
                                                                messageDescription="SIP TLS: Registration transport error";
                                                                break;
                                                case eUserMsgCode_Cs_SipTLS_RegistrationServerNotResponding:
                                                                messageDescription="SIP TLS: No response from Registration server";
                                                                break;
                                                case eUserMsgCode_Cs_SipTLS_CertificateHasExpired:
                                                                messageDescription="SIP TLS: Certificate has expired";
                                                                break;
                                                case eUserMsgCode_Cs_SipTLS_CertificateWillExpireInLessThanAWeek:
                                                                messageDescription="SIP TLS: Certificate will expire in less than a week";
                                                                break;
                                                case eUserMsgCode_Cs_SipTLS_CertificateSubjNameIsNotValid_Or_DnsFailed:
                                                                messageDescription="SIP TLS: Certificate subject name is not valid or DNS failed to resolve this name";
                                                                break;
                                                case eUserMsgCode_Cs_SipTLS_RemoteCertificateFailure:
                                                                if(errorSubCode1 != TLS_CERTIFICATE_VALID || errorSubCode2 != TLS_SPC_OCSPRESULT_CERTIFICATE_VALID)
                                                                {
                                                                                errReasonsStr+=GetCSSubErrorString(errorSubCode1,errorSubCode2);
					messageDescription="SIP TLS: Remote certificate failure:Service name=";
					messageDescription+=m_CommServiceService->GetServiceName();
					messageDescription+=":";
					messageDescription+=errReasonsStr.GetString();					
				}
				else 
				{						
					messageDescription="SIP TLS: TLS handshake failure. Remote certificate failure:Service name=";
					messageDescription+= m_CommServiceService->GetServiceName();
				}
				break;
			case eUserMsgCode_Cs_SipTLS_RmxCertificateFailure:	
				if(errorSubCode1 != TLS_CERTIFICATE_VALID || errorSubCode2 != TLS_SPC_OCSPRESULT_CERTIFICATE_VALID)
				{					
					errReasonsStr+=GetCSSubErrorString(errorSubCode1,errorSubCode2);					
					messageDescription="SIP TLS: RMX certificate failure:Service name=";
					messageDescription+=m_CommServiceService->GetServiceName();
					messageDescription+=":";
					messageDescription+=errReasonsStr.GetString();
				}
				else
				{					
					messageDescription="SIP TLS: TLS handshake failure. RMX certificate failure:Service name=";
					messageDescription+=m_CommServiceService->GetServiceName();
                                                                }
                                                                break;

                                                default:
                                                                TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::OnCsActiveAlarmInd"
                                                                                                                                                                                  <<  "Incorrect message description"
                                                                                                                                                                                  << "\nMessage:  " << GetUserMsgCode_CsStr((eUserMsgCode_CS)tmpActiveAlarm->messageCode);
                                                                return;
                                }


                                if (tmpActiveAlarm->location == eUserMsgLocation_SysAlerts) {
                                                AddCSActiveAlarmSingleton(AA_EXTERNAL_ALERT_CS, tmpActiveAlarm->messageCode, messageDescription.GetString());
//                                            AddActiveAlarm(             FAULT_GENERAL_SUBJECT,
//                                                                                                            AA_EXTERNAL_ALERT_CS,
//                                                                                                            MAJOR_ERROR_LEVEL,
//                                                                                                            messageDescription,
//                                                                                                            true,
//                                                                                                            true,
//                                                                                                            tmpActiveAlarm->messageCode );

                                } else if (tmpActiveAlarm->location == eUserMsgLocation_Faults) {
                                                CHlogApi::CsGeneralFaults(messageDescription.GetString());
                                }
                } else {

                                DWORD user_id = CreateIDFromCSID(AA_EXTERNAL_ALERT_CS, tmpActiveAlarm->messageCode, m_csId);
                                if (TRUE == IsActiveAlarmExistByErrorCodeUserId(AA_EXTERNAL_ALERT_CS, user_id))
                                {
                                                RemoveCSActiveAlarm(AA_EXTERNAL_ALERT_CS, tmpActiveAlarm->messageCode);
                                }
                                else
                                {
                                                TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::OnCsActiveAlarmInd"
                                                                                                                                                                  << "\nMessage:  " << GetUserMsgCode_CsStr((eUserMsgCode_CS)tmpActiveAlarm->messageCode)
                                                                                                                                                                  << "does not exist";
                                }
                }

}
//////////////////////////////////////////////////////////////////////
void CSignalingTask::OnCsPingInd (CSegment* pMsg)
{
    PTRACE(eLevelInfoNormal, "CSignalingTask::OnCsPingInd");

    CSegment* pSeg = new CSegment(*pMsg);

    CManagerApi api(eProcessCSMngr);
    api.SendMsg(pSeg, CS_PING_IND);


//    m_pCsPinger->DispatchEvent(CS_PING_IND, pMsg);

}
////////////////////////////////////////////////////////////////////////////////////
void CSignalingTask::SendIceTypeToCS(int iceType)
{
	CSmallString logStr = "CCSMngrManager::SendIceTypeToCS";

	string data;

	if (iceType == eIceEnvironment_None)
		data = "NONE";
	else if (iceType == eIceEnvironment_ms)
		data = "MS";
	else
		data = "STANDARD";

	CSysConfig *sysConfig = m_pProcess->GetSysConfig();

	const char *currentSectionName = GetCfgSectionName(eCfgSectionCSModule);

	sysConfig->OverWriteParam(CFG_KEY_ICE_TYPE, data, currentSectionName);

	const CCfgData *cfgData = sysConfig->GetCfgEntryByKey(CFG_KEY_ICE_TYPE);

	if( false == CCfgData::TestValidity(cfgData) )
	{
		logStr << "failed to validate sysConfig, key: " << CFG_KEY_ICE_TYPE;
		TRACESTR(eLevelInfoNormal) << logStr.GetString();

		return;
	}

	logStr << " change ICE type to: " << data.c_str();

	STATUS sendStat = m_CommStartupService->SendConfigParamSingle(cfgData);

	if (STATUS_OK != sendStat)
	{
		logStr << "\nFailed sending to CS! Status: " << sendStat;
	}

	TRACESTR(eLevelInfoNormal) << logStr.GetString();
}

////////////////////////////////////////////////////////////////////////////////////
void CSignalingTask::SendIceGruuToCs(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CSignalingTask::SendICEGruuToCs");
	CSysConfig *sysConfig = m_pProcess->GetSysConfig();
	const CCfgData *cfgData = sysConfig->GetCfgEntryByKey("SIP_CONTACT_OVERRIDE_STR");

	if( false == CCfgData::TestValidity(cfgData) )
	{
		TRACEINTO << "failed to validate sysConfig, key: SIP_CONTACT_OVERRIDE_STR" ;
		return;
	}

	STATUS sendStat = m_CommStartupService->SendConfigParamSingle(cfgData);
	if (STATUS_OK != sendStat)
	{
		TRACEINTO << "Failed sending to CS! Status: " << sendStat;
	}
}

////////////////////////////////////////////////////////////////////////////////////
void CSignalingTask::OnCsMngrIceTypeInd(CSegment* pSeg)
{
	WORD iceType;
	*pSeg >> iceType;

	SendIceTypeToCS(iceType);
	SendIceGruuToCs(NULL);
}

////////////////////////////////////////////////////////////////////////////////////
void CSignalingTask::AddRemoveAAInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "CSignalingTask::AddRemoveAAInd";

	WORD error_code, operation;
	*pSeg >> error_code;
	*pSeg >> operation;

	TRACESTR(eLevelInfoNormal) << "CSignalingTask::AddRemoveAAInd - error_code = "<<error_code<<" operation = "<<operation;

	switch (operation)
	{
		case eUSerMsgOperation_add:
			AddCSActiveAlarmSingleton(AA_EXTERNAL_ALERT_CS, error_code, "Lets hope it is working...");
			break;
		case eUSerMsgOperation_remove:
			RemoveCSActiveAlarm(AA_EXTERNAL_ALERT_CS, error_code);
			break;
		default:
			TRACESTR(eLevelInfoNormal) << "CSignalingTask::AddRemoveAAInd - default";
	}

}
////////////////////////////////////////////////////////////////////////////////////
void CSignalingTask::OnCsMngrSipServerTypeInd(CSegment* pSeg)
{
	WORD serverType;
	*pSeg >> serverType;

	SendSipServerTypeToCS(serverType);
}
//////////////////////////////////////////////////////////////////////////////
DWORD CSignalingTask::AddCSActiveAlarm(WORD errorCode,
                                       DWORD userID,
                                       const char* description)
{
    DWORD user_id = CreateIDFromCSID(errorCode, userID, m_csId);

    std::ostringstream info;
    info << "Error Code: " << errorCode
         << ", Error Name: " << GetAlarmName(errorCode)
         << ", CS ID: "      << m_csId
         << ", User ID: "    << user_id;

    PASSERTSTREAM_AND_RETURN_VALUE(
        IsActiveAlarmExistByErrorCodeUserId(errorCode, user_id),
        "Unable to add: active alarm already exists, " << info.str(),
        0xFFFFFFFF);

    std::ostringstream msg;
    msg << description << " (cs id " << m_csId <<")";

    DWORD id = AddActiveAlarm(FAULT_GENERAL_SUBJECT,
                              errorCode,
                              MAJOR_ERROR_LEVEL,
                              msg.str(),
                              true, true,
                              user_id);

    PASSERTSTREAM_AND_RETURN_VALUE(
        0xFFFFFFFF == id,
        "Unable to add active alarm, " << info.str(),
        0xFFFFFFFF);

    TRACEINTOFUNC << "Added active alarm, " << info.str();

    return id;
}

//////////////////////////////////////////////////////////////////////////////
void CSignalingTask::RemoveCSActiveAlarm(WORD errorCode,
                                         DWORD userID)
{
    DWORD user_id = CreateIDFromCSID(errorCode, userID, m_csId);

    if (IsActiveAlarmExistByErrorCodeUserId(errorCode, user_id))
    {
        std::ostringstream info;
        info << "Error Code: " << errorCode
             << ", Error Name: " << GetAlarmName(errorCode)
             << ", CS ID: "      << m_csId
             << ", User ID: "    << user_id;

    	TRACEINTO << "Removed active alarm, " << info.str();
    }

    RemoveActiveAlarmByErrorCodeUserId(errorCode, user_id);
}

//////////////////////////////////////////////////////////////////////
// static
DWORD CSignalingTask::CreateIDFromCSID(WORD errorCode, DWORD userID, DWORD csID)
{
    return (10000 * csID) + (100 * errorCode) + userID;
}

//////////////////////////////////////////////////////////////////////
DWORD CSignalingTask::AddCSActiveAlarmSingleton(DWORD errorCode, DWORD userId, const string &description)
{
	bool isExist = IsActiveAlarmExistByErrorCode(errorCode);
	if(true == isExist)
	{
		return 0xFFFFFFFF;
	}

	DWORD id = AddCSActiveAlarm(errorCode, userId, description.c_str());

    return id;
}


////////////////////////////////////////////////////////////////////////////////////
void CSignalingTask::SendSipServerTypeToCS(int serverType)
{
	CSmallString logStr = "CSignalingTask::SendSipServerTypeToCS";

	string cs_data;
	string mcms_data;

	if (serverType == eSipServer_ms )
	{
	    mcms_data = "CS_MS_ENVIRONMENT";
	    cs_data = "YES";
	}
	else
	{
	    mcms_data = "GENERIC";
	    cs_data = "NO";
	}

	CSysConfig *sysConfig = m_pProcess->GetSysConfig();

	const char *currentSectionName = GetCfgSectionName(eCfgSectionCSModule);

	sysConfig->OverWriteParam("CS_MS_ENVIRONMENT", cs_data, currentSectionName);

	const CCfgData *cfgData = sysConfig->GetCfgEntryByKey("CS_MS_ENVIRONMENT");

	if( false == CCfgData::TestValidity(cfgData) )
	{
		logStr << "failed to validate sysConfig, key: " << "CS_MS_ENVIRONMENT";
		TRACESTR(eLevelInfoNormal) << logStr.GetString();

		return;
	}

	logStr << " change SipServer type to: " << mcms_data.c_str();

	STATUS sendStat = m_CommStartupService->SendConfigParamSingle(cfgData);

	if (STATUS_OK != sendStat)
	{
		logStr << "\nFailed sending to CS! Status: " << sendStat;
	}

	TRACESTR(eLevelInfoNormal) << logStr.GetString();
}
////////////////////////////////////////////////////////////
#define BOOL_TO_STR(val,target,index) if(val){target[index] = "YES";}else {target[index]="NO";}
void CSignalingTask::SendSecurityPKIToCs(CSegment* pSeg)
{

	CLargeString logStr = "CSignalingTask::SendSecurityPKIToCs";

	string cfgKeys[] = {"REQUEST_PEER_CERTIFICATE","OCSP_GLOBAL_RESPONDER_URI","USE_RESPONDER_OCSP_URI",
						"ALLOW_INCOMPLETE_REV_CHECK",
						"SKIP_VALIDATE_OCSP_CERT",
						"REVOCATION_METHOD"};

	int SIZE_PKI_DATA=6;
	string cs_data[SIZE_PKI_DATA];
	CIPServiceList *pIpServListDynamic = m_pProcess->GetIpServiceListDynamic();
	CIPService *pService = pIpServListDynamic->GetService(m_csId);
	if(pService)
	{
		CManagementSecurity* pPKICfg = pService->GetManagementSecurity();

		BOOL_TO_STR(pPKICfg->IsRequestPeerCertificate(),cs_data,0);
		if(pPKICfg->GetOCSPGlobalResponderURI().empty())
		{
			cs_data[1] = "none";
		}
		else
		{
			cs_data[1] = pPKICfg->GetOCSPGlobalResponderURI();
		}		
		BOOL_TO_STR(pPKICfg->IsUseResponderOcspUri(),cs_data,2);
		BOOL_TO_STR(pPKICfg->IsIncompleteRevocation(),cs_data,3);
		BOOL_TO_STR(pPKICfg->IsSkipValidationOcspCert(),cs_data,4);

		switch(pPKICfg->getRevocationMethodType())
			{
				case eNoneMethod:   cs_data[5]="none";
									break;
				case eCrl:          cs_data[5]= "crl";
									break;
				case eOcsp:			cs_data[5]="ocsp";
									break;
			}

		CSysConfig *sysConfig = m_pProcess->GetSysConfig();
		const char *currentSectionName = GetCfgSectionName(eCfgSectionCSModule);
		for(int i=0;i<SIZE_PKI_DATA;i++)
		{
			sysConfig->OverWriteParam(cfgKeys[i], cs_data[i], currentSectionName);
			logStr << " Key =" << cfgKeys[i]  << " ,Value =" <<cs_data[i] << " \n ";
			const CCfgData *cfgData = sysConfig->GetCfgEntryByKey(cfgKeys[i]);
			if( false == CCfgData::TestValidity(cfgData) )
			{
				logStr << "failed to validate sysConfig, key: " << cfgKeys[i];
				TRACESTR(eLevelInfoNormal) << logStr.GetString();
						return;
			}
			STATUS sendStat = m_CommStartupService->SendConfigParamSingle(cfgData);

			if (STATUS_OK != sendStat)
			{
				logStr << "\nFailed sending to CS! Status: " << sendStat;
			}
		}
		TRACESTR(eLevelInfoNormal) << logStr.GetString();
	}

}



BOOL CSignalingTask::IsStartupFinished() const
{
  eMcuState systemState = CProcessBase::GetProcess()->GetSystemState();
  if (eMcuState_Invalid == systemState || eMcuState_Startup == systemState)
    return FALSE;

  return TRUE;
}
STATUS CSignalingTask::SendCsNewIndicationToProxyService()
{
	STATUS status = STATUS_OK;
	eProductType curProductType=CProcessBase::GetProcess()->GetProductType();
	
	status = m_CommProxyService->SendCsNewIndication(m_csId);
	
	return status;
}

BOOL CSignalingTask::SendIpServicesTCDumpParams()
{
	m_CommTCPDumpService->SetIsConnected(true);
	if ( m_CommTCPDumpService->SendIpServiceList() != STATUS_OK )
	{
		TRACEINTOFUNC << "Failed SendIpServiceList";

		return FALSE;
	}
	TRACEINTOFUNC << "CSignalingTask::SendIpServicesTCDumpParams: After SendIpServicesTCPDumpParams";
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////
/*STATUS CSignalingTask::HandleTerminalAddCsAA(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: Number and isExternal must be specified\n";
		answer << "usage: Bin/McuCmd add_aa [destination] [Number of AA] [isExternal: YES/NO]\n";
		return STATUS_FAIL;
	}

	const string &strNum     = command.GetToken(eCmdParam1);

	int numOfAAs = atoi(strNum.c_str());

	if (numOfAAs == 2)
		RemoveCSActiveAlarm(AA_EXTERNAL_ALERT_CS, SipTLS_RegistrationServerNotResponding);
	else
		AddCSActiveAlarm(AA_EXTERNAL_ALERT_CS, SipTLS_RegistrationServerNotResponding, "Lets hope it is working...");
}*/

////////////////////////////////////////////////////////////////////////////////////
void CSignalingTask::SendMcuVersionIdToCs()
{
	CSmallString logStr = "CSignalingTask::SendMcuVersionIdToCs";

	char mcuVer[MCU_VERSION_ARRAY_LEN] ;
	memset(mcuVer,0,MCU_VERSION_ARRAY_LEN);

	VERSION_S ver = m_pProcess->GetMcuVersion();

//	TRACEINTO << ver.ver_major  << "." << ver.ver_minor << "." << ver.ver_release;

	sprintf(mcuVer, "%d.%d.%d",ver.ver_major, ver.ver_minor, ver.ver_release);

	CSysConfig *sysConfig = m_pProcess->GetSysConfig();

	const char *currentSectionName = GetCfgSectionName(eCfgSectionCSModule);

	sysConfig->OverWriteParam(CFG_KEY_MCU_VERSION_ID, mcuVer, currentSectionName);

	const CCfgData *cfgData = sysConfig->GetCfgEntryByKey(CFG_KEY_MCU_VERSION_ID);

	if( false == CCfgData::TestValidity(cfgData) )
	{
		logStr << "failed to validate sysConfig, key: " << CFG_KEY_MCU_VERSION_ID;
		TRACESTR(eLevelInfoNormal) << logStr.GetString();

		return;
	}

	cfgData = sysConfig->GetCfgEntryByKey(CFG_KEY_MCU_VERSION_ID);
	logStr << " change MCU version ID to: " << cfgData->GetData().c_str();

	STATUS sendStat = m_CommStartupService->SendConfigParamSingle(cfgData);

	if (STATUS_OK != sendStat)
	{
		logStr << "\nFailed sending to CS! Status: " << sendStat;
	}

	TRACESTR(eLevelInfoNormal) << logStr.GetString();
}
