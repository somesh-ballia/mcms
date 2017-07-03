
//////////////////////////////////////////////////////////////////////

#include <algorithm>

#include "SipProxyManager.h"
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
#include "ServiceConfigList.h"
#include "SipProxyServiceManager.h"
#include "SipProxyTaskApi.h"
#include "TraceStream.h"


//#include "IpPartyMonitorDefinitions.h"
//#include "../../IncludeExternal/MPL/Card/CardMngrICE/ICEApiDefinitions.h"
//#include "../../IncludeExternal/MPL/Card/CardMngrICE/IceCmReq.h"




////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CSipProxyManager)
ONEVENT(XML_REQUEST  						  	  ,IDLE    		  ,CSipProxyManager::HandlePostRequest )
ONEVENT(CSAPI_MSG				   				  ,ANYCASE	      ,CSipProxyManager::OnCSApiMsg)

	ONEVENT(CS_SERVICE_CFG_UPDATE_IND			,ANYCASE	,CSipProxyManager::OnServiceCfgUpdate)




PEND_MESSAGE_MAP(CSipProxyManager,CManagerTask);

WORD 	g_numOfPresentedConf;


extern void SipProxyMonitorEntryPoint(void* appParam);
extern const CSipProxyServiceManager*  GetServiceTask(DWORD serviceId);

extern "C" void SipProxyDispatcherEntryPoint(void* appParam)
{
	CSipProxyDispatcherTask * spDispatcherTask = new CSipProxyDispatcherTask(FALSE);
	spDispatcherTask->Create(*(CSegment*)appParam);
}
////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void SipProxyManagerEntryPoint(void* appParam)
{  
	CSipProxyManager* pSPManager = new CSipProxyManager();
	pSPManager->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////
/*void CGKManager::DeclareStartupConditions()
{	
	CActiveAlarm aa(FAULT_GENERAL_SUBJECT, 
					AA_NO_IP_SERVICE_PARAMS, 
					MAJOR_ERROR_LEVEL, 
					"No IP service was received from CSMngr",
					false,
					false);
 	AddStartupCondition(aa);
}*/

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CSipProxyManager::GetMonitorEntryPoint()
{
	return SipProxyMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSipProxyManager::CSipProxyManager()
{
	
	 m_pProcess = (CSipProxyProcess*)CSipProxyProcess::GetProcess();
	 g_numOfPresentedConf = 0;
	//for (int i = 0; i < MAX_SERVICES_NUM; i++)	 
	//	m_ServicesTable[i] = NULL;
		
}

/////////////////////////////////////////////////////////////////////////////
//Overwrite of ManagerTask function ::CreateDispatcher
/////////////////////////////////////////////////////////////////////////////
void CSipProxyManager::CreateDispatcher()
{
	m_pDispatcherApi = new CTaskApi;
	CreateTask(m_pDispatcherApi, SipProxyDispatcherEntryPoint, m_pRcvMbx);
}


void CSipProxyManager::CreateServiceTasks()
{
	for( int i = 1; i <= MAX_SERVICES_NUM; i++ )
	{	 
	        //printf("CSipProxyManager::CreateServiceTasks %d\n", i);
		//SIPProxyMngrApi* pManagerApi = new SIPProxyMngrApi;
		CTaskApi* pTaskApi = new CTaskApi;
		pTaskApi->Create( SipProxyServiceManagerEntryPoint, GetRcvMbx() ); 
		CSipProxyServiceManager* pServiceTask = (CSipProxyServiceManager*)pTaskApi->GetTaskAppPtr();
		pServiceTask->SetServiceId(i);
		pServiceTask->SetIsServiceActive(FALSE);
		//Begin Startup Feature
		CProcessBase::GetProcess()->m_NetSettings.LoadFromFile();
		BYTE status = (BYTE)CProcessBase::GetProcess()->m_NetSettings.m_DnsConfigStatus;
		pServiceTask->SetMngmntDNSStatus(TRUE,status);
		//End Startup Feature
		m_pProcess->AddServiceTask(pServiceTask);
	        POBJDELETE(pTaskApi);

	}

		
}
void CSipProxyManager::SelfKill()
{
	CSipProxyServiceManager* pServiceTask = NULL;	
	std::vector< CSipProxyServiceManager * >::iterator iter;

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
CSipProxyManager::~CSipProxyManager()
{
	
}


/////////////////////////////////////////////////////////////////////////////
void*  CSipProxyManager::GetMessageMap()                                        
{
	return (void*)m_msgEntries;    
}


void CSipProxyManager::ManagerPostInitActionsPoint()
{
	// this function is called just before WaitForEvent
	PTRACE(eLevelInfoNormal,__FUNCTION__);



	CreateServiceTasks();
	RequestIPServicesFromCsManager();


}

//////////////////////////////////////////////////////////////////////
void  CSipProxyManager::RequestIPServicesFromCsManager()
{
	PTRACE(eLevelInfoNormal, "CSipProxyManager::RequestIPServicesFromCsManager");
	CSegment*  pRetParam = new CSegment;

	const COsQueue* pCsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

	STATUS res = pCsMbx->Send(pRetParam,CS_PROXY_IP_SERVICE_PARAM_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipProxyManager::AskMcuMngrForConfigurationStatus()
{
	PTRACE(eLevelInfoNormal, "CSipProxyManager::AskMcuMngrForConfigurationStatus");
	CSegment*  pRetParam = new CSegment;

	const COsQueue* pCsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessMcuMngr, eManager);

	STATUS res = pCsMbx->Send(pRetParam, SIPPROXY_MCUMNGR_CONFIGURATION_REQ);
}

////////////////////////////////////////////////////////////
void  CSipProxyManager::OnServiceCfgUpdate(CSegment* pSeg)
{

	PTRACE(eLevelInfoNormal, "CSipProxyManager::OnServiceCfgUpdate");
	CServiceConfigList *pServiceConfigList = new CServiceConfigList();
	pServiceConfigList->DeSerialize(pSeg);
	CProcessBase::GetProcess()->SetServiceConfigList(pServiceConfigList);


	//////////////////////////test//////////////////
	CServiceConfigList *testservicecfg=CProcessBase::GetProcess()->GetServiceConfigList();
	DWORD tt=0;
	testservicecfg->GetDWORDDataByKey(1,"GW_OVERLAY_TIMEOUT",tt);

	FTRACESTR(eLevelInfoNormal) << "CSipProxyManager::OnServiceCfgUpdate = " << tt;
}



/////////////////////////////////////////////////////////////////////////////
void CSipProxyManager::OnCSApiMsg(CSegment *pSeg)
{
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg, CS_API_TYPE);
	
	CSipProxyMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("***CSipProxyManager::OnCSApiMsg",CS_API_TYPE);	
	
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
	
	//DispatchEvent(opcode, pMsgToSend);
	CSipProxyTaskApi api(serviceId);
	api.SendMsg(pMsgToSend,opcode);
	api.DestroyOnlyApi();
	//PushMessageToQueue(opcode, eProcessCSApi);
	
	POBJDELETE(pMsgToSend);
}

/////////////////////////////////////////////////////////////////////////////
WORD GetPresentedConfNumber()
{
	return g_numOfPresentedConf;
}

/////////////////////////////////////////////////////////////////////////////
void IncreasePresentedConfNumber()
{
	g_numOfPresentedConf++;
}

/////////////////////////////////////////////////////////////////////////////
void DecreasePresentedConfNumber()
{
	if (g_numOfPresentedConf)
		g_numOfPresentedConf--;
}
