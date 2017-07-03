//+========================================================================+
//                  EpSimCSEndpointsTask.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpSimCSEndpointsTask.h                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef __EPSIMCSENDPOINTSTASK_H__
#define __EPSIMCSENDPOINTSTASK_H__

#include "CSIDTaskApp.h"

#define CS_SIM_MAX_CALL_PORTS	3200

const WORD ENDPOINTS_TARGET_TASK = 1;
const WORD PROXY_TARGET_TASK     = 3;
const WORD GK_TARGET_TASK 	     = 4;
const WORD ILLEGAL_TARGET_TASK   = 100;

class CClientSocket;
class CMplMcmsProtocol;
//class CGkTaskApi;
//class CProxyTaskApi;

/////////////////////////////////////////////////////////////////////////////
//
//   SimCSEndpointsModule - Central Signaling Sim module Task
//
/////////////////////////////////////////////////////////////////////////////

typedef struct  {
	APIU32 srcIpAddress;
	APIU32 port;
} CCallPorts;

class CSimCSEndpointsModule : public CCSIDTaskApp
{
CLASS_TYPE_1(CSimCSEndpointsModule, CCSIDTaskApp)
public:
			// Constructors
	CSimCSEndpointsModule(void);
	virtual ~CSimCSEndpointsModule();
	virtual const char* NameOf() const { return "CSimCSEndpointsModule";}

	eEPSimTaskType GetTaskType(void) const;

			// Initializations
	void  Create(CSegment& appParam);	// called by entry point
	void* GetMessageMap();

			// Operations
	void  SendToCsApi(CMplMcmsProtocol& rMplProtocol) const;
	void  SendToCsApi(CSegment& rParam) const;

protected:
		// Action functions

	// Manager: connect card
	void OnMngrConnectSocketIdle(CSegment* pMsg);
	// Socket: connection established
	void OnSocketConnectedSetup(CSegment* pMsg);
	// Socket: connection failed
	void OnSocketFailedSetup(CSegment* pMsg);
	// Socket: connection dropped
	void OnSocketDroppedSetup(CSegment* pMsg);
	void OnSocketDroppedConnect(CSegment* pMsg);
	void OnSocketPauseConnect(CSegment* pMsg);
    void OnSocketResumePause(CSegment* pMsg);
	
	// Socket : message received
	void OnSocketRcvIdle(CSegment* pMsg);
	void OnSocketRcvSetup(CSegment* pMsg);
	void OnSocketRcvStartup(CSegment* pMsg);
	void OnSocketRcvConnect(CSegment* pMsg);
	// pass mbx
	void OnReceiveMbxEndpointsIdle(CSegment* pMsg);
	void OnReceiveMbxProxy(CSegment* pMsg);
	void OnReceiveMbxGK(CSegment* pMsg);
	// timer
	void OnTimerSendCsNewIndSetup(CSegment* pMsg);
	

	// command to CSAPI
	void OnReceiveCommandToCSAPI(CSegment* pMsg);

	// get port req
	void OnCsSigGetPortReq( CMplMcmsProtocol* pMplProtocol );
	void SaveSrcIpAddress( APIU32 srcIpAddress, APIU32 port );
	void SendCsSigGetPortInd( CMplMcmsProtocol* pMplProtocol );
	void ReleaseSrcIpAddress( APIU32 srcIpAddress, APIU32 port );

	// service functions
	void OnCsNewServiceInitReq( CMplMcmsProtocol* pMplProtocol );
	void OnCsCommonParamReq( CMplMcmsProtocol* pMplProtocol );
	void OnCsEndServiceInitReq( CMplMcmsProtocol* pMplProtocol );
	void SendEndIpServiceInitInd();
	void SendErrorEndServiceInd();

	void OnCsProxyNotifyReq( CMplMcmsProtocol* pMplProtocol );

	// party keep-alive
	void OnCsH323PartyKeepAliveReq( CMplMcmsProtocol* pMplProtocol );
	void OnCsSipPartyKeepAliveReq( CMplMcmsProtocol* pMplProtocol );
	
	void OnCsGetKeepAliveReq( CMplMcmsProtocol* pMplProtocol );

    //Ping from EMA
    void OnCsPingReq(CMplMcmsProtocol* pMplProtocol );
    
protected:
			// Utilities
	void  InitTask(){;}
	BOOL  IsSingleton() const {return NO;}
	const char* GetTaskName() const;
//	int ConnectToMcmsCSApi();
	int SendNewCSInd();
	int SendConfigParamInd();
	int SendEndConfigParamInd();
	int SendEndCSStartupInd();
	int OnNewCSReq();
	void OnSocketDropped(CSegment* pMsg);
	void CsFillCsProtocol( CMplMcmsProtocol* pMplProt,const DWORD opcode,const BYTE* pData,const DWORD nDataLen ) const;
	void SendMsgToTargetTask( CMplMcmsProtocol* pMplProtocol, const WORD targetTask ) const;
	WORD GetTargetTask( const OPCODE opcode ) const;

	// Yuri R
	void OnCsTerminalCommandReq( CMplMcmsProtocol* pMplProtocol );
	
protected:
			// Attributes
	CTaskApi*	   m_pEndpointsApi;
//	CProxyTaskApi* m_pProxyApi;
//	CGkTaskApi*    m_pGKeeperApi;

	CClientSocket*  m_pSocketConnection;
	DWORD		m_portCall;
	CCallPorts	m_list[CS_SIM_MAX_CALL_PORTS];

	DWORD 		m_ServiceId;
	DWORD		m_sendCsNewIndCounter;
	string		m_ipServiceName;
	
	PDECLAR_MESSAGE_MAP
};

#endif // __EPSIMCSENDPOINTSTASK_H__
