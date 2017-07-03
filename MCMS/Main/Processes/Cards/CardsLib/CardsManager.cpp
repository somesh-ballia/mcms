// CardsManager.cpp

#include "CardsManager.h"

#include <stdlib.h>
#include <ostream>
#include <netinet/in.h>

#include "MediaIpParameters.h"
#include "CardsManager.h"
#include "CardMngrLoaded.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "OpcodesMcmsShelfMngr.h"
#include "TraceStream.h"
#include "Trace.h"
#include "SwitchApi.h"
#include "SwitchTask.h"
#include "MfaApi.h"
#include "MfaTask.h"
#include "RtmIsdnApi.h"
#include "RtmIsdnTask.h"
#include "CardsProcess.h"
#include "StatusesGeneral.h"
#include "HwMonitoring.h"
#include "FaultsDefines.h"
#include "MplMcmsProtocolTracer.h"
#include "TerminalCommand.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "SystemFunctions.h"
#include "McmsDaemonApi.h"
#include "RtmIsdnMngrInternalDefines.h"
#include "RtmIsdnMngrInternalStructs.h"
#include "AcRequestStructs.h"
#include "AcDefinitions.h"
#include "OpcodesMcmsAudioCntl.h"
#include "AuditorApi.h"
#include "CardsDefines.h"
#include "OsFileIF.h"
#include "HlogApi.h"
#include "IncludePaths.h"
#include "ConfigManagerApi.h"
#include "EthernetSettingsMonitoring.h"
#include "SlotsNumberingConversionTableWrapper.h"
#include "IceCmInd.h"
#include  "UtilityMngrInternalStruct.h"
#include "CardsStructs.h"
#include "TraceStream.h"
#include "TraceClass.h"
#include "BoardNumberData.h"
#include "Request.h"
#include "SlotsNumberingConversionTableWrapper.h"
#include "DummyEntry.h"
#include "TerminalCommand.h"
#include "OpcodesMcmsCommon.h"
#include "HostCommonDefinitions.h"
#include "SNMPDefines.h"
#include "DefinesGeneral.h"


extern "C" void CardsDispatcherEntryPoint(void* appParam);
extern void CardsMonitorEntryPoint(void* appParam);
extern char* ProductTypeToString(APIU32 productType);
extern char* CardTypeToString(APIU32 cardType);
extern char* CardStateToString(APIU32 cardType);
extern char* ProcessTypeToString(eProcessType processType);
extern const char* GetSystemCardsModeStr(eSystemCardsMode theMode);
extern char* CardUnitConfiguredTypeToString(APIU32 unitConfigType);
extern const char* UnitReconfigStatusToString(APIU32 unitReconfigStatus);
extern char* IpTypeToString(APIU32 ipType, bool caps = false);
extern char* IpV6ConfigurationTypeToString(APIU32 configType, bool caps = false);

extern const char *GetUserMsgCode_CsStr(enum eUserMsgCode_CS theCode);
extern const char *GetUserMsgCodeEmbStr(enum eUserMsgCode_Emb theCode);
extern const char *GetUserMsgLocationStr(enum eUserMsgLocation theLocation);
extern const char *GetUserMsgOperationStr(enum eUserMsgOperation theOperation);
extern const char *GetUserMsgAutoRemovalStr(enum eUserMsgAutoRemoval theAutoRemoval);
extern const char *GetUserMsgProcessType_CsStr(enum eUserMsgProcessType_CS theType);
extern const char *GetUserMsgProcessType_EmbStr(enum eUserMsgProcessType_Emb theType);

#define MAX_LEGAL_NUM_OF_RTM_ISDN_STARTUPS		5
#define MAX_COUNTER_OF_RTM_ISDN_STARTUPS		1000
#define FIXED_RESCUE_SWITCH_ID		128

#define SWAPL(X) (((X)&0xff)<<24)+(((X)&0xff00)<<8)+(((X)&0xff0000)>>8)+(((X)&0xff000000)>>24)

////////////////////////////////////////////////////////////////////////////
//               Task StateMachine's states
////////////////////////////////////////////////////////////////////////////
/*
- CardsManager task is IDLE until receiving MCUMNGR_AUTHENTICATION_SUCCESS_REQ.
- After receiving MCUMNGR_AUTHENTICATION_SUCCESS_REQ the task becomes READY

// default states: defined in StateMachine.h
//const  WORD  IDLE      = 0;
//const  WORD  ANYCASE   = 0xFFFF;
*/
const    WORD  READY     = 1;

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CCardsManager)
    ONEVENT(MPLAPI_MSG,                         			ANYCASE, CCardsManager::OnMplApiMsg )
    ONEVENT(ACK_IND,                                        ANYCASE, CCardsManager::IgnoreMessage )

    ONEVENT(SYSTEM_CARDS_MODE_STARTUP_TIMER,                ANYCASE, CCardsManager::OnTimerRmxSystemCardsModeStartupTimeout )
    ONEVENT(MFA_TASK_ALREADY_EXIST_TIMER,                   ANYCASE, CCardsManager::OnTimerMfaTaskAlreadyExistTimeout )
    ONEVENT(MFA1_HOT_SWAP_TIMER,                             ANYCASE, CCardsManager::OnTimerMfaHotSwapTimeout )
ONEVENT(MFA2_HOT_SWAP_TIMER,                             ANYCASE, CCardsManager::OnTimerMfaHotSwapTimeout )
ONEVENT(MFA3_HOT_SWAP_TIMER,                             ANYCASE, CCardsManager::OnTimerMfaHotSwapTimeout )
ONEVENT(MFA4_HOT_SWAP_TIMER,                             ANYCASE, CCardsManager::OnTimerMfaHotSwapTimeout )
    ONEVENT(MFA_STARTUP_TIMER,								ANYCASE, CCardsManager::OnTimerMfaStartupTimeout )

    ONEVENT(MCUMNGR_TO_CARDS_LICENSING_IND, 			    ANYCASE, CCardsManager::OnMcuMngrLicensingInd )

    ONEVENT(CM_CARD_MNGR_LOADED_IND,            			READY,   CCardsManager::OnCardMngrLoadedInd )
    ONEVENT(MCUMNGR_IP_TYPE_IND,							ANYCASE, CCardsManager::OnMcuMngrIpTypeInd)
    ONEVENT(CM_CARD_MNGR_LOADED_IND,            			IDLE,    CCardsManager::OnMplMsgWhileNotREADY )

    ONEVENT(CS_CARDS_MEDIA_IP_PARAMS_IND,       			ANYCASE, CCardsManager::OnMediaIpParamsFromCsInd )
    ONEVENT(CS_CARDS_MEDIA_IP_PARAMS_END_IND,   			ANYCASE, CCardsManager::OnMediaIpParamsEndFromCsInd )

    ONEVENT(IVR_MUSIC_ADD_SOURCE_REQ,           			ANYCASE, CCardsManager::OnIvrMusicAddSourceReq )
    ONEVENT(CM_CARD_FOLDER_MOUNT_IND,						ANYCASE, CCardsManager::OnCardFolderMountInd )
//  ONEVENT(CM_IVR_FOLDER_MOUNT_IND,						ANYCASE, CCardsManager::OnIvrFolderMountInd )
    ONEVENT(EMB_USER_MSG_IND,								ANYCASE, CCardsManager::OnEmbUserMsgInd )

    ONEVENT(CS_CARDS_DELETE_IP_SERVICE_IND,     			ANYCASE, CCardsManager::OnDeleteIpServiceReq )
    ONEVENT(CM_DELETE_IP_SERVICE_IND,           			ANYCASE, CCardsManager::OnDeleteIpServiceInd )

    ONEVENT(CARDS_RESTART_AUTHENTICATION_PROCEDURE_REQ,	    ANYCASE, CCardsManager::OnCardsResetAllBoardsReq )
    ONEVENT(CARD_REMOVED_IND,	                            ANYCASE, CCardsManager::OnCardRemovedInd )
    ONEVENT(BOARDS_EXPECTED_IND,                            ANYCASE, CCardsManager::OnBoardsExpectedInd )
    ONEVENT(MODE_DETECTION_IND,                             ANYCASE, CCardsManager::OnModeDetectionInd )

    ONEVENT(RTM_ISDN_PARAMS_IND,							ANYCASE, CCardsManager::OnRtmIsdnParamsInd )
    ONEVENT(RTM_ISDN_PARAMS_END_IND,						ANYCASE, CCardsManager::IgnoreMessage )
    ONEVENT(RTM_ISDN_SPAN_MAPS_IND,							ANYCASE, CCardsManager::OnRtmIsdnSpanMapsInd )
    ONEVENT(RTM_ISDN_ATTACH_SPAN_MAP_IND,					ANYCASE, CCardsManager::OnRtmIsdnAttachSpanMapsInd )
    ONEVENT(RTM_ISDN_DETACH_SPAN_MAP_IND,					ANYCASE, CCardsManager::OnRtmIsdnDetachSpanMapsInd )
    ONEVENT(RTM_ISDN_DELETE_SERVICE_IND,					ANYCASE, CCardsManager::OnRtmIsdnDeleteServiceInd )

    ONEVENT(RESOURCE_SYSTEM_CARDS_MODE_REQ,					ANYCASE, CCardsManager::OnResourceSystemCardsModeReq )
    ONEVENT(CONFPARTY_SYSTEM_CARDS_MODE_REQ,				ANYCASE, CCardsManager::OnConfPartySystemCardsModeReq )
    ONEVENT(MCUMNGR_SYSTEM_CARDS_MODE_REQ,				    ANYCASE, CCardsManager::OnMcuMngrSystemCardsModeReq )
    ONEVENT(UTILITY_SYSTEM_CARDS_MODE_REQ,				    ANYCASE, CCardsManager::OnUtilitySystemCardsModeReq )
    ONEVENT(SNMP_MFA_INTERFACE_IP_REQ,					    ANYCASE, CCardsManager::OnSNMPInterfaceReq )
    ONEVENT(AC_TYPE_REQ,                    				ANYCASE, CCardsManager::OnAudioCntrlrActiveReq )

    ONEVENT(RESOURCE_UNIT_RECONFIG_REQ,                    	ANYCASE, CCardsManager::OnResourceReconfigReq )
    ONEVENT(CARDS_NEW_SYS_CFG_PARAMS_IND,                  	ANYCASE, CCardsManager::OnNewSysCfgParamsInd )
    ONEVENT(INSTALLER_NEW_VERSION_IS_READY_TO_CARDS,        ANYCASE, CCardsManager::OnVersionIsReady )
    ONEVENT(LAST_CARD_INSTALLATION_FINISHED,                ANYCASE, CCardsManager::OnLastCardInstallationFinished )
    ONEVENT(INSTALLER_RESET_ALL_TO_CARDS,	                ANYCASE, CCardsManager::OnResetAllCardsReqFromInstaller )
    ONEVENT(CHECK_ALL_IPMC_IND_RECEIVED,	                ANYCASE, CCardsManager::OnCheckAll_IPMC_IND_received )
    ONEVENT(TOTAL_IPMC_IND_RECEIVED_TOUT,	                ANYCASE, CCardsManager::OnIPMC_IND_receivedTOUT )
    ONEVENT(SYSMONITOR_CARDS_ETHERNET_SETTINGS_MONITORING_IND,ANYCASE, CCardsManager::OnEthSettingMonitoringFromSysMonitor )
	ONEVENT(ETHERNET_SETTINGS_ACTIVE_PORTS_SWITCH_IND,		ANYCASE, CCardsManager::OnEthSettingActivePortsSwitchInd )
	ONEVENT(ETHERNET_SETTINGS_MONITORING_SWITCH_IND,		ANYCASE, CCardsManager::OnEthSettingMonitoringSwitchInd )
	ONEVENT(ETHERNET_SETTINGS_CLEAR_MAX_COUNTERS_SWITCH_IND,ANYCASE, CCardsManager::OnEthSettingClearMaxCountersSwitchInd )
	ONEVENT(CARD_CONFIG_REQ,								ANYCASE, CCardsManager::OnConfigCardReq )//2 modes cop/cp
	ONEVENT(SIPPROXY_TO_CARDS_ICE_INIT_REQ,					ANYCASE, CCardsManager::OnSipProxyToCardsIceInitReq )
	ONEVENT(ICE_INIT_TIMER,									ANYCASE, CCardsManager::OnTimerIceInitTimeout )
	ONEVENT(ICE_INIT_IND_RECEIVED,							ANYCASE, CCardsManager::OnIceInitIndReceived)
	ONEVENT(ICE_STATUS_IND_RECEIVED,							ANYCASE, CCardsManager::OnIceStatIndReceived)
	ONEVENT(LOGGER_CARDS_MAX_TRACE_LEVEL,                   ANYCASE, CCardsManager::OnLoggerCardsMaxTraceLevel)
	ONEVENT(UTILITY_TO_CARDS_START_TCP_DUMP_REQ,			    ANYCASE,	CCardsManager::OnStartTcpDump)
    ONEVENT(UTILITY_TO_CARDS_STOP_TCP_DUMP_REQ,				ANYCASE,	CCardsManager::OnStopTcpDump)
    ONEVENT(START_TCP_DUMP_IND,							    ANYCASE,	CCardsManager::OnStartTcpDumpInd)
    ONEVENT(STOP_TCP_DUMP_IND,							    ANYCASE,	CCardsManager::OnStopTcpDumpInd)
	ONEVENT( SNMP_CONFIG_TO_OTHER_PROGRESS, 			  ANYCASE,	  CCardsManager::OnSNMPConfigInd)
PEND_MESSAGE_MAP(CCardsManager, CManagerTask);


BEGIN_SET_TRANSACTION_FACTORY(CCardsManager)
	ON_TRANS("TRANS_CARD","RESCUE_CARD",    CBoardNumberData, CCardsManager::RescueCard )
END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CCardsManager)

    ONCOMMAND("cards_state",		CCardsManager::HandleTerminalCardsState,	    "display cards' state")
    ONCOMMAND("reset_to_daemon",	CCardsManager::HandleTerminalResetReqToDaemon,	"send reset request from Cards process to McmsDaemon process")
    ONCOMMAND("set_debug_mode_mpl",	CCardsManager::HandleTerminalSetDebugModeMpl,	"send debug_mode to all cards")
    ONCOMMAND("rtm_isdn_ka",		CCardsManager::HandleTerminalEnableDisableRtmIsdnKeepAlive, "enable/disable RtmIsdn KeepAlive treatment")
    ONCOMMAND("sys_cards_mode",		CCardsManager::HandleTerminalSysCardsMode,		"display system's cards mode")

    ONCOMMAND("usr_msg_emb",		CCardsManager::HandleTerminalUsrMsgEmb,		"simulates a user msg from Emb")

    //temp - for debugging
    ONCOMMAND("ethtest",		CCardsManager::HandleTerminalEthTest,		"test eth")
    ONCOMMAND("test_upgrade",		CCardsManager::HandleTestUpgrade,		"test_upgrade - simulate version upgrade")
    ONCOMMAND("test_ipmc_upgrade",	CCardsManager::HandleTestIpmcUpgrade,		"test_ipmc_upgrade - simulate ipmc upgrade")
    ONCOMMAND("test_reset_all_cards",	CCardsManager::HandleTestResetAllCards,		"test_rest_all_cards - simulate reset cards after software upgrade")
    ONCOMMAND("ka_received_timeout",	CCardsManager::HandleTerminalKaRecievedTimeout,		"display number of KA recived timeout ")


END_TERMINAL_COMMANDS



////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void CardsManagerEntryPoint(void* appParam)
{
	CCardsManager* pCardsManager = new CCardsManager;
	pCardsManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CCardsManager::GetMonitorEntryPoint()
{
	return CardsMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCardsManager::CCardsManager()
{
	TRACESTR(eLevelInfoNormal) << "CCardsManager - constructor";

	m_pProcess                     = (CCardsProcess*)CCardsProcess::GetProcess();
	m_isMediaIpParamsEndReceived   = NO;
	m_isResourceSystemCardsModeReq = NO;
	m_isModeDetectionAlreadyTreated = NO;

	m_numOfRtmIsdnStartups_boardId_1 = 0;
	m_numOfRtmIsdnStartups_boardId_2 = 0;
	m_numOfRtmIsdnStartups_boardId_3 = 0;
	m_numOfRtmIsdnStartups_boardId_4 = 0;

// 01/01/2009: SRE requires that removing the last board will NOT affect cards' mode
//	m_isCardModeMpmPlusDueToNoCardsOnSystem = false;

    m_isStartupOver = false;
	m_bSNMPEnabled  = FALSE;

    m_numOfBreezeCards_NGB = 0;
    m_numOfMPMRXCards_NGB  = 0;

    m_isNewVersionBlockedForMPMRX = false;

}

////////////////////////////////////////////////////////////////////////////
CCardsManager::~CCardsManager()
{
}

//////////////////////////////////////////////////////////////////////
void CCardsManager::SelfKill()
{
	//  was put in commrnt due to leak problems
	//	TRACESTR(eLevelInfoNormal) << "CMfaTask::SelfKill";


	// =======================================================
	// a differentiation should be done between:
	//   - Process failure (Mfa tasks should be killed), and
	//   - CardsManager failure (Mfa tasks should stay)
	// =======================================================


	// ===== 1. killing dispatcher (to avoid getting messages to MFAs after they already dead)
	if (m_pDispatcherApi)
	{
		m_pDispatcherApi->SyncDestroy();
		POBJDELETE(m_pDispatcherApi);
	}

	// ===== 2. killing Mfa tasks
	int boardIdx=0, subBoardIdx=0;
	for (boardIdx=0; boardIdx<MAX_NUM_OF_BOARDS; boardIdx++)
	{
		for (subBoardIdx=0; subBoardIdx<MAX_NUM_OF_SUBBOARDS; subBoardIdx++)
		{
			COsQueue* mfaMbx = m_pProcess->GetMfaMbx(boardIdx, subBoardIdx);

			if (mfaMbx)
			{
				CTaskApi api;
				api.CreateOnlyApi(*mfaMbx);
				api.SendOpcodeMsg(DESTROY);
			}
		}
	}

	CManagerTask::SelfKill();
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::ManagerPostInitActionsPoint()
{
	// this function is called just before WaitForEvent
	TRACESTR(eLevelInfoNormal) << "CCardsManager::InitTask";

	//BRIDGE-13450
	BOOL isSystemAfterVersionInstall = IsFileExists(SYSTEM_AFTER_VERSION_INSTALLED_FILE);
	if (TRUE == isSystemAfterVersionInstall)
		m_pProcess->SetMaxTimeForStartup(STARTUP_TIME_LIMIT_AFTER_INSTALLATION);

	// ask CentralSignaling to send mediaIpParams

	m_state = IDLE;


    // SystemCardsModeSystemCardsMode(Mix/Pure)
    // --- a. init and send to McuMngr and ConfParty
    InitSystemCardsMode();
    if( eProductFamilySoftMcu == m_pProcess->GetProductFamily() )
    {
     SendCurSystemCardsModeToSpecProcess(eProcessMcuMngr); //olga
     SendCurSystemCardsModeToSpecProcess(eProcessConfParty);
    }

    // --- b. start startup timer to recalculate the mode when timeout is reached
    DWORD timeToEndStartup = m_pProcess->GetMaxTimeForStartup();
	StartTimer( SYSTEM_CARDS_MODE_STARTUP_TIMER, timeToEndStartup );


	// ask CentralSignaling to send mediaIpParams
	SendMediaIpParamsReqToCS();

	// ask ConfParty to send 'ivrAddMusicSource' structures
	SendIvrMusicGetSourceReqToConfParty();

	StartTimer(MFA_STARTUP_TIMER, CARD_STARTUP_TIME_LIMIT);
	
	CManagerApi api(eProcessSNMPProcess);
	CSegment *pSeg = new CSegment;
	DWORD type = eProcessCards;
	*pSeg << type;
	api.SendMsg(pSeg, SNMP_OTHER_PROCESS_READY);


	// Begin Startup Feature
	m_pProcess->m_NetSettings.LoadFromFile();
	// End startup
}

void CCardsManager::CreateDispatcher()
{
	m_pDispatcherApi = new CTaskApi;
	CreateTask(m_pDispatcherApi, CardsDispatcherEntryPoint, m_pRcvMbx);
//	m_pDispatcherApi->Create(CardsDispatcherEntryPoint, *m_pRcvMbx);
}

/////////////////////////////////////////////////////////////////////
void CCardsManager::DeclareStartupConditions()
{
	CActiveAlarm aa1(FAULT_GENERAL_SUBJECT,
					AA_NO_IP_SERVICE_PARAMS,
					MAJOR_ERROR_LEVEL,
					"No IP service was received from CSMngr",
					false,
					false);
	AddStartupCondition(aa1);

	CActiveAlarm aa2(FAULT_GENERAL_SUBJECT,
					AA_NO_MUSIC_SOURCE,
					MAJOR_ERROR_LEVEL,
					"No IVR music source was received from ConfParty",
					true,
					true);
	AddStartupCondition(aa2);

	CActiveAlarm aa3(FAULT_GENERAL_SUBJECT,
					AA_NO_LICENSING,
					MAJOR_ERROR_LEVEL,
					"Licensing was not received from McuMngr",
					false,
					false);
	AddStartupCondition(aa3);

	// SoftMcu: will not receive CARD_MNGR_LOADED_IND from switch card because no GideonSim
	if( eProductFamilySoftMcu != m_pProcess->GetProductFamily() )
	{
		CActiveAlarm aa4(FAULT_GENERAL_SUBJECT,
					AA_SWITCH_NOT_LOADED,
					MAJOR_ERROR_LEVEL,
					"Switch board loaded indication was not received from MPL",
					true,
					true);
		AddStartupCondition(aa4);

		AddStartupCondDependency(AA_NO_LICENSING, AA_SWITCH_NOT_LOADED);
	}

/*
// 16.03.06: CM_CARD_MNGR_LOADED_IND is not a condition anymore
	CActiveAlarm aa5(FAULT_GENERAL_SUBJECT,
					AA_NO_CARD_MNGR_LOADED,
					MAJOR_ERROR_LEVEL,
					"No card manager loaded indication received",
					true,
					true);
	AddStartupCondition(aa5);
*/

}

//////////////////////////////////////////////////////////////////////
void CCardsManager::OnTimerRmxSystemCardsModeStartupTimeout()
{
    m_isStartupOver = true;

    //Rachel Cohen 15.5.12  we keep the number of mpmx cards and number of mpmrx cards for NGB phase1
  //  m_numOfBreezeCards_NGB = m_pProcess->GetNumOfBreezeBoards();
   // m_numOfMPMRXCards_NGB  = m_pProcess->GetNumOfMPMRXBoards();

    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
    eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();

    //vngr-25521 rmx startup with no cards the default mode MPMRX only
    eNGBSystemCardsMode NGBSystemCardsMode_old= m_pProcess->GetNGBSystemCardsMode();

    TRACEINTO << "\nCCardsManager::OnTimerRmxSystemCardsModeStartupTimeout NGBSystemCardsMode_old = " << GetNGBSystemCardsModeStr(NGBSystemCardsMode_old);
    if (eNGBSystemCardsMode_illegal == NGBSystemCardsMode_old)
    {
    	if (curProductFamily == eProductFamilySoftMcu)
    	{
    		if (eProductTypeNinja == curProductType || eProductTypeCallGeneratorSoftMCU == curProductType)
    			m_pProcess->SetNGBSystemCardsMode(eNGBSystemCardsMode_mpmrx_only); //MPMRX
    		else
    			m_pProcess->SetNGBSystemCardsMode(eNGBSystemCardsMode_breeze_only);
    	}
    	else
    	{
    		//BRIDGE-12020
    		eSystemCardsMode cardsModeFromFile = m_pProcess->GetSystemCardsModeFromFile();
    		eNGBSystemCardsMode ngbCardsMode   = m_pProcess->ConvertSystemCardsModeToNGBCardsMode(cardsModeFromFile);

    		m_pProcess->SetNGBSystemCardsMode(ngbCardsMode);
    	}
    }



// 29/07/2008: SRE requires that MPM will not be supported at V4.0. 'MPM mode' is not accepted.
// 09/2008: SRE requires that MPM will be supported at V4.0...
    RecalculateSystemCardsMode();

    if ((eProductFamilyRMX == curProductFamily || eProductFamilySoftMcu == curProductFamily)
    		&& (eProductTypeRMX4000 != curProductType) && (eProductTypeRMX1500 != curProductType))
    {
	    RecalculateSingleMediaBoardOnSecondSlot();
    }
}


//////////////////////////////////////////////////////////////////////
void CCardsManager::OnTimerMfaTaskAlreadyExistTimeout()
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnTimerMfaTaskAlreadyExistTimeout - delete timer";

	DeleteTimer(MFA_TASK_ALREADY_EXIST_TIMER);

}
//////////////////////////////////////////////////////////////////////
void CCardsManager::OnTimerMfaHotSwapTimeout(CSegment* pParam)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnTimerMfaTHotSwapTimeout - delete timer";

	DWORD boardId =0 ;
	*pParam >> boardId;

	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnTimerMfaTHotSwapTimeout - delete timer boardId " << boardId;

	m_pProcess->SetCardHotSwapStatus(boardId,NO);

 	switch(boardId)
  	{
 		case FIXED_BOARD_ID_MEDIA_1:
 			DeleteTimer(MFA1_HOT_SWAP_TIMER);
  			break;

  		case FIXED_BOARD_ID_MEDIA_2:
  			DeleteTimer(MFA2_HOT_SWAP_TIMER);
  			break;

 		case FIXED_BOARD_ID_MEDIA_3:
 			DeleteTimer(MFA3_HOT_SWAP_TIMER);
  			break;

 		case FIXED_BOARD_ID_MEDIA_4:
 			DeleteTimer(MFA4_HOT_SWAP_TIMER);
  			break;


  		default:
  			TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnCrardREmovedInd illegal board id";
  	}


}

//////////////////////////////////////////////////////////////////////
void CCardsManager::OnTimerMfaStartupTimeout()
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnTimerMfaStartupTimeout";

	// producing an Alert for SpanConfiguredOnNonExistingCard is moved to RtmIsdnMngr process
//	ProduceAlertsIfRtmSpanConfiguredOnNonExistingCard();
}

/*
//////////////////////////////////////////////////////////////////////
void CCardsManager::ProduceAlertsIfRtmSpanConfiguredOnNonExistingCard()
{
	int i=0;
	DWORD curBoardId=0, curSpanId=0;

	DWORD fixedMpmSubBoardId	= FIRST_SUBBOARD_ID;
	DWORD rtmSubBoardId			= fixedMpmSubBoardId+1;

	for (i=0; i<MAX_ISDN_SPAN_MAPS_IN_LIST; i++)
	{
		RTM_ISDN_SPAN_MAP_S curSpan = m_pProcess->GetRtmIsdnSpanMap(i);

		if ( '\0' != curSpan.serviceName[0] ) // span is attached to a service - let's ensure that the card exists
		{
			curBoardId = (DWORD)curSpan.boardId;
			curSpanId  = (DWORD)curSpan.spanId;

			COsQueue* pMbx = m_pProcess->GetMfaMbx(curBoardId, rtmSubBoardId);
			if ( !pMbx )
			{
				COstrStream errStr;
				errStr << "Red Alarm state at board " << curBoardId << ", subBoard " << rtmSubBoardId << ", Span " << curSpanId;
				DWORD alarmId = m_pProcess->CreateIdFromBoardIdSpanId(curBoardId, curSpanId);

		        AddActiveAlarm( FAULT_CARD_SUBJECT, AA_RED_ALARM, MAJOR_ERROR_LEVEL, errStr.str().c_str(),
		        		        true, true, alarmId, curBoardId, curSpanId, FAULT_TYPE_MPM );
			}
		}
	} // end loop over spans

}
*/

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnMplApiMsg(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnMplApiMsg";

	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);
	CMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("CARDS_RECEIVED_FROM_MPL");

	OPCODE opcd = pMplMcmsProtocol->getOpcode(); // extract the internal opcode...

	pSeg->ResetRead();
	DispatchEvent(opcd, pSeg);                     //  ... and send it to the stateMachine
	PushMessageToQueue(opcd, pSeg->GetLen(), eProcessMplApi);

	POBJDELETE(pMplMcmsProtocol);
}
/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnResourceReconfigReq(CSegment* pSeg)
{
	// ===== 1. retrieve the data
	UNIT_RECONFIG_S* pReconfigStruct = new UNIT_RECONFIG_S;
	pSeg->Get( (BYTE*)pReconfigStruct, sizeof(UNIT_RECONFIG_S) );

	// ===== 2. print to trace
    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnResourceReconfigReq - struct received from Resource process"
                                  << "\nBoardId:  " << pReconfigStruct->boardId
                                  << "\nUnitId:   " << pReconfigStruct->unitId
                                  << "\nUnitType: " << ::CardUnitConfiguredTypeToString(pReconfigStruct->unitType)
                                  << "\nStatus:   " << ::UnitReconfigStatusToString(pReconfigStruct->unitStatus);

	// ===== 3. send to card
    SendReconfigUnitToMpm(pReconfigStruct);

    delete pReconfigStruct;
}


/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendReconfigUnitToMpm(UNIT_RECONFIG_S* pReconfigStruct)
{
    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendReconfigUnitToMpm - struct sent to MPM"
                                  << "\nBoardId:  " << pReconfigStruct->boardId
                                  << "\nUnitId:   " << pReconfigStruct->unitId
                                  << "\nUnitType: " << ::CardUnitConfiguredTypeToString(pReconfigStruct->unitType)
                                  << "\nStatus:   " << ::UnitReconfigStatusToString(pReconfigStruct->unitStatus);

    CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(CM_UNIT_RECONFIG_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, pReconfigStruct->boardId, FIRST_SUBBOARD_ID/*MPM board*/);
	mplPrtcl->AddData(sizeof(UNIT_RECONFIG_S), (char*)pReconfigStruct);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}

//////////////////////////////////////////////////////////////////////
void CCardsManager::IgnoreMessage()
{
	// (nothing to be done)
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnMplMsgWhileNotREADY(CSegment* pSeg)
// 'READY' state means that Authentication validation ended successfully (by McuMngr).
//   In case that a message is received before an MCUMNGR_AUTHENTICATION_SUCCESS_REQ is,
//   it means that Mcms did not pass the authentication successfully yet!
//   Therefore Mpl should resend the message.

{
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);
	OPCODE opcd     = pMplMcmsProtocol->getOpcode();
	BYTE boardId    = pMplMcmsProtocol->getPhysicalInfoHeaderBoard_id(),
		 subBoardId = pMplMcmsProtocol->getPhysicalInfoHeaderSub_board_id();

/*
// 16.03.06: CM_CARD_MNGR_LOADED_IND is not a condition anymore
	if (CM_CARD_MNGR_LOADED_IND == opcd)
	{
		RemoveActiveAlarmByErrorCode(AA_NO_CARD_MNGR_LOADED);
	}
*/

	// print to trace
    TRACESTR(eLevelInfoNormal)
    	<< "\nCCardsManager::OnMplMsgWhileNotREADY. Opcode: "  << opcd
    	<< "\nBoardId: " << (DWORD)boardId << ", SubBoardId: " << (DWORD)subBoardId;

    /***********************************************************************************/
    /* 19.7.10 VNGR-16271 fixed by Rachel Cohen                                        */
    /* MPL has a timer of 10 seconds for sending CM_CARD_MNGR_LOADED_IND . but         */
    /* when we are in IDLE state (before Authentication validation ended successfully) */
    /* we send CARDS_NOT_READY_REQ and it stop the timer in MPL side                   */
    /* I remove that sending cause I want the timer in MPL to continue                 */
    /***********************************************************************************/
    //SendCardsNotReadyToMplApi(boardId, subBoardId);

     /***********************************************************************************/
     /* 10.8.10 VNGR-16851 fixed by Rachel Cohen                                        */
     /* The fix for VNGR-16271 added that bug .we enter that msg in case of MPL         */
     /* and in case of switch. when switch send MNGR_LOADED it has no timer to resend   */
     /* that msg .so when it is a switch we need to send  CARDS_NOT_READY msg that      */
     /* switch will resend the MNGR_LOADED msg .                                        */
     /***********************************************************************************/



    if ((!IsTarget()) ||(boardId == SWITCH_BBOARD_ID)  ||
        ((CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator) && (boardId == 0)))
    {
    	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnMplMsgWhileNotREADY";
    	 SendCardsNotReadyToMplApi(boardId, subBoardId);
    }



    //*******************************************************************************************************
    /* 13/3/14 Rachel Cohen
     * we want to keep the card mode for a case that on startup the license arrive in delay.
     *
     *************************************************************************************************/
    // ===== 2. get the new cardMngrLoaded data into a CCardMngrLoaded object
    CCardMngrLoaded* pCardMngr = new CCardMngrLoaded;
    pCardMngr->SetData(pMplMcmsProtocol->GetData());



    // ===== 5. Create a task for this Switch/MFA/RtmIsdn
    eCardType curCardType = pCardMngr->GetType();


    if ( (true == m_pProcess->IsMediaCard(curCardType) ) && (m_isModeDetectionAlreadyTreated == NO) )
    {
    	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnMplMsgWhileNotREADY  - the cardType : " << ::CardTypeToString(curCardType);

    	if (m_pProcess->IsBreezeCard(curCardType))
    	{
    		m_pProcess->SetSystemCardsModeCur(SYSTEM_CARDS_MODE_MPMX_VAL);
    		m_pProcess->SetNGBSystemCardsMode(eNGBSystemCardsMode_breeze_only);
    	}

    }


    POBJDELETE(pCardMngr);
    POBJDELETE(pMplMcmsProtocol);

}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendCardsNotReadyToMplApi(BYTE boardId, BYTE subBoardId)
{
	TRACESTR(eLevelInfoNormal) << "CCardsManager::SendCardsNotReadyToMplApi";

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(CARDS_NOT_READY_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, boardId, subBoardId);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendCardsNotSupportedToMplApi(BYTE boardId, BYTE subBoardId)
{
    TRACESTR(eLevelInfoNormal) << "CCardsManager::SendCardsNotSupportedToMplApi";

    CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

    mplPrtcl->AddCommonHeader(CARD_NOT_SUPPORTED_IND);
    mplPrtcl->AddMessageDescriptionHeader();
    mplPrtcl->AddPhysicalHeader(1, boardId, subBoardId);

    CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL");
    mplPrtcl->SendMsgToMplApiCommandDispatcher();

    POBJDELETE(mplPrtcl);
}


/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnMcuMngrLicensingInd(CSegment* pSeg)
{
	// ===== 1. get the parameters from the structure received into process's attribute
	pSeg->Get( (BYTE*)(m_pProcess->GetLicensingStruct()), sizeof(CARDS_LICENSING_S) );
	m_pProcess->SetIsAuthenticationSucceeded(YES);

	// ===== 2. print the data received

	eProductType theType = (eProductType)(m_pProcess->GetAuthenticationStruct()->productType);
	const char* productTypeStr = ::ProductTypeToString(theType);

	CLargeString traceStr = "\nCCardsManager::OnMcuMngrLicensingInd -";
	traceStr << "\nProduct Type: ";
	if (productTypeStr)
	{
		traceStr << productTypeStr;
	}
	else
	{
		traceStr << "(invalid: " << theType << ")";
	}
	traceStr << "\nSwitch boardId: "		<< m_pProcess->GetAuthenticationStruct()->switchBoardId
	         << ", switch subBoardId: "		<< m_pProcess->GetAuthenticationStruct()->switchSubBoardId
	         << "\nIs federal enabled: "	<< (m_pProcess->GetLicensingStruct()->federal ? "YES" : "NO")
	         << "\nMultiple Services: "		<< (m_pProcess->GetLicensingStruct()->multipleServices ? "YES" : "NO");

	traceStr << "\nChassis version: "
			 << m_pProcess->GetAuthenticationStruct()->chassisVersion.ver_major  << "."
			 << m_pProcess->GetAuthenticationStruct()->chassisVersion.ver_minor  << "."
			 << m_pProcess->GetAuthenticationStruct()->chassisVersion.ver_release  << "."
			 << m_pProcess->GetAuthenticationStruct()->chassisVersion.ver_internal  << "\n";

	TRACESTR(eLevelInfoNormal) << traceStr.GetString();


	//  ===== 3. update process's numOfBoards accordingly
	m_pProcess->SetNumOfBoards();


	// ===== Now CardsManager is ready to receive other Mpl messages
	//        (If an Mpl message is received before MCUMNGR_AUTHENTICATION_SUCCESS_REQ is received,
	//         then MPL should resend the message - func OnMplMsgWhileNotREADY)
	m_state = READY;

	// Notify Mfa task
	SendLicenseIndToMfaBoard();

	RemoveActiveAlarmByErrorCode(AA_NO_LICENSING);
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnCardMngrLoadedInd(CSegment* pSeg)
{
	// ===== 1. extract the mplMcmsProtocol from segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	// validate data size
	STATUS sizeStat = pMplMcmsProtocol->ValidateDataSize( sizeof(CM_CARD_MNGR_LOADED_S) );
	if (STATUS_OK != sizeStat)
	{
		POBJDELETE(pMplMcmsProtocol);

		return;
	}


	// ===== 2. get the new cardMngrLoaded data into a CCardMngrLoaded object
	CCardMngrLoaded* pCardMngr = new CCardMngrLoaded;
	pCardMngr->SetData(pMplMcmsProtocol->GetData());

    // ==== 2.5 validate and replace invalid strings
    pCardMngr->ValidateStrings();

 	// ===== 3. extract boardId, subBoardId
	BYTE msgBoardId    = pMplMcmsProtocol->getPhysicalInfoHeaderBoard_id(),
		 msgSubBoardId = pMplMcmsProtocol->getPhysicalInfoHeaderSub_board_id();

 	// ===== 4. print the data to trace (using Dump function)
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnCardMngrLoadedInd (boardId: " << (int)msgBoardId
	                              << ", subBoardId: " <<  (int)msgSubBoardId << ")\n" << *pCardMngr;

	// ===== 5. Create a task for this Switch/MFA/RtmIsdn
	eCardType curCardType = pCardMngr->GetType();
	if ( eSwitch == curCardType )
	{
		RemoveActiveAlarmByErrorCode(AA_SWITCH_NOT_LOADED);

		BYTE switchBoardIdFromProcess    = m_pProcess->GetAuthenticationStruct()->switchBoardId,
		     switchSubBoardIdFromProcess = m_pProcess->GetAuthenticationStruct()->switchSubBoardId;

		if ( (msgBoardId != switchBoardIdFromProcess) /*|| (msgSubBoardId != switchSubBoardIdFromProcess)*/ )
		{
			CMedString errStr = "Mismatching switch boardId: ";
			errStr << "From msg - boardId " << msgBoardId << ", subBoardId " << msgSubBoardId
			       << "; From AuthenticationInd - boardId " << switchBoardIdFromProcess
			       <<  ", subBoardId " << switchSubBoardIdFromProcess << "\n";
			PASSERTMSG( 1, errStr.GetString() );
		}

		CreateSwitchTask(pCardMngr, msgBoardId, msgSubBoardId);
	}

	else if ( true == m_pProcess->IsMediaCard(curCardType) )
	{
	    //in NGB for phase 1 we do not aloud to enter additional cards
	    if ( m_isModeDetectionAlreadyTreated == YES)
	    {
	        TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnCardMngrLoadedInd after MODE DETECTION was sent user trying to add new card"
	                               << "\nThe MNGR_LOADED msg from  - boardId " << (int)msgBoardId << ", subBoardId " << (int)msgSubBoardId ;


	               // ===== 1. produce Alert (if needed)
	        bool isAlertOn = HandleNGBSystemCardsModeAlerts(curCardType,msgBoardId,msgSubBoardId);
	        if (isAlertOn == true)
	        {
	            SendCardsNotSupportedToMplApi(msgBoardId,msgSubBoardId);
	            POBJDELETE(pCardMngr);
	            POBJDELETE(pMplMcmsProtocol);
	            TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnCardMngrLoadedInd after after MODE DETECTION was sent user trying to add new card - but we block it";
	            return;
	        }
	    }

		CreateMfaTask(pCardMngr, msgBoardId, msgSubBoardId, curCardType);
	}

	else if (eRtmIsdn == curCardType || eRtmIsdn_9PRI == curCardType || eRtmIsdn_9PRI_10G == curCardType)
	{
		CreateRtmIsdnTask(pCardMngr, msgBoardId, msgSubBoardId,curCardType);
	}

	else
	{
		TRACESTR(eLevelInfoNormal)<< "\nCCardsProcess::OnCardMngrLoadedInd - Illegal card type! "
		                      << "\nCard Type: " << ::CardTypeToString(curCardType);
	}

	// ===== 6. Add to CardDB
	AddToCardsMonitoringDB(pCardMngr, msgBoardId, msgSubBoardId);

	// ===== 7. Recalculate num of boards expected vs. num of actual boards on system
	if ( true == m_pProcess->IsMediaCard(curCardType) )
	{
		RecalculateBoardsExpectedVsActual();
	}

	// ===== 8. recalculation should be at startup
	if (true == m_isStartupOver)
	{
// 29/07/2008: SRE requires that MPM will not be supported at V4.0. 'MPM mode' is not accepted.
// 09/2008: SRE requires that MPM will be supported at V4.0...
	    RecalculateSystemCardsMode();

	    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	    eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();
	    if ((eProductFamilyRMX == curProductFamily || eProductFamilySoftMcu == curProductFamily)
	    		&& (eProductTypeRMX4000 != curProductType) && (eProductTypeRMX1500 != curProductType))
	    {
	    	RecalculateSingleMediaBoardOnSecondSlot();
	    }
    }


	// 16.03.06: CM_CARD_MNGR_LOADED_IND is not a condition anymore
	//RemoveActiveAlarmByErrorCode(AA_NO_CARD_MNGR_LOADED);

    SendCardEventToAuditor(true, msgBoardId, msgSubBoardId, curCardType);

	POBJDELETE(pCardMngr);
	POBJDELETE(pMplMcmsProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::CreateSwitchTask(CCardMngrLoaded* cardMngr, BYTE boardId, BYTE subBoardId)
{
	TRACESTR(eLevelInfoNormal)<< "\nCCardsProcess::CreateSwitchTask; "
	                      << "boardId: " << (int)boardId << ", subBoardId: " << (int)subBoardId;

	// ===== 1. if a task for this board already exists, then send reset req to daemon
	//BOOL isAlreadyExist = TreatTaskAlreadyExists_ResetMcms(boardId, subBoardId);
	// VNGFE-2150 (VNGR-11393)
    BOOL isAlreadyExist = IsTaskAlreadyExists(boardId, subBoardId);
	if (isAlreadyExist)
	{
		TreatTaskAlreadyExists(boardId, subBoardId, eSwitch);
	}
	else
	{
		// ===== 2. create the task
		CSwitchApi *pSwitchApi = new CSwitchApi;
	    pSwitchApi->Create(switchEntryPoint, *m_pRcvMbx);

		// ===== 3. update dispatcher's table
		CSwitchTask* pSwitchTask = (CSwitchTask*)pSwitchApi->GetTaskAppPtr();
		COsQueue& rcvMbx  = pSwitchTask->GetRcvMbx();
		m_pProcess->AddToMfaTasksList(&rcvMbx, boardId, subBoardId);

		// ===== 4. update task's parametrers
		pSwitchTask->SetBoardId(boardId);
		pSwitchTask->SetSubBoardId(subBoardId);
		pSwitchTask->SetCardMngrStructData(cardMngr);

		pSwitchTask->SetTaskState(eTaskStateReady);
		pSwitchTask->SetTaskStatus(eTaskNormal);
		// was added here since Switch_task_name is set according to BoardId; and task_status should be set to Normal
		//   only after task_name is set, in order to get an appropriate task_name in ActiveAlarms

		POBJDELETE(pSwitchApi);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::CreateMfaTask(CCardMngrLoaded* cardMngr, BYTE boardId, BYTE subBoardId, eCardType curCardType)
{
	TRACESTR(eLevelInfoNormal)<< "\nCCardsProcess::CreateMfaTask; "
	                      << "boardId: " << (int)boardId << ", subBoardId: " << (int)subBoardId;

	// ===== 1. if a task for this board already exists, then send reset req to daemon

    BOOL isAlreadyExist = IsTaskAlreadyExists(boardId, subBoardId);


	if (isAlreadyExist)
	{
		if ( IsValidTimer(MFA_TASK_ALREADY_EXIST_TIMER) )
		{

			TRACESTR(eLevelInfoNormal) << "\nCCardsManager::CreateMfaTask- timer MFA_TASK_ALREADY_EXIST_TIMER is on"
			                                 <<" we do not close the task";
			return;

		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCCardsManager::CreateMfaTask - timer MFA_TASK_ALREADY_EXIST_TIMER is off";
			TreatTaskAlreadyExists(boardId, subBoardId, curCardType);
		}

	}
	else
	{


		// ===== 2. create the task
		CMfaApi *pMfaApi = new CMfaApi;
	    pMfaApi->Create(mfaEntryPoint, *m_pRcvMbx);

		// ===== 3. update dispatcher's table
		CMfaTask* pMfaTask = (CMfaTask*)pMfaApi->GetTaskAppPtr();
		COsQueue& rcvMbx  = pMfaTask->GetRcvMbx();
		m_pProcess->AddToMfaTasksList(&rcvMbx, boardId, subBoardId);

		// ===== 4. update task's parametrers
		pMfaTask->SetBoardId(boardId);
		pMfaTask->SetSubBoardId(subBoardId);
		pMfaTask->SetCardMngrStructData(cardMngr);
        pMfaTask->SetTaskState(eTaskStateReady);
		pMfaTask->SetTaskStatus(eTaskNormal);
		// was added here since Mfa_task_name is set according to BoardId; and task_status should be set to Normal
		//   only after task_name is set, in order to get an appropriate task_name in ActiveAlarms

		// ===== 5. start MFA_TASK_ALREADY_EXIST_TIMER timer
		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::CreateMfaTask - start MFA_TASK_ALREADY_EXIST_TIMER timer" ;
		StartTimer(MFA_TASK_ALREADY_EXIST_TIMER, 30 *SECOND);

        POBJDELETE(pMfaApi);
    }

}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::CreateRtmIsdnTask(CCardMngrLoaded* cardMngr, BYTE boardId, BYTE subBoardId, eCardType curCardType)
{
	// ensure that num of boards' startup does not exceed the allowed number
	int curNumOfStartups=0;

	int firstMediaBoardId	= m_pProcess->Get1stMediaBoardId(),
		secondMediaBoardId	= m_pProcess->Get2ndMediaBoardId(),
		thirdMediaBoardId	= m_pProcess->Get3rdMediaBoardId(),
		fourthMediaBoardId	= m_pProcess->Get4thMediaBoardId();

	if ( firstMediaBoardId == (int)boardId )
	{
		if (MAX_COUNTER_OF_RTM_ISDN_STARTUPS >= m_numOfRtmIsdnStartups_boardId_1)
			m_numOfRtmIsdnStartups_boardId_1++;

		curNumOfStartups = m_numOfRtmIsdnStartups_boardId_1;
	}
	else if ( secondMediaBoardId == (int)boardId )
	{
		if (MAX_COUNTER_OF_RTM_ISDN_STARTUPS >= m_numOfRtmIsdnStartups_boardId_2)
			m_numOfRtmIsdnStartups_boardId_2++;

		curNumOfStartups = m_numOfRtmIsdnStartups_boardId_2;
	}
	else if ( thirdMediaBoardId == (int)boardId )
	{
		if (MAX_COUNTER_OF_RTM_ISDN_STARTUPS >= m_numOfRtmIsdnStartups_boardId_3)
			m_numOfRtmIsdnStartups_boardId_3++;

		curNumOfStartups = m_numOfRtmIsdnStartups_boardId_3;
	}
	else if ( fourthMediaBoardId == (int)boardId )
	{
		if (MAX_COUNTER_OF_RTM_ISDN_STARTUPS >= m_numOfRtmIsdnStartups_boardId_4)
			m_numOfRtmIsdnStartups_boardId_4++;

		curNumOfStartups = m_numOfRtmIsdnStartups_boardId_4;
	}


	CSmallString numOfStartupsStr;
	numOfStartupsStr << "\nNumOfStartups: " << curNumOfStartups;

	TRACESTR(eLevelInfoNormal)<< "\nCCardsProcess::CreateRtmIsdnTask; "
	                      << "boardId: " << (int)boardId << ", subBoardId: " << (int)subBoardId
	                      << numOfStartupsStr.GetString();
	////////////shlomit
	// ===== 1. if a task for RtmIsdn board already exists, no reset is done
	BOOL isAlreadyExist = IsTaskAlreadyExists(boardId, subBoardId);

	if (isAlreadyExist)
	{
		TreatTaskAlreadyExists(boardId, subBoardId, eRtmIsdn);

	}
	else

	{

		// ===== 2. create the task
		CRtmIsdnApi *pRtmIsdnApi = new CRtmIsdnApi;
	    pRtmIsdnApi->Create(RtmIsdnEntryPoint, *m_pRcvMbx);

		// ===== 3. update dispatcher's table
		CRtmIsdnTask* pRtmIsdnTask = (CRtmIsdnTask*)pRtmIsdnApi->GetTaskAppPtr();
		COsQueue& rcvMbx  = pRtmIsdnTask->GetRcvMbx();
		m_pProcess->AddToMfaTasksList(&rcvMbx, boardId, subBoardId);

		// ===== 4. update task's parametrers
		pRtmIsdnTask->SetBoardId(boardId);
		pRtmIsdnTask->SetSubBoardId(subBoardId);
		pRtmIsdnTask->SetCardMngrStructData(cardMngr);

/*
//old implementation of SlotsNumbering feature

		eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
		if ( IsTarget() && (eProductTypeRMX4000 == curProductType) )
		{
			// In RMX4000: Rtm's subBoardId is the boardId of its 'parent' MPM.
			//    When sending a msg to the CM of the Rtm, it should actually be routed
			//    to the boardId of the 'parent' MPM. That's why Rtm task
			//    has a separate (sub)boardId for msg_sending. For debugging purposes the boardId
			//    is also switched to be 'subBoardId' upon msg sending (for logs and traces).
			pRtmIsdnTask->SetBoardId_ForMsgSending(subBoardId);
			pRtmIsdnTask->SetSubBoardId_ForMsgSending(boardId);
		}
		else
		{
			pRtmIsdnTask->SetBoardId_ForMsgSending(boardId);
			pRtmIsdnTask->SetSubBoardId_ForMsgSending(subBoardId);
		}
*/

		pRtmIsdnTask->SetTaskState(eTaskStateReady);
		pRtmIsdnTask->SetTaskStatus(eTaskNormal);
		// was added here since RtmIsdn_task_name is set according to BoardId; and task_status should be set to Normal
		//   only after task_name is set, in order to get an appropriate task_name in ActiveAlarms
        POBJDELETE(pRtmIsdnApi);
	}
}
/////////////////////////////////////////////////////////////////////////////
BOOL CCardsManager::TreatTaskAlreadyExists_ResetMcms(BYTE boardId, BYTE subBoardId)
{
	BOOL isAlreadyExist = FALSE;

	COsQueue* pMfaMbx = m_pProcess->GetMfaMbx((DWORD)boardId, (DWORD)subBoardId);
	if (NULL != pMfaMbx)
	{
		isAlreadyExist = TRUE;

		// ===== 1. print to log
		CSmallString errStr = "CM_Loaded indication was received again; ";
		errStr << "boardId: " << (int)boardId << ", subBoardId: " << (int)subBoardId;

		TRACESTR(eLevelInfoNormal) << "\n" << errStr.GetString()
		                       << "\nResetMcms request is sent to Daemon!";

		// ===== 2. update list (in case reset is not done)
		m_pProcess->TurnMfaTaskToZombie(pMfaMbx);

		// ===== 3. produce an ASSERT
		PASSERTMSG( 1, errStr.GetString() );

		// ===== 4. send resetMcms request to Daemon
        string desc = errStr.GetString();
        CMcmsDaemonApi api;
        STATUS res = api.SendResetReq(desc);
	}

	return isAlreadyExist;

	/*
	// old code: kill old task, create new task (instead of sending a reset request)
	COsQueue* oldMfaMbx = m_pProcess->GetMfaMbx(boardId, subBoardId);
	if (oldMfaMbx)                       // MFA task already exists...
	{
		CTaskApi api;
		api.CreateOnlyApi(*oldMfaMbx);
		api.SendOpcodeMsg(DESTROY);      // ...then kill it!
	}
	m_pProcess->RemoveFromMfaTasksList(oldMfaMbx);
	*/
}

/////////////////////////////////////////////////////////////////////////////
BOOL CCardsManager::TreatTaskAlreadyExists_NoReset(BYTE boardId, BYTE subBoardId, int numOfStartups)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::TreatTaskAlreadyExists_NoReset";

	BOOL isAlreadyExist = FALSE;

	COsQueue* pMbx = m_pProcess->GetMfaMbx((DWORD)boardId, (DWORD)subBoardId);
	if (NULL != pMbx)
	{
		isAlreadyExist = TRUE;

		if (MAX_LEGAL_NUM_OF_RTM_ISDN_STARTUPS >= numOfStartups)
		{
			TaskAlreadyExists_AskStartupAgain(boardId, subBoardId, pMbx);
		}

		else
		{
			TaskAlreadyExists_TooManyStartups(boardId, subBoardId, pMbx);
		}
	}

	return isAlreadyExist;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CCardsManager::IsTaskAlreadyExists(BYTE boardId, BYTE subBoardId)
{
	BOOL isAlreadyExist = FALSE;

	COsQueue* pMbx = m_pProcess->GetMfaMbx((DWORD)boardId, (DWORD)subBoardId);
	if (NULL != pMbx)
	{
		isAlreadyExist = TRUE;
	}

	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::IsTaskAlreadyExists " << (int)isAlreadyExist;

	return isAlreadyExist;
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::TreatTaskAlreadyExists(BYTE boardId, BYTE subBoardId, eCardType cardType)
{

	// ===== 1. print to log
	CMedString errStr = "\nCCardsManager::TreatTaskAlreadyExists - CM_Loaded indication was received again";
	errStr << "\nBoardId: " << (int)boardId << ", subBoardId: " << (int)subBoardId;
	TRACESTR(eLevelInfoNormal) << errStr.GetString();

	// ===== 2. send 'not ready' (to gain some time for removing the task from the list
	SendCardsNotReadyToMplApi(boardId, subBoardId);

    m_pProcess->RemoveCard(boardId, subBoardId, cardType);

	// ===== 3. remove card
	COsQueue* pMbx = m_pProcess->GetMfaMbx((DWORD)boardId, (DWORD)subBoardId);

	// ===== 4. destroy the task
	CTaskApi taskApi;
    taskApi.CreateOnlyApi(*pMbx);
    taskApi.Destroy();

	// ===== 5. remove irrelevant Alert

	bool     isBreezeExists     = m_pProcess->IsBreezeCardExistInDB(),
	         isMPMRXExists     = m_pProcess->IsMPMRXCardExistInDB();

    //eSystemCardsMode curMode = m_pProcess->GetSystemCardsModeCur();
	eNGBSystemCardsMode NGBSystemCardsMode= m_pProcess->GetNGBSystemCardsMode();

   /* if ( ((eSystemCardsMode_mpm == curMode)			&& (false == isMpmPlusExists || false == isBreezeExists)) ||
    	 ((eSystemCardsMode_mpm_plus == curMode)	&& (false == isMpmExists     || false == isBreezeExists)) ||
		 ((eSystemCardsMode_breeze == curMode) 	    && (false == isMpmPlusExists || false == isMpmExists)) )*/
    if (
  ((eNGBSystemCardsMode_breeze_only == NGBSystemCardsMode)           && (cardType == eMpmRx_Half || cardType == eMpmRx_Full)) ||
  ((eNGBSystemCardsMode_mpmrx_only == NGBSystemCardsMode)    && ( cardType == eMpmx_40 || cardType == eMpmx_20 || cardType == eMpmx_80))
  /*((eNGBSystemCardsMode_mixed_mode == NGBSystemCardsMode)      && (m_pProcess->GetNumOfMPMRXBoards() >= m_numOfMPMRXCards_NGB ) &&
          (cardType == eMpmRx_Half || cardType == eMpmRx_Full)  ) ||

  ((eNGBSystemCardsMode_mixed_mode == NGBSystemCardsMode)      &&     (m_pProcess->GetNumOfBreezeBoards() >= m_numOfBreezeCards_NGB ) &&
                      ( cardType == eMpmx_40 || cardType == eMpmx_20 || cardType == eMpmx_80))*/)

    {
    	RemoveActiveAlarmByErrorCode(AA_CARDS_CONFIG_EVENT);
	    	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::TreatTaskAlreadyExists - Alert was removed";
	    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CCardsManager::TaskAlreadyExists_AskStartupAgain(BYTE boardId, BYTE subBoardId, COsQueue* pMbx)
{
	// ===== 1. send 'not ready' (to gain some time for removing the task from the list
	SendCardsNotReadyToMplApi(boardId, subBoardId);

	// ===== 2. print to log
	CMedString errStr = "CM_Loaded indication was received again. Ask startup again";
	errStr << "\nBoardId: " << (int)boardId << ", subBoardId: " << (int)subBoardId;

	TRACESTR(eLevelInfoNormal) << "\n" << errStr.GetString()
	                       << "\nRtmIsdn board - No Reset is done";

	// ===== 3. update Resources
	SendDisableAllSpansToRsrcAlloc( (WORD)boardId, (WORD)subBoardId );

	// ===== 5. remove from list
	m_pProcess->RemoveFromMfaTasksList(pMbx);

	// ===== 4. destroy the task
	CTaskApi taskApi;
    taskApi.CreateOnlyApi(*pMbx);
    taskApi.Destroy();

	// ===== 6. produce an ASSERT
	PASSERTMSG( 1, errStr.GetString() );
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::TaskAlreadyExists_TooManyStartups(BYTE boardId, BYTE subBoardId, COsQueue* pMbx)
{
	// ===== 1. print to log
	CMedString errStr = "CM_Loaded indication was received again. Too many startups!";
	errStr << "\nBoardId: " << (int)boardId << ", subBoardId: " << (int)subBoardId;

	TRACESTR(eLevelInfoNormal) << "\n" << errStr.GetString()
	                       << "\nRtmIsdn board - task becomes Zombie";

	// ===== 2. update Resources
	SendDisableAllSpansToRsrcAlloc( (WORD)boardId, (WORD)subBoardId );

	// ===== 3. remove from list
	m_pProcess->TurnMfaTaskToZombie(pMbx);

	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId((DWORD)boardId, (DWORD)subBoardId);
	if ( false == IsActiveAlarmExistByErrorCodeUserId(AA_RTM_ISDN_STARTUP_PROBLEM, (DWORD)boardId) )
	{
 		AddActiveAlarm( FAULT_CARD_SUBJECT,
 		                AA_RTM_ISDN_STARTUP_PROBLEM,
 		                MAJOR_ERROR_LEVEL,
 		                "RtmIsdn startup was received too many times",
 		                true,
 		                true,
 		                displayBoardId,	// displayBoardId as 'userId' (token)
 		                displayBoardId,
 		                0,				// unitId
 		                FAULT_TYPE_MPM );
	}

	// ===== 4. produce an ASSERT
	PASSERTMSG( 1, errStr.GetString() );
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendDisableAllSpansToRsrcAlloc(WORD boardId, WORD subBoardId)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendDisableAllSpansToRsrcAlloc "
	                       << "(BoardId: " << boardId << ", SubBoardId: " << subBoardId << ")";

    RTM_ISDN_BOARD_ID_S* pDisableStruct = new RTM_ISDN_BOARD_ID_S;

	// ===== 1. initialize the list
	pDisableStruct->boardId		= boardId;
	pDisableStruct->subBoardId	= subBoardId;

	// ===== 2. fill the Segment with items from the list
	CSegment*  pRetParam = new CSegment;
	pRetParam->Put( (BYTE*)pDisableStruct, sizeof(RTM_ISDN_BOARD_ID_S) );

	delete pDisableStruct;


	// ===== 3. send
	const COsQueue* pResourceMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResource, eManager);

	STATUS res = pResourceMbx->Send(pRetParam, RTM_ISDN_DISABLE_ALL_SPANS_IND, &GetRcvMbx());
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::AddToCardsMonitoringDB(CCardMngrLoaded* cardMngr, BYTE boardId, BYTE subBoardId)
{
	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId( (DWORD)boardId, (DWORD)subBoardId );

    CCommDynCard newCard( cardMngr->GetType(),
		                  cardMngr->GetSerialNumber(),
		                  boardId, subBoardId, (WORD)displayBoardId,
					      cardMngr->GetHardwareVersion(),
						  cardMngr->GetSwVersionsList() );


  m_pProcess->GetCardsMonitoringDB()->Add(newCard);

  if (m_pProcess->GetIsSNMPEnabled() && eRtmIsdn == cardMngr->GetType())
	 {
    CSegment* seg = new CSegment;
    *seg << static_cast<unsigned int>(eTT_ISDNStatus)
         << 2u;
    CManagerApi(eProcessSNMPProcess).SendMsg(seg, SNMP_UPDATE_TELEMETRY_DATA_IND);
	 }
}

void CCardsManager::RemoveFromCardsMonitoringDB(BYTE boardId, BYTE subBoardId)
{
 	m_pProcess->GetCardsMonitoringDB()->Cancel(boardId, subBoardId);
}

void CCardsManager::SendMediaIpParamsReqToCS()
{
	TRACESTR(eLevelInfoNormal) << "CCardsManager::SendMediaIpParamsReqToCS";

	CSegment*  pRetParam = new CSegment;

	const COsQueue* pCsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

	STATUS res = pCsMbx->Send(pRetParam,CS_CARDS_MEDIA_IP_PARAMS_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendIvrMusicGetSourceReqToConfParty()
{
	TRACESTR(eLevelInfoNormal) << "CCardsManager::SendIvrMusicGetSourceReqToConfParty";

	CSegment*  pRetParam = new CSegment;

	const COsQueue* pConfPartyMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessConfParty, eManager);

	STATUS res = pConfPartyMbx->Send(pRetParam,IVR_MUSIC_GET_SOURCE_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnMediaIpParamsFromCsInd(CSegment* pSeg)
{
	// ===== 1. get the new MediaIpParams
	CS_MEDIA_IP_PARAMS_S *pNewMediaIpParamsStruct = (CS_MEDIA_IP_PARAMS_S*)pSeg->GetPtr();

	/// temp - hard coded for integration testing!!!
	/// pNewMediaIpParamsStruct->ipParams.interfacesList[0].ipType = eIpType_Both;
	/// pNewMediaIpParamsStruct->ipParams.interfacesList[0].iPv6s[0].configurationType = eV6Configuration_Manual;
	/// string tmpIPv6Addrss = "2001:470:1f01:115:0:0:0:3";
	/// memset(pNewMediaIpParamsStruct->ipParams.interfacesList[0].iPv6s[0].iPv6Address, 0, IPV6_ADDRESS_LEN);
	/// strncpy((char*)(pNewMediaIpParamsStruct->ipParams.interfacesList[0].iPv6s[0].iPv6Address), tmpIPv6Addrss.c_str(), IPV6_ADDRESS_LEN);
	/// temp end!!!


	// ===== 2. get the new mediaIpParams data into a CCsMediaIpParameters object,
	//          and print the data to trace (using Dump function)
	CCsMediaIpParameters *pNewMediaIpParams = new CCsMediaIpParameters(pNewMediaIpParamsStruct);
	pNewMediaIpParams->SetData(pNewMediaIpParamsStruct);
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnMediaIpParamsFromCsInd\n" << *pNewMediaIpParams;
	POBJDELETE(pNewMediaIpParams);

	// ===== 2. fill CMfaTask's attribute with data from structure received
	SplitMediaIpParamsAndInsertToVector(pNewMediaIpParamsStruct);

	//Begin Startup Feature
	OnMcuMngrIpTypeInd(NULL);
	//End Startup Feature
	m_pProcess->SetStartupCond(eCardsIpService, TRUE);
	RemoveActiveAlarmByErrorCode(AA_NO_IP_SERVICE_PARAMS);
}

//////////////////////////////////////////////////////////////////////
void CCardsManager::OnMcuMngrIpTypeInd(CSegment *pSeg)
{
	eIpType curIpType;
	eV6ConfigurationType curIpv6ConfigType;

	//Begin Startup Feature
	 curIpType						= m_pProcess->m_NetSettings.m_iptype;
	 curIpv6ConfigType	= m_pProcess->m_NetSettings.m_ipv6ConfigType;
	 //End Startup Feature


	// ===== 2. print to trace
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnMcuMngrIpTypeInd - params received from McuMngr:"
								  << "\nIpType:     " << ::IpTypeToString(curIpType)
								  << "\nconfigType: " << ::IpV6ConfigurationTypeToString(curIpv6ConfigType)
//	                              << "\nipv4:       " << ipv4
//	                              << "\nipv6_0:     " << ipv6_0
//	                              << "\nipv6_1:     " << ipv6_1
//	                              << "\nipv6_2:     " << ipv6_2
	                              << "\nMediaIpParameters structs (used in Cards context) are now being updated with IpType and ConfigType...";

	// ===== 3. update the mediaIpParams in process's vector
	CMediaIpParameters* mediaIpParamsFromVector = m_pProcess->GetMediaIpParamsVector()->GetFirst();
	while (mediaIpParamsFromVector)
	{
		mediaIpParamsFromVector->SetIpTypeInAllInterfaces(curIpType);
		mediaIpParamsFromVector->SetIpV6ConfigurationTypeInAllInterfaces(curIpv6ConfigType);

		mediaIpParamsFromVector = m_pProcess->GetMediaIpParamsVector()->GetNext();
	}
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::SplitMediaIpParamsAndInsertToVector(CS_MEDIA_IP_PARAMS_S *pMediaIpParamsStruct)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SplitMediaIpParamsAndInsertToVector";
	// ===== 1. fill CMfaTask's attribute with data from structure received
	//            Note: Interfaces list from CS is cleared
	CMediaIpParameters*  pTempMediaIpParams = new CMediaIpParameters;
	pTempMediaIpParams->SetData(*pMediaIpParamsStruct);

	eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
	ePlatformType curPlatformType = m_pProcess->ConvertProductTypeToPlatformType(curProductType);
	pTempMediaIpParams->SetMediaIpParamsPlatformType(curPlatformType);

	// ===== 2. split the structure by boardId's in the spans
	DWORD i=0, j=0;
	BOOL isSpanExistForCurBoardId=NO;
	IP_INTERFACE_S *curInterface = new IP_INTERFACE_S;
	CIpParameters  *newIpParams  = new CIpParameters;


	for (i = 0; i <= MAX_NUM_OF_BOARDS; i++)
	{
		isSpanExistForCurBoardId = NO;
		CMediaIpParameters *pNewMediaIpParams   = new CMediaIpParameters(*pTempMediaIpParams);
		newIpParams->SetData(pNewMediaIpParams->GetIpParams());

		newIpParams->ClearInterfacesList();
        newIpParams->ClearPortSpeedList();

		for (j = 0; j < MAX_NUM_OF_BOARDS*MAX_NUM_OF_PQS; j++)
		{
			memcpy( curInterface, &(pMediaIpParamsStruct->ipParams.interfacesList[j]), sizeof(IP_INTERFACE_S) );

			if (curInterface->boardId == i)
			{
				newIpParams->InsertToInterfacesList(*curInterface);
				isSpanExistForCurBoardId = YES;
			}
		}

		// ===== if a span exists for current boardId then add the new MediaIpParams to process's list
		if (YES == isSpanExistForCurBoardId)
		{
			memcpy(
			        &(pNewMediaIpParams->GetIpParams().interfacesList),
			        newIpParams->GetIpInterfacesList(),
			        (sizeof(IP_INTERFACE_S)*MAX_NUM_OF_PQS)
			        );

			// setting BoardId and SubBoardId of the CMediaIpParameters.
/*			int slotNum = m_pProcess->GetSlotNumberConvertor().GetPhysicalSlotNumber(i);
*/
			DWORD fixedSubBoardId = FIRST_SUBBOARD_ID;
			pNewMediaIpParams->SetBoardId(i);
			pNewMediaIpParams->SetSubBoardId(fixedSubBoardId);

			//  adding the new MediaIpParams to process's list
			m_pProcess->GetMediaIpParamsVector()->Insert(pNewMediaIpParams);

			// send it to Mfa task (if exists)
			SendNewMediaIpToMfaBoard(pNewMediaIpParams);
		}
		else
		{
			POBJDELETE(pNewMediaIpParams);
		}

	} // end loop (MAX_NUM_OF_BOARDS)


	delete(curInterface);
	POBJDELETE(newIpParams);
	POBJDELETE(pTempMediaIpParams);
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnMediaIpParamsEndFromCsInd()
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnMediaIpParamsEndFromCsInd";
	m_isMediaIpParamsEndReceived = YES;

	if(STATUS_FAIL == m_pProcess->IsCondOk(eCardsIpService))
	{
		UpdateStartupConditionByErrorCode(AA_NO_IP_SERVICE_PARAMS, eStartupConditionFail);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendNewMediaIpToMfaBoard(CMediaIpParameters *pNewMediaIpParams)
{
	DWORD boardId    = pNewMediaIpParams->GetBoardId();
	DWORD subBoardId = pNewMediaIpParams->GetSubBoardId();

	int i=0, j=0;
	for (i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			COsQueue* queue = m_pProcess->GetMfaMbx(i, j);
			if (queue)
			{
				// ===== 1. prepare the data to be sent
				CSegment* pSegToBoard = new CSegment;
				*pSegToBoard << boardId
				             << subBoardId;
				pSegToBoard->Put( (BYTE*)(pNewMediaIpParams->GetMediaIpParamsStruct()), sizeof(MEDIA_IP_PARAMS_S) );

				// ==== 2. send the msg
				CMfaApi mfaApi;
				mfaApi.CreateOnlyApi(*queue);
				mfaApi.SendMsg(pSegToBoard, CARDS_NEW_MEDIA_IP_IND);
			}
		} // end loop over subBoards
	} // end loop over soards
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendLicenseIndToMfaBoard()
{
	int i=0, j=0;
	for (i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			COsQueue* queue = m_pProcess->GetMfaMbx(i, j);
			if (queue)
			{
				// ===== 1. prepare the data to be sent
				CSegment* pSegToBoard = new CSegment;

				// ==== 2. send the msg
				CMfaApi mfaApi;
				mfaApi.CreateOnlyApi(*queue);
				mfaApi.SendMsg(pSegToBoard, CARDS_LICENSE_IND);
			}
		} // end loop over subBoards
	} // end loop over soards
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnIvrMusicAddSourceReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnIvrMusicAddSourceReq";

	m_pProcess->SetIsIvrMusicAddSrcReqReceived(true);

	// ===== 1. get the new ivrAddMusicSource structure
	SIVRAddMusicSource *pStructFromSegment = (SIVRAddMusicSource*)pSeg->GetPtr();

	// ===== 2. copy it to a local structure (for using in the vector)
	SIVRAddMusicSource *pNewIvrAddMusicSource = new SIVRAddMusicSource;
	memcpy(pNewIvrAddMusicSource, pStructFromSegment, sizeof(SIVRAddMusicSource));

	// ===== 3. print the data to trace
	m_pProcess->PrintIvrAddMusicSourceDataToTrace(pNewIvrAddMusicSource, eLevelInfoNormal);

	// ===== 4. if it's a new MusicSource, then insert the structure received to process's vector
	//          (according to confParty guys, a musicSrc with the same id shouldn't be inserted twice).
	DWORD srcId = pNewIvrAddMusicSource->sourceID;
	if ( NULL == m_pProcess->GetIvrAddMusicSourceVector()->Find(srcId) )
	{
		m_pProcess->GetIvrAddMusicSourceVector()->Insert(pNewIvrAddMusicSource);
	}
	else
	{
		delete pNewIvrAddMusicSource;
	}

	RemoveActiveAlarmByErrorCode(AA_NO_MUSIC_SOURCE);

	// ===== 5. each MfaTask should know that the ind has been received, so the MusicSrc can now be sent to the MPM
	SendIvrMusicAddSourceReqReceivedToAllBoards();
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendIvrMusicAddSourceReqReceivedToAllBoards()
{
	int i=0, j=0;
	for (i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			COsQueue* queue = m_pProcess->GetMfaMbx(i, j);
			if (queue)
			{
				CSegment* pSegToBoard = new CSegment; // no real need to allocate an object here
				CMfaApi mfaApi;
				mfaApi.CreateOnlyApi(*queue);
				mfaApi.SendMsg(pSegToBoard, IVR_MUSIC_ADD_SOURCE_RECEIVED_IND);
			}
		} // end loop over subBoards
	} // end loop over soards
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnCardFolderMountInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnCardFolderMountInd";

	// ===== 1. extract the mplMcmsProtocol from segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	// validate data size
	STATUS sizeStat = pMplMcmsProtocol->ValidateDataSize( sizeof(CM_FOLDER_MOUNT_S) );
	if (STATUS_OK != sizeStat)
	{
		POBJDELETE(pMplMcmsProtocol);
		return;
	}

	// ===== 2. get the folderMounting data
	CM_FOLDER_MOUNT_S pFolderMount;
	memset( &pFolderMount, 0, sizeof(CM_FOLDER_MOUNT_S));
	memcpy( &pFolderMount, pMplMcmsProtocol->GetData(), sizeof(CM_FOLDER_MOUNT_S) );

	// ===== 3. print the data to log
	m_pProcess->PrintFolderMountDataToTrace(pFolderMount);

	// ===== 4. produce Active Alarm, if needed
	if (STATUS_OK != pFolderMount.ulMounted)
	{
		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
				AA_CARD_FOLDER_MOUNTING_FAILED,
				MAJOR_ERROR_LEVEL,
				"Failed to mount card folder",
                 FALSE);
	}
	POBJDELETE(pMplMcmsProtocol);
}

/*
/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnIvrFolderMountInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnIvrFolderMountInd";

	// ===== 1. extract the mplMcmsProtocol from segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	// ===== 2. get the folderMounting data
	CM_FOLDER_MOUNT_S pFolderMount;
	memset( &pFolderMount, 0, sizeof(CM_FOLDER_MOUNT_S));
	memcpy( &pFolderMount, pMplMcmsProtocol->GetData(), sizeof(CM_FOLDER_MOUNT_S) );

	// ===== 3. print the data to log
	m_pProcess->PrintFolderMountDataToTrace(pFolderMount);

	// ===== 4. send msg to appropriate task, if needed (the actual treatment is in the board task)
	if (STATUS_OK != pFolderMount.ulMounted)
	{
		SendIvrFolderMountIndToAppripriateBoard(pMplMcmsProtocol);
	}

	POBJDELETE(pMplMcmsProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendIvrFolderMountIndToAppripriateBoard(CMplMcmsProtocol* pProtocol)
{
	STATUS resStatus = STATUS_OK;

	// ===== 1. prepare the data to be sent
	CSegment* pSegToBoard = new CSegment;
	pSegToBoard->Put((unsigned char*)pProtocol->GetData(), pProtocol->getDataLen());

	// ===== 2. get Board's Mbx
	BYTE msgBoardId   = pProtocol->getPhysicalInfoHeaderBoard_id(),
	     msgSubBoardId = pProtocol->getPhysicalInfoHeaderSub_board_id();
	COsQueue* queue    = m_pProcess->GetMfaMbx(msgBoardId, msgSubBoardId);

	if (!queue)
	{
		// ===== 3. print to trace
    	TRACESTR(eLevelInfoNormal)
    		<< "\nCCardsManager::SendIvrFolderMountIndToAppripriateBoard: No Mbx for "
			<< "boardId: " << (WORD)msgBoardId << ", subBoardId: " << (WORD)msgSubBoardId;

		// ===== 4. produce Active Alarm
		AddActiveAlarm( FAULT_CARD_SUBJECT, AA_IVR_FOLDER_MOUNTING_FAILED, MAJOR_ERROR_LEVEL, "", true, true, 0, msgBoardId);
	}

	else // queue ok
	{
		// ===== 5. send the message
		CMfaApi mfaApi;
		mfaApi.CreateOnlyApi(*queue);
		resStatus = mfaApi.SendMsg(pSegToBoard, CM_IVR_FOLDER_MOUNT_IND);
	}
}
*/


////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnEmbUserMsgInd(CSegment* pSeg)
{
	// ===== 1. extract the mplMcmsProtocol from segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	// validate data size
	STATUS sizeStat = pMplMcmsProtocol->ValidateDataSize( sizeof(USER_MSG_S) );
	if (STATUS_OK != sizeStat)
	{
		POBJDELETE(pMplMcmsProtocol);
		return;
	}


	// ===== 2. retrieve  data, extract boardId & subBoardId and print
	USER_MSG_S theMsg;
	memcpy( &theMsg, pMplMcmsProtocol->GetData(), sizeof(USER_MSG_S) );

	BYTE msgBoardId		= pMplMcmsProtocol->getPhysicalInfoHeaderBoard_id(),
		 msgSubBoardId	= pMplMcmsProtocol->getPhysicalInfoHeaderSub_board_id();

	int boardId		= (int)msgBoardId,
		subBoardId	= (int)msgSubBoardId;

	PrintEmbUsrMsg(&theMsg, boardId, subBoardId);


	// ===== 3. handle
	HandleEmbUserMsg(&theMsg, boardId, subBoardId);
	POBJDELETE(pMplMcmsProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::PrintEmbUsrMsg(USER_MSG_S *msgStruct, int boardId, int subBoardId)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::PrintEmbUsrMsg"
					<< "\nBoardId: " << boardId << ", subBoardId: " <<subBoardId
	                << "\nMessageCode: "	<< GetUserMsgCodeEmbStr((eUserMsgCode_Emb)msgStruct->messageCode)
	                << "\nLocation:    "	<< GetUserMsgLocationStr((eUserMsgLocation)msgStruct->location)
	                << "\nOperation:   "	<< GetUserMsgOperationStr((eUserMsgOperation)msgStruct->operation)
	                << "\nAutoRemoval: "	<< GetUserMsgAutoRemovalStr((eUserMsgAutoRemoval)msgStruct->autoRemoval)
	                << "\nProcessType: "	<< GetUserMsgProcessType_EmbStr((eUserMsgProcessType_Emb)msgStruct->process_type);
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::HandleEmbUserMsg(USER_MSG_S *msgStruct, int boardId, int subBoardId)
{
	eUserMsgOperation theOperation = (eUserMsgOperation)msgStruct->operation;
	switch (theOperation)
	{
		case eUSerMsgOperation_add:
		{
			AddEmbUserMsg(msgStruct, boardId, subBoardId);
			break;
		}

		case eUSerMsgOperation_remove:
		{
			RemoveEmbUserMsg(msgStruct, boardId, subBoardId);
			break;
		}

		default:
		{
			PASSERTMSG(true, "illegal UserMsgOperation");
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::AddEmbUserMsg(USER_MSG_S *msgStruct, int boardId, int subBoardId)
{
	string sDesc = ConvertUserMsgToString((eUserMsgCode_Emb)(msgStruct->messageCode));

	eUserMsgLocation theLocation = (eUserMsgLocation)msgStruct->location;
	switch (theLocation)
	{
		// ===== 1. to be added to Alerts list
		case eUserMsgLocation_SysAlerts:
		{
			DWORD identifier = m_pProcess->CreateIdFromBoardIdMsgId( boardId,
																	 (int)(msgStruct->messageCode),
																	 (int)(msgStruct->autoRemoval) );

			WORD alertType = ( (boardId == m_pProcess->GetAuthenticationStruct()->switchBoardId)
							   ?
							   FAULT_TYPE_SWITCH : FAULT_TYPE_MPM );
		if (m_isStartupOver == true || msgStruct->messageCode == eUserMsgMessage_Emb_Card_not_supported_with_old_PSU
				     || msgStruct->messageCode == eUserMsgMessage_Emb_Card_not_supported_with_old_CNTL) {
			AddActiveAlarm(FAULT_CARD_SUBJECT, AA_EXTERNAL_ALERT_EMB,
					MAJOR_ERROR_LEVEL, sDesc.c_str(), true, true, identifier,
					boardId, 0, alertType);
		} else {

			AddActiveAlarm(FAULT_CARD_SUBJECT, AA_EXTERNAL_ALERT_EMB,
					MAJOR_ERROR_LEVEL, sDesc.c_str(), false, false, identifier,
					boardId, 0, alertType);

		}

	        break;
		}

		// ===== 2. to be added to Faults list only
		case eUserMsgLocation_Faults:
		{
			CHlogApi::EmbUserMsgFault(boardId, subBoardId, sDesc, FAULT_TYPE_MPM);

			TRACESTR(eLevelInfoNormal) << "\nCCardsManager::AddEmbUserMsg"
										  << "\nA fault was added (boardId " << boardId << ", subBoardId " << subBoardId << ")"
										  << "\nThe description: " << sDesc.c_str();

			break;
		}

		default:
		{
			PASSERTMSG(true, "illegal USerMsgLocation");
			break;
		}
	} // end switch (theLocation)
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::RemoveEmbUserMsg(USER_MSG_S *msgStruct, int boardId, int subBoardId)
{
	string sDesc = ConvertUserMsgToString((eUserMsgCode_Emb)(msgStruct->messageCode));

	DWORD identifier = m_pProcess->CreateIdFromBoardIdMsgId( boardId,
															 (int)(msgStruct->messageCode),
															 (int)(msgStruct->autoRemoval) );

	if ( IsActiveAlarmExistByErrorCodeUserId(AA_EXTERNAL_ALERT_EMB, identifier) )
	{
		RemoveActiveAlarmByErrorCodeUserId(AA_EXTERNAL_ALERT_EMB, identifier);

		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::RemoveEmbUserMsg"
									  << "\nBoardId " << boardId << ", subBoardId " << subBoardId
									  << "\nThe description: " << sDesc.c_str();
	}

	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::RemoveEmbUserMsg"
									  << "\nBoardId " << boardId << ", subBoardId " << subBoardId
									  << "\nThe description: " << sDesc.c_str()
									  << "\nAlert was not added, hence was not removed";
	}
}

/////////////////////////////////////////////////////////////////////////////
string CCardsManager::ConvertUserMsgToString(eUserMsgCode_Emb theCode)
{
	string retStr = "";

	switch (theCode)
	{
		case (eUserMsgMessage_Emb_Test):
		{
			retStr = EMB_USE_MSG_DESC_TEST;
			break;
		}
		case (eUserMsgMessage_Emb_UBootFlashFailure):
		{
			retStr = EMB_USE_MSG_DESC_UBOOT_FLASH_FAILURE;
			break;
		}
		case (eUserMsgMessage_Emb_FpgaVersLoadesFailure):
		{
			retStr = EMB_USE_MSG_DESC_FPGA_VERSION_LOAD_FAILURE;
			break;
		}
		case (eUserMsgMessage_Emb_FAN_SpeedBelowMinimum):
		{
			retStr = EMB_USE_MSG_DESC_FAN_SPEED_BELOW_MIN;
			break;
		}
		case (eUserMsgMessage_Emb_FAN_NoPower):
		{
			retStr = EMB_USE_MSG_DESC_FSM_FAN_NO_POWER;
			break;
		}
		case (eUserMsgMessage_Emb_FSM_NoCard):
		{
			retStr = EMB_USE_MSG_DESC_FSM_NO_CARD;
			break;
		}
		case (eUserMsgMessage_Emb_FSM4000_LoadingProblem):
		{
			retStr = FAILED_TO_LOAD_FSM_4000_SOFTWARE;
			break;
		}
		case (eUserMsgMessage_Emb_FSM4000_NoLinkToCard1):
		{
			retStr = NO_LINK_BETWEEN_FSM_4000_AND_CARD_IN_SLOT_1;
			break;
		}
		case (eUserMsgMessage_Emb_FSM4000_NoLinkToCard2):
		{
			retStr = NO_LINK_BETWEEN_FSM_4000_AND_CARD_IN_SLOT_2;
			break;
		}
		case (eUserMsgMessage_Emb_FSM4000_NoLinkToCard3):
		{
			retStr = NO_LINK_BETWEEN_FSM_4000_AND_CARD_IN_SLOT_3;
			break;
		}
		case (eUserMsgMessage_Emb_FSM4000_NoLinkToCard4):
		{
			retStr = NO_LINK_BETWEEN_FSM_4000_AND_CARD_IN_SLOT_4;
			break;
		}
		case (eUserMsgMessage_Emb_FLASH_Erase):
		{
			retStr = CARD_RECOVERY_COMPLETED;
			break;
		}
		case (eUserMsgMessage_Emb_Card1_not_supported):
		{
			retStr = CARD1_NOT_SUPPORTED;
			break;
		}
		case (eUserMsgMessage_Emb_Card2_not_supported):
		{
			retStr = CARD2_NOT_SUPPORTED;
			break;
		}
		case (eUserMsgMessage_Emb_Card3_not_supported):
		{
			retStr = CARD3_NOT_SUPPORTED;
			break;
		}
		case (eUserMsgMessage_Emb_Card4_not_supported):
		{
			retStr = CARD4_NOT_SUPPORTED;
			break;
		}
		case (eUserMsgMessage_Emb_Card_not_supported_with_old_PSU):
		{
			retStr = CARD_NOT_SUPPORTED_OLD_PSU;
			break;
		}
		case (eUserMsgMessage_Emb_Card_not_supported_with_old_CNTL):
		{
			retStr = CARD_NOT_SUPPORTED_OLD_CNTL;
			break;
		}
		default:
		{
			retStr = EMB_USE_MSG_DESC_ILLEGAL;
			PASSERTMSG(100+theCode, EMB_USE_MSG_DESC_ILLEGAL);
			break;
		}
	} // end switch (theCode)

	return retStr;
}



/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnCardsResetAllBoardsReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnCardsResetAllBoardsReq";

	BYTE switchBoardId    = 0,
	     switchSubBoardId = FIRST_SUBBOARD_ID;
	*pSeg >> switchBoardId;

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(CARDS_RESTART_AUTHENTICATION_PROCEDURE_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, switchBoardId, switchSubBoardId);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnDeleteIpServiceReq(CSegment* pSeg)
{
	// ===== 1. get the structure from segment
	Del_Ip_Service_S *pDelIpServiceStruct = (Del_Ip_Service_S*)pSeg->GetPtr();

 	// ===== 2. print the data to trace
	TRACESTR(eLevelInfoNormal)
	    << "\nCCardsManager::OnDeleteIpService"
	    << "\nService Id: "   << pDelIpServiceStruct->service_id
		<< ", Service Name: " << pDelIpServiceStruct->service_name;

	// ===== 3. delete from process's list
	m_pProcess->GetMediaIpParamsVector()->Remove(pDelIpServiceStruct->service_id);

	// ===== 4. send the deletion request to MplApi
	SendDeleteIpServiceReqToMplApi(pDelIpServiceStruct);
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendDeleteIpServiceReqToMplApi(Del_Ip_Service_S *pDelSrv)
{
	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol;

	mplPrtcl->AddCommonHeader(CM_DELETE_IP_SERVICE_REQ);
	mplPrtcl->AddMessageDescriptionHeader();

//	mplPrtcl->AddPhysicalHeader(0, m_boardId, m_subBoardId);  // boxId - TEMP!!

	mplPrtcl->AddData(sizeof(Del_Ip_Service_S), (char*)&pDelSrv);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPLAPI");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();
	POBJDELETE(mplPrtcl);
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnDeleteIpServiceInd(CSegment* pSeg)
{
	// ===== 1. extract delIpService struct from the segment received
	CM_DELETE_IP_SERVICE_S* pDelIpServ = (CM_DELETE_IP_SERVICE_S*)pSeg->GetPtr();
	//CM_DELETE_IP_SERVICE_S* pDdelIpSrv = new CM_DELETE_IP_SERVICE_S;
	//         pSeg->Get( (BYTE*)(pDdelIpSrv), sizeof(CM_DELETE_IP_SERVICE_S) );

 	// ===== 2. print the data to trace
	TRACESTR(eLevelInfoNormal)
	    << "\nCCardsManager::OnDeleteIpServiceInd"
	    << "\nService Id: " << pDelIpServ->service_id
		<< ", Status: "     << pDelIpServ->status;

	// ===== 3. handle the message
	if ( STATUS_OK != pDelIpServ->status)
	{
		TRACESTR(eLevelInfoNormal) << "CCardsManager::OnDeleteIpServiceInd - failed to delete ip service";

		/*  -------------------------------------------- */
		/*  ------ a. add servicName to the TRACE ------ */
		/*  ------ b. do something with the failure ---- */
		/*  -------------------------------------------- */
	}
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnSysConfigTableChanged(const string &key, const string &data)
{
	if(CFG_KEY_DEBUG_MODE == key)
	{
		CSmallString logStr = "\nCCardsManager::OnSysConfigTableChanged - ";
		logStr << "Key: " << key.c_str() << ", Value: " << data.c_str();
		TRACESTR(eLevelInfoNormal) << logStr.GetString();

		SendDebugModeChangedToAllCards(data);
	}
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendDebugModeChangedToAllCards(const string &data)
{
	CSmallString logStr = "\nCCardsManager::SendDebugModeChangedToAllCards - ";
	logStr << "Data: " << data.c_str();
	TRACESTR(eLevelInfoNormal) << logStr.GetString();

	int fixedSubBoardId = FIRST_SUBBOARD_ID;
	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			COsQueue* queue = m_pProcess->GetMfaMbx(i, j);
			if (queue)
			{
				SendDebugModeChangedToSpecificCard(i, j, data);
			}
		} // end loop over subBoards
	} // end loop over boards
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendDebugModeChangedToSpecificCard(DWORD boardId, DWORD subBoardId, const string &data)
{
	BOOL isLegalValue = YES;
	CLargeString logStr = "\nCCardsProcess::SendDebugModeChangedToSpecificCard - ";
	logStr << "boardId: " << boardId << ", subBoardId: " << subBoardId;

	// ===== 2. send to Mfa
	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	if (CFG_STR_YES == data)
	{
		mplPrtcl->AddCommonHeader(DEBUG_MODE_YES_REQ);
		logStr << "\nDebugMode flag: " << CFG_STR_YES;
	}
	else if (CFG_STR_NO == data)
	{
		mplPrtcl->AddCommonHeader(DEBUG_MODE_NO_REQ);
		logStr << "\nDebugMode flag: " << CFG_STR_NO;
	}
	else
	{
		isLegalValue = NO;
		logStr << "\nIllegal DebugMode flag: " << data.c_str() << "! nothing is sent to MPL";
	}

	if (YES == isLegalValue)
	{
		mplPrtcl->AddMessageDescriptionHeader();
		mplPrtcl->AddPhysicalHeader(1, boardId, subBoardId);

		CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL_API");
		mplPrtcl->SendMsgToMplApiCommandDispatcher();
	}

	POBJDELETE(mplPrtcl);

	TRACESTR(eLevelInfoNormal) << logStr.GetString();
}



////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendUpgradeReqToAllCards()
{
	int fixedSubBoardId = FIRST_SUBBOARD_ID;
	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		COsQueue* queue = m_pProcess->GetMfaMbx(i, 1);
		if (queue)
		{
		    CCommCardDB* pCardDB = m_pProcess->GetCardsMonitoringDB();
		    eCardType curCardType = pCardDB->GetCardType(i, 1);
		    TRACESTR(eLevelInfoNormal) << "CCardsManager::SendUpgradeReqToAllCards - card mode "<< m_pProcess->GetNGBSystemCardsMode()
		    << " curCardType " << curCardType;

		    if (/*m_pProcess->GetNGBSystemCardsMode() == eNGBSystemCardsMode_mixed_mode  && */(curCardType == eMpmRx_Half || curCardType == eMpmRx_Full) && (m_isNewVersionBlockedForMPMRX == true))
		    {
		        //Block further communication with this card
		        CConfigManagerApi apiConfigurator;
		        std::string ip = GetEmbBoardIPAddress(i);
		        if("" != ip)
		            apiConfigurator.AddDropRuleToShorewall(ip);
		        TRACESTR(eLevelInfoNormal) << "CCardsManager::SendUpgradeReqToAllCards - we are in mixed mode mpmrx card upgrade is blocked";
		    }
		    else
		        SendUpgradeReqToSpecificCard(i, 1);
		}
		//Block in fw, deny insertion of new board while installing
		else
		{
			//Block further communication with this card
			CConfigManagerApi apiConfigurator;
			std::string ip = GetEmbBoardIPAddress(i);
			if("" != ip)
				apiConfigurator.AddDropRuleToShorewall(ip);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendUpgradeReqToSpecificCard(DWORD boardId, DWORD subBoardId)
{
	m_pProcess->UpdateSoftwareProgress(boardId,0);
	m_pProcess->AddBoardInstallingIpmc(boardId); // by default every board required IPMC
	m_pProcess->IncBoardIpmcIndCounter(boardId);

    CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();


    mplPrtcl->AddCommonHeader(CM_UPGRADE_NEW_VERSION_READY_REQ);

    mplPrtcl->AddMessageDescriptionHeader();
    mplPrtcl->AddPhysicalHeader(1, boardId, subBoardId);

    TRACESTR(eLevelInfoNormal) << "SendUpgradeReqToSpecificCard" << boardId << "  " << subBoardId;

    CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL_API");
    mplPrtcl->SendMsgToMplApiCommandDispatcher();


    POBJDELETE(mplPrtcl);
}

//////////////////////////////////////////////////////////////////////
std::string CCardsManager::GetEmbBoardIPAddress(DWORD board_id)
{
	std::string ip = "";
	switch(board_id)
	{
		case(1):
		{
			ip = MEDIA_CARD_1_IP_ADDRESS;
			break;
		}
		case(2):
		{
			ip = MEDIA_CARD_2_IP_ADDRESS;
			break;
		}
		case(3):
		{
			ip = MEDIA_CARD_3_IP_ADDRESS;
			break;
		}
		case(4):
		{
			ip = MEDIA_CARD_4_IP_ADDRESS;
			break;
		}
	}
	return ip;
}


////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendIpmcUpgradeReqToAllCards()
{
	TRACESTR(eLevelInfoNormal) << "CCardsManager::SendIpmcUpgradeReqToAllCards";



	int fixedSubBoardId = FIRST_SUBBOARD_ID;
	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			COsQueue* queue = m_pProcess->GetMfaMbx(i, j);
			if (queue)
			{

			    CCommCardDB* pCardDB = m_pProcess->GetCardsMonitoringDB();
			    eCardType curCardType = pCardDB->GetCardType(i, 1);
			    if (/*m_pProcess->GetNGBSystemCardsMode() == eNGBSystemCardsMode_mixed_mode  &&*/ (curCardType == eMpmRx_Half || curCardType == eMpmRx_Full) && (m_isNewVersionBlockedForMPMRX == true))
			    {
			        //Block further communication with this card
			        CConfigManagerApi apiConfigurator;
			        std::string ip = GetEmbBoardIPAddress(i);
			        if("" != ip)
			            apiConfigurator.AddDropRuleToShorewall(ip);
			        TRACESTR(eLevelInfoNormal) << "CCardsManager::SendIpmcUpgradeReqToAllCards - we are in mixed mode mpmrx card upgrade is blocked";
			    }
			    else

			        SendIpmcUpgradeReqToSpecificCard(i, j);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendIpmcUpgradeReqToAllCardsRequireIpmc()
{
	TRACESTR(eLevelInfoNormal) << "CCardsManager::SendIpmcUpgradeReqToAllCardsRequireIpmc";

	int num_of_ipmc_required = 0;

	int fixedSubBoardId = FIRST_SUBBOARD_ID;
	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			COsQueue* queue = m_pProcess->GetMfaMbx(i, j);
			if (queue)
			{
			    CCommCardDB* pCardDB = m_pProcess->GetCardsMonitoringDB();
			    eCardType curCardType = pCardDB->GetCardType(i, 1);
			    if (/*m_pProcess->GetNGBSystemCardsMode() == eNGBSystemCardsMode_mixed_mode  &&*/ (curCardType == eMpmRx_Half || curCardType == eMpmRx_Full) && (m_isNewVersionBlockedForMPMRX == true))
			    {
			        //Block further communication with this card
			        CConfigManagerApi apiConfigurator;
			        std::string ip = GetEmbBoardIPAddress(i);
			        if("" != ip)
			            apiConfigurator.AddDropRuleToShorewall(ip);
			        TRACESTR(eLevelInfoNormal) << "CCardsManager::SendIpmcUpgradeReqToAllCardsRequireIpmc we block mpmrx upgrade";
			    }
			    else if (m_pProcess->TestIfCardNeedIpmcUpgrade(i))
			    {
			        num_of_ipmc_required++;
			        SendIpmcUpgradeReqToSpecificCard(i, j);
			    }
			}
		}
	}

	if (num_of_ipmc_required == 0)
	{
		CManagerApi api(eProcessInstaller);
		api.SendOpcodeMsg(INSTALLER_LAST_CARD_IPMC_FINISHED);

		//ipmc part is skiped
		// perform reset now !!!
		//PASSERT(1);
	}
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendIpmcUpgradeReqToSpecificCard(DWORD boardId, DWORD subBoardId)
{
    CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();


    mplPrtcl->AddCommonHeader(CM_UPGRADE_START_WRITE_IPMC_REQ);

    mplPrtcl->AddMessageDescriptionHeader();
    mplPrtcl->AddPhysicalHeader(1, boardId, subBoardId);

    TRACESTR(eLevelInfoNormal) << "SendIpmcUpgradeReqToSpecificCard" << boardId << "  " << subBoardId;

    CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL_API");
    mplPrtcl->SendMsgToMplApiCommandDispatcher();


    POBJDELETE(mplPrtcl);

}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendResetReqToAllCards()
{
	int fixedSubBoardId = FIRST_SUBBOARD_ID;
	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			COsQueue* queue = m_pProcess->GetMfaMbx(i, j);
			if (queue)
			{
				SendResetReqToSpecificCard(i, j);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendResetReqToSpecificCard(DWORD boardId, DWORD subBoardId)
{
    CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();


    mplPrtcl->AddCommonHeader(RESET_CARD_REQ);

    mplPrtcl->AddMessageDescriptionHeader();
    mplPrtcl->AddPhysicalHeader(1, boardId, subBoardId);

    TRACESTR(eLevelInfoNormal) << "SendIpmcUpgradeReqToSpecificCard" << boardId << "  " << subBoardId;

    CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL_API");
    mplPrtcl->SendMsgToMplApiCommandDispatcher();


    POBJDELETE(mplPrtcl);

}


////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnVersionIsReady(CSegment* pSeg)
{
    TRACESTR(eLevelInfoNormal) << "CCardsManager::OnVersionIsReady";
    m_pProcess->ResetBoardInstalling();
    m_pProcess->ResetBoardInstallingIpmc();

    DWORD temp_flag;
    *pSeg >> temp_flag;
    m_isNewVersionBlockedForMPMRX = (bool) temp_flag;

    TRACESTR(eLevelInfoNormal) << "CCardsManager::OnVersionIsReady m_isNewVersionBlockedForMPMRX " << m_isNewVersionBlockedForMPMRX ;

    SendUpgradeReqToAllCards();
    SendUpgradeStartedIndToMplApi();
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnLastCardInstallationFinished(CSegment* pSeg)
{
    TRACESTR(eLevelInfoNormal) << " CCardsManager::OnLastCardInstallationFinished ";
    if(m_pProcess->GetBoardIpmcIndCounter() == 0)
    {
    	SendIpmcUpgradeReqToAllCardsRequireIpmc();
    	//   SendIpmcUpgradeReqToAllCards();
    	// Update Installer that this state is completed
    	CManagerApi api(eProcessInstaller);
    	api.SendOpcodeMsg(INSTALLER_LAST_CARD_INSTALLATION_FINISHED);
    }
    else
    	StartTimer(CHECK_ALL_IPMC_IND_RECEIVED, 0 *SECOND);
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnCheckAll_IPMC_IND_received(CSegment* pSeg)
{
    TRACESTR(eLevelInfoNormal) << " CCardsManager::OnCheckAll_IPMC_IND_received ";
    if(m_pProcess->GetBoardIpmcIndCounter() == 0)
    {
    	DeleteTimer(TOTAL_IPMC_IND_RECEIVED_TOUT);
    	SendIpmcUpgradeReqToAllCardsRequireIpmc();
    	//   SendIpmcUpgradeReqToAllCards();
    	// Update Installer that this stae is completed
    	CManagerApi api(eProcessInstaller);
    	api.SendOpcodeMsg(INSTALLER_LAST_CARD_INSTALLATION_FINISHED);
    }
    else
    {
    	StartTimer(CHECK_ALL_IPMC_IND_RECEIVED, 10 *SECOND);
    	StartTimer(TOTAL_IPMC_IND_RECEIVED_TOUT, 120 * SECOND);
    }
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnIPMC_IND_receivedTOUT(CSegment* pSeg)
{
	string retStr = "CCardsManager::OnIPMC_IND_receivedTOUT - failed to receive IPMC_IND from all cards. ";
	PASSERTMSG(1, retStr.c_str());

	SendIpmcUpgradeReqToAllCardsRequireIpmc();
	//   SendIpmcUpgradeReqToAllCards();
	// Update Installer that this stae is completed
	CManagerApi api(eProcessInstaller);
	api.SendOpcodeMsg(INSTALLER_LAST_CARD_INSTALLATION_FINISHED);
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnResetAllCardsReqFromInstaller(CSegment* pSeg)
{
  eProductType type = m_pProcess->GetProductType();
  TRACECOND_AND_RETURN(type >= eProductTypeSoftMCU,
    "Skipped cards reset for " << ProductTypeToString(type));

  TRACEINTO << "Reset cards for " << ProductTypeToString(type);
  SendResetReqToAllCards();
}


////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnNewSysCfgParamsInd(CSegment* pSeg)
{
	BOOL bIsJITCMode = FALSE;
	BOOL bIsDebugMode = FALSE;
	BOOL bIsSeparatedNetworks = FALSE;
	BOOL bIsMultipleServices = FALSE;

	*pSeg >> bIsDebugMode
          >> bIsJITCMode
          >> bIsSeparatedNetworks
          >> bIsMultipleServices;

	SendNewSysCfgParamsToAllCards(bIsDebugMode,
                                  bIsJITCMode,
                                  bIsSeparatedNetworks,
                                  bIsMultipleServices);

}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendNewSysCfgParamsToAllCards(BOOL bDebugMode,
                                                  BOOL bJITCMode,
                                                  BOOL bSeparatedNetworks,
                                                  BOOL bMultipleServices)
{
	TRACEINTO << __PRETTY_FUNCTION__ << ": "
              << "Debug: " << bDebugMode
	          << " JITC: " << bJITCMode
	          << " bSeparatedNetworks: " << bSeparatedNetworks
	          << " bMultipleServices: " << bMultipleServices;

	WORD i=0, j=0;
	for (i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			COsQueue* queue = m_pProcess->GetMfaMbx(i, j);
			if (queue)
			{
				// ===== 1. prepare the data to be sent
				CSegment* pSegToBoard = new CSegment;
				*pSegToBoard << i
							 << j
							 << (DWORD)bDebugMode
							 << (DWORD)bJITCMode
							 << (DWORD)bSeparatedNetworks
                             << (DWORD)bMultipleServices;

				// ==== 2. send the msg
				CMfaApi mfaApi;
				mfaApi.CreateOnlyApi(*queue);
				mfaApi.SendMsg(pSegToBoard, CARDS_NEW_SYS_CFG_PARAMS_IND);
			}
		} // end loop over subBoards
	} // end loop over soards
}

/////////////////////////////////////////////////////////////////////
STATUS CCardsManager::HandleTerminalCardsState(CTerminalCommand & command,std::ostream& answer)
{
	CCommCardDB* pCardDB = m_pProcess->GetCardsMonitoringDB();
	WORD fixedSubBoardId = FIRST_SUBBOARD_ID;

	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			eCardType curCardType = pCardDB->GetCardType(i, j);

			if ( (eEmpty != curCardType) &&									// not empty
			     (NUM_OF_CARD_TYPES > curCardType) && (0 <= curCardType) )	// yet legal type
			{
				eCardState curCardState = pCardDB->GetCardState(i, j);

				answer << "Board "		<< i
				       << ", SubBoard "	<< j << " -"
				       << " type: "		<< ::CardTypeToString(curCardType)
				       << ", state: "	<< ::CardStateToString(curCardState)
				       << "\n";
			}
		}
	} // end loop over m_pCard

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
STATUS CCardsManager::HandleTerminalResetReqToDaemon(CTerminalCommand & command,std::ostream& answer)
{
	answer << "Reset request is sent from Cards process to McmsDaemon process";

	m_pProcess->SendResetReqToDaemon("reset from Cards process - simulation");

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
STATUS CCardsManager::HandleTerminalSetDebugModeMpl(CTerminalCommand & command,std::ostream& answer)
{
	const DWORD numOfParams = command.GetNumOfParams();
	if(numOfParams != 1)
	{
		answer << "Error" << endl;
		answer << "Usage: ca set_debug_mode_mpl Cards [YES/NO]";
		return STATUS_OK;
	}

	const string &data = command.GetToken(eCmdParam1);
 	answer << "Debug_mode " << data.c_str() << " is being sent to all cards";

   	SendDebugModeChangedToAllCards(data);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
STATUS CCardsManager::HandleTerminalEnableDisableRtmIsdnKeepAlive(CTerminalCommand & command,std::ostream& answer)
{
	const DWORD numOfParams = command.GetNumOfParams();
	if(numOfParams != 1)
	{
		answer << "Error" << endl;
		answer << "Usage: ca rtm_isdn_ka Cards [YES/NO]";
		return STATUS_OK;
	}

	const string &data = command.GetToken(eCmdParam1);
	if ("YES" == data)
	{
		m_pProcess->SetIsToTreatRtmIsdnKeepAlive(YES);
	 	answer << "\nRTM_ISDN KeepAlive will be treated\n";
	}
	else
	{
		m_pProcess->SetIsToTreatRtmIsdnKeepAlive(NO);
	 	answer << "\nRTM_ISDN KeepAlive will not be treated\n";
	}

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////
STATUS CCardsManager::HandleTerminalSysCardsMode(CTerminalCommand & command,std::ostream& answer)
{
	eSystemCardsMode modeCur	= m_pProcess->GetSystemCardsModeCur();
	eSystemCardsMode modeFile	= m_pProcess->GetSystemCardsModeFromFile();

	answer << "System's cards mode:"
		   << "\nActual:      " << ::GetSystemCardsModeStr(modeCur)
		   << "\nIn the file: " << ::GetSystemCardsModeStr(modeFile);

	return STATUS_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCardsManager::HandleTerminalEthTest(CTerminalCommand & command,std::ostream& answer)
{
	ETH_SETTINGS_SPEC_S pEthSettings;

	memset(&pEthSettings, 0, sizeof(pEthSettings));
	
	pEthSettings.portParams.slotId = 8;
	pEthSettings.portParams.portNum = 6;

    CSegment* pRetSeg = new CSegment;
    pRetSeg->Put( (BYTE*)&pEthSettings, sizeof(ETH_SETTINGS_SPEC_S) );
    OnEthSettingMonitoringFromSysMonitor(pRetSeg);

    return STATUS_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCardsManager::HandleTestUpgrade(CTerminalCommand & command,std::ostream& answer)
{
    m_pProcess->ResetBoardInstalling();
    m_pProcess->ResetBoardInstallingIpmc();

    SendUpgradeReqToAllCards();
    return STATUS_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCardsManager::HandleTestIpmcUpgrade(CTerminalCommand & command,std::ostream& answer)
{

    SendIpmcUpgradeReqToAllCards();
    return STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCardsManager::HandleTestResetAllCards(CTerminalCommand & command,std::ostream& answer)
{

    SendResetReqToAllCards();
    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
STATUS CCardsManager::HandleTerminalUsrMsgEmb(CTerminalCommand & command,std::ostream& answer)
{
	if(4 != command.GetNumOfParams())
	{
		answer << "Error" << endl;
		answer << "Usage: ca usr_msg_emb Cards [boradId] [msgCode] [location] [operation]";
		return STATUS_OK;
	}

	const string &sBoardId	= command.GetToken(eCmdParam1),
				 &sMsgCode	= command.GetToken(eCmdParam2),
				 &sLocation	= command.GetToken(eCmdParam3),
				 &sOperation	= command.GetToken(eCmdParam4);

	int boardId = atoi(sBoardId.c_str());

	USER_MSG_S theMsg;
	memset( &theMsg, 0, sizeof(USER_MSG_S) );
	theMsg.messageCode	= atoi(sMsgCode.c_str());
	theMsg.location		= atoi(sLocation.c_str());
	theMsg.operation	= atoi(sOperation.c_str());


	PrintEmbUsrMsg(&theMsg, boardId, 1/*subBoardId*/);

	HandleEmbUserMsg(&theMsg, boardId, 1/*subBoardId*/);

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////
STATUS CCardsManager::HandleTerminalKaRecievedTimeout(CTerminalCommand & command,std::ostream& answer)
{
		m_pProcess->GetCardsMonitoringDB()->m_unAnswerdKA[0];

		answer << "get to HandleTerminalKaRecievedTimeou t"<< endl;;
		answer <<  m_pProcess->GetCardsMonitoringDB()->m_unAnswerdKA[0];
		return STATUS_OK;

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnRtmIsdnParamsInd(CSegment* pSeg)
{
 	// ===== 1. extract data
	RTM_ISDN_PARAMS_MCMS_S* pRtmIsdnParams = new RTM_ISDN_PARAMS_MCMS_S;
	pSeg->Get( (BYTE*)pRtmIsdnParams, sizeof(RTM_ISDN_PARAMS_MCMS_S) );

	// ===== 2. print
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnRtmIsdnParamsInd";
	m_rtmIsdnCommonMethods.PrintRtmIsdnParamsMcmsStruct(*pRtmIsdnParams, "CCardsManager::OnRtmIsdnParamsInd");

	// ===== 3. add to process's attribute
	STATUS addStat = m_pProcess->AddRtmIsdnParamsToList(*pRtmIsdnParams);
	if (STATUS_OK != addStat)
	{
		string retStr = "CCardsManager::OnRtmIsdnParamsInd - failed to add RtmIsdnParams; status: ";
		retStr += m_pProcess->GetStatusAsString(addStat);
		PASSERTMSG(1, retStr.c_str());
	}

	delete pRtmIsdnParams;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnRtmIsdnSpanMapsInd(CSegment* pSeg)
{
 	// ===== 1. extract data
    RTM_ISDN_SPAN_MAPS_LIST_S* pRtmIsdnSpanMapsList = new RTM_ISDN_SPAN_MAPS_LIST_S;
	pSeg->Get( (BYTE*)pRtmIsdnSpanMapsList, sizeof(RTM_ISDN_SPAN_MAPS_LIST_S) );

	// ===== 2. print
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnRtmIsdnSpanMapsInd";
	m_rtmIsdnCommonMethods.PrintRtmIsdnSpansMapsListStruct(*pRtmIsdnSpanMapsList, "CCardsManager::OnRtmIsdnSpanMapsInd");

	// ===== 3. add to process's attribute
	STATUS addStat = m_pProcess->UpdateSpanMapsList(*pRtmIsdnSpanMapsList);
	if (STATUS_OK != addStat)
	{
		string retStr = "CCardsManager::OnRtmIsdnSpanMapsInd - failed to add RtmSpanMapList; status: ";
		retStr += m_pProcess->GetStatusAsString(addStat);
		PASSERTMSG(1, retStr.c_str());
	}

	delete pRtmIsdnSpanMapsList;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnRtmIsdnAttachSpanMapsInd(CSegment* pSeg)
{
 	// ===== 1. extract data
    RTM_ISDN_SPAN_MAP_S* pSpanMapStruct = new RTM_ISDN_SPAN_MAP_S;
	pSeg->Get( (BYTE*)pSpanMapStruct, sizeof(RTM_ISDN_SPAN_MAP_S) );

	// ===== 2. print
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnRtmIsdnAttachSpanMapsInd";
	m_rtmIsdnCommonMethods.PrintRtmIsdnSpanMapStruct(*pSpanMapStruct, "CCardsManager::OnRtmIsdnAttachSpanMapsInd");

	// ===== 3. add to process's attribute
	STATUS attachStat = m_pProcess->UpdateSpecSpanMapInList(*pSpanMapStruct);
	if (STATUS_OK == attachStat)
	{

		SendUpdateSpanMapToRtmIsdnTask(pSpanMapStruct->boardId, pSpanMapStruct->spanId, RTM_ISDN_ATTACH_SPAN_MAP_IND);
	}
	else
	{
		string retStr = "CCardsManager::OnRtmIsdnAttachSpanMapsInd - failed to update (attach) SpanMap; status: ";
		retStr += m_pProcess->GetStatusAsString(attachStat);
		PASSERTMSG(1, retStr.c_str());
	}

	delete pSpanMapStruct;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnRtmIsdnDetachSpanMapsInd(CSegment* pSeg)
{
 	// ===== 1. extract data
    SPAN_DISABLE_S* pSpanToDisableStruct = new SPAN_DISABLE_S;
	pSeg->Get( (BYTE*)pSpanToDisableStruct, sizeof(SPAN_DISABLE_S) );

	// ===== 2. print
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnRtmIsdnDetachSpanMapsInd";
	m_rtmIsdnCommonMethods.PrintRtmIsdnSpansDisableStruct(*pSpanToDisableStruct, "CCardsManager::OnRtmIsdnDetachSpanMapsInd");

	// ===== 3. add to process's attribute
	STATUS detachStat = m_pProcess->DetachSpecSpanMapInList(*pSpanToDisableStruct);
	if (STATUS_OK == detachStat)
	{
		// ===== 4. update RTM
		SendUpdateSpanMapToRtmIsdnTask(pSpanToDisableStruct->boardId, pSpanToDisableStruct->spanId, RTM_ISDN_DETACH_SPAN_MAP_IND);
	}
	else
	{
		string retStr = "CCardsManager::OnRtmIsdnDetachSpanMapsInd - failed to update (detach) SpanMap; status: ";
		retStr += m_pProcess->GetStatusAsString(detachStat);
		PASSERTMSG(1, retStr.c_str());
	}

	delete pSpanToDisableStruct;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendUpdateSpanMapToRtmIsdnTask(WORD spanBoardId, WORD spanId, OPCODE theOpcode)
{
	string sOpcode = m_pProcess->GetOpcodeAsString(theOpcode);

	bool isMbx = false;

	DWORD boardId = m_pProcess->GetSlotsNumberingConversionTable()->GetBoardId( (DWORD)spanBoardId, RTM_ISDN_SUBBOARD_ID );
	
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendUpdateSpanMapToRtmIsdnTask"
								<< "\n product type:"<<CProcessBase::GetProcess()->GetProductType();
	
	if (CProcessBase::GetProcess()->GetProductType() == eProductTypeNinja)
		boardId = 1;
	
	for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
	{
		COsQueue* queue = m_pProcess->GetMfaMbx( (DWORD)boardId, j );
		if (queue)
		{
			isMbx = true;

			TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendUpdateSpanMapToRtmIsdnTask"
			                              << "\nBoardId: " << boardId << ", spanId: " << spanId
			                              << ", opcode: "  << sOpcode
			                              << " (sent to subBoardId " << j << ")";

			// ===== 1. prepare the data to be sent
			CSegment* pSegToBoard = new CSegment;
			*pSegToBoard << spanId;

			// ==== 2. send the msg
			CRtmIsdnApi rtmIsdnApi;
			rtmIsdnApi.CreateOnlyApi(*queue);
			rtmIsdnApi.SendMsg(pSegToBoard, theOpcode);
		}
	} // end loop over subBoards

	if (false == isMbx)
	{
		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendUpdateSpanMapToRtmIsdnTask - no Mbx for Rtm "
									  << "in boardId: " << boardId << "(opcode: " << sOpcode.c_str() << ")";
// ASSERT was removed: user should be able to configure a span on non-existing board; no ASSERT is needed.
//		PASSERTMSG(1, retStr.GetString());
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnRtmIsdnDeleteServiceInd(CSegment* pSeg)
{
	// ===== 1. extract data
	RTM_ISDN_SERVICE_NAME_S* pServiceToDelete = new RTM_ISDN_SERVICE_NAME_S;
	pSeg->Get( (BYTE*)pServiceToDelete, sizeof(RTM_ISDN_SERVICE_NAME_S) );

	// ===== 2. print
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnRtmIsdnDeleteServiceInd";
	m_rtmIsdnCommonMethods.PrintRtmIsdnServiceNameStruct(*pServiceToDelete, "CCardsManager::OnRtmIsdnDeleteServiceInd");

	// ===== 3. delete service from process's attribute
	STATUS delStat = m_pProcess->CancelRtmIsdnParams(*pServiceToDelete);
	if (STATUS_OK == delStat)
	{
		// ===== 4. update RTM
		for (int i=0; i<MAX_ISDN_SPAN_MAPS_IN_LIST; i++)
		{
			RTM_ISDN_SPAN_MAP_S curSpan = m_pProcess->GetRtmIsdnSpanMap(i);

			string serviceNameFromSpan = (char*)(curSpan.serviceName);
			if ( serviceNameFromSpan == (char*)(pServiceToDelete->serviceName) )
			{
				SendUpdateSpanMapToRtmIsdnTask(curSpan.boardId, curSpan.spanId, RTM_ISDN_DETACH_SPAN_MAP_IND);
			}
		}

		// ===== 5. detach relevant spans in process's attribute
		m_pProcess->DetachAllSpansInList( (char*)(pServiceToDelete->serviceName) );
	}

	else // delStat != STATUS_OK
	{
		string retStr = "CCardsManager::OnRtmIsdnDeleteServiceInd - failed to cancel RtmIsdnParams; status: ";
		retStr += m_pProcess->GetStatusAsString(delStat);
		PASSERTMSG(1, retStr.c_str());
	}

	delete pServiceToDelete;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnSNMPInterfaceReq(CSegment* pSeg)
{
    m_pProcess->SendSnmpMfaInterface();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnCardRemovedInd(CSegment* pSeg)
{
	DWORD boardId,subBoardId, dCardType;
	*pSeg >> boardId;
	*pSeg >> subBoardId;
	*pSeg >> dCardType;

    eCardType curType = (eCardType)dCardType;

	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnCrardREmovedInd"
	                              << " (boardId "		<< boardId
	                              << "subBoardId "	<< subBoardId
	                              << "cardType: "	<< ::CardTypeToString(curType) << ")";

    DWORD mfaSubBoardId = FIRST_SUBBOARD_ID;

	if (mfaSubBoardId == subBoardId)
	    {    // if it's MFA that was removed...
	         //  ...then remove also its RTMs
		     m_pProcess->SetCardHotSwapStatus(boardId,YES);
	        // turn on timer of 10 seconds
		     CSegment *pTimerSegment = new CSegment;
		     *pTimerSegment << (DWORD)boardId;

		 	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnCrardREmovedInd start HOT_SWAP_TIMER on board id " << boardId;

		 	switch(boardId)
		  	{
		 		case FIXED_BOARD_ID_MEDIA_1:
		 			StartTimer(MFA1_HOT_SWAP_TIMER, 10 *SECOND,pTimerSegment);
		  			break;

		  		case FIXED_BOARD_ID_MEDIA_2:
		  			StartTimer(MFA2_HOT_SWAP_TIMER, 10 *SECOND,pTimerSegment);
		  			break;

		 		case FIXED_BOARD_ID_MEDIA_3:
		 			StartTimer(MFA3_HOT_SWAP_TIMER, 10 *SECOND,pTimerSegment);
		  			break;

		 		case FIXED_BOARD_ID_MEDIA_4:
		 			StartTimer(MFA4_HOT_SWAP_TIMER, 10 *SECOND,pTimerSegment);
		  			break;


		  		default:
		  			TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnCrardREmovedInd illegal board id";
		  	}

	    }


	m_pProcess->RemoveFromMfaTasksList(boardId,subBoardId);
	m_pProcess->GetCardsMonitoringDB()->Cancel(boardId,subBoardId);

		// 29/07/2008: SRE requires that MPM will not be supported at V4.0. 'MPM mode' is not accepted.
		// 09/2008: SRE requires that MPM will be supported at V4.0...
		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnCrardREmovedInd before recalculate";

	    RecalculateSystemCardsMode();

		eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
		eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();
    	if ((eProductFamilyRMX == curProductFamily || eProductFamilySoftMcu == curProductFamily)
    			&& (eProductTypeRMX4000 != curProductType)&& (eProductTypeRMX1500 != curProductType))
		{
			RecalculateSingleMediaBoardOnSecondSlot();
		}

		// ===== remove irrelevant Alert


    	bool isBreezeExists     = m_pProcess->IsBreezeCardExistInDB(),
	         isMPMRXExists     = m_pProcess->IsMPMRXCardExistInDB();

	    //eSystemCardsMode curMode = m_pProcess->GetSystemCardsModeCur();

	    eNGBSystemCardsMode NGBSystemCardsMode= m_pProcess->GetNGBSystemCardsMode();

	    	if (
		 ((eNGBSystemCardsMode_breeze_only == NGBSystemCardsMode)			&& (curType == eMpmRx_Half || curType == eMpmRx_Full)) ||
    	 ((eNGBSystemCardsMode_mpmrx_only == NGBSystemCardsMode)  	&& ( curType == eMpmx_40 || curType == eMpmx_20 || curType == eMpmx_80))
		 /*((eNGBSystemCardsMode_mixed_mode == NGBSystemCardsMode)      && (m_pProcess->GetNumOfMPMRXBoards() >= m_numOfMPMRXCards_NGB ) &&
	             (curType == eMpmRx_Half || curType == eMpmRx_Full)  ) ||

	     ((eNGBSystemCardsMode_mixed_mode == NGBSystemCardsMode)      &&     (m_pProcess->GetNumOfBreezeBoards() >= m_numOfBreezeCards_NGB ) &&
	                         ( curType == eMpmx_40 || curType == eMpmx_20 || curType == eMpmx_80))*/)

    {
    	RemoveActiveAlarmByErrorCode(AA_CARDS_CONFIG_EVENT);
	    	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnCardRemovedInd - Alert was removed";
	    }

    // ===== send to Auditor
    SendCardEventToAuditor(false, boardId, subBoardId, curType);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnBoardsExpectedInd(CSegment* pSeg)
{
	// changing the startup timers is required only after installation
	BOOL isSystemAfterVersionInstall = IsFileExists(SYSTEM_AFTER_VERSION_INSTALLED_FILE);

	if (FALSE == isSystemAfterVersionInstall)
	{
		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnBoardsExpectedInd"
							   << "\nSystem is not after version installation; nothing should therefore be done";
	}

	else
	{
		int numOfBoardsExpected = m_pProcess->GetNumOfBoardsExpected();

		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnBoardsExpectedInd"
							   << "\nSystem is after version installation NumOfBoardsExpected: " << numOfBoardsExpected;

		if (0 == numOfBoardsExpected)
		{
			BOOL isFirstKeepAliveAlreadyTreated;
			*pSeg >> isFirstKeepAliveAlreadyTreated;
			// startup timer should be short
			if (isFirstKeepAliveAlreadyTreated == NO)
				RestartStartupTimers(STARTUP_TIME_LIMIT_WHEN_NO_BOARDS);
		}

		else
		{
			RecalculateBoardsExpectedVsActual();
		}
	} // end (TRUE == isSystemAfterVersionInstall)
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCardsManager::OnModeDetectionInd(CSegment* pSeg)
{
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

    DWORD curCard = eEmpty;
    DWORD numOfBreezeCards = 0;
    DWORD numOfMPMRXCards  = 0;

	*pSeg >> curCard;
	*pSeg >> numOfBreezeCards;
	*pSeg >> numOfMPMRXCards;

	eCardType curCardType = (eCardType)curCard;

	TRACESTR(eLevelInfoNormal) << " CCardsManager::OnModeDetectionInd : numOfBreezeCards = " << numOfBreezeCards
	        << ", numOfMPMRXCards = " << numOfMPMRXCards;

	//we want to do it only once
	if (m_numOfBreezeCards_NGB == 0 && m_numOfMPMRXCards_NGB == 0)
	{
	    m_numOfBreezeCards_NGB = numOfBreezeCards;
	    m_numOfMPMRXCards_NGB  = numOfMPMRXCards;
	}

	/*******************************************************************/
	/* 2.7.10 changes in simulation ,added by Rachel Cohen             */
	/*        Gideon sin read the system configuration from MPL_SIM.XML*/
	/*        and send the card type to card manager                   */
	/*******************************************************************/
	//if (IsTarget())
	//{
		string curCardTypeStr = SYSTEM_CARDS_MODE_DEFAULT_VAL;

		if(m_pProcess->IsMpmRXCard(curCardType))
			curCardTypeStr = SYSTEM_CARDS_MODE_MPMRX_VAL;
		else if( m_pProcess->IsBreezeCard(curCardType) || m_pProcess->IsSoftCard(curCardType) ) //OLGA - SoftMCU
		{
			if (eProductTypeCallGeneratorSoftMCU == curProductType || eProductTypeNinja == curProductType )
				curCardTypeStr = SYSTEM_CARDS_MODE_MPMRX_VAL;
			else
				curCardTypeStr = SYSTEM_CARDS_MODE_MPMX_VAL;
		}



		//only for debugging
		//curCardTypeStr = SYSTEM_CARDS_MODE_MPMX_VAL;

		TRACESTR(eLevelInfoNormal) << " CCardsManager::OnModeDetectionInd : curCardType = " << curCardType
		<< ", curCardTypeStr = " << curCardTypeStr;
		//Set new mode
		m_pProcess->SetSystemCardsModeCur(curCardTypeStr);
		FILE* pNewModeFile = fopen( SYSTEM_CARDS_MODE_FILE_FULL_PATH.c_str(), "w" );
		if (pNewModeFile)
		{
		    eNGBSystemCardsMode cardMode= m_pProcess->GetNGBSystemCardsMode();
		    if (cardMode == eNGBSystemCardsMode_mpmrx_only && numOfMPMRXCards> 0 ) curCardTypeStr = SYSTEM_CARDS_MODE_MPMRX_ONLY_VAL;
		   // else if (cardMode == eNGBSystemCardsMode_mixed_mode) curCardTypeStr = SYSTEM_CARDS_MODE_MIXED_MODE_VAL;

		    TRACESTR(eLevelInfoNormal) << " CCardsManager::OnModeDetectionInd : writing to file cardMode = " << cardMode;

			fwrite(curCardTypeStr.c_str(), sizeof(char), curCardTypeStr.length(), pNewModeFile);
			fclose(pNewModeFile);

			m_pProcess->SetSystemCardsModeFromFile(m_pProcess->GetSystemCardsModeCur());

		}
		else
			PASSERT(1); // the file should be exists
	//}
	//else
	//{
	//	TRACESTR(eLevelInfoNormal) << " CCardsManager::OnModeDetectionInd simulation --> don't change cards mode according to KeepAlive, curCardType = " << m_pProcess->GetSystemCardsModeCur();
	//}
	//Set new flag (detection=YES) ( for use in OnCardMngrLoadedInd)
	m_isModeDetectionAlreadyTreated = YES;

	//Send to McuMngr and ConfParty
    SendCurSystemCardsModeToSpecProcess(eProcessMcuMngr);
    SendCurSystemCardsModeToSpecProcess(eProcessConfParty);
    SendCurSystemCardsModeToSpecProcess(eProcessMplApi);
    SendCurSystemCardsModeToSpecProcess(eProcessUtility);

	if( m_isResourceSystemCardsModeReq )
	    SendCurSystemCardsModeToSpecProcess(eProcessResource);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::RecalculateBoardsExpectedVsActual()
{
	// changing the startup timers is required only after installation
	BOOL isSystemAfterVersionInstall = IsFileExists(SYSTEM_AFTER_VERSION_INSTALLED_FILE);

	if (FALSE == isSystemAfterVersionInstall)
	{
		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::RecalculateBoardsExpectedVsActual"
							   << "\nSystem is not after version installation; nothing should therefore be done";
	}

	else
	{
		int numOfBoardsExpected = m_pProcess->GetNumOfBoardsExpected(),
			numOfBoardsOnSystem = m_pProcess->GetCardsMonitoringDB()->GetNumOfMediaBoards();

		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::RecalculateBoardsExpectedVsActual"
							   << "\nNumOfBoardsExpected: " << numOfBoardsExpected
							   << "\nNumOfBoardsOnSystem: " << numOfBoardsOnSystem;

		if (numOfBoardsOnSystem == numOfBoardsExpected) // all expected boards are up
		{
			// startup timer should be normal (without waiting a long time for IPMC burning)
			DWORD startupTime;
			string key = "MAX_STARTUP_TIME";
			m_pProcess->GetSysConfig()->GetDWORDDataByKey(key, startupTime);

			RestartStartupTimers(startupTime);
		}
	} // end (TRUE == isSystemAfterVersionInstall)
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::RestartStartupTimers(DWORD newStartupTimeout)
{
	eProcessType curProcess = eProcessTypeInvalid;
	int newStartupTimeoutInSeconds = newStartupTimeout / SECOND;	// 'newStartupTimeout' is in ticks

	// ===== send the info to each process
	for (int i=0; i<NUM_OF_PROCESS_TYPES; i++)
	{
		// ===== 1. fill the Segment
		CSegment*  pParamSegment = new CSegment;
	    *pParamSegment << newStartupTimeout;

		// ===== 2. send
		curProcess = (eProcessType)i;
		CManagerApi api(curProcess);
		api.SendMsg(pParamSegment, RESTART_STARTUP_TIMER);

		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::RestartStartupTimers"
							   << "\nNew startup timeout sent to " << ::ProcessTypeToString(curProcess) << " process: "
							   << newStartupTimeout << " ticks (" << newStartupTimeoutInSeconds << " seconds)";
	}


	// ===== 3. update McuMngr specifically (for McuMngr's specific needs)
	CSegment*  pParamSeg = new CSegment;
	*pParamSeg << newStartupTimeout;
	CManagerApi api(eProcessMcuMngr);
	api.SendMsg(pParamSeg, UPDATE_SYSTEM_STARTUP_DURATION_IND);


	// ===== 4. update specific timer in Cards process
//	if ( eTaskStateStartup == GetTaskState() )
	if ( IsValidTimer(SYSTEM_CARDS_MODE_STARTUP_TIMER) )
	{
		DeleteTimer(SYSTEM_CARDS_MODE_STARTUP_TIMER);
		StartTimer(SYSTEM_CARDS_MODE_STARTUP_TIMER, newStartupTimeout);

		TRACESTR(eLevelInfoNormal) << "\nCManagerTask::OnRestartStartupTimer - " << ::ProcessTypeToString(curProcess)
	    							  << "\nSYSTEM_CARDS_MODE_STARTUP_TIMER's timeout was restarted to "
	    							  << newStartupTimeout << " ticks (" << newStartupTimeoutInSeconds << " seconds)";
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnAudioCntrlrActiveReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnAudioCntrlrActiveReq";
	TAcTypeReq dataStruct;
    memset(&dataStruct,0,sizeof(TAcTypeReq));
    pSeg->Get( (BYTE*)&dataStruct, sizeof(TAcTypeReq));

	TRACESTR(eLevelInfoNormal) << "\n<<>>nCCardsManager::OnAudioCntrlrActiveReq dataStruct.unBoardId:"<<dataStruct.unBoardId<<"\n"
                           <<"m_subBoardId"<< dataStruct.unSubBoardId<<"\n"
                           <<"dataStruct.unUnitId"<< dataStruct.unUnitId<<"\n"
                           <<"dataStruct.eAudioContollerType"<<dataStruct.eAudioContollerType<<"\n";

			COsQueue* queue = m_pProcess->GetMfaMbx(dataStruct.unBoardId,dataStruct.unSubBoardId);
			if (queue)
			{
				// ===== 1. prepare the data to be sent
				CSegment* pSegToBoard = new CSegment;
				*pSegToBoard << dataStruct.unBoardId
				             << dataStruct.unSubBoardId
				             << dataStruct.unUnitId
				             << dataStruct.eAudioContollerType;
				pSegToBoard->Put( (BYTE*)&dataStruct, sizeof(TAcTypeReq) );

				// ==== 2. send the msg
				CMfaApi mfaApi;
				mfaApi.CreateOnlyApi(*queue);
				mfaApi.SendMsg(pSegToBoard, AC_TYPE_REQ);
			}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// the following two methods should be united to a single 'OnSystemCardsModeReq'
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnResourceSystemCardsModeReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnResourceSystemCardsModeReq";
	if( YES == m_isModeDetectionAlreadyTreated )
	    SendCurSystemCardsModeToSpecProcess(eProcessResource);

	m_isResourceSystemCardsModeReq = YES;//we suppose that Switch raises only once
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnConfPartySystemCardsModeReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnConfPartySystemCardsModeReq";
	if( YES == m_isModeDetectionAlreadyTreated )
		SendCurSystemCardsModeToSpecProcess(eProcessConfParty);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnMcuMngrSystemCardsModeReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnMcuMngrSystemCardsModeReq";
	if( YES == m_isModeDetectionAlreadyTreated )
		SendCurSystemCardsModeToSpecProcess(eProcessMcuMngr);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnUtilitySystemCardsModeReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnUtilitySystemCardsModeReq";
	if( YES == m_isModeDetectionAlreadyTreated )
		SendCurSystemCardsModeToSpecProcess(eProcessUtility);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendCurSystemCardsModeToSpecProcess(eProcessType processType)
{
	eSystemCardsMode curMode = m_pProcess->GetSystemCardsModeCur();

	//************************************************************************************************************
	// 7.6.12 VNGR- 25395 Rachel Cohen it is for phase 1 need to be changed (we need to add cards mode "MIXED_MODE" and send it to resources and confparty
	/*if (processType == eProcessMcuMngr || processType == eProcessResource)
	{
	    eNGBSystemCardsMode cardMode= m_pProcess->GetNGBSystemCardsMode();
	    if (cardMode == eNGBSystemCardsMode_mixed_mode)
	        curMode = eSystemCardsMode_mixed_mode;
	}*/
	//************************************************************************************************************

	// ===== 1. fill the Segment
	CSegment*  pRetParam = new CSegment;
    *pRetParam << (DWORD)curMode;

	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendCurSystemCardsModeToSpecProcess"
                           << "\nMode sent to "  << ::ProcessTypeToString(processType) << " process: "
                           << ::GetSystemCardsModeStr(curMode);

	// ===== 2. send
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(processType, eManager);

	STATUS res = pMbx->Send(pRetParam, MCMS_SYSTEM_CARDS_MODE_IND, &GetRcvMbx());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// bool isInstaled :  false - card was removed
//                    true  - card was instaled
void CCardsManager::SendCardEventToAuditor(bool isInstaled,
                                           DWORD boardId,
                                           DWORD subBoardId,
                                           eCardType cardType)const
{

    const char * boardName = "";
    if(eSwitch == cardType)
    {
    	boardName = "Switch";
    }
    else if (eRtmIsdn == cardType)
    {
    	boardName = "RTM-ISDN";
    }
    else if (eRtmIsdn_9PRI == cardType)
    {
    	boardName = "RTM-ISDN9";
    }
    else if (eRtmIsdn_9PRI_10G == cardType)
    {
    	boardName = "RTM-ISDN9_10G";
    }
    /* else if (eMpmx_80 == cardType)
    {
        boardName = "MPMX_80";
    }
    else if (eMpmx_40 == cardType)
    {
        boardName = "MPMX_40";
    }*/
    else if( m_pProcess->IsBreezeCard(cardType))
    {
    	boardName = "MPMX";
    }
    else if( m_pProcess->IsMpmRXCard(cardType))
    {
        boardName = "MPMRX";
    }
    else if( m_pProcess->IsMpmPlusCard(cardType) )
    {
   		boardName = "MPM+";
   	}
 	else if( m_pProcess->IsMpmCard(cardType) )
	{
		boardName = "MPM";
	}
    else if( m_pProcess->IsSoftCard(cardType) ) //OLGA - SoftMCU
    {
        boardName = "MPMX_SOFT";
    }
	else
	{
		boardName = "MPM";
		PASSERT(1);
	}

    const char * operation = (isInstaled ? "installed" : "removed");

    CLargeString action;
    action << boardName << " card " << operation;

    CLargeString description;
    description << "An " << boardName << " card was " << operation;

    CLargeString descriptionEx;
    descriptionEx << "The " << boardName << " card was " << operation
                  << (isInstaled ? " in" : " from")
                  << " slot {" << boardId << "}";

    AUDIT_EVENT_HEADER_S outAuditHdr;
    CAuditorApi::PrepareAuditHeader(outAuditHdr,
                                    "",
                                    eMcms,
                                    "",
                                    "",
                                    eAuditEventTypeInternal,
                                    eAuditEventStatusOk,
                                    action.GetString(),
                                    description.GetString(),
                                    "",
                                    descriptionEx.GetString());
    CFreeData freeData;
    CAuditorApi::PrepareFreeData(freeData,
                                 "",
                                 eFreeDataTypeXml,
                                 "",
                                 "",
                                 eFreeDataTypeXml,
                                 "");
    CAuditorApi api;
    api.SendEventMcms(outAuditHdr, freeData);
}

//////////////////////////////////////////////////////////////////////////////
void CCardsManager::InitSystemCardsMode()
{


	// SoftMcu: will not receive DETENSION_IND from switch card because no GideonSim
	if( eProductFamilySoftMcu == m_pProcess->GetProductFamily() )
	{
		m_isModeDetectionAlreadyTreated = YES;


		eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

		if (eProductTypeNinja == curProductType || eProductTypeCallGeneratorSoftMCU == curProductType)
		    m_pProcess->SetSystemCardsModeCur(SYSTEM_CARDS_MODE_DEFAULT_VAL); //MPMRX
		else
			m_pProcess->SetSystemCardsModeCur(SYSTEM_CARDS_MODE_MPMX_VAL);

		return;
	}


	string fileStatusStr = "File ";
	fileStatusStr += SYSTEM_CARDS_MODE_FILE_FULL_PATH;

	FILE* pModeFile = fopen( SYSTEM_CARDS_MODE_FILE_FULL_PATH.c_str(), "r" );

	if (pModeFile)
	{
		ALLOCBUFFER(sSystemCardsModeFromFile, SYSTEM_CARDS_MODE_STR_LEN);
		int numRead = fread( sSystemCardsModeFromFile, sizeof(char), SYSTEM_CARDS_MODE_STR_LEN - 1, pModeFile );

		if (numRead)
		{
			sSystemCardsModeFromFile[numRead] = '\0';

            TRACEINTO << "CCardsProcess::InitSystemCardsMode --- cards mode string: " << sSystemCardsModeFromFile;

            if (/*(strcmp(sSystemCardsModeFromFile,SYSTEM_CARDS_MODE_MIXED_MODE_VAL) == 0 ) ||*/ (strcmp(sSystemCardsModeFromFile,SYSTEM_CARDS_MODE_MPMRX_ONLY_VAL) == 0))
            {
                TRACEINTO << "CCardsProcess::InitSystemCardsMode --- found equal: " << sSystemCardsModeFromFile;
                strcpy(sSystemCardsModeFromFile,SYSTEM_CARDS_MODE_MPMRX_VAL);
            }

			bool good = m_pProcess->SetSystemCardsModeCur(sSystemCardsModeFromFile);
			if (!good)
			{
			        AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
                                             AA_CARDS_WRONG_FILE_MODE,
       				                         MAJOR_ERROR_LEVEL,
                                 			 "System cards file's mode is wrong, default is used",
                                 			 true,
                                 			 true );
			}

            fileStatusStr += " is ok";
        }
        else // nothing was read
        {
             fileStatusStr += " exists but empty";
        }

		DEALLOCBUFFER(sSystemCardsModeFromFile);
        fclose(pModeFile);
	}

    else // file does not exist - create (default)
    {
        BOOL res = CreateDirectory(SYSTEM_CARDS_MODE_DIR_FULL_PATH.c_str());
        if(!res)
        {
            TRACEINTO << "\nFailed to create dir : " << SYSTEM_CARDS_MODE_DIR_FULL_PATH;
            return;
        }

        FILE* pNewModeFile = fopen( SYSTEM_CARDS_MODE_FILE_FULL_PATH.c_str(), "w" );

        if (pNewModeFile)
        {
            string curCardTypeStr = SYSTEM_CARDS_MODE_DEFAULT_VAL;

            eNGBSystemCardsMode cardMode= m_pProcess->GetNGBSystemCardsMode();
            if (cardMode == eNGBSystemCardsMode_mpmrx_only && m_numOfMPMRXCards_NGB > 0 ) curCardTypeStr = SYSTEM_CARDS_MODE_MPMRX_ONLY_VAL;
            //else if (cardMode == eNGBSystemCardsMode_mixed_mode) curCardTypeStr = SYSTEM_CARDS_MODE_MIXED_MODE_VAL;

            fwrite(curCardTypeStr.c_str(), sizeof(char), curCardTypeStr.length(), pNewModeFile);
            fclose(pNewModeFile);

			m_pProcess->SetSystemCardsModeCur(SYSTEM_CARDS_MODE_DEFAULT_VAL);

       }

        fileStatusStr += " did not exist, and was now created (with the default value)";
    }


    m_pProcess->SetSystemCardsModeFromFile(m_pProcess->GetSystemCardsModeCur());


    fileStatusStr += "\nMode: ";
    fileStatusStr += ::GetSystemCardsModeStr(m_pProcess->GetSystemCardsModeCur());
    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::InitSystemCardsMode"
                           << "\n" << fileStatusStr;


}

//////////////////////////////////////////////////////////////////////////////
void CCardsManager::RecalculateSystemCardsMode()
{
	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();

	if (curProductFamily == eProductFamilySoftMcu)
		return;

	BOOL isCardsModeChangeable = NO;
	m_pProcess->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_CARDS_MODE_CHANGEABLE, isCardsModeChangeable);

	if (YES == isCardsModeChangeable)
	{
		eSystemCardsMode modeFromFile_old = m_pProcess->GetSystemCardsModeFromFile();
		eSystemCardsMode modeFromFile_new = modeFromFile_old;

	    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::RecalculateSystemCardsMode before"
	                               << "\nOld mode: " << ::GetSystemCardsModeStr(modeFromFile_old);

	    // ===== 1. produce Alert (if needed)
	    bool isAlertOn = HandleSystemCardsModeAlerts();
	    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::RecalculateSystemCardsMode";
	    if (true == isAlertOn)
	        return;

	    bool isSystemCardsModeChanged = IsSystemCardsModeChanged();
	    if (true == isSystemCardsModeChanged)
	    {
	        // ===== 2. update file and process
	        UpdateSystemCardsModeFile();
	        modeFromFile_new = m_pProcess->GetSystemCardsModeFromFile();
	        // rachel it is for phase 2 SendCurSystemCardsModeToSpecProcess(eProcessResource);
	    }

	    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::RecalculateSystemCardsMode"
	                           << "\nOld mode: " << ::GetSystemCardsModeStr(modeFromFile_old)
	                           << "\nOld mode no : " << modeFromFile_old
	                           << "\nNew mode: " << ::GetSystemCardsModeStr(modeFromFile_new);
	}

	else // SysCfg: CardsMode should not change
	{
	    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::RecalculateSystemCardsMode"
	                           << "\nNo change in cards mode due to SysCfg flag";
	}
}

//////////////////////////////////////////////////////////////////////////////
void CCardsManager::RecalculateSingleMediaBoardOnSecondSlot()
{
    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::RecalculateSingleMediaBoardOnSecondSlot";

    BYTE bChassies_Warning = FALSE;

    if (m_pProcess->GetAuthenticationStruct()->chassisVersion.ver_major == 1 &&
    	m_pProcess->GetAuthenticationStruct()->chassisVersion.ver_minor == 3 &&
    	m_pProcess->GetAuthenticationStruct()->chassisVersion.ver_release == 0 &&
    	m_pProcess->GetAuthenticationStruct()->chassisVersion.ver_internal == 0)
    	bChassies_Warning = TRUE;

    if ( true == m_pProcess->IsSingleMediaBoardOnSecondSlot() && bChassies_Warning)
    {
        AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
        						 AA_CARDS_CONFIG_EVENT,
                                 MAJOR_ERROR_LEVEL,
                                 "Please re-locate the Media card to the upper media slot",
                                 true,
                                 true,
                                 SINGLE_MEDIA_SECOND_SLOT_ALERT_USER_ID );

    }
}

//////////////////////////////////////////////////////////////////////////////
bool CCardsManager::IsSystemCardsModeChanged()
{
    bool isChanged = false;

    eSystemCardsMode modeFromFile	= m_pProcess->GetSystemCardsModeFromFile();

    int numOfBreezeBoards           = m_pProcess->GetNumOfBreezeBoards();
    int numOfMPMRXBoards            = m_pProcess->GetNumOfMPMRXBoards();

    bool isAnyMediaCardStillExists	= m_pProcess->IsAnyMediaCardExistsInDB();

  /*  if (true == isAnyMediaCardStillExists)			// the mode should not change due to removing the last board
    {
        eNGBSystemCardsMode NGBSystemCardsMode= m_pProcess->GetNGBSystemCardsMode();

	    if ( (NGBSystemCardsMode == eNGBSystemCardsMode_mixed_mode) &&
	         (((eSystemCardsMode_mpmrx == modeFromFile) && (0 == numOfMPMRXBoards )&&  (0< numOfBreezeBoards)) ||
	         ((eSystemCardsMode_breeze == modeFromFile) && (0 < numOfMPMRXBoards) )))
	    {
	        //rachel it will be in phase 2 isChanged = true;
			PASSERT(1);//due to auto detection the mode is set according to Switch info and shouldn't be changed
	    }
    }*/

    string isAnyExistStr = ( (true == isAnyMediaCardStillExists) ? "yes" : "no" );
    string isChangedStr  = ( (true == isChanged)            ? "yes" : "no" );
    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::IsSystemCardsModeChanged"
                           << "\nMode from file:              " << ::GetSystemCardsModeStr(modeFromFile)

                           << "\nNum of Breeze boards in DB:  " << numOfBreezeBoards
                           << "\nNum of MPMRX boards in DB:   " << numOfMPMRXBoards
                           << "\nAny card exists in DB:       " << isAnyExistStr
                           << "\nSystemCardsMode changed:     " << isChangedStr;

    return isChanged;
}

//////////////////////////////////////////////////////////////////////////////
void CCardsManager::UpdateSystemCardsModeFile()
{
	string sNewVal,sNewValForFile;
	bool isBreezeExistsInDB     = m_pProcess->IsBreezeCardExistInDB();
    bool isMpmPlusExistsInDB	= m_pProcess->IsMpmPlusCardExistInDB();
    bool isAnyCardExistsInDB	= m_pProcess->IsAnyMediaCardExistsInDB();
    bool isMpmxRxExistsInDB     = m_pProcess->IsMPMRXCardExistInDB();

    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

    if (true == isMpmxRxExistsInDB || eProductTypeCallGeneratorSoftMCU == curProductType || eProductTypeNinja == curProductType)
           sNewVal = SYSTEM_CARDS_MODE_MPMRX_VAL;
    else if (true == isBreezeExistsInDB)
		sNewVal = SYSTEM_CARDS_MODE_MPMX_VAL;

    else { // put the default

    	/*if (true == isMpmPlusExistsInDB)
    		sNewVal = SYSTEM_CARDS_MODE_MPM_PLUS_VAL;
    	else
    		sNewVal = SYSTEM_CARDS_MODE_MPM_VAL;*/
        sNewVal = SYSTEM_CARDS_MODE_MPMRX_VAL;
    }

    sNewValForFile = sNewVal;
    // cards mode is MPM+ when there is an MPM+ board, or there is no cards at all
// 01/01/2009: SRE requires that removing the last board will NOT affect cards' mode


    // MPM cards are disabled on RMX4000, therefore it cannot accept 'MPM' mode

    /*if ( (eProductTypeRMX1500 == curProductType) && (SYSTEM_CARDS_MODE_MPMX_VAL != sNewVal) )
	{
	    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::UpdateSystemCardsModeFile"
	                           << "\nRMX1500 system cannot be modified to 'MPM' mode or 'MPM_PLUS' mode, therefore it stays "
	                           << ::GetSystemCardsModeStr(m_pProcess->GetSystemCardsModeFromFile());
	    return;
	}*/


    FILE* pNewModeFile = fopen( SYSTEM_CARDS_MODE_FILE_FULL_PATH.c_str(), "w" );
    if (pNewModeFile)
    {
        eNGBSystemCardsMode cardMode= m_pProcess->GetNGBSystemCardsMode();
        if (cardMode == eNGBSystemCardsMode_mpmrx_only && m_numOfMPMRXCards_NGB > 0) sNewValForFile = SYSTEM_CARDS_MODE_MPMRX_ONLY_VAL;
       // else if (cardMode == eNGBSystemCardsMode_mixed_mode) sNewValForFile = SYSTEM_CARDS_MODE_MIXED_MODE_VAL;


        fwrite(sNewValForFile.c_str(), sizeof(char), sNewValForFile.length(), pNewModeFile);
        fclose(pNewModeFile);

        m_pProcess->SetSystemCardsModeFromFile(sNewVal);

// 01/01/2009: SRE requires that removing the last board will NOT affect cards' mode
/*
        // for handling Alert after next change (if occurs)
        if ( (SYSTEM_CARDS_MODE_MPM_PLUS_VAL == sNewVal) && (false == isAnyCardExistsInDB) )
        	m_isCardModeMpmPlusDueToNoCardsOnSystem = true;
        else
        	m_isCardModeMpmPlusDueToNoCardsOnSystem = false;
*/
    }

    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::UpdateSystemCardsModeFile"
                           << "\nNew string written to file: " << sNewVal
                           << "\nTherefore new mode in file: " << ::GetSystemCardsModeStr(m_pProcess->GetSystemCardsModeFromFile());
}

/*
//////////////////////////////////////////////////////////////////////////////
void CCardsManager::HandleSystemCardsModeAlerts(eSystemCardsMode originalMode)
{
// 26.04.09: SRE: MPM is ALWAYS disabled in V4.5

	bool isMpmExists = m_pProcess->IsMpmCardExistInDB();

	if (true == isMpmExists)
	{
		AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
								 AA_CARDS_CONFIG_EVENT,
								 MAJOR_ERROR_LEVEL,
								 MPM_BLOCKING_ALERT_DESCRIPTION,
								 true,
								 true,
								 MPM_BLOCKING_ALERT_USER_ID );
	}
}

*/
//////////////////////////////////////////////////////////////////////////////
bool CCardsManager::HandleSystemCardsModeAlerts()
{
	BOOL bJitcMode = FALSE;
    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
    sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);

    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();


	bool isBreezeExists     = m_pProcess->IsBreezeCardExistInDB();
    bool isMPMRXExists      = m_pProcess->	IsMPMRXCardExistInDB();
	bool isAlert			= false;

    eNGBSystemCardsMode NGBSystemCardsMode= m_pProcess->GetNGBSystemCardsMode();

	string sDesc = "";

	if ( (eNGBSystemCardsMode_breeze_only == NGBSystemCardsMode) && (true == isMPMRXExists) )
	{
	    isAlert = true;
	    //sDesc = "MCU currently in MPMX ONLY Mode and the MPMRX card is disabled (You can use MPMX only). To enable the MPMRX card, reset the MCU";
	    sDesc = "MCU currently in MPMX ONLY Mode and the MPMRX card is disabled (You can use MPMX only). To enable the MPMRX card, reset the MCU";
	}


	else if ( (eNGBSystemCardsMode_mpmrx_only == NGBSystemCardsMode) && ( true == isBreezeExists) )
	{

		isAlert = true;
       // sDesc = "MCU currently in MPMRX ONLY Mode and the MPMX card is disabled (You can use MPMRX only). To switch to Mixed Mode and enable the MPMX card, reset the MCU";
        sDesc = "MCU currently in MPMRX ONLY Mode and the MPMX card is disabled (You can use MPMRX only). To enable the MPMX card, remove any MPMRX cards and reset the MCU";

	}

	if (true == isAlert  )
	{
		AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
								 AA_CARDS_CONFIG_EVENT,
								 MAJOR_ERROR_LEVEL,
								 sDesc.c_str(),
								 true,
								 true );
	}
	return isAlert;
}

//////////////////////////////////////////////////////////////////////////////
bool CCardsManager::HandleNGBSystemCardsModeAlerts(eCardType newCardType,BYTE boardId,BYTE subBoardId)
{
    BOOL bJitcMode = FALSE;
    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
    sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);

    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();





    bool isAlert            = false;

    eNGBSystemCardsMode NGBSystemCardsMode= m_pProcess->GetNGBSystemCardsMode();

    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::HandleNGBSystemCardsModeAlerts"
                               << "\nnewCardType: " << newCardType
                               << "\nGetNGBSystemCardsModeStr: " << GetNGBSystemCardsModeStr(NGBSystemCardsMode)
                               << "\nStartup: "<< m_isStartupOver
                               <<"\n\nGet Num Of Breeze Boards: " << m_pProcess->GetNumOfBreezeBoards()
                               <<"\nGet Num Of MPMRX Boards: " << m_pProcess->GetNumOfMPMRXBoards()
                               <<"\n\nm_numOfBreezeCards_NGB: " << m_numOfBreezeCards_NGB
                               <<"\nm_numOfMPMRXCards_NGB: " << m_numOfMPMRXCards_NGB;
    string sDesc = "";

    if ( (eNGBSystemCardsMode_breeze_only == NGBSystemCardsMode) && (newCardType == eMpmRx_Half || newCardType == eMpmRx_Full) )
    {
        isAlert = true;
       // sDesc = "MCU currently in MPMX ONLY Mode and the MPMRX card is disabled (You can use MPMX only). To switch to Mixed Mode and enable the MPMRX card, reset the MCU";
        sDesc = "MCU currently in MPMX ONLY Mode and the MPMRX card is disabled (You can use MPMX only). To enable the MPMRX card, reset the MCU";

    }


    else if ( (eNGBSystemCardsMode_mpmrx_only == NGBSystemCardsMode) && ( newCardType == eMpmx_40 || newCardType == eMpmx_20 || newCardType == eMpmx_80) )
    {

        isAlert = true;
        //sDesc = "MCU currently in MPMRX ONLY Mode and the MPMX card is disabled (You can use MPMRX only). To switch to Mixed Mode and enable the MPMX card, reset the MCU";
        sDesc = "MCU currently in MPMRX ONLY Mode and the MPMX card is disabled (You can use MPMRX only).To enable the MPMX card, remove any MPMRX cards and reset the MCU";

    }
   /* else if (  (eNGBSystemCardsMode_mixed_mode == NGBSystemCardsMode) &&
            (m_pProcess->GetNumOfBreezeBoards() >= m_numOfBreezeCards_NGB ) &&
            ( newCardType == eMpmx_40 || newCardType == eMpmx_20 || newCardType == eMpmx_80) &&
            (FALSE == IsTaskAlreadyExists(boardId, subBoardId)))// that last check is for resetMfa from putty in that case the task exist

    {

        isAlert = true;
        sDesc = "In Mixed Mode a System Reset is required to activate additionally inserted cards.";
    }
    else if ( (eNGBSystemCardsMode_mixed_mode == NGBSystemCardsMode) &&
             (m_pProcess->GetNumOfMPMRXBoards() >= m_numOfMPMRXCards_NGB ) &&
             (newCardType == eMpmRx_Half || newCardType == eMpmRx_Full) &&
             (FALSE == IsTaskAlreadyExists(boardId, subBoardId))) // that last check is for resetMfa from putty in that case the task exist
    {

        isAlert = true;
        sDesc = "In Mixed Mode a System Reset is required to activate additionally inserted cards.";
    }*/

    if (true == isAlert  )
    {

        AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
                                 AA_CARDS_CONFIG_EVENT,
                                 MAJOR_ERROR_LEVEL,
                                 sDesc.c_str(),
                                 true,
                                 true );
    }
    return isAlert;
}

/*
//////////////////////////////////////////////////////////////////////////////
// old version: 'Pure' (MPM+ only) and 'Mixed' (MPM+ and MPM) modes
bool CCardsManager::IsSystemCardsModeChanged()
{
    bool isChanged = false;

    bool isAnyCardStillExists = m_pProcess->IsAnyMediaCardExistsInDB();
    if (true == isAnyCardStillExists)			// the mode should not change due to removing the last board
    {
        eSystemCardsMode modeFromFile	= m_pProcess->GetSystemCardsModeFromFile();
        bool isMfaExistsInDB			= m_pProcess->IsMpmCardExistInDB();

	    if ( ((eSystemCardsMode_mpm_plus	== modeFromFile) && (true	== isMfaExistsInDB)) ||
	         ((eSystemCardsMode_mpm			== modeFromFile) && (false	== isMfaExistsInDB))  )
	    {
	        isChanged = true;
	    }
    }

    string isMfaExistStr = ( (true == isMfaExistsInDB)      ? "yes" : "no" );
    string isAnyExistStr = ( (true == isAnyCardStillExists) ? "yes" : "no" );
    string isChangedStr  = ( (true == isChanged)            ? "yes" : "no" );
    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::IsSystemCardsModeChanged"
                           << "\nMode from file:          " << ::GetSystemCardsModeStr(modeFromFile)
                           << "\nMPM exists in DB:        " << isMfaExistStr
                           << "\nAny card exists in DB:   " << isAnyExistStr
                           << "\nSystemCardsMode changed: " << isChangedStr;

    return isChanged;
}

//////////////////////////////////////////////////////////////////////////////
void CCardsManager::HandleSystemCardsModeAlerts(eSystemCardsMode oldMode, eSystemCardsMode newMode)
{
	string sDesc = "";

    // ===== 1. MPM -> MPM+
    if ( (eSystemCardsMode_mpm == oldMode) && (eSystemCardsMode_mpm_plus == newMode) )
    {
    	int numOfBaraks = m_pProcess->GetNumOfMpmPlusBoards();
    	sDesc = ( (1 < numOfBaraks) ?
    			  "The MPM+ cards are disabled. Please remove any MPM card and reset the system in order to use the MPM+ cards in the system"
    			  :
    	          "The MPM+ card is disabled. Please remove any MPM card and reset the system in order to use the MPM+ card in the system" );

		AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
								 AA_CARDS_CONFIG_EVENT, //AA_SYSTEM_CARDS_MODE_TO_MPM_PLUS,
								 MAJOR_ERROR_LEVEL,
								 sDesc.c_str(), //"MPM+ card is enabled in MPM mode; please reset the MCU in order to use it in MPM+ mode",
								 true,
								 true );
    } // end MPM -> MPM+


    // ===== 2. MPM+ -> MPM
    else if ( (eSystemCardsMode_mpm_plus == oldMode) && (eSystemCardsMode_mpm == newMode) )
    {
    	int numOfMFAs = m_pProcess->GetNumOfMpmBoards();
    	sDesc = ( (1 < numOfMFAs) ?
    			  "The MPM cards are disabled. Please remove any MPM+ card and reset the system in order to use the MPM cards in the system"
    			  :
    	          "The MPM card is disabled. Please remove any MPM+ card and reset the system in order to use the MPM card in the system" );

        AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
        						 AA_CARDS_CONFIG_EVENT, //AA_SYSTEM_CARDS_MODE_TO_MPM,
                                 MAJOR_ERROR_LEVEL,
                                 sDesc.c_str(), //"MPM card is disabled; please reset the MCU in order to use all cards in the system",
                                 true,
                                 true );
    } // end MPM+ -> MPM
}

//////////////////////////////////////////////////////////////////////////////
void CCardsManager::HandleSystemCardsModeAlerts(eSystemCardsMode oldMode, eSystemCardsMode newMode)
{
    // ===== 1. MPM -> MPM+
    if ( (eSystemCardsMode_mpm == oldMode) && (eSystemCardsMode_mpm_plus == newMode) )
    {
        // if the alternative Alert exists, it means that the system gets back to its 'original' mode
        if ( true == IsActiveAlarmExistByErrorCode(AA_SYSTEM_CARDS_MODE_TO_MPM) )
        {
            RemoveActiveAlarmByErrorCode(AA_SYSTEM_CARDS_MODE_TO_MPM);
        }

        // else - it means that the mode is changed: an Alert should be produced
        else
        {
        	int numOfBaraks = m_pProcess->GetNumOfMpmPlusBoards();
        	string desc = ( (1 < numOfBaraks) ?
        			       "The MPM+ cards are functioning as MPM cards. To utilize the full MPM+ capabilities of the cards please reset the MCU"
        				   :
        	        	   "The MPM+ card is functioning as an MPM card. To utilize the card's full MPM+ capability please reset the MCU" );

    		AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
        						 AA_SYSTEM_CARDS_MODE_TO_MPM_PLUS,
                                 MAJOR_ERROR_LEVEL,
                                 //"MPM+ card is enabled in MPM mode; please reset the MCU in order to use it in MPM+ mode",
                                 desc.c_str(),
                                 true,
                                 true );
        }
    } // end MPM -> MPM+



    // ===== 2. MPM+ -> MPM
    if ( (eSystemCardsMode_mpm_plus == oldMode) && (eSystemCardsMode_mpm == newMode) )
    {
        // if the alternative Alert exists, it means that the system gets back to its 'original' mode
        if ( true == IsActiveAlarmExistByErrorCode(AA_SYSTEM_CARDS_MODE_TO_MPM_PLUS) )
        {
            RemoveActiveAlarmByErrorCode(AA_SYSTEM_CARDS_MODE_TO_MPM_PLUS);
        }

        // else - it means that the mode is changed: an Alert should be produced
        else
        {
            AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
            						 AA_SYSTEM_CARDS_MODE_TO_MPM,
                                     MAJOR_ERROR_LEVEL,
                                     "MPM card is disabled; please reset the MCU in order to use all cards in the system",
                                     true,
                                     true );
        }
    } // end MPM+ -> MPM
}
*/

/*
moved to the specific task
/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnEthSettingInd(CSegment* pSeg)
{
	ETH_SETTINGS_SPEC_S* pEthSettings = (ETH_SETTINGS_SPEC_S*)pSeg->GetPtr();

	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnEthSettingInd - params received from CM:"
								  << "\nSlotId: " << pEthSettings->portParams.slotId
								  << ", portId: " << pEthSettings->portParams.portNum;

	CEthernetSettingsStructWrappersList *pEthList = m_pProcess->GetEthernetSettingsStructsList();
	pCurEth = pEthList->SetSpecEthernetSettingsStructWrapper(pEthSettings);
}
*/

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnEthSettingMonitoringFromSysMonitor(CSegment* pSeg)
{
//	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnEthSettingMonitoringFromSysMonitor";

	// ===== 1. get the parameters from the structure received
	ETH_SETTINGS_SPEC_S* pEthSettings = new ETH_SETTINGS_SPEC_S;
	pSeg->Get( (BYTE*)pEthSettings, sizeof(ETH_SETTINGS_SPEC_S) );

	// ===== 2. replace boardId with 'displayBoardId' (since it is used for monitoring)
//	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(pEthSettings->portParams.slotId, FIXED_CM_SUBBOARD_ID);
//	pEthSettings->portParams.slotId = displayBoardId;

	/********************************************************************************
	//  VNGR-23938 Rachel Cohen   the port arrive in msg no need to translate it.
	*********************************************************************************/
	//DWORD portIdOnSwitch = GetLanPortOnSwitch(pEthSettings->portParams.portNum);
	
	pEthSettings->portParams.slotId		= FIXED_DISPLAY_BOARD_ID_SWITCH;
	
	pEthSettings->portParams.portNum	= pEthSettings->portParams.portNum;   //portIdOnSwitch;

	// ===== 3. update the process
	CEthernetSettingsStructWrappersList *pEthList = m_pProcess->GetEthernetSettingsStructsList();
	pEthList->SetSpecEthernetSettingsStructWrapper(pEthSettings);

	/*temp - for debugging*/
//	CEthernetSettingsStructWrapper tempStruct(true);
//	tempStruct.SetEthSettingsStruct(pEthSettings);
//	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnEthSettingMonitoringFromSysMonitor - struct received from SysMonitor:\n"
//								  << tempStruct;
	/*temp - for debugging*/

	SendEthernetSettingsActivePortsReqToSwitch();


	delete pEthSettings;
}

/////////////////////////////////////////////////////////////////////////////
/*DWORD CCardsManager::GetLanPortOnSwitch(const DWORD portOnCpu)
{
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if ( (eProductTypeRMX4000 == curProductType))
	{
		switch (portOnCpu)
		{
		case ETH_SETTINGS_PORT_MANAGEMENT:
			return ETH_SETTINGS_CPU_MNGMNT_1_PORT_ON_SWITCH_BOARD;

		case ETH_SETTINGS_PORT_SIGNALING:
			return ETH_SETTINGS_CPU_SGNLNG_1_PORT_ON_SWITCH_BOARD;

		default:
			return 0;
		}
	}
	if ( (eProductTypeRMX1500 == curProductType))
	{
		switch (portOnCpu)
		{
		case ETH_SETTINGS_CPU_MNGMNT_PORT_ON_SWITCH_BOARD_RMX1500:
			return ETH_SETTINGS_CPU_MNGMNT_PORT_ON_SWITCH_BOARD_RMX1500;

		case ETH_SETTINGS_CPU_MNGMNTB_PORT_ON_SWITCH_BOARD_RMX1500:
			return ETH_SETTINGS_CPU_MNGMNTB_PORT_ON_SWITCH_BOARD_RMX1500;

		default:
			return 0;
		}
	}

}*/

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnEthSettingActivePortsSwitchInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnEthSettingActivePortsSwitchInd";

	SendEthernetSettingsActivePortsReqToSwitch();

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if ( (eProductTypeRMX1500 == curProductType) ||  (eProductTypeRMX4000 == curProductType) )
	   SendEthSetSysMonitoringReq();
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendEthernetSettingsActivePortsReqToSwitch()
{
	ETH_SETTINGS_ACTIVE_PORTS_LIST_S *pActivePortsListStruct = new ETH_SETTINGS_ACTIVE_PORTS_LIST_S;

	BYTE switchBoardIdFromProcess    = m_pProcess->GetAuthenticationStruct()->switchBoardId,
	     switchSubBoardIdFromProcess = m_pProcess->GetAuthenticationStruct()->switchSubBoardId;

	FillEthernetSettingsActivePortsList(pActivePortsListStruct);
	PrintEthernetSettingsActivePortsListStruct(pActivePortsListStruct, "CCardsManager::SendEthernetSettingsActivePortsReqToSwitch");

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(ETHERNET_SETTINGS_ACTIVE_PORTS_SWITCH_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, switchBoardIdFromProcess, switchSubBoardIdFromProcess);
	mplPrtcl->AddData(sizeof(ETH_SETTINGS_ACTIVE_PORTS_LIST_S), (char*)pActivePortsListStruct);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
	PDELETE(pActivePortsListStruct);
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::FillEthernetSettingsActivePortsList(ETH_SETTINGS_ACTIVE_PORTS_LIST_S *pInStruct)
{
	CEthernetSettingsStructWrappersList	*pEthList	= m_pProcess->GetEthernetSettingsStructsList();
	CEthernetSettingsStructWrapper		*pCurEth	= NULL;
	ETH_SETTINGS_SPEC_S					*pCurStruct	= NULL;

	DWORD curSlotId = 0,
		  curPortId = 0;

	memset( pInStruct, 0, sizeof(ETH_SETTINGS_ACTIVE_PORTS_LIST_S) );

	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		pCurEth = pEthList->GetSpecEthernetSettingsStructWrapper(i);
		if (pCurEth)
		{
			pCurStruct	= pCurEth->GetEthSettingsStruct();
			curSlotId	= pCurStruct->portParams.slotId;
			curPortId	= pCurStruct->portParams.portNum;

			pInStruct->activePortsList[i].portParams.slotId		= curSlotId;
			pInStruct->activePortsList[i].portParams.portNum	= curPortId;
			pInStruct->activePortsList[i].isMounted				= pCurEth->GetIsMounted();


			pInStruct->activePortsList[i].isActive			= pCurStruct->monitoringParams.ulActLinkStatus;

			pInStruct->activePortsList[i].e802_1xSuppPortStatus			= pCurStruct->monitoringParams.e802_1xSuppPortStatus;
			pInStruct->activePortsList[i].e802_1xMethod			= pCurStruct->monitoringParams.e802_1xMethod;
			pInStruct->activePortsList[i].e802_1xFailReason			= pCurStruct->monitoringParams.e802_1xFailReason;


		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::PrintEthernetSettingsActivePortsListStruct(ETH_SETTINGS_ACTIVE_PORTS_LIST_S *pStruct, const string theCaller)
{
	string strToPrint = "\nCCardsManager::PrintEthernetSettingsActivePortsListStruct (caller: ";
	strToPrint += theCaller;
	strToPrint += ")";

	char tmpBuf[ONE_LINE_BUFFER_LEN];
	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		memset(tmpBuf, 0, ONE_LINE_BUFFER_LEN);

		int isActiveFromStruct	= (int)(pStruct->activePortsList[i].isActive);
		int isMountedFromStruct	= (int)(pStruct->activePortsList[i].isMounted);

		snprintf( tmpBuf, sizeof(tmpBuf),
				 "\nSlotId %d, PortId %d - isActive: %s (%d), isMounted: %s (%d) , 802_1xSuppPortStatus %d , 802_1xMethod %d  , 802_1xFailReason %d",
				 pStruct->activePortsList[i].portParams.slotId,
				 pStruct->activePortsList[i].portParams.portNum,
				 (isActiveFromStruct ? "yes" : "no"),
				 isActiveFromStruct,
				 (isMountedFromStruct ? "yes" : "no"),
				 isMountedFromStruct ,
				 pStruct->activePortsList[i].e802_1xSuppPortStatus,
				 pStruct->activePortsList[i].e802_1xMethod,
				 pStruct->activePortsList[i].e802_1xFailReason);

		strToPrint += tmpBuf;
	}

	TRACESTR(eLevelInfoNormal) << strToPrint;
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnEthSettingMonitoringSwitchInd(CSegment* pSeg)
{
	// ===== 1. extract the mplMcmsProtocol from segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	// validate data size
	STATUS sizeStat = pMplMcmsProtocol->ValidateDataSize( sizeof(ETH_SETTINGS_PORT_DESC_S) );
	if (STATUS_OK != sizeStat)
	{
		POBJDELETE(pMplMcmsProtocol);
		return;
	}


	// ===== 2. get the new ethSettingsPortDesc data into an ETH_SETTINGS_PORT_DESC_S struct
	ETH_SETTINGS_PORT_DESC_S portDesc;
	memset( &portDesc, 0, sizeof(ETH_SETTINGS_PORT_DESC_S));
	memcpy( &portDesc, pMplMcmsProtocol->GetData(), sizeof(ETH_SETTINGS_PORT_DESC_S) );

	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnEthSettingMonitoringSwitchInd - params received from CM:"
								  << "\nSlotId: " << portDesc.slotId
								  << "\nPortId: " << portDesc.portNum;


	// ===== 3. send the relevant info to Switch
	CEthernetSettingsStructWrappersList	*pEthList	= m_pProcess->GetEthernetSettingsStructsList();
	CEthernetSettingsStructWrapper		*pCurEth	= NULL;

	for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
	{
		pCurEth = pEthList->GetSpecEthernetSettingsStructWrapper(i);
		if (*pCurEth == portDesc) // similar slotId and portId
		{
			SendEthernetSettingsMonitoringReqToSwitch(pCurEth);
		}
	} // end loop
	POBJDELETE(pMplMcmsProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendEthernetSettingsMonitoringReqToSwitch(CEthernetSettingsStructWrapper *pCurEth)
{
	ETH_SETTINGS_SPEC_S *pSpecSettingsStruct = new ETH_SETTINGS_SPEC_S;
	memcpy( pSpecSettingsStruct,
			pCurEth->GetEthSettingsStruct(),
			sizeof(ETH_SETTINGS_SPEC_S) );

	PrintSpecSettingsStruct(pSpecSettingsStruct, "CCardsManager::SendEthernetSettingsMonitoringReqToSwitch");


	SwapEndian(pSpecSettingsStruct);


	BYTE switchBoardIdFromProcess    = m_pProcess->GetAuthenticationStruct()->switchBoardId,
	     switchSubBoardIdFromProcess = m_pProcess->GetAuthenticationStruct()->switchSubBoardId;

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(ETHERNET_SETTINGS_MONITORING_SWITCH_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, switchBoardIdFromProcess, switchSubBoardIdFromProcess);
	mplPrtcl->AddData(sizeof(ETH_SETTINGS_SPEC_S), (char*)pSpecSettingsStruct);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
	PDELETE(pSpecSettingsStruct);
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::PrintSpecSettingsStruct(ETH_SETTINGS_SPEC_S *pStruct, const string theCaller)
{
	char bufToPrint[TEN_LINE_BUFFER_LEN];
	memset(bufToPrint, 0, TEN_LINE_BUFFER_LEN);

	sprintf( bufToPrint,
			 "\nSlotId: %d\nPortId: %d\nRxPackets: %d\nRxBadPackets: %d\nRxCRC: %d\nRxOctets: %d\nMaxRxPackets: %d\nMaxRxPackets: %d\nMaxRxCRC: %d\nMaxRxOctets: %d\nTxPackets: %d\nTxBadPackets: %d\nTxFifoDrops: %d\nTxOctets: %d\nMaxTxPackets: %d\nMaxTxBadPackets: %d\nMaxTxFifoDrops: %d\nMaxTxOctets: %d\nActLinkStatus: %d\nActLinkMode: %d\nActLinkAutoNeg: %d\nAdvLinkMode: %d\nAdvLinkAutoNeg: %d\ne802_1xSuppPortStatus: %d\ne802_1xMethod: %d\ne802_1xFailReason: %d ",
			 pStruct->portParams.slotId,
			 pStruct->portParams.portNum,
			 pStruct->monitoringParams.ulRxPackets,
			 pStruct->monitoringParams.ulRxBadPackets,
			 pStruct->monitoringParams.ulRxCRC,
			 pStruct->monitoringParams.ulRxOctets,
			 pStruct->monitoringParams.ulMaxRxPackets,
			 pStruct->monitoringParams.ulMaxRxBadPackets,
			 pStruct->monitoringParams.ulMaxRxCRC,
			 pStruct->monitoringParams.ulMaxRxOctets,
			 pStruct->monitoringParams.ulTxPackets,
			 pStruct->monitoringParams.ulTxBadPackets,
			 pStruct->monitoringParams.ulTxFifoDrops,
			 pStruct->monitoringParams.ulTxOctets,
			 pStruct->monitoringParams.ulMaxTxPackets,
			 pStruct->monitoringParams.ulMaxTxBadPackets,
			 pStruct->monitoringParams.ulMaxTxFifoDrops,
			 pStruct->monitoringParams.ulMaxTxOctets,
			 pStruct->monitoringParams.ulActLinkStatus,
			 pStruct->monitoringParams.ulActLinkMode,
			 pStruct->monitoringParams.ulActLinkAutoNeg,
			 pStruct->monitoringParams.ulAdvLinkMode,
			 pStruct->monitoringParams.ulAdvLinkAutoNeg,
			 pStruct->monitoringParams.e802_1xSuppPortStatus,
			 pStruct->monitoringParams.e802_1xMethod,
			 pStruct->monitoringParams.e802_1xFailReason );

	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::PrintSpecSettingsStruct "
						   << "(caller: " << theCaller << ")" << bufToPrint;
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnEthSettingClearMaxCountersSwitchInd(CSegment* pSeg)
{
	// ===== 1. extract the mplMcmsProtocol from segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	// validate data size
	STATUS sizeStat = pMplMcmsProtocol->ValidateDataSize( sizeof(ETH_SETTINGS_PORT_DESC_S) );
	if (STATUS_OK != sizeStat)
	{
		POBJDELETE(pMplMcmsProtocol);
		return;
	}


	// ===== 2. get the new ethSettingsPortDesc data into an ETH_SETTINGS_PORT_DESC_S struct
	ETH_SETTINGS_PORT_DESC_S portDesc;
	memset( &portDesc, 0, sizeof(ETH_SETTINGS_PORT_DESC_S));
	memcpy( &portDesc, pMplMcmsProtocol->GetData(), sizeof(ETH_SETTINGS_PORT_DESC_S) );

	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnEthSettingClearMaxCountersSwitchInd - params received from CM:"
									  << "\nSlotId: " << portDesc.slotId
									  << "\nPortId: " << portDesc.portNum;


	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	if (m_pProcess->IsCNRLPort(portDesc.portNum,curProductType) )
	{
		SendClearMaxCountersToSysMonitoring(portDesc);   // we need to continue and send to mplapi in order to response to EMA  transaction.
		m_pProcess->GetEthernetSettingsStructsList()->ClearSpecMaxCounters(portDesc.slotId,portDesc.portNum);

		CEthernetSettingsStructWrappersList	*pEthList	= m_pProcess->GetEthernetSettingsStructsList();
		CEthernetSettingsStructWrapper		*pCurEth	= NULL;

		for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
		{
			pCurEth = pEthList->GetSpecEthernetSettingsStructWrapper(i);
			if (*pCurEth == portDesc) // similar slotId and portId
			{

				pCurEth->ClearMaxCounters();
				ETH_SETTINGS_SPEC_S* pCurStruct = NULL;
				pCurStruct = pCurEth->GetEthSettingsStruct();

				CEthernetSettingsStructWrapper tempStruct(true);
				tempStruct.SetEthSettingsStruct(pCurStruct);
				TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnEthSettingClearMaxCountersSwitchInd - struct received from CM:\n" << tempStruct;
			}
		} // end loop
	}


	// ===== 3. send the relevant info to Switch														// configured by CM

	if (portDesc.slotId == FIXED_DISPLAY_BOARD_ID_SWITCH)
			portDesc.slotId = FIXED_BOARD_ID_SWITCH;         //before sending msg to MFA we need to replace displayBoardId with BoardId
	else
		    portDesc.slotId = m_pProcess->GetSlotsNumberingConversionTable()->GetBoardId( (DWORD)portDesc.slotId, RTM_ISDN_SUBBOARD_ID);


		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnEthSettingClearMaxCountersSwitchInd - after conversion :"
									  << "\nSlotId: " << portDesc.slotId
									  << "\nPortId: " << portDesc.portNum;


	SendClearMaxCountersReqToMplApi(portDesc);


	POBJDELETE(pMplMcmsProtocol);

}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnSipProxyToCardsIceInitReq(CSegment* pSeg)
{
	// 1. Retrieve the data
	ICE_SERVER_TYPES_S* pIceServerTypesStruct = new ICE_SERVER_TYPES_S;

	pSeg->Get( (BYTE*)pIceServerTypesStruct, sizeof(ICE_SERVER_TYPES_S) );

	// ===== 2. print to trace
	PrintSipProxyIceInitRequest(pIceServerTypesStruct);

	// ===== 3. send to all the cards
    SendIceInitToAllMpmCM(pIceServerTypesStruct);

}
/////////////////////////////////////////////////////////////////////////////
void CCardsManager::PrintSipProxyIceInitRequest(ICE_SERVER_TYPES_S* pIceServerTypesStruct)
{
    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::PrintSipProxyIceInitRequest - struct received from SipProxy process - id = " << pIceServerTypesStruct->req_id
								  << "\nuser_name:" << pIceServerTypesStruct->authParams.user_name
        						  << "\npassword: " << pIceServerTypesStruct->authParams.password
        						  << "\nrealm: "    << pIceServerTypesStruct->authParams.realm
    							  << "\nStun_pass_server_params- sIpAddr: " << pIceServerTypesStruct->stun_pass_server_params.sIpAddr
    							  << " port: " << pIceServerTypesStruct->stun_pass_server_params.port
    							  << "\nStun_udp_server_params- sIpAddr: " << pIceServerTypesStruct->stun_udp_server_params.sIpAddr
    							  << " port: " << pIceServerTypesStruct->stun_udp_server_params.port
    							  << "\nStun_tcp_server_params - sIpAddr: " << pIceServerTypesStruct->stun_tcp_server_params.sIpAddr
    							  << " port: " << pIceServerTypesStruct->stun_tcp_server_params.port
    							  << "\nRelay_udp_server_params - sIpAddr: " << pIceServerTypesStruct->relay_udp_server_params.sIpAddr
    							  << " port: " << pIceServerTypesStruct->relay_udp_server_params.port
    							  << "\nRelay_tcp_server_params - sIpAddr: " << pIceServerTypesStruct->relay_tcp_server_params.sIpAddr
    							  << " port: " << pIceServerTypesStruct->relay_tcp_server_params.port
    							  << "\nisEnableBWPolicyCheck: " << pIceServerTypesStruct->isEnableBWPolicyCheck
    							  << "\nforced_MS_version: " << ntohl(pIceServerTypesStruct->forced_MS_version);
 }
/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendIceInitToAllMpmCM(ICE_SERVER_TYPES_S* pIceServerTypesStruct)
{
	//start timer to get all the responses
	StartTimer(ICE_INIT_TIMER, ICE_INIT_TIME_LIMIT);

	CCommCardDB* pCardDB = m_pProcess->GetCardsMonitoringDB();

	int fixedSubBoardId = FIRST_SUBBOARD_ID;	/*MPM board*/

	m_pProcess->InitIceResponseList();

	m_pProcess->GetCardsIceResponseList()->SetReqId(pIceServerTypesStruct->req_id);

	//BRIDGE-1358
	if(IsMultipleServicesMode())
		m_pProcess->GetCardsIceResponseList()->SetIceServerTypes(pIceServerTypesStruct, pIceServerTypesStruct->service_id);	//add request to array to monitor the response
	else
		m_pProcess->GetCardsIceResponseList()->SetIceServerTypes(pIceServerTypesStruct);//add request to array to monitor the response

	//_M_S_
	int board_id=0;
	int subBoardId = 0;
	BYTE bFound = FALSE;
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendIceInitToAllMpmCM - service_id=" << pIceServerTypesStruct->service_id;

	CMediaIpParameters* mediaIpParamsFromVector = NULL;
	CMediaIpParamsVector * mediaIpParamsVector = m_pProcess->GetMediaIpParamsVector();
	if(mediaIpParamsVector == NULL)
	{
		TRACESTR(eLevelInfoNormal) << "\nCardsManager::SendIceInitToAllMpmCM  - GetMediaIpParamsVector() == NULL!!";
		return;
	}

	int nNumOfMediaParams = mediaIpParamsVector->Size();
	if(nNumOfMediaParams <=0)
	{
		TRACESTR(eLevelInfoNormal) << "\nCardsManager::SendIceInitToAllMpmCM  - nNumOfMediaParams <=0!!";
		return;
	}

	for(int i = 0; i < nNumOfMediaParams; i++)
	{
		TRACEINTO << "dbg ----------i:" << i;
		mediaIpParamsFromVector = mediaIpParamsVector->At(i);
		if (mediaIpParamsFromVector == NULL)
			TRACESTR(eLevelInfoNormal) << "\nCardsManager::SendIceInitToAllMpmCM - nothing!!!";
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCardsManager::SendIceInitToAllMpmCM - boardId=" << mediaIpParamsFromVector->GetBoardId()
									<< " , subBoardId=" << mediaIpParamsFromVector->GetSubBoardId() << " , index=" << i;

			if(mediaIpParamsFromVector->GetServiceId() == pIceServerTypesStruct->service_id)
			{
				TRACEINTO << "dbg  IN mediaIpParamsFromVector->GetServiceId() == pIceServerTypesStruct->service_id";
				board_id = mediaIpParamsFromVector->GetBoardId();
				subBoardId = mediaIpParamsFromVector->GetSubBoardId(); // ??

				eCardType curCardType = pCardDB->GetCardType(board_id, fixedSubBoardId);

				if (m_pProcess->IsMediaCard(curCardType))		//send the request only to media cards
				{
					TRACEINTO << "dbg  IN m_pProcess->IsMediaCard(curCardType)";
					COsQueue* queue = m_pProcess->GetMfaMbx(board_id, fixedSubBoardId);
					if (queue)
					{
						TRACESTR(eLevelInfoNormal) << "\nCardsManager::SendIceInitToAllMpmCM  - Send ICE init to specific board";
						m_pProcess->SendIceInitToSpecificMpmCM(board_id, fixedSubBoardId, pIceServerTypesStruct);
					}
				}
			}
		}
	}

	//print db status
	m_pProcess->PrintIceResponseListDB();
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnTimerIceInitTimeout()
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnTimerIceInitTimeout - didn't get the answer from all the cards. Sending what I have.";

	SendIceInitIndResultToCsMngr();
}

//_M_S_
/////////////////////////////////////////////////////////////////////////////
BOOL CCardsManager::IsMultipleServicesMode()
{
	BOOL isMultipleServicesCfgFlag = NO, isMultipleServicesMode = NO;

	CSysConfig *sysConfig = m_pProcess->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_MULTIPLE_SERVICES, isMultipleServicesCfgFlag);
	if (m_pProcess->GetLicensingStruct()->multipleServices && isMultipleServicesCfgFlag)
		isMultipleServicesMode = YES;

	return isMultipleServicesMode;
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnIceInitIndReceived(CSegment* pSeg)
{
	DWORD reqId = 0;

	if(NULL == pSeg)
	{
		PTRACE(eLevelError, "OnIceInitIndReceived(): Input segment is NULL");
		return;
	}

	*pSeg >> reqId;

	//_M_S_
	if(IsMultipleServicesMode())
	{
		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnIceInitIndReceived - Multiple services mode - received ICE response";
		DeleteTimer(ICE_INIT_TIMER);
		SendIceInitIndResultToCsMngr();
	}
	else if (m_pProcess->IsAllICEResponseReceived(reqId))
	{
		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnIceInitIndReceived - received all ICE responses";
		DeleteTimer(ICE_INIT_TIMER);

		SendIceInitIndResultToCsMngr();
	}
	else
		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnIceInitIndReceived - still waiting for more ICE responses";
}
/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnIceStatIndReceived(CSegment* pSeg)
{
	DWORD BoardID = 0;
	DWORD SubBoardID = 0;

	if(NULL == pSeg)
	{
		PTRACE(eLevelError, "OnIceStatIndReceived(): Input segment is NULL");
		return;
	}

	// ===== 1. extract ice_init_ind struct from the segment received
	//BoardID << pSeg;
	//SubBoardID <<pSeg;
	ICE_STATUS_IND_S* pIceStatusInd =  (ICE_STATUS_IND_S*)pSeg->GetPtr();

	// ===== 2. print the data to trace
	TRACESTR(eLevelInfoNormal)
		<< "\nCCCardsManager::OnIceStatIndReceived: "
		<< "\nSTUN_udp_status:  " << pIceStatusInd->STUN_udp_status
		<< "\nSTUN_tcp_status:  " << pIceStatusInd->STUN_tcp_status
		<< "\nRelay_udp_status: " << pIceStatusInd->Relay_udp_status
		<< "\nRelay_tcp_status: " << pIceStatusInd->Relay_tcp_status
		<< "\nfw_type:          " << pIceStatusInd->fw_type
		<< "\nIce Type:			" << pIceStatusInd->ice_env;


	ICE_STATUS_IND_S IceStatusInd;
	memcpy(&IceStatusInd,pIceStatusInd,sizeof(ICE_STATUS_IND_S));

	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnIceStatIndReceived BoardID= "  << BoardID << "SubBoardID";

	const COsQueue* pCsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);
	//STATUS res = pCsMbx->Send(pParam,CARDS_TO_CS_ICE_DETAILS_UPDATE_STATUS);

	SendIceStatIndResultToCsMngr(&IceStatusInd);

}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendIceInitIndResultToCsMngr()
{
	CCardsIceResponseList* pCardsIceResponseList = m_pProcess->GetCardsIceResponseList();

	if (pCardsIceResponseList)
	{
		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendIceInitIndResultToCsMngr pCardsIceResponseList->GetReqId() = "<<pCardsIceResponseList->GetReqId()<<" pCardsIceResponseList->GetNumOfCards()="<<pCardsIceResponseList->GetNumOfCards();

		//_M_S_
		for(int j = 0; j<MAX_NUM_OF_BOARDS ; j++)
		{
			if(pCardsIceResponseList->GetIceServerTypes(j) == NULL)
				continue;

			//updating cs manager with the results
			CSegment*  pParam = new CSegment;

			*pParam << pCardsIceResponseList->GetReqId();
			*pParam << pCardsIceResponseList->GetNumOfCards();
			ICE_SERVER_TYPES_S* pIce_servers_type = pCardsIceResponseList->GetIceServerTypes(j);
			pParam->Put( (BYTE*)pIce_servers_type, sizeof(ICE_SERVER_TYPES_S) );

			for (int i=0; i<pCardsIceResponseList->GetNumOfCards(); i++)
			{
				CCardIceResponse* currCardIceResponse = pCardsIceResponseList->GetCardResponse(i);
				if(currCardIceResponse)
				{
					*pParam << (WORD)currCardIceResponse->GetBoardId();
					*pParam << (WORD)currCardIceResponse->GetSubBoardId();
					*pParam << (WORD)(m_pProcess->GetCardServiceId(currCardIceResponse->GetBoardId(), currCardIceResponse->GetSubBoardId()));

					ICE_INIT_IND_S ice_response = currCardIceResponse->GetIceResponse();
					pParam->Put( (BYTE*)&ice_response, sizeof(ICE_INIT_IND_S) );
				}
				else
				{
					PTRACE(eLevelError, "CCardsManager::SendIceInitIndResultToCsMngr: currCardIceResponse is NULL");
					continue;
				}
			}

			const COsQueue* pCsMbx =
					CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

			STATUS res = pCsMbx->Send(pParam,CARDS_TO_CS_ICE_DETAILS);
		}
	}
	else
		PTRACE(eLevelError, "OnIceInitIndReceived(): The card response list is NULL");	
}
/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendIceStatIndResultToCsMngr(ICE_STATUS_IND_S* IceStatusInd)
{
	CCardsIceResponseList* pCardsIceResponseList = m_pProcess->GetCardsIceResponseList();

	if (pCardsIceResponseList)
	{
		TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendIceStatIndResultToCsMngr pCardsIceResponseList->GetReqId() = "<<pCardsIceResponseList->GetReqId()<<" pCardsIceResponseList->GetNumOfCards()="<<pCardsIceResponseList->GetNumOfCards();

		//updating cs manager with the results
		CSegment*  pParam = new CSegment;
		*pParam << pCardsIceResponseList->GetNumOfCards();

		for(int j = 0; j<MAX_NUM_OF_BOARDS ; j++)
		{
			if(pCardsIceResponseList->GetIceServerTypes(j) == NULL)
				continue;

			//*pParam << pCardsIceResponseList->GetReqId();
			ICE_SERVER_TYPES_S* pIce_servers_type = pCardsIceResponseList->GetIceServerTypes(j);
			pParam->Put( (BYTE*)pIce_servers_type, sizeof(ICE_SERVER_TYPES_S) );

			for (int i=0; i<pCardsIceResponseList->GetNumOfCards(); i++)
			{
				CCardIceResponse* currCardIceResponse = pCardsIceResponseList->GetCardResponse(i);
				if(currCardIceResponse)
				{
					*pParam << (WORD)currCardIceResponse->GetBoardId();
					*pParam << (WORD)currCardIceResponse->GetSubBoardId();
					*pParam << (WORD)(m_pProcess->GetCardServiceId(currCardIceResponse->GetBoardId(), currCardIceResponse->GetSubBoardId()));

					ICE_INIT_IND_S ice_response = currCardIceResponse->GetIceResponse();
					ICE_STATUS_IND_S ice_stat_response;

					ice_stat_response.ice_env 	 		= IceStatusInd->ice_env;
					ice_stat_response.fw_type	 		= IceStatusInd->fw_type;
					ice_stat_response.STUN_udp_status 	= IceStatusInd->STUN_udp_status;
					ice_stat_response.STUN_tcp_status 	= IceStatusInd->STUN_udp_status;
					ice_stat_response.Relay_udp_status	= IceStatusInd->Relay_udp_status;
					ice_stat_response.Relay_tcp_status 	= IceStatusInd->Relay_tcp_status;
					pParam->Put( (BYTE*)&ice_stat_response, sizeof(ICE_STATUS_IND_S) );

				}
				else
				{
					PTRACE(eLevelError, "CCardsManager::SendIceStatIndResultToCsMngr: currCardIceResponse is NULL");
					continue;
				}
			}

			const COsQueue* pCsMbx =
					CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

			STATUS res = pCsMbx->Send(pParam,CARDS_TO_CS_ICE_DETAILS_UPDATE_STATUS);

		}
	}
	else
		PTRACE(eLevelError, "SendIceStatIndResultToCsMngr(): The card response list is NULL");
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendClearMaxCountersToSysMonitoring(ETH_SETTINGS_PORT_DESC_S &portDesc)
{
	DWORD portId = portDesc.portNum;

	CSegment* pSeg = new CSegment();
	*pSeg << portId;


	CManagerApi api(eProcessSystemMonitoring);
	api.SendMsg(pSeg, ETHERNET_SETTINGS_CLEAR_MAX_COUNTERS_REQ);

    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendClearMaxCountersToSysMonitoring"
                           << "\nBoardId:    " << portDesc.slotId
                           << "\nPortId:     " << portId;

}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendEthSetSysMonitoringReq()
{
	//DWORD portId = portDesc.portNum;

	//CSegment* pSeg = new CSegment();
	//*pSeg << portId;


	CManagerApi api(eProcessSystemMonitoring);
	api.SendMsg(NULL, ETHERNET_SETTINGS_SYSTEM_MONITORING_REQ);

    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendEthSetSysMonitoring";


}



/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendClearMaxCountersReqToMplApi(ETH_SETTINGS_PORT_DESC_S &portDesc)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendClearMaxCountersReqToMplApi - params sent to CM:"
	                       << "\nslotId:    " << (int)(portDesc.slotId)
	                       << "\nportNum:   " << (int)(portDesc.portNum);

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(ETHERNET_SETTINGS_CLEAR_MAX_COUNTERS_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, portDesc.slotId, FIXED_CM_SUBBOARD_ID);
	mplPrtcl->AddData(sizeof(ETH_SETTINGS_PORT_DESC_S), (char*)&portDesc);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SwapEndian(ETH_SETTINGS_SPEC_S *pSpecSettingsStruct)
{
	pSpecSettingsStruct->monitoringParams.ulRxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxPackets);
	pSpecSettingsStruct->monitoringParams.ulRxBadPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxBadPackets);
	pSpecSettingsStruct->monitoringParams.ulRxCRC = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxCRC);
	pSpecSettingsStruct->monitoringParams.ulRxOctets = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxOctets);
	pSpecSettingsStruct->monitoringParams.ulMaxRxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxRxPackets);
	pSpecSettingsStruct->monitoringParams.ulMaxRxBadPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxRxBadPackets);
	pSpecSettingsStruct->monitoringParams.ulMaxRxCRC = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxRxCRC);
	pSpecSettingsStruct->monitoringParams.ulMaxRxOctets = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxRxOctets);
	pSpecSettingsStruct->monitoringParams.ulRxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxPackets);
	pSpecSettingsStruct->monitoringParams.ulRxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxPackets);
	pSpecSettingsStruct->monitoringParams.ulTxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulTxPackets);
	pSpecSettingsStruct->monitoringParams.ulTxBadPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulTxBadPackets);
	pSpecSettingsStruct->monitoringParams.ulTxFifoDrops = SWAPL(pSpecSettingsStruct->monitoringParams.ulTxFifoDrops);
	pSpecSettingsStruct->monitoringParams.ulTxOctets = SWAPL(pSpecSettingsStruct->monitoringParams.ulTxOctets);
	pSpecSettingsStruct->monitoringParams.ulMaxTxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxTxPackets);
	pSpecSettingsStruct->monitoringParams.ulMaxTxBadPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxTxBadPackets);
	pSpecSettingsStruct->monitoringParams.ulMaxTxFifoDrops = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxTxFifoDrops);
	pSpecSettingsStruct->monitoringParams.ulMaxTxOctets = SWAPL(pSpecSettingsStruct->monitoringParams.ulMaxTxOctets);
	pSpecSettingsStruct->monitoringParams.ulRxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxPackets);
	pSpecSettingsStruct->monitoringParams.ulRxPackets = SWAPL(pSpecSettingsStruct->monitoringParams.ulRxPackets);
	pSpecSettingsStruct->monitoringParams.ulActLinkStatus = SWAPL(pSpecSettingsStruct->monitoringParams.ulActLinkStatus);
	pSpecSettingsStruct->monitoringParams.ulActLinkMode = SWAPL(pSpecSettingsStruct->monitoringParams.ulActLinkMode);
	pSpecSettingsStruct->monitoringParams.ulActLinkAutoNeg = SWAPL(pSpecSettingsStruct->monitoringParams.ulActLinkAutoNeg);
	pSpecSettingsStruct->monitoringParams.ulAdvLinkMode = SWAPL(pSpecSettingsStruct->monitoringParams.ulAdvLinkMode);
	pSpecSettingsStruct->monitoringParams.ulAdvLinkAutoNeg = SWAPL(pSpecSettingsStruct->monitoringParams.ulAdvLinkAutoNeg);
	pSpecSettingsStruct->monitoringParams.e802_1xSuppPortStatus = SWAPL(pSpecSettingsStruct->monitoringParams.e802_1xSuppPortStatus);
	pSpecSettingsStruct->monitoringParams.e802_1xMethod = SWAPL(pSpecSettingsStruct->monitoringParams.e802_1xMethod);
	pSpecSettingsStruct->monitoringParams.e802_1xFailReason = SWAPL(pSpecSettingsStruct->monitoringParams.e802_1xFailReason);
}

/*
moved to the specific task
////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnEthSettingClearMaxCountersInd(CSegment* pSeg)
{
	ETH_SETTINGS_STATE_S* pEthState = (ETH_SETTINGS_STATE_S*)pSeg->GetPtr();

	eEthSettingsState curState = (eEthSettingsState)(pEthState->configState);

	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnEthSettingClearMaxCountersInd - params received from CM:"
								  << "\nslotId:      " << (int)(pEthState->portParams.slotId)
								  << "\nportNum:     " << (int)(pEthState->portParams.portNum)
								  << "\nconfigState: " << (eEthSettingsState_ok == curState ? "ok" : "fail");

	SendClearMaxCountersIndToSwitch(pEthState);
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendClearMaxCountersIndToSwitch(ETH_SETTINGS_STATE_S* pEthState)
{
	eEthSettingsState curState = (eEthSettingsState)(pEthState->configState);

	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendClearMaxCountersIndToSwitch - params sent to CM:"
						   << "\nslotId:      " << (int)(pEthState->portParams.slotId)
						   << "\nportNum:     " << (int)(pEthState->portParams.portNum)
						   << "\nconfigState: " << (eEthSettingsState_ok == curState ? "ok" : "fail");

	BYTE switchBoardIdFromProcess    = m_pProcess->GetAuthenticationStruct()->switchBoardId,
	     switchSubBoardIdFromProcess = m_pProcess->GetAuthenticationStruct()->switchSubBoardId;

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(ETHERNET_SETTINGS_CLEAR_MAX_COUNTERS_SWITCH_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, switchBoardIdFromProcess, switchSubBoardIdFromProcess);
	mplPrtcl->AddData(sizeof(ETH_SETTINGS_STATE_S), (char*)pEthState);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("MCUMNGR_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}
*/
/////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnConfigCardReq(CSegment* pSeg)//2 modes cop/cp
{
	// ===== 1. retrieve the data

	CARDS_CONFIG_PARAMS_S* pCardsConfigParams = new CARDS_CONFIG_PARAMS_S;
	pSeg->Get( (BYTE*)pCardsConfigParams, sizeof(CARDS_CONFIG_PARAMS_S) );

	// ===== 2. print to trace
    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnConfigCardReq - struct received from ConfParty process"
                                  << "\nunSystemConfMode:  " << pCardsConfigParams->unSystemConfMode
                                  << "\nunFutureUse:   " << pCardsConfigParams->unFutureUse;

	// ===== 3. send to all cards
    for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
    {

		COsQueue* queue = m_pProcess->GetMfaMbx(i, FIRST_SUBBOARD_ID);
		if (queue) //connection is active - board exists.
		{
			CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

			mplPrtcl->AddCommonHeader(CARD_CONFIG_REQ);
			mplPrtcl->AddMessageDescriptionHeader();
			mplPrtcl->AddPhysicalHeader(1, i, FIRST_SUBBOARD_ID);//FIRST_SUBBOARD_ID means MPM board
			mplPrtcl->AddData(sizeof(CARDS_CONFIG_PARAMS_S), (char*)pCardsConfigParams);

			CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL");
			mplPrtcl->SendMsgToMplApiCommandDispatcher();

			POBJDELETE(mplPrtcl);
		}
    }

    delete pCardsConfigParams;
}

/////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendUpgradeStartedIndToMplApi()
{
	BOOL upgradeStarted = YES;

	CSegment* pSeg = new CSegment();
	*pSeg << upgradeStarted;

	CManagerApi api(eProcessMplApi);
	api.SendMsg(pSeg, UPGRADE_STARTED_IND);

    TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendUpgradeStartedIndToMplApi";
}

void CCardsManager::OnLoggerCardsMaxTraceLevel(CSegment* msg)
{
    CProcessBase* proc = CProcessBase::GetProcess();
    PASSERT_AND_RETURN(NULL == proc);

    LOG_LEVEL_S prm;
    *msg >> prm.log_level;

    // Sends to all cards
    for (int i = 0; i < MAX_NUM_OF_BOARDS; i++)
    {
        COsQueue* queue = m_pProcess->GetMfaMbx(i, FIRST_SUBBOARD_ID);
        if (NULL == queue)
            continue;

        OPCODE opcode = SET_LOG_LEVEL_REQ;

        CMplMcmsProtocol mpl;
        mpl.AddCommonHeader(opcode);
        mpl.AddMessageDescriptionHeader();
        mpl.AddPhysicalHeader(1, i, FIRST_SUBBOARD_ID);
        mpl.AddData(sizeof (LOG_LEVEL_S), reinterpret_cast<const char*>(&prm));

        CMplMcmsProtocolTracer(mpl).TraceMplMcmsProtocol(__PRETTY_FUNCTION__);
        STATUS stat = mpl.SendMsgToMplApiCommandDispatcher();

        PASSERTSTREAM_AND_RETURN(STATUS_OK != stat,
            "Unable to send " << proc->GetOpcodeAsString(opcode)
                << " (" << opcode << ") to " << ProcessNames[eProcessMplApi]
                << ", boardID " << i
                << ": " << proc->GetStatusAsString(stat));

        TRACESTRFUNC(eLevelError) << proc->GetOpcodeAsString(opcode)
                                     << " (" << opcode << ") to "
                                     << ProcessNames[eProcessMplApi]
                                     << ", boardID " << i
                                     << ", log level "
                                     << CTrace::GetTraceLevelNameByValue(prm.log_level)
                                     << " (" << prm.log_level << ")"
                                     << " is sent successfully";
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnStartTcpDump(CSegment* pSeg)
{
	// ===== 1. extract data
	UTILITY_START_TCP_DUMP_S* pTcpDumpParams = new UTILITY_START_TCP_DUMP_S;
	pSeg->Get( (BYTE*)pTcpDumpParams, sizeof(UTILITY_START_TCP_DUMP_S) );

	  TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnStartTcpDump - struct sent to MPM"
	                                  << "\nBoardId:  " << pTcpDumpParams->boardId
	                                  << "\nParams:   " << pTcpDumpParams->Params.sTcpDumpString;

	    CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

		mplPrtcl->AddCommonHeader(START_TCP_DUMP_REQ);
		mplPrtcl->AddMessageDescriptionHeader();
		mplPrtcl->AddPhysicalHeader(1, pTcpDumpParams->boardId, FIRST_SUBBOARD_ID/*MPM board*/);
		mplPrtcl->AddData(sizeof(TCP_DUMP_CONFIG_REQ_S), (char *)pTcpDumpParams->Params.sTcpDumpString);

		CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL");
		mplPrtcl->SendMsgToMplApiCommandDispatcher();

		POBJDELETE(mplPrtcl);

	delete pTcpDumpParams;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnStopTcpDump(CSegment* pSeg)
{
	// ===== 1. extract data
	UTILITY_START_TCP_DUMP_S* pTcpDumpParams = new UTILITY_START_TCP_DUMP_S;
	pSeg->Get( (BYTE*)pTcpDumpParams, sizeof(UTILITY_START_TCP_DUMP_S) );

	  TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnStopTcpDump - struct sent to MPM"
	                                  << "\nBoardId:  " << pTcpDumpParams->boardId;




	    CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

		mplPrtcl->AddCommonHeader(STOP_TCP_DUMP_REQ);
		mplPrtcl->AddMessageDescriptionHeader();
		mplPrtcl->AddPhysicalHeader(1, pTcpDumpParams->boardId, FIRST_SUBBOARD_ID/*MPM board*/);
		//mplPrtcl->AddData(sizeof(TCP_DUMP_CONFIG_REQ_S), (char *)pTcpDumpParams->Params.sTcpDumpString);

		CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL");
		mplPrtcl->SendMsgToMplApiCommandDispatcher();

		POBJDELETE(mplPrtcl);

	delete pTcpDumpParams;
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnStartTcpDumpInd(CSegment* pSeg)
{
	// ===== 1. extract the mplMcmsProtocol from segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	// validate data size
	STATUS sizeStat = pMplMcmsProtocol->ValidateDataSize( sizeof(TCP_DUMP_CONFIG_IND_S) );
	if (STATUS_OK != sizeStat)
	{
		POBJDELETE(pMplMcmsProtocol);
		return;
	}


	// ===== 2. retrieve  data, extract boardId & subBoardId and print
	TCP_DUMP_CONFIG_IND_S theMsg;
	memcpy( &theMsg, pMplMcmsProtocol->GetData(), sizeof(TCP_DUMP_CONFIG_IND_S) );

	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnStartTcpDumpInd Status="<< theMsg.eDumpStatus
	                       <<" ErrorReason="<< theMsg.uErrorReason;


	//BYTE msgBoardId		= pMplMcmsProtocol->getPhysicalInfoHeaderBoard_id(),
		// msgSubBoardId	= pMplMcmsProtocol->getPhysicalInfoHeaderSub_board_id();

	//int boardId		= (int)msgBoardId,
		//subBoardId	= (int)msgSubBoardId;
	CSegment* pSegData = new CSegment();
	pSegData->Put( (BYTE*)&theMsg, sizeof(TCP_DUMP_CONFIG_IND_S) );

	SendStartTcpDumpStatusToUtilityProcess(pSegData);

/*In case  eDumpStatus is eTcpDumpOk – success , tcp dump  is running
In case eDumpStatus is eTcpDumpInternalError – check uErrorReason one of values in eTcpDumpInternalErrorReason
In case eDumpStatus is eTcpDumpSystemError – check LINUX error codes*/



	POBJDELETE(pMplMcmsProtocol);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendStartTcpDumpStatusToUtilityProcess(CSegment* pSeg)
{


	// ===== 1. fill the Segment

	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendStartTcpDumpStatusToUtilityProcess";


	// ===== 2. send
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessUtility, eManager);

	STATUS res = pMbx->Send(pSeg, CARDS_TO_UTILITY_START_TCP_DUMP_STATUS, &GetRcvMbx());
}

////////////////////////////////////////////////////////////////////////////
void CCardsManager::OnStopTcpDumpInd(CSegment* pSeg)
{
	// ===== 1. extract the mplMcmsProtocol from segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	// validate data size
	STATUS sizeStat = pMplMcmsProtocol->ValidateDataSize( sizeof(TCP_DUMP_CONFIG_IND_S) );
	if (STATUS_OK != sizeStat)
	{
		POBJDELETE(pMplMcmsProtocol);
		return;
	}


	// ===== 2. retrieve  data, extract boardId & subBoardId and print
	TCP_DUMP_CONFIG_IND_S theMsg;
	memcpy( &theMsg, pMplMcmsProtocol->GetData(), sizeof(TCP_DUMP_CONFIG_IND_S) );

	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::OnStopTcpDumpInd Status="<< theMsg.eDumpStatus
	                       <<" ErrorReason="<< theMsg.uErrorReason;


	//BYTE msgBoardId		= pMplMcmsProtocol->getPhysicalInfoHeaderBoard_id(),
		// msgSubBoardId	= pMplMcmsProtocol->getPhysicalInfoHeaderSub_board_id();

	//int boardId		= (int)msgBoardId,
		//subBoardId	= (int)msgSubBoardId;
	CSegment* pSegData = new CSegment;
	pSegData->Put( (BYTE*)&theMsg, sizeof(TCP_DUMP_CONFIG_IND_S) );

	SendStopTcpDumpStatusToUtilityProcess(pSegData);
	/*switch (theMsg.eDumpStatus)
	{
	case eTcpDumpOk:

		break;
	case eTcpDumpOk:
		break;
	case eTcpDumpOk:
		break;
	default:


	}*/
/*In case  eDumpStatus is eTcpDumpOk – success , tcp dump  is running
In case eDumpStatus is eTcpDumpInternalError – check uErrorReason one of values in eTcpDumpInternalErrorReason
In case eDumpStatus is eTcpDumpSystemError – check LINUX error codes*/



	POBJDELETE(pMplMcmsProtocol);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsManager::SendStopTcpDumpStatusToUtilityProcess(CSegment* pSeg)
{


	// ===== 1. fill the Segment

	TRACESTR(eLevelInfoNormal) << "\nCCardsManager::SendStopTcpDumpStatusToUtilityProcess";


	// ===== 2. send
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessUtility, eManager);

	STATUS res = pMbx->Send(pSeg, CARDS_TO_UTILITY_STOP_TCP_DUMP_STATUS, &GetRcvMbx());
}

void CCardsManager::OnSNMPConfigInd(CSegment* pSeg)
{
  BOOL bSNMPEnabled;
  *pSeg >> bSNMPEnabled;
  
  TRACEINTO << "bSNMPEnabled: " << (bSNMPEnabled ? "true" : "false");

  CCardsProcess* proc = static_cast<CCardsProcess*>(CProcessBase::GetProcess());
  PASSERT_AND_RETURN(NULL == proc);
  proc->SetIsSNMPEnabled(bSNMPEnabled);
}

void CCardsManager::RescueCard(CRequest* pRequest)
{
    STATUS ret = STATUS_FAIL;
    DWORD boardId = 0;
    if(pRequest->GetAuthorization() != SUPER )
    {
    	TRACESTR(eLevelInfoNormal) << "CardsManager::RescueCard - No permission to rescue card";
    	pRequest->SetStatus(STATUS_NO_PERMISSION);
    	pRequest->SetConfirmObject(new CDummyEntry);
       return ;
    }

    CBoardNumberData *pBoardNumberData = (CBoardNumberData*)(pRequest->GetRequestObject());

    DWORD displayBoardId = pBoardNumberData->GetBoardId();

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if(eProductTypeNinja == curProductType)
	{	//Send Rescue Card Request -> MFA -> MPProxy -> VMP  in Ninja
		ret = SendRescueToMFA(displayBoardId, ALL_DSP_UNIT_ID);	
	}
	else
	{
		CSlotsNumberingConversionTableWrapper* pConversionTable = ((CCardsProcess *)CProcessBase::GetProcess())->GetSlotsNumberingConversionTable();

		DWORD boardId = pConversionTable->GetBoardId(displayBoardId, MFA_SUBBOARD_ID);

		DWORD rescueBoardId = TranslateBoardIdToRescueBoardId(boardId);

		ret = SendRescueToCard(rescueBoardId);
	}
	
	if (ret != STATUS_OK)
	{
		TRACESTR(eLevelInfoNormal) << "CardsManager::RescueCard - failed to send. displayBoardId " << displayBoardId << " boardId " <<boardId;

		pRequest->SetExDescription("Rescue card command failed.");

		pRequest->SetStatus(STATUS_FAIL);

	}
	else
	{

		pRequest->SetStatus(STATUS_OK);
		pRequest->SetConfirmObject(new CDummyEntry);
	}
}

#define RTM_SUB_BOARD_NUM                                 2
#define NO_DISPLAY_BOARD_ID                               0xFFFF

STATUS CCardsManager::SendRescueToMFA(DWORD rescueBoardId, DWORD dspUnitId)
{
#ifndef RESCUE_CARD_REQ
#define  RESCUE_CARD_REQ                             5010018
#endif

    WORD phyboardId = NO_DISPLAY_BOARD_ID, dspboardId = NO_DISPLAY_BOARD_ID, subBoardId = NO_DISPLAY_BOARD_ID;

    if(rescueBoardId >= DSP_CARD_SLOT_ID_0 && rescueBoardId <= DSP_CARD_SLOT_ID_2 )
    {
        //reuse Rescue Card(reset all dsp card) Request  as reset unit message -> MFA -> MPProxy -> VMP  in Ninja
        phyboardId = FIXED_BOARD_ID_MAIN_SOFTMCU;
        subBoardId = FIXED_CM_SUBBOARD_ID;
        dspboardId = rescueBoardId - DSP_CARD_SLOT_ID_0;
    }
    else if(rescueBoardId == ISDN_CARD_SLOT_ID)
    {
        //reuse Rescue Card(reset RTM DSP card) Request  as reset unit message -> MFA -> VMP  in Ninja
        phyboardId = 1;
        subBoardId = RTM_SUB_BOARD_NUM;
        dspboardId = 1;
    }
    else
    {
        TRACESTR(eLevelInfoNormal) << "unknown boardId : " << rescueBoardId << "  DSPUnitID : " <<  dspUnitId;
        return STATUS_FAIL;
    }
       
	CMplMcmsProtocol pMplMcmsProtocol;
	pMplMcmsProtocol.AddCommonHeader(RESCUE_CARD_REQ);
	pMplMcmsProtocol.AddMessageDescriptionHeader();
	pMplMcmsProtocol.AddPhysicalHeader(1, phyboardId, subBoardId);
	RESCUE_CARD_REQ_S rescueCardParam;
	rescueCardParam.boardID = dspboardId;
	rescueCardParam.unitID = dspUnitId;
	pMplMcmsProtocol.AddData(sizeof(RESCUE_CARD_REQ_S), (const char*)(&rescueCardParam));

	TRACESTR(eLevelInfoNormal) << "Sending rescue request to mfa. PhyboardId : " << phyboardId << "  subBoardId : " <<  subBoardId<< "  dspboardId : " <<  dspboardId<< "  DSPUnitID : " <<  dspUnitId;

	return pMplMcmsProtocol.SendMsgToMplApiCommandDispatcher();
}

STATUS CCardsManager::SendRescueToCard(DWORD rescueBoardId)
{
	CTerminalCommand command;
	const char* terminal_file_name = ttyname(0);
	if (NULL == terminal_file_name)
	{
	    terminal_file_name = "/dev/null";
	}
	command.AddToken(terminal_file_name);
	const string RESCUE_TOKEN = "SetCardsRescue";
	command.AddToken(RESCUE_TOKEN);

	command.AddToken(rescueBoardId);

	COstrStream ostr;
	command.Serialize(ostr);

	CMplMcmsProtocol mplProt;
	mplProt.AddCommonHeader(TERMINAL_COMMAND);
	mplProt.AddMessageDescriptionHeader();
	DWORD switchBoardId = IsTarget() ? FIXED_BOARD_ID_SWITCH : FIXED_BOARD_ID_SWITCH_SIM;

	mplProt.AddPhysicalHeader(1, switchBoardId, SWITCH_SUBBOARD_ID, 0);

	mplProt.AddPortDescriptionHeader(DUMMY_PARTY_ID,
	                                   DUMMY_CONF_ID,
	                                   DUMMY_CONNECTION_ID);
	string data = ostr.str();


	TRACESTR(eLevelInfoNormal) << "Sending rescue command to switch : " << switchBoardId <<  " command " << data;

	char   buffer[1024];
	strncpy(buffer, data.c_str(), sizeof(buffer) - 1);
	buffer[sizeof(buffer) - 1] = '\0';

	mplProt.AddData(1024, buffer);

	STATUS status = mplProt.SendMsgToMplApiCommandDispatcher();

	return status;

}

DWORD CCardsManager::TranslateBoardIdToRescueBoardId(DWORD boardId)
{
	if (boardId == FIXED_BOARD_ID_SWITCH)
	{
		return  FIXED_RESCUE_SWITCH_ID;
	}
	DWORD multiple = 1;
	DWORD rescueBoardId = 0;
	for (DWORD i = 0; i < boardId; ++i)
	{
		rescueBoardId = multiple;
		multiple = rescueBoardId * 2;
	}
	return rescueBoardId;

}

//--------------------------------------------------------------------------
 const char* CCardsManager::GetNGBSystemCardsModeStr(eNGBSystemCardsMode theMode)
{
  return (0 <= theMode && theMode < NUM_OF_NGB_SYSTEM_CARDS_MODES
         ?
         sNGBSystemCardsModes[theMode] : "Invalid mode");
}

