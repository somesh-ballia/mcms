#ifndef RTMISDNTASK_H_
#define RTMISDNTASK_H_


#include "MfaTask.h"
#include "RtmIsdnMngrInternalStructs.h"
#include "RtmIsdnParamsWrapper.h"
#include "RtmIsdnMngrCommonMethods.h"


extern "C" void RtmIsdnEntryPoint(void* appParam);

#define MEDIA_IP_STATUS_INIT_VAL	-1



class CRtmIsdnTask : public CMfaTask
{
public:
	CRtmIsdnTask();
	virtual ~CRtmIsdnTask();


	virtual const char* NameOf() const { return "CRtmIsdnTask";}
	void	InitTask();
	BOOL	IsSingleton() const {return NO;}

	void	InitRtmIsdnParamsStruct();

/*
//old implementation of SlotsNumbering feature
	void  SetBoardId_ForMsgSending(DWORD id);
	DWORD GetBoardId_ForMsgSending();

	void  SetSubBoardId_ForMsgSending(DWORD id);
	DWORD GetSubBoardId_ForMsgSending();
*/

protected:
	void	CreateTaskName();

	void    SendLanEthSettingToMcuMngr();

	void	SendRtmEntityLoadedIndToProcesses();
	void	SendRtmEntityLoadedIndToSpecProcess(eProcessType theProcess);
	void    SendRtmIsdnNumPriCardToRtmIsdnMngrProcess(WORD numPri);
	void	SendDisableAllSpansToRsrcAlloc();

	void	SendInitInfoToMplApi();
//	void	SendVLanIdReqToMplApi(); // will be sent at Switch task
	void	SendMediaIpConfigReqToMplApi();
	void	SendMediaIpConfigReqToMplApi(DWORD ipAddressMedia, DWORD ipAddressRtm);
	void	SendMedaiIpConfigReqSpecToMplApi(const MEDIA_IP_PARAMS_S& theStruct);
	void	SendRtmIsdnParamsReqToMplApi();
	void	SendRtmIsdnSpanConfigReqToMplApi();
	BOOL	SendRtmIsdnSpanConfigReqSpecToMplApi(const RTM_ISDN_SPAN_MAP_S &theStruct);
	void	GetParamsToSendFromService( int specServId,
	                                    const RTM_ISDN_PARAMS_MCMS_S* structFromProcess,
	                                    RTM_ISDN_SPAN_CONFIG_REQ_S &structToSend );

	void	OnMediaIpConfigInd(CSegment* pSeg);
	void	OnSpanStatusIndNotReady(CSegment* pSeg);
	void	OnSpanStatusInd(CSegment* pSeg);
	void	OnAttachSpanMapIndNotReady(CSegment* pSeg);
	void	OnAttachSpanMapInd(CSegment* pSeg);
	void	OnDetachSpanMapIndNotReady(CSegment* pSeg);
	void	OnDetachSpanMapInd(CSegment* pSeg);
	void	OnShmMfaFailureInd(CSegment* pSeg);
	void	RemoveSpanStatusAlerts(WORD spanId);


	void	TreatSpanStatusInd(const RTM_ISDN_SPAN_STATUS_IND_S &theStruct);
	void	TreatSpanStatusAlerts(DWORD curSpanId, eSpanAlarmType curAlarm, eDChannelStateType curDChannel);
	void	TreatSpanAlarmAlert(eSpanAlarmType curAlarm, DWORD alarmId, DWORD spanId);
	void	TreatSpanDChannelAlert(eDChannelStateType curDChannel, DWORD alarmId, DWORD spanId);
	void	AddSpanStatusActiveAlarm(DWORD theOpcode, DWORD userId, DWORD spanId, BYTE errorLevel);
//	DWORD	CreateUserId(const DWORD boardId, const DWORD spanId);

	void	SendSpanStatusToRtmIsdnMngrProcess(DWORD curSpanId, eSpanAlarmType curAlarm, eDChannelStateType curDChannel, eClockingType curClock);
	void	SendSpanEnabledToRsrcAlloc(DWORD curSpanId, eSpanAlarmType curAlarm, eDChannelStateType curDChannel);

	void	PrintSpanConfigReqDataToTrace(const RTM_ISDN_SPAN_CONFIG_REQ_S& theStruct);
	void	PrintSpanStatusDataToTrace(const RTM_ISDN_SPAN_STATUS_IND_S& theStruct, bool isReady=true);
	void	PrintSpanEnabledDataToTrace(const SPAN_ENABLED_S& theStruct);

	void	OnRtmIsdnKeepAliveInd(CSegment* pSeg);
	void	OnTimerKeepAliveReceiveTimeout();
	void	OnTimerKeepAliveSendTimeout();
	void	TreatRtmIsdnKeepAliveFailure();

	void	StartReadyActionsIfPossible();
	void	OnTimerStartupTimeout();

	void	UpdateIpConfigStat(const string ipAddressStr, const DWORD configStat, const string configStatStr);
	void	RemoveAlertStartupFailureIfPossible(const string theCaller);
	bool	IsAllMediaIpConfigIndReceived();
	const string	GetMediaIpConfigStatusAsString(DWORD configStat);
	//void    TreatSpanStatusIndForRemoveCard();
	void    HotSwapRtmRemoveCard();
	DWORD 	CreateIdFromBoardIdSpanId(const DWORD boardId, const DWORD spanId);
	void   	RemoveAllActiveAlarms(CSmallString callerStr, DWORD displayBoardId);
	void 	SendFailoverIsdnFailureInd();

	virtual void AddFilterOpcodePoint();


	CRtmIsdnParamsWrapper		m_rtmIsdnParams;
	CRtmIsdnMngrCommonMethods	m_rtmIsdnCommonMethods;

	int	m_mediaIpStatus[NUM_OF_IP_ADDRESSES_PER_BOARD];

//old implementation of SlotsNumbering feature
//	DWORD m_boardId_ForMsgSending;
//	DWORD m_subBoardId_ForMsgSending;


	PDECLAR_MESSAGE_MAP
};


#endif /*RTMISDNTASK_H_*/
