// LicenseServerManager.cpp

#include "LicenseServerManager.h"

#include "Trace.h"
#include "ProcessBase.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "ConfigManagerApi.h"
#include "TraceStream.h"
#include "DummyEntry.h"
#include "Request.h"



#include "FlcLicensing.h"
#include "FlcLicenseManager.h"
#include "FlcMachineType.h"
#include "FlcCapabilityRequest.h"
#include "FlcCapabilityResponse.h"
#include "FlcComm.h"

#include "IdentityClient.h"
#include "LicensingManager.h"
#include "LicensingServerStructs.h"

#include "stubs.h"

PBEGIN_MESSAGE_MAP(CLicenseServerManager)
  ONEVENT(XML_REQUEST, IDLE, CLicenseServerManager::HandlePostRequest)
  ONEVENT(SYSTEM_LICENSE_REFRESH_TIMER, ANYCASE, CLicenseServerManager::OnRefreshLicenseInfo)
  ONEVENT(LICENSE_SERVER_PARAMS_IND, ANYCASE, CLicenseServerManager::ProcessServerConfiguration)
PEND_MESSAGE_MAP(CLicenseServerManager, CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY(CLicenseServerManager)
ON_TRANS("TRANS_LICENSE", "SYNC_LICENSE", CDummyEntry, CLicenseServerManager::SyncLicenseInfo)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CLicenseServerManager)
	ONCOMMAND("refresh",			CLicenseServerManager::HandleTerminalRefreshLicenseInfo,			"acquired last simulated Server Configuration and Print to Screen")
END_TERMINAL_COMMANDS

extern void LicenseServerMonitorEntryPoint(void* appParam);

void LicenseServerManagerEntryPoint(void* appParam)
{
  CLicenseServerManager* mngr = new CLicenseServerManager;
  mngr->Create(*(CSegment*)appParam);
}

TaskEntryPoint CLicenseServerManager::GetMonitorEntryPoint()
{
  return LicenseServerMonitorEntryPoint;
}


CLicenseServerManager::CLicenseServerManager() 
 : isRefreshing(false)
{

   lMgr = new LicensingManager(MCU_MCMS_DIR+"/TS");

   m_pProcess = (CLicenseServerProcess*)CLicenseServerProcess::GetProcess();
}

CLicenseServerManager::~CLicenseServerManager()
{
   if(lMgr)
      delete lMgr;
}

void CLicenseServerManager::ManagerPostInitActionsPoint()
{
  //
  // only when LicenseServerParams is recieved need to start timer and proceed with license acquisition
  CSegment*  pRetParam = new CSegment;

  CManagerApi api(eProcessMcuMngr);
  api.SendMsg(pRetParam, LICENSE_SERVER_PARAMS_REQ);
}

void CLicenseServerManager::SyncLicenseInfo(CRequest* pRequest)
{
  PTRACE(eLevelInfoNormal, "Processing SyncLicenseInfo Transaction");
  pRequest->SetConfirmObject(new CDummyEntry);
  if( !CheckServerType() )
  {
      pRequest->SetStatus(STATUS_FAIL);
      return;
  }

  RefreshLicenseInfo();

  pRequest->SetStatus(STATUS_OK);

  return;
}

void CLicenseServerManager::OnRefreshLicenseInfo(CSegment*)
{
  PTRACE(eLevelInfoNormal, "Processing Refresh License");
  if( !CheckServerType() )
  {
      return;
  }
  string strTmp;
  RefreshLicenseInfo();
  return;
}

void CLicenseServerManager::RefreshLicenseInfo()
{
  // prevent re-entrant refresh, since it is a long blocking operation and we unlock the process semaphore
  if( isRefreshing )
  {
      TRACEINTO << "License Information Refresh is already in progress" << endl;
      return;
  }

  isRefreshing=true;

  if (m_pProcess->IsFlexeraSimulationMode())
  {
	  //refresh from file
	  lMgr->InitLicenseServerParamsFromFile();
	  lMgr->AcquireLicense();
  }
  else
  {
	  UnlockRelevantSemaphore();
	  lMgr->AcquireLicense();
	  LockRelevantSemaphore();
  }


  //once we own the semaphore we can drop the flag
  isRefreshing=false;

  lMgr->Send();

  // get refresh interval in seconds from Capability Response
  int refreshInterval = lMgr->getRenewalInterval();


  if( refreshInterval == 0 )
  {
     CProcessBase* proc = CProcessBase::GetProcess();
     PASSERT_AND_RETURN(NULL == proc);

     CSysConfig* cfg = proc->GetSysConfig();
     PASSERT_AND_RETURN(NULL == cfg);

     DWORD refresh_intvl;
     BOOL res;

     res = cfg->GetDWORDDataByKey(CFG_KEY_LICENSE_VALIDATION_INTERVAL, refresh_intvl);
     PASSERTSTREAM_AND_RETURN(!res, "GetDWORDDataByKey: " << CFG_KEY_LICENSE_VALIDATION_INTERVAL);
     refreshInterval = refresh_intvl;
  }

  TRACEINTO << "New license refresh interval is " << refreshInterval << endl;

  StartTimer(SYSTEM_LICENSE_REFRESH_TIMER, 60*refreshInterval);
}

bool  CLicenseServerManager::CheckServerType()
{
  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN_VALUE(NULL == proc, false);

  CSysConfig* cfg = proc->GetSysConfig();
  PASSERT_AND_RETURN_VALUE(NULL == cfg, false);

  eProductType curProductType = proc->GetProductType();

  return (curProductType == eProductTypeEdgeAxis ? true : false);
}

void  CLicenseServerManager::ProcessServerConfiguration(CSegment* msg)
{
  PTRACE(eLevelInfoNormal, "Processing new License Server Configuration");

  if( !CheckServerType() )
      return;

  LICENSE_SERVER_DATA_S licenseInfo;
  memset(&licenseInfo,0,sizeof(LICENSE_SERVER_DATA_S));

  msg->Get( (BYTE*)&licenseInfo, sizeof(LICENSE_SERVER_DATA_S) );

  //save configuration
  if( licenseInfo.primaryLicenseServerHost == NULL || licenseInfo.primaryLicenseServerPort == 0 )
  {
      TRACEINTO << "Incorrect server and/or port parameters are received. Waiting for the correct information.";
      return; 
  }
  else
  {
      TRACEINTO << "\nLicense Server Info:\nLicense Server\t\t" << (char *)licenseInfo.primaryLicenseServerHost << ":"
                                     << (int)licenseInfo.primaryLicenseServerPort
                                     << "\nMcu UUID\t\t\t" << (char *)licenseInfo.mcuHostId
                                     << "\nMcu Version\t\t\t"<< licenseInfo.mcuVersion.ver_major << "."
                                     << licenseInfo.mcuVersion.ver_minor
                                     <<"\nMcu Port Capacity\t\t"<<licenseInfo.maxMcuCapacity;
  }

  lMgr->setPrimaryServerUri(licenseInfo.primaryLicenseServerHost, licenseInfo.primaryLicenseServerPort);
  lMgr->setCustHostId(licenseInfo.mcuHostId);
  lMgr->setMaxPortCapacity(licenseInfo.maxMcuCapacity);

  // licenseInfo.mcuVersion is not currently used
  
  lMgr->init();

  RefreshLicenseInfo();
}

STATUS CLicenseServerManager::HandleTerminalRefreshLicenseInfo(CTerminalCommand & command, std::ostream& answer)
{
	if (!m_pProcess->IsFlexeraSimulationMode()) //if not in simulation return
	{
		answer << "Process is not under Simulation Mode!!!";
	}
	else
	{
		RefreshLicenseInfo();
		lMgr->GetLastAcquiredTabled(answer);
	}
	return STATUS_OK;
}

