
//////////////////////////////////////////////////////////////////////

#include <algorithm>

#include "GKServiceManager.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "Trace.h"
#include "IpMngrOpcodes.h"
#include "GKGlobals.h"
#include "StatusesGeneral.h"
#include "MplMcmsProtocol.h"
#include "RvCommonDefs.h"
#include "TaskApi.h"
#include "CsCommonStructs.h"
#include "SysConfig.h"
#include "ConfigHelper.h"
#include "HlogApi.h"
#include "FaultsDefines.h"
#include "HostCommonDefinitions.h"
#include "WrappersGK.h"
#include "WrappersMcuMngr.h"
#include "TraceStream.h"
#include "GKManagerUtils.h"
#include "HostCommonDefinitions.h"
#include "GkMplMcmsProtocolTracer.h"
#include "TerminalCommand.h"
#include "ManagerApi.h"
#include "McuMngrInternalStructs.h"
#include "ConfPartyManagerApi.h"
#include "ServiceConfigList.h"
#include "GKProcess.h"

const char* altGkStateStrings[7] =
{
    "NotAltGk",
	"Start",
	"TimeoutOrRegularReject",
	"RCF",
	"ReqAck",
	"AnotherTrigger",
	"LastAltGkState"
};

struct CGkCallHunterByConnID : std::unary_function<CGkCall*, bool>
{
	DWORD m_connID;

	CGkCallHunterByConnID(DWORD connId)
	{
		m_connID = connId;
	}

    bool operator()( CGkCall *gkCall) const
    {
		return (m_connID == gkCall->GetConnId());
    }
};

struct CGkCallHunterByConfID : std::unary_function<CGkCall*, bool>
{
	DWORD m_confID;

	CGkCallHunterByConfID(DWORD confId)
	{
		m_confID = confId;
	}

    bool operator()( CGkCall *gkCall) const
    {
		return (m_confID == gkCall->GetConfRsrcId());
    }
};


#define GapFromRealRrqInterval   5
#define IntervalBetweenRRJAndRRQ 10
#define RasMessageTimerInStack   18
#define RasMessageTimeout        RasMessageTimerInStack+10

//timers:
const WORD REGISTRATION_POLLING_TIMER = 120;
const WORD REGISTRATION_TIMER 		  = 121;
const WORD ARQ_TIMER 		  		  = 122;
const WORD DRQ_TIMER 		  		  = 123;
const WORD DNS_POLLING_TIMER		  = 124;
const WORD RAI_TIMER                  = 125;
const WORD ON_LIST_OF_GK_CALLS_REQ	  = 127; //Anat need to check

const WORD CS_QUEUE_AND_SEND_TIMER	  = 130;




////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CGKServiceManager)
//ONEVENT(XML_REQUEST  						  	  ,IDLE    		  ,CGKServiceManager::HandlePostRequest )


/**** Indication from GK: ***/
ONEVENT(CSAPI_MSG				   				  ,ANYCASE	      ,CGKServiceManager::OnCSApiMsg)
ONEVENT(H323_CS_RAS_GRQ_IND       				  ,IDLE           ,CGKServiceManager::OnGRQInd)
ONEVENT(H323_CS_RAS_RRQ_IND       	  			  ,IDLE           ,CGKServiceManager::OnRRQInd)
ONEVENT(H323_CS_RAS_GKDRQ_IND     				  ,IDLE			  ,CGKServiceManager::OnDRQFromGKInd)
ONEVENT(H323_CS_RAS_ARQ_IND       				  ,IDLE           ,CGKServiceManager::OnARQInd)
ONEVENT(H323_CS_RAS_DRQ_IND       				  ,IDLE           ,CGKServiceManager::OnDRQInd)
ONEVENT(H323_CS_RAS_TIMEOUT_IND   				  ,IDLE			  ,CGKServiceManager::OnTimeOutInd)
ONEVENT(H323_CS_RAS_URQ_IND       				  ,IDLE           ,CGKServiceManager::OnURQInd)
ONEVENT(H323_CS_RAS_GKURQ_IND     				  ,IDLE           ,CGKServiceManager::OnGkURQReqInd)
ONEVENT(H323_CS_RAS_FAIL_IND      				  ,IDLE			  ,CGKServiceManager::OnGkFailInd)
ONEVENT(H323_CS_RAS_GKLRQ_IND     				  ,IDLE           ,CGKServiceManager::OnGkLRQInd)
ONEVENT(H323_CS_RAS_BRQ_IND       				  ,IDLE			  ,CGKServiceManager::OnBRQInd)
ONEVENT(H323_CS_RAS_GKIRQ_IND     				  ,IDLE			  ,CGKServiceManager::OnGkIRQInd)
ONEVENT(H323_CS_RAS_GKBRQ_IND     				  ,IDLE			  ,CGKServiceManager::OnBRQFromGKInd)
ONEVENT(H323_CS_RAS_RAC_IND     				  ,IDLE			  ,CGKServiceManager::OnRACInd)

/***** Requests from party: ******/
ONEVENT(H323_CS_RAS_DRQ_RESPONSE_REQ 			  ,IDLE			  ,CGKServiceManager::OnDRQFromGKResponseReq)
ONEVENT(H323_CS_RAS_ARQ_REQ          			  ,IDLE           ,CGKServiceManager::OnARQReq)
ONEVENT(H323_CS_RAS_DRQ_REQ          			  ,IDLE           ,CGKServiceManager::OnDRQReq)
ONEVENT(H323_CS_RAS_BRQ_REQ           			  ,IDLE           ,CGKServiceManager::OnBRQReq)
ONEVENT(H323_CS_RAS_BRQ_RESPONSE_REQ  			  ,IDLE			  ,CGKServiceManager::OnBRQFromGKResponseReq)
ONEVENT(H323_CS_RAS_IRR_REQ          			  ,IDLE           ,CGKServiceManager::OnIrrPollingReq)
ONEVENT(H323_CS_RAS_IRR_RESPONSE_REQ          	  ,IDLE           ,CGKServiceManager::OnIrqFromGKResponseReq)
ONEVENT(H323_CS_RAS_REMOVE_GK_CALL_REQ			  ,IDLE			  ,CGKServiceManager::OnPartyRemoveGkCall)
ONEVENT(GK_MANAGER_PARTY_KEEP_ALIVE_IND			  ,IDLE			  ,CGKServiceManager::OnPartyKeepAliveInd)
ONEVENT(PARTY_GK_UPDATE_CONF_ID			  		  ,IDLE			  ,CGKServiceManager::OnPartyUpdateConfIdInd)
ONEVENT(CP_GK_CONF_ID_CLEAN_UP			  		  ,IDLE			  ,CGKServiceManager::OnConfPartyCleanUpConfIdInd)
ONEVENT(CP_GK_PARTY_ID_CLEAN_UP			  		  ,IDLE			  ,CGKServiceManager::OnConfPartyCleanUpPartyIdInd)

/***** Events from CsManager: ******/
ONEVENT(CS_GKMNGR_IP_SERVICE_PARAM_IND			  ,IDLE		  	  ,CGKServiceManager::OnCsMngrServiceParamsInd)
ONEVENT(CS_GKMNGR_IP_SERVICE_UPDATE_PARAM_IND	  ,IDLE	  		  ,CGKServiceManager::OnCsMngrServiceParamsInd_Upd)
ONEVENT(CS_GKMNGR_DELETE_IP_SERVICE_IND			  ,IDLE		      ,CGKServiceManager::OnCSMngrDeleteIpServiceInd)

/****** Local events: ******/
ONEVENT(REGISTRATION_POLLING_TIMER     			  ,IDLE           ,CGKServiceManager::OnRegistrationPollingTimeout)
ONEVENT(REGISTRATION_TIMER    		  			  ,ANYCASE        ,CGKServiceManager::OnRegistrationTimeout)
ONEVENT(ON_LIST_OF_GK_CALLS_REQ		  			  ,IDLE           ,CGKServiceManager::OnListOfGkCallsReq)
ONEVENT(DNS_POLLING_TIMER		      			  ,IDLE           ,CGKServiceManager::OnDnsPollingTimeout)
ONEVENT(RAI_TIMER	        	      			  ,IDLE           ,CGKServiceManager::OnRAITimer)

ONEVENT(CS_QUEUE_AND_SEND_TIMER                   ,ANYCASE        ,CGKServiceManager::OnCsQueueTimeout)

/****** Events From ConfPartyManager: ********/
ONEVENT(GKMNGR_RESOURCE_INFO_IND                  ,IDLE           ,CGKServiceManager::OnResourceReportGenerated)
/****** Events from Lobby and DNS Manager: ******/
ONEVENT(DNS_RESOLVE_IND			      			  ,IDLE           ,CGKServiceManager::OnDNSResolveInd)
ONEVENT(LOBBY_LRQ_RESPONSE		      			  ,IDLE			  ,CGKServiceManager::OnLobbyAnswerToGkLRQ)

/*****  Events from GkCall: ******/
ONEVENT(GK_CALL_NO_RESPONSE_FROM_PARTY			  ,IDLE			  ,CGKServiceManager::OnGkCallNoResponseFromParty)


ONEVENT(MCUMNGR_TO_GK_LICENSING_IND               		,ANYCASE		  ,CGKServiceManager::OnMcuMngrLicensing)
ONEVENT(FAILOVER_SLAVE_BECOME_MASTER			,ANYCASE			,CGKServiceManager::OnFailoverSlaveBcmMasterInd)
ONEVENT(FAILOVER_REFRESH_GK_REG_IND 				,ANYCASE			,CGKServiceManager::OnFailoverRefreshRegInd)
ONEVENT(FAILOVER_START_MASTER_BECOME_SLAVE		 ,ANYCASE 			,CGKServiceManager::OnFailoverMasterBcmSlaveInd)

//ONEVENT(ON_GW_CONFIGURATION_CHANGE  ,IDLE           ,CGKServiceManager::OnGwConfigurationChange)

PEND_MESSAGE_MAP(CGKServiceManager,CAlarmableTask);


////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

//BEGIN_SET_TRANSACTION_FACTORY(CGKServiceManager)
////ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CGKServiceManager::HandleOperLogin)
//END_TRANSACTION_FACTORY
//
//////////////////////////////////////////////////////////////////////////////
////               Terminal Commands
//////////////////////////////////////////////////////////////////////////////
//BEGIN_TERMINAL_COMMANDS(CGKServiceManager)
//  ONCOMMAND("set_gk_ip",CGKServiceManager::HandleSetGkIp,"set gk ip")
//END_TERMINAL_COMMANDS

extern void GKMonitorEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void GKServiceManagerEntryPoint(void* appParam)
{
	CGKServiceManager* pGKManager = new CGKServiceManager();
	pGKManager->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////
void CGKServiceManager::DeclareStartupConditions()
{
	CActiveAlarm aa(FAULT_GENERAL_SUBJECT,
					AA_NO_IP_SERVICE_PARAMS,
					MAJOR_ERROR_LEVEL,
					"No IP service was received from CSMngr",
					false,
					false);
 	AddStartupCondition(aa);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CGKServiceManager::GetMonitorEntryPoint()
{
	return GKMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGKServiceManager::CGKServiceManager()
{
	m_prevFaultsOpcode      = eGkFaultsAndServiceStatusOk;

	m_pGkToCsInterface 		= new CGKToCsInterface;
	m_pCallThroughGkList 	= new std::set<CGkCall*>;
	m_pHoldCallsList 		= new std::set<CGkCall*>;

	for (int i = 0; i < MAX_SERVICES_NUM; i++)
		m_ServicesTable[i] = NULL;

	m_managementIpAddress = 0;
	m_managementIptype = eIpType_IpV4; // BRIDGE-6051
	m_bIsAvfLicense       = FALSE;
    m_pLastAdHocReport = NULL;
     m_TimeElapsedSinceLastRai = 0;
     m_ServiceId = 0;

}

//////////////////////////////////////////////////////////////////////
CGKServiceManager::~CGKServiceManager()
{
	POBJDELETE(m_pGkToCsInterface);

	CGkCall* pErasedGkCall = NULL;
	CGkCallSet::iterator iter;

	//delete and destroy m_pCallThroughGkList:
	iter =  m_pCallThroughGkList->begin();
	while (iter != m_pCallThroughGkList->end())
	{
		pErasedGkCall = (*iter);
		m_pCallThroughGkList->erase(iter);
		POBJDELETE(pErasedGkCall);
		iter =  m_pCallThroughGkList->begin();
	}
	PDELETE(m_pCallThroughGkList);

	//delete and destroy m_pHoldCallsList:
	iter =  m_pHoldCallsList->begin();
	while (iter != m_pHoldCallsList->end())
	{
		pErasedGkCall = (*iter);
		m_pHoldCallsList->erase(iter);
		POBJDELETE(pErasedGkCall);
		iter =  m_pHoldCallsList->begin();
	}
	PDELETE(m_pHoldCallsList);

	for (int i = 0; i < MAX_SERVICES_NUM; i++)
		POBJDELETE(m_ServicesTable[i]);
    POBJDELETE (m_pLastAdHocReport);
}

//////////////////////////////////////////////////////////////////////
//STATUS CGKServiceManager::HandleTerminalPing(CSegment * seg,std::ostream& answer)
//{
//	PTRACE(eLevelError,"pong to logger");
//	answer << "pong to console";
//	return STATUS_OK;
//}


//////////////////////////////////////////////////////////////////////
void CGKServiceManager::ManagerPostInitActionsPoint()
{
	// this function is called just before WaitForEvent
	PTRACE(eLevelInfoNormal,__FUNCTION__);

	CSegment *pRetSeg = new CSegment;
	SendReqToCSMngr(pRetSeg, CS_GKMNGR_IP_SERVICE_PARAM_REQ);

	//Begin Startup Feature
	CProcessBase* proc = CProcessBase::GetProcess();
	m_managementIptype = proc->m_NetSettings.m_iptype;
	m_managementIpAddress = proc->m_NetSettings.m_ipv4;
	//End Startup Feature
}

//////////////////////////////////////////////////////////////////////
void CGKServiceManager::RequestManagementIp()
{
	CManagerApi api(eProcessMcuMngr);
    STATUS res = api.SendOpcodeMsg(MCUMNGR_GK_MNGMNT_REQ);
	if (res != STATUS_OK)
		FPASSERT(MCUMNGR_GK_MNGMNT_REQ);
}



/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnMcuMngrLicensing(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CGKServiceManager::OnMcuMngrLicensing - License parameters were received.");

	const GK_LICENSING_S *pStruct = (GK_LICENSING_S*)pParam->GetPtr();

	m_bIsAvfLicense = (pStruct->partner_Avaya) ? TRUE : FALSE;

	TRACESTR(eLevelInfoNormal) << "CGKServiceManager::OnMcuMngrLicensing"
                                  << CGKLicensingWrapper(*pStruct);
}


//////////////////////////////////////////////////////////////////////
void CGKServiceManager::SendStartRegistartionTimer(OPCODE opcode, DWORD ticks, DWORD serviceId, DWORD cardState)
{
	CSegment* pSeg = new CSegment;
	*pSeg << serviceId;
	*pSeg << cardState;

	if (opcode == REGISTRATION_POLLING_TIMER)
		if (ticks > GapFromRealRrqInterval)
			ticks -= GapFromRealRrqInterval;

	StartTimer(opcode, ticks * SECOND, pSeg);
}

//////////////////////////////////////////////////////////////////////
void CGKServiceManager::SendStartDnsTimer(DWORD serviceId)
{
	CSegment* pSeg = new CSegment;
	*pSeg << serviceId;
	StartTimer(DNS_POLLING_TIMER, DefaultOfDnsInterval * SECOND, pSeg);
}

//////////////////////////////////////////////////////////////////////
void CGKServiceManager::SendStartPartyMessageTimer(OPCODE opcode, DWORD ticks, DWORD serviceId, DWORD connId, DWORD partyId, cmRASTransaction transactionOpcode)
{
	CSegment* pSeg = new CSegment;
	*pSeg << serviceId;
	*pSeg << connId;
	*pSeg << partyId;
	*pSeg << (DWORD)transactionOpcode;
	StartTimer(opcode, ticks * SECOND, pSeg);
}

//////////////////////////////////////////////////////////////////////
HeaderToGkManagerStruct* CGKServiceManager::AllocateAndGetHeaderFromParty(CSegment* pParam)
{
	DWORD headerLen = sizeof(HeaderToGkManagerStruct);
	HeaderToGkManagerStruct* pHeaderFromParty= (HeaderToGkManagerStruct*)new BYTE[headerLen];
	memset(pHeaderFromParty, 0, headerLen);
	pParam->Get((BYTE*)pHeaderFromParty, headerLen);
	return pHeaderFromParty;
}

/////////////////////////////////////////////////////////////////////////////
void*  CGKServiceManager::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
/*void  CGKMngr::Create(CSegment& appParam)
{
//CTaskApp::Create(appParam,"GKMG");

  // ori's addition (proxy)
  CTaskApp::Create(appParam,CTaskLoader::GetCurrentTaskName(E_gkmg)); // get task name from task loader
  CTaskLoader::SetTaskParams(E_gkmg,this,OS_TASK_PRIORITY); // set task loader params
  // till here

	CGkmnTbl* pGKMTbl = new CGkmnTbl;
	::SetpGKMTbl(pGKMTbl);

	  SetPriority(OS_TASK_PRIORITY);
	  Run();
	  m_pDNSMngrApi->SetGkMngrApi();
}*/


//////////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::ResolveDomainReq(DWORD serviceId, const char* pGkName)
{
	PTRACE2(eLevelInfoNormal, "CGKServiceManager::ResolveDomainReq, resolving name =", pGkName);
	CSegment*  pRetParam = new CSegment;
	*pRetParam << serviceId
			   << pGkName
			   << (WORD)eProcessGatekeeper;

	const COsQueue* pDnsMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessDNSAgent, eManager);

	STATUS res = pDnsMbx->Send(pRetParam,DNS_RESOLVE_DOMAIN_REQ);
}


/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnDNSResolveInd(CSegment* pParam)
{
	TRACEINTO << __FUNCTION__;
	WORD  serviceId = 0;
	ipAddressStruct gkIpAddr[TOTAL_NUM_OF_IP_ADDRESSES];
	ALLOCBUFFER(hostName, DnsQueryNameSize);//H243_NAME_LEN);

	*pParam >> serviceId
			>> hostName;

	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&gkIpAddr[i], 0, sizeof(ipAddressStruct));
		pParam->Get((BYTE*)&gkIpAddr[i], sizeof(ipAddressStruct));
		TRACEINTO << CIpAddrStructWrapper(gkIpAddr[i], "GK DNS RESOLUTION IP");
	}

	CGkService* pService = m_ServicesTable[serviceId];
	if (!pService)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnDNSResolveInd - pService is null (1). Service Id = ", serviceId);
		DEALLOCBUFFER(hostName);
		return;
	}
	eIpType serviceType = pService->GetServiceIpTypes();

	BYTE isIpVerMatch = 0;
	int	j = 0;
	for(j=0; j<TOTAL_NUM_OF_IP_ADDRESSES ; j++)
	{
		if(::IsIpNull(&(gkIpAddr[j])) == false)
		{
			if (gkIpAddr[j].ipVersion == (APIU32)eIpVersion4 && (serviceType == eIpType_Both || serviceType == eIpType_IpV4))
			{
				isIpVerMatch = 1;
				break;
			}
			if (gkIpAddr[j].ipVersion == (APIU32)eIpVersion6 && (serviceType == eIpType_Both || serviceType == eIpType_IpV6))
			{
				isIpVerMatch = 1;
				break;
			}
		}
	}

	if (isIpVerMatch == 0)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnDNSResolveInd - DNS IP DOES NOT MATCH Service type. Service Id = ", serviceId);
		DEALLOCBUFFER(hostName);
		
		return;
	}

	CIpAddressPtr gkIp = CIpAddress::CreateIpAddress(gkIpAddr[j]);

	PTRACE2INT(eLevelError,"CGKServiceManager::OnDNSResolveInd - Service Id = ", serviceId);
	PTRACE2(eLevelError,"CGKServiceManager::OnDNSResolveInd - hostName = ", hostName);
//	PTRACE2INT(eLevelError,"CGKServiceManager::OnDNSResolveInd - gkIpAddr = ", gkIpAddr);

	DeleteTimer(DNS_POLLING_TIMER);

	BYTE bIsError = FALSE;

	if (gkIp.get())
	{
//	    CGkService* pService = m_ServicesTable[serviceId];
		if (!pService)
		{
			PTRACE2INT(eLevelError,"CGKServiceManager::OnDNSResolveInd - pService is null (2). Service Id = ", serviceId);
			return;
		}

		if (pService->GetNumOfDnsWaiting() == 0)
		{
			PTRACE2INT(eLevelError, "CGKServiceManager::OnDNSResolveInd, Card isn't in waiting phase. Service Id = ", serviceId);
			DEALLOCBUFFER(hostName);
			
			return;
		}

		BYTE bIsParamsOk = TRUE;
		BYTE bIsResultOnPrimeGk = FALSE;

		CSmallString gkName = pService->GetGkName();
		const char* gkStr   = gkName.GetString();
		bIsParamsOk = !strncmp(gkStr, hostName, DnsQueryNameSize);//H243_NAME_LEN);
		if (bIsParamsOk)
			bIsResultOnPrimeGk = TRUE;
		else
		{
			CSmallString altGkName = pService->GetNameAltGkConfigured();
			const char* altGkStr   = altGkName.GetString();
			bIsParamsOk = !strncmp(altGkStr, hostName, DnsQueryNameSize);//H243_NAME_LEN);
		}

		if (bIsParamsOk)
		{
			pService->DecreaseNumOfDnsWaiting();

			if (bIsResultOnPrimeGk)
			{
				pService->SetPrimeGkIpFromDns(&(gkIpAddr[j]));
			}
			else
			{
				pService->SetAltGkIpFromDns(&(gkIpAddr[j]));
				pService->InsertAltGkInServiceToAltList();//in a regular case, it is inserted in function InitParam
			}

			if (pService->GetNumOfDnsWaiting() == 0)
			{
				pService->SetParamsReadiness(TRUE);
				//BRIDGE-11938 - the m_managementIpAddress is useless here
				//if (m_managementIpAddress || (m_managementIptype == eIpType_IpV6) )
					CsEndStartup(pService);
			}
		}

		else
		{
			PTRACE2INT(eLevelError, "CGKServiceManager::OnDNSResolveInd, Problem with service or host name. Service Id = ", serviceId);
			bIsError = TRUE;
		}
	}

	else
	{
		PTRACE2INT(eLevelError, "CGKServiceManager::OnDNSResolveInd, Resolution result: Gk Ip = 0, Not sending REGISTER request. Service Id = ", serviceId);
		bIsError = TRUE;
	}

	if (bIsError)
	{
		SendStartDnsTimer(serviceId);
		CreateAndSendUpdateServicePropertiesReq(serviceId, CARD_MINOR_ERROR, eGKFaultDnsHostNotFound, eGKDiscovery, "", DefaultOfRrqInterval);
		UpdateFaultsAndActiveAlarms(serviceId, eGKFaultDnsHostNotFound);
	}
	DEALLOCBUFFER(hostName);
}

////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnDnsPollingTimeout(CSegment* pParam)
{
	WORD serviceId;
	*pParam >> serviceId;

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnDnsPollingTimeout. Service Id = ", serviceId);

    CGkService* pService = m_ServicesTable[serviceId];
	if (pService)
	{
		if (pService->GetNumOfDnsWaiting())
		{
			CIpAddressPtr primaryGkIp = pService->GetGkIp();
			if (primaryGkIp.get())
			{
				const char* gkName = pService->GetGkName();
				if (strlen(gkName) != 0) //send request to DNS
				{
					PTRACE(eLevelInfoNormal,"CGKServiceManager::OnDnsPollingTimeout. No Ip for primary Gk. Wait for DNS answer.");
					ResolveDomainReq(serviceId, gkName);
				}
        else
          PTRACE(eLevelInfoNormal,"CGKServiceManager::OnDnsPollingTimeout. Primary Gk name is null");
			}
      else
        PTRACE(eLevelInfoNormal,"CGKServiceManager::OnDnsPollingTimeout. Primary Gk address is null");
			CIpAddressPtr altGkIp = pService->GetAltGkIpConfigured();

			if (altGkIp.get())
			{
				const char* altGkName = pService->GetNameAltGkConfigured();
				if (strlen(altGkName) != 0)  //send request to DNS
				{
					PTRACE(eLevelInfoNormal,"CGKServiceManager::OnDnsPollingTimeout. No Ip for alternate Gk. Wait for DNS answer.");
					ResolveDomainReq(serviceId, altGkName);
				}
        else
          PTRACE(eLevelInfoNormal,"CGKServiceManager::OnDnsPollingTimeout. Alternate Gk name is null");
			}
      else
        PTRACE(eLevelInfoNormal,"CGKServiceManager::OnDnsPollingTimeout. Alternate Gk address is null");
		}
		else
			PTRACE2INT(eLevelError,"CGKServiceManager::OnDnsPollingTimeout. Service isn't waiting to DNS resolve. Service Id = ", serviceId);
	}
	else
		PTRACE2INT(eLevelError,"CGKServiceManager::OnDnsPollingTimeout. Service not found. Service Id = ", serviceId);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnCsMngrServiceParamsInd_Upd(CSegment* pParam)
{
    int sizeOfStruct = sizeof(GkManagerServiceParamsIndStruct);
    GkManagerServiceParamsIndStruct  sServiceParamsIndSt;
    memset(&sServiceParamsIndSt, 0, sizeOfStruct);
    pParam->Get((BYTE*)&sServiceParamsIndSt, sizeOfStruct);

    DWORD               serviceId = sServiceParamsIndSt.serviceId;
    GkH235AuthParam     CurrentH235Params;

    PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnCsMngrServiceParamsInd_Upd. bIsGkInService = ", sServiceParamsIndSt.bIsGkInService);

    CGkService* pService = m_ServicesTable[serviceId];

    if(NULL == pService || !sServiceParamsIndSt.bIsGkInService)
    {
        PTRACE(eLevelInfoNormal,"CGKServiceManager::OnCsMngrServiceParamsInd_Upd. No service available");
        return;
    }


    pService->Get_H235Params(&CurrentH235Params);

    int IsUpdate = 0;

    if(  (sServiceParamsIndSt.authenticationParams.isAuthenticationEnabled != CurrentH235Params.nIsAuth)
       ||(0 != strcmp(sServiceParamsIndSt.authenticationParams.user_name, CurrentH235Params.AuthUserName))
       ||(0 != strcmp(sServiceParamsIndSt.authenticationParams.password, CurrentH235Params.AuthPassword))
      )
    {//1
        //if(0 == CurrentH235Params.nIsAuth)
        //{
        //    if(TRUE == sServiceParamsIndSt.authenticationParams.isAuthenticationEnabled)
        //        IsUpdate = 1;
        //}
        //else
        //    IsUpdate = 0;
        IsUpdate = 1;
    }

    if(0 != IsUpdate)
    {
       BYTE bIsGRQNeeded = FALSE;
       eRegistrationStatus gkState = pService->GetRegStatus();
       if(eRegister == gkState)
       {
            pService->SetDiscovery(FALSE);
            this->OnURQReq (pService);
       }
       CurrentH235Params.nIsAuth = sServiceParamsIndSt.authenticationParams.isAuthenticationEnabled;
       strncpy(CurrentH235Params.AuthUserName, sServiceParamsIndSt.authenticationParams.user_name, H235MaxAuthUserName -1);
       strncpy(CurrentH235Params.AuthPassword, sServiceParamsIndSt.authenticationParams.password , H235MaxAuthPwd -1);

       pService->Set_H235Params(&CurrentH235Params);

       if(eRegister != gkState)
       {
           PTRACE(eLevelInfoNormal, "URQ delete REGISTRATION_POLLING_TIMER");
           DeleteTimer(REGISTRATION_POLLING_TIMER);


           DeleteTimer(REGISTRATION_POLLING_TIMER);
           BOOL bIsRrqWithoutGrq = FALSE;
           CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
           std::string key = "RRQ_WITHOUT_GRQ";
           if( pServiceSysCfg != NULL )
           	pServiceSysCfg->GetBOOLDataByKey(pService->GetServiceId(), key, bIsRrqWithoutGrq);
           if(bIsRrqWithoutGrq)
        	   this-> CreateAndSendRRQ(pService, FALSE);
           else
        	   this->CreateAndSendGRQ(pService);
           CreateAndSendUpdateServicePropertiesReq(serviceId, CARD_NORMAL, eGkFaultsAndServiceStatusOk, eGKNonRegistrated, "",pService->GetTimeToLive());
           SendStartRegistartionTimer(REGISTRATION_POLLING_TIMER, pService->GetTimeToLive(), serviceId, CARD_NORMAL);
       }
    }
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnCsMngrServiceParamsInd(CSegment* pParam)
{
	//RemoveActiveAlarmByErrorCode(AA_NO_IP_SERVICE_PARAMS);

	int sizeOfStruct = sizeof(GkManagerServiceParamsIndStruct);
	GkManagerServiceParamsIndStruct* pServiceParamsIndSt = new GkManagerServiceParamsIndStruct;
	memset(pServiceParamsIndSt, 0, sizeOfStruct);
	pParam->Get((BYTE*)pServiceParamsIndSt, sizeOfStruct);

	DWORD serviceId = pServiceParamsIndSt->serviceId;
	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnCsMngrServiceParamsInd. Service Id = ", serviceId);

	TRACEINTO << CGkManagerServiceParamsIndStructWrapper(*pServiceParamsIndSt);

	RemoveServiceFromGkCallsTable(serviceId); //remove all the entries of call from the table in case of reset to the card

	/*	if ( IsOnCardEndStartUpMessageIsIllegal(boardId, spanType) )
	{
		PTRACE(eLevelError|H323GK_TRACE,"CGKMngr::OnCardEndStartUp - GateKeeper isn't external - the message ON_CARD_END_STARTUP shouldn't have been sent to the GkMngr" );
		DBGPASSERT_AND_RETURN(boardId);
	}*/
	// In case the service is of one IP type only  - GkManager will remove from CSIp addresses the opposite
	// Address -
	//If IpV6 only - Remove IpV4 CS address and vice versa
	if (pServiceParamsIndSt->service_ip_protocol_types == eIpType_IpV4)
	{
		for (int i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES; i++)
		{
			if (pServiceParamsIndSt->csIp[i].ipVersion == eIpVersion6)
				memset(&(pServiceParamsIndSt->csIp[i]),0,sizeof(ipAddressStruct));
		}
	}
	else if (pServiceParamsIndSt->service_ip_protocol_types == eIpType_IpV6)
	{
		for (int i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES; i++)
		{
			if (pServiceParamsIndSt->csIp[i].ipVersion == eIpVersion4)
			{
				memset(&(pServiceParamsIndSt->csIp[i]),0,sizeof(ipAddressStruct));
				pServiceParamsIndSt->csIp[i].ipVersion = eIpVersion6;
			}

		}
	}

	// Here (In case we have a real GK/Alt GK address) We'll set the order of the CS Ips
	// According to Ip Type + ScopeId (In case of IpV6).
	CIpAddressPtr primeGkIp	= CIpAddress::CreateIpAddress(pServiceParamsIndSt->gkIp);
	CIpAddressPtr alterGkIp	= CIpAddress::CreateIpAddress(pServiceParamsIndSt->alternateGkIp);
	if (primeGkIp.get())
		SetCsIpArrayAccordingToIpTypeAndScopeId(pServiceParamsIndSt,primeGkIp);
	else if (alterGkIp.get())
		SetCsIpArrayAccordingToIpTypeAndScopeId(pServiceParamsIndSt,alterGkIp);



	TRACEINTO << CGkManagerServiceParamsIndStructWrapper(*pServiceParamsIndSt);
	BYTE bIsGRQNeeded = FALSE;
	//first we need to unregister from last gk:
	HandleChangesInService (serviceId, pServiceParamsIndSt->bIsGkInService, &bIsGRQNeeded); //check if the service was update and handle this update

	if (pServiceParamsIndSt->bIsGkInService)
	{
		//BYTE isResetCard = FALSE;	//if the card was reseted or if it's the first time it ends startup.

		CGkService* pService = new CGkService(serviceId);
		if (!pService)
		{
			PTRACE2INT(eLevelError,"CGKMngr::OnCsMngrServiceParamsInd - pService is null. serviceId = ", serviceId);
			PDELETE(pServiceParamsIndSt);
			return;
		}

		pService->InitParam(pServiceParamsIndSt);

		if (m_ServicesTable[serviceId] != NULL)
			POBJDELETE(m_ServicesTable[serviceId]);
		m_ServicesTable[serviceId] = pService;

	/* ///////////////////////DNS ///////////////////////////// */
		CIpAddressPtr primaryGkIp	= pService->GetGkIp();
		CIpAddressPtr altGkIp		= pService->GetAltGkIpConfigured();

		BYTE bNoInfoForPrime = FALSE;
		if ( (primaryGkIp.get() == 0) || (altGkIp.get() == 0) )
		{
			pService->SetNumOfDnsWaiting(0);
			if (!primaryGkIp.get())
			{
				const char* gkName = pService->GetGkName();
				if (strlen(gkName) != 0)  //send request to DNS
				{
					PTRACE(eLevelInfoNormal,"CGKServiceManager::OnCsMngrServiceParamsInd. No Ip for primary Gk. Wait for DNS answer.");
					pService->SetNumOfDnsWaiting(1);
					ResolveDomainReq(serviceId, gkName);
				}
				else
				{
					//pBoard->m_isResetCard	= isResetCard;
					PTRACE(eLevelError,"CGKServiceManager::OnCsMngrServiceParamsInd. No Ip and no name for primary Gk!!");
					bNoInfoForPrime = TRUE;
				}
			}
			if (!altGkIp.get())
			{
				const char* altGkName = pService->GetNameAltGkConfigured();
				//pBoard->m_isResetCard			= isResetCard;
				if (strlen(altGkName) != 0)  //send request to DNS
				{
					PTRACE(eLevelInfoNormal,"CGKServiceManager::OnCsMngrServiceParamsInd. No Ip for alternate Gk. Wait for DNS answer.");
					pService->IncreaseNumOfDnsWaiting();
					ResolveDomainReq(serviceId, altGkName);
				}
				else
					PTRACE(eLevelInfoNormal,"CGKServiceManager::OnCsMngrServiceParamsInd. No Ip and no name for alternate Gk!!");
			}

			if ((pService->GetNumOfDnsWaiting()!= 0) || bNoInfoForPrime)
			{
				SendStartDnsTimer(serviceId);
				PDELETE(pServiceParamsIndSt);
				return;
			}
		}

		/* return to here after the DNS answer */
		if (bIsGRQNeeded) //in case of DNS, we don't won't to set this value, because we will send the GRQ not after the UrqInd, but after the DNS response.
			pService->SetDiscovery(FALSE);

		pService->SetParamsReadiness(TRUE);
		//BRIDGE-11938 - the m_managementIpAddress is useless here
		//if (m_managementIpAddress || (m_managementIptype == eIpType_IpV6) )
			CsEndStartup(pService);
	}

	else
		PTRACE(eLevelInfoNormal,"CGKServiceManager::OnCsMngrServiceParamsInd. No GK in service.");

	PDELETE(pServiceParamsIndSt);
}


/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::CsEndStartup(CGkService* pService)
{
	if (pService->GetDiscovery())//in case the Ip of the GK was changed, we don't have to wait for a response from the GK
	{	//send RRQ without GRQ & GCF :
		BOOL bIsRrqWithoutGrq = FALSE;

		// in order to support gatekeeepres (like SE-200 & PathNavigator) which may be configured to function as
		// "NOT default gatekeeper" and as a result, for any GRQ they respond with GRJ,
		// we send them just a RRQ.
//		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
		std::string key = "RRQ_WITHOUT_GRQ";
//		pSysConfig->GetBOOLDataByKey(key, bIsRrqWithoutGrq);
		if( pServiceSysCfg != NULL )
		   pServiceSysCfg->GetBOOLDataByKey(pService->GetServiceId(), key, bIsRrqWithoutGrq);
		// At this point, we are not able to get know if we are in Avaya enviroment.
		// So sending RRQ without GRQ is done although it is not supported in Avaya env. Thus, we soppose to be responded with RRJ.
		if (bIsRrqWithoutGrq)
			CreateAndSendRRQ(pService, FALSE);
		else
			CreateAndSendGRQ(pService);

	}
}

/////////////////////////////////////////////////////////////////////////////
//check if the service was update and handle this update
void CGKServiceManager::HandleChangesInService(DWORD serviceId, WORD bIsGkInService, BYTE* bIsGRQNeeded)
{
	PTRACE(eLevelInfoNormal,"CGKServiceManager::HandleChangesInService");
	CGkService* pService = m_ServicesTable[serviceId];

	if (pService) //in case the service was update
	{
		pService->SetIsGkInService(bIsGkInService);

		if (pService->IsIpGk())
		{
			if (bIsGkInService) //the GK stayes external:
			{
				*bIsGRQNeeded = TRUE;
				OnURQReq(pService);
				m_ServicesTable[serviceId] = NULL;
				POBJDELETE(pService);
			}

			else //the GK was external but now it's not:
			{
				OnURQReq(pService);
				pService->SetGkIpToZero();
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
/*BYTE CGKServiceManager::IsOnCardEndStartUpMessageIsIllegal(WORD serviceId)
{
BYTE bIsMesageIllegal = FALSE; //legal message

  CGKService* pService = m_ServicesTable[serviceId];

	if ( !IsGKExternal (boardId, spanType) )
	{
	bIsMesageIllegal = ( (pBoard && (pBoard->m_IpGK == 0) && (pBoard->m_waitingBoardForDNS.numOfWaiting == 0) )|| //the GK isn't external and also wasn't external before the update
	(pBoard == NULL) );//the service wasn't update and the Gk is not external
	}
	return bIsMesageIllegal;
}*/


////////////////////////////////////////////////////////////
void CGKServiceManager::CreateAndSendUpdateServicePropertiesReq(DWORD serviceId, WORD cardStatus, eGkFaultsAndServiceStatus eServiceOpcode,
														 eGKConnectionState eGkConnState, char gkId[H243_NAME_LEN], WORD rrqPolingInterval)
{
	GkManagerUpdateServicePropertiesReqStruct st;
	memset(&st, '\0', sizeof(GkManagerUpdateServicePropertiesReqStruct));

	st.serviceId         = serviceId;
	st.serviceStatus 	 = cardStatus;
	st.eServiceOpcode 	 = eServiceOpcode;
	st.rrqPolingInterval = rrqPolingInterval;
	st.eGkConnState      = eGkConnState;
	st.ipPrecedenceAudio = 0;
	st.ipPrecedenceVideo = 0;

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService)
		pService->GetQosParameters(st.ipPrecedenceAudio, st.ipPrecedenceVideo);

	int sizeOfGkId = strlen(gkId);
	if (sizeOfGkId)
		strncpy(st.gkId, gkId, sizeOfGkId);

	TRACEINTO << CGkManagerUpdateServicePropertiesReqStructWrapper(st);

	CSegment* pRetSeg = new CSegment;
	pRetSeg->Put((BYTE*)&st, sizeof(GkManagerUpdateServicePropertiesReqStruct));
	SendReqToCSMngr(pRetSeg, CS_GKMNGR_UPDATE_SERVICE_PROPERTIES_REQ);
}

////////////////////////////////////////////////////////////
void CGKServiceManager::UpdateFaultsAndActiveAlarms(DWORD serviceId, eGkFaultsAndServiceStatus eFaultsOpcode)
{
	if (eFaultsOpcode != eGkFaultsAndServiceStatusOk) //in case of status ok - no message should be sent to faluts and active alarms
	{
		if (m_prevFaultsOpcode != eFaultsOpcode)
		{
			ALLOCBUFFER(faultsStr, 2*ONE_LINE_BUFFER_LEN);
			memset(faultsStr, 0, 2*ONE_LINE_BUFFER_LEN );
			//strncpy(faultsStr, GetFaultsOpcodeAsString(eFaultsOpcode), ONE_LINE_BUFFER_LEN);
			sprintf(faultsStr, "%s, (cs Id %d)", GetFaultsOpcodeAsString(eFaultsOpcode),serviceId);

			BYTE bIsError = IsErrorFaultsOpcode(eFaultsOpcode);
			if (bIsError)
			{
				//active alarms (active alarms send also to fault)
				if (IsErrorFaultsOpcode(m_prevFaultsOpcode) )//Active alarms should contain only the last fault
					RemoveActiveAlarmByUserId(m_prevFaultsOpcode);
				AddActiveAlarm (FAULT_GENERAL_SUBJECT, AA_GATE_KEEPER_ERROR, MAJOR_ERROR_LEVEL, faultsStr, true, true, (DWORD)eFaultsOpcode);
			}
			else //in case of an informative opcode - do not insert it to active alarms. Only to faults
			{
				//Need to remove Active alarm in case there is active alarm on GK error
				if(IsErrorFaultsOpcode(m_prevFaultsOpcode))
					RemoveActiveAlarmByUserId(m_prevFaultsOpcode);
				CHlogApi::GateKeeperMessage(serviceId, SYSTEM_MESSAGE, faultsStr);
			}
			DEALLOCBUFFER(faultsStr);
		}
	}

	//Active alarms should contain errors
	if ( !IsErrorFaultsOpcode(eFaultsOpcode) && IsErrorFaultsOpcode(m_prevFaultsOpcode) )
		RemoveActiveAlarmByUserId(m_prevFaultsOpcode);

	m_prevFaultsOpcode = eFaultsOpcode;
}


///////////////////////////////////////////////////////////////////////////
BYTE CGKServiceManager::IsErrorFaultsOpcode(eGkFaultsAndServiceStatus eFaultsOpcode)
{
	if ((eFaultsOpcode != eGkFaultRegistrationSucceeded) && (eFaultsOpcode != eGkFaultsAndServiceStatusOk) )
		return TRUE;
	else
		return FALSE;
}

///////////////////////////////////////////////////////////////////////////
char* CGKServiceManager::GetFaultsOpcodeAsString(eGkFaultsAndServiceStatus eFaultsOpcode)
{
	if (eFaultsOpcode < NumOfGkFaultsAndServiceStatus)
		return GkFaultsAndServiceStatusNames[eFaultsOpcode];
	else
		return "GK problem";
}

///////////////////////////////////////////////////////////////////////////
BYTE CGKServiceManager::GetFaultsLevelAccordingToOpcde(eGkFaultsAndServiceStatus eFaultsOpcode)
{
	BYTE faultsLevel = MAJOR_ERROR_LEVEL; //for now - all the gk errors are MAJORS. //minors.
	return faultsLevel;
}


///////////////////////////////////////////////////////////////////////////
// This function is been called in case the service was changed and the board was registered to
// Gk before the change. In this case we send URQ to the GK.
BYTE CGKServiceManager::OnURQReq(CGkService* pService)
{
	if (pService->IsIpGk() == FALSE)
		return FALSE;

	int structSize = 0;
	DWORD serviceId = pService->GetServiceId();
	gkReqRasURQ* pUrqReq = pService->CreateURQ(&structSize);
	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(pUrqReq), structSize);
	m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_URQ_REQ, pMsg, serviceId, STATUS_OK, 0, 0, 0, 0, serviceId);
	POBJDELETE(pMsg);
	PDELETEA(pUrqReq);

	UpdateQosParams(pService);
    CreateAndSendUpdateServicePropertiesReq(serviceId, CARD_NORMAL, eGkFaultsAndServiceStatusOk, eGKUnRegistration, "",pService->GetTimeToLive());
	UpdateFaultsAndActiveAlarms(serviceId, eGkFaultsAndServiceStatusOk);

	DeleteTimer(REGISTRATION_POLLING_TIMER);

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////
void CGKServiceManager::CreateAndSendGRQ(CGkService* pService)
{
	PTRACE2INT(eLevelInfoNormal,"CGkService::CreateAndSendGRQ, ServiceId = ",pService->GetServiceId());
	
	BOOL bIsBlocking = CheckBlockOutgoingGRrq();
	if(bIsBlocking)
		return;
	
	DWORD serviceId = pService->GetServiceId();
	if (AVF_DEBUG_MODE == TRUE)
	{
        PTRACE (eLevelInfoNormal, "CGKServiceManager::CreateAndSendGRQ  DEBUG_AVF ON");
		pService->SetIsAvaya(TRUE);
		m_bIsAvfLicense = TRUE;
	}
	int structSize = 0;
	gkReqRasGRQ* pGrqReq = pService->CreateGRQ(m_bIsAvfLicense, &structSize);

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(pGrqReq), structSize);
	m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_GRQ_REQ, pMsg, serviceId, STATUS_OK, 0, 0, 0, 0, serviceId);
	POBJDELETE(pMsg);
	PDELETEA(pGrqReq);

	SendStartRegistartionTimer(REGISTRATION_TIMER, RasMessageTimeout, serviceId);

	pService->SetDiscovery(TRUE);
	CreateAndSendUpdateServicePropertiesReq(serviceId, CARD_NORMAL, eGkFaultsAndServiceStatusOk, eGKDiscovery, "", pService->GetTimeToLive());
	//UpdateFaultsAndActiveAlarms(serviceId, eGkFaultsAndServiceStatusOk);
}


//////////////////////////////////////////////////////////////////////
 void CGKServiceManager::OnCSApiMsg(CSegment *pSeg)
{
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg, CS_API_TYPE);

	CGkMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("***CGKServiceManager::OnCSApiMsg",CS_API_TYPE);

	OPCODE opcode    = pMplMcmsProtocol->getOpcode();
	APIU32 connId    = pMplMcmsProtocol->getPortDescriptionHeaderConnection_id();
	APIU32 partyId   = pMplMcmsProtocol->getPortDescriptionHeaderParty_id();
	APIU32 serviceId = pMplMcmsProtocol->getCentralSignalingHeaderServiceId();
	APIS32 status    = pMplMcmsProtocol->getCentralSignalingHeaderStatus();

	CSegment* pMsgToSend = new CSegment;
	pMsgToSend->Put(connId);
	pMsgToSend->Put(partyId);
	pMsgToSend->Put(serviceId);
	pMsgToSend->Put((APIU32)status);
	if(pMplMcmsProtocol->getDataLen())
		pMsgToSend->Put((unsigned char*)pMplMcmsProtocol->GetData(),pMplMcmsProtocol->getDataLen());
	POBJDELETE(pMplMcmsProtocol);

	DispatchEvent(opcode, pMsgToSend);
	PushMessageToQueue(opcode, eProcessCSApi);

	POBJDELETE(pMsgToSend);
}

//////////////////////////////////////////////////////////////////////
void CGKServiceManager::GetCsHeaderParams(CSegment* pParam, APIU32 *pConnId, APIU32 *pPartyId, APIU32 *pServiceId, APIS32 *pStatus)
{
	APIU32 tempStatus = 0;
	*pConnId = 0;
	*pPartyId = 0;
	*pServiceId = 0;


	*pParam >> *pConnId;
	*pParam >> *pPartyId;
	*pParam >> *pServiceId;
	*pParam	>> tempStatus;

	*pStatus = (APIS32)tempStatus;
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnGRQInd(CSegment* pParam)
{
	APIU32 connId;
	APIU32 partyId;
	APIU32 serviceId;
	APIS32 status;

	GetCsHeaderParams(pParam, &connId, &partyId, &serviceId, &status);

    DWORD msgLen =  pParam->GetWrtOffset() - pParam->GetRdOffset();
	gkIndRasGRQ* pGRQIndSt= (gkIndRasGRQ*)new BYTE[msgLen];
	memset(pGRQIndSt, 0, msgLen);
	pParam->Get((BYTE*)pGRQIndSt, msgLen);

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnGRQInd - Service Id = ", serviceId);

    CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnGRQInd - pService is null. Service Id = ", serviceId);
		PDELETEA(pGRQIndSt);
		DBGPASSERT_AND_RETURN(1);
	}

	DeleteTimer(REGISTRATION_TIMER);

	if (status == STATUS_OK)
	{
		if ( (pService->GetAltGkProcess() == eNotInProcess) ||
			((pService->GetAltGkProcess() != eNotInProcess) && (pService->IsAltGkPermanent() == TRUE)) )
		{//if we register to a temporary alt gk, we don't want to save it's info
            PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnGRQInd GCF : GkIdentLen: ", pGRQIndSt->gkIdentLength);
			pService->SetGkIdent(pGRQIndSt->gatekeeperIdent, pGRQIndSt->gkIdentLength);
			pService->SetGkIdentLen(pGRQIndSt->gkIdentLength);

			//bridge-1283
			APIU8	nullIP[16] 	= {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
											  	   0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

			if((pGRQIndSt->rasAddress.transAddr.ipVersion == eIpVersion4 && pGRQIndSt->rasAddress.transAddr.addr.v4.ip != 0)
					|| (pGRQIndSt->rasAddress.transAddr.ipVersion == eIpVersion6 &&  (memcmp(pGRQIndSt->rasAddress.transAddr.addr.v6.ip, nullIP, IPV6_ADDRESS_BYTES_LEN)) ))
				pService->SetGkIpSt(&pGRQIndSt->rasAddress);
			//pService->m_multcast  = (pService->m_IpGK == 0) ? TRUE : FALSE;
		}

		//----- H.235 Storage Received (required) from GK encrypt. Method
        pService->Set_EncryptMethodRequired(pGRQIndSt->nRequredAuthMethod);

		//If the GK wishes to clear the alt list, it will send an empty list, in the RCF.
		//PN doesn't send a list in the GCF, but only in the RCF
		if (pGRQIndSt->rejectOrConfirmCh.choice.altGkList.numOfAltGks != 0)
			pService->InitAltGkList(&pGRQIndSt->rejectOrConfirmCh.choice.altGkList);
		CreateAndSendRRQ(pService, FALSE/*bIsPolling*/);
	}
	else // send reject reason and start timer for next RRQ/GRQ
	{
		int numOfAlts = pGRQIndSt->rejectOrConfirmCh.choice.altGkList.numOfAltGks;
		BYTE bGotGRJ = TRUE; // If alternate GK process has already started, this flag will keep us in HandleAltGk process.
		eAltGkState bHandleAltGk = IsNeedToHandleAltGk(pService, H323_CS_RAS_GRQ_REQ, connId, numOfAlts, bGotGRJ);

		eGkFaultsAndServiceStatus opcode;
		GetOpcodeForGRJ(pGRQIndSt->rejectOrConfirmCh.choice.rejectInfo.rejectReason, opcode);

		eGKConnectionState gkState = eGKNonRegistrated;
		if (pService->GetRegStatus() != eNonRegistered)
		{
			pService->SetRegStatus(eUnRegistered);
			gkState = eGKUnRegistration;
		}
		CreateAndSendUpdateServicePropertiesReq(serviceId, CARD_MINOR_ERROR, opcode, gkState, "", pService->GetTimeToLive());
		UpdateFaultsAndActiveAlarms(serviceId, opcode);

		// If alt Gk is needed, we don't want the timer in the maintenance, because the
		// re-registration will be done by the alt gk process.
		if (bHandleAltGk == eNotAltGk)
			SendStartRegistartionTimer(REGISTRATION_POLLING_TIMER,pService->GetTimeToLive(), serviceId, CARD_MINOR_ERROR);
		else
			DecideOnPhaseInAltGk(pService, bHandleAltGk, H323_CS_RAS_GRQ_REQ, connId, &pGRQIndSt->rejectOrConfirmCh.choice.rejectInfo);
	}
	PDELETEA(pGRQIndSt);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::CreateAndSendRRQ(CGkService* pService, BYTE bIsPolling)
{
	PTRACE2INT(eLevelInfoNormal,"CGkService::CreateAndSendRRQ, ServiceId = ",pService->GetServiceId());

	int structSize = 0;
    
	BOOL bIsBlocking = CheckBlockOutgoingGRrq();
	if(bIsBlocking)
		return;

	gkReqRasRRQ* pReqRRQ = pService->CreateRRQ(bIsPolling, &structSize);

	if (pReqRRQ->timeToLive == 0)// we can't send at the first RRQ timeToLive zero. we need a value
		pReqRRQ->timeToLive = pService->GetTimeToLive();

	CreateMcuDetails(pReqRRQ->mcuDetails);

	WORD ttl = pReqRRQ->timeToLive;
	DWORD serviceId = pService->GetServiceId();

	//PTRACE(eLevelInfoNormal,"CGkService::CreateAndSendRRQ, Get Service Id %d",serviceId);
	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(pReqRRQ), structSize);
	m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_RRQ_REQ, pMsg, serviceId, STATUS_OK, 0, 0, 0, 0, serviceId);
	PDELETEA(pReqRRQ);
	POBJDELETE(pMsg);

	SendStartRegistartionTimer(REGISTRATION_TIMER, RasMessageTimeout, serviceId);

    CreateAndSendUpdateServicePropertiesReq(serviceId, CARD_NORMAL, eGkFaultsAndServiceStatusOk, eGKRegistration, "", ttl);
	//UpdateFaultsAndActiveAlarms(serviceId, eGkFaultsAndServiceStatusOk);
}

/////////////////////////////////////////////////////////////////////////////
//Second parameter is signal how to treat parameters - update or nullify
void CGKServiceManager::UpdateQosParams(CGkService* pService, gkIndRasRRQ* pRRQIndSt)
{
	if (!pService)
		return;
	//RCF receieved - check DSCP parameters and update service
	if (pRRQIndSt)
	{
		if (pRRQIndSt->fsAvayaFeDscpInd.fsId != H460_K_FsId_Avaya)
		{
			PTRACE(eLevelInfoNormal, "CGKServiceManager::UpdateQosParams - DSCP parameters are not received or feature set ID is incorrect");
			return;
		}
		PTRACE2INT(eLevelInfoNormal, "CGKServiceManager::UpdateQosParams - audio = ", pRRQIndSt->fsAvayaFeDscpInd.audioDscp);
		PTRACE2INT(eLevelInfoNormal, "CGKServiceManager::UpdateQosParams - video = ", pRRQIndSt->fsAvayaFeDscpInd.videoDscp);
		pService->SetQosParameters(pRRQIndSt->fsAvayaFeDscpInd.audioDscp, pRRQIndSt->fsAvayaFeDscpInd.videoDscp);
		return;
	}
	//It is not RCF - nullify DSCP parameters at service
	pService->SetQosParameters(0, 0);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnRRQInd(CSegment* pParam)
{
  APIU32 connId;
  APIU32 partyId;
  APIU32 serviceId;
  APIS32 status;
  GetCsHeaderParams(pParam, &connId, &partyId, &serviceId, &status);

  DWORD        msgLen = pParam->GetWrtOffset() - pParam->GetRdOffset();
  gkIndRasRRQ* pRRQIndSt = (gkIndRasRRQ*)new BYTE[msgLen];
  memset(pRRQIndSt, 0, msgLen);
  pParam->Get((BYTE*)pRRQIndSt, msgLen);

  PTRACE2INT(eLevelInfoNormal, "CGKServiceManager::OnRRQInd - Service Id = ", serviceId);

  CGkService* pService = m_ServicesTable[serviceId];
  if (pService == NULL)
  {
    PDELETEA(pRRQIndSt);
    PTRACE2INT(eLevelError, "CGKServiceManager::OnRRQInd - Failed 'pService' is NULL, ServiceId:", serviceId);
    PASSERT_AND_RETURN(1);
  }

  DeleteTimer(REGISTRATION_TIMER);

  if (status == STATUS_OK)
  {
    // only if it is not RRQ polling, we will get the AVF
    if (pService->GetRegStatus() != eRegister)
    {
      eAVFStatus avfStatus = CheckAvayaStatus(pService, pRRQIndSt->fs);
      if (avfStatus == eAVFAvayaOk)
      {
        // RAI Timer - Start Only if not Started yet (timer is for all the services)
        if (!IsValidTimer(RAI_TIMER))
        {
          pService->ReceivedRAC();
          StartTimer(RAI_TIMER, DefaultOfRAIInterval * SECOND, NULL);
        }

        UpdateQosParams(pService, pRRQIndSt);
      }
      else if (avfStatus != eAVFNotAvaya)
      {
        // problems with Avaya authentication - send URQ
        PDELETEA(pRRQIndSt);
        OnURQReq(pService);
        return;
      }
    }
    else
      StopRAITimerIfNeeded();

    pService->Set_LastRejectReason(0);

	if ( (pService->GetAltGkProcess() == eNotInProcess) ||
		((pService->GetAltGkProcess() != eNotInProcess) && pService->IsAltGkPermanent()) )
	{//if we register to a temporary alt gk, we don't want to save it's info
      if (pRRQIndSt->timeToLive != 0)
      {
        WORD timeToLive = (pRRQIndSt->timeToLive < MIN_RRQ_INTERVAL) ? MIN_RRQ_INTERVAL : pRRQIndSt->timeToLive;
        pService->SetTimeToLive(timeToLive);
      }
      PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnRRQInd RCF: GkIdentLen: ", pRRQIndSt->gkIdentLength);

      pService->SetGkIdent(pRRQIndSt->gatekeeperIdent, pRRQIndSt->gkIdentLength);
      pService->SetGkIdentLen(pRRQIndSt->gkIdentLength);
    }

    pService->SetRRJCounter(0); // init for the next case of RRJ
    pService->SetRegStatus(eRegister);
    pService->SetEpIdent(pRRQIndSt->endpointIdent, pRRQIndSt->epIdentLength);
    pService->SetEpIdentLen(pRRQIndSt->epIdentLength);

    if (pService->GetAltGkProcess())
      HandleAltGk(pService, eRCF);

    pService->InitAltGkList(&pRRQIndSt->rejectOrConfirmCh.choice.altGkList);

    WORD gkIdLen = pService->GetGkIdentLen();
	if (gkIdLen)
	{
		char* bmpGkId = pService->GetGkIdent();
		ALLOCBUFFER(gkIdentBuf, gkIdLen+1);
		memset(gkIdentBuf, 0, gkIdLen+1);
		if(bmpGkId)
			ConvertIdFromBmpString(bmpGkId, gkIdLen, gkIdentBuf);
		else
			PASSERTMSG(1, "GetGkIdent return NULL");
			
        CreateAndSendUpdateServicePropertiesReq(serviceId, CARD_NORMAL, eGkFaultsAndServiceStatusOk, eGKRegistrated, gkIdentBuf, pService->GetTimeToLive());
			
		DEALLOCBUFFER(gkIdentBuf); 
	}		
	else
    {
        CreateAndSendUpdateServicePropertiesReq(serviceId, CARD_NORMAL, eGkFaultsAndServiceStatusOk, eGKRegistrated, "", pService->GetTimeToLive());
    }

    eGkFaultsAndServiceStatus eFaultsOpcode = eGkFaultsAndServiceStatusOk;
    if (m_prevFaultsOpcode != eGkFaultsAndServiceStatusOk)
      eFaultsOpcode = eGkFaultRegistrationSucceeded; // registration succeeded after registration failure

    UpdateFaultsAndActiveAlarms(pService->GetServiceId(), eFaultsOpcode);

    SendStartRegistartionTimer(REGISTRATION_POLLING_TIMER, pService->GetTimeToLive(), serviceId, CARD_NORMAL);
  }
  else // (status != STATUS_OK)
  {
    PTRACE(eLevelError, "CGKServiceManager::OnRRQInd - GK rejected RRQ");

    UpdateQosParams(pService);
    int         numOfAlts = pRRQIndSt->rejectOrConfirmCh.choice.rejectInfo.altGkList.numOfAltGks;
    if(numOfAlts == 0)
      numOfAlts = pService->GetSizeAltGkList();
    eAltGkState bHandleAltGk = IsNeedToHandleAltGk(pService, H323_CS_RAS_RRQ_REQ, connId, numOfAlts);

    // send reject reason
    eGkFaultsAndServiceStatus opcode;
    WORD rrjCounter = pService->GetRRJCounter();
    DWORD reason = pRRQIndSt->rejectOrConfirmCh.choice.rejectInfo.rejectReason;

    // get opcode and new rrj counter (may be initilized to zero)
    GetOpcodeAndRRJCounterForRRJ(reason, opcode, rrjCounter);
    rrjCounter++;
    pService->SetRRJCounter(rrjCounter);

    PTRACE2INT(eLevelError, "===== RRJ Reason:  ", reason);

    pService->Set_LastRejectReason((int)reason);

    // if GK demanding a full registration (no matter if we sent him LWRRQ within time to live or not),
    // lets send full RRQ
    if (opcode == eGkFaultGkRejectedRrqReasonIsFullRegistrationRequired)
    {
      PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnRRQInd RRJ: GkIdentLen: ", pRRQIndSt->gkIdentLength);
      pService->SetGkIdent(pRRQIndSt->gatekeeperIdent, pRRQIndSt->gkIdentLength);
      pService->SetGkIdentLen(pRRQIndSt->gkIdentLength);
      CreateAndSendRRQ(pService, FALSE);
      PDELETEA(pRRQIndSt);
      StopRAITimerIfNeeded();
      return;
    }

    eGKConnectionState eGkState = eGKNonRegistrated;
    if (pService->GetRegStatus() != eNonRegistered)
      pService->SetRegStatus(eUnRegistered);

    if (cmRASReasonSecurityDenial == reason)
      eGkState = eGKNonRegistrated_SecurityDenial;

    CreateAndSendUpdateServicePropertiesReq(serviceId, CARD_MINOR_ERROR, opcode, eGkState, "", pService->GetTimeToLive());
    UpdateFaultsAndActiveAlarms(pService->GetServiceId(), opcode);

    DecideOnPhaseInAltGk(pService, bHandleAltGk, H323_CS_RAS_RRQ_IND, connId, &pRRQIndSt->rejectOrConfirmCh.choice.rejectInfo);
    StopRAITimerIfNeeded();
    SendStartRegistartionTimer(REGISTRATION_POLLING_TIMER, pService->GetTimeToLive(), serviceId, CARD_NORMAL);//HOMOLOGATION. Sasha. RAS_TE_REG_03. Add this line.
  }
  PDELETEA(pRRQIndSt);
}


/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnURQInd(CSegment* pParam)
{
	APIU32 connId;
	APIU32 partyId;
	APIU32 serviceId;
	APIS32 status;
	GetCsHeaderParams(pParam, &connId, &partyId, &serviceId, &status);

    DWORD msgLen =  pParam->GetWrtOffset() - pParam->GetRdOffset();
	gkIndRasURQ* pURQIndSt = (gkIndRasURQ*)new BYTE[msgLen];
	memset(pURQIndSt, 0, msgLen);
	pParam->Get((BYTE*)pURQIndSt, msgLen);

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnURQInd - Service Id = ", serviceId);

    CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnURQInd - pService is null. Service Id = ", serviceId);
		PDELETEA(pURQIndSt);
		return;
	}

    if (status != STATUS_OK)
    {
		PTRACE(eLevelError, "CGKServiceManager::OnURQInd - GK rejected URQ");
        pService->Set_LastRejectReason((int)pURQIndSt->rejectInfo.rejectReason);
    }
    else
        pService->Set_LastRejectReason(0);

	//In case URQ was sent in order to unregister and then re-register again:
	if (pService->GetDiscovery() == FALSE && pService->GetIsGkInService())
	{
		//send RRQ without GRQ & GCF :
		BOOL bIsRrqWithoutGrq = FALSE;
		// in order to support gatekeeepres (like SE-200 & PathNavigator) which may be configured to function as
		// "NOT default gatekeeper" and as a result, for any GRQ they respond with GRJ,
		// we send them just a RRQ.
//		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
		std::string key = "RRQ_WITHOUT_GRQ";
		if( pServiceSysCfg != NULL )
		    pServiceSysCfg->GetBOOLDataByKey(pService->GetServiceId(), key, bIsRrqWithoutGrq);
		// At this point, we are not able to get know if we are in Avaya enviroment.
		// So sending RRQ without GRQ is done although it is not supported in Avaya env. Thus, we soppose to be responded with RRJ.
		if (bIsRrqWithoutGrq)
		    CreateAndSendRRQ(pService, FALSE);
		else
		    CreateAndSendGRQ(pService);
	}

	//The GK was removed from the service. The card sent URQ and now got the answer.
	//The GKMTbl should only contains cards that are supposed to be registered to GK.
	if (pService->IsIpGk() == FALSE)
	{
        CreateAndSendUpdateServicePropertiesReq(pService->GetServiceId(), CARD_NORMAL, eGkFaultsAndServiceStatusOk, eGKNonRegistrated, "", 0);
		UpdateFaultsAndActiveAlarms(pService->GetServiceId(), eGkFaultsAndServiceStatusOk);

		if (pService->GetNumOfDnsWaiting() == 0) //only if gk's ip is 0, because the GK was removed from the service.
			POBJDELETE(m_ServicesTable[serviceId]);
	}

	PDELETEA(pURQIndSt);
    StopRAITimerIfNeeded();
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnTimeOutInd(CSegment* pParam)
{
	APIU32 connId;
	APIU32 partyId;
	APIU32 serviceId;
	APIS32 status;
	GetCsHeaderParams(pParam, &connId, &partyId, &serviceId, &status);

    DWORD msgLen =  pParam->GetWrtOffset() - pParam->GetRdOffset();
	gkIndRasTimeout* pTimeoutIndSt= (gkIndRasTimeout*)new BYTE[msgLen];
	memset(pTimeoutIndSt, 0, msgLen);
	pParam->Get((BYTE*)pTimeoutIndSt, msgLen);

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnTimeOutInd - ServiceId = ", serviceId);

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnTimeOutInd - pService is null. serviceId = ", serviceId);
		PDELETEA(pTimeoutIndSt);
		return;
	}

	if (pService->GetIsGkInService())
	{
		BYTE bHandleTimeout = TRUE;

		//alt gk section:
		if (pTimeoutIndSt)
		{
			if (pTimeoutIndSt->transaction == cmRASResourceAvailability )
			{
				//Treat as if RAC Received
				pService->ReceivedRAC();
				bHandleTimeout = FALSE;
			}
			else if ((pService->GetAltGkProcess() == eNotInProcess) && ((pTimeoutIndSt->transaction == cmRASGatekeeper) || (pTimeoutIndSt->transaction == cmRASRegistration)) )
			{//in case we are not in alt Gk process, there is a chance that the timeout is on a
				//registration message, which was sent before the alt gk process began.
				//Therefore, we should ignore it.
				BYTE bIsSameGkAsCurrent = pService->CompareToGkAddress(&pTimeoutIndSt->gkAddress);
				if (bIsSameGkAsCurrent == FALSE)
				{
					PTRACE(eLevelInfoNormal,"CGKServiceManager::OnTimeOutInd - Gk Ip is different than current Gk, and alt Gk isn't in process. Message is discarded.");
					bHandleTimeout = FALSE;
				}else {
					pService->SetRegStatus(eNonRegistered); // BRIDGE-3523
				}
			}
			else if (pTimeoutIndSt->transaction >= cmRASUnregistration && pTimeoutIndSt->transaction != cmRASAdmission)  // ARQ case : BRIDGE-3523
			{
				PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnTimeOutInd - transaction is not GRQ or RRQ or ARQ. Message is discarded., transaction=", pTimeoutIndSt->transaction);
				bHandleTimeout = FALSE;
			}

			if (bHandleTimeout)
			{
				if (pTimeoutIndSt->transaction != cmRASUnregistration)
				{
					DWORD rasOpcode = TranslateTimeoutOpcodeToRasOpcode(pTimeoutIndSt->transaction);
					if (rasOpcode)
					{
						if (rasOpcode == H323_CS_RAS_RRQ_REQ)
							UpdateQosParams(pService);
						eAltGkState handleAltGk = IsNeedToHandleAltGk(pService, rasOpcode, connId, 0/*numOfAltGks*/, TRUE/*bGkUnresponsive*/);
						if (handleAltGk != eNotAltGk)
						{
							bHandleTimeout = FALSE;
							if ((pTimeoutIndSt->transaction == cmRASGatekeeper) || (pTimeoutIndSt->transaction == cmRASRegistration) )
							{
								pService->SetRegStatus(eNonRegistered);
								if ((handleAltGk == eAnotherTrigger) && (pService->GetAltGkProcess() == eSearchFromAltList))
									pService->SetHoldRRQAfterAltEnd(TRUE);
							}

							else if ((handleAltGk == eAnotherTrigger) ||
								((handleAltGk != eAnotherTrigger) && (pService->GetAltGkProcess() == eNotInProcess) )) //in case of starting the alt process as a result of this timeout
								DecideOnHoldStateAndPutPartyInHold(serviceId, connId, partyId, rasOpcode);

							if (handleAltGk != eAnotherTrigger)
								DecideOnPhaseInAltGk(pService, handleAltGk, rasOpcode, connId);

							if (pService->GetAltGkProcess() == eStartFromOriginal && pTimeoutIndSt->transaction == cmRASDisengage)
								bHandleTimeout = TRUE;
						}

						//check if the timeout is on a call, that was put in a hold state, after the alt gk was already found
						else if( (pTimeoutIndSt->transaction == cmRASAdmission) || (pTimeoutIndSt->transaction == cmRASDisengage) || (pTimeoutIndSt->transaction == cmRASBandwidth))
						{
							CGkCall* pThisCall = GetGkCall(connId);
							if (pThisCall)
							{
								if (pThisCall->IsCallInHoldState())
								{
									bHandleTimeout = FALSE;
									SendReqOnHoldCall(pThisCall);
								}
								else if ( (pTimeoutIndSt->transaction == cmRASAdmission) || (pTimeoutIndSt->transaction == cmRASDisengage) )
								{ // Bug 20013 - In case  the Gk disengages during the call - This is the wrong flow on our part
									// In this case we need to disconnect the call - Since we will never receive answer for this ARQ/DRQ
									bHandleTimeout = TRUE;
								}
							}
						}
					}
				}
			}
			//end of alt gk section

			if (bHandleTimeout)
			{
				switch (pTimeoutIndSt->transaction)
				{
					case cmRASGatekeeper:
					case cmRASRegistration:
						DeleteTimer(REGISTRATION_TIMER);
						InformCsMngrOnTimeout (pService, pTimeoutIndSt->transaction);
						break;

					case cmRASUnregistration:
						InformCsMngrOnTimeout (pService, pTimeoutIndSt->transaction);
						break;

					case cmRASAdmission:
					case cmRASDisengage:
						HandlePartyMessageTimeout(pTimeoutIndSt->transaction, pService, connId, partyId);
						break;

					default:
						PTRACE2INT(eLevelError,"CGKServiceManager::OnTimeOutInd - TimeOut transaction is:" ,pTimeoutIndSt->transaction);
				}
			}
		}
		else
			PTRACE2INT(eLevelError,"CGKServiceManager::OnTimeOutInd - pTimeoutIndSt is null. Service Id = ", serviceId);
	}
	//The GK was removed from the service. The card sent URQ and now got the answer.
	//The GKMTbl should only contains cards that are supposed to be registered to GK.
	else if (pService->IsIpGk() == 0)
//		if (pService->m_waitingBoardForDNS.numOfWaiting == 0) //only if gk's ip is 0, because the GK was removed from the service.
			m_ServicesTable[serviceId] = NULL;

	PDELETEA(pTimeoutIndSt);
}


/////////////////////////////////////////////////////////////////////////////
DWORD CGKServiceManager::TranslateTimeoutOpcodeToRasOpcode(DWORD timeoutOpcode)
{
	DWORD rasOpcode = 0;
	switch (timeoutOpcode)
	{
		case cmRASGatekeeper:
			rasOpcode = H323_CS_RAS_GRQ_REQ;
			break;
		case cmRASRegistration:
			rasOpcode = H323_CS_RAS_RRQ_REQ;
			break;
		case cmRASUnregistration:
			rasOpcode = H323_CS_RAS_URQ_REQ;
			break;
		case cmRASAdmission:
			rasOpcode = H323_CS_RAS_ARQ_REQ;
			break;
		case cmRASDisengage:
			rasOpcode = H323_CS_RAS_DRQ_REQ;
			break;
		case cmRASBandwidth:
			rasOpcode = H323_CS_RAS_BRQ_REQ;
			break;
		case cmRASLocation:
			rasOpcode = H323_CS_RAS_LRQ_REQ;
			break;
		case cmRASInfo:
			rasOpcode = H323_CS_RAS_IRR_REQ;
			break;
		default:
			break;
	}
	return rasOpcode;
}


////////////////////////////////////////////////////////////////////////////
//Timer of sending ARQ/DRQ message to GK - no answer from CS
void CGKServiceManager::OnPartyMessageTimeout(CSegment* pParam)
{
	DWORD serviceId, connId, partyId, transactionOpcode;
	*pParam >> serviceId;
	*pParam >> connId;
	*pParam >> partyId;
	*pParam >> transactionOpcode;

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnPartyMessageTimeout. service Id = ", serviceId);

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnPartyMessageTimeout - pService is null. serviceId = ", serviceId);
		return;
	}

	HandlePartyMessageTimeout((cmRASTransaction)transactionOpcode, pService, connId, partyId);
}


///////////////////////////////////////////////////////////////////////////
//Timer of sending ARQ/DRQ message to GK - no answer from CS
void CGKServiceManager::HandlePartyMessageTimeout(cmRASTransaction transactionOpcode, CGkService* pService, DWORD connId, DWORD partyId)
{
	DWORD serviceId = pService->GetServiceId();
	if ( (transactionOpcode ==cmRASAdmission) || (transactionOpcode == cmRASDisengage) )
	{
		STATUS bRemoved = RemoveCallFromGkCallsTable(connId, serviceId);
		if (bRemoved == STATUS_OK)
			InformParfyOnTimeout (pService, transactionOpcode, connId, partyId);

		if (transactionOpcode == cmRASDisengage)
		{
			m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_REMOVE_GK_CALL_REQ, NULL, serviceId, STATUS_OK, connId, partyId, 0, 0, serviceId);
			if (pService->GetRegStatus() == eUnRegistered)//In case the GK sent GKURQ while there were active caboardIdlls on this board
				if (!AreThereAnyCallsInService(serviceId) && (pService->GetURQhsRas()) )
					HandleGkURQ(pService);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnARQReq(CSegment* pParam)
{
	DWORD arqStructLen;
	*pParam >> arqStructLen;

	gkReqRasARQ* pARQFromParty = (gkReqRasARQ*)new BYTE[arqStructLen];
	memset(pARQFromParty, 0, arqStructLen);
	pParam->Get((BYTE*)pARQFromParty, arqStructLen);

	HeaderToGkManagerStruct* pHeaderFromParty = AllocateAndGetHeaderFromParty(pParam);

	DWORD serviceId = pHeaderFromParty->serviceId;
	DWORD connId    = pHeaderFromParty->connId;
	DWORD partyId   = pHeaderFromParty->partyId;
	DWORD confRsrcId   = pHeaderFromParty->confId;

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnARQReq - connId = ", connId);
	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnARQReq - confRsrcId = ", confRsrcId);

	//check if answer failure to party:
	eGkFailOpcode eStatusToParty;
	BYTE  bSendGkFailToParty = FALSE;
	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnARQReq - pService is null. Service Id = ", serviceId);
		eStatusToParty = eRegistrationProblemStatus;
		bSendGkFailToParty = TRUE;
	}

	else
	{
		if ((pService->GetAltGkProcess() == eStartFromOriginal) || (pService->GetAltGkProcess() == eSearchFromAltListAfterOneCycle))
		{
			CMedString trace_str;
			trace_str << " Connection Id = " << connId;
			trace_str << " ServiceId = "    << serviceId;
			PTRACE2(eLevelError,"CGKServiceManager::OnARQReq - Alt Gk is in process after one cycle of failure. Therefore, ARQ isn't sent.", trace_str.GetString());

			eStatusToParty      = eAltGkProcessStatus;
			bSendGkFailToParty = TRUE;
		}

		else if (pService->GetRegStatus() != eRegister)
		{
			PTRACE2INT(eLevelError,"CGKServiceManager::OnARQReq - Not registered. Therefore, ARQ isn't sent. Service Id = ", serviceId);
			eStatusToParty      = eRegistrationProblemStatus;
			bSendGkFailToParty = TRUE;
		}
	}

	if (bSendGkFailToParty)
		SendGkFailureToParty(connId, partyId, H323_CS_RAS_ARQ_REQ, eStatusToParty);

	else //check if the call is/should be in hold state. If yes, ARQ won't be sent
	{
  		CGkCall* pCallParams = GetGkCall(connId);;
  		BYTE bFound = FALSE;
  		if (pCallParams)
  		{//if this is a hold call (because of a move to alt Gk), we didn't remove the call from the gk calls list when we asked the party to re-send the ARQ
			bFound = TRUE;
			eCallStatus eState = pCallParams->GetCallState();
			if (eState == eArqHold)
				PTRACE(eLevelInfoNormal,"CGKServiceManager::OnARQReq - Current message is in eArqHold state.");
			else
				PTRACE2INT(eLevelError,"CGKServiceManager::OnARQReq - Current message is in gk list, but not in eArqHold state. State = ", (int)eState);
  		}
  		else
  			pCallParams = new CGkCall(GetRcvMbx(), connId, partyId, serviceId, confRsrcId);

		BYTE bCallInHold = FALSE;
		if ((pService->GetAltGkProcess() == eSearchFromAltList) || (pService->GetAltGkProcess() == eSearchFromAltListWithRegistration) )
		{
			CMedString trace_str;
			trace_str << " Mcms Conn Id = " << connId;
			trace_str << ", Service Id = "  << serviceId;
			PTRACE2(eLevelError,"CGKServiceManager::OnARQReq - Alt Gk is in process. Therefore, the call is held.", trace_str.GetString());

			//In case of ARQ, the call isn't yet in m_pCallThroughGkList
			pCallParams->SetCallState(eArqHold);
			m_pCallThroughGkList->insert(pCallParams);

			PutPartyInHold(serviceId, connId, partyId, eArqHold);
			bCallInHold = TRUE;
		}

		if (bCallInHold == FALSE)
		{
			if (!bFound)
			{
				pCallParams->UpdateParams(pARQFromParty->cid, pARQFromParty->callId, pARQFromParty->bIsDialIn, eArqReq);
				m_pCallThroughGkList->insert(pCallParams);
			}
			pService->CreateARQ(pARQFromParty);

			CSegment* pMsg = new CSegment;
			int structSize = sizeof(gkReqRasARQ) - 1; //reduce srcAndDestInfo[1];
			structSize += (pARQFromParty->totalDynLen + 1);
			pMsg->Put((BYTE*)(pARQFromParty), structSize);
			m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_ARQ_REQ, pMsg, serviceId, pHeaderFromParty->status, connId, partyId, 0, 0, serviceId);
			POBJDELETE(pMsg);
		}
	}

	PDELETEA(pARQFromParty);
	PDELETEA(pHeaderFromParty);
}


/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnARQInd(CSegment* pParam)
{
	APIU32 connId;
	APIU32 partyId;
	APIU32 serviceId;
	APIS32 status;
	GetCsHeaderParams(pParam, &connId, &partyId, &serviceId, &status);

    DWORD msgLen =  pParam->GetWrtOffset() - pParam->GetRdOffset();
	gkIndRasARQ* pARQIndSt= (gkIndRasARQ*)new BYTE[msgLen];
	memset(pARQIndSt, 0, msgLen);
	pParam->Get((BYTE*)pARQIndSt, msgLen);

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnARQInd - ConnId = ", connId);

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnARQInd - pService is null. serviceId = ", serviceId);
		PDELETEA(pARQIndSt);
		return;
	}

	CGkCall* pThisCall = GetGkCall(connId);
	if (!pThisCall)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnARQInd - Search in the gatekeeper manager tables has failed. connId = ", connId);
		PDELETEA(pARQIndSt);
		return;
	}

	pThisCall->UpdateCallParamsAccordingToARQInd(pARQIndSt->crv, pARQIndSt->conferenceId, pARQIndSt->callId);

	BYTE bAnswerArqIndToParty = TRUE;

	if ( (pThisCall->GetCallState() == eArqReq) || (pThisCall->GetCallState() == eArqHold))
	{// a case of a regular call
		if(status != STATUS_OK)
		{
			int numOfAlts = pARQIndSt->rejectInfo.altGkList.numOfAltGks;
			eAltGkState bHandleAltGk = IsNeedToHandleAltGk(pService, H323_CS_RAS_ARQ_REQ, connId, numOfAlts);
			if (bHandleAltGk != eNotAltGk)
			{//we will only handle in case the alt gk is temporary or we are already in the alt gk proceess
				CMedString trace_str;
				trace_str << ", Mcms Conn Id = " << connId;
				trace_str << ", Service Id = "   << serviceId;
				trace_str << ", Alt Gk State = " << GetAltGkStateStr(bHandleAltGk);
				PTRACE2(eLevelError,"CGKServiceManager::OnARQInd - Gk Rejected ARQ. Alt Gk Process.", trace_str.GetString());

				if (numOfAlts && !(pARQIndSt->rejectInfo.bAltGkPermanent)) //HANDLING TEMPORARY ALT GK
					//need to check also numOfAlts, because if the structure of alt Gk doesn't exist, bAltGkPermanent is always false
					PTRACE2INT(eLevelInfoNormal, "CGKServiceManager::OnARQInd - We don't support a temporary alt GK. Service number = ", serviceId);
				else
				{
					bAnswerArqIndToParty = FALSE;
					if (pThisCall->GetCallState() != eArqHold)
						DecideOnHoldStateAndPutPartyInHold(serviceId, connId, partyId, H323_CS_RAS_ARQ_REQ);
					if ( (bHandleAltGk == eStart) || (bHandleAltGk == eTimeoutOrRegularReject) )
						DecideOnPhaseInAltGk(pService, bHandleAltGk, H323_CS_RAS_ARQ_REQ, connId, &pARQIndSt->rejectInfo);
				}
			}
			else // (bHandleAltGk == eNotAltGk)
			{
				RemoveCallFromGkCallsTable(pThisCall,serviceId);
			}
		}
		else //status == STATUS_OK
		{
			UpdateCallStateInGkList(pThisCall, eArqInd);

			if (pARQIndSt->crv == 0)
			//the arq has returned with bad field input, because of a problem in the
				PTRACE2INT(eLevelError,"CGKServiceManager::OnARQInd - error. call has arrived with crv zero!! conn id = ", connId);
		}
	}//end of if (pThisCall->GetCallState() == eArqReq) || (pThisCall->GetCallState() == eArqHold)

	else if ((pThisCall->GetCallState() == eDrqAfterArq) || (pThisCall->GetCallState() == eDrqAfterArqHold))
	{
		//the party already sent eDrqReq.
		//so we need to send eDrqReq to the card, if the indication is not ARJ,
		//and after that disregard it's DRQ_IND.
		//Don't send eArqInd to the party
		if(status == STATUS_OK)
		{
			UpdateCallStateInGkList(pThisCall, eSendDrqAfterArq);

			if(pARQIndSt->crv != 0)// maybe not the way to handle this but for the time been this igkConnIds the way
				SendDRQReq(pThisCall);
			else
				DBGPASSERT(connId);
            pService->Set_LastRejectReason(0);
		}
		else 	// the party already sent DRQReq to the GkManager so replay with DCF instead of ARJ
		{
			DWORD partyId = pThisCall->GetPartyId();
			ReplayDRQIndToParty(connId, partyId);
			RemoveCallFromGkCallsTable(pThisCall,serviceId);
            pService->Set_LastRejectReason((int)pARQIndSt->rejectInfo.rejectReason);
		}

		bAnswerArqIndToParty = FALSE;
	}//end of if ((pThisCall->GetCallState() == DRQ_AFTER_ARQ) || (pThisCall->GetCallState() == eDrqAfterArqHold)

	else
	{
		CMedString trace_str;
		trace_str << " Mcms Conn Id = " << connId;
		trace_str << ", Call state = "  << pThisCall->GetCallState();
		PTRACE2(eLevelInfoNormal,"CGKServiceManager::OnARQInd - We succeeded to find an alt gk for this call.", trace_str.GetString());
		bAnswerArqIndToParty = FALSE;
	}

	if (pService->GetAltGkProcess() && (status == STATUS_OK))
	{// Receiving ARQ indication from gatekeeper and sending to party
		//in ARQ we can't check the conn id:
		if (pService->GetTriggerOpcode() == H323_CS_RAS_ARQ_REQ)
		{
			CMedString trace_str;
			trace_str << " Mcms Conn Id = " << connId;
			trace_str << ", Service Id = "  << serviceId;
			PTRACE2(eLevelInfoNormal,"CGKServiceManager::OnARQInd - We succeeded to find an alt gk for this call.", trace_str.GetString());

			HandleAltGk(pService, eReqAck); //we succeeded with finding an alt GK for this ARQ
		}
		//else: error in code!!
	}

	if (bAnswerArqIndToParty)
	{
		ArqIndToPartyStruct stToParty;
		DWORD stSize = sizeof (ArqIndToPartyStruct);
		memset(&stToParty, '\0', stSize);

		stToParty.status = status;

		if (pService->GetIsRegAsGw())
		{
			const char *gkPrefix = pService->GetPrefix();
			if (strlen(gkPrefix))
				strncpy( stToParty.gwPrefix, gkPrefix, PHONE_NUMBER_DIGITS_LEN);
		}

		// The H323Cntl must be updated about the working mode, the service contains this mode
		if (pService->GetIsAvaya() == TRUE)
		{
			if (pARQIndSt->avfFeVndIdInd.fsId != H460_K_FsId_Avaya)
			{
				PTRACE2INT(eLevelError, "CGKServiceManager::OnARQInd - Error. Avaya feature set id is not set!!! ConnId = ", connId);
				pARQIndSt->avfFeVndIdInd.fsId = H460_K_FsId_Avaya;
			}
		}
		else
			pARQIndSt->avfFeVndIdInd.fsId = H460_K_FsId_None;

		CSegment* pSeg = new CSegment;
		pSeg->Put((BYTE*)&stToParty, stSize);

		DWORD arqStructLen = sizeof(gkIndRasARQ);
		pSeg->Put((BYTE*) pARQIndSt, arqStructLen);

	/*	if (pService->IsGkPathNavigator())
		{
			if  (pARQInd// receiving inf. from party and sending DRQ to GatekeeperSt->callModel == cmCallModelTypeGKRouted)
				*seg << (WORD)ePnRouted;
			else
				*seg << (WORD)ePnDirect;
		}
		else
			*seg << (WORD)ePnOff;*/

		SendReqToConfParty(H323_CS_RAS_ARQ_IND, connId, partyId, pSeg);

		//Call Error Handling:
		if (status == STATUS_OK) //in case of ACF
			pThisCall->StartPartyKeepAliveFlow();
	}
	PDELETEA(pARQIndSt);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnDRQReq(CSegment* pParam)
{
	DWORD drqStructLen;
	*pParam >> drqStructLen;

	gkReqRasDRQ* pDRQFromParty = (gkReqRasDRQ*)new BYTE[drqStructLen];
	memset(pDRQFromParty, 0, drqStructLen);
	pParam->Get((BYTE*)pDRQFromParty, drqStructLen);

	HeaderToGkManagerStruct* pHeaderFromParty = AllocateAndGetHeaderFromParty(pParam);

  	DWORD serviceId = pHeaderFromParty->serviceId;
	DWORD connId    = pHeaderFromParty->connId;
	DWORD partyId   = pHeaderFromParty->partyId;

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnDRQReq - conn id = ", connId);

	APIS32 statusToCard = pHeaderFromParty->status;

	CGkCall* pCurrCallParams = GetGkCall(connId);
	if(!pCurrCallParams)
    {
		PTRACE2INT(eLevelError,"CGKServiceManager::OnDRQReq - call not found in Table - conn id = ", connId);
		PDELETEA(pDRQFromParty);
		PDELETEA(pHeaderFromParty);
		ReplayDRQIndToParty(connId, partyId);
		return;
    }

	if (pCurrCallParams->GetCallState() == eArqInd)
	{
		PTRACE(eLevelInfoNormal,"CGKServiceManager::OnDRQReq - search found at eArqInd!!" );
		UpdateCallStateInGkList(pCurrCallParams, eDrqReq);
	}
	else if(pCurrCallParams->GetCallState() == eArqReq)
	{
		PTRACE( eLevelInfoNormal,"CGKServiceManager::OnDRQReq - search found at eArqReq!! - don't send" );
		UpdateCallStateInGkList(pCurrCallParams, eDrqAfterArq);
		PDELETEA(pDRQFromParty);
		PDELETEA(pHeaderFromParty);
		return;
	}

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnDRQReq - no information per service - service id = ",serviceId);
		DWORD partyId = pCurrCallParams->GetPartyId();
		ReplayDRQIndToParty(connId, partyId);
		RemoveCallFromGkCallsTable(pCurrCallParams,serviceId);
	}
	else
    {
		if (pService->GetAltGkProcess() == eSearchFromAltList)
		{
			CMedString trace_str;
			trace_str << " Mcms Conn Id = " << pCurrCallParams->GetCallId();
			trace_str << ", Service Id = "     << pService->GetServiceId();
			PTRACE2(eLevelError,"CGKServiceManager::OnDRQReq - Alt Gk is in process. Therefore, DRQ is held.", trace_str.GetString());

			PutPartyInHold(pService->GetServiceId(), connId, pCurrCallParams->GetPartyId(), eDrqHold);
			PDELETEA(pDRQFromParty);
			PDELETEA(pHeaderFromParty);
			return;
		}

		BYTE bCardNotRegistered = (pService->GetRegStatus() != eRegister) && (pService->GetRegStatus() != eUnRegistered);
		if (bCardNotRegistered || (pService->GetAltGkProcess() == eStartFromOriginal) ||
			(pService->GetAltGkProcess() == eSearchFromAltListAfterOneCycle) )
		{
			if (bCardNotRegistered)
				PTRACE (eLevelError,"CGKServiceManager::OnDRQReq - card isn't registered" );
			else // altGkProcess
			{
				CMedString trace_str;
				trace_str << " Connection Id = " << connId;
				trace_str << " Service Id = "    << serviceId;
				PTRACE2(eLevelError,"CGKServiceManager::OnDRQReq - Alt Gk is in process after one cycle. Therefore, DRQ isn't sent.", trace_str.GetString());
			}

			DWORD partyId = pCurrCallParams->GetPartyId();
			ReplayDRQIndToParty(connId, partyId);

			//delete the entry from the table
			RemoveCallFromGkCallsTable(pCurrCallParams,serviceId);
			statusToCard = -1; //this status means than the stackIf won't send Drq to the Gk and only free the call resources.
		}

		pService->CreateDRQ(pDRQFromParty);
		CSegment* pMsg = new CSegment;
		pMsg->Put((BYTE*)(pDRQFromParty),sizeof(gkReqRasDRQ));
		m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_DRQ_REQ, pMsg, serviceId, statusToCard, connId, partyId, 0, 0, serviceId);
		POBJDELETE(pMsg);

	//	SendStartPartyMessageTimer(DRQ_TIMER, RasMessageTimeout, serviceId, connId, partyId, cmRASDisengage);
	}

    PDELETEA(pDRQFromParty);
    PDELETEA(pHeaderFromParty);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnDRQInd(CSegment* pParam)
{
	APIU32 connId;
	APIU32 partyId;
	APIU32 serviceId;
	APIS32 status;
	GetCsHeaderParams(pParam, &connId, &partyId, &serviceId, &status);

    DWORD msgLen =  pParam->GetWrtOffset() - pParam->GetRdOffset();
	gkIndRasDRQ* pDRQIndSt = (gkIndRasDRQ*)new BYTE[msgLen];
	memset(pDRQIndSt, 0, msgLen);
	pParam->Get((BYTE*)pDRQIndSt, msgLen);

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnDRQInd - conn Id = ",connId);
	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnDRQInd - pService is null. serviceId = ", serviceId);
		PDELETEA(pDRQIndSt);
		return;
	}

	if(pDRQIndSt->rejectInfo.rejectReason)
		PTRACE2INT(eLevelError,"CGKServiceManager::OnDRQInd - Reject reason = ",pDRQIndSt->rejectInfo.rejectReason);

	CGkCall* pThisCall = GetGkCall(connId);
	if (!pThisCall)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnDRQInd - call not found in table - connId  = " , connId);
		PDELETEA(pDRQIndSt);
		return;
	}

	BYTE bDeleteAndAnswerToParty = TRUE;
	BYTE bDeleteOnly = FALSE;

	eCallStatus callState = pThisCall->GetCallState();

	if ( (callState == eDrqReq) || (callState == eSendDrqAfterArq) || (callState == eDrqHold) || (callState == eDrqReqFromGkManager) )
	{
		if (status != STATUS_OK)
		{
            pService->Set_LastRejectReason((int)pDRQIndSt->rejectInfo.rejectReason);

			int numOfAlts = pDRQIndSt->rejectInfo.altGkList.numOfAltGks;
			eAltGkState bHandleAltGk = IsNeedToHandleAltGk(pService, H323_CS_RAS_DRQ_REQ, connId, numOfAlts);
			if (bHandleAltGk != eNotAltGk)
			{
				bDeleteAndAnswerToParty = FALSE;

				CMedString trace_str;
				trace_str << " Mcms Conn Id = "  << connId;
				trace_str << ", Service Id = "   << serviceId;
				trace_str << ", Alt Gk State = " << GetAltGkStateStr(bHandleAltGk);
				PTRACE2(eLevelError,"CGKServiceManager::OnDRQInd - Gk Rejected DRQ. ", trace_str.GetString());

				if (numOfAlts && !pDRQIndSt->rejectInfo.bAltGkPermanent) //HANDLING TEMPORARY ALT GK
				{	//need to check also numOfAlts, because if the structure of alt Gk doesn't exist, bAltGkPermanent is always false
					PTRACE2INT(eLevelInfoNormal, "CGKServiceManager::OnDRQInd - We don't support a temporGetGkCallary alt GK. Service number =", serviceId);
					bDeleteAndAnswerToParty = TRUE;
				}
				else
				{
					if (pThisCall->GetCallState()!= eDrqHold)
						DecideOnHoldStateAndPutPartyInHold(serviceId, connId, partyId, H323_CS_RAS_DRQ_REQ);
					if ( (bHandleAltGk == eStart) || (bHandleAltGk == eTimeoutOrRegularReject) )
						DecideOnPhaseInAltGk(pService, bHandleAltGk, H323_CS_RAS_DRQ_REQ, connId, &pDRQIndSt->rejectInfo);
				}
			}
		}
        else
            pService->Set_LastRejectReason(0);
	}
	else if(callState == eDrqReqFromGkAfterViolentClose)
	{
		RemoveCallFromGkCallsTable(pThisCall, serviceId);
		PDELETEA(pDRQIndSt);//B.S. klocwork 1035
		return;
	}
    else
		PTRACE(eLevelError, "CGKServiceManager::OnDRQInd - error. call is in a different status");

	//In case the GK sent GKURQ while there were active calls on this board
	if (pService->GetRegStatus() == eUnRegistered)
	{
		if (!AreThereAnyCallsInService(serviceId) && pService->GetURQhsRas())
			HandleGkURQ(pService);
	}

	if (pService->GetAltGkProcess() && (status == STATUS_OK))
    {
		if (IsCurrIndMessageSameAsAltGkTrigger(pService, H323_CS_RAS_DRQ_REQ, connId))
		{
			CMedString trace_str;
			trace_str << " Mcms Conn Id = " << connId;
			trace_str << ", Service Id = "  << serviceId;
			PTRACE2(eLevelInfoNormal,"CGKServiceManager::OnDRQInd - We succeeded to find an alt gk for this call.", trace_str.GetString());

			HandleAltGk(pService, eReqAck); //we succeeded with finding an alt GK for this ARQ
		}
		//else: error in code!!
	}

	if (bDeleteAndAnswerToParty)
	{
		DWORD partyId = pThisCall->GetPartyId();
		if (callState == eDrqReqFromGkManager)
		{
			CSegment* pSeg = new CSegment;
			WORD bIsMessageFromRealGk = FALSE;
			*pSeg << bIsMessageFromRealGk;
			SendReqToConfParty(H323_CS_RAS_GKDRQ_IND, connId, partyId, pSeg);
		}
		else
			ReplayDRQIndToParty(connId, partyId);

		RemoveCallFromGkCallsTable(pThisCall, serviceId);
	}
	PDELETEA(pDRQIndSt);
}

/////////////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::HandleGkURQ(CGkService* pService)
{
	gkReqURQResponse* pGkURQResponseReq = pService->CreateURQResponse();
	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(pGkURQResponseReq),sizeof(gkReqURQResponse));
	m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_URQ_RESPONSE_REQ, pMsg, pService->GetServiceId(), STATUS_OK, 0, 0, 0, 0, pService->GetServiceId());
	POBJDELETE(pMsg);
	PDELETE(pGkURQResponseReq);

    //---HOMOLOGATION. Received URQ -> TimeToLive restored from configured -----//
    pService->SetTimeToLive(pService->GetConfiguredTimeToLive());
    PTRACE2INT(eLevelInfoNormal, "CGKServiceManager::HandleGkURQ - RRQ TimeToLive restored from configured:",  pService->GetTimeToLive());
    //--------------------------------------------------------------------------// 
    CreateAndSendUpdateServicePropertiesReq(pService->GetServiceId(), CARD_MINOR_ERROR, eGkFaultUnregistration,
												eGKUnRegistration, "", pService->GetTimeToLive());

	UpdateFaultsAndActiveAlarms(pService->GetServiceId(), eGkFaultUnregistration);

	SendStartRegistartionTimer(REGISTRATION_POLLING_TIMER, pService->GetTimeToLive(), pService->GetServiceId(), CARD_MINOR_ERROR);

	//the GK sent it in order to direct us to another GK
	if(pService->GetGkUrqAltList() &&  (pService->GetGkUrqAltList()->numOfAltGks != 0) )
		pService->SetAltGkPermanent(TRUE); //according to the standard: "A Gatekeeper may send a URQ to an endpoint with a list of Alternate Gatekeepers. The endpoint shall ignore the values of the needToRegister and altGKisPermanent fields and assume that those values are TRUE."
	StartAltGkProcess(pService, 0/*opcode*/, 0/*gkConnId*/, NULL/*pRejectInfo*/, pService->GetGkUrqAltList()); //don't send opcode and message because we don't use them in case of permanent alt gk
}

//////////////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::ReplayDRQIndToParty(DWORD connId, DWORD partyId)
{
	SendReqToConfParty(DRQ_IND_OR_FAIL, connId, partyId);
}

/////////////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::SendGkFailureToParty(DWORD connId, DWORD partyId, DWORD failOpcode, eGkFailOpcode eStatusToParty)
{
	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::SendGkFailureToParty - ConnId = ", connId);

	CSegment* pSeg = new CSegment;
	*pSeg << failOpcode;

	DWORD tempStatus = (DWORD)eStatusToParty;
	*pSeg << tempStatus;

	SendReqToConfParty(H323_CS_RAS_FAIL_IND, connId, partyId, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnDRQFromGKInd(CSegment* pParam)
{
	APIU32 connId;
	APIU32 partyId;
	APIU32 serviceId;
	APIS32 status;
	GetCsHeaderParams(pParam, &connId, &partyId, &serviceId, &status);

    DWORD msgLen =  pParam->GetWrtOffset() - pParam->GetRdOffset();
	gkIndDRQFromGk* pGkDRQIndSt = (gkIndDRQFromGk*)new BYTE[msgLen];
	memset(pGkDRQIndSt, 0, msgLen);
	pParam->Get((BYTE*)pGkDRQIndSt, msgLen);

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnDRQFromGKInd - conn Id = ", connId);

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnDRQFromGKInd - pService is null. serviceId = ", serviceId);
		PDELETEA(pGkDRQIndSt);
		return;
	}

	BYTE bPartySentDrq = FALSE; // the mcms has already sent eDrqReq to the Gatekeeper
	BYTE bIgnoreGkDrq  = FALSE;
	BYTE bViolentClose = FALSE;


	CGkCall* pThisCall = GetGkCall(connId);
    if (!pThisCall)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnDRQFromGKInd - call not found in table - connId is = " , connId);
		// indication for GK
		gkReqDRQResponse *pDrqResponseReq = pService->CreateDRQResponse(pGkDRQIndSt);
		CSegment* pMsg = new CSegment;
		pMsg->Put((BYTE*)(pDrqResponseReq), sizeof(gkReqDRQResponse));
		m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_DRQ_RESPONSE_REQ, pMsg, serviceId, STATUS_OK, connId, partyId, 0, 0, serviceId);
		PDELETE(pDrqResponseReq);
		POBJDELETE(pMsg);
		PDELETEA(pGkDRQIndSt);
		return;
	}

	eCallStatus callState = pThisCall->GetCallState();
	if ( (callState == eArqInd) || (callState == eDrqReqFromGkManager) )
		UpdateCallStateInGkList(pThisCall, eGkDrqInd);

	else if ( (callState == eDrqReq )|| (callState == eDrqReqFromGkAfterViolentClose) )
	{// the mcms has already sent eDrqReq to the Gatekeeper ==> treat this indication as DRQIND
		PTRACE(eLevelError,"CGKServiceManager::OnDRQFromGKInd - call state is eDrqReq!!");
		bPartySentDrq = TRUE;
		RemoveCallFromGkCallsTable(pThisCall,serviceId);
		if(callState == eDrqReqFromGkAfterViolentClose)
			bViolentClose = TRUE;
	}
	else if ((callState == eDrqAfterArq) || (callState == eDrqAfterArqHold) || (callState == eDrqHold))
	{	//the Party already sent DRQ, but drq wasn't sent to GK => ignore the GkeDrqReq
		PTRACE(eLevelError,"CGKServiceManager::OnDRQFromGKInd - Ignore Gk DRQ!!");
		bIgnoreGkDrq = TRUE;
	}

	if (bIgnoreGkDrq == FALSE)
	{
		if (bPartySentDrq)
		{
			if(!bViolentClose)
			{
				// indication for party
				DWORD partyId = 0xFFFFFFFF;
				if(pThisCall)				
					partyId = pThisCall->GetPartyId();
				
				ReplayDRQIndToParty(connId, partyId);
			}

			// indication for GK
			gkReqDRQResponse *pDrqResponseReq = pService->CreateDRQResponse(pGkDRQIndSt);
			CSegment* pMsg = new CSegment;
			pMsg->Put((BYTE*)(pDrqResponseReq), sizeof(gkReqDRQResponse));
			m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_DRQ_RESPONSE_REQ, pMsg, serviceId, STATUS_OK, connId, partyId, 0, 0, serviceId);
			PDELETE(pDrqResponseReq);
			POBJDELETE(pMsg);
		}
		else
		{
			CSegment* pSeg = new CSegment;
			WORD bIsMessageFromRealGk = TRUE;
			*pSeg << bIsMessageFromRealGk;
			pSeg->Put((BYTE*)pGkDRQIndSt, msgLen);
			DWORD partyId = pThisCall->GetPartyId();
			SendReqToConfParty(H323_CS_RAS_GKDRQ_IND, connId, partyId, pSeg);
		}
	}
	PDELETEA(pGkDRQIndSt);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnDRQFromGKResponseReq(CSegment* pParam)
{
	DWORD drqStructLen;
	*pParam >> drqStructLen;

	gkReqDRQResponse* pGkDRQResponseFromParty = (gkReqDRQResponse*)new BYTE[drqStructLen];
	memset(pGkDRQResponseFromParty, 0, drqStructLen);
	pParam->Get((BYTE*)pGkDRQResponseFromParty, drqStructLen);

	HeaderToGkManagerStruct* pHeaderFromParty = AllocateAndGetHeaderFromParty(pParam);

	DWORD serviceId = pHeaderFromParty->serviceId;
	DWORD connId    = pHeaderFromParty->connId;
	DWORD partyId   = pHeaderFromParty->partyId;

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnDRQFromGKResponseReq - conn Id = ", connId);

	// find the GkCall in the list according to GkConnId.
	CGkCall* pCurrCallParams = GetGkCall(pHeaderFromParty->connId);
	if (pCurrCallParams == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnDRQFromGKResponseReq - call not found in table - connId is = " , connId);
		PDELETEA(pGkDRQResponseFromParty);
		PDELETEA(pHeaderFromParty);
		return;
	}

	if(pCurrCallParams->GetCallState() == eGkDrqInd)
		RemoveCallFromGkCallsTable(pCurrCallParams, serviceId);

	CGkService* pService = m_ServicesTable[serviceId];
    if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnDRQFromGKResponseReq - pService is null. serviceId = ", serviceId);
		PDELETEA(pGkDRQResponseFromParty);
		PDELETEA(pHeaderFromParty);
		return;
	}

	APIS32 statusToCs = pHeaderFromParty->status;
	if (pService->GetRegStatus() != eRegister)
		statusToCs = -1;

	pService->CreateDRQResponse(pGkDRQResponseFromParty);
	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(pGkDRQResponseFromParty),sizeof(gkReqDRQResponse));
	m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_DRQ_RESPONSE_REQ, pMsg, pService->GetServiceId(), statusToCs, connId, partyId, 0, 0, pService->GetServiceId());
	POBJDELETE(pMsg);

	PDELETEA(pGkDRQResponseFromParty);
	PDELETEA(pHeaderFromParty);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnIrqFromGKResponseReq(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CGKServiceManager::OnIrqFromGKResponseReq");
	CreateAndSendIrrReq(pParam, H323_CS_RAS_IRR_RESPONSE_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnIrrPollingReq(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CGKServiceManager::OnIrrPollingReq");
	CreateAndSendIrrReq(pParam, H323_CS_RAS_IRR_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::CreateAndSendIrrReq(CSegment* pParam, DWORD opcode)
{
	DWORD irrStructLen;
	*pParam >> irrStructLen;

	gkReqRasIRR* pIRRFromParty = (gkReqRasIRR*)new BYTE[irrStructLen];
	memset(pIRRFromParty, 0, irrStructLen);
	pParam->Get((BYTE*)pIRRFromParty, irrStructLen);

	HeaderToGkManagerStruct* pHeaderFromParty = AllocateAndGetHeaderFromParty(pParam);

	DWORD serviceId = pHeaderFromParty->serviceId;
	DWORD connId    = pHeaderFromParty->connId;
	DWORD partyId   = pHeaderFromParty->partyId;

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::CreateAndSendIrrReq - conn Id = ", connId);

	BYTE bFoundCall = FALSE;
	CGkCall *pCallParams = GetGkCall(connId);
	if (pCallParams)
	{
		if(  (pCallParams->GetCallState() == eArqInd)
           ||(pCallParams->GetCallState() == eDrqReq)//HOMOLOGATION.RAS_TE_DIS_06
          )
			bFoundCall = TRUE;
	}

	if (!bFoundCall)
	{
		SendGkFailureToParty(connId, partyId, opcode);//in order to delete Irr Timer
		PDELETEA(pIRRFromParty);
		PDELETEA(pHeaderFromParty);
		return;
	}

	CGkService* pService = m_ServicesTable[serviceId];

	if (pService)
	{
		if (pService->GetAltGkProcess())
		{
			CMedString trace_str;
			trace_str << " Connection Id = " << connId;
			trace_str << ", Service Id = "    << serviceId;
			PTRACE2(eLevelError,"CGKServiceManager::CreateAndSendIrrReq - Alt Gk is in process. Therefore, IRR isn't sent.", trace_str.GetString());
			PDELETEA(pIRRFromParty);
			PDELETEA(pHeaderFromParty);
			return;
		}
		else if( pService->GetRegStatus() != eRegister )
			SendGkFailureToParty(connId, partyId, opcode);//in order to delete Irr Timer
		else if( pService->GetRegStatus() == eRegister )
		{
			pService->CreateIRR(pIRRFromParty);
			PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::CreateAndSendIrrReq - ConnId is ", connId);
			CSegment seg;
			seg.Put( (BYTE*)(pIRRFromParty), irrStructLen );
			AddMessageToTheCSQueue(opcode, &seg, pService->GetServiceId(), pHeaderFromParty->status, connId, partyId, 0, 0, pService->GetServiceId());
		}
	}
	else
		PTRACE2INT(eLevelError,"CGKServiceManager::CreateAndSendIrrReq - no information per service - serviceId = ", serviceId);

	PDELETEA(pIRRFromParty);
	PDELETEA(pHeaderFromParty);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnGkURQReqInd(CSegment* pParam)
{
	APIU32 connId;
	APIU32 partyId;
	APIU32 serviceId;
	APIS32 status;
	GetCsHeaderParams(pParam, &connId, &partyId, &serviceId, &status);

    DWORD msgLen =  pParam->GetWrtOffset() - pParam->GetRdOffset();
	gkIndURQFromGk* pGkURQIndSt = (gkIndURQFromGk*)new BYTE[msgLen];
	memset(pGkURQIndSt, 0, msgLen);
	pParam->Get((BYTE*)pGkURQIndSt, msgLen);

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnGkURQReqInd - service Id = ", serviceId);

    //------ H.235 GK Authentication -----
    int nIndex_RasReason = pGkURQIndSt->unRegisterReason;
    PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnGkURQReqInd - nIndex_unRegReason = ", nIndex_RasReason);

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnGkURQReqInd - pService is null. serviceId = ", serviceId);
		PDELETEA(pGkURQIndSt);
		return;
	}
	UpdateQosParams(pService);
	pService->SetURQhsRas(pGkURQIndSt->hsRas);
	pService->SetGkUrqAltList(&pGkURQIndSt->altGkList);
    //---HOMOLOGATION. #Test RAS_TE_URG_03 ---------------------------------------//
    if( pService->GetRegStatus() == eUnRegistered )
    {
        gkReqURQResponse* pGkURQResponseReq = pService->CreateURQResponse();
        CSegment* pMsg = new CSegment;
        pMsg->Put((BYTE*)(pGkURQResponseReq),sizeof(gkReqURQResponse));
        m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_URQ_RESPONSE_REQ, pMsg
            , pService->GetServiceId(), cmRASReasonNotCurrentlyRegistered
            , 0, 0, 0, 0, pService->GetServiceId());
        POBJDELETE(pMsg);
        PDELETE(pGkURQResponseReq);
        PDELETEA(pGkURQIndSt);
        return;
    }
    //----------------------------------------------------------------------------//

    //---HOMOLOGATION. Sasha&Misha. RAS_TE_URG_04 --------------------------------//
	BYTE bIsActiveCalls = FindActiveAndHoldCallsInService(serviceId);
    if( bIsActiveCalls )
    {
        gkReqURQResponse* pGkURQResponseReq = pService->CreateURQResponse();
        CSegment* pMsg = new CSegment;
        pMsg->Put((BYTE*)(pGkURQResponseReq),sizeof(gkReqURQResponse));
        m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_URQ_RESPONSE_REQ, pMsg
            , pService->GetServiceId(), cmRASReasonCallInProgress
            , 0, 0, 0, 0, pService->GetServiceId());
        POBJDELETE(pMsg);
        PDELETE(pGkURQResponseReq);
        PDELETEA(pGkURQIndSt);
        return;
    }
    //----------------------------------------------------------------------------//
	pService->SetRegStatus(eUnRegistered);

	BYTE bWaitForDisconnectCalls = RemoveCallsInService(serviceId);
	if (!bWaitForDisconnectCalls)
		HandleGkURQ(pService);

	PDELETEA(pGkURQIndSt);
    StopRAITimerIfNeeded();

    //------ H.235 GK Authentication -----
    // Check URQ reason.
    // cmRASReasonSecurityDenial IN URQ!!!
    // Case: GK has done change in security strategy.
    // Has changed or:
    //      Authentication flag Disable --> Enable
    //      Parameters of user (user name, password)
    if(cmRASReasonSecurityDenial == nIndex_RasReason)
    {
        PTRACE(eLevelInfoNormal, "URQ delete REGISTRATION_POLLING_TIMER");
        DeleteTimer(REGISTRATION_POLLING_TIMER);
        PTRACE(eLevelInfoNormal, "URQ create new REGISTRATION_POLLING_TIMER (via 5 second)");
        SendStartRegistartionTimer(REGISTRATION_POLLING_TIMER, 10, pService->GetServiceId(), CARD_NORMAL);
        pService->SetRegStatus(eNonRegistered);
    }
}


////////////////////////////////////////////////////////////////////////////
//Timer of sending a message to GK - no answer from CS
void CGKServiceManager::OnRegistrationTimeout(CSegment* pParam)
{
	DWORD serviceId;
	WORD cardStatus;
	*pParam >> serviceId;
	*pParam >> cardStatus;

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnRegistrationTimeout. service Id = ", serviceId);

	CGkService *pService = m_ServicesTable[serviceId];
	if (pService != NULL)
		InformCsMngrOnTimeout(pService, cmRASRegistration);
	else
		PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnRegistrationTimeout - pService not found. service Id = ", serviceId);
}

/////////////////////////////////////////////////////////////////////////////
//Timer of reregistration
void CGKServiceManager::OnRegistrationPollingTimeout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CGKServiceManager::OnRegistrationPollingTimeout");

	DWORD serviceId;
	WORD cardStatus;
	*pParam >> serviceId;
	*pParam >> cardStatus;

	CGkService *pService = m_ServicesTable[serviceId];
	if (pService != NULL)
		ReRegisterToGk(pService, cardStatus);
	else
		PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnRegistrationPollingTimeout - pService not found. service Id = ", serviceId);
}

//////////////////////////////////////////////////////////////////////////
void CGKServiceManager::ReRegisterToGk (CGkService* pService, WORD cardStatus)
{
	PTRACE(eLevelInfoNormal,"CGKServiceManager::ReRegisterToGk");
	BYTE bSendRRQ = FALSE;
	bSendRRQ =( !pService->GetAltGkProcess() &&
		(((cardStatus == CARD_NORMAL) && (pService->GetRegStatus() == eRegister)) ||
		((cardStatus == CARD_MINOR_ERROR) && (pService->GetRegStatus() == eUnRegistered) && (pService->GetRRJCounter() == 1))));

	if (bSendRRQ)
		CreateAndSendRRQ(pService);

	else if( (pService->GetAltGkProcess() != eSearchFromAltList)&&
		((cardStatus == CARD_MINOR_ERROR) || ((cardStatus == CARD_NORMAL) && (pService->GetRegStatus() != eRegister)) ))
	{
		BOOL bIsRrqWithoutGrq = FALSE;
		CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
		std::string key = "RRQ_WITHOUT_GRQ";
		if( pServiceSysCfg != NULL )
		      pServiceSysCfg->GetBOOLDataByKey(pService->GetServiceId(), key, bIsRrqWithoutGrq);
		 if(bIsRrqWithoutGrq)
		      this-> CreateAndSendRRQ(pService, FALSE);
		  else
		      this->CreateAndSendGRQ(pService);
	}
	else if (pService->GetAltGkProcess())
		pService->SetHoldRRQAfterAltEnd(TRUE);

	else
		PTRACE2INT(eLevelInfoNormal, "CGKServiceManager::ReRegisterToGk - Message wasn't sent. Serivce Id = ", pService->GetServiceId());
}


//////////////////////////////////////////////////////////////////////////
/*void CGKServiceManager::OnGwConfigurationChange(CSegment* pParam)
{
	DWORD localIP;
	CGkService* pService;
	CCommDynCard* pCard;
	WORD cardstat;

	//for all the cards in the table:
	for (int boardId = 1; boardId < NUM_BOARDS; boardId++)
	{
		localIP = GetCardIp(boardId);
		pService = (CGkService*)( ::GetpGKMTbl()->GetGkmBrdPtr(boardId ,localIP) );
		if(pService)
		{
			pCard = (CCommDynCard*) ::GetpCardDB()->GetCurrentCard(pService->m_BoardId);
			cardstat = pCard->GetState();
			ReRegisterToGk(pService, cardstat);
		}
	}
}
*/
////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnCSMngrDeleteIpServiceInd(CSegment* pParam)
{
	int sizeOfStruct = sizeof(Del_Ip_Service_S);
	Del_Ip_Service_S* pDelServiceIndSt = new Del_Ip_Service_S;
	memset(pDelServiceIndSt, 0, sizeOfStruct);
	pParam->Get((BYTE*)pDelServiceIndSt, sizeOfStruct);

	DWORD serviceId = pDelServiceIndSt->service_id;
	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnCSMngrDeleteIpServiceInd. Service Id = ", serviceId);

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
		PTRACE2INT(eLevelError,"CGKServiceManager::OnCSMngrDeleteIpServiceInd - couldn't get pService. service Id = ", serviceId);
	else
	{
		RemoveServiceFromGkCallsTable(serviceId);
		m_ServicesTable[serviceId] = NULL;
	}

	PDELETE(pDelServiceIndSt);
}

////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnGkFailInd(CSegment* pParam)
{
	APIU32 connId;
	APIU32 partyId;
	APIU32 serviceId;
	APIS32 status;
	GetCsHeaderParams(pParam, &connId, &partyId, &serviceId, &status);

    DWORD msgLen =  pParam->GetWrtOffset() - pParam->GetRdOffset();
	gkIndRasFail* pGkFailIndSt = (gkIndRasFail*)new BYTE[msgLen];
	memset(pGkFailIndSt, 0, msgLen);
	pParam->Get((BYTE*)pGkFailIndSt, msgLen);

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnGkFailInd - service Id = ", serviceId);

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnGkFailInd - pService is null. serviceId = ", serviceId);
		PDELETEA(pGkFailIndSt);
		return;
	}

	if (pService->GetIsGkInService())
	{
		BYTE bHandleGkFailure = TRUE;
		APIU32 opcode = pGkFailIndSt->FailIndicationOpcode;
		DBGPASSERT(opcode);

		if (opcode != H323_CS_RAS_URQ_REQ)
		{//In case of alt Gk: treat it as timeout from GK
			if (opcode == H323_CS_RAS_RRQ_REQ)
				UpdateQosParams(pService);

			eAltGkState handleAltGk = IsNeedToHandleAltGk(pService, opcode, connId, 0, TRUE);
			if (handleAltGk != eNotAltGk)
			{
				bHandleGkFailure = FALSE;
				if ((opcode == H323_CS_RAS_GRQ_REQ) || (opcode == H323_CS_RAS_RRQ_REQ) )
				{
					pService->SetRegStatus(eNonRegistered);
					DeleteTimer(REGISTRATION_TIMER);
				}

				else if ((handleAltGk == eAnotherTrigger) ||
					((handleAltGk != eAnotherTrigger) && !pService->GetAltGkProcess()) ) //in case of starting the alt process as a result of this timeout
					DecideOnHoldStateAndPutPartyInHold(serviceId, connId, partyId, opcode);

				if (handleAltGk != eAnotherTrigger)
					DecideOnPhaseInAltGk(pService, handleAltGk, opcode, connId);
			}
		}

		if (bHandleGkFailure)
		{
			switch (opcode)
			{
				case H323_CS_RAS_GRQ_REQ:
					InformCsMngrOnTimeout (pService, cmRASGatekeeper);
					break;

				case H323_CS_RAS_RRQ_REQ:
					InformCsMngrOnTimeout (pService, cmRASRegistration);
					break;

				case H323_CS_RAS_URQ_REQ:
					InformCsMngrOnTimeout (pService, cmRASUnregistration);
					break;

				case H323_CS_RAS_ARQ_REQ:
				{
					STATUS bRemoved = RemoveCallFromGkCallsTable(connId, serviceId);
					if (bRemoved == STATUS_OK)
						InformParfyOnTimeout(pService, cmRASAdmission, connId, partyId);
					break;
				}

				case H323_CS_RAS_DRQ_REQ:
				{
					STATUS bRemoved = RemoveCallFromGkCallsTable(connId, serviceId);
					if (bRemoved == STATUS_OK)
						InformParfyOnTimeout(pService, cmRASDisengage, connId, partyId);
					m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_REMOVE_GK_CALL_REQ, NULL, pService->GetServiceId(), STATUS_OK, connId, partyId, 0, 0, pService->GetServiceId());
					//In case the GK sent GKURQ while there were active calls on this board
					if (pService->GetRegStatus() == eUnRegistered)
						if (!AreThereAnyCallsInService(pService->GetServiceId()) && (pService->GetURQhsRas()) )
							HandleGkURQ(pService);

					break;
				}
				default:
					PTRACE2INT(eLevelError,"CGKServiceManager::OnGkFailInd - Fail indication opcode = ", opcode);
			}
		}
	}
	PDELETEA(pGkFailIndSt);
}

////////////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::InformCsMngrOnTimeout(CGkService* pService, DWORD transaction, BYTE bIsAltTimeout)
{
	if (transaction == cmRASUnregistration)
	{
		if (pService->GetDiscovery() == FALSE)
		{
			BOOL bIsRrqWithoutGrq = FALSE;
			CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
			std::string key = "RRQ_WITHOUT_GRQ";
			if( pServiceSysCfg != NULL )
				     pServiceSysCfg->GetBOOLDataByKey(pService->GetServiceId(), key, bIsRrqWithoutGrq);
			if(bIsRrqWithoutGrq)
				  CreateAndSendRRQ(pService, FALSE);
		    else
				  CreateAndSendGRQ(pService);
			//In case URQ was sent in order to unregister and then re-register again:


		}
		else
			PTRACE(eLevelError, "CGKServiceManager::InformCsMngrOnTimeout - URQ Timeout");
		return;
	}

	eGKConnectionState eGkState = eGKStateInvalid;
	eGkFaultsAndServiceStatus eCardOpcode = eGkFaultsAndServiceStatusOk;

	if (transaction == cmRASGatekeeper)
	{
		eCardOpcode = eGkFaultDiscovery;//it's not STATUS_TIMEOUT_GATEKEEPER_DISCOVERY in
		eGkState    = eGKDiscovery;
	}
	//order to keep the previous opcode in the maintenance the same as the one for the GRQ.

	if (transaction == cmRASRegistration)
	{
		eCardOpcode = eGkFaultRegistreationTimeout;
		eGkState   = eGKTimeoutRegistration;
	}

	if (pService->GetRegStatus() == eNonRegistered)
		eGkState = eGKNonRegistrated;

	eGkFaultsAndServiceStatus eFaultsOpcode;
	if (!bIsAltTimeout)
		eFaultsOpcode = eCardOpcode;
	else
		eFaultsOpcode = eGkFaultRegistrationToAltGkFailed;

    CreateAndSendUpdateServicePropertiesReq(pService->GetServiceId(), CARD_MINOR_ERROR, eCardOpcode, eGkState, "",pService->GetTimeToLive());
	UpdateFaultsAndActiveAlarms(pService->GetServiceId(), eFaultsOpcode);

	if ((transaction == cmRASGatekeeper) || (transaction == cmRASRegistration))
	{
		SendStartRegistartionTimer(REGISTRATION_POLLING_TIMER, pService->GetTimeToLive(), pService->GetServiceId(), CARD_MINOR_ERROR);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::InformParfyOnTimeout(CGkService* pService, DWORD transaction, DWORD mcmsConnId, DWORD partyId)
{
	if (transaction == cmRASAdmission)
		SendGkFailureToParty(mcmsConnId, partyId, H323_CS_RAS_ARQ_REQ, eArqTimeoutStatus);
	else if (transaction == eAltGkProcessStatus)
		SendGkFailureToParty(mcmsConnId, partyId, H323_CS_RAS_ARQ_REQ, eAltGkProcessStatus);
	else //DRQTIMEOUT
		ReplayDRQIndToParty(mcmsConnId, partyId);
}

////////////////////////////////////////////////////////////////////////////
STATUS CGKServiceManager::RemoveCallFromGkCallsTable(DWORD connId, DWORD serviceId)
{
	CGkCall *pCall = GetGkCall(connId);
	if (!pCall)//not exists
	{
		PTRACE(eLevelError, "CGKServiceManager::RemoveCallFromGkCallsTable - error. call not found in Table.");
		return STATUS_FAIL;
	}

	return RemoveCallFromGkCallsTable(pCall,serviceId);
}

//////////////////////////////////////////////////////////////////////
STATUS CGKServiceManager::RemoveCallFromGkCallsTable(CGkCall *pCall, DWORD serviceId)
{
	STATUS status = RemoveCallFromGkCallsTableWithoutDelete(pCall, serviceId);
	POBJDELETE(pCall);
	return status;
}

//////////////////////////////////////////////////////////////////////
STATUS CGKServiceManager::RemoveCallFromGkCallsTableWithoutDelete(CGkCall *pCall, DWORD serviceId)
{
  	CGkCall* pGkCall = GetGkCall(pCall);
	if (!pGkCall)//not exists
	{
		PTRACE(eLevelError, "CGKServiceManager::RemoveCallFromGkCallsTable - error. call not found in Table.");
		return STATUS_FAIL;
	}
	else if (pGkCall->GetServiceId() != serviceId)
	{
		PTRACE(eLevelError, "CGKServiceManager::RemoveCallFromGkCallsTable - the service Id is not match.");
		return STATUS_FAIL;
	}
	else
	{
		m_pCallThroughGkList->erase(pGkCall);
		return STATUS_OK;
	}
}


/////////////////////////////////////////////////////////////////////////////
//In case the service is deleted
void CGKServiceManager::RemoveServiceFromGkCallsTable(DWORD serviceId)
{
	CGkCallSet::iterator iter = m_pCallThroughGkList->begin();
	CGkCallSet::iterator currentIter;
	CGkCall* pGkCall;

	while (iter !=  m_pCallThroughGkList->end())
	{
		currentIter = iter;
		++iter; // increment iter at the beginning, because maybe currentIter should be removed
		pGkCall = *currentIter;
		if( !CPObject::IsValidPObjectPtr(pGkCall))
		{
			  PTRACE(eLevelError,"CGKServiceManager::RemoveServiceFromGkCallsTable : pGkCall is not valid");
			  DBGPASSERT(YES);
			  continue;
		}

		if(pGkCall->GetServiceId() == serviceId)
		{
			PTRACE2INT(eLevelError,"CGKServiceManager::RemoveServiceFromGkCallsTable - No call should be on this card!! mcmsConnId = ", pGkCall->GetConnId());

			//inform the party on disconnect the call
			if (pGkCall->GetCallState() == eArqInd)
				SendReqToConfParty(STOP_IRRTIMER, pGkCall->GetConnId(), pGkCall->GetPartyId());

			POBJDELETE(pGkCall);
			// remove pGkCall from the list
			m_pCallThroughGkList->erase(currentIter);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
BYTE CGKServiceManager::AreThereAnyCallsInService(DWORD serviceId)
{
	CGkCallSet::iterator iter;
	CGkCall* pGkCall;

	BYTE found = 0;

	for (iter = m_pCallThroughGkList->begin() ; iter !=  m_pCallThroughGkList->end() && !found; iter++)
	{
		pGkCall = *iter;
		if(pGkCall->GetServiceId() == serviceId)
			found = TRUE;
	}

	return found;
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::UpdateCallStateInGkListAfterAltFound(DWORD serviceId, DWORD triggerConnId)
{
	CGkCallSet::iterator iter;
	CGkCall* pGkCall;

	eCallStatus newState;

	PTRACE(eLevelInfoNormal,"CGKServiceManager::UpdateCallStateInGkListAfterAltFound");
	for (iter = m_pCallThroughGkList->begin(); iter != m_pCallThroughGkList->end(); iter++)
	{
		pGkCall = *iter;

		if ( (pGkCall->GetServiceId() == serviceId) && (pGkCall->GetConnId() != triggerConnId) )
		{
			PTRACE(eLevelInfoNormal,"CGKServiceManager::UpdateCallStateInGkListAfterAltFound - Change party state");
			eCallStatus currentState = pGkCall->GetCallState();
			newState = eInvalidCallState;
			if (currentState == eArqReq)
			{
				newState = eArqHold;
				PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::UpdateCallStateInGkListAfterAltFound - Change party state to eArqHold. Mcms Conn Id = ", pGkCall->GetConnId());
			}
			else if (currentState == eDrqAfterArq)
			{
				newState = eDrqAfterArqHold;
				PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::UpdateCallStateInGkListAfterAltFound - Change party state to eDrqAfterArqHold. Mcms Conn Id = ", pGkCall->GetConnId());
			}
			else if ((currentState == eDrqReq) || (currentState == eSendDrqAfterArq) )
			{
				newState = eDrqHold;
				PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::UpdateCallStateInGkListAfterAltFound - Change party state to eDrqHold. Mcms Conn Id = ", pGkCall->GetConnId());
			}

			if (newState != eInvalidCallState)
				UpdateCallStateInGkList(pGkCall, newState);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::UpdateCallStateInGkList(CGkCall* pCallParams, eCallStatus eNewState)
{
	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::UpdateCallStateInGkList - new state = ", eNewState);
	pCallParams->SetCallState(eNewState);
}

/////////////////////////////////////////////////////////////////////////////
//In case the service is deleted
void CGKServiceManager::RemoveServiceFromGkHoldCallsTable(DWORD serviceId)
{
	PTRACE(eLevelInfoNormal,"CGKServiceManager::RemoveServiceFromGkHoldCallsTable");
	if (m_pHoldCallsList->size())
	{
		CGkCallSet::iterator iter = m_pHoldCallsList->begin();
		CGkCallSet::iterator currentIter;
		CGkCall* pHoldCall;

		while (iter !=  m_pHoldCallsList->end())
		{
			currentIter = iter;
			++iter; // increment iter at the beginning, because maybe currentIter should be removed
			pHoldCall = *currentIter;

			if(pHoldCall->GetServiceId() == serviceId)
			{
				POBJDELETE(pHoldCall);
				m_pHoldCallsList->erase(currentIter);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::RemoveCallFromGkHoldCallsTable(DWORD serviceId, WORD mcmsConnId)
{
	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::RemoveServiceFromGkHoldCallsTable - conn id = ", mcmsConnId);
	if (m_pHoldCallsList->size())
	{
		CGkCallSet::iterator iter = m_pHoldCallsList->begin();
		CGkCallSet::iterator currentIter;
		CGkCall* pHoldCall;

		while (iter != m_pHoldCallsList->end())
		{
			currentIter = iter;
			++iter; // increment iter at the beginning, because maybe currentIter should be removed
			pHoldCall = *currentIter;
			if((pHoldCall->GetServiceId() == serviceId) && (pHoldCall->GetConnId() == mcmsConnId))
			{
				POBJDELETE(pHoldCall);
				m_pHoldCallsList->erase(currentIter);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
CGkCall *CGKServiceManager::ResumeHoldCalls(DWORD serviceId, DWORD triggerConnId)
{
	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::ResumeHoldCalls - conn id = ", triggerConnId);
	if (m_pHoldCallsList->size())
	{
		CGkCall* pCurCall;

		CGkCallSet::iterator iter = m_pHoldCallsList->begin();
		CGkCallSet::iterator currentIter;
		CGkCall* pHoldCall;

		while (iter !=  m_pHoldCallsList->end())
		{
			currentIter = iter;
			++iter; // increment iter at the beginning, because maybe currentIter should be removed
			pHoldCall = *currentIter;

			if (pHoldCall->GetServiceId() == serviceId)
			{
				DWORD connId = pHoldCall->GetConnId();
				if (connId != triggerConnId)
				{//if thely are equal, we already send the req to the alt gk:
					pCurCall = GetGkCall(connId);
					if (pCurCall)
						SendReqOnHoldCall(pCurCall);
				}
				POBJDELETE(pHoldCall);
				m_pHoldCallsList->erase(currentIter);
			}
		}
	}

    return NULL;
}

/////////////////////////////////////////////////////////////////////////////
//In case the Gk URQ
BYTE CGKServiceManager::RemoveCallsInService(DWORD servicedId)
{
	BYTE bWaitForDisconnect = FALSE;
	CGkCallSet::iterator iter;
	CGkCall* pGkCall;

	for (iter = m_pCallThroughGkList->begin() ; iter !=  m_pCallThroughGkList->end(); iter++)
	{
		pGkCall = *iter;
		if(pGkCall->GetServiceId() == servicedId)
		{
			PTRACE2INT(eLevelError,"CGKServiceManager::RemoveCallsInBoard - mcmsConnId = ", pGkCall->GetConnId());
			DWORD connId = pGkCall->GetConnId();
			DBGPASSERT(connId);
			DWORD partyId = pGkCall->GetPartyId();

			//inform the party on disconnect the call
			SendReqToConfParty(REMOVE_GK_CALL, connId, partyId);
			CGkService* pService = m_ServicesTable[servicedId];
			if (pService)
				bWaitForDisconnect = TRUE;
		}
	}

	RemoveServiceFromGkHoldCallsTable(servicedId);	//search for hold calls:

	return bWaitForDisconnect;
}
/////////////////////////////////////////////////////////////////////////////
//Homologation
//In case the Gk URQ
BYTE CGKServiceManager::FindActiveAndHoldCallsInService(DWORD servicedId)
{
    BYTE bIsActiveCall = FALSE;
    CGkCallSet::iterator iter;
    CGkCall* pGkCall;

    for (iter = m_pCallThroughGkList->begin() ; iter !=  m_pCallThroughGkList->end(); iter++)
    {
        pGkCall = *iter;
        if(pGkCall->GetServiceId() == servicedId)
        {
                bIsActiveCall = 1;
        }
    }

    if (bIsActiveCall != TRUE && m_pHoldCallsList->size())
    {
        CGkCallSet::iterator iter2;
        CGkCall* pHoldCall;

        for (iter2 = m_pHoldCallsList->begin() ; iter2 !=  m_pHoldCallsList->end() ; iter2++)
        {
            pHoldCall = *iter2;

            if(pHoldCall->GetServiceId() == servicedId)
            {
                bIsActiveCall = 2;
            }				
        }		
    }

    PTRACE2INT(eLevelError,"CGKServiceManager::FindActiveAndHoldCallsInService  - bIsActiveCall = ", bIsActiveCall);

    return bIsActiveCall;
}

/////////////////////////////////////////////////////////////////////
CGkCall* CGKServiceManager::GetGkCall(DWORD connId)
{
	CGkCallHunterByConnID gkCallHunter(connId);
	CGkCallSet::iterator it = find_if(m_pCallThroughGkList->begin(), m_pCallThroughGkList->end (), gkCallHunter);

	if(it == m_pCallThroughGkList->end())
	{
		return NULL;
	}
	return *it;
}

/////////////////////////////////////////////////////////////////////
CGkCall* CGKServiceManager::GetGkCall(CGkCall* pGkCall)
{
	DWORD connId = pGkCall->GetConnId();
	return GetGkCall(connId);
}

//////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::SendDRQReq(CGkCall *pCallParam)
{
	PASSERT_AND_RETURN(NULL == pCallParam);
	APIS32 statusToCs = STATUS_OK;


	DWORD serviceId = pCallParam->GetServiceId();
	CGkService* pService = m_ServicesTable[serviceId];
	if (pService != NULL)
	{
		gkReqRasDRQ* pDRQFromParty = new gkReqRasDRQ;
		InitDRQReqInsteadOfTheParty(pDRQFromParty, pCallParam);

		if ( (pService->GetRegStatus() != eRegister) && (pCallParam->GetCallState()!= eDrqReqFromGkAfterViolentClose) )
		{
			//don't send message to the party. the party already send the DRQ.
			RemoveCallFromGkCallsTable(pCallParam, serviceId);
			statusToCs = -1;
		}

		pService->CreateDRQ(pDRQFromParty);
		CSegment* pMsg = new CSegment;
		pMsg->Put((BYTE*)(pDRQFromParty),sizeof(gkReqRasDRQ));
		DWORD connId  = 0xFFFFFFFF;
		DWORD partyId = 0xFFFFFFFF;
		
		if(pCallParam)  //reg status is eRegister or call state is eDrqReqFromGkAfterViolentClose
		{
			connId  = pCallParam->GetConnId();
			partyId = pCallParam->GetPartyId();
			
		}
		m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_DRQ_REQ, pMsg, pService->GetServiceId(), statusToCs, connId, partyId, 0, 0, pService->GetServiceId());
		POBJDELETE(pMsg);
		PDELETE(pDRQFromParty);

	//	SendStartPartyMessageTimer(DRQ_TIMER, RasMessageTimeout, serviceId, connId, partyId, cmRASDisengage);
	}
	else
		PTRACE2INT(eLevelError,"CGKServiceManager::SendDRQReq - no information per service. Service Id = ", serviceId);
}


/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::InitDRQReqInsteadOfTheParty(gkReqRasDRQ* pDRQFromParty, const CGkCall* pCallParam)
{
	pDRQFromParty->bIsDialIn	    = pCallParam->GetIsDialIn();
	pDRQFromParty->disengageReason  = cmRASDisengageReasonUndefinedReason;
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnListOfGkCallsReq()
{
	if(!m_pCallThroughGkList)
	{
		PTRACE(eLevelError,"CGKServiceManager::OnListOfGkCallsReq ");
		return;
	}

	ALLOCBUFFER(s, 150);
	ALLOCBUFFER(CallsParam, 150 * m_pCallThroughGkList->size());
	strcat(CallsParam, "\n");

	CGkCallSet::iterator iter;
	CGkCall* pGkCall;

	BYTE found = 0;

	for (iter = m_pCallThroughGkList->begin() ; iter !=  m_pCallThroughGkList->end() && !found; iter++)
	{
		pGkCall = *iter;
		int callState = pGkCall->GetCallState();
		int statusLen = 30;
		ALLOCBUFFER(statusBuf, statusLen);
		memset(statusBuf, '\0', statusLen);
		switch (callState)
		{
			case eArqReq:
				strncpy (statusBuf,"eArqReq",statusLen);
			break;
			case eArqHold:
				strncpy (statusBuf,"eArqHold",statusLen);
			break;
			case eArqInd:
				strncpy (statusBuf,"eArqInd",statusLen);
			break;
			case eDrqReq:
				strncpy (statusBuf,"eDrqReq",statusLen);
			break;
			case eDrqHold:
				strncpy (statusBuf,"eDrqHold",statusLen);
			break;
			case eDrqInd:
				strncpy (statusBuf,"eDrqInd",statusLen);
			break;
			case eGkDrqInd:
				strncpy (statusBuf,"eGkDrqInd",statusLen);
			break;
			case eDrqAfterArq:
				strncpy (statusBuf,"eDrqAfterArq",statusLen);
			break;
			case eSendDrqAfterArq:
				strncpy (statusBuf,"eSendDrqAfterArq",statusLen);
			break;
			case eDrqAfterArqHold:
				strncpy (statusBuf,"eDrqAfterArqHold",statusLen);
			break;
			default:
				statusBuf[0] = '\0';
		}

		sprintf(s, "ServiceId = %d , mcmsConnId = %d , CallStatus = %s", pGkCall->GetServiceId(), pGkCall->GetConnId(),statusBuf);

		strcat(CallsParam,s);
		strcat(CallsParam, "\n");

		DEALLOCBUFFER(statusBuf);
	}

	if (m_pCallThroughGkList->size() == 0)
		PTRACE(eLevelError,"Gk calls list is empty");
	else
		PTRACE2(eLevelError,"CALL PARAMETERS:", CallsParam);

	DEALLOCBUFFER(CallsParam);
	DEALLOCBUFFER(s);
}

/////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnGkLRQInd(CSegment* pParam)
{
	APIU32 connId;
	APIU32 partyId;
	APIU32 serviceId;
	APIS32 status;
	GetCsHeaderParams(pParam, &connId, &partyId, &serviceId, &status);

    DWORD msgLen =  pParam->GetWrtOffset() - pParam->GetRdOffset();
	gkIndLRQFromGk* pGkLRQIndSt = (gkIndLRQFromGk*)new BYTE[msgLen];
	memset(pGkLRQIndSt, 0, msgLen);
	pParam->Get((BYTE*)pGkLRQIndSt, msgLen);

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnGkLRQInd - service Id = ", serviceId);

//	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
	BOOL bIsPseudoMode = 0;
	std::string key = "GK_PSEUDO_MODE";
//	pSysConfig->GetBOOLDataByKey(key, bIsPseudoMode);
	if( pServiceSysCfg != NULL )
	   pServiceSysCfg->GetBOOLDataByKey(serviceId, key, bIsPseudoMode);

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnGkLRQInd - pService is null. serviceId = ", serviceId);
		PDELETEA(pGkLRQIndSt);
		return;
	}

	else if (!bIsPseudoMode)
	{
		PTRACE(eLevelError,"CGKServiceManager::OnGkLRQInd - not pseudo Gk mode" );
		CreateAndSendGkLRQResponse(0, pService, pGkLRQIndSt->hsRas);  //send LRJ to the card
	}

	/*else if (pService->m_bIsAvaya)
	{//in case of AVAYA + callId - always behave as active and send directly to lobby
		char* callId = pGkLRQind->callId;
		BYTE bIsCallId = FALSE;
		if (callId != NULL)
		{
			for(int iCheck = 0; iCheck < Size16; iCheck++)
				bIsCallId |= (callId[iCheck] != 0);
		}
		if (bIsCallId)
		{
			PTRACE(eLevelInfoNormal,"CGKServiceManager::OnGKLRQind - Activating lobby");
			SendLrqToLobby(pGkLRQind, callId, serviceId);
			return;
		}
		else //in Avaya we must have call ID
		{
			DBGPASSERT(boardId);
			PTRACE(eLevelError,"CGKServiceManager::OnGKLRQind - No call id in Avaya environment");
			OnGkLRQResponse(0, pService, pGkLRQind->hsRas);  //send LRJ to the card
			return;
		}
	}

	else*/
		SendLrqToLobby(pGkLRQIndSt, /*callId, */serviceId);
	PDELETEA(pGkLRQIndSt);
}

/////////////////////////////////////////////////////////////////////////////////////////
//LRQ: GKMNGR -> LOBBY
void CGKServiceManager::SendLrqToLobby(gkIndLRQFromGk* pGkLRQind,/*char* callId, */ DWORD serviceId)
{
	DWORD hsRas      = pGkLRQind->hsRas;
	char* destInfo   = pGkLRQind->destinationInfo;
	char* sourceInfo = pGkLRQind->sourceInfo;

	CSegment* pSeg = new CSegment;
	*pSeg << destInfo
		  << sourceInfo
		  << hsRas
		  << serviceId;

	SendReqToConfParty(GK_MANAGER_SEARCH_CALL, LOBBY_CONNECTION_ID, DUMMY_PARTY_ID, pSeg);
}

/////////////////////////////////////////////////////////////////////////////////////////
//LRQ: LOBBY -> GKMNGR
void CGKServiceManager::OnLobbyAnswerToGkLRQ(CSegment* pParam)
{
	BYTE  result;
	DWORD hsRas;
	DWORD serviceId;

	*pParam >> result;
	*pParam >> serviceId;
	*pParam >> hsRas;

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnLobbyAnswerToGkLRQ - result is = ",result);

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService)
		CreateAndSendGkLRQResponse(result, pService, hsRas);
	else
		PTRACE2INT(eLevelError,"CGKServiceManager::OnLobbyAnswerToGkLRQ - pService wasn't found - service id = ", serviceId);
}

///////////////////////////////////////////////////////////////////////
// Sending LCF/LRJ to GK
void CGKServiceManager::CreateAndSendGkLRQResponse(BYTE result, CGkService* pService, int hsRas)
{
	APIS32 statusToCs;

	if (result == 0)
		statusToCs = -1;
	else  //send LCF
	{
		statusToCs = STATUS_OK;
	}

	gkReqLRQResponse* pReqGkLRQ = pService->CreateLRQResponse(hsRas);

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(pReqGkLRQ), sizeof(gkReqLRQResponse));
	m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_LRQ_RESPONSE_REQ, pMsg, pService->GetServiceId(), statusToCs, 0, 0, 0, 0, pService->GetServiceId());
	POBJDELETE(pMsg);
	PDELETE(pReqGkLRQ);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnBRQFromGKInd(CSegment* pParam)
{
	APIU32 connId;
	APIU32 partyId;
	APIU32 serviceId;
	APIS32 status;
	GetCsHeaderParams(pParam, &connId, &partyId, &serviceId, &status);

    DWORD msgLen = pParam->GetWrtOffset() - pParam->GetRdOffset();
	gkIndBRQFromGk* pBRQfromGKIndSt = (gkIndBRQFromGk*)new BYTE[msgLen];
	memset(pBRQfromGKIndSt, 0, msgLen);
	pParam->Get((BYTE*)pBRQfromGKIndSt, msgLen);

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnBRQFromGKInd - conn Id = ", connId);

	CGkCall *pThisCall = GetGkCall(connId);
	if (!pThisCall)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnBRQFromGKInd - call not found in table - connId is = " , connId);
		PDELETEA(pBRQfromGKIndSt);
		return;
	}

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnBRQFromGKInd - pService is null. Service Id = ", serviceId);
		PDELETEA(pBRQfromGKIndSt);
		return;
	}

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)pBRQfromGKIndSt, msgLen);

	SendReqToConfParty(H323_CS_RAS_GKBRQ_IND, connId, pThisCall->GetPartyId(), pSeg);
	PDELETEA(pBRQfromGKIndSt);
}


/////////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnBRQFromGKResponseReq(CSegment *pParam)
{
	DWORD brqStructLen;
	*pParam >> brqStructLen;

	gkReqBRQResponse* pBrqFromParty = (gkReqBRQResponse*)new BYTE[brqStructLen];
	memset(pBrqFromParty, 0, brqStructLen);
	pParam->Get((BYTE*)pBrqFromParty, brqStructLen);

	HeaderToGkManagerStruct* pHeaderFromParty = AllocateAndGetHeaderFromParty(pParam);

	DWORD serviceId = pHeaderFromParty->serviceId;
	DWORD connId    = pHeaderFromParty->connId;
	DWORD partyId   = pHeaderFromParty->partyId;

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnBRQFromGKResponseReq - conn id = ", connId);

	CGkCall *pGkCall = GetGkCall(connId);
	if (pGkCall == NULL)
		PTRACE(eLevelError,"CGKServiceManager::OnBRQFromGKResponseReq - call not found in Table ");
	else
	{
		if (pGkCall->GetCallState() == eArqInd)
		{
			CGkService* pService = m_ServicesTable[serviceId];
			if (pService)
			{
				pService->CreateBRQResponse(pBrqFromParty);
				CSegment* pMsg = new CSegment;
				pMsg->Put((BYTE*)(pBrqFromParty),sizeof(gkReqBRQResponse));
				m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_BRQ_RESPONSE_REQ, pMsg, pService->GetServiceId(), pHeaderFromParty->status, connId, partyId, 0, 0, pService->GetServiceId());
				POBJDELETE(pMsg);
			}
			else
				PTRACE2INT(eLevelError,"CGKServiceManager::OnBRQFromGKResponseReq - no information per service. Service Id = ", serviceId);
		}
		else
			PTRACE2INT(eLevelError,"CGKServiceManager::OnBRQFromGKResponseReq - Call state isn't ARQ_IND. conn Id = ", connId);
	}
	PDELETEA(pBrqFromParty);
	PDELETEA(pHeaderFromParty);
}

/////////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnBRQReq(CSegment* pParam)
{
	DWORD brqStructLen;
	*pParam >> brqStructLen;

	gkReqRasBRQ* pBrqFromParty = (gkReqRasBRQ*)new BYTE[brqStructLen];
	memset(pBrqFromParty, 0, brqStructLen);
	pParam->Get((BYTE*)pBrqFromParty, brqStructLen);

	HeaderToGkManagerStruct* pHeaderFromParty = AllocateAndGetHeaderFromParty(pParam);

	DWORD serviceId = pHeaderFromParty->serviceId;
	DWORD connId    = pHeaderFromParty->connId;
	DWORD partyId   = pHeaderFromParty->partyId;

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnBRQReq - conn id = ", connId);

	BYTE bCallFound  = FALSE;
	CGkCall *pThisCall = GetGkCall(connId);
	if (!pThisCall)
		PTRACE2INT(eLevelError,"CGKServiceManager::OnBRQReq - call not found in Table. conn id = ", connId);
	else if (pThisCall->GetCallState() == eArqInd)
		bCallFound = TRUE;
	else
		PTRACE(eLevelInfoNormal, "CGKServiceManager::OnBRQReq - error. call isn't in eArqInd status");

	if (bCallFound)
	{
		BYTE bSendBrqToGk = FALSE;
		CGkService* pService = m_ServicesTable[serviceId];
		if (pService == NULL)
			PTRACE2INT(eLevelError,"CGKServiceManager::OnBRQReq - pService is null. serive Id = ",serviceId);
		else
		{
			if ((pService->GetAltGkProcess() == eStartFromOriginal) || (pService->GetAltGkProcess() == eSearchFromAltListAfterOneCycle) )
			{
				CMedString trace_str;
				trace_str << " Connection Id = " << connId;
				trace_str << ", Service Id = "   << serviceId;
				PTRACE2(eLevelError,"CGKServiceManager::OnBRQReq - Alt Gk is in process after one cycle. Therefore, BRQ isn't sent.", trace_str.GetString());
			}
			else if( pService->GetRegStatus() != eRegister)
				PTRACE(eLevelError,"CGKServiceManager::OnBRQReq - card isn't registered");
			else if (pService->GetAltGkProcess() == eSearchFromAltList)
			{
				CMedString trace_str;
				trace_str << " Connection Id = " << connId;
				trace_str << ", Service Id = "   << serviceId;
				PTRACE2(eLevelError,"CGKServiceManager::OnBRQReq - Alt Gk is in process. Therefore, BRQ is held.", trace_str.GetString());

				PutPartyInHold(serviceId, connId, pThisCall->GetPartyId(), eBrqHold);
			}
			else
				bSendBrqToGk = TRUE;
		}

		if (bSendBrqToGk)
		{
			pService->CreateBRQ(pBrqFromParty);
			CSegment* pMsg = new CSegment;
			pMsg->Put((BYTE*)(pBrqFromParty),sizeof(gkReqRasBRQ));
			m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_BRQ_REQ, pMsg, pService->GetServiceId(), pHeaderFromParty->status, connId, partyId, 0, 0, pService->GetServiceId());
			POBJDELETE(pMsg);
		}
	}
	PDELETEA(pBrqFromParty);
	PDELETEA(pHeaderFromParty);
}

/////////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnBRQInd(CSegment* pParam)
{
	APIU32 connId;
	APIU32 partyId;
	APIU32 serviceId;
	APIS32 status;
	GetCsHeaderParams(pParam, &connId, &partyId, &serviceId, &status);

    DWORD msgLen =  pParam->GetWrtOffset() - pParam->GetRdOffset();
	gkIndRasBRQ* pBRQIndSt = (gkIndRasBRQ*)new BYTE[msgLen];
	memset(pBRQIndSt, 0, msgLen);
	pParam->Get((BYTE*)pBRQIndSt, msgLen);

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnBRQInd - conn id = ", connId);

	if(pBRQIndSt->rejectInfo.rejectReason)
		PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnBRQInd - reject reason = ",pBRQIndSt->rejectInfo.rejectReason);

	BYTE bReturnFromFunc = FALSE;
	CGkCall *pThisCall = GetGkCall(connId);
	if (!pThisCall)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnBRQInd - call not found in Table. conn id = d", connId);
		PDELETEA(pBRQIndSt); //B.S. klocwork 1032
		return;
	}

	if ((pThisCall->GetCallState() != eArqInd) && (pThisCall->GetCallState() != eBrqHold) )
	{
		PTRACE(eLevelInfoNormal, "CGKServiceManager::OnBRQInd - error. call isn't in eArqInd status");
		bReturnFromFunc = TRUE;
	}

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnBRQInd - pService is null. serive Id = ",serviceId);
		bReturnFromFunc = TRUE;
	}

	if (bReturnFromFunc)
	{
		PDELETEA(pBRQIndSt);
		return;
	}

	BYTE bAnswerToParty = TRUE;

	if (status != STATUS_OK)
	{
		int numOfAlts = pBRQIndSt->rejectInfo.altGkList.numOfAltGks;
		eAltGkState bHandleAltGk = IsNeedToHandleAltGk(pService, H323_CS_RAS_BRQ_REQ, connId, numOfAlts);
		if (bHandleAltGk != eNotAltGk)
		{//we will only handle in case the alt gk is temporary or we are already in the alt gk proceess
			bAnswerToParty = FALSE;

			CMedString trace_str;
			trace_str << " Conn Id = " << connId;
			trace_str << ", Service Id = " << serviceId;
			trace_str << ", Alt Gk State = " << GetAltGkStateStr(bHandleAltGk);
			PTRACE2(eLevelError,"CGKServiceManager::OnBRQInd - Gk Rejected BRQ. ", trace_str.GetString());

			if (numOfAlts && !pBRQIndSt->rejectInfo.bAltGkPermanent) //HANDLING TEMPORARY ALT GK
			{//need to check also numOfAlts, because if the structure of alt Gk doesn't exist, bAltGkPermanent is always false
				PTRACE2INT(eLevelInfoNormal, "CGKServiceManager::OnBRQInd - We don't support a temporary alt GK. Service number = ", serviceId);
				bAnswerToParty = TRUE;
			}
			else
			{
				if (pThisCall->GetCallState() != eBrqHold)
					DecideOnHoldStateAndPutPartyInHold(serviceId, connId, pThisCall->GetPartyId(), H323_CS_RAS_BRQ_REQ);

				if ( (bHandleAltGk == eStart) || (bHandleAltGk == eTimeoutOrRegularReject) )
					DecideOnPhaseInAltGk(pService, bHandleAltGk, H323_CS_RAS_BRQ_REQ, connId, &pBRQIndSt->rejectInfo);
			}
		}

        pService->Set_LastRejectReason((int)pBRQIndSt->rejectInfo.rejectReason);
	}

	if (pService->GetAltGkProcess() && (status == STATUS_OK))
	{
		if (IsCurrIndMessageSameAsAltGkTrigger(pService, H323_CS_RAS_BRQ_REQ, connId))
		{
			CMedString trace_str;
			trace_str << " Conn Id = " << connId;
			trace_str << ", Service Id = "  << serviceId;
			PTRACE2(eLevelInfoNormal,"CGKServiceManager::OnBRQInd - We succeeded to find an alt gk for this call.", trace_str.GetString());

			HandleAltGk(pService, eReqAck); //we succeeded with finding an alt GK for this ARQ
		}
		//else: error in code!!

        pBRQIndSt->rejectInfo.rejectReason = 0;
	}

	if (bAnswerToParty)
	{
		CSegment* pSeg = new CSegment;
		pSeg->Put((BYTE*)pBRQIndSt, msgLen);
		SendReqToConfParty(H323_CS_RAS_BRQ_IND, connId, partyId, pSeg);
	}
	PDELETEA(pBRQIndSt);
}

/////////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnGkIRQInd(CSegment* pParam)
{
	APIU32 connId;
	APIU32 partyId;
	APIU32 serviceId;
	APIS32 status;
	GetCsHeaderParams(pParam, &connId, &partyId, &serviceId, &status);

    DWORD msgLen =  pParam->GetWrtOffset() - pParam->GetRdOffset();
	gkIndGKIRQ* pGkIRQIndSt = (gkIndGKIRQ*)new BYTE[msgLen];
	memset(pGkIRQIndSt, 0, msgLen);
	pParam->Get((BYTE*)pGkIRQIndSt, msgLen);

    //---HOMOLOGATION. Sasha&Misha.-----------------------------------------------------//
    PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnGkIRQInd - conn id = ", connId);
    PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnGkIRQInd - partyId = ", partyId);

    if( 0==connId && 0==partyId ) //Michael - handling IRQ with request for all calls
    {
        CGkCallSet::iterator iter;
        CGkCall* pGkCall;

        //pGkIRQIndSt->hsRas = 0;

        if( m_pCallThroughGkList->size() == 0 )
        {
            PTRACE(eLevelInfoNormal,"CGKServiceManager::OnGkIRQInd - m_pCallThroughGkList->size() == 0");

            DWORD irrStructLen = sizeof(gkReqRasIRR);
            gkReqRasIRR* pIrrReq = (gkReqRasIRR*)new BYTE[irrStructLen];
            memset(pIrrReq, '\0', irrStructLen);

            pIrrReq->hsRas			 = pGkIRQIndSt->hsRas;
            pIrrReq->n931Crv         = 0;

            CSegment* pMsg = new CSegment;
            pMsg->Put((BYTE*)(pIrrReq),irrStructLen);
            m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_IRR_RESPONSE_REQ, pMsg, serviceId, 0, connId, partyId, 0, 0, serviceId);
            POBJDELETE(pMsg);
            PDELETEA(pIrrReq);
        }
        else
        {
            PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnGkIRQInd - m_pCallThroughGkList->size() = ", m_pCallThroughGkList->size());
            for (iter = m_pCallThroughGkList->begin() ; iter !=  m_pCallThroughGkList->end(); iter++)
            {
                pGkCall = *iter;
                //if (pGkCall->GetCallState() == eArqInd)
                {
                    CSegment* pSeg = new CSegment;
                    pSeg->Put((BYTE*)pGkIRQIndSt, msgLen);
                    SendReqToConfParty(H323_CS_RAS_GKIRQ_IND, pGkCall->GetConnId(), pGkCall->GetPartyId(), pSeg);
                    PTRACE(eLevelInfoNormal,"CGKServiceManager::OnGkIRQInd - SendReqToConfParty");
                }
            }
        }
        PDELETEA(pGkIRQIndSt);
        return;

    } // homologation end
	CGkCall *pGkCall = GetGkCall(connId);
	if(!pGkCall)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnGkIRQInd - call not found in table. connId = %d", connId);
		PDELETEA(pGkIRQIndSt);
		return;
	}

	BYTE bFoundCall = FALSE;
	if(  (pGkCall->GetCallState() == eArqInd)
       ||(pGkCall->GetCallState() == eDrqReq)
      )
		bFoundCall = TRUE;
	else
		PTRACE(eLevelInfoNormal, "CGKServiceManager::OnGkIRQInd - error. call isn't in eArqInd status");

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnGkIRQInd - pService is null. serive Id = ",serviceId);
		PDELETEA(pGkIRQIndSt);
		return;
	}

	if (bFoundCall)
	{
		CSegment* pSeg = new CSegment;
		pSeg->Put((BYTE*)pGkIRQIndSt, msgLen);
		SendReqToConfParty(H323_CS_RAS_GKIRQ_IND, connId, partyId, pSeg);
	}
	PDELETEA(pGkIRQIndSt);
}


//////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//Path Navigator
//////////////////////////////////////////////////////////////////////////
void CGKServiceManager::CreateMcuDetails (ctNonStandardParameterSt &mcuDetails)
{
	CreateNonStandardInfo(mcuDetails.info);

	char ipStr[32];
	if (m_managementIpAddress != 0)
	{
		SystemDWORDToIpString(m_managementIpAddress, ipStr);
	}
	else // BRIDGE-6051
	{
		snprintf(ipStr, 32, "::1" );
	}
	snprintf(mcuDetails.data, sizeof(mcuDetails.data), "MCU_IP:%s;MCU_SW:2.1;", ipStr);
}

//////////////////////////////////////////////////////////////////////////
void CGKServiceManager::CreateNonStandardInfo (ctNonStandardIdentifierSt &info)
{
info.manufacturerCode = Accord_manufacturerCode;
info.t35CountryCode   = Israel_t35CountryCode;
info.t35Extension     = Israel_t35Extension;
}

///////////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::GetOpcodeForGRJ (DWORD rejectReason, eGkFaultsAndServiceStatus &opcode)
{
	switch (rejectReason)
	{
		case cmRASReasonResourceUnavailable:   //0    - gatekeeper resources exhausted
		case cmRASResourceUnavailable:		   //26
			{ opcode = eGkFaultGkRejectedGrqReasonIsResourceUnavailable; break;}

		case cmRASReasonInvalidRevision:       //2
			{ opcode = eGkFaultGkRejectedGrqReasonIsInvalidRevision; break;}

		case cmRASReasonUndefined:             //18
			{ opcode = eGkFaultGkRejectedGrqReasonIs18; break;}

		case cmRASReasonTerminalExcluded:      //19     - permission failure, not a resource failure
			{ opcode = eGkFaultGkRejectedGrqReasonIsTerminalExcluded; break;}

		case cmRASReasonSecurityDenial:        //24
			{ opcode = eGkFaultGkRejectedGrqReasonIsSecurityDenial; break;}

		case cmRASReasonGenericData:               //39
			{ opcode = eGkFaultGkRejectedGrqReasonIsGenericData; break;}

		case cmRASReasonNeededFeatureNotSupported: //40
			{ opcode = eGkFaultGkRejectedGrqReasonIsNeededFeatureNotSupported; break;}

		default:
			{ opcode = eGkFaultGkRejectedGrqReasonIsStatusIllegal; break;}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::GetOpcodeAndRRJCounterForRRJ (DWORD rejectReason, eGkFaultsAndServiceStatus &opcode, WORD& RRJCounter)
{
	// Rules for RRJCounter:
	//	When we receive the first RRJ, we send full RRQ immediately.
	//	After that, if we receive RRJ, we send GRQ every 120 seconds.

	switch (rejectReason)
	{
		case cmRASReasonResourceUnavailable:   //0    - gatekeeper resources exhausted
		case cmRASResourceUnavailable: //26
			{ opcode = eGkFaultGkRejectedRrqReasonIsResourceUnavailable; break;}

		case cmRASReasonInvalidRevision:             //2
			{ opcode = eGkFaultGkRejectedRrqReasonIsInvalidRevision; break;}

		case cmRASReasonInvalidCallSignalAddress:    //3
			{ opcode = eGkFaultGkRejectedRrqReasonIsInvalidCallSignalAddress; break;}

		case cmRASReasonInvalidRASAddress:           //4   supplied address is invalid
			{ opcode = eGkFaultGkRejectedRrqReasonIsInvalidRasAddress; break;}

		case cmRASReasonInvalidTerminalType:         //5
			{ opcode = eGkFaultGkRejectedRrqReasonIsTerminalType; break;}

		case cmRASReasonDiscoveryRequired:          //11  - registration permission has aged
			{
				opcode = eGkFaultGkRejectedRrqReasonIsDiscoveryRequired;
				if (RRJCounter == 0)
					RRJCounter++; //in order to send GRQ next time
				break;
			}

		case cmRASReasonDuplicateAlias:              //12    - alias registered to another endpoint
			{ opcode = eGkFaultGkRejectedRrqReasonIsDuplicateAlias; break;}

		case cmRASReasonTransportNotSupported:       //13     - one or more of the transports
			{ opcode = eGkFaultGkRejectedRrqReasonIsTransportNotSupported;  break;}

		case cmRASReasonUndefined:                   //18
			{ opcode = eGkFaultGkRejectedRrqReasonIs18; break;}

		case cmRASReasonSecurityDenial:              //24
			{ opcode = eGkFaultGkRejectedRrqReasonIsSecurityDenial; break;}

		case cmRASReasonTransportQOSNotSupported:    //25
			{ opcode = eGkFaultGkRejectedRrqReasonIsTransportQOSNotSupported; break;}

		case cmRASReasonInvalidAlias:                //27     - alias not consistent with gatekeeper rules
			{ opcode = eGkFaultGkRejectedRrqReasonIsInvalidAlias; break;}

		case cmRASReasonFullRegistrationRequired:    //31     - registration permission has expired
			{
				opcode = eGkFaultGkRejectedRrqReasonIsFullRegistrationRequired;
				if (RRJCounter != 0)
					RRJCounter = 0; //in order to send RRQ next time
				break;
			}

		case cmRASReasonAdditiveRegistrationNotSupported: //34
			{ opcode = eGkFaultGkRejectedRrqReasonIsAdditiveRegistrationNotSupported; break;}

		case cmRASReasonInvalidTerminalAliases:     //35
			{ opcode = eGkFaultGkRejectedRrqReasonIsInvalidTerminalAliases; break;}

		case cmRASReasonGenericData:               //39
			{ opcode = eGkFaultGkRejectedRrqReasonIsGenericData; break;}

		case cmRASReasonNeededFeatureNotSupported: //40
			{ opcode = eGkFaultGkRejectedRrqReasonIsNeededFeatureNotSupported; break;}

		default:
			{ opcode = eGkFaultGkRejectedRrqReasonIsStatusIllegal; break;}
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CGKServiceManager::OnPartyRemoveGkCall(CSegment* pParam)
{
	HeaderToGkManagerStruct* pHeaderFromParty = AllocateAndGetHeaderFromParty(pParam);

	DWORD serviceId = pHeaderFromParty->serviceId;
	DWORD connId    = pHeaderFromParty->connId;
	DWORD partyId   = pHeaderFromParty->partyId;

	PTRACE2INT(eLevelInfoNormal, "CGKServiceManager::OnPartyRemoveGkCall. Conn Id = ", connId);

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnPartyRemoveGkCall - pService is null. service Id = ", serviceId);
		PDELETEA(pHeaderFromParty);
		return;
	}

	CGkCall *pCallParams = GetGkCall(connId);
	if(!pCallParams) // in case the allocation failes
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnPartyRemoveGkCall - pCallParams is null. ConnId = ", connId);
		PDELETEA(pHeaderFromParty);
		return;
	}
	BYTE bIsCallARQHeld = FALSE;
	if (pCallParams)
		bIsCallARQHeld = (pCallParams->GetCallState()  == eArqHold);

	//1) Remove from GK Lists:
	//1.1: Remove from the list of calls through gk:
	RemoveCallFromGkCallsTable (pCallParams, serviceId);

	//1.2: Remove from hold calls list:
	RemoveCallFromGkHoldCallsTable(serviceId, connId);

	//2) Send Remove To Card:
	if (!bIsCallARQHeld)
		m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_REMOVE_GK_CALL_REQ, NULL, pService->GetServiceId(), pHeaderFromParty->status, connId, partyId, 0, 0, pService->GetServiceId());
	PDELETEA(pHeaderFromParty);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnGkCallNoResponseFromParty(CSegment* pParam)
{
	DWORD connId;
	*pParam >> connId;

	PTRACE2INT(eLevelInfoNormal, "CGKServiceManager::OnGkCallNoResponseFromParty. Conn Id = ", connId);

	CGkCall *pThisCall = GetGkCall(connId);
	if (!pThisCall)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnGkCallNoResponseFromParty - pThisCall is null. ConnId = ", connId);
		return;
	}

	if (pThisCall->GetCallState() == eArqInd)
	{
		SendDRQReq(pThisCall);
		UpdateCallStateInGkList(pThisCall, eDrqReqFromGkManager);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnPartyKeepAliveInd(CSegment* pParam)
{
	HeaderToGkManagerStruct* pHeaderFromParty = AllocateAndGetHeaderFromParty(pParam);
	DWORD connId = pHeaderFromParty->connId;

	CGkCall *pCallParams = GetGkCall(connId);
	if(!pCallParams) // in case the allocation failes
		PTRACE2INT(eLevelError,"CGKServiceManager::OnPartyAnswerToKeepAlive - pCallParams is null. ConnId = ", connId);
	else
		pCallParams->OnPartyAnswerToKeepAlive();
	PDELETEA(pHeaderFromParty);
}




/////////////////////////////////////
//			Alternate GK		   //
/////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::DecideOnHoldStateAndPutPartyInHold(DWORD serviceId, DWORD connId, DWORD partyId, DWORD opcode)
{
	PTRACE2INT(eLevelInfoNormal, "CGKServiceManager::DecideOnHoldStateAndPutPartyInHold - Conn Id = ", connId);
	CGkCall* pThisCall = GetGkCall(connId);
	if (pThisCall)
	{
		eCallStatus currentState = pThisCall->GetCallState();
		eCallStatus holdState    = eInvalidCallState;

		switch (opcode)
		{
			case H323_CS_RAS_ARQ_REQ:
			{
				if (currentState == eArqReq)
					holdState = eArqHold;
				else if (currentState == eDrqAfterArq)
					holdState = eDrqAfterArqHold;
				else
					PTRACE2INT(eLevelError, "CGKServiceManager::DecideOnHoldStateAndPutPartyInHold - Invalid current call state after sending ARQ. State = ", currentState);
				break;
			}

			case H323_CS_RAS_DRQ_REQ:
			{
				if ((currentState == eDrqReq) || (currentState == eSendDrqAfterArq) )
					holdState = eDrqHold;
				else
					PTRACE2INT(eLevelError, "CGKServiceManager::DecideOnHoldStateAndPutPartyInHold - Invalid current call state after sending DRQ. State = ", currentState);
				break;
			}

			case H323_CS_RAS_BRQ_REQ:
			{
				if (currentState == eArqInd)
					holdState = eBrqHold;
				else if (currentState == eDrqReq)
					holdState = eDrqHold;
				else
					PTRACE2INT(eLevelError, "CGKServiceManager::DecideOnHoldStateAndPutPartyInHold - Invalid current call state after sending BRQ. State = ", currentState);
			}
		}

		if (holdState != eInvalidCallState)
			PutPartyInHold(serviceId, connId, partyId, holdState);
	}
	else
		PTRACE2INT(eLevelError, "CGKServiceManager::DecideOnHoldStateAndPutPartyInHold - Call wasn't found in table. Conn Id = ", connId);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::PutPartyInHold(DWORD serviceId, DWORD connId, DWORD partyId, eCallStatus holdState)
{
	PTRACE2INT(eLevelInfoNormal, "CGKServiceManager::PutPartyInHold - Conn Id = ", connId);
	CGkCall* pThisCall = GetGkCall(connId);
	if (pThisCall)
	{
		//1) Update in calls list:
		UpdateCallStateInGkList(pThisCall, holdState);

		//2) Insert to hold list:
		CGkCall *pHoldCall = new CGkCall(GetRcvMbx(), connId);
		pHoldCall->SetSeviceId(serviceId);
		pHoldCall->SetConfRsrcId(pThisCall->GetConfRsrcId());
		m_pHoldCallsList->insert(pHoldCall);

		//3) Report to Party:
		if (holdState != eBrqHold)
		{
			DWORD holdOpcode = (holdState == eArqHold) ? H323_CS_RAS_ARQ_REQ : H323_CS_RAS_DRQ_REQ;

			CSegment* pSeg = new CSegment;
			*pSeg << holdOpcode;
			SendReqToConfParty(HOLD_GK_REQ, connId, partyId, pSeg);
		}
	}
	else
		PTRACE2INT(eLevelError, "CGKServiceManager::PutPartyInHold - Call wasn't found in table. Conn Id = ", connId);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CGKServiceManager::IsCurrIndMessageSameAsAltGkTrigger(CGkService* pService, DWORD rasOpcode, DWORD connId)
{
	BYTE bIsSameTrigger = TRUE;
	bIsSameTrigger &= (rasOpcode == pService->GetTriggerOpcode());
	if (bIsSameTrigger)
		bIsSameTrigger &= (connId == pService->GetTriggerConnId());
	return bIsSameTrigger;
}

/////////////////////////////////////////////////////////////////////////////
eAltGkState CGKServiceManager::IsNeedToHandleAltGk(CGkService* pService, DWORD rasOpcode, DWORD connId, int numOfAltGks, BYTE bGkUnresponsive, BYTE bGotGRJ)
{
	eAltGkState bHandle = eNotAltGk;
	eAltGkProcess eAltProcess = pService->GetAltGkProcess();

	if( ((eAltProcess == eSearchFromAltList) || (eAltProcess == eFoundFromAltList) )&&
		((rasOpcode == H323_CS_RAS_GRQ_REQ) || (rasOpcode == H323_CS_RAS_RRQ_REQ) ) )
		return eAnotherTrigger;//that means that the rasOpcode was send before we start the alt Gk process

	else if ( (eAltProcess == eSearchFromAltList) || (eAltProcess == eSearchFromAltListWithRegistration) )
	{	//in case we are in the first cycle: need to check only for the calls opcodes
		BYTE bIsSameTrigger = IsCurrIndMessageSameAsAltGkTrigger(pService, rasOpcode, connId);
		if (!bIsSameTrigger)
		{//another trigger
			if ( (bGkUnresponsive || bGotGRJ) && (rasOpcode == H323_CS_RAS_GRQ_REQ) )
			{//In case the trigger wasn't a registration message, but when we tried to register to the alt, we sent first GRQ, and now got GRQ timeout or GRJ,
			 // we should treat it like a failure to register to the alt
				PTRACE (eLevelInfoNormal, "CGKServiceManager::IsNeedToHandleAltGk - eTimeoutOrRegularReject. m_triggerOpcode isn't a registration opcode");
				return eTimeoutOrRegularReject;
			}
			else
			{
				PTRACE2INT(eLevelInfoNormal, "CGKServiceManager::IsNeedToHandleAltGk - Another trigger. Conn is = ", connId);
				return eAnotherTrigger;//altInProcess because of another trigger.
			}
		}
	}

	if (numOfAltGks)
	{
		if (eAltProcess && ! pService->IsAltGkPermanent())
		{//if we are in the process of finding a temporary alt gk, we won't replace the alt list
			PTRACE (eLevelInfoNormal, "CGKServiceManager::IsNeedToHandleAltGk - continue with finding temporary alternate Gk");
			bHandle = eTimeoutOrRegularReject;
		}

		else if ( !eAltProcess || (eAltProcess && pService->IsAltGkPermanent()))
		{  //a new process or a reject from a permanent
			PTRACE (eLevelInfoNormal, "CGKServiceManager::IsNeedToHandleAltGk - Alternate Gk is required");
			bHandle = eStart; //we should save a list, only in case it comes from a permanent alt GK
		}
	}

	else if (eAltProcess)
	{//if we are in the process of finding a permanent alt gk, we should continue to the next alt gk in the list
		PTRACE (eLevelInfoNormal, "CGKServiceManager::IsNeedToHandleAltGk - need to continue with finding alternate Gk from last index");
		bHandle = eTimeoutOrRegularReject;
	}

	else if (bGkUnresponsive)
	{//we should check if the GK has sent us altList before
		if (pService->GetSizeAltGkList())
			bHandle = eStart;
	}

	return bHandle;
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::DecideOnPhaseInAltGk(CGkService* pService, eAltGkState altGkState, DWORD opcode, DWORD connId,
										rejectInfoSt* pRejectInfo, altGksListSt* pAltGkList)
{
	if (altGkState == eStart)
		StartAltGkProcess(pService, opcode, connId, pRejectInfo);
	else
		HandleAltGk(pService, altGkState);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::StartAltGkProcess(CGkService* pService, DWORD opcode, DWORD connId,
								   rejectInfoSt* pRejectInfo, altGksListSt* pAltGkList)
{
	/*if (! ::GetpSystemCfg()->IsEnableAltGk())
		return;*/

	PTRACE2INT(eLevelInfoNormal, "CGKServiceManager::StartAltGkProcess - service number = ", pService->GetServiceId());

	if (pRejectInfo) //(1) result of reject with alt list
	{
		if (pRejectInfo->bAltGkPermanent == FALSE)
		{
			PTRACE(eLevelInfoNormal, "CGKServiceManager::StartAltGkProcess - We don't support a temporary alt GK");
			return;
		}
		pService->InitAltGkListFromReject(pRejectInfo);
	}
	else if (pAltGkList) //(2) result of GK URQ with alt list
		pService->InitAltGkList(pAltGkList);

	if (pRejectInfo == NULL)
		//(3) result of unresponsive GK or (2)result of GK URQ with alt list
		pService->SetAltGkPermanent(TRUE); //according to the standard

	// Need to save the triggers parameters for cases, in which we should send the specific message to the alt GK.
	pService->SetTriggerOpcode(opcode);
	pService->SetTriggerConnId(connId);

	if (pService->GetAltGkProcess() == eNotInProcess)
		pService->SetAltGkProcess(eSearchFromAltList);
	else if (pService->GetAltGkProcess() == eStartFromOriginal)
		pService->SetAltGkProcess(eSearchFromAltListAfterOneCycle);
	//else: m_eAltGkProcess stay the same

	pService->SetCurrAltGkToSearch(0);

	HandleAltGk(pService, eStart);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::HandleAltGk(CGkService* pService, eAltGkState altGkState)
{
	if ( (altGkState == eRCF) &&  ((pService->GetTriggerOpcode() == H323_CS_RAS_RRQ_REQ) || (pService->GetTriggerOpcode() == 0) )  )
		altGkState = eReqAck;

	else
	{
		if (pService->GetCurrAltGkToSearch() > pService->GetSizeAltGkList())
		{
			PTRACE2INT(eLevelError, "CGKServiceManager::HandleAltGk - m_currAltGkToSearch is too big. Service number =", pService->GetServiceId());
			return;
		}

		if ( (pService->GetSizeAltGkList() == 0) && ( (altGkState == eStart) || (altGkState == eTimeoutOrRegularReject)) )
		{
			pService->SetAltGkProcess(eNotInProcess);
			PTRACE2INT(eLevelError, "CGKServiceManager::HandleAltGk - length of list is 0. Service number = ", pService->GetServiceId());
			pService->SetAltGkProcess(eNotInProcess);
			return;
		}
	}

	switch (altGkState)
	{
		case eStart:
		case eTimeoutOrRegularReject:
		{
			if (altGkState != eStart)
			{//search in the next entry
				if (pService->GetAltGkProcess() != eStartFromOriginal)
				{//if we tried to connect to the original, we already did (pService->m_currAltGkToSearch = 0)
					int	currAltGkToSearch = pService->GetCurrAltGkToSearch();
					pService->SetCurrAltGkToSearch(currAltGkToSearch+1);

					if (pService->GetCurrAltGkToSearch() >= pService->GetSizeAltGkList())
					{//we reached to the end of the list
						pService->SetCurrAltGkToSearch(0);
						if (pService->IsAltGkPermanent() || (pService->GetRegStatus() != eRegister) )
						{//start from the begining
							PTRACE2INT(eLevelError, "CGKServiceManager::HandleAltGk - Alternate Gk wasn't found. Start again from origin Gk. Service number = ", pService->GetServiceId());
							StartFromOriginalGk(pService);
							return;
						}
						else //regard it as reject
						{
							PTRACE2INT(eLevelError, "CGKServiceManager::HandleAltGk - Temporary alternate Gk wasn't found. Service number = ", pService->GetServiceId());
							pService->SetAltGkProcess(eNotInProcess);
						}
						break;
					}
				}
				else
				{
					PTRACE2INT(eLevelError, "CGKServiceManager::HandleAltGk - m_eAltGkProcess was changed from StratFromOriginal to SearchFromAltListAfterOneCycle. service number = ", pService->GetServiceId());
					pService->SetAltGkProcess(eSearchFromAltListAfterOneCycle);
					pService->SetCurrAltGkToSearch(0); //just to make sure, although it should be 0.
				}
			}

			alternateGkSt* pAltStruct = pService->GetAltGkMember();
			pService->SetCurAltGkSt(pAltStruct);

			//end conidition!
			if ( *(pService->GetCurAltGkIp()) == *(pService->GetGkIp()) && (pService->GetCurrAltGkToSearch() >= pService->GetSizeAltGkList()))
			{
				PTRACE2INT(eLevelError, "CGKServiceManager::HandleAltGk - Current alt Gk is the original one. Therefore, we end the loop. Service number = ", pService->GetServiceId());
				pService->SetAltGkProcess(eSearchFromAltListAfterOneCycle);
				AltGkNotFound(pService);
			}

			if (pService->GetAltGkProcess() == eSearchFromAltListAfterOneCycle)
			{//send GRQ after the polling timer to the current alt Gk:

				InformCsMngrOnTimeout(pService, cmRASGatekeeper, TRUE);//will start the polling timer
				return;
			}
            DWORD triggerOpcode = pService->GetTriggerOpcode();// HOMOLOGATION. Sasha. Add for RAS_TE_ADM_09; _10
            PTRACE2INT(eLevelError, "CGKServiceManager::HandleAltGk - trigger is ", triggerOpcode);// HOMOLOGATION. Sasha. Add for RAS_TE_ADM_09; _10

            if (  (pService->IsNeedToRegisterToAltGk())
                &&(triggerOpcode != H323_CS_RAS_ARQ_REQ) // HOMOLOGATION. Sasha. Add for RAS_TE_ADM_09; _10 
               )
			{
				if (pService->GetAltGkProcess() == eSearchFromAltList)
					pService->SetAltGkProcess(eSearchFromAltListWithRegistration);

				if (pService->IsDiscoveryRequiredToAltGk())
				{
					BOOL bIsRrqWithoutGrq = FALSE;
					CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
					std::string key = "RRQ_WITHOUT_GRQ";
					if( pServiceSysCfg != NULL )
						pServiceSysCfg->GetBOOLDataByKey(pService->GetServiceId(), key, bIsRrqWithoutGrq);
					if(bIsRrqWithoutGrq)
						CreateAndSendRRQ(pService, FALSE);
					else
						CreateAndSendGRQ(pService);
				}
				else
					CreateAndSendRRQ(pService);
			}
			else
			{
				pService->SetAltGkProcess(eFoundFromAltList);

				BYTE bReqSent = SendReqToAltGk(pService);
				if (bReqSent == FALSE)
					StartFromOriginalGk(pService);
			}
			break;
		}

		case eRCF:
		{//we succeeded to registered to the alt Gk, so now we can send the message
			DWORD triggerOpcode = pService->GetTriggerOpcode();
			if (triggerOpcode)
				if ((triggerOpcode !=  H323_CS_RAS_GRQ_REQ) || (triggerOpcode !=  H323_CS_RAS_RRQ_REQ) )
				//means that the trigger to handle alt GK wasn't a registreation request
				{
					pService->SetAltGkProcess(eFoundFromAltList);
					BYTE bReqSent = SendReqToAltGk(pService);
					AltGkFound(pService);
				}
				break;
		}

		case eReqAck:
			AltGkFound(pService);
			break;

		default:
			PTRACE2INT(eLevelError, "CGKServiceManager::HandleAltGk - Invalid altGkState = ", altGkState);
			break;
	}

	if (pService->GetAltGkProcess() == eNotInProcess) //also in case of eReqAck
		pService->SetAltGkParamsToZero();
}

/////////////////////////////////////////////////////////////////////////////
BYTE CGKServiceManager::SendReqToAltGk(CGkService* pService)
{
	PTRACE(eLevelInfoNormal, "CGKServiceManager:AltGkFound");

	BYTE bSuccess = FALSE;
	CGkCall* pThisCall = GetGkCall(pService->GetTriggerConnId());
	if (pThisCall)
		bSuccess = SendReqOnHoldCall(pThisCall);
	else
		PTRACE2INT(eLevelError,"CGKServiceManager::SendReqToAltGk - Call wasn't found. Conn Id = ", pService->GetTriggerConnId());

	return bSuccess;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CGKServiceManager::SendReqOnHoldCall(CGkCall* pGkCall)
{
	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::SendReqOnHoldCall - Conn Id = ", pGkCall->GetConnId());
	BYTE bSuccess = TRUE;
	eCallStatus holdState = pGkCall->GetCallState();
	switch (holdState)
	{
		case eDrqAfterArqHold:
		case eDrqHold:
		{
			if (holdState == eDrqAfterArqHold)
				UpdateCallStateInGkList(pGkCall, eSendDrqAfterArq);
			else
				UpdateCallStateInGkList(pGkCall, eDrqReq);
			SendDRQReq(pGkCall);
			break;
		}

		case eArqHold:
		case eBrqHold:
		{//ask party to do it
			DWORD resentOpcode = (holdState == eArqHold) ? H323_CS_RAS_ARQ_REQ : H323_CS_RAS_BRQ_REQ;
			CSegment*  pSeg = new CSegment;
			*pSeg << (DWORD)resentOpcode;
			SendReqToConfParty(RESEND_GK_REQ, pGkCall->GetConnId(), pGkCall->GetPartyId(), pSeg);

			//if (holdState == eArqHold)
			//{ We don't delete the call from the lists, although the control of the new
			// call is now be moved to the party, because when the party asks again to
			//send ARQ, we won't create another GkConnId.
			//	}

			if (holdState == eBrqHold)
				UpdateCallStateInGkList(pGkCall, eArqInd);
			break;
		}

		default:
		{
			bSuccess = FALSE;
			CMedString trace_str;
			trace_str << " Mcms Connection Id = " << pGkCall->GetConnId();
			trace_str << " Call state = "         << holdState;
			PTRACE2(eLevelError,"CGKServiceManager::SendReqOnHoldCall - Invalid Call state.", trace_str.GetString());
		}
	}
	return bSuccess;
}

/////////////////////////////////////////////////////////////////////////////
void  CGKServiceManager::AltGkFound(CGkService* pService)
{
	PTRACE(eLevelInfoNormal,"CGKServiceManager::AltGkFound");
	if (pService->IsAltGkPermanent())
	{
		pService->SetAltGkAsPermanent();
		SendStartRegistartionTimer(REGISTRATION_POLLING_TIMER, pService->GetTimeToLive(), pService->GetServiceId(), CARD_NORMAL);
	}

	DWORD triggerOpcode = pService->GetTriggerOpcode();
	if (pService->GetHoldRRQAfterAltEnd())
	{
		if ( (triggerOpcode != H323_CS_RAS_RRQ_REQ) && (triggerOpcode != H323_CS_RAS_GRQ_REQ) )
			ReRegisterToGk(pService, CARD_NORMAL);//yael: check card status!!
		pService->SetHoldRRQAfterAltEnd(FALSE);
	}

	DWORD triggerConnId = pService->GetTriggerConnId();
	DWORD serviceId = pService->GetServiceId();

	//search for hold calls:
	//if there are hold calls => resume them
	ResumeHoldCalls(serviceId,triggerConnId);

	// If we sent a request before the alt gk process has started, and got timeout on
	//this request after the alt gk process finished, we actually won't move this request
	//to the alt gk.
	//Like in the following scenario: RRQ, ARQ, RRQ Timeout, GRQ, GCF, RRQ, RCF, ARQ Timeout.
	//Therefore, after we move to the alt gk, we mark the calls, on which a requset was
	//sent, in hold states.
	UpdateCallStateInGkListAfterAltFound(serviceId,triggerConnId);
}

/////////////////////////////////////////////////////////////////////////////
void  CGKServiceManager::AltGkNotFound(CGkService* pService)
{
	PTRACE(eLevelInfoNormal, "CGKServiceManager::AltGkNotFound");
	//1) search for hold calls:
	if (m_pHoldCallsList->size())
	{
		DWORD serviceId = pService->GetServiceId();
		DWORD connId, transaction;
		CGkCall* pCurCall;

		CGkCallSet::iterator iter = m_pHoldCallsList->begin();
		CGkCallSet::iterator currentIter;
		CGkCall* pHoldCall;

		while (iter !=  m_pHoldCallsList->end())
		{
			currentIter = iter;
			++iter; // increment iter at the beginning, because maybe currentIter should be removed
			pHoldCall = *currentIter;
			if (pHoldCall->GetServiceId() == serviceId)
			{
				connId = pHoldCall->GetConnId();
				pCurCall = GetGkCall(connId);
				if (pCurCall)
				{
					eCallStatus holdState = pCurCall->GetCallState();
					switch (holdState)
					{
						case eDrqAfterArqHold:
						case eDrqHold:// receiving inf. from party which response to Gatekeeper DRQ
						case eArqHold:
						{
							STATUS bRemoved = RemoveCallFromGkCallsTable(connId, serviceId);
							if (bRemoved == STATUS_OK)
							{
								if (holdState == eArqHold)
									transaction = (DWORD)eAltGkProcessStatus;
								 else
								 	transaction = cmRASDisengage;
								PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::AltGkNotFound - Inform parfy on timeout. Mcms Conn Id = ", connId);
								DWORD partyId = pCurCall->GetPartyId();
								InformParfyOnTimeout(pService, transaction, connId, partyId);
								if (holdState == eDrqHold)
									m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_REMOVE_GK_CALL_REQ, NULL, pService->GetServiceId(),
									                                STATUS_OK, connId, partyId, 0, 0, pService->GetServiceId());
							}
							else
								PTRACE2INT(eLevelError,"CGKServiceManager::AltGkNotFound - Error in remove call. Mcms Conn Id = ", connId);

							break;
						}
						case eBrqHold:
							UpdateCallStateInGkList(pCurCall, eArqInd);
							// receiving inf. from party which response to Gatekeeper DRQ
							//do nothing
							break;
						default:
						{
							CMedString trace_str;
							trace_str << "  Connection Id = " << pCurCall->GetConnId();
							trace_str << ", Call state = "    << holdState;
							PTRACE2(eLevelError,"CGKServiceManager::AltGkNotFound - Invalid Call state.", trace_str.GetString());
							break;
						}
					}
					POBJDELETE(pHoldCall);
					m_pHoldCallsList->erase(currentIter); //Remove from the hold call list
				}
			}
		}
	}

	// 2) Init params
	pService->SetTriggerOpcode(0);
	pService->SetTriggerConnId(0);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::StartFromOriginalGk(CGkService* pService)
{
	PTRACE(eLevelInfoNormal, "CGKServiceManager::StartFromOriginalGk");
	pService->SetAltGkProcess(eStartFromOriginal);
	pService->RestoreOriginalGkParams();
	AltGkNotFound(pService);
	//send GRQ after the polling timer to the original Gk:
	InformCsMngrOnTimeout(pService, cmRASGatekeeper, TRUE);//will start the polling timer
}

/////////////////////////////////////////////////////////////////////////////
const char* CGKServiceManager::GetAltGkStateStr(eAltGkState state)
{
	return (state < eLastAltGkState) ? altGkStateStrings[state] : "Unknown State";
}

/////////////////////////////////////////////////////////////////////////////
/*STATUS CGKServiceManager::HandleSetGkIp(CTerminalCommand & command,std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(2 != numOfParams)
	{
		answer << "Incorrect usage\n";
		answer << "Usage : set_gk_ip {index to set 0-4} {ip address}";
		return STATUS_OK;
	}

	int indexToSet = atoi(command.GetToken(eCmdParam1).c_str());
	const string & strIpAddr = command.GetToken(eCmdParam2);

	DWORD ip = SystemIpStringToDWORD(strIpAddr.c_str());

	const DWORD serviceId = 3;

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService != NULL)
	{
		ipAddressStruct ipAddr;
		ipAddr.addr.v4.ip = ip;
		ipAddr.ipVersion  = eIpVersion4;
		pService->UpdateCardPropGkIp(&ipAddr, indexToSet);
		if ((AVF_DEBUG_MODE == TRUE) && (m_bIsAvfLicense == FALSE || pService->GetIsAvaya() == FALSE))
		{
	        PTRACE (eLevelInfoNormal, "CGKServiceManager::CheckAvayaStatus DEBUG_AVF ON");
			pService->SetIsAvaya(TRUE);
			m_bIsAvfLicense = TRUE;
		}
		answer 	<< "update report : \n"
				<< "service id : " 		<< serviceId 	<< "\n"
				<< "index to set : " 	<< indexToSet 	<< "\n"
				<< "ip address : " 		<< strIpAddr.c_str() << " = " << ip	<< "\n";
	}
	else
	{
		answer << "no ip service " << serviceId << "\n";
	}

    return STATUS_OK;
}*/

/////////////////////////////////////////////////////////////////////////////
// This function check is the Apollo environment authenticated according
// to the following rules:
//   --------------------------------------------------------
//   |  System Mode    | GK Mode         |     Action       |
//   |(licensing mode) |(H460 identified)| (message to EMA) |
//   --------------------------------------------------------
// 1 |     FALSE       |     FALSE       | Nothing - not AVF|
// 2 |     FALSE       |     TRUE        | Major-message 1  |
// 3 |     TRUE        |     FALSE       | Nothing - not AVF|
// 4 |     TRUE        |     TRUE        | Nothing - Avaya  |
//   --------------------------------------------------------
// Message 1 - Avaya CM was identified but the MCU is not configured to Avaya
// 			   The MCU was not configured for supporting AVF
// The service is also contains boolean variable isAvaya but its value is based
// on the current authentication procedure
eAVFStatus CGKServiceManager::CheckAvayaStatus(CGkService* pService, h460FsSt& rH460FsSt)
{
	if (!pService)
		return eAVFNotAvaya;

	pService->SetIsAvaya(FALSE);
	if (AVF_DEBUG_MODE == TRUE)
	{
        PTRACE (eLevelInfoNormal, "CGKServiceManager::CheckAvayaStatus DEBUG_AVF ON");
		pService->SetIsAvaya(TRUE);
		m_bIsAvfLicense = TRUE;
	}
	//remove all possibly active alarms which were activated by GKManager
	RemoveActiveAlarmByErrorCode(GKMNGR_AVF_WRONG_MCU_CONFIG);

	DWORD serviceId = pService->GetServiceId();
	//table's positions 1 and 2
	if (m_bIsAvfLicense == FALSE)
	{
		//position 1
		if (rH460FsSt.supportFs.fsId != H460_K_FsId_Avaya)
			return eAVFNotAvaya;
		//position 2
		AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT, GKMNGR_AVF_WRONG_MCU_CONFIG, MAJOR_ERROR_LEVEL, 
								"The MCU was not configured for supporting AVF",
								true, true);
		PTRACE2INT(eLevelError, "CGKServiceManager::CheckAvayaStatus - The MCU was not configured for supporting AVF. Service Id =", serviceId);
		return eAVFSystemFail;
	}
	//position 3
	if (rH460FsSt.supportFs.fsId != H460_K_FsId_Avaya)
		return eAVFNotAvaya;
	//position 4
	pService->SetIsAvaya(TRUE);
	return eAVFAvayaOk;
}


/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnRAITimer(CSegment* pParam)
{
    PTRACE(eLevelInfoNormal, "CGKServiceManager::OnRAITimer");
    CConfPartyManagerApi ConfPartyApi;
    ConfPartyApi.GkResourceQuery(m_ServiceId);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnResourceReportGenerated (CSegment* pParam)
{
    PTRACE(eLevelInfoNormal, "CGKServiceManager::OnResourceReportGenerated");

    CIPAdHocProfilesReport * pTmpAdHocReport = new CIPAdHocProfilesReport;
    pTmpAdHocReport->DeSerialize (NATIVE, *pParam);
    m_TimeElapsedSinceLastRai += DefaultOfRAIInterval;
    if (m_pLastAdHocReport &&
        (*pTmpAdHocReport == *m_pLastAdHocReport) &&
        (m_TimeElapsedSinceLastRai < DefaultRAIMaximumInterval))
    {
        //No Change
        PTRACE(eLevelInfoNormal, "CGKServiceManager::OnResourceReportGenerated Same Report as last time - do nothing");
        POBJDELETE(pTmpAdHocReport);
    }
    else
    {
        //Allocating the General RAI buffer, this will be used for all the services
        int raiMsgSize = 0;
        gkReqRasRAI * pGeneralRAIBuffer = AllocateAndCreateAdHocRAIReport(pTmpAdHocReport, raiMsgSize);

        //Analyze and send to the CS
        for (int i = 0; i < MAX_SERVICES_NUM; i++)
        {
            CGkService* pService = m_ServicesTable[i];
            if (pService && pService->GetIsAvaya() && pService->IsOKToSendRAI() && pService->GetRegStatus() == eRegister)
            {
                // We send the RAI per Service
                pService->SetGkRasAddress(&pGeneralRAIBuffer->gatekeeperAddress);
                //Send to CS
                CSegment* pMsg = new CSegment;
                pMsg->Put((BYTE*)(pGeneralRAIBuffer), raiMsgSize);
                m_pGkToCsInterface->SendMsgToCS(H323_CS_RAS_RAI_REQ, pMsg, pService->GetServiceId(), STATUS_OK, 0, 0, 0, 0, pService->GetServiceId());
                m_TimeElapsedSinceLastRai = 0;
                pService->SentRAI();
                POBJDELETE(pMsg);
            }
        }
        PDELETEA(pGeneralRAIBuffer);
        POBJDELETE(m_pLastAdHocReport);
        m_pLastAdHocReport = pTmpAdHocReport;
    }
    //Restart the Timer
    StartTimer(RAI_TIMER, DefaultOfRAIInterval * SECOND, NULL);
}

/////////////////////////////////////////////////////////////////////////////
void  CGKServiceManager::StopRAITimerIfNeeded()
{
    BYTE bFoundServiceToSendRAI = FALSE;
    for (int i = 0; i < MAX_SERVICES_NUM && !bFoundServiceToSendRAI; i++)
    {
        CGkService* pService = m_ServicesTable[i];
        if (pService && pService->GetIsAvaya() && pService->GetRegStatus() == eRegister)
            bFoundServiceToSendRAI = TRUE;
    }
    if (!bFoundServiceToSendRAI && IsValidTimer(RAI_TIMER))
        DeleteTimer(RAI_TIMER);
}

/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnRACInd(CSegment * pSeg)
{
  	APIU32 connId;
	APIU32 partyId;
	APIU32 serviceId;
	APIS32 status;
	GetCsHeaderParams(pSeg, &connId, &partyId, &serviceId, &status);

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnRACInd - Service Id = ", serviceId);

	CGkService* pService = m_ServicesTable[serviceId];
	if (pService == NULL)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnRACInd  - pService is null. Service Id = ", serviceId);
		DBGPASSERT_AND_RETURN(serviceId);
	}
    else
        pService->ReceivedRAC();
}

/////////////////////////////////////////////////////////////////////////////
gkReqRasRAI * CGKServiceManager::AllocateAndCreateAdHocRAIReport(CIPAdHocProfilesReport * pAdHocReport,int &raiMsgSize)
{
    //This function fill the RAI structure not for a specific service, so the transport address
    // of the gatekeeper should be filled seperately.
    raiMsgSize = (sizeof(gkReqRasRAI) - sizeof(char) +
                 (sizeof (h460ConferenceProfileExt) * pAdHocReport->GetNumOfProfiles()));

    //Allocation of the buffer, should be deleted after using it!
    gkReqRasRAI * pRaiMsgBuffer = (gkReqRasRAI*) new BYTE[raiMsgSize];
    if (!pRaiMsgBuffer)
    {
        PTRACE2INT (eLevelError, "CGKServiceManager::AllocateAndCreateAdHocRAIReport Could Not Allocate Buffer of size = ", raiMsgSize);
        raiMsgSize = 0;
        return NULL;
    }
    // Fill the buffer
    pRaiMsgBuffer->bAlmostOutOfResources = pAdHocReport->IsAlmostOutOfResources();

    APIU32 maximumAudioCapacity = 0;
  	APIU32 maximumVideoCapacity = 0;
  	APIU32 currentAudioCapacity = 0;
  	APIU32 currentVideoCapacity = 0;
    pAdHocReport->GetMaximumCapacity(maximumAudioCapacity, maximumVideoCapacity);
    pAdHocReport->GetCurrentCapacity(currentAudioCapacity, currentVideoCapacity);
    pRaiMsgBuffer->maximumAudioCapacity = maximumAudioCapacity;
    pRaiMsgBuffer->maximumVideoCapacity = maximumVideoCapacity;
    pRaiMsgBuffer->currentAudioCapacity = currentAudioCapacity;
    pRaiMsgBuffer->currentVideoCapacity = currentVideoCapacity;

    pRaiMsgBuffer->numOfSupportedProfiles = pAdHocReport->GetNumOfProfiles();
    pRaiMsgBuffer->xmlDynamicProps.numberOfDynamicParts = pAdHocReport->GetNumOfProfiles();
    pRaiMsgBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(h460ConferenceProfileExt) * pAdHocReport->GetNumOfProfiles() ;
    profilesVector * pProfilesVector = pAdHocReport->GetAdhocProfilesPtr();

    h460ConferenceProfileExt *pProf = (h460ConferenceProfileExt*)(&pRaiMsgBuffer->profilesArray[0]);
    for (profilesVector::iterator itr= pProfilesVector->begin() ; itr != pProfilesVector->end() ; ++itr)
    {
        AdHocProfileResourceSheet * pCurrProfile = (*itr);
        if (pCurrProfile != NULL)
        {
            pProf->xmlHeader.dynamicType =  tblH460Conf;
            pProf->xmlHeader.dynamicLength           = sizeof(h460ConferenceProfileExt);
            strncpy(pProf->h460ConfProfile.profileE164ID, pCurrProfile->profileID , MaxAliasLength);
            pProf->h460ConfProfile.minimumPorts = pCurrProfile->minimumPorts;
            pProf->h460ConfProfile.partyCallRate = pCurrProfile->callRate;
            pProf->h460ConfProfile.numOfPortsAvailable = pCurrProfile->portsAvailable;
            pProf->h460ConfProfile.maxNumOfPortsCapacity = pCurrProfile->maxPortsAvailable;
            pProf->h460ConfProfile.numOfConferencesAvailable = pCurrProfile->conferencesAvailable;
            pProf->h460ConfProfile.maxNumofConferencesCapacity = pCurrProfile->maxConferenceAvailable;
            pProf->h460ConfProfile.videoBitRateType = pCurrProfile->videoRateType;
            pProf++;
        }
    }
    return pRaiMsgBuffer;
}
/////////////////////////////////////////////////////////////////////////////
void CGKServiceManager::SetCsIpArrayAccordingToIpTypeAndScopeId(GkManagerServiceParamsIndStruct* pServiceParamsIndSt, CIpAddressPtr GkIp)
{
	TRACEINTO << __FUNCTION__ << "\n";

	enIpVersion eGkIpVer = GkIp.get()->GetVersion();
	ipAddressStruct	csIp[TOTAL_NUM_OF_IP_ADDRESSES];
	memset(&csIp,0,sizeof(ipAddressStruct)*TOTAL_NUM_OF_IP_ADDRESSES);
	int i = 0;
	int j = 0;
	BYTE isMatch = 0;
	enScopeId eGkScopeId = eScopeIdOther;
	if (eGkIpVer == eIpVersion6)
		eGkScopeId = (enScopeId)((CIpV6Address*)GkIp.get())->GetScopId();

	if (eGkIpVer == eIpVersion4 && eGkIpVer == (enIpVersion)pServiceParamsIndSt->csIp[0].ipVersion)
	{
		// A private case for IpV4 as first address
		return;
	}
	char a[128];

	TRACEINTO << __FUNCTION__ << " First loop \n";
	for (i = 0;i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
	{
		memset(a, '\0', 128);
		TRACEINTO << "Temp Arr : [" << i << "] - " << ipToString(csIp[i], a, 1);
		memset(a, '\0', 128);
		TRACEINTO << "pServiceParamsIndSt Arr : [" << i << "] - " << ipToString(pServiceParamsIndSt->csIp[i], a, 1);
	}

	// 1. Finding the exact address type and scope (In IpV6 case).
	for (i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
	{
		if (::IsIpNull(&(pServiceParamsIndSt->csIp[i])) == FALSE)
		{
			if (eGkIpVer == (enIpVersion)pServiceParamsIndSt->csIp[i].ipVersion && eGkIpVer == eIpVersion4)
			{
				memcpy(&(csIp[0]), &(pServiceParamsIndSt->csIp[i]), sizeof(ipAddressStruct));
				isMatch = 1;
				memset(&(pServiceParamsIndSt->csIp[i]), 0 , sizeof(ipAddressStruct));
			}
			else if (eGkIpVer == (enIpVersion)pServiceParamsIndSt->csIp[i].ipVersion && eGkIpVer == eIpVersion6)
			{
				if (pServiceParamsIndSt->csIp[i].addr.v6.scopeId == (DWORD)eGkScopeId)
				{
					memcpy(&(csIp[0]), &(pServiceParamsIndSt->csIp[i]), sizeof(ipAddressStruct));
					isMatch = 1;
					memset(&(pServiceParamsIndSt->csIp[i]), 0, sizeof(ipAddressStruct));
				}
			}
		}
		if (isMatch)
			break;
	}
	TRACEINTO << __FUNCTION__ << " Second loop \n";
	for (i = 0;i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
	{
		memset(a, '\0', 128);
		TRACEINTO << "Temp Arr : [" << i << "] - " << ipToString(csIp[i], a, 1);
		memset(a, '\0', 128);
		TRACEINTO << "pServiceParamsIndSt Arr : [" << i << "] - " << ipToString(pServiceParamsIndSt->csIp[i], a, 1);
	}

	// 2. Copying the rest of the addresses with the correct priority.
	if (isMatch)
	{
		// We found and located the first prioritized addr.
		// In this case we will copy the array according to Ip type.
		for (i = 1; i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
		{
			for (j = 0; j < TOTAL_NUM_OF_IP_ADDRESSES; j++)
			{
				if (::IsIpNull(&(pServiceParamsIndSt->csIp[j])) == FALSE)
				{
					if (eGkIpVer == eIpVersion4)
					{
						memcpy(&(csIp[i]), &(pServiceParamsIndSt->csIp[j]), sizeof(ipAddressStruct));
						memset(&(pServiceParamsIndSt->csIp[j]), 0, sizeof(ipAddressStruct));
						break;
					}
					else if (eGkIpVer == eIpVersion6)
					{
						if ((enIpVersion)pServiceParamsIndSt->csIp[j].ipVersion == eIpVersion6)
						{
							memcpy(&(csIp[i]), &(pServiceParamsIndSt->csIp[j]), sizeof(ipAddressStruct));
							memset(&(pServiceParamsIndSt->csIp[j]), 0, sizeof(ipAddressStruct));
							break;

						}
					}
				}
			}
		}
		TRACEINTO << __FUNCTION__ << " Third loop - MATCH  \n";
		for (i = 0;i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
		{
			memset(a, '\0', 128);
			TRACEINTO << "Temp Arr : [" << i << "] - " << ipToString(csIp[i], a, 1);
			memset(a, '\0', 128);
			TRACEINTO << "pServiceParamsIndSt Arr : [" << i << "] - " << ipToString(pServiceParamsIndSt->csIp[i], a, 1);
		}


	}
	else
	{
		// This can only happen in IpV6 case - We have CS addresses but not in the matching ScopeId.
		// In this case - We simply copy all IpV6 addresses and then the V4 - That's the best match we can provide.
		for (i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
		{
			for (j = 0; j < TOTAL_NUM_OF_IP_ADDRESSES; j++)
			{
				if (::IsIpNull(&(pServiceParamsIndSt->csIp[j])) == FALSE)
				{
					if (eGkIpVer == eIpVersion6)
					{
						if ((enIpVersion)pServiceParamsIndSt->csIp[j].ipVersion == eIpVersion6)
						{
							memcpy(&(csIp[i]), &(pServiceParamsIndSt->csIp[j]), sizeof(ipAddressStruct));
							memset(&(pServiceParamsIndSt->csIp[j]), 0, sizeof(ipAddressStruct));
							break;

						}
					}
				}
			}
		}
		TRACEINTO << __FUNCTION__ << " Third loop -  NO MATCH  \n";
		for (i = 0;i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
		{
			memset(a, '\0', 128);
			TRACEINTO << "Temp Arr : [" << i << "] - " << ipToString(csIp[i], a, 1);
			memset(a, '\0', 128);
			TRACEINTO << "pServiceParamsIndSt Arr : [" << i << "] - " << ipToString(pServiceParamsIndSt->csIp[i], a, 1);
		}
	}
	// In case we copied all IpV6 addresses - We need to copy the V4 (If there is one).
	BYTE isIpv4Copy = 0;
	if (eGkIpVer == eIpVersion6)
	{
		for (i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
		{
			if(::IsIpNull(&(csIp[i])) == TRUE)
			{
				for (j = 0; j < TOTAL_NUM_OF_IP_ADDRESSES ; j++)
				{
					if (::IsIpNull(&(pServiceParamsIndSt->csIp[j])) == FALSE)
					{
						memcpy(&(csIp[i]), &(pServiceParamsIndSt->csIp[j]), sizeof(ipAddressStruct));
						memset(&(pServiceParamsIndSt->csIp[j]), 0, sizeof(ipAddressStruct));
						isIpv4Copy = 1;
						break;

					}
				}
			}
			if (isIpv4Copy)
				break;
		}
	}
	TRACEINTO << __FUNCTION__ << " Last loop   \n";
	for (i = 0;i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
	{
		memset(a, '\0', 128);
		TRACEINTO << "Temp Arr : [" << i << "] - " << ipToString(csIp[i], a, 1);
		memset(a, '\0', 128);
		TRACEINTO << "pServiceParamsIndSt Arr : [" << i << "] - " << ipToString(pServiceParamsIndSt->csIp[i], a, 1);
	}
	memcpy(&(pServiceParamsIndSt->csIp), csIp , sizeof(ipAddressStruct)*TOTAL_NUM_OF_IP_ADDRESSES);

	TRACEINTO << __FUNCTION__ << " After MEMCPY \n";
	for (i = 0;i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
	{
		memset(a, '\0', 128);
		TRACEINTO << "Temp Arr : [" << i << "] - " << ipToString(csIp[i], a, 1);
		memset(a, '\0', 128);
		TRACEINTO << "pServiceParamsIndSt Arr : [" << i << "] - " << ipToString(pServiceParamsIndSt->csIp[i], a, 1);
	}

}


void CGKServiceManager::CreateTaskName()
{
	char buff[256];
	sprintf(buff, "CGKServiceManager (ServiceId %d)", m_ServiceId);
	m_TaskName = buff;
}

void CGKServiceManager::SetServiceId(DWORD id)
{
	m_ServiceId = id;
	CreateTaskName();
}

//////////////////////////////////////////////////////////////////////
DWORD CGKServiceManager::GetServiceId()
{
	return m_ServiceId;
}

//////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnFailoverSlaveBcmMasterInd(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGKServiceManager::OnFailoverSlaveBcmMasterInd");
	for (int i = 0 ; i < MAX_SERVICES_NUM; i ++)
	{
		CGkService* pService = m_ServicesTable[i];
		if (pService)
		{
			if (pService->AreParamsReady())
			{
				//Unregister the Master
				OnURQReq(pService);

				pService->SetDiscovery(FALSE);
#if 0
				BOOL bIsRrqWithoutGrq = FALSE;


				CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
				std::string key = "RRQ_WITHOUT_GRQ";

				if( pServiceSysCfg != NULL )
				pServiceSysCfg->GetBOOLDataByKey(pService->GetServiceId(), key, bIsRrqWithoutGrq);

				if (bIsRrqWithoutGrq)
					CreateAndSendRRQ(pService, FALSE);
				else
					CreateAndSendGRQ(pService);
#endif
			}
		}
	}

}
////////////////////////////////////////////////////////
void CGKServiceManager::OnFailoverRefreshRegInd(CSegment* pMsg)
{
    PTRACE(eLevelInfoNormal,"CGKServiceManager::OnFailoverRefreshRegInd!");

    
    for (int i = 0 ; i < MAX_SERVICES_NUM; i ++)
    {
        CGkService* pService = m_ServicesTable[i];
        if (pService)
        {
        	if (pService->AreParamsReady())
        	{
        		pService->SetDiscovery(FALSE);
        		BOOL bIsRrqWithoutGrq = FALSE;


        		CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
        		std::string key = "RRQ_WITHOUT_GRQ";

        		if( pServiceSysCfg != NULL )
        		pServiceSysCfg->GetBOOLDataByKey(pService->GetServiceId(), key, bIsRrqWithoutGrq);

        		if (bIsRrqWithoutGrq)
        			CreateAndSendRRQ(pService, FALSE);
        		else
        			CreateAndSendGRQ(pService);
        	}
        }
    }
}

void CGKServiceManager::OnFailoverMasterBcmSlaveInd(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGKServiceManager::OnFailoverMasterBcmSlaveInd");
	for (int i = 0 ; i < MAX_SERVICES_NUM; i ++)
	{
		CGkService* pService = m_ServicesTable[i];
		if (pService)
		{
			if (pService->AreParamsReady())
			{
				OnURQReq(pService);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnPartyUpdateConfIdInd(CSegment* pParam)
{
  	APIU32 connId;
	APIU32 confId;
	*pParam >> connId;
	*pParam >> confId;

	PTRACE2INT(eLevelInfoNormal,"CGKServiceManager::OnPartyUpdateConfIdInd confId ", confId);

	CGkCall *pThisCall = GetGkCall(connId);
	if (!pThisCall)
	{
		PTRACE2INT(eLevelError,"CGKServiceManager::OnPartyUpdateConfIdInd - pThisCall is null. ConnId = ", connId);
		return;
	}
	pThisCall->SetConfRsrcId(confId);

}

//////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnConfPartyCleanUpConfIdInd(CSegment* pParam)
{
	CGkCallSet::iterator iter = m_pCallThroughGkList->begin();
	CGkCallSet::iterator currentIter;
	CGkCall* pGkCall;
	APIU32 confId;
	*pParam >> confId;

	while (iter !=  m_pCallThroughGkList->end())
	{
		currentIter = iter;
		++iter; // increment iter at the beginning, because maybe currentIter should be removed
		pGkCall = *currentIter;
		if( !CPObject::IsValidPObjectPtr(pGkCall))
		{
			  PTRACE(eLevelError,"CGKServiceManager::OnConfPartyCleanUpConfIdInd : pGkCall is not valid");
			  DBGPASSERT(YES);
			  continue;
		}

		if(pGkCall->GetConfRsrcId() == confId)
		{
			PTRACE2INT(eLevelError,"CGKServiceManager::OnConfPartyCleanUpConfIdInd - cleaning = ", pGkCall->GetConnId());

			//inform the party on disconnect the call
			if (pGkCall->GetCallState() == eArqInd)
			{
				UpdateCallStateInGkList(pGkCall, eDrqReqFromGkAfterViolentClose);
				SendDRQReq(pGkCall);

			}
			else
			{
				POBJDELETE(pGkCall);
				m_pCallThroughGkList->erase(currentIter);
			}
		}
	}

}


//////////////////////////////////////////////////////////////////////
void CGKServiceManager::OnConfPartyCleanUpPartyIdInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CGKServiceManager::OnConfPartyCleanUpPartyIdInd");
  CGkCallSet::iterator iter;
  CGkCall* pGkCall;
  APIU32 partyId;
  *pParam >> partyId;

  for (iter = m_pCallThroughGkList->begin() ; iter !=  m_pCallThroughGkList->end(); iter++)
  {
    pGkCall = *iter;
    if( !CPObject::IsValidPObjectPtr(pGkCall))
    {
        PTRACE(eLevelError,"CGKServiceManager::OnConfPartyCleanUpPartyIdInd : pGkCall is not valid");
        DBGPASSERT(YES);
        continue;
    }

    if(pGkCall->GetPartyId() == partyId)
    {
      PTRACE2INT(eLevelError,"CGKServiceManager::OnConfPartyCleanUpPartyIdInd - cleaning = ", pGkCall->GetConnId());

      //inform the party on disconnect the call
      if (pGkCall->GetCallState() == eArqInd)
      {
        UpdateCallStateInGkList(pGkCall, eDrqReqFromGkAfterViolentClose);
        SendDRQReq(pGkCall);

      }
      else
      {
        m_pCallThroughGkList->erase(pGkCall);
        POBJDELETE(pGkCall);
      }
    }
  }
}

////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Function name:CheckBlockOutgoingGRrq()
//  Return value: TRUE: Block RRQ and GRQ, FALSE: non-blocking
BOOL  CGKServiceManager::CheckBlockOutgoingGRrq()
{
	BOOL bJitcMode = FALSE; 
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey("ULTRA_SECURE_MODE", bJitcMode);

	if(bJitcMode)
	{
		return FALSE;
	}
	else
	{
		CGKProcess* pProcess = (CGKProcess*)CProcessBase::GetProcess();
		if (((pProcess->GetIsFailoverFeatureEnabled() == true) && (pProcess->GetIsFailoverSlaveMode() == true))
	                   ||(false == pProcess->GetIsFailoverEndCfg()))
	        	{   
	                	PTRACE(eLevelInfoNormal,"CGKServiceManager::CheckBlockOutgoingGRrq, Failover not ready or became the Slave one, blocking!");
			return TRUE;
		}
	}
	return FALSE;
}

////////////////////////////////////////////////////////
DWORD CGKServiceManager::GetDelayBetweenMessagesValue()
{
	DWORD defaulValue = 10;

	DWORD flagValue = GetSystemCfgFlagInt<DWORD>("GK_DELAY_BETWEEN_MESSAGES_TO_CS");

	if( flagValue != 0 )
		return  flagValue;

	return defaulValue;
}
////////////////////////////////////////////////////////
void CGKServiceManager::OnCsQueueTimeout()
{
	PTRACE(eLevelInfoNormal,"CGKServiceManager::OnCsQueueTimeout");

	if(!messages_.empty())
	{
		CSegment *pMsg = messages_.front();

		SendQueueedMessageToCS(pMsg);

		messages_.pop();
		POBJDELETE(pMsg);

		if(!messages_.empty())
		{
			StartTimer(CS_QUEUE_AND_SEND_TIMER, GetDelayBetweenMessagesValue());
		}
	}
}

////////////////////////////////////////////////////////
void  CGKServiceManager::AddMessageToTheCSQueue( OPCODE opcode, CSegment* pSeg, DWORD serviceId, APIS32 status, DWORD connId, DWORD partyId, DWORD confId, DWORD callIndex, WORD csId )
{
	TRACEINTO <<  " opcode " << opcode;

	CSegment *pMsg = new CSegment;
	*pMsg << opcode
	      << serviceId
	      << status
	      << connId
	      << partyId
	      << confId
	      << callIndex
	      << csId
	      << (*pSeg);

	messages_.push(pMsg);

	if (!IsValidTimer(CS_QUEUE_AND_SEND_TIMER))
		StartTimer(CS_QUEUE_AND_SEND_TIMER, GetDelayBetweenMessagesValue());

}

////////////////////////////////////////////////////////
void  CGKServiceManager::SendQueueedMessageToCS( CSegment *pMsg )
{
	OPCODE opcode;
	CSegment seg;
	DWORD serviceId;
	APIS32 status;
	DWORD connId;
	DWORD partyId;
	DWORD confId;
	DWORD callIndex;
	WORD csId;

	*pMsg >> opcode
		  >> serviceId
		  >> status
		  >>connId
		  >> partyId
		  >> confId
		  >> callIndex
		  >> csId;

	      seg.CopySegmentFromReadPosition(*pMsg);//need to copy because pMsg read offset is not 0

	TRACEINTO <<  " opcode " << opcode;

	m_pGkToCsInterface->SendMsgToCS(opcode, &seg, serviceId, status, connId, partyId, confId, callIndex, csId);

}

