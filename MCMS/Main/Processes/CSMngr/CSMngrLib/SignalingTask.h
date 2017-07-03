// SignalingTask.h: interface for the CSignalingTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SIGNALINGTASK__H)
#define SIGNALINGTASK__H

#include "AlarmableTask.h"
#include "CsStructs.h"
#include "CSMngrProcess.h"
#include "CSMngrDefines.h"

extern "C" void signalingEntryPoint(void* appParam);

class CCommStartupService;
class CCSMngrMplMcmsProtocolTracer;
class CCommServiceService;
class CCommGKService;
class CCommSnmpService;
class CCommProxyService;
class CCommConfService;


class CIPService;
class CCSMngrManager;
class CCommCardService;

class CCommMcuService;

class CCommRsrcService;

class CCommDnsAgentService;

class CCommMcuMngrService;

class CCSMngrMessageValidator;
class CTerminalCommand;

class CCfgData;
class CCsPinger;
class CIPServiceList;
class CCommCertMngrService;
class CCommTCPDumpService;


//class CCommRsrcService;
class CRouter;

class CSignalingTask : public CAlarmableTask
{
public:
	CSignalingTask();
	virtual ~CSignalingTask();

	const char* GetTaskName() const {return m_TaskName.c_str();}
	virtual const char* NameOf() const { return "CSignalingTask";}

	void 			  SetCsNewIndData(char* data);
	CS_New_Ind_S*     GetCsNewIndData();


	void              SetCsId(WORD id);
	WORD              GetCsId();
	
    STATUS StartStartupConfiguration();
    void SetIsCsMngrEndIpConfigIndReceived(bool val);

private:
	void InitTask();
	void SelfKill();
	
	virtual void AddFilterOpcodePoint();

	BOOL IsSingleton() const {return NO;}
	
	void CreateTaskName();
	
	STATUS OnCsConfigParamInd(CSegment * seg);
	STATUS OnCsEndConfigParamInd(CSegment *pSeg);
	STATUS OnCsEndStartUpInd(CSegment *pSeg);
	STATUS OnCsNewServiceInitInd(CSegment *pSeg);
	STATUS OnCsCommonParamInd(CSegment *pSeg);
	STATUS OnCsCommonParamReadyInd(CSegment *pSeg);
	STATUS OnCsEndServiceInitInd(CSegment *pSeg);
	STATUS OnCsDelServiceInd(CSegment *pSeg);
	STATUS OnCsReconnectInd(CSegment *pSeg);
	void   OnCsKeepAliveInd(CSegment* pSeg);
	void   OnCsCompStatusInd(CSegment* pSeg);
	void   OnCsActiveAlarmInd(CSegment* pSeg);
	void   OnCsPingInd (CSegment* pMsg);
	
	STATUS OnCsMngrEndIpConfigInd(CSegment *pSeg);
	void   OnCsMngrProxyIpServiceParamReq(CSegment* pSeg);
	void   OnCsMngrSnmpUnterfaceReq(CSegment* pSeg);
	
	void   OnCsMngrIceTypeInd(CSegment* pSeg);
	void   OnCsMngrSipServerTypeInd(CSegment* pSeg);
	void   OnTimerSignalPortInactTimeout();	

	void  SendCsKeepAliveRequestToCs();
	void  OnTimerKeepAliveSendTimeout();
	void  OnTimerKeepAliveReceiveTimeout();
	void  PrintLastKeepAliveTimes();
	void  TreatCsKeepAliveFailure();
	void  SendResetReqToDaemon(CSmallString errStr);
	void  CheckKeepAliveResults(csKeepAliveSt *tmpKA);
	void   CheckSignalPortforFailover(compStatuses status);	
	BOOL  IsKeepAliveChangedFromPrevious(csKeepAliveSt *tmpKA);
	DWORD ProduceFaultAndLogger(eCsFailureType failureType, int id=0,
	                             compTypes curCompType=emNonComponent, compStatuses curCompStatus=emCompOk);
	BOOL  IsNewFailure(int id, compStatuses unitStatus);
	void  InitkeepAliveStruct();
	
	bool GetSignalingStartupEndedReceived()const{return m_flagIsSignalingStartupEndedReceived;}
	void SetSignalingStartupEndedReceived(bool val){m_flagIsSignalingStartupEndedReceived = val;}
    bool GetIsCsMngrEndIpConfigIndReceived()const{return m_flagIsCsMngrEndIpConfigIndReceived;}
    
    void SetNICIpV6AutoConfig();
    void TakeInfoMsgFromCS();
	BOOL  isCSIPV6AutoConfigforNinjaGesher(CIPService* pService);
    void HandleNetworkSeparationConfigurations();
    void HandleMultipleNetworksConfigurations();
    void ConfigureNetwork(CIPService* pService, const eConfigInterfaceType ifType);
    void SetIpv6Params();
    void RetrieveRouterList(CIPService *pService, std::list<CRouter> & routerList);
    STATUS AddRoutingTableRule(CIPService *pService);
    STATUS AddRoutingTableNetRule(CIPService *pService);
    STATUS TryStartSignalingServiceConfiguration();
    void   OnCheckCsIpconfig();
    const char *GetStateStr(WORD state);
    void SetState(WORD newState);
    
    void SendIceTypeToCS(int iceType);
    void SendIceGruuToCs(CSegment* pSeg);
    void SendSipServerTypeToCS(int serverType);
    BOOL IsStartupFinished() const;
    
    DWORD AddCSActiveAlarm(WORD errorCode,
                           DWORD userID,
                           const char* description);
    void RemoveCSActiveAlarm(WORD errorCode, DWORD userID);
    static DWORD CreateIDFromCSID(WORD errorCode, DWORD userID, DWORD csID);
    DWORD AddCSActiveAlarmSingleton(DWORD errorCode, DWORD userId, const string &description);
   
    void AddRemoveAAInd(CSegment* pSeg);
    void GenerateCompStatusStStr(ostream &ostr);
    STATUS SendCsNewIndicationToProxyService();

    void   SendSecurityPKIToCs(CSegment* pSeg);
	CSmallString GetCSSubErrorString(eUserMsgSubErrorCode1_CS errorSubCode1,eUserMsgSubErrorOCSPCode2_CS errorSubCode2);
	BOOL SendIpServicesTCDumpParams();

	void SendMcuVersionIdToCs();


	string 				m_TaskName;
	CCSMngrProcess*     m_pProcess;
	CS_New_Ind_S		m_CsNewIndStruct;

	WORD                m_csId;

	CCommTCPDumpService	*m_CommTCPDumpService;
	
	CCommStartupService	*m_CommStartupService;
	CCommServiceService	*m_CommServiceService;
	CCommGKService		*m_CommGKService;
	CCommSnmpService    *m_CommSnmpService;
	CCommProxyService	*m_CommProxyService;
	CCommConfService	*m_CommConfService;
//	CCommRsrcService	*m_CommRcrsService;
	

	CCSMngrMplMcmsProtocolTracer *m_CSMngrMplMcmsProtocolTracer;
	
	bool m_flagIsSignalingStartupEndedReceived;
	bool m_flagIsCsMngrEndIpConfigIndReceived;
	
	BYTE                m_numOfkeepAliveTimeoutReached;
	DWORD               m_keepAliveReceivePeriod;
	CStructTm			m_lastKeepAliveReqTime;
	CStructTm			m_lastKeepAliveIndTime;
	csKeepAliveSt       m_keepAliveStructure;
	DWORD               m_keepAliveSendPeriod;
	BOOL                m_isKeepAliveFailureAlreadyTreated;
	//EXT-4321: to detect the signaling port's status for failover
	BOOL 				m_flagIsSignalPortUp;	
	
	csCompStatusSt      m_compStatusStructure;
	
	USER_MSG_S			m_activeAlarmStructure;
	
	PDECLAR_MESSAGE_MAP
};

#endif // !defined(SIGNALINGTASK__H)
