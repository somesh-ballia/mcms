// CSMngrManager.cpp 

#include "CSMngrManager.h"

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "OpcodesMcmsInternal.h"
#include "Request.h"
#include "CIPServiceDel.h"
#include "CommCardService.h"
#include "CommConfService.h"
#include "CommMcuService.h"
#include "CommStartupService.h"
#include "CommRsrcService.h"
#include "CommProxyService.h"
#include "CommIceService.h"
#include "CommGKService.h"
#include "CommDnsAgentService.h"
#include "CommSnmpService.h"
#include "CommCertMngrService.h"
#include "CommMcuMngrService.h"
#include "SysConfigKeys.h"
#include "DummyEntry.h"
#include "TraceStream.h"
#include "HlogApi.h"
#include "TerminalCommand.h"
#include "IpServiceValidator.h"
#include "CSMngrMessageValidator.h"
#include "WrappersResource.h"
#include "ConfigManagerApi.h"
#include "IpParameters.h"
#include "SignalingApi.h"
#include "SignalingTask.h"
#include "CsPinger.h"
#include "IceCmInd.h"
#include "IceCmReq.h"
#include "AlarmStrTable.h"
#include "MultipleServicesFunctions.h"
#include "IpCsOpcodes.h"
#include "CSMngrMplMcmsProtocolTracer.h"
#include "CSMngrStatuses.h"
#include "McmsDaemonApi.h"
#include "EncodeHelper.h"
#include "FipsMode.h"
#include "SipProxyTaskApi.h"
#include "IceTaskApi.h"
#include "CommTCPDumpService.h"
#include "DefinesIpServiceStrings.h"
#include "OsFileIF.h"
//for gesher
#include "RvgwSslPorts.h"
#include "ConfigHelper.h"
#include "SysConfigEma.h"
//end

#define BAD_CHARACTERS_FOR_URI " \"<>@'%:\\"
#define V35_GATEWAY_PASSWORDS_FILE_PATH   "Cfg/V35GWatewayPasswords1.txt"
#define CSMNGR_GET_SERVICE_INFO_TIMER_TIME_OUT_VALUE 2*SECOND
#define	DEFAULT_CS_CHECK_CS_IP_CONFIG_PERIOD  		SECOND*60


const DWORD ACTUAL_NUM_OF_SPANS_IN_SERVICE = 5; // for Call Generator - net_config.sh
CCSMngrProcess* pCSMngrProcess = NULL;

extern "C"void CsDispatcherEntryPoint(void* appParam);
extern char* CsCompTypeToString(compTypes compType);
extern char* IpTypeToString(APIU32 ipType, bool caps = false);
extern char
		* IpV6ConfigurationTypeToString(APIU32 configType, bool caps = false);
extern bool IsJitcAndNetSeparation();
extern std::string GetIpServiceTmpFileName();
extern STATUS ConfigDnsInOS(CIPService* pService, const string theCaller);
extern void CSMngrMonitorEntryPoint(void* appParam);

PBEGIN_MESSAGE_MAP( CCSMngrManager)
  ONEVENT(CSAPI_MSG, ANYCASE, CCSMngrManager::OnCSApi_Msg)
  ONEVENT(CS_NEW_IND, ANYCASE, CCSMngrManager::OnCsNewInd)
  ONEVENT(MCUMNGR_TO_CSMNGR_END_IP_CONFIG_IND, ANYCASE, CCSMngrManager::OnMcuMngrEndIpConfigInd)
  ONEVENT(MCUMNGR_TO_CSMNGR_MULTIPLE_SERVICES_IND, ANYCASE, CCSMngrManager::OnMultipleServicesInd)
  ONEVENT(MCUMNGR_CSMNGR_V35GW_PARAMS_REQ, ANYCASE, CCSMngrManager::OnMcuMngrV35GwParamsReq)
  ONEVENT(CS_CARDS_MEDIA_IP_PARAMS_REQ, ANYCASE, CCSMngrManager::OnCsCardsMediaIpParamReq)
  ONEVENT(CS_CARDS_MEDIA_IP_CONFIG_IND, ANYCASE, CCSMngrManager::OnCsCardsMediaIpConfigInd)
  ONEVENT(CS_ETH_SETTING_IND, ANYCASE, CCSMngrManager::OnCsCardsEthernetSettingInd)
  ONEVENT(CS_CONF_IP_SERVICE_PARAM_REQ, ANYCASE, CCSMngrManager::OnCsConfIpServiceParamReq)
  ONEVENT(CS_RSRC_IP_SERVICE_PARAM_REQ, ANYCASE, CCSMngrManager::OnCsRcrsIpServiceParamReq)
  ONEVENT(CS_RSRC_UDP_PORT_RANGE_IND, ANYCASE, CCSMngrManager::OnCsRcrsUdpPortRangeInd)
  ONEVENT(CS_RSRC_CFS_IND, ANYCASE, CCSMngrManager::OnCFSInd)
  ONEVENT(CS_PROXY_IP_SERVICE_PARAM_REQ, ANYCASE, CCSMngrManager::OnCsProxyIpServiceParamReqReady)
  ONEVENT(ICE_IP_SERVICE_PARAM_REQ, ANYCASE, CCSMngrManager::OnIceIpServiceParamReqReady)
  ONEVENT(CS_PROXY_REGISTRAR_STATUS_UPDATE_IND, ANYCASE, CCSMngrManager::OnCsProxyRegistrarStatus)
  ONEVENT(CS_GKMNGR_IP_SERVICE_PARAM_REQ, ANYCASE, CCSMngrManager::OnCsGKIpServiceParamReq)
  ONEVENT(CS_GKMNGR_UPDATE_SERVICE_PROPERTIES_REQ, ANYCASE, CCSMngrManager::OnCsGKIpServiceUpdatePropertiesReq)
  ONEVENT(CS_GKMNGR_CLEAR_GK_PARAMS_FROM_PROPERTIES_REQ, ANYCASE, CCSMngrManager::OnCsGKClearAltPropertiesReq)
  ONEVENT(CS_GKMNGR_SET_GK_IP_IN_PROPERTIES_REQ, ANYCASE, CCSMngrManager::OnCsGKIpInPropertiesReq)
  ONEVENT(CS_GKMNGR_SET_GK_ID_IN_PROPERTIES_REQ, ANYCASE, CCSMngrManager::OnCsGKIdInPropertiesReq)
  ONEVENT(CS_GKMNGR_SET_GK_NAME_IN_PROPERTIES_REQ, ANYCASE, CCSMngrManager::OnCsGKNameInPropertiesReq)
  ONEVENT(CS_DNS_AGENT_RESOLVE_IND, ANYCASE, CCSMngrManager::OnDnsResolveInd)
  ONEVENT(SNMP_CS_INTERFACE_IP_REQ, ANYCASE, CCSMngrManager::OnSnmpInterfaceReq)
  ONEVENT(CONF_BLOCK_IND, ANYCASE, CCSMngrManager::OnConfBlockInd)
  ONEVENT(CS_PING_IND, ANYCASE, CCSMngrManager::OnCsPingInd)
  ONEVENT(EP_PROCESS_STARTED, ANYCASE, CCSMngrManager::OnEPProcessStarted)
  ONEVENT(CSMNGR_CERTMNGR_IP_SERVICE_PARAM_REQ, ANYCASE, CCSMngrManager::OnCsCertMngrIpServiceParamReq)
  ONEVENT(CSMNGR_MCUMNGR_IP_SERVICE_PARAM_REQ, ANYCASE, CCSMngrManager::OnCsMcuMngrIpServiceParamReq)
  ONEVENT(CARDS_TO_CS_ICE_DETAILS, ANYCASE, CCSMngrManager::OnCardsToCsIceDetails)
  ONEVENT(CARDS_TO_CS_ICE_DETAILS_UPDATE_STATUS, ANYCASE, CCSMngrManager::OnCardsToCsIceDetailsUpdateStatus)
  ONEVENT(FAILOVER_CSMNGR_UPDATE_SERVICE_IND, ANYCASE, CCSMngrManager::OnFailoverUpdateServiceInd)
  ONEVENT(CS_TCPDUMP_IP_SERVICE_PARAM_REQ, ANYCASE,  CCSMngrManager::OnCsTCPDumpIpServiceParamReq)
  ONEVENT(UTILITY_INTERFACES_CONFIGURATION_REQ, ANYCASE, CCSMngrManager::OnInterfaceConfigurationReq)
  ONEVENT(CS_UTILITY_IS_UP_TIMER, ANYCASE, CCSMngrManager::OnTimerSendIpServiceToUtilityProcess)
  ONEVENT(CHECK_ETH2_CS_TIMER, ANYCASE, CCSMngrManager::OnCheckEth2CS)
  ONEVENT(FIRE_CHECK_ETH2_CS, ANYCASE, CCSMngrManager::OnFireEth2CS)
  
  ONEVENT(MCUMNGR_PRECEDENCE_SETTINGS,	ANYCASE, CCSMngrManager::OnMcuMngrPrecedenceSettings)

  ONEVENT(CS_CARDS_MS_CS_IP_CONFIG_END	,ANYCASE		,CCSMngrManager::OnCSIPConfigEndMSPerService )

  //SNMP
  ONEVENT(SNMP_CONFIG_TO_OTHER_PROGRESS,          ANYCASE, CCSMngrManager::OnSNMPConfigInd)
  ONEVENT(SIGNALINGTASK_TO_CSMNGR_SERVICE_UP_IND, ANYCASE, CCSMngrManager::OnSignalingServiceUpInd)
  ONEVENT(CSMNGR_GET_SERVICE_INFO_TIMER,          ANYCASE, CCSMngrManager::OnTimerGetServiceInfoInd)
  ONEVENT(MCUMNGR_TO_ALL_PROCESSES_STATE_REQ	, ANYCASE, CCSMngrManager::CSMngrCalculateSetProcessState )
  ONEVENT(MCUMNGR_TO_CSMNGR_IP_BECAME_VALID_IND	, ANYCASE, CCSMngrManager::CSMngrRestartStartupProcedure )

  ONEVENT(DNSAGENT_TO_CSMNGR_GET_IPCONFIG,        ANYCASE, CCSMngrManager::OnDnsAgentGetIpconfig)
PEND_MESSAGE_MAP(CCSMngrManager, CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY( CCSMngrManager)
  ON_TRANS("TRANS_IP_SERVICE", "NEW_IP_SERVICE", CIPService, CCSMngrManager::AddIpService)
  ON_TRANS("TRANS_IP_SERVICE", "DEL_IP_SERVICE", CIPServiceDel, CCSMngrManager::DeleteIpService)
  ON_TRANS("TRANS_IP_SERVICE", "UPDATE_IP_SERVICE", CIPService, CCSMngrManager::UpdateIpService)
  ON_TRANS("TRANS_IP_SERVICE", "SET_DEFAULT_H323_SERVICE", CIPServiceDel, CCSMngrManager::SetDefaultIPService)
  ON_TRANS("TRANS_IP_SERVICE", "SET_DEFAULT_SIP_SERVICE", CIPServiceDel, CCSMngrManager::SetDefaultSIPService)
  ON_TRANS("TRANS_MCU", "SET_PING", CPingSet, CCSMngrManager::SetPing)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CCSMngrManager)
  ONCOMMAND("bombi", CCSMngrManager::HandleTerminalBombi,
		"send a lot of messages to all the cs's")
  ONCOMMAND("bombiFor1", CCSMngrManager::HandleTerminalBombiFor1,
		"send a lot of messages to 1 specific cs, usage: bombiFor1 [number_of_messages] [csId]")
  ONCOMMAND("ping4", CCSMngrManager::HandleTerminalPing4,
		"ipv4 Ping to CS, usage: ping4 [Destination] [addr] [csId]")
  ONCOMMAND("ping6", CCSMngrManager::HandleTerminalPing6,
		"ipv6 Ping to CS, usage: ping6 [Destination] [addr] [csId]")
  // for Call Generator - net_config.sh
  ONCOMMAND("update_ip_service_configuration", CCSMngrManager::HandleTerminalUpdateIPServiceConfiguration,
		"Update the network configuration - IP, MASK and Default GW")
  ONCOMMAND("active_alarm", CCSMngrManager::HandleTerminalActiveAlarm,
		"simulate add/remove active alarm")
  ONCOMMAND("enc", CCSMngrManager::HandleTerminalEnc, "test enc")
END_TERMINAL_COMMANDS

void CSMngrManagerEntryPoint(void* appParam)
{
	CCSMngrManager* pCSMngrManager = new CCSMngrManager;
	pCSMngrManager->Create(*(CSegment*)appParam);
}

TaskEntryPoint CCSMngrManager::GetMonitorEntryPoint() {
	return CSMngrMonitorEntryPoint;
}

CCSMngrManager::CCSMngrManager() {
	m_CommDnsAgentService = NULL;
	m_CommCardService = NULL;
	m_CommConfService = NULL;
	m_CommMcuService = NULL;
	m_CommRcrsService = NULL;
	m_CommProxyService = NULL;
	m_CommIceService = NULL;
	m_CommGKService = NULL;
	m_CommSnmpService = NULL;
	m_CommCertMngrService = NULL;
	m_CommMcuMngrService = NULL;
	m_CommTCPDumpService = NULL;
	m_CSMngrMplMcmsProtocolTracer = NULL;
	m_CSMngrMessageValidator = NULL;
	m_precedentSetting      = NULL;
	pCSMngrProcess = dynamic_cast<CCSMngrProcess*> (CProcessBase::GetProcess());

	m_StatusReadIpServiceList = STATUS_OK;

	m_flagIsMediaIpConfigReceived = false;
	m_flagIsIpTypeReceived = false;
	m_flagIsIpListSentToCardsAfterMediaIpConfigReceived = false;
	m_flagIsRsrcAskedForIpServiceList = false;
	m_flagIsTCPDumpAskedForIpServiceList = false;
	m_sentMngmtTCPDump = false;

	for (int i = 0; i < NUM_OF_PROCESSES_TO_SEND_IP_LIST; i++) {
		m_ipListAlreadySent[i] = false;
	}

	m_sysIpType = eIpType_IpV4;
	m_sysV6ConfigurationType = eV6Configuration_Auto;

	m_pCsPinger = new CCsPinger(this);
	m_pCsPinger->SetProcess(pCSMngrProcess);

	for (int i = 0; i < MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
		m_flagAreIpOk[i] = false;

	m_mcuMngrIpTypeIndAlreadyReceived = false;
	m_bSNMPEnabled = FALSE;	
	m_isEth2Up = TRUE;
	m_firedEth2CS = FALSE;

}

CCSMngrManager::~CCSMngrManager() {
	FreeHeapMemory();
}

void CCSMngrManager::SelfKill() {
	// ===== 1. killing dispatcher (to avoid getting messages to MFAs after they already dead)
	if (m_pDispatcherApi) {
		m_pDispatcherApi->SyncDestroy();
		POBJDELETE(m_pDispatcherApi);
	}

	// ===== 2. killing Mfa tasks
	int csIdx = 0;
	for (csIdx = 0; csIdx < MAX_NUMBER_OF_SERVICES_IN_RMX_4000; csIdx++) {
		COsQueue* signalingMbx = pCSMngrProcess->GetSignalingMbx(csIdx);

		if (signalingMbx) {
			CTaskApi api;
			api.CreateOnlyApi(*signalingMbx);
			api.SendOpcodeMsg(DESTROY);
		}
	}

	CManagerTask::SelfKill();
}

void CCSMngrManager::CreateDispatcher() {
	m_pDispatcherApi = new CTaskApi;
	CreateTask(m_pDispatcherApi, CsDispatcherEntryPoint, m_pRcvMbx);
}

void CCSMngrManager::AllocHeapMemory() {
	m_CSMngrMplMcmsProtocolTracer = new CCSMngrMplMcmsProtocolTracer;
	m_CSMngrMessageValidator = new CCSMngrMessageValidator;

	m_CommStartupService = new CCommStartupService;
	m_CommStartupService->SetMplMcmsProtocolTracer(
			m_CSMngrMplMcmsProtocolTracer);

	m_CommDnsAgentService = new CCommDnsAgentService;
	m_CommCardService = new CCommCardService;
	m_CommConfService = new CCommConfService;
	m_CommMcuService = new CCommMcuService;
	m_CommRcrsService = new CCommRsrcService;
	m_CommProxyService = new CCommProxyService;
	m_CommIceService = new CCommIceService;
	m_CommGKService = new CCommGKService;
	m_CommSnmpService = new CCommSnmpService;
	m_CommCertMngrService = new CCommCertMngrService;
	m_CommMcuMngrService = new CCommMcuMngrService;
	m_CommTCPDumpService = new CCommTCPDumpService;
}

void CCSMngrManager::FreeHeapMemory() {
	POBJDELETE(m_CommStartupService);
	POBJDELETE(m_CommDnsAgentService);
	POBJDELETE(m_CommCardService);
	POBJDELETE(m_CommConfService);
	POBJDELETE(m_CommMcuService);
	POBJDELETE(m_CommRcrsService);
	POBJDELETE(m_CommProxyService);
	POBJDELETE(m_CommIceService);
	POBJDELETE(m_CommGKService);
	POBJDELETE(m_CommSnmpService);
	POBJDELETE(m_CSMngrMplMcmsProtocolTracer);
	POBJDELETE(m_CSMngrMessageValidator);
	POBJDELETE(m_CommCertMngrService);
	POBJDELETE(m_CommMcuMngrService);
	POBJDELETE(m_pCsPinger);
	POBJDELETE(m_CommTCPDumpService);
}

void CCSMngrManager::AddFilterOpcodePoint() {
	AddFilterOpcodeToQueue(CSAPI_MSG);
}

void CCSMngrManager::DeclareStartupConditions() {
	// TODO - Jud. Need to add AA according to csId (David)
	CActiveAlarm aa1(FAULT_GENERAL_SUBJECT, CS_STARTUP_FAILED,
			MAJOR_ERROR_LEVEL,
			"No cs_new_indication received - CS did not start", true, true);
	AddStartupCondition(aa1);

	CActiveAlarm aa2(FAULT_GENERAL_SUBJECT, AA_NO_LICENSING, MAJOR_ERROR_LEVEL,
			"Licensing was not received from McuMngr", false, false);
	AddStartupCondition(aa2);

	CActiveAlarm aa5(FAULT_GENERAL_SUBJECT, NO_SERVICE_FOUND_IN_DB,
			MAJOR_ERROR_LEVEL, "No Services in DB", false, false);
	AddStartupCondition(aa5);

	AddStartupCondDependency(CS_STARTUP_FAILED, AA_NO_LICENSING);
	AddStartupCondDependency(AA_NO_LICENSING, NO_SERVICE_FOUND_IN_DB);
}


eIpType CCSMngrManager::GetSysIpTypeForSim() const {
	if (!IsTarget()) {
		std::string fname = MCU_TMP_DIR+"/SysIpTypeForSim";
		FILE* pSysIpTypeForSimFile = fopen(fname.c_str(), "r");
		if (pSysIpTypeForSimFile) {
			TRACEINTO
					<< "\nCCSMngrManager::GetSysIpTypeForSim - "+MCU_TMP_DIR+"/SysIpTypeForSim exists!";

			char* line = NULL;
			size_t len = 0;
			ssize_t read;
			read = getline(&line, &len, pSysIpTypeForSimFile);
			if (read != -1) {
				if (line == NULL)
					return eIpType_None;

				if (strcmp(line, "IpV4") == 0)
					return eIpType_IpV4;

				if (strcmp(line, "IpV6") == 0)
					return eIpType_IpV6;

				if (strcmp(line, "Both") == 0)
					return eIpType_Both;
			}

			if (line)
				free(line);

			fclose(pSysIpTypeForSimFile);
		}
	}

	return eIpType_None;
}

void CCSMngrManager::SetMngmntIpParamsInProcess(eIpType ipType,
                                                eV6ConfigurationType
                                                ipv6ConfigType,
                                                DWORD ipV4Add,
                                                std::string ipv6Add_0,
                                                std::string ipv6Add_1,
                                                std::string ipv6Add_2,
                                                std::string defGwIpv6,
                                                DWORD defGwIpv6Mask,
                                                char ipAddressStr[IP_ADDRESS_LEN],
                                                BOOL dnsStatus,
                                                char ipv6_address[IPV6_ADDRESS_LEN])
{
  pCSMngrProcess->SetSysIpType(ipType);
  pCSMngrProcess->SetSysIPv6ConfigType(ipv6ConfigType);
  pCSMngrProcess->SetMngmntAddress_IPv4(ipV4Add);
  pCSMngrProcess->SetMngmntAddress_IPv6(0, ipv6Add_0);
  pCSMngrProcess->SetMngmntAddress_IPv6(1, ipv6Add_1);
  pCSMngrProcess->SetMngmntAddress_IPv6(2, ipv6Add_2);
  pCSMngrProcess->SetMngmntDefaultGatewayIPv6(defGwIpv6);
  pCSMngrProcess->SetMngmntDefaultGatewayMaskIPv6(defGwIpv6Mask);
  pCSMngrProcess->SetMngmntDnsIpV4Address(ipAddressStr);
  pCSMngrProcess->SetMngmntDnsStatus(dnsStatus);
  pCSMngrProcess->SetMngmntDnsIpV6Address(ipv6_address);
}

STATUS CCSMngrManager::OnMcuMngrEndIpConfigInd(CSegment* pSeg) {
	TRACEINTO << "CCSMngrManager::OnMcuMngrEndIpConfigInd";

	STATUS status = STATUS_OK;

	CLargeString errorMsg;

	CIPServiceList* pList = NULL;
	CIPService* pService = NULL;

	pList = pCSMngrProcess->GetIpServiceListDynamic();
	if (pList) {
		pService = pList->GetFirstService();
		while (pService) // check all the services
		{
			eIpType ipType = pService->GetIpType();
			if (ipType != eIpType_IpV6) // no need to check v4 duplication in ipv6_Only.
				status = pCSMngrProcess->ValidateDuplicateIpAddrCS(pService,
						errorMsg);

			bool areIpOk = (STATUS_OK == status);

			// TBD - Jud - David needs to treat this AA with the specific service Id
			if (areIpOk)
				RemoveActiveAlarmByErrorCode(DUPLICATE_IP_CS_MNGMNT);
			else
				AddActiveAlarmSingleton(
						FAULT_GENERAL_SUBJECT,
						DUPLICATE_IP_CS_MNGMNT,
						MAJOR_ERROR_LEVEL,
						"Duplicate IP (Central Signaling and Management Network Service)",
						true, true);

			m_flagAreIpOk[pService->GetId() - 1] = areIpOk;

			TRACEINTO << "\nCCSMngrManager::OnMcuMngrEndIpConfigInd"
					<< std::endl << "For CSId: " << pService->GetId()
					<< "; Duplication Check : " << (areIpOk ? "OK" : "FAIL");

			COsQueue* signalingMbx = pCSMngrProcess->GetSignalingMbx(
					pService->GetId() - 1);

			if (areIpOk && signalingMbx) {
				TRACEINTO << "\nCSMNGR_END_IP_CONFIG_IND - sending";
				CSignalingApi api;
				api.CreateOnlyApi(*signalingMbx);
				api.SendOpcodeMsg(CSMNGR_END_IP_CONFIG_IND);
			} else if (signalingMbx == NULL)
				TRACEINTO << "\nCCSMngrManager::OnMcuMngrEndIpConfigInd"
						<< endl << "Failed to find SignalingTask for csId: "
						<< pService->GetId()
						<< "Maybe the task didn't start yet...";

			pService = pList->GetNextService();
		}
	}

	return STATUS_OK;
}

void CCSMngrManager::OnMultipleServicesInd(CSegment* pParam) {
	BYTE isMultipleServices = FALSE;
	*pParam >> isMultipleServices;

	if (isMultipleServices == FALSE)

		pCSMngrProcess->SetIsMultipleServices(eFALSE);
	else
		pCSMngrProcess->SetIsMultipleServices(eTRUE);

	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::OnMultipleServicesInd"
			<< "\nMultiple services mode : " << (isMultipleServices ? "YES"
			: "NO");
}

void CCSMngrManager::OnMcuMngrV35GwParamsReq(CSegment* pParam)
{
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::OnMcuMngrV35GwParamsReq";

	CIPServiceList *pServicesList = pCSMngrProcess->GetIpServiceListStatic();
	if (!pServicesList)
		return;

	CIPService *pService = pServicesList->GetFirstService();
	if(pService)
		SendV35GwUpdateIndToMcuMngr(*pService);
    /*while(NULL != pService)
    {
    	if (pService->GetIsV35GwEnabled())
    		SendV35GwUpdateIndToMcuMngr(*pService);

    	pService = pServicesList->GetNextService();
    }*/
}

void CCSMngrManager::OnCSApi_Msg(CSegment* pSeg) {
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg, CS_API_TYPE);

	m_CSMngrMplMcmsProtocolTracer->SetData(pMplMcmsProtocol);
	m_CSMngrMplMcmsProtocolTracer->TraceMplMcmsProtocol(
			"CSMngr received message from CSApi", CS_API_TYPE);
	m_CSMngrMplMcmsProtocolTracer->SetData(NULL);

	OPCODE opcode = pMplMcmsProtocol->getOpcode();

	STATUS status = m_CSMngrMessageValidator->ValidateMessageLen(
			*pMplMcmsProtocol);
	if (STATUS_OK != status)
		return;

	pSeg->ResetRead();

	DispatchEvent(opcode, pSeg);
	PushMessageToQueue(opcode, pSeg->GetLen(), eProcessCSApi);

	POBJDELETE(pMplMcmsProtocol);
}

STATUS CCSMngrManager::OnCsNewInd(CSegment* pSeg) {
	TRACESTR(eLevelInfoHigh) << "\nCCSMngrManager::OnCsNewInd";

	// ===== 1. extract the mplMcmsProtocol from segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg, CS_API_TYPE);

	// validate data size
	STATUS sizeStat = pMplMcmsProtocol->ValidateDataSize(sizeof(CS_New_Ind_S));
	if (STATUS_OK != sizeStat) {
		delete pMplMcmsProtocol;
		return sizeStat;
	}

	// ===== 2. extract service id
	OPCODE opcode = pMplMcmsProtocol->getOpcode();
	WORD msgCsId = pMplMcmsProtocol->getCentralSignalingHeaderCsId();
	msgCsId = CCSMngrProcess::CheckAndFixCSID(msgCsId, opcode);

	TRACESTR(eLevelInfoHigh) << "\nOnCsNewInd - msgCsId = " << msgCsId;
	TRACESTR(eLevelInfoHigh) << __PRETTY_FUNCTION__ << ": " << "msgCsId = "
			<< msgCsId;

	// ===== 3. remove startup condition AA
	RemoveActiveAlarmByErrorCode(CS_STARTUP_FAILED);

	// Jud - TBD. if licensing is needed using multiple services flag
	//TODO: why license is not received! debug first
	//if (pCSMngrProcess->IsLicensingReceived())
	CreateSignalingTask(pMplMcmsProtocol->GetData(), msgCsId);

	delete pMplMcmsProtocol;

	return STATUS_OK;
}

void CCSMngrManager::CreateSignalingTask(char* data, WORD csId) {
	TRACESTR(eLevelInfoNormal)
			<< "\nCCSMngrManager::CreateSignalingTask; CsId: " << (int) csId;

	// ===== 1. if a task for this cs already exists, then send reset req to daemon. TBD when no reset option will be avaliable
	BOOL isAlreadyExist = IsTaskAlreadyExists(csId);

	if (isAlreadyExist)
		TRACESTR(eLevelInfoNormal)
				<< "\nCCSMngrManager::CreateSignalingTask - task already exists. Nothing to be done";
	else {
		// ===== 2. create the task
		CSignalingApi* pSignalingApi = new CSignalingApi;
		pSignalingApi->Create(signalingEntryPoint, *m_pRcvMbx);

		// ===== 3. update dispatcher's table
		CSignalingTask* pSignalingTask =
				(CSignalingTask*) pSignalingApi->GetTaskAppPtr();
		COsQueue& rcvMbx = pSignalingTask->GetRcvMbx();
		pCSMngrProcess->AddToSignalingTasksList(&rcvMbx, csId - 1);

		// ===== 4. update task's parametrers
		pSignalingTask->SetCsId(csId);
		pSignalingTask->SetCsNewIndData(data);
		pSignalingTask->SetTaskState(eTaskStateReady);
		pSignalingTask->SetTaskStatus(eTaskNormal);
		pSignalingTask->SetIsCsMngrEndIpConfigIndReceived(m_flagAreIpOk[csId
				- 1]);
		// pSignalingTask->StartStartupConfiguration();
		POBJDELETE(pSignalingApi);
	}
}

BOOL CCSMngrManager::IsTaskAlreadyExists(WORD csId) {
	BOOL isAlreadyExist = FALSE;

	COsQueue* pMbx = pCSMngrProcess->GetSignalingMbx(csId - 1);
	if (NULL != pMbx)
		isAlreadyExist = TRUE;

	return isAlreadyExist;
}

STATUS CCSMngrManager::OnCsCardsMediaIpConfigInd(CSegment* pSeg) 
{
    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();        
	if (curProductType == eProductTypeSoftMCUMfw)
	{
        TRACESTR(eLevelInfoHigh) << "CCSMngrManager::OnCsCardsMediaIpConfigInd, no need re-configured mediaIp.";
        return STATUS_OK;
	}
    
	int serviceId = -1;
	STATUS status = m_CommCardService->ReceiveMediaIpConfigInd(pSeg, serviceId);
	TRACESTR(eLevelInfoHigh) << "CCSMngrManager::OnCsCardsMediaIpConfigInd"
			<< "\nStatus: " << status;

	if (STATUS_OK == status) {
		m_flagIsMediaIpConfigReceived = true;

		eIpType ipType = pCSMngrProcess->GetSysIpType();
		eV6ConfigurationType configType =
				pCSMngrProcess->GetSysIPv6ConfigType();

		// ==== sending the address(es) to other processes
		// if CS_CARDS_MEDIA_IP_CONFIG_IND is received, it means that the card is up;
		// however it may occur that the card has re-evoked after going down.
		// In that case the addresses should be resent to the processes
		for (int i = 0; i < NUM_OF_PROCESSES_TO_SEND_IP_LIST; i++) {
			// (however the list should not be resent to Cards process,
			// since CS_CARDS_MEDIA_IP_CONFIG_IND itslef is received from Cards process...)
			// Judith - send only update to the resources. not all the list.

			if (eToSendIpList_Cards != i && eToSendIpList_Resource != i
					&& eToSendIpList_TCPDump != i)
				m_ipListAlreadySent[i] = false;
		}

		SendIpServiceListIfNeeded(serviceId, FALSE);
	}

	return status;
}

STATUS CCSMngrManager::OnCsCardsEthernetSettingInd(CSegment* pSeg) {
	if (NULL == pSeg) {
		TRACESTR(eLevelInfoHigh)
				<< "CCSMngrManager::OnCsCardsEthernetSettingInd(): Input segment is NULL";
		return STATUS_FAIL;
	}

	DWORD board_id;
	DWORD sub_board_id;
	DWORD linkStatus;

	*pSeg >> board_id >> sub_board_id >> linkStatus;

	TRACESTR(eLevelInfoHigh)
			<< "\nCCSMngrManager::OnCsCardsEthernetSettingInd() - board id = "
			<< board_id << " sub board id = " << sub_board_id
			<< " is loaded - board status = " << linkStatus;

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	int pos = board_id > 0 ? board_id - 1 : 0;
	if (sub_board_id == 2) // for the second port
		pos++;

	m_media_board_status[pos].board_id = board_id;
	m_media_board_status[pos].sub_board_id = sub_board_id;
	m_media_board_status[pos].link_status = linkStatus;

	pCSMngrProcess->GetServiceIdFromDynamicList(board_id, sub_board_id);

	return STATUS_OK;
}

/*
 * Judith: ipServiceIndex - we send the service index, in case we send specific service to the process -
 * at the moment, it happens only when updating ipv6, and sending the update to resources.
 * bSendAllTheList - here to solve the problem when switch is up after the process already requests the ip service list
 * and didn't get it from us.
 * For now - the service list will be send when the switch is up, and when the process request it (twice).
 * related to bug VNGR-13884
 *
 */
void CCSMngrManager::SendIpServiceListIfNeeded(const int serviceId,
		BOOL bSendIpServiceListAfterGettingIpType) {
	TRACESTR(eLevelInfoHigh) << "CCSMngrManager::SendIpServiceListIfNeeded";

	// just for avoiding the chance of a future exception
	// (currently the segment is not used in the following methods, so 'NULL' could have been sent)
	CSegment* pTmpSeg = new CSegment();

	if ((false == m_ipListAlreadySent[eToSendIpList_Cards]) && (false
			== m_flagIsIpListSentToCardsAfterMediaIpConfigReceived)) {
		OnCsCardsMediaIpParamReq(pTmpSeg);
		m_flagIsIpListSentToCardsAfterMediaIpConfigReceived = true;
		TRACESTR(eLevelInfoHigh)
				<< "CCSMngrManager::SendIpServiceListIfNeeded - IP Services list was sent to Cards process";
	}

	if (false == m_ipListAlreadySent[eToSendIpList_ConfParty]) {
		OnCsConfIpServiceParamReq(pTmpSeg);
		TRACESTR(eLevelInfoHigh)
				<< "CCSMngrManager::SendIpServiceListIfNeeded - IP Services list was sent to ConfParty process";
	}

	if (m_flagIsRsrcAskedForIpServiceList == true) {
		if (bSendIpServiceListAfterGettingIpType
				|| !m_ipListAlreadySent[eToSendIpList_Resource]) {
			OnCsRcrsIpServiceParamReq(pTmpSeg);
			TRACESTR(eLevelInfoHigh)
					<< "CCSMngrManager::SendIpServiceListIfNeeded - IP Services list was sent to Resource process";
		} else // the list was already sent to the resources - update is needed
		{
			m_CommRcrsService->SendUpdateIpV6ParamReq(serviceId);
			TRACESTR(eLevelInfoHigh)
					<< "CCSMngrManager::SendIpServiceListIfNeeded - specific IP Service was sent to Resource process";
		}
	}

	if (!m_ipListAlreadySent[eToSendIpList_Proxy]) {
		OnCsProxyIpServiceParamReq(pTmpSeg, "SendIpServiceListIfNeeded");
		TRACESTR(eLevelInfoHigh)
				<< "CCSMngrManager::SendIpServiceListIfNeeded - IP Services list was sent to SipProxy process";
	}

	if (!m_ipListAlreadySent[eToSendIpList_Ice]) {
		OnIceIpServiceParamReq(pTmpSeg, "SendIpServiceListIfNeeded");
		TRACESTR(eLevelInfoHigh)
				<< "CCSMngrManager::SendIpServiceListIfNeeded - IP Services list was sent to Ice process";
	}


	if (!m_ipListAlreadySent[eToSendIpList_Gk]) {
		OnCsGKIpServiceParamReq(pTmpSeg);
		TRACESTR(eLevelInfoHigh)
				<< "CCSMngrManager::SendIpServiceListIfNeeded - IP Services list was sent to Gatekeeper process";
	}

	if (!m_ipListAlreadySent[eToSendIpList_CertMngr]) {
		OnCsCertMngrIpServiceParamReq(pTmpSeg);
		TRACESTR(eLevelInfoHigh)
				<< "CCSMngrManager::SendIpServiceListIfNeeded - IP Services list was sent to CertMngr process";
	}

	if (false == m_ipListAlreadySent[eToSendIpList_McuMngr]) {
		OnCsMcuMngrIpServiceParamReq(pTmpSeg);
		TRACESTR(eLevelInfoHigh)
				<< "CCSMngrManager::SendIpServiceListIfNeeded - IP Services list was sent to McuMngr process";
	}

	// TODO(drabkin) Uncomment then Utility process will be delivered
	if (false == m_ipListAlreadySent[eToSendIpList_TCPDump]) {
		OnCsTCPDumpIpServiceParamReq(pTmpSeg);
		TRACESTR(eLevelInfoHigh)
				<< "CCSMngrManager::SendIpServiceListIfNeeded - IP Services list was sent to TCPDump process";
	}

	POBJDELETE(pTmpSeg);
}

STATUS CCSMngrManager::OnCsCardsMediaIpParamReq(CSegment* pSeg) {
	STATUS status = STATUS_OK;

	TRACESTR(eLevelInfoHigh) << "CCSMngrManager::OnCsCardsMediaIpParamReq";

	m_CommCardService->SetIsConnected(true);

	status = m_CommCardService->SendIpServiceList();
	CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
			"to send IPS LIST to Cards", status);
	StartTimer(CS_UTILITY_IS_UP_TIMER, 10 * SECOND);
	status = m_CommCardService->SendIpServiceParamEndInd();
	CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
			"to send END params to Cards", status);

	m_ipListAlreadySent[eToSendIpList_Cards] = true;

	return status;
}

STATUS CCSMngrManager::OnCsConfIpServiceParamReq(CSegment* pSeg) {
	STATUS status = STATUS_OK;

	TRACESTR(eLevelInfoHigh) << "CCSMngrManager::OnCsConfIpServiceParamReq";

	// in all following cases we already have the needed addresses (so they can be sent to other processes)
	if (!IsTarget()  
			|| (true == m_flagIsMediaIpConfigReceived) 
			|| ((true == m_flagIsIpTypeReceived) && ((eIpType_IpV4 == m_sysIpType)
				|| (eV6Configuration_Manual == m_sysV6ConfigurationType)))
			|| (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator)) {

			m_CommConfService->SetIsConnected(true);

			status = m_CommConfService->SendIpServiceList();
			CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
					"to send IPS LIST to Conf-Party", status);

			status = m_CommConfService->SendIpServiceParamEndInd();
			CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
					"to send END params to Conf-Party", status);

			m_ipListAlreadySent[eToSendIpList_ConfParty] = true;
		
	} else
		TRACESTR(eLevelInfoHigh) << "CCSMngrManager::OnCsConfIpServiceParamReq"
				<< "\nMediaIpConfigInd was not received yet (and it may be IPv6 Auto mode)";

	return status;
}

STATUS CCSMngrManager::OnCsRcrsIpServiceParamReq(CSegment* pSeg) {
	STATUS status = STATUS_OK;

	TRACESTR(eLevelInfoHigh) << "CCSMngrManager::OnCsRcrsIpServiceParamReq";
	// in all following cases we already have the needed addresses (so they can be sent to other processes)

	if ((!IsTarget()) || (true == m_flagIsMediaIpConfigReceived) || ((true
			== m_flagIsIpTypeReceived))
			|| (CProcessBase::GetProcess()->GetProductFamily()
					== eProductFamilyCallGenerator)) {
		m_CommRcrsService->SetIsConnected(true);

		status = m_CommRcrsService->SendIpServiceList();
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
				"to send IPS LIST to Rsrc", status);

		status = m_CommRcrsService->SendIpServiceParamEndInd();
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
				"to send END params to Rsrc", status);

		m_ipListAlreadySent[eToSendIpList_Resource] = true;

		TRACESTR(eLevelInfoHigh)
				<< "CCSMngrManager::OnCsRcrsIpServiceParamReq - send ip service list to resources";
	}

	else
		TRACESTR(eLevelInfoHigh) << "CCSMngrManager::OnCsRcrsIpServiceParamReq"
				<< "\nMediaIpConfigInd was not received yet (and it may be IPv6 Auto mode)";

	m_flagIsRsrcAskedForIpServiceList = true; // to know if the resources asked for the list for at least one time

	return status;
}

STATUS CCSMngrManager::OnCFSInd(CSegment* pSeg) {
	const RSRC_CFS_S* pParam = (const RSRC_CFS_S*) pSeg->GetPtr();
	pCSMngrProcess->SetLicensingMaxNumOfParties(pParam->MaxPartiesNum);

	TRACEINTO << "\n" << __PRETTY_FUNCTION__ << "\n" << CCFSWrapper(*pParam);

	RemoveActiveAlarmByErrorCode(AA_NO_LICENSING);

	return STATUS_OK;
}

STATUS CCSMngrManager::OnCsRcrsUdpPortRangeInd(CSegment* pSeg) {
	const UDP_PORT_RANGE_S* pParam = (const UDP_PORT_RANGE_S*) pSeg->GetPtr();

	TRACEINTO << "\n" << __PRETTY_FUNCTION__ << "\n" << CUDPPortRangeWrapper(
			*pParam);

	CIPServiceList* listDyn = pCSMngrProcess->GetIpServiceListDynamic();
	const WORD numUdpPorts = pParam->UdpLastPort - pParam->UdpFirstPort;
	listDyn->SetUdpPortRange(pParam->UdpFirstPort, numUdpPorts,
			pParam->ServiceId);

	CIPServiceList* listStatic = pCSMngrProcess->GetIpServiceListStatic();
	listStatic->SetUdpPortRange(pParam->UdpFirstPort, numUdpPorts,
			pParam->ServiceId);

	return STATUS_OK;
}

STATUS CCSMngrManager::OnCsProxyIpServiceParamReq(CSegment* pSeg,
		const string theCaller /*=""*/) {
	STATUS status = STATUS_OK;
	TRACEINTO << "CCSMngrManager::OnCsProxyIpServiceParamReq (caller: "
			<< theCaller << ")";

	// in all following cases we already have the needed addresses (so they can be sent to other processes)
	if ((!IsTarget()) || (true == m_flagIsMediaIpConfigReceived) || ((true
			== m_flagIsIpTypeReceived) && ((eIpType_IpV4 == m_sysIpType)
			|| (eV6Configuration_Manual == m_sysV6ConfigurationType)))) {
		m_CommProxyService->SetIsConnected(true);

		status = m_CommProxyService->SendIpServiceList();
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
				"to send IPS LIST to Proxy", status);

		status = m_CommProxyService->SendIpServiceParamEndInd();
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
				"to send END params to Proxy", status);

		m_ipListAlreadySent[eToSendIpList_Proxy] = true;
	}

	else
		TRACESTR(eLevelInfoHigh)
				<< "CCSMngrManager::OnCsProxyIpServiceParamReq"
				<< "\nMediaIpConfigInd was not received yet (and it may be IPv6 Auto mode)";

	return status;
}

STATUS CCSMngrManager::OnIceIpServiceParamReq(CSegment* pSeg,
		const string theCaller /*=""*/) {
	STATUS status = STATUS_OK;
	TRACEINTO << "CCSMngrManager::OnIceIpServiceParamReq (caller: "
			<< theCaller << ")";

	// in all following cases we already have the needed addresses (so they can be sent to other processes)
	if ((!IsTarget()) || (true == m_flagIsMediaIpConfigReceived) || ((true
			== m_flagIsIpTypeReceived) && ((eIpType_IpV4 == m_sysIpType)
			|| (eV6Configuration_Manual == m_sysV6ConfigurationType)))) {
		m_CommIceService->SetIsConnected(true);

		status = m_CommIceService->SendIpServiceList();
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
				"to send IPS LIST to Ice", status);

		status = m_CommIceService->SendIpServiceParamEndInd();
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
				"to send END params to Ice", status);

		m_ipListAlreadySent[eToSendIpList_Ice] = true;
	}

	else
		TRACESTR(eLevelInfoHigh)
				<< "CCSMngrManager::OnIceIpServiceParamReq"
				<< "\nMediaIpConfigInd was not received yet (and it may be IPv6 Auto mode)";

	return status;
}

STATUS CCSMngrManager::OnCsProxyIpServiceParamReqReady(CSegment* pSeg) {
	STATUS status = STATUS_OK;
	// in all following cases we already have the needed addresses (so they can be sent to other processes)
	if ((!IsTarget()) || (true == m_flagIsMediaIpConfigReceived) || ((true
			== m_flagIsIpTypeReceived) && ((eIpType_IpV4 == m_sysIpType)
			|| (eV6Configuration_Manual == m_sysV6ConfigurationType)))) {
		status = OnCsProxyIpServiceParamReq(pSeg,
				"OnCsProxyIpServiceParamReqReady");

		for (int i = 0; i < MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++) {
			COsQueue* queue = pCSMngrProcess->GetSignalingMbx(i);
			if (queue) {
				CSignalingApi api;
				api.CreateOnlyApi(*queue);
				api.SendOpcodeMsg(CSMNGR_PROXY_IP_SERVICE_PARAM_REQ);
			}
		}
	} else
		TRACESTR(eLevelInfoHigh)
				<< "CCSMngrManager::OnCsProxyIpServiceParamReqReady"
				<< "\nMediaIpConfigInd was not received yet (and it may be IPv6 Auto mode)";

	return status;
}

STATUS CCSMngrManager::OnIceIpServiceParamReqReady(CSegment* pSeg) {
	STATUS status = STATUS_OK;
	// in all following cases we already have the needed addresses (so they can be sent to other processes)
	if ((!IsTarget()) || (true == m_flagIsMediaIpConfigReceived) || ((true
			== m_flagIsIpTypeReceived) && ((eIpType_IpV4 == m_sysIpType)
			|| (eV6Configuration_Manual == m_sysV6ConfigurationType)))) {
		status = OnIceIpServiceParamReq(pSeg,
				"OnIceIpServiceParamReqReady");


	} else
		TRACESTR(eLevelInfoHigh)
				<< "CCSMngrManager::OnIceIpServiceParamReqReady"
				<< "\nMediaIpConfigInd was not received yet (and it may be IPv6 Auto mode)";

	return status;
}

STATUS CCSMngrManager::OnCsProxyRegistrarStatus(CSegment* pSeg) {
	const SIP_PROXY_STATUS_PARAMS_S* pData =
			(const SIP_PROXY_STATUS_PARAMS_S*) pSeg->GetPtr();
	STATUS status = m_CommProxyService->ReceiveSipProxyStatusInd(*pData);
	CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
			"to receive Proxy Status from SipProxy", status);
	return status;
}

STATUS CCSMngrManager::OnCsGKIpServiceParamReq(CSegment* pSeg) {
	STATUS status = STATUS_OK;

	// in all following cases we already have the needed addresses (so they can be sent to other processes)
	if ((!IsTarget()) || (true == m_flagIsMediaIpConfigReceived) || ((true
			== m_flagIsIpTypeReceived) && ((eIpType_IpV4 == m_sysIpType)
			|| (eV6Configuration_Manual == m_sysV6ConfigurationType)))) {
		m_CommGKService->SetIsConnected(true);

		status = m_CommGKService->SendIpServiceList();
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
				"to send IPS LIST to GateKeeper", status);

		m_ipListAlreadySent[eToSendIpList_Gk] = true;
	}

	else
		TRACESTR(eLevelInfoHigh) << "CCSMngrManager::OnCsGKIpServiceParamReq"
				<< "\nMediaIpConfigInd was not received yet (and it may be IPv6 Auto mode)";

	return status;
}

STATUS CCSMngrManager::OnCsGKIpServiceUpdatePropertiesReq(CSegment* pSeg) {
	const GkManagerUpdateServicePropertiesReqStruct* pParam =
			(const GkManagerUpdateServicePropertiesReqStruct*) pSeg->GetPtr();
	STATUS status = m_CommGKService->ReceiveUpdatePropertiesReq(*pParam);

	if (STATUS_OK == status) {
		CIPServiceList* list = pCSMngrProcess->GetIpServiceListDynamic();
		CIPService* pServiceDynamic = list->GetService(pParam->serviceId);
		if (NULL == pServiceDynamic) {
			PASSERTMSG(TRUE, "NULL == pServiceDynamic");
			return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
		}

		status = m_CommConfService->SendIpServiceParamInd(pServiceDynamic);
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
				"to send IPS to Conf-Party", status);

		status = m_CommSnmpService->SendIpServiceParamInd(pServiceDynamic);
		CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
				"to send IPS to SNMP process", status);
	}

	return status;
}

STATUS CCSMngrManager::OnCsGKClearAltPropertiesReq(CSegment* pSeg) {
	const ClearGkParamsFromPropertiesReqStruct* pParam =
			(const ClearGkParamsFromPropertiesReqStruct*) pSeg->GetPtr();
	STATUS status = m_CommGKService->ReceiveClearPropertiesReq(*pParam);

	if (status == STATUS_OK)
		SendSnmpCsInterface(pParam->serviceId);

	return status;
}

STATUS CCSMngrManager::OnCsGKIpInPropertiesReq(CSegment* pSeg) {
	const SetGkIPInPropertiesReqStruct* pParam =
			(const SetGkIPInPropertiesReqStruct*) pSeg->GetPtr();
	STATUS status = m_CommGKService->ReceiveIpInPropertiesReq(*pParam);

	if (status == STATUS_OK)
		SendSnmpCsInterface(pParam->serviceId);

	return status;
}

STATUS CCSMngrManager::OnCsGKIdInPropertiesReq(CSegment* pSeg) {
	const SetGkIdInPropertiesReqStruct* pParam =
			(const SetGkIdInPropertiesReqStruct*) pSeg->GetPtr();
	STATUS status = m_CommGKService->ReceiveIdInPropertiesReq(*pParam);

	if (status == STATUS_OK)
		SendSnmpCsInterface(pParam->serviceId);

	return status;
}

STATUS CCSMngrManager::OnCsGKNameInPropertiesReq(CSegment* pSeg) {
	const SetGkNameInPropertiesReqStruct* pParam =
			(const SetGkNameInPropertiesReqStruct*) pSeg->GetPtr();
	STATUS status = m_CommGKService->ReceiveNameInPropertiesReq(*pParam);

	if (status == STATUS_OK)
		SendSnmpCsInterface(pParam->serviceId);

	return status;
}

STATUS CCSMngrManager::OnDnsResolveInd(CSegment* pSeg) {
	const DNS_PARAMS_IP_S* pParam = (const DNS_PARAMS_IP_S*) pSeg->GetPtr();
	STATUS status = m_CommDnsAgentService->ReceiveDnsResolution(*pParam);
	CCSMngrProcess::TraceToLogger("CCSMngrManager::OnDnsResolveInd",
			"to receive DNS resolution from DnsAgent", status);
	return status;
}
static unsigned int rvgw_number(bool isV35MultipleServices)
{
  CProcessBase* proc = CProcessBase::GetProcess();
  FPASSERT_AND_RETURN_VALUE(NULL == proc, 2);
  if(!isV35MultipleServices)
	  return 2;
  eProductType type = proc->GetProductType();
  switch (type)
  {
    case eProductTypeRMX4000: return 8;
    case eProductTypeRMX2000: return 4;
    case eProductTypeRMX1500: return 2;
	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
  }

  FPASSERTSTREAM_AND_RETURN_VALUE(true, "Illegal type " << type, 2);
}

STATUS CCSMngrManager::AddIpService(CRequest* pRequest) {
	PTRACE(eLevelInfoNormal, "CCSMngrManager::AddIpService");

	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	if (pRequest->GetAuthorization() != SUPER) {
		PTRACE(eLevelInfoNormal,
				"CCSMngrManager::AddIpService: No permission to add ip service");
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CIPServiceList* pIpServiceListStatic =
			pCSMngrProcess->GetIpServiceListStatic();

	// in network separation when V35_ULTRA_SECURED_SUPPORT is on, you can have up to 2 services: 1 for RVGW and 1 for regular service
	BOOL isV35JitcSupport = NO;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT,
			isV35JitcSupport);

	BOOL isSeparatedNetworks = NO;
	sysConfig->GetBOOLDataByKey(CFG_KEY_SEPARATE_NETWORK, isSeparatedNetworks);
	BOOL isV35MultipleService = NO;
	sysConfig->GetBOOLDataByKey(CFG_KEY_V35_MULTIPLE_SERVICES, isV35MultipleService);

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (eProductTypeGesher != curProductType && eProductTypeNinja
			!= curProductType) {
		BOOL isMultipleServices = FALSE;
		if (pCSMngrProcess->GetIsMultipleServices() == eTRUE)
			isMultipleServices = TRUE;

		if (pIpServiceListStatic->GetServiceNumber() > 0
				&& (!isMultipleServices)
				&& (!(isV35JitcSupport && pIpServiceListStatic->GetServiceNumber() < rvgw_number(isV35MultipleService)))) {
			if (isV35JitcSupport)
				PTRACE(
						eLevelInfoNormal,
						"CCSMngrManager::AddIpService: This RMX doesn't support Multiple services. Only two services are allowed");
			else
				PTRACE(
						eLevelInfoNormal,
						"CCSMngrManager::AddIpService: This RMX doesn't support Multiple services. Only one service is allowed");

			pRequest->SetStatus(STATUS_TOO_MANY_SERVICES);
			return STATUS_OK;
		}
	}

	if (eProductTypeSoftMCU == curProductType || eProductTypeSoftMCUMfw == curProductType || eProductTypeEdgeAxis == curProductType) 
	{
		//only if there is one ip service configured
		if (pIpServiceListStatic->GetServiceNumber() != 0)
		{
			PTRACE(eLevelInfoNormal, "CCSMngrManager::AddIpService: Soft MCU doesn't support Multiple services. Only one service is allowed");

			pRequest->SetStatus(STATUS_TOO_MANY_SERVICES);
			return STATUS_OK;
		}
	}

	int max_num_of_services = MAX_NUMBER_OF_SERVICES_IN_RMX_2000_AND_1500;

	if (eProductTypeRMX4000 == curProductType)
		max_num_of_services = MAX_NUMBER_OF_SERVICES_IN_RMX_4000;

	if (eProductTypeGesher == curProductType) {
		max_num_of_services = 3;//TODO: define it in macro.
	}
	if (eProductTypeNinja == curProductType) {
		max_num_of_services = 2;//TODO: define it in macro.
	}
	if (pIpServiceListStatic->GetServiceNumber() >= max_num_of_services) {
		PTRACE(eLevelInfoNormal,
				"CCSMngrManager::AddIpService: Too many services for the RMX");
		pRequest->SetStatus(STATUS_TOO_MANY_SERVICES);
		return STATUS_OK;
	}

	CIPService* pnewIpService = (CIPService*) pRequest->GetRequestObject();

	CIPSpan* pSpan = pnewIpService->GetFirstSpan();
	int pos =0;
	CMedString debugStr = "CCSMngrManager::AddIpService:\n";
	while (pSpan)
	{
		DWORD ipV4 = pSpan->GetIPv4Address();
		debugStr << "pos :"<<pos <<" ip " << ipV4 << "\n";
		pSpan = pnewIpService->GetNextSpan();
		pos++;
	}
	PTRACE(eLevelInfoNormal, debugStr.GetString());
	//if the syatem is in V35 JITC support
//	if (isSeparatedNetworks && isV35JitcSupport)
//	{
//		//1. check that no more than 1 service is for V35
//		CIPService* pIpService = pIpServiceListStatic->GetFirstService();
//		//check if we have only 1 RVGW and 1 regular service
//		if (pnewIpService->GetIsV35GwEnabled() && pIpServiceListStatic->IsV35InUsed())//pIpService->GetIsV35GwEnabled())		//if the new service is V35, the old service can't be V35
//		{
//			PTRACE(eLevelInfoNormal, "CCSMngrManager::AddIpService: Too many V35 services for the RMX");
//			pRequest->SetStatus(STATUS_V35_GATEWAY_IS_ALREADY_CONFIGURED_IN_ANOTHER_SERVICE);
//			return STATUS_OK;
//		}
//
//		if (pIpServiceListStatic->GetIsServiceAdded() && !pnewIpService->GetIsV35GwEnabled()
//						&& !pIpServiceListStatic->IsV35InUsed()	)	//if the new service is V35, the old service can't be V35
//		{
//			PTRACE(eLevelInfoNormal, "CCSMngrManager::AddIpService: One of the services must be V35 service");
//			pRequest->SetStatus(STATUS_V35_SERVICE_IS_NEEDED);
//			return STATUS_OK;
//		}
//
//		//2. check that the second span define for v35
//		if (pnewIpService->GetIsV35GwEnabled())
//		{
//			if (pnewIpService->IsV35ValidPortDefinition() == FALSE)
//			{
//				PTRACE(eLevelInfoNormal, "CCSMngrManager::AddIpService: V35 service can't use ip addresses defined in port 1(in XML it is port 2).");
//				pRequest->SetStatus(STATUS_V35_MUST_BE_DEFINED_IN_THE_FIRST_PORT);
//				return STATUS_OK;
//			}
//		}
	// in SoftMcuMFW only SIP supported
	if (eProductTypeSoftMCUMfw == curProductType)
	{
		if (pnewIpService->GetIPProtocolType() != eIPProtocolType_SIP )
		{
			TRACEINTO << " in SoftMcuMFW only SIP supported. Status=STATUS_UNSUPPORTED_H323_PROTOCOL ("
					<< pCSMngrProcess->GetStatusAsString(STATUS_UNSUPPORTED_H323_PROTOCOL) << ")";
			pRequest->SetExDescription(pCSMngrProcess->GetStatusAsString(STATUS_UNSUPPORTED_H323_PROTOCOL).c_str());
			pRequest->SetStatus(STATUS_UNSUPPORTED_H323_PROTOCOL);

			return STATUS_OK;
		}
	}

#if 0
    //DNS Per Service need this Status, why set off?
	if (eProductTypeNinja == curProductType)
	{
		CIpDns* pNewDns = pnewIpService->GetpDns();

		if (eServerStatusOff != pNewDns->GetStatus())
		{
			TRACEINTO << "WARN:  DNS should be off for CS IP Service in Ninja.";
			
			pNewDns->SetStatus(eServerStatusOff);
		}
	}
#endif

	CLargeString errorMsg;
	bool isMustExist = false;
	bool isMustNotExist = true;
	STATUS status =
			CIpServiceListValidator(*pIpServiceListStatic).ValidateSingleCS(
					*pnewIpService, isMustExist, isMustNotExist, errorMsg);
	if (STATUS_OK != status) {
		std::string strStatus =
				"CCSMngrManager::AddIpService: Service validation FAILED, status : ";
		strStatus += pCSMngrProcess->GetStatusAsString(status);
		PTRACE(eLevelInfoNormal, strStatus.c_str());

		pRequest->SetStatus(status);
		pRequest->SetExDescription(errorMsg.GetString());

		return STATUS_OK;
	}

	// checking duplication Mngmnt vs. CS (the method was moved out of ValidateSingleCS method)
	status = pCSMngrProcess->ValidateDuplicateIpAddrCS(pnewIpService, errorMsg);
	if (STATUS_OK != status) {
		std::string strStatus =
				"CCSMngrManager::AddIpService: Service validation FAILED, status : ";
		strStatus += pCSMngrProcess->GetStatusAsString(status);
		PTRACE(eLevelInfoNormal, strStatus.c_str());

		pRequest->SetStatus(status);
		pRequest->SetExDescription(errorMsg.GetString());

		return STATUS_OK;
	}

	// checking duplicate span definition
	status = pCSMngrProcess->ValidateDuplicateSpanDefinition(
			pIpServiceListStatic, pnewIpService);
	if (STATUS_OK != status) {
		std::string strStatus =
				"CCSMngrManager::AddIpService: Service validation FAILED, status : ";
		strStatus += pCSMngrProcess->GetStatusAsString(status);
		PTRACE(eLevelInfoNormal, strStatus.c_str());

		pRequest->SetStatus(status);
		pRequest->SetExDescription(errorMsg.GetString());

		return STATUS_OK;
	}

	RemoveActiveAlarmByErrorCode(DUPLICATE_IP_CS_MNGMNT);

	status = pCSMngrProcess->FixPortRange(*pnewIpService);
	if (STATUS_OK != status) {
		if( STATUS_ILLEGAL_FIRST_UDP_PORT_VALUE != status )
		{
			PTRACE(eLevelInfoNormal, "Fixed port range is not in signaling range");

			pRequest->SetStatus(status);
			pRequest->SetExDescription(pCSMngrProcess->GetStatusAsString(status).c_str());
		}
		else
		{
			TRACEINTO << " in SoftMcuMFW illegal UDP first port value. Status=STATUS_ILLEGAL_FIRST_UDP_PORT_VALUE ("
					<< pCSMngrProcess->GetStatusAsString(status) << "). Available range - from: "
					<< (int)MFW_UDP_FIRST_PORT_MIN << " to: " << (int)MFW_UDP_FIRST_PORT_MAX;
			pRequest->SetExDescription(pCSMngrProcess->GetStatusAsString(status).c_str());
			pRequest->SetStatus(status);
		}
	}
	/*BRIDGE-3645 - this block is not needed also mutilple service in ipv6 past issue was fixed
	if (CheckIfDefaultGWIPV6IsValid(pnewIpService) == FALSE) {
		pRequest->SetStatus(
				STATUS_IPV6_ALREADY_DEFINED_DIFFERENTLY_IN_ANOTHER_SERVICE);
		return STATUS_OK;
	}
    */
	//pCSMngrProcess->AddNewIpService(*pnewIpService);

	TRACEINTOFUNC << "CCSMngrManager::AddIpService "<< pnewIpService->GetId();
	m_CommMcuMngrService->SendIpServiceParamInd(pnewIpService);

	// V35Gw password is received as plaintext and should be stored encrypted
	TreatV35GwPasswordFromEMA(*pnewIpService, eOperationOnService_Add);

	// H323 password is received as plaintext and should be stored encrypted
	TreatH323PasswordFromEMA(*pnewIpService, eOperationOnService_Add);

	pCSMngrProcess->AddNewIpService(*pnewIpService);
	
	CIPServiceList* pServiceListDynamic = pCSMngrProcess->GetIpServiceListDynamic();
	CreateNewSelfSignedCertificate(pnewIpService,pServiceListDynamic->GetServiceNumber());
	CIPService* pNewIpServiceDynamic = pServiceListDynamic->GetService(pnewIpService->GetId());

	bool isLicensingReceived = pCSMngrProcess->IsLicensingReceived();

	RemoveActiveAlarmByErrorCode(NO_SERVICE_FOUND_IN_DB);

	AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT, IP_SERVICE_ADDED,
			MAJOR_ERROR_LEVEL,
			"IP Network Service was added. Please reset the MCU", true, true);

	// Always ask user to reset MCU
	pRequest->SetStatus(STATUS_OK_WARNING);

	return STATUS_OK;
}

STATUS CCSMngrManager::DeleteIpService(CRequest* pRequest) {
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	if (pRequest->GetAuthorization() != SUPER) {
		FPTRACE(eLevelInfoNormal,
				"CCSMngrManager::DeleteIpService: No permission to delete ip service");
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CIPServiceDel* pDeleteIpService =
			(CIPServiceDel*) pRequest->GetRequestObject();
	if (!pDeleteIpService) {
		PTRACE(eLevelInfoNormal,
				"CCSMngrManager::DeleteIpService : ip service not exists");
		pRequest->SetStatus(STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS);
		return STATUS_OK;
	}

	const char* serviceName = pDeleteIpService->GetIPServiceName();
	if (!serviceName) {
		PTRACE(eLevelInfoNormal,
				"CCSMngrManager::DeleteIpService : No Name in Ip Service");
		pRequest->SetStatus(STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS);
		return STATUS_OK;
	}

	CIPServiceList* pIpServListStatic =
			pCSMngrProcess->GetIpServiceListStatic();
	int ind = pIpServListStatic->FindService(serviceName);
	if (NOT_FIND == ind) {
		PTRACE(eLevelInfoNormal,
				"CCSMngrManager::DeleteIpService : Ip Service Not found");
		pRequest->SetStatus(STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS);
		return STATUS_OK;
	}

	CIPService* pDeletedService = pIpServListStatic->GetService(serviceName);
	if (IsValidPObjectPtr(pDeletedService)) {
		DeleteSelfSignedCertificate(pDeletedService);
		pIpServListStatic->UpdateDeletedOccupiedSpan(pDeletedService);

		m_CommMcuMngrService->SendDelIpService(pDeletedService);
		TreatV35GwPasswordFromEMA(*pDeletedService, eOperationOnService_Delete);
		TreatH323PasswordFromEMA(*pDeletedService, eOperationOnService_Delete);

		/* VNGR-20807: notify ConfParty manager about Network Service Deletion, so that
		 *      it can remove that service from all profiles */
		m_CommConfService->SendDelIpService(pDeletedService);

		m_CommMcuMngrService->SendDelIpService(pDeletedService);

		pIpServListStatic->Cancel(serviceName);
		pIpServListStatic->WriteXmlFile(GetIpServiceTmpFileName().c_str());

		AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
				SYSTEM_CONFIGURATION_CHANGED, MAJOR_ERROR_LEVEL,
				"System configuration changed. Please reset the MCU.", true,
				true);

		return STATUS_OK;
	}
	else
	{
		PASSERT(1);
		return STATUS_FAIL;
	}
}


void CCSMngrManager::CreateNewSelfSignedCertificate(CIPService* pUpdatedIpService, WORD service_id)
{
	std::string output;
	const char* span_name = GetHostNameFromService(pUpdatedIpService).c_str();
	char strToAdd[256];
	snprintf(strToAdd, 256, "Scripts/Self_Signed_Cert.sh Create_ip_service_certificate %s %d", span_name, service_id);
	SystemPipedCommand(strToAdd,output);


}

void CCSMngrManager::DeleteSelfSignedCertificate(CIPService* pUpdatedIpService)
{
	std::string output;
	char strToAdd[256];
	snprintf(strToAdd, 256, "Scripts/Self_Signed_Cert.sh Delete_ip_service_certificate %d", pUpdatedIpService->GetId());
	SystemPipedCommand(strToAdd,output);


}


void CCSMngrManager::SyncCustomCfgFileWithIceParams(std::string key,DWORD data)
{

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (eProductTypeSoftMCUMfw != curProductType)
		return;
	CSysConfigEma* cfgFile = new CSysConfigEma();
	cfgFile->LoadFromFile(eCfgCustomParam);
		//update flag
	if (cfgFile->IsParamExist(key))
	{

		CCfgData* cfgData = cfgFile->GetCfgEntryByKey(key);
		std::string string_data;
		std::stringstream out;
		out << data;
		string_data = out.str();
		if (cfgData->GetData() != string_data)// && cfgDataMemory->GetIsReset()==true)
		{
			cfgData->SetData(string_data);
			cfgFile->SaveToFile("/mcu_custom_config/custom.cfg");
		}


	}






}





// service1 : the new Ip Service arrived from EMA
// service2 : the Ip Service to be updated
bool CCSMngrManager::UpdateGKIfNeeded(CIPService& service1,
		CIPService& service2) {
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::UpdateGKIfNeeded";

	bool wasUpdated = false;

	// Dynamic properties of ip service.
	CDynIPSProperties* dynService = service2.GetDynamicProperties();
	CH323Info& h323Info = dynService->GetH323Info();
	CProxyDataContent& primaryGK = h323Info.GetPrimaryGk();
	CProxyDataContent& altGK = h323Info.GetAltGk();
	CGKInfo& gkInfo = dynService->GetGKInfo();

	CSmallString gkName = service1.GetGatekeeperName();
	BYTE gk = service1.GetGatekeeper();
	WORD gkMode = service1.GetGatekeeperMode();
	DWORD gkExternalAddr = service1.GetExternalGatekeeperAddr(); // GetRealExternalGatekeeperAddr();
	CSmallString gkAltGKName = service1.GetAltGatekeeperName();
	WORD gkDiscoveryPort = service1.GetGatekeeperDiskoveryPort();

	const CH323Alias* dialPrefix = service1.GetDialInPrefix();
	WORD PRQPolling = service1.IsRRQPolling();
	WORD RRQPollingInterval = service1.GetRRQPollingInterval();
	BOOL isRegAsGW = service1.GetIsRegAsGW();

	// Check for every gatekeeper part to update, if it's not the same
	// as arrived from outside (EMA)
	if (gkName != service2.GetGatekeeperName()) {
		service2.SetGatekeeperName(gkName);

		// update the dynamic part
		primaryGK.SetIPv4Address(((HostName&) gkName).IpToDWORD());
		primaryGK.SetName(gkName.GetString());
		gkInfo.SetPrimaryGkIp(((HostName&) gkName).IpToDWORD());

		wasUpdated = true;
	}

	if (gk != service2.GetGatekeeper()) {
		service2.SetGatekeeper(gk);
		wasUpdated = true;
	}

	if (gkMode != service2.GetGatekeeperMode()) {
		service2.SetGatekeeperMode(gkMode);
		wasUpdated = true;
	}

	if (gkExternalAddr != service2.GetExternalGatekeeperAddr()) {
		HostName hostGkName(gkExternalAddr);
		service2.SetGatekeeperName(hostGkName);

		primaryGK.SetIPv4Address(gkExternalAddr);
		gkInfo.SetPrimaryGkIp(gkExternalAddr);

		wasUpdated = true;
	}

	if (gkAltGKName != service2.GetAltGatekeeperName()) {
		service2.SetAltGatekeeperName(gkAltGKName);

		DWORD altGKName = ((HostName&) gkAltGKName).IpToDWORD();
		altGK.SetIPv4Address(altGKName);
		altGK.SetName(gkAltGKName.GetString());
		gkInfo.SetAltGkIp(altGKName);

		wasUpdated = true;
	}

	if (gkDiscoveryPort != service2.GetGatekeeperDiskoveryPort()) {
		service2.SetGatekeeperDiskoveryPort(gkDiscoveryPort);
		wasUpdated = true;
	}

	const CH323Alias* dialPrefix2 = service2.GetDialInPrefix();
	if (!(*dialPrefix == *dialPrefix2)) {
		service2.SetDialInPrefix(dialPrefix->GetAliasName(),
				dialPrefix->GetAliasType());
		wasUpdated = true;
	}

	if (PRQPolling != service2.IsRRQPolling()) {
		service2.SetRRQPolling(PRQPolling);
		wasUpdated = true;
	}

	if (RRQPollingInterval != service2.GetRRQPollingInterval()) {
		service2.SetRRQPollingInterval(RRQPollingInterval);
		wasUpdated = true;
	}

	if (isRegAsGW != service2.GetIsRegAsGW()) {
		service2.SetIsRegAsGW(isRegAsGW);
		wasUpdated = true;
	}

	// Check if alias list was changed, in first Span
	CIPSpan* pIpSpan1 = service1.GetFirstSpan();
	CIPSpan* pIpSpan2 = service2.GetFirstSpan();
	int numOfAliases1 = pIpSpan1->GetAliasNamesNumber();
	int numOfAliases2 = pIpSpan2->GetAliasNamesNumber();
	bool replaceSpan = false;

	if (numOfAliases2 != numOfAliases1)
		replaceSpan = true;
	else
		for (int i = 0; i < numOfAliases1; ++i) {
			CH323Alias* alias1 = pIpSpan1->GetAlias(i);
			CH323Alias* alias2 = pIpSpan2->GetAlias(i);

			if (!alias1 && !alias2)
				continue;

			if (!alias1 || !alias2 || (*alias1 != *alias2)) {
				replaceSpan = true;
				break;
			}
		}

	if (replaceSpan) {
		TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::UpdateGKIfNeeded: "
				<< "Span should be replaced";
		wasUpdated = true;

		pIpSpan2->ReplaceAliasList(pIpSpan1->GetAliasList(), numOfAliases1);

		CIPSpan ipSpan = *pIpSpan2;
		service2.ReplaceSpan_NoCheck(0, ipSpan);
	}

	if (wasUpdated == true)
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::UpdateGKIfNeeded - need to be updated";

	return wasUpdated;
}

// service1 : the new Ip Service arrived from EMA
// service2 : the Ip Service to be updated
bool CCSMngrManager::UpdateSipProxyIfNeeded(CIPService& service1,
		CIPService& service2, bool& isIceTheOnlyChange) {
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::UpdateSipProxyIfNeeded";

	bool wasUpdated = false;

	// for static params:
	CSip* service1Sip = service1.GetpSip();
	CSip* service2Sip = service2.GetpSip();
	const CBaseSipServer* pProxy1, *pProxy2; // outbound
	const CSipServer* pRegistrar1, *pRegistrar2; // Registrars
	CSipAdvanced* pSipAdvanced1 = service1.GetpSipAdvanced(); // ice
	CSipAdvanced* pSipAdvanced2 = service2.GetpSipAdvanced();

	// for dynamic params:
	CDynIPSProperties* dynService = service2.GetDynamicProperties();
	CProxyInfo& proxyInfo = dynService->GetProxyInfo();
	SIP_PROXY_STATUS_PARAMS_S dynamicData = proxyInfo.GetSipProxyParams();

	// Update Sip servers status on/off:
	if (service1Sip->GetConfigurationOfSIPServers()
			!= service2Sip->GetConfigurationOfSIPServers()) {
		wasUpdated = true;
		service2Sip->SetConfigurationOfSIPServers(
				service1Sip->GetConfigurationOfSIPServers());
	}

	// Update Primary Registrar params:
	pRegistrar1 = service1Sip->GetpRegistrar(); // primary registrar 1
	pRegistrar2 = service2Sip->GetpRegistrar(); // primary registrar 2

	if ((pRegistrar1->GetName() != pRegistrar2->GetName())
			|| (pRegistrar1->GetDomainName() != pRegistrar2->GetDomainName())
			|| (pRegistrar1->GetPort() != pRegistrar2->GetPort())
			|| (pRegistrar1->GetStatus() != pRegistrar2->GetStatus())) {
		wasUpdated = true;
		// static params:
		service2Sip->SetRegistrar(*pRegistrar1);
		// dynamic params:
		dynamicData.ProxyList[0].Role = eServerTypePrimary;
		strncpy(dynamicData.ProxyList[0].Name,
				(pRegistrar1->GetName()).GetString(),
				sizeof(dynamicData.ProxyList[0].Name) - 1);
		dynamicData.ProxyList[0].Name[sizeof(dynamicData.ProxyList[0].Name) - 1]
				= '\0';
		dynamicData.ProxyList[0].IpV4.v4.ip = 0;
		dynamicData.ProxyList[0].IpV6.v6.ip[0] = 0;
		dynamicData.ProxyList[0].Status = eServerStatusAuto;
	}

	// Update Alt Registrar params:
	pRegistrar1 = service1Sip->GetpAltRegistrar(); // Alt registrar 1
	pRegistrar2 = service2Sip->GetpAltRegistrar(); // Alt registrar 2

	if ((pRegistrar1->GetName() != pRegistrar2->GetName())
			|| (pRegistrar1->GetDomainName() != pRegistrar2->GetDomainName())
			|| (pRegistrar1->GetPort() != pRegistrar2->GetPort())
			|| (pRegistrar1->GetStatus() != pRegistrar2->GetStatus())) {
		wasUpdated = true;
		// static params:
		service2Sip->SetAltRegistrar(*pRegistrar1);
		// dynamic params:
		dynamicData.ProxyList[1].Role = eServerTypeAlternate;
		strncpy(dynamicData.ProxyList[1].Name,
				(pRegistrar1->GetName()).GetString(),
				sizeof(dynamicData.ProxyList[1].Name) - 1);
		dynamicData.ProxyList[1].Name[sizeof(dynamicData.ProxyList[1].Name) - 1]
				= '\0';
		dynamicData.ProxyList[0].IpV4.v4.ip = 0;
		dynamicData.ProxyList[0].IpV6.v6.ip[0] = 0;
		dynamicData.ProxyList[1].Status = eServerStatusAuto;
	}

	// Update Outbound proxy params:
	pProxy1 = service1Sip->GetpProxy(); // outbound 1
	pProxy2 = service2Sip->GetpProxy(); // outbound 2

	if ((pProxy1->GetName() != pProxy2->GetName()) || (pProxy1->GetPort()
			!= pProxy2->GetPort()) || (pProxy1->GetStatus()
			!= pProxy2->GetStatus())) {
		wasUpdated = true;
		service2Sip->SetProxy(*pProxy1);
	}

	// Update refresh time
	if (service1Sip->GetRefreshRegistrationTout()
			!= service2Sip->GetRefreshRegistrationTout()) {
		wasUpdated = true;
		service2Sip->SetRefreshRegistrationTout(
				service1Sip->GetRefreshRegistrationTout());
	}

	// Set the update of the dynamic info:
	if (wasUpdated) {
		if (service1Sip->GetConfigurationOfSIPServers() == eConfSipServerAuto) // if sip server is off
			memset(&dynamicData, 0, sizeof(SIP_PROXY_STATUS_PARAMS_S));

		proxyInfo.SetSipProxyParams(dynamicData);
	}

	// Update SipAdvanced (ice)
	if (*pSipAdvanced1 != *pSipAdvanced2) {
		if (wasUpdated == false) {
			isIceTheOnlyChange = true;
			TRACESTR(eLevelInfoNormal)
					<< "CCSMngrManager::UpdateSipProxyIfNeeded - only ice was updated";
		}

		wasUpdated = true;

		*pSipAdvanced2 = *pSipAdvanced1;
	}

	if (wasUpdated == true)
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::UpdateSipProxyIfNeeded - need to be updated";

	return wasUpdated;
}

// service1 : the new Ip Service arrived from EMA
// service2 : the Ip Service to be updated
bool CCSMngrManager::UpdateV35GwIfNeeded(CIPService& service1,
		CIPService& service2, bool& isResetApacheRequired) {
	TRACESTR(eLevelInfoNormal) << "CCSMngrManager::UpdateV35GwIfNeeded";

	bool wasUpdated = false;
	isResetApacheRequired = false;

	BOOL isV35GwEnabled_1 = service1.GetIsV35GwEnabled();
	DWORD v35GwIpAddress_1 = service1.GetV35GwIpAddress();
	std::string v35GwUsername_1 = service1.GetV35GwUsername();
	std::string v35GwPassword_dec_1 = service1.GetV35GwPassword_dec();
	std::string v35GwPassword_enc_1 = service1.GetV35GwPassword_enc();

	BOOL isV35GwEnabled_2 = service2.GetIsV35GwEnabled();
	DWORD v35GwIpAddress_2 = service2.GetV35GwIpAddress();
	std::string v35GwUsername_2 = service2.GetV35GwUsername();
	std::string v35GwPassword_dec_2 = service2.GetV35GwPassword_dec();
	std::string v35GwPassword_enc_2 = service2.GetV35GwPassword_enc();

	if ((isV35GwEnabled_1 != isV35GwEnabled_2) || (v35GwIpAddress_1
			!= v35GwIpAddress_2) || (v35GwUsername_1 != v35GwUsername_2)
			|| (v35GwPassword_dec_1 != v35GwPassword_dec_2)
			|| (v35GwPassword_enc_1 != v35GwPassword_enc_2)) {
		wasUpdated = true;

		// if V35_GW becomes enabled/disabled- then Apache reset is required
		if (isV35GwEnabled_1 != isV35GwEnabled_2)
			isResetApacheRequired = true;

		service2.SetIsV35GwEnabled(isV35GwEnabled_1);
		service2.SetV35GwIpAddress(v35GwIpAddress_1);
		service2.SetV35GwUsername(v35GwUsername_1);
		service2.SetV35GwPassword_dec(v35GwPassword_dec_1);
		service2.SetV35GwPassword_enc(v35GwPassword_enc_1);
	}

	if (true == wasUpdated)
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::UpdateV35GwIfNeeded - need to be updated"
				<< "\nReset Apache is "
				<< (isResetApacheRequired ? "" : "not ") << "required";

	return wasUpdated;
}

// service1 : the new Ip Service arrived from EMA
// service2 : the Ip Service to be updated
bool CCSMngrManager::UpdateSecurityIfNeeded(CIPService& service1,
		CIPService& service2) {
	TRACESTR(eLevelInfoNormal) << "CCSMngrManager::UpdateSecurityIfNeeded";

	bool wasUpdated = false;

	BOOL isH323Enabled_1 = service1.GetH323AuthenticationEnable();
	std::string username_1 = service1.GetH323AuthenticationUserName();
	std::string vH323Password_1 = service1.GetH323AuthenticationPassword();
	BOOL isH323Enabled_2 = service2.GetH323AuthenticationEnable();
	std::string username_2 = service2.GetH323AuthenticationUserName();
	std::string vH323Password_2 = service2.GetH323AuthenticationPassword();

	if ((isH323Enabled_1 != isH323Enabled_2) || (username_1 != username_2)
			|| (vH323Password_1 != vH323Password_2)) {
		wasUpdated = true;
		service2.SetH323AuthenticationEnable(isH323Enabled_1);
		service2.SetH323AuthenticationUserName(username_1.c_str());
		service2.SetH323AuthenticationPassword(vH323Password_1.c_str());

		// H323 password is received as plaintext and should be stored encrypted
		TreatH323PasswordFromEMA(service1, eOperationOnService_Update);
	}
	else if (!username_1.empty())
	{
		// H323 password  received as plaintext and should always be stored encrypted,
		//otherwise the password in IPServiceList.xml will store in plaintext
		TreatH323PasswordFromEMA(service1, eOperationOnService_Update);
	}

	if (true == wasUpdated)
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::UpdateSecurityIfNeeded - need to be updated";

	return wasUpdated;
}

// Check if 2 services are equal one to onther, by dumping their fields.
bool CCSMngrManager::AreServicesEqual(CIPService& service1,
		CIPService& service2, string sPurpose, bool isFromEMA /*=false*/,
		bool fixPorts /*=true*/) {
	// motivation: compare the static list updated by GK changes with Ip Service arrived from EMA
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::AreServicesEqual (for "
			<< sPurpose << ")";

	// ==>  service1 <-> service2
	CXMLDOMElement* pUpdatedXMLNode = NULL, *pStaticXMLNode = NULL;

	if (fixPorts) {
		// Set static udp ports, to 0, if it wasn't fixed by operator.
		CIPSpan* pSpan = service2.GetFirstSpan();
		while (NULL != pSpan) {
			CCommH323PortRange* portRange = pSpan->GetPortRange();
			if (FALSE == portRange->IsEnabledPortRange())
				portRange->SetUdpPortRange(0, 0);
			pSpan = service2.GetNextSpan();
		}

		// Set static udp ports, to 0, if it wasn't fixed by operator.
		pSpan = service1.GetFirstSpan();
		while (NULL != pSpan) {
			CCommH323PortRange* portRange = pSpan->GetPortRange();
			if (FALSE == portRange->IsEnabledPortRange()) {
				portRange->SetUdpPortRange(0, 0);
			}

			pSpan = service1.GetNextSpan();
		}
	}

	service1.SerializeXml(pUpdatedXMLNode, UPDATE_CNT_BEGIN_END, isFromEMA);
	service2.SerializeXml(pStaticXMLNode, UPDATE_CNT_BEGIN_END, isFromEMA);

	char* updatedDumpStr = NULL;
	DWORD updatedDumLen = 0;
	pUpdatedXMLNode->DumpDataAsStringWithAttribute(&updatedDumpStr,
			&updatedDumLen, 0, TRUE);

	delete pUpdatedXMLNode;

	char* staticDumpStr = NULL;
	DWORD staticDumLen = 0;
	pStaticXMLNode->DumpDataAsStringWithAttribute(&staticDumpStr,
			&staticDumLen, 0, TRUE);

	delete pStaticXMLNode;

	bool servicesEqual = false;
	if (!strcmp(staticDumpStr, updatedDumpStr))
		servicesEqual = true;
	//else
	//{
	//	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::AreServicesEqual: "
	//							 << "\nstaticDumpStr:\n" << staticDumpStr;

	//	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::AreServicesEqual: "
	//							 << "\nupdatedDumpStr:\n" << updatedDumpStr;
	//}

	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::AreServicesEqual (for "
			<< sPurpose << "):" << "\n=== Services are "
			<< (servicesEqual ? "equal" : "different");

	PDELETEA(updatedDumpStr);
	PDELETEA(staticDumpStr);

	return servicesEqual;
}

void CCSMngrManager::EncryptV35GwPassword(CIPService& theService) {
	std::string sEnc;
	EncodeHelper eH;
	eH.EncodeAes(theService.GetV35GwPassword_dec(), sEnc);

	theService.SetV35GwPassword_enc(sEnc);
}

void CCSMngrManager::DecryptV35GwPassword(CIPService& theService) {
	std::string sDec;
	EncodeHelper eH;
	eH.DecodeAes(theService.GetV35GwPassword_enc(), sDec);

	theService.SetV35GwPassword_dec(sDec);
}

void CCSMngrManager::DecryptV35GwPasswordsFromFile(CIPServiceList& theList) {
	CIPService* pService = theList.GetFirstService();
	while (NULL != pService) {
		DecryptV35asswordsFromFileToService(*pService);

		pService = theList.GetNextService();
	}
}

void CCSMngrManager::DecryptGKAuthenticationPasswordsFromFile(
		CIPServiceList& theList) {
	CIPService* pService = theList.GetFirstService();
	while (NULL != pService) {
		DecryptH323Password(*pService);

		pService = theList.GetNextService();
	}
}

STATUS CCSMngrManager::DecryptV35asswordsFromFileToService(
		CIPService& theService) {
	TRACESTR(eLevelInfoNormal)
			<< "CCSMngrManager::DecryptV35PwdFromFileToService";

	STATUS retStatus = STATUS_OK;

	// ===== 1. open the file
	FILE* pFile = fopen(V35_GATEWAY_PASSWORDS_FILE_PATH, "r");
	if (!pFile) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::DecryptV35PwdFromFileToService"
				<< "\nFAILED to open file " << V35_GATEWAY_PASSWORDS_FILE_PATH
				<< " for reading";
		return STATUS_FAIL;
	}

	// ===== 2. read the file and update the string
	string sLine = ""; // will contain a single line that is read from file
	string sName = theService.GetName();
	char cLine[512];
	bool bIsFileCurrpted = false;

	int tmpI = 0;
	memset(cLine, 0, 512);
	while (fgets(cLine, sizeof cLine, pFile)) // loop over file's lines
	{
		sLine = cLine;

		tmpI++;
		if (sLine=="\n" || sLine=="")
			continue;
		TRACESTR(eLevelInfoNormal) << "DecryptV35PwdFromFileToService - "
				<< tmpI << "\nName: " << sName << "\nLine: " << sLine;

		// if it's the current service, then decrypt the enc pwd into the suitable member in the service
		if (!strncmp(sLine.c_str(), sName.c_str(), sName.length())) // it's the current service (operator '==' cannot be used since sLine contains 'endl' char)
		{
			memset(cLine, 0, 512);
			if (!fgets(cLine, sizeof cLine, pFile)) // read the next line that contains pwd length from file
			{
				bIsFileCurrpted = true;
				retStatus = STATUS_FAIL;
				PASSERT(1);
				break;
			}

			int pwdLength = atoi(cLine);
			if (pwdLength < 0) {
				bIsFileCurrpted = true;
				retStatus = STATUS_FAIL;
				PASSERT(2);
				break;
			} else if (pwdLength == 0)
				continue;
			else if (pwdLength > (int) (sizeof(cLine) / sizeof(char))) // for the upcoming fread pwdLength is the count and char is the element, the count is divided by sizeof(char) for documentation - compiler will resolve the division - no performance overhead

				pwdLength = sizeof(cLine) / sizeof(char);

			memset(cLine, 0, 512);
			int pwdRead = fread(cLine, sizeof(char), pwdLength, pFile); // read the next line that contains pwd from file
			if (pwdRead != pwdLength) {
				bIsFileCurrpted = true;
				retStatus = STATUS_FAIL;
				PASSERT(3);
				break;
			}

			sLine = cLine;
			string sDec = "";

			TRACESTR(eLevelInfoNormal) << "DecryptV35PwdFromFileToService2 - "
					<< tmpI << "\nName: " << sName << "\ncLine: " << cLine;

			if (sLine != "" && sLine != "\n") {
				EncodeHelper eH;
				eH.DecodeAes(sLine.c_str(), sDec);

				theService.SetV35GwPassword_enc(sLine);
				theService.SetV35GwPassword_dec(sDec);
			}

			TRACESTR(eLevelInfoNormal)
					<< "DecryptV35PwdFromFileToService - sLine == Name";
			break;
		}

		memset(cLine, 0, 512);
	} // end loop over file's lines

	if (true == bIsFileCurrpted)
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::DecryptV35PwdFromFileToService"
				<< "\nBad file " << V35_GATEWAY_PASSWORDS_FILE_PATH
				<< "\n(ServiceName " << sLine << " without password";

	// ===== 3. close the file
	int fcloseReturn = fclose(pFile);
	if (FCLOSE_SUCCESS != fcloseReturn) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::PrepareUpdatedV35GwPasswordsFileString_DeleteEntry"
				<< "\nFAILED to close file " << V35_GATEWAY_PASSWORDS_FILE_PATH;
	}

	return retStatus;
}

void CCSMngrManager::TreatV35GwPasswordFromEMA(CIPService& theService,
		eOperationOnService theOperation) {
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::TreatV35GwPasswordFromEMA";

	// add or update service
	if ((eOperationOnService_Add == theOperation)
			|| (eOperationOnService_Update == theOperation))
		// V35Gw password is received from EMA as plaintext,
		// and 'DeSerializeXml' method stores it in m_V35GwPassword_dec field.

		// ===== 1. encrypt the password
		EncryptV35GwPassword(theService);

	STATUS status = UpdateV35GwPasswordsFile(theService, theOperation);
}

STATUS CCSMngrManager::UpdateV35GwPasswordsFile(CIPService& theService,
		eOperationOnService theOperation) {
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::UpdateV35GwPasswordsFile";

	STATUS retStatus = STATUS_OK;

	if (eOperationOnService_Add == theOperation)
		retStatus = AddEntryToV35GwPasswordsFile(theService);
	else if (eOperationOnService_Update == theOperation)
		retStatus = UpdateEntryInV35GwPasswordsFile(theService);
	else
		retStatus = RemoveEntryFromV35GwPasswordsFile(theService);

	return retStatus;
}

STATUS CCSMngrManager::AddEntryToV35GwPasswordsFile(CIPService& theService) {
	TRACESTR(eLevelInfoNormal)
			<< "CCSMngrManager::AddEntryToV35GwPasswordsFile";

	STATUS retStatus = STATUS_OK;

	FILE* pFile = fopen(V35_GATEWAY_PASSWORDS_FILE_PATH, "a");
	if (!pFile) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::AddEntryToV35GwPasswordsFile"
				<< "\nFAILED to append to file "
				<< V35_GATEWAY_PASSWORDS_FILE_PATH;
		return STATUS_FAIL;
	}

	char sLen[10];
	snprintf(sLen, sizeof(sLen), "%d", theService.GetV35GwPassword_enc_Length());

	string sToWrite = "\n";
	sToWrite += theService.GetName();
	sToWrite += "\n";
	sToWrite += sLen;
	sToWrite += "\n";
	sToWrite += theService.GetV35GwPassword_enc();

	int theLength = sToWrite.length();

	int bytesWritten = fwrite(sToWrite.c_str(), sizeof(BYTE), theLength, pFile);
	retStatus = (bytesWritten == theLength ? STATUS_OK : STATUS_FAIL);

	if (STATUS_OK != retStatus)
		PASSERT(1);

	int fcloseReturn = fclose(pFile);
	if (FCLOSE_SUCCESS != fcloseReturn) {
		TRACESTR(eLevelInfoNormal)
				<< "\nCCSMngrManager::AddEntryToV35GwPasswordsFile"
				<< "\nFAILED to close file " << V35_GATEWAY_PASSWORDS_FILE_PATH;
	}

	return retStatus;
}

STATUS CCSMngrManager::UpdateEntryInV35GwPasswordsFile(CIPService& theService) {
	TRACESTR(eLevelInfoNormal)
			<< "CCSMngrManager::UpdateEntryInV35GwPasswordsFile";

	STATUS retStatus = STATUS_OK;

	BOOL isExist = IsFileExists(V35_GATEWAY_PASSWORDS_FILE_PATH);
	TRACESTR(eLevelInfoNormal)
			<< "CCSMngrManager::UpdateEntryInV35GwPasswordsFile - 1";
	if (FALSE == isExist) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::UpdateEntryInV35GwPasswordsFile - 2";
		// file does not exist; treat it as a new service
		AddEntryToV35GwPasswordsFile(theService);
	} else {
		// ===== 1. read the file and update the pwd
		string sUpdatedFileString = ""; // will contain the whole file to be re-written (after updating)
		retStatus = PrepareUpdatedV35GwPasswordsFileString_UpdateEntry(
				theService, sUpdatedFileString);
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::UpdateEntryInV35GwPasswordsFile - 3";

		if (STATUS_OK == retStatus)
			// ===== 2. write the updated file
			retStatus = WriteV35GwPasswordsFile(sUpdatedFileString);
	}

	return retStatus;
}

STATUS CCSMngrManager::RemoveEntryFromV35GwPasswordsFile(CIPService& theService) {
	TRACESTR(eLevelInfoNormal)
			<< "CCSMngrManager::RemoveEntryFromV35GwPasswordsFile";

	STATUS retStatus = STATUS_OK;

	// ===== 1. read the file and remove the entry
	BOOL isExist = IsFileExists(V35_GATEWAY_PASSWORDS_FILE_PATH);
	if (TRUE == isExist) {
		std::string sUpdatedFileString; // will contain the whole file to be re-written (after updating)
		retStatus = PrepareUpdatedV35GwPasswordsFileString_DeleteEntry(
				theService, sUpdatedFileString);

		if (STATUS_OK == retStatus)
			// ===== 2. write the updated file
			retStatus = WriteV35GwPasswordsFile(sUpdatedFileString);
	} // end if file exists

	return retStatus;
}

STATUS CCSMngrManager::PrepareUpdatedV35GwPasswordsFileString_UpdateEntry(
		CIPService& theService, std::string& sUpdatedFileString) {
	TRACESTR(eLevelInfoNormal)
			<< "CCSMngrManager::PrepareUpdatedV35GwPasswordsFileString_UpdateEntry";

	STATUS retStatus = STATUS_OK;

	// ===== 1. open the file
	FILE* pFile = fopen(V35_GATEWAY_PASSWORDS_FILE_PATH, "r");
	if (!pFile) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::PrepareUpdatedV35GwPasswordsFileString_UpdateEntry"
				<< "\nFAILED to open file " << V35_GATEWAY_PASSWORDS_FILE_PATH
				<< " for reading";
		return STATUS_FAIL;
	}

	// ===== 2. read the file and update the string
	std::string sLine; // will contain a single line that is read from file
	const std::string sName = theService.GetName();
	char cLine[512];
	bool bIsFileCurrpted = false;
	bool bIsEntryExists = false;

	int tmpI = 0;
	memset(cLine, 0, 512);
	while (fgets(cLine, sizeof cLine, pFile)) // loop over file's lines
	{
		sLine = cLine;

		tmpI++;
		TRACESTR(eLevelInfoNormal)
				<< "PrepareUpdatedV35GwPasswordsFileString_UpdateEntry - "
				<< tmpI << "\nName: " << sName << "\nLine: " << sLine;

		// write current line (serviceName or pwd) to the updated file
		sUpdatedFileString += sLine;

		// if it's the updated service then specifically update the pwd (that appears in the consecutive line)
		if (!strncmp(sLine.c_str(), sName.c_str(), sName.length())) // it's the updated service (operator '==' cannot be used since sLine contains 'endl' char)
		{
			bIsEntryExists = true;

			char sLen[10];
			snprintf(sLen, sizeof(sLen), "%d",
					theService.GetV35GwPassword_enc_Length());

			sUpdatedFileString += sLen;
			sUpdatedFileString += "\n";

			sUpdatedFileString += theService.GetV35GwPassword_enc(); // and write the updated pwd instead
			sUpdatedFileString += "\n";

			memset(cLine, 0, 512);
			if (!fgets(cLine, sizeof cLine, pFile)) // skip the next line that contains current (old) pwd length from file
			{
				bIsFileCurrpted = true;
				PASSERT(1);
				break;
			}

			int oldPwdLength = atoi(cLine);
			if (oldPwdLength < 0) {
				bIsFileCurrpted = true;
				retStatus = STATUS_FAIL;
				PASSERT(2);
				break;
			} else if (oldPwdLength == 0)
				continue;
			else if (oldPwdLength > (int) (sizeof(cLine) / sizeof(char))) // for the upcoming fread oldPwdLength is the count and char is the element, the count is divided by sizeof(char) for documentation - compiler will resolve the division - no performance overhead

				oldPwdLength = sizeof(cLine) / sizeof(char);

			memset(cLine, 0, 512);
			int pwdRead = fread(cLine, sizeof(char), oldPwdLength, pFile); // read the next line that contains current (old) pwd from file
			if (pwdRead != oldPwdLength) {
				bIsFileCurrpted = true;
				retStatus = STATUS_FAIL;
				PASSERT(3);
				break;
			}
		} // end sLine == ServiceName

		memset(cLine, 0, 512);
	} // end loop over file's lines

	PASSERTSTREAM(bIsFileCurrpted, "Bad file "
			<< V35_GATEWAY_PASSWORDS_FILE_PATH << ", (ServiceName " << sLine
			<< " without password");

	// ===== 3. close the file
	int fcloseReturn = fclose(pFile);
	if (FCLOSE_SUCCESS != fcloseReturn) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::PrepareUpdatedV35GwPasswordsFileString_UpdateEntry"
				<< "\nFAILED to close file " << V35_GATEWAY_PASSWORDS_FILE_PATH;
	}

	if (false == bIsEntryExists) {
		TRACESTR(eLevelInfoNormal)
				<< "PrepareUpdatedV35GwPasswordsFileString_UpdateEntry - MANUAL LINE PREPARED";

		char sLen[10];
		snprintf(sLen, sizeof(sLen), "%d",
				theService.GetV35GwPassword_enc_Length());

		sUpdatedFileString += "\n";
		sUpdatedFileString += sName;
		sUpdatedFileString += "\n";
		sUpdatedFileString += sLen;
		sUpdatedFileString += "\n";
		sUpdatedFileString += theService.GetV35GwPassword_enc();
	}

	return retStatus;
}

STATUS CCSMngrManager::PrepareUpdatedV35GwPasswordsFileString_DeleteEntry(
		CIPService& theService, std::string& sUpdatedFileString) {
	TRACESTR(eLevelInfoNormal)
			<< "CCSMngrManager::PrepareUpdatedV35GwPasswordsFileString_DeleteEntry";

	STATUS retStatus = STATUS_OK;

	// ===== 1. open the file
	FILE* pFile = fopen(V35_GATEWAY_PASSWORDS_FILE_PATH, "r");
	if (!pFile) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::PrepareUpdatedV35GwPasswordsFileString_DeleteEntry"
				<< "\nFAILED to open file " << V35_GATEWAY_PASSWORDS_FILE_PATH
				<< " for reading";
		return STATUS_FAIL;
	}

	// ===== 2. read the file and update the string
	std::string sLine; // will contain a single line that is read from file
	std::string sName = theService.GetName();
	char cLine[512];
	bool bIsFileCurrpted = false;

	int tmpI = 0;
	memset(cLine, 0, 512);
	while (fgets(cLine, sizeof cLine, pFile)) // loop over file's lines
	{
		sLine = cLine;

		tmpI++;
		TRACESTR(eLevelInfoNormal)
				<< "PrepareUpdatedV35GwPasswordsFileString_DeleteEntry - "
				<< tmpI << "\nName: " << sName << "\nLine: " << sLine;

		// if it's not the deleted service, then write current line (serviceName or pwd) to the updated file
		if (strncmp(sLine.c_str(), sName.c_str(), sName.length())) // it's not the deleted service (operator '!=' cannot be used since sLine contains 'endl' char)
		{
			TRACESTR(eLevelInfoNormal)
					<< "PrepareUpdatedV35GwPasswordsFileString_DeleteEntry - sLine != Name!!";
			sUpdatedFileString += sLine;
		}

		else // it's the deleted service; so skip current line (containing serviceName) and the consecutive line (containing pwd)
		{
			TRACESTR(eLevelInfoNormal)
					<< "PrepareUpdatedV35GwPasswordsFileString_DeleteEntry - sLine == Name!!";

			memset(cLine, 0, 512);
			if (!fgets(cLine, sizeof cLine, pFile)) // skip next line (containing pwd length)

				bIsFileCurrpted = true;

			memset(cLine, 0, 512);
			if (!fgets(cLine, sizeof cLine, pFile)) // skip next line (containing pwd)

				bIsFileCurrpted = true;
		}

		memset(cLine, 0, 512);
	} // end loop over file's lines

	if (true == bIsFileCurrpted)
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::PrepareUpdatedV35GwPasswordsFileString_DeleteEntry"
				<< "\nBad file " << V35_GATEWAY_PASSWORDS_FILE_PATH
				<< "\n(ServiceName " << sLine << " without password";

	// ===== 3. close the file
	int fcloseReturn = fclose(pFile);
	if (FCLOSE_SUCCESS != fcloseReturn) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::PrepareUpdatedV35GwPasswordsFileString_DeleteEntry"
				<< "\nFAILED to close file " << V35_GATEWAY_PASSWORDS_FILE_PATH;
	}

	return retStatus;
}

STATUS CCSMngrManager::WriteV35GwPasswordsFile(string sToWrite) {
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::WriteV35GwPasswordsFile";

	STATUS retStatus = STATUS_OK;

	FILE* pFile = fopen(V35_GATEWAY_PASSWORDS_FILE_PATH, "w");
	if (!pFile) {
		TRACESTR(eLevelInfoNormal) << "CCSMngrManager::WriteV35GwPasswordsFile"
				<< "\nFAILED to open file " << V35_GATEWAY_PASSWORDS_FILE_PATH
				<< " for writing";
		return STATUS_FAIL;
	}

	int theLength = sToWrite.length();

	int bytesWritten = fwrite(sToWrite.c_str(), sizeof(BYTE), theLength, pFile);
	retStatus = (bytesWritten == theLength ? STATUS_OK : STATUS_FAIL);

	if (STATUS_OK != retStatus)
		PASSERT(true);

	int fcloseReturn = fclose(pFile);
	if (FCLOSE_SUCCESS != fcloseReturn) {
		TRACESTR(eLevelInfoNormal)
				<< "\nCCSMngrManager::WriteV35GwPasswordsFile"
				<< "\nFAILED to close file " << V35_GATEWAY_PASSWORDS_FILE_PATH;
	}

	return retStatus;
}

STATUS CCSMngrManager::UpdateIpService(CRequest* pRequest) {
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::UpdateIpService";


	STATUS status = STATUS_OK;

	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(status);

	if (pRequest->GetAuthorization() != SUPER) {
		FPTRACE(eLevelInfoNormal,
				"CCSMngrManager::UpdateIpService: No permission to update ip service");
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CIPService* pUpdatedIpService = (CIPService*) pRequest->GetRequestObject();
	if (!pUpdatedIpService) {
		PTRACE(eLevelInfoNormal,
				"CCSMngrManager::UpdateIpService : ip service not exists");
		pRequest->SetStatus(STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS);
		return STATUS_OK;
	}

	status = pCSMngrProcess->FixPortRange(*pUpdatedIpService);
	if (status != STATUS_OK) {

		if( STATUS_ILLEGAL_FIRST_UDP_PORT_VALUE != status )
		{
			WORD maxNumOfCalls = pUpdatedIpService->GetMaxNumOfCalls();
			if (pCSMngrProcess->GetLicensingMaxNumOfParties() < maxNumOfCalls)
				maxNumOfCalls = pCSMngrProcess->GetLicensingMaxNumOfParties();

			PTRACE(eLevelInfoNormal,
					"CCSMngrManager::UpdateIpService : max number of ports exceeded");
			char exDesc[256];
			sprintf(exDesc, "Max number of ports can't exceed %d", maxNumOfCalls
					* 2);
			pRequest->SetStatus(status);
			pRequest->SetExDescription(exDesc);
		}
		else
		{
			TRACEINTO << " in SoftMcuMFW illegal UDP first port value. Status=STATUS_ILLEGAL_FIRST_UDP_PORT_VALUE ("
					<< pCSMngrProcess->GetStatusAsString(status) << "). Available range - from: "
					<< (int)MFW_UDP_FIRST_PORT_MIN << " to: " << (int)MFW_UDP_FIRST_PORT_MAX;
			pRequest->SetExDescription(pCSMngrProcess->GetStatusAsString(status).c_str());
			pRequest->SetStatus(status);
		}
		return STATUS_OK;
	}

	 //VNGFE-6622
  	if (pUpdatedIpService->GetIPProtocolType() == eIPProtocolType_SIP)
  		pUpdatedIpService->SetGatekeeper(GATEKEEPER_NONE);
  
	CLargeString errorMsg;
	CIPServiceList* pIpServListStatic =
			pCSMngrProcess->GetIpServiceListStatic();
	bool isMustExist = true;
	bool isMustNotExist = false;
	status = CIpServiceListValidator(*pIpServListStatic).ValidateSingleCS(
			*pUpdatedIpService, isMustExist, isMustNotExist, errorMsg);
	if (STATUS_OK != status) {
		std::string strStatus =
				"CCSMngrManager::UpdateIpService: Service validation FAILED, status : ";
		strStatus += pCSMngrProcess->GetStatusAsString(status);
		PTRACE(eLevelInfoNormal, strStatus.c_str());

		pRequest->SetExDescription(errorMsg.GetString());
		pRequest->SetStatus(status);

		return STATUS_OK;
	}

	// checking duplication Mngmnt vs. CS (the method was moved out of ValidateSingleCS method)
	status = pCSMngrProcess->ValidateDuplicateIpAddrCS(pUpdatedIpService,
			errorMsg);
	if (STATUS_OK != status) {
		std::string strStatus =
				"CCSMngrManager::UpdateIpService: Service validation FAILED, status : ";
		strStatus += pCSMngrProcess->GetStatusAsString(status);
		PTRACE(eLevelInfoNormal, strStatus.c_str());

		pRequest->SetExDescription(errorMsg.GetString());
		pRequest->SetStatus(status);

		return STATUS_OK;
	}

	// checking duplicate span definition
	CIPService* pStaticIpService = pIpServListStatic->GetService(
			pUpdatedIpService->GetName());
	status = pCSMngrProcess->ValidateDuplicateSpanDefinition(pIpServListStatic,
			pUpdatedIpService, pStaticIpService);
	if (STATUS_OK != status) {
		std::string strStatus =
				"CCSMngrManager::UpdateIpService: Service validation FAILED, status : ";
		strStatus += pCSMngrProcess->GetStatusAsString(status);
		PTRACE(eLevelInfoNormal, strStatus.c_str());

		pRequest->SetExDescription(errorMsg.GetString());
		pRequest->SetStatus(status);

		return STATUS_OK;
	}

	BOOL isV35JitcSupport = NO;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT,
				isV35JitcSupport);

	BOOL isSeparatedNetworks = NO;
	sysConfig->GetBOOLDataByKey(CFG_KEY_SEPARATE_NETWORK, isSeparatedNetworks);

	BOOL isV35MultipleServices = NO;
	sysConfig->GetBOOLDataByKey(CFG_KEY_V35_MULTIPLE_SERVICES, isV35MultipleServices);

	// checking V35 Gateway is already configured on another service
	if ((TRUE == pUpdatedIpService->GetIsV35GwEnabled()) && !isV35MultipleServices) {
		status = pCSMngrProcess->ValidateV35GwAlreadyConfigured(
				pUpdatedIpService->GetName());
		if (STATUS_OK != status) {
			std::string strStatus =
					"CCSMngrManager::UpdateIpService: Service validation FAILED, status : ";
			strStatus += pCSMngrProcess->GetStatusAsString(status);
			PTRACE(eLevelInfoNormal, strStatus.c_str());

			pRequest->SetExDescription(errorMsg.GetString());
			pRequest->SetStatus(status);

			return STATUS_OK;
		}
	}



	if (isV35JitcSupport && isSeparatedNetworks && !isV35MultipleServices) // if there are 2 services in this status, one must be V35 service

		if (pUpdatedIpService->GetIsV35GwEnabled() == FALSE) // check that the other service has V35 configuration
		{
			status = pCSMngrProcess->ValidateV35GwAlreadyConfigured(
					pUpdatedIpService->GetName());
			if (status == STATUS_OK) // the other service also doesn't configured as V35
			{
				std::string strStatus =
						"CCSMngrManager::UpdateIpService: Service validation FAILED, status : ";
				strStatus += pCSMngrProcess->GetStatusAsString(
						STATUS_V35_SERVICE_IS_NEEDED);
				PTRACE(eLevelInfoNormal, strStatus.c_str());

				pRequest->SetExDescription(errorMsg.GetString());
				pRequest->SetStatus(STATUS_V35_SERVICE_IS_NEEDED);

				return STATUS_OK;
			}
		}
	/*BRIDGE-3645 - this block is not needed also mutilple service in ipv6 past issue was fixed
	if (CheckIfDefaultGWIPV6IsValid(pUpdatedIpService) == FALSE) {
		pRequest->SetStatus(
				STATUS_IPV6_ALREADY_DEFINED_DIFFERENTLY_IN_ANOTHER_SERVICE);
		return STATUS_OK;
	}*/

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	if (curProductType == eProductTypeSoftMCU || eProductTypeEdgeAxis == curProductType || curProductType == eProductTypeCallGeneratorSoftMCU)
	{
		status = pCSMngrProcess->ValidateSwMcuFields(pUpdatedIpService); //check that only fields that CAN be updated in SMCU were changed
		if (status != STATUS_OK) // the other service also doesn't configured as V35
		{
			std::string strStatus =
					"CCSMngrManager::UpdateIpService: Service validation FAILED, status : ";
			strStatus += pCSMngrProcess->GetStatusAsString(status);
			PTRACE(eLevelInfoNormal, strStatus.c_str());

			pRequest->SetExDescription(errorMsg.GetString());
			pRequest->SetStatus(status);

			return STATUS_OK;
		}
	}
	if ( eProductTypeSoftMCUMfw == curProductType )
	{
		status = pCSMngrProcess->ValidateMFWFields(pUpdatedIpService, pRequest);
		if( status != STATUS_OK )
		{
			pRequest->SetStatus(status);

			return STATUS_OK;
		}
	}
	// in SoftMcuMFW only SIP supported
	if (eProductTypeSoftMCUMfw == curProductType)
	{
		if (pUpdatedIpService->GetIPProtocolType() != eIPProtocolType_SIP )
		{
			TRACEINTO << " in SoftMcuMFW only SIP supported. Status=STATUS_UNSUPPORTED_H323_PROTOCOL ("
					<< pCSMngrProcess->GetStatusAsString(STATUS_UNSUPPORTED_H323_PROTOCOL) << ")";
			pRequest->SetExDescription(pCSMngrProcess->GetStatusAsString(STATUS_UNSUPPORTED_H323_PROTOCOL).c_str());
			pRequest->SetStatus(STATUS_UNSUPPORTED_H323_PROTOCOL);

			return STATUS_OK;
		}
	}
	if (GetHostNameFromService(pUpdatedIpService)
			 !=GetHostNameFromService(pStaticIpService))
	{
		CreateNewSelfSignedCertificate(pUpdatedIpService, pUpdatedIpService->GetId());
	}

	bool isStunPortChanged = false;
	bool isTurnPortChanged = false;

	CIceStandardParams* pIceUpdatedIpService = GetIceParamsFromService(pUpdatedIpService);
	if (pIceUpdatedIpService)
	{
		CIceStandardParams* pIceStaticIpService = GetIceParamsFromService(pStaticIpService);
		if (pIceStaticIpService)
		{
			if (pIceUpdatedIpService->GetSTUNServerPort() != pIceStaticIpService->GetSTUNServerPort())
				isStunPortChanged = true;
			if (pIceUpdatedIpService->GetTURNServerPort() != pIceStaticIpService->GetTURNServerPort())
				isTurnPortChanged = true;
		}
	}

#if 0
	if (eProductTypeNinja == curProductType)
	{
		CIpDns* pUpdatedDns = pUpdatedIpService->GetpDns();

		if (eServerStatusOff != pUpdatedDns->GetStatus())
		{
			TRACEINTO << "WARN:  DNS should be off for CS IP Service in Ninja.";
			
			pUpdatedDns->SetStatus(eServerStatusOff);
		}
	}
#endif

	status = PerformUpdateServiceProcedures(pUpdatedIpService, true);
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::UpdateIpService - end"
			<< "\nPerformUpdateServiceProcedures returned status " << status
			<< " (" << pCSMngrProcess->GetStatusAsString(status) << ")";

	// V35Gw password is received as plaintext and should be stored encrypted
	if ((STATUS_OK == status) || (STATUS_OK_WARNING == status))
    {
		TreatV35GwPasswordFromEMA(*pUpdatedIpService,
				eOperationOnService_Update);
		if (isStunPortChanged)
		{
			TRACEINTOFUNC << "CCSMngrManager::UpdateIpService SyncCustomCfgFileWithIceParams STUN port";
			if (GetIceParamsFromService(pUpdatedIpService) != NULL)
			   SyncCustomCfgFileWithIceParams("STUN_SERVER_PORT",GetIceParamsFromService(pUpdatedIpService)->GetSTUNServerPort());
		}
		if (isTurnPortChanged)
		{
			TRACEINTOFUNC << "CCSMngrManager::UpdateIpService SyncCustomCfgFileWithIceParams TURN port";
			if (GetIceParamsFromService(pUpdatedIpService) != NULL)
			   SyncCustomCfgFileWithIceParams("TURN_SERVER_PORT",GetIceParamsFromService(pUpdatedIpService)->GetTURNServerPort());
		}
	}

	//check dns configuration
	CIpDns* pDns = pUpdatedIpService->GetpDns();
	DWORD ipv4_0 = pDns->GetIPv4Address(0);
	char ipv6_0[IPV6_ADDRESS_LEN];
	memset(ipv6_0, '\0', sizeof(ipv6_0));

	pDns->GetIPv6Address(0, ipv6_0, 0);

	if(eServerStatusOff != pDns->GetStatus() && (ipv4_0 == 0) && (0 == strcmp("", ipv6_0) || 0 == strcmp(ipv6_0, "::")))

	{
		pRequest->SetStatus(STATUS_FAIL_TO_CONFIG_DNS);
		pRequest->SetExDescription(pCSMngrProcess->GetStatusAsString(STATUS_FAIL_TO_CONFIG_DNS).c_str());
		return STATUS_OK;
	}

	TRACEINTOFUNC << "CCSMngrManager::UpdateIpService "<< pUpdatedIpService->GetId();
	m_CommMcuMngrService->SendIpServiceParamInd(pUpdatedIpService);

	pRequest->SetStatus(status);

	return STATUS_OK;
}

void CCSMngrManager::OnFailoverUpdateServiceInd(CSegment* pSeg) {
	DWORD len;
	*pSeg >> len;

	char* pServiceElementStr = new char[len + 1];
	pServiceElementStr[len] = '\0';
	*pSeg >> pServiceElementStr;

	FTRACESTR(eLevelInfoHigh)
			<< "CCSMngrManager::OnFailoverUpdateServiceInd - the string:\n"
			<< pServiceElementStr;

	CXMLDOMDocument* pDom = new CXMLDOMDocument;
	PASSERTMSG_AND_RETURN(NULL == pDom, "new pDom failed");
	if (pDom->Parse((const char**) &pServiceElementStr) == SEC_OK) {
		CXMLDOMElement* pRoot = pDom->GetRootElement();
		if (pRoot) {
			char szErrorMsg[ERROR_MESSAGE_LEN];
			CIPService* pServiceFromFailoverTask = new CIPService;
			pServiceFromFailoverTask->DeSerializeXml(pRoot, szErrorMsg, "");

			// print the struct received (for debugging)
			CIpParameters ipParamsFromFailoverTask;
			pServiceFromFailoverTask->ConvertToIpParamsStruct(
					*(ipParamsFromFailoverTask .GetIpParamsStruct()));
			TRACESTR(eLevelInfoHigh)
					<< "CCSMngrManager::OnFailoverUpdateServiceInd - the struct:\n"
					<< ipParamsFromFailoverTask;
			// fix BRIDGE-9497 to get the corresponding IPService by Penrod 
			CIPService* pStaticIpService =
					pCSMngrProcess->GetIpServiceListStatic()->GetService(pServiceFromFailoverTask->GetId());
			if(pStaticIpService)
			{
        			CIPService* pUpdatedService = new CIPService(*pStaticIpService);
        			UpdateSyncedFields(pServiceFromFailoverTask, pUpdatedService);

        			PerformUpdateServiceProcedures(pUpdatedService);
        			POBJDELETE(pUpdatedService);
			}

			POBJDELETE(pServiceFromFailoverTask);
		} // end if (pRoot)
	} // end if (Parse==SEC_OK)
	
	if (pServiceElementStr) { delete [] pServiceElementStr; pServiceElementStr = NULL; }
	if (pDom) PDELETE(pDom);
}

void CCSMngrManager::UpdateSyncedFields(CIPService* pServiceFromFailoverTask,
		CIPService* pUpdatedService) {
	CIPSpan* pSpanFromFailoverTask = NULL;
	CIPSpan* pSpanFromUpdatedService = NULL;

	// ===== 1. GK
	// --- 1a. is enabld
	BYTE gk = pServiceFromFailoverTask->GetGatekeeper();
	pUpdatedService->SetGatekeeper(gk);

	// --- 1b. primary gk names
	const CSmallString& gkName = pServiceFromFailoverTask->GetGatekeeperName();
	pUpdatedService->SetGatekeeperName(gkName);

	// --- 1c. alternate gk names
	const CSmallString& altGkName =
			pServiceFromFailoverTask->GetAltGatekeeperName();
	pUpdatedService->SetAltGatekeeperName(altGkName);

	// --- 1d. prefix and aliases
	// from SRS: "In case there is a prefix then use it
	// otherwise use the first master alias and add it the suffix "_bck"
	// (the alias will always be H.323 ID)"
	const CH323Alias* pPrefix = pServiceFromFailoverTask->GetDialInPrefix();
	pUpdatedService->SetDialInPrefix(*pPrefix);

	pSpanFromFailoverTask = pServiceFromFailoverTask->GetFirstSpan();
	pSpanFromUpdatedService = pUpdatedService->GetFirstSpan();

	pSpanFromUpdatedService->CancelAliasList();

	const char* prefixName = pPrefix->GetAliasName();
	CH323Alias* pAliasFromFailoverTask = pSpanFromFailoverTask->GetFirstAlias();
	while (pAliasFromFailoverTask) // no prefix exists
	{
		const char* aliasNameFromFailoverTask =
				pAliasFromFailoverTask->GetAliasName();
		if (*aliasNameFromFailoverTask == '\0')
			break;

		std::string sNewAliasName = ForceAppendToAliasName(
				aliasNameFromFailoverTask, "_bck");

		CH323Alias newAlias;
		newAlias.SetAliasName(sNewAliasName.c_str());
		newAlias.SetAliasType(PARTY_H323_ALIAS_H323_ID_TYPE);
		pSpanFromUpdatedService->AddAlias(newAlias);

		pAliasFromFailoverTask = pSpanFromFailoverTask->GetNextAlias();
	}

	// --- 1e. is register as gw
	BOOL isGwRegFromFailoverTask = pServiceFromFailoverTask->GetIsRegAsGW();
	pUpdatedService->SetIsRegAsGW(isGwRegFromFailoverTask);

	// --- 1f. gk mode
	WORD gkModeFromFailoverTask = pServiceFromFailoverTask->GetGatekeeperMode();
	pUpdatedService->SetGatekeeperMode(gkModeFromFailoverTask);

	// --- 1g. refresh registraion params
	WORD isRrqPollingFromFailoverTask =
			pServiceFromFailoverTask->IsRRQPolling();
	pUpdatedService->SetRRQPolling(isRrqPollingFromFailoverTask);

	WORD rrqPollingIntervalFromFailoverTask =
			pServiceFromFailoverTask->GetRRQPollingInterval();
	pUpdatedService->SetRRQPollingInterval(rrqPollingIntervalFromFailoverTask);

	// ===== 2. SIP
	CSip* pSipFromFailoverTask = pServiceFromFailoverTask->GetSip();
	pUpdatedService->SetSip(*pSipFromFailoverTask);

	// ===== 3. Ports ranges
	pSpanFromFailoverTask = pServiceFromFailoverTask->GetFirstSpan();
	pSpanFromUpdatedService = pUpdatedService->GetFirstSpan();

	CCommH323PortRange* pPortRange = NULL;
	while (pSpanFromFailoverTask && pSpanFromUpdatedService) {
		pPortRange = pSpanFromFailoverTask->GetPortRange();
		pSpanFromUpdatedService->SetPortRange(*pPortRange);

		pSpanFromFailoverTask = pServiceFromFailoverTask->GetNextSpan();
		pSpanFromUpdatedService = pUpdatedService->GetNextSpan();
	}

	// ===== 4. QOS
	CQualityOfService* pQosFromFailoverTask =
			pServiceFromFailoverTask->GetpQualityOfService();
	pUpdatedService->SetQualityOfService(*pQosFromFailoverTask);

	// ===== 4. DNS (probably not needed; the 'real' DNS is configured in the Mngmnt service)
	CIpDns* pDnsFromFailoverTask = pServiceFromFailoverTask->GetpDns();
	pUpdatedService->SetDns(*pDnsFromFailoverTask);

	pUpdatedService->SetIPProtocolType(
			pServiceFromFailoverTask->GetIPProtocolType());
}

string CCSMngrManager::ForceAppendToAliasName(const char* originalAliasName,
		const char* strToAppend) {
	string retStr = "";

	int lenAlias = strlen(originalAliasName);
	int lenToAppend = strlen(strToAppend);

	if ((0 == lenAlias) || (ALIAS_NAME_LEN <= lenToAppend)) // no need or impossible to append

		return originalAliasName;

	// test if we need to append or to remove the strToAppend
	if (lenAlias > lenToAppend) {
		string aliasStr = originalAliasName;
		string sub = aliasStr.substr(lenAlias - lenToAppend, lenToAppend);

		if (sub.find(strToAppend) != string::npos) {
			retStr = aliasStr.substr(0, lenAlias - lenToAppend);
			return retStr;
		}
	}

	if (ALIAS_NAME_LEN > lenAlias + lenToAppend) // no problem to append
	{
		retStr = originalAliasName;
		retStr += strToAppend;
	} else // originalAliasName should be shortened
	{
		char buf[ALIAS_NAME_LEN + 1];
		memset(buf, 0, ALIAS_NAME_LEN + 1);
		snprintf(buf, ALIAS_NAME_LEN + 1, "%s", originalAliasName);

		int whereToStart = ALIAS_NAME_LEN - lenToAppend - 1;
		memcpy(buf + whereToStart, strToAppend, lenToAppend);
		buf[whereToStart + lenToAppend + 1] = '\0';

		retStr = buf;
	}

	return retStr;
}
STATUS    CCSMngrManager::PerformUpdateServiceProceduresV2(CIPService* pUpdatedService,bool isFromEMA )
{
	std::string ipServiceFileName, ipServiceFileNameTmp;
	CIPServiceList* pIpServListStatic =pCSMngrProcess->GetIpServiceListStatic();

	CIPService* pStaticIpService = pIpServListStatic->GetService(pUpdatedService->GetName());
	PASSERTMSG_AND_RETURN_VALUE(!pStaticIpService, "CCSMngrManager::PerformUpdateServiceProceduresV2 - Fail, pStaticIpService is NULL.", STATUS_FAIL);

	CIPServiceList* pDynamicIpServiceList =pCSMngrProcess->GetIpServiceListDynamic();
	CIPService* pDynamicIpService = pDynamicIpServiceList->GetService(pUpdatedService->GetName());


	pCSMngrProcess->GetIpServiceFileNames(ipServiceFileName,ipServiceFileNameTmp);
	BOOL isServiceEqual = TRUE;
	STATUS status = STATUS_OK;

	if (pDynamicIpService == NULL)
			TRACESTR(eLevelInfoNormal) << "CCSMngrManager::PerformUpdateServiceProcedures - service doesn't exist in dynamic list - nothing to update!";

	else
	{
		// ======> Change pDynamicIpService to be locally, because it's a pointer
		CIPService dynamicLocal = *pDynamicIpService;
		// in Failover Master mode, operations that require reset are blocked
		if ((true == isFromEMA) && (true
				== pCSMngrProcess->GetIsFailoverFeatureEnabled()) && (false
				== pCSMngrProcess->GetIsFailoverSlaveMode()))
		{
			TRACESTR(eLevelInfoNormal)
					<< "CCSMngrManager::PerformUpdateServiceProcedures - checking UpdateService for Failover Master mode";

			if (FALSE == isServiceEqual) {
				TRACESTR(eLevelInfoNormal)
						<< "CCSMngrManager::PerformUpdateServiceProcedures - UpdateService is blocked (Failover Master mode)";
				return STATUS_ACTION_ILLEGAL_AT_FAILOVER_MASTER_MODE;
			}
		}
	}

	//Compare with H323 password encrypted.
	std::string vH323Password_1 = pUpdatedService->GetH323AuthenticationPassword();
	std::string vH323Password_2 = pStaticIpService->GetH323AuthenticationPassword();
	if(!vH323Password_1.empty())
	    TreatH323PasswordFromEMA(*pUpdatedService,eOperationOnService_Update);
	if(!vH323Password_2.empty())
	    TreatH323PasswordFromEMA(*pStaticIpService,eOperationOnService_Update);


	CIPService copyOfUpdatedService = *pUpdatedService;
	CIPService copyOfStaticService = *pStaticIpService;


	isServiceEqual  = AreServicesEqual(copyOfUpdatedService,copyOfStaticService, "producing System Alert", isFromEMA);

	if(!isServiceEqual)
    {
		bool isIceTheOnlyChange = false;
		CIPService dynamicLocal = *pDynamicIpService;
		// The following method will update Gatekeeper part of dynamic service, if it's needed.
		bool isGKchanged = UpdateGKIfNeeded(*pUpdatedService, dynamicLocal);
				// The following method will update Sip Proxy part of dynamic service, if it's needed.
		bool isSipProxychanged = UpdateSipProxyIfNeeded(*pUpdatedService,dynamicLocal, isIceTheOnlyChange);
		// The following method will update Gatekeeper part of dynamic service, if it's needed.
		bool isSecuritychanged = UpdateSecurityIfNeeded(*pUpdatedService,dynamicLocal);


		PerformUpdate_UpdateProcess(pUpdatedService,dynamicLocal,isGKchanged,isSipProxychanged,isSecuritychanged,isIceTheOnlyChange);


		pIpServListStatic->UpdateOccupiedSpan(pUpdatedService);
		pIpServListStatic->Update(*pUpdatedService);
		pIpServListStatic->WriteXmlFile(ipServiceFileNameTmp.c_str());
		AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
						SYSTEM_CONFIGURATION_CHANGED, MAJOR_ERROR_LEVEL,
						"System configuration changed. Please reset the MCU.", true,
						true);
		status = STATUS_OK_WARNING;
	}


	return status;

}

STATUS CCSMngrManager::PerformUpdate_UpdateProcess(CIPService* pUpdatedService,CIPService& dynamicLocal,bool isGKchanged,bool isSipProxychanged,bool isSecuritychanged,bool isIceTheOnlyChange)
{
	TRACESTR(eLevelInfoNormal)<< "BRIDGE-8284 " << "\n isGKchanged =  " << isGKchanged << "\n"
									  <<" 	isSipProxychanged = " <<  isSipProxychanged <<"   isSecuritychanged = " << isSecuritychanged;
	std::string ipServiceFileName, ipServiceFileNameTmp;
	pCSMngrProcess->GetIpServiceFileNames(ipServiceFileName,ipServiceFileNameTmp);
	CIPServiceList* pIpServListStatic =pCSMngrProcess->GetIpServiceListStatic();

	CIPService* pStaticIpService = pIpServListStatic->GetService(pUpdatedService->GetName());
	STATUS status = STATUS_OK;
	if (isGKchanged || isSipProxychanged || isSecuritychanged) {

		STATUS retStat = SendSyncLockMsgToConfPartyProcess();
				// STATUS retStat = STATUS_OK;
		if (STATUS_OK == retStat)
		{
				TRACESTR(eLevelInfoNormal)<<  "Conf party locked conf/s";
					// Send to Gatekeeper process
					if (isGKchanged) {
						CDynIPSProperties* dynService =
								dynamicLocal.GetDynamicProperties();
						CH323Info& gkInfo = dynService->GetCSGKInfo();
						gkInfo.SetIsUpdatedFromCsModule(true);
						m_CommGKService->SendIpServiceParamInd(&dynamicLocal);
					}

					// Send to Sip Proxy process
					if (isSipProxychanged)
						m_CommProxyService->SendIpServiceParamInd(&dynamicLocal);

					// Send to ConfParty process
					m_CommConfService->SendIpServiceParamInd(&dynamicLocal);

					m_CommConfService->SendReleaseMsgReq();

					TRACESTR(eLevelInfoNormal)<<"Release message was sent to ConfParty";


					// Update dynamic list + file, for only GK + other dynamic parts of Ip Service
					CIPServiceList* pIpServListDynamic =
							pCSMngrProcess->GetIpServiceListDynamic();

					PASSERTMSG_AND_RETURN_VALUE(!pIpServListDynamic, "CCSMngrManager::PerformUpdate_UpdateProcess - Fail, pIpServListDynamic is NULL", STATUS_FAIL);

					pIpServListDynamic->Update(dynamicLocal);
					pIpServListDynamic->WriteXmlFile(ipServiceFileName.c_str());

					// update static list with GK updates from EMA
					if (isGKchanged)
						UpdateGKIfNeeded(*pUpdatedService, *pStaticIpService);

					// update static list with Sip Proxy updates from EMA
					if (isSipProxychanged && (pUpdatedService)) {
						CSipAdvanced* pSipAdvanced =
								pUpdatedService->GetpSipAdvanced();
						if (pSipAdvanced) {
							int iceType = pSipAdvanced->GetIceEnvironment();

							// update ICE Server to not available at signaling monitoring DB - VNGR-19424
							CIceInfo iceInfo; // Initialized with notavailable status
							if ((iceType == eIceEnvironment_None)
									&& (pUpdatedService))
								pIpServListDynamic->UpdateDynamic(
										pUpdatedService->GetId(), iceInfo);

							int sipServerType = eSipServer_generic;
							if (pUpdatedService->GetSip()->GetConfigurationOfSIPServers())
								sipServerType
										= pUpdatedService->GetSip()->GetSipServerType();

							bool tmpIsIceTheOnlyChange = false; // dummy - not in used.

							UpdateSipProxyIfNeeded(*pUpdatedService,
									*pStaticIpService, tmpIsIceTheOnlyChange);

							TRACEINTO
									<< "\nCCSMngrManager::PerformUpdateServiceProcedures"
									<< std::endl << "For CSId: "
									<< pUpdatedService->GetId()
									<< " update ice type";

							COsQueue* signalingMbx =
									pCSMngrProcess->GetSignalingMbx(
											pUpdatedService->GetId() - 1);

							if (signalingMbx) {
								TRACEINTO
										<< "CCSMngrManager::PerformUpdateServiceProcedures - CSMNGR_ICE_TYPE_IND - sending";
								CSegment* pSegToSignaling = new CSegment;
								*pSegToSignaling << (WORD) iceType;

								CSignalingApi api;
								api.CreateOnlyApi(*signalingMbx);
								api.SendMsg(pSegToSignaling, CSMNGR_ICE_TYPE_IND);

								TRACEINTO
										<< "CCSMngrManager::PerformUpdateServiceProcedures - CSMNGR_SIP_SERVER_TYPE_IND - sending";
								CSegment* pSegToCS = new CSegment;
								*pSegToCS << (WORD) sipServerType;

								api.SendMsg(pSegToCS, CSMNGR_SIP_SERVER_TYPE_IND);
							} else if (signalingMbx == NULL)
								TRACEINTO
										<< "CCSMngrManager::PerformUpdateServiceProcedures"
										<< endl
										<< "Failed to find SignalingTask for csId: "
										<< pUpdatedService->GetId()
										<< "Maybe the task didn't start yet...";
						} else
							status = STATUS_INTERNAL_ERROR;
					}


					if (isSecuritychanged) {
						m_CommGKService->SendUpdateIpServiceParamInd(&dynamicLocal);
						UpdateSecurityIfNeeded(*pUpdatedService, *pStaticIpService);
					}
				}
				else if (STATUS_ILLEGAL != retStat) // timeout
				{
					TRACESTR(eLevelInfoNormal)<< "Conf Party timed out";
					status = STATUS_INTERNAL_ERROR;
				}
				else // if SendSyncLockMsgToConfPartyProcess was unsuccessful
				{
					PTRACE(eLevelInfoNormal,
							"Conf Party failed to lock conferences");
					status = STATUS_FAIL_TO_UPDATE_GK;

					// VSGNINJA-583: Not sending out URQ if GK set to OFF while ongoing conference.
					// Per Lior and Noa' comments, we should send URQ before reboot.
					// And send out un-register to sip server.
					// Send to Gatekeeper process
					if (isGKchanged) {
						CDynIPSProperties* dynService =
								dynamicLocal.GetDynamicProperties();
						CH323Info& gkInfo = dynService->GetCSGKInfo();
						gkInfo.SetIsUpdatedFromCsModule(true);
						m_CommGKService->SendIpServiceParamInd(&dynamicLocal);
					}

					// Send to Sip Proxy process
					if (isSipProxychanged)
						m_CommProxyService->SendIpServiceParamInd(&dynamicLocal);

				}
			}

	return status;
}

STATUS CCSMngrManager::PerformUpdateServiceProcedures(CIPService* pUpdatedService, bool isFromEMA /*=false*/) {

	STATUS status = STATUS_OK;

	PASSERT_AND_RETURN_VALUE(pUpdatedService == NULL, STATUS_FAIL);
	CIPServiceList* pIpServListStatic =
			pCSMngrProcess->GetIpServiceListStatic();
	if (!pIpServListStatic) {
		PASSERTMSG(1, "Static Ip Service is null !");
		return STATUS_ILLEGAL;
	}

	int service_id = pIpServListStatic->GetServiceIdByName(
			pUpdatedService->GetName());

	if (NOT_FIND == service_id)
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::PerformUpdateServiceProcedures - can't find service name in dynamic list - nothing to update!!";

	else {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::PerformUpdateServiceProcedures -service id by list:"
				<< service_id;
		CServiceConfig* pServiceConfig = pUpdatedService->GetServiceConfig();
		pServiceConfig->SetId(service_id);
	}

	pIpServListStatic->UpdateCounters();

	CIPServiceList* pDynamicIpServiceList =
			pCSMngrProcess->GetIpServiceListDynamic();
	if (!pDynamicIpServiceList) {
		PASSERTMSG(1, "Dynmaic Ip Service is null !");
		return STATUS_ILLEGAL;
	}
	///BRIDGE-8284
	return PerformUpdateServiceProceduresV2(pUpdatedService,isFromEMA);
/*
	CIPService* pStaticIpService = pIpServListStatic->GetService(pUpdatedService->GetName());

	bool isSipProxychanged = false;
	bool isIceTheOnlyChange = false;
	bool isV35Changed = false;
	bool isResetApache = false;

	std::string ipServiceFileName, ipServiceFileNameTmp;
	pCSMngrProcess->GetIpServiceFileNames(ipServiceFileName,ipServiceFileNameTmp);


	CIPService* pDynamicIpService = pDynamicIpServiceList->GetService(pUpdatedService->GetName());


	if (pDynamicIpService == NULL)
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::PerformUpdateServiceProcedures - service doesn't exist in dynamic list - nothing to update!";
	else {
		// ======> Change pDynamicIpService to be locally, because it's a pointer
		CIPService dynamicLocal = *pDynamicIpService;

		// in Failover Master mode, operations that require reset are blocked
		if ((true == isFromEMA) && (true
				== pCSMngrProcess->GetIsFailoverFeatureEnabled()) && (false
				== pCSMngrProcess->GetIsFailoverSlaveMode())) {
			TRACESTR(eLevelInfoNormal)
					<< "CCSMngrManager::PerformUpdateServiceProcedures - checking UpdateService for Failover Master mode";

			CIPService copyOfUpdatedService = *pUpdatedService;
			CIPService copyOfCurrentService = *pDynamicIpService;

			// copy the GK section, that does not require reboot
			UpdateGKIfNeeded(copyOfUpdatedService, copyOfCurrentService);
			// copy the Security section, that does not require reboot
			UpdateSecurityIfNeeded(copyOfUpdatedService, copyOfCurrentService);

			// compare the services (after GK section became the same)
			bool onlyGkUpdated =
					AreServicesEqual(copyOfUpdatedService,
							copyOfCurrentService, "blocking Failover Master",
							isFromEMA);
			if (false == onlyGkUpdated) {
				TRACESTR(eLevelInfoNormal)
						<< "CCSMngrManager::PerformUpdateServiceProcedures - UpdateService is blocked (Failover Master mode)";
				return STATUS_ACTION_ILLEGAL_AT_FAILOVER_MASTER_MODE;
			}
		}

		// The following method will update Gatekeeper part of dynamic service, if it's needed.
		bool isGKchanged = UpdateGKIfNeeded(*pUpdatedService, dynamicLocal);
		// The following method will update Sip Proxy part of dynamic service, if it's needed.
		isSipProxychanged = UpdateSipProxyIfNeeded(*pUpdatedService,
				dynamicLocal, isIceTheOnlyChange);

		// The following method will update Gatekeeper part of dynamic service, if it's needed.
		bool isSecuritychanged = UpdateSecurityIfNeeded(*pUpdatedService,
				dynamicLocal);

		// Updating V35GW does not require system reset, but may require Apache reset
		isV35Changed = UpdateV35GwIfNeeded(*pUpdatedService, dynamicLocal,
				isResetApache);
		TRACESTR(eLevelInfoNormal)<< "BRIDGE-8284 " << "\n isGKchanged =  " << isGKchanged << "\n"
								  <<" 	isSipProxychanged = " <<  isSipProxychanged << "\n isV35Changed ="  << isV35Changed
								  <<"   isSecuritychanged = " << isSecuritychanged;
		if (isGKchanged || isSipProxychanged || isV35Changed
				|| isSecuritychanged) {
			STATUS retStat = SendSyncLockMsgToConfPartyProcess();
			// STATUS retStat = STATUS_OK;
			if (STATUS_OK == retStat) {
				PTRACE(eLevelInfoNormal, "Conf party locked conf/s");

				// Send to Gatekeeper process
				if (isGKchanged) {
					CDynIPSProperties* dynService =
							dynamicLocal.GetDynamicProperties();
					CH323Info& gkInfo = dynService->GetCSGKInfo();
					gkInfo.SetIsUpdatedFromCsModule(true);
					m_CommGKService->SendIpServiceParamInd(&dynamicLocal);
				}

				// Send to Sip Proxy process
				if (isSipProxychanged)
					m_CommProxyService->SendIpServiceParamInd(&dynamicLocal);

				// Send to ConfParty process
				m_CommConfService->SendIpServiceParamInd(&dynamicLocal);

				m_CommConfService->SendReleaseMsgReq();
				PTRACE(eLevelInfoNormal,
						"Release message was sent to ConfParty");

				// Update dynamic list + file, for only GK + other dynamic parts of Ip Service
				CIPServiceList* pIpServListDynamic =
						pCSMngrProcess->GetIpServiceListDynamic();

				pIpServListDynamic->Update(dynamicLocal);
				pIpServListDynamic->WriteXmlFile(ipServiceFileName.c_str());

				// update static list with GK updates from EMA
				if (isGKchanged)
					UpdateGKIfNeeded(*pUpdatedService, *pStaticIpService);

				// update static list with Sip Proxy updates from EMA
				if (isSipProxychanged && (pUpdatedService)) {
					CSipAdvanced* pSipAdvanced =
							pUpdatedService->GetpSipAdvanced();
					if (pSipAdvanced) {
						int iceType = pSipAdvanced->GetIceEnvironment();

						// update ICE Server to not available at signaling monitoring DB - VNGR-19424
						CIceInfo iceInfo; // Initialized with notavailable status
						if ((iceType == eIceEnvironment_None)
								&& (pIpServListDynamic) && (pUpdatedService))
							pIpServListDynamic->UpdateDynamic(
									pUpdatedService->GetId(), iceInfo);

						int sipServerType = eSipServer_generic;
						if (pUpdatedService->GetSip()->GetConfigurationOfSIPServers())
							sipServerType
									= pUpdatedService->GetSip()->GetSipServerType();

						bool tmpIsIceTheOnlyChange = false; // dummy - not in used.

						UpdateSipProxyIfNeeded(*pUpdatedService,
								*pStaticIpService, tmpIsIceTheOnlyChange);

						TRACEINTO
								<< "\nCCSMngrManager::PerformUpdateServiceProcedures"
								<< std::endl << "For CSId: "
								<< pUpdatedService->GetId()
								<< " update ice type";

						COsQueue* signalingMbx =
								pCSMngrProcess->GetSignalingMbx(
										pUpdatedService->GetId() - 1);

						if (signalingMbx) {
							TRACEINTO
									<< "CCSMngrManager::PerformUpdateServiceProcedures - CSMNGR_ICE_TYPE_IND - sending";
							CSegment* pSegToSignaling = new CSegment;
							*pSegToSignaling << (WORD) iceType;

							CSignalingApi api;
							api.CreateOnlyApi(*signalingMbx);
							api.SendMsg(pSegToSignaling, CSMNGR_ICE_TYPE_IND);

							TRACEINTO
									<< "CCSMngrManager::PerformUpdateServiceProcedures - CSMNGR_SIP_SERVER_TYPE_IND - sending";
							CSegment* pSegToCS = new CSegment;
							*pSegToCS << (WORD) sipServerType;

							api.SendMsg(pSegToCS, CSMNGR_SIP_SERVER_TYPE_IND);
						} else if (signalingMbx == NULL)
							TRACEINTO
									<< "CCSMngrManager::PerformUpdateServiceProcedures"
									<< endl
									<< "Failed to find SignalingTask for csId: "
									<< pUpdatedService->GetId()
									<< "Maybe the task didn't start yet...";
					} else
						status = STATUS_INTERNAL_ERROR;
				}

				if (isV35Changed) {
					m_CommCardService->SendIpServiceParamInd(&dynamicLocal);
					UpdateV35GwIfNeeded(*pUpdatedService, *pStaticIpService,
							isResetApache);
				}

				if (isSecuritychanged) {
					m_CommGKService->SendUpdateIpServiceParamInd(&dynamicLocal);
					UpdateSecurityIfNeeded(*pUpdatedService, *pStaticIpService);
				}
			} 
			else if (STATUS_ILLEGAL != retStat) // timeout
			{
				PTRACE(eLevelInfoNormal, "Conf Party timed out");
				status = STATUS_INTERNAL_ERROR;
			}
			else // if SendSyncLockMsgToConfPartyProcess was unsuccessful
			{
				PTRACE(eLevelInfoNormal,
						"Conf Party failed to lock conferences");
				status = STATUS_FAIL_TO_UPDATE_GK;

				// VSGNINJA-583: Not sending out URQ if GK set to OFF while ongoing conference.
				// Per Lior and Noa' comments, we should send URQ before reboot.
				// And send out un-register to sip server.
				// Send to Gatekeeper process
				if (isGKchanged) {
					CDynIPSProperties* dynService =
							dynamicLocal.GetDynamicProperties();
					CH323Info& gkInfo = dynService->GetCSGKInfo();
					gkInfo.SetIsUpdatedFromCsModule(true);
					m_CommGKService->SendIpServiceParamInd(&dynamicLocal);
				}

				// Send to Sip Proxy process
				if (isSipProxychanged)
					m_CommProxyService->SendIpServiceParamInd(&dynamicLocal);
				
			}
		}
	}

	CIPService copyOfUpdatedService = *pUpdatedService;
	CIPService copyOfStaticService = *pStaticIpService;
	//BRIDGE-8284
	bool sendAlarm = false;

	sendAlarm =	!AreServicesEqual(copyOfUpdatedService,
			copyOfStaticService, "producing System Alert", isFromEMA);

	pIpServListStatic->UpdateOccupiedSpan(pUpdatedService);

	if (sendAlarm) {
		pIpServListStatic->Update(*pUpdatedService);
	}

	if (!sendAlarm) {
		BYTE bMSEnviroment = FALSE;
		int sipServerConfig =
				pUpdatedService->GetSip()->GetConfigurationOfSIPServers();
		if (sipServerConfig == eConfSipServerManually) {
			int sipServerType = pUpdatedService->GetSip()->GetSipServerType();
			if (sipServerType == eSipServer_ms)
				bMSEnviroment = TRUE;
		}

		if (isSipProxychanged && bMSEnviroment && !isIceTheOnlyChange)
			sendAlarm = TRUE;
	}

	pIpServListStatic->WriteXmlFile(ipServiceFileNameTmp.c_str());

	// Always send Active Alarm and ask for MCU reset if V35 is changed
	if (sendAlarm || isV35Changed) {
		AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
				SYSTEM_CONFIGURATION_CHANGED, MAJOR_ERROR_LEVEL,
				"System configuration changed. Please reset the MCU.", true,
				true);

		TRACESTR(eLevelInfoNormal)<< "BRIDGE-8284 set status to warning (ask to restart mcu)";
		status = STATUS_OK_WARNING;
	}

	else // auto-reset Apache only if '!sendAlarm' (otherwise the whole RMX should be reset anyway); similar to VNGR-18955
	if (true == isResetApache)
		SendV35GwUpdateIndToMcuMngr(*pUpdatedService);
    ///BRIDGE-5683 jitc because of sip registration problems to redcom untill issues resolve we must perform reset.
	//if(isFromEMA)
	//	status = STATUS_OK_WARNING;

	return status; // STATUS_OK;*/
}

void CCSMngrManager::SendV35GwUpdateIndToMcuMngr(CIPService& theService) {
	TRACESTR(eLevelInfoNormal)
			<< "\nCCSMngrManager::SendV35GwUpdateIndToMcuMngr";

//    BOOL isV35GwEnabled = theService.GetIsV35GwEnabled();
    BOOL isV35GwEnabled = FALSE;
    WORD boardId=0, pqId=0;
    //RetrieveV35GwSpanParams(theService, boardId, pqId);

    //TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::SendV35GwUpdateIndToMcuMngr";
						   //<< "\nIs Enabled: " << (isV35GwEnabled ? "yes" : "no")
						   //<< "\nBoardId: " << boardId << ", PQ Id: " << pqId;
    CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
   	BOOL isSSLSockets = NO;
   	sysConfig->GetBOOLDataByKey("V35_USE_SSL_PORTS", isSSLSockets);

    CIPServiceList *pServicesList = pCSMngrProcess->GetIpServiceListStatic();
    if (!pServicesList)
    	return;
    WORD  counter =0;
    CSegment  v35Seg;
   	CIPService *pService = pServicesList->GetFirstService();
   	int portIndex=0;
    while(NULL != pService)
    {
       	if ( TRUE == pService->GetIsV35GwEnabled() )
       	{
       		isV35GwEnabled = TRUE;
       		WORD boardId=0, pqId=0;
       	    RetrieveV35GwSpanParams(*pService, boardId, pqId);
       	    if(portIndex < MAX_RVGW_PORTS)
       	    {
       	    	if(isSSLSockets)
       	    		pService->SetV35GwPort(rvgwSslPorts[portIndex]);
       	    	pService->SetV35GwAlias(rvgwAliasNames[portIndex]);
       	    	
       	    	v35Seg << boardId << pqId << rvgwAliasNames[portIndex]<< rvgwSslPorts[portIndex]        	    	       
       	    	         <<pService->GetV35GwUsername() << pService->GetV35GwPassword_dec();
       	    	portIndex++;
       	    	counter++;
       	    }
       	}
       	pService = pServicesList->GetNextService();
   }

    CSegment* pSeg = new CSegment;
    if(isV35GwEnabled)
    {
    	*pSeg << isV35GwEnabled
    		  << counter
    		  << v35Seg;
    }
    else
    {
    	//*pSeg << isV35GwEnabled;
    	*pSeg << isV35GwEnabled << counter << boardId << pqId << theService.GetName();
    }
	/**pSeg << isV35GwEnabled
		  << boardId
		  << pqId;
	 */
	CManagerApi apiMcuMngr(eProcessMcuMngr);
    apiMcuMngr.SendMsg(pSeg, CSMNGR_TO_MCUMNGR_V35GW_UPDATE_IND);
}

void CCSMngrManager::RetrieveV35GwSpanParams(CIPService& theService,
		WORD& boardId, WORD& pqId) {
	// Looking for the first (and only) media board Span that is configured.
	// This should be the Span to which the V35 GW is attached
	// (however the first span is for Signaling, so the search should start from the 2nd span)
	CIPSpan* pSpan = theService.GetFirstSpan();
	pSpan = theService.GetNextSpan();

	bool bIpV4AddressExists = false, bIpV6AddressExists = false;

	for (int i = 1; i < MAX_SPAN_NUMBER_IN_SERVICE; i++) {
		if (!pSpan) {
			TRACESTR(eLevelInfoNormal)
					<< "\nCCSMngrManager::RetrieveV35GwSpanParams - no span!"
					<< "\nSpan idx: " << i << "\nBoardId: " << boardId
					<< ", PQ Id: " << pqId;
			break;
		}

		// searching for IPv4 address
		bIpV4AddressExists = false;
		if (pSpan->GetIPv4Address() != 0)
			bIpV4AddressExists = true;

		// if an address is configured, it means that this is the Span to which the V35 GW is attached
		if (bIpV4AddressExists /* || bIpV6AddressExists*/) {
			CalculateBoardIdPqNum(i, boardId, pqId);
			TRACESTR(eLevelInfoNormal)
					<< "\nCCSMngrManager::RetrieveV35GwSpanParams"
					<< "\nSpan idx: " << i << "\nBoardId: " << boardId
					<< ", PQ Id: " << pqId;

			break;
		}

		pSpan = theService.GetNextSpan();
	}

	return;
}

void CCSMngrManager::CalculateBoardIdPqNum(int spanIdx, WORD& boardId,
		WORD& pqId) {
	switch (spanIdx) {
	case 1: {
		boardId = FIXED_BOARD_ID_MEDIA_1;
		pqId = 1;
		break;
	}
	case 2: {
		boardId = FIXED_BOARD_ID_MEDIA_1;
		pqId = 2;
		break;
	}
	case 3: {
		boardId = FIXED_BOARD_ID_MEDIA_2;
		pqId = 1;
		break;
	}
	case 4: {
		boardId = FIXED_BOARD_ID_MEDIA_2;
		pqId = 2;
		break;
	}
	case 5: {
		boardId = FIXED_BOARD_ID_MEDIA_3;
		pqId = 1;
		break;
	}
	case 6: {
		boardId = FIXED_BOARD_ID_MEDIA_3;
		pqId = 2;
		break;
	}
	case 7: {
		boardId = FIXED_BOARD_ID_MEDIA_4;
		pqId = 1;
		break;
	}
	case 8: {
		boardId = FIXED_BOARD_ID_MEDIA_4;
		pqId = 2;
		break;
	}
	default: {
		boardId = 0;
		pqId = 0;
		PASSERT(1);
	}
	} // end switch
}

STATUS CCSMngrManager::SendSyncLockMsgToConfPartyProcess() {
	PTRACE(eLevelInfoNormal,
			"CCSMngrManager::SendSyncLockMsgToConfPartyProcess()");

	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE) TRUE;
	*pSeg << (BYTE) eConfBlockReason_CSMngr_Position_2;

	CSegment rspMsg;
	OPCODE rspOpcode;
	OPCODE outOpCode = CONF_BLOCK_IND;

	BYTE success = 0;
	CTaskApi confPartyManagerApi(eProcessConfParty, eManager);
	STATUS respStatus = confPartyManagerApi.SendMessageSync(pSeg, outOpCode,
			(int) (2 * SECOND), rspOpcode, rspMsg);
	if (STATUS_OK == respStatus) {
		// verify response message
		if (0 < rspMsg.GetLen()) {
			rspMsg >> success;
			if (STATUS_OK != success) {
				PTRACE(eLevelInfoNormal,
						"SendSyncLockUnlockMsgToConfPartyProcess() : ConfParty returned false");
				respStatus = STATUS_ILLEGAL;
			}
		} else {
			PTRACE(eLevelInfoNormal,
					"SendSyncLockUnlockMsgToConfPartyProcess() : ConfParty returned error msg");
			respStatus = STATUS_ILLEGAL;
		}
	} else // timeout or dead process
	{
		m_CommConfService->SendReleaseMsgReq();
		PASSERTMSG(
				respStatus,
				"CCSMngrManager::SendSyncLockUnlockMsgToConfPartyProcess : CONF LOCK TIME OUT !!! ");
	}

	return respStatus;
}

STATUS CCSMngrManager::SetDefaultIPService(CRequest* pRequest) {
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	if (pRequest->GetAuthorization() != SUPER) {
		FPTRACE(eLevelInfoNormal,
				"CCSMngrManager::SetDefaultIPService: No permission to set default ip service");
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CIPServiceDel* pStringWrapper =
			(CIPServiceDel*) pRequest->GetRequestObject();
	if (NULL == pStringWrapper) {
		FPTRACE(eLevelInfoNormal,
				"CCSMngrManager::SetDefaultIPService: empty request");
		pRequest->SetStatus(STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS);
		return STATUS_OK;
	}

	const char* defaultServiceName = pStringWrapper->GetIPServiceName();
	if (NULL == defaultServiceName) {
		FPTRACE(eLevelInfoNormal,
				"CCSMngrManager::SetDefaultIPService: no name come from EMA");
		pRequest->SetStatus(STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS);
		return STATUS_OK;
	}

	CIPServiceList* pIpServListStatic =
			pCSMngrProcess->GetIpServiceListStatic();
	pIpServListStatic->SetH323DefaultName(defaultServiceName);
	pIpServListStatic->WriteXmlFile(GetIpServiceTmpFileName().c_str());

	// Send default service update to resources and ConfParty
	m_CommRcrsService->SendDefaultService();
	m_CommConfService->SendDefaultService();

	return STATUS_OK;
}

STATUS CCSMngrManager::SetDefaultSIPService(CRequest* pRequest) {
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	if (pRequest->GetAuthorization() != SUPER) {
		FPTRACE(eLevelInfoNormal,
				"CCSMngrManager::SetDefaultSIPService: No permission to set default ip service");
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CIPServiceDel* pStringWrapper =
			(CIPServiceDel*) pRequest->GetRequestObject();
	if (NULL == pStringWrapper) {
		FPTRACE(eLevelInfoNormal,
				"CCSMngrManager::SetDefaultIPService: empty request");
		pRequest->SetStatus(STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS);
		return STATUS_OK;
	}

	const char* defaultServiceName = pStringWrapper->GetIPServiceName();
	if (NULL == defaultServiceName) {
		FPTRACE(eLevelInfoNormal,
				"CCSMngrManager::SetDefaultIPService: no name come from EMA");
		pRequest->SetStatus(STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS);
		return STATUS_OK;
	}

	CIPServiceList* pIpServListStatic =
			pCSMngrProcess->GetIpServiceListStatic();
	pIpServListStatic->SetSIPDefaultName(defaultServiceName);
	pIpServListStatic->WriteXmlFile(GetIpServiceTmpFileName().c_str());

	// Send default service update to resources and ConfParty
	m_CommRcrsService->SendDefaultService();
	m_CommConfService->SendDefaultService();

	return STATUS_OK;
}

void CCSMngrManager::SendSnmpCsInterface(DWORD serviceId) {
	CIPServiceList* list = pCSMngrProcess->GetIpServiceListDynamic();
	if (NULL == list) {
		PASSERTMSG(TRUE, "No dynamic service list");
		return;
	}

	CIPService* pServiceDynamic = list->GetService(serviceId);
	if (NULL == pServiceDynamic) {
		PASSERTMSG(TRUE, "No dynamic service");
		return;
	}

	STATUS status = m_CommSnmpService->SendIpServiceParamInd(pServiceDynamic);
	CCSMngrProcess::TraceToLogger(
			"CCSMngrManager::OnCsGKClearAltPropertiesReq",
			"to send IPS to SNMP process", status);
}
void   CCSMngrManager::SetMngntIpParamsMcmsNetwork()
{
	  pCSMngrProcess->SetSysIpType(pCSMngrProcess->m_NetSettings.m_iptype);
	  pCSMngrProcess->SetSysIPv6ConfigType(pCSMngrProcess->m_NetSettings.m_ipv6ConfigType);
	  pCSMngrProcess->SetMngmntAddress_IPv4(pCSMngrProcess->m_NetSettings.m_ipv4);

	  std::string sIpv6_0,sIpv6_1,sIpv6_2,sIpv6_defGW,sIpv6mask;
	  pCSMngrProcess->m_NetSettings.ConvertIpv6AddressToString( pCSMngrProcess->m_NetSettings.m_ipv6_0,sIpv6_0,sIpv6mask);


	  //sIpv6_0 = sIpv6_0 +"/" +sIpv6mask;
	  sIpv6_0 =formatString("%s/%s",sIpv6_0.c_str(),sIpv6mask.c_str());

	  pCSMngrProcess->SetMngmntAddress_IPv6(0, sIpv6_0);
	  pCSMngrProcess->m_NetSettings.ConvertIpv6AddressToString( pCSMngrProcess->m_NetSettings.m_ipv6_1,sIpv6_1,sIpv6mask);
	  sIpv6_1 =  formatString("%s/%s",sIpv6_1.c_str(),sIpv6mask.c_str());
	  //sIpv6_1 = sIpv6_1 +"/" +sIpv6mask;
	  pCSMngrProcess->SetMngmntAddress_IPv6(1, sIpv6_1);
	  pCSMngrProcess->m_NetSettings.ConvertIpv6AddressToString( pCSMngrProcess->m_NetSettings.m_ipv6_2,sIpv6_2,sIpv6mask);
	  sIpv6_2 = formatString("%s/%s",sIpv6_2.c_str(),sIpv6mask.c_str());
	  //sIpv6_2 = sIpv6_2 +"/" +sIpv6mask;
	  pCSMngrProcess->SetMngmntAddress_IPv6(2, sIpv6_2);
	  pCSMngrProcess->m_NetSettings.ConvertIpv6AddressToString( pCSMngrProcess->m_NetSettings.m_ipv6_DefGw,sIpv6_defGW,sIpv6mask);
	  sIpv6_defGW = formatString("%s/%s",sIpv6_defGW.c_str(),sIpv6mask.c_str());
	  //sIpv6_defGW = sIpv6_defGW +"/" +sIpv6mask;
	  pCSMngrProcess->SetMngmntDefaultGatewayIPv6(sIpv6_defGW);
	  pCSMngrProcess->SetMngmntDefaultGatewayMaskIPv6( pCSMngrProcess->m_NetSettings.m_ipv6_DefGw.mask);

      eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	  if (curProductType != eProductTypeSoftMCUMfw && curProductType != eProductTypeEdgeAxis)
        pCSMngrProcess->SetIpv6Params(pCSMngrProcess->m_NetSettings.m_ipv4,sIpv6_0, sIpv6_1, sIpv6_2, sIpv6_2,
			  	  	  	  	  	    pCSMngrProcess->m_NetSettings.m_ipv6_0.mask,
			  	  	  	  	  	    pCSMngrProcess->m_NetSettings.m_ipv6_1.mask,
			  	  	  	  		    pCSMngrProcess->m_NetSettings.m_ipv6_2.mask,
	                                pCSMngrProcess->m_NetSettings.m_ipv6_DefGw.mask,
	                                m_CommConfService, m_CommSnmpService,FALSE);

	  m_sysV6ConfigurationType = pCSMngrProcess->m_NetSettings.m_ipv6ConfigType;
	  m_sysIpType			   = pCSMngrProcess->m_NetSettings.m_iptype;

	  if(eServerStatusSpecify == pCSMngrProcess->m_NetSettings.m_ServerDnsStatus)
		 pCSMngrProcess->SetMngmntDnsStatus(TRUE);
	  else
		  pCSMngrProcess->SetMngmntDnsStatus(FALSE);

	  char  Ipv4[IP_ADDRESS_LEN];
	  memset(Ipv4,0,sizeof(Ipv4));
	  SystemDWORDToIpString(pCSMngrProcess->m_NetSettings.m_ipv4DnsServer,Ipv4);
	  pCSMngrProcess->SetMngmntDnsIpV4Address(Ipv4);

	  pCSMngrProcess->m_NetSettings.ConvertIpv6AddressToString( pCSMngrProcess->m_NetSettings.m_ipv6_DnsServer,sIpv6_0,sIpv6mask);
	  sIpv6_0 = sIpv6_0 +"/" +sIpv6mask;
	  pCSMngrProcess->SetMngmntDnsIpV6Address((char*)sIpv6_0.c_str());

	  m_mcuMngrIpTypeIndAlreadyReceived = true;
	  m_flagIsIpTypeReceived = true;
	  pCSMngrProcess->SetIpTypeReceivedStatus(TRUE);

}



void CCSMngrManager::ManagerPostInitActionsPoint() {
	TestAndEnterFipsMode(); // for using encryption (for V35GW password)

	AllocHeapMemory();

	std::string ipServiceFileName, ipServiceFileNameTmp;

	BYTE bSystemMode = pCSMngrProcess->GetIpServiceFileNames(ipServiceFileName,
			ipServiceFileNameTmp);

	BOOL bUpgradeFromOldCfgToNewOne = FALSE;
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	BOOL bUpdateXmlFields = FALSE;
	BOOL bCompareXmlFields = FALSE;

    TRACESTR(eLevelInfoNormal) << "CCSMngrManager::ManagerPostInitActionsPoint curProductType=" << curProductType;
        
	if (curProductType == eProductTypeSoftMCU || curProductType == eProductTypeSoftMCUMfw || eProductTypeEdgeAxis == curProductType || curProductType == eProductTypeCallGeneratorSoftMCU)
	{
		//pCSMngrProcess->ReadSystemInterfaceList();

		//if default file does not exist - create a new file.
		if ((!(IsFileExists(IP_SERVICE_LIST_PATH))) && (!(IsFileExists(
				IP_SERVICE_LIST_TMP_PATH)))) {
			TRACESTR(eLevelInfoNormal) << "CCSMngrManager::ManagerPostInitActionsPoint create new file";
			//create the file
			CSysConfig::SwitchCfgFiles(IP_SERVICE_LIST_PATH,
					IP_SERVICE_LIST_TMP_PATH,
					IP_VERSION_CFG_SOFT_MCU_SERVICES_LIST_PATH,
					IP_VERSION_CFG_SOFT_MCU_SERVICES_LIST_PATH, TRUE);
			bUpdateXmlFields = TRUE;
		} else {
			/*			  std::string strCloudIp = GetCloudIp();

			 if (strCloudIp.length() > 0)
			 bUpdateXmlFields = TRUE;
			 else*/
			bCompareXmlFields = TRUE; //Check that the details still valid.
		}
	}
	// relevant form v7.1 that doesn't support 2 span in one card to V7G

	if (bSystemMode == eSystemMode_Multiple_services) {
		BOOL bUpgradeFrom1SpanToTwo = CSysConfig::SwitchCfgFiles(
				IP_MULTIPLE_SERVICES_TWO_SPANS_LIST_PATH,
				IP_MULTIPLE_SERVICES_TWO_SPANS_LIST_TMP_PATH,
				IP_MULTIPLE_SERVICES_LIST_PATH,
				IP_MULTIPLE_SERVICES_LIST_TMP_PATH, FALSE);

		if (bUpgradeFrom1SpanToTwo == FALSE) // didn't switch the files
			bUpgradeFromOldCfgToNewOne = CSysConfig::SwitchCfgFiles(
					IP_MULTIPLE_SERVICES_TWO_SPANS_LIST_PATH,
					IP_MULTIPLE_SERVICES_TWO_SPANS_LIST_TMP_PATH,
					IP_SERVICE_LIST_PATH, IP_SERVICE_LIST_TMP_PATH, TRUE);
	} else if (bSystemMode == eSystemMode_Jitc_v35)
		CSysConfig::SwitchCfgFiles(IP_SERVICES_JITC_V35_LIST_PATH,
				IP_SERVICES_JITC_V35_LIST_TMP_PATH, IP_SERVICE_LIST_PATH,
				IP_SERVICE_LIST_TMP_PATH, TRUE);
	else {
        TRACESTR(eLevelInfoNormal) 
            << "CCSMngrManager::ManagerPostInitActionsPoint bSystemMode=" << bSystemMode;
        
		BOOL bCopyMSFileToOneServiceFile = IsToCopyMSFileToOneServiceFile();
		if (bCopyMSFileToOneServiceFile) {
			TRACESTR(eLevelInfoNormal)
					<< "CCSMngrManager::ManagerPostInitActionsPoint - copy MS file";
			CSysConfig::SwitchCfgFiles(IP_SERVICE_LIST_PATH,
					IP_SERVICE_LIST_TMP_PATH, IP_MULTIPLE_SERVICES_LIST_PATH,
					IP_MULTIPLE_SERVICES_LIST_TMP_PATH, TRUE);
		} else {
		    TRACESTR(eLevelInfoNormal) 
                    << "CCSMngrManager::ManagerPostInitActionsPoint switchCfgFiles";
			CSysConfig::SwitchCfgFiles(IP_SERVICE_LIST_PATH,
					IP_SERVICE_LIST_TMP_PATH);
		}
	}

	// ===== 1. init the services
	CIPServiceList* pIpServListStatic = pCSMngrProcess->GetIpServiceListStatic();
	PASSERT_AND_RETURN(pIpServListStatic==NULL);

	pIpServListStatic->ReadXmlFile(ipServiceFileName.c_str(), eNoActiveAlarm,
			eRenameFile);
   
	//for SoftMCU - update router and cpu fields
	//IP_SERVICE_LIST_PATH and IP_SERVICE_LIST_TMP_PATH not exsit
	if (bUpdateXmlFields)
	{
		pIpServListStatic->UpdateServiceListDefaults();
	}
	//for softMCU - check if the ip addresses changed from the last startup
	else if (bCompareXmlFields) {
		if (pIpServListStatic->CompareServiceListValues() == TRUE) {
			//raise AA that interface doesn't exist anymore. this AA can be remove only after updating the service and reset teh service.
			AddActiveAlarmSingleton(
					FAULT_GENERAL_SUBJECT,
					NETWORK_INTERFACE_IS_NOT_CONFIGURED,
					MAJOR_ERROR_LEVEL,
					"Network interface is not configured. Please, update the network service with exist ip address and restart the soft mcu service",
					true, true);
		}
	}

	if (bUpgradeFromOldCfgToNewOne) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::ManagerPostInitActionsPoint - first time after upgrade. Copy old service files to new service configuration files";
		CopyOCSFilesToRightLocation(); // happened only on the first upgrade
	}

	DecryptV35GwPasswordsFromFile(*pIpServListStatic);
	DecryptGKAuthenticationPasswordsFromFile(*pIpServListStatic);

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	CIceStandardParams* pIceStandardParams = NULL;
	if (pIpServListStatic && (pIpServListStatic->GetFirstService()) && (pIpServListStatic->GetFirstService()->GetpSipAdvanced()))
	{
		pIceStandardParams = pIpServListStatic->GetFirstService()->GetpSipAdvanced()->GetpIceStandardParams();
	}
//	FTRACESTR(eLevelInfoNormal) << "CCSMngrManager::ManagerPostInitActionsPoint examin stun configuration";
//	if (pIceStandardParams->GetSTUNServerPort() == DEFAULT_ICE_STUN_TURN_SERVER_PORT)
//	{

//		if (stun_port != DEFAULT_ICE_STUN_TURN_SERVER_PORT)
//		{

	if (eProductTypeSoftMCUMfw == curProductType)
	{
		DWORD stun_port;
		if ((sysConfig->GetDWORDDataByKey("STUN_SERVER_PORT",stun_port)) && pIceStandardParams)
		{
			pIceStandardParams->SetSTUNServerPort(stun_port);
			TRACESTR(eLevelInfoNormal)<< "Setting custom stun port  " << stun_port;
		}

		DWORD turn_port;
		if ((sysConfig->GetDWORDDataByKey("TURN_SERVER_PORT",turn_port))  && pIceStandardParams)
		{
			pIceStandardParams->SetTURNServerPort(turn_port);
			TRACESTR(eLevelInfoNormal)<< "Setting custom turn port  " << stun_port;
		}

		FTRACESTR(eLevelInfoNormal) << "CCSMngrManager::ManagerPostInitActionsPoint turn on to standard";
		//pIpServListStatic->GetFirstService()->GetpSipAdvanced()->SetIceEnvironment(eIceEnvironment_Standard);
	}

	BOOL bJITCMode;
	sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJITCMode);
	if (bJITCMode) {
		CLargeString errorMsg;
		int numOfTlsOccurrences =
				CIpServiceListValidator(*pIpServListStatic).RemoveSipTls(
						errorMsg);
		if (numOfTlsOccurrences && pIpServListStatic)
			pIpServListStatic->WriteXmlFile(ipServiceFileNameTmp.c_str());

	}

	if (pIpServListStatic)
	{
		pIpServListStatic->SetUpdateCounter(0);
	}

	CIPServiceList* pIpServListDynamic =
			pCSMngrProcess->GetIpServiceListDynamic();
	*pIpServListDynamic = *pIpServListStatic;

	//save the files updated details, if they were just created
	if ((bUpdateXmlFields || bCompareXmlFields) && eProductTypeEdgeAxis != curProductType)
	{
		pIpServListStatic->WriteXmlFile(ipServiceFileNameTmp.c_str());
		pIpServListDynamic->WriteXmlFile(ipServiceFileName.c_str());
	}
	//Begin Startup Feature
	pCSMngrProcess->m_NetSettings.LoadFromFile();
	SetMngntIpParamsMcmsNetwork();
	//End Startup Feature

	CIPService* pService = pIpServListDynamic->GetFirstService();
	BYTE bAlreadyRetrieveIPv6AddressesInAutoMode = FALSE;
	std::string srv_name;

	while (pService) {
		srv_name = pService->GetName();

		//if IPv4 Address is 0.0.0.0, send alarm
		eIpType curIpType = pService->GetIpType();

		if((eIpType_Both == curIpType) && (eProductTypeSoftMCU == curProductType || eProductTypeEdgeAxis == curProductType))
		{
			//in case of smcu, if ipv6 address is not available and mcu is in both ipv4 and ipv6 mode, then change the mode to ipv4 only to allow
			//the system to work properly.
		    string iPv6_str="";
		    SystemPipedCommand("echo -n `/sbin/ifconfig | grep Scope:Global | grep 'inet6 addr:' | cut -d':' -f2- | cut -d' ' -f2 | awk 'NR==1'`", iPv6_str);
		    if(iPv6_str == "")
		    {
		    	pService->SetIpType(eIpType_IpV4);
		    	TRACESTR(eLevelInfoNormal) << "ipv6 address is not available, set service to ipv4 mode only";
		    }
		}

		CIPSpan * csSpan = pService->GetFirstSpan();
		if((NULL == csSpan) ||
			((eIpType_IpV6 != curIpType) && (NULL != csSpan) &&  (0 == csSpan->GetIPv4Address()) && (eProductTypeSoftMCUMfw == curProductType)))
		{
            CSmallString sDesc = "Configuring service: ";
			sDesc << pService->GetName() << ", Signaling IP address is invalid.";
                   TRACESTR(eLevelInfoNormal) << sDesc.GetString();                                             
			AddActiveAlarmSingleton(
			                FAULT_GENERAL_SUBJECT,
			                CS_IP_IS_ZERO,
			                MAJOR_ERROR_LEVEL,
			                sDesc.GetString(),
			                true, true);
		}
	
		//modified for gesher/ninja
		eProductType curProductType =
				CProcessBase::GetProcess()->GetProductType();
		if ((eProductTypeGesher == curProductType || eProductTypeNinja
				== curProductType) && eIpServiceType_Signaling
				== pService->GetIpServiceType()) {
			AddCSMediaIpInterfacesGesherNinja(pService, bSystemMode,
					curProductType);
		}

		//end
		// ===== 2. on IPv6_Auto mode, the addresses should be read from the interface
		eIpType ipType = pService->GetIpType();
		eV6ConfigurationType configType = pService->GetIpV6ConfigurationType();
		if (((eIpType_IpV6 == ipType) || (eIpType_Both == ipType))
				&& (eV6Configuration_Auto == configType || eProductTypeSoftMCU == curProductType || eProductTypeSoftMCUMfw == curProductType  || eProductTypeEdgeAxis == curProductType)
				&& !bAlreadyRetrieveIPv6AddressesInAutoMode
				&& !pService->GetIsV35GwEnabled()) 
	    {
            TRACESTR(eLevelInfoNormal) << "CCSMngrManager::ManagerPostInitActionsPoint IPV6 CS AutoMode";
			bAlreadyRetrieveIPv6AddressesInAutoMode = TRUE;
			pCSMngrProcess->RetrieveIPv6AddressesInAutoMode(ipType);
		}

		// ===== 3. config Dns (resolving)
		// in case of (JITCMode && ManagmentSeparation), the resolving is done by CSMngr (with IpService params)
		// instead of by McuMngr (with MngmntNetworkService params)

		pIpServListDynamic = pCSMngrProcess->GetIpServiceListDynamic(); // re-retrieving the service, since the pointer was changed within <RetrieveIPv6AddressesInAutoMode> method

		/*if (pIpServListDynamic) {
			CIPService* pService = pIpServListDynamic->GetService(
					srv_name.c_str());

			if (pService) {
				CIpDns* pTmpDns = pService->GetpDns();

				if (pTmpDns) {
					eServerStatus dnsRegistrationMode = pTmpDns->GetStatus();
					if ((eServerStatusSpecify == dnsRegistrationMode) && (true
							== ::IsJitcAndNetSeparation()))
						::ConfigDnsInOS(pService,
								"CCSMngrManager::ManagerPostInitActionsPoint");
				} // end if pTmpDns
			}
			//deleted because this meaningless call move the m_ind_serv++ and jumped over one service. R.D.
			//pService = pIpServListDynamic->GetNextService();
		} else {
			break;
		}*/

		pService = pIpServListDynamic->GetNextService();
	} // end while pService

	if (IsTarget() || eProductTypeGesher == curProductType || eProductTypeNinja
			== curProductType)
		StartMultipleCS();

	pIpServListStatic->UpdateEnableDisableFlag();

	//Send SNMP's ready to SNMPProcess
	StartTimer(CSMNGR_GET_SERVICE_INFO_TIMER,
			CSMNGR_GET_SERVICE_INFO_TIMER_TIME_OUT_VALUE);

	CManagerApi api(eProcessSNMPProcess);
	CSegment *pSeg = new CSegment;
	DWORD type = eProcessCSMngr;
	*pSeg << type;
	api.SendMsg(pSeg, SNMP_OTHER_PROCESS_READY);

}

void CCSMngrManager::StartMultipleCS() {
	FTRACESTR(eLevelInfoNormal) << "CCSMngrManager::StartMultipleCS";

	CIPServiceList* pIpServListStatic =
			pCSMngrProcess->GetIpServiceListStatic();
	pIpServListStatic->CalcMaxNumOfPorts();
	pCSMngrProcess->GetIpServiceListDynamic()->CalcMaxNumOfPorts();

	CSegment* pSeg = new CSegment;

	*pSeg << (WORD) pIpServListStatic->GetServiceNumber();

	for (CIPService* service = pIpServListStatic->GetFirstService(); service
			!= NULL; service = pIpServListStatic->GetNextService()) {
		*pSeg << (DWORD) service->GetId() << (WORD) service->GetMaxNumOfCalls();
	}

	CManagerApi apiMcmsDaemon(eProcessMcmsDaemon);
	apiMcmsDaemon.SendMsg(pSeg, CSMNGR_TO_MCMSDAEMON_START_SIGNALING);
}

void CCSMngrManager::ManagerStartupActionsPoint() {
	if (STATUS_OK == m_StatusReadIpServiceList) {
		CIPServiceList* pIpServList = pCSMngrProcess->GetIpServiceListDynamic();
		if (0 == pIpServList->GetServiceNumber())
			AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
					NO_SERVICE_FOUND_IN_DB, MAJOR_ERROR_LEVEL,
					"IP Service not found in the Network Services list", true,
					true);
		else
			RemoveActiveAlarmByErrorCode(NO_SERVICE_FOUND_IN_DB);
	} else {
		std::string strStatus = pCSMngrProcess->GetStatusAsString(
				m_StatusReadIpServiceList);
		AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT, NO_SERVICE_FOUND_IN_DB,
				MAJOR_ERROR_LEVEL, strStatus.c_str(), true, true);
	}
}

void CCSMngrManager::OnSysConfigTableChanged(const std::string& key,
		const std::string& data) {
	if (CFG_KEY_DEBUG_MODE == key) {
		CSmallString logStr = "\nCCSMngrManager::OnSysConfigTableChanged - ";
		logStr << "Key: " << key.c_str() << ", Value: " << data.c_str();
		TRACESTR(eLevelInfoNormal) << logStr.GetString();

		SendDebugModeChangedToAllCS(data);
	}
}

void CCSMngrManager::SendDebugModeChangedToAllCS(const std::string& data) {
	CSmallString logStr = "CCSMngrManager::SendDebugModeChangedToAllCS - ";

	CSysConfig* sysConfig = pCSMngrProcess->GetSysConfig();
	const CCfgData* cfgData = sysConfig->GetCfgEntryByKey(CFG_KEY_DEBUG_MODE);
	if (false == CCfgData::TestValidity(cfgData)) {
		logStr << "failed to validate sysConfig, key: " << CFG_KEY_DEBUG_MODE;
		TRACESTR(eLevelInfoNormal) << logStr.GetString();
		return;
	}

	string dataStr;
	sysConfig->GetDataByKey(CFG_KEY_DEBUG_MODE, dataStr);

	BOOL isDebugMode = FALSE;
	BOOL res = sysConfig->GetBOOLDataByKey(CFG_KEY_DEBUG_MODE, isDebugMode);
	if (FALSE == res)
		logStr << "Illegal DebugMode flag: " << dataStr.c_str()
				<< "! nothing is sent to CS";

	else {
		logStr << "DebugMode flag: " << dataStr.c_str();

		for (int i = 0; i < MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++) {
			COsQueue* queue = pCSMngrProcess->GetSignalingMbx(i);
			if (queue)
				SendDebugModeChangedToSpecificSignaling(i, cfgData);
		}
	}

	TRACESTR(eLevelInfoNormal) << logStr.GetString();
}

void CCSMngrManager::SendDebugModeChangedToSpecificSignaling(const int csId,
		const CCfgData* cfgData) {
	m_CommStartupService->SetCsId(csId);
	STATUS sendStat = m_CommStartupService->SendConfigParamSingle(cfgData);

	if (STATUS_OK != sendStat)
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::SendDebugModeChangedToSpecificSignaling"
				<< "\nFailed sending to CS: " << csId << " Status: "
				<< sendStat;
}

STATUS CCSMngrManager::HandleTerminalBombi(CTerminalCommand& command,
		std::ostream& answer) {
	DWORD numOfMessages = 1024 * 100;
	if (0 < command.GetNumOfParams()) {
		const std::string& param1 = command.GetToken(eCmdParam1);
		numOfMessages = (DWORD) atoi(param1.c_str());
		if (numOfMessages > 1024 * 100) {
			TRACESTR(eLevelInfoNormal)
					<< "HandleTerminalBombi numOfMessages is maximum 1024*100";
			return STATUS_FAIL;
		}
	}

	for (int i = 0; i < MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++) {
		COsQueue* queue = pCSMngrProcess->GetSignalingMbx(i);
		if (queue)
			for (DWORD msg_ind = 0; msg_ind < numOfMessages; msg_ind++) {
				m_CommStartupService->SetCsId(i);
				m_CommStartupService->SendNewReq(42);
			}
	}

	return STATUS_OK;
}

STATUS CCSMngrManager::HandleTerminalBombiFor1(CTerminalCommand& command,
		std::ostream& answer) {
	DWORD numOfMessages = 1024 * 100;
	WORD csId = 0;

	if (1 < command.GetNumOfParams()) // need 2 params - 1 for the number of messages, and 2 for the csId
	{
		const string& param1 = command.GetToken(eCmdParam1);
		numOfMessages = atoi(param1.c_str());
		if (numOfMessages > 1024 * 100) {
			TRACESTR(eLevelInfoNormal)
					<< "HandleTerminalBombiFor1 numOfMessages is maximum 1024*100";
			return STATUS_FAIL;
		}

		const std::string& param2 = command.GetToken(eCmdParam2);
		csId = atoi(param2.c_str());
	}

	m_CommStartupService->SetCsId(csId);
	for (DWORD i = 0; i < numOfMessages; i++)
		m_CommStartupService->SendNewReq(42);

	return STATUS_OK;
}

void CCSMngrManager::OnSnmpInterfaceReq(CSegment* pMsg) {
	for (int i = 0; i < MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++) {
		COsQueue* queue = pCSMngrProcess->GetSignalingMbx(i);
		if (queue) {
			CSignalingApi api;
			api.CreateOnlyApi(*queue);
			api.SendOpcodeMsg(CSMNGR_SNMP_INTERFACE_REQ);
		}
	}
}

void CCSMngrManager::OnConfBlockInd(CSegment* pMsg) {
	TRACESTR(eLevelInfoHigh) << "CCSMngrManager::OnConfBlockInd"
			<< "\nThe response was received here instead of in the right mbx";
}

STATUS CCSMngrManager::SetPing(CRequest* pRequest) {
	CPingSet* pReqPingData = (CPingSet*) pRequest->GetRequestObject();
	CPingSet* pPingData = new CPingSet(*pReqPingData);

	if (pRequest->GetAuthorization() != SUPER) {
		FPTRACE(eLevelInfoNormal,
				"CCSMngrManager::SetDefaultSIPService: No permission to set ping");
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		pRequest->SetConfirmObject(new CPingSet(*pPingData));
		POBJDELETE(pPingData);
		return STATUS_OK;
	}

	if (NULL == pPingData) {
		FPTRACE(eLevelInfoNormal, "CCSMngrManager::SetPing: empty request");
		pRequest->SetStatus(STATUS_FAIL);
		pRequest->SetConfirmObject(new CPingSet(*pPingData));
		POBJDELETE(pPingData);
		return STATUS_OK;
	}

	char* dest = pPingData->GetDestination();

	if (NULL == dest || 0 == strcmp(dest, "")) {
		FPTRACE(eLevelInfoNormal, "CCSMngrManager::SetPing: empty destination");
		pRequest->SetStatus(STATUS_FAIL);
		pRequest->SetConfirmObject(new CPingSet(*pPingData));
		POBJDELETE(pPingData);
		return STATUS_OK;
	}

	if (!IsGoodHostName(dest) && !::isIpV6Str(dest) && !::isIpV4Str(dest)) {
		FPTRACE(eLevelInfoNormal,
				"CCSMngrManager::SetPing: invalid host name in destination");
		pRequest->SetStatus(STATUS_FAIL);
		pRequest->SetConfirmObject(new CPingSet(*pPingData));
		POBJDELETE(pPingData);
		return STATUS_OK;
	}

	ePingIpType ipType = pPingData->GetIpType();
	if (ipType != ePingIpType_IPv4 && ipType != ePingIpType_IPv6) {
		FPTRACE(eLevelInfoNormal, "CCSMngrManager::SetPing: invalid ip type");
		pRequest->SetStatus(STATUS_FAIL);
		pRequest->SetConfirmObject(new CPingSet(*pPingData));
		POBJDELETE(pPingData);
		return STATUS_OK;
	}

	if (ePingIpType_IPv4 == ipType && eIpType_IpV6 == m_sysIpType) {
		FPTRACE(eLevelInfoNormal,
				"CCSMngrManager::SetPing: wrong ip type, system is configured to IPV6 only!");
		pRequest->SetStatus(STATUS_FAIL);
		pRequest->SetConfirmObject(new CPingSet(*pPingData));
		POBJDELETE(pPingData);
		return STATUS_OK;
	}

	if (ePingIpType_IPv6 == ipType && eIpType_IpV4 == m_sysIpType) {
		FPTRACE(eLevelInfoNormal,
				"CCSMngrManager::SetPing: wrong ip type, system is configured to IPV4 only!");
		pRequest->SetStatus(STATUS_FAIL);
		pRequest->SetConfirmObject(new CPingSet(*pPingData));
		POBJDELETE(pPingData);
		return STATUS_OK;
	}

	STATUS pingStatus = m_pCsPinger->SetPing(pPingData);

	pRequest->SetConfirmObject(new CPingSet(*pPingData));
	pRequest->SetStatus(pingStatus);

	if (STATUS_OK == pingStatus)
		pCSMngrProcess->SetPing(pPingData);
	else
		POBJDELETE(pPingData);

	return STATUS_OK;
}

void CCSMngrManager::OnCsPingInd(CSegment* pMsg) {
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::OnCsPingInd";
	m_pCsPinger->DispatchEvent(CS_PING_IND, pMsg);
}

STATUS CCSMngrManager::HandleTerminalPing4(CTerminalCommand& command,
		std::ostream& answer) {
	return HandleTerminalPing(ePingIpType_IPv4, command, answer);
}

STATUS CCSMngrManager::HandleTerminalPing6(CTerminalCommand& command,
		std::ostream& answer) {
	return HandleTerminalPing(ePingIpType_IPv6, command, answer);
}

STATUS CCSMngrManager::HandleTerminalPing(ePingIpType ipType,
		CTerminalCommand& command, std::ostream& answer) {
	const DWORD numOfParams = command.GetNumOfParams();
	if (numOfParams != 2) {
		answer << "Error" << endl;
		answer << "Usage: ca ping4 [Destination] [csId]";
		return STATUS_OK;
	}

	const std::string& dest = command.GetToken(eCmdParam1);
	const std::string& service_name = command.GetToken(eCmdParam2);

	CIPServiceList* list = pCSMngrProcess->GetIpServiceListStatic();
	if (list == NULL) {
		answer << "Error" << endl;
		answer << "Ip service list is empty";
		return STATUS_OK;
	}

	CIPService* pService = list->GetService(service_name.c_str());
	if (pService == NULL) {
		answer << "Error" << endl;
		answer << "Ip service doesn't exist in Ip service list";
		return STATUS_OK;
	}

	CPingData* pPingData = new CPingData(ipType, dest.c_str());
	STATUS pingStatus = m_pCsPinger->SetPing(pPingData);
	answer << "pinging " << dest
			<< "- check log for result. immediate STATUS = " << pingStatus;
	return STATUS_OK;
}

STATUS CCSMngrManager::HandleTerminalUpdateIPServiceConfiguration(
		CTerminalCommand& command, std::ostream& answer) {
	const std::string& IP_address = command.GetToken(eCmdParam1);
	const std::string& mask = command.GetToken(eCmdParam2);
	const std::string& default_GW = command.GetToken(eCmdParam3);

	DWORD dwIpAddrr = SystemIpStringToDWORD(IP_address.c_str());
	DWORD dwMask = SystemIpStringToDWORD(mask.c_str());
	DWORD dwDefaultGW = SystemIpStringToDWORD(default_GW.c_str());

	// update service accordingly
	CIPServiceList* pIpServiceList = pCSMngrProcess->GetIpServiceListStatic();
	CIPService* pIpService;

	pIpService = pIpServiceList->GetFirstService();
	if (!pIpService) {
		pIpService = new CIPService;
		answer << "\nNo pIpServiceList->GetFirstService";

		pIpServiceList->Add(*pIpService);
	}

	pIpService = pIpServiceList->GetFirstService();

	pIpService->SetName("Default Auto IP Service");
	pIpService->SetIpServiceType(eIpServiceType_Signaling);
	pIpService->SetGatekeeper(GATEKEEPER_NONE);
	pIpService->SetNetMask(dwMask);
	pIpService->SetDefaultGatewayIPv4(dwDefaultGW);

	CIPSpan* pSpan;
	for (DWORD i = 0; i < ACTUAL_NUM_OF_SPANS_IN_SERVICE; i++) {
		pSpan = pIpService->GetSpanByIdx(i);
		if (!pSpan) {
			pSpan = new CIPSpan();
			pIpService->AddSpan_NoCheck(*pSpan);
			delete pSpan;
		}

		pSpan = pIpService->GetSpanByIdx(i);
		if (pSpan) {
			pSpan->SetLineNumber(i);
			pSpan->SetIPv4Address(dwIpAddrr);
		}
	}

	pIpServiceList->SetH323DefaultName("Default Auto IP Service");
	pIpServiceList->SetUdpPortRange(49152, 320);
	pIpServiceList->WriteXmlFile(GetIpServiceTmpFileName().c_str());
	return STATUS_OK;
}

BYTE CCSMngrManager::IsGoodHostName(const char* cName) {
	BYTE bResult = NO;
	bResult = (strcspn(cName, BAD_CHARACTERS_FOR_URI) == strlen(cName));
	return bResult;
}

void CCSMngrManager::ClearIPv6Addresses() {
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::ClearIPv6Addresses";

	// ===== 1. update static list
	CIPServiceList* pIpServListStatic =
			pCSMngrProcess->GetIpServiceListStatic();
	CIPService* pService = pIpServListStatic->GetFirstService();
	while (pService) {
		pService->ClearIPv6Addresses();
		pIpServListStatic->UpdateCounters();
		pIpServListStatic->WriteXmlFile(GetIpServiceTmpFileName().c_str());
		pService = pIpServListStatic->GetNextService();
	}

	// ===== 2. update dynamic list
	CIPServiceList* pIpServListDyn = pCSMngrProcess->GetIpServiceListDynamic();
	CIPService* pServiceD = pIpServListDyn->GetFirstService();
	while (pServiceD) {
		pServiceD->ClearIPv6Addresses();
		pIpServListDyn->UpdateCounters();

		pServiceD = pIpServListDyn->GetNextService();
	}
}

void CCSMngrManager::OnEPProcessStarted(void) {
	PASSERTMSG_AND_RETURN(IsTarget(), "The code should not run at target");
	PASSERT_AND_RETURN(NULL == pCSMngrProcess);

	SystemSleep(2 * SECOND, FALSE);

	// Reads IP services
	CIPServiceList* list = pCSMngrProcess->GetIpServiceListStatic();
	PASSERT_AND_RETURN(NULL == list);

	list->CalcMaxNumOfPorts();
	pCSMngrProcess->GetIpServiceListDynamic()->CalcMaxNumOfPorts();

	// Launches with appropriate CSSim number
	for (CIPService* service = list->GetFirstService(); service != NULL; service
			= list->GetNextService()) {
		std::ostringstream cmd;
		cmd << MCU_MCMS_DIR+"/Bin/McuCmd start_cs EndpointsSim " << service->GetId()
				<< " " << service->GetMaxNumOfCalls() << " 2>&1";

		std::string ans;
		STATUS stat = SystemPipedCommand(cmd.str().c_str(), ans);
		PASSERTSTREAM_AND_RETURN(stat != STATUS_OK, "SystemPipedCommand: "
				<< cmd.str() << ": " << ans);
	}
}

void CCSMngrManager::OnCardsToCsIceDetails(CSegment* pSeg) {
	//_M_S
	if (NULL == pSeg) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::OnCardsToCsIceDetails - Input segment is NULL";
		return;
	}

	TRACESTR(eLevelInfoHigh) << "\nCCSMngrManager::OnCardsToCsIceDetails";

	CIceInfo iceInfo;

	DWORD reqId = 0;
	WORD num_of_cards = 0, board_id = 0, sub_board_id = 0;

	// ===== 1. fill attribute with data from structure received
	*pSeg >> reqId;
	*pSeg >> num_of_cards;

	ICE_SERVER_TYPES_S iceServerTypes;
	pSeg->Get((BYTE*) &iceServerTypes, sizeof(ICE_SERVER_TYPES_S));

	iceInfo.SetIceServersType(iceServerTypes);

	ICE_INIT_IND_S ice_init_ind_array[num_of_cards <= 0 ? 1 : num_of_cards];

	//send the info to SipProxyMngr
	WORD nServiceId = 0;
	int arrCards[num_of_cards];
	for (int i = 0; i < num_of_cards; i++)
		arrCards[i] = -1;
	int arrServiceList[MAX_SERVICES_NUM + 1];
	for (int i = 0; i < MAX_SERVICES_NUM + 1; i++)
		arrServiceList[i] = 0;

	for (int i = 0; i < num_of_cards; i++) {
		*pSeg >> board_id;
		*pSeg >> sub_board_id;
		*pSeg >> nServiceId;
		pSeg->Get((BYTE*) &ice_init_ind_array[i], sizeof(ICE_INIT_IND_S));

		iceInfo.SetIceInitInd(ice_init_ind_array[i], board_id, sub_board_id);

		if (nServiceId > 0 && nServiceId <= MAX_SERVICES_NUM) {
			arrServiceList[nServiceId]++;
			arrCards[i] = nServiceId;
		}
	}

	// update signaling monitoring DB
	CIPServiceList* pIpServListDyn = pCSMngrProcess->GetIpServiceListDynamic();
	if (pIpServListDyn) {
		if (pIpServListDyn->GetService(iceServerTypes.service_id) != NULL) {
			TRACESTR(eLevelInfoNormal)
					<< "CCSMngrManager::OnCardsToCsIceDetails - Update signaling monitoring DB ServiceId = "
					<< iceServerTypes.service_id;
			pIpServListDyn->UpdateDynamic(iceServerTypes.service_id, iceInfo);

		}
	}

	// send the info to SipProxyMngr or to IceServiceMngr
	for (int nServiceIndex = 1; nServiceIndex < MAX_SERVICES_NUM + 1; nServiceIndex++) {
		if (pIpServListDyn) {
			if (arrServiceList[nServiceIndex] > 0
					&& pIpServListDyn->GetService(nServiceIndex) != NULL) {
				PTRACE2INT(
						eLevelInfoNormal,
						"CCSMngrManager::OnCardsToCsIceDetails - send the info to SipProxy ",
						nServiceIndex);
				CSegment* pParam = new CSegment;
				*pParam << reqId;
				*pParam << (WORD)(arrServiceList[nServiceIndex]);

				for (int j = 0; j < num_of_cards; j++) {
					if (nServiceIndex == arrCards[j])
						pParam->Put((BYTE*) &(ice_init_ind_array[j]),
								sizeof(ICE_INIT_IND_S));
				}

				if(eIceEnvironment_WebRtc != iceServerTypes.ice_env)
				{
					CSipProxyTaskApi api(nServiceIndex);
					api.SendMsg(pParam, UPDATE_ICE_END);
				} else {
					CIceTaskApi iceApi(nServiceIndex);
					iceApi.SendMsg(pParam, UPDATE_ICE_END);
				}
			}
		} else
			PASSERT(1);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////
void CCSMngrManager::OnCardsToCsIceDetailsUpdateStatus(CSegment* pSeg)
{

	if (NULL == pSeg)
	{
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::OnCardsToCsIceDetails - Input segment is NULL";
		return;
	}

	TRACESTR(eLevelInfoHigh) << "\nCCSMngrManager::OnCardsToCsIceDetailsUpdateStatus";

	CIceInfo iceInfo;

	DWORD reqId = 0;
	WORD num_of_cards = 0, board_id = 0, sub_board_id = 0;

	// ===== 1. fill attribute with data from structure received
	*pSeg >> num_of_cards;

	ICE_SERVER_TYPES_S iceServerTypes;
	pSeg->Get((BYTE*) &iceServerTypes, sizeof(ICE_SERVER_TYPES_S));

	iceInfo.SetIceServersType(iceServerTypes);

	ICE_INIT_IND_S ice_init_ind_array[num_of_cards <= 0 ? 1 : num_of_cards];
	ICE_STATUS_IND_S ice_stat_ind;

	//send the info to SipProxyMngr
	WORD nServiceId = 0;
	int arrCards[num_of_cards];
	for (int i = 0; i < num_of_cards; i++)
		arrCards[i] = -1;
	int arrServiceList[MAX_SERVICES_NUM + 1];
	for (int i = 0; i < MAX_SERVICES_NUM + 1; i++)
		arrServiceList[i] = 0;

	*pSeg >> board_id;
	*pSeg >> sub_board_id;
	*pSeg >> nServiceId;
	pSeg->Get((BYTE*) &ice_stat_ind, sizeof(ICE_STATUS_IND_S));

	ICE_INIT_IND_S ice_init_ind;
	memset(&ice_init_ind,eIceServerUnavailble,sizeof(ICE_INIT_IND_S));
	ice_init_ind.Relay_tcp_status = ice_stat_ind.Relay_tcp_status;
	ice_init_ind.Relay_udp_status = ice_stat_ind.Relay_udp_status;
	ice_init_ind.STUN_udp_status  = ice_stat_ind.STUN_udp_status;
	ice_init_ind.STUN_tcp_status  = ice_stat_ind.STUN_tcp_status;
	ice_init_ind.fw_type 		  = ice_stat_ind.fw_type;
	ice_init_ind.ice_env		  = ice_stat_ind.ice_env;


	iceInfo.SetIceInitInd(ice_init_ind, board_id, sub_board_id);

	// update signaling monitoring DB
	CIPServiceList* pIpServListDyn = pCSMngrProcess->GetIpServiceListDynamic();
	if (pIpServListDyn) {
		if (pIpServListDyn->GetService(iceServerTypes.service_id) != NULL) {
			TRACESTR(eLevelInfoNormal)
					<< "CCSMngrManager::OnCardsToCsIceDetails - Update signaling monitoring DB ServiceId = "
					<< iceServerTypes.service_id;
			pIpServListDyn->UpdateDynamic(iceServerTypes.service_id, iceInfo);

		}
	}

	ICE_STATUS_IND_S IceStatusInd;
	memcpy(&IceStatusInd,&ice_stat_ind,sizeof(ICE_STATUS_IND_S));

	CSegment*  pParam = new CSegment();
	pParam->Put((BYTE*)&IceStatusInd,sizeof(ICE_STATUS_IND_S));



	if(eIceEnvironment_WebRtc == iceServerTypes.ice_env)
	{
		CIceTaskApi iceApi(1);
		iceApi.SendMsg(pParam, UPDATE_STATUS_END);

	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrManager::HandleTerminalActiveAlarm(CTerminalCommand& command,
		std::ostream& answer) {
	if (3 != command.GetNumOfParams()) {
		answer
				<< "usage: ca active_alarm CSMngr [errorCode] [CSID] [operation]";
		return STATUS_FAIL;
	}

	WORD error_code;
	std::istringstream buf1(command.GetToken(eCmdParam1));
	if (!(buf1 >> error_code)) {
		answer << "error: illegal error code";
		return STATUS_FAIL;
	}

	DWORD cs_id;
	std::istringstream buf2(command.GetToken(eCmdParam2));
	if (!(buf2 >> cs_id)) {
		answer << "error: illegal CS ID";
		return STATUS_FAIL;
	}

	WORD operation;
	std::istringstream buf3(command.GetToken(eCmdParam3));
	if (!(buf3 >> operation)) {
		answer << "error: illegal operation";
		return STATUS_FAIL;
	}

	COsQueue* signalingMbx = pCSMngrProcess->GetSignalingMbx(cs_id - 1);

	CSegment* pSegToSignaling = new CSegment;
	*pSegToSignaling << (WORD) error_code;
	*pSegToSignaling << (WORD) operation;

	CSignalingApi api;
	api.CreateOnlyApi(*signalingMbx);
	api.SendMsg(pSegToSignaling, CSMNGR_ADD_REMOVE_AA_IND);

	answer << "transfer the message to Signaling Task";
	return STATUS_OK;
}

STATUS CCSMngrManager::HandleTerminalEnc(CTerminalCommand& command,
		std::ostream& answer) {
	DWORD numOfParams = command.GetNumOfParams();
	if (1 != numOfParams) {
		answer << "error: Illegal number of parameters\n";
		return STATUS_FAIL;
	}

	const std::string& tempPlain = command.GetToken(eCmdParam1);
	std::string tempEnc;
	std::string tempDec;
	std::string base64tempEnc;
	std::string base64tempDec;

	EncodeHelper eH;
	eH.EncodeAes(tempPlain, tempEnc);

	base64tempEnc = eH.base64_encode((unsigned char*) tempEnc.c_str(), strlen(
			tempEnc.c_str()));
	base64tempDec = eH.base64_decode(base64tempEnc);

	eH.DecodeAes(base64tempDec, tempDec);

	answer << "\nEncodeAes" << "\nPlain: " << tempPlain << "\nEnc:   "
			<< tempEnc << "\nDec:   " << tempDec;

	return STATUS_OK;
}

STATUS CCSMngrManager::OnCsCertMngrIpServiceParamReq(CSegment* pSeg) {
	TRACEINTO << __PRETTY_FUNCTION__;

	STATUS status = STATUS_OK;

	m_CommCertMngrService->SetIsConnected(true);

	status = m_CommCertMngrService->SendIpServiceList();
	CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
			"to send IPS LIST to CertMngr", status);

	status = m_CommCertMngrService->SendIpServiceParamEndInd();
	CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
			"to send END params to CertMngr", status);

	m_ipListAlreadySent[eToSendIpList_CertMngr] = true;

	return status;
}

STATUS CCSMngrManager::OnCsMcuMngrIpServiceParamReq(CSegment* pSeg) {
	TRACESTR(eLevelInfoNormal)
			<< "CCSMngrManager::OnCsMcuMngrIpServiceParamReq";

	STATUS status = STATUS_OK;

	m_CommMcuMngrService->SetIsConnected(true);

	status = m_CommMcuMngrService->SendIpServiceList();
	CCSMngrProcess::TraceToLogger(__PRETTY_FUNCTION__,
			"to send IPS LIST to McuMngr", status);

	m_ipListAlreadySent[eToSendIpList_McuMngr] = true;
	return status;
}

STATUS CCSMngrManager::OnCsTCPDumpIpServiceParamReq(CSegment*) {
	eProductType prodType = CProcessBase::GetProcess()->GetProductType();
	if (eProductTypeGesher == prodType || eProductTypeNinja == prodType) {
		if (true == m_ipListAlreadySent[eToSendIpList_TCPDump]) {
			TRACEINTOFUNC
					<< "IP services has been sent to TCP Dump. Donot send again.";
			return STATUS_OK;
		}
	}

	// Indicates that the request sent
	m_flagIsTCPDumpAskedForIpServiceList = true;

	if (!m_sentMngmtTCPDump)
	{
		m_sentMngmtTCPDump = SendManagementTCPDumpParams();
	}

	if (IsTarget() && !m_flagIsMediaIpConfigReceived)
	{

		OPCODE code = CS_CARDS_MEDIA_IP_CONFIG_IND;
		TRACEINTOFUNC << pCSMngrProcess->GetOpcodeAsString(code) << " ("
				<< code << ") was not received yet";


		if (eIpType_IpV4 != m_sysIpType && eV6Configuration_Manual
				!= m_sysV6ConfigurationType) {
			TRACEINTOFUNC << "IPv6 Auto mode configuration, wait...";
			return STATUS_OK;
		}
	}

	BOOL sentIpService = SendIpServicesTCDumpParams();

	if (m_sentMngmtTCPDump && sentIpService)
	{
		m_ipListAlreadySent[eToSendIpList_TCPDump] = true;

		// Request was processed, remove the indication
		m_flagIsTCPDumpAskedForIpServiceList = false;

		TRACEINTOFUNC << "IP services were sent to TCP Dump";
	}
	return STATUS_OK;
}

BOOL CCSMngrManager::SendManagementTCPDumpParams()
{
	if (!m_flagIsIpTypeReceived)
	{
		OPCODE code = MCUMNGR_IP_TYPE_IND;
		TRACEINTOFUNC << pCSMngrProcess->GetOpcodeAsString(code) << " ("
				<< code << ") was not received yet, wait...";
		return FALSE;
	}

	if (m_CommTCPDumpService == NULL)
	{
		TRACEINTOFUNC << " m_CommTCPDumpService is NULL ";
		return FALSE;
	}

	m_CommTCPDumpService->SetIsConnected(true);

	DWORD ip = pCSMngrProcess->GetMngmntAddress_IPv4();
	if (0 == ip) {
		TRACEINTOFUNC << MANAGEMENT_NETWORK_NAME
				<< " is not configured yet, wait...";
		return FALSE;
	}
	eIpType	 ipType = pCSMngrProcess->GetSysIpType();

	string ipV6Str = ""; 
	if (ipType == eIpType_IpV6)
	{

		//IPV6 Only
		unsigned int i = 0;
		BOOL indexGlobal = NUM_OF_IPV6_ADDRESSES + 1;
		BOOL indexSite = NUM_OF_IPV6_ADDRESSES + 1;
		BOOL indexLink = NUM_OF_IPV6_ADDRESSES + 1;
		string sCurIPv6Address = "";
		string ipAddressV6[NUM_OF_IPV6_ADDRESSES] = "";
		eIPv6AddressScope ipAddrScope[NUM_OF_IPV6_ADDRESSES];
		for (int j=0; j<MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES; j++)
		{
			sCurIPv6Address = "";
			ipAddressV6[j] = "";
			eIPv6AddressScope eCurIPV6AddrScope = eIPv6AddressScope_other;
			sCurIPv6Address = pCSMngrProcess->GetMngmntAddress_IPv6(j);
			
			if ((sCurIPv6Address.length() != 0) && ("::" != sCurIPv6Address))
			{
				eCurIPV6AddrScope = GetIPv6AddressScope(sCurIPv6Address.c_str());
				ipAddressV6[j] = sCurIPv6Address;
				ipAddrScope[j] = eCurIPV6AddrScope;
				if(eCurIPV6AddrScope == eIPv6AddressScope_global)
				{
					indexGlobal = j;
				}
				else if (eCurIPV6AddrScope == eIPv6AddressScope_siteLocal)
				{
					indexSite = j;
				}
				else if (eCurIPV6AddrScope == eIPv6AddressScope_linkLocal)
				{
					indexLink = j;
				}
				
			}
		}

		if(indexGlobal != NUM_OF_IPV6_ADDRESSES + 1)
		{
			ipV6Str = ipAddressV6[indexGlobal];
		}
		else if (indexSite != NUM_OF_IPV6_ADDRESSES + 1)
		{
			ipV6Str = ipAddressV6[indexSite];
		}
		else if (indexLink != NUM_OF_IPV6_ADDRESSES + 1)
		{
			ipV6Str = ipAddressV6[indexLink];
		}
		else 
		{
			ipV6Str = ipAddressV6[0];
		}
		
	}

	STATUS stat = m_CommTCPDumpService->SendMngmIpService(ip, ipType, ipV6Str.c_str());
	if (STATUS_OK != stat)
	{
		TRACEINTOFUNC << "Failed SendMngmIpService";
		return FALSE;
	}
	TRACEINTOFUNC << "After SendMngmIpService";
	return TRUE;
}

BOOL CCSMngrManager::SendIpServicesTCDumpParams()
{
	if (m_CommTCPDumpService == NULL)
		{
			TRACEINTOFUNC << " m_CommTCPDumpService is NULL ";
			return FALSE;
		}

	m_CommTCPDumpService->SetIsConnected(true);
	if ( m_CommTCPDumpService->SendIpServiceList() != STATUS_OK )
	{
		TRACEINTOFUNC << "Failed SendIpServiceList";

		return FALSE;
	}
	
	TRACEINTOFUNC << "After SendIpServicesTCPDumpParams";
	return TRUE;
}


void CCSMngrManager::CopyOCSFilesToRightLocation() {
	TRACEINTO << __PRETTY_FUNCTION__;

	CIPServiceList* pIpServListStatic =
			pCSMngrProcess->GetIpServiceListStatic();

	CIPService* pService = pIpServListStatic->GetFirstService();

	if (NULL == pService) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::CopyOCSFilesToRightLocation - Empty services list - no OCS files need to be copied";
		return;
	}

	// Copies OCS files to new CS location. They will be valid only after next
	// reset in the CS.
	// The command checks existence of .pem, .pfx, .txt files and copies
	// them to appropriate CS directory.
	// 'echo FooBar' on else prevents passing error to SystemPipedCommand.

	std::string new_dir_location = MCU_MCMS_DIR+("/KeysForCS/cs" + pService->GetId());

	if (IsDirectoryEmpty(new_dir_location.c_str())) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::CopyOCSFilesToRightLocation - copy the files from old location to new";

		std::ostringstream cmd;
		cmd
				<< "[ $(ls "+MCU_MCMS_DIR+"/KeysForCS/*.[pt][efx][mxt] 2> /dev/null | wc -l) != 0 ] && "
				<< "cp -Rf "+MCU_MCMS_DIR+"/KeysForCS/*.[pt][efx][mxt] "
				<< MCU_MCMS_DIR+"/KeysForCS/cs" << pService->GetId()
				<< "/ 2>&1 || echo FooBar";

		std::string ans;
		STATUS stat = SystemPipedCommand(cmd.str().c_str(), ans);
		PASSERTSTREAM(STATUS_OK != stat, "SystemPipedCommand: " << cmd.str()
				<< ": " << ans);
	} else
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::CopyOCSFilesToRightLocation - target folder is full. no need to copy OCS files to new location";
}

BOOL CCSMngrManager::CheckIfDefaultGWIPV6IsValid(CIPService* pUpdatedIpService) {
	if (eIpType_IpV4 == m_sysIpType) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::CheckIfDefaultGWIPV6IsValid - No need to validate default gw IPV6 in IPV4 system";
		return TRUE;
	}

	if (eV6Configuration_Auto == m_sysV6ConfigurationType) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::CheckIfDefaultGWIPV6IsValid - No need to validate default gw IPV6 in Auto mode";
		return TRUE;
	}

	CIPServiceList* pIpServListStatic =
			pCSMngrProcess->GetIpServiceListStatic();
	CIPService* pService = pIpServListStatic->GetFirstService();

	char updatedDefGW[IPV6_ADDRESS_LEN];
	pUpdatedIpService->GetDefaultGatewayIPv6(updatedDefGW);

	// 1. check if the default ipv6 GW is empty
	if (pUpdatedIpService->GetDefaultGatewayMaskIPv6() == 64 && strcmp(
			updatedDefGW, "::") == 0)
		return TRUE;

	// 2. check if the ipv6 router is different in one of the other services
	if (pCSMngrProcess->GetMngmntDefaultGatewayMaskIPv6() != 64
			&& pCSMngrProcess->GetMngmntDefaultGatewayMaskIPv6()
					!= pUpdatedIpService->GetDefaultGatewayMaskIPv6()) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::CheckIfDefaultGWIPV6IsValid - default gw mask in ipv6 can't be different between the service and the management";
		return FALSE;
	}

	if (pCSMngrProcess->GetMngmntDefaultGatewayIPv6() != "::"
			&& pCSMngrProcess->GetMngmntDefaultGatewayIPv6() != updatedDefGW) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::CheckIfDefaultGWIPV6IsValid - default gw ipv6 can't be different between the service and the Management. In this update service it is: "
				<< updatedDefGW << " while in the management it is "
				<< pCSMngrProcess->GetMngmntDefaultGatewayIPv6();
		return FALSE;
	}

	// 3. check if the ipv6 router is different in one of the other services
	while (pService) {
		if (strcmp(pService->GetName(), pUpdatedIpService->GetName()) != 0) {
			// check if the default ipv6 router is different than the one defined in other services
			if (pService->GetDefaultGatewayMaskIPv6() != 64
					&& pService->GetDefaultGatewayMaskIPv6()
							!= pUpdatedIpService->GetDefaultGatewayMaskIPv6()) {
				TRACESTR(eLevelInfoNormal)
						<< "CCSMngrManager::CheckIfDefaultGWIPV6IsValid - default gw mask in ipv6 can't be different between the services";
				return FALSE;
			}

			char curDefGW[IPV6_ADDRESS_LEN];
			pService->GetDefaultGatewayIPv6(curDefGW);

			if (strcmp(curDefGW, "::") != 0 && strcmp(curDefGW, "") != 0
					&& (strcmp(curDefGW, updatedDefGW) != 0)) {
				TRACESTR(eLevelInfoNormal)
						<< "CCSMngrManager::CheckIfDefaultGWIPV6IsValid - default gw ipv6 can't be different between the services. in this update service it is: "
						<< updatedDefGW << " while in " << pService->GetName()
						<< " it is " << curDefGW;
				return FALSE;
			}
		}

		pService = pIpServListStatic->GetNextService();
	}

	return TRUE;
}

// Judith - should work in a different way: The default gw can't be the same only if it is defined on the same media card!!! if it defined in different services with differet cards, it is OK
BOOL CCSMngrManager::CheckIfDefaultGWIPV4IsValid(CIPService* pUpdatedIpService) {
	if (eIpType_IpV6 == m_sysIpType) {
		TRACESTR(eLevelInfoNormal)
				<< "CCSMngrManager::CheckIfDefaultGWIPV4IsValid - No need to validate default gw IPV4 in IPV6 system";
		return TRUE;
	}

	CIPServiceList* pIpServListStatic =
			pCSMngrProcess->GetIpServiceListStatic();
	CIPService* pService = pIpServListStatic->GetFirstService();

	DWORD def_gw_ipv4 = pUpdatedIpService->GetDefaultGatewayIPv4();

	// 1. check if the default ipv4 GW is empty
	if (def_gw_ipv4 == 0)
		return TRUE;

	// 2. check if the ipv4 router is the same as in one of the other services
	while (pService) {
		if (strcmp(pService->GetName(), pUpdatedIpService->GetName()) != 0)
			if (pService->GetDefaultGatewayIPv4() == def_gw_ipv4) {
				TRACESTR(eLevelInfoNormal)
						<< "CCSMngrManager::CheckIfDefaultGWIPV4IsValid - default gw in ipv4 can't be the same as the other services";
				return FALSE;
			}

		pService = pIpServListStatic->GetNextService();
	}

	return TRUE;
}

void CCSMngrManager::TreatH323PasswordFromEMA(CIPService& theService,
		eOperationOnService theOperation) {
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::TreatH323PasswordFromEMA";

	// add or update service
	if ((eOperationOnService_Add == theOperation)
			|| (eOperationOnService_Update == theOperation))
		// H323 password is received from EMA as plaintext,
		// and 'DeSerializeXml' method stores it in m_Password field.

		// ===== 1. encrypt the password
		EncryptH323Password(theService);
}

void CCSMngrManager::EncryptH323Password(CIPService& theService) {
	std::string sEnc;
	std::string base64tempEnc;

	theService.GetpSecurity()->GetIPAuthentication()->GetSecondHTTPDigestAuthenticationElements()->SetAuthenticationProtocol(
			eAES);
	std::string
			tmpPass(
					theService.GetpSecurity()->GetIPAuthentication()->GetSecondHTTPDigestAuthenticationElements()->GetPassword().GetString());

	EncodeHelper eH;
	eH.EncodeAes(tmpPass, sEnc);
	base64tempEnc = eH.base64_encode((unsigned char*) sEnc.c_str(), strlen(
			sEnc.c_str()));

	theService.GetpSecurity()->GetIPAuthentication()->GetSecondHTTPDigestAuthenticationElements()->SetPassword_enc(
			base64tempEnc);
}

void CCSMngrManager::DecryptH323Password(CIPService& theService) {
	TRACESTR(eLevelInfoNormal) << "\nCCSMngrManager::DecryptH323Password";

	std::string sDec;
	std::string base64tempEnc;
	std::string base64tempDec;

	CSmallString tmpPass;

	tmpPass
			= theService.GetpSecurity()->GetIPAuthentication()->GetSecondHTTPDigestAuthenticationElements()->GetPassword();

	if (tmpPass.GetStringLength() == 0)
		return;

	EncodeHelper eH;
	theService.GetpSecurity()->GetIPAuthentication()->GetSecondHTTPDigestAuthenticationElements()->SetAuthenticationProtocol(
			eAES);
	
	base64tempEnc = tmpPass.GetString();
	base64tempDec = eH.base64_decode(base64tempEnc);
	eH.DecodeAes(base64tempDec, sDec);

	CSmallString passTmp(sDec.c_str());
	theService.GetpSecurity()->GetIPAuthentication()->GetSecondHTTPDigestAuthenticationElements()->SetPassword(
			passTmp);
	theService.GetpSecurity()->GetIPAuthentication()->GetSecondHTTPDigestAuthenticationElements()->SetPassword_enc(
			tmpPass.GetString());
}

void CCSMngrManager::OnInterfaceConfigurationReq(CSegment* pSeg) {
	DWORD len;
	*pSeg >> len;

	TRACESTR(eLevelInfoNormal)
			<< "CCSMngrManager::OnInterfaceConfigurationReq - Print string "
			<< len;
}

BOOL CCSMngrManager::IsToCopyMSFileToOneServiceFile() {
	if ((!(IsFileExists(IP_SERVICE_LIST_PATH))) && (!(IsFileExists(
			IP_SERVICE_LIST_TMP_PATH))))
		if (IsFileExists(IP_MULTIPLE_SERVICES_LIST_TMP_PATH)) {
			CIPServiceList TempServList;
			TempServList.ReadXmlFile(IP_MULTIPLE_SERVICES_LIST_TMP_PATH,
					eNoActiveAlarm, eRenameFile);

			if (TempServList.GetServiceNumber() == 1) // if only one service configure in MS file
			{
				TRACESTR(eLevelInfoNormal)
						<< "CCSMngrManager::IsToCopyMSFileToOneServiceFile - copy MS file to one service file, since MS file contain only 1 service";
				return TRUE;
			}
		}

	return FALSE;
}

void CCSMngrManager::OnTimerSendIpServiceToUtilityProcess(CSegment* pParam) {
	if (IsStartupFinished()) {
		TRACEINTOFUNC << "Startup is finished, update IP service";
		m_CommCardService->SendIpServiceParamUtility();
		SendPrecedentSetting();
		return;
	}

	CProcessBase* proc = CProcessBase::GetProcess();
	PASSERT_AND_RETURN(NULL == proc);

	unsigned int timeout = 10 * SECOND;
	OPCODE opcode = CS_UTILITY_IS_UP_TIMER;
	StartTimer(opcode, timeout);

	TRACEINTOFUNC << "Timer " << proc->GetOpcodeAsString(opcode)
			<< " will fire in " << timeout / SECOND << " seconds";
}
void   CCSMngrManager::SendPrecedentSetting()
{

		char buffer[32] = "";
		TRACESTR(eLevelInfoNormal) << "CCSMngrManager::OnMcuMngrPrecedenceSettings";
		if(!m_precedentSetting)
		{
			TRACEINTOFUNC << "warning called SendPrecedentSetting when m_precedentSetting is NULL";
			return;
		}
		if (m_precedentSetting->IsPrecedenceEnabled() != 1)
		{
			sprintf(buffer, "%d", ERROR_SIGNALING_DSCP_VALUE);
		}
		else
		{
			BYTE signalingDSCP = m_precedentSetting->GetSignalingDSCP();
			sprintf(buffer, "%d", (WORD) signalingDSCP);
		}
		std::ostringstream cmd;
		cmd << MCU_MCMS_DIR+"/Bin/McuCmd csdscpsip CS " << buffer;
		std::string ans;
		STATUS stat;
		stat = SystemPipedCommand(cmd.str().c_str(), ans);
		POBJDELETE(m_precedentSetting);
}
//=====================================================================================================================================//
void CCSMngrManager::OnMcuMngrPrecedenceSettings(CSegment* pSeg) {

	if(m_precedentSetting)
		POBJDELETE(m_precedentSetting);
	m_precedentSetting  = new CPrecedenceSettings();
	m_precedentSetting->DeSerialize(NATIVE, *pSeg);
	if (IsStartupFinished())
	{
		SendPrecedentSetting();
	}
}

/////////////////////////////////////////////////////////////////////
BOOL CCSMngrManager::IsStartupFinished() const {
	eMcuState systemState = CProcessBase::GetProcess()->GetSystemState();
	if (eMcuState_Invalid == systemState || eMcuState_Startup == systemState)
		return FALSE;

	return TRUE;
}
std::string CCSMngrManager::GetCloudIp() const {
	std::string strCloudIp = "";
	eProductFamily product_family = CProcessBase::GetProcess()->GetProductFamily();

	if (product_family == eProductFamilySoftMcu)

	{
		std::string fname = MCU_TMP_DIR+"/cloudIp";
		FILE *pCloudIpFile = fopen(fname.c_str(), "r");
		if (pCloudIpFile) {
			char * line = NULL;
			size_t len = 0;
			ssize_t read;
			read = getline(&line, &len, pCloudIpFile);
			if (read != -1)
				strCloudIp = line;

			TRACESTR(eLevelInfoNormal)
					<< "\nCCSMngrManager::GetCloudIp() - strCloudIp = "
					<< strCloudIp;

			if (line)
				free(line);

			fclose(pCloudIpFile);
		} else
			PTRACE(eLevelInfoNormal, "No cloud IP address");
	}

	return strCloudIp;

}

void CCSMngrManager::OnCSIPConfigEndMSPerService(CSegment* pParam) {
	TRACESTR(eLevelInfoNormal)
			<< "CCSMngrManager::OnCSIPConfigEndMSPerService ";

	DWORD serviceId = 0;
	DWORD boardId = 0;
	DWORD pqId = 0;

	*pParam >> serviceId >> boardId >> pqId;

	pCSMngrProcess->SetCSIpConfigMasterBoardId(serviceId, boardId);
	pCSMngrProcess->SetCSIpConfigMasterPqId(serviceId, pqId);
	TRACESTR(eLevelInfoNormal)
			<< "CCSMngrManager::OnCSIPConfigEndMSPerService - serviceId: "
			<< serviceId << " boardId " << boardId << " pqId " << pqId;

}

//added for debug IP Settings
void printIpAddressDebug(DWORD ipaddress) {
	unsigned char *ip4, *netmaskv4, *gwv4;
	char sIpAddress[128] = "";
	ip4 = (unsigned char*) &ipaddress;
	sprintf(sIpAddress, "%d.%d.%d.%d", (int) ip4[3], (int) ip4[2],
			(int) ip4[1], (int) ip4[0]);
	printf("[debug]: ipaddress= %s\n", sIpAddress);
}

void PrintIPServiceInfoDebug(CIPService *ipService) {

	//DWORD ipv4Address = ipService->GetNetIPaddress();
	CIPSpan * ipSpan = ipService->GetFirstSpan();
	DWORD ipv4Address = ipSpan->GetIPv4Address();
	DWORD ipv4NetMask = ipService->GetNetMask();
	DWORD ipv4DefaultGateway = ipService->GetDefaultGatewayIPv4();
	printf("==========================\n");
	printf("[debug]: ip=%u, mask=%u, gw=%u, ipServiceType=%d\n", ipv4Address,
			ipv4NetMask, ipv4DefaultGateway, ipService->GetIpServiceType());
	printIpAddressDebug(ipv4Address);
	printIpAddressDebug(ipv4NetMask);
	printIpAddressDebug(ipv4DefaultGateway);
	printf("==========================\n");

}
//end

//added for IP Settings
////////////////////////////////////////////////////////////////////////////
void RetrieveRouterList(CIPService *pService, std::list<CRouter> & routerList) {
	const CH323Router *currentRouter = pService->GetFirstRouter();
	while (NULL != currentRouter) {
		if (false == currentRouter->IsDefault()) {
			char targetIpBuffer[128];
			char subnetMaskIPv4Buffer[128];
			char gatewayBuffer[128];

			SystemDWORDToIpString(currentRouter->GetRemoteIP(), targetIpBuffer);
			SystemDWORDToIpString(currentRouter->GetSubnetMask(),
					subnetMaskIPv4Buffer);
			SystemDWORDToIpString(currentRouter->GetRouterIP(), gatewayBuffer);

			CRouter param;
			param.m_type = (currentRouter->GetRemoteFlag()
					== H323_REMOTE_NETWORK ? router_net : router_host);
			param.m_targetIP = targetIpBuffer;
			param.m_subNetmask = subnetMaskIPv4Buffer;
			param.m_gateway = gatewayBuffer;

			routerList.push_back(param);
		}
		currentRouter = pService->GetNextRouter();
	}
}
/////////////////////////////////////////////////////////////////////


STATUS CCSMngrManager::CheckCsMediaDuplicateIp(char* sIp)
{	
	int status = 0;
	
	// arping help:
	//--------------------
//	Usage: arping [-fqbDUAV] [-c count] [-w timeout] [-I device] [-s source] destination
//  	-f : quit on first reply
//  	-q : be quiet
//  	-b : keep broadcasting, don't go unicast
//  	-D : duplicate address detection mode (DAD).(from manual: Returns 0, if DAD succeeded i.e. no replies are received)
//  	-U : Unsolicited ARP mode, update your neighbours
//  	-A : ARP answer mode, update your neighbours
//  	-V : print version and exit
//  	-c count : how many packets to send (manual: Stop after sending count ARP REQUEST packets. With deadline option, arping waits for count ARP REPLY packets, until the timeout expires.)
//  	-w timeout : how long to wait for a reply (manual: Specify  a  timeout,  in seconds, before arping exits regardless of how many packets have been sent or received. In this case arping 
//					does not stop after count packet are sent, it waits either for deadline expire or until count probes are answered.)
//  	-I device : which ethernet device to use (eth0)
//  	-s source : source ip address (from manaual: In DAD mode (with option -D) set to 0.0.0.0.)
//  	destination : ask for what ip address
	eProductType curProductType =
					CProcessBase::GetProcess()->GetProductType();

	if ((eProductTypeGesher == curProductType ) || (eProductTypeNinja == curProductType ))
	{
		STATUS status;

		std::ostringstream cmdStr_arping;
		std::string ans;



		cmdStr_arping << "sudo /usr/bin/arping -D -c 4 -w 2 -s 0.0.0.0 -I eth0 " << sIp
				<< " |grep 'Received' | awk -F ' ' '{print $2}'| grep '1' ";


		status = SystemPipedCommand(cmdStr_arping.str().c_str(), ans);


		if (ans !="")
		{
			TRACESTR(eLevelInfoNormal) << "\nFound duplicate CS&Media IP: " << sIp;

			std::string description = "DUPLICATE IP: Found duplicate ip for ip: " + string(sIp);
			AddActiveAlarmNoFlush(FAULT_GENERAL_SUBJECT,
					CS_NOT_CONFIGURED,
					MAJOR_ERROR_LEVEL,
					description.c_str(),
					true,
					true);
		}
	}
					
	
	return STATUS_OK;			
}


//copied from CSignalingTask::ConfigureNetwork
void CCSMngrManager::SetCsMediaIpConfig(CIPService *ipService,
		const eConfigInterfaceType ifType) {
	//PrintIPServiceInfoDebug(ipService);
	//printf("[debug]:SetCsMediaIpConfig itType=%d", ifType);
	
	if (ipService) 
	{
		eIpType ipType = ipService->GetIpType();
		BOOL bLAN_REDUNDANCY = NO;
		bLAN_REDUNDANCY = CProcessBase::GetProcess()->GetLanRedundancy(ipType);

		eProductType curProductType =
				CProcessBase::GetProcess()->GetProductType();
		std::string answer;
		if (bLAN_REDUNDANCY && (curProductType == eProductTypeGesher)) {
			SystemPipedCommand("sudo /sbin/ifconfig eth1 down", answer);
			SystemPipedCommand("sudo /sbin/ifconfig eth2 down", answer);
			SystemPipedCommand("sudo /sbin/ifconfig eth3 down", answer);
			SystemPipedCommand(
					"sudo /sbin/modprobe bonding mode=active-backup miimon=100",
					answer);
			SystemPipedCommand("sudo /sbin/ifconfig bond0 up", answer);
			SystemPipedCommand("sudo /sbin/ifenslave -d bond0 eth1 eth2 eth3",
					answer);
			SystemPipedCommand("sudo /sbin/ifenslave bond0 eth1 eth2 eth3",
					answer);
		}
		
		std::string ifName;

		ifName = GetLogicalInterfaceName(ifType, ipType);

		TRACESTR(eLevelInfoNormal)
				<< "\nCSignalingTask::ConfigureNetwork for service="
				<< ipService->GetName() << " , If Name=" << ifName;

		CConfigManagerApi api;

		//1. Interface up
		//api.InterfaceUp(ifName);

		char defaultGatewayIPv4Str[IP_ADDRESS_LEN] = "";
		char defaultGatewayIPv6Str[IPV6_ADDRESS_LEN] = "";
		BOOL bIsAutoConfig = FALSE;

		//2. IPv6 Autoconfiguration
		eV6ConfigurationType ipv6CfgType =
				ipService->GetIpV6ConfigurationType();
		if (eV6Configuration_Auto == ipv6CfgType)
			bIsAutoConfig = TRUE;

		//api.SetNICIpV6AutoConfig(ifName, bIsAutoConfig);

		//3. Get ipv4 address
		char ipStr[IP_ADDRESS_LEN] = "";
		char netmaskStr[IP_ADDRESS_LEN] = "";
		char iPv4defGWStr[IP_ADDRESS_LEN] = "";
		char IPv4broadcastStr[IP_ADDRESS_LEN] = "";
		SystemDWORDToIpString(ipService->GetNetMask(), netmaskStr);
		SystemDWORDToIpString(ipService->GetDefaultGatewayIPv4(), iPv4defGWStr);

		strcpy(IPv4broadcastStr, iPv4defGWStr);
		char * tmp = strrchr(IPv4broadcastStr, '.');
		strcpy(tmp + 1, "255");

		CIPSpan* pTmpSpan = ipService->GetSpanByIdx(0);
		if (pTmpSpan) {
			DWORD ipAddress = pTmpSpan->GetIPv4Address();
			SystemDWORDToIpString(ipAddress, ipStr);

			CheckCsMediaDuplicateIp(ipStr);
			
			//printIpAddressDebug(ipAddress);
			//4. IPv4 Default GW setup
			DWORD ipv4_gateway = ipService->GetDefaultGatewayIPv4();
			ipService->GetDefaultGatewayIPv6(defaultGatewayIPv6Str, FALSE);

			DWORD mask = ipService->GetNetMask();

			SystemDWORDToIpString(ipv4_gateway, defaultGatewayIPv4Str);
			std::string stIpv4Gateway = defaultGatewayIPv4Str;

			std::list<CRouter> routerList;
			RetrieveRouterList(ipService, routerList);

			//printf("[debug]: add ip interface\n");
			//PrintIPServiceInfoDebug(ipService);
			if (eIpType_IpV4 == ipType || eIpType_Both == ipType) 
			{
				STATUS res = api.AddIpInterface(ifType, ipType, "0.0.0.0", ipStr,
						netmaskStr, iPv4defGWStr, IPv4broadcastStr, routerList);

				if (stIpv4Gateway != "0.0.0.0" && stIpv4Gateway
						!= "255.255.255.255") 
				{
					//Cleanup
					TRACESTR(eLevelInfoNormal)
							<< "\nCCSMngrManager::ConfigureNetwork, ipv4 cleanup";
					api.DelRouteTableRule(ifType, ipType, ipStr, mask,
							stIpv4Gateway);
					//Add
					//api.AddDefaultGW(ifName, stIpv4Gateway);
					TRACESTR(eLevelInfoNormal)
							<< "\nCCSMngrManager::ConfigureNetwork, ipv4 Add";
					api.AddRouteTableRule(ifType, ipType, ipStr, mask,
						stIpv4Gateway , "" , routerList);
				}
			}

			if (eIpType_IpV6 == ipType || eIpType_Both == ipType) 
			{

				//5. Get IPv6 address
				std::string stIPv6Address = pTmpSpan->GetIPv6Address();
                char 	iPv6_netMask[IPV6_ADDRESS_LEN] = "";
                pTmpSpan->GetIPv6SubnetMaskStr(0, iPv6_netMask);
				//6. IPv6 Default GW setup
				std::string stIpv6Gateway = defaultGatewayIPv6Str;
                char  iPv6defGWMaskStr[IPV6_ADDRESS_LEN];
            	memset(iPv6defGWMaskStr, 0, IPV6_ADDRESS_LEN);
            	// (Configurator uses addresses without brackets) 
                
                TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SetCsMediaIpConfig"
                   << "\nipv6CfgType: " << ipv6CfgType
				   << "\nNew IPV6 address to config: " << stIPv6Address
				   << "\nIPv6_netMask: " << iPv6_netMask
				   << "\nIPv6 defGW: " << stIpv6Gateway
				   << "\nIPv6defGWMaskStr: " << iPv6defGWMaskStr;
                
                if (eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
                {
                	BYTE bIsAutoCfg = TRUE; 
                	if (eV6Configuration_Manual == ipv6CfgType)
                	{
                		bIsAutoCfg = FALSE;
                	}
                	STATUS res = api.AddIpV6Interface( ifType,
                                                 bIsAutoCfg,
												 stIPv6Address,
												 iPv6_netMask,
												 stIpv6Gateway,
												 iPv6defGWMaskStr );
                }
                else
                {
				//Cleanup

					api.DelRouteTableRule(ifType, ipType, stIPv6Address, mask,
							stIpv6Gateway, TRUE);
					//Add
					api.AddRouteTableRule(ifType, ipType, stIPv6Address, mask,
							stIpv6Gateway,iPv6_netMask, routerList, TRUE);
                }

				
			}
		} else {
			PASSERTMSG(1, "CCSMngrManager::ConfigureNetwork, invalid span");
		}

		//	SetIpv6Params();	// VNGFE-2988 - Judith: The service router is deleted. Ori thinks we don't need to erase teh IPV6 addresses anymore
	}
	return;
}

void CCSMngrManager::AddCSMediaIpInterfacesGesherNinja(CIPService *pService,
		BYTE bSystemMode, eProductType curProductType) 
{

	PASSERTMSG_AND_RETURN(pService==NULL, "pService is null");

	TRACESTR(eLevelInfoNormal) << "Configure Signal/Media IP for S800 / 1800";
	PASSERT_AND_RETURN(pService==NULL);
	eIpType ipType = pService->GetIpType();

	BOOL isLanRedundancy = CProcessBase::GetProcess()->GetLanRedundancy(ipType);
	
	if (isLanRedundancy)//LAN redundancy is higher priority than Multiple Service
	{
		TRACESTR(eLevelInfoNormal) << "Configuring service: "
				<< pService->GetName() << " for Lan Redundancy.";
		SetCsMediaIpConfig(pService, eLanRedundancySignalingNetwork);
		return;
	}
	//no Multiple Service, bind to eth1

    BOOL bIsNotIpV4 = FALSE;
	if(pService && ( (eIpType_IpV6 == pService->GetIpType()) || (eIpType_Both == pService->GetIpType())) )
    {
        bIsNotIpV4 = TRUE;
    }
    
	if (bSystemMode == eSystemMode_None || (TRUE == bIsNotIpV4)) 
    {
		TRACESTR(eLevelInfoNormal) << "Configuring service: "
				<< pService->GetName() << "  sysMode: " << bSystemMode << "bIsIpV4: " << bIsNotIpV4;
		SetCsMediaIpConfig(pService, eSignalingNetwork);
	} else if (bSystemMode == eSystemMode_Multiple_services) {
		int ethNum = 0;
		CIPService tmpService = *pService;
		//printf("[debug]: configuring ip service [%s] for multiple service.\n",tmpService.GetName());
		int loopCount = MAX_NUMBER_OF_IPSERVICES_GESHER;
		DWORD boardId = MULTIPLE_SERVICE_GESHER_FAKE_BOARD_ID;
		if (eProductTypeNinja == curProductType) {
			loopCount = MAX_NUMBER_OF_IPSERVICES_NINJA;
			boardId = MULTIPLE_SERVICE_NINJA_FAKE_BOARD_ID;
		}
		CIPSpan * tmpSpan = tmpService.GetFirstSpan();
		tmpSpan = tmpService.GetNextSpan();//skip the first one, which is CS IP.
		for (ethNum = 0; (ethNum < loopCount) && tmpSpan; ethNum++) {
			bool isEnabled = tmpSpan->GetIsSpanEnable();
			DWORD ipV4 = tmpSpan->GetIPv4Address();
			if ((FALSE == isEnabled) || (ipV4 == 0) ) {
				//printf("[debug]: span %d 's ip address is 0\n", ethNum);
				tmpSpan = tmpService.GetNextSpan();
			} else {
				//printf("[debug]: found span %d 's ip address is %u\n", ethNum, ip_v4);
				DWORD serviceId = pService->GetId();
				DWORD subBoardId = ethNum + 1;//board ID starting from 1.
				pCSMngrProcess->SetCSIpConfigMasterBoardId(serviceId, boardId);
				pCSMngrProcess->SetCSIpConfigMasterPqId(serviceId, subBoardId);
				eConfigInterfaceType eIfType = GetSignalingNetworkType(boardId,
						subBoardId);
				TRACESTR(eLevelInfoNormal)
						<< "Configure Signal/Media IP for S800/1800: service: ["
						<< serviceId << "] " << "boardId: [" << boardId << "] "
						<< "sub_boardId [" << subBoardId << "] " << "if Type ["
						<< eIfType << "] " << "IP[v4]: "
						<< tmpSpan->GetIPv4Address();
				SetCsMediaIpConfig(pService, eIfType);
				break;//found it.
			}
		}
	} else {
		//TODO: for JITC
		TRACESTR(eLevelWarn) << "Don't support JITC for RPCS 1800&800S now";
	}
}
//end

void CCSMngrManager::OnSNMPConfigInd(CSegment* pSeg) {
	BOOL bSNMPEnabled = FALSE;
	*pSeg >> bSNMPEnabled;

	PTRACE2INT(eLevelInfoNormal, "CCSMngrManager::OnSNMPConfigInd = ",
			(int) bSNMPEnabled);
	SetIsSNMPEnabled(bSNMPEnabled);
}

void CCSMngrManager::OnSignalingServiceUpInd(CSegment* pSeg) {
	WORD csID = 0xFFFF;
	*pSeg >> csID;

	m_vecServiceID.push_back(csID);

	PTRACE2INT(eLevelInfoNormal, "CCSMngrManager::OnSignalingServiceUpInd = ",
			(int) csID);
}

// Only one service is up, the service is up.
void CCSMngrManager::OnTimerGetServiceInfoInd(CSegment* pSeg)
{
  // Updates data regardless SNMP enableness.
  unsigned int h323 = 1;  // Disabled.
  unsigned int sip  = 1;

  CIPServiceList* pIpServListDynamic = pCSMngrProcess->GetIpServiceListDynamic();
  CIPService*     pService           = pIpServListDynamic->GetFirstService();
  while (pService)
  {
    eIPProtocolType type = pService->GetIPProtocolType();
    switch (type)
    {
      case eIPProtocolType_SIP     : sip  = 3; break;  // Failed.
      case eIPProtocolType_H323    : h323 = 3; break;
      case eIPProtocolType_SIP_H323: sip  = 3; h323 = 3; break;
      default: PASSERTSTREAM(true, "Illegal protocol type " << type);
    }
    pService = pIpServListDynamic->GetNextService();
  }

  if (1 == h323 && 1 == sip)
    return;

  CSegment* seg = new CSegment;
  *seg << 2u
       << static_cast<unsigned int>(eTT_H323Status) << h323
       << static_cast<unsigned int>(eTT_SIPStatus)  << sip;

  CManagerApi(eProcessSNMPProcess).SendMsg(seg, SNMP_UPDATE_MULTIPLE_TELEMETRY_DATA_IND);
}

STATUS CCSMngrManager::CSMngrCalculateSetProcessState(CSegment *pSeg)
{
	//Very Important : this function called means that the McuMngr Is up and we can start the interaction with McuMngr Proccess
	TRACEINTO << "Enter";
	//calling the baseClass
	CalculateSetProcessState();
	// ask IpType (of Management service)
	//Begin Startup Feature
	 return STATUS_OK;
	//END Startup Feature

}

void CCSMngrManager::CSMngrRestartStartupProcedure(CSegment *pSeg)
{

	ManagerPostInitActionsPoint();


}

STATUS CCSMngrManager::OnFireEth2CS(CSegment* pSeg)
{
	TRACEINTOFUNC << "OnFireEth2CS " << (int)m_firedEth2CS;
	
	if (m_firedEth2CS)
	{
		return STATUS_OK;
	}
	m_firedEth2CS = TRUE;
	OnCheckEth2CS(pSeg);
	return STATUS_OK;
}

STATUS CCSMngrManager::OnCheckEth2CS(CSegment* pSeg)
{		
	StartTimer(CHECK_ETH2_CS_TIMER, DEFAULT_CS_CHECK_CS_IP_CONFIG_PERIOD);
	
	CConfigManagerApi configuratorApi;
		
    BOOL isEth2Up = TRUE;
    if (configuratorApi.CheckCSEth2(isEth2Up) != STATUS_OK)
    {
    	TRACEINTOFUNC << "OnCheckEth2CS Failed CheckCSEth2";
    	return STATUS_OK;
    }
    
	
	//TRACEINTOFUNC << "CheckCsIpconfigAfterOnceSuccessfullyConfigured isEth2Up " << (int)isEth2Up  << " m_isEth2Up " << (int)m_isEth2Up  ;
	
	if (m_isEth2Up != isEth2Up)
	{
		m_isEth2Up = isEth2Up;

		TRACEINTOFUNC << "Sending SNMP_CS_LINK_STATUS_IND " << (int)m_isEth2Up << "\n";

		CSegment *pSeg = new CSegment;
		*pSeg << m_isEth2Up;

		CManagerApi apiSnmp(eProcessSNMPProcess);

		apiSnmp.SendMsg(pSeg, SNMP_CS_LINK_STATUS_IND);
	}

	return STATUS_OK;
}


STATUS CCSMngrManager::OnDnsAgentGetIpconfig(CSegment* pParam)
{
	// MAX_SERVICES_NUM
	CIPServiceList* pIpServList = pCSMngrProcess->GetIpServiceListStatic();
	CIPService*     pService    = pIpServList->GetFirstService();

	size_t  i            = 0;
	WORD    nNumServices = pIpServList->GetServiceNumber();
	eIpType ipType       = eIpType_None;

	DNS_IP_CONFIG_PARAM_S  config;
	memset(&config,0,sizeof(DNS_IP_CONFIG_PARAM_S));

	config.servicesNum  = nNumServices;

	nNumServices = min(nNumServices, (WORD)MAX_NUM_OF_IP_SERVICES);

	while (pService && i < nNumServices)
	{
		if (eIpServiceType_Management == pService->GetIpServiceType())
		{
			ipType = pService->GetIpType();
		}
		else if (eIpServiceType_Signaling == pService->GetIpServiceType())
		{
			if (ipType == eIpType_None)
				ipType = pService->GetIpType();

			CIpDns* pDns = pService->GetpDns();
			if (pDns)
			{
				config.services[i].dnsStatus = static_cast<WORD>(pDns->GetStatus());
				strncpy(config.services[i].szDomainName,pDns->GetDomainName().GetString(),DNS_AGENT_DOMAIN_NAME_LEN-1);

				for (int j=0; j<NUM_OF_DNS_SERVERS; ++j)
				{
					config.services[i].serversIpv4List[j] = pDns->GetIPv4Address(j);
					pDns->GetIPv6Address(j,config.services[i].serversIpv6List[j]);
				}
			}
		}

		pService = pIpServList->GetNextService();
		++i;
	}
	config.ipType = static_cast<WORD>(ipType);

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&config,sizeof(DNS_IP_CONFIG_PARAM_S));
	CManagerApi(eProcessDNSAgent).SendMsg(pSeg, DNSAGENT_TO_CSMNGR_GET_IPCONFIG);

	return STATUS_OK;
}






