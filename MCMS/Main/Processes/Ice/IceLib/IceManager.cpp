
//////////////////////////////////////////////////////////////////////

#include <algorithm>

#include "IceManager.h"
#include  "MplMcmsProtocol.h"
#include  "OpcodesMcmsCommon.h"
#include  "OpcodesMcmsInternal.h"
#include  "Trace.h"
#include  "SIPProxyStructs.h"
#include  "StatusesGeneral.h"
#include  "DataTypes.h"
#include  "NStream.h"
#include  "CsCommonStructs.h"
#include  "SystemFunctions.h"
#include  "IceProcess.h"
#include  "FaultsDefines.h"
#include  "ConfPartyManagerApi.h"
#include  <algorithm>
#include  "ApiStatuses.h"
#include  "InternalProcessStatuses.h"
#include "SysConfig.h"
#include "ServiceConfigList.h"
#include "IceServiceManager.h"
#include "IceTaskApi.h"
#include "TraceStream.h"


////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CIceManager)
ONEVENT(XML_REQUEST  						  	,IDLE    		  	,CIceManager::HandlePostRequest )
ONEVENT(CS_SERVICE_CFG_UPDATE_IND				,ANYCASE			,CIceManager::OnServiceCfgUpdate)
ONEVENT(CS_PROXY_IP_SERVICE_PARAM_IND			,ANYCASE			,CIceManager::OnIpServiceParamInd)
ONEVENT(CS_PROXY_IP_SERVICE_PARAM_END_IND		,ANYCASE			,CIceManager::OnIpServiceParamEnd)

PEND_MESSAGE_MAP(CIceManager,CManagerTask);


extern void IceMonitorEntryPoint(void* appParam);


extern "C" void IceDispatcherEntryPoint(void* appParam)
{
	CIceDispatcherTask * spDispatcherTask = new CIceDispatcherTask(FALSE);
	spDispatcherTask->Create(*(CSegment*)appParam);
}
////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void IceManagerEntryPoint(void* appParam)
{  
	CIceManager* pSPManager = new CIceManager();
	pSPManager->Create(*(CSegment*)appParam);
}


//////////////////////////////////////////////////////////////////////
TaskEntryPoint CIceManager::GetMonitorEntryPoint()
{
	return IceMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CIceManager::CIceManager()
{
	 m_pProcess = (CIceProcess*)CIceProcess::GetProcess();
}

/////////////////////////////////////////////////////////////////////////////
//Overwrite of ManagerTask function ::CreateDispatcher
/////////////////////////////////////////////////////////////////////////////
void CIceManager::CreateDispatcher()
{
	m_pDispatcherApi = new CTaskApi;
	CreateTask(m_pDispatcherApi, IceDispatcherEntryPoint, m_pRcvMbx);
}


void CIceManager::SelfKill()
{
	CIceServiceManager* pServiceTask = NULL;
	std::vector< CIceServiceManager * >::iterator iter;

	//delete and destroy m_services:
	iter =	m_pProcess->m_services.begin();
	while (iter != m_pProcess->m_services.end())
	{
		pServiceTask = (*iter);
		m_pProcess->m_services.erase(iter); 
		CTaskApi*  pTaskApi = new CTaskApi;
		pTaskApi->CreateOnlyApi(pServiceTask->GetRcvMbx());
		pTaskApi->Destroy();
		POBJDELETE(pTaskApi);
		iter =	m_pProcess->m_services.begin();
	}
	SystemSleep(500);
	CManagerTask::SelfKill();
}	


//////////////////////////////////////////////////////////////////////
CIceManager::~CIceManager()
{
	
}

/////////////////////////////////////////////////////////////////////////////
void*  CIceManager::GetMessageMap()
{
	return (void*)m_msgEntries;    
}


void CIceManager::ManagerPostInitActionsPoint()
{
	// this function is called just before WaitForEvent
	PTRACE(eLevelInfoNormal,__FUNCTION__);
	RequestIPServicesFromCsManager();

}

//////////////////////////////////////////////////////////////////////
void  CIceManager::RequestIPServicesFromCsManager()
{
	PTRACE(eLevelInfoNormal, "CIceManager::RequestIPServicesFromCsManager");
	CSegment*  pRetParam = new CSegment;

	const COsQueue* pCsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

	STATUS res = pCsMbx->Send(pRetParam,ICE_IP_SERVICE_PARAM_REQ);
}

////////////////////////////////////////////////////////////
void  CIceManager::OnServiceCfgUpdate(CSegment* pSeg)
{

	PTRACE(eLevelInfoNormal, "CIceManager::OnServiceCfgUpdate");
	CServiceConfigList *pServiceConfigList = new CServiceConfigList();
	pServiceConfigList->DeSerialize(pSeg);
	CProcessBase::GetProcess()->SetServiceConfigList(pServiceConfigList);


	//////////////////////////test//////////////////
	//CServiceConfigList *testservicecfg=CProcessBase::GetProcess()->GetServiceConfigList();
	//DWORD tt=0;
	//testservicecfg->GetDWORDDataByKey(1,"GW_OVERLAY_TIMEOUT",tt);

	//FTRACESTR(eLevelInfoNormal) << "CIceManager::OnServiceCfgUpdate = " << tt;
}

////////////////////////////////////////////////////////////
void  CIceManager::OnIpServiceParamInd(CSegment* pSeg)
{
	CSegment* pTmpSeg = new CSegment(*pSeg);

	CSipProxyIpParams* pService = new CSipProxyIpParams;
	pService->Deserialize(pSeg);
	COstrStream msg;
	pService->Dump(msg);
	PTRACE2(eLevelInfoNormal, "CIceManager::OnIpServiceParamInd:\n ", msg.str().c_str());

	// Checks whether the received service changes an existing service params and handle the changes if needed
	//HandleChangesInServiceIfNeeded(pService);

	//Check if service id already exists
	DWORD serviceId = pService->GetServiceId();
	CIceServiceManager* pCheckedServiceTask = NULL;
	BOOL bIsServiceIdExists = FALSE;
	std::vector< CIceServiceManager * >::iterator itr = m_pProcess->m_services.begin();
	while (itr != m_pProcess->m_services.end())
	{
		pCheckedServiceTask = (CIceServiceManager*)(*itr);
		if(serviceId == pCheckedServiceTask->GetServiceId())
		{
			bIsServiceIdExists = TRUE;
			TRACEINTO << "Service ID already exists, serviceId:" << serviceId;
			break;
		}
		++itr;
	}


	if(pService->GetIsWebRtcICE() && !bIsServiceIdExists)
	{
		TRACEINTO << "webRTC service, serviceId:" << serviceId;
		CTaskApi* pTaskApi = new CTaskApi;
		pTaskApi->Create( IceWebRtcServiceManagerEntryPoint, GetRcvMbx() );
		CWebRtcServiceManager* pServiceTask = (CWebRtcServiceManager*)pTaskApi->GetTaskAppPtr();
		pServiceTask->SetServiceId(serviceId);
		pServiceTask->SetIsServiceActive(FALSE);
		pServiceTask->SetWebRTCService(pTmpSeg);

		m_pProcess->AddServiceTask(pServiceTask);
			POBJDELETE(pTaskApi);
	} else {
		PTRACE(eLevelInfoNormal, "CIceManager::OnIpServiceParamInd- NON webRTC service");
	}

	POBJDELETE(pTmpSeg);
	POBJDELETE(pService);

}


/////////////////////////////////////////////////////////////////////////////
// Send Create ICE user for each service. Need to deceide which server is it
// by service id and service type: webrtc, standard or MC
void  CIceManager::OnIpServiceParamEnd(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CIceManager::OnIpServiceParamEnd");

	CIceServiceManager* pServiceTask = NULL;
	std::vector< CIceServiceManager * >::iterator itr = m_pProcess->m_services.begin();
	while (itr != m_pProcess->m_services.end())
	{
		pServiceTask = (CIceServiceManager*)(*itr);
		//call to service manager ready
		pServiceTask->SetIsServiceActive(TRUE);
		pServiceTask->SetEndService();
		++itr;
	}

}

