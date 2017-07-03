
//////////////////////////////////////////////////////////////////////

#include <algorithm>

#include "GKManager.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "Trace.h"
#include "IpMngrOpcodes.h"
#include "GKGlobals.h"
#include "StatusesGeneral.h"
#include "MplMcmsProtocol.h"
#include "RvCommonDefs.h"
#include "TaskApi.h"
//#include "CsCommonStructs.h"
#include "SysConfig.h"
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
//#include "ConfPartyManagerApi.h"
#include "GkTaskApi.h"
#include "ServiceConfigList.h"




////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CGKManager)
ONEVENT(XML_REQUEST  						  	  ,IDLE    		  ,CGKManager::HandlePostRequest )
//ONEVENT(MCUMNGR_GK_MNGMNT_IND			          ,ANYCASE		  ,CGKManager::OnMcuMngrManagementIpUpdate)
ONEVENT(CSAPI_MSG				   				  ,ANYCASE	      ,CGKManager::OnCSApiMsg)
ONEVENT(FAILOVER_SLAVE_BECOME_MASTER			  ,ANYCASE		,CGKManager::OnFailoverSlaveBcmMasterInd)
ONEVENT(CS_SERVICE_CFG_UPDATE_IND				  ,ANYCASE		  ,CGKManager::OnCSMngrServiceCfgUpdate)
ONEVENT(FAILOVER_REFRESH_GK_REG_IND                                  ,ANYCASE               , CGKManager::OnGkMngrFailoverRefreshRegInd)
ONEVENT(FAILOVER_START_MASTER_BECOME_SLAVE             ,ANYCASE               , CGKManager::OnFailoverMasterBcmSlaveInd)
PEND_MESSAGE_MAP(CGKManager,CManagerTask);



extern void GKMonitorEntryPoint(void* appParam);
extern const CGKServiceManager*  GetServiceTask(DWORD serviceId);

extern "C" void GKDispatcherEntryPoint(void* appParam)
{
	CGKDispatcherTask * gkDispatcherTask = new CGKDispatcherTask(FALSE);
	gkDispatcherTask->Create(*(CSegment*)appParam);
}
////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void GKManagerEntryPoint(void* appParam)
{  
	CGKManager* pGKManager = new CGKManager();
	pGKManager->Create(*(CSegment*)appParam);
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
TaskEntryPoint CGKManager::GetMonitorEntryPoint()
{
	return GKMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGKManager::CGKManager()
{
	
	 m_pProcess = (CGKProcess*)CGKProcess::GetProcess();

	//for (int i = 0; i < MAX_SERVICES_NUM; i++)	 
	//	m_ServicesTable[i] = NULL;
		
}

/////////////////////////////////////////////////////////////////////////////
//Overwrite of ManagerTask function ::CreateDispatcher
/////////////////////////////////////////////////////////////////////////////
void CGKManager::CreateDispatcher()
{
	m_pDispatcherApi = new CTaskApi;
	CreateTask(m_pDispatcherApi, GKDispatcherEntryPoint, m_pRcvMbx);
//	m_pDispatcherApi->Create(ConfPartyDispatcherEntryPoint,*m_pRcvMbx);
}


void CGKManager::CreateServiceTasks()
{
	for( int i = 1; i <= MAX_SERVICES_NUM; i++ )
	{ 
		CGKManagerApi* pGkManagerApi = new CGKManagerApi;
		pGkManagerApi->Create( GKServiceManagerEntryPoint, GetRcvMbx() ); 
		CGKServiceManager* pGkServiceTask = (CGKServiceManager*)pGkManagerApi->GetTaskAppPtr();
		pGkServiceTask->SetServiceId(i);
		pGkServiceTask->SetIsServiceActive(FALSE);
		m_pProcess->AddServiceTask(pGkServiceTask);
		POBJDELETE(pGkManagerApi);

	}
	
}

void CGKManager::SelfKill()
{
	CGKServiceManager* pGkServiceTask = NULL;	
	std::vector< CGKServiceManager * >::iterator iter;
	
	//delete and destroy m_services:
	iter =  m_pProcess->m_services.begin();
	while (iter != m_pProcess->m_services.end())
	{
		pGkServiceTask = (*iter);
		m_pProcess->m_services.erase(iter);		
		CTaskApi*  pTaskApi = new CTaskApi;
		pTaskApi->CreateOnlyApi(pGkServiceTask->GetRcvMbx());
		pTaskApi->Destroy();
		POBJDELETE(pTaskApi);
		
		iter =  m_pProcess->m_services.begin();
	}
	
	CManagerTask::SelfKill();

}

//////////////////////////////////////////////////////////////////////
CGKManager::~CGKManager()
{
	
}


/////////////////////////////////////////////////////////////////////////////
void*  CGKManager::GetMessageMap()                                        
{
	return (void*)m_msgEntries;    
}


void CGKManager::ManagerPostInitActionsPoint()
{
	// this function is called just before WaitForEvent
	PTRACE(eLevelInfoNormal,__FUNCTION__);
	//Begin Startup Feature
	m_pProcess->m_NetSettings.LoadFromFile();
	// End Startup Feature
	CreateServiceTasks();
	CSegment *pRetSeg = new CSegment;
	SendReqToCSMngr(pRetSeg, CS_GKMNGR_IP_SERVICE_PARAM_REQ);

}

//////////////////////////////////////////////////////////////////////
void CGKManager::RequestManagementIp()
{	
	CManagerApi api(eProcessMcuMngr);
    STATUS res = api.SendOpcodeMsg(MCUMNGR_GK_MNGMNT_REQ);
	if (res != STATUS_OK)
		FPASSERT(MCUMNGR_GK_MNGMNT_REQ);
}
void CGKManager::OnMcuMngrManagementIpUpdate(CSegment* pParam)
{
	// this function is called just before WaitForEvent
	CGatekeeperTaskApi api(3);
	api.SendMsg(pParam,MCUMNGR_GK_MNGMNT_IND);
	api.DestroyOnlyApi();
	
}

////////////////////////////////////////////////////////////
void CGKManager::UpdateFaultsAndActiveAlarms(DWORD serviceId, eGkFaultsAndServiceStatus eFaultsOpcode)
{
	if (eFaultsOpcode != eGkFaultsAndServiceStatusOk) //in case of status ok - no message should be sent to faluts and active alarms
	{
		if (m_prevFaultsOpcode != eFaultsOpcode)
		{			
			ALLOCBUFFER(faultsStr, ONE_LINE_BUFFER_LEN);
			memset(faultsStr, 0, ONE_LINE_BUFFER_LEN );
			strncpy(faultsStr, GetFaultsOpcodeAsString(eFaultsOpcode), ONE_LINE_BUFFER_LEN);
			faultsStr[ONE_LINE_BUFFER_LEN - 1] = '\0';
			
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
BYTE CGKManager::IsErrorFaultsOpcode(eGkFaultsAndServiceStatus eFaultsOpcode)
{
	if ((eFaultsOpcode != eGkFaultRegistrationSucceeded) && (eFaultsOpcode != eGkFaultsAndServiceStatusOk) )
		return TRUE;
	else
		return FALSE;
}

///////////////////////////////////////////////////////////////////////////
char* CGKManager::GetFaultsOpcodeAsString(eGkFaultsAndServiceStatus eFaultsOpcode)
{
	if (eFaultsOpcode < NumOfGkFaultsAndServiceStatus)
		return GkFaultsAndServiceStatusNames[eFaultsOpcode];
	else
		return "GK problem";
}

///////////////////////////////////////////////////////////////////////////
BYTE CGKManager::GetFaultsLevelAccordingToOpcde(eGkFaultsAndServiceStatus eFaultsOpcode)
{
	BYTE faultsLevel = MAJOR_ERROR_LEVEL; //for now - all the gk errors are MAJORS. //minors.
	return faultsLevel;
}


void CGKManager::OnCSApiMsg(CSegment *pSeg)
{
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg, CS_API_TYPE);
	
	CGkMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("***CGKManager::OnCSApiMsg",CS_API_TYPE);	
	
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
	CGatekeeperTaskApi api(serviceId);
	api.SendMsg(pMsgToSend,opcode);
	api.DestroyOnlyApi();
	//PushMessageToQueue(opcode, eProcessCSApi);
	
	POBJDELETE(pMsgToSend);
}

/////////////////////////////////////////////////////////////////////////////
void  CGKManager::OnCSMngrServiceCfgUpdate(CSegment* pSeg)
{

	PTRACE(eLevelInfoNormal, "CGKManager::OnCSMngrServiceCfgUpdate");
	CServiceConfigList *pServiceConfigList = new CServiceConfigList();
	pServiceConfigList->DeSerialize(pSeg);
	CProcessBase::GetProcess()->SetServiceConfigList(pServiceConfigList);


	/////////////////////////////test////////////////////////////////////////////
	CServiceConfigList *testservicecfg=CProcessBase::GetProcess()->GetServiceConfigList();
	DWORD tt=0;
	bool res =testservicecfg->GetDWORDDataByKey(1,"SIP_REGISTER_DELAY_MILLI_SEC",tt);

	FTRACESTR(eLevelInfoNormal) << "CGKManager::OnCSMngrServiceCfgUpdate = " << res << "tt:" << tt;
}


void	CGKManager::OnFailoverSlaveBcmMasterInd(CSegment *pSeg)
{
	for( DWORD i = 0; i < MAX_SERVICES_NUM; i++ )
	{
		CGKServiceManager* pSvrMgr = m_pProcess->GetServiceTask(i);
		if (pSvrMgr)
		{
			if (pSvrMgr->GetIsServiceActive())
			{
				CTaskApi*  pTaskApi = new CTaskApi;
				pTaskApi->CreateOnlyApi(pSvrMgr->GetRcvMbx());
				pTaskApi->SendOpcodeMsg(FAILOVER_SLAVE_BECOME_MASTER);
				pTaskApi->DestroyOnlyApi();
				POBJDELETE(pTaskApi);
			}
		}
	}	
}
void CGKManager::OnGkMngrFailoverRefreshRegInd(CSegment* pMsg)
{
  WORD isSetEndFlag = false;

  *pMsg >> isSetEndFlag;

   bool bIsSetEndFlag = (bool) isSetEndFlag;

    PTRACE2INT(eLevelInfoNormal, "CGKManager::OnGkMngrFailoverRefreshRegInd, isSetEndFlag = ", (int)bIsSetEndFlag);

    if(bIsSetEndFlag)
    m_pProcess->SetIsFailoverEndcfg(true);

    for( DWORD i = 0; i < MAX_SERVICES_NUM; i++ )
    {
    	CGKServiceManager* pSvrMgr = m_pProcess->GetServiceTask(i);
    	if (pSvrMgr)
    	{
    		if (pSvrMgr->GetIsServiceActive())
    		{  		
			CTaskApi*  pTaskApi = new CTaskApi;
			pTaskApi->CreateOnlyApi(pSvrMgr->GetRcvMbx());
			pTaskApi->SendOpcodeMsg(FAILOVER_REFRESH_GK_REG_IND);
			pTaskApi->DestroyOnlyApi();
			POBJDELETE(pTaskApi);
    		}
    	}
    }	
}
void	CGKManager::OnFailoverMasterBcmSlaveInd(CSegment *pSeg)
{
	for( DWORD i = 0; i < MAX_SERVICES_NUM; i++ )
	{
		CGKServiceManager* pSvrMgr = m_pProcess->GetServiceTask(i);
		if (pSvrMgr)
		{
			if (pSvrMgr->GetIsServiceActive())
			{
				CTaskApi*  pTaskApi = new CTaskApi;
				pTaskApi->CreateOnlyApi(pSvrMgr->GetRcvMbx());
				pTaskApi->SendOpcodeMsg(FAILOVER_START_MASTER_BECOME_SLAVE);
				pTaskApi->DestroyOnlyApi();
				POBJDELETE(pTaskApi);
			}
		}
	}	
}


