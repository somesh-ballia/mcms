// FailoverManager.cpp: implementation of the CFailoverManager class.
//
//////////////////////////////////////////////////////////////////////


#include "FailoverManager.h"
#include "FailDetectionTask.h"
#include "FailoverSyncTask.h"
#include "FailoverConfiguration.h"
#include "OpcodesMcmsCommon.h"
#include "Trace.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "psosxml.h"
#include "XmlMiniParser.h"
#include "OpcodesMcmsInternal.h"
#include "FaultsDefines.h"
#include "ManagerApi.h"
#include "LogInConfirm.h"
#include "FailoverCommunication.h"
#include "Request.h"
#include "InternalProcessStatuses.h"
#include "FailoverDefines.h"
#include "ObjString.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "DummyEntry.h"
#include "StringsLen.h"
#include "McmsDaemonApi.h"
#include "ConfigManagerApi.h"
#include "InitCommonStrings.h"
#include "FipsMode.h"
#include "GkTaskApi.h"
#include "ServiceConfigList.h"
#include "Versions.h"

extern char* ProcessTypeToString(eProcessType processType);


/////////////////////////////////////////////////////////////////////////////
//
//   STATES:
//  IDLE -> INIT_CONFIGURATION -> CONNECTING_SOCKET -> MATER_SLAVE_DETERMINATION -> SLAVE_READY -> SLAVE_RESTORE_MASTER
//const WORD  IDLE        = 0;        // default state -  defined in base class
const WORD  INIT_CONFIGURATION			 = 5;
const WORD  CONNECTING_SOCKET  			 = 6;
const WORD  MATER_SLAVE_DETERMINATION    = 7;
const WORD  SLAVE_READY         		 = 8;
const WORD  SLAVE_RESTORE_MASTER	     = 9;
const WORD  LOGOUT_RESPONSE				 = 10;
const WORD  RELOGIN_AFTER_LOGOUT		 = 11;
//const WORD  ANYCASE     = 0xFFFF;   // any other state -  defined in base class



////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CFailoverManager)
	ONEVENT(XML_REQUEST,   		   IDLE,  CFailoverManager::HandlePostRequest )

  	// Socket: connection established
	ONEVENT(SOCKET_CONNECTED, 			CONNECTING_SOCKET, 	CFailoverManager::OnSocketConnectedConnectingSocket)

	// Socket: connection failed
	ONEVENT(SOCKET_FAILED,   			CONNECTING_SOCKET,	CFailoverManager::OnSocketFailedConnectingSocket)

	// Socket: connection dropped
	ONEVENT(SOCKET_DROPPED,  			CONNECTING_SOCKET, 	CFailoverManager::OnSocketDroppedConnectingSocket)
	ONEVENT(SOCKET_DROPPED,   			MATER_SLAVE_DETERMINATION,	CFailoverManager::OnSocketDroppedMasterSlaveDetermination)
	ONEVENT(SOCKET_DROPPED,   			SLAVE_READY, 			CFailoverManager::OnSocketDroppedSlaveReady)
	ONEVENT(SOCKET_DROPPED,   			SLAVE_RESTORE_MASTER,	CFailoverManager::OnSocketDroppedSlaveRestoreMaster)
	ONEVENT(SOCKET_DROPPED,   			RELOGIN_AFTER_LOGOUT, 	CFailoverManager::OnSocketDroppedReloginAfterLogout)
	ONEVENT(SOCKET_DROPPED,   			IDLE,					CFailoverManager::OnSocketDroppedIdle)

	// Socket: receive indication
	ONEVENT(FAILOVER_SOCKET_RCV_IND,    MATER_SLAVE_DETERMINATION,  CFailoverManager::OnSocketRcvIndMasterSlaveDetermination)
	ONEVENT(FAILOVER_SOCKET_RCV_IND,    IDLE,  		CFailoverManager::OnSocketRcvIndIdle)
    ONEVENT(FAILOVER_SOCKET_RCV_IND,    LOGOUT_RESPONSE,  	CFailoverManager::OnSocketRcvIndlogout)
    ONEVENT(FAILOVER_SOCKET_RCV_IND,    RELOGIN_AFTER_LOGOUT,  	CFailoverManager::OnSocketRcvIndlogout)
    ONEVENT(FAILOVER_SOCKET_RCV_IND,    ANYCASE,  	CFailoverManager::OnSocketRcvIndAnycase)

	ONEVENT(MASTER_DOWN_IND,			ANYCASE,	CFailoverManager::OnMasterDownInd)
	ONEVENT(WHOLE_SYNC_COMPLETED_IND,	ANYCASE,	CFailoverManager::OnWholeSyncCompletedInd)

	ONEVENT(FAILOVER_MCUMNGR_CONFIG_IND,		INIT_CONFIGURATION  ,CFailoverManager::OnMcuMngrConfigIndInitConfiguration)
	ONEVENT(FAILOVER_AUTHENTICATION_CONFIG_IND,	INIT_CONFIGURATION  ,CFailoverManager::OnAuthenticationConfigIndInitConfiguration)
	ONEVENT(FAILOVER_MCUMNGR_GET_STATE_EX_IND,		IDLE,			CFailoverManager::OnMcuMngrGetStateExIdle)
	ONEVENT(FAILOVER_MCUMNGR_GET_STATE_EX_IND,		ANYCASE,		CFailoverManager::OnMcuMngrGetStateExAnycase)

			//Self timers:
	ONEVENT(FAILOVER_MASTER_GET_STATE_FROM_SLAVE_TIMER,	IDLE,	    CFailoverManager::OnTimerMasterGetStateFromSlaveTimeoutIdle)
	ONEVENT(FAILOVER_MASTER_GET_STATE_FROM_SLAVE_TIMER,	ANYCASE,	CFailoverManager::OnTimerMasterGetStateFromSlaveTimeoutAnycase)

	ONEVENT(FAILOVER_RECONNECT_SOCKET_TIMER,		ANYCASE,	CFailoverManager::OnTimerReconnectSocketTimeout)
	ONEVENT(FAILOVER_RELOGIN_TIMER,					ANYCASE,	CFailoverManager::OnTimerReloginTimeout)

	ONEVENT(FAILOVER_MASTER_PING_TIMER, 			IDLE,		CFailoverManager::OnTimerMasterPingTimeoutIdle)
	ONEVENT(FAILOVER_MASTER_PING_TIMER, 			ANYCASE,	CFailoverManager::OnTimerMasterPingTimeoutAnycase)

	//login arrived
	ONEVENT(FAILOVER_LOGIN_ARRIVED,					ANYCASE,	CFailoverManager::OnLoginArrived)

	//conf party indications:
	ONEVENT(FAILOVER_CONFPARTY_END_PREPARE_MASTER_BECOME_SLAVE,	    SLAVE_READY,   CFailoverManager::OnConfPartyEndPrepareMasterBecomeSlaveSlaveReady)
	ONEVENT(FAILOVER_CONFPARTY_END_PREPARE_MASTER_BECOME_SLAVE, 	ANYCASE,	   CFailoverManager::OnConfPartyEndPrepareMasterBecomeSlaveAnycase)

	ONEVENT(FAILOVER_RESTART_SLAVE,	    SLAVE_READY,   CFailoverManager::OnFailoverRestartSlaveInd)

	//auto reboot event
	ONEVENT(FAILOVER_EVENT_TRIGGER_IND, 	ANYCASE,	   CFailoverManager::OnEventTriggerIndAnycase)

	ONEVENT(FAILOVER_LED_STATUS_IND, 	ANYCASE,	   CFailoverManager::LedHotStandbyStateInd)
	ONEVENT(MCUMNGR_SECURITY_MODE_IND,					    ANYCASE,   CFailoverManager::OnMcuMngrSecurityModeInd)

    /*Begin:added by Richer for BRIDGE-14263, 2014.7.16*/
    ONEVENT(FAILOVER_INIT_TIMER, 	ANYCASE,	   CFailoverManager::OnTimerInitFailoverTimeout)
    /*End:added by Richer for BRIDGE-14263, 2014.7.16*/


PEND_MESSAGE_MAP(CFailoverManager,CManagerTask);


////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CFailoverManager)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CFailoverManager::HandleOperLogin)
ON_TRANS("TRANS_HOTBACKUP", "UPDATE",       CFailoverConfiguration,	CFailoverManager::OnServerSetFailoverConfiguration)

END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CFailoverManager)
	ONCOMMAND("netdown",CFailoverManager::HandleTerminalNetworkDown,"test terminal commands")
	
END_TERMINAL_COMMANDS


extern void FailoverMonitorEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void FailoverManagerEntryPoint(void* appParam)
{
	CFailoverManager* pFailoverManager = new CFailoverManager;
	pFailoverManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CFailoverManager::GetMonitorEntryPoint()
{
	return FailoverMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFailoverManager::CFailoverManager()
{
	m_pProcess = (CFailoverProcess*)(CProcessBase::GetProcess());

	PASSERT_AND_RETURN(NULL == m_pProcess);
	m_pProcess->m_NetSettings.LoadFromFile(); //load settings from mcmsnetwork file

	m_eMasterSlaveCurrentState = eMasterSlaveNone;

	m_pFailoverCommunicationFromProcess = new CFailoverCommunication();
	m_pProcess->SetFailoverCommunication(m_pFailoverCommunicationFromProcess);

	m_pMngmntInfoFromProcess = new MASTER_SLAVE_DATA_S();
	CVersions version;
	std::string versionFilePath = VERSIONS_FILE_PATH;
	version.ReadXmlFile(versionFilePath.c_str());
	VERSION_S mcuVer = version.GetMcuVersion();
	m_pMngmntInfoFromProcess->mcuVer.ver_major=mcuVer.ver_major;
	m_pMngmntInfoFromProcess->mcuVer.ver_minor=mcuVer.ver_minor;
	m_pMngmntInfoFromProcess->mcuVer.ver_release=mcuVer.ver_release;
	m_pMngmntInfoFromProcess->mcuVer.ver_internal=mcuVer.ver_internal;
	m_pMngmntInfoFromProcess->ipType = m_pProcess->m_NetSettings.m_iptype;
	m_pMngmntInfoFromProcess->mngmntIpAddress = m_pProcess->m_NetSettings.m_ipv4;
	m_pMngmntInfoFromProcess->defaultGwIpAddress = m_pProcess->m_NetSettings.m_ipv4_DefGw;

	m_pProcess->SetMngmntInfo(m_pMngmntInfoFromProcess);

	m_pFailDetectionApi = NULL;
	m_pFailoverSyncApi = NULL;
	/*Set it by default. So the master will continue to connect the slave even if it fail after starting up*/
	m_bIsNetworkProblemsInMaster = TRUE;

	m_bSimuNetworkDown = FALSE;
}

//////////////////////////////////////////////////////////////////////
CFailoverManager::~CFailoverManager()
{
	POBJDELETE(m_pFailDetectionApi);
	POBJDELETE(m_pFailoverSyncApi);
	POBJDELETE(m_pMngmntInfoFromProcess);//Fix Core ,the m_pFailoverCommunication created in the managerTask ,and should be deleted in this context VNGR-19696
	POBJDELETE(m_pFailoverCommunicationFromProcess);//Fix Core ,the m_pFailoverCommunication created in the managerTask ,and should be deleted in this context VNGR-19696


	if (IsValidTimer(FAILOVER_RECONNECT_SOCKET_TIMER))
		DeleteTimer(FAILOVER_RECONNECT_SOCKET_TIMER);

	if (IsValidTimer(FAILOVER_RELOGIN_TIMER))
		DeleteTimer(FAILOVER_RELOGIN_TIMER);

	if (IsValidTimer(FAILOVER_MASTER_PING_TIMER))
		DeleteTimer(FAILOVER_MASTER_PING_TIMER);

	if (IsValidTimer(FAILOVER_MASTER_GET_STATE_FROM_SLAVE_TIMER))
		DeleteTimer(FAILOVER_MASTER_GET_STATE_FROM_SLAVE_TIMER);
    
    /*Begin:added by Richer for BRIDGE-14263, 2014.7.16*/
    if (IsValidTimer(FAILOVER_INIT_TIMER))
        DeleteTimer(FAILOVER_INIT_TIMER);
    /*End:added by Richer for BRIDGE-14263, 2014.7.16*/
}

/////////////////////////////////////////////////////////////////////
void CFailoverManager::SelfKill()
{
	if (m_pFailDetectionApi)
		m_pFailDetectionApi->Destroy();

	if (m_pFailoverSyncApi)
		m_pFailoverSyncApi->Destroy();

	if (m_pFailoverCommunicationFromProcess)
		m_pFailoverCommunicationFromProcess->DisconnectSocket();

	CManagerTask::SelfKill();
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::ManagerPostInitActionsPoint()
{
	PTRACE(eLevelInfoNormal,"\nCFailoverManager::ManagerPostInitActionsPoint");
	TestAndEnterFipsMode();
	 // (1) Init Master/slave configuration object
    /*added by Richer for BRIDGE-14263, 2014.7.16*/
    if (TRUE == IsStartupFinished())
    {   
        InitFailoverConfiguration();
    }
    else
    {
        /*if startup is not over, start a timer to check it, and then  init failover after startup is over*/
        StartTimer(FAILOVER_INIT_TIMER, FAILOVER_INIT_TIMER_TIMEOUT);
    }
}

/*Begin:added by Richer for BRIDGE-14263, 2014.7.16*/
/////////////////////////////////////////////////////////////////////////////
BOOL CFailoverManager::IsStartupFinished() const
{
	eMcuState systemState = CProcessBase::GetProcess()->GetSystemState();
	if( eMcuState_Invalid == systemState || eMcuState_Startup == systemState )
		return FALSE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::OnTimerInitFailoverTimeout(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::OnTimerInitFailoverTimeout";

	BOOL bIsStartupFinished = IsStartupFinished();
	if (bIsStartupFinished == TRUE)
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::OnTimerInitFailoverTimeout - initFailover";
        
		DeleteTimer(FAILOVER_INIT_TIMER);
        
		InitFailoverConfiguration();
	}
	else 
	{
		StartTimer(FAILOVER_INIT_TIMER, FAILOVER_INIT_TIMER_TIMEOUT);
	}
}
/*End:added by Richer for BRIDGE-14263, 2014.7.16*/

//////////////////////////////////////////////////////////////////////
void CFailoverManager::OnLoginArrived(CSegment* pSeg)
{
	bool bIsError = FALSE;

	char  actual_pair_mcu_ip[IP_ADDRESS_STR_LEN];
	WORD pairRmxCurrentState = 0;
	*pSeg >> pairRmxCurrentState
		  >> actual_pair_mcu_ip;
	std::string str;

	if (m_eMasterSlaveCurrentState == eMasterSlaveNone)//SET THE CURRENT STATE TO THE CONFIGURATION IF IT NONE
	{
		m_eMasterSlaveCurrentState = ConvertHotBackupTypeToMasterSlaveState(m_pProcess->GetFailoverConfiguration()->GetHotBackupType());;
	}
	if (IsTarget())
	{
		if (m_eMasterSlaveCurrentState != eMasterSlaveNone && actual_pair_mcu_ip != m_pProcess->GetFailoverConfiguration()->GetOtherRmxIp())
		{
			bIsError = TRUE;
			str = "The pairing MCUs do not have reciprocal IP addresses";
			SendActiveAlarmDeterminationConflict(str);
			m_eMasterSlaveCurrentState = eMasterSlaveNone;
		}
	}
	if (m_eMasterSlaveCurrentState == eSlaveState  && pairRmxCurrentState == eSlaveState)
	{
		bIsError = TRUE;
		str = "Both RMX's are configured as slave";
		SendActiveAlarmDeterminationConflict(str);
	}
	else if((m_eMasterSlaveCurrentState == eMasterConfigurationState) && pairRmxCurrentState==eMasterConfigurationState)
	{
		bIsError = TRUE;
		str = "Both RMX's are configured as master";
		SendActiveAlarmDeterminationConflict(str);

		StopMasterCheckOnGetStateFromSlave();
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnLoginArrived - sync OK";
		if (m_state == SLAVE_RESTORE_MASTER)
			m_state = IDLE;
		StartMasterTimersIfNeeded();
        // Notify GatekeeperProcess to re-registrate as a decided master.
        if(eMasterActualState== m_eMasterSlaveCurrentState)
			SendFailoverRefreshRegToGatekeeper(FALSE);        
	}
	/*else if(m_eMasterSlaveCurrentState != eMasterSlaveNone && pairRmxCurrentState == eMasterSlaveNone)
	{
		bIsError = TRUE;
		str = "Hot backup is not enabled on the paired MCU";
		SendActiveAlarmDeterminationConflict(str);
	}*/

	eFailoverStatusType eSyncStatus = eSynchOk;
	if (bIsError == TRUE)
		eSyncStatus = eFail;
	UpdateMasterSlaveState(eSyncStatus);

	if (bIsError == FALSE)
		RemoveFailoverActiveAlarms();

	CSegment*  pRspMsg = new CSegment;
	*pRspMsg << (WORD)m_eMasterSlaveCurrentState;
	ResponedClientRequest(FAILOVER_LOGIN_ARRIVED, pRspMsg);
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::InitFailoverConfiguration()
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::InitFailoverConfiguration";
	STATUS retStatus = STATUS_OK;

	retStatus = m_pProcess->GetFailoverConfiguration()->ReadXmlFile(FAILOVER_CFG_FILE,eNoActiveAlarm, eRenameFile);

	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - InitFailoverConfiguration: read file status";

	if ( (STATUS_FILE_NOT_EXIST == retStatus) || (STATUS_OPEN_FILE_FAILED == retStatus) ) // no file exists (yet) - create a default, hard-coded MngmntNetworkInterface
	{
		if (STATUS_FILE_NOT_EXIST == retStatus)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - InitFailoverConfiguration: STATUS_FILE_NOT_EXIST";
		}
		if (STATUS_OPEN_FILE_FAILED == retStatus)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - InitFailoverConfiguration: STATUS_OPEN_FILE_FAILED";
		}

		m_pProcess->GetFailoverConfiguration()->WriteXmlFile(FAILOVER_CFG_FILE,"HOTBACKUP_DATA");
		/* Flora added for LED&LCD Feature */
		LedHotStandbyStateInd();
	}
	m_pFailoverCommunicationFromProcess->SetIp(m_pProcess->GetFailoverConfiguration()->GetOtherRmxIp());
	m_eMasterSlaveCurrentState=	ConvertHotBackupTypeToMasterSlaveState(m_pProcess->GetFailoverConfiguration()->GetHotBackupType());
	EndSetFailoverConfiguration();
}


/////////////////////////////////////////////////////////////////////
void CFailoverManager::EndSetFailoverConfiguration()
{
	if (m_pProcess->GetFailoverConfiguration()->GetIsEnabled() == TRUE)
	{
		eFailoverType eState = (eFailoverType)m_pProcess->GetFailoverConfiguration()->GetHotBackupType();
		TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - EndSetFailoverConfiguration - eState = "<< eState;
		DWORD opcode;
		if ((eState == eSlave) || (eState == eMaster))
		{
			opcode = (eState == eSlave) ? FAILOVER_START_SLAVE : FAILOVER_START_MASTER;

			CManagerApi confPartyMngrApi(eProcessConfParty);
			STATUS res  = confPartyMngrApi.SendMessageSync(NULL, opcode, 1 * SECOND);
			if (res != STATUS_OK)
				FPASSERT(opcode);

			CManagerApi resourceMngrApi(eProcessResource);
			res  = resourceMngrApi.SendMessageSync(NULL, opcode, 1 * SECOND);
			if (res != STATUS_OK)
				FPASSERT(opcode);
			
			bool isSlave = ( (eSlave == eState) ? true : false );
			SendFailoverParamsToProcesses(true, isSlave, "EndSetFailoverConfiguration");
		}
		
		/*Send Peer IP address to McuMngr VNGR-23142*/
		SendFailoverParamsToMcuMngr();
		
		ConnectToPairRMX();
	}
    // Notify GatekeeperProcess to start registration. (H323 Registration MUST after the failover configuration ends)
    SendFailoverRefreshRegToGatekeeper(TRUE);
}


/////////////////////////////////////////////////////////////////////
eMasterSlaveState CFailoverManager::ConvertHotBackupTypeToMasterSlaveState(DWORD HotBackupType)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - ConvertHotBackupTypeToMasterSlaveState - HotBackupType = "<< HotBackupType;
	if (m_pProcess->GetFailoverConfiguration()->GetIsEnabled()==FALSE)
		return eMasterSlaveNone;
	else if (HotBackupType==eMaster)
		return eMasterConfigurationState;
	else if (HotBackupType==eSlave)
		return eSlaveState;

	return eMasterSlaveNone;
}

//////////////////////////////////////////////////////////////////////
STATUS CFailoverManager::OnServerSetFailoverConfiguration(CRequest *pRequest)
{
    if (pRequest->GetAuthorization() != SUPER)
    {
        pRequest->SetStatus(STATUS_NO_PERMISSION);
        pRequest->SetConfirmObject(new CDummyEntry());
        FPTRACE(eLevelInfoNormal,"CFailoverManager::OnServerSetFailoverConfiguration: No permission to set Failover Configuration");
        return STATUS_NO_PERMISSION;
    }
	
	if (IsValidTimer(FAILOVER_RELOGIN_TIMER))
		DeleteTimer(FAILOVER_RELOGIN_TIMER);//we just in case delete the timer so that it wont interfere in the set operation

	PTRACE(eLevelError,  "CFailoverManager::OnServerSetFailoverConfiguration" );
	STATUS status = STATUS_OK;

	BOOL bIsChanged = FALSE;

	CFailoverConfiguration * pFailoverConfiguration = new CFailoverConfiguration();
	*pFailoverConfiguration = *(m_pProcess->GetFailoverConfiguration());

	string  oldOtherRmxIp = pFailoverConfiguration->GetOtherRmxIp();
	WORD    oldIsEnabled = pFailoverConfiguration->GetIsEnabled();
	DWORD  oldHotBackupType= pFailoverConfiguration->GetHotBackupType();

	*(pFailoverConfiguration) = *(CFailoverConfiguration*)pRequest->GetRequestObject() ;

	BOOL	newIsEnabled = pFailoverConfiguration->GetIsEnabled();
	string  newOtherRmxIp = pFailoverConfiguration->GetOtherRmxIp();
	DWORD  	newHotBackupType = pFailoverConfiguration->GetHotBackupType();
	BOOL	newManualTrigger = pFailoverConfiguration->GetManualTrigger();
		
	CManagerApi confPartyMngrApi(eProcessConfParty);
	CManagerApi resourceMngrApi(eProcessResource);
	STATUS res;

	/* if the manual trigger is enabled */
	if (newIsEnabled == TRUE && newHotBackupType == eMaster && newManualTrigger == TRUE)
	{
		PTRACE(eLevelInfoNormal,	"CFailoverManager::OnServerSetFailoverConfiguration: Manual trigger is enabled" );
		/*reply EMA first*/
		pRequest->SetConfirmObject(pFailoverConfiguration);
		std::string responseTrancsName("TRANS_HOTBACKUP");
		pRequest->SetTransName(responseTrancsName);
		pRequest->SetStatus(status);

		/*To info the McuManager for fast failour*/
		SendFailoverTriggerToMcuMngr();
		/*Sleep 3 seconds to make sure Slave will get the trigger indication*/
		SystemSleep(100 * 3);

		/*Send MCMSDeamon to reboot the system*/
		CMcmsDaemonApi api;
		std::string resetDes = "Hot backup was triggered manually";
		STATUS res = api.SendResetReq(resetDes);
		return STATUS_OK;
	}
	
	///if ip has changed
	if ((newIsEnabled == TRUE && oldIsEnabled == TRUE) && (newOtherRmxIp != oldOtherRmxIp))
	{
		PTRACE(eLevelError,  "CFailoverManager::OnServerSetFailoverConfiguration: ip has changed" );
		if ((newHotBackupType==eSlave) && (oldHotBackupType==eSlave))
		{
			PTRACE(eLevelError,  "CFailoverManager::OnServerSetFailoverConfiguration: pair ip has changed in Slave" );	
			bIsChanged = TRUE;

			res = confPartyMngrApi.SendOpcodeMsg(FAILOVER_RESTART_SLAVE);

			if (res != STATUS_OK)
				FPASSERT(FAILOVER_RESTART_SLAVE);

			res = resourceMngrApi.SendOpcodeMsg(FAILOVER_RESTART_SLAVE);
			if (res != STATUS_OK)
				FPASSERT(FAILOVER_RESTART_SLAVE);

			*(m_pProcess->GetFailoverConfiguration())=*pFailoverConfiguration;//proccess member is related to stop StopCommunicate ,SetIp and ConnectToPairRMX
			StopCommunicate("ipChanged");
			//m_pFailoverCommunicationFromProcess->SetIp(pFailoverConfiguration->GetOtherRmxIp());
			ConnectToPairRMX();
		}
		else if ((newHotBackupType==eMaster) && (oldHotBackupType==eMaster))
		{
			PTRACE(eLevelError,  "CFailoverManager::OnServerSetFailoverConfiguration: pair ip has changed in Master" );	
			bIsChanged = TRUE;
			*(m_pProcess->GetFailoverConfiguration())=*pFailoverConfiguration;
			
			StartMaster();

			if (IsValidTimer(FAILOVER_MASTER_PING_TIMER))
				DeleteTimer(FAILOVER_MASTER_PING_TIMER);

			StopMasterCheckOnGetStateFromSlave();
			ConnectToPairRMX();
		}
		else
			StopMasterCheckOnGetStateFromSlave();//related to old ip
	}


	else if ((newIsEnabled == FALSE && oldIsEnabled == TRUE && oldHotBackupType==eSlave) || /*case 1*/
 	 (newHotBackupType==eMaster && (oldHotBackupType==eSlave /* case 2*/||(newIsEnabled == TRUE && oldIsEnabled == FALSE) /* case 3*/)))
	{
		PTRACE(eLevelError,  "CFailoverManager::OnServerSetFailoverConfiguration: disabling slave or changing from slave to master or enabling master" );
		bIsChanged = TRUE;

		CSegment rspMsg;
		OPCODE resOpcode;

		STATUS responseStatus  = confPartyMngrApi.SendMessageSync(NULL, FAILOVER_START_MASTER, 1 * SECOND,resOpcode, rspMsg);

		if (responseStatus==STATUS_OK)
		{
			rspMsg >> status;
			STATUS responseStatus  = resourceMngrApi.SendMessageSync(NULL, FAILOVER_START_MASTER, 1 * SECOND,resOpcode, rspMsg);
			if (responseStatus==STATUS_OK)
			{
				if (status == STATUS_OK) //is status from conf party is ok - take from resource process
					rspMsg >> status;

				if (status != STATUS_OK) //add warning
					status |= WARNING_MASK;
				*(m_pProcess->GetFailoverConfiguration())=*pFailoverConfiguration;

				if (oldIsEnabled == TRUE) //need to stop slave functionality only if it had that before
					StartMaster();

				//only in case 3
				if (newHotBackupType == eMaster)// && newIsEnabled == TRUE && oldIsEnabled == FALSE)
					ConnectToPairRMX();
				
				SendFailoverParamsToProcesses(newIsEnabled, false, "OnServerSetFailoverConfiguration --- 1");
			}
		}
	}

	else if (newIsEnabled == FALSE && oldIsEnabled == TRUE && oldHotBackupType==eMaster)
	{
		PTRACE(eLevelError,  "CFailoverManager::OnServerSetFailoverConfiguration: disabling master" );
		bIsChanged = TRUE;
		StartMaster();

		if (IsValidTimer(FAILOVER_MASTER_PING_TIMER))
			DeleteTimer(FAILOVER_MASTER_PING_TIMER);

		StopMasterCheckOnGetStateFromSlave();

		SendFailoverParamsToProcesses(false, false, "OnServerSetFailoverConfiguration --- 2");
	}

	else if (newIsEnabled == TRUE  && newHotBackupType==eSlave && (oldIsEnabled == FALSE || oldHotBackupType==eMaster))
	{
		PTRACE(eLevelError,  "CFailoverManager::OnServerSetFailoverConfiguration: enabling slave or changing from master to slave" );
		bIsChanged = TRUE;

		CSegment rspMsg;
		OPCODE resOpcode;

		STATUS responseStatus  = confPartyMngrApi.SendMessageSync(NULL, FAILOVER_START_SLAVE, 1 * SECOND,resOpcode, rspMsg);

		if (responseStatus==STATUS_OK)
		{
			rspMsg >> status;
			if (status == STATUS_OK)
			{
				STATUS responseStatus  = resourceMngrApi.SendMessageSync(NULL, FAILOVER_START_SLAVE, 1 * SECOND,resOpcode, rspMsg);
				if (responseStatus == STATUS_OK)
				{
					rspMsg >> status;
					if (status == STATUS_OK)
					{
						*(m_pProcess->GetFailoverConfiguration())=*pFailoverConfiguration;
						//m_pFailoverCommunicationFromProcess->SetIp(pFailoverConfiguration->GetOtherRmxIp());
						ConnectToPairRMX();
						
						StopMasterCheckOnGetStateFromSlave();//related to old ip
						SendFailoverParamsToProcesses(true, true, "OnServerSetFailoverConfiguration --- 3");
					}
					else
					{
						PTRACE(eLevelError, "CFailoverManager::OnServerSetFailoverConfiguration:enabling slave or changing from master to slave not allowed because there are reservations" );
						responseStatus  = confPartyMngrApi.SendMessageSync(NULL, FAILOVER_START_MASTER, 1 * SECOND);
						if (responseStatus != STATUS_OK)
							FPASSERT(FAILOVER_START_MASTER);
						
						// no need to update processes - the Update operation was cancelled
						// SendFailoverParamsToProcesses(true, false);
					}
				}
			}
			else
			{
				PTRACE(eLevelError, "CFailoverManager::OnServerSetFailoverConfiguration:enabling slave or changing from master to slave not allowed because there are ongoing conferences" );
			}
		}
	}

	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnServerSetFailoverConfiguration - status = "<< status;

	if (status == STATUS_OK || (status & WARNING_MASK))
	{
		if (newIsEnabled == FALSE)
			RemoveFailoverActiveAlarms();


		if ((pFailoverConfiguration->GetIsEnabled() == false) && (bIsChanged == TRUE))
		{
			pFailoverConfiguration->SetMasterSlaveState(eFailoverStatusNone);
		}

		*(m_pProcess->GetFailoverConfiguration())=*pFailoverConfiguration;
		//updating current hotback type only in case there was a change from ema. This is done in order to avoid
		//changing m_eMasterSlaveCurrentState from masterActual to masterConfiguration in case there was no change from ema.
		if (bIsChanged == TRUE)
		{
			m_eMasterSlaveCurrentState=	ConvertHotBackupTypeToMasterSlaveState(newHotBackupType);
		}
		pFailoverConfiguration->WriteXmlFile(FAILOVER_CFG_FILE, "HOTBACKUP_DATA");

		/* Flora added for LED&LCD Feature */
		LedHotStandbyStateInd();
		
		TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnServerSetFailoverConfiguration after ConvertHotBackupTypeToMasterSlaveState - m_eMasterSlaveCurrentState = "<< GetMasterSlaveStateStr(m_eMasterSlaveCurrentState);
		pRequest->SetConfirmObject(pFailoverConfiguration);
	}
	else
		pRequest->SetConfirmObject(new CDummyEntry());

	std::string responseTrancsName("TRANS_HOTBACKUP");
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetStatus(status);
	//POBJDELETE(pFailoverConfiguration);
	SendFailoverParamsToMcuMngr();
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
void CFailoverManager::SendFailoverParamsToProcesses(bool isFeatureEnabled, bool isSlave, string theCaller)
{
	SendFailoverParamsToSpecProcess(eProcessConfParty,	isFeatureEnabled, isSlave, theCaller);
	SendFailoverParamsToSpecProcess(eProcessResource,	isFeatureEnabled, isSlave, theCaller);
	SendFailoverParamsToSpecProcess(eProcessMcuMngr,	isFeatureEnabled, isSlave, theCaller);
	SendFailoverParamsToSpecProcess(eProcessCSMngr,		isFeatureEnabled, isSlave, theCaller);
	SendFailoverParamsToSpecProcess(eProcessGatekeeper,	isFeatureEnabled, isSlave, theCaller);        
}

/////////////////////////////////////////////////////////////////////
void CFailoverManager::SendFailoverParamsToSpecProcess(eProcessType processType, bool isFeatureEnabled, bool isSlave, string theCaller)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::SendFailoverParamsToSpecProcess (caller: " << theCaller << ")"
                           << "\nStartSlave sent to "  << ::ProcessTypeToString(processType) << " process"
                           << "\nIsFeatureEnabled: " << (isFeatureEnabled	? "yes" : "no")
                           << "\nIsSlave:          " << (isSlave			? "yes" : "no");

	CSegment* pSeg = new CSegment();
	*pSeg << (WORD)isFeatureEnabled
		  << (WORD)isSlave;
	
	CManagerApi api(processType);
	api.SendMsg(pSeg, FAILOVER_SET_PARAMS_IND);
}

///////////////////////////////////////////////////////////////////////
void CFailoverManager::SendFailoverRefreshRegToGatekeeper(bool isSetEndFlag)
{
    TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::SendFailoverRefreshRegToGatekeeper " ;
        
    CSegment* pSeg = new CSegment();
    *pSeg << (WORD)isSetEndFlag;
	
	CManagerApi api(eProcessGatekeeper);
	api.SendMsg(pSeg, FAILOVER_REFRESH_GK_REG_IND);

}

/////////////////////////////////////////////////////////////////////
void CFailoverManager::SendFailoverParamsToMcuMngr()
{
	//added by huiyu. 2011.7.13
	//////////////////////////////////////////////////////////////////
	string pairIP = m_pProcess->GetFailoverConfiguration()->GetOtherRmxIp();
	CSegment*  seg = new CSegment;
	*seg << pairIP;
		
	CManagerApi mcuMngrApi(eProcessMcuMngr);
	STATUS res = mcuMngrApi.SendMsg(seg,FAILOVER_MCUMNGR_UPDATE_PAIR_IP_IND);
	if (res != STATUS_OK)
		FPASSERT(FAILOVER_MCUMNGR_UPDATE_PAIR_IP_IND);
}

/////////////////////////////////////////////////////////////////////
void CFailoverManager::SendFailoverTriggerToMcuMngr()
{
	CManagerApi mcuMngrApi(eProcessMcuMngr);
	STATUS res = mcuMngrApi.SendOpcodeMsg(FAILOVER_MCUMNGR_EVENT_TRIGGER_IND);
	if (res != STATUS_OK)
		FPASSERT(FAILOVER_MCUMNGR_EVENT_TRIGGER_IND);
}


/////////////////////////////////////////////////////////////////////
void CFailoverManager::SendStartMasterBecomeSlaveToConfPartyAndReservation()
{//at this phase no ongoing conferences should be on the RMX. But reservation can be, therefore RA process will delete them.
 //So the answer for the sync message should always be OK.

	CManagerApi resourceMngrApi(eProcessResource);
	resourceMngrApi.SendOpcodeMsg(FAILOVER_START_MASTER_BECOME_SLAVE);

	CManagerApi confPartyMngrApi(eProcessConfParty);
	confPartyMngrApi.SendOpcodeMsg(FAILOVER_START_MASTER_BECOME_SLAVE);

    //Need to notify Gatekeeper process to unregister
	CManagerApi gkmgrApi(eProcessGatekeeper);
	gkmgrApi.SendOpcodeMsg(FAILOVER_START_MASTER_BECOME_SLAVE);    
}

/////////////////////////////////////////////////////////////////////
void CFailoverManager::RemoveFailoverActiveAlarms()
{
	RemoveMasterSlaveConfigurationActiveAlarms();
	RemoveMasterNetworkIssueActiveAlarm();
	if (IsActiveAlarmExistByErrorCode(AA_HOT_BACKUP_PAIR_MCU_IS_UNREACHABLE))
		RemoveActiveAlarmByErrorCode(AA_HOT_BACKUP_PAIR_MCU_IS_UNREACHABLE);
}

/////////////////////////////////////////////////////////////////////
void CFailoverManager::RemoveMasterSlaveConfigurationActiveAlarms()
{
	if (IsActiveAlarmExistByErrorCode(AA_HOT_BACKUP_MASTER_SLAVE_CONFIGURATION_CONFLICT))
		RemoveActiveAlarmByErrorCode(AA_HOT_BACKUP_MASTER_SLAVE_CONFIGURATION_CONFLICT);
}

/////////////////////////////////////////////////////////////////////
void CFailoverManager::RemoveMasterNetworkIssueActiveAlarm()
{
	if (IsActiveAlarmExistByErrorCode(AA_HOT_BACKUP_NETWORK_ISSUE))
		RemoveActiveAlarmByErrorCode(AA_HOT_BACKUP_NETWORK_ISSUE);
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::GetInfoForCommunication()
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::GetInfoForCommunication";
	m_state = INIT_CONFIGURATION;

	CManagerApi mcuMngrApi(eProcessMcuMngr);
	STATUS res = mcuMngrApi.SendOpcodeMsg(FAILOVER_MCUMNGR_CONFIG_REQ);
	if (res != STATUS_OK)
		FPASSERT(FAILOVER_MCUMNGR_CONFIG_REQ);

	CManagerApi authenticationApi(eProcessAuthentication);
	res = authenticationApi.SendOpcodeMsg(FAILOVER_AUTHENTICATION_CONFIG_REQ);
	if (res != STATUS_OK)
		FPASSERT(FAILOVER_AUTHENTICATION_CONFIG_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::OnMcuMngrConfigIndInitConfiguration(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnMcuMngrConfigIndInitConfiguration";
	BYTE bIsSecured;
	*pMsg >> bIsSecured;

	DWORD defaultGwIp;
	*pMsg >> defaultGwIp;

	m_pMngmntInfoFromProcess->defaultGwIpAddress = defaultGwIp;

	m_pFailoverCommunicationFromProcess->SetSecured(bIsSecured);
	if (m_pFailoverCommunicationFromProcess->IsConfigurationReady())
		ConnectToPairRMX();
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::SendActiveAlarmDeterminationConflict(string strDescription)
{

	if(! IsActiveAlarmExistByErrorCode(AA_HOT_BACKUP_MASTER_SLAVE_CONFIGURATION_CONFLICT))
	{
		AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
						AA_HOT_BACKUP_MASTER_SLAVE_CONFIGURATION_CONFLICT,
						MAJOR_ERROR_LEVEL,
						strDescription.c_str(),
						true,
						true );
	}
	else
		UpdateActiveAlarmDescriptionByErrorCode(AA_HOT_BACKUP_MASTER_SLAVE_CONFIGURATION_CONFLICT, strDescription);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::HandleMasterSlaveDeterminationLoginResponse(eMasterSlaveState masterSlaveStateResponse, CLogInConfirm *pLogInConfirm)
{
	bool isConflictOccured=false;
	std::string strAlarm;
	VERSION_S pair_version = pLogInConfirm->GetMCU_Version();
	char ipStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString((DWORD)(m_pMngmntInfoFromProcess->mngmntIpAddress),ipStr);
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - HandleMasterSlaveDeterminationLoginResponse"
							   << "\nMcuVer: " << (DWORD)m_pMngmntInfoFromProcess->mcuVer.ver_major   << "." << (DWORD)m_pMngmntInfoFromProcess->mcuVer.ver_minor << "."
											   << (DWORD)m_pMngmntInfoFromProcess->mcuVer.ver_release << "." << (DWORD)m_pMngmntInfoFromProcess->mcuVer.ver_internal
											   << "\nMngmnt IP address: " << ipStr
											   << "\nMy current state: " << GetMasterSlaveStateStr(m_eMasterSlaveCurrentState);


	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - HandleMasterSlaveDeterminationLoginResponse"
								   << "\nOTHER McuVer: " << (DWORD)pair_version.ver_major   << "." << (DWORD)pair_version.ver_minor << "."
												   << (DWORD)pair_version.ver_release << "." << (DWORD)pair_version.ver_internal
												   << "\nPeer state: " << GetMasterSlaveStateStr(masterSlaveStateResponse)
												   << "\n";
	if (IsValidTimer(FAILOVER_RELOGIN_TIMER))
		DeleteTimer(FAILOVER_RELOGIN_TIMER);

	if (pair_version.ver_major != m_pMngmntInfoFromProcess->mcuVer.ver_major || pair_version.ver_minor != m_pMngmntInfoFromProcess->mcuVer.ver_minor || pair_version.ver_release != m_pMngmntInfoFromProcess->mcuVer.ver_release)
	{
		strAlarm = "Hot Backup can only be enabled if the Master and Slave MCUs have the same version number.";
		SendActiveAlarmDeterminationConflict(strAlarm);
		isConflictOccured=true;
		if (m_eMasterSlaveCurrentState == eSlaveState)
		{
			PTRACE(eLevelError,  "CFailoverManager::HandleMasterSlaveDeterminationLoginResponse:i am slave version conflict" );
			m_state = RELOGIN_AFTER_LOGOUT;
			SendLogoutMessage();
		}
		else
		{
			PTRACE(eLevelError,  "CFailoverManager::HandleMasterSlaveDeterminationLoginResponse:i am not slave version conflict" );
			m_state = LOGOUT_RESPONSE;
			SendLogoutMessage();
		}
	}

	else if (m_eMasterSlaveCurrentState != eMasterSlaveNone && masterSlaveStateResponse==eMasterSlaveNone)
	{
		strAlarm = "Master slave configuration conflict. Peer MCU doesn't enable hotbackup";
		SendActiveAlarmDeterminationConflict(strAlarm);
		isConflictOccured=true;
		if (m_eMasterSlaveCurrentState == eSlaveState)
		{
			PTRACE(eLevelError,  "CFailoverManager::HandleMasterSlaveDeterminationLoginResponse:i am slave and pair is none" );
			m_state = RELOGIN_AFTER_LOGOUT;
			SendLogoutMessage();
		}
		else
		{
			PTRACE(eLevelError,  "CFailoverManager::HandleMasterSlaveDeterminationLoginResponse:i am not slave and pair is none" );
			m_state = LOGOUT_RESPONSE;
			SendLogoutMessage();
		}
	}

	else if (m_eMasterSlaveCurrentState==eSlaveState)
	{
		if (masterSlaveStateResponse==eSlaveState)
		{
			PTRACE(eLevelError,  "CFailoverManager::HandleMasterSlaveDeterminationLoginResponse:Both RMX's are configured as slave" );
			strAlarm = "Both RMX's are configured as slave";
			SendActiveAlarmDeterminationConflict(strAlarm);
			m_state = RELOGIN_AFTER_LOGOUT;
			SendLogoutMessage();
			isConflictOccured=true;
		}
		else if (masterSlaveStateResponse == eMasterConfigurationState || masterSlaveStateResponse == eMasterActualState)
		{
			PTRACE(eLevelError,  "CFailoverManager::HandleMasterSlaveDeterminationLoginResponse:i am slave configuration and pair is a master-->start slave" );
			StartSlave();
		}
	}

	else if (m_eMasterSlaveCurrentState == eMasterConfigurationState)
	{
		if (masterSlaveStateResponse == eSlaveState)
		{
			PTRACE(eLevelError,  "CFailoverManager::HandleMasterSlaveDeterminationLoginResponse:i am master configuration and pair is a slave-->start master" );
			m_eMasterSlaveCurrentState = eMasterActualState;
			StartMaster();
		}

		else if (masterSlaveStateResponse == eMasterConfigurationState)
		{
			PTRACE(eLevelError,  "CFailoverManager::HandleMasterSlaveDeterminationLoginResponse:Both RMX's are configured as master" );
			strAlarm = "Both RMX's are configured as master";
			SendActiveAlarmDeterminationConflict(strAlarm);

			StopMasterCheckOnGetStateFromSlave();

			isConflictOccured=true;

			m_state = LOGOUT_RESPONSE;
			SendLogoutMessage();
		}

		else if (masterSlaveStateResponse == eMasterActualState)
		{
			PTRACE(eLevelError,  "CFailoverManager::HandleMasterSlaveDeterminationLoginResponse:i am master configuration and pair is a master actual-->start slave" );
			m_eMasterSlaveCurrentState = eSlaveState;
			m_state = SLAVE_READY;
			m_pProcess->GetFailoverConfiguration()->SetHotBackupType(eSlave);
			m_pProcess->GetFailoverConfiguration()->WriteXmlFile(FAILOVER_CFG_FILE, "HOTBACKUP_DATA");
			UpdateMasterSlaveState(eAttempting);

			/* Flora added for LED&LCD Feature */
			LedHotStandbyStateInd();
			
			SendFailoverParamsToProcesses(true, true, "OnConfPartyEndPrepareMasterBecomeSlaveSlaveReady");
			SendStartMasterBecomeSlaveToConfPartyAndReservation();
		}
	}

	if (!isConflictOccured)
		RemoveMasterSlaveConfigurationActiveAlarms();
	else
		UpdateMasterSlaveState(eFail);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::OnConfPartyEndPrepareMasterBecomeSlaveSlaveReady(CSegment* pMsg)
{
	DWORD status;
	*pMsg >> status;

	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnConfPartyEndPrepareMasterBecomeSlaveSlaveReady - status = " << status;

	if (status != STATUS_OK)
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnConfPartyEndPrepareMasterBecomeSlaveSlaveReady - FAILUE: RESETTING RMX";
		FPASSERT(FAILOVER_CONFPARTY_END_PREPARE_MASTER_BECOME_SLAVE);
		CMcmsDaemonApi api;
		STATUS res = api.SendResetReq("Hot backup master was reboot in order to become a slave");
		return;
	}

	CManagerApi resourceMngrApi(eProcessResource);
	STATUS res = resourceMngrApi.SendMessageSync(NULL, FAILOVER_START_SLAVE, 1 * SECOND);
	if (res != STATUS_OK)
	{
		FPASSERT(FAILOVER_START_SLAVE);
		SendActiveAlarmDeterminationConflict("Master slave configuration conflict");
		UpdateMasterSlaveState(eFail);
	}
	else
	{
		StartSlave();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::OnConfPartyEndPrepareMasterBecomeSlaveAnycase(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnConfPartyEndPrepareMasterBecomeSlaveAnycase -  DO NOTHING!!";
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::OnAuthenticationConfigIndInitConfiguration(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnAuthenticationConfigIndInitConfiguration";
	DWORD userLen;
	*pMsg >> userLen;

	ALLOCBUFFER(userStr, userLen+1);
	userStr[userLen] = '\0';
	*pMsg >> userStr;

	DWORD passwordLen;
	*pMsg >> passwordLen;

	ALLOCBUFFER(passwordStr, passwordLen+1);
	passwordStr[passwordLen] = '\0';
	*pMsg >> passwordStr;

	m_pFailoverCommunicationFromProcess->SetUserAndPassword(userStr, passwordStr);
	if (m_pFailoverCommunicationFromProcess->IsConfigurationReady())
		ConnectToPairRMX();

	DEALLOCBUFFER(userStr);
	DEALLOCBUFFER(passwordStr);
}

/////////////////////////////////////////////////////////////////////////////
// (2) Init connection between master and slave
void CFailoverManager::ConnectToPairRMX()
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - ConnectToPairRMX";
	m_pFailoverCommunicationFromProcess->SetIp(m_pProcess->GetFailoverConfiguration()->GetOtherRmxIp());

	if (m_pProcess->GetFailoverConfiguration()->GetIsEnabled() == FALSE)
	{
		m_state = IDLE;
		return;
	}

	if (m_pFailoverCommunicationFromProcess->IsConfigurationReady() == FALSE) //if no configuration was asked before
	{
		GetInfoForCommunication();
		return;
	}

	else
	{
		m_state = CONNECTING_SOCKET;
		m_pFailoverCommunicationFromProcess->InitSocket(this);
		m_pFailoverCommunicationFromProcess->ConnectSocket();
	}

	//////update the configuration
	UpdateMasterSlaveState(eAttempting);
}


/////////////////////////////////////////////////////////////////////////////
// Socket: connection failed
void CFailoverManager::OnSocketFailedConnectingSocket(CSegment* pMsg)
{
	FTRACESTR(eLevelError) << "CFailoverManager::OnSocketFailedConnectingSocket - can't establish connection with the other RMX - ";

	std::string strMsg = "Failed to establish connection to ";
	SendUnreachableUrlAlarm(strMsg);
	
	// update state
	UpdateMasterSlaveState(eFail);

	// endless loop in case of slave or master with network problem
	BOOL bStarReconnectSocketTimer = IsNeedToReconnectSocket();
	if (bStarReconnectSocketTimer == TRUE)
		StartTimer(FAILOVER_RECONNECT_SOCKET_TIMER, FAILOVER_RECONNECT_SOCKET_TIMEOUT);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CFailoverManager::IsNeedToReconnectSocket()
{//only in case of slave or master with network problem
	BOOL bStarReconnectSocketTimer = FALSE;
	if (m_pProcess->GetFailoverConfiguration()->GetIsEnabled() == TRUE)
	{
		eFailoverType eState = (eFailoverType)m_pProcess->GetFailoverConfiguration()->GetHotBackupType();
		if (eState == eSlave)
			bStarReconnectSocketTimer = TRUE;
		else // master
		{
			if (m_bIsNetworkProblemsInMaster == TRUE)
				bStarReconnectSocketTimer = TRUE;
		}
	}
	return bStarReconnectSocketTimer;
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::OnTimerReconnectSocketTimeout(CSegment* pParam)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::OnTimerReconnectSocketTimeout";
	if (CPObject::IsValidPObjectPtr(m_pFailoverCommunicationFromProcess))
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::OnTimerReconnectSocketTimeout - Try to connect socket";
		m_pFailoverCommunicationFromProcess->ConnectOrReconnectSocket();
	}

	BOOL bStarReconnectSocketTimer = IsNeedToReconnectSocket();
	if (bStarReconnectSocketTimer == TRUE)
		StartTimer(FAILOVER_RECONNECT_SOCKET_TIMER, FAILOVER_RECONNECT_SOCKET_TIMEOUT);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::OnTimerReloginTimeout(CSegment* pParam)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::OnTimerReloginTimeout";
	m_state = MATER_SLAVE_DETERMINATION;
	SendLoginMessage();
}


////////////////////////////////////////////////////////////////////////////
void CFailoverManager::OnSocketRcvIndlogout(CSegment* pMsg)
{
	DWORD len = 0;
	
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketRcvIndlogout";
	*pMsg >> len;
	
	ALLOCBUFFER(pXMLString, len+1);
	*pMsg >> pXMLString;
	pXMLString[len] = '\0';

	if (strstr(pXMLString, "RESPONSE_TRANS_MCU") != NULL)
	{
		if (CXmlMiniParser::GetResponseStatus(pXMLString) == STATUS_OK)
		{
			if (m_state==RELOGIN_AFTER_LOGOUT)
			{
				TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketRcvIndlogout - logout response with status ok- send login again";
				StartTimer(FAILOVER_RELOGIN_TIMER, FAILOVER_RELOGIN_TIMEOUT);
			}
			else if (m_state == LOGOUT_RESPONSE)
			{
				m_state = IDLE;
				StopCommunicate("MasterSlaveDeterminationConflict");
			}
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketRcvIndlogout - logout response with status NOT ok!!!";
		}
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketRcvIndlogout - NOT login response";
	}

	DEALLOCBUFFER(pXMLString);
		
	return;
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::SendUnreachableUrlAlarm(string strMsg)
{
	RemoveMasterSlaveConfigurationActiveAlarms();

	std::string url = "";
	if (CPObject::IsValidPObjectPtr(m_pFailoverCommunicationFromProcess))
		url += m_pFailoverCommunicationFromProcess->GetStrUrl();

	std::string str = strMsg + url;

	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - SendUnreachableUrlAlarm - url" << url;

	if(! IsActiveAlarmExistByErrorCode(AA_HOT_BACKUP_PAIR_MCU_IS_UNREACHABLE))
	{
		AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
					AA_HOT_BACKUP_PAIR_MCU_IS_UNREACHABLE,
					MAJOR_ERROR_LEVEL,
					str.c_str(),
					true,
					true);
	}
	else
		UpdateActiveAlarmDescriptionByErrorCode(AA_HOT_BACKUP_PAIR_MCU_IS_UNREACHABLE, str.c_str());
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::OnSocketDroppedConnectingSocket(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketDroppedConnectingSocket";
}

/////////////////////////////////////////////////////////////////////////////
//In this case it's OK - after changing from slave to master
void CFailoverManager::OnSocketDroppedIdle(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketDroppedIdle";
}

/////////////////////////////////////////////////////////////////////////////
//In this case it's OK
void CFailoverManager::OnSocketDroppedSlaveRestoreMaster(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketDroppedSlaveRestoreMaster";
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::OnSocketDroppedMasterSlaveDetermination(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketDroppeddMasterSlaveDetermination";
	std::string strMsg = "Failed to establish connection to ";
	SendUnreachableUrlAlarm(strMsg);
	SocketDropped(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::OnSocketDroppedSlaveReady(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketDroppedSlaveReady";
	SocketDropped(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::OnSocketDroppedReloginAfterLogout(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketDroppedReloginAfterLogout";
	SocketDropped(pMsg);
}


////////////////////////////////////////////////////////////////////////////
void CFailoverManager::SocketDropped(CSegment* pMsg)
{
	m_state = CONNECTING_SOCKET;

	if (CPObject::IsValidPObjectPtr(m_pFailoverCommunicationFromProcess))
		m_pFailoverCommunicationFromProcess->SetSocketDropped();

	StartTimer(FAILOVER_RECONNECT_SOCKET_TIMER, FAILOVER_RECONNECT_SOCKET_TIMEOUT);
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::OnSocketConnectedConnectingSocket(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketConnectedConnectingSocket";

	if(IsActiveAlarmExistByErrorCode(AA_HOT_BACKUP_PAIR_MCU_IS_UNREACHABLE))
		RemoveActiveAlarmByErrorCode(AA_HOT_BACKUP_PAIR_MCU_IS_UNREACHABLE);

	if (IsValidTimer(FAILOVER_RECONNECT_SOCKET_TIMER))
		DeleteTimer(FAILOVER_RECONNECT_SOCKET_TIMER);

	// (3) Master/slave determination
	StartMasterSlaveDetermination();
}


//////////////////////////////////////////////////////////////////////
void CFailoverManager::StartMasterSlaveDetermination()
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - StartMasterSlaveDetermination";
	m_state = MATER_SLAVE_DETERMINATION;
	SendLoginMessage();
}


//////////////////////////////////////////////////////////////////////
void CFailoverManager::SendLogoutMessage()
{
	//TODO: move to a different location
	DWORD mcuToken	= m_pProcess->GetMcuToken();
	DWORD msgId		= m_pProcess->GetAndIncreaseMsgId();

	CXMLDOMElement* pRootNode =  NULL;
	pRootNode = new CXMLDOMElement("TRANS_MCU");

	CXMLDOMElement* pTempNode;
	pTempNode = pRootNode->AddChildNode("TRANS_COMMON_PARAMS");
	pTempNode->AddChildNode("MCU_TOKEN", mcuToken);
	pTempNode->AddChildNode("MCU_USER_TOKEN", mcuToken);
	pTempNode->AddChildNode("MESSAGE_ID", msgId);

	pTempNode = pRootNode->AddChildNode("ACTION");
	pTempNode = pTempNode->AddChildNode("LOGOUT");

	char *pStrReq=NULL;
	pRootNode->DumpDataAsLongStringEx(&pStrReq);

	if (pStrReq)
	{
		m_pFailoverCommunicationFromProcess->SendToSocket(pStrReq);
		delete []pStrReq;
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::SendLogoutMessage - pStrReq = NULL";
	}

	PDELETE(pRootNode);
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::SendLoginMessage()
{
	//TODO: move to a different location
	CXMLDOMElement* pRootNode =  NULL;
	pRootNode = new CXMLDOMElement("TRANS_MCU");

	CXMLDOMElement* pTempNode;
	pTempNode = pRootNode->AddChildNode("TRANS_COMMON_PARAMS");
	pTempNode->AddChildNode("MCU_TOKEN", -1);
	pTempNode->AddChildNode("MCU_USER_TOKEN", 0);
	pTempNode->AddChildNode("MESSAGE_ID", 0);

	pTempNode = pRootNode->AddChildNode("ACTION");
	pTempNode = pTempNode->AddChildNode("LOGIN");

	CXMLDOMElement* pMcuIpNode;
	pMcuIpNode = pTempNode->AddChildNode("MCU_IP");

	std::string ip = m_pProcess->GetFailoverConfiguration()->GetOtherRmxIp();
	WORD port = m_pProcess->GetFailoverConfiguration()->GetOtherRmxPort();

	char ipStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString((DWORD)(m_pMngmntInfoFromProcess->mngmntIpAddress),ipStr);

	pMcuIpNode->AddChildNode("IP", ip.c_str());
	pMcuIpNode->AddChildNode("LISTEN_PORT", port);

	pTempNode->AddChildNode("USER_NAME", m_pFailoverCommunicationFromProcess->GetStrUser());
	pTempNode->AddChildNode("PASSWORD", m_pFailoverCommunicationFromProcess->GetStrPassword());
	pTempNode->AddChildNode("STATION_NAME", "SLAVE_RMX");
	pTempNode->AddChildNode("COMPRESSION", TRUE, _BOOL);
	pTempNode->AddChildNode("CONFERENCE_RECORDER", FALSE, _BOOL);
	pTempNode->AddChildNode("HOTBACKUP_ACTUAL_TYPE", m_eMasterSlaveCurrentState,FAILOVER_MASTER_SLAVE_STATE);
	pTempNode->AddChildNode("CLIENT_IP", ipStr);

	char *pStrReq=NULL;
	pRootNode->DumpDataAsLongStringEx(&pStrReq);

	if (pStrReq)
	{
		m_pFailoverCommunicationFromProcess->SendToSocket(pStrReq);
		delete []pStrReq;
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::SendLoginMessage - pStrReq = NULL";
	}

	PDELETE(pRootNode);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::OnSocketRcvIndIdle(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketRcvIndIdle";
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::OnSocketRcvIndAnycase(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketRcvIndAnycase";
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::OnSocketRcvIndMasterSlaveDetermination(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketRcvIndMasterSlaveDetermination";

	DWORD len;
	*pMsg >> len;

	ALLOCBUFFER(pXMLString, len+1);
	*pMsg >> pXMLString;
	pXMLString[len] = '\0';

	if (strstr(pXMLString, "RESPONSE_TRANS_MCU") != NULL)
	{
		if (CXmlMiniParser::GetResponseStatus(pXMLString) == STATUS_OK)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketRcvIndMasterSlaveDetermination - login response with status ok";
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketRcvIndMasterSlaveDetermination - login response with status NOT ok!!!";
			StartTimer(FAILOVER_RELOGIN_TIMER, FAILOVER_RELOGIN_TIMEOUT);
			DEALLOCBUFFER(pXMLString);
			
			return;
		}
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnSocketRcvIndMasterSlaveDetermination - NOT login response";
		DEALLOCBUFFER(pXMLString);
		
		return;
	}


	//compare response info to configuration

    CXMLDOMDocument xmlDoc;
    xmlDoc.Parse((const char**)&pXMLString);

    CXMLDOMElement *pLoginResponseNode = xmlDoc.GetRootElement();
	CXMLDOMElement *pActionNode = NULL;
	pLoginResponseNode->getChildNodeByName(&pActionNode,"ACTION");
	if (pActionNode)
	{
		pActionNode->getChildNodeByName(&pActionNode,"LOGIN");
	}
	if (pActionNode)
	{
                // TODO drabkin add multiple service flag instead TRUE
		CLogInConfirm *pLogInConfirm = new CLogInConfirm(TRUE, TRUE);
		char szErrorMsg[ERROR_MESSAGE_LEN];
		pLogInConfirm->DeSerializeXml(pActionNode, szErrorMsg);
		DWORD mcuToken = pLogInConfirm->GetMcuToken();
		m_pProcess->SetMcuToken(mcuToken);

		eMasterSlaveState masterSlaveStateResponse=(eMasterSlaveState)(pLogInConfirm->GetMasterSlaveState());
		HandleMasterSlaveDeterminationLoginResponse(masterSlaveStateResponse,pLogInConfirm);

		/*
		for debug purposes only - comment the prev. line and open the following lines
		m_eMasterSlaveCurrentState = eSlaveState;
		StartSlave();
		*/

		POBJDELETE(pLogInConfirm);
	}

	DEALLOCBUFFER(pXMLString);
}


//////////////////////////////////////////////////////////////////////
void CFailoverManager::StartMaster()
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - StartMaster";
	m_state = IDLE;

	StopCommunicate("StartMaster");

	StartMasterTimersIfNeeded(); //real master
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::StartMasterTimersIfNeeded()
{
	if (m_pProcess->GetFailoverConfiguration()->GetIsEnabled() == TRUE)
	{
		eFailoverType eState = (eFailoverType)m_pProcess->GetFailoverConfiguration()->GetHotBackupType();
		if (eState == eMaster)
		{
			StartTimer(FAILOVER_MASTER_PING_TIMER, FAILOVER_MASTER_PING_TIMEOUT);
			StartTimer(FAILOVER_MASTER_GET_STATE_FROM_SLAVE_TIMER, FAILOVER_MASTER_GET_STATE_FROM_SLAVE_TIMEOUT);
		}
	}
}

/////////////////////////////////////////////////////////////////////
void CFailoverManager::StopMasterCheckOnGetStateFromSlave()
{
	if (IsValidTimer(FAILOVER_MASTER_GET_STATE_FROM_SLAVE_TIMER))
		DeleteTimer(FAILOVER_MASTER_GET_STATE_FROM_SLAVE_TIMER);

	RemoveMasterNetworkIssueActiveAlarm(); //if exists
}

/////////////////////////////////////////////////////////////////////
void CFailoverManager::MasterGetStateFromSlaveError()
{
	//1. AA
	if(! IsActiveAlarmExistByErrorCode(AA_HOT_BACKUP_NETWORK_ISSUE))
	{
		AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
						AA_HOT_BACKUP_NETWORK_ISSUE,
						MAJOR_ERROR_LEVEL,
						"Expected keep alive from slave RMX has not received",
						true,
						true );
	}

	//2. Sync status
	UpdateMasterSlaveState(eFail);
}

/////////////////////////////////////////////////////////////////////
void CFailoverManager::MasterGetStateFromSlaveRemoveError()
{
	//1. AA
	RemoveMasterNetworkIssueActiveAlarm();
	UpdateMasterSlaveState(eSynchOk);
}

/////////////////////////////////////////////////////////////////////
void CFailoverManager::OnMcuMngrGetStateExIdle(CSegment* pSeg)
{
	bool bContinue = false;
	if (m_pProcess->GetFailoverConfiguration()->GetIsEnabled() == TRUE)
	{
		eFailoverType eState = (eFailoverType)m_pProcess->GetFailoverConfiguration()->GetHotBackupType();
		if (eState == eMaster)
			bContinue = true;
	}
	if (bContinue == false)
		return;

	//check IPs. If ok - restart timer
	DWORD len;
	*pSeg >> len;

	ALLOCBUFFER(ipStr,len+1);
	ipStr[len] = '\0';
	*pSeg >> ipStr;
	std::string slaveRmxIp = m_pProcess->GetFailoverConfiguration()->GetOtherRmxIp();

	if ( (ipStr == slaveRmxIp) || (IsTarget() == false) )
	{
		if (IsValidTimer(FAILOVER_MASTER_GET_STATE_FROM_SLAVE_TIMER))
			DeleteTimer(FAILOVER_MASTER_GET_STATE_FROM_SLAVE_TIMER);
		StartTimer(FAILOVER_MASTER_GET_STATE_FROM_SLAVE_TIMER, FAILOVER_MASTER_GET_STATE_FROM_SLAVE_TIMEOUT);
		MasterGetStateFromSlaveRemoveError();
	}

	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnMcuMngrGetStateExIdle - ip doesn't match: ip from get state = " << ipStr << ", my slave rmx ip = " <<slaveRmxIp;
	}

	DEALLOCBUFFER(ipStr);
}

/////////////////////////////////////////////////////////////////////
void CFailoverManager::OnMcuMngrGetStateExAnycase(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnMcuMngrGetStateExAnycase - Do nothing!!!";
}

/////////////////////////////////////////////////////////////////////
void CFailoverManager::OnTimerMasterGetStateFromSlaveTimeoutIdle(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnTimerMasterGetStateFromSlaveTimeoutIdle";
	MasterGetStateFromSlaveError();
}

/////////////////////////////////////////////////////////////////////
void CFailoverManager::OnTimerMasterGetStateFromSlaveTimeoutAnycase(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnTimerMasterGetStateFromSlaveTimeoutAnycase - Do nothing!!!";
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::StartSlave()
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - StartSlave";

	m_state = SLAVE_READY;

	//1. Start Device failure detecting task
	StartFailDetection();

	//2. Start System synchronization tasks
	StartFailoverSync();

	//3. Stop master timers
	m_bIsNetworkProblemsInMaster = FALSE;
	if (IsValidTimer(FAILOVER_MASTER_PING_TIMER))
		DeleteTimer(FAILOVER_MASTER_PING_TIMER);

	StopMasterCheckOnGetStateFromSlave();
}


//////////////////////////////////////////////////////////////////////
void CFailoverManager::StartFailDetection()
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - StartFailDetection";
	if (m_pFailDetectionApi != NULL)
	{//means that we got to this phase after the socket disconnected and reconnected again.
		m_pFailDetectionApi->ResumeSlaveTask();
		return;
	}

	m_pFailDetectionApi = new CFailDetectionApi;
	m_pFailDetectionApi->Create(FailDetectionEntryPoint, *m_pRcvMbx);

	CFailDetectionTask *pFdTask	= (CFailDetectionTask*)m_pFailDetectionApi->GetTaskAppPtr();
	COsQueue& failDetectionMbx	= pFdTask->GetRcvMbx();
	m_pProcess->SetFailDetectionTaskMbx(&failDetectionMbx);
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::StartFailoverSync()
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - StartFailoverSync";
	if (m_pFailoverSyncApi != NULL)
	{//means that we got to this phase after the socket disconnected and reconnected again.
		UpdateMasterSlaveState(eSynchOk);
		return;
	}

	m_pFailoverSyncApi = new CFailoverSyncApi;
	m_pFailoverSyncApi->Create(FailoverSyncEntryPoint, *m_pRcvMbx);

	CFailoverSyncTask *pFsTask	= (CFailoverSyncTask*)m_pFailoverSyncApi->GetTaskAppPtr();
	COsQueue& failoverSyncMbx	= pFsTask->GetRcvMbx();
	m_pProcess->SetFailoverSyncTaskMbx(&failoverSyncMbx);
	
	// update state
	UpdateMasterSlaveState(eAttempting);
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::OnMasterDownInd(CSegment* pSeg)
{
	//Check ping to default GW. If OK -> become master.
	//							Else -> Restart slave failure detection task when the socket is reconnected.
	BOOL bIsNetworkOk = IsNetworkOk();
	if (bIsNetworkOk == FALSE)
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnMasterDownInd - Network problems";
		return;
	}

	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnMasterDownInd - Become MASTER";

	m_state = SLAVE_RESTORE_MASTER;

	//////update the configuration
	m_pProcess->GetFailoverConfiguration()->SetHotBackupType(eMaster);
	m_pProcess->GetFailoverConfiguration()->WriteXmlFile( FAILOVER_CFG_FILE, "HOTBACKUP_DATA");

	/* Flora added for LED&LCD Feature */
	LedHotStandbyStateInd();
	//updating the current member
	m_eMasterSlaveCurrentState = eMasterActualState;
	
	SendFailoverParamsToProcesses(true, false, "OnMasterDownInd");
	CManagerApi confPartyMngrApi(eProcessConfParty);
	STATUS res = confPartyMngrApi.SendOpcodeMsg(FAILOVER_SLAVE_BECOME_MASTER);
	if (res != STATUS_OK)
		FPASSERT(FAILOVER_SLAVE_BECOME_MASTER);

	CManagerApi resourceMngrApi(eProcessResource);
	res = resourceMngrApi.SendOpcodeMsg(FAILOVER_SLAVE_BECOME_MASTER);
	if (res != STATUS_OK)
		FPASSERT(FAILOVER_SLAVE_BECOME_MASTER);
	
	CManagerApi mcuMngrApi(eProcessMcuMngr);
	res = mcuMngrApi.SendOpcodeMsg(FAILOVER_SLAVE_BECOME_MASTER);
	if (res != STATUS_OK)
		FPASSERT(FAILOVER_SLAVE_BECOME_MASTER);

	CManagerApi gkmgrApi(eProcessGatekeeper);
	res = gkmgrApi.SendOpcodeMsg(FAILOVER_SLAVE_BECOME_MASTER);
	if (res != STATUS_OK)
		FPASSERT(FAILOVER_SLAVE_BECOME_MASTER);

	StopCommunicate("OnMasterDownInd");

	std::string strMsg = "Failed to establish connection to ";
	SendUnreachableUrlAlarm(strMsg);
	/*update the status as failed since master is down*/
	UpdateMasterSlaveState(eFail);
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::OnWholeSyncCompletedInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnWholeSyncCompletedInd";
	
	// update state
	UpdateMasterSlaveState(eSynchOk);
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::StopCommunicate(const string theCaller)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::StopCommunicate";
	if (IsValidTimer(FAILOVER_RECONNECT_SOCKET_TIMER))
		DeleteTimer(FAILOVER_RECONNECT_SOCKET_TIMER);

	if (CPObject::IsValidPObjectPtr(m_pFailDetectionApi))
	{
		m_pFailDetectionApi->Destroy();
		POBJDELETE(m_pFailDetectionApi);
	}

	if (CPObject::IsValidPObjectPtr(m_pFailoverSyncApi))
	{
		m_pFailoverSyncApi->Destroy();
		POBJDELETE(m_pFailoverSyncApi);
	}

	if (CPObject::IsValidPObjectPtr(m_pFailoverCommunicationFromProcess))
	{
		m_pFailoverCommunicationFromProcess->DisconnectSocket();
		/*to fix the memory leakage VNGR-20863  David Liang  2011/05/23*/
		/* to free the socket here since we will recreate a new one in ConnecttoPairRMX*/
		m_pFailoverCommunicationFromProcess->FreeSocket();	
	}
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::OnTimerMasterPingTimeoutIdle(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::OnTimerMasterPingTimeoutIdle";

	BOOL bIsNetworkOk = IsNetworkOk();
	if (bIsNetworkOk == FALSE)
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::OnTimerMasterPingTimeoutIdle - Network problems";
		DeleteTimer(FAILOVER_MASTER_PING_TIMER);

		StopMasterCheckOnGetStateFromSlave();
		m_bIsNetworkProblemsInMaster = TRUE;
		//If we are master and there are network problems - we should become the slave:
		//added by huiyu. If we are master and there are problems - we should be become configuration state
		if(m_eMasterSlaveCurrentState == eMasterActualState)
		{
			m_eMasterSlaveCurrentState = eMasterConfigurationState;
		}
		
		ConnectToPairRMX();
	}
	else 
	{
		/*Set m_bIsNetworkProblemsInMaster as FASLSE since we can ping to GW successfully*/
		m_bIsNetworkProblemsInMaster = FALSE;
		StartTimer(FAILOVER_MASTER_PING_TIMER, FAILOVER_MASTER_PING_TIMEOUT);
	}
}

//////////////////////////////////////////////////////////////////////
BOOL CFailoverManager::IsNetworkOk()
{
	STATUS retStatus = STATUS_OK;

	string answer;
	CConfigManagerApi api;

	//added by huiyu.
	char ipStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString((DWORD)(m_pMngmntInfoFromProcess->mngmntIpAddress),ipStr);

	char defaultGwIpStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString(m_pMngmntInfoFromProcess->defaultGwIpAddress, defaultGwIpStr);

	DWORD  ipType = m_pMngmntInfoFromProcess->ipType;

	retStatus = api.FailoverArpingRequest(ipStr,defaultGwIpStr,ipType, answer);
	if (retStatus)
		TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::IsNetworkOk - retStatus = " << retStatus;

	BOOL bIsNetworkOk = (retStatus == STATUS_OK) ? TRUE : FALSE;

	if (m_bSimuNetworkDown == TRUE)
		return FALSE;
	
	return bIsNetworkOk;
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::OnTimerMasterPingTimeoutAnycase(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::OnTimerMasterPingTimeoutAnycase - Do Nothing!!";
}

//////////////////////////////////////////////////////////////////////

void CFailoverManager::LedHotStandbyStateInd(CSegment* pMsg)
{
	std::string answer;
	std::string cmd;
	eProductType procType = CProcessBase::GetProcess()->GetProductType();
	if ((procType == eProductTypeNinja) || (procType == eProductTypeGesher))
	{
		if (m_pProcess->GetFailoverConfiguration()->GetIsEnabled() == TRUE)
		{
			if (eSynchOk == m_pProcess->GetFailoverConfiguration()->GetMasterSlaveState())
			{
				if (eMaster == m_pProcess->GetFailoverConfiguration()->GetHotBackupType())
				{
					cmd = "sudo "+MCU_MCMS_DIR+"/Bin/LightCli HotStandby is_master";
					SystemPipedCommand(cmd.c_str(),answer);
					return;
				}
				else if (eSlave == m_pProcess->GetFailoverConfiguration()->GetHotBackupType())
				{
					cmd = "sudo "+MCU_MCMS_DIR+"/Bin/LightCli HotStandby is_slave";
					SystemPipedCommand(cmd.c_str(),answer);
					return;
				}
			}

		}

		cmd = "sudo "+MCU_MCMS_DIR+"/Bin/LightCli HotStandby single_mode";
		SystemPipedCommand(cmd.c_str(),answer);
	}
}
void CFailoverManager::UpdateMasterSlaveState(eFailoverStatusType eState)
{
	if (m_pProcess->GetFailoverConfiguration()->GetIsEnabled() == TRUE) {
		/*Only the hotbakup is enabled, updated the status.   VNGR-19872*/	
		m_pProcess->GetFailoverConfiguration()->SetMasterSlaveState(eState);
		m_pProcess->GetFailoverConfiguration()->WriteXmlFile(FAILOVER_CFG_FILE, "HOTBACKUP_DATA");
	}

	LedHotStandbyStateInd();
}


//////////////////////////////////////////////////////////////////////
const char* CFailoverManager::GetMasterSlaveStateStr(eMasterSlaveState eState)
{
	const char* name;
	if ((eState >= 0) && (eState < NUM_OF_MASTER_SLAVE_STATES))
		name = sMasterSlaveState[eState];
	else
		name = "invalid state";
	return name;
}

STATUS CFailoverManager::HandleTerminalNetworkDown(CTerminalCommand& cmd, std::ostream& ans)
{
	m_bSimuNetworkDown = TRUE;
	ans<<"network down"<<std::endl;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverManager::OnEventTriggerIndAnycase(CSegment* pMsg)
{
	std::string resetDes = "";
    DWORD eventType = 100000;
	
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnEventTriggerIndAnycase";
	*pMsg >> eventType;
	
	/*Only do the failover:  hotbackup is enable && devices is master*/
	if((m_pProcess->GetFailoverConfiguration()->GetIsEnabled() == FALSE) ||
		(m_pProcess->GetFailoverConfiguration()->GetHotBackupType() != eMaster))
	{	
		TRACESTR(eLevelInfoNormal) << "CFailoverManager: ignore the Event Trigger!";
		return;
	}

	switch (eventType) {
		case eFailoverMpmCardFailure:
			resetDes = "Lost connection with media card activeated hot backup"; 
			break;
		case eFailoverIsdnCardFailure:
			resetDes = "Lost connection with ISDN card activeated hot backup"; 
			break;
		case eFailoverSignalPortFailure:
			resetDes = "Lost connection with signaling port activeated hot backup"; 
			break;
	}

	map<DWORD,FAILOVER_EVENT_INFO> mapEventInfo = m_pProcess->GetFailoverConfiguration()->GetEventList();
	
	map<DWORD,FAILOVER_EVENT_INFO>::iterator itFind;
	itFind = mapEventInfo.find(eventType);
	if(itFind != mapEventInfo.end())
	{
		if(itFind->second.bEnabled)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverManager - OnEventTriggerIndAnycase - " << itFind->second.szDescription;
			/*To info the McuManager for fast failour*/
			SendFailoverTriggerToMcuMngr();
			/*Sleep 3 seconds to make sure Slave will get the trigger indication*/
			SystemSleep(100 * 3);
			
			CMcmsDaemonApi api;
			STATUS res = api.SendResetReq(resetDes);	
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CFailoverManager::OnFailoverRestartSlaveInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverManager::OnFailoverRestartSlaveInd";

	CManagerApi confPartyMngrApi(eProcessConfParty);
	CManagerApi resourceMngrApi(eProcessResource);

	STATUS res;

	res = confPartyMngrApi.SendOpcodeMsg(FAILOVER_RESTART_SLAVE);

	if (res != STATUS_OK)
		FPASSERT(FAILOVER_RESTART_SLAVE);

	res = resourceMngrApi.SendOpcodeMsg(FAILOVER_RESTART_SLAVE);
	if (res != STATUS_OK)
		FPASSERT(FAILOVER_RESTART_SLAVE);

	StopCommunicate("OnFailoverRestartSlaveInd");
	ConnectToPairRMX();
}

//////////////////////////////////////////////////////////////////////////////
void   CFailoverManager::OnMcuMngrSecurityModeInd(CSegment* pMsg)
{
	BYTE bSecured = FALSE;
	BYTE bisRequestPeerCertificate = FALSE;
	*pMsg >> bSecured;
	*pMsg >> bisRequestPeerCertificate;
	m_pProcess->SetRequestPeerCertificate(bisRequestPeerCertificate);
	if (m_pFailoverCommunicationFromProcess != NULL )
		m_pFailoverCommunicationFromProcess->SetSecured(bSecured);
	string msg1 = (bSecured) ? "bSecured = YES ," : "bSecured = NO ,";
	string msg2 = (bisRequestPeerCertificate) ? "RequestPeerCertificate = YES " : "RequestPeerCertificate = NO ";
	TRACEINTOFUNC << msg1 << msg2;
}
