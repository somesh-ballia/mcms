// RtmIsdnTask.cpp: implementation of the CRtmIsdnTask class.
//
//////////////////////////////////////////////////////////////////////


#include "RtmIsdnTask.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsRtmIsdnMaintenance.h"
#include "OpcodesMcmsCardMngrMaintenance.h"
#include "TraceStream.h"
#include "CardsProcess.h"
#include "HwMonitoring.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "Segment.h"
#include "MplMcmsProtocol.h"
#include "MplMcmsProtocolTracer.h"
#include "ApiStatuses.h"
#include "FaultsDefines.h"
#include "EthernetSettingsMonitoring.h"
#include "SlotsNumberingConversionTableWrapper.h"
#include "ConfigManagerApi.h"


extern char* ProcessTypeToString(eProcessType processType);

extern const char* SpanTypeToString(eSpanType eType);
extern const char* FramingTypeToString(eFramingType eType);
extern const char* LineCodingTypeToString(eLineCodingType eType);
extern const char* SideTypeToString(eSideType eType);
extern const char* SwitchTypeToString(eSwitchType eType);
extern char* CardTypeToString(APIU32 cardType);

extern const char* SpanAlarmToString(eSpanAlarmType theType);
extern const char* DChannelStateToString(eDChannelStateType theType);
extern const char* ClockingToString(eClockingType theType);

extern char* CardMediaIpConfigStatusToString(DWORD mediaIpConfigStatus);
extern char* IpTypeToString(APIU32 ipType, bool caps = false);



////////////////////////////////////////////////////////////////////////////
//               Task StateMachine's states
////////////////////////////////////////////////////////////////////////////
/*

- RtmIsdn task is CONFIGURATION until receiving CM_MEDIA_IP_CONFIG_IND.
- After receiving CM_MEDIA_IP_CONFIG_IND the task becomes READY

// default states: defined in StateMachine.h
//const  WORD  IDLE           = 0;
//const  WORD  ANYCASE        = 0xFFFF;
*/
//const    WORD  STARTUP        = 1;
const    WORD  CONFIGURATION  = 2;
const    WORD  READY          = 3;



////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CRtmIsdnTask)
    ONEVENT( ACK_IND,					ANYCASE,	CRtmIsdnTask::IgnoreMessage )
    ONEVENT( CM_MEDIA_IP_CONFIG_IND,	ANYCASE,	CRtmIsdnTask::OnMediaIpConfigInd )

	ONEVENT( RTM_ISDN_SPAN_STATUS_IND,	  CONFIGURATION,	CRtmIsdnTask::OnSpanStatusIndNotReady )
	ONEVENT( RTM_ISDN_SPAN_STATUS_IND,	  READY,			CRtmIsdnTask::OnSpanStatusInd )

	ONEVENT( RTM_ISDN_ATTACH_SPAN_MAP_IND,CONFIGURATION,CRtmIsdnTask::OnAttachSpanMapIndNotReady )
	ONEVENT( RTM_ISDN_ATTACH_SPAN_MAP_IND,READY,		CRtmIsdnTask::OnAttachSpanMapInd )

	ONEVENT( RTM_ISDN_DETACH_SPAN_MAP_IND,CONFIGURATION,CRtmIsdnTask::OnDetachSpanMapIndNotReady )
	ONEVENT( RTM_ISDN_DETACH_SPAN_MAP_IND,READY,		CRtmIsdnTask::OnDetachSpanMapInd )

	ONEVENT( MFA_STARTUP_TIMER,			ANYCASE,	CRtmIsdnTask::OnTimerStartupTimeout )
	ONEVENT( SM_MFA_FAILURE_IND,         ANYCASE,   CRtmIsdnTask::OnShmMfaFailureInd )

	ONEVENT( RTM_ISDN_KEEP_ALIVE_IND,	ANYCASE,	CRtmIsdnTask::OnRtmIsdnKeepAliveInd )
	ONEVENT( KEEP_ALIVE_TIMER_RECEIVE,	ANYCASE,	CRtmIsdnTask::OnTimerKeepAliveReceiveTimeout )
	ONEVENT( KEEP_ALIVE_TIMER_SEND,		ANYCASE,	CRtmIsdnTask::OnTimerKeepAliveSendTimeout )

	ONEVENT( HOT_SWAP_RTM_REMOVED,		ANYCASE,	CRtmIsdnTask::HotSwapRtmRemoveCard )

	// messgaes that should not arrive to RtmIsdnTaks task
	ONEVENT( CARDS_NEW_MEDIA_IP_IND,					ANYCASE,	CRtmIsdnTask::IgnoreMessage )
	ONEVENT(ETHERNET_SETTINGS_IND,						ANYCASE,	CRtmIsdnTask::IgnoreMessage )
	ONEVENT(ETHERNET_SETTINGS_CLEAR_MAX_COUNTERS_IND,	ANYCASE,	CRtmIsdnTask::IgnoreMessage )

	ONEVENT( CARDS_NEW_SYS_CFG_PARAMS_IND,		ANYCASE,	CRtmIsdnTask::IgnoreMessage )
	ONEVENT( IVR_MUSIC_ADD_SOURCE_RECEIVED_IND, ANYCASE,	CRtmIsdnTask::IgnoreMessage )

PEND_MESSAGE_MAP(CRtmIsdnTask,CAlarmableTask);



////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void RtmIsdnEntryPoint(void* appParam)
{
	CRtmIsdnTask * pRtmIsdnTask = new CRtmIsdnTask;
	pRtmIsdnTask->Create(*(CSegment*)appParam);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CRtmIsdnTask::CRtmIsdnTask()
{
	TRACESTR(eLevelInfoNormal) << "CRtmIsdnTask - constructor";

	m_pProcess = (CCardsProcess*)CCardsProcess::GetProcess();

	m_boardId					= (DWORD)this;
	m_subBoardId				= (DWORD)this;
//old implementation of SlotsNumbering feature
//	m_boardId_ForMsgSending		= m_boardId;
//	m_subBoardId_ForMsgSending	= m_subBoardId;

	CreateTaskName();

	CSysConfig *sysConfig = m_pProcess->GetSysConfig();
	sysConfig->GetDWORDDataByKey("KEEP_ALIVE_RECEIVE_PERIOD", m_keepAliveReceivePeriod);
	m_keepAliveSendPeriod = DEFAULT_KEEP_ALIVE_SEND_PERIOD;

	m_isKeepAliveAlreadyReceived		= NO;
	m_numOfkeepAliveTimeoutReached		= 0;
	m_isKeepAliveFailureAlreadyTreated	= NO;
	m_isAssertKeepAliveAfterNoConnectionAlreadyProduced	= NO;
	m_lastKeepAliveReqTime.InitDefaults();
	m_lastKeepAliveIndTime.InitDefaults();

	for (int i=0; i<NUM_OF_IP_ADDRESSES_PER_BOARD; i++)
	{
		m_mediaIpStatus[i] = MEDIA_IP_STATUS_INIT_VAL;
	}
}

//////////////////////////////////////////////////////////////////////
CRtmIsdnTask::~CRtmIsdnTask()
{
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::InitTask()
{
    m_state = CONFIGURATION;
	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);

	eCardType cardType = m_cardMngr.GetType();
	if (eRtmIsdn == cardType || eRtmIsdn_9PRI == cardType || eRtmIsdn_9PRI_10G == cardType)
	{
		m_pProcess->GetCardsMonitoringDB()->SetCardState(m_boardId, m_subBoardId, eNormal);
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::InitTask"
		                       << "\nCard on boardId " << m_boardId << " subBoardId " << m_subBoardId
		                       << " (type: " << ::CardTypeToString(cardType) << ")"
		                       << " updated its state to Startup";

		m_pProcess->GetCardsMonitoringDB()->SetCardType(m_boardId, m_subBoardId, cardType);
		// set isMounted (for Eth settings configuration) - MPM's LAN ports become mounted once its RTM is added
		m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(displayBoardId, ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD, true);
        if (eRtmIsdn_9PRI == cardType || eRtmIsdn_9PRI_10G == cardType)
        {
        	m_pProcess->GetEthernetSettingsStructsList()->SetIsMounted(displayBoardId, ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD, true);
        	eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
        	if (curProductType != eProductTypeRMX1500 )
        	    SendRtmIsdnNumPriCardToRtmIsdnMngrProcess(RTM_ISDN_9PRI);
        }
        else if(eRtmIsdn == cardType)
        	SendRtmIsdnNumPriCardToRtmIsdnMngrProcess(RTM_ISDN_12PRI);


		InitRtmIsdnParamsStruct();

		SendRtmEntityLoadedIndToProcesses();
		SendInitInfoToMplApi();

		StartTimer(MFA_STARTUP_TIMER, CARD_STARTUP_TIME_LIMIT);

		OnTimerKeepAliveSendTimeout(); // start KeepALive polling
	}

	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::InitTask - illegal card type. Card Type: " << cardType;
	}


	// remove an unnecessary Alert
	if ( true == IsActiveAlarmExistByErrorCodeUserId(AA_POWER_OFF_PROBLEM, displayBoardId) )
	{
		RemoveActiveAlarmMfaByErrorCodeUserId("CRtmIsdnTask::InitTask", AA_POWER_OFF_PROBLEM, displayBoardId);
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::InitTask - 'old' PowerOff Alert should be removed (the card exists, after all)";
	}
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::InitRtmIsdnParamsStruct()
{
	CSysConfig *sysConfig = m_pProcess->GetSysConfig();

	string	sTemp;
	sysConfig->GetDataByKey(CFG_KEY_ISDN_COUNTRY_CODE,			sTemp);
	m_rtmIsdnParams.SetCountryCode(sTemp);

	DWORD	dTemp;
	sysConfig->GetHexDataByKey(CFG_KEY_ISDN_IDLE_CODE_T1,		dTemp);
	m_rtmIsdnParams.SetIdleCodeT1(dTemp);

	sysConfig->GetHexDataByKey(CFG_KEY_ISDN_IDLE_CODE_E1,		dTemp);
	m_rtmIsdnParams.SetIdleCodeE1(dTemp);

	sysConfig->GetDWORDDataByKey(CFG_KEY_ISDN_NUM_OF_DIGITS,	dTemp);
	m_rtmIsdnParams.SetNumOfDigits(dTemp);

	BOOL	bTemp;
	sysConfig->GetBOOLDataByKey(CFG_KEY_ISDN_CLOCK,				bTemp);
	m_rtmIsdnParams.SetClocking(bTemp);
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::CreateTaskName()
{
	char buff[256];
	sprintf(buff, "RtmIsdnTask (BoardId %d, SubBoardId %d)", m_boardId, m_subBoardId);
	m_TaskName = buff;
}

/////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::SendFailoverIsdnFailureInd()
{
	DWORD eventType = eFailoverIsdnCardFailure;
	CSegment *pMsg = new CSegment;

	*pMsg << eventType;

	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendFailoverIsdnFailureInd, trigger the Hot backup event:" << eventType; 

	CManagerApi api(eProcessFailover);
	STATUS status = api.SendMsg(pMsg, FAILOVER_EVENT_TRIGGER_IND);
	if (status != STATUS_OK)
	{	
		FPASSERT(FAILOVER_EVENT_TRIGGER_IND);
	}
}

/////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::OnTimerKeepAliveReceiveTimeout()
{
	CMedString dataStr = "RtmIsdnTask::OnTimerKeepAliveReceiveTimeout, m_numOfkeepAliveTimeoutReached = ";
	if (YES == m_pProcess->GetIsToTreatRtmIsdnKeepAlive() )
	{
		//if ( RTM_ISDN_KEEP_ALIVE_RETRIES > m_numOfkeepAliveTimeoutReached )
		if ( 40 > m_numOfkeepAliveTimeoutReached )
		{
			m_numOfkeepAliveTimeoutReached++;
			dataStr           << m_numOfkeepAliveTimeoutReached;
			TRACESTR(eLevelInfoNormal) << dataStr.GetString();

		}
		else
		{
			dataStr           << m_numOfkeepAliveTimeoutReached;
			TRACESTR(eLevelInfoNormal) << dataStr.GetString();
			if ( NO == m_isKeepAliveFailureAlreadyTreated )
			{
				m_isKeepAliveFailureAlreadyTreated = YES;
				TreatRtmIsdnKeepAliveFailure();
			}
		}
	} // end isToTreatKeepAlive

	// and anyway, keep sending KeepAlive requests
	OnTimerKeepAliveSendTimeout();
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::OnRtmIsdnKeepAliveInd(CSegment* pSeg)
{
	if ( (YES == m_isKeepAliveFailureAlreadyTreated) &&
	     (NO == m_isAssertKeepAliveAfterNoConnectionAlreadyProduced) )
	{
		m_isAssertKeepAliveAfterNoConnectionAlreadyProduced = YES;

		CLargeString errStr = "RtmIsdn board ";
		errStr << "(boardId: " << m_boardId << ", subBoardId: " << m_subBoardId << ")"
		       << " - KeepAliveInd received after NoConnection happened";
		PASSERTMSG( 1, errStr.GetString() );
	}

	SystemGetTime(m_lastKeepAliveIndTime);

	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
	RemoveActiveAlarmMfaByErrorCodeUserId("OnRtmIsdnKeepAliveInd", AA_NO_CONNECTION_WITH_RTM_ISDN, displayBoardId);
	m_numOfkeepAliveTimeoutReached = 0;

	// ===== 1. print (if needed)
	if ( NO == m_isKeepAliveAlreadyReceived )
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::OnRtmIsdnKeepAliveInd - first KeepAlive indication received "
		                              << "(boardId: " << m_boardId << ", subBoardId: " << m_subBoardId << ")";
		m_isKeepAliveAlreadyReceived = YES;
	}

// - - - TEMP: keepALive is printed (integration phase)
//	BOOL isPeriodTrace = FALSE;
//	m_pProcess->GetSysConfig()->GetDataByKey(CFG_KEY_PERIOD_TRACE, isPeriodTrace);
//	if (YES == isPeriodTrace)
//	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::OnRtmIsdnKeepAliveInd - KeepAlive indication received "
		                              << "(boardId: " << m_boardId << ", subBoardId: " << m_subBoardId << ")";
//	}

	// ===== 2. restart the timer
	DeleteTimer(KEEP_ALIVE_TIMER_RECEIVE);
	StartTimer(KEEP_ALIVE_TIMER_SEND, m_keepAliveSendPeriod*SECOND);
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::TreatRtmIsdnKeepAliveFailure()
{
	// ===== 1. produce a Fault (NoConnectionWithCard)
	DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
	AddActiveAlarmMfa( "TreatRtmIsdnKeepAliveFailure",
	                   FAULT_CARD_SUBJECT,
	                   AA_NO_CONNECTION_WITH_RTM_ISDN,
	                   MAJOR_ERROR_LEVEL,
	                   "No connection with RtmIsdn",
	                   true,
	                   true,
	                   displayBoardId,	// displayBoardId as 'userId' (token)
	                   displayBoardId,
	                   0,				// unitId
	                   FAULT_TYPE_MPM );

	m_pProcess->PrintLastKeepAliveTimes(m_boardId, m_lastKeepAliveReqTime, m_lastKeepAliveIndTime);

	// ===== 2. ask Resource to disable all spans
	SendDisableAllSpansToRsrcAlloc();

	// ===== 3. ask reset from Daemon
//	CSmallString errStr = "No connection with RTM; ";
//	errStr << "board Id: " << m_boardId;
//	m_pProcess->SendResetReqToDaemon(errStr);
	/////////////////COsQueue& rcvMbx = GetRcvMbx();
	/////////////////m_pProcess->TurnMfaTaskToZombie(&rcvMbx); // no messages will be received by this task
	// ===== 3. new treatment for Hot Swap.
     /* SendCardRemoveIndToRsrcalloc(m_boardId,m_subBoardId);
      SendHotSwapCardRemovedToRtmTask();
      SendHotSwapCardRemovedToCardMngr();
      SelfKill();*/

	eCardType cardType = m_cardMngr.GetType();
	m_pProcess->RemoveCard(m_boardId, m_subBoardId, cardType);

    /////////////
	// ===== 4. for next time KA is received (if at all) - treat it as first KA
	m_isKeepAliveAlreadyReceived = NO;

	// ===== 5. Send Indication to Failover manager
	SendFailoverIsdnFailureInd();
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::SendDisableAllSpansToRsrcAlloc()
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendDisableAllSpansToRsrcAlloc "
	                       << "(BoardId: " << m_boardId << ", SubBoardId: " << m_subBoardId << ")";

    RTM_ISDN_BOARD_ID_S* pDisableStruct = new RTM_ISDN_BOARD_ID_S;
	memset(pDisableStruct, 0, sizeof(RTM_ISDN_BOARD_ID_S));

	// ===== 1. initialize the list
	pDisableStruct->boardId		= (WORD)m_boardId;
	pDisableStruct->subBoardId	= (WORD)m_subBoardId;

	// ===== 2. fill the Segment with items from the list
	CSegment*  pRetParam = new CSegment;
	pRetParam->Put( (BYTE*)pDisableStruct, sizeof(RTM_ISDN_BOARD_ID_S) );

	delete pDisableStruct;


	// ===== 3. send
	const COsQueue* pResourceMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResource, eManager);

	STATUS res = pResourceMbx->Send(pRetParam, RTM_ISDN_DISABLE_ALL_SPANS_IND, &GetRcvMbx());
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::OnTimerKeepAliveSendTimeout()
{
//old implementation of SlotsNumbering feature
//	SendMpmKeepAliveRequestToMplApi(RTM_ISDN_KEEP_ALIVE_REQ, m_boardId_ForMsgSending, m_subBoardId_ForMsgSending);
	SendMpmKeepAliveRequestToMplApi(RTM_ISDN_KEEP_ALIVE_REQ);
	StartTimer(KEEP_ALIVE_TIMER_RECEIVE, m_keepAliveReceivePeriod*SECOND);
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::AddFilterOpcodePoint()
{
	AddFilterOpcodeToQueue(RTM_ISDN_KEEP_ALIVE_REQ);
}


//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::SendRtmEntityLoadedIndToProcesses()
{
	SendRtmEntityLoadedIndToSpecProcess(eProcessResource);
	SendRtmEntityLoadedIndToSpecProcess(eProcessRtmIsdnMngr);
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::SendRtmEntityLoadedIndToSpecProcess(eProcessType theProcess)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendRtmEntityLoadedIndToSpecProcess "
	                       << " - send to "  << ::ProcessTypeToString(theProcess)
	                       << "\n(BoardId: " << m_boardId << ", SubBoardId: " << m_subBoardId << ")";

    RTM_ISDN_ENTITY_LOADED_S* pRtmEntityLoadedStruct = new RTM_ISDN_ENTITY_LOADED_S;
	memset(pRtmEntityLoadedStruct, 0, sizeof(RTM_ISDN_ENTITY_LOADED_S));

	// ===== 1. initialize the list
	pRtmEntityLoadedStruct->boardId		= (WORD)m_boardId;
	pRtmEntityLoadedStruct->subBoardId	= (WORD)m_subBoardId;

	eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
	if (curProductType == eProductTypeRMX1500 )
	   pRtmEntityLoadedStruct->numOfSpans	= (WORD)MAX_ISDN_SPAN_MAPS_IN_BOARD_RMX1500;
	else if (curProductType == eProductTypeNinja )
	   pRtmEntityLoadedStruct->numOfSpans	= (WORD)MAX_ISDN_SPAN_MAPS_IN_BOARD_RMX1800;
	else
		pRtmEntityLoadedStruct->numOfSpans	= (WORD)MAX_ISDN_SPAN_MAPS_IN_BOARD;

	// ===== 2. fill the Segment with items from the list
	CSegment*  pRetParam = new CSegment;
	pRetParam->Put( (BYTE*)pRtmEntityLoadedStruct, sizeof(RTM_ISDN_ENTITY_LOADED_S) );

	delete pRtmEntityLoadedStruct;


	// ===== 3. send
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(theProcess, eManager);

	STATUS res = pMbx->Send(pRetParam, RTM_ISDN_ENTITY_LOADED_IND, &GetRcvMbx());
}


//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::SendRtmIsdnNumPriCardToRtmIsdnMngrProcess(WORD numPri)
{
	//TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendRtmIsdnNumPriCardToRtmIsdnMngrProcess numPri "<<numPri;


	CSegment*  pRetParam = new CSegment;
	*pRetParam << (WORD) m_boardId;
	*pRetParam << (WORD) numPri;


	// ===== . send
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessRtmIsdnMngr, eManager);

	STATUS res = pMbx->Send(pRetParam, RTM_ISDN_NUM_PRI_CARD, &GetRcvMbx());
}



//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::SendInitInfoToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "CRtmIsdnTask::SendInitInfoToMplApi";

//	SendVLanIdReqToMplApi();	 // will be sent at Switch task
	SendMediaIpConfigReqToMplApi();
	SendRtmIsdnParamsReqToMplApi();
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::SendMediaIpConfigReqToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendMediaIpConfigReqToMplApi";

	DWORD mediaIp=0, rtmIp=0;

	int firstMediaBoardId	= m_pProcess->Get1stMediaBoardId(),
		secondMediaBoardId	= m_pProcess->Get2ndMediaBoardId(),
		thirdMediaBoardId	= m_pProcess->Get3rdMediaBoardId(),
		fourthMediaBoardId	= m_pProcess->Get4thMediaBoardId();

	if (firstMediaBoardId == (int)m_boardId)
	{
//		mediaIp	= SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_1);		// 30.04.07: will be sent at MFA's startup
		rtmIp	= SystemIpStringToDWORD(RTM_ISDN_RTM_IP_BOARD_1);
	}
	else if (secondMediaBoardId == (int)m_boardId)
	{
//		mediaIp	= SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_2);		// 30.04.07: will be sent at MFA's startup
		rtmIp	= SystemIpStringToDWORD(RTM_ISDN_RTM_IP_BOARD_2);
	}
	else if (thirdMediaBoardId == (int)m_boardId)
	{
		rtmIp	= SystemIpStringToDWORD(RTM_ISDN_RTM_IP_BOARD_3);
	}
	else if (fourthMediaBoardId == (int)m_boardId)
	{
		rtmIp	= SystemIpStringToDWORD(RTM_ISDN_RTM_IP_BOARD_4);
	}
	else // not FIRST/SECOND/THIRD/FOURTH_MEDIA_BOARD_SLOT_NUMBER
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendMediaIpConfigReqToMplApi - illegal boardId: " << m_boardId;
		return;
	}

	SendMediaIpConfigReqToMplApi(mediaIp, rtmIp);
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::SendMediaIpConfigReqToMplApi( DWORD ipAddressMedia,
                                                 DWORD ipAddressRtm)
{
	MEDIA_IP_PARAMS_S structToSend;
	memset( &structToSend, 0, sizeof(MEDIA_IP_PARAMS_S) );

	DWORD netMask	= SystemIpStringToDWORD(RTM_ISDN_SUBNET_MASK),
	      vLan_Mode	= YES,
	      vLan_Id	= RTM_ISDN_VLAN_ID;

	structToSend.ipParams.networkParams.subnetMask				= netMask;
	structToSend.ipParams.networkParams.vLanMode				= vLan_Mode;
	structToSend.ipParams.networkParams.vLanId					= vLan_Id;

	eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
	ePlatformType curPlatformType = m_pProcess->ConvertProductTypeToPlatformType(curProductType);
	structToSend.platformType									= (APIU32)(curPlatformType);

/*
	// 30.04.07: will be sent at MFA's startup
	// ==== 1. send Media address
	structToSend.ipParams.interfacesList[0].iPv4.iPv4Address	= ipAddressMedia;
	structToSend.ipParams.interfacesList[0].ipType				= eIpType_IpV4;
	SendMedaiIpConfigReqSpecToMplApi(structToSend);
*/

	// ==== 2. send Rtm address
	structToSend.ipParams.interfacesList[0].iPv4.iPv4Address	= ipAddressRtm;
	structToSend.ipParams.interfacesList[0].ipType				= eIpType_IpV4;
	SendMedaiIpConfigReqSpecToMplApi(structToSend);
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::SendMedaiIpConfigReqSpecToMplApi(const MEDIA_IP_PARAMS_S& theStruct)
{
	// ===== 1. print
	CMediaIpParameters* pCurStruct = new CMediaIpParameters(theStruct);
	TRACESTR(eLevelInfoNormal)
		<< "\nMediaIpParams sent to RTM "
		<< "(BoardId: " << m_boardId << ", SubBoardId: " << m_subBoardId << "):\n"
		<< *pCurStruct;
	POBJDELETE(pCurStruct);

	// ===== 2. send
	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol;


    mplPrtcl->AddCommonHeader(CM_RTM_MEDIA_IP_CONFIG_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
//old implementation of SlotsNumbering feature
//	mplPrtcl->AddPhysicalHeader(1, m_boardId_ForMsgSending, m_subBoardId_ForMsgSending);
	mplPrtcl->AddPhysicalHeader(1, m_boardId, m_subBoardId);
	mplPrtcl->AddData( sizeof(MEDIA_IP_PARAMS_S), (char*)&theStruct );

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPLAPI");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();


	POBJDELETE(mplPrtcl);
}

void CRtmIsdnTask::SendLanEthSettingToMcuMngr()
{
	// === prepare segment
	TRACESTR(eLevelInfoNormal)<<"\nCRtmIsdnTask::SendRtmIsdnOrLanToMcuMngr";

	//TRACESTR(eLevelInfoNormal) << str.GetString();

	// === send
	//CManagerApi api(eProcessMcuMngr);
	//api.SendMsg(pSeg, RTM_LANS_IND);






	 DWORD DisplayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, 2);

	    bool isMounted = m_pProcess->GetEthernetSettingsStructsList()->GetIsMounted(DisplayBoardId,ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD);
	    if (isMounted == true)
	    {
		// === prepare segment
		CSegment*  pSeg = new CSegment;

		*pSeg << (WORD)m_boardId;
		*pSeg << (WORD)ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD;


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

			//TRACESTR(eLevelInfoNormal) << str.GetString();

			// === send
			CManagerApi api(eProcessMcuMngr);
			api.SendMsg(pSeg, RTM_LANS_IND);

		    }

		     isMounted = m_pProcess->GetEthernetSettingsStructsList()->GetIsMounted(DisplayBoardId,ETH_SETTINGS_MEDIA_3_PORT_ON_MEDIA_BOARD);
				    if (isMounted == true)
				    {
					// === prepare segment
					CSegment*  pSeg = new CSegment;

					*pSeg << (WORD)m_boardId;
					*pSeg << (WORD)ETH_SETTINGS_MEDIA_3_PORT_ON_MEDIA_BOARD;


					// === print to log

					//TRACESTR(eLevelInfoNormal) << str.GetString();

					// === send
					CManagerApi api(eProcessMcuMngr);
					api.SendMsg(pSeg, RTM_LANS_IND);

				    }


}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::SendRtmIsdnParamsReqToMplApi()
{
	RTM_ISDN_PARAMETERS_S* theStruct = m_rtmIsdnParams.GetRtmIsdnParamsStruct();

	string retStr = "\nCRtmIsdnTask::SendRtmIsdnParamsReqToMplApi\n\n";
	retStr += m_rtmIsdnParams.PrintStructData();
	TRACESTR(eLevelInfoNormal) << retStr.c_str();

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol;
	mplPrtcl->AddCommonHeader(RTM_ISDN_PARAMETERS_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
//old implementation of SlotsNumbering feature
//	mplPrtcl->AddPhysicalHeader(1, m_boardId_ForMsgSending, m_subBoardId_ForMsgSending);
	mplPrtcl->AddPhysicalHeader(1, m_boardId, m_subBoardId);
	mplPrtcl->AddData(sizeof(RTM_ISDN_PARAMETERS_S), (char*)theStruct);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPLAPI");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::GetParamsToSendFromService(int specServId,
                                               const RTM_ISDN_PARAMS_MCMS_S* structFromProcess,
                                               RTM_ISDN_SPAN_CONFIG_REQ_S &structToSend)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::GetParamsFromService";

	if (structFromProcess)
	{
		// ===== 1. prepare
		eSpanType		eSpan		= (eSpanType)(structFromProcess->spanDef.spanType);
		eFramingType	eFraming	= (eFramingType)(structFromProcess->spanDef.framing);
		eLineCodingType	eLine		= (eLineCodingType)(structFromProcess->spanDef.lineCoding);
		eSideType		eSide		= (eSideType)(structFromProcess->spanDef.side);
		eSwitchType		eSwitch		= (eSwitchType)(structFromProcess->spanDef.switchType);

		APIU32 dSpanType	= m_rtmIsdnCommonMethods.SpanTypeToApiu32(eSpan);
		APIU32 dFraming		= m_rtmIsdnCommonMethods.FramingTypeToApiu32(eFraming);
		APIU32 dLineCode	= m_rtmIsdnCommonMethods.LineCodingTypeToApiu32(eLine);
		APIU32 dSide		= m_rtmIsdnCommonMethods.SideTypeToApiu32(eSide);
		APIU32 dSwitchType	= m_rtmIsdnCommonMethods.SwitchTypeToApiu32(eSwitch);

		// ===== 2. set params
		structToSend.unit_type			= dSpanType;
		structToSend.framing			= dFraming;
		structToSend.line_code			= dLineCode;
		structToSend.functional_group	= dSide;
		structToSend.switchType			= dSwitchType;

		structToSend.signaling_mode		= SIGNALLING_CCS;	// always on GL1
		structToSend.d_chanl_needed		= YES;				// always on GL1
		structToSend.userConfigClock	= CLOCK_NONE;		// always on GL1
	}
}


//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::OnMediaIpConfigInd(CSegment* pSeg)
{
	eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();

	SendLanEthSettingToMcuMngr();

	// ===== 1. get the new MediaIpConfig
	MEDIA_IP_CONFIG_S *pNewMediaIpConfigStruct = (MEDIA_IP_CONFIG_S*)pSeg->GetPtr();

	char ipAddressArr[IP_ADDRESS_LEN];
	SystemDWORDToIpString(pNewMediaIpConfigStruct->iPv4.iPv4Address, ipAddressArr);
	ipAddressArr[IP_ADDRESS_LEN-1] = 0;
	//What with ipV6Addresses?

	const string ipAddressStr = ipAddressArr;
	const string configStatStr = GetMediaIpConfigStatusAsString(pNewMediaIpConfigStruct->status);

	// ===== 2. update table and become READY (if possible)
	UpdateIpConfigStat(ipAddressStr, pNewMediaIpConfigStruct->status, configStatStr);
	StartReadyActionsIfPossible();

	// ===== 3. print the data to trace
    CLargeString retStr = "\nCRtmIsdnTask::OnMediaIpConfigInd - structure received from MPL:";
    retStr << "\nStatus: "      << configStatStr.c_str()
           << "\nService Id : " << pNewMediaIpConfigStruct->serviceId;

    retStr << "\nIPv4 Address:  " << ipAddressStr;
	for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		retStr << "\nIPv6 Address " << i << ": "
		       << (char*)(pNewMediaIpConfigStruct->iPv6[i].iPv6Address);
	}
	retStr << "\nIP Type: " << IpTypeToString(pNewMediaIpConfigStruct->ipType);

	retStr << "\nPQ Number: "   << pNewMediaIpConfigStruct->pqNumber;

    TRACESTR(eLevelInfoNormal) << retStr.GetString();

 	// ===== 4. update card's status
 	UpdateCardStateAccordingToTaskState("CRtmIsdnTask::OnMediaIpConfigInd");

    if ( eMediaIpConfig_Ok != (eMediaIpConfigStatus)(pNewMediaIpConfigStruct->status) )
    {
		// ===== 5. add ActiveAlarm and log
		string alarmStr = "RTM IP config - address :";
		alarmStr += ipAddressStr.c_str();
		alarmStr += ", failure type: ";
		alarmStr += configStatStr.c_str();

		DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
		AddActiveAlarmMfa( "CRtmIsdnTask::OnMediaIpConfigInd",
		                   FAULT_CARD_SUBJECT,
		                   AA_MEDIA_IP_CONFIGURATION_FAILURE,
		                   MAJOR_ERROR_LEVEL,
		                   alarmStr.c_str(),
		                   true,
		                   true,
		                   displayBoardId,	// displayBoardId as 'userId' (token)
		                   displayBoardId,
		                   0,				// unitId
		                   FAULT_TYPE_MPM );

		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::OnMediaIpConfigInd - status not ok!"
                               << "\nBoardId: " << m_boardId << ", SubBoardId: " << m_subBoardId
                               << " - " << alarmStr.c_str();
    }

    else // status == ok
	{
		RemoveAlertStartupFailureIfPossible("OnMediaIpConfigInd");
	}


}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::StartReadyActionsIfPossible()
{
	if (true == IsAllMediaIpConfigIndReceived() )
	{
		m_state = READY;

		// start Span configuration
		SendRtmIsdnSpanConfigReqToMplApi();
	}
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::SendRtmIsdnSpanConfigReqToMplApi()
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendRtmIsdnSpanConfigReqToMplApi";

	BOOL isAnyCur	= FALSE,
	     isAnyAtAll	= FALSE;

	const RTM_ISDN_SPAN_MAPS_LIST_S* theList = m_pProcess->GetRtmIsdnSpanMapsList();
	CSlotsNumberingConversionTableWrapper* pTable = m_pProcess->GetSlotsNumberingConversionTable();
	DWORD boardId = 0;

	for (int i=0; i<MAX_ISDN_SPAN_MAPS_IN_LIST; i++)
	{
		eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
		if (curProductType == eProductTypeRMX1500 && theList->spanMap[i].boardId == 1)
			boardId = theList->spanMap[i].boardId;
		else
		    boardId = pTable->GetBoardId( (DWORD)(theList->spanMap[i].boardId), RTM_ISDN_SUBBOARD_ID );
		
		if (curProductType == eProductTypeNinja)
			boardId = 1;
			
		if ( boardId == m_boardId )
		{
			isAnyCur = SendRtmIsdnSpanConfigReqSpecToMplApi(theList->spanMap[i]);
			if (TRUE == isAnyCur)
			{
				isAnyAtAll = TRUE;
			}
		}
	} // end loop over spanMaps

	if (FALSE == isAnyAtAll)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendRtmIsdnSpanConfigReqToMplApi boardId :"<< boardId << "m_boardId " << m_boardId
		                       << "\n(no spans in list; nothing was sent to RTM)";
	}
}

//////////////////////////////////////////////////////////////////////
BOOL CRtmIsdnTask::SendRtmIsdnSpanConfigReqSpecToMplApi(const RTM_ISDN_SPAN_MAP_S &theStruct)
{
	// ===== 1. get params from service
	RTM_ISDN_SPAN_CONFIG_REQ_S structToSend;
	memset(&structToSend, 0, sizeof(RTM_ISDN_SPAN_CONFIG_REQ_S));
	
	BOOL isAny = FALSE;
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendRtmIsdnSpanConfigReqSpecToMplApi"
								<< "\ntheStruct.serviceName:" << theStruct.serviceName;


								
	for (int i=0; i<MAX_ISDN_SERVICES_IN_LIST; i++)
	{
		const RTM_ISDN_PARAMS_MCMS_S* curParamsStruct = m_pProcess->GetRtmIsdnParamsStruct(i);
		if (curParamsStruct)
			TRACESTR(eLevelInfoNormal) << "\ncurParamsStruct"<<curParamsStruct->serviceName;
		
		if ( (NULL != curParamsStruct) &&			// there is a service
		     ('\0' != theStruct.serviceName[0]) )	// span is attached to a service
		{
			if ( !memcmp( theStruct.serviceName,			// span is attached to THIS service
			              curParamsStruct->serviceName,
			              RTM_ISDN_SERVICE_PROVIDER_NAME_LEN) )
			{
				isAny = TRUE;

				// ===== 2. print
				TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendRtmIsdnSpanConfigReqSpecToMplApi"
				                       << "\nThe span: "
				                       << "\nBoardId: "		<< theStruct.boardId
				                       << ", SpanId: "		<< theStruct.spanId
				                       << ", Service Name: "	<< (char*)theStruct.serviceName
				                       << "\n(will be sent to actual boardId " << m_boardId << ")";

			   	structToSend.span_id = theStruct.spanId;
				GetParamsToSendFromService(i, curParamsStruct, structToSend);
				PrintSpanConfigReqDataToTrace(structToSend);

				// ===== 3. send
				CMplMcmsProtocol *mplPrtcl = new CMplMcmsProtocol;
				mplPrtcl->AddCommonHeader(RTM_ISDN_SPAN_CONFIG_REQ);
				mplPrtcl->AddMessageDescriptionHeader();
			//old implementation of SlotsNumbering feature
			//	mplPrtcl->AddPhysicalHeader(1, m_boardId_ForMsgSending, m_subBoardId_ForMsgSending);
				mplPrtcl->AddPhysicalHeader(1, m_boardId, m_subBoardId);
				mplPrtcl->AddData(sizeof(RTM_ISDN_SPAN_CONFIG_REQ_S), (char*)&structToSend);

				CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPLAPI");
				mplPrtcl->SendMsgToMplApiCommandDispatcher();

				POBJDELETE(mplPrtcl);

				break;
			}

		} // end if curParamsStruct
	} // end loop over services

	return isAny;
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::UpdateIpConfigStat(const string ipAddressStr, const DWORD configStat, const string configStatStr)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::UpdateIpConfigStat"
	                       << "\nBoardId: " << m_boardId << ", subBoardId: " << m_subBoardId
	                       << "\nAddress: " << ipAddressStr.c_str()
	                       << ", ConfigStat: " << configStat << " (" << configStatStr.c_str() << ")";

	for (int i=0; i<NUM_OF_IP_ADDRESSES_PER_BOARD; i++)
	{
		if (MEDIA_IP_STATUS_INIT_VAL == m_mediaIpStatus[i])
		{
			m_mediaIpStatus[i] = configStat;

			TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::UpdateIpConfigStat - table was updated:"
	                               << "\nBoardId: " << m_boardId << ", subBoardId: " << m_subBoardId
			                       << "\nIp address " << i << " (" << ipAddressStr.c_str() << ")"
			                       << " new status: " << configStat << " (" << configStatStr.c_str() << ")";

			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::RemoveAlertStartupFailureIfPossible(const string theCaller)
{
	if (true == IsAllMediaIpConfigIndReceived() )
	{
		DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
		RemoveActiveAlarmMfaByErrorCodeUserId(theCaller.c_str(), AA_RTM_ISDN_STARTUP_FAILURE, displayBoardId);
	}
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::OnSpanStatusIndNotReady(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::OnSpanStatusIndNotReady";

	// ===== 1. extract spanStatus struct from the segment received
	//          and insert it to m_unitsLoadedStatusesStruct
	RTM_ISDN_SPAN_STATUS_IND_S spanStatStruct;
	DWORD  structLen = sizeof(RTM_ISDN_SPAN_STATUS_IND_S);

	memset(&spanStatStruct, 0, structLen);
	pSeg->Get((BYTE*)(&spanStatStruct), structLen);

	// ===== 2. print data to trace
	PrintSpanStatusDataToTrace(spanStatStruct, false);
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::OnSpanStatusInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::OnSpanStatusInd";

	// ===== 1. extract spanStatus struct from the segment received
	//          and insert it to m_unitsLoadedStatusesStruct
	RTM_ISDN_SPAN_STATUS_IND_S spanStatStruct;
	DWORD  structLen = sizeof(RTM_ISDN_SPAN_STATUS_IND_S);

	memset(&spanStatStruct, 0, structLen);
	pSeg->Get((BYTE*)(&spanStatStruct), structLen);

	// ===== 2. print data to trace
	PrintSpanStatusDataToTrace(spanStatStruct);

	// ===== 3. treat the message
	TreatSpanStatusInd(spanStatStruct);
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::TreatSpanStatusInd(const RTM_ISDN_SPAN_STATUS_IND_S &theStruct)
{
	DWORD				curSpanId	= theStruct.span_id;
	eSpanAlarmType		curAlarm	= m_rtmIsdnCommonMethods.SpanAlarmTypeToEnum(theStruct.alarm_status);
	eDChannelStateType	curDChannel	= m_rtmIsdnCommonMethods.DChannelTypeToEnum(theStruct.d_chnl_status);
	eClockingType		curClock	= m_rtmIsdnCommonMethods.ClockingTypeToEnum(theStruct.clocking_status);

	// ===== 1. Produce/Remove Alerts (if needed)
	TreatSpanStatusAlerts(curSpanId, curAlarm, curDChannel);

	// ===== 2. send the spanStatus data to relevant processes
	SendSpanStatusToRtmIsdnMngrProcess(curSpanId, curAlarm, curDChannel, curClock);
	SendSpanEnabledToRsrcAlloc(curSpanId, curAlarm, curDChannel);
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::TreatSpanStatusAlerts(DWORD curSpanId, eSpanAlarmType curAlarm, eDChannelStateType curDChannel)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::TreatSpanStatusAlerts"
	                       << "\nBoardId:    " << m_boardId
	                       << "\nSubBoardId: " << m_subBoardId
	                       << "\nSpanId:     " << curSpanId
	                       << "\nAlarm:      " << ::SpanAlarmToString(curAlarm)
	                       << "\ndChannel:   " << ::DChannelStateToString(curDChannel);

	DWORD alarmId = CreateIdFromBoardIdSpanId(m_boardId, curSpanId);
	TreatSpanAlarmAlert(curAlarm, alarmId, curSpanId);
	TreatSpanDChannelAlert(curDChannel, alarmId, curSpanId);
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::TreatSpanAlarmAlert(eSpanAlarmType curAlarm, DWORD alarmId, DWORD spanId)
{
	if (eSpanAlarmTypeYellow == curAlarm)
	{
		RemoveActiveAlarmMfaByErrorCodeUserId("TreatRedAlarmAlert", AA_RED_ALARM, alarmId);
		AddSpanStatusActiveAlarm(AA_YELLOW_ALARM, alarmId, spanId, SYSTEM_MESSAGE);
	}
	else if (eSpanAlarmTypeRed == curAlarm)
	{
		RemoveActiveAlarmMfaByErrorCodeUserId("TreatRedAlarmAlert", AA_YELLOW_ALARM, alarmId);
		RemoveActiveAlarmMfaByErrorCodeUserId("TreatRedAlarmAlert", AA_D_CHANNEL_NOT_ESTABLISHED, alarmId);
		AddSpanStatusActiveAlarm(AA_RED_ALARM, alarmId, spanId, SYSTEM_MESSAGE);
	}
	else // eSpanAlarmTypeNone
	{
		RemoveActiveAlarmMfaByErrorCodeUserId("TreatRedAlarmAlert", AA_RED_ALARM, alarmId);
		RemoveActiveAlarmMfaByErrorCodeUserId("TreatRedAlarmAlert", AA_YELLOW_ALARM, alarmId);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::TreatSpanDChannelAlert(eDChannelStateType curDChannel, DWORD alarmId, DWORD spanId)
{
	if (eDChannelStateTypeNotEstablished == curDChannel)
	{
		// there is no point in producing 'D_Channel' Alert if there is no line at all
		if ( false == IsActiveAlarmExistByErrorCodeUserId(AA_RED_ALARM, alarmId) )
		{
			AddSpanStatusActiveAlarm(AA_D_CHANNEL_NOT_ESTABLISHED, alarmId, spanId, MAJOR_ERROR_LEVEL);
		}
	}
	else // eDChannelStateTypeEstablished
	{
		RemoveActiveAlarmMfaByErrorCodeUserId("TreatSpanDChannelAlert", AA_D_CHANNEL_NOT_ESTABLISHED, alarmId);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::AddSpanStatusActiveAlarm(DWORD theOpcode, DWORD userId, DWORD spanId, BYTE errorLevel)
{
	// ===== 1. prepare the string
	COstrStream errStr;
	if (AA_RED_ALARM == theOpcode)
	{
		errStr << "Red Alarm state ";
	}
	else if (AA_YELLOW_ALARM == theOpcode)
	{
		errStr << "Yellow Alarm state ";
	}
	else if (AA_D_CHANNEL_NOT_ESTABLISHED == theOpcode)
	{
		errStr << "D-Channel not established ";
	}
	errStr << "at board " << m_boardId << ", subBoard " << m_subBoardId << ", Span " << spanId;


	// ===== 2. produce the log & Alert (if needed)
	if ( true == IsActiveAlarmExistByErrorCodeUserId(theOpcode, userId) )
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::AddSpanStatusActiveAlarm - Alert already exists"
		                              << "\n" << errStr.str().c_str();
	}

	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::AddSpanStatusActiveAlarm"
		                              << "\n" << errStr.str().c_str();

		DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
		AddActiveAlarmMfa( "CRtmIsdnTask::AddSpanStatusActiveAlarm",
			               FAULT_CARD_SUBJECT,
			               theOpcode,
			               errorLevel,
			               errStr.str().c_str(),
			               true,
			               true,
			               userId,
			               displayBoardId,
			               spanId,
			               FAULT_TYPE_MPM );
	}
}

/////////////////////////////////////////////////////////////////////////////
//void CRtmIsdnTask::TreatDChannelAlert(DWORD curSpanId, eDChannelStateType curDChannel)
//{
//}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::SendSpanStatusToRtmIsdnMngrProcess( DWORD curSpanId, eSpanAlarmType curAlarm,
                                                       eDChannelStateType curDChannel, eClockingType curClock )
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendSpanStatusToRtmIsdnMngrProcess"
	                       << "\nBoardId:    " << m_boardId
	                       << "\nSubBoardId: " << m_subBoardId
	                       << "\nSpanId:     " << curSpanId
	                       << "\nAlarm:      " << ::SpanAlarmToString(curAlarm)
	                       << "\ndChannel:   " << ::DChannelStateToString(curDChannel)
	                       << "\nClocking:   " << ::ClockingToString(curClock);

	// ===== 1. fill the Segment with data from struct
	RTM_ISDN_SPAN_STATUS_MCMS_S spanStatStruct;
	memset(&spanStatStruct, 0, sizeof(RTM_ISDN_SPAN_STATUS_MCMS_S));
	
	spanStatStruct.boardId			= m_boardId;
	spanStatStruct.subBoardId		= m_subBoardId;
	spanStatStruct.spanId			= (WORD)(curSpanId);
	spanStatStruct.alarm_status		= (WORD)(curAlarm);
	spanStatStruct.d_chnl_status	= (WORD)(curDChannel);
	spanStatStruct.clocking_status	= (WORD)(curClock);

	CSegment*  pRetParam = new CSegment;
	pRetParam->Put( (BYTE*)&spanStatStruct, sizeof(RTM_ISDN_SPAN_STATUS_MCMS_S) );

	// ===== 2. send
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessRtmIsdnMngr, eManager);

	STATUS res = pMbx->Send(pRetParam,RTM_ISDN_SPAN_STATUS_MCMS_IND,&GetRcvMbx());

	return;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::SendSpanEnabledToRsrcAlloc(DWORD curSpanId, eSpanAlarmType curAlarm, eDChannelStateType curDChannel)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendSpanEnabledToRsrcAlloc"
	                       << "\nBoardId:    " << m_boardId
	                       << "\nSubBoardId: " << m_subBoardId
	                       << "\nSpanId:     " << curSpanId
	                       << "\nAlarm:      " << ::SpanAlarmToString(curAlarm)
	                       << "\ndChannel:   " << ::DChannelStateToString(curDChannel)
	                       <<"\nMAX_ISDN_SPAN_MAPS_IN_LIST : "<<MAX_ISDN_SPAN_MAPS_IN_LIST;

	// ===== 1. find span
	const RTM_ISDN_SPAN_MAPS_LIST_S* theList = m_pProcess->GetRtmIsdnSpanMapsList();
	CSlotsNumberingConversionTableWrapper* pTable = m_pProcess->GetSlotsNumberingConversionTable();
	DWORD boardId = 0;

	int i=0;
	for (i=0; i<MAX_ISDN_SPAN_MAPS_IN_LIST; i++)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendSpanEnabledToRsrcAlloc"
			                       << "\ntheList->spanMap[i].boardId:    " << theList->spanMap[i].boardId
			                       << "\ntheList->spanMap[i].spanId:	" << theList->spanMap[i].spanId;

		eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
		if (curProductType == eProductTypeRMX1500 && theList->spanMap[i].boardId == 1)
			boardId = theList->spanMap[i].boardId;
		else
		    boardId = pTable->GetBoardId( (DWORD)(theList->spanMap[i].boardId), RTM_ISDN_SUBBOARD_ID );

		if (curProductType == eProductTypeNinja)\
			boardId = 1;

		if ( (m_boardId == boardId) && (curSpanId == theList->spanMap[i].spanId) )
			break;
	}

	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendSpanEnabledToRsrcAlloc i = " << i;

	// ===== 2. set data
	if (MAX_ISDN_SPAN_MAPS_IN_LIST > i) // span found
	{
		SPAN_ENABLED_S spanEnabledStruct;
		spanEnabledStruct.boardId	= m_boardId;
		spanEnabledStruct.spanId	= curSpanId;

		memcpy( spanEnabledStruct.serviceName,
		        theList->spanMap[i].serviceName,
		        RTM_ISDN_SERVICE_PROVIDER_NAME_LEN);

		if ( (eSpanAlarmTypeNone			== curAlarm)	&&
		     (eDChannelStateTypeEstablished	== curDChannel) )
		{
			spanEnabledStruct.spanEnabledStatus = YES;
		}
		else
		{
			spanEnabledStruct.spanEnabledStatus = NO;
		}

		// ===== 3. print
		PrintSpanEnabledDataToTrace(spanEnabledStruct);

		// ===== 4. send
		CSegment*  pRetParam = new CSegment;
		pRetParam->Put( (BYTE*)&spanEnabledStruct, sizeof(SPAN_ENABLED_S) );

		const COsQueue* pMbx =
				CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResource, eManager);

		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendSpanEnabledToRsrcAlloc before sending";


		STATUS res = pMbx->Send(pRetParam,RTM_ISDN_SPAN_ENABLED_IND,&GetRcvMbx());
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::SendSpanEnabledToRsrcAlloc after sending res = "<< res;
	}

	return;
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::PrintSpanConfigReqDataToTrace(const RTM_ISDN_SPAN_CONFIG_REQ_S& theStruct)
{
	CLargeString retStr = "\nCRtmIsdnTask::PrintSpanConfigReqDataToTrace: ";

	eSpanType		dSpanType	= m_rtmIsdnCommonMethods.SpanTypeToEnum(theStruct.unit_type);
	eFramingType	dFraming	= m_rtmIsdnCommonMethods.FramingTypeToEnum(theStruct.framing);
	eLineCodingType	dLineCode	= m_rtmIsdnCommonMethods.LineCodingTypeToEnum(theStruct.line_code);
	eSideType		dSide		= m_rtmIsdnCommonMethods.SideTypeToEnum(theStruct.functional_group);
	eSwitchType		dSwitchType	= m_rtmIsdnCommonMethods.SwitchTypeToEnum(theStruct.switchType);

	retStr << "\nBoardId:    "	<< (int)m_boardId
           << "\nSubBoardId: " << (int)m_subBoardId
	       << "\nSpanId:     "	<< theStruct.span_id
	       << "\nSpanType:   "	<< ::SpanTypeToString(dSpanType)
	       << "\nFraming:    "	<< ::FramingTypeToString(dFraming)
	       << "\nLineCode:   "	<< ::LineCodingTypeToString(dLineCode)
	       << "\nSide:       "	<< ::SideTypeToString(dSide)
	       << "\nSignaling:  "	<< theStruct.signaling_mode << " (not supported!)"
	       << "\nd_channel:  "	<< theStruct.d_chanl_needed << " (not supported!)"
	       << "\nSwitchType: "	<< ::SwitchTypeToString(dSwitchType)
	       << "\nUserClock:  "	<< theStruct.userConfigClock << " (not supported!)";

	TRACESTR(eLevelInfoNormal) << retStr.GetString();
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::PrintSpanStatusDataToTrace(const RTM_ISDN_SPAN_STATUS_IND_S& theStruct, bool isReady/*=true*/)
{
	CLargeString retStr = "\nCRtmIsdnTask::PrintSpanStatusDataToTrace:";

	if (false == isReady)
	{
		retStr << " RtmIsdn task is not ready - MediaIpConfigInd was not received!";
	}

	eSpanAlarmType		curAlarm	= m_rtmIsdnCommonMethods.SpanAlarmTypeToEnum(theStruct.alarm_status);
	eDChannelStateType	curDChannel	= m_rtmIsdnCommonMethods.DChannelTypeToEnum(theStruct.d_chnl_status);
	eClockingType		curClock	= m_rtmIsdnCommonMethods.ClockingTypeToEnum(theStruct.clocking_status);

	retStr << "\nBoardId:     "	<< (int)m_boardId
           << "\nSubBoardId:  " << (int)m_subBoardId
	       << "\nSpanId:      "	<< theStruct.span_id
	       << "\nAlarmStatus: "	<< ::SpanAlarmToString(curAlarm)
	       << "\nDChannel:    "	<< ::DChannelStateToString(curDChannel)
	       << "\nClocking:    "	<< ::ClockingToString(curClock);

	TRACESTR(eLevelInfoNormal) << retStr.GetString();
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::PrintSpanEnabledDataToTrace(const SPAN_ENABLED_S& theStruct)
{
	CLargeString retStr = "\nCRtmIsdnTask::PrintSpanEnabledDataToTrace: ";

	retStr << "\nBoardId:     "	<< (int)m_boardId
	       << "\nSubBoardId:  " << (int)m_subBoardId
	       << "\nSpanId:      "	<< theStruct.spanId
	       << "\nServiceName: "	<< (char*)(theStruct.serviceName)
	       << "\nIsEnabled:   "	<< theStruct.spanEnabledStatus;

	TRACESTR(eLevelInfoNormal) << retStr.GetString();
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::OnAttachSpanMapIndNotReady(CSegment* pSeg)
{
	WORD spanIdFromSeg = 0;
	*pSeg >> spanIdFromSeg;

	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::OnAttachSpanMapInd - "
	                       << "boardId: "  << m_boardId << ", subBoardId: " << m_subBoardId
	                       << ", spanId: " << spanIdFromSeg
	                       << "\nRtmIsdn task is not ready - MediaIpConfigInd was not received!";
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::OnAttachSpanMapInd(CSegment* pSeg)
{
	// ===== 1. extract data from segment
	WORD spanIdFromSeg = 0;
	*pSeg >> spanIdFromSeg;

	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::OnAttachSpanMapInd - "
	                       << "boardId: "  << m_boardId << ", subBoardId: " << m_subBoardId
	                       << ", spanIdFromSeg: " << spanIdFromSeg;

	BOOL isAny = FALSE;

	// ===== 2. find relevant span
	CSlotsNumberingConversionTableWrapper* pTable = m_pProcess->GetSlotsNumberingConversionTable();
	const RTM_ISDN_SPAN_MAPS_LIST_S* theList = m_pProcess->GetRtmIsdnSpanMapsList();
	RTM_ISDN_SPAN_MAP_S curSpan;
	DWORD actualSpanBoardId = 0;

	for (int i=0; i<MAX_ISDN_SPAN_MAPS_IN_LIST; i++)
	{
		curSpan = theList->spanMap[i];

		eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();

		if (curProductType == eProductTypeRMX1500  && theList->spanMap[i].boardId == 1)
			actualSpanBoardId = curSpan.boardId;
		else
		    actualSpanBoardId = pTable->GetBoardId(curSpan.boardId, RTM_ISDN_SUBBOARD_ID);

		if (eProductTypeNinja == curProductType)
			actualSpanBoardId = 1;
		
		if ( (actualSpanBoardId == m_boardId) && (curSpan.spanId == spanIdFromSeg) )
		{
			TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::OnAttachSpanMapInd - spanConfogReq is sent to Rtm"
	                                      << "\nBoardId: " << m_boardId << " (displayed as boardId " << curSpan.boardId << ")"
	                                      << ", subBoardId: " << m_subBoardId << ", spanId: "  << spanIdFromSeg;

			// ===== 3. send request
			isAny = SendRtmIsdnSpanConfigReqSpecToMplApi(curSpan);
		}

	} // end loop over spanMaps

	if (FALSE == isAny)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::OnAttachSpanMapInd - "
		                       << "\nno spans was found for boardId " << m_boardId << " (displayed as boardId " << curSpan.boardId << ")"
		                       << ", spanId " << spanIdFromSeg << "\n - nothing was sent to RTM";
	}
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::OnDetachSpanMapIndNotReady(CSegment* pSeg)
{
	// ===== 1. extract data from segment
	WORD spanIdFromSeg = 0;
	*pSeg >> spanIdFromSeg;

	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::OnDetachSpanMapInd - "
	                       << "boardId: "  << m_boardId << ", subBoardId: " << m_subBoardId
	                       << ", spanId: " << spanIdFromSeg
	                       << "\nRtmIsdn task is not ready - MediaIpConfigInd was not received!";
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::OnDetachSpanMapInd(CSegment* pSeg)
{
	// ===== 1. extract data from segment
	WORD spanIdFromSeg = 0;
	*pSeg >> spanIdFromSeg;

	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::OnDetachSpanMapInd - "
	                       << "boardId: "  << m_boardId << ", subBoardId: " << m_subBoardId
	                       << ", spanId: " << spanIdFromSeg;

	// ===== 2. remove Alerts
	RemoveSpanStatusAlerts(spanIdFromSeg);

	// ===== 3. prepare data to send
	RTM_ISDN_SPAN_DISABLE_REQ_S spanToDisable;
	spanToDisable.span_id = (APIU32)spanIdFromSeg;

	// ===== 4. send request
	CMplMcmsProtocol *mplPrtcl = new CMplMcmsProtocol;
	mplPrtcl->AddCommonHeader(RTM_ISDN_SPAN_DISABLE_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
//old implementation of SlotsNumbering feature
//	mplPrtcl->AddPhysicalHeader(1, m_boardId_ForMsgSending, m_subBoardId_ForMsgSending);
	mplPrtcl->AddPhysicalHeader(1, m_boardId, m_subBoardId);
	mplPrtcl->AddData(sizeof(RTM_ISDN_SPAN_DISABLE_REQ_S), (char*)&spanToDisable);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPLAPI");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::OnShmMfaFailureInd(CSegment* pSeg)
{
	WORD  subBoardId     = 2; // (RTM's subBoardId: FIRST_SUBBOARD_ID+1)
	DWORD compStatus     = 0,
	      problemBitMask = 0;

	// ===== 1. get data from segment
	*pSeg >> compStatus;
	*pSeg >> problemBitMask;
	*pSeg >> subBoardId;

	// ===== 2. print to log file
	CLargeString errStr = "\nCRtmIsdnTask::OnShmMfaFailureInd";
	errStr << "\nBoardId: "         << m_boardId << ", subBoardId: " << subBoardId//m_subBoardId
	       << ". Status: "			<< compStatus
	       << ", problem bitmask: "	<< problemBitMask;

	// ===== 3. treat Mfa accordingly
	TRACESTR(eLevelInfoNormal) << errStr.GetString();
//	if ( eSmComponentOk == compStatus)
//	{
//		RemoveSmActiveAlarms();
//	}

//	else // compStatus != ok
//	{
//		BOOL isResetting = FALSE;

		   // ===== 4. check if card was removed
		   if ((problemBitMask & SM_COMP_POWER_OFF_BITMASK)  && ((eSmComponentOk == compStatus))) ///powr_off (Hot Swap)
		   {
			   	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::OnShmMfaFailureInd - removing the RTM extension";
			   	/****************************************************************************/
			   	/* 1.8.10 VNGR- 16394 added by Rachel Cohen                                 */
			   	/* we need to increase the parameter "m_UpdateCounter" that EMA will        */
			   	/* update the hot swap change .                                             */
			   	/****************************************************************************/
				m_pProcess->GetCardsMonitoringDB()->SetCardState(m_boardId, m_subBoardId, eDisabled);

			   	/****************************************************************************/
			   	/* 20.7.10 added by Rachel Cohen                                            */
			   	/* we need to return the ethsetting port id to its default portid=2 RTM_LAN */
			   	/* cause Rtm_LAN has no task and we will get no indication on it in case of */
			   	/* hot swap.                                                                */
			   	/****************************************************************************/
			   	CMfaTask::SendLanEthSettingToMcuMngr();

				// handle ActiveAlarms
				DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);

				RemoveAllActiveAlarms("OnShmMfaFailureInd", displayBoardId);

				AddActiveAlarmMfa( "OnShmMfaFailureInd",
								   FAULT_CARD_SUBJECT,
								   AA_POWER_OFF_PROBLEM,
								   MAJOR_ERROR_LEVEL,
								   "Power off problem",
								   true,
								   true,
								   displayBoardId/*as 'userId' (token)*/,
								   displayBoardId,
								   0,
								   FAULT_TYPE_MPM );

				HotSwapRtmRemoveCard();


		     }



/*
//////////////////////////
// Temp: until ShelfMngr will send the right status of the MFA!!
//		if ( eSmComponentResetting == compStatus)
//			isResetting = TRUE;

		BOOL isFailureProblem = UpdateActiveAlarmsIfNeeded(problemBitMask, isResetting); // [ printing to log file is performed within 'UpdateActiveAlarmsIfNeeded' method ]

		// ===== 4. in Failure problem: notify rsrcAllocator and ask reset from Daemon
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
*/
//	} // end  compStatus != ok
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::RemoveSpanStatusAlerts(WORD spanId)
{
	DWORD alarmId = CreateIdFromBoardIdSpanId(m_boardId, (DWORD)spanId);

	RemoveActiveAlarmMfaByErrorCodeUserId("RemoveSpanStatusAlerts", AA_RED_ALARM, alarmId);
	RemoveActiveAlarmMfaByErrorCodeUserId("RemoveSpanStatusAlerts", AA_YELLOW_ALARM, alarmId);
	RemoveActiveAlarmMfaByErrorCodeUserId("RemoveSpanStatusAlerts", AA_D_CHANNEL_NOT_ESTABLISHED, alarmId);
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::OnTimerStartupTimeout()
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::OnTimerStartupTimeout "
	                       << " (Board id: " << m_boardId << ", SubBoardId: " << m_subBoardId << ")";

	// ===== 1. check if all necessary indications has been received
	bool isAllMediaIpConfigIndReceived = IsAllMediaIpConfigIndReceived();

	if (false == isAllMediaIpConfigIndReceived)
	{
		string errStr = "MediaIpConfigInd was not received";

		// ===== 2. produce Active Alarm
		DWORD displayBoardId = m_pProcess->GetSlotsNumberingConversionTable()->GetDisplayBoardId(m_boardId, m_subBoardId);
 		AddActiveAlarmMfa( "CRtmIsdnTask::OnTimerStartupTimeout",
 		                   FAULT_CARD_SUBJECT,
 		                   AA_RTM_ISDN_STARTUP_FAILURE,
 		                   MAJOR_ERROR_LEVEL,
 		                   errStr.c_str(),
 		                   true,
 		                   true,
 		                   displayBoardId,	// displayBoardId as 'userId' (token)
 		                   displayBoardId,
 		                   0,				// unitId
 		                   FAULT_TYPE_MPM );

		// ===== 3. print to log
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::OnTimerStartupTimeout "
		                       << " (Board id: " << m_boardId << ", SubBoardId: " << m_subBoardId << ") - "
		                       << errStr.c_str();
	}
}

//////////////////////////////////////////////////////////////////////
bool CRtmIsdnTask::IsAllMediaIpConfigIndReceived()
{
	bool isReceived = true;

	// 30.04.07: RtmMediaIp address is sent on MFA startup (instead of on RTM startup)
	//for (int i=0; i<NUM_OF_IP_ADDRESSES_PER_BOARD; i++)
	for (int i=0; i<NUM_OF_IP_ADDRESSES_PER_BOARD-1; i++)
	{
		if (MEDIA_IP_STATUS_INIT_VAL == m_mediaIpStatus[i])
		{
			isReceived = false;
			break;
		}
	}

	return isReceived;
}

//////////////////////////////////////////////////////////////////////
const string CRtmIsdnTask::GetMediaIpConfigStatusAsString(DWORD configStat)
{
	string retStr;

	char* tmpStatStr = ::CardMediaIpConfigStatusToString(configStat);
	if (tmpStatStr)
	{
		retStr = tmpStatStr;
	}
	else
	{
		retStr = "(invalid: ";
		retStr += configStat;
		retStr += ")";
	}

	return retStr;
}
/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::HotSwapRtmRemoveCard()
{
//	m_pProcess->SendCardRemoveIndToRsrcalloc(m_boardId,m_subBoardId);//2= sub board id of rtm

	const RTM_ISDN_SPAN_MAPS_LIST_S* theList = m_pProcess->GetRtmIsdnSpanMapsList();
	RTM_ISDN_SPAN_MAP_S curSpan;

	for (int i=0; i<MAX_ISDN_SPAN_MAPS_IN_LIST; i++)
	{
		curSpan = theList->spanMap[i];
		if ( (curSpan.boardId == m_boardId) &&
			 ('\0' != curSpan.serviceName[0]) ) // span is attached to a service
		{
			//m_pProcess->SendCardRemoveIndToRsrcalloc(m_boardId,i);
			TreatSpanStatusAlerts(curSpan.spanId, eSpanAlarmTypeRed, eDChannelStateTypeEstablished);
		   	SendSpanStatusToRtmIsdnMngrProcess(curSpan.spanId, eSpanAlarmTypeRed, eDChannelStateTypeEstablished, eClockingTypeNone);
		}
	}

	eCardType cardType = m_cardMngr.GetType();
	m_pProcess->RemoveCard(m_boardId, m_subBoardId, cardType);

	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnTask::HotSwapRtmRemoveCard "
	                       << " (Board id: " << m_boardId << ", SubBoardId: " << m_subBoardId << ")";
 //  	sleep(30);
	SetSelfKill();
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/*void CRtmIsdnTask::SM_failuretoHotSwapRtmRemoveCard()
{
m_pProcess->RemoveCard(m_boardId,m_subBoardId);
	RTM_ISDN_SPAN_STATUS_IND_S theStruct;
	DWORD				curSpanId	= 0;
	eSpanAlarmType		curAlarm	= eSpanAlarmTypeRed;//unavilable
	eDChannelStateType	curDChannel	= eDChannelStateTypeEstablished;
	eClockingType		curClock	= eClockingTypeNone;
	m_pProcess->SendCardRemoveIndToRsrcalloc(m_boardId,2);//2= sub board id of rtm
	for (int i=0; i<(int)MAX_ISDN_SPAN_MAPS_IN_BOARD; i++)
	{   curSpanId=i;
		//m_pProcess->SendCardRemoveIndToRsrcalloc(m_boardId,curSpanId);
		TreatSpanStatusAlerts(curSpanId, curAlarm, curDChannel);
	   	SendSpanStatusToRtmIsdnMngrProcess(curSpanId, curAlarm, curDChannel, curClock);
	}
}
*/
////////////////////////////////////////////////////////////////////////////////
DWORD CRtmIsdnTask::CreateIdFromBoardIdSpanId(const DWORD boardId, const DWORD spanId)
{
	// just to gnerate a unique number from BoardId + spanId
	DWORD retId = (1000*boardId) + spanId;
	return retId;
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::RemoveAllActiveAlarms(CSmallString callerStr, DWORD displayBoardId)
{
	RemoveSmActiveAlarms(callerStr, displayBoardId);

	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_NO_CONNECTION_WITH_RTM_ISDN,	displayBoardId);
	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_MEDIA_IP_CONFIGURATION_FAILURE,	displayBoardId);
	RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_RTM_ISDN_STARTUP_FAILURE,		displayBoardId);

	for (int i=0; i<MAX_ISDN_SPAN_MAPS_IN_BOARD; i++)
	{
		DWORD alarmId = CreateIdFromBoardIdSpanId(m_boardId, i);

		RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_RED_ALARM,					alarmId);
		RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_YELLOW_ALARM,				alarmId);
		RemoveActiveAlarmMfaByErrorCodeUserId(callerStr, AA_D_CHANNEL_NOT_ESTABLISHED,	alarmId);
	}
}




//////////////////////////////////////////////////////////////////////


/*
//old implementation of SlotsNumbering feature
//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::SetBoardId_ForMsgSending(DWORD id)
{
	m_boardId_ForMsgSending = id;
}

//////////////////////////////////////////////////////////////////////
DWORD CRtmIsdnTask::GetBoardId_ForMsgSending()
{
	return m_boardId_ForMsgSending;
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnTask::SetSubBoardId_ForMsgSending(DWORD id)
{
	m_subBoardId_ForMsgSending = id;
}

//////////////////////////////////////////////////////////////////////
DWORD CRtmIsdnTask::GetSubBoardId_ForMsgSending()
{
	return m_subBoardId_ForMsgSending;
}
*/
