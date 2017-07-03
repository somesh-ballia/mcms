// SipProxyManager.h: interface for the CSipProxyManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_SipProxyServiceMANAGER_H__)
#define _SipProxyServiceMANAGER_H__

#include "Macros.h"
#include "AlarmableTask.h"

#include "NStream.h"
#include <vector>


#ifndef __SIPPROXYCONFCNTL_H__
#include  "SipProxyConfCntl.h"
#endif

#include "SIPProxyIpParameters.h"
#include "SipUtils.h"
#include "DefinesIpService.h"
#include "SysConfig.h"


void SipProxyServiceManagerEntryPoint(void* appParam);
eSipRegistrationConfType SipProxyServiceManagerGetConfType(COneConfRegistration *confReg);

typedef struct
{
	mcTransportAddress	m_proxyAddress;
	char 	m_pProxyName[H243_NAME_LEN];
}StBusyServerParam;

typedef std::vector<StBusyServerParam*> BusyServerVec;

class CDNSMngrApi;

//////////////////////////////////////////////////////////////
// CConfList
//////////////////////////////////////////////////////////////

class CConfList :  public CPObject
{
CLASS_TYPE_1(CConfList,CPObject )
// public functions
public:
	CConfList();
	~CConfList();
	virtual const char* NameOf() const { return "CConfList";}

	void HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	void HandleEventByIndex(OPCODE opcode, DWORD Id, CSegment* pMsg);
	int  CreateConfCntrl( COneConfRegistration *data, COsQueue* pRcvMbx, CMplMcmsProtocol* pMockMplProtocol = NULL,CSipProxyIpParams* pService=NULL);
	int  StartRegister( int indInConfs, DWORD delay );
	int  Unregister( int indInConfs );
	int  StopReg(int indInConfs );
	int  DeleteConfCntrl( int indInConfs );
	int  ResetConfCntl( int indInConfs );
	DWORD  FindIdByCallId(char* callId);
	int  SetBusyServer(int indInConfs, BYTE busyServer);
	int ChangePresence( int indInConfs, BYTE presenceState );
	int SetTransportErrorState(int indInConfs);
	int DialogRecoveryMessageInd(int indInConfs, DWORD partyId, DWORD connId);
	int MsKeepAliveToutErrInd(int indInConfs, DWORD errCode);
	int SendCrlfMessageOnTransportError(int indInConfs);
// protected functions
protected:

// public parameters
public:

// protected parameters
protected:
	COneConf	*m_pConfList[MAX_CONF_REGISTRATIONS];

};	// CConfList



//////////////////////////////////////////////////////////////
// CServicesList
//////////////////////////////////////////////////////////////

class CServicesList :  public CPObject
{
CLASS_TYPE_1(CServicesList, CPObject)
// public functions
public:
	CServicesList();
	~CServicesList();
	virtual const char* NameOf() const { return "CServicesList";}


	CSipProxyIpParams* GetFirstService();
	CSipProxyIpParams* GetNextService(int serviceID);

	CSipProxyIpParams*  find(int serviceID);
	int		Insert( CSipProxyIpParams* pService );
	void	Remove( int serviceID );
	char*	GetServiceName(int serviceID);
	eIpType GetIpType(int serviceID);
//	DWORD   GetIpV4(int serviceID);
//	char*   GetIpV6(int serviceID);
//	DWORD   GetIpVersion(int serviceID);
//	DWORD 	GetIpV6scopeid(int serviceID);
	ipAddressStruct		GetServiceIpAddress(int serviceID, WORD index);
//	mcTransportAddress	GetIpAddressIpV4(int serviceID);
//	mcTransportAddress	GetIpAddressIpV6(int serviceID);

	DWORD	GetOutboundProxyIpV4(int serviceID);
	char*	GetOutboundProxyIpV6(int serviceID);
	DWORD 	GetOutboundProxyV6scopeid(int serviceID);
	WORD	GetOutboundProxyPort(int serviceID);
	DWORD 	GetOutboundProxyIpVersion(int serviceID);
	mcTransportAddress	GetOutboundProxyAddress(int serviceID);

	DWORD   GetAlternateProxyIpV4(int serviceID);
	char*   GetAlternateProxyIpV6(int serviceID);
	DWORD 	GetAlternateProxyV6scopeid(int serviceID);
	WORD	GetAlternateProxyPort(int serviceID);
	DWORD 	GetAlternateProxyIpVersion(int serviceID);
	mcTransportAddress	GetAlternateProxyAddress(int serviceID);

	DWORD	GetProxyIpV4(int serviceID);
	char*	GetProxyIpV6(int serviceID);
	WORD	GetProxyPort(int serviceID);
	DWORD 	GetProxyV6scopeid(int serviceID);
	DWORD 	GetProxyIpVersion(int serviceID);
	mcTransportAddress	GetProxyAddress(int serviceID);

	char*	GetRegHostName(int serviceID);
	char*	GetAltRegHostName(int serviceID);
	char*	GetProxyName(int serviceID);
	char*	GetOutboundProxyName(int serviceID);
	char*	GetAltProxyName(int serviceID);
	enTransportType	GetTransportType(int serviceID);
	WORD	GetRefreshTout(int serviceID);
	DWORD   GetServersConfig(int serviceID);
	WORD    GetDNSStatus(int serviceID);
	BYTE	IsDHCP(int serviceID);
	BYTE	IsRegOnGoing(int serviceID);
	BYTE	IsRegMRs(int serviceID);
	BYTE	IsRegEQs(int serviceID);
	BYTE 	IsRegFactories(int serviceID);
	BYTE 	IsRegGWProfiles(int serviceID);

	DWORD	GetRegistrarStatus(int serviceID);
	void SetRegistrarStatus(int serviceID, eServerType role, DWORD status, mcTransportAddress proxyIp);
// protected functions
protected:

// public parameters
public:

// protected parameters
protected:
	std::vector< CSipProxyIpParams * > 	m_services;

};	// CConfList


//////////////////////////////////////////////////////////////
// CSipProxyManager
//////////////////////////////////////////////////////////////
class CSipProxyServiceManager : public CAlarmableTask
{
CLASS_TYPE_1(CSipProxyServiceManager,CAlarmableTask )
public:
	CSipProxyServiceManager();
	CSipProxyServiceManager(CMplMcmsProtocol* pMock);
	virtual ~CSipProxyServiceManager();
	virtual const char* NameOf() const { return "CSipProxyServiceManager";}

	const char* GetTaskName() const{ return m_TaskName.c_str();}
	BOOL  IsSingleton() const {return NO;}
	virtual void  InitTask() {;}
	virtual void CreateTaskName();
	virtual void NullActionFunction(CSegment* pParam);
	
	void SetServiceId(DWORD id);
	DWORD GetServiceId();
	void SetIsServiceActive(BOOL active){isActive = active;}
	DWORD GetIsServiceActive(){return isActive;}

	virtual void Dump(COstrStream& msg) const;
	
	TaskEntryPoint GetMonitorEntryPoint();

	// global functions// common
	//void OnCSApiMsg(CSegment* pSeg);
    void SetMngmntDNSStatus(BYTE dNSConfigWasReceived,WORD dNSStatus);

	virtual void*  GetMessageMap();

	// protected functions
protected:

	void	RequestIPServicesFromCsManager();
	void	AskMcuMngrForConfigurationStatus();
	void	OnIpServiceParamIndSetup(CSegment* pSeg);
	void	OnIpServiceParamIndSetup2(CSegment* pSeg);
	void	OnIpServiceParamIndSetup3(CSegment* pSeg);
	void    UpdateCardsMngrForLocalIceInit(DWORD serviceId);
	void	OnIpServiceParamIndConnect(CSegment* pSeg);
	void	OnIpServiceParamDel(CSegment* pSeg);
	void	OnIpServiceParamEnd(CSegment* pSeg);
	void  	OnSC_upInd(CSegment* pSeg);
	void	OnMcuMngrConfigSetup(CSegment* pSeg);
	void	OnMcuMngrConfigCONNECT(CSegment* pSeg);
	void	OnMediaCardSetupEnd(CSegment* pSeg);

	void	HandleRegisterResponse(CSegment* pParam);
	void  	HandleTransportError(CSegment* pParam);
	void	HandleRegisterFailure(CSegment* pParam);
	void	HandleSubscribeResponse(CSegment* pParam);
	void	HandleNotifyInd(CSegment* pParam);
	void	HandleNotifyResponse(CSegment* pParam);
	void	OnCardDataIndication(CSegment* pParam);
	void    HandleServiceResponse(CSegment* pParam);
	void    OnCSEndIceInd(CSegment* pParam);
	void    OnFakeIceEndInit(CSegment* pParam);

	// action functions
	void	OnCardEndStartup(CSegment* pParam);
	void	OnCardCrash(CSegment* pParam);
	void	OnAddConf(CSegment* pParam);
	void	OnDelConf(CSegment* pParam);
	void	OnKillOneConf(CSegment* pParam);
	void	OnTimerRetryProxy(CSegment* pParam);
	void  	OnTimerRetryBusyProxy(CSegment* pParam);
	void	OnTimerRetryAskConfsDb(CSegment* pParam);
	void	OnDNSResolveInd(CSegment* pParam);
	void	OnDNSServiceInd(CSegment* pParam);
	void	OnRegistrarStatus(CSegment* pParam);
	
	void	HandleChangesInServiceIfNeeded(CSipProxyIpParams* pReceivedService);
	void	UnregisterAndDeleteAll(DWORD serviceId);
	void	UnregisterAndDeleteAccordingToConfTypes(DWORD serviceId, BYTE delOngoings, BYTE delMRs, BYTE delEQs, BYTE delFactories, BYTE delDiscovery,BYTE delGWProfiles,BYTE delICEUser);
	void	UpdateExpiredTimeForIdleEntries(DWORD serviceId, DWORD newExpires);
	int		FindConfInDB( DWORD confID, char *confName, int serviceId );
	BYTE  	IsExistOtherRegWithSameName(int index, int serviceId);
	BYTE	IsConfFromOldServiceExist();
	int		InsertConfToDB( DWORD confID, char *confName, WORD serviceId, DWORD expires, BYTE useMainProxy, BOOL bIsDiscovery, BYTE bIsOngoing, BYTE bIsMR, BYTE bIsEQ, BYTE bIsFactory, BYTE bIsGW,WORD sipServerType,char *dummyName,BYTE IsMSICEUser = FALSE,BYTE IsStandICEUser = FALSE);
	int		InsertIceConfToDB( DWORD confID, char *confName, WORD serviceId, DWORD expires, BYTE useMainProxy, BOOL bIsDiscovery, BYTE bIsOngoing, BYTE bIsMR, BYTE bIsEQ, BYTE bIsFactory, BYTE bIsGW,WORD sipServerType,char *dummyName,BYTE IsMSICEUser = FALSE,BYTE IsStandICEUser = FALSE);
	int		RemoveFromConfDB( int indInDB );
	void	DeleteConfCntlAndDB(int index);
	int 	StartRegister(int index);
	DWORD   CalculateDelay();
	int		AreCardParamsChanged(WORD serviceId, BYTE spanType, int indDB = -1);
//	void	RegConfOnAlternate(DWORD Id, BYTE bStartTimer = TRUE);
	void	MoveAllRegsToAlternate(DWORD serviceId, int index);
	void	SetServerBusyStartRetryTimer(DWORD Id, DWORD retryAfterVal);
	void	MoveRegistrationsBetweenCards();
	void	RemoveRegistrationsUsingOtherCard();
	BYTE	IsOldMasterCard(WORD serviceId);
	void    SetRegistrarStatus(WORD serviceID, char* proxyName, DWORD status, mcTransportAddress proxyIp, DWORD confId, WORD confType,DWORD expire, BYTE bSetActiveAlarm = TRUE);
	void	CreateDummyConf(WORD serviceId);
	void	CreateDummyConfIfDBisEmpty(WORD serviceId);
	void    CreateNewICEUser(BOOL IsProxyEnable);//WORD serviceId);
	void    CreateICEUserInDB(WORD serviceId,BOOL IsProxyEnable);
	void  	SipProxyConnect();
//	STATUS	HandleTerminalDumpSipProxy(CTerminalCommand & command, std::ostream& answer);
	BYTE	IsProxyBusy(DWORD proxyIp);
	void	SetBusyExpiredTime(TICKS busyExpiredTime) {m_busyExpiredTime = busyExpiredTime;}
	TICKS	GetBusyExpiredTime() {return m_busyExpiredTime;}
	void	AddServerToBusyServersList(DWORD Id);
	void	DeleteBusyServerList();
	BYTE	IsUseBusyServer(DWORD Id);
	void 	SetConfRegistrationStatus(WORD serviceID, DWORD confId, WORD confType, DWORD status,DWORD expire);
	BYTE 	GetMultipleServices();
	void 	OnMultipleServicesInd(CSegment* pParam);
	void  	OnChangePresenceState(CSegment* pParam);
  void OnDialogRecoveryMessageInd(CSegment *pSeg);
  void OnMsKeepAliveToutErrInd(CSegment *pSeg);

   void 	McuMngrConfigSetup();
	// public parameters



public:


protected:
	COneConfRegistration	*m_pConfRegDB[MAX_CONF_REGISTRATIONS];
	CConfList				*m_confList;
	CServicesList			*m_servicesList;
	DWORD					m_conferencesCounter;
	BYTE					m_UseMainProxy;
	WORD					m_timerRetry;
	BYTE					m_SCisUp;
	BYTE					m_serviceEndWasReceived;
	BYTE					m_DNSConfigWasReceived;
	BYTE					m_DNSStatus;
	BusyServerVec			m_BusyProxyVector;
	TICKS					m_busyExpiredTime;
	WORD					m_numOfBlastRegRequests;
	TICKS					m_firstRegisterTicksTime;
	TICKS					m_lastRegisterTicksTime;
	BYTE					m_mediaCardSetupEndWasReceived;

	CMplMcmsProtocol		*m_pMockMplProtocol;

	PDECLAR_MESSAGE_MAP
//	PDECLAR_TRANSACTION_FACTORY
	DWORD			    m_ServiceId;
	char			    m_ServiceName[ NET_SERVICE_PROVIDER_NAME_LEN ];
  char          m_Dummy_name[H243_NAME_LEN];
	string m_TaskName;
	BOOL isActive;
//	PDECLAR_TERMINAL_COMMANDS

	eIceEnvironmentType m_ServiceIceType;
	CSipProxyIpParams* m_pProxyService;
	WORD m_pICEserviceId;


private:
	BYTE  m_bSystemMultipleServices;
	//virtual void DeclareStartupConditions();
	virtual void ManagerStartupActionsPoint();
	void	UpdateDNSStatusAtAllEntries();
	void BuildDummyName(char *IpStr);
	void GetRelayHostNameAcordingToIPTypeConfiguration(const char* pRelayHostName, char* editedHostName, size_t editedHostNameLen);
};




#endif // !defined(_SipProxyServiceMANAGER_H__)

