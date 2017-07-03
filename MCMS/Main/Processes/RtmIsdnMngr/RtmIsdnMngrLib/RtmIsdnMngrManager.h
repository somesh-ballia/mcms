// RtmIsdnMngrManager.h: interface for the CRtmIsdnMngrManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_RTM_ISDN_MNGR_MANAGER_H__)
#define _RTM_ISDN_MNGR_MANAGER_H__


#include "ManagerTask.h"
#include "Macros.h"
#include "RtmIsdnMngrInternalStructs.h"
#include "RtmIsdnMngrCommonMethods.h"
#include "RtmIsdnService.h"
#include "RtmIsdnDefines.h"

class CRtmIsdnMngrProcess;


#define RTM_CARDS_ARRAY_LEN		MAX_NUM_OF_RTM_ISDN_BOARDS_RMX4000+1 // '+1' so the idx can be used as boardId



//////////////////////////////////////////////////////////////////////
void RtmIsdnMngrManagerEntryPoint(void* appParam);


//////////////////////////////////////////////////////////////////////
class CRtmIsdnMngrManager : public CManagerTask
{
CLASS_TYPE_1(CRtmIsdnMngrManager,CManagerTask )
public:
	CRtmIsdnMngrManager();
	virtual ~CRtmIsdnMngrManager();

	TaskEntryPoint GetMonitorEntryPoint();

	void ManagerPostInitActionsPoint();
    virtual void ManagerStartupActionsPoint();
    void   IgnoreMessage();

    void  OnCardTimerStartupTimeout();
	void  OnRtmIsdnSpanStatusInd(CSegment* pSeg);
	void  OnRtmIsdnSpanDisableInd(CSegment* pSeg);
	void  OnRtmIsdnPhoneRangeDelInd(CSegment* pSeg);
	void  OnRtmServiceCancelInd(CSegment* pSeg);
	void  OnMcuMngrLicensingInd(CSegment* pSeg);
	void  OnRtmIsdnEntityLoadedInd(CSegment* pSeg);
	void  OnSlotsNumberingConversionTableInd(CSegment* pSeg);


	// Transaction Map
	STATUS HandleSetNewService(CRequest* pSetRequest);
	STATUS HandleSetUpdateService(CRequest* pSetRequest);
	STATUS HandleSetDelService(CRequest* pSetRequest);
	STATUS HandleSetSetDefaultService(CRequest* pSetRequest);

	STATUS HandleSetAddPhoneNumRange(CRequest* pSetRequest);
//	STATUS HandleSetUpdatePhoneNumRange(CRequest* pSetRequest);
	STATUS HandleSetDelPhoneNumRange(CRequest* pSetRequest);

	STATUS HandleSetUpdateSpan(CRequest* pSetRequest);


	void SendServiceParamsStructToProcesses();
	void SendServiceParamsStructToSpecProcess(RTM_ISDN_PARAMS_MCMS_S &theStruct, eProcessType theProcess);
	void SendServiceParamsEndToProcesses();
	void SendServiceParamsEndToSpecProcess(eProcessType theProcess);
	void SendDefaultServiceNameToProcesses();
	void SendDefaultServiceNameToSpecProcess(RTM_ISDN_SERVICE_NAME_S &theStruct, eProcessType theProcess);
	void SendSpansMapListStructToCardsProcess();
	void SendServiceToProcesses(CRtmIsdnService &other);
	void SendDeleteServiceToSpecProcess(const char* serviceName, eProcessType theProcess);

	STATUS SendMsgSyncCancelServiceToResourceProcess(const RTM_ISDN_SERVICE_CANCEL_S &serviceToCancel);
	STATUS DeleteService(const char* serviceName, bool isToCheckStat);

	void   SendAttachSpanMapToCardsProcess(const RTM_ISDN_SPAN_MAP_S &spanToUpdate);
	void   DetachSpansFor_NumPriRtmIsdnCard(CSegment* pSeg);
	void   SendDetachSpanMapToCardsProcess(const SPAN_DISABLE_S &spanToDetach);
	STATUS SendMsgSyncDisableSpanMapToResourceProcess(const SPAN_DISABLE_S &spanToDisable);
	STATUS DetachSpanMap(const SPAN_DISABLE_S &theStruct,bool isSpanValid=true);
	STATUS SetIsSpanValid(const SPAN_DISABLE_S &theStruct ,WORD boardId);

	void   AddNoRtmCardActiveAlarmIfNeeded(const WORD boardId);
	void   RemoveNoRtmCardActiveAlarmIfNeeded(const WORD boardId);

	STATUS SendUpdatePhoneRangeReqToProcesses(const OPCODE theOpcode, const char* serviceName, const CRtmIsdnPhoneNumberRange &phoneRange);
	STATUS SendUpdatePhoneRangeReqToSpecProcess(const OPCODE theOpcode, RTM_ISDN_PHONE_RANGE_UPDATE_S &theStruct, eProcessType theProcess);
	STATUS SendMsgSyncUpdatePhoneRangeToResourceProcess(CSegment* pSegSent, const OPCODE theOpcode);
	STATUS DeletePhoneRange(const char* serviceName, const CRtmIsdnPhoneNumberRange &phoneRange, bool isToCheckStat);

    BOOL	GetIsPstnLicensed() const;
	void		SetIsPstnLicensed(const BOOL isLicensed);

	bool	IsRtmIsdnLoaded(const int boardId) const;
	void		SetRtmIsdnLoaded_BoardId(const int boardId);

	void PrintSpanStatusDataToTrace(const RTM_ISDN_SPAN_STATUS_MCMS_S& theStruct);
	void TreatClockSourceStateAlerts(const WORD boardId, const WORD subBoardId=2/*rtmIsdn subBoard. bad coding!*/);
	void PrintRtmIsdnEntityDataToTrace(const RTM_ISDN_ENTITY_LOADED_S& theStruct);


	eSystemSpanType  GetSystemSpanType() const;
    void  SetSystemSpanType(const eSystemSpanType theType);
	bool  IsAcceptedSpanType(eSpanType theType);
	void  SetParamsAccordingToSpanType(eSpanType theType);

	bool  IsAcceptedAdditionalConfiguredSpan(WORD boardId);

	void OnTimerGetServiceInfoInd();
	void OnSNMPConfigInd(CSegment* pSeg);
	void SetIsSNMPEnabled(BOOL bEnabled) 		 { m_bSNMPEnabled = bEnabled; }
	BOOL GetIsSNMPEnabled() const				 { return m_bSNMPEnabled; }
private:
	CRtmIsdnMngrProcess* m_pProcess;
	CRtmIsdnMngrCommonMethods m_rtmIsdnCommonMethods;
	
	BOOL m_isSystemNormalWithSingleClockSource;
	BOOL m_isPstnLicensed;

	eSystemSpanType	m_systemSpanType;
	int				m_maxAllowedConfiguredSpans;
	
	int  m_RtmIsdnLoaded_BoardId[RTM_CARDS_ARRAY_LEN]; // an array of RtmIsdn boards that are loaded

	BOOL m_bSNMPEnabled;

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS

};

#endif
