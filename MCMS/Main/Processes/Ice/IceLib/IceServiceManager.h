#if !defined(_ICESERVICEMANAGER_H__)
#define _ICESERVICEMANAGER_H__

#include "Macros.h"
#include "AlarmableTask.h"

#include "NStream.h"
#include <vector>


#include "SIPProxyIpParameters.h"
#include "SipUtils.h"
#include "DefinesIpService.h"
#include "SysConfig.h"
#include  "IceCmReq.h"
#include "IceCmInd.h"

//const
const WORD   DEFAULT_STUN_PORT     = 3478;


void IceServiceManagerEntryPoint(void* appParam);
void IceWebRtcServiceManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////
// CIceServiceManager
//////////////////////////////////////////////////////////////
class CIceServiceManager : public CAlarmableTask
{
CLASS_TYPE_1(CIceServiceManager,CAlarmableTask )
public:
	CIceServiceManager();

	virtual ~CIceServiceManager();
	virtual const char* NameOf() const { return "CIceServiceManager";}
	virtual void  InitTask() {;}
	virtual void CreateTaskName();
	virtual void NullActionFunction(CSegment* pParam);
	virtual void SetEndService();
	virtual void Dump(COstrStream& msg) const;
	virtual void*  GetMessageMap();
	const char* GetTaskName() const{ return m_TaskName.c_str();}
	BOOL  IsSingleton() const {return NO;}
	void SetServiceId(DWORD id);
	DWORD GetServiceId();
	void SetIsServiceActive(BOOL active){isActive = active;}
	DWORD GetIsServiceActive(){return isActive;}
	TaskEntryPoint GetMonitorEntryPoint();


	// protected functions
protected:

	void	OnIpServiceParamDel(CSegment* pSeg);
	void	OnMcuMngrConfigSetup(CSegment* pSeg);
	void	OnMcuMngrConfigCONNECT(CSegment* pSeg);
	void	OnMediaCardSetupEnd(CSegment* pSeg);

	// action functions
	void	HandleChangesInServiceIfNeeded(CSipProxyIpParams* pReceivedService);
	BYTE 	GetMultipleServices();
	void 	OnMultipleServicesInd(CSegment* pParam);
	void 	McuMngrConfigSetup();


protected:

	BYTE					m_SCisUp;
	BYTE					m_serviceEndWasReceived;
	BYTE					m_mediaCardSetupEndWasReceived;


	PDECLAR_MESSAGE_MAP
	//	PDECLAR_TRANSACTION_FACTORY
	DWORD			    m_ServiceId;
	char			    m_ServiceName[ NET_SERVICE_PROVIDER_NAME_LEN ];

	string m_TaskName;
	BOOL isActive;
//	PDECLAR_TERMINAL_COMMANDS

	eIceEnvironmentType m_ServiceIceType;
	CSipProxyIpParams* m_pProxyService;
	WORD m_pICEserviceId;

private:
	BYTE  m_bSystemMultipleServices;
	virtual void ManagerStartupActionsPoint();

};

//////////////////////////////////////////////////////////////
// CWebRtcServiceManager
//////////////////////////////////////////////////////////////
class CWebRtcServiceManager : public CIceServiceManager
{
CLASS_TYPE_1(CWebRtcServiceManager,CIceServiceManager )
public:
	CWebRtcServiceManager();

	virtual ~CWebRtcServiceManager();
	virtual const char* NameOf() const { return "CWebRtcServiceManager";}

	virtual void NullActionFunction(CSegment* pParam);
	virtual void*  GetMessageMap();
	virtual void SetEndService();
	void    OnCSEndIceInd(CSegment* pParam);
	void  	OnCSEndIceStatInd(CSegment* pParam);
	void	SetWebRTCService(CSegment* pSeg);


protected:

	PDECLAR_MESSAGE_MAP

	// ice base members
	char m_stunPassServerHostName[H243_NAME_LEN];
	char 	m_stunPassServerUserName[H243_NAME_LEN];
	ICE_SERVER_PARAMS_S m_stunPassServerAddress;
	char 	m_stunPassServerPassword[H243_NAME_LEN];
	char	m_stunPassServerRealm[H243_NAME_LEN];

	char m_stunServerHostName[H243_NAME_LEN];
	ICE_SERVER_PARAMS_S m_stunServerUDPAddress;
	ICE_SERVER_PARAMS_S m_stunServerTCPAddress;
	char m_RelayServerHostName[H243_NAME_LEN];
	ICE_SERVER_PARAMS_S m_RelayServerUDPAddress;
	ICE_SERVER_PARAMS_S m_RelayServerTCPAddress;
	BOOL m_PassServerResolved;
	BOOL m_StunServerResolved;
	BOOL m_TurnServerResolved;

private:
	void FillWebRTCICEparams(CSipProxyIpParams* pService);
	void UpdateCM();
	void needToResolveHostname(char *StunHostToResolve, char *RelayHostToResolve, char *StunPassHostToResolve);


};// end of WebRtcServiceManager


#endif // !defined(_ICESERVICEMANAGER_H__)

