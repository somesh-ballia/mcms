// ExchangeModuleManager.cpp

#include "ExchangeModuleManager.h"

#include "OpcodesMcmsCommon.h"
#include "Trace.h"
#include "FipsMode.h"
#include "StatusesGeneral.h"
#include "Segment.h"
#include "OpcodesMcmsInternal.h"
#include "CommResApi.h"
#include "TraceStream.h"
#include "DummyEntry.h"
#include "ApiStatuses.h"
#include "ExchangeModuleCfg.h"
#include "ExchangeModuleProcess.h"
#include "ExchangeModuleCfg.h"
#include "ExchangeClientCntl.h"
#include "ExchangeDataTypes.h"
#include "McmsProcesses.h"
#include "ProcessBase.h"
#include "TaskApi.h"
#include "ManagerApi.h"

extern const char* GetStatusAsString(const int status);

const DWORD CHECK_STARTUP_END_TIME = 1 * SECOND;

const WORD UPDATE_EXCHNAGE_CFG_PARAMETERS = 2022;
const WORD CHECK_STARTUP_END_TOUT         = 2023;

PBEGIN_MESSAGE_MAP(CExchangeModuleManager)
  ONEVENT(XML_REQUEST, IDLE , CExchangeModuleManager::HandlePostRequest)
  ONEVENT(UPDATE_EXCHNAGE_CFG_PARAMETERS,	ANYCASE, CExchangeModuleManager::OnUpdateExchangeCfgParams)
  ONEVENT(MCUMNGR_TO_EXCHANGE_STARTUP_END, ANYCASE, CExchangeModuleManager::OnMcuMngrStartupEndInd)
  ONEVENT(CHECK_STARTUP_END_TOUT, ANYCASE, CExchangeModuleManager::OnCheckStartupEndToutAnycase)

PEND_MESSAGE_MAP(CExchangeModuleManager, CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY(CExchangeModuleManager)
	ON_TRANS("TRANS_MCU","SET_MCU_EXCHANGE_CONFIG_PARAMS",CExchangeModuleCfg,CExchangeModuleManager::HandleSetExchangeModuleCfg)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CExchangeModuleManager)
	ONCOMMAND("items",CExchangeModuleManager::HandleTerminalItems,"print calendar items list")
END_TERMINAL_COMMANDS

extern void ExchangeModuleMonitorEntryPoint(void* appParam);

void ExchangeModuleManagerEntryPoint(void* appParam)
{
	CExchangeModuleManager * pExchangeModuleManager = new CExchangeModuleManager;
	pExchangeModuleManager->Create(*(CSegment*)appParam);
}

CExchangeModuleManager::CExchangeModuleManager()
{
	m_pExchangeModuleCfg = NULL;
	m_pExchangeClientCntl = NULL;
}

CExchangeModuleManager::~CExchangeModuleManager()
{
	POBJDELETE(m_pExchangeModuleCfg);
	POBJDELETE(m_pExchangeClientCntl);
}

TaskEntryPoint CExchangeModuleManager::GetMonitorEntryPoint()
{
	return ExchangeModuleMonitorEntryPoint;
}

void CExchangeModuleManager::ManagerPostInitActionsPoint()
{
  PTRACE(eLevelInfoNormal,"CExchangeModuleManager::ManagerPostInitActionsPoint");

  ///VNGR-24112 fips mode cause this issue.
    TestAndEnterFipsMode(false);

	// delete CFG if exists
	POBJDELETE(m_pExchangeModuleCfg);
	m_pExchangeModuleCfg = new CExchangeModuleCfg();
	m_pExchangeModuleCfg->SetIsEMAState(FALSE);
	// read from Configuration file. If failed - write default configuration
	int status = m_pExchangeModuleCfg->ReadXmlFile();
	if( status != STATUS_OK )
		m_pExchangeModuleCfg->WriteXmlFile();

	// update Process with new CFG
	CExchangeModuleProcess* pProcess = (CExchangeModuleProcess*)CProcessBase::GetProcess();
	pProcess->SetMcuExchangeCfg(*m_pExchangeModuleCfg);
	//Send configuration to ConfParty
	SendExchangeOnOfConfigurationToConfParty(m_pExchangeModuleCfg->GetServiceEnabled());

	//Begin Startup Feature
	pProcess->m_NetSettings.LoadFromFile();
	 if(eIpType_IpV6 != pProcess->m_NetSettings.m_iptype)
	 {
			 char ipStr[IP_ADDRESS_LEN]="";
			SystemDWORDToIpString(pProcess->m_NetSettings.m_ipv4,ipStr);
			 pProcess->SetMngnt(ipStr);
	 }
	 else
	 {
		 std::string ipv6,ipv6Mask;
		 pProcess->m_NetSettings.ConvertIpv6AddressToString(pProcess->m_NetSettings.m_ipv6_0,ipv6,ipv6Mask);
		 pProcess->SetMngnt(ipv6);
	 }
	 //End Startup Feature

	// wait for startup ends to run m_pExchangeClientCntl
	StartTimer(CHECK_STARTUP_END_TOUT,CHECK_STARTUP_END_TIME);
}

void CExchangeModuleManager::OnCheckStartupEndToutAnycase(CRequest *pRequest)
{
	if( IsStartupFinished() == FALSE )
	{
		PTRACE(eLevelInfoNormal,"CExchangeModuleManager::OnCheckStartupEndToutAnycase - System Still In startup");
		StartTimer(CHECK_STARTUP_END_TOUT,CHECK_STARTUP_END_TIME);
		return;
	}

	PTRACE(eLevelInfoNormal,"CExchangeModuleManager::OnCheckStartupEndToutAnycase - System finished startup");

	// delete Client CNTL if exists
	POBJDELETE(m_pExchangeClientCntl);
	m_pExchangeClientCntl = new CExchangeClientCntl(this);
	m_pExchangeClientCntl->UpdateParameters(m_pExchangeModuleCfg);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeModuleManager::SendExchangeOnOfConfigurationToConfParty(bool bExchangeFeatureActivated)
{
	// ===== 1. fill the Segment
	CSegment*  pRetParam = new CSegment;
    *pRetParam << (BOOL)bExchangeFeatureActivated;

	TRACESTR(eLevelInfoNormal) << "\nCExchangeModuleManager::SendExchangeOnOfConfigurationToConfParty"
                           << "\nSize sent to ConfParty process: 1";

	// ===== 2. send
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessConfParty, eManager);

	STATUS res = pMbx->Send(pRetParam, EXCHANGEMNGR_CONFIG_TO_CONFPARTY_IND, &GetRcvMbx());
}

/////////////////////////////////////////////////////////////////////////////
STATUS CExchangeModuleManager::HandleSetExchangeModuleCfg(CRequest *pRequest)
{
	PTRACE(eLevelInfoNormal,"CExchangeModuleManager::HandleSetExchangeModuleCfg");

	if (pRequest->GetAuthorization() == SUPER)
	{
		STATUS status = STATUS_OK;
		std::string statusDesc = "";

		// 1. Check if system finished Startup, if not - status
		if( IsStartupFinished() == FALSE )
		{
			PTRACE(eLevelInfoNormal,"CExchangeModuleManager::HandleSetExchangeModuleCfg - System In startup");
			status = STATUS_ACTION_ILLEGAL_AT_SYSTEM_STARTUP;
		}

		// 2. Validate new configuration
		CExchangeModuleCfg* pNewCfg = NULL;
		if (STATUS_OK == status )
		{
			pNewCfg = (CExchangeModuleCfg*)(pRequest->GetRequestObject());
			status = CExchangeClientCntl::ValidateConfiguration(*pNewCfg);

		}

		// 3. Self message - update configuration
		if (STATUS_OK == status )
		{
			CSegment* pMsg = new CSegment();
			pNewCfg->Serialize(*pMsg);
			CManagerApi exchangeManagerApi(eProcessExchangeModule);
			exchangeManagerApi.SendMsg(pMsg,UPDATE_EXCHNAGE_CFG_PARAMETERS);

			status = STATUS_IN_PROGRESS;
		}

		// 4. send response to client
		pRequest->SetConfirmObject(new CDummyEntry);
		pRequest->SetExDescription(statusDesc.c_str());
		pRequest->SetStatus(status);

		if( STATUS_OK  != status && STATUS_IN_PROGRESS != status)
		{
			PTRACE2INT(eLevelInfoNormal,"CExchangeModuleManager::HandleSetExchangeModuleCfg - status BAD=",status);
			return status;
		}
    }
    else
	{
		PTRACE(eLevelInfoNormal,"CExchangeModuleManager::HandleSetExchangeModuleCfg: No permission to update MCU exchange configuration");
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CExchangeModuleManager::OnUpdateExchangeCfgParams(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CExchangeModuleManager::OnUpdateExchangeCfgParams");

	CExchangeModuleCfg* pNewCfg = new CExchangeModuleCfg();
	pNewCfg->DeSerialize(*pParam);

	std::string strCfg = pNewCfg->Dump();
	PTRACE2(eLevelInfoNormal,"CExchangeModuleManager::OnUpdateExchangeCfgParams - ",strCfg.c_str());

	std::string statusDesc;
	STATUS status = m_pExchangeClientCntl->TestConnection(*pNewCfg,statusDesc);

	CProcessBase * process = CProcessBase::GetProcess();
	CSegment *pSeg = new CSegment;
	*pSeg << (WORD)status;
	*pSeg << statusDesc.c_str();
	CTaskApi exchangeMonitorApi(eProcessExchangeModule,eMonitor);
	STATUS returnStatus = STATUS_OK;
	CSegment rspMsg;
	OPCODE resOpcode;
	STATUS statusFromExchangeProcess = STATUS_OK;

	STATUS responseStatus = exchangeMonitorApi.SendMessageSync(pSeg,EXCHNAGE_MONITOR_UPDATE_SET_CFG_PARAMS,100,resOpcode, rspMsg);

	if (STATUS_OK == responseStatus && STATUS_OK == status)
	{
		BOOL bIsExchangeEnabledBeforeChange =  m_pExchangeModuleCfg->GetServiceEnabled();
		*m_pExchangeModuleCfg = *pNewCfg;
		m_pExchangeModuleCfg->EncrptPassword();
		m_pExchangeModuleCfg->WriteXmlFile();

		// update CFG in Client CNTL
		m_pExchangeClientCntl->UpdateParameters(m_pExchangeModuleCfg);
		if (bIsExchangeEnabledBeforeChange!=m_pExchangeModuleCfg->GetServiceEnabled())//only if configuration Changed notify ConfParty on the change
			SendExchangeOnOfConfigurationToConfParty(m_pExchangeModuleCfg->GetServiceEnabled());

		// update Process with new CFG
		CExchangeModuleProcess* pProcess = (CExchangeModuleProcess*)CProcessBase::GetProcess();
		pProcess->SetMcuExchangeCfg(*m_pExchangeModuleCfg);
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CExchangeModuleManager::OnUpdateExchangeCfgParams: Failed to send Status to Exchange monitor status=:",responseStatus);
	}

	POBJDELETE(pNewCfg);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CExchangeModuleManager::HandleTerminalItems(CSegment* seg, std::ostream& answer)
{
	std::stringstream list;

	CExchangeModuleProcess* pProcess = (CExchangeModuleProcess*)CProcessBase::GetProcess();
	const CCalendarItemList* pCalendarItemList = pProcess->GetCalendarItemList();
	pCalendarItemList->Dump(list);

	//PTRACE2(eLevelError,"CExchangeModuleManager::HandleTerminalItems - ",list.str().c_str());
	answer << list.str().c_str();

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
void CExchangeModuleManager::OnMcuMngrStartupEndInd(CSegment *pSeg)
{
	// 13.06.10 - do nothing since VNGR-15168 treatment

//	TRACEINTO << "\nCExchangeModuleManager::OnMcuMngrStartupEndInd";
//	m_pExchangeClientCntl->SetSystemStartupOver();
}

/////////////////////////////////////////////////////////////////////////////
BOOL CExchangeModuleManager::IsStartupFinished() const
{
	eMcuState systemState = CProcessBase::GetProcess()->GetSystemState();
	if( eMcuState_Invalid == systemState || eMcuState_Startup == systemState )
		return FALSE;
	return TRUE;
}



