// InstallerManager.cpp: implementation of the CInstallerManager class.
//
//////////////////////////////////////////////////////////////////////

#include "InstallerManager.h"

// fipssyms.h should be first to take FIPS version of AES_set_encrypt_key
#include <openssl/fipssyms.h>

#include <openssl/md5.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "OpcodesMcmsCommon.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "DummyEntry.h"
#include "ConfigManagerApi.h"
#include "ApiStatuses.h"
#include "ApacheDefines.h"
#include "TraceStream.h"
#include "Request.h"
#include "McuMngrStructs.h"
#include "OpcodesMcmsShelfMngr.h"
#include "OpcodesMcmsInternal.h"
#include "ConfigManagerOpcodes.h"
#include "MplMcmsProtocolTracer.h"
#include "Versions.h"
#include "ProcessBase.h"
#include "InstallerProcess.h"
#include "InstallerDefines.h"
#include "SysConfig.h"
#include "TerminalCommand.h"
#include "InstallerOpcodes.h"
#include "SystemFunctions.h"
#include "FaultsDefines.h"
#include "IncludePaths.h"
#include "OsFileIF.h"
#include "InstallerPreviousVersion.h"
#include "SysConfigKeys.h"
#include "AuditorApi.h"
#include "IPMCInterfaceApi.h"
#include "ConfPartyManagerApi.h"
#include "McmsDaemonApi.h"
#include "SslFunc.h"
#include "FipsMode.h"
#include "KeyCodeSaveLoader.h"
#include "ValidateKeyCode.h"
#include "LiteProfile.h"
#include "ResRsrcCalculator.h"
#include "AlarmStrTable.h"
#include "SysConfigEma.h"

extern char* AuthorizationGroupTypeToString(int authorizationGroup);

OPCODE progressStatus[] =
{
	STATUS_OK,
	STATUS_BACKUP_IN_PROGRESS,
	STATUS_RESTORE_IN_PROGRESS
};

#define TRAILER_MAX_SIZE 1000 //Just an estimation
#define MAX_ROW_LEN      100
#define BUFFER_SIZE      40000
#define MD5_RESULT_LEN   32 + 1 //(MD5 result len always 4 DWORDs) + 1 for null termination

//Install is linked to MCU_DATA_DIR/new_version
#define NEW_VERSION_NAME 		"Install/new_version.bin"
#define FALLBACK_VERSION_NAME 	((std::string)(MCU_DATA_DIR+"/fallback"))
#define CARDS_BEGIN_INSTALL		1 * 60 * SECOND
#define FPGA_CHECK_AND_BURNING_INTERVAL		50 * SECOND

#define FPGA_IMAGE_FILE_PATH       (MCU_MRMX_DIR+"/bin/fpga_upgrade/ninja_fpga_image.bin")
#define FPGA_IMAGE_FILE_IN_LOOP_PATH    (MCU_TMP_DIR+"/loop10/usr/rmx1000/bin/fpga_upgrade/ninja_fpga_image.bin")
#define FPGA_FORCE_UPGRADE_FLG              (MCU_MCMS_DIR+"/States/forceFPGAUpgrade.flg")
////////////////////////////////////////////////////////////////////////////
//               Task StateMachine's states
////////////////////////////////////////////////////////////////////////////
/*
- InstallerManager task is is in INSTALLING state as long as version installation is in process
- Otherwise it's in READY state
// default states: defined in StateMachine.h
//const  WORD  IDLE      = 0;
//const  WORD  ANYCASE   = 0xFFFF;
*/
const WORD READY		= 1;
const WORD INSTALLING	= 2;

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CInstallerManager)
  ONEVENT(XML_REQUEST                              , IDLE      , CInstallerManager::HandlePostRequest )
  ONEVENT(MCUMNGR_TO_INSTALLER_CFS_PARAMS_IND      , ANYCASE   , CInstallerManager::OnMcuMngrInstallerCfsParamsInd )
  ONEVENT(INSTALLER_FINISH_INSTALLATION            , ANYCASE   , CInstallerManager::OnInstallerFinishInstallation )
ONEVENT(INSTALLER_VALIDATE_KEY_CODE                , ANYCASE   , CInstallerManager::OnInstallerValidateKeyCode )

  ONEVENT(INSTALLER_TIMEOUT_INSTALATION            , INSTALLING, CInstallerManager::OnInstallationTimeoutInstall )
  ONEVENT(INSTALLER_TIMEOUT_INSTALATION            , READY     , CInstallerManager::OnInstallationTimeoutReady )
  ONEVENT(MCUMNGR_INIT_KEYCODE_FAILURE_IND         , ANYCASE   , CInstallerManager::OnMcuMngrInitKeycodeFailureInd )
  ONEVENT(TOUT_CARDS_BEGIN_INSTALL            	   , INSTALLING, CInstallerManager::OnTimeoutCardsBeginInstall )
  ONEVENT(INSTALLER_SOFTWARE_PROGRESS_FROM_CARDS   , ANYCASE   , CInstallerManager::OnInstallationSoftwareProgress )
  ONEVENT(INSTALLER_LAST_CARD_INSTALLATION_FINISHED, ANYCASE   , CInstallerManager::OnInstallationSoftwareDone )
  ONEVENT(INSTALLER_IPMC_PROGRESS_FROM_CARDS       , ANYCASE   , CInstallerManager::OnInstallationIpmcProgress )
  ONEVENT(INSTALLER_LAST_CARD_IPMC_FINISHED        , ANYCASE   , CInstallerManager::OnInstallationIpmcFromCardsFinished )
  ONEVENT(CHECK_CPU_IPMC_PROGRESS			       , ANYCASE   , CInstallerManager::OnCheckCpuIPMCProgress )
  ONEVENT(RESET_TIMEOUT						       , ANYCASE   , CInstallerManager::OnResetTimer )

  ONEVENT(BACKUP_RESTORE_TO_INSTALLER_START_IND    , ANYCASE   , CInstallerManager::OnBackupRestoreStartInd )
  ONEVENT(BACKUP_RESTORE_TO_INSTALLER_FINISH_IND   , ANYCASE   , CInstallerManager::OnBackupRestoreFinishInd )
  ONEVENT(BACKUP_RESTORE_PROGRESS_TIMER            , ANYCASE   , CInstallerManager::OnBackupRestoreProgressTimeout )

  ONEVENT(IPMCIF_TO_INSTALLER_UPGRADE_PROGRESS     , ANYCASE   , CInstallerManager::OnIPMCUpgradeProgress )
  ONEVENT(IPMCIF_TO_INSTALLER_NO_UPGRADE_NEEDED    , ANYCASE   , CInstallerManager::OnIPMCUpgradeNotNeeded )
  ONEVENT(FPGA_UPGRADE_PROGRESS                  , ANYCASE , CInstallerManager::OnFPGAUpgradeProgress ) 
  ONEVENT(FPGA_CHECK_AND_BURNING_TIMER                  , ANYCASE , CInstallerManager::OnFPGAChechAndBurningTimer ) 

  ONEVENT(MCUMNGR_AUTHENTICATION_STRUCT_REQ		, ANYCASE, CInstallerManager::OnMcuMngrAuthenticationStruct )
  ONEVENT(INSTALLER_TURN_OFF_LOCAL_TRACER_TIMER						       , ANYCASE   , CInstallerManager::OnTurnOffLocalTracerTimer )

PEND_MESSAGE_MAP(CInstallerManager,CManagerTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CInstallerManager)
    ON_TRANS("TRANS_MCU","BEGIN_RECEIVING_VERSION"    ,CDummyEntry  , CInstallerManager::HandleReceiveVersion)
    ON_TRANS("TRANS_MCU","FINISHED_TRANSFER_VERSION"  ,CDummyEntry  , CInstallerManager::HandleFinishReceiveVersion)
    ON_TRANS("TRANS_MCU","UPDATE_KEY_CODE"            ,CKeyCode     , CInstallerManager::HandleUpdateKeyCode)
    ON_TRANS("TRANS_MCU","INSTALL_PREVIOUS_VERSION"   ,CDummyEntry  , CInstallerManager::HandleInstallPreviousVersion)
END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CInstallerManager)
    ONCOMMAND("licensing_info"       ,CInstallerManager::HandleTerminalLicensingInfo,
            "licensing_info - display licensing information")
    ONCOMMAND("test_new_ver_validity",CInstallerManager::HandleTerminalCheckNewVersionValidity,
                    ("test_new_ver_validation "+MCU_DATA_DIR+"/new_version/new_version.bin YES/NO : this will test validity (trailer + MD5 + SHA1), YES - for TDD , NO - not being under TDD").c_str())
END_TERMINAL_COMMANDS

extern void InstallerMonitorEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void InstallerManagerEntryPoint(void* appParam)
{
	CInstallerManager * pInstallerManager = new CInstallerManager;
	pInstallerManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CInstallerManager::GetMonitorEntryPoint()
{
	return InstallerMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CInstallerManager::CInstallerManager()
{
	m_state = READY;
	m_pProcess  = (CInstallerProcess*)CInstallerProcess::GetProcess();
	m_productType  = m_pProcess->GetProductType();
	m_isKeyCodeFailure = false;

	m_pCfsParamsStruct = new INSTALLER_CFS_S;
	memset(m_pCfsParamsStruct, 0, sizeof(INSTALLER_CFS_S));

	m_backupRestoreInProgress = eBRIdle;
    m_installOperationFinished = false;
    m_installFlowFinished = false;
	m_isCfsParamsReceived = false;

    m_lastMinIpmcProgressCpu = -1;
    m_CpuIPMCUpgradeNotNeeded = false;
    m_lastMinIpmcProgressCards = -1;

    m_isNewVersionBlockedInMPMRX = false;

	m_fpgaRetryCount = 1;
	m_fpgaImgPath = "";
	memset(&m_fpgaUpgradeStatus, 0, sizeof(FPGA_UPGRADE_S));
	m_isUnderTDD=false;
	m_handleReceiveVersionStarted = false;

	m_pAuthenticationStruct = new MCMS_AUTHENTICATION_S;
	m_pAuthenticationStruct->isCtrlNewGeneration = true;

	m_onGoingConf = false;

}

/////////////////////////////////////////////////////////////////////////////
/*
CInstallerManager::CInstallerManager(const CInstallerManager &other)
:CManagerTask(other)
{


	POBJDELETE(m_pProcess);
	m_pProcess = new CInstallerProcess(other.m_pProcess);

	m_state = other.m_state;
	m_isKeyCodeFailure = other.m_isKeyCodeFailure;

	delete m_pCfsParamsStruct;
	m_pCfsParamsStruct = new INSTALLER_CFS_S;
	memcpy(m_pCfsParamsStruct, other.m_pCfsParamsStruct, sizeof(INSTALLER_CFS_S));
}

///////////////////////////////////////////////////////////////////////////////
CInstallerManager& CInstallerManager::operator=(const CInstallerManager& other)
{
	POBJDELETE(m_pProcess);
	m_pProcess = new CInstallerProcess(other.m_pProcess);

	m_state = other.m_state;
	m_isKeyCodeFailure = other.m_isKeyCodeFailure;

	delete m_pCfsParamsStruct;
	m_pCfsParamsStruct = new INSTALLER_CFS_S;
	memcpy(m_pCfsParamsStruct, other.m_pCfsParamsStruct, sizeof(INSTALLER_CFS_S));

	return *this;
}
*/
//////////////////////////////////////////////////////////////////////
CInstallerManager::~CInstallerManager()
{
	delete m_pCfsParamsStruct;
	delete m_pAuthenticationStruct;
}

void CInstallerManager::ManagerStartupActionsPoint()
{
  AskMcuMngrForCfsParams();
  TestAndEnterFipsMode();
  TestNinjaFPGAAndAlarm();

  //BRIDGE-12676
  SendAuthenticationStructReqToMcuMngr();


}

void CInstallerManager::OnBackupRestoreStartInd(CSegment* pSeg)
{
	BYTE state;
	*pSeg >> state;
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnBackupRestoreStartInd: state = " << strBackupRestoreAction[state];
	m_backupRestoreInProgress = (eBackupRestoreAction)state;

	if(eBackup == state)
	{
		StartTimer(BACKUP_RESTORE_PROGRESS_TIMER, (BACKUP_RESTORE_TIMEOUT+10*SECOND));
	}
}

////////////////////////////////////////////////////////////////////////////
void CInstallerManager::OnBackupRestoreFinishInd()
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnBackupRestoreFinishInd";
	m_backupRestoreInProgress = eBRIdle;
	DeleteTimer(BACKUP_RESTORE_PROGRESS_TIMER);
}

////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::OnBackupRestoreProgressTimeout()
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnBackupRestoreProgressTimeout";
	m_backupRestoreInProgress = eBRIdle;
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CInstallerManager::OnInstallationSoftwareProgress(CSegment* pSeg)
{
	DWORD percent = 0;
	*pSeg >> percent;

	DeleteTimer(TOUT_CARDS_BEGIN_INSTALL);

	CSmallString message = "Cards upgrade ";
	message << percent << "%";
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnInstallationSoftwareProgress " << message.GetString();

	if (percent < 100)
	  m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_swLoading,eStatusInProgress);


	m_pProcess->UpdateSoftwareInstallProgress(eInstallPhaseType_swLoading,percent);

}


////////////////////////////////////////////////////////////////////////////
void CInstallerManager::OnInstallationIpmcProgress(CSegment* pSeg)
{
	DWORD percent = 0;
	*pSeg >> percent;

	CSmallString message = "Cards upgrade ";
	message << percent << "%";
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnInstallationIpmcProgress " << message.GetString();


	m_lastMinIpmcProgressCards = percent;

	CalcIPMCBurnProgress(m_lastMinIpmcProgressCards, m_lastMinIpmcProgressCpu);

	//Enable system reset only if CPU ipmc is not being burned, and cards ipmc has finished
	if(!m_installFlowFinished && -1 == m_lastMinIpmcProgressCpu && 100 == m_lastMinIpmcProgressCards)
		InstallationFinished();
}

////////////////////////////////////////////////////////////////////////////
void CInstallerManager::CalcIPMCBurnProgress(int cards, int cpu)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::CalcIPMCBurnProgress, cpu=" << cpu << ", cards=" << cards;

	int meanIPMCProgress = 0;

	if(-1 == cpu && -1 != cards)
		meanIPMCProgress = cards;
	else if (-1 == cards && -1 != cpu)
		meanIPMCProgress = cpu;
	else if (-1 == cpu && -1 == cards)
		return;
	else
		meanIPMCProgress = (cards + cpu) / 2;

	if (meanIPMCProgress < 100)
		m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_ipmcBurning,eStatusInProgress);

	m_pProcess->UpdateSoftwareInstallProgress(eInstallPhaseType_ipmcBurning,meanIPMCProgress);
}

////////////////////////////////////////////////////////////////////////////
void CInstallerManager::OnInstallationIpmcFromCardsFinished(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnInstallationIpmcFromCardsFinished";

	//Enable system reset only if CPU ipmc is not being burned
	//If it is, reset will happen automatically, once CPU ipmc burning is completed.
	if(!m_installFlowFinished && m_CpuIPMCUpgradeNotNeeded)
		InstallationFinished();
	else if(!m_installFlowFinished && (-1 == m_lastMinIpmcProgressCpu))
		StartTimer(CHECK_CPU_IPMC_PROGRESS, 90 * SECOND);

}

////////////////////////////////////////////////////////////////////////////
void CInstallerManager::OnCheckCpuIPMCProgress(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnCheckCpuIPMCProgress";
	if(!m_installFlowFinished && (-1 == m_lastMinIpmcProgressCpu))
		InstallationFinished();
	else
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnCheckCpuIPMCProgress, cpu ipmc is burning";
}

////////////////////////////////////////////////////////////////////////////
void CInstallerManager::InstallationFinished()
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::InstallationFinished";

	m_installFlowFinished = true;

	m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_ipmcBurning,eStatusSuccess);
	m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_completed,eStatusSuccess);

	AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
			NEW_VERSION_INSTALLED,
			MAJOR_ERROR_LEVEL,
			"A new version was installed. Reset the MCU",
			true,
			true
	);

	DeleteFile(SYSTEM_AFTER_VERSION_INSTALLED_FILE);

	CManagerApi apiCards(eProcessCards);
	apiCards.SendOpcodeMsg(INSTALLER_RESET_ALL_TO_CARDS);

	StartTimer(RESET_TIMEOUT, 30 * SECOND );
}



////////////////////////////////////////////////////////////////////////////
void CInstallerManager::OnResetTimer(CSegment* pSeg)
{
	CManagerApi apiCards(eProcessCards);
	apiCards.SendOpcodeMsg(INSTALLER_RESET_ALL_TO_CARDS);

	bool isAAExist = IsActiveAlarmExistByErrorCode(SHA1_VERSION);
	if (isAAExist)
	{
		CManagerApi apiAuthentication(eProcessAuthentication);
		apiAuthentication.SendOpcodeMsg(INSTALLER_AUTHETICATION_REMOVE_ENC_OPERATOR_FILE);
	}

	PASSERTMSG(TRUE, "Hardware watchdog was set to 15 seconds");
	CIPMCInterfaceApi apiIPMC;
	apiIPMC.SetWatchdog(15);
	apiIPMC.TriggerWatchdog();

	SystemSleep(20);
	CMcmsDaemonApi api;
	STATUS res = api.SendResetReq("A new version was installed.");
}

////////////////////////////////////////////////////////////////////////////
void CInstallerManager::OnTimeoutCardsBeginInstall(CSegment* pSeg)
{
	OnInstallationSoftwareDone(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CInstallerManager::OnInstallationSoftwareDone(CSegment* pSeg)
{
    m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_swLoading,eStatusSuccess);
    m_pProcess->UpdateSoftwareInstallProgress(eInstallPhaseType_swLoading,100);

    m_lastMinIpmcProgressCpu = -1;
    m_lastMinIpmcProgressCards = -1;

    SendFlushtoLogger();

    // initiate MCMS IPMC burning...
    CIPMCInterfaceApi apiIPMC;
    apiIPMC.TurnOffWatchdog();
    apiIPMC.UpgradeIPMCVersion();

}

////////////////////////////////////////////////////////////////////////////
void CInstallerManager::OnIPMCUpgradeProgress(CSegment* pSeg)
{
	DWORD percent = 0;
	*pSeg >> percent;

	if(100 != percent)
	{
		CSmallString message = "IPMC upgrade ";
		message << percent << "%";
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnIPMCUpgradeProgress " << message.GetString();
		bool isAAExist = IsActiveAlarmExistByErrorCode(AA_VERSION_IPMC_INSTALL_PROGRESS);
		if (isAAExist)
		{
			UpdateActiveAlarmDescriptionByErrorCode(AA_VERSION_IPMC_INSTALL_PROGRESS, message.GetString());
		}
		else
		{
			AddActiveAlarm(FAULT_GENERAL_SUBJECT,
					AA_VERSION_IPMC_INSTALL_PROGRESS,
					SYSTEM_MESSAGE,
					message.GetString(),
					true,
					true,
					0xFFFFFFFF,
					5);
		}
	}
	m_lastMinIpmcProgressCpu = percent;

	CalcIPMCBurnProgress(m_lastMinIpmcProgressCards, m_lastMinIpmcProgressCpu);

	if(!m_installFlowFinished && (percent == 90))
		SendFlushtoLogger();

	//Enable system reset only if CPU ipmc is not being burned
	//If it is, reset will happen automatically, once CPU ipmc burning is completed.
	if(!m_installFlowFinished && (percent > 98))
		InstallationFinished();

}

////////////////////////////////////////////////////////////////////////////
void CInstallerManager::OnIPMCUpgradeNotNeeded(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnIPMCUpgradeNotNeeded ";

	m_CpuIPMCUpgradeNotNeeded = true;

	CalcIPMCBurnProgress(m_lastMinIpmcProgressCards, m_lastMinIpmcProgressCpu);

	//Enable system reset only if cards ipmc has finished
	if(!m_installFlowFinished && (100 == m_lastMinIpmcProgressCards))
		InstallationFinished();
}

////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::SendMsgToBackupRestore(OPCODE action)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::SendMsgToBackupRestore";
	//Send to notify BackupRestore that Installation starts/ends
	CSegment*  pMsg = new CSegment;

    const COsQueue* pBackupRestoreMbx
    	= CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessBackupRestore,eManager);
	STATUS res = pBackupRestoreMbx->Send(pMsg, action);

	return res;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::HandleReceiveVersion(CRequest *pRequest)
{
    STATUS retStatus = STATUS_OK;
    CLargeString retStr = "\nCInstallerManager::HandleReceiveVersion ";
    TRACESTR(eLevelInfoNormal) << retStr.GetString();

    m_handleReceiveVersionStarted = true;
    StartLocalTracer(true);

	AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
			    UPGRADE_RECEIVE_VERSION,
				MAJOR_ERROR_LEVEL,
				GetAlarmDescription(UPGRADE_RECEIVE_VERSION),
				true,
				true
		);


    BOOL bSafeUpgradeMode = NO;
    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
    sysConfig->GetBOOLDataByKey(CFG_KEY_ENFORCE_SAFE_UPGRADE, bSafeUpgradeMode);

    if (bSafeUpgradeMode == NO)
    {

    	bool isAAExist = IsActiveAlarmExistByErrorCode(AA_SAFE_UPGRADE_OFF);
    	if (!isAAExist)

    	{
    		AddActiveAlarm(FAULT_GENERAL_SUBJECT,
    				AA_SAFE_UPGRADE_OFF,
    				SYSTEM_MESSAGE,
    				"Warning: Upgrade started and SAFE Upgrade protection is turned OFF",
    				true,
    				true,
    				0xFFFFFFFF,
    				5);
    	}
    }


    if(pRequest->GetAuthorization() != SUPER)
    {
    	FPTRACE(eLevelInfoNormal,"CInstallerManager::HandleReceiveVersion: No permission");
    	pRequest->SetStatus(STATUS_NO_PERMISSION);
    	pRequest->SetConfirmObject(new CDummyEntry);
    	return STATUS_OK;
    }

	/* VNGR-23085:  If the Cfs Params is not received from MCUManager, don't allow the software upgrade*/
	/* We need to use m_pCfsParamsStruct in our installation */
	if(m_isCfsParamsReceived == FALSE)
	{
		FPTRACE(eLevelInfoNormal,"CInstallerManager::HandleReceiveVersion: Cfs Params not received yet.");
		pRequest->SetStatus(STATUS_MCU_VERSION_INSTALL_FAILED);
		pRequest->SetExDescription("Authentication indication is not received from MCU Manager. Software upgrade is not allowed.");
		pRequest->SetConfirmObject(new CDummyEntry);
		return STATUS_OK;
	}

	m_installOperationFinished = false;

	m_isNewVersionBlockedInMPMRX = false;

	if(m_backupRestoreInProgress != eBRIdle)
	{
		retStatus = progressStatus[m_backupRestoreInProgress];
	}
	else
	{
		SendMsgToBackupRestore(INSTALLER_TO_BACKUP_RESTORE_START_IND);
		if (INSTALLING == m_state)
		{
			retStr << "\n---- Task is in INSTALLING state";
			retStatus = STATUS_IN_PROGRESS;
		}
		else
		{
			CConfigManagerApi api;
			api.RemountVersionPartition(TRUE, TRUE, TRUE, 5);
			eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
			if(eProductTypeGesher != curProductType && eProductTypeNinja != curProductType)
			{
				api.HandleFallbackVersion(5);
			}
		}

	}

	if(STATUS_OK == retStatus)
		retStatus = SendConfBlockToConfParty(TRUE, 5);

	//BRIDGE-13966
	if (retStatus == STATUS_ILLEGAL_WHILE_ONGOING_CONFERENCE_EXISTS)
		m_onGoingConf = true;
	else
		m_onGoingConf = false;

	//In case the response from ConfParty process wasn't received
	if ((STATUS_OK != retStatus) && (STATUS_ILLEGAL_WHILE_ONGOING_CONFERENCE_EXISTS != retStatus))
		  SendConfBlockToConfParty(FALSE, 1);	//release conference block in case of a failure

    // ===== reply to EMA
	pRequest->SetStatus(retStatus);
    pRequest->SetConfirmObject(new CDummyEntry);

	retStr << "\n---- Status: " << m_pProcess->GetStatusAsString(retStatus).c_str();
	TRACESTR(eLevelInfoNormal) << retStr.GetString();

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::SendConfBlockToConfParty(BYTE isBlock, WORD timeOutInSec)
{
	STATUS result = STATUS_FAIL;
    CSegment*  pParam = new CSegment();
    *pParam << (BYTE)isBlock;
    *pParam << (BYTE)eConfBlockReason_Installer_Position_1;

    // ===== 2. send to ConfPartyMngr
    OPCODE opcode;
    CConfPartyManagerApi ConfPartyApi;
    STATUS status = ConfPartyApi.SendMessageSync(pParam, CONF_BLOCK_IND, timeOutInSec*SECOND, opcode);
    if(status == STATUS_OK)
    {
    	result =  opcode;
    	if(STATUS_OK != result)
    		result = STATUS_ILLEGAL_WHILE_ONGOING_CONFERENCE_EXISTS;
    }
    else
    	result = status;

    TRACESTR(eLevelInfoNormal) << "CInstallerManager::SendConfBlockToConfParty, res=" << result <<" isBlock = " << (TRUE == isBlock ? "TRUE" : "FALSE");
    return result;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::HandleFinishReceiveVersion(CRequest *pRequest)
{

    if(pRequest->GetAuthorization() != SUPER)
	{
		FPTRACE(eLevelInfoNormal,"CInstallerManager::HandleFinishReceiveVersion: No permission");
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

    if(m_installOperationFinished || m_onGoingConf == true)
    {
    	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::HandleFinishReceiveVersion : m_onGoingConf " << (DWORD)m_onGoingConf;

        pRequest->SetStatus(STATUS_MCU_VERSION_INSTALL_FAILED);
        return STATUS_OK;
    }

    STATUS retStatus = STATUS_OK;
    string retStr = "\nCInstallerManager::HandleFinishReceiveVersion : ";

	RemoveActiveAlarmByErrorCode(UPGRADE_RECEIVE_VERSION);

	if (INSTALLING == m_state)
	{
		retStr += "\n---- State: INSTALLING";
        retStatus = STATUS_IN_PROGRESS;
	}
    else
    {
        retStr += "\n---- State: NOT INSTALLING(GOOD)";
        m_pProcess->SetInstallationStatus(STATUS_INSTALLATION_IN_PROGRESS);

        BOOL isDebugMode = FALSE;
        CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_DEBUG_MODE, isDebugMode);
        if(isDebugMode)
        	retStatus = STATUS_INSTALLTION_WHILE_DEBUG_MODE | WARNING_MASK;
    }

    pRequest->SetStatus(retStatus);
    pRequest->SetConfirmObject(new CDummyEntry);

    CManagerApi api(eProcessInstaller);
    api.SendOpcodeMsg(INSTALLER_FINISH_INSTALLATION);

    StartTimer(INSTALLER_TIMEOUT_INSTALATION, TIMEOUT_INSTALATION );
    TRACESTR(eLevelInfoNormal) << "CInstallerManager::HandleFinishReceiveVersion start timer INSTALLER_TIMEOUT_INSTALATION";


    TRACESTR(eLevelInfoNormal) << retStr.c_str()
                           << "\n---- Status: " << m_pProcess->GetStatusAsString(retStatus).c_str();

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::HandleInstallPreviousVersion(CRequest *pRequest)
{

	if(pRequest->GetAuthorization() != SUPER)
	{
		FPTRACE(eLevelInfoNormal,"CInstallerManager::HandleInstallPreviousVersion: No permission");
		pRequest->SetConfirmObject(new CDummyEntry);		
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

    if(m_installOperationFinished || m_onGoingConf == true)
    {
    	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::HandleInstalPreviousVersion : m_onGoingConf " << (DWORD)m_onGoingConf;
        pRequest->SetConfirmObject(new CDummyEntry);    
        pRequest->SetStatus(STATUS_MCU_VERSION_INSTALL_FAILED);
        return STATUS_OK;
    }


    //BRIDGE-14504
    BOOL isExist = IsFileExists(FALLBACK_VERSION_NAME);
    if (isExist == FALSE)
    {
    	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::HandleInstalPreviousVersion : No Fallback";
    	pRequest->SetConfirmObject(new CDummyEntry);
    	pRequest->SetStatus(STATUS_NO_FALLBACK_VERSION);
    	return STATUS_OK;

    }

	STATUS retStatus = STATUS_OK;
	CLargeString retStr = "\nCInstallerManager::HandleInstalPreviousVersion : ";


	if (INSTALLING == m_state)
	{
        return STATUS_FAIL;
	}
    else
    {

	    // Fix BRIDGE-2151  forbid restore last version with active conference.
	    retStatus = SendConfBlockToConfParty(TRUE, 5);

	    //BRIDGE-13966
	    if (retStatus == STATUS_ILLEGAL_WHILE_ONGOING_CONFERENCE_EXISTS)
	    	m_onGoingConf = true;
	    else
	    	m_onGoingConf = false;

	    //In case the response from ConfParty process wasn't received
	    if ((STATUS_OK != retStatus) && (STATUS_ILLEGAL_WHILE_ONGOING_CONFERENCE_EXISTS != retStatus))
		    SendConfBlockToConfParty(FALSE, 1);	//release conference block in case of a failure


	    retStr << "\n---- Status: " << m_pProcess->GetStatusAsString(retStatus).c_str();
	    TRACESTR(eLevelInfoNormal) << retStr.GetString();

	    if(STATUS_OK != retStatus)
	    {
	       // ===== reply to EMA
	       pRequest->SetConfirmObject(new CDummyEntry);    
	       pRequest->SetStatus(retStatus);
	       return STATUS_OK;
	    }
	    //Fix BRIDGE-2151 end

        m_pProcess->SetInstallationStatus(STATUS_INSTALLATION_IN_PROGRESS);
    }


	CInstallPreviousVersion* pInstallPrevVer = new CInstallPreviousVersion();

	if (pInstallPrevVer)
	{
		*pInstallPrevVer = *(CInstallPreviousVersion*)pRequest->GetRequestObject();
		eVersionType versionType = pInstallPrevVer->GetVersionType();
	}

	retStatus = STATUS_IN_PROGRESS;

    pRequest->SetStatus(retStatus);
    pRequest->SetConfirmObject(new CDummyEntry);

    FPTRACE(eLevelInfoNormal,"CInstallerManager::HandleInstallPreviousVersion m_state = INSTALLING ");
    m_state = INSTALLING;
    CManagerApi api(eProcessInstaller);
    CSegment *pSeg = new CSegment;
    *pSeg << (DWORD) eVersionTypeFallback; // SAGI - fallback is hardcoded in this stage
    STATUS res = api.SendMsg(pSeg, INSTALLER_FINISH_INSTALLATION);

    if (res == STATUS_OK)
    {
		// BRIDGE-14695. Next update activation key will be treated as usual upgrade
		m_handleReceiveVersionStarted = true;
        FPTRACE(eLevelInfoNormal, "Setting upgrade indication flag to true");
    }
    else {
        FPTRACE(eLevelInfoNormal,"Failed INSTALLER_FINISH_INSTALLATION");

    }

	return retStatus;
}


/////////////////////////////////////////////////////////////////////////////
// for Call Generator - ValidateProductFamily
STATUS CInstallerManager::ValidateProductFamily(TRAILER_INFO_S &trailerInfo)
{
    if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
    {
    	PTRACE(eLevelInfoNormal,"CInstallerManager::ValidateProductFamily - ERROR - system is not CG!!");
    	return STATUS_OK;
    }

    TRACESTR(eLevelInfoNormal) << "CInstallerManager::ValidateProductFamily";
	STATUS status = STATUS_OK;

	string firmVersion = trailerInfo.FirmwareVersion;
	string productFamily = ::ProductFamilyToString(m_pProcess->GetProductFamily());
	string subVersion = firmVersion.substr(0, 13);

    if (subVersion.substr(0,13) != "CALLGENERATOR" || productFamily.substr(0,2) != "CG")
    {
        if(productFamily != subVersion)
        {
            TRACESTR(eLevelInfoNormal) << "ValidateProductFamily: " << " productFamily = " << productFamily
                                   << ", firmVersion = " << firmVersion << ", subVersion = " << subVersion;

            status = STATUS_PRODUCT_FAMILY_TYPE_MISMATCH;
        }
    }


	return status;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::OnInstallerFinishInstallation(CSegment *pSeg)
{
	STATUS retStatus = STATUS_OK;

	bool bInstallFailed = false;

	//Gesher&Ninja upgrade
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	bool bNeedUnMount = false;

    if(eProductTypeGesher != curProductType && eProductTypeNinja != curProductType)
    {
	   /* if(FALSE == IsTarget())
	    {
	        PTRACE(eLevelInfoNormal,"CInstallerManager::OnInstallerFinishInstallation m_state = READY ");
	        m_state = READY;
	        m_pProcess->ResetSoftwareInstall();

	        CManagerApi apiCards(eProcessCards);
			// apiCards.SendOpcodeMsg(INSTALLER_NEW_VERSION_IS_READY_TO_CARDS);

			CSegment *pSeg = new CSegment;
			*pSeg << (DWORD) m_isNewVersionBlockedInMPMRX; // Rachel need to block downgrade of MPMRX to 7.x versions
			apiCards.SendMsg(pSeg, INSTALLER_NEW_VERSION_IS_READY_TO_CARDS);
		
	        SendMsgToBackupRestore(INSTALLER_TO_BACKUP_RESTORE_FINISH_IND);
	        return retStatus;
	    }*/
    }
    else
    {
        LedUpgradeInProgress();
    }


    CConfigManagerApi configMngrapi;

    // ===== 1. copy fallback version to new_version if needed
    if (pSeg)
    {
        // ===== write permission
    	configMngrapi.RemountVersionPartition(TRUE,FALSE);

        eVersionType versionType;
        DWORD temp_versionType;
        *pSeg >> temp_versionType;
        versionType = (eVersionType) temp_versionType;

        if (versionType == eVersionTypeFallback)
        {
            TRACESTR(eLevelInfoNormal) << "Copy fallback version into new_version";
            STATUS stat = configMngrapi.RestoreFallbackVersion();

            if (stat != STATUS_OK)
            {

                TRACESTR(eLevelError)
                    << "CInstallerManager::OnInstallerFinishInstallation copy fallback failed: ";
                bInstallFailed = true;
            }
            else
            {
                TRACESTR(eLevelInfoNormal)
                    << "CInstallerManager::OnInstallerFinishInstallation copy fallback OK";
            }
        }

    }

    // ===== 2. disable write permission (after new version has been added)
    configMngrapi.RemountVersionPartition(FALSE);

    CLargeString retStr = "\nCInstallerManager::OnInstallerFinishInstallation ";
    TRACESTR(eLevelInfoNormal) << retStr.GetString();

    retStr << "\n---- Finish Remount(FALSE)";
    TRACESTR(eLevelInfoNormal) << retStr.GetString();

    // ===== 3. update task's state (it will be in INSTALLING state until version installation is done)
    m_state = INSTALLING;
    TRACESTR(eLevelInfoNormal) << "CInstallerManager::OnInstallerFinishInstallation Set m_state status INSTALLING";

    // ===== 4. check version MD5 validation, and accept version if ok
    TRACESTR(eLevelInfoNormal) << "Get Trailier";
    TRAILER_INFO_S TrailerInfoStruct;
	TrailerInfoStruct.ProductType = "";
	retStatus = GetTrailerInfo(TrailerInfoStruct,NEW_VERSION_NAME);

	if(retStatus == STATUS_OK)
	{


		//***********************************************************************************
		// 4.9.12 Rachel Cohen
		// from version 7.8 the encryption will be in SHA256 we need to reset the
		// user/password file in case we downgrade to versions 7.5 7.6 7.7
		//***********************************************************************************
		string firmVersion = TrailerInfoStruct.FirmwareVersion;
		string subVersion = firmVersion.substr(4, 3);

		TRACESTR(eLevelInfoNormal) << ", firmVersion = " << firmVersion << ", subVersion = " << subVersion;


		//BRIDGE-12676  9.4.14 Rachel Cohen
		retStatus = preventSWDowngradeInNewCtrl(firmVersion);




		if(retStatus == STATUS_OK)
		{
			string ver7_5 = "7.5";
			string ver7_6 = "7.6";
			string ver7_7 = "7.7";
			string ver7_8 = "7.8";


			if ((subVersion == ver7_5) || (subVersion == ver7_6) || (subVersion == ver7_7) || (subVersion == ver7_8))

			{

				//CManagerApi apiAuthentication(eProcessAuthentication);
				//apiAuthentication.SendOpcodeMsg(INSTALLER_AUTHETICATION_REMOVE_ENC_OPERATOR_FILE);


				AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
						SHA1_VERSION,
						MAJOR_ERROR_LEVEL,
						"RMX user/password list will be reset.",
						true,
						true
				);




			}
			else
			{
				if(FALSE == IsTarget() && eProductTypeGesher != curProductType && eProductTypeNinja != curProductType)
				{
					PTRACE(eLevelInfoNormal,"CInstallerManager::OnInstallerFinishInstallation m_state = READY ");
					m_state = READY;
					m_pProcess->ResetSoftwareInstall();

					CManagerApi apiCards(eProcessCards);
					// apiCards.SendOpcodeMsg(INSTALLER_NEW_VERSION_IS_READY_TO_CARDS);

					CSegment *pSeg = new CSegment;
					*pSeg << (DWORD) m_isNewVersionBlockedInMPMRX; // Rachel need to block downgrade of MPMRX to 7.x versions
					apiCards.SendMsg(pSeg, INSTALLER_NEW_VERSION_IS_READY_TO_CARDS);

					SendMsgToBackupRestore(INSTALLER_TO_BACKUP_RESTORE_FINISH_IND);
					return retStatus;
				}
			}
		}


		//Gesher&Ninja must check ProductType
		if(eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
		{
			std::string product;
			if(eProductTypeGesher == curProductType)
				product = "GESHER";
			else if(eProductTypeNinja == curProductType)
				product = "NINJA";

			retStatus = CheckProductTypeValidity(TrailerInfoStruct.ProductType, product);

			if(STATUS_OK != retStatus){
				TRACESTR(eLevelError) << "CInstallerManager::OnInstallerFinishInstallation(): ProductType mismatch";
				bInstallFailed = true;
			}
		}
		//Standard RMX image(RMX2000/RMX4000) should have no ProuctType
		else
		{
			if(TrailerInfoStruct.ProductType.size()){
				TRACESTR(eLevelError) << "CInstallerManager::OnInstallerFinishInstallation(): This is not Standard RMX software. ProductType: " << TrailerInfoStruct.ProductType;
				retStatus = STATUS_FAIL;
				bInstallFailed = true;
			}
		}
	}

	if(retStatus == STATUS_OK)
	{
		// BLOCKED old version on MPMRX cards
		string firmVersion = TrailerInfoStruct.FirmwareVersion;

		string subVersion = firmVersion.substr(4, 1);

		TRACESTR(eLevelInfoNormal) << ", firmVersion = " << firmVersion << ", subVersion = " << subVersion;

		if (subVersion < "8")
		{
			TRACESTR(eLevelInfoNormal) << "BLOCKED VERSION on MPMRX cards!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  \n";
			m_isNewVersionBlockedInMPMRX = true;
		}



		TRACEINTO << "GetTrailerInfo SHA1: " << TrailerInfoStruct.NoTrailerSHA1SUM;

		retStatus = CheckSHA1(TrailerInfoStruct.NoTrailerSHA1SUM,
				NEW_VERSION_NAME,
				TrailerInfoStruct.NoTrailerSize);

        if (STATUS_OK != retStatus)
            bInstallFailed = true;
    }
	else
	{
		TRACESTR(eLevelError) << "CInstallerManager::OnInstallerFinishInstallation(): fail to retrieve version name";
		bInstallFailed = true;
	}

    if (STATUS_OK == retStatus) // new version exists
    {
    	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnInstallerFinishInstallation ---- using new version's script ";
    	//Mount new version, so new version's script is accessible
    	std::string result = "";

    	if (TRUE == IsTarget() || eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
    	{
    		retStatus = configMngrapi.MountNewVersion();
    		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnInstallerFinishInstallation - MountNewVersion result ---- "
    				<< m_pProcess->GetStatusAsString(retStatus).c_str();

    		if(STATUS_OK == retStatus) bNeedUnMount = true;


		//if Script exists in new version:
	    if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	    {
	    	if(STATUS_OK == retStatus)
			{
				TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnInstallerFinishInstallation ---- new version is mounted, accessing script ";

				//check safe upgrade
				/*BOOL bSafeUpgradeMode = NO;
				CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
				sysConfig->GetBOOLDataByKey(CFG_KEY_ENFORCE_SAFE_UPGRADE, bSafeUpgradeMode);

				if (bSafeUpgradeMode == NO)
				{

					bool isAAExist = IsActiveAlarmExistByErrorCode(AA_SAFE_UPGRADE_OFF);
					if (!isAAExist)

					{
						AddActiveAlarm(FAULT_GENERAL_SUBJECT,
								AA_SAFE_UPGRADE_OFF,
								SYSTEM_MESSAGE,
								"Warning: Upgrade started and SAFE Upgrade protection is turned OFF",
								true,
								true,
								0xFFFFFFFF,
								5);
					}
				}*/


				//Run the script of the new version. If returned value is 'YES' then new version may be cycled with current one.
				retStatus = configMngrapi.FirmwareCheck(result);
				TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnInstallerFinishInstallation - FirmwareCheck result ---- "
									   << "status=" << m_pProcess->GetStatusAsString(retStatus).c_str()
									   << ", result=" << result<<".";

				if(STATUS_OK == retStatus)
					if("NO" == result)
					{
						retStatus = STATUS_NEW_VERSION_CANNOT_BE_INSTALLED;
						RemoveActiveAlarmByErrorCode(AA_SAFE_UPGRADE_OFF);
					}

				DumpScriptsLogs();

				//NINJA: copy FPGA image to MCU_TMP_DIR for next step: FPGA Upgrade.
				if(STATUS_OK == retStatus && eProductTypeNinja == curProductType)
				{
					std::string fpgaImgFileInLoopPath = FPGA_IMAGE_FILE_IN_LOOP_PATH;
					BackupFPGAImg(m_fpgaImgPath, fpgaImgFileInLoopPath.c_str());
				}
			}
			//If the Script does not exist - let the version installation flow continue as long as product type is RMX2000.
			else
			{
				std::string cmd = "cat "+MCU_MCMS_DIR+"/ProductType";
				SystemPipedCommand(cmd.c_str(), result);
				TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnInstallerFinishInstallation ---- product type:" << result;
				retStatus = STATUS_OK;
			}
	    }


	    	if(STATUS_OK == retStatus)
	    	{
	    		retStatus = configMngrapi.UnmountNewVersion();
	    		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnInstallerFinishInstallation - UnmountNewVersion result ---- "
	    				<< m_pProcess->GetStatusAsString(retStatus).c_str();
	    	}
	    	else
	    	{
	    		bInstallFailed = true;
	    		if(bNeedUnMount) // Fix BRIDGE-2283
	    		{
	    			STATUS mountStatus = configMngrapi.UnmountNewVersion();
	    			TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnInstallerFinishInstallation - UnmountNewVersion result2 ---- "
	    					<< m_pProcess->GetStatusAsString(mountStatus).c_str();
	    			bNeedUnMount = false;
	    		}
	    	}
	    }
		
		if(STATUS_OK == retStatus)
		{
			retStr = "\nCInstallerManager::OnInstallerFinishInstallation ---- checking Mcu version vs. current version ";
			TRACESTR(eLevelInfoNormal) << retStr.GetString();

			VERSION_S mcuVersion;
			mcuVersion = ConvertStringToVersionStruct(TrailerInfoStruct.FirmwareVersion);
			TRACESTR(eLevelInfoNormal) << "TrailerInfoStruct.FirmwareVersion - " << TrailerInfoStruct.FirmwareVersion;
			TRACESTR(eLevelInfoNormal) << "mcuVersion" << mcuVersion.ver_major << "." <<mcuVersion.ver_minor ;

			retStatus = CheckCurrentVersionVsMcuVersion(mcuVersion, YES);

			if (STATUS_OK == retStatus)
			{

				if(FALSE == IsTarget() && eProductTypeGesher != curProductType && eProductTypeNinja != curProductType)
				{
					PTRACE(eLevelInfoNormal,"CInstallerManager::OnInstallerFinishInstallation m_state = READY ");
					m_state = READY;
					m_pProcess->ResetSoftwareInstall();

					CManagerApi apiCards(eProcessCards);
					// apiCards.SendOpcodeMsg(INSTALLER_NEW_VERSION_IS_READY_TO_CARDS);

					CSegment *pSeg = new CSegment;
					*pSeg << (DWORD) m_isNewVersionBlockedInMPMRX; // Rachel need to block downgrade of MPMRX to 7.x versions
					apiCards.SendMsg(pSeg, INSTALLER_NEW_VERSION_IS_READY_TO_CARDS);

					SendMsgToBackupRestore(INSTALLER_TO_BACKUP_RESTORE_FINISH_IND);
					return retStatus;
				}


				retStatus = AcceptVersion(TrailerInfoStruct.FirmwareVersion, YES);
				if(STATUS_CFS_MSG_6_WARNING_MASKED == retStatus || STATUS_CFS_MSG_7_WARNING_MASKED == retStatus)
				{
					if(eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
					{
						retStatus = STATUS_OK;
					}
					else
					{
						retStatus = CompareTrailerVersions(TrailerInfoStruct.TrailerVersion);
					}

					//STATUS_OK means we can go ahead with new installation flow
					if ((STATUS_OK == retStatus) &&
						(CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator) &&
						(curProductType != eProductTypeGesher) &&
						(curProductType != eProductTypeNinja))
						DistributeNewVersionToAll();

					//old installation flow
					else
					{
						if(curProductType == eProductTypeGesher)
						{
							;
						}
						else if(curProductType == eProductTypeNinja)
						{
							m_fpgaRetryCount = 1;
							// swloading complete
							GesherInstallationSoftwareDone();
							SendFPGAUpgradeToConfig();
						}
						else
						{
							AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
									NEW_VERSION_INSTALLED,
									MAJOR_ERROR_LEVEL,
									"A new version was installed. Reset the MCU",
									true,
									true);
							m_state = READY;
							TRACESTR(eLevelInfoNormal) << "CInstallerManager::OnInstallerFinishInstallation Set m_state status READY - old installation flow";
							m_pProcess->SetInstallationStatus(STATUS_OK /*retStatus*/);
							DeleteTimer(INSTALLER_TIMEOUT_INSTALATION);

							TRACESTR(eLevelInfoNormal) << "CInstallerManager::OnInstallerFinishInstallation delete timer INSTALLER_TIMEOUT_INSTALATION";
						}
					}
				}
				else
					bInstallFailed = true;

				m_installOperationFinished = true;
            /*************************************************************************************************/
				/* 16.6.10 Bios update added by Rachel Cohen                                                     */
				/* If we enter Bios update flow on startup and the file /Config/states/Bios.flg has been created */
				/* we want to remove it for a new installed version . cause if the update didn't succeed, it will*/
				/* try again in the startup of the new install version.                                          */
				/*************************************************************************************************/
				if(eProductTypeGesher != curProductType && eProductTypeNinja != curProductType)
				{
				std::string answerTmp;
				std::string cmd = "rm -f "+MCU_CONFIG_DIR+"/states/BIOS.flg";
				STATUS statTmp = SystemPipedCommand(cmd.c_str(),answerTmp);

				TRACESTR(eLevelInfoNormal) <<
			    "CConfiguratorManager::OnInstallerFinishInstallation - delete file BIOS.flg: " << answerTmp;
				}

			}
			else
				if (STATUS_CFS_MSG_10_WARNING_MASKED == retStatus || STATUS_CFS_MSG_11_WARNING_MASKED == retStatus)
				{
					DeleteTimer(INSTALLER_TIMEOUT_INSTALATION);
					m_pProcess->SetInstallationStatus(retStatus);
					m_state = READY;
					TRACESTR(eLevelInfoNormal) << "CInstallerManager::OnInstallerFinishInstallation Set m_state status READY - ret status" << retStatus << "delete timer INSTALLER_TIMEOUT_INSTALATION";
				}
				else
					bInstallFailed = true;
    	}

    }
    else // new version doesn't exist
    {
    	if(STATUS_PRODUCT_FAMILY_TYPE_MISMATCH != retStatus  && STATUS_NEW_VERSION_CANNOT_BE_INSTALLED!= retStatus ) // if not product type mismatch in CG
    	{
    	    retStatus = STATUS_CFS_MSG_12;//"Version download failed"
    	}
    	retStr = "\nCInstallerManager::OnInstallerFinishInstallation ---- new version doesn't exist ";
        TRACESTR(eLevelInfoNormal) << retStr.GetString();

        // Send BackupRestore to update installation has finished
        SendMsgToBackupRestore(INSTALLER_TO_BACKUP_RESTORE_FINISH_IND);

        bInstallFailed = true;
    }

    // ===== 5. update task's state (version installation is done now)

    if (bInstallFailed)
    {
    	// VNGR-10979
    	SendEventToAuditor("Version download failed");

    	// VNGR-11014
    	configMngrapi.RemountVersionPartition(TRUE);
    	configMngrapi.DeleteTempFiles();
    	configMngrapi.RemountVersionPartition(FALSE);
    	
    	SendConfBlockToConfParty(FALSE);	//release conference block in case of a failure

    	m_pProcess->SetInstallationStatus(retStatus);
    	m_state = READY;
    	TRACESTR(eLevelInfoNormal) << "CInstallerManager::OnInstallerFinishInstallation Set state status READY (bInstallFailed)";
	if(curProductType == eProductTypeGesher || curProductType == eProductTypeNinja)
	{
		LedUpgradeFailed();
	}
    }
	else
	{
		//VNGR-24993, after instalation finished, delete the flag file to import the default IVR conf from StaticCfg after reset.
		BOOL res = DeleteFile(DEFAULT_IVR_CONFIG_IMPORTED.c_str(),false);
		TRACESTR(eLevelInfoNormal) << "CInstallerManager::OnInstallerFinishInstallation : Deleting the file : "
                                << DEFAULT_IVR_CONFIG_IMPORTED << "; status = " << res << "\n";

		if(curProductType == eProductTypeGesher)
		{
			if(STATUS_CFS_MSG_10_WARNING_MASKED != retStatus && STATUS_CFS_MSG_11_WARNING_MASKED != retStatus)
			{
				GesherInstallationSoftwareDone();

				m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_ipmcBurning,eStatusSuccess);
				m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_completed,eStatusSuccess);
			
				SendFlushtoLogger();
				m_installFlowFinished = true;
				
				AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
						NEW_VERSION_INSTALLED,
						MAJOR_ERROR_LEVEL,
						"A new version was installed. Reset the MCU",
						true,
						true
				);

				DeleteFile(SYSTEM_AFTER_VERSION_INSTALLED_FILE);
				LedUpgradeComplete();
				StartTimer(RESET_TIMEOUT, 30 * SECOND );
			}
		}
	}

    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::DistributeNewVersionToAll()
{
    if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
    {
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::DistributeNewVersionToAll";
		CConfigManagerApi apiConfigurator;
		apiConfigurator.ExposeEmbeddedNewVersion();

		CManagerApi apiCards(eProcessCards);

		//apiCards.SendOpcodeMsg(INSTALLER_NEW_VERSION_IS_READY_TO_CARDS);

        CSegment *pSeg = new CSegment;
        *pSeg << (DWORD) m_isNewVersionBlockedInMPMRX; // Rachel need to block downgrade of MPMRX to 7.x versions
        apiCards.SendMsg(pSeg, INSTALLER_NEW_VERSION_IS_READY_TO_CARDS);

		StartTimer(TOUT_CARDS_BEGIN_INSTALL, CARDS_BEGIN_INSTALL);


			// tell mcms daemon that ipmc trigger stop stop from now...

		CManagerApi apiMcmsDaemon(eProcessMcmsDaemon);
		apiMcmsDaemon.SendOpcodeMsg(INSTALLER_STOP_IPMC_WD);
    }

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::OnInstallationTimeoutInstall()
{
    TRACESTR(eLevelInfoNormal) << "CInstallerManager::OnInstallationTimeoutInstall Set m_state status READY";
    m_state = READY;
    PASSERTMSG(TRUE, "Timeout reached during installation flow!");
    SendMsgToBackupRestore(INSTALLER_TO_BACKUP_RESTORE_FINISH_IND);

    //InstallationFinished();
	m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_ipmcBurning,sStatusFailure);
	m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_completed,sStatusFailure);

	AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
			NEW_VERSION_INSTALLED,
			MAJOR_ERROR_LEVEL,
			"Version installation did not finished, MCU will be reset, installation will continue after reset",
			true,
			true
	);
	PASSERTMSG(TRUE, "Software installation will finished at next reset");
	CreateFile(SYSTEM_AFTER_VERSION_INSTALLED_FILE);

	CManagerApi apiCards(eProcessCards);
	apiCards.SendOpcodeMsg(INSTALLER_RESET_ALL_TO_CARDS);

	StartTimer(RESET_TIMEOUT, 30 * SECOND );

	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::OnInstallationTimeoutReady()
{
    TRACESTR(eLevelInfoNormal) << "CInstallerManager::OnInstallationTimeoutReady";
    SendMsgToBackupRestore(INSTALLER_TO_BACKUP_RESTORE_FINISH_IND);
    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::GetTrailerInfo(TRAILER_INFO_S& TrailerInfoStruct,
                                         std::string version_name)
{
    // check existence
    BOOL isExist = IsFileExists(version_name);
    PASSERTSTREAM_AND_RETURN_VALUE(!isExist,
        version_name << ": No such file or directory",
        STATUS_FILE_NOT_EXISTS);

    // open file
    FILE* fileVer = fopen(version_name.c_str(), "r");
    PASSERTSTREAM_AND_RETURN_VALUE(NULL == fileVer,
        "Unable to open file " << version_name << ": " << strerror(errno),
        STATUS_FILE_OPEN_ERROR);

    // we position the indicator to TRAILER_MAX_SIZE bytes backwards from EOF
    fseek(fileVer, -TRAILER_MAX_SIZE, SEEK_END);

    char *pRowContent = new char[MAX_ROW_LEN];
    char *pTrailerInfo = NULL;
    WORD InfoLine = 0,EndOfInfo = NO;

    while(!EndOfInfo && (fgets(pRowContent,MAX_ROW_LEN,fileVer) != NULL) )
    {
        if(!InfoLine) //First info line was not found yet.
        {
            pTrailerInfo = strstr(pRowContent,"TrailerVersion:");
            if(pTrailerInfo)
            {
                //Move pFirmwareVersion to the TrailerVersion number data.
                pTrailerInfo += sizeof("TrailerVersion:");
                TrailerInfoStruct.TrailerVersion = pTrailerInfo;

                //Remove the '\n' character which resides last
                if(TrailerInfoStruct.TrailerVersion.size())
                    TrailerInfoStruct.TrailerVersion.erase(TrailerInfoStruct.TrailerVersion.end()-1);
                InfoLine++;
            }
        }
        else //First info line was found. Retrieve other info lines
        {
        	  TRACESTR(eLevelInfoNormal) << "CInstallerManager::GetTrailerInfo "<< TrailerInfoStruct.TrailerVersion;


            if(TrailerInfoStruct.TrailerVersion == "1.0.0")
                ParseTrailer_V_1_0_0(InfoLine, pRowContent, EndOfInfo, pTrailerInfo, TrailerInfoStruct);
            else if(TrailerInfoStruct.TrailerVersion == "1.0.1")
                ParseTrailer_V_1_0_1(InfoLine, pRowContent, EndOfInfo, pTrailerInfo, TrailerInfoStruct);
            else if(TrailerInfoStruct.TrailerVersion == "1.1.0")
                ParseTrailer_V_1_0_0(InfoLine, pRowContent, EndOfInfo, pTrailerInfo, TrailerInfoStruct); //Gesher&Ninja Trailer's first part is same as "1.0.0" of RMX2000 
        }
    }


    //***************************************************************************************************
    // VNGR-18736 26.12.10 added by Rachel Cohen
    // Trailer Info Changes - in version V7.5 we added 4 new fields :
    // 1. SubTrailerVersion
    // 2. EmbSize
    // 3. EmbMd5Sum
    // 4. NoTrailerSHA1SUM
    // ParseTrailer_V_1_0_2 and VerifyTrailerInfoStruct_V_1_0_2 will handle that delta
    //****************************************************************************************************
    TRACESTR(eLevelInfoNormal) << "CInstallerManager::GetTrailerInfo pRowContent "<< pRowContent;
  TrailerInfoStruct.SubTrailerVersion = "";
  pTrailerInfo = strstr(pRowContent,"SubTrailerVersion:");
  WORD InfoSubLine = 0;
  if(pTrailerInfo)
  {
	  TRACESTR(eLevelInfoNormal) << "CInstallerManager::GetTrailerInfo pTrailerInfo "<< pTrailerInfo;
	  //Move pFirmwareVersion to the TrailerVersion number data.
	  pTrailerInfo += sizeof("SubTrailerVersion:");
	  TrailerInfoStruct.SubTrailerVersion = pTrailerInfo;

	  //Remove the '\n' character which resides last
	  if(TrailerInfoStruct.SubTrailerVersion.size())
		  TrailerInfoStruct.SubTrailerVersion.erase(TrailerInfoStruct.SubTrailerVersion.end()-1);
	  InfoLine++;
	  InfoSubLine++;

	  EndOfInfo = NO;
	  while(!EndOfInfo && (fgets(pRowContent,MAX_ROW_LEN,fileVer) != NULL) )
	  {
	         if(TrailerInfoStruct.SubTrailerVersion == "1.0.2")
	                ParseTrailer_V_1_0_2(InfoLine, pRowContent, EndOfInfo, pTrailerInfo, TrailerInfoStruct);
	         else if(TrailerInfoStruct.SubTrailerVersion == "1.1.0")
	                ParseSubTrailer_V_1_1_0(InfoLine, InfoSubLine, pRowContent, EndOfInfo, pTrailerInfo, TrailerInfoStruct);
	  }
  }
  else  TRACESTR(eLevelInfoNormal) << "CInstallerManager::GetTrailerInfo - pTrailerInfo is NULL "<< pRowContent;



  TRACESTR(eLevelInfoNormal) << "CInstallerManager::GetTrailerInfo pRowContent after SubTrailerVersion section"<< pRowContent;
TrailerInfoStruct.Sub2TrailerVersion = "";
pTrailerInfo = strstr(pRowContent,"Sub2TrailerVersion:");
WORD InfoSub2Line = 0;
if(pTrailerInfo)
{
	  TRACESTR(eLevelInfoNormal) << "CInstallerManager::GetTrailerInfo pTrailerInfo Sub2TrailerVersion"<< pTrailerInfo;
	  //Move pFirmwareVersion to the TrailerVersion number data.
	  pTrailerInfo += sizeof("Sub2TrailerVersion:");
	  TrailerInfoStruct.Sub2TrailerVersion = pTrailerInfo;

	  //Remove the '\n' character which resides last
	  if(TrailerInfoStruct.Sub2TrailerVersion.size())
		  TrailerInfoStruct.Sub2TrailerVersion.erase(TrailerInfoStruct.Sub2TrailerVersion.end()-1);
	  InfoLine++;
	  InfoSub2Line++;


	  EndOfInfo = NO;
	  while(!EndOfInfo && (fgets(pRowContent,MAX_ROW_LEN,fileVer) != NULL) )
	  {
	         if(TrailerInfoStruct.Sub2TrailerVersion == "1.0.3")
	                ParseTrailer_V_1_0_3(InfoLine, pRowContent, EndOfInfo, pTrailerInfo, TrailerInfoStruct);
	         if(TrailerInfoStruct.Sub2TrailerVersion == "1.1.0")
	                ParseSub2Trailer_V_1_1_0(InfoLine, InfoSub2Line, pRowContent, EndOfInfo, pTrailerInfo, TrailerInfoStruct);
	  }
}
else  TRACESTR(eLevelInfoNormal) << "CInstallerManager::GetTrailerInfo - pTrailerInfo is NULL for Sub2TrailerVersion"<< pRowContent;



    fclose(fileVer);

    delete[] pRowContent;

	STATUS result = STATUS_FAIL;
	STATUS oldResult=STATUS_FAIL;
	STATUS newResult=STATUS_FAIL;

	if(TrailerInfoStruct.TrailerVersion == "1.0.0")
		oldResult = VerifyTrailerInfoStruct_V_1_0_0(TrailerInfoStruct);
	else if(TrailerInfoStruct.TrailerVersion == "1.0.1")
		oldResult = VerifyTrailerInfoStruct_V_1_0_1(TrailerInfoStruct);
	else if(TrailerInfoStruct.TrailerVersion == "1.1.0")
		oldResult = VerifyTrailerInfoStruct_V_1_0_0(TrailerInfoStruct);  // Gesher&Ninja's first part is same as RMX2000

	if(TrailerInfoStruct.SubTrailerVersion == "")
		newResult=STATUS_OK;
	else if(TrailerInfoStruct.SubTrailerVersion == "1.0.2")
		newResult = VerifyTrailerInfoStruct_V_1_0_2(TrailerInfoStruct);

	else if(TrailerInfoStruct.SubTrailerVersion == "1.0.3")  //that should be removed just to fix the bug of 7.6.0.85
		result= STATUS_OK;

	else if(TrailerInfoStruct.SubTrailerVersion == "1.1.0")  //that should be removed just to fix the bug of 7.6.0.85
		newResult = VerifySubTrailerInfoStruct_V_1_1_0(TrailerInfoStruct);
	
	//That line should not be removed - it is for future versions.
	else 	newResult = VerifyTrailerInfoStruct_V_1_0_2(TrailerInfoStruct);


	if(TrailerInfoStruct.Sub2TrailerVersion == "1.0.3")
		newResult = VerifyTrailerInfoStruct_V_1_0_3(TrailerInfoStruct);
	else if(TrailerInfoStruct.Sub2TrailerVersion == "1.1.0")
		newResult = VerifySub2TrailerInfoStruct_V_1_1_0(TrailerInfoStruct);

	  TRACESTR(eLevelInfoNormal) << "CInstallerManager::GetTrailerInfo oldResult "<< oldResult << "newResult " <<newResult;
	if ((oldResult==STATUS_OK) && (newResult==STATUS_OK))
		result= STATUS_OK;


	return result;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::VerifyTrailerInfoStruct_V_1_0_0(TRAILER_INFO_S TrailerInfoStruct)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_0";

	if( TrailerInfoStruct.FirmwareVersion.size() &&
		TrailerInfoStruct.FirmwareDate.size()    &&
		TrailerInfoStruct.KernelSize.size()      &&
		TrailerInfoStruct.KernelMD5SUM.size()    &&
		TrailerInfoStruct.NoTrailerSize.size()   &&
		TrailerInfoStruct.NoTrailerMD5SUM.size()  )
		return STATUS_OK ; //All struct fields must contain info
	else
		return STATUS_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::VerifyTrailerInfoStruct_V_1_0_1(TRAILER_INFO_S TrailerInfoStruct)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_1";

	// all struct fields must contain info
	if( TrailerInfoStruct.FirmwareVersion.size() &&
		TrailerInfoStruct.FirmwareDate.size()    &&
		TrailerInfoStruct.KernelSize.size()      &&
		TrailerInfoStruct.KernelMD5SUM.size()    &&
		TrailerInfoStruct.SquashFsSize.size()    &&
		TrailerInfoStruct.SquashFsMD5SUM.size()  &&
		TrailerInfoStruct.NoTrailerSize.size()   &&
		TrailerInfoStruct.NoTrailerMD5SUM.size() )
		return STATUS_OK;

	return STATUS_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::VerifyTrailerInfoStruct_V_1_0_2(TRAILER_INFO_S TrailerInfoStruct)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_2";

	if ( TrailerInfoStruct.SubTrailerVersion.size())
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_2 SubTrailerVersion OK";

	if ( TrailerInfoStruct.EmbSize.size())
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_2 EmbSize OK";

	if ( TrailerInfoStruct.EmbMd5SUM.size())
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_2  EmbMd5SUM OK";

	if ( TrailerInfoStruct.NoTrailerSHA1SUM.size())
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_2 NoTrailerSHA1SUM  OK";

	// all struct fields must contain info
	if( TrailerInfoStruct.SubTrailerVersion.size()	&&
		TrailerInfoStruct.EmbSize.size()		 &&
		TrailerInfoStruct.EmbMd5SUM.size()		 &&
		TrailerInfoStruct.NoTrailerSHA1SUM.size())
		return STATUS_OK;

	return STATUS_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::VerifyTrailerInfoStruct_V_1_0_3(TRAILER_INFO_S TrailerInfoStruct)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_3";

	if ( TrailerInfoStruct.Sub2TrailerVersion.size())
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_3 Sub2TrailerVersion OK";

	if ( TrailerInfoStruct.NewSqFsSize.size())
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_3 NewSqFsSize OK";

	if ( TrailerInfoStruct.NewSqFsMD5SUM.size())
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_3  NewSqFsMD5SUM OK";


	// all struct fields must contain info
	if( TrailerInfoStruct.Sub2TrailerVersion.size()	&&
		TrailerInfoStruct.NewSqFsSize.size()		 &&
		TrailerInfoStruct.NewSqFsMD5SUM.size())
		return STATUS_OK;

	return STATUS_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::VerifySubTrailerInfoStruct_V_1_1_0(TRAILER_INFO_S TrailerInfoStruct)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifySubTrailerInfoStruct_V_1_1_0";

	if ( TrailerInfoStruct.SubTrailerVersion.size())
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifySubTrailerInfoStruct_V_1_1_0 SubTrailerVersion OK";

	if ( TrailerInfoStruct.NoTrailerSHA1SUM.size())
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifySubTrailerInfoStruct_V_1_1_0 NoTrailerSHA1SUM  OK";

	// all struct fields must contain info
	if( TrailerInfoStruct.SubTrailerVersion.size()	&&
		TrailerInfoStruct.NoTrailerSHA1SUM.size())
		return STATUS_OK;

	return STATUS_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::VerifySub2TrailerInfoStruct_V_1_1_0(TRAILER_INFO_S TrailerInfoStruct)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_3";

	if ( TrailerInfoStruct.Sub2TrailerVersion.size())
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_3 Sub2TrailerVersion OK";

	if ( TrailerInfoStruct.NewSqFsSize.size())
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_3 NewSqFsSize OK";

	if ( TrailerInfoStruct.NewSqFsMD5SUM.size())
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_3  NewSqFsMD5SUM OK";

	if ( TrailerInfoStruct.ProductType.size())
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::VerifyTrailerInfoStruct_V_1_0_3  NewSqFsMD5SUM OK";

	// all struct fields must contain info
	if( TrailerInfoStruct.Sub2TrailerVersion.size()	&&
		TrailerInfoStruct.NewSqFsSize.size()		 &&
		TrailerInfoStruct.NewSqFsMD5SUM.size()	&&
		TrailerInfoStruct.ProductType.size())
		return STATUS_OK;

	return STATUS_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
void CInstallerManager::ParseTrailer_V_1_0_0(WORD &InfoLine, char *pRowContent, WORD &EndOfInfo, char *pTrailerInfo, TRAILER_INFO_S& TrailerInfoStruct)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::ParseTrailer_V_1_0_0";
	switch(InfoLine)
	{

		case 1:
		{
			pTrailerInfo = strstr(pRowContent,"FirmwareVersion:");
			if(pTrailerInfo)
			{
				//Move pFirmwareVersion to the version number data.
				pTrailerInfo += sizeof("FirmwareVersion:");
				TrailerInfoStruct.FirmwareVersion = pTrailerInfo;

				//Remove the '\n' character which resides last
				if(TrailerInfoStruct.FirmwareVersion.size())
					TrailerInfoStruct.FirmwareVersion.erase(TrailerInfoStruct.FirmwareVersion.end()-1);
				InfoLine++;
			}
			else
				TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_0 - fail to find FirmwareVersion !";

			break;
		}

		case 2:
		{
			pTrailerInfo = strstr(pRowContent,"FirmwareDate:");

			if(pTrailerInfo)
			{
				//Move pTrailerInfo to the FirmwareDate data.
				pTrailerInfo += sizeof("FirmwareDate:");
				TrailerInfoStruct.FirmwareDate = pTrailerInfo;
				//Remove the '\n' character which resides last
				if(TrailerInfoStruct.FirmwareDate.size())
					TrailerInfoStruct.FirmwareDate.erase(TrailerInfoStruct.FirmwareDate.end()-1);
				InfoLine++;
			}
			else
				TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_0 - fail to find FirmwareDate !";

			break;
		}

		case 3:
		{
			pTrailerInfo = strstr(pRowContent,"KernelSize:");

			if(pTrailerInfo)
			{
				//Move pTrailerInfo to the KernelSize data.
				pTrailerInfo += sizeof("KernelSize:");
				TrailerInfoStruct.KernelSize = pTrailerInfo;
				//Remove the '\n' character which resides last
				if(TrailerInfoStruct.KernelSize.size())
					TrailerInfoStruct.KernelSize.erase(TrailerInfoStruct.KernelSize.end()-1);
				InfoLine++;
			}
			else
				TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_0 - fail to find KernelSize !";

			break;
		}

		case 4:
		{
			pTrailerInfo = strstr(pRowContent,"KernelMD5SUM:");

			if(pTrailerInfo)
			{
				//Move pTrailerInfo to the KernelMD5SUM data.
				pTrailerInfo += sizeof("KernelMD5SUM:");
				TrailerInfoStruct.KernelMD5SUM = pTrailerInfo;
				//Remove the '\n' character which resides last
				if(TrailerInfoStruct.KernelMD5SUM.size())
					TrailerInfoStruct.KernelMD5SUM.erase(TrailerInfoStruct.KernelMD5SUM.end()-1);
				InfoLine++;
			}
			else
				TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_0 - fail to find KernelMD5SUM !";

			break;
		}

		case 5:
		{
			pTrailerInfo= strstr(pRowContent,"NoTrailerSize:");

			if(pTrailerInfo)
			{
				//Move pTrailerInfo to the NoTrailerSize data.
				pTrailerInfo += sizeof("NoTrailerSize:");
				TrailerInfoStruct.NoTrailerSize = pTrailerInfo;
				//Remove the '\n' character which resides last
				if(TrailerInfoStruct.NoTrailerSize.size())
					TrailerInfoStruct.NoTrailerSize.erase(TrailerInfoStruct.NoTrailerSize.end()-1);
				InfoLine++;
			}
			else
				TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_0 - fail to find NoTrailerSize !";

			break;
		}

		case 6:
		{
			pTrailerInfo = strstr(pRowContent,"NoTrailerMD5SUM:");

			if(pTrailerInfo)
			{
				//Move pTrailerInfo to the NoTrailerMD5SUM data.
				pTrailerInfo += sizeof("NoTrailerMD5SUM:");
				TrailerInfoStruct.NoTrailerMD5SUM = pTrailerInfo;
				//Remove the '\n' character which resides last
				if(TrailerInfoStruct.NoTrailerMD5SUM.size())
					TrailerInfoStruct.NoTrailerMD5SUM.erase(TrailerInfoStruct.NoTrailerMD5SUM.end()-1);
				InfoLine++;
			}
			else
				TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_0 - fail to find NoTrailerMD5SUM !";

			break;
		}

		default:
		{
			EndOfInfo = YES;
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CInstallerManager::ParseTrailer_V_1_0_1(WORD &InfoLine, char *pRowContent, WORD &EndOfInfo, char *pTrailerInfo, TRAILER_INFO_S& TrailerInfoStruct)
{
	TRACESTR(eLevelInfoNormal) << "CInstallerManager::ParseTrailer_V_1_0_1 InfoLine "<< InfoLine <<" \n" <<pRowContent;
	switch(InfoLine)
	{

		case 1:
		{
			pTrailerInfo = strstr(pRowContent,"FirmwareVersion:");
			if(pTrailerInfo)
			{
				//Move pFirmwareVersion to the version number data.
				pTrailerInfo += sizeof("FirmwareVersion:");
				TrailerInfoStruct.FirmwareVersion = pTrailerInfo;

				//Remove the '\n' character which resides last
				if(TrailerInfoStruct.FirmwareVersion.size())
					TrailerInfoStruct.FirmwareVersion.erase(TrailerInfoStruct.FirmwareVersion.end()-1);
				InfoLine++;
			}
			else
				TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_1 - fail to find FirmwareVersion !";

			break;
		}
		case 2:
		{
			pTrailerInfo = strstr(pRowContent,"FirmwareDate:");

			if(pTrailerInfo)
			{
				//Move pTrailerInfo to the FirmwareDate data.
				pTrailerInfo += sizeof("FirmwareDate:");
				TrailerInfoStruct.FirmwareDate = pTrailerInfo;
				//Remove the '\n' character which resides last
				if(TrailerInfoStruct.FirmwareDate.size())
					TrailerInfoStruct.FirmwareDate.erase(TrailerInfoStruct.FirmwareDate.end()-1);
				InfoLine++;
			}
			else
				TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_1 - fail to find FirmwareDate !";

			break;
		}

		case 3:
		{
			pTrailerInfo = strstr(pRowContent,"KernelSize:");

			if(pTrailerInfo)
			{
				//Move pTrailerInfo to the KernelSize data.
				pTrailerInfo += sizeof("KernelSize:");
				TrailerInfoStruct.KernelSize = pTrailerInfo;
				//Remove the '\n' character which resides last
				if(TrailerInfoStruct.KernelSize.size())
					TrailerInfoStruct.KernelSize.erase(TrailerInfoStruct.KernelSize.end()-1);
				InfoLine++;
			}
			else
				TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_1 - fail to find KernelSize !";

			break;
		}

		case 4:
		{
			pTrailerInfo = strstr(pRowContent,"KernelMD5SUM:");

			if(pTrailerInfo)
			{
				//Move pTrailerInfo to the KernelMD5SUM data.
				pTrailerInfo += sizeof("KernelMD5SUM:");
				TrailerInfoStruct.KernelMD5SUM = pTrailerInfo;
				//Remove the '\n' character which resides last
				if(TrailerInfoStruct.KernelMD5SUM.size())
					TrailerInfoStruct.KernelMD5SUM.erase(TrailerInfoStruct.KernelMD5SUM.end()-1);
				InfoLine++;
			}
			else
				TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_1 - fail to find KernelMD5SUM !";

			break;
		}

		case 5:
		{
			pTrailerInfo= strstr(pRowContent,"SquashFsSize:");

			if(pTrailerInfo)
			{
				//Move pTrailerInfo to the NoTrailerSize data.
				pTrailerInfo += sizeof("SquashFsSize:");
				TrailerInfoStruct.SquashFsSize = pTrailerInfo;
				//Remove the '\n' character which resides last
				if(TrailerInfoStruct.SquashFsSize.size())
					TrailerInfoStruct.SquashFsSize.erase(TrailerInfoStruct.SquashFsSize.end()-1);
				InfoLine++;
			}
			else
				TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_1 - fail to find SquashFsSize !";

			break;
		}

		case 6:
		{
			pTrailerInfo = strstr(pRowContent,"SquashFsMD5SUM:");

			if(pTrailerInfo)
			{
				//Move pTrailerInfo to the NoTrailerMD5SUM data.
				pTrailerInfo += sizeof("SquashFsMD5SUM:");
				TrailerInfoStruct.SquashFsMD5SUM = pTrailerInfo;
				//Remove the '\n' character which resides last
				if(TrailerInfoStruct.SquashFsMD5SUM.size())
					TrailerInfoStruct.SquashFsMD5SUM.erase(TrailerInfoStruct.SquashFsMD5SUM.end()-1);
				InfoLine++;
			}
			else
				TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_1 - fail to find SquashFsMD5SUM !";

			break;
		}

		case 7:
		{
			pTrailerInfo= strstr(pRowContent,"NoTrailerSize:");

			if(pTrailerInfo)
			{
				//Move pTrailerInfo to the NoTrailerSize data.
				pTrailerInfo += sizeof("NoTrailerSize:");
				TrailerInfoStruct.NoTrailerSize = pTrailerInfo;
				//Remove the '\n' character which resides last
				if(TrailerInfoStruct.NoTrailerSize.size())
					TrailerInfoStruct.NoTrailerSize.erase(TrailerInfoStruct.NoTrailerSize.end()-1);
				InfoLine++;
			}
			else
				TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_1 - fail to find NoTrailerSize !";

			break;
		}

		case 8:
		{
			pTrailerInfo = strstr(pRowContent,"NoTrailerMD5SUM:");

			if(pTrailerInfo)
			{
				//Move pTrailerInfo to the NoTrailerMD5SUM data.
				pTrailerInfo += sizeof("NoTrailerMD5SUM:");
				TrailerInfoStruct.NoTrailerMD5SUM = pTrailerInfo;
				//Remove the '\n' character which resides last
				if(TrailerInfoStruct.NoTrailerMD5SUM.size())
					TrailerInfoStruct.NoTrailerMD5SUM.erase(TrailerInfoStruct.NoTrailerMD5SUM.end()-1);
				InfoLine++;
			}
			else
				TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_1 - fail to find NoTrailerMD5SUM !";

			break;
		}

		default:
		{
			EndOfInfo = YES;
			break;
		}
	}
}



/////////////////////////////////////////////////////////////////////////////
void CInstallerManager::ParseTrailer_V_1_0_2(WORD &InfoLine, char *pRowContent, WORD &EndOfInfo, char *pTrailerInfo, TRAILER_INFO_S& TrailerInfoStruct)
{
	TRACESTR(eLevelInfoNormal) << "CInstallerManager::ParseTrailer_V_1_0_2 InfoLine "<< InfoLine <<" \n" <<pRowContent;
	switch(InfoLine)
	{

	case 10:
	{
		pTrailerInfo= strstr(pRowContent,"EmbSize:");

		if(pTrailerInfo)
		{
			//Move pTrailerInfo to the NoTrailerSize data.
			pTrailerInfo += sizeof("EmbSize:");
			TrailerInfoStruct.EmbSize = pTrailerInfo;
			//Remove the '\n' character which resides last
			if(TrailerInfoStruct.EmbSize.size())
				TrailerInfoStruct.EmbSize.erase(TrailerInfoStruct.EmbSize.end()-1);
			InfoLine++;
		}
		else
			TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_2 - fail to find EmbSize !";

		break;
	}

	case 11:
	{
		pTrailerInfo= strstr(pRowContent,"EmbMd5SUM:");

		if(pTrailerInfo)
		{
			//Move pTrailerInfo to the NoTrailerSize data.
			pTrailerInfo += sizeof("EmbMd5SUM:");
			TrailerInfoStruct.EmbMd5SUM = pTrailerInfo;
			//Remove the '\n' character which resides last
			if(TrailerInfoStruct.EmbMd5SUM.size())
				TrailerInfoStruct.EmbMd5SUM.erase(TrailerInfoStruct.EmbMd5SUM.end()-1);
			InfoLine++;
		}
		else
			TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_2 - fail to find EmbMd5SUM !";

		break;
	}
	case 12:
	{
		pTrailerInfo = strstr(pRowContent,"NoTrailerSHA1SUM:");

		if(pTrailerInfo)
		{
			//Move pTrailerInfo to the NoTrailerSHA1SUM data.
			pTrailerInfo += sizeof("NoTrailerSHA1SUM:");
			TrailerInfoStruct.NoTrailerSHA1SUM = pTrailerInfo;
			//Remove the '\n' character which resides last
			if(TrailerInfoStruct.NoTrailerSHA1SUM.size())
				TrailerInfoStruct.NoTrailerSHA1SUM.erase(TrailerInfoStruct.NoTrailerSHA1SUM.end()-1);
			InfoLine++;
		}
		else
		{
			TRACESTR(eLevelError)
			<< "\nCInstallerManager::ParseTrailer_V_1_0_2 - fail to find NoTrailerSHA1SUM !";
		}
		break;
	}


	default:
	{
		EndOfInfo = YES;
		break;
	}

	}
}

/////////////////////////////////////////////////////////////////////////////
void CInstallerManager::ParseTrailer_V_1_0_3(WORD &InfoLine, char *pRowContent, WORD &EndOfInfo, char *pTrailerInfo, TRAILER_INFO_S& TrailerInfoStruct)
{
	TRACESTR(eLevelInfoNormal) << "CInstallerManager::ParseTrailer_V_1_0_3 InfoLine "<< InfoLine <<" \n" <<pRowContent;
	switch(InfoLine)
	{



	case 14:
	{
		pTrailerInfo= strstr(pRowContent,"NewSqFsSize:");

		if(pTrailerInfo)
		{
			//Move pTrailerInfo to the NoTrailerSize data.
			pTrailerInfo += sizeof("NewSqFsSize:");
			TrailerInfoStruct.NewSqFsSize = pTrailerInfo;
			//Remove the '\n' character which resides last
			if(TrailerInfoStruct.NewSqFsSize.size())
				TrailerInfoStruct.NewSqFsSize.erase(TrailerInfoStruct.NewSqFsSize.end()-1);
			InfoLine++;
		}
		else
			TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_3 - fail to find NewSqFsSize !";

		break;
	}

	case 15:
	{
		pTrailerInfo= strstr(pRowContent,"NewSqFsMD5SUM:");

		if(pTrailerInfo)
		{
			//Move pTrailerInfo to the NoTrailerSize data.
			pTrailerInfo += sizeof("NewSqFsMD5SUM:");
			TrailerInfoStruct.NewSqFsMD5SUM = pTrailerInfo;
			//Remove the '\n' character which resides last
			if(TrailerInfoStruct.NewSqFsMD5SUM.size())
				TrailerInfoStruct.NewSqFsMD5SUM.erase(TrailerInfoStruct.NewSqFsMD5SUM.end()-1);
			InfoLine++;
		}
		else
			TRACESTR(eLevelError) << "\nCInstallerManager::ParseTrailer_V_1_0_3 - fail to find NewSqFsMD5SUM !";

		break;
	}



	default:
	{
		EndOfInfo = YES;
		break;
	}

	}
}

/////////////////////////////////////////////////////////////////////////////
void CInstallerManager::ParseSubTrailer_V_1_1_0(WORD &InfoLine, WORD &InfoSubLine, char *pRowContent, WORD &EndOfInfo, char *pTrailerInfo, TRAILER_INFO_S& TrailerInfoStruct)
{
	TRACESTR(eLevelInfoNormal) << "CInstallerManager::ParseSubTrailer_V_1_1_0 InfoLine "<< InfoLine <<" \n" <<pRowContent;
	switch(InfoSubLine)
	{
	case 1:
	{
		pTrailerInfo = strstr(pRowContent,"NoTrailerSHA1SUM:");

		if(pTrailerInfo)
		{
			//Move pTrailerInfo to the NoTrailerSHA1SUM data.
			pTrailerInfo += sizeof("NoTrailerSHA1SUM:");
			TrailerInfoStruct.NoTrailerSHA1SUM = pTrailerInfo;
			//Remove the '\n' character which resides last
			if(TrailerInfoStruct.NoTrailerSHA1SUM.size())
				TrailerInfoStruct.NoTrailerSHA1SUM.erase(TrailerInfoStruct.NoTrailerSHA1SUM.end()-1);
			InfoLine++;
			InfoSubLine++;
		}
		else
		{
			TRACESTR(eLevelError)
			<< "\nCInstallerManager::ParseSubTrailer_V_1_1_0 - fail to find NoTrailerSHA1SUM !";
		}
		break;
	}


	default:
	{
		EndOfInfo = YES;
		break;
	}

	}
}

/////////////////////////////////////////////////////////////////////////////
void CInstallerManager::ParseSub2Trailer_V_1_1_0(WORD &InfoLine, WORD &InfoSub2Line, char *pRowContent, WORD &EndOfInfo, char *pTrailerInfo, TRAILER_INFO_S& TrailerInfoStruct)
{
	TRACESTR(eLevelInfoNormal) << "CInstallerManager::ParseSub2Trailer_V_1_1_0 InfoLine "<< InfoLine <<" \n" <<pRowContent;
	switch(InfoSub2Line)
	{

	case 1:
	{
		pTrailerInfo= strstr(pRowContent,"NewSqFsSize:");

		if(pTrailerInfo)
		{
			//Move pTrailerInfo to the NoTrailerSize data.
			pTrailerInfo += sizeof("NewSqFsSize:");
			TrailerInfoStruct.NewSqFsSize = pTrailerInfo;
			//Remove the '\n' character which resides last
			if(TrailerInfoStruct.NewSqFsSize.size())
				TrailerInfoStruct.NewSqFsSize.erase(TrailerInfoStruct.NewSqFsSize.end()-1);
			InfoLine++;
			InfoSub2Line++;
		}
		else
			TRACESTR(eLevelError) << "\nCInstallerManager::ParseSub2Trailer_V_1_1_0 - fail to find NewSqFsSize !";

		break;
	}

	case 2:
	{
		pTrailerInfo= strstr(pRowContent,"NewSqFsMD5SUM:");

		if(pTrailerInfo)
		{
			//Move pTrailerInfo to the NoTrailerSize data.
			pTrailerInfo += sizeof("NewSqFsMD5SUM:");
			TrailerInfoStruct.NewSqFsMD5SUM = pTrailerInfo;
			//Remove the '\n' character which resides last
			if(TrailerInfoStruct.NewSqFsMD5SUM.size())
				TrailerInfoStruct.NewSqFsMD5SUM.erase(TrailerInfoStruct.NewSqFsMD5SUM.end()-1);
			InfoLine++;
			InfoSub2Line++;
		}
		else
			TRACESTR(eLevelError) << "\nCInstallerManager::ParseSub2Trailer_V_1_1_0 - fail to find NewSqFsMD5SUM !";

		break;
	}

	case 3:
	{
		pTrailerInfo= strstr(pRowContent,"ProductType:");

		if(pTrailerInfo)
		{
			//Move pTrailerInfo to the NoTrailerSize data.
			pTrailerInfo += sizeof("ProductType:");
			TrailerInfoStruct.ProductType= pTrailerInfo;
			//Remove the '\n' character which resides last
			if(TrailerInfoStruct.ProductType.size())
				TrailerInfoStruct.ProductType.erase(TrailerInfoStruct.ProductType.end()-1);
			InfoLine++;
			InfoSub2Line++;
		}
		else
			TRACESTR(eLevelError) << "\nCInstallerManager::ParseSub2Trailer_V_1_1_0 - fail to find ProductType !";

		break;
	}

	default:
	{
		EndOfInfo = YES;
		break;
	}

	}
}

STATUS CInstallerManager::CheckProductTypeValidity(std::string & productTypeList, std::string & product)
{
	size_t found=productTypeList.find(product);
	if (found!=string::npos){
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::CheckProductTypeValidity - Find ProductType: " << product 
			<< "  ProductTypeList: "<< productTypeList;
		return STATUS_OK;
	}
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::CheckProductTypeValidity - Failed to find ProductType: " << product 
			<< "  ProductTypeList: "<< productTypeList;
	return STATUS_FAIL;
}

STATUS CInstallerManager::CheckSHA1(const std::string& stamp,
                                    const std::string& fname,
                                    const std::string& size)
{
    // Keeps backward compatibility
    TRACECOND_AND_RETURN_VALUE(stamp.empty(),
      "SHA1 stamp is empty in " << fname,
      STATUS_OK);

    std::string hex;
    STATUS stat = CalculateFileSHA1Value(fname, size, hex);
    if (STATUS_OK != stat)
        return stat;

    // Checksum is OK
    if (stamp == hex)
        return STATUS_OK;

    PASSERTSTREAM(true,
        "Checksum SHA1 mismatch, file: " << fname
            << ", stamp: " << stamp << ", hex: " << hex);

    return STATUS_FAIL;
}

// This function takes 6 seconds. Sometimes WD to daemon does not work
// so we unlock the semaphore before "read()" from file function.
STATUS CInstallerManager::CalculateFileSHA1Value(const std::string& fname,
                                                 const std::string& size,
                                                 std::string& out)
{
    // how many bytes we should calculate their SHA1 sum
    DWORD NumOfBytesToRead = size.empty() ? 0 : atoi(size.c_str());
    PASSERTMSG_AND_RETURN_VALUE(NumOfBytesToRead == 0,
        "Unable to calculate SHA1: no bytes are asked to be read",
        STATUS_FAIL);

    BOOL isExist = IsFileExists(fname);
    PASSERTSTREAM_AND_RETURN_VALUE(!isExist,
        fname << ": No such file or directory",
        STATUS_FILE_NOT_EXISTS);

    // open file
    FILE* fp = fopen(fname.c_str(), "rb");

    PASSERTSTREAM_AND_RETURN_VALUE(NULL == fp,
        "Unable to open file " << fname << ": " << strerror(errno),
        STATUS_FILE_OPEN_ERROR);

    SHA_CTX ctx;
    int res = SHA1_Init(&ctx);
    if (1 != res)
    {
        PASSERTSTREAM(true,
            "SHA1_Init failed with " << res
                << ": " << CSslFunctions::SSLErrMsg());
        fclose(fp);
        return STATUS_FAIL;
    }

    DWORD OrigNumToRead = NumOfBytesToRead;
    DWORD ActualReadSize = 0;
    DWORD ReadSize = (NumOfBytesToRead < BUFFER_SIZE) ? NumOfBytesToRead : BUFFER_SIZE;

    unsigned char buf[BUFFER_SIZE];
    while ((ActualReadSize = ReadUnlockedSemaphore(buf, 1, ReadSize, fp)))
    {
        // file read failed
        if (ActualReadSize != ReadSize)
        {
            PASSERTSTREAM(true,
                "Unable to read file " << fname
                    << ": " << strerror(errno));
            fclose(fp);
            return STATUS_FAIL;
        }

        // file read succeeded
        res = SHA1_Update(&ctx, buf, ActualReadSize);
        if (1 != res)
        {
            PASSERTSTREAM(true,
                "SHA1_Update failed with " << res
                    << ": " << CSslFunctions::SSLErrMsg());
            fclose(fp);
            return STATUS_FAIL;
        }

        NumOfBytesToRead -= ActualReadSize;

        if(NumOfBytesToRead == 0)
            break;

        if(NumOfBytesToRead < ReadSize)
            ReadSize = NumOfBytesToRead;
    }

    unsigned char sha1buf[SHA_DIGEST_LENGTH];
    res = SHA1_Final(sha1buf, &ctx);
    if (1 != res)
    {
        PASSERTSTREAM(true,
            "SHA1_Final failed with " << res
                << ": " << CSslFunctions::SSLErrMsg());
        fclose(fp);
        return STATUS_FAIL;
    }

    // we need to convert binary formatted result to Hex string formatted
    DWORD* psha1buf = (DWORD*)sha1buf;

    char hex[5 * 8 + 1];
    snprintf(hex, sizeof(hex),
            "%08x%08x%08x%08x%08x", htonl(*(psha1buf)),
                                    htonl(*(psha1buf + 1)),
                                    htonl(*(psha1buf + 2)),
                                    htonl(*(psha1buf + 3)),
                                    htonl(*(psha1buf + 4)));
    fclose(fp);

    FTRACEINTO << "Checksum SHA1 of " << OrigNumToRead
               << " bytes in " << fname
               << " is " << hex;

    out = hex;
    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
int CInstallerManager::ReadUnlockedSemaphore(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	int read =-1;
	if (m_isUnderTDD)
		read = fread(ptr, size, nmemb, stream);
	else
	{
		UnlockRelevantSemaphore();
		read = fread(ptr, size, nmemb, stream);
		LockRelevantSemaphore();
	}
    return read;
}

/////////////////////////////////////////////////////////////////////////////
void CInstallerManager::OnInstallerValidateKeyCode(CSegment *pSeg)
{
	STATUS retStatus = STATUS_OK;
	std::string statusDesc = "";


	if(!pSeg)
	{
		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnInstallerValidateKeyCode"
									  << "\nInput segment is NULL";
		return ;
	}

	char msgkeycode[EXACT_KEYCODE_LENGTH_IN_GL1+1];
	*pSeg >> msgkeycode;
	msgkeycode[EXACT_KEYCODE_LENGTH_IN_GL1] = '\0';

	FTRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnInstallerValidateKeyCode"
								   << "\nThe keycode is "<< msgkeycode;

	CKeyCode keyCode(msgkeycode) ;
	CKeyCode* pkeyCode = &keyCode;

	CSmallString serialNumStr;
	serialNumStr << m_pCfsParamsStruct->mplSerialNumber;

	//if on startup we got popup request of activation key
	//we need to calculate with current version .
	//if its an upgrade flow we will check the keycode with the trailer version
	if (m_handleReceiveVersionStarted == false)
	{
		retStatus = pkeyCode->Validate(m_productType, serialNumStr);

		if (STATUS_OK==retStatus)
		{
			int failReason = 0;
			retStatus = ValidateKeyCode(m_productType, msgkeycode, failReason);
			if (STATUS_OK != retStatus)
				TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnInstallerValidateKeyCode - invalid activation key, reason=[" << failReason << "]";
		}

		TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnInstallerValidateKeyCode - status: " << retStatus;
		if (STATUS_OK == retStatus)
		{
			//remove AA
			RemoveActiveAlarmByErrorCode(ACTIVATION_KEY_IS_NEEDED);
			CheckMultipleServicesBit(pkeyCode);
			retStatus = TreatKeyCodeUpdate_notVersionUpgrade(pkeyCode);
		}
	}
	else
	{	
		retStatus = TreatKeyCodeUpdate_versionUpgrade(pkeyCode);
		TRACESTR(eLevelInfoNormal) << "\nTreatKeyCodeUpdate_versionUpgrade - status: " << retStatus;
		
		if (STATUS_OK == retStatus)
		{
			RemoveActiveAlarmByErrorCode(ACTIVATION_KEY_IS_NEEDED);
		}
	}


	// send the status and statusDesc to Installer monitor
	SendUpdateKeyCodeStatusToInstallerMonitor(retStatus,statusDesc);

	return ;
}

void CInstallerManager::CheckMultipleServicesBit(CKeyCode* pkeyCode)
{
	char szOptions[CFS_OPTIONS_BITMASK_LENGTH];
	memset(szOptions, 0, CFS_OPTIONS_BITMASK_LENGTH);
	pkeyCode->GetOptionsFromKeyCode( szOptions, (char*)(pkeyCode->GetKeyCode().GetString()) );
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::CheckMultipleServicesBit : Options is "<< szOptions;
	assert(strlen(szOptions)<=8);
	unsigned long ulOption = strtoul(szOptions, NULL, 16);
	int const isMultipleServicesOn = IsBitOn(ulOption, keycode::MULTIPLE_SERVICES_BIT);
	if (!isMultipleServicesOn)
	{
		TRACESTR(eLevelInfoNormal) << "Disable multiple services in license, Check system flag of multiple services";
		BOOL isMultipleServices = NO;
		std::string key_multiple_services = CFG_KEY_MULTIPLE_SERVICES;
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		sysConfig->GetBOOLDataByKey(key_multiple_services, isMultipleServices);
		if(YES == isMultipleServices)
		{
			CCfgData * cfgData = NULL;
			//update configuration in memory
			CSysMap* memoryMap = sysConfig->GetMap();
			CSysMap::iterator iTer = memoryMap->find(key_multiple_services);
			if ( memoryMap->end() != iTer)
			{
				cfgData = (CCfgData*)iTer->second;
				cfgData->SetData("NO");
			}

			//load from SystemCfgUser.xml and write back with change
			CSysConfigEma* cfgFile = new CSysConfigEma();
			cfgFile->LoadFromFile(eCfgParamUser);
			//update flag
			if (cfgFile->IsParamExist(key_multiple_services))
			{
				cfgData = cfgFile->GetCfgEntryByKey(key_multiple_services);
				if (cfgData->GetData() != "NO" ) // && (cfgDataMemory->GetIsReset() == true))
				{
					cfgData->SetData("NO");
					//write to file
					TRACEINTO << "save into SystemCfgUserTmp.xml: " << key_multiple_services << endl;
					cfgFile->SaveToFile(eCfgParamUser);
					TRACEINTO << "done saving into SystemCfgUserTmp.xml: " << key_multiple_services << endl;
				}
			}
			//end load from SystemCfgUserTmp.xml and write back with change
		}
	}

}


void CInstallerManager::SendUpdateKeyCodeStatusToInstallerMonitor(STATUS status , std::string  description)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::SendUpdateKeyCodeStatusToInstallerMonitor";

	CProcessBase * process = CProcessBase::GetProcess();
	CSegment *pSeg = new CSegment;
	*pSeg << (WORD)status;
	*pSeg << description.c_str();
	CTaskApi installerMonitorApi(eProcessInstaller,eMonitor);

	CSegment rspMsg;
	OPCODE resOpcode;


	STATUS responseStatus = installerMonitorApi.SendMessageSync(pSeg,INSTALLER_MONITOR_UPDATE_KEY_STATUS_PARAMS,100,resOpcode, rspMsg);
	if (responseStatus != STATUS_OK)
	{
		PTRACE2INT(eLevelInfoNormal,"\nCInstallerManager::OnInstallerValidateKeyCode: Failed to send Status to Installer monitor status=:",responseStatus);
	}
	return ;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::HandleUpdateKeyCode(CRequest* pSetRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::HandleUpdateKeyCode - new keycode received";

	STATUS retStatus = STATUS_OK;
	std::string statusDesc = "";

	if (pSetRequest->GetAuthorization() == SUPER)
	{
		// ===== 1. extract keycode from segment received
		CKeyCode* pkeyCode = (CKeyCode*)(pSetRequest->GetRequestObject());

			// ===== 3. treat the keyCode
			char keyCodeType = pkeyCode->GetKeyCodeType();
			if ('X' == keyCodeType)
			{
			    /*************************************************************************************/
				/* 24.11.10 VNGR-17768 added by Rachel Cohen                                         */
				/* when insert activation key ema log out . the timer in the Ema expired since the   */
				/* validation action take more time In Rmx1500 .                                     */
				/*************************************************************************************/

				// ===== 1.send the status STATUS_IN_PROGRESS and decription "" to Installer monitor to init its params
				SendUpdateKeyCodeStatusToInstallerMonitor(STATUS_IN_PROGRESS,statusDesc);

				// ===== 2.Self message - to continue validate the key code
				CMedString keycodeStr = pkeyCode->GetKeyCode();

				TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::HandleUpdateKeyCode"
				<< "\nKey: " << keycodeStr.GetString();

				char keycode[EXACT_KEYCODE_LENGTH_IN_GL1+1];
				strncpy(keycode, keycodeStr.GetString(), EXACT_KEYCODE_LENGTH_IN_GL1);
				keycode[EXACT_KEYCODE_LENGTH_IN_GL1] = '\0';

				TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::HandleUpdateKeyCode"
				<< "\nKey: " << keycode;


				CSegment* pParam = new CSegment();
				*pParam << keycode;

				CManagerApi api(eProcessInstaller);

				api.SendMsg(pParam, INSTALLER_VALIDATE_KEY_CODE);


				// ===== 3.  send response to client
				retStatus = STATUS_IN_PROGRESS ;
				pSetRequest->SetStatus(retStatus); //must be STATUS_OK
				pSetRequest->SetConfirmObject(new CDummyEntry());


				return STATUS_OK;

			}

			else
			{
				retStatus = STATUS_CFS_MSG_1; // "Invalid Activation Key"
				TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::HandleUpdateKeyCode - invalid keycode type: " << keyCodeType;
			}

//		} // end validation succeeded
	} // end if SuperUser

	else // Authorization != SUPER
	{
		retStatus = STATUS_NO_PERMISSION;

		// print to log
		CSmallString traceStr = "\nCInstallerManager::HandleUpdateKeyCode: No permission to update key code;";
		char* authorizationGroupTypeStr = ::AuthorizationGroupTypeToString( pSetRequest->GetAuthorization() );
		if (authorizationGroupTypeStr)
		{
			traceStr << " Authorization Group: " << authorizationGroupTypeStr;
		}
		else
		{
			traceStr << " Authorization Group: (invalid: " << pSetRequest->GetAuthorization() << ")";
		}
		TRACESTR(eLevelInfoNormal) << traceStr.GetString();
	}


	if(STATUS_VERSION_ALREADY_INSTALLED == retStatus)
    {
		retStatus = STATUS_CFS_MSG_8;
		pSetRequest->SetExDescription("Version already installed");
    }

	pSetRequest->SetStatus(retStatus);
	pSetRequest->SetConfirmObject(new CDummyEntry());

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::TreatKeyCodeUpdate_notVersionUpgrade(CKeyCode* pkeyCode)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::TreatKeyCodeUpdate_notVersionUpgrade";
	STATUS retStat = STATUS_OK;

	VERSION_S mcuVersion;

	/*Not an upgrade - use current version for mcu version*/
	memset(&mcuVersion, 0, sizeof(VERSION_S));
	mcuVersion = m_pCfsParamsStruct->verFromKeycode;

	SendUpdateCfsKeyCodeReqToMplApi( pkeyCode->GetKeyCode(), mcuVersion );	// update MPL

	if (false == m_isKeyCodeFailure) // reset is needed
	{
		AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
	                             NEW_ACTIVATION_KEY_LOADED,
	                             MAJOR_ERROR_LEVEL,
	                             "A new activation key was loaded. Reset the MCU",
	                             true,
	                             true
	                           );

		retStat = STATUS_CFS_MSG_0_WARNING_MASKED;//"Activation Key was loaded successfully. Reset the MCU to activate"
	}

	else // original keycode was invalid (bad or reset); the new keycode is accepted without resetting
	{
		m_isKeyCodeFailure = false;
		SendUpdatedKeyCodeToMcuMngr(pkeyCode);

		retStat = STATUS_OK;
	}

	return retStat;
}

STATUS CInstallerManager::TreatKeyCodeUpdate_versionUpgrade(CKeyCode* pkeyCode)
{
	STATUS retStatus = STATUS_OK;
	CLargeString retStr = "\nCInstallerManager::TreatKeyCodeUpdate_versionUpgrade ";
	TRACESTR(eLevelInfoNormal) << retStr.GetString();
    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
    if (m_isUnderTDD)
    	curProductType = eProductTypeRMX2000;
	TRAILER_INFO_S TrailerInfoStruct;
	retStatus = GetTrailerInfo(TrailerInfoStruct,NEW_VERSION_NAME);

	if (STATUS_OK == retStatus)
	{
		retStatus = CheckSHA1(TrailerInfoStruct.NoTrailerSHA1SUM,
				NEW_VERSION_NAME,
				TrailerInfoStruct.NoTrailerSize);



		//***********************************************************************************
		// 4.9.12 Rachel Cohen
		  // from version 7.8 the encryption will be in SHA256 we need to reset the
		  // user/password file in case we downgrade to versions 7.5 7.6 7.7
		  //**********************************************************************************
		string firmVersion = TrailerInfoStruct.FirmwareVersion;



		string subVersion = firmVersion.substr(4, 3);
		string ver7_5 = "7.5";
		string ver7_6 = "7.6";
		string ver7_7 = "7.7";
		string ver7_8 = "7.8";


		TRACESTR(eLevelInfoNormal) << ", firmVersion = " << firmVersion << ", subVersion = " << subVersion;

		if ((subVersion == ver7_5) || (subVersion == ver7_6) || (subVersion == ver7_7) || (subVersion == ver7_8))

		{

			CManagerApi apiAuthentication(eProcessAuthentication);
			apiAuthentication.SendOpcodeMsg(INSTALLER_AUTHETICATION_REMOVE_ENC_OPERATOR_FILE);


			AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
					SHA1_VERSION,
					MAJOR_ERROR_LEVEL,
                    GetAlarmDescription(SHA1_VERSION),
					true,
					true
			);




		}







	}
	else
	{
            TRACESTR(eLevelError) << "CInstallerManager::TreatKeyCodeUpdate_versionUpgrade(): fail to retrieve version name";
	}


	if (STATUS_OK == retStatus) // new version exists
	{
		if(curProductType == eProductTypeGesher || curProductType == eProductTypeNinja)
		{
		    LedUpgradeInProgress();
		}
		
		retStr << "\n---- Checking Mcu version vs. current version ";
		TRACESTR(eLevelInfoNormal) << retStr.GetString();

		VERSION_S mcuVersion = ConvertStringToVersionStruct(TrailerInfoStruct.FirmwareVersion);

		CSmallString serialNumStr;
		serialNumStr << m_pCfsParamsStruct->mplSerialNumber;
		retStatus = pkeyCode->Validate(m_productType, serialNumStr, (int)mcuVersion.ver_major, (int)mcuVersion.ver_minor);
		if (STATUS_OK==retStatus)
		{
			int failReason = 0;
			retStatus = ValidateKeyCode(m_productType, pkeyCode->GetKeyCode().GetString(),failReason,(int)mcuVersion.ver_major, (int)mcuVersion.ver_minor);
			if (STATUS_OK != retStatus)
				TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::TreatKeyCodeUpdate_versionUpgrade - invalid activation key, reason=[" << failReason << "]";
		}

		if (STATUS_OK == retStatus)
		{

			if(FALSE == IsTarget()  && eProductTypeGesher != curProductType && eProductTypeNinja != curProductType)
							{
								PTRACE(eLevelInfoNormal,"CInstallerManager::TreatKeyCodeUpdate_versionUpgrade m_state = READY ");
								m_state = READY;
								m_pProcess->ResetSoftwareInstall();

								CManagerApi apiCards(eProcessCards);
								// apiCards.SendOpcodeMsg(INSTALLER_NEW_VERSION_IS_READY_TO_CARDS);

								CSegment *pSeg = new CSegment;
								*pSeg << (DWORD) m_isNewVersionBlockedInMPMRX; // Rachel need to block downgrade of MPMRX to 7.x versions
								apiCards.SendMsg(pSeg, INSTALLER_NEW_VERSION_IS_READY_TO_CARDS);

								SendMsgToBackupRestore(INSTALLER_TO_BACKUP_RESTORE_FINISH_IND);
								return retStatus;
							}

			retStatus = AcceptVersion(TrailerInfoStruct.FirmwareVersion, NO);

			if (STATUS_CFS_MSG_8 != retStatus) // means: AcceptVersion has succeeded
			{
				// update task's state (it will be in INSTALLING state until version installation is done)
				m_state = INSTALLING;
				TRACESTR(eLevelInfoNormal) << "Set state status INSTALLING";

				SendUpdateCfsKeyCodeReqToMplApi( pkeyCode->GetKeyCode(), mcuVersion );	// update MPL

				//STATUS_OK means we can go ahead with new installation flow
				STATUS tmpStatus;
                
                if(eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
                {
                    tmpStatus = STATUS_OK;
                }
                else
                {
  					tmpStatus = CompareTrailerVersions(TrailerInfoStruct.TrailerVersion);                        
                }

				if((STATUS_OK == tmpStatus) &&
				(eProductTypeGesher != curProductType) &&
				(eProductTypeNinja != curProductType))
				{
					retStatus = STATUS_OK;
					StartTimer(INSTALLER_TIMEOUT_INSTALATION, TIMEOUT_INSTALATION );
					TRACESTR(eLevelInfoNormal) << "CInstallerManager::TreatKeyCodeUpdate_versionUpgrade start timer INSTALLER_TIMEOUT_INSTALATION";
					DistributeNewVersionToAll();
				}
				else if(curProductType == eProductTypeGesher)
				{
			
					GesherInstallationSoftwareDone();

					m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_ipmcBurning,eStatusSuccess);
					m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_completed,eStatusSuccess);
					
					SendFlushtoLogger();
					m_installFlowFinished = true;
					
					AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
							NEW_VERSION_INSTALLED,
							MAJOR_ERROR_LEVEL,
							"A new version was installed. Reset the MCU",
							true,
							true
					);
					m_state = READY;
					TRACESTR(eLevelInfoNormal) << "CInstallerManager::TreatKeyCodeUpdate_versionUpgrade Set m_state status READY";
					DeleteFile(SYSTEM_AFTER_VERSION_INSTALLED_FILE);
					DeleteTimer(INSTALLER_TIMEOUT_INSTALATION);
					LedUpgradeComplete();
					StartTimer(RESET_TIMEOUT, 30 * SECOND );
					TRACESTR(eLevelInfoNormal) << "CInstallerManager::TreatKeyCodeUpdate_versionUpgrade delete timer INSTALLER_TIMEOUT_INSTALATION";
				}
				else if(curProductType == eProductTypeNinja)
				{
					retStatus = STATUS_OK;  // no need reset.
					m_fpgaRetryCount = 1;
					// swloading complete
					GesherInstallationSoftwareDone();
					SendFPGAUpgradeToConfig();
				}
				//old installation flow
				else
				{
					AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
							NEW_VERSION_INSTALLED,
							MAJOR_ERROR_LEVEL,
							"A new version was installed. Reset the MCU",
							true,
							true);
					m_state = READY;
					TRACESTR(eLevelInfoNormal) << "CInstallerManager::TreatKeyCodeUpdate_versionUpgrade Set m_state status READY";

					m_pProcess->SetInstallationStatus(retStatus);
					DeleteTimer(INSTALLER_TIMEOUT_INSTALATION);
					TRACESTR(eLevelInfoNormal) << "CInstallerManager::TreatKeyCodeUpdate_versionUpgrade delete timer INSTALLER_TIMEOUT_INSTALATION";
				}
			}
			else
			{
				if(curProductType == eProductTypeGesher || curProductType == eProductTypeNinja)
				{
				    LedUpgradeFailed();
				}
			}

		}
		else
		{
			retStatus = STATUS_CFS_MSG_1;//"Invalid Activation Key"

			retStr << "\n---- keycode validation failed! ";
			TRACESTR(eLevelInfoNormal) << retStr.GetString();
			if(curProductType == eProductTypeGesher || curProductType == eProductTypeNinja)
			{
			    LedUpgradeFailed();
			}
		}
    }

    else // new version doesn't exist
    {
//    	retStatus = STATUS_CFS_MSG_5_WARNING_MASKED;//"Version number upgrade completed successfully"
		retStatus = STATUS_CFS_MSG_1;//"Invalid Activation Key"

		retStr << "\n---- New version doesn't exist ";
 		TRACESTR(eLevelInfoNormal) << retStr.GetString();
 	}

	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::TreatKeyCodeUpdate_versionUpgrade"
	                       << "\n---- Status: " << m_pProcess->GetStatusAsString(retStatus).c_str();

	return retStatus;
}

STATUS CInstallerManager::CheckNew_U_KeyCodeVsOriginalKeycode(VERSION_S newKcVersion)
{
	STATUS retStatus = STATUS_OK;
	CLargeString retStr = "\nCInstallerManager::Treat_U_KeyCodeUpdate - CheckNew_U_KeyCodeVsOriginalKeycode ";

	VERSION_S originalVersion = m_pCfsParamsStruct->verFromKeycode;

	retStr << "\nOriginal version: "     << originalVersion.ver_major << "." << originalVersion.ver_minor
		   << ", New keycode version: "  << newKcVersion.ver_major    << "." << newKcVersion.ver_minor;


	// ===== compare version numbers
	if ( (newKcVersion.ver_major > originalVersion.ver_major) ||	// newKcVersion >= originalVersion
	     ((newKcVersion.ver_major == originalVersion.ver_major) && (newKcVersion.ver_minor >= originalVersion.ver_minor)) )
	{
		retStr << "\n---- Versions ok";
	}

	else // newKcVersion < oldKcVersion
	{
		//retStatus = STATUS_KEYCODES_VERSIONS_MISMATCH;
		retStatus = STATUS_CFS_MSG_3_WARNING_MASKED;
		retStr << "\n---- Version in new keyCode is smaller than original version!";
	}

	retStr << "\n---- Status: " << m_pProcess->GetStatusAsString(retStatus).c_str();
	TRACESTR(eLevelInfoNormal) << retStr.GetString();

	return retStatus;
}

////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::CheckCurrentVersionVsMcuVersion(VERSION_S mcuVer, BOOL isInstallVersion)
{
	STATUS retStatus = STATUS_OK;
	CLargeString retStr = "\nCInstallerManager::CheckCurrentVersionVsMcuVersion ";

	VERSION_S curVersion = m_pCfsParamsStruct->verFromKeycode;
	retStr << "\nCurrent version: " << curVersion.ver_major << "." << curVersion.ver_minor
		   << ", Mcu version: "     << mcuVer.ver_major     << "." << mcuVer.ver_minor;

	// ===== compare version numbers
	if ( (curVersion.ver_major == mcuVer.ver_major) &&
		 (curVersion.ver_minor == mcuVer.ver_minor) )
	{
		retStr << "\n---- Versions ok ";
	}

    // downgrade - Licensing backward compatibility
    //if this is a downgrade, and the current version is 8.0 and up and new version is 7.x or down
    //we do not support Licensing backward compatibility cause the ports in v8.0 is count in HD
    else if ((((curVersion.ver_major > mcuVer.ver_major)  ||
            ((curVersion.ver_major == mcuVer.ver_major) && (curVersion.ver_minor > mcuVer.ver_minor)))
            && (((curVersion.ver_major >=8 ) && (mcuVer.ver_major <= 7)) )))
    {
        retStr << "\n---- Current version is different from the one of the installed version! ";

        // response to EMA
        if (YES == isInstallVersion)
        {
            //add AA
            AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
                        ACTIVATION_KEY_IS_NEEDED,
                        MAJOR_ERROR_LEVEL,
                        GetAlarmDescription(ACTIVATION_KEY_IS_NEEDED),
                        true,
                        true
                );
            retStatus = STATUS_CFS_MSG_10_WARNING_MASKED;//"Software version loaded. A matching Activation Key is required"
        }

        else
            retStatus = STATUS_CFS_MSG_11_WARNING_MASKED;//"Activation key loaded successfully but it matches a lower software version number"

    }
	//support for downgrade - Licensing backward compatibility
	//if this is a downgrade, and the new version is 7.6 or more
	//(eature is new in 7.6 and supported since this version
	else if ((((curVersion.ver_major > mcuVer.ver_major)  ||
			((curVersion.ver_major == mcuVer.ver_major) && (curVersion.ver_minor > mcuVer.ver_minor)))
			&& (((mcuVer.ver_major == 7) && (mcuVer.ver_minor >= 6)) ||(mcuVer.ver_major > 7))))
	{
		retStr << "\n---- Versions ok for downgrade ";
	}

	else // curVersion != newMcuVersion
	{
		retStr << "\n---- Current version is different from the one of the installed version! ";

		// response to EMA
		if (YES == isInstallVersion)
		{
			//add AA
			AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
					    ACTIVATION_KEY_IS_NEEDED,
						MAJOR_ERROR_LEVEL,
						GetAlarmDescription(ACTIVATION_KEY_IS_NEEDED),
						true,
						true
				);
			retStatus = STATUS_CFS_MSG_10_WARNING_MASKED;//"Software version loaded. A matching Activation Key is required"
		}

		else
			retStatus = STATUS_CFS_MSG_11_WARNING_MASKED;//"Activation key loaded successfully but it matches a lower software version number"
	}

	retStr << "\n---- Status: " << m_pProcess->GetStatusAsString(retStatus).c_str();
	TRACESTR(eLevelInfoNormal) << retStr.GetString();

	return retStatus;
}

////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::CompareTrailerVersions(std::string newTrailerVersion)
{
	STATUS result = STATUS_OK;
	TRAILER_INFO_S fallbackTrailerInfoStruct;
	BOOL isExist = IsFileExists(FALLBACK_VERSION_NAME);
	if (isExist)
	{
		result = GetTrailerInfo(fallbackTrailerInfoStruct,FALLBACK_VERSION_NAME);

		if(STATUS_OK == result)
		{
			std::string oldTrailerVersion = fallbackTrailerInfoStruct.TrailerVersion;

			if(newTrailerVersion == "1.0.0" && oldTrailerVersion == "1.0.1")
				result = STATUS_FAIL;
			if(newTrailerVersion == "1.0.1" && oldTrailerVersion == "1.0.1")
				result = STATUS_OK;
		}
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////
//STATUS CInstallerManager::CheckKeyCodeVersionVsMcuVersion(VERSION_S verFromKc, VERSION_S mcuVer, BOOL isInstallVersion)
//{
//	STATUS retStatus = STATUS_OK;
//	CLargeString retStr = "\nCInstallerManager::CheckKeyCodeVersionVsMcuVersion - CheckNewKeyCodeVsNewVersion ";
//
//	retStr << "\nKeycode version: " << verFromKc.ver_major << "." << verFromKc.ver_minor
//		   << ", Mcu version: "     << mcuVer.ver_major    << "." << mcuVer.ver_minor;
//
//	// ===== compare version numbers
//	if ( (verFromKc.ver_major > mcuVer.ver_major) ||	// verFromKc >= mcuVer
//	     ((verFromKc.ver_major == mcuVer.ver_major) && (verFromKc.ver_minor >= mcuVer.ver_minor)) )
//	{
//		retStr << "\n---- Versions ok ";
//	}
//
//	else // newKcVersion < newMcuVersion
//	{
//		retStr << "\n---- Version in keyCode is smaller than mcu version! ";
//
//		// response to EMA
//		if (YES == isInstallVersion)
//			//retStatus = STATUS_NEW_MCU_VERSION_MISMATCHES_KEYCODE_VERSION;
//			retStatus = STATUS_CFS_MSG_10_WARNING_MASKED;
//
//		else
//			//retStatus = STATUS_NEW_KEYCODE_VERSION_MISMATCHES_MCU_VERSION;
//			retStatus = STATUS_CFS_MSG_11_WARNING_MASKED;
//	}
//
//	retStr << "\n---- Status: " << m_pProcess->GetStatusAsString(retStatus).c_str();
//	TRACESTR(eLevelInfoNormal) << retStr.GetString();
//
//	return retStatus;
//}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerManager::AcceptVersion(const std::string & new_version_name, BOOL isInstallVersion)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::AcceptVersion; version name: " << new_version_name;

	STATUS retStatus = STATUS_OK;

	CConfigManagerApi api;
	api.RemountVersionPartition(TRUE,FALSE);
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::AcceptVersion - finish Remount(TRUE), start CycleVersions";

	retStatus = api.CycleVersions(new_version_name);
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::AcceptVersion - finish CycleVersions ---- "
	                       << "cycling status: " << m_pProcess->GetStatusAsString(retStatus).c_str()
	                       << " ---- start Remount(FALSE)";

	api.RemountVersionPartition(FALSE, TRUE);
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::AcceptVersion - finish Remount(FALSE)";

	if (STATUS_OK == retStatus) // cycling succeeded
	{
		if (YES == isInstallVersion)
		{
			retStatus = STATUS_CFS_MSG_6_WARNING_MASKED;//"Version is loaded successfully. Reset the MCU to activate"
		}
		else
		{
			retStatus = STATUS_CFS_MSG_7_WARNING_MASKED;//"Activation Key and version are loaded successfully. Reset the MCU to activate"
		}
		std::string audit_text = "New version accepted: ";
		audit_text += new_version_name;
		
		SendEventToAuditor(audit_text.c_str());

		// ===== create a file for next startup, to indicate that a new version is loaded
    	BOOL res = CreateFile(SYSTEM_AFTER_VERSION_INSTALLED_FILE);
        TRACESTR(eLevelInfoNormal) << "CInstallerManager::AcceptVersion : Creating the file : "
                               << SYSTEM_AFTER_VERSION_INSTALLED_FILE << "; status = " << res << "\n";

        if (TRUE != res)
        {
	        int errCode = errno;
	        CMedString str = "CInstallerManager::AcceptVersion\n";
	        str << "FAILED to create file " << SYSTEM_AFTER_VERSION_INSTALLED_FILE << "\n"
	            << "Errno : " << errCode;
	        TRACESTR(eLevelInfoNormal) << str.GetString();
        }
	}


	else // cycling failed
	{
//		if (YES == isInstallVersion)
		if(STATUS_VERSION_ALREADY_INSTALLED != retStatus)
        {
			retStatus = STATUS_CFS_MSG_8;//"Version load failed"
        }
//
//		else
//			retStatus = STATUS_CFS_MSG_9_WARNING_MASKED;//"Activation key loaded successfully but software version load failed"
	}

	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::AcceptVersion"
	                       << "\n---- Status: " << m_pProcess->GetStatusAsString(retStatus).c_str();
	return retStatus;
}

/////////////////////////////////////////////////////////////////////////////
void CInstallerManager::SetVersionInCfsParamsStruct(VERSION_S ver)
{
	m_pCfsParamsStruct->verFromKeycode.ver_major	= ver.ver_major;
	m_pCfsParamsStruct->verFromKeycode.ver_minor	= ver.ver_minor;
	m_pCfsParamsStruct->verFromKeycode.ver_release	= ver.ver_release;
	m_pCfsParamsStruct->verFromKeycode.ver_internal	= ver.ver_internal;
}

/////////////////////////////////////////////////////////////////////////////
VERSION_S CInstallerManager::ConvertStringToVersionStruct(const string & versionStr)
{
	VERSION_S versionStruct;
	memset(&versionStruct,0,sizeof(VERSION_S));
	int prefix_len = strlen(VERSION_NAME_PREFIX);
	if(strstr((char*)versionStr.c_str(), AMOS_VERSION_NAME_PREFIX))
		prefix_len = strlen(AMOS_VERSION_NAME_PREFIX);
	if(strstr((char*)versionStr.c_str(), CG_VERSION_NAME_PREFIX))
		prefix_len = strlen(CG_VERSION_NAME_PREFIX);


    char* tmp = (char*)versionStr.c_str() + prefix_len;

    versionStruct.ver_major = atoi(tmp);
    tmp = strchr(tmp,'.') + 1;
    if( tmp )
    {
		versionStruct.ver_minor = atoi(tmp);
		tmp = strchr(tmp,'.') + 1;
		if( tmp )
		{
			versionStruct.ver_release = atoi(tmp);
			tmp = strchr(tmp,'.') + 1;
			if( tmp )
			{
				versionStruct.ver_internal = atoi(tmp);
			}
		}
    }

	return versionStruct;
}

/////////////////////////////////////////////////////////////////////
void CInstallerManager::SendUpdateCfsKeyCodeReqToMplApi(CSmallString keycodeStr, VERSION_S mcuVersion)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::SendUpdateCfsKeyCodeReqToMplApi - KeyCode: " << keycodeStr.GetString();

	CFS_KEYCODE_S keyCodeStruct;
	memset(&keyCodeStruct, 0, sizeof(CFS_KEYCODE_S));
	strncpy((char*) keyCodeStruct.keycode, keycodeStr.GetString(), sizeof(keyCodeStruct.keycode)-1 );
	keyCodeStruct.keycode[sizeof(keyCodeStruct.keycode)-1] = '\0';
	keyCodeStruct.keyCodeVersion = mcuVersion;

	
	KeyCodeSaveLoader keySaveLoader;
	keySaveLoader.SaveKeyCode((char const *)keyCodeStruct.keycode, keyCodeStruct.keyCodeVersion);

	CMplMcmsProtocol *mplPrtcl = new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(MCUMNGR_UPDATE_CFS_KEYCODE_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, m_pCfsParamsStruct->switchBoardId, m_pCfsParamsStruct->switchSubBoardId);
	mplPrtcl->AddData(sizeof(CFS_KEYCODE_S), (char*)&keyCodeStruct);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("INSTALLER_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);

}

/////////////////////////////////////////////////////////////////////
void CInstallerManager::SendUpdatedKeyCodeToMcuMngr(CKeyCode* pkeyCode)
{
	STATUS res = STATUS_OK;
	CMedString keycodeStr = pkeyCode->GetKeyCode();

	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::SendUpdatedKeyCodeToMcuMngr"
						   << "\nKey: " << keycodeStr.GetString();

	char keycode[EXACT_KEYCODE_LENGTH_IN_GL1+1];
	strncpy(keycode, keycodeStr.GetString(), EXACT_KEYCODE_LENGTH_IN_GL1);
	keycode[EXACT_KEYCODE_LENGTH_IN_GL1] = '\0';

	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::SendUpdatedKeyCodeToMcuMngr"
								  << "\nKey: " << keycode;


	// ===== 1. insert host name to the segment
	CSegment* pParam = new CSegment();
	*pParam << keycode;

	// ===== 2. send to McuMngr
	const COsQueue* pMcuMngrMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessMcuMngr, eManager);

	res = pMcuMngrMbx->Send(pParam, INSTALLER_KEYCODE_UPDATE_IND);
}

/////////////////////////////////////////////////////////////////////////////
void  CInstallerManager::AskMcuMngrForCfsParams()
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::AskMcuMngrForCfsParams";
	CSegment*  pRetParam = new CSegment;

	const COsQueue* pMcuMngrMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessMcuMngr, eManager);

	STATUS res = pMcuMngrMbx->Send(pRetParam, INSTALLER_TO_MCUMNGR_CFS_PARAMS_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void CInstallerManager::OnMcuMngrInstallerCfsParamsInd(CSegment* pSeg)
{
	// ===== 1. get the parameters from the structure received
	pSeg->Get( (BYTE*)m_pCfsParamsStruct, sizeof(INSTALLER_CFS_S) );

	m_isCfsParamsReceived = TRUE;
	
	// ===== 2. print the data received
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnMcuMngrInstallerCfsParamsInd: "
	                              << "\nMpl Serial number: " << m_pCfsParamsStruct->mplSerialNumber
	                              << "\nswitchBoardId: "     << m_pCfsParamsStruct->switchBoardId
	                              << "\nswitchSubBoardId: "  << m_pCfsParamsStruct->switchSubBoardId
	                              << "\nMCU Version: "       << m_pCfsParamsStruct->verFromKeycode.ver_major    << "."
	                                                         << m_pCfsParamsStruct->verFromKeycode.ver_minor    << "."
	                                                         << m_pCfsParamsStruct->verFromKeycode.ver_release  << "."
	                                                         << m_pCfsParamsStruct->verFromKeycode.ver_internal << "\n";
}

STATUS CInstallerManager::HandleTerminalCheckNewVersionValidity(CTerminalCommand& command,
                                                                std::ostream& answer)
{
	STATUS retStatus = STATUS_FAIL;

	std::string VersionName;
	VersionName = command.GetToken(eCmdParam1);
	if (VersionName == "Invalide Token")
	{
		answer << "Default new version name: " << NEW_VERSION_NAME << std::endl;
		VersionName = NEW_VERSION_NAME;
	}
	else
    {
			answer << "Version name: " << VersionName << std::endl;
    }


	m_isUnderTDD=false;
	if (command.GetToken(eCmdParam2)=="YES")
	{
		m_isUnderTDD=true;
	}

	TRAILER_INFO_S TrailerInfoStruct;
	retStatus = GetTrailerInfo(TrailerInfoStruct,VersionName.c_str());

	if (retStatus == STATUS_OK)
	{
		retStatus = CheckSHA1(TrailerInfoStruct.NoTrailerSHA1SUM,
                                      VersionName.c_str(),
                                      TrailerInfoStruct.NoTrailerSize);
		if (STATUS_OK == retStatus)
		{
		    if (TrailerInfoStruct.NoTrailerSHA1SUM.empty())
		        answer << "SHA1 stamp is empty\n";
		    else
		    {
		        answer << "Checksum SHA1 is valid\n";
		    }
		}
		else
        {
		    answer << "Checksum SHA1 is mismatch\n";
        }
	}
	else
    {
		answer << "Fail to retrieve trailer information\n";
    }

	return retStatus;
}

STATUS CInstallerManager::HandleTerminalLicensingInfo(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(0 != numOfParams)
	{
		answer << "error: No parameters should be added\n";
		answer << "usage: Bin/McuCmd licensing_info Installer\n";
		return STATUS_FAIL;
	}

	answer << "Serial number: "			<< m_pCfsParamsStruct->mplSerialNumber
	       << "\nswitchBoardId: "		<< m_pCfsParamsStruct->switchBoardId
	       << "\nswitchSubBoardId: "	<< m_pCfsParamsStruct->switchSubBoardId
	       << "\nMCU Version:"			<< m_pCfsParamsStruct->verFromKeycode.ver_major   << "."
	                                    << m_pCfsParamsStruct->verFromKeycode.ver_minor   << "."
	                                    << m_pCfsParamsStruct->verFromKeycode.ver_release << "."
	                                    << m_pCfsParamsStruct->verFromKeycode.ver_internal << "\n";

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CInstallerManager::OnMcuMngrInitKeycodeFailureInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::OnMcuMngrInitKeycodeFailureInd";

	m_isKeyCodeFailure = true;
}

/////////////////////////////////////////////////////////////////////////////////////
void CInstallerManager::SendEventToAuditor(const char* pszMessage)
{
    eAuditEventStatus status = eAuditEventStatusFail;
    string statusDesc;
    string descEx;

    AUDIT_EVENT_HEADER_S outAuditHdr;
    CAuditorApi::PrepareAuditHeader(outAuditHdr,
                                   "",
                                   eMcms,
                                   "",
                                   "",
                                   eAuditEventTypeHttp,
                                   status,
                                   "Install Version",
                                   "",
                                   pszMessage,
                                   pszMessage);
    CFreeData freeData;
    CAuditorApi::PrepareFreeData(freeData,
                                 "",
                                 eFreeDataTypeXml,
                                 "",
                                 "",
                                 eFreeDataTypeText,
                                 "");

    CAuditorApi api;
    api.SendEventMcms(outAuditHdr, freeData);
}

/////////////////////////////////////////////////////////////////////
//NINJA: FPGA Upgrade
STATUS CInstallerManager::FPGAImageHeaderCompare(std::string szFPGAImgPath)
{
    std::string cmd = "sudo "+MCU_MCMS_DIR+"/Scripts/NinjaFPGACompare.sh " + szFPGAImgPath;
    std::string answer;
    std::string upgradeFlagPath = FPGA_FORCE_UPGRADE_FLG;

    if(IsFileExists(upgradeFlagPath.c_str()))
    {
        return STATUS_FAIL;
    }
    
    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CInstallerManager::FPGAImageHeaderCompare :" << cmd << std::endl << "answer:" << answer << ", stat:" << stat;

    if (STATUS_OK == stat && "YES" == answer)
    {
        stat = STATUS_OK;
    }
    else
    {
        stat = STATUS_FAIL;
    }

    TRACESTR(eLevelInfoNormal) <<
		"CInstallerManager::FPGAImageHeaderCompare - compare result: "<< stat;
    
    return stat;
}


/////////////////////////////////////////////////////////////////////////////
namespace
{
#define FPGA_IMAGE_FILE_TMP_PATH    (MCU_TMP_DIR+"/ninja_fpga_image.bin")
#define FPGA_CONFIG_FILE_PATH     ((std::string)(MCU_MRMX_DIR+"/bin/fpga_upgrade/fpga_upgrade.cfg"))
#define FPGA_STARTUP_RETRY_RECORD_FILE_PATH     ((std::string)(MCU_MCMS_DIR+"/States/fpga_startup_retry.txt"))
#define FPGA_RETRY_NUM                       3
#define FPGA_STARTUP_RETRY_NUM         4
#define FPGA_SECTION_NAME           "FPGA"
#define FPGA_ENTRY_RETRY             "retry"
#define FPGA_ENTRY_STARTUP_RETRY     "startup_retry"


     int GetFPGAConfigParam(const char* pszSectionName, const char* pszEntryName, int defInt)
    {
        CProfile pf;
        int defNum = defInt;
        if (!pf.Open(FPGA_CONFIG_FILE_PATH.c_str()))
        {
            goto done;
        }

        defNum = pf.GetInt(pszSectionName, pszEntryName, defInt);

    done:
        return defNum;
    }


    int GetFPGAStartupRetryNumber()
    {
        int retryNum = GetFPGAConfigParam(FPGA_SECTION_NAME, FPGA_ENTRY_STARTUP_RETRY, FPGA_STARTUP_RETRY_NUM);
        if(retryNum < 1) retryNum = 1;

        return retryNum;
    }

     int GetFPGAStartupRetryRecord()
    {
        CProfile pf;
        int retryRecord = 0;
        if (!pf.Open(FPGA_STARTUP_RETRY_RECORD_FILE_PATH.c_str()))
        {
            goto done;
        }

        retryRecord = pf.GetInt(FPGA_SECTION_NAME, FPGA_ENTRY_STARTUP_RETRY, 0);

    done:
        return retryRecord;
    }

     void ClearFPGAStartupRetryRecord()
     {
        DeleteFile(FPGA_STARTUP_RETRY_RECORD_FILE_PATH);
     }
     
     void IncreaseFPGAStartupRetryRecord()
     {
        CProfile pf;
        int currentRecord = GetFPGAStartupRetryRecord();
        if (!pf.Create(FPGA_STARTUP_RETRY_RECORD_FILE_PATH.c_str()))
        {
            return;
        }
        currentRecord++;
        pf.SetInt(FPGA_SECTION_NAME, FPGA_ENTRY_STARTUP_RETRY, currentRecord);
     }

}

void CInstallerManager::TestNinjaFPGAAndAlarm()
{
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	BOOL bNeedFPGABurning = FALSE;
	BOOL isSystemAfterVersionInstall = FALSE;
	std::string fpgaFilePath = FPGA_IMAGE_FILE_PATH;
	int startupRetryNum = FPGA_STARTUP_RETRY_NUM;
	int startupRetryCount = 0;
	if(curProductType != eProductTypeNinja)
	{
		return;
	}

	//Check current FPGA validation
	std::string isFPGAbroken = MCU_TMP_DIR+"/FPGAISSUE";
	if(IsFileExists(isFPGAbroken))
	{
	    
            TRACESTR(eLevelInfoNormal) << "FPGA failure. Please upload and upgrade the software.";                                             
            AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                        FPGA_CHECK_FAILED,
                        MAJOR_ERROR_LEVEL,
                        "FPGA failure. Please upload and upgrade the software.",
                        true, true
            );        
	}

       //Check installation complete.
       /*
       isSystemAfterVersionInstall = IsFileExists(SYSTEM_AFTER_VERSION_INSTALLED_FILE);
	if (TRUE == isSystemAfterVersionInstall)
	{
	       TRACESTR(eLevelInfoNormal) << "isSystemAfterVersionInstall existed. Please wait for FPGA burning to complete.";
              bNeedFPGABurning = TRUE;
              goto done;
	}*/

      //Caculate header MD5
      if(STATUS_OK != FPGAImageHeaderCompare(fpgaFilePath.c_str()))
      {
              TRACESTR(eLevelInfoNormal) << "FPGA on current MCU is different from current image bin. Please wait for FPGA burning to complete.";
              bNeedFPGABurning = TRUE;
              goto done;
      }

done:

      if(bNeedFPGABurning)
      {
            startupRetryNum = GetFPGAStartupRetryNumber();
            startupRetryCount = GetFPGAStartupRetryRecord();

            if(startupRetryCount >= startupRetryNum)
            {
                TRACESTR(eLevelInfoNormal) << "retryCount is no less than retryNum, skip FPGA burning. retryNum : " << startupRetryNum << " retryCount : " << startupRetryCount;
            }
            else
            {
                TRACESTR(eLevelInfoNormal) << "retryCount is less than retryNum, run FPGA burning. retryNum : " << startupRetryNum << " retryCount : " << startupRetryCount;
                TRACESTR(eLevelInfoNormal) << "FPGA failure. Please wait for FPGA burning to complete.";                                             
                AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                            FPGA_STARTUP_BURNING,
                            MAJOR_ERROR_LEVEL,
                            "FPGA failure. Please wait for FPGA burning to complete.",
                            true, true
                );
                StartTimer(FPGA_CHECK_AND_BURNING_TIMER, FPGA_CHECK_AND_BURNING_INTERVAL);
                IncreaseFPGAStartupRetryRecord();
            }
      }
      else
      {
            ClearFPGAStartupRetryRecord();
      }
}

void CInstallerManager::OnFPGAChechAndBurningTimer(CSegment* pSeg)
{

	DeleteTimer(FPGA_CHECK_AND_BURNING_TIMER);
	TRACESTR(eLevelInfoNormal) <<
			"CInstallerManager::OnFPGAChechAndBurningTimer - start fpga burning";

       m_state = INSTALLING;
       m_pProcess->SetInstallationStatus(STATUS_INSTALLATION_IN_PROGRESS);
       LedUpgradeInProgress();
       m_fpgaRetryCount = 1;
       // swloading complete
       GesherInstallationSoftwareDone();
	   std::string fpgaFilePath = FPGA_IMAGE_FILE_PATH;
       BackupFPGAImg(m_fpgaImgPath, fpgaFilePath.c_str());
       SendFPGAUpgradeToConfig();
	
	return;
}
    
int CInstallerManager::GetFPGARetryNumber()
{
    int retryNum = GetFPGAConfigParam(FPGA_SECTION_NAME, FPGA_ENTRY_RETRY, FPGA_RETRY_NUM);
    if(retryNum < 1) retryNum = 1;

    TRACESTR(eLevelInfoNormal) <<
			"CInstallerManager::GetFPGARetryNumber - retryNum : "<< retryNum;

    return retryNum;
}

void CInstallerManager::GesherInstallationSoftwareDone()
{
    // swloading complete
    m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_swLoading,eStatusInProgress);
    m_pProcess->UpdateSoftwareInstallProgress(eInstallPhaseType_swLoading,50);
    SystemSleep(1 * SECOND);
    m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_swLoading,eStatusSuccess);
    m_pProcess->UpdateSoftwareInstallProgress(eInstallPhaseType_swLoading,100);	
}

STATUS CInstallerManager::BackupFPGAImg(std::string & szTargetImgPath, std::string szSrcImgPath)
{
	std::string answerTmp;

	//TODO: Get current FPGA Imag File name
	std::string cmd = "sudo cp -rf "  + szSrcImgPath + " " + FPGA_IMAGE_FILE_TMP_PATH;
	STATUS statTmp = SystemPipedCommand(cmd.c_str(), answerTmp);
	szTargetImgPath = FPGA_IMAGE_FILE_TMP_PATH;
		
	TRACESTR(eLevelInfoNormal) <<
			"CInstallerManager::CopyFPGAImgToTmp - "<< cmd <<" : " << answerTmp;
	
	return statTmp;
}

STATUS CInstallerManager::SendFPGAUpgradeToConfig()
{
    //init m_fpgaUpgradeStatus
    m_fpgaUpgradeStatus.stepTime[eFPGAErase] = 200 * SECOND;
    m_fpgaUpgradeStatus.stepTime[eFPGAUpgrade] = (200 + 160) * SECOND;
    m_fpgaUpgradeStatus.stepTime[eFPGAReadBack] = (200 + 160 + 110) * SECOND;
    m_fpgaUpgradeStatus.oneStepTime[eFPGAErase] = 200 * SECOND;
    m_fpgaUpgradeStatus.oneStepTime[eFPGAUpgrade] = 160 * SECOND;
    m_fpgaUpgradeStatus.oneStepTime[eFPGAReadBack] = 110 * SECOND;
    m_fpgaUpgradeStatus.stepIndex = eFPGAErase;
    m_fpgaUpgradeStatus.currentTime = 0;
    m_fpgaUpgradeStatus.totalTime = m_fpgaUpgradeStatus.stepTime[eFPGAActionMaxNum - 1];
    m_fpgaUpgradeStatus.retryMax = GetFPGARetryNumber();

    DeleteTimer(INSTALLER_TIMEOUT_INSTALATION);

    // Sends asynchronous request to configurator
    CSegment* seg = new CSegment;
    *seg << m_fpgaImgPath;
	
    STATUS stat = CTaskApi::SendMsgWithTrace(eProcessConfigurator,
                                             eManager,
                                             seg,
                                             CONFIGURATOR_FPGA_UPGRADE);
    TRACESTR(eLevelInfoNormal) <<
    	"CInstallerManager::SendFPGAUpgradeToConfig - path: "<<m_fpgaImgPath << " stat: "<< stat;

    return stat;
}

void CInstallerManager::OnFPGAUpgradeProgress(CSegment* pSeg)
{
	DWORD	stepIndex = 0, percent = 0;
	STATUS   stat = STATUS_OK;
	*pSeg >> stepIndex;
	*pSeg >> stat;
	*pSeg >> percent;
	
	TRACESTR(eLevelInfoNormal) <<
			"CInstallerManager::OnFPGAUpgradeProgress - step: "<< stepIndex << " stat: " << stat << " percent: "<< percent;
	
	m_fpgaUpgradeStatus.stepIndex = (eFPGAUpgradeAction)stepIndex;

	if((STATUS_FAIL == stat) && (m_fpgaRetryCount >= m_fpgaUpgradeStatus.retryMax))   // fpga upgrade failed.
	{
	    	m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_ipmcBurning,sStatusFailure);
	    	m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_completed,sStatusFailure);
	
	    	SendEventToAuditor("Version download failed");

	    	CConfigManagerApi configMngrapi;
	    	configMngrapi.RemountVersionPartition(TRUE);
	    	configMngrapi.DeleteTempFiles();
	    	configMngrapi.RemountVersionPartition(FALSE);
	    	
	    	SendConfBlockToConfParty(FALSE);	//release conference block in case of a failure

		DeleteFile(SYSTEM_AFTER_VERSION_INSTALLED_FILE);

	    	m_pProcess->SetInstallationStatus(STATUS_CFS_MSG_12);
	    	m_state = READY;
	    	TRACESTR(eLevelInfoNormal) << "CInstallerManager::OnFPGAUpgradeProgress Set state status READY (bInstallFailed)";
		LedUpgradeFailed();
		
	}
	else if((STATUS_FAIL == stat) && (m_fpgaRetryCount < m_fpgaUpgradeStatus.retryMax))  // fpga upgrade failed but we can retry.
	{
		m_fpgaRetryCount++;
		TRACESTR(eLevelInfoNormal) <<
			"CInstallerManager::OnFPGAUpgradeProgress - fpga upgrade failed.  retry : "<< m_fpgaRetryCount;
		SendFPGAUpgradeToConfig();
	}
	else if(eFPGAActionMaxNum == stepIndex)
	{
		LedUpgradeComplete();
		std::string upgradeFlagPath = FPGA_FORCE_UPGRADE_FLG;
		DeleteFile(upgradeFlagPath.c_str());
		InstallationFinished();
	}
	else
	{
		if((m_fpgaUpgradeStatus.stepIndex >= 0) && (m_fpgaUpgradeStatus.stepIndex < eFPGAActionMaxNum))
		{
		    // Change step percent to real percent
		   m_fpgaUpgradeStatus.currentTime = (m_fpgaUpgradeStatus.stepIndex > 0 ? m_fpgaUpgradeStatus.stepTime[m_fpgaUpgradeStatus.stepIndex - 1] : 0)  
                                                                    + m_fpgaUpgradeStatus.oneStepTime[m_fpgaUpgradeStatus.stepIndex] * percent / 100;
           
                int realPercent = m_fpgaUpgradeStatus.currentTime * 100 / m_fpgaUpgradeStatus.totalTime;
                m_pProcess->UpdateSoftwareInstallStatus(eInstallPhaseType_ipmcBurning,eStatusInProgress);
                m_pProcess->UpdateSoftwareInstallProgress(eInstallPhaseType_ipmcBurning, realPercent);
                TRACESTR(eLevelInfoNormal) <<
                "CInstallerManager::OnFPGAUpgradeProgress - real percent: "<< realPercent;
		}
	}

	return;
}

void CInstallerManager::LedUpgradeInProgress()
{
	string answerTmp;
	string cmd = "sudo "+MCU_MCMS_DIR+"/Bin/LightCli UsbUpgrade usb_upgrade_in_progress &> /dev/null";
	STATUS statTmp = SystemPipedCommand(cmd.c_str(), answerTmp);
	TRACESTR(eLevelInfoNormal) <<
			"CInstallerManager::led_upgrade_in_progress - cmd: "<< cmd << " stat: " << statTmp;
}

void CInstallerManager::LedUpgradeComplete()
{         
	string answerTmp;
	string cmd = "sudo "+MCU_MCMS_DIR+"/Bin/LightCli UsbUpgrade usb_upgrade_completed &> /dev/null";
	STATUS statTmp = SystemPipedCommand(cmd.c_str(), answerTmp);
	TRACESTR(eLevelInfoNormal) <<
			"CInstallerManager::led_upgrade_complete - cmd: "<< cmd << " stat: " << statTmp;
}

void CInstallerManager::LedUpgradeFailed()
{
/*
   	string answerTmp;
	string cmd = "sudo "+MCU_MCMS_DIR+"/Bin/LightCli HotStandby single_mode &> /dev/null";
	STATUS statTmp = SystemPipedCommand(cmd.c_str(), answerTmp);
	TRACESTR(eLevelInfoNormal) <<
			"CInstallerManager::led_upgrade_failed - cmd: "<< cmd << " stat: " << statTmp;
*/
    LedUpgradeComplete();

    CManagerApi api(eProcessFailover);
    STATUS status = api.SendOpcodeMsg(FAILOVER_LED_STATUS_IND);
    TRACESTR(eLevelInfoNormal) <<
    			"CInstallerManager::LedUpgradeFailed - send FAILOVER_LED_STATUS_IND to Failover.  status: "<< status;
}


/////////////////////////////////////////////////////////////////////
void CInstallerManager::DumpScriptsLogs()
{
	std::string filePath =OS_STARTUP_LOGS_PATH;

	TRACEINTO << "CInstallerManager::DumpScriptsLogs\n\n*************************\n\n\tSCRIPS LOGS\n\n*************************\n\n";

		//Prepare OS Logfile name
		filePath += "safeUpgrade.log";
		::DumpFile(filePath);

	TRACEINTO << "CInstallerManager::DumpScriptsLogs\n\n*******************************\n\n\tEND OF SCRIPTS LOGS\n\n*******************************\n\n";
}


//////////////////////////////////////////////////////////////////////
void CInstallerManager::SendAuthenticationStructReqToMcuMngr()
{
	TRACESTR(eLevelInfoNormal) << "\nCInstallerManager::SendAuthenticationStructReqToMcuMngr";
	CSegment*  pRetParam = new CSegment;

	const COsQueue* pMcuMngrMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessMcuMngr, eManager);

	STATUS res = pMcuMngrMbx->Send(pRetParam, INSTALLER_AUTHENTICATION_STRUCT_REQ);
}




void CInstallerManager::OnMcuMngrAuthenticationStruct(CSegment* pSeg)
{
	//m_isAuthenticationStructAlreadyReceived = YES;

	// ===== 1. get the parameters from the structure received into process's attribute
	pSeg->Get( (BYTE*)m_pAuthenticationStruct, sizeof(MCMS_AUTHENTICATION_S) );

	// ===== 2. print the data received
	TRACESTR(eLevelInfoNormal) << "\n CInstallerManager::OnMcuMngrAuthenticationSuccess "
                                << *m_pAuthenticationStruct;
}

//BRIDGE-12676
STATUS CInstallerManager::preventSWDowngradeInNewCtrl(string firmVersion)
{
	STATUS retStatus = STATUS_OK;

	char firmVersionArray[100];
	memset(firmVersionArray,0,sizeof(firmVersionArray));
	memcpy(firmVersionArray,firmVersion.c_str(), min(firmVersion.length(),sizeof(firmVersionArray)-1));


	string subVersion = firmVersion.substr(4, 3);
	string ver7_6 = "7.6";

	//versions RMX_7.6.1C.46.001-RMX_7.6.1C.46.034 should not be downgraded in new ctrl to.
	char * version7_6_1C_46 =strstr(firmVersionArray, "RMX_7.6.1C.46.");
	bool regularVersionC=true;

	if (version7_6_1C_46 != NULL)
	{

		version7_6_1C_46 = version7_6_1C_46 + strlen("RMX_7.6.1C.46.");
		char lastSubVersion7_6[10];
		memset(lastSubVersion7_6,0,sizeof(lastSubVersion7_6));
		strncpy(lastSubVersion7_6,version7_6_1C_46, sizeof(lastSubVersion7_6)-1);

		TRACESTR(eLevelInfoNormal) << "CInstallerManager::OnInstallerFinishInstallation lastSubVersion7_6 " << lastSubVersion7_6;


		//versions above RMX_7.6.1C.46.034 - we should aloud- request from Alex Libanov
		if (strncmp(lastSubVersion7_6 , "034",3) <= 0)
			regularVersionC = false;
	}



	//that version according to Alex Libanov is only on RMX1500 and our fix is to 2000/4000
	//versions RMX_7.6.1J.136.001-RMX_7.6.1J.136.004 should not downgrade in new ctrl to.
	/*memset(firmVersionArray,0,sizeof(firmVersionArray));
	memcpy(firmVersionArray,firmVersion.c_str(),firmVersion.length());

	char * version7_6_1J_136 =strstr(firmVersionArray, "RMX_7.6.1J.136.");
	bool regularVersionJ=true;

	if (version7_6_1J_136 != NULL)
	{

		version7_6_1J_136 = version7_6_1J_136 + strlen("RMX_7.6.1J.136.");
		char lastSubVersion7_6[10];
		memset(lastSubVersion7_6,0,sizeof(lastSubVersion7_6));
		strcpy(lastSubVersion7_6,version7_6_1J_136);

		TRACESTR(eLevelInfoNormal) << "CInstallerManager::OnInstallerFinishInstallation lastSubVersion7_6J " << lastSubVersion7_6;


		//versions above RMX_7.6.1C.46.034 - we should aloud- request from Alex Libanov
		if (strncmp(lastSubVersion7_6 , "004",3) <= 0)
			regularVersionJ = false;
	}*/






	//versions below V8.2 should not be downgraded in new ctrl to.
	float subVersionf = strtof(subVersion.c_str(),NULL);
	TRACESTR(eLevelInfoNormal) << "CInstallerManager::OnInstallerFinishInstallation subVersionf " << subVersionf;


	if (m_pAuthenticationStruct->isCtrlNewGeneration == true)
	{

		if (((subVersion != ver7_6) && (subVersionf < (float)(8.2)) ) || ((subVersion == ver7_6) && (!regularVersionC)) )
		{
			retStatus = STATUS_NEW_VERSION_CANNOT_BE_INSTALLED;
			RemoveActiveAlarmByErrorCode(AA_SAFE_UPGRADE_OFF);
			TRACESTR(eLevelInfoNormal) << "CInstallerManager::OnInstallerFinishInstallation version downgrade fail ";

			CMedString retStr =  "RMX Software version change from ";
			retStr	<< m_pCfsParamsStruct->verFromKeycode.ver_major    << "."
					<< m_pCfsParamsStruct->verFromKeycode.ver_minor    << "."
					<< m_pCfsParamsStruct->verFromKeycode.ver_release  << "."
					<< m_pCfsParamsStruct->verFromKeycode.ver_internal << " to "
					<< firmVersion.c_str()
					<< " is not supported, For a list of valid upgrades and downgrades, refer to RMX documentation";



			AddActiveAlarmFaultOnlySingleton( FAULT_GENERAL_SUBJECT,
					                          NEW_VERSION_DOWNGRADE_FAILED,
					                          MAJOR_ERROR_LEVEL,
					                          retStr.GetString());




		}
	}

	return retStatus;
}

//////////////////////////////////////////////////////////////////////
///For the Process of upgrade it is important to see logs even if we get brute force reboot in the Middle
//we will than start local tracer on Processes: Logger (to see embedded/cards prints) Installer Apache and McuMngr
STATUS CInstallerManager::StartLocalTracer(bool bStart)
{
	STATUS stat = STATUS_OK;
	CTerminalCommand command;
	std::ostringstream answer;
	std::string output,strParamStart="on";
	if (bStart==false)
		strParamStart = "off";
	else //turn of local tracer if no reset after 30 Minutes
		StartTimer(INSTALLER_TURN_OFF_LOCAL_TRACER_TIMER, TIMEOUT_INSTALATION );

	const char* terminal_file_name = ttyname(0);
	if (NULL == terminal_file_name)
	{
	    terminal_file_name = "/dev/null";
	}
	command.AddToken(terminal_file_name);
	command.AddToken("Installer");
	command.AddToken("set_local_tracer");
	command.AddToken(strParamStart.c_str());
	stat = HandleTerminalTraceLocal(command,answer);

	std::string setLocalTracerCmd = "Bin/McuCmd set_local_tracer Logger "  ; //former "Bin/McuCmd kill QAAPI"
	setLocalTracerCmd += strParamStart;
	stat &= SystemPipedCommand(setLocalTracerCmd.c_str(), output);

	setLocalTracerCmd = "Bin/McuCmd set_local_tracer ApacheModule ";
	setLocalTracerCmd += strParamStart;
	stat &= SystemPipedCommand(setLocalTracerCmd.c_str(), output);

	setLocalTracerCmd = "Bin/McuCmd set_local_tracer McuMngr ";
	setLocalTracerCmd += strParamStart;
	stat &= SystemPipedCommand(setLocalTracerCmd.c_str(), output);

	return stat;
}

////////////////////////////////////////////////////////////////////////////
void CInstallerManager::OnTurnOffLocalTracerTimer(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "CInstallerManager::OnTurnOffLocalTracerTimer we stop local tracer to hence Installation not finished";
	StartLocalTracer(false);
}
