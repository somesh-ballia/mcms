// MfaTask.cpp

#include "MfaTask.h"

#include "Segment.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsInternal.h"
#include "MplMcmsProtocol.h"
#include "TraceStream.h"
#include "Trace.h"
#include "ProcessBase.h"
#include "TaskApi.h"
#include "CardsProcess.h"
#include "StatusesGeneral.h"
#include "FaultsDefines.h"
#include "HwMonitoring.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "MplMcmsProtocolTracer.h"
#include "HlogApi.h"
#include "StringsLen.h"
#include "SystemFunctions.h"
#include "RecordingRequestStructs.h"
#include "OpcodesMcmsCardMngrRecording.h"
#include "MfaApi.h"
#include "ApiStatuses.h"
#include "ArtRequestStructs.h"
#include "ArtIndicationStructs.h"
#include "../../../McmIncld/MPL/Card/PhysicalPortAudioCntl/AcRequestStructs.h"
#include "../../../McmIncld/MPL/Card/PhysicalPortAudioCntl/OpcodesMcmsAudioCntl.h"
#include "EthernetSettingsMonitoring.h"
#include "SlotsNumberingConversionTableWrapper.h"
#include "ConfigManagerApi.h"
#include "StringsMaps.h"
#include "OpcodesMcmsCardMngrICE.h"
#include "MultipleServicesFunctions.h"
#include "IpService.h"
#include "SipProxyTaskApi.h"
#include "SNMPStructs.h"
#include "AlarmStrTable.h"

#include "OsFileIF.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <fstream>

extern char* CardUnitConfiguredTypeToString(APIU32 unitConfigType);
extern char* CardUnitLoadedStatusToString(APIU32 unitLoadedStatus);
extern char* CardMediaIpConfigStatusToString(APIU32 mediaIpConfigStatus);
extern char* CardStateToString(APIU32 cardState);
extern const char* UnitReconfigStatusToString(APIU32 unitReconfigStatus);
extern const char* GetSystemCardsModeStr(eSystemCardsMode theMode);
extern char* CardTypeToString(APIU32 cardType);
extern char* IpTypeToString(APIU32 ipType, bool caps = false);
extern STATUS 	ConfigMoreDnsInOS(const char *pStrDomain, DWORD dnsAddress, const string theCaller);

const    WORD  STARTUP        = 1;
const    WORD  CONFIGURATION  = 2;
const    WORD  READY          = 3;

#define  STATUS_NO_ART            1    //Means - this unit is not ART and therefore irrelevant.
#define  STATUS_WAIT_FOR_REPLAY   2    //Means - req was sent and we wait for replay

#define SWAPL(X) (((X)&0xff)<<24)+(((X)&0xff00)<<8)+(((X)&0xff0000)>>8)+(((X)&0xff000000)>>24)


PBEGIN_MESSAGE_MAP(CMfaTask)
    ONEVENT( ACK_IND,                    ANYCASE,        CMfaTask::IgnoreMessage )
    ONEVENT( RSRCALLOC_UNIT_CONFIG_IND,  ANYCASE,        CMfaTask::OnRsrcAllocUnitConfigInd )
    ONEVENT( CM_UNIT_LOADED_IND,         CONFIGURATION,  CMfaTask::OnUnitLoadedInd )
    ONEVENT( CM_MEDIA_IP_CONFIG_IND,     READY,          CMfaTask::OnMediaIpConfigInd )
    ONEVENT( CM_CARD_MNGR_RECONNECT_IND, ANYCASE,        CMfaTask::OnCmReconnectInd )
    ONEVENT( CARDS_NEW_MEDIA_IP_IND,     ANYCASE,        CMfaTask::OnCardsNewMediaIpInd )
    ONEVENT( SPEC_MFA_ACTIVE_ALARMS_IND, ANYCASE,        CMfaTask::OnSpecMfaActiveAlarmInd )
	ONEVENT( MFA_STARTUP_TIMER,          ANYCASE,        CMfaTask::OnTimerStartupTimeout )
    ONEVENT( SM_MFA_FAILURE_IND,         ANYCASE,        CMfaTask::OnShmMfaFailureInd )
    ONEVENT( IVR_MUSIC_ADD_SOURCE_RECEIVED_IND, ANYCASE, CMfaTask::OnIvrMusicAddSourceReqReceivedInd )

	ONEVENT( CM_KEEP_ALIVE_IND,          ANYCASE,        CMfaTask::OnMfaKeepAliveInd )
	ONEVENT( KEEP_ALIVE_TIMER_RECEIVE,   ANYCASE,        CMfaTask::OnTimerKeepAliveReceiveTimeout )
	ONEVENT( KEEP_ALIVE_TIMER_SEND,      ANYCASE,        CMfaTask::OnTimerKeepAliveSendTimeout )
    ONEVENT( BAD_SPONTANEOUS_IND,      ANYCASE,        CMfaTask::OnBadSpontIndFromMFA )
    ONEVENT( AC_TYPE_REQ,			   ANYCASE,        CMfaTask::OnAudioCntrlrActiveReq )
    ONEVENT( CM_UNIT_RECONFIG_IND,     ANYCASE,        CMfaTask::OnCmUnitReconfigInd )
    ONEVENT(ETHERNET_SETTINGS_IND,						ANYCASE,	CMfaTask::OnEthSettingInd )
    ONEVENT(ETHERNET_SETTINGS_CLEAR_MAX_COUNTERS_IND,	ANYCASE,	CMfaTask::OnEthSettingClearMaxCountersInd )
    ONEVENT( CARDS_NEW_SYS_CFG_PARAMS_IND,  ANYCASE,     CMfaTask::OnNewSysCfgParamsInd )
    ONEVENT( OLD_SYSCFG_PARAMS_IND, 		ANYCASE,	 CMfaTask::OnOldSysCfgParamsInd )
    ONEVENT( CARDS_LICENSE_IND, 			READY,	 	 CMfaTask::OnLicenseInd )

	// messgaes that should not arrive to Mfa task (they actually arrive due to unknown cardType before sending etc)
	ONEVENT( RTM_ISDN_ATTACH_SPAN_MAP_IND,	ANYCASE,	CMfaTask::IgnoreMessage )
	ONEVENT( RTM_ISDN_DETACH_SPAN_MAP_IND,	ANYCASE,	CMfaTask::IgnoreMessage )
	ONEVENT( CM_UPGRADE_NEW_VERSION_READY_ACK_IND,	ANYCASE,	CMfaTask::OnCmUpgradeNewVersionReadyAckInd )

	ONEVENT( HOT_SWAP_RTM_REMOVED,		ANYCASE,	CMfaTask::IgnoreMessage )

	ONEVENT( CM_UPGRADE_PROGRESS_IND,   ANYCASE,	CMfaTask::OnCmUpgradeProcessInd )
	ONEVENT( CM_UPGRADE_IPMC_IND, 		ANYCASE, 	CMfaTask::OnCmUpgradeIpmcInd )
	ONEVENT( CM_UPGRADE_IPMC_PROGRESS_IND,	ANYCASE, CMfaTask::OnCmUpgradeIpmcProcessInd )
	ONEVENT( IPMC_UPGRADE_TIMER,       	ANYCASE, 	CMfaTask::OnCmUpgradeIpmcTimer )

	//Fips 140 indications
	ONEVENT( ART_FIPS_140_IND,          ANYCASE,         CMfaTask::OnArtFips140Ind)
	ONEVENT( FIPS_140_TIMER_RECEIVE_ALL_IND,  ANYCASE,   CMfaTask::OnTimerReceiveAllFips140IndsTimeout)
	ONEVENT( CARD_CONFIG_REQ,  			ANYCASE,         CMfaTask::OnCardConfigReq)//2 modes cop/cp
	ONEVENT( ICE_INIT_IND,					ANYCASE,	CMfaTask::OnIceInitInd)
	ONEVENT( ICE_STATUS_IND,				ANYCASE,	CMfaTask::OnIceStatusInd) // N.A. DEBUG
    ONEVENT( CM_CS_EXTERNAL_IP_CONFIG_IND,					ANYCASE,	CMfaTask::OnCSExternalConfigInd)
    ONEVENT( CM_CS_INTERNAL_IP_CONFIG_IND,                  ANYCASE,    CMfaTask::OnCSInternalConfigInd)
    ONEVENT( CM_DNS_CONFIG_IND,                  ANYCASE,    CMfaTask::OnCSDnsConfigInd)
    ONEVENT(MFA_IP_CONFIG_MSGS_TIMER,  ANYCASE,   CMfaTask::OnTimerReceiveAllIPConfigMsgsTimeout)
    ONEVENT(CM_MEDIA_CONFIGURATION_COMPLETED_IND,  ANYCASE,   CMfaTask::OnMediaConfigurationCompletedInd)
    ONEVENT(CS_EXT_INT_IP_CONFIG_FIRST_SERVICE_TIMER,   ANYCASE,        CMfaTask::OnTimerCSExtIntIPConfigFirstTimeout )
    ONEVENT(CS_EXT_INT_IP_CONFIG_SECOND_SERVICE_TIMER,   ANYCASE,        CMfaTask::OnTimerCSExtIntIPConfigSecondTimeout )

    ONEVENT(CS_EXT_INT_WAIT_REPLY_TIMER,   ANYCASE,        CMfaTask::OnTimerCSExtIntWaitTimeout )

	/*timer for signaling port */
	ONEVENT(MFA_SIGNAL_FIRST_PORT_INACTIVITY_TIMER,  ANYCASE,   CMfaTask::OnTimerFirstSignalPortInactTimeout)
    ONEVENT(MFA_SIGNAL_SECOND_PORT_INACTIVITY_TIMER,  ANYCASE,   CMfaTask::OnTimerSecondSignalPortInactTimeout)
PEND_MESSAGE_MAP(CMfaTask,CAlarmableTask);

void mfaEntryPoint(void* appParam)
{
	CMfaTask * pMfaTask = new CMfaTask;
	pMfaTask->Create(*(CSegment*)appParam);
}

CMfaTask::CMfaTask()
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask - constructor";

	m_pProcess = (CCardsProcess*)CCardsProcess::GetProcess();
	m_pUnitsTypesConfiguredStruct = new CM_UNITS_CONFIG_S;
	m_pIpMediaConfigVector        = new CMediaIpConfigVector;

	CSysConfig *sysConfig = m_pProcess->GetSysConfig();
	sysConfig->GetDWORDDataByKey("KEEP_ALIVE_RECEIVE_PERIOD", m_keepAliveReceivePeriod);
	m_keepAliveSendPeriod = DEFAULT_KEEP_ALIVE_SEND_PERIOD;

	m_isKeepAliveAlreadyReceived		= NO;
	m_numOfkeepAliveTimeoutReached		= 0;
	m_isKeepAliveFailureAlreadyTreated	= NO;
	m_isAssertKeepAliveAfterNoConnectionAlreadyProduced	= NO;
	m_lastKeepAliveReqTime.InitDefaults();
	m_lastKeepAliveIndTime.InitDefaults();

	m_isStartupOverActionsAlreadyPerformed			= NO;
	m_isStartupCompleteIndAlreadySentToRsrcAlloc	= NO;
	m_isFirstKaAfterCompleteAlreaySentToRsrcAlloc	= NO;
	m_isUnitLoadedIndReceived						= NO;
	m_isMediaIpConfigIndReceived					= NO;
	m_isRtmMediaIpConfigIndReceived					= NO;
    if (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator)
    {
        m_isRtmMediaIpConfigIndReceived = YES;
    }
	m_isMediaIpExistsForThisMfa						= NO;
	m_isIvrMusicSourceReqAlreaySentToMpm			= NO;

	m_mediaIpConfigStatus = eMediaIpConfig_IpFail;

	m_isEnabled = true; // MPM card is disabled on MPM+ (Pure) mode (and is not reported to Resource process)
    					// 07/2008: SRE requires that MPM+ will (also) be disabled if the system is in MPM mode

	m_boardId = (DWORD)this;
	m_subBoardId = (DWORD)this;

	for(WORD i = 0; i < MAX_NUM_OF_UNITS ; i++)
		m_unitsFips140StatusList[i] = STATUS_NO_ART;

	//  **checksum is out of scope (02/2009)**
	//  m_checksumStatus.unStatus = STATUS_OK;

	m_mediaIpConfigStruct = new MEDIA_IP_CONFIG_S;

	m_isJitcMode = NO;
	m_resetOnOldJitcInd = NO;
	m_isInstalling = NO;
	m_require_ipmc_upgrade = NO;
	m_isInstallingIpmc = NO;

	m_isLanActive = FALSE;

	m_ipConfigMsgACKCounter = 0;
	m_isMediaConfigurationCompletedIndReceived = NO;
	
	m_isFirstPortUp = NO;
	m_isSecondPortUp = NO;

	CreateTaskName();
}

CMfaTask::~CMfaTask()
{
	POBJDELETE(m_pIpMediaConfigVector);
	PDELETE(m_pUnitsTypesConfiguredStruct);
	PDELETE(m_mediaIpConfigStruct);
}

/////////////////////////////////////////////////////////////////////
void CMfaTask::AlarmableTaskCreateEndPoint()
{
	// base: set status to normal.

	// was added here since task_name is set only later (and task_status should be set to Normal
	//   only after task_name is set, in order to get an appropriate task_name in ActiveAlarms)
	// task_state is therefore set in method <CCardsManager::CreateMfaTask>,
	//   only after <SetBoardId> and <SetSubBoardId> are called (and task_name is set accordingly)
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::InitTask()
{
    m_state = STARTUP;
    eCardType cardType = m_cardMngr.GetType();

//===================================================================
// 06/2008: SRE requires that MPM card will be 'disabled' on MPM+ (PURE) mode
//	if ( true == m_pProcess->IsMpmCard(cardType) )

//===================================================================
// 07/2008: SRE requires that also MPM+ will be 'disabled' on MPM (MIX) mode
//		m_isEnabled = m_pProcess->IsCardEnabled_SystemCardsMode(cardType);

//===================================================================
// 29/07/2008: SRE requires that MPM will not be supported in V4.0 at all. MPM (MIX) mode is not acceptable
//	if ( true == m_pProcess->IsMpmCard(cardType) )
//	{
//		TreatMpmExistsOnSystem(cardType);
//	}

//===================================================================
// 09/2008: SRE requires that MPM will be supported in V4.0...

    m_isEnabled = m_pProcess->IsCardEnabled_ProductType(cardType);
    if (true == m_isEnabled)
    {
    	m_isEnabled = m_pProcess->IsCardEnabled_SystemCardsMode(cardType);
    }


    if (true == m_isEnabled)
    {
    	m_isEnabled = m_pProcess->IsCardEnabled_specRMX2000C(cardType); // should be removed on other versions!!!
    }

	if ( (true == m_isEnabled) && (true == m_pProcess->IsMediaCard(cardType)) )
	{
		m_pProcess->GetCardsMonitoringDB()->SetCardState(m_boardId, m_subBoardId, eCardStartup);

		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::InitTask"
		                       << "\nCard on boardId " << m_boardId << " subBoardId " << m_subBoardId
		                       << " (type: " << ::CardTypeToString(cardType) << ")"
		                       << " updated its state to Startup";

		SendDebugModeToMfa();
		SendSysCfgParamsToMfa();
		//SendSystemCardsModeToMfa(); //can be removed mfa do not use that msg
//		TreatIvrMountReadStatus(); //can be removed after Resource implements their improvement
		SendUnitConfigReqToRsrcalloc();

		StartTimer(MFA_STARTUP_TIMER, CARD_STARTUP_TIME_LIMIT);

		AskSwitchForActiveAlarms();
	}

	else
	{
		if (false == m_isEnabled)
		{
			// update 'disabled' state in HW_Monitoring
			m_pProcess->GetCardsMonitoringDB()->SetCardState(m_boardId, m_subBoardId, eDisabled);
		}

		string isEnabledStr  = ( (true == m_isEnabled) ? "yes" : "no" );
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::InitTask - illegal or disabled card"
                               << " (boardId " << m_boardId << " subBoardId " << m_subBoardId << ")"
                               << "\nCard Type:  " << ::CardTypeToString(cardType)
                               << "\nIs Enabled: " << isEnabledStr;
	}


	// remove an unnecessary Alert
	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
	if ( true == IsActiveAlarmExistByErrorCodeUserId(AA_POWER_OFF_PROBLEM, displayBoardId) )
	{
		RemoveActiveAlarmMfaByErrorCodeUserId("CMfaTask::InitTask", AA_POWER_OFF_PROBLEM, displayBoardId);
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::InitTask - 'old' PowerOff Alert should be removed (the card exists, after all)";
	}

}

void CMfaTask::CreateTaskName()
{
	char buff[256];
	sprintf(buff, "MfaTask (BoardId %d, SubBoardId %d)", m_boardId, m_subBoardId);
	m_TaskName = buff;
}

void CMfaTask::AskSwitchForActiveAlarms()
{
	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();
	if( eProductFamilySoftMcu == curProductFamily )
		return;

	DWORD switchBoardId    = (DWORD)(m_pProcess->GetAuthenticationStruct()->switchBoardId),
	      switchSubBoardId = (DWORD)(m_pProcess->GetAuthenticationStruct()->switchSubBoardId);

	COsQueue* switchQueue = m_pProcess->GetMfaMbx(switchBoardId, switchSubBoardId);
	if (switchQueue)
	{
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::AskSwitchForActiveAlarms - "
		                              << "Mfa (boardId " << m_boardId << ") asks its Alarms from Switch";

		// ===== 1. prepare the data to be sent
		CSegment* pSegToBoard = new CSegment;
		*pSegToBoard << m_boardId
		             << m_subBoardId;

		// ==== 2. send the msg
		CMfaApi mfaApi;
		mfaApi.CreateOnlyApi(*switchQueue);
		mfaApi.SendMsg(pSegToBoard, SPEC_MFA_ACTIVE_ALARMS_REQ);
	}

	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::AskSwitchForActiveAlarms - "
		                              << "Switch task does not exist! Mfa (boardId " << m_boardId << ") does nothing";
	}
}


//////////////////////////////////////////////////////////////////////
void CMfaTask::OnSpecMfaActiveAlarmInd(CSegment* pSeg)
{
	ACTIVE_ALARMS_SPECIFIC_CARD_S* pAaStruct = (ACTIVE_ALARMS_SPECIFIC_CARD_S*)pSeg->GetPtr();

	// ==== 2. print to log
	CLargeString AaStructStr;
	m_pProcess->ActiveAlarmsStructAsString(pAaStruct, AaStructStr, m_boardId);
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnSpecMfaActiveAlarmInd -"
	                              << AaStructStr.GetString();

	ProduceActiveAlarmsTransferredFromSwitch(pAaStruct);
}

///////////////////////////////////////////////////////////////////////
void CMfaTask::ProduceActiveAlarmsTransferredFromSwitch(ACTIVE_ALARMS_SPECIFIC_CARD_S* pAaStruct)
{
	BOOL doesAlarmExist = NO;
	BOOL doesAlarmFaultOnlyExist = NO;


	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
	DWORD displayBoardIdPrivate;

	if ( (YES == pAaStruct->isVoltageProblem) &&
	     (false == IsActiveAlarmExistByErrorCode(AA_VOLTAGE_PROBLEM)) )
	{
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::ProduceActiveAlarmsTransferredFromSwitch AA_VOLTAGE_PROBLEM "
			                              << "m_boardId " <<m_boardId<< "m_subBoardId "<<m_subBoardId<<" displayBoardId " <<displayBoardId;
		doesAlarmExist = YES;
		AddActiveAlarmMfa( "ProduceActiveAlarmsTransferredFromSwitch", FAULT_CARD_SUBJECT, AA_VOLTAGE_PROBLEM, MAJOR_ERROR_LEVEL,
                            "Card voltage problem", true, true, displayBoardId/*as 'userId' (token)*/, displayBoardId, 0, FAULT_TYPE_MPM );
	}

	if ( (YES == pAaStruct->isRtmIsdnMissingProblem) &&
		 (false == IsActiveAlarmExistByErrorCode(AA_RTM_LAN_OR_ISDN_MISSING)) )
	{
		doesAlarmExist = YES;
		displayBoardIdPrivate = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, RTM_ISDN_SUBBOARD_ID);
    AddActiveAlarmMfa("ProduceActiveAlarmsTransferredFromSwitch",
                      FAULT_CARD_SUBJECT,
                      AA_RTM_LAN_OR_ISDN_MISSING,
                      MAJOR_ERROR_LEVEL,
                      GetAlarmDescription(AA_RTM_LAN_OR_ISDN_MISSING),
                      true,
                      true,
                      displayBoardIdPrivate,
                      displayBoardIdPrivate,
                      0,
                      FAULT_TYPE_MPM);
	}

	if ( (YES == pAaStruct->isTemperatureMajorProblem) &&
	     (false == IsActiveAlarmExistByErrorCode(AA_TEMPERATURE_MAJOR_PROBLEM)) )
	{
		doesAlarmExist = YES;
		AddActiveAlarmMfa( "ProduceActiveAlarmsTransferredFromSwitch", FAULT_CARD_SUBJECT, AA_TEMPERATURE_MAJOR_PROBLEM, MAJOR_ERROR_LEVEL,
                            "Temperature has reached a problematic level and requires attention", true, true, displayBoardId/*as 'userId' (token)*/, displayBoardId, 0, FAULT_TYPE_MPM );
	}

	if ( (YES == pAaStruct->isTemperatureCriticalProblem) &&
	     (false == IsActiveAlarmExistByErrorCode(AA_TEMPERATURE_CRITICAL_PROBLEM)) )
	{
		doesAlarmExist = YES;
		AddActiveAlarmMfa( "ProduceActiveAlarmsTransferredFromSwitch", FAULT_CARD_SUBJECT, AA_TEMPERATURE_CRITICAL_PROBLEM, MAJOR_ERROR_LEVEL,
                            "Temperature has reached a critical level. MCU will shut down", true, true, displayBoardId/*as 'userId' (token)*/, displayBoardId, 0, FAULT_TYPE_MPM );
	}
	///////////////
	if ( (YES == pAaStruct->isPowerOffProblem) &&
	     (false == IsActiveAlarmExistByErrorCode(AA_POWER_OFF_PROBLEM)) )
	{
//		doesAlarmExist = YES;
//		AddActiveAlarmMfa( "ProduceActiveAlarmsTransferredFromSwitch", FAULT_CARD_SUBJECT, AA_POWER_OFF_PROBLEM, MAJOR_ERROR_LEVEL,
//		                            "Power off problem", true, true, displayBoardId/*as 'userId' (token)*/, displayBoardId, 0, FAULT_TYPE_MPM );
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::ProduceActiveAlarmsTransferredFromSwitch - 'old' PowerOff Alert is not produced (the card exists, after all)";
    }
    ///////////////
	if ( (YES == pAaStruct->isFailureProblem) &&
	     (false == IsActiveAlarmExistByErrorCode(AA_FAILURE_PROBLEM)) )
	{
		doesAlarmExist = YES;
		AddActiveAlarmMfa( "ProduceActiveAlarmsTransferredFromSwitch", FAULT_CARD_SUBJECT, AA_FAILURE_PROBLEM, MAJOR_ERROR_LEVEL,
                            "Unknown card error", true, true, displayBoardId/*as 'userId' (token)*/, displayBoardId, 0, FAULT_TYPE_MPM );
	}

	if ( (YES == pAaStruct->isOtherProblem) &&
	     (false == IsActiveAlarmFaultOnlyExistByErrorCodeUserId(AA_OTHER_PROBLEM, displayBoardId)) )
	{
		doesAlarmFaultOnlyExist = YES;

		AddActiveAlarmFaultOnlyMfa( "ProduceActiveAlarmsTransferredFromSwitch", FAULT_CARD_SUBJECT, AA_OTHER_PROBLEM, MAJOR_ERROR_LEVEL,
		                            "Unspecified card error", displayBoardId/*as 'userId' (token)*/, displayBoardId, 0, FAULT_TYPE_MPM );

	}

	if (YES == doesAlarmExist)
	{
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::ProduceActiveAlarmsTransferredFromSwitch doesAlarmExist";
	}


	if (YES == doesAlarmFaultOnlyExist)
	{
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::ProduceActiveAlarmsTransferredFromSwitch doesAlarmFaultOnlyExist";
	}
}

/////////////////////////////////////////////////////////////////////
void CMfaTask::OnTimerKeepAliveReceiveTimeout()
{
	CMedString dataStr = "CMfaTask::OnTimerKeepAliveReceiveTimeout, m_numOfkeepAliveTimeoutReached = ";

	// VNGR-21504: check if the card still exist
	DWORD status = 0;
	int result;
	BOOL bCardExist = TRUE;

	result = m_pProcess->GetSubBoardStatus(m_boardId, m_subBoardId, status);

	if (result == 0 && status == eSmComponentNotExist)
	{
		bCardExist = FALSE;
	}

	dataStr << "(result: " << result << " status: " << status
		<< " bCardExist: " << bCardExist << ") ";

	if ( CM_KEEP_ALIVE_RETRIES > m_numOfkeepAliveTimeoutReached && bCardExist )
	{
		m_numOfkeepAliveTimeoutReached++;
		// set the the number ok keep alive send without get back indication
		m_pProcess->GetCardsMonitoringDB()->SetUnAnswerdKA(m_boardId, m_numOfkeepAliveTimeoutReached);
		dataStr           << m_numOfkeepAliveTimeoutReached;
		TRACESTR(eLevelInfoNormal) << dataStr.GetString();

		m_pProcess->PrintLastKeepAliveTimes(m_boardId, m_lastKeepAliveReqTime, m_lastKeepAliveIndTime);

	}
	else
	{
		dataStr           << m_numOfkeepAliveTimeoutReached;
		TRACESTR(eLevelInfoNormal) << dataStr.GetString();
		if ( NO == m_isKeepAliveFailureAlreadyTreated )
		{
			m_isKeepAliveFailureAlreadyTreated = YES;
			if (!m_isInstalling && !m_isInstallingIpmc)
			{
				TreatMfaKeepAliveFailure();
				return;   //when we enter TreatMfaKeepAliveFailure() we do not want to keep sending keepAliveReq
			}
		}
	}

	// and anyway, keep sending KeepAlive requests
	OnTimerKeepAliveSendTimeout();
}
//////////////////////////////////////////////////////////////////////
void CMfaTask::OnMfaKeepAliveInd(CSegment* pSeg)
{
	SystemGetTime(m_lastKeepAliveIndTime);

	// ===== 1. print (if periodTrace==YES)
	BOOL isPeriodTrace = FALSE;
	m_pProcess->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_PERIOD_TRACE, isPeriodTrace);
	if (YES == isPeriodTrace)
	{
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnMfaKeepAliveInd - KeepAlive indication received "
		                              << "(boardId: " << m_boardId << ", subBoardId: " << m_subBoardId << ")";
	}

	// ===== 2. stop previous timer
	DeleteTimer(KEEP_ALIVE_TIMER_RECEIVE);
	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
	RemoveActiveAlarmMfaByErrorCodeUserId("OnMfaKeepAliveInd", AA_NO_CONNECTION_WITH_CARD, displayBoardId);
	m_numOfkeepAliveTimeoutReached = 0;

	// ===== 3. get data from segment received
	KEEP_ALIVE_S *tmpKA = (KEEP_ALIVE_S*)pSeg->GetPtr();

	// ===== 4. check results
	//  ---------- 3a. first KeepAlive
	if ( NO == m_isKeepAliveAlreadyReceived )
	{
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnMfaKeepAliveInd - first KeepAlive indication received";
		m_isKeepAliveAlreadyReceived = YES;
		CheckKeepAliveResults(tmpKA, eFirstKa);
		memcpy( &m_keepAliveStructure, tmpKA, sizeof(KEEP_ALIVE_S) );
	}

	//  ---------- 3b. not first KeepAlive, but changed from previous
	else if ( YES == IsKeepAliveChangedFromPrevious(tmpKA) )
	{
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnMfaKeepAliveInd - KeepAlive changed from previous";
		CheckKeepAliveResults(tmpKA, eChangedKa);
		memcpy( &m_keepAliveStructure, tmpKA, sizeof(KEEP_ALIVE_S) );
	}

	else
	{
		// ===== 5. a special case where KeepAlive should be sent to Resource process although not changed from Previous
		if ( (NO  == m_isFirstKaAfterCompleteAlreaySentToRsrcAlloc) &&
		     (YES == m_isStartupCompleteIndAlreadySentToRsrcAlloc) )
		{
			TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnMfaKeepAliveInd - send first KeepAlive to Resource process (after 'CompleteInd' was sent)";
			PrintKeepAliveResults(tmpKA, eFirstKaToRsrc);
			SendKeepAliveToRsrcAlloc(tmpKA);
			m_isFirstKaAfterCompleteAlreaySentToRsrcAlloc = YES;
		}
	}

	StartTimer(KEEP_ALIVE_TIMER_SEND, m_keepAliveSendPeriod*SECOND);
}
//////////////////////////////////////////////////////////////////////
void CMfaTask::OnMfaChecksumInd(CSegment* pSeg)
{
/*	**checksum is out of scope (02/2009)**
 *
 * CM_CHECKSUM_IND_S* pMfaChecksumStruct = (CM_CHECKSUM_IND_S*)pSeg->GetPtr();

	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnMfaChecksumInd - checksum status is : "
						   << (pMfaChecksumStruct->unStatus ? "FAIL" : "OK");

	m_checksumStatus = *pMfaChecksumStruct;

	HandleStartupOverIfNeeded("OnMfaChecksumInd");

*
* */
}
//////////////////////////////////////////////////////////////////////
void CMfaTask::OnArtFips140Ind(CSegment* pSeg)
{
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	DWORD unit = pMplMcmsProtocol->getPhysicalInfoHeaderUnit_id();

	TArtFips140Ind pArtStruct;
	memcpy( &pArtStruct, pMplMcmsProtocol->GetData(), sizeof(TArtFips140Ind) );

	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnArtFips140Ind - a fips 140 replay was received form unit id: "
						   << unit << " status - "
						   << ((pArtStruct.unStatus == E_FIPS_IND_STATUS_OK) ? "OK" : "FAIL"  );


	RSRCALLOC_ART_FIPS_140_IND_S* pArtFips140IndToRsrcAlloc = NULL;

	if( unit >= MAX_NUM_OF_UNITS )
	{
		TRACESTR(eLevelError) << "\nCMfaTask::OnArtFips140Ind - illegal unit id: " << unit;
	}
	else if(m_unitsFips140StatusList[unit] == STATUS_WAIT_FOR_REPLAY)
	{
		m_unitsFips140StatusList[unit] = (pArtStruct.unStatus == E_FIPS_IND_STATUS_OK) ? STATUS_OK : STATUS_FAIL ;

		/*
		 *  Don't need to send indication to resource,
		 *  ART will stop responding in case of failure
		 *  ===========================================
		//Forward msg to resource
		pArtFips140IndToRsrcAlloc = new RSRCALLOC_ART_FIPS_140_IND_S;
		pArtFips140IndToRsrcAlloc->physicalHeader.board_id     = m_boardId;
		pArtFips140IndToRsrcAlloc->physicalHeader.sub_board_id = m_subBoardId;
		pArtFips140IndToRsrcAlloc->physicalHeader.unit_id = unit;
		// to resource process we'll send only Ok or not
		pArtFips140IndToRsrcAlloc->status =  (pArtStruct.unStatus == E_FIPS_IND_STATUS_OK) ? STATUS_OK : STATUS_FAIL ;

		// ===== 1. fill a Segment
		CSegment*  pRetParam = new CSegment;
		pRetParam->Put( (BYTE*)pArtFips140IndToRsrcAlloc, sizeof(RSRCALLOC_ART_FIPS_140_IND_S) );

		delete pArtFips140IndToRsrcAlloc;
		POBJDELETE(pMplMcmsProtocol);

		// ===== 2. send msg to resource
		const COsQueue* pResourceMbx =
				CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResource, eManager);

		STATUS res = pResourceMbx->Send(pRetParam, ART_FIPS_140_IND);
		*/
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnArtFips140Ind - a fips 140 replay was received for a non-wait ART. ";
	}

	HandleStartupOverIfNeeded("OnArtFips140Ind");
	POBJDELETE(pMplMcmsProtocol);
}
//////////////////////////////////////////////////////////////////////
void CMfaTask::SendFips140ActiveAlarmIfNeeded()
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendFips140ActiveAlarmIfNeeded";

	//In case federal if OFF - all fips statuses are OK
	//First check checksum status - because it fails all card.

//  **checksum is out of scope (02/2009)**
//	if(m_checksumStatus.unStatus == STATUS_WAIT_FOR_REPLAY ||
//	   m_checksumStatus.unStatus == STATUS_FAIL)
//	{
//		SendAnActiveAlarmAboutChecksumFailure();
//	}
//	else //Second ARTs status
//	{
		for(WORD i = 0; i < MAX_NUM_OF_UNITS ; i++)
		{
			if(m_unitsFips140StatusList[i] == STATUS_WAIT_FOR_REPLAY ||
			   m_unitsFips140StatusList[i] == STATUS_FAIL)
			{
				SendAnActiveAlarmAboutAllFailedARTs();
				break;
			}
		}
//	}
}
//////////////////////////////////////////////////////////////////////
BOOL CMfaTask::AreAllFips140IndsReceived()
{
	for(WORD i = 0; i < MAX_NUM_OF_UNITS ; i++)
	{
		if(m_unitsFips140StatusList[i] == STATUS_WAIT_FOR_REPLAY)
		{
			return NO;
		}
	}
	//  **checksum is out of scope (02/2009)**
//	if(m_checksumStatus.unStatus == STATUS_WAIT_FOR_REPLAY)
//		return NO;

	return YES;
}
//////////////////////////////////////////////////////////////////////
void CMfaTask::CheckKeepAliveResults(KEEP_ALIVE_S *tmpKA, eMfaKeepAlivePhase kaPhase)
{
	PrintKeepAliveResults(tmpKA, kaPhase);

	if (false == m_isEnabled)
	{
		// KeepAlive results are not treated on disabled board, otherwise lots of unnneeded SystemAlerts will be produced
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::CheckKeepAliveResults"
									  << "\nThe card (boardId " << m_boardId << ", subBoardId " << m_subBoardId
									  << ") is disabled; KeepAlive results are not treated";
	}

	else // m_isEnabled = true
	{
	// ===== update RsrcAlloc process
	SendKeepAliveToRsrcAlloc(tmpKA);

	eCardUnitLoadedStatus curUnitNewStatus = eOk;
	DWORD faultId = 0;
  BOOL doesFlush = TRUE;

	for (int i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		curUnitNewStatus  = (eCardUnitLoadedStatus)(tmpKA->statusOfUnitsList[i]);

        if(26 == i) // 10.12.06: the DSP_26 is ignored, since a board with 25 units only is acceptable
        {
            continue;
        }

		else if ( (eOk == curUnitNewStatus) && (0 != i) ) // "(0 != i)" since we don't want that all CardMngr's ActiveAlarms will be removed according to regular KeepAlive ind
		{
			// ===== 1. Remove ActiveAlarm Fault Only (if exists)
			RemoveActiveAlarmMfaByErrorCodeUserId("CheckKeepAliveResults", AA_UNIT_NOT_RESPONDING, i, TRUE);
		}

		else if ( (eOk != curUnitNewStatus) && (eNotExist != curUnitNewStatus) )
		{
			TRACESTR(eLevelInfoNormal) << "\nCMfaTask::CheckKeepAliveResults curUnitNewStatus " << (DWORD)curUnitNewStatus;

			// ===== 2. if it's first KeepALive or a new failure, then update CardDb,
			// produce a Fault (ComponentFatal) etc.
			if ((eFirstKa == kaPhase) ||
			    ((eFirstKa != kaPhase) && IsNewFailure(i, curUnitNewStatus)))
			{
				// Fault only
				RemoveActiveAlarmMfaByErrorCodeUserId("CheckKeepAliveResults", AA_UNIT_NOT_RESPONDING, i, TRUE);
				doesFlush = FALSE;
				if (curUnitNewStatus == eFatal)
				{

					DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);

					CMedString fltStr, alertStr;
					alertStr << "Unit Failure";
					fltStr   << "Unit Failure - boardId: " << displayBoardId
							<< ", unitId: "               << i << "\n";



					AddActiveAlarmMfa( "CheckKeepAliveResults", FAULT_CARD_SUBJECT, AA_UNIT_NOT_RESPONDING, MAJOR_ERROR_LEVEL,
							alertStr.GetString(), true, true, displayBoardId, displayBoardId, 0, FAULT_TYPE_MPM );

				}
				else

					ProduceFaultAndLogger(eFailureTypeUnitFailure, i, doesFlush);
			}
		} // end if(curUnitNewStatus != ok)
	} // end loop over MAX_NUM_OF_UNITS

    if (!doesFlush)
    {
        FlushActiveAlarm();
        UpdateCardStateAccordingToTaskState("CheckKeepAliveResults");
    }
	} // end (m_isEnabled = true)
}

//////////////////////////////////////////////////////////////////////
BOOL CMfaTask::IsKeepAliveChangedFromPrevious(KEEP_ALIVE_S *tmpKA)
{
	DWORD isChanged = NO;

	for (int i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		if( tmpKA->statusOfUnitsList[i] !=  m_keepAliveStructure.statusOfUnitsList[i] )
		{
			isChanged = YES;
			break;
		}
	}

	return isChanged;
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::PrintKeepAliveResults(KEEP_ALIVE_S *KaStruct, eMfaKeepAlivePhase kaPhase)
{
	CLargeString keepAliveStr = "===== MFA KeepAlive";
	if (eFirstKa == kaPhase)
	{
		keepAliveStr << " - first KA received =====";
	}
	else if (eChangedKa == kaPhase)
	{
		keepAliveStr << " - KA Changed! =====";
	}
	else if (eFirstKaToRsrc == kaPhase)
	{
		keepAliveStr << " - first KA sent to Resource process =====";
	}
	keepAliveStr << " BoardId: " << m_boardId << ", SubBoardId: " << m_subBoardId;


	char* cardStateStr = ::CardStateToString(KaStruct->status);
	if (cardStateStr)
	{
		keepAliveStr << "\nGeneral Status: " << cardStateStr;
	}
	else
	{
		keepAliveStr << "\nGeneral Status: (invalid: " << KaStruct->status << ")";
	}

	keepAliveStr << "\nUnits' Statuses"
	             << "\n===============\n";
	for (int i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		char* cardUnitLoadedStatusStr = ::CardUnitLoadedStatusToString(KaStruct->statusOfUnitsList[i]);
		if (cardUnitLoadedStatusStr)
		{
			keepAliveStr << "Unit " << i << ": " << cardUnitLoadedStatusStr << "\n";
		}
		else
		{
			keepAliveStr << "Unit " << i << ": (invalid: " << KaStruct->statusOfUnitsList[i] << ")\n";
		}
	}

	TRACESTR(eLevelInfoNormal) << keepAliveStr.GetString();
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendKeepAliveToRsrcAlloc(KEEP_ALIVE_S *KaStruct)
{
	//  30.10.06: if MediaIpConfigInd was not received from MPL, then resources should not become enabled
	if ( (NO == m_isMediaIpConfigIndReceived) || (NO == m_isRtmMediaIpConfigIndReceived) )
	{
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendKeepAliveToRsrcAlloc - keepAlive is not being sent since MediaIp was not configured";
		return;
	}

	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendKeepAliveToRsrcAlloc";

	if (YES == m_isStartupCompleteIndAlreadySentToRsrcAlloc)
	{
		m_isFirstKaAfterCompleteAlreaySentToRsrcAlloc = YES;
	}

	RSRCALLOC_KEEP_ALIVE_S* pKeepAliveToRsrcAlloc = new RSRCALLOC_KEEP_ALIVE_S;
	memset(pKeepAliveToRsrcAlloc, 0, sizeof *pKeepAliveToRsrcAlloc);

	pKeepAliveToRsrcAlloc->physicalHeader.board_id     = m_boardId;
	pKeepAliveToRsrcAlloc->physicalHeader.sub_board_id = m_subBoardId;
	memcpy( &pKeepAliveToRsrcAlloc->keepAliveStruct, KaStruct, sizeof(KEEP_ALIVE_S) );

	// ===== 1. fill a Segment
	CSegment*  pRetParam = new CSegment;
	pRetParam->Put( (BYTE*)pKeepAliveToRsrcAlloc, sizeof(RSRCALLOC_KEEP_ALIVE_S) );

	delete pKeepAliveToRsrcAlloc;

	// ===== 2. send
	const COsQueue* pResourceMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResource, eManager);

	STATUS res = pResourceMbx->Send(pRetParam, CM_KEEP_ALIVE_IND);
}

/*
//////////////////////////////////////////////////////////////////////
void CMfaTask::RemoveAllActiveAlarms()
{
	for (int i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		RemoveActiveAlarm(i);
	}
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::RemoveActiveAlarm(int unitId)
{
	if ( eOk != (eCardUnitLoadedStatus)(m_pUnitsFailuresList->unitFailure[unitId].unitStatus) )
	// means: there was an ActiveAlarm for this unit
	{
		DWORD faultId = m_pUnitsFailuresList->unitFailure[unitId].faultId;
		if ( IsActiveAlarmExistById(faultId) ) // just an extra checking
		{
			m_pProcess->GetCardsMonitoringDB()->DelStatus(FAULT_CARD_SUBJECT, AA_UNIT_NOT_RESPONDING, MAJOR_ERROR_LEVEL, m_boardId);
			RemoveActiveAlarmById(faultId);
		}
	}
}
*/
//////////////////////////////////////////////////////////////////////
BOOL CMfaTask::IsNewFailure(int id, eCardUnitLoadedStatus unitStatus)
{
	BOOL isNew = YES;

	if ( m_keepAliveStructure.statusOfUnitsList[id] == (APIU32)unitStatus )
		isNew = NO;

	return isNew;
}

//////////////////////////////////////////////////////////////////////
// DWORD CMfaTask::ProduceFaultAndLogger(eFailureType failureType, int id, BOOL doesFlush/*=TRUE*/)
void CMfaTask::ProduceFaultAndLogger(eFailureType failureType, int id, BOOL doesFlush/*=TRUE*/)
{
	// ===== 1. prepare error description
	CMedString fltStr, alertStr;
	DWORD errCode=0; // , faultId=0;

	BOOL isFaultOnly = false;
	switch (failureType)
	{
		case eFailureTypeUnitFailure:
		{
			alertStr << "Unit Failure";
			fltStr   << "Unit Failure - boardId: " << m_boardId
		    	     << ", subBoardId: "           << m_subBoardId
		    	     << ", unitId: "               << id << "\n";

			errCode = AA_UNIT_NOT_RESPONDING;
			isFaultOnly = true;
			break;
		}

		case eFailureTypeCardNoConnection:
		{
			alertStr << "No connection with MPM card";
			fltStr   << "No connection with MPM; board Id: " << m_boardId << "\n";
			errCode = AA_NO_CONNECTION_WITH_CARD;
//			SetTaskStatus(eTaskMajor); // dont call it
			break;
		}

		default:
		{
			alertStr << "Card failure - Invalid failure type";
			fltStr   = "Card failure - illegal failure type!";
			errCode = AA_UNIT_NOT_RESPONDING;
			isFaultOnly = true;
			break;
		}
	} // end switch (failureType)


	// ===== 2. produce ActiveAlarm
	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);

	if (AA_NO_CONNECTION_WITH_CARD == errCode)
	{
		if (!isFaultOnly){
			 AddActiveAlarmMfa( "ProduceFaultAndLogger", FAULT_CARD_SUBJECT, errCode, MAJOR_ERROR_LEVEL,
										 alertStr.GetString(), true, true,
										 displayBoardId, // displayBoardId as 'userId' (token)
										 displayBoardId,
										 id,
										 FAULT_TYPE_MPM );
		}
		else {
			 AddActiveAlarmFaultOnlyMfa( "ProduceFaultAndLogger", FAULT_CARD_SUBJECT, errCode, MAJOR_ERROR_LEVEL,
										 alertStr.GetString(),
										 displayBoardId, // displayBoardId as 'userId' (token)
										 displayBoardId,
										 id,
										 FAULT_TYPE_MPM );

		}
	}
	else
	{
		DWORD unitType  = m_pUnitsTypesConfiguredStruct->unitsParamsList[id].type;

		WORD  faultType = FAULT_TYPE_ILLEGAL;
		if (eArt == unitType)
			faultType = FAULT_TYPE_ART;
		else if (eVideo == unitType)
			faultType = FAULT_TYPE_VIDEO;
		else if (eArtCntlr == unitType)
			faultType = FAULT_TYPE_ART_CNTRLR;

		if (!isFaultOnly){
			 AddActiveAlarmMfa( "ProduceFaultAndLogger", FAULT_UNIT_SUBJECT, errCode, MAJOR_ERROR_LEVEL,
										 alertStr.GetString(), true, true,
										 id, // unitId as 'userId' (token)
										 displayBoardId,
										 id,
										 faultType,
										 doesFlush);
		}
		else {
			 AddActiveAlarmFaultOnlyMfa( "ProduceFaultAndLogger", FAULT_UNIT_SUBJECT, errCode, MAJOR_ERROR_LEVEL,
										 alertStr.GetString(),
										 id, // unitId as 'userId' (token)
										 displayBoardId,
										 id,
										 faultType);



		}
	}


	// ===== 3. to Logger
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::ProduceFaultAndLogger - " << fltStr.GetString();

	return; // faultId;
}

void CMfaTask::TreatMfaKeepAliveFailure()
{
  // ===== 1. produce a Fault (NoConnectionWithCard)
  RemoveAllActiveAlarms("TreatMfaKeepAliveFailure");
  ProduceFaultAndLogger(eFailureTypeCardNoConnection, 0);

  m_pProcess->PrintLastKeepAliveTimes(m_boardId, m_lastKeepAliveReqTime,
                                      m_lastKeepAliveIndTime);

  // ===== 2. send message to RsrcAllocator
  if (true == m_isEnabled)
    SendKeepAliveFailureToRsrcAlloc(eNoConnection);

  // ===== 3. new treatment for Hot Swap.
  eCardType cardType = m_cardMngr.GetType();
  m_pProcess->RemoveCard(m_boardId, m_subBoardId, cardType);
  
  //BRIDGE-7441. MFA tasks are not cleaned. Many mfa tasks were for board 2 and subBoard 1 . Reason: TreatMfaKeepAliveFailure didnt   really clean MfaTask.

  SetSelfKill();



  // ===== 6. remove irrelevant Alert
  bool isMpmExists     = m_pProcess->IsMpmCardExistInDB(),
       isMpmPlusExists = m_pProcess->IsMpmPlusCardExistInDB(),
       isBreezeExists  = m_pProcess->IsBreezeCardExistInDB();

  eSystemCardsMode curMode = m_pProcess->GetSystemCardsModeCur();

  if (((eSystemCardsMode_mpm == curMode) &&
       (false == isMpmPlusExists || false == isBreezeExists)) ||
      ((eSystemCardsMode_mpm_plus == curMode) &&
       (false == isMpmExists || false == isBreezeExists)) ||
      ((eSystemCardsMode_breeze == curMode) &&
       (false == isMpmExists || false == isMpmPlusExists)))
  {
    RemoveActiveAlarmByErrorCodeUserId(AA_CARDS_CONFIG_EVENT,
                                       MPM_BLOCKING_ALERT_USER_ID);
    TRACEINTOFUNC << "AA_CARDS_CONFIG_EVENT was removed";
  }

  FlushActiveAlarm();
  UpdateCardStateAccordingToTaskState("TreatMfaKeepAliveFailure");
}

void CMfaTask::SendKeepAliveFailureToRsrcAlloc(eCardState cardState, OPCODE theOpcode)
{
	RSRCALLOC_KEEP_ALIVE_S* pKeepAliveToRsrcAlloc = new RSRCALLOC_KEEP_ALIVE_S;

	// ===== 1. fill the structure
	pKeepAliveToRsrcAlloc->physicalHeader.board_id     = m_boardId;
	pKeepAliveToRsrcAlloc->physicalHeader.sub_board_id = m_subBoardId;
	pKeepAliveToRsrcAlloc->keepAliveStruct.status = (DWORD)cardState;
	for (int i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		pKeepAliveToRsrcAlloc->keepAliveStruct.statusOfUnitsList[i] = (DWORD)eFatal;
	}

	// ===== 2. fill a Segment
	CSegment*  pRetParam = new CSegment;
	pRetParam->Put( (BYTE*)pKeepAliveToRsrcAlloc, sizeof(RSRCALLOC_KEEP_ALIVE_S) );

	delete pKeepAliveToRsrcAlloc;

	// ===== 3. send
	const COsQueue* pResourceMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResource, eManager);

	STATUS res = pResourceMbx->Send(pRetParam, theOpcode);
}

/////////////////////////////////////////////////////////////////////////////
void CMfaTask::SendResetMfaReqToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendResetMfaReqToMplApi";

	BYTE switchBoardId    = m_pProcess->GetAuthenticationStruct()->switchBoardId,
	     switchSubBoardId = m_pProcess->GetAuthenticationStruct()->switchSubBoardId;

	MFA_RESET_S *pMfaToReset = new MFA_RESET_S;
	pMfaToReset->resetMfa[m_boardId] = YES;

	SendMessageToMfa(switchBoardId, switchSubBoardId, CARDS_RESET_MFA_REQ, sizeof(MFA_RESET_S), (char*)pMfaToReset);
	delete pMfaToReset;
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SelfKill()
{
//  was put in commrnt due to leak problems
//	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SelfKill";

	// ===== 1. update entry in array
	COsQueue& rcvMbx = GetRcvMbx();
	m_pProcess->TurnMfaTaskToZombie(&rcvMbx);

	// ===== 2. destroy timer
	//DestroyTimer();

	// ===== 3. call father's SelfKill
	CTaskApp::SelfKill();
}

void CMfaTask::IgnoreMessage()
{
	// (nothing to be done)
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SetCardMngrStructData(const char *data)
{
	m_cardMngr.SetData(data);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SetCardMngrStructData(CCardMngrLoaded* other)
{
	m_cardMngr.SetData(other);
}

//////////////////////////////////////////////////////////////////////
CCardMngrLoaded CMfaTask::GetCardMngr()
{
	return m_cardMngr;
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SetBoardId(DWORD id)
{
	m_boardId = id;
	CreateTaskName();
}

//////////////////////////////////////////////////////////////////////
DWORD CMfaTask::GetBoardId()
{
	return m_boardId;
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SetSubBoardId(DWORD id)
{
	m_subBoardId = id;
	CreateTaskName();
}

//////////////////////////////////////////////////////////////////////
DWORD CMfaTask::GetSubBoardId()
{
	return m_subBoardId;
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SetIsEnabled(bool isEnabled)
{
	m_isEnabled = isEnabled;
}

//////////////////////////////////////////////////////////////////////
bool CMfaTask::GetIsEnabled()
{
	return m_isEnabled;
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SetUnitsTypesConfigured(CSegment* pSeg)
{
	// ===== 1. extract the info received from RsrcAllocator
	pSeg->Get( (BYTE*)m_pUnitsTypesConfiguredStruct, sizeof(CM_UNITS_CONFIG_S) );

 	// ===== 2. print the data to trace (using Dump function)
	PrintUnitTypesConfiguredDataFromRsrcAlloc();
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::PrintUnitTypesConfiguredDataFromRsrcAlloc()
{
	CManDefinedString *pStr = new CManDefinedString(MAX_NUM_OF_UNITS*ONE_LINE_BUFFER_LEN);

	*pStr << "\nCMfaTask::PrintUnitTypesConfiguredDataFromRsrcAlloc\n"
		  <<   "===================================================\n\n";

	int i=0;
	for (i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		char* cardUnitConfiguredTypeStr = ::CardUnitConfiguredTypeToString(m_pUnitsTypesConfiguredStruct->unitsParamsList[i].type);

		*pStr << "Unit " << i << ":   ";

		if (cardUnitConfiguredTypeStr)
		{
			*pStr << "Type: " << cardUnitConfiguredTypeStr << ",      ";
		}
		else
		{
			*pStr << "Type: (invalid: " << m_pUnitsTypesConfiguredStruct->unitsParamsList[i].type << "),      ";
		}

		*pStr << "PQ Number: " << m_pUnitsTypesConfiguredStruct->unitsParamsList[i].pqNumber << "\n";
	}


	TRACESTR(eLevelInfoNormal) << pStr->GetString();

	POBJDELETE(pStr);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SetUnitsTypesConfigured(int idx, APIU32 unitType)
{
	if ( (0 <= idx) && (MAX_NUM_OF_UNITS > idx) )
	{
		m_pUnitsTypesConfiguredStruct->unitsParamsList[idx].type = unitType;
	}
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::PrintUnitTypesConfiguredDataToMfa(CM_UNITS_CONFIG_S* pConfigParamsList)
{
	CManDefinedString *pStr = new CManDefinedString(MAX_NUM_OF_UNITS*ONE_LINE_BUFFER_LEN);

	*pStr << "\nCMfaTask::PrintUnitTypesConfiguredDataToMfa\n"
		  <<   "===========================================\n";

	int i=0;
	for (i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		char* cardUnitConfiguredTypeStr = ::CardUnitConfiguredTypeToString(pConfigParamsList->unitsParamsList[i].type);

		*pStr << "Unit " << i << ":   ";

		if (cardUnitConfiguredTypeStr)
		{
			*pStr << "Type: "<< cardUnitConfiguredTypeStr  << ",  ";
		}
		else
		{
			*pStr << "Type: (invalid: "<< pConfigParamsList->unitsParamsList[i].type  << "),  ";
		}


		*pStr << "PQ Number: " << pConfigParamsList->unitsParamsList[i].pqNumber << "\n";
	}

	TRACESTR(eLevelInfoNormal) << pStr->GetString();

	POBJDELETE(pStr);
}

//////////////////////////////////////////////////////////////////////
CM_UNITS_CONFIG_S* CMfaTask::GetUnitsTypesConfigured()
{
	return m_pUnitsTypesConfiguredStruct;
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SetUnitsLoadedStatuses(char* data)
{
	memcpy(&m_unitsLoadedStatusesStruct, data, sizeof(CM_UNIT_LOADED_S));
}

//////////////////////////////////////////////////////////////////////
CM_UNIT_LOADED_S CMfaTask::GetUnitsLoadedStatuses()
{
	return m_unitsLoadedStatusesStruct;
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::PrintUnitLoadedDataToTrace()
{
	CManDefinedString *pStr =
	            new CManDefinedString( ONE_LINE_BUFFER_LEN + (MAX_NUM_OF_UNITS*ONE_LINE_BUFFER_LEN) );

	*pStr << "\nCMfaTask::PrintUnitLoadedDataToTrace\n";

	char* cardUnitLoadedStatusStr = ::CardUnitLoadedStatusToString(m_unitsLoadedStatusesStruct.status);
	if (cardUnitLoadedStatusStr)
	{
		*pStr << "\nGeneral Status: " << cardUnitLoadedStatusStr << "\n";
	}
	else
	{
		*pStr << "\nGeneral Status: (invalid: " << m_unitsLoadedStatusesStruct.status << ")\n";
	}

	*pStr << "\nUnits' Statuses:\n";
	DWORD i=0, stat=0;
	for (i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		char* cardUnitLoadedStatusStr = ::CardUnitLoadedStatusToString(m_unitsLoadedStatusesStruct.statusOfUnitsList[i]);

		if (cardUnitLoadedStatusStr)
		{
			*pStr << "Unit " << i << ":  " << cardUnitLoadedStatusStr << "\n";
		}
		else
		{
			*pStr << "Unit " << i << ": (invalid: " << m_unitsLoadedStatusesStruct.statusOfUnitsList[i] << ")\n";
		}
	}

	TRACESTR(eLevelInfoNormal) << pStr->GetString();

	POBJDELETE(pStr);
}

////////////////////////////////////////////////////////////////////////////
void CMfaTask::SendDebugModeToMfa()
{
	CMedString logStr = "\nCCardsProcess::SendDebugModeToMfa - ";
	logStr << "boardId: " << m_boardId << ", subBoardId: " << m_subBoardId;

	// ===== 1. read & validate SysConfig param
	CSysConfig *sysConfig = m_pProcess->GetSysConfig();
	const CCfgData *cfgData = sysConfig->GetCfgEntryByKey(CFG_KEY_DEBUG_MODE);
	if( false == CCfgData::TestValidity(cfgData) )
	{
		logStr << "\nFailed to validate sysConfig, key: " << CFG_KEY_DEBUG_MODE;
		TRACESTR(eLevelInfoNormal) << logStr.GetString();
		return;
	}


	string dataStr;
	sysConfig->GetDataByKey(CFG_KEY_DEBUG_MODE, dataStr);

	BOOL isDebugMode = FALSE;
	BOOL res = sysConfig->GetBOOLDataByKey(CFG_KEY_DEBUG_MODE, isDebugMode);

	if ( (FALSE == res) ||
	     ((CFG_STR_YES != dataStr) && (CFG_STR_NO  != dataStr)) ) 	// illegal data!
	{
		logStr << "\nIllegal DebugMode flag: "
		       << "key - " << CFG_KEY_DEBUG_MODE << ", value: " << dataStr.c_str()
		       << ". Nothing is sent to MPL!";
	}

	else
	{
		// ===== 2. send to MPL
		logStr << "\nDebugMode flag: " << dataStr.c_str();
		DWORD fixedSubBoardId = FIRST_SUBBOARD_ID;
		DWORD opcode = 0;

		if (CFG_STR_YES == dataStr)
		{
			opcode = DEBUG_MODE_YES_REQ;
		}
		else
		{
			opcode = DEBUG_MODE_NO_REQ;
		}

		SendMessageToMfa(m_boardId, fixedSubBoardId, opcode, 0, NULL);

	} // end (legal data)

	TRACESTR(eLevelInfoNormal) << logStr.GetString();
}

bool CMfaTask::isMsIceConfigured() {
	std::string answer;
	std::string cmd;
	cmd = "cat " + MCU_MCMS_DIR
			+ "/Cfg/IPServiceList.xml | grep '<ICE_ENVIRONMENT>'";
	SystemPipedCommand(cmd.c_str(), answer);
	size_t found = string::npos;
	found = answer.find("iceEnvironment_ms");
	bool isMsIceCongifured = false;
	//iat least one service is different from 'numOfPcmInMpmx' - it means that ice ms is configured
	if (found != string::npos)
		isMsIceCongifured = true;

	return isMsIceCongifured;
}

void CMfaTask::SendSysCfgParamsToMfa()
{
	CMedString logStr = "\nCCardsProcess::SendSysCfgParamsToMfa - ";
	logStr << "boardId: " << m_boardId << ", subBoardId: " << m_subBoardId;

	// ===== 1. read & validate SysConfig param
	// 1.1. Read & Validate DEBUG_MODE
	BOOL isDebugMode = FALSE;
	string dataDebugModeStr;
	CSysConfig *sysConfig = m_pProcess->GetSysConfig();
	const CCfgData *cfgData = sysConfig->GetCfgEntryByKey(CFG_KEY_DEBUG_MODE);
	if( false == CCfgData::TestValidity(cfgData) )
	{
	    logStr << "\nFailed to validate sysConfig, key: " << CFG_KEY_DEBUG_MODE;
	    TRACESTR(eLevelError) << logStr.GetString();

	}
	else
	{

	    sysConfig->GetDataByKey(CFG_KEY_DEBUG_MODE, dataDebugModeStr);

	    BOOL res = sysConfig->GetBOOLDataByKey(CFG_KEY_DEBUG_MODE, isDebugMode);

	    if ( (FALSE == res) ||
	            ((CFG_STR_YES != dataDebugModeStr) && (CFG_STR_NO  != dataDebugModeStr)) ) 	// illegal data!
	    {
	        logStr << "\nIllegal DebugMode flag: "
	        << "key - " << CFG_KEY_DEBUG_MODE << ", value: " << dataDebugModeStr.c_str()
	        << ". default value is sent to MPL!";
	        TRACESTR(eLevelError) << logStr.GetString();

	        isDebugMode = FALSE;
	    }
	}


	// 1.1. Read & Validate JITC_MODE
	BOOL isJITCMode = FALSE;
    string dataJITCModeStr;
	cfgData = sysConfig->GetCfgEntryByKey(CFG_KEY_JITC_MODE);
	if( false == CCfgData::TestValidity(cfgData) )
	{
	    logStr << "\nFailed to validate sysConfig, key: " << CFG_KEY_JITC_MODE;
	    TRACESTR(eLevelError) << logStr.GetString();

	}
	else
	{

	    sysConfig->GetDataByKey(CFG_KEY_JITC_MODE, dataJITCModeStr);


	    BOOL res = sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, isJITCMode);

	    if ( (FALSE == res) ||
	            ((CFG_STR_YES != dataJITCModeStr) && (CFG_STR_NO  != dataJITCModeStr)) ) 	// illegal data!
	    {
	        logStr << "\nIllegal JITCMode flag: "
	        << "key - " << CFG_KEY_JITC_MODE << ", value: " << dataJITCModeStr.c_str()
	        << ".  default value is sent to MPL!";
	        TRACESTR(eLevelError) << logStr.GetString();

	        isJITCMode = FALSE;
	    }
	}

	m_isJitcMode = isJITCMode;
	m_resetOnOldJitcInd = YES;

	string dataSeparatedNetworksStr;
	sysConfig->GetDataByKey(CFG_KEY_SEPARATE_NETWORK, dataSeparatedNetworksStr);

	BOOL isSeparatedNetworks = NO;
	BOOL res = sysConfig->GetBOOLDataByKey(CFG_KEY_SEPARATE_NETWORK, isSeparatedNetworks);

	if ( (FALSE == res) ||
	        ((CFG_STR_YES != dataSeparatedNetworksStr) && (CFG_STR_NO  != dataSeparatedNetworksStr)) ) 	// illegal data!
	{
	    logStr << "\nIllegal SeparatedNetworks flag: "
	    << "key - " << CFG_KEY_SEPARATE_NETWORK << ", value: " << dataSeparatedNetworksStr.c_str()
	    << ". default value is sent to MPL!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    isSeparatedNetworks = NO;
	}

	string dataReportDSPCrashStr;
	sysConfig->GetDataByKey(CFG_KEY_REPORT_DSP_CRASH, dataReportDSPCrashStr);

	BOOL bReportDSPCrash = NO;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_REPORT_DSP_CRASH, bReportDSPCrash);

	if ( (FALSE == res) ||
	        ((CFG_STR_YES != dataReportDSPCrashStr) && (CFG_STR_NO  != dataReportDSPCrashStr)) ) 	// illegal data!
	{
	    logStr << "\nIllegal Report DSP Crash flag: "
	    << "key - " << CFG_KEY_REPORT_DSP_CRASH << ", value: " << dataReportDSPCrashStr.c_str()
	    << ". default value  is sent to MPL!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    bReportDSPCrash = NO;
	}

	string pcmLanguageStr;
	sysConfig->GetDataByKey("PCM_LANGUAGE", pcmLanguageStr);

	int pcmLanguageVal;
	CStringsMaps::GetValue(PCM_LANGUAGE_ENUM,pcmLanguageVal,(char*)pcmLanguageStr.c_str());

	BOOL isMultipleServicesCfgFlag = NO, isMultipleServicesMode = NO;
	sysConfig->GetBOOLDataByKey(CFG_KEY_MULTIPLE_SERVICES, isMultipleServicesCfgFlag);
	if (m_pProcess->GetLicensingStruct()->multipleServices && isMultipleServicesCfgFlag)
	{
	    isMultipleServicesMode = YES;
	    isSeparatedNetworks = NO;
	}

	string dataRTMLanStr;
	BOOL bRTM_LAN = NO;

	//NGB request
	if (m_cardMngr.GetType() == eMpmRx_Half || m_cardMngr.GetType() == eMpmRx_Full )
	{
		dataRTMLanStr = CFG_STR_YES;
		bRTM_LAN = YES;
	}
        else
        {
         sysConfig->GetDataByKey(CFG_KEY_RMX2000_RTM_LAN, dataRTMLanStr);
         res = sysConfig->GetBOOLDataByKey(CFG_KEY_RMX2000_RTM_LAN, bRTM_LAN);

	 if ( (FALSE == res) ||
	        ((CFG_STR_YES != dataRTMLanStr) && (CFG_STR_NO  != dataRTMLanStr)) ) 	// illegal data!
	 {
	    logStr << "\nIllegal RMX2000_RTM_LAN flag: "
	    << "key - " << CFG_KEY_RMX2000_RTM_LAN << ", value: " << dataRTMLanStr.c_str()
	    << ". default value  is sent to MPL!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    bRTM_LAN = NO;
	 }
        }

	std::string dataLanRedundancyStr;
	sysConfig->GetDataByKey(CFG_KEY_LAN_REDUNDANCY, dataLanRedundancyStr);

	BOOL bLAN_REDUNDANCY = NO;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_LAN_REDUNDANCY, bLAN_REDUNDANCY);

	if ((FALSE == res) ||
	        ((CFG_STR_YES != dataLanRedundancyStr) && (CFG_STR_NO  != dataLanRedundancyStr)))
	{
	    TRACEINTO << "\nIllegal CFG_KEY_LAN_REDUNDANCY flag: "
	    << "key - " << CFG_KEY_LAN_REDUNDANCY
	    << ", value: " << dataLanRedundancyStr.c_str()
	    << ". default value is sent to MPL!";

	    bLAN_REDUNDANCY = NO;

	}

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if ((eProductTypeGesher == curProductType) || (eProductTypeNinja == curProductType))
	{
		bLAN_REDUNDANCY = CProcessBase::GetProcess()->GetLanRedundancy();
		if (bLAN_REDUNDANCY == YES)
		{
			dataLanRedundancyStr = "YES";
		}
		else
		{
			dataLanRedundancyStr = "NO";
		}
	}

	string dataV35JitcSupportStr;
	sysConfig->GetDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, dataV35JitcSupportStr);

	BOOL isV35JitcSupport = NO;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, isV35JitcSupport);

	if ( (FALSE == res) ||
	        ((CFG_STR_YES != dataV35JitcSupportStr) && (CFG_STR_NO  != dataV35JitcSupportStr)) ) 	// illegal data!
	{
	    logStr << "\nIllegal V35_ULTRA_SECURED_SUPPORT flag: "
	    << "key - " << CFG_KEY_V35_ULTRA_SECURED_SUPPORT << ", value: " << dataV35JitcSupportStr.c_str()
	    << ". default value is sent to MPL!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    isV35JitcSupport = NO;
	}

	// Enable TC package
	string dataEnableTcPackageStr;
	sysConfig->GetDataByKey(CFG_KEY_ENABLE_TC_PACKAGE, dataEnableTcPackageStr);

	BOOL isEnableTcPackage = NO;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_TC_PACKAGE, isEnableTcPackage);
	if ( (FALSE == res) ||
	        ((CFG_STR_YES != dataEnableTcPackageStr) && (CFG_STR_NO  != dataEnableTcPackageStr)) ) 	// illegal data!
	{
	    logStr << "\nIllegal nableTcPackage flag: "
	    << "key - " << CFG_KEY_ENABLE_TC_PACKAGE << ", value: " << dataEnableTcPackageStr.c_str()
	    << ". default value is sent to MPL!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    isEnableTcPackage = NO ;
	}

	//TC (Traffic Control) latency size
	DWORD dataTcLatencySize;
	res = sysConfig->GetDWORDDataByKey(CFG_KEY_TC_LATENCY_SIZE, dataTcLatencySize);

	if ( (FALSE == res) ||
			(1 > dataTcLatencySize) || (1000  <  dataTcLatencySize) ) 	// illegal data!
	{
	    logStr << "\nIllegal dataTcLatencySize flag: "
	    << "key - " << CFG_KEY_TC_LATENCY_SIZE << ", value: " << dataTcLatencySize
	    << ". default value is sent to MPL!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    dataTcLatencySize = 500 ; //set the default value

	}


	//Tc (Traffic Control) Burst Size
	DWORD datTcBurstSize;
	res = sysConfig->GetDWORDDataByKey(CFG_KEY_TC_BURST_SIZE, datTcBurstSize);

	if ((FALSE == res) || (30  <  datTcBurstSize)) 	// illegal data!
	{
	    logStr << "\nIllegal datTcBurstSize flag: "
	    << "key - " << CFG_KEY_TC_BURST_SIZE << ", value: " << datTcBurstSize
	    << ". default value is sent to MPL!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    datTcBurstSize = 10; //set the default value
	}

	// For Ntp synchronization  VNGR-22433
	DWORD dataNtpLocalServerStratum;

	res = sysConfig->GetDWORDDataByKey(CFG_KEY_NTP_LOCAL_SERVER_STRATUM, dataNtpLocalServerStratum);

	if ( (FALSE == res) ||
	        (1 > dataNtpLocalServerStratum) || (15  <  dataNtpLocalServerStratum) ) 	// illegal data!
	{
	    logStr << "\nIllegal dataTcLatencySize flag: "
	    << "key - " << CFG_KEY_NTP_LOCAL_SERVER_STRATUM << ", value: " << dataNtpLocalServerStratum
	    << ". default value is sent to MPL!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    dataNtpLocalServerStratum = 14;  //set the default value
	}

	string dataMonitoringPacketStr;
	sysConfig->GetDataByKey(CFG_KEY_MONITORING_PACKET, dataMonitoringPacketStr);

	BOOL isMonitoringPacket = YES;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_MONITORING_PACKET, isMonitoringPacket);
	if ( (FALSE == res) ||
	        ((CFG_STR_YES != dataMonitoringPacketStr) && (CFG_STR_NO  != dataMonitoringPacketStr)) )  // illegal data!
	{
	    logStr << "\nIllegal MonitoringPacket flag: "
	    << "key - " << CFG_KEY_MONITORING_PACKET << ", value: " << dataMonitoringPacketStr.c_str()
	    << ". default value is sent to ShelfMngr!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    isMonitoringPacket = YES;
	}


	string dataIpResponseEchoStr;
	sysConfig->GetDataByKey(CFG_KEY_IP_RESPONSE_ECHO, dataIpResponseEchoStr);

	BOOL response_echo = YES;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_IP_RESPONSE_ECHO, response_echo);
	if ( (FALSE == res) ||
	        ((CFG_STR_YES != dataIpResponseEchoStr) && (CFG_STR_NO  != dataIpResponseEchoStr)) )  // illegal data!
	{
	    logStr << "\nIllegal IpResponseEcho flag: "
	    << "key - " << CFG_KEY_IP_RESPONSE_ECHO << ", value: " << dataIpResponseEchoStr.c_str()
	    << ". default value is sent to MPL!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    response_echo = YES;
	}



	string dataCheckArpingStr;
	sysConfig->GetDataByKey(CFG_KEY_DUPLICATE_IP_DETECTION, dataCheckArpingStr);

	BOOL check_arping = YES;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_DUPLICATE_IP_DETECTION, check_arping);
	if ( (FALSE == res) ||
	        ((CFG_STR_YES != dataCheckArpingStr) && (CFG_STR_NO  != dataCheckArpingStr)) )  // illegal data!
	{
	    logStr << "\nIllegal CheckArping flag: "
	    << "key - " << CFG_KEY_DUPLICATE_IP_DETECTION << ", value: " << dataCheckArpingStr.c_str()
	    << ". default value is sent to MPL!";
	    TRACESTR(eLevelError) << logStr.GetString();
	    check_arping = YES;
	}

	//RTM_LAN MTU SIZE
	DWORD rtmLanMtuSize;
	res = sysConfig->GetDWORDDataByKey(CFG_KEY_MEDIA_NIC_MTU_SIZE, rtmLanMtuSize);

	if ( (FALSE == res) ||
	        (500 > rtmLanMtuSize) || (20000  <  rtmLanMtuSize) )   // illegal data!
	{
	    logStr << "\nIllegal rtmLanMtuSize flag: "
	    << "key - " << CFG_KEY_MEDIA_NIC_MTU_SIZE<< ", value: " << rtmLanMtuSize
	    << ". default value is sent to MPL!";
	    TRACESTR(eLevelError) << logStr.GetString();

	    rtmLanMtuSize = 1500 ; //set the default value

	}


	eSystemCardsMode systemCardsMode = m_pProcess->GetSystemCardsModeCur();


	BOOL disableCellNQ;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_DISABLE_CELLS_NETWORK_IND, disableCellNQ);
	PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetBOOLDataByKey: " << CFG_KEY_DISABLE_CELLS_NETWORK_IND);

	DWORD percentMajorNQ;
	res = sysConfig->GetDWORDDataByKey(CFG_KEY_NETWORK_IND_MAJOR_PERCENTAGE, percentMajorNQ);
	PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetDWORDDataByKey: " << CFG_KEY_NETWORK_IND_MAJOR_PERCENTAGE);
	PASSERTSTREAM_AND_RETURN((percentMajorNQ < 1 || percentMajorNQ > 100), "Illegal value for " CFG_KEY_NETWORK_IND_MAJOR_PERCENTAGE);

	DWORD percentCriticalNQ;
	res = sysConfig->GetDWORDDataByKey(CFG_KEY_NETWORK_IND_CRITICAL_PERCENTAGE, percentCriticalNQ);
	PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetDWORDDataByKey: " << CFG_KEY_NETWORK_IND_CRITICAL_PERCENTAGE);
	PASSERTSTREAM_AND_RETURN((percentCriticalNQ <= percentMajorNQ || percentCriticalNQ > 100), "Illegal value for " CFG_KEY_NETWORK_IND_CRITICAL_PERCENTAGE);

	DWORD numOfParticipants;
	res = sysConfig->GetDWORDDataByKey(CFG_KEY_NUM_OF_PARTICIPANTS_TO_CHANGE_MRMP_LOG_LEVEL, numOfParticipants);

	if ( (FALSE == res) || (45  <  numOfParticipants) )   // illegal data!
	{
	    logStr << "\nIllegal rtmLanMtuSize flag: "
	    << "key - " << CFG_KEY_NUM_OF_PARTICIPANTS_TO_CHANGE_MRMP_LOG_LEVEL<< ", value: " << numOfParticipants;

	    TRACESTR(eLevelError) << logStr.GetString();

	    numOfParticipants = 30 ; //set the default value

	}

	BOOL enableSendingIcmpDestinationUnreachable;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_SENDING_ICMP_DESTINATION_UNREACHABLE, enableSendingIcmpDestinationUnreachable);
	PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetBOOLDataByKey: " << CFG_KEY_ENABLE_SENDING_ICMP_DESTINATION_UNREACHABLE);

	BOOL enableAcceptingIcmpRedirect;
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_ACCEPTING_ICMP_REDIRECT, enableAcceptingIcmpRedirect);
	PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetBOOLDataByKey: " << CFG_KEY_ENABLE_ACCEPTING_ICMP_REDIRECT);


	DWORD numOfPcmInMpmx;
	res = sysConfig->GetDWORDDataByKey(CFG_KEY_NUM_OF_PCM_IN_MPMX, numOfPcmInMpmx);

	if ( (FALSE == res) || (numOfPcmInMpmx !=0 && numOfPcmInMpmx !=1 &&  numOfPcmInMpmx !=4))   // illegal data!
	{
		logStr << "\nIllegal numOfPcmInMpmx flag: "
				<< "key - " << CFG_KEY_NUM_OF_PCM_IN_MPMX<< ", value: " << numOfPcmInMpmx;

		TRACESTR(eLevelError) << logStr.GetString();

		numOfPcmInMpmx = 1 ; //set the default value

	}

	BOOL isPreferSvcOverLyncCapacity = YES; // PREFER_SVC_OVER_LYNC_CAPACITY
	res = sysConfig->GetBOOLDataByKey(CFG_KEY_PREFER_SVC_OVER_LYNC_CAPACITY, isPreferSvcOverLyncCapacity);
	PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetBOOLDataByKey: " << CFG_KEY_PREFER_SVC_OVER_LYNC_CAPACITY);


	bool isMsIceCongifured = isMsIceConfigured();
	//If msIce configured and default avue was not changed - set value to 0
	if(isMsIceCongifured == true && numOfPcmInMpmx == 1)
		numOfPcmInMpmx = 0;

	// ===== 2. send to MPL
	if(isJITCMode && isSeparatedNetworks && isV35JitcSupport && isMultipleServicesMode)
		isMultipleServicesMode = FALSE;
	//BRIDGE-5887
	if(isJITCMode && isV35JitcSupport)
	{
		dataLanRedundancyStr = "NO";
		bLAN_REDUNDANCY = NO;
	}

	// mfa thresholds
	unsigned int threshold =0;
	res = sysConfig->GetDWORDDataByKey(CFG_KEY_MFA_SYSTEM_LOAD_AVERAGE_THRESHOLD, threshold);
	PASSERTSTREAM(!res, "CSysConfig::GetDWORDDataByKey: " << CFG_KEY_MFA_SYSTEM_LOAD_AVERAGE_THRESHOLD);

	unsigned int cpu_intervals =0;
	res = sysConfig->GetDWORDDataByKey(CFG_KEY_CM_HIGH_CPU_USAGE_INTERVAL_IN_MINUTES, cpu_intervals);
	PASSERTSTREAM(!res, "CSysConfig::GetDWORDDataByKey: " << CFG_KEY_MFA_SYSTEM_LOAD_AVERAGE_THRESHOLD);

	logStr << "\nDebugMode flag: " << dataDebugModeStr.c_str();
	logStr << "\nJITCMode flag:  " << dataJITCModeStr.c_str();
	logStr << "\nSeparated Networks flag: " << dataSeparatedNetworksStr.c_str();
	logStr << "\npcm language:   " << pcmLanguageStr.c_str() << " val: " << pcmLanguageVal;
	logStr << "\nReport DSP crash flag: " << dataReportDSPCrashStr.c_str();
	logStr << "\nMultiple services mode: " << (YES ==  isMultipleServicesMode? "YES" : "NO");
	logStr << "\nV35 JITC support mode: " << (YES ==  isV35JitcSupport? "YES" : "NO");
	logStr << "\nRMX200 RTM LAN flag: " << dataRTMLanStr.c_str();
	logStr << "\nRMX200 LAN REDUNDANCY flag: " << dataLanRedundancyStr.c_str();
	logStr << "\nEnable Traffic Control package " << dataEnableTcPackageStr.c_str();
	logStr << "\nTraffic Control latency size " << dataTcLatencySize;
	logStr << "\nTraffic Control Burst Size " << datTcBurstSize;
	logStr << "\nMonitoring Packet: " << (isMonitoringPacket ? "YES" : "NO");
	logStr << "\n" CFG_KEY_IP_RESPONSE_ECHO ": " << (response_echo ? "YES" : "NO");
	logStr << "\n" CFG_KEY_NTP_LOCAL_SERVER_STRATUM ": " << dataNtpLocalServerStratum;
	logStr << "\nSystem Cards Mode: " << systemCardsMode;
	logStr << "\n" CFG_KEY_DISABLE_CELLS_NETWORK_IND ": " << (disableCellNQ ? "YES" : "NO");
	logStr << "\n" CFG_KEY_NETWORK_IND_MAJOR_PERCENTAGE ": " << percentMajorNQ;
	logStr << "\n" CFG_KEY_NETWORK_IND_CRITICAL_PERCENTAGE ": " << percentCriticalNQ;
	logStr << "\n" << CFG_KEY_DUPLICATE_IP_DETECTION << ": " << (check_arping ? "YES" : "NO");
	logStr << "\nRTM_LAN mtu size " << rtmLanMtuSize;
	logStr << "\nNum of Participants " << numOfParticipants;
	logStr << "\n" << CFG_KEY_ENABLE_SENDING_ICMP_DESTINATION_UNREACHABLE << ": " << (enableSendingIcmpDestinationUnreachable ? "YES" : "NO");
	logStr << "\n" << CFG_KEY_ENABLE_ACCEPTING_ICMP_REDIRECT << ": " << (enableAcceptingIcmpRedirect ? "YES" : "NO");
	logStr << "\n" << CFG_KEY_MFA_SYSTEM_LOAD_AVERAGE_THRESHOLD << ": " << threshold;
	logStr << "\n" << CFG_KEY_CM_HIGH_CPU_USAGE_INTERVAL_IN_MINUTES << ": " << cpu_intervals;
	logStr << "\nnumOfPcmInMpmx flag: " << numOfPcmInMpmx;
	logStr << "\n"<< CFG_KEY_PREFER_SVC_OVER_LYNC_CAPACITY << ": " << (isPreferSvcOverLyncCapacity ? "YES" : "NO");

	SYSCFG_PARAMS_S SysCfgParamsStruct;
	memset(&SysCfgParamsStruct, 0, sizeof SysCfgParamsStruct);

	SysCfgParamsStruct.unDebugMode = isDebugMode;
	SysCfgParamsStruct.unJITCMode = isJITCMode;
	SysCfgParamsStruct.unSeparatedNetworks = isSeparatedNetworks;
	SysCfgParamsStruct.unPcmLanguage = pcmLanguageVal;
	SysCfgParamsStruct.unReportDspCrash = bReportDSPCrash;
	SysCfgParamsStruct.unMultipleServices = isMultipleServicesMode;
	SysCfgParamsStruct.unV35Mode = isV35JitcSupport;
	SysCfgParamsStruct.unRtmLan = bRTM_LAN;
	SysCfgParamsStruct.unLanRedundancy = bLAN_REDUNDANCY;
	SysCfgParamsStruct.unEnableTcPackage = isEnableTcPackage;
	SysCfgParamsStruct.unTcLatencySize = dataTcLatencySize;
	SysCfgParamsStruct.unTcBurstSize = datTcBurstSize;
	SysCfgParamsStruct.unMonitoringPacket = isMonitoringPacket;
	SysCfgParamsStruct.unNtpLocalServerStratum = dataNtpLocalServerStratum;
	SysCfgParamsStruct.unPacketLossMajorValue = percentMajorNQ;
	SysCfgParamsStruct.unPacketLossCriticalValue = percentCriticalNQ;
	//SysCfgParamsStruct.fEnablePacketLossIndication = !(disableSelfNQ && disableCellNQ) && (systemCardsMode >= eSystemCardsMode_breeze);
	SysCfgParamsStruct.fEnablePacketLossIndication = YES;
	SysCfgParamsStruct.unIsIpV4ResposeEcho = response_echo;
	SysCfgParamsStruct.unCheckArping = check_arping;
	SysCfgParamsStruct.unRtmLanMtuSize = rtmLanMtuSize;
	SysCfgParamsStruct.unMaxNumOfParticipantsToChangeLogLevel = numOfParticipants;
	SysCfgParamsStruct.unEnableAcceptingIcmpRedirect = enableAcceptingIcmpRedirect;
	SysCfgParamsStruct.unEnableSendingIcmpDestinationUnreachable = enableSendingIcmpDestinationUnreachable;
	SysCfgParamsStruct.unNumOfPcmInMpmx = numOfPcmInMpmx;
	SysCfgParamsStruct.unIsPreferSvcOverLyncCapacity = isPreferSvcOverLyncCapacity;
	// bit wise operation to send the THRESHOLD and intervals params on the same value
	cpu_intervals = cpu_intervals << 16;

	SysCfgParamsStruct.unMfaThreshold = cpu_intervals | threshold ;

	SendMessageToMfa(m_boardId,
                   FIRST_SUBBOARD_ID,
                   SYSCFG_PARAMS_REQ,
                   sizeof(SYSCFG_PARAMS_S),
                   (char*)&SysCfgParamsStruct);

	TRACEINTOFUNC << logStr.GetString();
}

void CMfaTask::OnNewSysCfgParamsInd(CSegment* pSeg)
{
	SYSCFG_PARAMS_S NewSysCfgParamsStruct;
	memset(&NewSysCfgParamsStruct, 0, sizeof NewSysCfgParamsStruct);

	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnNewSysCfgParamsInd; boardId: "<< m_boardId;

	// ===== 1. get the parameters from the structure received into process's attribute
	WORD boardId=0, subBoardId=0;
	*pSeg >> boardId
		  >> subBoardId
		  >> NewSysCfgParamsStruct.unDebugMode
		  >> NewSysCfgParamsStruct.unJITCMode
		  >> NewSysCfgParamsStruct.unSeparatedNetworks
		  >> NewSysCfgParamsStruct.unMultipleServices;


	// ===== 2. if it's a service from the suitable boardID and subBoardId...
	if ( (boardId == m_boardId) && (subBoardId == m_subBoardId) )
	{
		m_resetOnOldJitcInd = NO;
		m_isJitcMode = NewSysCfgParamsStruct.unJITCMode;
		SendMessageToMfa(m_boardId, m_subBoardId, SYSCFG_PARAMS_REQ, sizeof(SYSCFG_PARAMS_S), (char*)&NewSysCfgParamsStruct);
	}
}

/////////////////////////////////////////////////////////////////////
void CMfaTask::OnOldSysCfgParamsInd(CSegment* pSeg)
{
	// ===== 1. fill object with data from structure received
	DWORD  structLen = sizeof(SYSCFG_PARAMS_S);
	SYSCFG_PARAMS_S oldSysCfgParamsStruct;
	memset(&oldSysCfgParamsStruct,0,structLen);
	pSeg->Get( (BYTE*)(&oldSysCfgParamsStruct), structLen );

	// ===== 2. If value is different from the one sent to it - reset the card.
	// Do not reset card - if indication is response on SysCfgParams from updated System.cfg
	if(m_resetOnOldJitcInd)
	{
		if(oldSysCfgParamsStruct.unJITCMode != m_isJitcMode)
		{
			if(SYSCFG_PARAM_VAL_ERROR_READ == oldSysCfgParamsStruct.unJITCMode || SYSCFG_PARAM_VAL_ERROR_WRITE == oldSysCfgParamsStruct.unJITCMode)
			{
				TRACESTR(eLevelError) << "CMfaTask::OnOldSysCfgParamsInd - Mfa encountered an error while accessing flash!!!";
				std::string desc = (const char*)oldSysCfgParamsStruct.sDescription;
				if(m_isJitcMode)
					desc += " - Card is not in JITC mode !!!";
				AddActiveAlarmMfa( "OnOldSysCfgParamsInd", FAULT_CARD_SUBJECT, AA_CARDS_FLASH_ACCESS_PROBLEM, MAJOR_ERROR_LEVEL,
									desc, true, true,
									m_boardId, // boardId as 'userId' (token)
									m_boardId,
									0,
									FAULT_TYPE_MPM );
			}
			else
			{
				TRACESTR(eLevelError) << "CMfaTask::OnOldSysCfgParamsInd - Mfa held different JITC_MODE value, resetting card!!!";
				//reset card
				SendMessageToMfa(m_boardId, m_subBoardId, RESET_CARD_REQ, 0, NULL);
			}
		}
		else
			TRACESTR(eLevelError) << "CMfaTask::OnOldSysCfgParamsInd - Mfa held same JITC_MODE value, no need to reset card.";
	}
}

/////////////////////////////////////////////////////////////////////
void CMfaTask::OnLicenseInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnLicenseInd";
}
//////////////////////////////////////////////////////////////////////
void CMfaTask::SendMessageToMfa(DWORD boardId, DWORD subBoardId, DWORD opcode, size_t size, char* pStruct)
{
	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();
	mplPrtcl->AddCommonHeader(opcode);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, boardId, subBoardId);

	if(size > 0 && pStruct)
		mplPrtcl->AddData( size, pStruct );

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendSystemCardsModeToMfa()
{
	DWORD curMode = (DWORD)( m_pProcess->GetSystemCardsModeCur() );

	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendSystemCardsModeToMfa"
						   << " (boardId: " << m_boardId << ", subBoardId: " << m_subBoardId << ")"
                           << "\nSystem cards mode: " << ::GetSystemCardsModeStr( (eSystemCardsMode)curMode );


	SendMessageToMfa(m_boardId, m_subBoardId, SYSTEM_CARDS_MODE_REQ, sizeof(DWORD), (char*)&curMode);
}


//////////////////////////////////////////////////////////////////////
/*
void CMfaTask::TreatIvrMountReadStatus()
{
	SendIvrMountStatusIndToRsrcAlloc(TRUE);

	//------------------------------------ old code ------------------------------------
	//SendIvrMountStatusIndToRsrcAlloc(TRUE); // 'TRUE' is temp:

	// to be inserted when field 'IvrMount' is implemented in CardMngrLoaded structure
	//     (instead of the 'TRUE' in the line above)
	SendIvrMountStatusIndToRsrcAlloc( m_cardMngr.GetIvrMountReadStatus() );
	if ( TRUE != m_cardMngr.GetIvrMountReadStatus() )
	{
 		AddActiveAlarmMfa( "TreatIvrMountReadStatus", FAULT_CARD_SUBJECT, AA_IVR_FOLDER_MOUNTING_FAILED,
		                   MAJOR_ERROR_LEVEL, "", true, true, 0, m_boardId, 0, FAULT_TYPE_MPM );

	}

	//------------------------------------ old code ------------------------------------
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendIvrMountStatusIndToRsrcAlloc(DWORD isMountSucceeded)
{
	// ===== 1. print to log
	CMedString traceStr = "\nCMfaTask::SendIvrMountStatusIndToRsrcAlloc -";
	traceStr << " BoardId: " << m_boardId;
	if (TRUE == isMountSucceeded)
	{
		traceStr << ", IVR Mount status: ok";
	}
	else
	{
		traceStr << ", IVR Mount status: fail";
	}
	TRACESTR(eLevelInfoNormal) << traceStr.GetString();


	// ===== 2. prepare segment
	CSegment*  pParam = new CSegment();
	*pParam << (DWORD)m_boardId;
	*pParam << (DWORD)isMountSucceeded;


	// ===== 3. send to RsrcAlloc
	const COsQueue* pRsrcMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResource, eManager);

	STATUS res = pRsrcMbx->Send(pParam, CARDS_RSRC_IVR_MOUNT_READ_STATUS_IND);
}
*/

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendUnitConfigReqToRsrcalloc()
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendUnitConfigReqToRsrcalloc";

    RSRCALLOC_UNITS_LIST_CONFIG_PARAMS_S* pConfigParamsList =
		             new RSRCALLOC_UNITS_LIST_CONFIG_PARAMS_S;
    memset(pConfigParamsList, 0, sizeof *pConfigParamsList);

	CSegment*  pRetParam = new CSegment;

	int i=0;


	// ===== 1. initialize the list
	BYTE unitsTypes[MAX_NUM_OF_UNITS], postResults[MAX_NUM_OF_UNITS];
	memcpy(&unitsTypes,  m_cardMngr.GetUnitsTypesList(),  MAX_NUM_OF_UNITS);
	memcpy(&postResults, m_cardMngr.GetPostResultsList(), MAX_NUM_OF_UNITS);

	pConfigParamsList->cardType = (DWORD)( m_cardMngr.GetType() );

	for(i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		pConfigParamsList->unitsConfigParamsList[i].physicalHeader.board_id     = m_boardId;
		pConfigParamsList->unitsConfigParamsList[i].physicalHeader.sub_board_id = m_subBoardId;
		pConfigParamsList->unitsConfigParamsList[i].physicalHeader.unit_id      = i;
		pConfigParamsList->unitsConfigParamsList[i].unitType                    = unitsTypes[i];

		// ================================================================================
		// when an answer is given regarding who sends the pqNumber of each unit,
		//  (MPL or hard coded), the following patch should be removed
		pConfigParamsList->unitsConfigParamsList[i].pqNumber = ( (MAX_NUM_OF_UNITS / 2) > i ? 1 : 2 );
		// ================================================================================


		pConfigParamsList->unitsConfigParamsList[i].status                      = postResults[i];
	}



	// ===== 2. fill the Segment with items from the list
	pRetParam->Put( (BYTE*)pConfigParamsList, sizeof(RSRCALLOC_UNITS_LIST_CONFIG_PARAMS_S) );


	delete pConfigParamsList;


	// ===== 3. send
	const COsQueue* pResourceMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResource, eManager);

    GetRcvMbx().Serialize(*pRetParam);

	STATUS res = pResourceMbx->Send(pRetParam,RSRCALLOC_UNIT_CONFIG_REQ);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::OnRsrcAllocUnitConfigInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "CMfaTask::OnRsrcAllocUnitConfigInd";

	// ===== 1. since RSRCALLOC_UNIT_CONFIG_IND was received, the task proceeds to next state
    m_state = CONFIGURATION;

	// ===== 2. fill CMfaTask's attribute with data from structure received
	SetUnitsTypesConfigured(pSeg);

	// ===== 3. send the request to MplApi (and from there - to MFA)
	SendUnitConfigReqToMplApi();
}
//////////////////////////////////////////////////////////////////////
void CMfaTask::OnCardConfigReq(CSegment* pSeg)//2 modes cop/cp - from RSRC
{
	TRACEINTO<<"MfaTask::OnCardConfigReq START\n";

	CARDS_CONFIG_PARAMS_S* CardsConfigParams = new CARDS_CONFIG_PARAMS_S;
	pSeg->Get( (BYTE*)CardsConfigParams, sizeof(CARDS_CONFIG_PARAMS_S) );

	TRACEINTO<<"MfaTask::OnCardConfigReq parameters:\n"
		<< "\nunSystemConfMode:  " << CardsConfigParams->unSystemConfMode
		<< "\nunFutureUse:   " << CardsConfigParams->unFutureUse;

	SendMessageToMfa(m_boardId, m_subBoardId,CARD_CONFIG_REQ,sizeof(CARDS_CONFIG_PARAMS_S), (char*)CardsConfigParams);
	delete CardsConfigParams;
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendUnitConfigReqToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendUnitConfigReqToMplApi";

 	// ===== 2. print the data to trace (using Dump function)
//	PrintUnitTypesConfiguredDataToMfa(m_pUnitsTypesConfiguredStruct);

	SendMessageToMfa(m_boardId, m_subBoardId, CM_UNIT_CONFIG_REQ, sizeof(CM_UNITS_CONFIG_S), (char*)m_pUnitsTypesConfiguredStruct);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::OnUnitLoadedInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnUnitLoadedInd";
	m_isUnitLoadedIndReceived = YES;

	// ===== 1. since CM_UNIT_LOADED_IND was received, the task becomes READY
    m_state = READY;

	// ===== 2. extract unitLoaded struct from the segment received
	//          and insert it to m_unitsLoadedStatusesStruct
	DWORD  structLen = sizeof(CM_UNIT_LOADED_S);
	memset(&m_unitsLoadedStatusesStruct,0,structLen);
	pSeg->Get((BYTE*)(&m_unitsLoadedStatusesStruct),structLen);

	// ===== 3. print data to trace
	PrintUnitLoadedDataToTrace();

	// ===== 4. send the mediaIp config request to MplApi (and from there - to MFA)
	SendMediaIpConfigReqToMplApi();

	//V35 support under JITC uses same idea as Multiple services - by vlan out via card and RTM_LAN
	if(IsMultipleServicesMode() || IsV35JITC())
		SendCSIpConfigRequestsToMplApi();

	// ===== 5. send the ivrAddMusicSource request to MplApi (and from there - to MFA)
	if ( (true == m_pProcess->GetIsIvrMusicAddSrcReqReceived()) &&
		 (NO == m_isIvrMusicSourceReqAlreaySentToMpm) )
	{
		SendIvrMusicAddSourceReqToMplApi();
	}

	// ===== 6. send the mediaRecording path to MplApi (and from there - to MFA)
	SendMediaRecordingReqToMplApi();

	// ===== 7. send the FIPS 140 requests to MplApi (and from there - to MFA)

	BOOL isJitcMode = NO;
	std::string key = "ULTRA_SECURE_MODE";
	m_pProcess->GetSysConfig()->GetBOOLDataByKey(key, isJitcMode);

	if(isJitcMode && m_pProcess->GetLicensingStruct()->federal)
		SendFIPS140RequestsToMplApi();

	//=====8. Send Dns Media Config to MplApi()
	SendDnsMediaConfigReqToMplApi();

//	//if need to send ice init request to the card
//	if (m_pProcess->GetCardsIceResponseList())
//	{
//		TRACESTR(eLevelInfoHigh) << "\nCMfaTask::OnUnitLoadedInd - send ice init to boardId: " << m_boardId<< " subBoardId: "<<m_subBoardId;
//		m_pProcess->SendIceInitToSpecificMpmCM(m_boardId, m_subBoardId);
//	}
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendCSIpConfigRequestsToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendCSIpConfigRequestsToMplApi";
	CMediaIpParameters* mediaIpParamsFromVector = m_pProcess->GetMediaIpParamsVector()->GetFirst();

	if (mediaIpParamsFromVector == NULL)
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendCSIpConfigRequestsToMplApi - nothing!!!";

	BYTE bFound = FALSE;

	while (NULL != mediaIpParamsFromVector)
	{
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendCSIpConfigRequestsToMplApi - boardId=" << mediaIpParamsFromVector->GetBoardId()
								<< " , subBoardId=" << mediaIpParamsFromVector->GetSubBoardId();

		if ( (mediaIpParamsFromVector->GetBoardId()    == m_boardId) &&
			 (mediaIpParamsFromVector->GetSubBoardId() == m_subBoardId) )
		{
			char ipAddressStr[IP_ADDRESS_LEN];
			SystemDWORDToIpString(mediaIpParamsFromVector->GetMediaIpParamsStruct()->csIp, ipAddressStr);

			DWORD PQ_Id = mediaIpParamsFromVector->GetIpParams().interfacesList[0].pqId;

			TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendCSIpConfigRequestsToMplApi - csIp = " << ipAddressStr << ", name = " << mediaIpParamsFromVector->GetMediaIpParamsStruct()->serviceName<<" ,PQId = "<<PQ_Id;
			DWORD V35GwIpv4 = mediaIpParamsFromVector->GetIpParams().v35GwIpv4Address;
			//if( IsMultipleServicesMode() || (IsV35JITC() && 2 == PQ_Id) )
			if( IsMultipleServicesMode() || (IsV35JITC()&&(V35GwIpv4 != 0)))
			{
				int serviceId = mediaIpParamsFromVector->GetMediaIpParamsStruct()->serviceId;
				if (m_pProcess->GetCSMsgState(serviceId)  == eStartConfig || m_pProcess->GetCSMsgState(serviceId)  == eConfigFailed)
				{
					TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendCSIpConfigRequestsToMplApi - change state eSendMsgWaitForReply service id "<< serviceId;
					m_pProcess->SetCSMsgState(serviceId,eSendMsgWaitForReply);
					m_pProcess->SetCSIpConfigBoardId(serviceId,m_boardId);
					SendCSInternalIpConfigReqToMplApi(mediaIpParamsFromVector);
					SendCSExternalIpConfigReqToMplApi(mediaIpParamsFromVector);

					CSegment *pSeg = new CSegment;

					*pSeg << (DWORD)serviceId;

					/* There should be only 2 services per card cause there are 2 ports*/

					if (!(IsValidTimer(CS_EXT_INT_IP_CONFIG_FIRST_SERVICE_TIMER)))

						StartTimer(CS_EXT_INT_IP_CONFIG_FIRST_SERVICE_TIMER, 120*SECOND,pSeg);

					else if (!(IsValidTimer(CS_EXT_INT_IP_CONFIG_SECOND_SERVICE_TIMER)))
						StartTimer(CS_EXT_INT_IP_CONFIG_SECOND_SERVICE_TIMER, 120*SECOND,pSeg);

					else
						TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendCSIpConfigRequestsToMplApi - we have timers problem service id "<< serviceId;




				}
				else if (m_pProcess->GetCSMsgState(serviceId)  == eSendMsgWaitForReply)
				{
					TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendCSIpConfigRequestsToMplApi -  state eSendMsgWaitForReply service id "<< serviceId;

					//if ( ! IsValidTimer(CS_EXT_INT_WAIT_REPLY_TIMER) )
					CSegment *pSeg = new CSegment;

					*pSeg << (DWORD)serviceId;

					StartTimer(CS_EXT_INT_WAIT_REPLY_TIMER, 60*SECOND,pSeg);
					//change that service state

				}
				else if (m_pProcess->GetCSMsgState(serviceId)  == eDone)
				{
					TRACESTR(eLevelInfoNormal) << "\nCMfaTask::cs ip already set service id "<< serviceId;
				}
				else
				{
					TRACESTR(eLevelInfoNormal) << "\nCMfaTask::illegal state service id "<< serviceId;
				}

				HandleDNSConfiguration(mediaIpParamsFromVector);
				HandleV35GWConfiguration(mediaIpParamsFromVector);
				bFound = TRUE;
			}
		}

		// ===== 4. get next mediaIpParams from list in process
		mediaIpParamsFromVector = m_pProcess->GetMediaIpParamsVector()->GetNext();
	}

	if (bFound == FALSE)
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendCSIpConfigRequestsToMplApi - Didn't find the card";
	else
        StartTimer(MFA_IP_CONFIG_MSGS_TIMER, 120 * SECOND);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendMediaIpConfigReqToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendMediaIpConfigReqToMplApi";

	// ===== 1. get the mediaIpParams from process's vector
	CMediaIpParameters* mediaIpParamsFromVector = m_pProcess->GetMediaIpParamsVector()->GetFirst();

	while (NULL != mediaIpParamsFromVector)
	{
		// ===== 2. if it's a service from the suitable boardID and subBoardId...
		if ( (mediaIpParamsFromVector->GetBoardId()    == m_boardId) &&
			 (mediaIpParamsFromVector->GetSubBoardId() == m_subBoardId) )
		{
			// ===== ...and ip address is valid...
			STATUS ipValidStatus = STATUS_OK;
			if ( YES == IsTarget() ) // otherwise no configuration is actually done
				ipValidStatus = ValidateIpAddressWithMask(mediaIpParamsFromVector);

			// ===== 3. Set vlan data (need only under jitc, separated mode)
			DWORD PQ_Id = mediaIpParamsFromVector->GetIpParams().interfacesList[0].pqId;
			SetVlanData(PQ_Id, mediaIpParamsFromVector->GetMediaIpParamsStruct(), TRUE);

			if (STATUS_OK == ipValidStatus)
			{
				m_isMediaIpExistsForThisMfa = YES;

				// ===== ...then print it to trace...
				TRACESTR(eLevelInfoNormal)
					<< "\nMediaIpParams sent to Mfa "
					<< "(BoardId: " << m_boardId << ", SubBoardId: " << m_subBoardId << "):"
					<< "\n=========================\n"
					<< *mediaIpParamsFromVector;

				// ===== ...and send it to MplApi (and from there to MFA)
				SendMessageToMfa(m_boardId,
								m_subBoardId,
								 CM_MEDIA_IP_CONFIG_REQ,
								 sizeof(MEDIA_IP_PARAMS_S),
								 (char*)mediaIpParamsFromVector->GetMediaIpParamsStruct());

			} // end ip valid
			else
			{
				TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendMediaIpConfigReqToMplApi - ip address mismatches netMask; "
				                       << "nothing is therefore sent to MFA";
			}
		} // end service is ok and belongs to this MFA

		else
		{
			TRACESTR(eLevelInfoNormal)
				<< "\n  VectorBoardId: " << mediaIpParamsFromVector->GetBoardId()
				<< ", Vector SubBId: " << mediaIpParamsFromVector->GetSubBoardId()
				<< ", Task's BrdId: "  << m_boardId
				<< ", Task's SubBId: " << m_subBoardId;
		}


		// ===== 4. get next mediaIpParams from list in process
		mediaIpParamsFromVector = m_pProcess->GetMediaIpParamsVector()->GetNext();
	}

	// 30.04.07: RtmMediaIp addresses are sent on MFA startup (instead of on RTM startup)
	SendRtmMediaIpConfigReqToMplApi();

	SendMediaIpConfigEndReqToMplApi();
}


//////////////////////////////////////////////////////////////////////
//Internal media card's NIC data (address, mask, Vlan
void CMfaTask::SendCSInternalIpConfigReqToMplApi(CMediaIpParameters* mediaIpParamsFromVector)
{
	DWORD PQ_Id = mediaIpParamsFromVector->GetIpParams().interfacesList[0].pqId;

	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendCSInternalIpConfigReqToMplApi, PQId:" << PQ_Id;

	MEDIA_IP_PARAMS_S structToSend;
	memset( &structToSend, 0, sizeof(MEDIA_IP_PARAMS_S) );

	// ===== 2. Set vlan data (according to the service allocated to this board)
	SetVlanDataForMultiServices(PQ_Id, &structToSend);

	// ===== 3. Set internal CS address (according to the service allocated to this board)
	SetMediaCardInternalIPv4MultiServices(PQ_Id, &structToSend);
	structToSend.serviceId =   mediaIpParamsFromVector->GetMediaIpParamsStruct()->serviceId;
	structToSend.ipParams.interfacesList[0].ipType 	= eIpType_IpV4;
	structToSend.ipParams.interfacesList[0].pqId	= PQ_Id;

	structToSend.csIp  = mediaIpParamsFromVector->GetMediaIpParamsStruct()->csIp;
	CMediaIpParameters* pCurStruct = new CMediaIpParameters(structToSend);
	// ===== ...then print it to trace...
	TRACESTR(eLevelInfoNormal)
	<< "\nCS Internal IP Config sent to Mfa "
	<< "(BoardId: " << m_boardId << ", SubBoardId: " << m_subBoardId << ", PQId:" << PQ_Id << "):"
	<< "\n=========================\n"
	<< *pCurStruct;
	POBJDELETE(pCurStruct);

	// ===== ...and send it to MplApi (and from there to MFA)
	SendMessageToMfa(m_boardId,
			m_subBoardId,
			CM_CS_INTERNAL_IP_CONFIG_REQ,
			sizeof(MEDIA_IP_PARAMS_S),
			(char*)&structToSend);

	m_ipConfigMsgACKCounter++;

	//TRACESTR(eLevelInfoNormal)
	//<< "\nCMfaTask::SendCSInternalIpConfigReqToMplApi m_ipConfigMsgACKCounter ++  " << m_ipConfigMsgACKCounter;

}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendCSExternalIpConfigReqToMplApi(CMediaIpParameters* mediaIpParamsFromVector)
{
	DWORD PQ_Id = mediaIpParamsFromVector->GetIpParams().interfacesList[0].pqId;

	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendCSExternalIpConfigReqToMplApi, PQId:" << PQ_Id;

	MEDIA_IP_PARAMS_S structToSend;
	memset( &structToSend, 0, sizeof(MEDIA_IP_PARAMS_S) );

	// ===== 1. Set external CS address (according to the service allocated to this board)
	SetCSExternalIPv4(mediaIpParamsFromVector, &structToSend);

	// ===== 2. Set vlan data (according to the service allocated to this board)
	SetVlanDataForMultiServices(PQ_Id, &structToSend);

	// ===== 3. Set internal CS address (according to the service allocated to this board)
	SetCSInternalIPv4(PQ_Id, &structToSend);

	structToSend.ipParams.interfacesList[0].ipType 	= eIpType_IpV4;
	structToSend.ipParams.interfacesList[0].pqId 	= PQ_Id;
	structToSend.serviceId =   mediaIpParamsFromVector->GetMediaIpParamsStruct()->serviceId;
	structToSend.csIp  = mediaIpParamsFromVector->GetMediaIpParamsStruct()->csIp;

	CMediaIpParameters* pCurStruct = new CMediaIpParameters(structToSend);
	// ===== ...then print it to trace...
	TRACESTR(eLevelInfoNormal)
	<< "\nCS External IP Config sent to Mfa "
	<< "(BoardId: " << m_boardId << ", SubBoardId: " << m_subBoardId << ", PQId: " << PQ_Id << "):"
	<< "\n=========================\n"
	<< *pCurStruct;
	POBJDELETE(pCurStruct);

	// ===== ...and send it to MplApi (and from there to MFA)
	SendMessageToMfa(m_boardId,
			m_subBoardId,
			CM_CS_EXTERNAL_IP_CONFIG_REQ,
			sizeof(MEDIA_IP_PARAMS_S),
			(char*)&structToSend);

	m_ipConfigMsgACKCounter++;

	//TRACESTR(eLevelInfoNormal)
	//	<< "\nCMfaTask::SendCSExternalIpConfigReqToMplApi m_ipConfigMsgACKCounter ++ " << m_ipConfigMsgACKCounter;

}

//////////////////////////////////////////////////////////////////////
void CMfaTask::HandleDNSConfiguration(CMediaIpParameters* mediaIpParamsFromVector)
{
	DWORD PQ_Id = mediaIpParamsFromVector->GetIpParams().interfacesList[0].pqId;

	DWORD vlanId = CalcMSvlanId(m_boardId, PQ_Id);
	DWORD internalGWAddress = GetVlanCardInternalIpv4Address(vlanId);
	DWORD port = 53;

	if(eServerStatusOff != mediaIpParamsFromVector->GetIpParams().dnsConfig.dnsServerStatus)
	{
		// 1. Configure 'internal' DNS ip address in /etc/resolv.conf
		const char* pStrDomain = (const char*)&mediaIpParamsFromVector->GetIpParams().dnsConfig.domainName;

		DWORD dnsIpv4 = mediaIpParamsFromVector->GetIpParams().dnsConfig.ipV4AddressList[0];
		if(0 != dnsIpv4)
		{
			TRACESTR(eLevelInfoNormal) << "\nCMfaTask::HandleDNSConfiguration:"
					<< "\nDomain: "	<< mediaIpParamsFromVector->GetIpParams().dnsConfig.domainName;

			::ConfigMoreDnsInOS(pStrDomain, internalGWAddress, "CMfaTask::HandleDNSConfiguration");

			// 2. Configure Media boards with proper NAT rule.
			SendNATRuleToMfa(vlanId, port, internalGWAddress, dnsIpv4);
		}
		else
			TRACESTR(eLevelInfoNormal) << "\nCMfaTask::HandleDNSConfiguration: No DNS configured for service: " << mediaIpParamsFromVector->GetMediaIpParamsStruct()->serviceName;
	}
	else
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::HandleDNSConfiguration: DNS is off";
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::HandleV35GWConfiguration(CMediaIpParameters* mediaIpParamsFromVector)
{
	DWORD PQ_Id = mediaIpParamsFromVector->GetIpParams().interfacesList[0].pqId;

	DWORD vlanId = CalcMSvlanId(m_boardId, PQ_Id);
	DWORD port = 443; //https wellknown port is 443
	DWORD internalGWAddress = GetVlanCardInternalIpv4Address(vlanId);
	DWORD V35GwIpv4 = mediaIpParamsFromVector->GetIpParams().v35GwIpv4Address;

	if(0 != V35GwIpv4)
	{
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::HandleV35GWConfiguration:" <<  V35GwIpv4;

		// Configure Media boards with proper NAT rule.
		SendNATRuleToMfa(vlanId, port, internalGWAddress, V35GwIpv4);
	}
	else
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::HandleDNSConfiguration: No DNS configured for service: " << mediaIpParamsFromVector->GetMediaIpParamsStruct()->serviceName;
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendNATRuleToMfa(DWORD vlanId, DWORD port, DWORD internalAddress, DWORD externalAddress)
{
	CM_CONFIG_DNAT_S structToSend;
	const DWORD sizeOfCM_CONFIG_DNAT_S = sizeof(CM_CONFIG_DNAT_S);

	memset(&structToSend, 0, sizeOfCM_CONFIG_DNAT_S);
	structToSend.vlanId = vlanId;
	structToSend.port = port;
	structToSend.internalIpV4.iPv4Address = internalAddress;
	structToSend.externalIpV4.iPv4Address = externalAddress;

	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendNATRuleToMfa:"
			<< "\nvlan: " 				<< structToSend.vlanId
			<< "\nport: " 				<< structToSend.port
			<< "\ninternal address: " 	<< structToSend.internalIpV4.iPv4Address
			<< "\nexternal address: " 	<< structToSend.externalIpV4.iPv4Address;

	SendMessageToMfa(m_boardId,
			m_subBoardId,
			CM_DNAT_CONFIG_REQ,
			sizeof(CM_CONFIG_DNAT_S),
			(char*)&structToSend);

	m_ipConfigMsgACKCounter++;
}

//////////////////////////////////////////////////////////////////////
/*
30.04.07: RtmMediaIp addresses are sent on MFA startup (instead of on RTM startup)
*/
void CMfaTask::SendRtmMediaIpConfigReqToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendRtmMediaIpConfigReqToMplApi";

	// ===== 1. prepare structure to send
	MEDIA_IP_PARAMS_S structToSend;
	memset( &structToSend, 0, sizeof(MEDIA_IP_PARAMS_S) );

	DWORD mediaIp=0;

	int firstMediaBoardId	= m_pProcess->Get1stMediaBoardId(),
		secondMediaBoardId	= m_pProcess->Get2ndMediaBoardId(),
		thirdMediaBoardId	= m_pProcess->Get3rdMediaBoardId(),
		fourthMediaBoardId	= m_pProcess->Get4thMediaBoardId();

	if (firstMediaBoardId == (int)m_boardId)
	{
		mediaIp	= SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_1);
	}
	else if (secondMediaBoardId == (int)m_boardId)
	{
		mediaIp	= SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_2);
	}
	else if (thirdMediaBoardId == (int)m_boardId)
	{
		mediaIp	= SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_3);
	}
	else if (fourthMediaBoardId == (int)m_boardId)
	{
		mediaIp	= SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_4);
	}
	else // not FIRST/SECOND/THIRD/FOURTH_MEDIA_BOARD_SLOT_NUMBER
	{
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendRtmMediaIpConfigReqToMplApi - illegal boardId: " << m_boardId;
		return;
	}

	structToSend.ipParams.interfacesList[0].iPv4.iPv4Address	= mediaIp;
	structToSend.ipParams.interfacesList[0].ipType				= eIpType_IpV4;
	structToSend.ipParams.networkParams.subnetMask				= SystemIpStringToDWORD(RTM_ISDN_SUBNET_MASK);
	structToSend.ipParams.networkParams.vLanMode				= YES;
	structToSend.ipParams.networkParams.vLanId					= RTM_ISDN_VLAN_ID;

	eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
	ePlatformType curPlatformType = m_pProcess->ConvertProductTypeToPlatformType(curProductType);
	structToSend.platformType									= (APIU32)(curPlatformType);


	// ===== 2. print
	CMediaIpParameters* pCurStruct = new CMediaIpParameters(structToSend);
	TRACESTR(eLevelInfoNormal)
		<< "\nCMfaTask::SendRtmMediaIpConfigReqToMplApi"
		<< "\nRtmMediaIpParams sent to MFA "
		<< "(BoardId: " << m_boardId << ", SubBoardId: " << m_subBoardId << "):\n"
		<< *pCurStruct;
	POBJDELETE(pCurStruct);


	// ===== 3. send
	SendMessageToMfa(m_boardId,
					 m_subBoardId,
					 CM_RTM_MEDIA_IP_CONFIG_REQ,
					 sizeof(MEDIA_IP_PARAMS_S),
					 (char*)&structToSend);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendMediaIpConfigEndReqToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendMediaIpConfigEndReqToMplApi";

	SendMessageToMfa(m_boardId, m_subBoardId, CM_MEDIA_IP_CONFIG_END_REQ, 0, NULL);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendIvrMusicAddSourceReqToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendIvrMusicAddSourceReqToMplApi";
	m_isIvrMusicSourceReqAlreaySentToMpm = YES;

	//CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol;

	// ===== 1. get the ivrAddMusicSource from process's vector
	SIVRAddMusicSource* pIvrAddMusicSourceFromVector = m_pProcess->GetIvrAddMusicSourceVector()->GetFirst();

	while (NULL != pIvrAddMusicSourceFromVector)
	{
		// ===== 2. print it to trace...
		m_pProcess->PrintIvrAddMusicSourceDataToTrace(pIvrAddMusicSourceFromVector);

		// ===== ...and send it to MplApi (and from there to MFA)
		SendMessageToMfa(m_boardId, m_subBoardId, IVR_ADD_MUSIC_SOURCE_REQ, sizeof(SIVRAddMusicSource), (char*)pIvrAddMusicSourceFromVector);

		// ===== 3. get next ivrAddMusicSource from list in process
		pIvrAddMusicSourceFromVector = m_pProcess->GetIvrAddMusicSourceVector()->GetNext();
	}

	//POBJDELETE(mplPrtcl);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendMediaRecordingReqToMplApi()
{

   /*if ( open(MEDIA_RECORDING_PHYSICAL_FILE_PATH,O_DIRECTORY) == -1) //Create the Folder only if needed
    if ( CreateDirectory(MEDIA_RECORDING_PHYSICAL_FILE_PATH) == FALSE)
      {
	    PTRACE(eLevelError," CMfaTask::SendMediaRecordingReqToMplApi() Can not create Recording directory");
	    PASSERT(1);
	    return ;
      }

    // ===== linking physical path to logical ( sent to CM )
    if( TRUE != CreateSymbolicLink(MEDIA_RECORDING_PHYSICAL_FILE_PATH, MEDIA_RECORDING_FILE_PATH) )
    {
     PTRACE(eLevelError," CMfaTask::SendMediaRecordingReqToMplApi() Can not create Recording link");
	 PASSERT(1);
	 return ;
    }
    // */

	// ===== 1. get mediaRecording path
	TStartupDebugRecordingParamReq* pMediaRecordingStruct = new TStartupDebugRecordingParamReq;
	memset( pMediaRecordingStruct->ucPathName,
	        0,
	        MAX_RECORDING_FULL_PATH_NAME);

	strncpy( pMediaRecordingStruct->ucPathName,
	         MEDIA_RECORDING_FILE_PATH.c_str(),
	         MAX_RECORDING_FULL_PATH_NAME-1 );

	/*//====== shortened path for mount=========//
	memset( pMediaRecordingStruct->ucPathMountName,
	        0,
	        MAX_RECORDING_FULL_PATH_NAME);

	strncpy( pMediaRecordingStruct->ucPathMountName,
	         MEDIA_RECORDING_MOUNT_PATH,
	         MAX_RECORDING_FULL_PATH_NAME-1 );*/


	// ===== 2. print to trace
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendMediaRecordingReqToMplApi - mediaRecording path sent to MPL: "
	                       << pMediaRecordingStruct->ucPathName;


	// ===== 3. send to MplApi (and from there to MFA)
	SendMessageToMfa(m_boardId,
				     m_subBoardId,
				     STARTUP_DEBUG_RECORDING_PARAM_REQ,
				     sizeof(TStartupDebugRecordingParamReq),
				     (char*)pMediaRecordingStruct);

	delete pMediaRecordingStruct;
}
//////////////////////////////////////////////////////////////////////
void CMfaTask::SendFIPS140RequestsToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendFIPS140RequestsToMplApi";

	// ===== 1. get the unSimulationErrCode from SysConfig
	CSysConfig *sysConfig = m_pProcess->GetSysConfig();
	std::string eSimValue;
	sysConfig->GetDataByKey(CFG_KEY_FIPS140_SIMULATE_CARD_PROCESS_ERROR, eSimValue);
	/*
	 * **checksum is out of scope (02/2009)**
	 *
	CM_CHECKSUM_REQ_S* pChecksumReq = new CM_CHECKSUM_REQ_S;
	pChecksumReq->unSimulationErrCode = m_pProcess->TranslateSysConfigDataToEnumForCardManager(eSimValue);
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendFIPS140RequestsToMplApi, checksum simulation flag is - " << eSimValue.c_str();

	// ===== 2. Send it to MplApi (and from there to MFA)
	CMplMcmsProtocol *mplPrtcl = new CMplMcmsProtocol;
	mplPrtcl->AddCommonHeader(CM_CHECKSUM_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, m_boardId, m_subBoardId);

	mplPrtcl->AddData( sizeof(pChecksumReq), (char*)pChecksumReq );

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPLAPI");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	// ===== 3. Set checksum status as WAIT_FOR_REPLAY
	m_checksumStatus.unStatus = STATUS_WAIT_FOR_REPLAY;

	POBJDELETE(mplPrtcl);
	POBJDELETE(pChecksumReq);

	*/

	//Send FIPS 140 test request to all ARTs.
	TArtFips140Req* pArtFips140Req = new TArtFips140Req;

	// ===== 1. get the unSimulationErrCode from SysConfig
	pArtFips140Req->unSimulationErrCode = m_pProcess->TranslateSysConfigDataToEnumForArt(eSimValue);
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendFIPS140RequestsToMplApi, FIPS 140 simulation flag is - " << eSimValue.c_str();

	for (WORD i = 0; i < MAX_NUM_OF_UNITS; i++)
	{
		// ===== 2. Send FIPS 140 request only to ARTs
		if(m_pUnitsTypesConfiguredStruct->unitsParamsList[i].type == eArt ||
		   m_pUnitsTypesConfiguredStruct->unitsParamsList[i].type == eArtCntlr)
		{
			CMplMcmsProtocol *mplPrtcl2 = new CMplMcmsProtocol;

			// ===== 3. Send it to MplApi (and from there to MFA)
			mplPrtcl2->AddCommonHeader(ART_FIPS_140_REQ);
			mplPrtcl2->AddMessageDescriptionHeader();
			mplPrtcl2->AddPhysicalHeader(1, m_boardId, m_subBoardId,i);
			mplPrtcl2->AddPortDescriptionHeader(0,0,0,0);
			mplPrtcl2->AddData( sizeof(pArtFips140Req), (char*)pArtFips140Req );

			CMplMcmsProtocolTracer(*mplPrtcl2).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPLAPI");
			mplPrtcl2->SendMsgToMplApiCommandDispatcher();

			// ===== 4. Set Fips 140 ART status as WAIT_FOR_REPLAY
			m_unitsFips140StatusList[i] = STATUS_WAIT_FOR_REPLAY;

			POBJDELETE(mplPrtcl2);
		}
	}

	POBJDELETE(pArtFips140Req);

	StartTimer(FIPS_140_TIMER_RECEIVE_ALL_IND, 30 * SECOND);
}
//////////////////////////////////////////////////////////////////////
void CMfaTask::OnTimerReceiveAllFips140IndsTimeout(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnTimerReceiveAllFips140IndsTimeout";

	HandleStartupOverIfNeeded("OnTimerReceiveAllFips140IndsTimeout");
}


//////////////////////////////////////////////////////////////////////
void CMfaTask::OnTimerReceiveAllIPConfigMsgsTimeout(CSegment* pSeg)
{
    TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnTimerReceiveAllIPConfigMsgsTimeout";

    TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnTimerReceiveAllIPConfigMsgsTimeout m_ipConfigMsgACKCounter: " << m_ipConfigMsgACKCounter;

    //m_ipConfigMsgACKCounter = 0;

    HandleStartupOverIfNeeded("OnTimerReceiveAllIPConfigMsgsTimeout");
}
//////////////////////////////////////////////////////////////////////
void CMfaTask::SendAnActiveAlarmAboutChecksumFailure()
{
	CMedString errStr = "Checksum failure !";

	AddActiveAlarmMfa( "SendAnActiveAlarmAboutChecksumFailure",
		                     FAULT_CARD_SUBJECT,
			                 AA_CARD_STARTUP_FAILURE,
			                 MAJOR_ERROR_LEVEL,
			                 errStr.GetString(),
			                 true,
			                 true,
			                 m_boardId, // boardId as 'userId' (token)
			                 m_boardId,
			                 0,
			                 FAULT_TYPE_MPM);
}
//////////////////////////////////////////////////////////////////////
void CMfaTask::SendAnActiveAlarmAboutAllFailedARTs()
{
	CMedString errStr = "Deterministic FIPS 140 test was failed in ARTs: ";

	for(WORD i = 0; i < MAX_NUM_OF_UNITS ; i++)
	{
		if(m_unitsFips140StatusList[i] == STATUS_WAIT_FOR_REPLAY ||
			m_unitsFips140StatusList[i] == STATUS_FAIL)
			errStr << i <<", ";
	}

	AddActiveAlarmMfa( "SendAnActiveAlarmAboutAllFailedARTs",
	                     FAULT_CARD_SUBJECT,
		                 AA_CARD_STARTUP_FAILURE,
		                 MAJOR_ERROR_LEVEL,
		                 errStr.GetString(),
		                 true,
		                 true,
		                 m_boardId, // boardId as 'userId' (token)
		                 m_boardId,
		                 0,
		                 FAULT_TYPE_MPM);
}
/////////////////////////////////////////////////////////////////////////
CMediaIpConfigVector* CMfaTask::GetMediaIpConfigVector()const
{
	return m_pIpMediaConfigVector;
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::OnMediaIpConfigInd(CSegment* pSeg)
{
    //eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();

    	SendLanEthSettingToMcuMngr(); // WE CAN MOVE IT TO OnEthSettingInd!!!! CALL IT ONLY ONCE.

	// ===== 1. get the new MediaIpConfig
	MEDIA_IP_CONFIG_S *pNewMediaIpConfigStruct = (MEDIA_IP_CONFIG_S*)pSeg->GetPtr();

	// ===== 2. print the data to trace
	PrintMediaIpConfigDataToTrace(pNewMediaIpConfigStruct, "OnMediaIpConfigInd");

    // ===== 3. produce ASSERT when no global address is received
    eIpType curTpe = (eIpType)(pNewMediaIpConfigStruct->ipType);
    if ( (eIpType_Both == curTpe) || (eIpType_IpV6 == curTpe) )
    {
    	ProduceIPv6AddressAssertIfNeeded(pNewMediaIpConfigStruct);
    }

	// ===== 4. update card's status
	UpdateCardStateAccordingToTaskState("OnMediaIpConfigInd");


	DWORD rtmMediaIp1		= SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_1),
	      rtmMediaIp2		= SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_2),
	      rtmMediaIp3		= SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_3),
	      rtmMediaIp4		= SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_4),
	      ipAddressFromMpl	= pNewMediaIpConfigStruct->iPv4.iPv4Address;

	if ( eMediaIpConfig_Ok != (eMediaIpConfigStatus)(pNewMediaIpConfigStruct->status) )
    {
		// ===== 5. add ActiveAlarm and log
		CSmallString statStr;
		char* tmpStatStr = ::CardMediaIpConfigStatusToString( (eMediaIpConfigStatus)(pNewMediaIpConfigStruct->status) );
		if (tmpStatStr)
		{
			statStr << tmpStatStr;
		}
		else
		{
			statStr << "(invalid: "<< pNewMediaIpConfigStruct->status << ")";
		}

		CSmallString alarmStr = "Failure type: ";
		alarmStr << statStr;

		if ( (ipAddressFromMpl != rtmMediaIp1) &&	// internal rtm addresses do not produce an Alert on status!=ok
			 (ipAddressFromMpl != rtmMediaIp2) &&
			 (ipAddressFromMpl != rtmMediaIp3) &&
			 (ipAddressFromMpl != rtmMediaIp4) )
		{
			DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
	 		AddActiveAlarmMfa( "OnMediaIpConfigInd", FAULT_UNIT_SUBJECT, AA_MEDIA_IP_CONFIGURATION_FAILURE,
	 		                   MAJOR_ERROR_LEVEL, alarmStr.GetString(), true, true,
	 		                   0, displayBoardId, pNewMediaIpConfigStruct->pqNumber, FAULT_TYPE_PQ );
		}

		if (true == m_isEnabled) // for DUP_IP for example, send msg to RSRS.
			SendKeepAliveFailureToRsrcAlloc(eNoConnection,RSRCALLOC_IP_CONFIG_FAIL_IND);

		// print to trace (even for internal rtm addresses)
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnMediaIpConfigInd - status not ok!"
                               << "\nBoardId: " << m_boardId << ", SubBoardId: " << m_subBoardId
                               << " - " << alarmStr.GetString();
    }

    else // status == ok
	{
		if ( (ipAddressFromMpl != rtmMediaIp1) &&	// internal rtm addresses do not produce an Alert on status!=ok
			 (ipAddressFromMpl != rtmMediaIp2) &&
			 (ipAddressFromMpl != rtmMediaIp3) &&
			 (ipAddressFromMpl != rtmMediaIp4) )
		{
			RemoveActiveAlarmMfaByErrorCode("OnMediaIpConfigInd", AA_MEDIA_IP_CONFIGURATION_FAILURE);
		}

		// ===== 6. convert MEDIA_IP_CONFIG_S to CMediaIpConfig
		CMediaIpConfig* pMediaIpConfig = new CMediaIpConfig;
		pMediaIpConfig->SetData(pNewMediaIpConfigStruct);

		// ===== 7. add the new MediaIpConfig to the list
		m_pIpMediaConfigVector->Insert(pMediaIpConfig);

		// ===== 8. send the new MediaIpConfig to CsMngr
		SendMediaIpConfigIndToCS(pMediaIpConfig);

		//	POBJDELETE(pMediaIpConfig); // (pMediaIpConfig is deleted when m_pIpMediaConfigVector is erased)
	}

	// ===== 9. handle operations of startup_over
	//            [moved to end of function since SNMP needs 'm_pIpMediaConfigVector->Insert' (section 6 above)]
//	DWORD ipAddressLittleEndian = ntohl(ipAddressFromMpl); // convert to LittleEndian, just for the checking (since the addresses are sent to Mcms in that format)
//	SetIsMediaIpConfigIndReceived(ipAddressLittleEndian);
	SetIsMediaIpConfigIndReceived( ipAddressFromMpl,
			                       rtmMediaIp1,
			                       rtmMediaIp2,
			                       rtmMediaIp3,
			                       rtmMediaIp4,
			                       (DWORD)(pNewMediaIpConfigStruct->status) );


	m_mediaIpConfigStruct->pqNumber = pNewMediaIpConfigStruct->pqNumber;
	m_mediaIpConfigStruct->status = pNewMediaIpConfigStruct->status;

	// AN - vngr-25498 - moved from OnUnitLoadedInd.
	// CM finish to configure media card IP and ready for ICE just here
	// if need to send ice init request to the card
	if (m_pProcess->GetCardsIceResponseList())
	{
		TRACESTR(eLevelInfoHigh) << "\nCMfaTask::OnMediaIpConfigInd - send ice init to boardId: " << m_boardId<< " subBoardId: "<<m_subBoardId;
		m_pProcess->SendIceInitToSpecificMpmCM(m_boardId, m_subBoardId, NULL);
	}

	HandleStartupOverIfNeeded("OnMediaIpConfigInd");
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::PrintMediaIpConfigDataToTrace(const MEDIA_IP_CONFIG_S *pNewMediaIpConfigStruct, const string theCaller)
{
    CLargeString retStr = "\nCMfaTask::PrintMediaIpConfigDataToTrace";
    retStr << " (caller: " << theCaller << ")";

    CMedString statStr;
	char* tmpStatStr = ::CardMediaIpConfigStatusToString( (eMediaIpConfigStatus)(pNewMediaIpConfigStruct->status) );
	if (tmpStatStr)
	{
		statStr << tmpStatStr;
	}
	else
	{
		statStr << "(invalid: "<< pNewMediaIpConfigStruct->status << ")";
	}

    retStr << "\nBoardId: "     << m_boardId << ", subBoardId: " << m_subBoardId
           << "\nStatus:          " << statStr
           << "\nService Id:      " << pNewMediaIpConfigStruct->serviceId;


    char ipAddressStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString(pNewMediaIpConfigStruct->iPv4.iPv4Address, ipAddressStr);

    retStr << "\nIPv4 Address:    " << ipAddressStr;
	for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		retStr << "\nIPv6 Address " << i << ":  "
		       << (char*)(pNewMediaIpConfigStruct->iPv6[i].iPv6Address);
	}
	retStr << "\nIP Type:         " << IpTypeToString(pNewMediaIpConfigStruct->ipType)
		   << "\nPQ Number:       " << pNewMediaIpConfigStruct->pqNumber
		   << "\nIPv6 Default GW: " << (char*)(pNewMediaIpConfigStruct->defaultGatewayIPv6);


    TRACESTR(eLevelInfoNormal) << retStr.GetString();
}

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendLanEthSettingToMcuMngr()
{
    /* if it is rtm_isdn we don't need to set port 2
     * there is only one port (port 1)
     * we will set the port only if it is already mounted.
     */
    DWORD DisplayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, 2);

    bool isMounted = m_pProcess->GetEthernetSettingsStructsList()->GetIsMounted(DisplayBoardId,ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD);
    if (isMounted == true)
    {
	// === prepare segment
	CSegment*  pSeg = new CSegment;

	*pSeg << (WORD)m_boardId;
	*pSeg << (WORD)ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD;


	// === print to log

	TRACESTR(eLevelInfoNormal)<<"\nCMfaTask::SendRtmIsdnOrLanToMcuMngr";

	//TRACESTR(eLevelInfoNormal) << str.GetString();

	// === send
	CManagerApi api(eProcessMcuMngr);
	api.SendMsg(pSeg, RTM_LANS_IND);

    }

     isMounted = m_pProcess->GetEthernetSettingsStructsList()->GetIsMounted(DisplayBoardId,ETH_SETTINGS_MEDIA_4_PORT_ON_MEDIA_BOARD);
    if (isMounted == true)
       {
   	// === prepare segment
   	CSegment*  pSeg = new CSegment;

   	*pSeg << (WORD)m_boardId;
   	*pSeg << (WORD)ETH_SETTINGS_MEDIA_4_PORT_ON_MEDIA_BOARD;


   	// === print to log

   	TRACESTR(eLevelInfoNormal)<<"\nCMfaTask::SendRtmIsdnOrLanToMcuMngr";

   	//TRACESTR(eLevelInfoNormal) << str.GetString();

   	// === send
   	CManagerApi api(eProcessMcuMngr);
   	api.SendMsg(pSeg, RTM_LANS_IND);

       }

    isMounted = m_pProcess->GetEthernetSettingsStructsList()->GetIsMounted(DisplayBoardId,ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD);


	if (IsMultipleServicesMode() || isMounted == true)
	{
	    /* if it is MS and rtm_lan we set port 1
	     * if it is MS and rtm_isdn we also set port 1
	     */
	    // === prepare segment
	     CSegment*  pSeg1 = new CSegment;

	     *pSeg1 << (WORD)m_boardId;
	     *pSeg1 << (WORD)ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD;


	     // === print to log

	     TRACESTR(eLevelInfoNormal)<<"\nCMfaTask::SendRtmIsdnOrLanToMcuMngr for port 1";

	     //TRACESTR(eLevelInfoHigh) << str.GetString();

	     // === send
	     CManagerApi api(eProcessMcuMngr);
	     api.SendMsg(pSeg1, RTM_LANS_IND);
	}



	 isMounted = m_pProcess->GetEthernetSettingsStructsList()->GetIsMounted(DisplayBoardId,ETH_SETTINGS_MEDIA_3_PORT_ON_MEDIA_BOARD);


		if (IsMultipleServicesMode() || isMounted == true)
		{
		    /* if it is MS and rtm_lan we set port 1
		     * if it is MS and rtm_isdn we also set port 1
		     */
		    // === prepare segment
		     CSegment*  pSeg1 = new CSegment;

		     *pSeg1 << (WORD)m_boardId;
		     *pSeg1 << (WORD)ETH_SETTINGS_MEDIA_3_PORT_ON_MEDIA_BOARD;


		     // === print to log

		     TRACESTR(eLevelInfoNormal)<<"\nCMfaTask::SendRtmIsdnOrLanToMcuMngr for port 1";

		     //TRACESTR(eLevelInfoHigh) << str.GetString();

		     // === send
		     CManagerApi api(eProcessMcuMngr);
		     api.SendMsg(pSeg1, RTM_LANS_IND);
		}
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CMfaTask::ProduceIPv6AddressAssertIfNeeded(const MEDIA_IP_CONFIG_S *pNewMediaIpConfigStruct)
{
    eIPv6AddressScope curScope = eIPv6AddressScope_other;
    bool isGlobalExists = false;

	for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		curScope = GetIPv6AddressScope( (char*)(pNewMediaIpConfigStruct->iPv6[i].iPv6Address) );

		if (eIPv6AddressScope_global == curScope)
		{
			isGlobalExists = true;
			break;
		}
	}

	if (false == isGlobalExists)
	{
		char buff[256];
		sprintf(buff, "No Global Media IPv6 address exists (boardId %d, subBoardId %d)", m_boardId, m_subBoardId);

		PASSERTMSG(TRUE, buff);
	}
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SetIsMediaIpConfigIndReceived( DWORD ipAddressToCheck,
		                                      DWORD rtmMediaIp1,
		                                      DWORD rtmMediaIp2,
		                                      DWORD rtmMediaIp3,
		                                      DWORD rtmMediaIp4,
		                                      DWORD status )
{
	// 1. print
	TRACESTR(eLevelInfoNormal) << "\nSetIsMediaIpConfigIndReceived"
	                       << "\nrtmMediaIp1:          " << rtmMediaIp1
	                       << "\nrtmMediaIp2:          " << rtmMediaIp2
	                       << "\nrtmMediaIp3:          " << rtmMediaIp3
	                       << "\nrtmMediaIp4:          " << rtmMediaIp4
	                       << "\nipAddress received:   " << ipAddressToCheck
	                       << "\nstatus:               " << ::CardMediaIpConfigStatusToString( (eMediaIpConfigStatus)status );

	// 2. check
	if ( YES == IsTarget() )
	{
		if ( (ipAddressToCheck == rtmMediaIp1) ||
			 (ipAddressToCheck == rtmMediaIp2) ||
			 (ipAddressToCheck == rtmMediaIp3) ||
			 (ipAddressToCheck == rtmMediaIp4) )
		{
			m_isRtmMediaIpConfigIndReceived = YES;
		}
		else // MediaIpConfigInd (not Rtm's) is received
		{
			m_isMediaIpConfigIndReceived	= YES;
			m_mediaIpConfigStatus			= (eMediaIpConfigStatus)status;
		}
	}
	else // not Target (no configuration is actually done on Pizza)
	{
		m_isRtmMediaIpConfigIndReceived	= YES;
		m_isMediaIpConfigIndReceived	= YES;
		m_mediaIpConfigStatus			= eMediaIpConfig_Ok;
	}
}


//////////////////////////////////////////////////////////////////////
void CMfaTask::CalculateTotalPQsState()
{
	// in GL_1 there is only one relevant PQ.
	//    In the future there will be a need to calculate the sum of all PQs statuses
	//    and decide whether the MFA got out of startup successfully or not.
	// Indication to Rsource process will be sent only if statuses were received for all relevant PQs

	DWORD pq1Status = (DWORD)eOk,
	      pq2Status = (DWORD)eOk;

	CMedString retStr = "\nCMfaTask::CalculateTotalPQsState - ";
	retStr << "pqNumber: " << m_mediaIpConfigStruct->pqNumber << ", status: " << m_mediaIpConfigStruct->status;

	if (1 == m_mediaIpConfigStruct->pqNumber)
	{
		pq1Status = m_mediaIpConfigStruct->status;
	}
	else if (2 == m_mediaIpConfigStruct->pqNumber)
	{
		pq2Status = m_mediaIpConfigStruct->status;
	}
	else
	{
		retStr << "\n----- illegal pqNumber! Fatal statuses are therefore sent to Resource process";
		pq1Status = eFatal;
		pq2Status = eFatal;
	}

	TRACESTR(eLevelInfoNormal) << retStr.GetString();


	// As mentioned above, in the future the indication to Rsource process will be sent
	//    only if statuses were received for all relevant PQs
	if ( eOk == m_cardMngr.GetIvrMountReadStatus() )
	{
		SendMfaStartupCompleteIndToRsrcAlloc(pq1Status, pq2Status);
		m_isStartupCompleteIndAlreadySentToRsrcAlloc = YES;
	}
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendMfaStartupCompleteToCsMngr()
{
	// ===== 1. print to log
	PrintMfaStartupCompleteIndToCsMngr();

	// ===== 2. prepare the segment
	CSegment*  pParam = new CSegment();
	*pParam << (DWORD)m_boardId;

	// ===== 3. send to CsMngr
	const COsQueue* pCsMngrMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);
    GetRcvMbx().Serialize(*pParam);
	STATUS res = pCsMngrMbx->Send(pParam, CARDS_CSMNGR_MFA_STARTUP_COMPLETE_IND);


}

//////////////////////////////////////////////////////////////////////
void CMfaTask::PrintMfaStartupCompleteIndToCsMngr()
{
	CMedString traceStr = "\nCMfaTask::PrintMfaStartupCompleteIndToCsMngr -";
	traceStr << " boardId: " << m_boardId;

	TRACESTR(eLevelInfoNormal) << traceStr.GetString();
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendMfaStartupCompleteIndToRsrcAlloc(DWORD pq1status, DWORD pq2status)
{
	// ===== 1. print to log
	PrintMfaStartupCompleteIndToRsrcAlloc(pq1status, pq2status);

	// ===== 2. prepare the segment
	CSegment*  pParam = new CSegment();
	*pParam << (DWORD)m_boardId;
	*pParam << (DWORD)pq1status;
	*pParam << (DWORD)pq2status;
	//  **checksum is out of scope (02/2009)**
	*pParam << (DWORD)STATUS_OK;

	// ===== 3. send to RsrcAlloc
	const COsQueue* pRsrcMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResource, eManager);
    GetRcvMbx().Serialize(*pParam);
	STATUS res = pRsrcMbx->Send(pParam, CARDS_RSRC_MFA_STARTUP_COMPLETE_IND);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::PrintMfaStartupCompleteIndToRsrcAlloc(DWORD pq1status, DWORD pq2status)
{
	char *pq1statusStr = ::CardMediaIpConfigStatusToString(pq1status),
	     *pq2statusStr = ::CardMediaIpConfigStatusToString(pq2status);

	CMedString traceStr = "\nCMfaTask::PrintMfaStartupCompleteIndToRsrcAlloc -";
	traceStr << " boardId: " << m_boardId;

	if (pq1statusStr)
	{
		traceStr << ", PQ1 Status: " << pq1statusStr;
	}
	else
	{
		traceStr << ", PQ1 Status: (invalid: " << pq1status << ")";
	}

	if (pq2statusStr)
	{
		traceStr << ", PQ2 Status: " << pq2statusStr;
	}
	else
	{
		traceStr << ", PQ2 Status: (invalid: " << pq2status << ")";
	}

	TRACESTR(eLevelInfoNormal) << traceStr.GetString();
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendEthSettingIndToCS(DWORD linkStatus)
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendEthSettingIndToCS";

	CSegment*  pSeg = new CSegment();
	*pSeg << m_boardId
		  << m_subBoardId
		  << linkStatus;

	const COsQueue* pCsMngrMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

	STATUS res = pCsMngrMbx->Send(pSeg, CS_ETH_SETTING_IND);
}
//////////////////////////////////////////////////////////////////////
void CMfaTask::SendMediaIpConfigIndToCS(CMediaIpConfig* pMediaIpConfig)
{
    TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnMediaIpConfigInd - structure sent to CS:" << *pMediaIpConfig;

	// ===== 1. fill the segment with MediaIpConfig parameters
    CS_MEDIA_IP_CONFIG_S param;
    memset(&param, 0, sizeof(CS_MEDIA_IP_CONFIG_S));

    param.boardId		= m_boardId;
    param.subBoardId	= m_subBoardId;
    memcpy(&param.mediaIpConfig, pMediaIpConfig->GetMediaIpConfigStruct(), sizeof(MEDIA_IP_CONFIG_S));

	CSegment*  pRetParam = new CSegment();
    pRetParam->Put((BYTE*)&param, sizeof(CS_MEDIA_IP_CONFIG_S));

	// ===== 2. send to CsMngr
	const COsQueue* pCsMngrMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

	STATUS res = pCsMngrMbx->Send(pRetParam, CS_CARDS_MEDIA_IP_CONFIG_IND);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::OnCmReconnectInd(CSegment* pSeg)
{
	// ===== 1. logger
	TRACESTR(eLevelInfoNormal)
		<< "\nCMfaTask::OnCmReconnectInd -"
		<< "\nMFA Card Manger Reconnected. boardId: " << m_boardId << ", SubBoardId: " << m_subBoardId;

	// ===== 2. fault
	CHlogApi::CmReconnect(m_boardId, m_subBoardId, FAULT_TYPE_MPM);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::OnCmUpgradeNewVersionReadyAckInd(CSegment* pSeg)
{
	UPGRADE_NEW_VERSION_READY_IND_S *pUpgradeDataInd = (UPGRADE_NEW_VERSION_READY_IND_S*)pSeg->GetPtr();
	if (NULL!=pUpgradeDataInd && pUpgradeDataInd->status == MOUNT_NOT_CONNECTED)
	{
		TRACESTR(eLevelInfoNormal)
			<< "\nCMfaTask::OnCmUpgradeNewVersionReadyAckInd -"
			<< "\nMFA Card failed in mount. boardId: " << m_boardId << ", SubBoardId: " << m_subBoardId;

		m_pProcess->DecBoardIpmcIndCounter(m_boardId);
		m_pProcess->DecearseBoardInstallingIpmc(m_boardId);

		//Block further communication with this card
		CConfigManagerApi apiConfigurator;
		std::string ip = GetEmbBoardIPAddress(m_boardId);
		apiConfigurator.AddDropRuleToShorewall(ip);

		return;
	}

	if (m_isInstalling)
	{
	  // CM_UPGRADE_NEW_VERSION_READY_ACK_IND received twice
	  PASSERT(1);
	}
	else
	{
	    char message[100];
        eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
        if (curProductType !=eProductTypeRMX1500)
		   sprintf(message,"Media card software upgrade 0%% board Id:%d",m_boardId);
        else sprintf(message,"Media card software upgrade 0%%");
		m_require_ipmc_upgrade = NO;
		AddActiveAlarm(FAULT_GENERAL_SUBJECT,
				AA_VERSION_INSTALL_PROGRESS,
				SYSTEM_MESSAGE,
				message,
				true,
				true,
				0xFFFFFFFF,
				m_boardId);

		m_isInstalling = YES;

		m_pProcess->AddBoardInstalling();
		m_pProcess->UpdateSoftwareProgress(m_boardId,0);
	}
}



//////////////////////////////////////////////////////////////////////
void CMfaTask::OnCmUpgradeProcessInd(CSegment* pSeg)
{

	DWORD  structLen = sizeof(UPGRADE_PROGRESS_IND_S);
	UPGRADE_PROGRESS_IND_S rStruct;
	memset(&rStruct,0,structLen);
	pSeg->Get( (BYTE*)(&rStruct), structLen );

	int p = rStruct.progress_precents;
	PASSERT(p > 100);
	char message[100];
	sprintf(message,"Media card software upgrade %d%% board Id:%d",p,m_boardId);

	TRACESTR(eLevelInfoNormal) << "CMfaTask::OnCmUpgradeProcessInd boardId: " << m_boardId << " %" << p;
	if (m_isInstalling)
	{
		UpdateActiveAlarmDescriptionByErrorCode(AA_VERSION_INSTALL_PROGRESS,
				message);

		m_pProcess->UpdateSoftwareProgress(m_boardId,p);
	}
	else
	{
	  // CM_UPGRADE_NEW_VERSION_READY_ACK_IND was never received
	  PASSERT(1);
	}

}


//////////////////////////////////////////////////////////////////////
void CMfaTask::OnCmUpgradeIpmcInd(CSegment* pSeg)
{

	DWORD  structLen = sizeof(UPGRADE_IPMC_IND_S);
	UPGRADE_IPMC_IND_S rStruct;
	memset(&rStruct,0,structLen);
	pSeg->Get( (BYTE*)(&rStruct), structLen );

	TRACESTR(eLevelInfoNormal) << "CMfaTask::OnCmUpgradeIpmcInd boardId: " << m_boardId
								  << "require IPMC upgrade: "  << rStruct.require_ipmc_upgrade;
	if (m_isInstalling)
	{
		m_isInstalling = NO;
		RemoveActiveAlarmByErrorCode(AA_VERSION_INSTALL_PROGRESS);
		m_require_ipmc_upgrade = rStruct.require_ipmc_upgrade;
		m_pProcess->DecearseBoardInstalling();
		m_pProcess->DecBoardIpmcIndCounter(m_boardId);

		if (m_require_ipmc_upgrade)
		{
			// this is already reported, just in case...
			m_pProcess->AddBoardInstallingIpmc(m_boardId);
			//VNGR-23162
			CConfigManagerApi apiConfigurator;
			std::string ip = GetEmbBoardIPAddress(m_boardId);
			apiConfigurator.AddDropRuleToShorewall(ip,TRUE);  // we drop only ping
		}
		else
		{
			m_pProcess->DecearseBoardInstallingIpmc(m_boardId);
			//Block further communication with this card
			CConfigManagerApi apiConfigurator;
			std::string ip = GetEmbBoardIPAddress(m_boardId);
			apiConfigurator.AddDropRuleToShorewall(ip);
		}
	}
	else
	{
		PASSERT(1);
	}

}

//////////////////////////////////////////////////////////////////////
std::string CMfaTask::GetEmbBoardIPAddress(DWORD board_id)
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

//////////////////////////////////////////////////////////////////////
void CMfaTask::OnCmUpgradeIpmcProcessInd(CSegment* pSeg)
{

	DeleteTimer(IPMC_UPGRADE_TIMER);

	DWORD  structLen = sizeof(UPGRADE_PROGRESS_IND_S);
	UPGRADE_PROGRESS_IND_S rStruct;
	memset(&rStruct,0,structLen);
	pSeg->Get( (BYTE*)(&rStruct), structLen );

	int p = rStruct.progress_precents;
	PASSERT(p > 100);

	int seconds = 10;
	//if (p > 95)
	//	seconds = 2;


	char message[100];
    eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
    if (curProductType !=eProductTypeRMX1500)
	  sprintf(message,"Media card IPMC software upgrade %d%% board Id:%d",p,m_boardId);
    else
    	sprintf(message,"Media card IPMC software upgrade %d%% ",p);

	TRACESTR(eLevelInfoNormal) << "CMfaTask::OnCmUpgradeIpmcProcessInd boardId: " << m_boardId << " %" << p;
	if (m_isInstallingIpmc)
	{
		UpdateActiveAlarmDescriptionByErrorCode(AA_VERSION_IPMC_INSTALL_PROGRESS,
				message);
		m_pProcess->UpdateIpmcProgress(m_boardId,p);
		StartTimer( IPMC_UPGRADE_TIMER, seconds*SECOND);
	}
	else
	{

		AddActiveAlarm(FAULT_GENERAL_SUBJECT,
				AA_VERSION_IPMC_INSTALL_PROGRESS,
				SYSTEM_MESSAGE,
				message,
				true,
				true,
				0xFFFFFFFF,
				m_boardId);

		StartTimer( IPMC_UPGRADE_TIMER, seconds*SECOND);
		m_pProcess->UpdateIpmcProgress(m_boardId,p);
		m_isInstallingIpmc = YES;
	}

}



////////////////////////////////////////////////////////////////////
void CMfaTask::OnCmUpgradeIpmcTimer(CSegment* pSeg)
{


	TRACESTR(eLevelInfoNormal) << "CMfaTask::OnCmUpgradeIpmcTimer  boardId: " << m_boardId;

	if (m_isInstallingIpmc)
	{
		m_isInstallingIpmc = NO;
		RemoveActiveAlarmByErrorCode(AA_VERSION_IPMC_INSTALL_PROGRESS);
		m_pProcess->DecearseBoardInstallingIpmc(m_boardId);

		//Block further communication with this card
		CConfigManagerApi apiConfigurator;
		std::string ip = GetEmbBoardIPAddress(m_boardId);
		apiConfigurator.AddDropRuleToShorewall(ip);

		//PASSERT(2);

		// update installer here
	}
	else
	{
		PASSERT(1);
	}

}


// //////////////////////////////////////////////////////////////////////
// void CMfaTask::OnCmUpgradeIpmcAlmostCompleteInd(CSegment* pSeg)
// {


//   TRACESTR(eLevelInfoNormal) << "CMfaTask::OnCmUpgradeIpmcAlmostCompleteInd boardId: " << m_boardId;

//     if (m_isInstallingIpmc)
//     {
//       	  m_isInstallingIpmc = NO;
// 	  RemoveActiveAlarmByErrorCode(AA_VERSION_IPMC_INSTALL_PROGRESS);
// 	  // update installer here
//     }
//     else
//     {
//       PASSERT(1);
//     }

// }





/*
//////////////////////////////////////////////////////////////////////
void CMfaTask::OnIvrFolderMountInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnIvrFolderMountInd";

	// ===== 1. extract the mplMcmsProtocol from segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pSeg);

	// ===== 2. get the folderMounting data
	CM_FOLDER_MOUNT_S pFolderMount;
	memset( &pFolderMount, 0, sizeof(CM_FOLDER_MOUNT_S));
	memcpy( &pFolderMount, pMplMcmsProtocol->GetData(), sizeof(CM_FOLDER_MOUNT_S) );

	// ===== 3. print the data to log
	m_pProcess->PrintFolderMountDataToTrace(pFolderMount);

	// ===== 4. report to RsrcAlloc
	SendIvrMountStatusToRsrcAlloc(pFolderMount.ulMounted);

	// ===== 5. if !ok, set cardState to Major, produce ActiveAlarm
	if (STATUS_OK != pFolderMount.ulMounted)
	{
 		AddActiveAlarmMfa( "OnIvrFolderMountInd", FAULT_CARD_SUBJECT, AA_IVR_FOLDER_MOUNTING_FAILED,
		                   MAJOR_ERROR_LEVEL, "", true, true, 0, m_boardId, 0, FAULT_TYPE_MPM );
	}

	POBJDELETE(pMplMcmsProtocol);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendIvrMountStatusToRsrcAlloc(DWORD ivrMountStatus)
{
	// ===== 1. prepare segment (to Resource) and string (to log)
	CSegment*  pParam = new CSegment();
	*pParam << (DWORD)m_boardId;

	CMedString traceStr = "\nCMfaTask::SendIvrMountToRsrcAlloc -";
	traceStr << " BoardId: " << m_boardId;

	if (STATUS_OK == ivrMountStatus)
	{
		traceStr << ", IVR Mount status: ok";
		*pParam << (DWORD)TRUE;		// (isSuccess)
	}
	else
	{
		traceStr << ", IVR Mount status: fail";
		*pParam << (DWORD)FALSE;	// (isSuccess)
	}

	// ===== 2. print to log
	TRACESTR(eLevelInfoNormal) << traceStr.GetString();

	// ===== 3. send to RsrcAlloc
	const COsQueue* pRsrcMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResource, eManager);

	STATUS res = pRsrcMbx->Send(pParam, CARDS_RSRC_IVR_MOUNT_READ_STATUS_IND);
}
*/

//////////////////////////////////////////////////////////////////////
void CMfaTask::OnTimerStartupTimeout()
{
	// ===== 1. prepare the error string
	CMedString errStr = "MPM startup incomplete: ";
	if (NO == m_isUnitLoadedIndReceived)
	{
		errStr << "Unit loading confirmation was not received";
	}
	else if (NO == m_isMediaIpExistsForThisMfa)
	{
		errStr << "No Media IP for this card";
	}
	else if ( (NO == m_isMediaIpConfigIndReceived) || (NO == m_isRtmMediaIpConfigIndReceived) )
	{
		errStr << "Media IP Configuration confirmation was not received";
	}
	else
	{
		errStr << "Unspecified problem";
	}

	// ===== 3. print to log
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnTimerStartupTimeout"
	                       << " (Board id: " << m_boardId << ", SubBoardId: " << m_subBoardId << ") - "
	                       << errStr.GetString();

	// ===== 2. handle operations of startup_over
	HandleStartupOver("OnTimerStartupTimeout");

	// ===== 4. produce ActiveAlarm (and update card's state)
	//   conditioned: only if the reason is not related to ipService
	//                or that no former indication about ipService absence was produced
	if ( (NO == m_isUnitLoadedIndReceived) ||
	     (false == IsActiveAlarmExistByErrorCode(AA_NO_IP_SERVICE_PARAMS)) )
	{
		DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
		AddActiveAlarmMfa( "OnTimerStartupTimeout",
	                       FAULT_CARD_SUBJECT,
	                       AA_CARD_STARTUP_FAILURE,
	                       MAJOR_ERROR_LEVEL,
	                       errStr.GetString(),
	                       true,
	                       true,
	                       displayBoardId, // displayBoardId as 'userId' (token)
	                       displayBoardId,
	                       0,
	                       FAULT_TYPE_MPM);
	}
}
//////////////////////////////////////////////////////////////////////
void CMfaTask::HandleStartupOverIfNeeded(CSmallString callerStr)
{
	BOOL areAllFips140IndsReceived	= AreAllFips140IndsReceived();
	BOOL isValidFips140Timer		= IsValidTimer(FIPS_140_TIMER_RECEIVE_ALL_IND);
    BOOL isValidRecAllMsgsTimer        = IsValidTimer(	MFA_IP_CONFIG_MSGS_TIMER);
	// ===== 1. prepare printing
	CLargeString condsStr = "\nConditions -";
	condsStr << "\nStartupOverActionsPerformed: " << (m_isStartupOverActionsAlreadyPerformed	? "yes" : "no")
			 << "\nMediaIpConfigIndReceived:    " << (m_isMediaIpConfigIndReceived				? "yes" : "no")
			 << "\nRtmMediaIpConfigIndReceived: " << (m_isRtmMediaIpConfigIndReceived			? "yes" : "no")
			 << "\nAllFips140IndsReceived:      " << (areAllFips140IndsReceived					? "yes" : "no")
			 << "\nTimer FIPS_140_TIMER_RECEIVE_ALL_IND: " << (isValidFips140Timer ? "valid" : "invalid")
             << "\nm_ipConfigMsgACKCounter:      " << (m_ipConfigMsgACKCounter==0                  ? "recieved all config ip ack msgs" : "did not recieved all config ip ack msgs")
             << "\nTimer MFA_IP_CONFIG_MSGS_TIMER: " << (isValidRecAllMsgsTimer ? "valid" : "invalid");

    TRACESTR(eLevelInfoNormal) << "\nCMfaTask::HandleStartupOverIfNeeded m_ipConfigMsgACKCounter: " << m_ipConfigMsgACKCounter;

	// ===== 2. preform operations (if needed)
    if ( (NO  == m_isStartupOverActionsAlreadyPerformed)	&&
            (YES == m_isMediaIpConfigIndReceived)				&&
            (YES == m_isRtmMediaIpConfigIndReceived)			&&
            //In case of fips - we should either wait for FIPS_140 timeout or receive all FIPS_140 indications
            ((FALSE == isValidFips140Timer) || (YES == areAllFips140IndsReceived)) &&
            (m_ipConfigMsgACKCounter <=0 ) )
    {

        if (NO == m_isMediaConfigurationCompletedIndReceived)
        {
            TRACESTR(eLevelInfoNormal) << "\nCMfaTask::HandleStartupOverIfNeeded we need to wait to CM_MEDIA_CONFIGURATION_COMPLETED_IND msg from CM";
            SendMessageToMfa(m_boardId, m_subBoardId, CM_MEDIA_CONFIGURATION_COMPLETED_REQ, 0, NULL);

        }
        else
        {
            m_isStartupOverActionsAlreadyPerformed = YES;

            //Relevant only in case fo FIPS.
            SendFips140ActiveAlarmIfNeeded();
            DeleteTimer(MFA_STARTUP_TIMER);
            DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
            RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_CARD_STARTUP_FAILURE, displayBoardId);
            HandleStartupOver(callerStr);

            // calculate PQs states for deciding on Mfa Startup completion
            CalculateTotalPQsState();

            TRACESTR(eLevelInfoNormal) << "\nCMfaTask::HandleStartupOverIfNeeded (caller: " << callerStr.GetString() << ") - done"
            << condsStr.GetString();
        }
    }

    else
    {
        TRACESTR(eLevelInfoNormal) << "\nCMfaTask::HandleStartupOverIfNeeded (caller: " << callerStr.GetString() << ") - not done"
        << condsStr.GetString();
    }
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::HandleStartupOver(CSmallString callerStr)
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::HandleStartupOver (caller: " << callerStr.GetString() << ")";

	// ===== 1. produce IveMount fault
	ProduceActiveAlarmIvrMountIfNeeded();

	// ===== 2. start KeepAlive polling
//	InitUnitsFailuresList();

// 30.07.06: no initiation; first keepAlive is treated separately
//	InitKeepAliveStruct();
	OnTimerKeepAliveSendTimeout();

    m_pProcess->SendSnmpMfaInterface();


}

//////////////////////////////////////////////////////////////////////
void CMfaTask::ProduceActiveAlarmIvrMountIfNeeded()
{
/*
	// to be inserted when field 'IvrMount' is implemented in CardMngrLoaded structure
	if ( eOk != m_cardMngr.GetIvrMountReadStatus() )
	{
 		AddActiveAlarmMfa( "ProduceActiveAlarmIvrMountIfNeeded", FAULT_CARD_SUBJECT, AA_IVR_FOLDER_MOUNTING_FAILED,
		                   MAJOR_ERROR_LEVEL, "", true, true, 0, m_boardId, 0, FAULT_TYPE_MPM );

	}
*/
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::OnTimerKeepAliveSendTimeout()
{

//old implementation of SlotsNumbering feature
//	SendMpmKeepAliveRequestToMplApi(CM_KEEP_ALIVE_REQ, m_boardId, m_subBoardId);
	SendMpmKeepAliveRequestToMplApi(CM_KEEP_ALIVE_REQ);
	StartTimer(KEEP_ALIVE_TIMER_RECEIVE, m_keepAliveReceivePeriod*SECOND);
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnTimerKeepAliveSendTimeout - Send CM_KEEP_ALIVE_REQ";
}


//////////////////////////////////////////////////////////////////////
//old implementation of SlotsNumbering feature
//void CMfaTask::SendMpmKeepAliveRequestToMplApi(OPCODE theOpcode, DWORD boardId, DWORD subBoardId)
void CMfaTask::SendMpmKeepAliveRequestToMplApi(OPCODE theOpcode)
{
	SystemGetTime(m_lastKeepAliveReqTime);

	SendMessageToMfa(m_boardId, m_subBoardId, theOpcode, 0, NULL);
}

//////////////////////////////////////////////////////////////////////
/*
void CMfaTask::InitKeepAliveStruct()
{
	APIU8 postResults[MAX_NUM_OF_UNITS];
	memcpy(&postResults, m_cardMngr.GetPostResultsList(), MAX_NUM_OF_UNITS);

	for (int i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		m_keepAliveStructure.statusOfUnitsList[i] = (APIU32)(postResults[i]);
	}
}
*/
//////////////////////////////////////////////////////////////////////
void CMfaTask::OnShmMfaFailureInd(CSegment* pSeg)
{
	WORD  subBoardId     = FIRST_SUBBOARD_ID;
	DWORD compStatus     = 0,
	      problemBitMask = 0;

	// ===== 1. get data from segment
	*pSeg >> compStatus;
	*pSeg >> problemBitMask;
	*pSeg >> subBoardId;

	// ===== 2. print to log file
	CLargeString errStr = "\nCMfaTask::OnShmMfaFailureInd";
	errStr << "\nBoardId: "         << m_boardId << ", subBoardId: " << subBoardId//m_subBoardId
	       << ". Status: "			<< compStatus
	       << ", problem bitmask: "	<< problemBitMask;

	// ===== 3. treat Mfa accordingly
	TRACESTR(eLevelInfoNormal) << errStr.GetString();

	// ===== 4. check if card was removed
    if ((problemBitMask & SM_COMP_POWER_OFF_BITMASK) && ((eSmComponentOk == compStatus))) //powr_off (Hot Swap)
    {
		// remove ActiveAlarm (if exists)
    	RemoveAllActiveAlarms("OnShmMfaFailureInd");

		DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
		AddActiveAlarmMfa( "OnShmMfaFailureInd", FAULT_CARD_SUBJECT, AA_POWER_OFF_PROBLEM, MAJOR_ERROR_LEVEL,
                            "Power off problem", true, true, displayBoardId/*as 'userId' (token)*/, displayBoardId, 0, FAULT_TYPE_MPM );

		eCardType cardType = m_cardMngr.GetType();
    	m_pProcess->RemoveCard(m_boardId, subBoardId, cardType);

    	// ===== 5. remove irrelevant Alert
        bool isMpmExists		= m_pProcess->IsMpmCardExistInDB(),
		     isMpmPlusExists	= m_pProcess->IsMpmPlusCardExistInDB(),
		     isBreezeExists     = m_pProcess->IsBreezeCardExistInDB();

        eSystemCardsMode curMode = m_pProcess->GetSystemCardsModeCur();

        if ( ((eSystemCardsMode_mpm == curMode)			&& (false == isMpmPlusExists || false == isBreezeExists)) ||
        	 ((eSystemCardsMode_mpm_plus == curMode)	&& (false == isMpmExists || false == isBreezeExists)) ||
			 ((eSystemCardsMode_breeze == curMode)  	&& (false == isMpmExists || false == isMpmPlusExists)) )
        {
        	RemoveActiveAlarmByErrorCodeUserId(AA_CARDS_CONFIG_EVENT, MPM_BLOCKING_ALERT_USER_ID);
        	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnShmMfaFailureInd - Alert was removed";
        }



    	SetSelfKill();
    }

    if ( (eSmComponentOk == compStatus) && (problemBitMask == 0 /* SM_COMP_CLEAR_BITMASK*/))
	{
		DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
		RemoveSmActiveAlarms("OnShmMfaFailureInd", displayBoardId);
	}
    else if ( (eSmComponentOk == compStatus) && (problemBitMask & SM_COMP_RTM_OR_ISDN_MISSING_BITMASK))
    {
    	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, RTM_ISDN_SUBBOARD_ID);
    	RemoveActiveAlarmMfaByErrorCodeUserId("OnShmMfaFailureInd", AA_RTM_LAN_OR_ISDN_MISSING,		displayBoardId);

    }

	else if (eSmComponentOk != compStatus)// compStatus != ok
	{
		BOOL isResetting = FALSE;

//		   if (problemBitMask & SM_COMP_POWER_OFF_BITMASK)///powr_off (Hot Swap)
//		   {
//			    m_pProcess->RemoveCard(m_boardId,subBoardId);
//			    SetSelfKill();
//		   }

//////////////////////////
// Temp: until ShelfMngr will send the right status of the MFA!!
//		if ( eShmComponentResetting == compStatus)
//			isResetting = TRUE;

		BOOL isFailureProblem = UpdateActiveAlarmsIfNeeded(problemBitMask, isResetting); // [ printing to log file is performed within 'UpdateActiveAlarmsIfNeeded' method ]

		// ===== 5. in Failure problem: notify rsrcAllocator and ask reset from Daemon
		if (TRUE == isFailureProblem)
		{
			CSmallString errStr = "SmMfaFailure - ";
			errStr << "boardId: "			<< m_boardId
			       << ". Status: "			<< compStatus
			       << ", problem bitmask: "	<< problemBitMask;

			// update Resource process
			SendKeepAliveFailureToRsrcAlloc();

			COsQueue& rcvMbx = GetRcvMbx();
			m_pProcess->TurnMfaTaskToZombie(&rcvMbx); // no messages will be received by this task

			// ask for reset
			m_pProcess->SendResetReqToDaemon(errStr);
		} // end  isFailureProblem

	} // end  compStatus != ok
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::RemoveSmActiveAlarms(CSmallString callerStr, DWORD boardId)
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::RemoveSmActiveAlarms - AA_VOLTAGE_PROBLEM" << " boardId "<< boardId;

	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_VOLTAGE_PROBLEM,				boardId);

	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, RTM_ISDN_SUBBOARD_ID);
	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_RTM_LAN_OR_ISDN_MISSING,		displayBoardId);
	/**************************************************************************************/
	/*28.4.10 added by rachel Cohen                                                       */
	/*in case the conversiontable arrived in delay and the active alarm is on the board id*/
	/* need to init the conversion table with legal values according the product type     */
	/**************************************************************************************/
	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_NO_LAN_CONNECTION_PORT_1,				displayBoardId);
	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_NO_LAN_CONNECTION_PORT_1,				boardId);
	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_NO_LAN_CONNECTION_PORT_2,				displayBoardId);
	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_NO_LAN_CONNECTION_PORT_2,				boardId);
	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_TEMPERATURE_MAJOR_PROBLEM,		boardId);
	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_TEMPERATURE_CRITICAL_PROBLEM,	boardId);
	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_FAILURE_PROBLEM,				boardId);
	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_POWER_OFF_PROBLEM,				boardId);
	// fault only
	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_OTHER_PROBLEM,					boardId, TRUE);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::RemoveAllActiveAlarms(CSmallString callerStr)
{
	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);

	RemoveSmActiveAlarms(callerStr, displayBoardId);

	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_NO_CONNECTION_WITH_CARD,	displayBoardId);
  RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_CARD_STARTUP_FAILURE,		displayBoardId);
	RemoveActiveAlarmMfaByErrorCode(callerStr, AA_MEDIA_IP_CONFIGURATION_FAILURE);

	for (int i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		// AA_UNIT_NOT_RESPONDING - Fault only
		RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_UNIT_NOT_RESPONDING, i, TRUE);
	}

	RemoveAllUserMsgEmbActiveAlarms(callerStr);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::RemoveAllUserMsgEmbActiveAlarms(CSmallString callerStr)
{
	// ===== 1. eUserMsgMessage_Emb_Test
	DWORD identifier_test	= m_pProcess->CreateIdFromBoardIdMsgId( m_boardId,
																	(int)eUserMsgMessage_Emb_Test,
																	(int)eUSerMsgAutoRemoval_cardRemove );

	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_EXTERNAL_ALERT_EMB, identifier_test);


	// ===== 2. eUserMsgMessage_Emb_UBootFlashFailure
	DWORD identifier_UBoot	= m_pProcess->CreateIdFromBoardIdMsgId( m_boardId,
																	(int)eUserMsgMessage_Emb_UBootFlashFailure,
																	(int)eUSerMsgAutoRemoval_cardRemove );

	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_EXTERNAL_ALERT_EMB, identifier_UBoot);

	// ===== 3. eUserMsgMessage_Emb_FpgaVersLoadesFailure
	DWORD identifier_Fpga	= m_pProcess->CreateIdFromBoardIdMsgId( m_boardId,
																	(int)eUserMsgMessage_Emb_FpgaVersLoadesFailure,
																	(int)eUSerMsgAutoRemoval_cardRemove );

	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_EXTERNAL_ALERT_EMB, identifier_Fpga);
}

//////////////////////////////////////////////////////////////////////
BOOL CMfaTask::UpdateActiveAlarmsIfNeeded(DWORD problemBitmask, BOOL isResetting)
{
	BOOL isFailureProblem     = FALSE,
	     isNewSpecificProblem = FALSE,
	     isNewProblem         = FALSE;

	// prepare string to log file
	CMedString errStr = "\nCMfaTask::UpdateActiveAlarmsIfNeeded\n";
	errStr << "MFA boardId " << m_boardId << " is not ok; problem in ";

	// ===== Voltage problem
	isNewSpecificProblem = UpdateSpecificActiveAlarm(SM_COMP_VOLTAGE_BITMASK, problemBitmask, AA_VOLTAGE_PROBLEM, "Voltage problem");
	if (TRUE == isNewSpecificProblem)
	{
		isNewProblem = TRUE;
		errStr << "- Voltage ";
	}

	isNewSpecificProblem = UpdateSpecificActiveAlarm(SM_COMP_RTM_OR_ISDN_MISSING_BITMASK,
                                                   problemBitmask,
                                                   AA_RTM_LAN_OR_ISDN_MISSING,
                                                   GetAlarmDescription(AA_RTM_LAN_OR_ISDN_MISSING));
	if (TRUE == isNewSpecificProblem)
	{
		isNewProblem = TRUE;
		errStr << "- No Lan Card ";
	}

	// ===== Temperature major problem
	isNewSpecificProblem = UpdateSpecificActiveAlarm(SM_COMP_TEMPERATURE_MAJOR_BITMASK, problemBitmask, AA_TEMPERATURE_MAJOR_PROBLEM, "Temperature problem - Major");
	if (TRUE == isNewSpecificProblem)
	{
		isNewProblem = TRUE;
		errStr << "- Temperature major ";
	}

	// ===== Temperature critical problem
	isNewSpecificProblem = UpdateSpecificActiveAlarm(SM_COMP_TEMPERATURE_CRITICAL_BITMASK, problemBitmask, AA_TEMPERATURE_CRITICAL_PROBLEM, "Temperature problem - Critical");
	if (TRUE == isNewSpecificProblem)
	{
		isNewProblem = TRUE;
		errStr << "- Temperature critical ";
	}

	isNewSpecificProblem = UpdateSpecificActiveAlarm(SM_COMP_POWER_OFF_BITMASK, problemBitmask, AA_POWER_OFF_PROBLEM, "Power off problem");
	if (TRUE == isNewSpecificProblem)
	{
		isNewProblem = TRUE;
		errStr << "- Power off ";
	}

	// ===== Reset problem
	if (TRUE == isResetting)
	{
		isNewSpecificProblem = AddResettingActiveAlarm();
		if (TRUE == isNewSpecificProblem)
		{
			isNewProblem     = TRUE;
			isFailureProblem = TRUE;
			errStr << "- Resetting ";
		}
	}
	else
	{
		isNewSpecificProblem = UpdateSpecificActiveAlarm(SM_COMP_FAILURE_BITMASK, problemBitmask, AA_FAILURE_PROBLEM, "Failure problem");
		if (TRUE == isNewSpecificProblem)
		{
			isNewProblem     = TRUE;
			isFailureProblem = TRUE;
			errStr << "- Failure ";
		}
	}


	// ===== not any known problem
	isNewSpecificProblem = UpdateSpecificActiveAlarm(SM_COMP_OTHER_BITMASK, problemBitmask, AA_OTHER_PROBLEM, "Unspecified problem", TRUE);
	if (TRUE == isNewSpecificProblem)
	{
		isNewProblem = TRUE;
		errStr << "- Other ";
	}


	// print to logger (if needed)
	if (TRUE == isNewProblem)
	{
		TRACESTR(eLevelInfoNormal) << errStr.GetString();
	}

	return isFailureProblem;
}

//////////////////////////////////////////////////////////////////////
BOOL CMfaTask::UpdateSpecificActiveAlarm(APIU32 specificBitMask, APIU32 problemBitmask, WORD errCode, string description, BOOL isFaultOnly)
{
	DWORD displayBoardId;
	BOOL isNewSpecificProblem = FALSE;

	if ( specificBitMask & problemBitmask )
	{
		bool isActiveAlarmOrFaultExist;

		if (!isFaultOnly) {
			isActiveAlarmOrFaultExist = IsActiveAlarmExistByErrorCode(errCode);
		}
		else {
			isActiveAlarmOrFaultExist = IsActiveAlarmFaultOnlyExistByErrorCode(errCode);
		}

		if ( false == isActiveAlarmOrFaultExist )
		{
			isNewSpecificProblem = TRUE;

			if (specificBitMask == SM_COMP_RTM_OR_ISDN_MISSING_BITMASK)
				displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, RTM_ISDN_SUBBOARD_ID);
			else
			   displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
			if (!isFaultOnly)
			{
				AddActiveAlarmMfa( "UpdateSpecificActiveAlarm", FAULT_CARD_SUBJECT, errCode,
								   MAJOR_ERROR_LEVEL, description, true, true, displayBoardId, displayBoardId, 0, FAULT_TYPE_MPM );
			}
			else
			{
				AddActiveAlarmFaultOnlyMfa( "UpdateSpecificActiveAlarm", FAULT_CARD_SUBJECT, errCode,
								   MAJOR_ERROR_LEVEL, description, displayBoardId, displayBoardId, 0, FAULT_TYPE_MPM );

			}
		}
	}
	else
	{
		RemoveActiveAlarmMfaByErrorCode("UpdateSpecificActiveAlarm", errCode, isFaultOnly);
	}

	return isNewSpecificProblem;
}

//////////////////////////////////////////////////////////////////////
BOOL CMfaTask::AddResettingActiveAlarm()
{
	BOOL isNewResettingProblem = FALSE;

	if ( false == IsActiveAlarmExistByErrorCode(AA_FAILURE_PROBLEM) )
	{
		isNewResettingProblem = TRUE;

		DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
		AddActiveAlarmMfa( "UpdateResettingActiveAlarm", FAULT_CARD_SUBJECT, AA_FAILURE_PROBLEM,
		                   MAJOR_ERROR_LEVEL, "Resetting card", true, true, 0, displayBoardId, 0, FAULT_TYPE_MPM );
	}

	return isNewResettingProblem;
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::TreatNoLanActiveAlarm(ETH_SETTINGS_SPEC_S* pEthSetting)
{

	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	DWORD displayBoardId = pEthSetting->portParams.slotId ; //the slotid has the value of the displayboard id

	//Last port status was active but in ethernet setting arrived port NOT ACTIVE
	if ( (pEthSetting->monitoringParams.ulActLinkStatus == PORT_INACTIVE))
	{


		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::TreatNoLanActiveAlarm"
				<< "\n PORT NOT ACTIVE" << "\n Slotid = "
				<< pEthSetting->portParams.slotId
				<< "\n port " << (int)pEthSetting->portParams.portNum;

		BOOL bLAN_REDUNDANCY = YES;
		sysConfig->GetBOOLDataByKey(CFG_KEY_LAN_REDUNDANCY, bLAN_REDUNDANCY);
		BOOL isV35JitcSupport = NO;
		BOOL isJITCMode = NO;
		sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, isV35JitcSupport);
		sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, isJITCMode);
		//BRIDGE-5887
		if(isJITCMode && isV35JitcSupport)
		{
			 bLAN_REDUNDANCY = NO;
		}

		DWORD phySlotId = pEthSetting->portParams.slotId;
		if (phySlotId >=13) phySlotId = (phySlotId % 13 )+1;

		eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();

		if (((pEthSetting->portParams.portNum == MEDIA_CARD_LAN_PORT_1) && (bLAN_REDUNDANCY == YES)) ||
				((curProductType != eProductTypeRMX1500) && (pEthSetting->portParams.portNum == MEDIA_CARD_LAN_PORT_1) && (true == m_pProcess->IsRtmIsdnCard(phySlotId,2))))


		{

			if (false == IsActiveAlarmExistByErrorCodeUserId(AA_NO_LAN_CONNECTION_PORT_1,displayBoardId))
			{


				// if ISDN_LAN connected and system config is NO - don't add that active alarm
				BOOL IsEnableActiveAlarm = FALSE;
				std::string key = "ENABLE_ACTIVE_ALARM_NO_LAN";
				sysConfig->GetBOOLDataByKey(key, IsEnableActiveAlarm);


				if  (!IsEnableActiveAlarm  /*&& m_pProcess->IsRtmIsdnCardExistInDB()*/)
					return;


				AddActiveAlarmMfa( "TreatNoLanActiveAlarm", FAULT_CARD_SUBJECT, AA_NO_LAN_CONNECTION_PORT_1,
						MAJOR_ERROR_LEVEL, "No LAN connection: Port 1", true, true, displayBoardId, displayBoardId, 0, FAULT_TYPE_MPM );

				BOOL isAAExist = SetPortAAAndReturnIfAnyAAExist(pEthSetting->portParams.portNum, TRUE);
				if (!isAAExist)
				{
					// first failure on this board
					SetPortAAAndReturnIfAnyAAExist(pEthSetting->portParams.portNum, FALSE);
				}
				// else // also the other port is bad  no need to report

				TRACESTR(eLevelInfoNormal) << "\nCMfaTask::TreatNoLanActiveAlarm "
						<< "ADD ACTIVE ALARM port 1. board id: " << m_boardId << "port id " << (int)pEthSetting->portParams.portNum << " isAAExist " << (int)isAAExist;

			}
		}
		else if ((pEthSetting->portParams.portNum == MEDIA_CARD_LAN_PORT_2) &&
				(false == IsActiveAlarmExistByErrorCodeUserId(AA_NO_LAN_CONNECTION_PORT_2,displayBoardId)))
		{

			// if ISDN_LAN connected and system config is NO - don't add that active alarm
			BOOL IsEnableActiveAlarm = FALSE;
			std::string key = "ENABLE_ACTIVE_ALARM_NO_LAN";
			sysConfig->GetBOOLDataByKey(key, IsEnableActiveAlarm);


			if  (!IsEnableActiveAlarm  /*&& m_pProcess->IsRtmIsdnCardExistInDB()*/)
				return;


			TRACESTR(eLevelInfoNormal) << "\nCMfaTask::TreatNoLanActiveAlarm"
					<< "ADD ACTIVE ALARM port 2. board id: " << m_boardId << "port id " << (int)pEthSetting->portParams.portNum;

			AddActiveAlarmMfa( "TreatNoLanActiveAlarm", FAULT_CARD_SUBJECT, AA_NO_LAN_CONNECTION_PORT_2,
					MAJOR_ERROR_LEVEL, "No LAN connection: Port 2", true, true, displayBoardId, displayBoardId, 0, FAULT_TYPE_MPM );

			BOOL isAAExist = SetPortAAAndReturnIfAnyAAExist(pEthSetting->portParams.portNum, TRUE);
			if (!isAAExist)
			{
				// first failure on this board
				m_pProcess->SendSnmpLinkStatus(m_boardId, pEthSetting->portParams.portNum, FALSE);
			}
			// else // also the other port is bad  no need to report


		}
	}

	else  //port active
	{
		BOOL SNMPUpdated = FALSE;
		if ((pEthSetting->portParams.portNum == MEDIA_CARD_LAN_PORT_1) &&
				(true == IsActiveAlarmExistByErrorCodeUserId(AA_NO_LAN_CONNECTION_PORT_1,displayBoardId)))

		{
			RemoveActiveAlarmMfaByErrorCodeUserId("CMfaTask::TreatNoLanActiveAlarm", AA_NO_LAN_CONNECTION_PORT_1,displayBoardId);
			BOOL isAAExist = SetPortAAAndReturnIfAnyAAExist(pEthSetting->portParams.portNum, FALSE);
			if (!isAAExist)
			{
				SNMPUpdated = TRUE;
				m_pProcess->SendSnmpLinkStatus(m_boardId, pEthSetting->portParams.portNum, TRUE);
			}
			// else still other port is bad

		}
		else if ((pEthSetting->portParams.portNum == MEDIA_CARD_LAN_PORT_2) &&
				(true == IsActiveAlarmExistByErrorCodeUserId(AA_NO_LAN_CONNECTION_PORT_2,displayBoardId)))
		{

			RemoveActiveAlarmMfaByErrorCodeUserId("CMfaTask::TreatNoLanActiveAlarm", AA_NO_LAN_CONNECTION_PORT_2,displayBoardId);
			BOOL isAAExist = SetPortAAAndReturnIfAnyAAExist(pEthSetting->portParams.portNum, FALSE);
			if (!isAAExist)
			{
				SNMPUpdated = TRUE;
				m_pProcess->SendSnmpLinkStatus(m_boardId, pEthSetting->portParams.portNum, TRUE);
			}
			// else still other port is bad
		}

		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::TreatNoLanActiveAlarm"
				<< "\n PORT ACTIVE"
				<< pEthSetting->portParams.slotId
				<< "\n port " << (int)pEthSetting->portParams.portNum << " SNMPUpdated " << (int)SNMPUpdated ;

	}

}

//////////////////////////////////////////////////////////////////////
void CMfaTask::OnCardsNewMediaIpInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnCardsNewMediaIpInd; boardId: "<< m_boardId;

	// ===== 1. get the parameters from the structure received into process's attribute
	DWORD boardId=0, subBoardId=0;
	*pSeg >> boardId
	      >> subBoardId;
	MEDIA_IP_PARAMS_S* pNewMediaIpStruct = (MEDIA_IP_PARAMS_S*)pSeg->GetPtr(1);

	// ===== 2. Set vlan data (need only under jitc, separated mode)
	CMediaIpParameters NewMediaIpPArams(*pNewMediaIpStruct);

	DWORD PQ_Id = NewMediaIpPArams.GetIpParams().interfacesList[0].pqId;
	SetVlanData(PQ_Id, pNewMediaIpStruct, FALSE);

	// ===== 3. if it's a service from the suitable boardID and subBoardId...
	if ( (boardId == m_boardId) && (subBoardId == m_subBoardId) )
	{
		// ===== ...then print it to trace...
		TRACESTR(eLevelInfoNormal)
			<< "\nNew MediaIpParams received (boardId: " << m_boardId << "):"
			<< "\n==========================\n"
			<< NewMediaIpPArams;

		if (YES == m_isUnitLoadedIndReceived)
		{
			// ===== ...and send it to MplApi (and from there to MFA) if possible
			TRACESTR(eLevelInfoNormal)
			    << "\nNew MediaIpParams sent to Mfa (boardId: " << m_boardId << "):"
				<< "\n=============================\n"
				<< NewMediaIpPArams;


			SendMessageToMfa(m_boardId, m_subBoardId, CM_MEDIA_IP_CONFIG_REQ, sizeof(MEDIA_IP_PARAMS_S), (char*)pNewMediaIpStruct );

			HandleV35GWConfiguration(&NewMediaIpPArams);
		}

		else // UnitLoadedInd hasn't been received yet
		{
			TRACESTR(eLevelInfoNormal)
			    << "\nNew MediaIpParams are not sent to Mfa (boardId: " << m_boardId << "), since UnitLoadedInd has not been received yet";
		}
	}
}

void CMfaTask::SendDnsMediaConfigReqToMplApi()
{
	CM_DNS_MEDIA_CONFIG_S dnsMediaConfig;
	memset(&dnsMediaConfig, 0, sizeof(CM_DNS_MEDIA_CONFIG_S));
	std::ifstream resolv_conf("/etc/resolv.conf");

	std::string line_with_domain;
	std::string domainStr = "search ";
	while(std::getline(resolv_conf, line_with_domain))
	{
		TRACESTR(eLevelInfoNormal) << "\nNew SendDnsMediaConfigReqToMplApi line_with_domain before = " << line_with_domain.c_str();
		std::size_t found = line_with_domain.find(domainStr);
		if (found!=std::string::npos)
		{
			line_with_domain.replace(found, domainStr.length(),"");
			//pService->GetpDns()->SetDomainName(line_with_domain.c_str());
			TRACESTR(eLevelInfoNormal) << "\nNew SendDnsMediaConfigReqToMplApi line_with_domain after = " << line_with_domain.c_str();
			size_t sizeToCopy = min((size_t)MAX_DNS_DOMAIN_NAME-1, strlen(line_with_domain.c_str()));
			 memcpy(&dnsMediaConfig.domainName, line_with_domain.c_str(), sizeToCopy);
			 dnsMediaConfig.domainName[sizeToCopy ] = '\0';
			break;
		}
	}
	std::string strFromFile;
	int idxIpAddress = 0;
	while (!resolv_conf.eof() && !resolv_conf.fail())
	{
		resolv_conf >> strFromFile;
		TRACESTR(eLevelInfoNormal) << "\nNew SendDnsMediaConfigReqToMplApi idxIpAddress = " << idxIpAddress << " strFromFile = " << strFromFile.c_str();
		if (isIpV4Str(strFromFile.c_str()) || isIpV6Str(strFromFile.c_str()))
		{
			size_t sizeToCopy = min((size_t)MAX_IP_LENGTH-1, strlen(strFromFile.c_str()));
			memcpy(&dnsMediaConfig.IpServer[idxIpAddress], strFromFile.c_str(), sizeToCopy);
			dnsMediaConfig.IpServer[idxIpAddress][sizeToCopy] = '\0';

			idxIpAddress++;
		}

	}
	resolv_conf.close();

	TRACESTR(eLevelInfoNormal) << "\nNew SendDnsMediaConfigReqToMplApi domain" <<dnsMediaConfig.domainName
			<< " ip1 = "
			<< dnsMediaConfig.IpServer[0]
			<< " ip2 = "
			<< dnsMediaConfig.IpServer[1]
			<< " ip2 = "
			<< dnsMediaConfig.IpServer[2];

	SendMessageToMfa(m_boardId,
			m_subBoardId,
			CM_DNS_MEDIA_CONFIG_REQ,
			sizeof(CM_DNS_MEDIA_CONFIG_S),
			(char*)&dnsMediaConfig);


}
//////////////////////////////////////////////////////////////////////
void CMfaTask::OnIvrMusicAddSourceReqReceivedInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnIvrMusicAddSourceReqReceivedInd (boardId: "<< m_boardId << ")";

	if (NO == m_isIvrMusicSourceReqAlreaySentToMpm)
	{
		SendIvrMusicAddSourceReqToMplApi();
	}
}

//////////////////////////////////////////////////////////////////////
//Vlan data is needed under (JITC Mode + Separated networks)
void CMfaTask::SetVlanData(DWORD PQ_Id, MEDIA_IP_PARAMS_S* pNewMediaIpStruct, BOOL bNoVlan)
{
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL isJITCMode = NO;
	BOOL isManagmentSep = NO;
	BOOL isV35JitcSupport = NO;
	BOOL isRMX2000RtmLan = NO;
	sysConfig->GetBOOLDataByKey("ULTRA_SECURE_MODE", isJITCMode);
	sysConfig->GetBOOLDataByKey(CFG_KEY_SEPARATE_NETWORK, isManagmentSep);
	sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, isV35JitcSupport);
	

    //NGB request
	if (m_cardMngr.GetType() == eMpmRx_Half || m_cardMngr.GetType() == eMpmRx_Half )
		isRMX2000RtmLan = YES;
        else
           sysConfig->GetBOOLDataByKey(CFG_KEY_RMX2000_RTM_LAN, isRMX2000RtmLan);


	DWORD V35GwIpv4 = pNewMediaIpStruct->ipParams.v35GwIpv4Address;

	//if (isJITCMode && isV35JitcSupport && PQ_Id==2 && bNoVlan)
	if ((isJITCMode && isV35JitcSupport && bNoVlan&& (0 != V35GwIpv4)) || (isJITCMode && isManagmentSep && isRMX2000RtmLan))
	{
		pNewMediaIpStruct->ipParams.networkParams.vLanMode = 0;
		pNewMediaIpStruct->ipParams.networkParams.vLanId = 0;
		TRACESTR(eLevelInfoNormal) << "MfaTask::SetVlanData - For board id = "<<m_boardId<<" and PQID = "<<PQ_Id<<" the vLanMode = "<<pNewMediaIpStruct->ipParams.networkParams.vLanMode<<" and vLanId = "<<pNewMediaIpStruct->ipParams.networkParams.vLanId;
	}
	else if(isJITCMode && isManagmentSep && !isRMX2000RtmLan)
	{
		pNewMediaIpStruct->ipParams.networkParams.vLanMode = 1;
		pNewMediaIpStruct->ipParams.networkParams.vLanId = CS_NETWORK_INTERFACE_SEPERATED_NUMBER;
		TRACESTR(eLevelInfoNormal) << "MfaTask::SetVlanData - For board id = "<<m_boardId<<" and PQID = "<<PQ_Id<<" the vLanMode = "<<pNewMediaIpStruct->ipParams.networkParams.vLanMode<<" and vLanId = "<<pNewMediaIpStruct->ipParams.networkParams.vLanId;
	}
	else
	{
		pNewMediaIpStruct->ipParams.networkParams.vLanMode = 0;
		TRACESTR(eLevelInfoNormal) << "MfaTask::SetVlanData - For board id = "<<m_boardId<<" and PQID = "<<PQ_Id<<" the vLanMode = "<<pNewMediaIpStruct->ipParams.networkParams.vLanMode;
	}

}

//////////////////////////////////////////////////////////////////////
//Vlan data is needed under Multiple services.
void CMfaTask::SetVlanDataForMultiServices(DWORD PQ_Id, MEDIA_IP_PARAMS_S* pMediaIpStruct)
{
	if(IsMultipleServicesMode() || IsV35JITC())
	{
		pMediaIpStruct->ipParams.networkParams.vLanMode = 1;
		pMediaIpStruct->ipParams.networkParams.vLanId = CalcMSvlanId(m_boardId, PQ_Id);
	}
	else
		pMediaIpStruct->ipParams.networkParams.vLanMode = 0;
}

//////////////////////////////////////////////////////////////////////
//Internal CS ipV4 address is needed to setup NAT rules in media card, under Multiple services mode
void CMfaTask::SetCSInternalIPv4(DWORD PQ_Id, MEDIA_IP_PARAMS_S* pMediaIpStruct)
{
	// 1. Set CS address in the VLAN
	DWORD vlanId = CalcMSvlanId(m_boardId, PQ_Id);
	DWORD ipAddress	= GetVlanCSInternalIpv4Address(vlanId);

	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SetCSInternalIPv4, vlan=" << vlanId << " , ip=" << ipAddress;

	pMediaIpStruct->ipParams.interfacesList[0].iPv4Internal.iPv4Address	= ipAddress;
}

//////////////////////////////////////////////////////////////////////
//External CS ipV4 address is needed to setup NAT rules in media card, under Multiple services mode
void CMfaTask::SetCSExternalIPv4(CMediaIpParameters* mediaIpParamsFromVector, MEDIA_IP_PARAMS_S* pMediaIpStruct)
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SetCSExternalIPv4 - CSip = "<<mediaIpParamsFromVector->GetMediaIpParamsStruct()->csIp;
	//DWORD mediaIp	= SystemIpStringToDWORD("200.10.0.57");
	pMediaIpStruct->ipParams.networkParams.subnetMask				= mediaIpParamsFromVector->GetMediaIpParamsStruct()->ipParams.networkParams.subnetMask;//SystemIpStringToDWORD("255.255.255.0");
	pMediaIpStruct->ipParams.networkParams.defaultGateway			= mediaIpParamsFromVector->GetMediaIpParamsStruct()->ipParams.networkParams.defaultGateway;//SystemIpStringToDWORD("200.10.0.1");
	pMediaIpStruct->ipParams.interfacesList[0].iPv4.iPv4Address		= mediaIpParamsFromVector->GetMediaIpParamsStruct()->csIp;
	//pMediaIpStruct->csIp = mediaIpParamsFromVector->GetMediaIpParamsStruct()->csIp;

}

//////////////////////////////////////////////////////////////////////
//Internal media card ipV4 address is needed under Multiple services mode
void CMfaTask::SetMediaCardInternalIPv4MultiServices(DWORD PQ_Id, MEDIA_IP_PARAMS_S* pMediaIpStruct)
{
	DWORD vlanId = CalcMSvlanId(m_boardId, PQ_Id);
	DWORD ipAddress	= GetVlanCardInternalIpv4Address(vlanId);

	char ipAddressStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString(ipAddress, ipAddressStr);

	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SetMediaCardInternalIPv4MultiServices, vlan=" << vlanId << " , ip=" << ipAddressStr;

	pMediaIpStruct->ipParams.interfacesList[0].iPv4.iPv4Address		= ipAddress;

	pMediaIpStruct->ipParams.networkParams.subnetMask				= SystemIpStringToDWORD("255.255.255.0");
}

//////////////////////////////////////////////////////////////////////
DWORD CMfaTask::AddActiveAlarmMfa( CSmallString callerStr, BYTE subject, DWORD errorCode, BYTE errorLevel,
                                   string description, bool isForEma, bool isForFaults,
                                   DWORD userId, DWORD boardId, DWORD unitId, WORD theType, BOOL doesFlush/*=TRUE*/)
{
	bool isActiveAlarmExist = IsActiveAlarmExistByErrorCodeUserId(errorCode, userId);

	if (!isActiveAlarmExist)
	{
		// ===== 1. add ActiveAlarm
		if(doesFlush)
		{
			AddActiveAlarm( subject, errorCode, errorLevel, description,
							isForEma, isForFaults, userId, boardId, unitId, theType );
		}
		else
		{
			AddActiveAlarmNoFlush( subject, errorCode, errorLevel, description,
								   isForEma, isForFaults, userId, boardId, unitId, theType );
		}
		// ===== 2. add status (in Cards' DB)
		//m_pProcess->GetCardsMonitoringDB()->AddStatus(subject, errorCode, errorLevel, boardId, unitId);
		//NEW_STATUS_LIST
		m_pProcess->GetCardsMonitoringDB()->AddStatusNew( subject, errorCode, errorLevel, userId,
		                                                  boardId, m_subBoardId, unitId, description, theType );


		// ===== 3. update card's status
		UpdateCardStateAccordingToTaskState(callerStr);
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "AA already exists errorCode " << errorCode << " userId " << userId;
	}

    return 0;
}
//////////////////////////////////////////////////////////////////////
DWORD CMfaTask::AddActiveAlarmFaultOnlyMfa( CSmallString callerStr, BYTE subject, DWORD errorCode, BYTE errorLevel,
									string description,	DWORD userId, DWORD boardId, DWORD unitId, WORD theType)
{

	bool isActiveAlarmFaultOnlyExist = IsActiveAlarmFaultOnlyExistByErrorCodeUserId(errorCode, userId);

	if (!isActiveAlarmFaultOnlyExist)
	{
		// ===== 1. add ActiveAlarm Fault Only
		AddActiveAlarmFaultOnly( subject, errorCode, errorLevel, description,
										userId, boardId, unitId, theType );

		// ===== 2. add status (in Cards' DB)
		//m_pProcess->GetCardsMonitoringDB()->AddStatus(subject, errorCode, errorLevel, boardId, unitId);
		//NEW_STATUS_LIST
		m_pProcess->GetCardsMonitoringDB()->AddStatusNew( subject, errorCode, errorLevel, userId,
		                                                  boardId, m_subBoardId, unitId, description, theType );


		// ===== 3. update card's status
		UpdateCardStateAccordingToTaskState(callerStr);

	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "AA fault only already exists errorCode " << errorCode << " userId " << userId;
	}

    return 0;

}

//////////////////////////////////////////////////////////////////////
void CMfaTask::RemoveActiveAlarmMfaByErrorCode(CSmallString callerStr, WORD errorCode, BOOL isFaultOnly)
{
	bool isActiveAlarmOrFaultExist;

	if (!isFaultOnly) {
		isActiveAlarmOrFaultExist = IsActiveAlarmExistByErrorCode(errorCode);
	}
	else {
		isActiveAlarmOrFaultExist = IsActiveAlarmFaultOnlyExistByErrorCode(errorCode);
	}

	if ( true == isActiveAlarmOrFaultExist )
	{
		if (!isFaultOnly) {
			// ===== 1. remove ActiveAlarm
			RemoveActiveAlarmByErrorCode(errorCode);
		}
		else {
			RemoveActiveAlarmFaultOnlyByErrorCode(errorCode);
		}
		// ===== 2. del status (from Cards' DB)
		//m_pProcess->GetCardsMonitoringDB()->DelStatus(FAULT_CARD_SUBJECT, errorCode, MAJOR_ERROR_LEVEL, m_boardId);
		//NEW_STATUS_LIST
		m_pProcess->GetCardsMonitoringDB()->DelStatusNewByErrorCode(errorCode, m_boardId, m_subBoardId);

		// ===== 3. update card's status
		UpdateCardStateAccordingToTaskState(callerStr);
	}
}
//////////////////////////////////////////////////////////////////////
void CMfaTask::RemoveActiveAlarmMfaByErrorCodeUserId(CSmallString callerStr, WORD errorCode, DWORD userId, BOOL isFaultOnly)
{
	if (errorCode == AA_VOLTAGE_PROBLEM)
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::RemoveActiveAlarmMfaByErrorCodeUserId caller :" << callerStr.GetString() << ":"
		<< " userId " <<userId <<" m_boardId "<<m_boardId << " m_subBoardId "<<m_subBoardId <<" isFaultOnly " << isFaultOnly ;

	bool isActiveAlarmOrFaultExist;

	if (!isFaultOnly) {
		isActiveAlarmOrFaultExist = IsActiveAlarmExistByErrorCodeUserId(errorCode, userId);
	}
	else {
		isActiveAlarmOrFaultExist = IsActiveAlarmFaultOnlyExistByErrorCodeUserId(errorCode, userId);
	}

	if ( isActiveAlarmOrFaultExist )
	{
		if (!isFaultOnly) {
			// ===== 1. remove ActiveAlarm
			RemoveActiveAlarmByErrorCodeUserId(errorCode, userId);
		}
		else {
			RemoveActiveAlarmFaultOnlyByErrorCodeUserId(errorCode, userId);
		}
		m_pProcess->GetCardsMonitoringDB()->DelStatusNewByErrorCodeUserId(errorCode, userId, m_boardId, m_subBoardId);

		// ===== 3. update card's status
		UpdateCardStateAccordingToTaskState(callerStr);
	}
}



void CMfaTask::UpdateCardStateAccordingToTaskState(CSmallString callerStr)
{
	eTaskStatus curTaskStatus = GetTaskStatus();
	eTaskState curTaskState = GetTaskState();
	eCardState  curCardState  = m_pProcess->ConvertTaskStatusToCardState(curTaskStatus, curTaskState);

	// VNGR-26115 - if there is active alarm on 1 of the units - board should be in major state	
	BOOL isUnitNotRespondFaultOnly = IsActiveAlarmFaultOnlyExistByErrorCode(AA_UNIT_NOT_RESPONDING);
	if (isUnitNotRespondFaultOnly)
	{
		curCardState = eMajorError;
	}	

	m_pProcess->GetCardsMonitoringDB()->SetCardState(m_boardId, m_subBoardId, curCardState);

	TRACEINTOFUNC << callerStr.GetString() << ":"
	              << "\nCard on boardId " << m_boardId << ", subBoardId " << m_subBoardId
	              << " - updated its state to " << ::CardStateToString(curCardState) << " isUnitNotRespondFaultOnly " << isUnitNotRespondFaultOnly;
}

void CMfaTask::AddFilterOpcodePoint()
{
	AddFilterOpcodeToQueue(CM_KEEP_ALIVE_IND);
}

void CMfaTask::OnBadSpontIndFromMFA(CSegment* msg)
{
	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();
	if( eProductFamilySoftMcu == curProductFamily )
	{
	    // ===== retrieve data from segment received
	    MFA_BAD_SPONTANEOUS_IND_S dataStruct;
	    memset(&dataStruct, 0, sizeof dataStruct);

	    msg->Get((BYTE*)&dataStruct, sizeof dataStruct);
	
            // ===== add to log & fault
	    const char* desc = (const char*)(dataStruct.badSpontaneousStruct.description);

	    TRACEINTOFUNC << "reason: " << dataStruct.badSpontaneousStruct.reason
			  << ", description: " << desc << ", MfaBoardId: " << dataStruct.MfaBoardId;

	    // Adds the string alarm to the Fault list in the EMA
	    CHlogApi::TaskFault(FAULT_CARD_SUBJECT,
			        BAD_SPONTANEOUS_INDICATION,
			        MAJOR_ERROR_LEVEL,
			        desc,
			        TRUE,
			        dataStruct.MfaBoardId,
			        0,
			        FAULT_TYPE_MPM);
	
		return;
	}

    BYTE board_id = m_pProcess->GetAuthenticationStruct()->switchBoardId;
    BYTE sub_board_id = m_pProcess->GetAuthenticationStruct()->switchSubBoardId;

    // Sends the message to Switch task
    COsQueue* mbx = m_pProcess->GetMfaMbx(board_id, sub_board_id);
    if (NULL == mbx)
        return;

    CTaskApi api;
    api.CreateOnlyApi(*mbx);

    OPCODE opcode = BAD_SPONTANEOUS_IND;

	CSegment*  pParam = new CSegment(*msg);
	*pParam << (DWORD)m_boardId;



    // Uses copy of the message because it is freed in Send
    STATUS stat = api.SendMsg(pParam, opcode);

    FPASSERTSTREAM_AND_RETURN(STATUS_OK != stat,
        "Unable to send " << m_pProcess->GetOpcodeAsString(opcode)
            << " (" << opcode << ") to Switch Task"
            << ": " << m_pProcess->GetStatusAsString(stat));

    FTRACEINTOFUNC << "Message "
                   << m_pProcess->GetOpcodeAsString(opcode)
                   << " (" << opcode << ") to Switch Task"
                   << " was sent successfully";
}

//////////////////////////////////////////////////////////////////////
STATUS CMfaTask::ValidateIpAddressWithMask(CMediaIpParameters* mediaIpParams)
{
	STATUS retStatus = STATUS_OK;

	IP_INTERFACE_S interfaces = mediaIpParams->GetIpParams().interfacesList[0];
	DWORD ipAddress	= interfaces.iPv4.iPv4Address;

	DWORD netMask	= mediaIpParams->GetIpParams().networkParams.subnetMask;
	DWORD defGw		= mediaIpParams->GetIpParams().networkParams.defaultGateway;

	if ( (ipAddress & netMask) != (defGw & netMask) )
	{
		char ipAddressStr[IP_ADDRESS_LEN],
		     netMaskStr[IP_ADDRESS_LEN],
		     defGwStr[IP_ADDRESS_LEN];
		SystemDWORDToIpString(ipAddress, ipAddressStr);
		SystemDWORDToIpString(netMask,   netMaskStr);
		SystemDWORDToIpString(defGw,     defGwStr);

		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::ValidateIpAddressWithMask FAILED : IpAddress mismatches NetMask:"
		                       << "\nBoardId: "    << m_boardId << ", subBoardId: " << m_subBoardId
		                       << "\nIp address: " << ipAddressStr
		                       << "\nNetMask:    " << netMaskStr
		                       << "\nDeault GW:  " << defGwStr;

		retStatus = STATUS_IP_ADDRESS_MISMATCHES_NETMASK;
	}
	else
	{
		PTRACE(eLevelInfoNormal, "\nCMfaTask::ValidateIpAddressWithMask SUCCEEDED");
		retStatus = STATUS_OK;
	}

	return retStatus;
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::OnAudioCntrlrActiveReq(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnAudioCntrlrActiveReq";
	TAcTypeReq dataStruct;
    memset(&dataStruct,0,sizeof(TAcTypeReq));
    pSeg->Get( (BYTE*)&dataStruct, sizeof(TAcTypeReq));

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol;

	mplPrtcl->AddCommonHeader(AC_TYPE_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(0, m_boardId ,m_subBoardId, dataStruct.unUnitId);
	mplPrtcl->AddData( sizeof(TAcTypeReq), (char*)&dataStruct);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPLAPI");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	TRACESTR(eLevelInfoNormal) << "\n<<>>CMfaTask::OnAudioCntrlrActiveReq m_boardId:"<<m_boardId <<"\n"
                           <<"m_subBoardId :"<< m_subBoardId<<"\n"
                           <<"dataStruct.unUnitId :"<< dataStruct.unUnitId<<"\n"
                           <<"dataStruct.unBoardId:"<<dataStruct.unBoardId<<"\n"
                           <<"m_subBoardId"<< dataStruct.unSubBoardId<<"\n"
                           <<"dataStruct.unUnitId"<< dataStruct.unUnitId<<"\n"
                           <<"dataStruct.eAudioContollerType"<<dataStruct.eAudioContollerType<<"\n";
	POBJDELETE(mplPrtcl);
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::OnCmUnitReconfigInd(CSegment* pSeg)
{
	// ===== 1. retrieve the data
	UNIT_RECONFIG_S* pReconfigStruct = new UNIT_RECONFIG_S;
    memset( pReconfigStruct, 0, sizeof(UNIT_RECONFIG_S) );
    pSeg->Get( (BYTE*)pReconfigStruct, sizeof(UNIT_RECONFIG_S) );

	// ===== 2. print to trace
    TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnCmUnitReconfigInd - struct received from MPM"
                                  << "\nBoardId:  " << pReconfigStruct->boardId
                                  << "\nUnitId:   " << pReconfigStruct->unitId
                                  << "\nUnitType: " << ::CardUnitConfiguredTypeToString(pReconfigStruct->unitType)
                                  << "\nStatus:   " << ::UnitReconfigStatusToString(pReconfigStruct->unitStatus);

	// ===== 3. send data to Resource process
    SendReconfigStructToRsrcAlloc(pReconfigStruct);

    // ===== 4. fill CMfaTask's attribute with data from structure received
    SetUnitsTypesConfigured(pReconfigStruct->unitId, pReconfigStruct->unitType);


    delete pReconfigStruct;
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::SendReconfigStructToRsrcAlloc(UNIT_RECONFIG_S* pReconfigStruct)
{
    // ===== 1. fill a Segment
	CSegment* pParam = new CSegment;
	pParam->Put( (BYTE*)pReconfigStruct, sizeof(UNIT_RECONFIG_S) );

	// ===== 2. print to trace
    TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendReconfigStructToRsrcAlloc - struct sent to Resource process"
                                  << "\nBoardId:  " << pReconfigStruct->boardId
                                  << "\nUnitId:   " << pReconfigStruct->unitId
                                  << "\nUnitType: " << ::CardUnitConfiguredTypeToString(pReconfigStruct->unitType)
                                  << "\nStatus:   " << ::UnitReconfigStatusToString(pReconfigStruct->unitStatus);

    // ===== 3. send
	const COsQueue* pResourceMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResource, eManager);

	STATUS res = pResourceMbx->Send(pParam, RESOURCE_UNIT_RECONFIG_IND);
	//	const COsQueue* pResourceMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResource,eManager);
	//	CTaskApi api;
	//	api.CreateOnlyApi(*pResourceMbx);
	//	api.SendMsg(pSegment,RESOURCE_UNIT_RECONFIG_IND);
}

//EXT-4321: The timer to check Signaling
void CMfaTask::OnTimerFirstSignalPortInactTimeout()
{
	CSegment *pMsg = new CSegment;
	DWORD eventType = eFailoverSignalPortFailure;
	*pMsg << eventType;
	CManagerApi api(eProcessFailover);
	api.SendMsg(pMsg, FAILOVER_EVENT_TRIGGER_IND);
	
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnTimerFirstSignalPortInactTimeout, trigger the Hot backup event!";
}
void CMfaTask::OnTimerSecondSignalPortInactTimeout()
{
	CSegment *pMsg = new CSegment;
	DWORD eventType = eFailoverSignalPortFailure;
	*pMsg << eventType;
	CManagerApi api(eProcessFailover);
	api.SendMsg(pMsg, FAILOVER_EVENT_TRIGGER_IND);
	
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnTimerSecondSignalPortInactTimeout, trigger the Hot backup event!";
}


void CMfaTask::CheckSignalPortStatusForFailover(ETH_SETTINGS_SPEC_S* pEthSettings)
{
	DWORD Eth_Inactivity_Duration = 30;
	int timer_timeout = 0;
	CSysConfig* pSysConfig = NULL;
	std::string key = CFG_KEY_ETH_INACTIVITY_DURATION;

	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnEthSettingInd - CheckSignalPortStatusForFailover\n" ;
	
	pSysConfig = CProcessBase::GetProcess()->GetSysConfig();;
	pSysConfig->GetDWORDDataByKey(key, Eth_Inactivity_Duration);	

	/* some special handle for the timer since MFA side already takes 24 (3*8) second */
	/* to detect whether the port is active or not*/
	timer_timeout = Eth_Inactivity_Duration - 24;
	if (timer_timeout <= 8) {
		/*to set 10 seconds to have a chance to receive one more MFA's port status message */
		timer_timeout = 10;		 
	}

	if (pEthSettings->portParams.portNum == 1) 
	{
		/*First port*/
		if (pEthSettings->monitoringParams.ulActLinkStatus == PORT_ACTIVE) 
		{
			/*port's link is up*/
			m_isFirstPortUp = YES;
			if (IsValidTimer(MFA_SIGNAL_FIRST_PORT_INACTIVITY_TIMER)) {
				DeleteTimer(MFA_SIGNAL_FIRST_PORT_INACTIVITY_TIMER);
			}
		} else {
			/*port's link is down*/
			if (m_isFirstPortUp == YES) {
				m_isFirstPortUp = NO;
				TRACESTR(eLevelInfoNormal) << "\nCMfaTask::CheckSignalPortStatusForFailover: First Port becoms down!";
				StartTimer(MFA_SIGNAL_FIRST_PORT_INACTIVITY_TIMER, timer_timeout*SECOND);
			}
		}
	}
	if (pEthSettings->portParams.portNum == 2) 
	{
		/*Second port*/
		if (pEthSettings->monitoringParams.ulActLinkStatus == PORT_ACTIVE) 
		{
			/*port's link is up*/
			m_isSecondPortUp = YES;
			if (IsValidTimer(MFA_SIGNAL_SECOND_PORT_INACTIVITY_TIMER)) {
				DeleteTimer(MFA_SIGNAL_SECOND_PORT_INACTIVITY_TIMER);
			}
		} else {
			/*port's link is down*/
			if (m_isSecondPortUp == YES) {
				m_isSecondPortUp = NO;
				TRACESTR(eLevelInfoNormal) << "\nCMfaTask::CheckSignalPortStatusForFailover: Second Port becoms down!";
				StartTimer(MFA_SIGNAL_SECOND_PORT_INACTIVITY_TIMER, timer_timeout*SECOND);
			}
		}
	}	
	return;

}


/////////////////////////////////////////////////////////////////////////////
void CMfaTask::SwapEndian(ETH_SETTINGS_SPEC_S *pSpecSettingsStruct)
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
    //802.1x
	pSpecSettingsStruct->monitoringParams.e802_1xSuppPortStatus = SWAPL(pSpecSettingsStruct->monitoringParams.e802_1xSuppPortStatus);
	pSpecSettingsStruct->monitoringParams.e802_1xMethod = SWAPL(pSpecSettingsStruct->monitoringParams.e802_1xMethod);
	pSpecSettingsStruct->monitoringParams.e802_1xFailReason = SWAPL(pSpecSettingsStruct->monitoringParams.e802_1xFailReason);


}

/////////////////////////////////////////////////////////////////////////////
void CMfaTask::OnEthSettingInd(CSegment* pSeg)
{
	// ===== 1. get the parameters from the structure received
	ETH_SETTINGS_SPEC_S* pEthSettings = (ETH_SETTINGS_SPEC_S*)pSeg->GetPtr();

	SwapEndian(pEthSettings);

	//SwapEndian(pEthSettings);

	/*************************************************************************************************************************************/
	/* VNGR 14077 Rachel Cohen 8/3/10                                                                                                    */
	/* pEthSettings->portParams.portNum is 1 when isdn_lan and 2 if rtm_lan so it is an error to go to conversion table with that value  */
	/* meaning GetDisplayBoardId(pEthSettings->portParams.slotId, pEthSettings->portParams.portNum)                                      */
	/*************************************************************************************************************************************/

	// ===== 2. replace boardId with 'displayBoardId' (since it is used for monitoring)
	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(pEthSettings->portParams.slotId, RTM_ISDN_SUBBOARD_ID);
	pEthSettings->portParams.slotId = displayBoardId;

	// ===== 3. update the process
	CEthernetSettingsStructWrappersList *pTheList = m_pProcess->GetEthernetSettingsStructsList();
	pTheList->SetSpecEthernetSettingsStructWrapper(pEthSettings);

	/********************************************************************************************
	// 04.01.10  add an active alarm on case LAN not active
	// added by Rachel Cohen
	*********************************************************************************************/
	//eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
	//if (curProductType==eProductTypeRMX4000){

	TreatNoLanActiveAlarm(pEthSettings);
	//}

	/*temptemptemp - for debugging*/
	//CEthernetSettingsStructWrapper tempStruct(true);
	//tempStruct.SetEthSettingsStruct(pEthSettings);
	//TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnEthSettingInd - struct received from CM:\n" << tempStruct
	//						<< "\n m_isLanActiv: " << (m_isLanActive == TRUE? "TRUE" : "FALSE");
	
	/*temptemptemp - for debugging*/

	if (pEthSettings->monitoringParams.ulActLinkStatus != m_isLanActive)
		SendEthSettingIndToCS(pEthSettings->monitoringParams.ulActLinkStatus);

	m_isLanActive = pEthSettings->monitoringParams.ulActLinkStatus;

	/*If the system is in multipleservice mode, we check the singaling port for Failover*/
	if (IsMultipleServicesMode())
	{
		CheckSignalPortStatusForFailover(pEthSettings);
	}
	
}

/////////////////////////////////////////////////////////////////////////////
void CMfaTask::OnEthSettingClearMaxCountersInd(CSegment* pSeg)
{
	ETH_SETTINGS_STATE_S* pEthState = (ETH_SETTINGS_STATE_S*)pSeg->GetPtr();

	eEthSettingsState curState = (eEthSettingsState)(pEthState->configState);

	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnEthSettingClearMaxCountersInd - params received from CM:"
								  << "\nslotId:      " << (int)(pEthState->portParams.slotId)
								  << "\nportNum:     " << (int)(pEthState->portParams.portNum)
								  << "\nconfigState: " << (eEthSettingsState_ok == curState ? "ok" : "fail");

	m_pProcess->SendClearMaxCountersIndToSwitch(pEthState);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CMfaTask::IsMultipleServicesMode()
{
	BOOL isMultipleServicesCfgFlag = NO, isMultipleServicesMode = NO;

	CSysConfig *sysConfig = m_pProcess->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_MULTIPLE_SERVICES, isMultipleServicesCfgFlag);
	if (m_pProcess->GetLicensingStruct()->multipleServices && isMultipleServicesCfgFlag)
		isMultipleServicesMode = YES;

	return isMultipleServicesMode;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CMfaTask::IsV35JITC()
{
	BOOL isV35JITCSupportFlag = NO;

	CSysConfig *sysConfig = m_pProcess->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, isV35JITCSupportFlag);

	BOOL result = m_isJitcMode && isV35JITCSupportFlag;

	return result;
}

void CMfaTask::OnIceInitInd(CSegment* pSeg)
{
	// ===== 1. extract ice_init_ind struct from the segment received
	ICE_INIT_IND_S* pIceInitInd =  (ICE_INIT_IND_S*)pSeg->GetPtr();

 	// ===== 2. print the data to trace
	TRACESTR(eLevelInfoNormal)
	    << "\nCMfaTask::OnIceInitInd from boardId: "<<m_boardId<<" subBoardId: "<<m_subBoardId<<" for request: "<<pIceInitInd->req_id
	    << "\nStatus: " << pIceInitInd->status
	    << "\nSTUN_Pass_status: " << pIceInitInd->STUN_Pass_status
		<< "\nSTUN_udp_status:  " << pIceInitInd->STUN_udp_status
		<< "\nSTUN_tcp_status:  " << pIceInitInd->STUN_tcp_status
		<< "\nRelay_udp_status: " << pIceInitInd->Relay_udp_status
		<< "\nRelay_tcp_status: " << pIceInitInd->Relay_tcp_status   // iceServersStatus - eIceInitOk
		<< "\nfw_type:          " << pIceInitInd->fw_type; //firewallTypes - eFwTypeUnknown

	m_pProcess->UpdateIceResponseList(pIceInitInd->req_id,pIceInitInd->status, m_boardId, m_subBoardId, pIceInitInd->STUN_Pass_status,
									  pIceInitInd->STUN_udp_status, pIceInitInd->STUN_tcp_status,
									  pIceInitInd->Relay_udp_status, pIceInitInd->Relay_tcp_status, pIceInitInd->fw_type);

	// ===== 2. send to CardsMngr
	CSegment*  pParam = new CSegment();
	*pParam << pIceInitInd->req_id;

	// ===== 2. send to ConfPartyMngr
	const COsQueue* pCardMngrMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCards, eManager);
	pCardMngrMbx->Send(pParam, ICE_INIT_IND_RECEIVED);
}

void CMfaTask::OnIceStatusInd(CSegment* pSeg)
{
	// ===== 1. extract ice_init_ind struct from the segment received
	ICE_STATUS_IND_S* pIceStatusInd =  (ICE_STATUS_IND_S*)pSeg->GetPtr();

	// ===== 2. print the data to trace
	TRACESTR(eLevelInfoNormal)
		<< "\nCMfaTask::OnIceStatusInd from boardId: "<<m_boardId<<" subBoardId: "<<m_subBoardId
		<< "\nSTUN_udp_status:  " << pIceStatusInd->STUN_udp_status
		<< "\nSTUN_tcp_status:  " << pIceStatusInd->STUN_tcp_status
		<< "\nRelay_udp_status: " << pIceStatusInd->Relay_udp_status
		<< "\nRelay_tcp_status: " << pIceStatusInd->Relay_tcp_status
		<< "\nfw_type:          " << pIceStatusInd->fw_type
		<< "\nIce Type:			" << pIceStatusInd->ice_env	;


	ICE_STATUS_IND_S IceStatusInd;
	IceStatusInd.STUN_udp_status = pIceStatusInd->STUN_udp_status;
	IceStatusInd.STUN_tcp_status = pIceStatusInd->STUN_tcp_status;
	IceStatusInd.Relay_udp_status = pIceStatusInd->Relay_udp_status;
	IceStatusInd.Relay_tcp_status = pIceStatusInd->Relay_tcp_status;
	IceStatusInd.fw_type = pIceStatusInd->fw_type;
	IceStatusInd.ice_env = pIceStatusInd->ice_env;


	// ===== 2. send to CardsMngr

	CSegment*  pParam = new CSegment();
	pParam->Put((BYTE*)&IceStatusInd,sizeof(ICE_STATUS_IND_S));

	// ===== 2. send to ConfPartyMngr
	const COsQueue* pCardMngrMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCards, eManager);
	pCardMngrMbx->Send(pParam, ICE_STATUS_IND_RECEIVED);

}

void CMfaTask::OnCSExternalConfigInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnCSExternalConfigInd";


	MEDIA_IP_PARAMS_S *pMediaIpParamsStruct = (MEDIA_IP_PARAMS_S*)pSeg->GetPtr();
	PTRACE2INT(eLevelInfoNormal, "CMfaTask::OnCSExternalConfigInd. Service id = ",pMediaIpParamsStruct->serviceId);

	DWORD serviceId = pMediaIpParamsStruct->serviceId;

	/**********************************************************************************
	* VNGFE-5846 added by Rachel Cohen 6.8.12
	* if service Id == 0 it means MFA card failed to config the ip.
	**********************************************************************************/

	if (pMediaIpParamsStruct->serviceId == 0)
	{
		m_pProcess->SetCSMsgState(serviceId,eConfigFailed);
		//DeleteTimer(CS_EXT_INT_IP_CONFIG_TIMER);
	}
	else
	{

		m_pProcess->SetExtMsgArrived(serviceId,true);
		if (m_pProcess->IsIntMsgArrived(serviceId)  && m_pProcess->IsExtMsgArrived(serviceId) )
		{
			TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnCSExternalConfigInd The CS IP external & Internal has been set successfully " ;

			m_pProcess->SetCSMsgState(serviceId,eDone);
			m_pProcess->SetCSIpConfigBoardId(serviceId,m_boardId);



			DWORD PQ_Id = pMediaIpParamsStruct->ipParams.interfacesList[0].pqId;

			CSegment*  pSeg = new CSegment();



			*pSeg << (DWORD)serviceId
			<< (DWORD)m_boardId
			<< (DWORD) PQ_Id;


			// =====  send to CsMngr
			const COsQueue* pCsMngrMbx =
					CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

			STATUS res = pCsMngrMbx->Send(pSeg, CS_CARDS_MS_CS_IP_CONFIG_END);

			FTRACESTR(eLevelError) << "CMfaTask::OnCSExternalConfigInd after sending " ;


			//DeleteTimer(CS_EXT_INT_IP_CONFIG_TIMER);
		}
	}

	  //  CSegment*  pRetParam = new CSegment();
	  //  *pRetParam << (WORD)m_dnsConfigurationStatus;

	  // ===== 3. send to DnsAgent
	  //CManagerApi api(eProcessSipProxy);

	 if (YES == IsTarget())
		 {
	    CSipProxyTaskApi api(pMediaIpParamsStruct->serviceId);
	    api.SendMsg(NULL, SIPPROXY_CONFIG_CS_IND);
	 }

	    m_ipConfigMsgACKCounter--;

	    //TRACESTR(eLevelInfoNormal)
	      	//	<< "\nCMfaTask::OnCSExternalConfigInd m_ipConfigMsgACKCounter -- " << m_ipConfigMsgACKCounter;

	    HandleStartupOverIfNeeded("OnCSExternalConfigInd");

}
void CMfaTask::OnCSInternalConfigInd(CSegment* pSeg)
{
    TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnCSInternalConfigInd";



    MEDIA_IP_PARAMS_S *pMediaIpParamsStruct = (MEDIA_IP_PARAMS_S*)pSeg->GetPtr();
    PTRACE2INT(eLevelInfoNormal, "CMfaTask::OnCSInternalConfigInd. Service id = ",pMediaIpParamsStruct->serviceId);

	DWORD serviceId = pMediaIpParamsStruct->serviceId;

	if (pMediaIpParamsStruct->serviceId == 0)
	{
		m_pProcess->SetCSMsgState(serviceId,eConfigFailed);
		//DeleteTimer(CS_EXT_INT_IP_CONFIG_TIMER);
	}
	else
	{
		m_pProcess->SetIntMsgArrived(serviceId,true);
		if (m_pProcess->IsIntMsgArrived(serviceId) && m_pProcess->IsExtMsgArrived(serviceId) )
		{
			TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnCSInternalConfigInd The CS IP external & Internal has been set successfully " ;

			m_pProcess->SetCSMsgState(serviceId,eDone);
			m_pProcess->SetCSIpConfigBoardId(serviceId,m_boardId);


			DWORD PQ_Id = pMediaIpParamsStruct->ipParams.interfacesList[0].pqId;

			CSegment*  pSeg = new CSegment();



			*pSeg << (DWORD)serviceId
			<< (DWORD)m_boardId
			<< (DWORD) PQ_Id;


			// =====  send to CsMngr
			const COsQueue* pCsMngrMbx =
					CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

			STATUS res = pCsMngrMbx->Send(pSeg, CS_CARDS_MS_CS_IP_CONFIG_END);

			FTRACESTR(eLevelError) << "CMfaTask::OnCSInternalConfigInd after sending " ;

			//DeleteTimer(CS_EXT_INT_IP_CONFIG_TIMER);
		}
	}
    m_ipConfigMsgACKCounter--;

    //TRACESTR(eLevelInfoNormal)
    	//	<< "\nCMfaTask::OnCSInternalConfigInd m_ipConfigMsgACKCounter -- " << m_ipConfigMsgACKCounter;

    HandleStartupOverIfNeeded("OnCSInternalConfigInd");
}

void CMfaTask::OnCSDnsConfigInd(CSegment* pSeg)
{
    TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnCSDnsConfigInd";

    m_ipConfigMsgACKCounter--;

    HandleStartupOverIfNeeded("OnCSDnsConfigInd");
}

//////////////////////////////////////////////////////////////////////
void CMfaTask::OnMediaConfigurationCompletedInd(CSegment* pSeg)
{
    TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnMediaConfigurationCompletedInd";

    m_isMediaConfigurationCompletedIndReceived = YES;

    HandleStartupOverIfNeeded("OnMediaConfigurationCompletedInd");
}


/////////////////////////////////////////////////////////////////////
void CMfaTask::OnTimerCSExtIntIPConfigFirstTimeout(CSegment* pParam )
{

	DWORD serviceId = 0 ;
	*pParam >> serviceId;

	FTRACESTR(eLevelError) << "CMfaTask::OnTimerCSExtIntIPConfigFirstTimeout service id " << serviceId << " CS msg state " <<m_pProcess->GetCSMsgState(serviceId) ;

	if (m_pProcess->GetCSMsgState(serviceId) == eSendMsgWaitForReply)
	    m_pProcess->SetCSMsgState(serviceId,eConfigFailed);
}

/////////////////////////////////////////////////////////////////////
void CMfaTask::OnTimerCSExtIntIPConfigSecondTimeout(CSegment* pParam )
{

	DWORD serviceId = 0 ;
	*pParam >> serviceId;

	FTRACESTR(eLevelError) << "CMfaTask::OnTimerCSExtIntIPConfigSecondTimeout service id " << serviceId << " CS msg state " <<m_pProcess->GetCSMsgState(serviceId) ;

	if (m_pProcess->GetCSMsgState(serviceId) == eSendMsgWaitForReply)
	    m_pProcess->SetCSMsgState(serviceId,eConfigFailed);

}

/////////////////////////////////////////////////////////////////////
void CMfaTask::OnTimerCSExtIntWaitTimeout(CSegment* pParam )
{


	DWORD serviceId =0 ;
	*pParam >> serviceId;

	/*the timer is per board so if there are 2 services the timer will arrive with the last service id */

	FTRACESTR(eLevelError) << "CMfaTask::OnTimerCSExtIntWaitTimeout service id from timer" << serviceId;


	CMediaIpParameters* mediaIpParamsFromVector = m_pProcess->GetMediaIpParamsVector()->GetFirst();

	if (mediaIpParamsFromVector == NULL)
		TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnTimerCSExtIntWaitTimeout - nothing!!!";

	BYTE bFound = FALSE;

	while (NULL != mediaIpParamsFromVector)
	{

		if ( (mediaIpParamsFromVector->GetBoardId()    == m_boardId) &&
			 (mediaIpParamsFromVector->GetSubBoardId() == m_subBoardId) /*&& (mediaIpParamsFromVector->GetMediaIpParamsStruct()->serviceId == serviceId)*/)
		{
			char ipAddressStr[IP_ADDRESS_LEN];
			SystemDWORDToIpString(mediaIpParamsFromVector->GetMediaIpParamsStruct()->csIp, ipAddressStr);

			DWORD PQ_Id = mediaIpParamsFromVector->GetIpParams().interfacesList[0].pqId;

			int serviceId = mediaIpParamsFromVector->GetMediaIpParamsStruct()->serviceId;
			FTRACESTR(eLevelError) << "CMfaTask::OnTimerCSExtIntWaitTimeout service id from vector" << serviceId;


			if (m_pProcess->GetCSMsgState(serviceId)  != eDone && m_pProcess->GetCSMsgState(serviceId)  != eStartConfig)

			{
				FTRACESTR(eLevelError) << "CMfaTask::OnTimerCSExtIntWaitTimeout msg state different from eDone. service id " << serviceId;


				if (m_pProcess->GetCSMsgState(serviceId) == eSendMsgWaitForReply)
				{
					CSegment *pSeg = new CSegment;

					*pSeg << (DWORD)serviceId;

					StartTimer(CS_EXT_INT_WAIT_REPLY_TIMER, 60*SECOND,pSeg);
				}
				else
				{
					TRACESTR(eLevelInfoNormal) << "\nCMfaTask::OnTimerCSExtIntWaitTimeout - csIp = " << ipAddressStr
					<< ", name = " << mediaIpParamsFromVector->GetMediaIpParamsStruct()->serviceName<<" ,PQId = "<<PQ_Id
					<< " service ID "<< serviceId;

					m_pProcess->SetCSMsgState(serviceId,eSendMsgWaitForReply);
					m_pProcess->SetCSIpConfigBoardId(serviceId,m_boardId);
					m_pProcess->SetIntMsgArrived(serviceId,false);
					m_pProcess->SetExtMsgArrived(serviceId,false);
					SendCSInternalIpConfigReqToMplApi(mediaIpParamsFromVector);
					SendCSExternalIpConfigReqToMplApi(mediaIpParamsFromVector);

					CSegment *pSeg = new CSegment;

					*pSeg << (DWORD)serviceId;



					if (!(IsValidTimer(CS_EXT_INT_IP_CONFIG_FIRST_SERVICE_TIMER)))

						StartTimer(CS_EXT_INT_IP_CONFIG_FIRST_SERVICE_TIMER, 60*SECOND,pSeg);

					else if (!(IsValidTimer(CS_EXT_INT_IP_CONFIG_SECOND_SERVICE_TIMER)))
						StartTimer(CS_EXT_INT_IP_CONFIG_SECOND_SERVICE_TIMER, 60*SECOND,pSeg);

					else
						TRACESTR(eLevelInfoNormal) << "\nCMfaTask::SendCSIpConfigRequestsToMplApi - we have timers problem service id "<< serviceId;


				}
			}



		}

		// ===== 4. get next mediaIpParams from list in process
				mediaIpParamsFromVector = m_pProcess->GetMediaIpParamsVector()->GetNext();
	}





}

// Currently As desined: snmp refer only to board not to port
// return value - is AA exist on port other then the port we update!!
BOOL CMfaTask::SetPortAAAndReturnIfAnyAAExist(BYTE portNumber, BOOL bAddAA)
{
	
	BOOL  isAAExist = FALSE;
	
	BOOL found = FALSE; // actually m_portAAs should be filled in at OnMediaIpConfigInd but the port number that recieved there is always 1 (???) so we fill this port list online 
	
	
	for (std::map<BYTE, BOOL>::iterator itPortAA = m_portAAs.begin(); 
			itPortAA != m_portAAs.end(); ++itPortAA)
	{
		if (itPortAA->first == portNumber)
		{
			found = TRUE;
			// TRACESTR(eLevelInfoNormal) << "\nSetPortAAAndReturnIfAnyAAExist found " << (int)portNumber << " bAddAA "   << (int)bAddAA;;														
			itPortAA->second = bAddAA;	
		}		
		else if (itPortAA->second == TRUE)
		{
			isAAExist = TRUE;
		}		
	}
	// should be filled at the beginning..
	if (!found)
	{
		TRACESTR(eLevelInfoNormal) << "SetPortAAAndReturnIfAnyAAExist:: Insert new port : " << (int)portNumber;
		m_portAAs.insert(std::pair<BYTE, BOOL>(portNumber, bAddAA));
	}
	
	return isAAExist;
}




