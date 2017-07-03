
#if !defined(_GKSERVICEMANAGER_H__)
#define _GKSERVICEMANAGER_H__

#include <set>
#include <queue>          // std::queue

#include "AlarmableTask.h"
#include "Macros.h"
#include "GKManagerStructs.h"
#include "GKManagerOpcodes.h"
#include "GKService.h"
#include "GKCall.h"
#include "Segment.h"
#include "DataTypes.h"
#include "GKToCsInterface.h"
#include "GkCsReq.h"
#include "GkCsInd.h"
#include "IPAdHocProfilesReport.h"
#include "GkTaskApi.h"

#define DefaultOfRrqInterval 120
#define	DefaultOfDnsInterval 120
#define DefaultOfRAIInterval 10

#define DefaultRAIMaximumInterval 120

typedef enum
{
	eNotAltGk,
	eStart,
	eTimeoutOrRegularReject,
	eRCF,
	eReqAck,
	eAnotherTrigger,
	eLastAltGkState
}eAltGkState;

typedef enum
{
	eAVFNotAvaya = 0,
	eAVFSystemFail,
	eAVFAvayaOk
} eAVFStatus;

typedef struct
{
	DWORD connId; //to find calls, which are put on hold before sending the ARQ request (the alt gk trigger)
	WORD  serviceId;
}holdCallSt;


typedef std::set<CGkCall*> CGkCallSet;

extern "C" void GKServiceManagerEntryPoint(void* appParam);

class CGKServiceManager : public CAlarmableTask
{
CLASS_TYPE_1(CGKServiceManager,CAlarmableTask )
public:
	CGKServiceManager();
	virtual ~CGKServiceManager();

	virtual const char* NameOf() const { return "CGKServiceManager";}
	virtual bool IsResetInStartupFail() const {return true;}

	void ManagerPostInitActionsPoint();
	
	TaskEntryPoint GetMonitorEntryPoint();

	PDECLAR_MESSAGE_MAP
//	PDECLAR_TRANSACTION_FACTORY
//	PDECLAR_TERMINAL_COMMANDS

//	void  Create(CSegment& appParam);
	const char* GetTaskName() const{ return m_TaskName.c_str();}
	BOOL  IsSingleton() const {return NO;}
	virtual void  InitTask() {;}
	void* 	GetMessageMap(); 
	virtual void CreateTaskName();
	
	void SetServiceId(DWORD id);
	DWORD GetServiceId();

	void SetIsServiceActive(BOOL active){isActive = active;}
	DWORD GetIsServiceActive(){return isActive;}
	

/* action functions */


//cs mngr	
    void  	OnCsMngrServiceParamsInd(CSegment* pParam);
    void  	OnCsMngrServiceParamsInd_Upd(CSegment* pParam);
	void  	OnCSMngrDeleteIpServiceInd(CSegment* pParam);
	void  	OnCSApiMsg(CSegment *pSeg);

//gk & conf party
	BYTE  	OnURQReq(CGkService* pService);
	void  	OnGRQInd(CSegment* pParam);
	void  	OnRRQInd(CSegment* pParam);
	void    OnDnsPollingTimeout(CSegment* pParam);
	void    OnDNSResolveInd(CSegment* pParam);
	void	OnARQReq(CSegment* pParam);
	void 	OnARQInd(CSegment* pParam);
	void 	OnDRQReq(CSegment* pParam);
	void 	OnDRQInd(CSegment* pParam);
	void 	OnTimeOutInd( CSegment* pParam );
	void  	OnURQInd(CSegment* pParam);
	void  	OnGkURQReqInd(CSegment* pParam);
	void  	OnDRQFromGKInd(CSegment* pParam);
	void  	OnDRQFromGKResponseReq(CSegment* pParam);
	void  	OnIrrPollingReq(CSegment* pParam);
	void  	OnIrqFromGKResponseReq(CSegment* pParam);
	void  	OnGkFailInd(CSegment* pParam);  
	void  	OnGkLRQInd(CSegment* pParam);
	void  	OnLobbyAnswerToGkLRQ(CSegment* pParam);
	void  	OnBRQFromGKInd(CSegment* pParam);
	void  	OnBRQFromGKResponseReq(CSegment *pParam);  
	void 	OnBRQReq(CSegment* pParam); 
	void  	OnBRQInd(CSegment* pParam); 
	void 	OnGkIRQInd(CSegment* pParam);
	
	void  	OnPartyRemoveGkCall(CSegment* pParam);
	void  	OnRegistrationTimeout(CSegment* pParam);
	void  	OnRegistrationPollingTimeout(CSegment* pParam);
	void 	OnPartyMessageTimeout(CSegment* pParam);
	void  	OnListOfGkCallsReq();
    //---HOMOLOGATION. Sasha&Misha. RAS_TE_URG_04 
    BYTE    FindActiveAndHoldCallsInService(DWORD servicedId);
	
// Gk Call:	
	void    OnGkCallNoResponseFromParty(CSegment* pParam);
	void    OnPartyKeepAliveInd(CSegment* pParam);	

    void    OnMcuMngrMNGMNTUpdate(CSegment* pParam);
    void    OnMcuMngrLicensing(CSegment* pParam);
	void 	OnFailoverSlaveBcmMasterInd(CSegment* pMsg);	
	void OnFailoverRefreshRegInd(CSegment* pMsg);
	void 	OnFailoverMasterBcmSlaveInd(CSegment* pMsg);	
protected:
	void 	CreateAndSendUpdateServicePropertiesReq(DWORD serviceId, WORD cardStatus, eGkFaultsAndServiceStatus eServiceOpcode,	eGKConnectionState eGkConnState, 
													char gkId[H243_NAME_LEN], WORD rrqPolingInterval = DefaultOfRrqInterval);
	void	UpdateFaultsAndActiveAlarms(DWORD serviceId, eGkFaultsAndServiceStatus eFaultsOpcode);
	BYTE 	IsErrorFaultsOpcode(eGkFaultsAndServiceStatus eFaultsOpcode);
	char*	GetFaultsOpcodeAsString(eGkFaultsAndServiceStatus eFaultsOpcode);
	BYTE	GetFaultsLevelAccordingToOpcde(eGkFaultsAndServiceStatus eFaultsOpcode);

	void 	HandleChangesInService(DWORD serviceId, WORD bIsGkInService, BYTE* bIsGRQNeeded);
	void 	GetCsHeaderParams(CSegment* pParam, APIU32 *pConnId, APIU32 *pPartyId, APIU32 *pServiceId, APIS32 *pStatus);
	
	void    RequestManagementIp();
	void 	CsEndStartup(CGkService* pService);

	void    ResolveDomainReq(DWORD serviceId, const char* pGkName);
	
	//timers
	void    SendStartRegistartionTimer(OPCODE opcode, DWORD ticks, DWORD serviceId, DWORD cardState = STATUS_OK);
	void    SendStartPartyMessageTimer(OPCODE opcode, DWORD ticks, DWORD serviceId, DWORD connId, DWORD partyId, cmRASTransaction transactionOpcode);
	void    SendStartDnsTimer(DWORD serviceId);
	void    OnRAITimer(CSegment* pParam = NULL);
    
	//timeouts:
	void 	InformParfyOnTimeout(CGkService* pService, DWORD transaction, DWORD mcmsConnId, DWORD partyId);
	void 	InformCsMngrOnTimeout(CGkService* pService, DWORD transaction, BYTE bIsAltTimeout = FALSE);
	void 	HandlePartyMessageTimeout(cmRASTransaction transactionOpcode, CGkService* pService, DWORD connId, DWORD partyId);
	
	void 	SendDRQReq(CGkCall *pCallParam);
	void 	InitDRQReqInsteadOfTheParty(gkReqRasDRQ* pDRQFromParty, const CGkCall* pCallParam);
	void 	ReplayDRQIndToParty(DWORD connId, DWORD partyId);
	void 	SendGkFailureToParty(DWORD connId, DWORD partyId, DWORD failOpcode, eGkFailOpcode eStatusToParty = eDefaultStatus);
						
	// function to support the Gatekeeper Table of the calls
	BYTE   	 AreThereAnyCallsInService(DWORD serviceId);
	BYTE 	 IsFoundGkCallInTable(CGkCall* pCall);
	STATUS 	 RemoveCallFromGkCallsTable(CGkCall *pCall,DWORD serviceId);	
	STATUS 	 RemoveCallFromGkCallsTable(DWORD connId,DWORD serviceId);
	STATUS   RemoveCallFromGkCallsTableWithoutDelete(CGkCall *pCall, DWORD serviceId);
	CGkCall* GetGkCall(DWORD connId);
	CGkCall* GetGkCall(CGkCall* pGkCall);
	void   	 RemoveServiceFromGkCallsTable(DWORD serviceId);
	BYTE   	 RemoveCallsInService(DWORD serviceId);
	void     UpdateCallStateInGkList(CGkCall* pCallParams, eCallStatus eNewState);
	HeaderToGkManagerStruct* AllocateAndGetHeaderFromParty(CSegment* pParam);
	void     OnPartyUpdateConfIdInd(CSegment* pParam);
	void     OnConfPartyCleanUpConfIdInd(CSegment* pParam);
	void     OnConfPartyCleanUpPartyIdInd(CSegment* pParam);

	//Function to support hold calls
	void     RemoveServiceFromGkHoldCallsTable(DWORD serviceId);
	void     RemoveCallFromGkHoldCallsTable(DWORD serviceId, WORD mcmsConnId);
	CGkCall *ResumeHoldCalls(DWORD serviceId, DWORD triggerConnId);	
	void     UpdateCallStateInGkListAfterAltFound(DWORD serviceId,DWORD triggerConnId);		

	void 	 ReRegisterToGk (CGkService* pService, WORD cardstat);
		
	// function to support the Gatekeeper Modes
///	BYTE IsOnCardEndStartUpMessageIsIllegal(WORD boardId, BYTE spanType);
	void 	 CreateAndSendGRQ(CGkService* pService);
	void 	 CreateAndSendRRQ(CGkService* pService, BYTE bIsPolling = TRUE);
	void 	 CreateAndSendIrrReq(CSegment* pParam, DWORD opcode);
	
	void 	 GetOpcodeForGRJ (DWORD rejectReason, eGkFaultsAndServiceStatus &opcode);
	void 	 GetOpcodeAndRRJCounterForRRJ (DWORD rejectReason, eGkFaultsAndServiceStatus &opcode, WORD& RRJCounter);
	void 	 HandleGkURQ(CGkService* pService);
	
	//functions for Path Navigator
	//void OnGwConfigurationChange(CSegment* pParam);   
	void CreateNonStandardInfo (ctNonStandardIdentifierSt &info);
	void CreateMcuDetails (ctNonStandardParameterSt &mcuDetails);
	
	//functions for Alternate Gk
	eAltGkState IsNeedToHandleAltGk(CGkService* pService, DWORD rasOpcode, DWORD connId, int numOfAltGks = 0, BYTE bGkUnresponsive = FALSE, BYTE bGotGRJ = FALSE);
	const char* GetAltGkStateStr(eAltGkState state);
	void  		DecideOnHoldStateAndPutPartyInHold(DWORD serviceId, DWORD connId, DWORD partyId, DWORD opcode);
	void  		PutPartyInHold(DWORD serviceId, DWORD connId, DWORD partyId, eCallStatus holdState);
	BYTE  		IsCurrIndMessageSameAsAltGkTrigger(CGkService* pService, DWORD rasOpcode, DWORD connId);
	void  		DecideOnPhaseInAltGk(CGkService* pService, eAltGkState altGkState, DWORD opcode, DWORD connId,
		 						rejectInfoSt* pRejectInfo = NULL, altGksListSt* pAltGkList = NULL);
	void  		StartAltGkProcess(CGkService* pService, DWORD opcode, DWORD connId,
								rejectInfoSt* pRejectInfo = NULL, altGksListSt* pAltGkList = NULL);	 
	void  		HandleAltGk(CGkService* pService, eAltGkState altGkState);
	DWORD 		TranslateTimeoutOpcodeToRasOpcode(DWORD timeoutOpcode);
	BYTE  		SendReqToAltGk(CGkService* pService);
	BYTE  		SendReqOnHoldCall(CGkCall* pGkCall);
	void  		AltGkFound(CGkService* pService);
	void  		AltGkNotFound(CGkService* pService);
	void  		StartFromOriginalGk(CGkService* pService);
	
	//LRQ:
	void  		CreateAndSendGkLRQResponse(BYTE result, CGkService* pService, int hsRas);	
	void  		SendLrqToLobby(gkIndLRQFromGk* pGkLRQind,/*char* callId, */ DWORD serviceId);

	//Avaya Authentication
	eAVFStatus  CheckAvayaStatus(CGkService* pService, h460FsSt& rH460FsSt);
    //RAI
    void StopRAITimerIfNeeded();
    void OnRACInd(CSegment * pSeg);
    gkReqRasRAI * AllocateAndCreateAdHocRAIReport(CIPAdHocProfilesReport * pAdHocReport,int &raiMsgSize);
    void OnResourceReportGenerated (CSegment* pParam);
    // IpV6
    
    void SetCsIpArrayAccordingToIpTypeAndScopeId(GkManagerServiceParamsIndStruct* pServiceParamsIndSt, CIpAddressPtr GkIp);
    BOOL  CheckBlockOutgoingGRrq();    
    void  AddMessageToTheCSQueue( OPCODE opcode, CSegment* pSeg, DWORD serviceId, APIS32 status, DWORD connId, DWORD partyId, DWORD confId, DWORD callIndex, WORD csId );
    void  SendQueueedMessageToCS( CSegment* pMsg );
    void OnCsQueueTimeout();
    DWORD GetDelayBetweenMessagesValue();

private:
	virtual void DeclareStartupConditions();
	
//	STATUS HandleSetGkIp(CTerminalCommand & command,std::ostream& answer);
	
	//QOS
	void 		 UpdateQosParams(CGkService* pService, gkIndRasRRQ* pRRQIndSt = NULL);
	
/*attributes:	*/
	CGkCallSet 					*m_pCallThroughGkList; //sorted by conn Id
	CGkCallSet 					*m_pHoldCallsList;
	CGkService					*m_ServicesTable[MAX_SERVICES_NUM];
	CGKToCsInterface			*m_pGkToCsInterface;	
	eGkFaultsAndServiceStatus	 m_prevFaultsOpcode;
    DWORD 						 m_managementIpAddress;
    BOOL						 m_bIsAvfLicense;			//this variable should be set according to license received from CSMngr
    CIPAdHocProfilesReport *     m_pLastAdHocReport;
    DWORD                        m_TimeElapsedSinceLastRai;
    DWORD			    m_ServiceId;
    char			    m_ServiceName[ NET_SERVICE_PROVIDER_NAME_LEN ];
    string m_TaskName;
    BOOL isActive;

    typedef std::queue<CSegment *> CsQueue;
    CsQueue messages_;
    eIpType 			 			 m_managementIptype;

};




#endif // !defined(_GKSERVICEMANAGER_H__)

