//+========================================================================+
//                  EpSimSipEndpointsTask.cpp                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpSimSipEndpointsTask.cpp                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#include "EndpointsSimProcess.h"

#include "Trace.h"
#include "Macros.h"

#include "CsStructs.h"
#include "IpCsOpcodes.h"
#include "IpMngrOpcodes.h"
#include "H323CsInd.h"
#include "H323CsReq.h"
#include "OpcodesMcmsCommon.h"
#include "StatusesGeneral.h"

#include "NStream.h"
#include "SocketApi.h"
#include "ClientSocket.h"
#include "MplMcmsProtocol.h"
#include "MplMcmsProtocolTracer.h"
#include "SystemFunctions.h"
#include "TerminalCommand.h"
#include "CfgApi.h"
#include "CSKeys.h"

#include "EndpointsSim.h"
#include "EpSimCSEndpointsTask.h"

#include "TraceStream.h"

#include "EndpointsSimConfig.h"
#include "GKTaskApi.h"
#include "ProxyTaskApi.h"
#include "OpcodesMcmsInternal.h"

/////////////////////////////////////////////////////////////////////////////
//
//   STATES:
//
//const WORD  IDLE        = 0;        // default state -  defined in base class
const WORD  SETUP         = 1;
const WORD  STARTUP       = 2;
const WORD  CONNECT       = 3;
const WORD  PAUSE         = 4;
//const WORD  ANYCASE     = 0xFFFF;   // any other state -  defined in base class

#define REQ_SIM_BUF_SIZE 512

// timers events (101-150)
#define TIMER_CS_NEW_IND	101

/////////////////////////////////////////////////////////////////////////////
//
//   EXTERNALS:
//

//  task creation function
extern "C" void epSimCSEntryPoint(void* appParam);
extern "C" void EpCsApiRxEntryPoint(void* appParam);
extern "C" void EpCsApiTxEntryPoint(void* appParam);

/////////////////////////////////////////////////////////////////////////////
//
//   SimCSEndpointsModule - Central Signaling Sim module Task
//
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSimCSEndpointsModule)
// 	// Manager: connect card
// 	ONEVENT( GIDEON_MSG,  IDLE,   CSimCSEndpointsModule::OnMngrGideonMsgIdle)
	// Manager: connect CS socket
	ONEVENT( CONNECT_CS_SOCKET,      IDLE,    CSimCSEndpointsModule::OnMngrConnectSocketIdle)
	// Socket: connection established
	ONEVENT( SOCKET_CONNECTED,       SETUP,   CSimCSEndpointsModule::OnSocketConnectedSetup)
	// Socket: connection failed
	ONEVENT( SOCKET_FAILED,          SETUP,   CSimCSEndpointsModule::OnSocketFailedSetup)
	// Socket: connection dropped
	ONEVENT( SOCKET_DROPPED,         SETUP,   CSimCSEndpointsModule::OnSocketDroppedSetup)
	ONEVENT( SOCKET_DROPPED,         CONNECT, CSimCSEndpointsModule::OnSocketDroppedConnect)

	ONEVENT( RESUME_SOCKET,          PAUSE,   CSimCSEndpointsModule::OnSocketResumePause)
	ONEVENT( PAUSE_SOCKET,           CONNECT, CSimCSEndpointsModule::OnSocketPauseConnect)

	// Socket : message received
	ONEVENT( SOCKET_RCV_MSG,         IDLE,     CSimCSEndpointsModule::OnSocketRcvIdle)
	ONEVENT( SOCKET_RCV_MSG,         SETUP,    CSimCSEndpointsModule::OnSocketRcvSetup)
	ONEVENT( SOCKET_RCV_MSG,         STARTUP,  CSimCSEndpointsModule::OnSocketRcvStartup)
	ONEVENT( SOCKET_RCV_MSG,         CONNECT,  CSimCSEndpointsModule::OnSocketRcvConnect)

	// timer, CS-Mngr didn't response to CS_NEW_IND
	ONEVENT( TIMER_CS_NEW_IND,       STARTUP,   CSimCSEndpointsModule::OnTimerSendCsNewIndSetup)

	ONEVENT( UPDATE_ENDPS_MBX,        IDLE,  CSimCSEndpointsModule::OnReceiveMbxEndpointsIdle)
//	ONEVENT( UPDATE_H323_MBX,         ANYCASE,  CSimCSEndpointsModule::OnReceiveMbxH323)
//	ONEVENT( UPDATE_SIP_MBX,          IDLE,  CSimCSEndpointsModule::OnReceiveMbxSIP)
//	ONEVENT( UPDATE_PROXY_MBX,        IDLE,  CSimCSEndpointsModule::OnReceiveMbxProxy)
//	ONEVENT( UPDATE_GK_MBX,		      IDLE,  CSimCSEndpointsModule::OnReceiveMbxGK)

	ONEVENT( SEND_TO_CSAPI,          CONNECT,  CSimCSEndpointsModule::OnReceiveCommandToCSAPI)
PEND_MESSAGE_MAP(CSimCSEndpointsModule,CTaskApp);

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//  task creation function
void epSimCSEntryPoint(void* appParam)
{
	CSimCSEndpointsModule*  pEpSimCSTask = new CSimCSEndpointsModule;
	pEpSimCSTask->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CSimCSEndpointsModule::CSimCSEndpointsModule(void)
{
	m_pSocketConnection = NULL;
	m_pEndpointsApi     = new CTaskApi;
//	m_pH323Api = new CTaskApi;
//	m_pSIPApi = new CTaskApi;
	//m_pProxyApi         = new CProxyTaskApi(1);
	//m_pGKeeperApi       = new CTaskApi;

	m_ServiceId = 0xFFFFFFFF;

	m_portCall = 6050;
	int i=0;
	for (i = 0; i < CS_SIM_MAX_CALL_PORTS; i++)
	{
		m_list[i].srcIpAddress = 0;
		m_list[i].port = 0;
	}
	m_sendCsNewIndCounter = 0;
	m_ipServiceName = "";
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CSimCSEndpointsModule::~CSimCSEndpointsModule()     // destructor
{
	PTRACE(eLevelInfoNormal, "CSimCSEndpointsModule::~CSimCSEndpointsModule" );

	POBJDELETE(m_pSocketConnection);
	POBJDELETE(m_pEndpointsApi);
//	POBJDELETE(m_pH323Api);
//	POBJDELETE(m_pSIPApi);
//	POBJDELETE(m_pProxyApi);
//	POBJDELETE(m_pGKeeperApi);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void* CSimCSEndpointsModule::GetMessageMap()
{
	return (void*)m_msgEntries;
}

eEPSimTaskType CSimCSEndpointsModule::GetTaskType(void) const
{
    return eCSTaskType;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::Create(CSegment& appParam)
{
	CCSIDTaskApp::Create(appParam);

//	m_pProxyApi = new CProxyTaskApi(GetCsId());
//	m_pProxyApi->CreateOnlyApi();

//	m_pGKeeperApi = new CGkTaskApi(GetCsId());
//	m_pGKeeperApi->CreateOnlyApi();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
const char* CSimCSEndpointsModule::GetTaskName() const
{
	return "SimCSEndpointsModuleTask";
}

/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::SendToCsApi(CMplMcmsProtocol& rMplProtocol) const
{
	if( !CPObject::IsValidPObjectPtr(m_pSocketConnection) ) {
		PASSERT(1);
		return;
	}

	CSegment  rParamSegment;
	rMplProtocol.Serialize(rParamSegment, CS_API_TYPE);

	SendToCsApi(rParamSegment);
//	CMplMcmsProtocolTracer(rMplProtocol).TraceMplMcmsProtocol("CS_SIM_SEND_TO_CS_API",CS_API_TYPE);
}

/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::SendToCsApi(CSegment& rParam) const
{
	if( !CPObject::IsValidPObjectPtr(m_pSocketConnection) ) {
		PASSERT(1);
		return;
	}

	CSegment* pMsg = new CSegment; // pMsg will not delete inside

	// put parameters to message
	*pMsg  << rParam;

	m_pSocketConnection->Send(pMsg); // pMsg will not delete inside

	POBJDELETE(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// App Manager: establish connection with CS-API

/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnMngrConnectSocketIdle(CSegment* pMsg)
{
	m_state = SETUP;

	char  szIpAddress[100];
	WORD  port=0;

	*pMsg	>> szIpAddress
			>> port;

	POBJDELETE(m_pSocketConnection);
 	m_pSocketConnection = new CClientSocket(this,EpCsApiRxEntryPoint,EpCsApiTxEntryPoint);
	m_pSocketConnection->Init(szIpAddress,port);
	m_pSocketConnection->Connect();
}

/////////////////////////////////////////////////////////////////////////////
// Socket: connection established

/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnSocketConnectedSetup(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnSocketConnectedSetup");

	m_state = STARTUP;

	int status = SendNewCSInd();
}

/////////////////////////////////////////////////////////////////////////////
// Socket: connection established

/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnTimerSendCsNewIndSetup(CSegment* pMsg)
{
	m_sendCsNewIndCounter++;
	char num[20];
	sprintf( num, "%d", m_sendCsNewIndCounter);
	PTRACE2(eLevelError,"CSimCSEndpointsModule::OnTimerSendCsNewIndSetup - send counter: ", num);

	// try again
	int status = SendNewCSInd();
}

/////////////////////////////////////////////////////////////////////////////
// Socket: connection failed

/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnSocketFailedSetup(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnSocketFailedSetup");

	// endless loop
	if( CPObject::IsValidPObjectPtr(m_pSocketConnection) )
		m_pSocketConnection->Connect();
}

/////////////////////////////////////////////////////////////////////////////
// Socket: connection dropped
//

/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnSocketDroppedSetup(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnSocketDroppedSetup");
	DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnSocketDroppedConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnSocketDroppedConnect");
	OnSocketDropped(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnSocketDropped(CSegment* pMsg)
{
	m_state = SETUP;

	if( CPObject::IsValidPObjectPtr(m_pSocketConnection) ) {
		m_pSocketConnection->Disconnect();
		m_pSocketConnection->Connect();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnSocketPauseConnect(CSegment* pMsg)
{
	m_state = PAUSE;
    PTRACE(eLevelInfoNormal,"++++---CSimCSEndpointsModule::OnSocketPause m_state = PAUSE");
	if( CPObject::IsValidPObjectPtr(m_pSocketConnection) ) {
		m_pSocketConnection->Disconnect();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnSocketResumePause(CSegment* pMsg)
{
	m_state = SETUP;
    PTRACE(eLevelInfoNormal,"++++---!!!!!!!!CSimCSEndpointsModule::OnSocketResume m_state = SETUP");
	if( CPObject::IsValidPObjectPtr(m_pSocketConnection) ) {
		m_pSocketConnection->Connect();
		}
}

/////////////////////////////////////////////////////////////////////////////
// Socket: message received
void CSimCSEndpointsModule::OnSocketRcvIdle(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnSocketRcvIdle");
	DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnSocketRcvSetup(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnSocketRcvSetup");
	DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnSocketRcvStartup(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnSocketRcvStartup");

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pMsg, CS_API_TYPE);
	OPCODE opcode = pMplProtocol->getOpcode();

	switch(opcode)
	{
		case CS_NEW_REQ:
			DeleteTimer( TIMER_CS_NEW_IND );
			OnNewCSReq();
			break;

		case CS_CONFIG_PARAM_REQ:
			SendConfigParamInd();
			break;

		case CS_END_CONFIG_PARAM_REQ:
			SendEndConfigParamInd();
			SendEndCSStartupInd();
			m_state = CONNECT;
			break;

		case SIP_CS_PROXY_REGISTER_REQ:
			// the real CS will send a response.
			break;
		case CS_TERMINAL_COMMAND_REQ: {
					OnCsTerminalCommandReq( pMplProtocol );
					break;
				}

		default:
		{
		    CProcessBase* process = CProcessBase::GetProcess();
		    

			if(process)
			{
		        PASSERTSTREAM(1,
		            "Arrived unsupported opcode: " << process->GetOpcodeAsString(opcode)
		                << " (" << opcode << ")");
			}
			else
			{
			    PASSERT(1);
			}
			break;
		}
	}

	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnSocketRcvConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnSocketRcvConnect");
	// deserialize protocol
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pMsg,CS_API_TYPE);

	switch( pMplProtocol->getOpcode() )
	{
		case CS_NEW_REQ:
		case CS_CONFIG_PARAM_REQ:
		case CS_END_CONFIG_PARAM_REQ:
		case CS_LAN_CFG_REQ: {
			// error
			PTRACE(eLevelError,"CSimCSEndpointsModule::OnSocketRcvConnect - Simulation ==>Command Error");
			break;
		}

		case CS_NEW_SERVICE_INIT_REQ:
			OnCsNewServiceInitReq( pMplProtocol );
			break;

		case CS_COMMON_PARAM_REQ:
		{
			OnCsCommonParamReq( pMplProtocol );
			break;
		}

		case CS_END_SERVICE_INIT_REQ:
		{
			OnCsEndServiceInitReq( pMplProtocol );
			break;
		}

		case H323_CS_SIG_GET_PORT_REQ: {
			OnCsSigGetPortReq( pMplProtocol );
			break;
			}

		case CS_TERMINAL_COMMAND_REQ: {
			OnCsTerminalCommandReq( pMplProtocol );
			break;
		}

		case H323_CS_PARTY_KEEP_ALIVE_REQ: {
			OnCsH323PartyKeepAliveReq( pMplProtocol );
			break;
		}

		case SIP_CS_PARTY_KEEP_ALIVE_REQ: {
			OnCsSipPartyKeepAliveReq( pMplProtocol );
			break;
		}

		case CS_KEEP_ALIVE_REQ: {
			PTRACE(eLevelError,"CSimCSEndpointsModule::OnSocketRcvConnect - Simulation ---------------------------------CS_KEEP_ALIVE_REQ");
			OnCsGetKeepAliveReq( pMplProtocol );
			break;
		}
        case CS_PING_REQ: {
			PTRACE(eLevelError,"CSimCSEndpointsModule::OnSocketRcvConnect - Simulation ---------------------------------CS_PING_REQ");
			OnCsPingReq( pMplProtocol );
			break;
		}
		case SIP_CS_PROXY_NOTIFY_REQ: {
			OnCsProxyNotifyReq(pMplProtocol);
			break;
		}
		default   :  {
			WORD targetTask = GetTargetTask( pMplProtocol->getOpcode() ); // ENDPOINTS_TARGET_TASK
			if( targetTask == ENDPOINTS_TARGET_TASK )
				PTRACE(eLevelError,"CSimCSEndpointsModule::OnSocketRcvConnect - Simulation ==>ToEndpoints");
			else if (targetTask == PROXY_TARGET_TASK)
				PTRACE(eLevelError,"CSimCSEndpointsModule::OnSocketRcvConnect - Simulation ==>ToProxy");
			else if (targetTask == GK_TARGET_TASK)
				PTRACE(eLevelError,"CSimCSEndpointsModule::OnSocketRcvConnect - Simulation ==>ToGK");
			else
				PTRACE(eLevelError,"CSimCSEndpointsModule::OnSocketRcvConnect - Simulation ==>IllegalDestination");
			SendMsgToTargetTask( pMplProtocol, targetTask );
			break;
		}

	}

	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnCsSigGetPortReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnCsSigGetPortReq - H323_CS_SIG_GET_PORT_REQ");

	mcReqGetPort* pStruct = (mcReqGetPort*)pMplProtocol->GetData();
	if (NULL == pStruct) {
		PTRACE(eLevelError,"CSimCSEndpointsModule::OnCsSigGetPortReq - GetData is NULL ");
		return;
	}
	APIU32 srcIpAddress = pStruct->srcIpAddress;

	// save IP parameters (to be unique)
	SaveSrcIpAddress( srcIpAddress, m_portCall );

	// send indication
	SendCsSigGetPortInd( pMplProtocol );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnCsH323PartyKeepAliveReq( CMplMcmsProtocol* pMplProtocolReq )
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnCsH323PartyKeepAliveReq - H323_CS_PARTY_KEEP_ALIVE_REQ");

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol(*pMplProtocolReq);

	BYTE fastBuf[4] = {0,0,0,0};

	CsFillCsProtocol( pMplProtocol, H323_CS_PARTY_KEEP_ALIVE_IND, fastBuf, 4);

	SendToCsApi(*pMplProtocol);


	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnCsSipPartyKeepAliveReq( CMplMcmsProtocol* pMplProtocolReq )
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnCsSipPartyKeepAliveReq - SIP_CS_PARTY_KEEP_ALIVE_REQ");

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol(*pMplProtocolReq);
	BYTE fastBuf[4] = {0,0,0,0};
	CsFillCsProtocol( pMplProtocol, SIP_CS_PARTY_KEEP_ALIVE_IND, fastBuf, 4);
	SendToCsApi(*pMplProtocol);

	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnCsNewServiceInitReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnCsNewServiceInitReq - CS_NEW_SERVICE_INIT_REQ");

	BYTE *req = (BYTE*)pMplProtocol->GetData();
	req += sizeof(DWORD);

	cfgBufHndl  commonRequestHandle;
	memset( (BYTE*)&commonRequestHandle, 0, sizeof(cfgBufHndl) );
	char rcvSectionName[STR_LEN];
	memset( rcvSectionName, 0, STR_LEN );
	int			inBufSize		= 0;
	int         numOfKeys       = 0;
	int			inSecNameSize	= STR_LEN;

	int res = ::CfgUnpackBuf(
		&commonRequestHandle,
		(char*) req,
		rcvSectionName,
		&inSecNameSize,
		&inBufSize,
		&numOfKeys);

	if(0 != res)
	{
		PASSERTMSG(1, "FAILED to unpack a message from CSMngr");
	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_NEW_SERVICE))
	{
		const int NumOfParams	= 2;
		const int ParamLen		= 256;

		DWORD serviceId = 0;

		int   keyLen	[NumOfParams] = {ParamLen,ParamLen};
		int	  dataSize	[NumOfParams] = {ParamLen,ParamLen};

		ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
		ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

		res = ::CfgGetParamsContinue(
			&commonRequestHandle,
			NumOfParams,
			commonRequestHandle.pCurBuf,
			keyName[0],	&keyLen[0], dataName[0]		,	&dataSize[0], cfString,
			keyName[1],	&keyLen[1], &serviceId		,   &dataSize[1], cfINT32
			);
		m_ServiceId = serviceId;

		if (0 != res													||
			0 != strncmp(keyName[0], CS_KEY_SERVICE_NAME , keyLen[0])	||
			0 != strncmp(keyName[1], CS_KEY_SERVICE_ID   , keyLen[1]))
		{
			PASSERTMSG(1, "CSimCSEndpointsModule::OnCsNewServiceInitReq  bad parameters");
		}

		// send indication to CSMngr
		PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnCsNewServiceInitReq - CS_NEW_SERVICE_INIT_IND");


		char tmp[REQ_SIM_BUF_SIZE];
		memset( tmp, 0, REQ_SIM_BUF_SIZE );
		char *fastBuf = tmp;
		fastBuf += sizeof(DWORD);

		int allocBufSize=0;
	    res = ::CfgSetParams(	1,
							fastBuf,
							REQ_SIM_BUF_SIZE-sizeof(DWORD),
							&allocBufSize,
							CS_KEY_OK_NEW_SERVICE,
							CS_KEY_SERVICE_NAME, dataName[0], cfString);
		if(0 != res)
		{
			 PTRACE2INT(eLevelInfoHigh,"CCommServiceService::SupportedEndServiceInitReq: - failed with status: ", res);
		}
		else
		{
			m_ipServiceName = dataName[0];
			fastBuf -= sizeof(DWORD);
			*((DWORD*)fastBuf) = (DWORD)allocBufSize;
			CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
			CsFillCsProtocol( pMplProtocol, CS_NEW_SERVICE_INIT_IND,(BYTE*)(fastBuf), allocBufSize + sizeof(DWORD));

			SendToCsApi(*pMplProtocol);

			POBJDELETE(pMplProtocol);
		}

		DEALLOCBUFFERS(keyName ,NumOfParams);
		DEALLOCBUFFERS(dataName,NumOfParams);
	}
	else
	{
		PASSERTMSG(1, "Illegal section name");
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnCsCommonParamReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnCsCommonParamReq - CS_COMMON_PARAM_REQ");

	BYTE *req = (BYTE*)pMplProtocol->GetData();
	req += sizeof(DWORD);

	cfgBufHndl  commonRequestHandle;
	memset( (BYTE*)&commonRequestHandle, 0, sizeof(cfgBufHndl) );
	char rcvSectionName[STR_LEN];
	memset( rcvSectionName, 0, STR_LEN );

	int			inBufSize		= 0;
	int         numOfKeys       = 0;
	int			inSecNameSize	= STR_LEN;
	const int 	ParamLen		= 256;

	int res = ::CfgUnpackBuf(
		&commonRequestHandle,
		(char*) req,
		rcvSectionName,
		&inSecNameSize,
		&inBufSize,
		&numOfKeys);

	if(0 != res)
	{
		PASSERTMSG(1, "OnCsCommonParamReq - FAILED to unpack a message from CSMngr");
	}

	TRACEINTO << "CSimCSEndpointsModule::OnCsCommonParamReq rcvSectionName=" << rcvSectionName;


	if(0 == strcmp(rcvSectionName, CS_KEY_SERVICE_NAME))
	{
		const int NumOfParams	= 2;

		int   keyLen	[NumOfParams] = {ParamLen,ParamLen};
		int	  dataSize	[NumOfParams] = {ParamLen,ParamLen};

		ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
		ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

		res = ::CfgGetParamsContinue(
			&commonRequestHandle,
			NumOfParams,
			commonRequestHandle.pCurBuf,
			keyName[0],	&keyLen[0], dataName[0]		,	&dataSize[0], cfString,
			keyName[1],	&keyLen[1], dataName[1]		,   &dataSize[1], cfINT32
			);

		if (0 != res													||
			0 != strncmp(keyName[0], CS_KEY_SERVICE_NAME , keyLen[0])	||
			0 != strncmp(keyName[1], CS_KEY_SERVICE_ID   , keyLen[1]))
		{
			PASSERTMSG(1, "CSimCSEndpointsModule::OnCsCommonParamReq  bad parameters");
		}
		DEALLOCBUFFERS(keyName ,NumOfParams);
		DEALLOCBUFFERS(dataName,NumOfParams);
	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_PORT_RANGE))
	{
		const int NumOfParams	= 2;

		int   keyLen	[NumOfParams] = {ParamLen,ParamLen};
		int	  dataSize	[NumOfParams] = {ParamLen,ParamLen};

		ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
		ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

		res = ::CfgGetParamsContinue(
			&commonRequestHandle,
			NumOfParams,
			commonRequestHandle.pCurBuf,
			keyName[0],	&keyLen[0], dataName[0]		,	&dataSize[0], cfINT16,
			keyName[1],	&keyLen[1], dataName[1]		,   &dataSize[1], cfINT16
			);

		if (0 != res													||
			0 != strncmp(keyName[0], CS_KEY_TCP_FIRST_PORT , keyLen[0])	||
			0 != strncmp(keyName[1], CS_KEY_TCP_NUM_PORTS   , keyLen[1]))
		{
			PASSERTMSG(1, "CSimCSEndpointsModule::OnCsCommonParamReq  bad parameters");
		}
		DEALLOCBUFFERS(keyName ,NumOfParams);
		DEALLOCBUFFERS(dataName,NumOfParams);

	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_SIP_TRANSPORT_TYPE))
	{
		const int NumOfParams	= 1;

		int   keyLen	[NumOfParams] = {ParamLen};
		int	  dataSize	[NumOfParams] = {ParamLen};

		ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
		ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

		res = ::CfgGetParamsContinue(
			&commonRequestHandle,
			NumOfParams,
			commonRequestHandle.pCurBuf,
			keyName[0],	&keyLen[0], dataName[0]		,	&dataSize[0], cfINT32
			);

		if (0 != res													||
			0 != strncmp(keyName[0], CS_KEY_TYPE , keyLen[0]))
		{
			PASSERTMSG(1, "CSimCSEndpointsModule::OnCsCommonParamReq  bad parameters");
		}
		DEALLOCBUFFERS(keyName ,NumOfParams);
		DEALLOCBUFFERS(dataName,NumOfParams);

	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_IP_INTERFACE))
	{
		const int NumOfParams	= 14;

		int   keyLen	[NumOfParams] = {ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen};
		int	  dataSize	[NumOfParams] = {ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen};

		ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
		ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

		res = ::CfgGetParamsContinue(
			&commonRequestHandle,
			NumOfParams,
			commonRequestHandle.pCurBuf,
			keyName[0],		&keyLen[0], dataName[0]		,	&dataSize[0], cfString,
			keyName[1],		&keyLen[1], dataName[1]		,   &dataSize[1], cfBOOL,
			keyName[2],		&keyLen[2], dataName[2]		,	&dataSize[2], cfBOOL,
			keyName[3],		&keyLen[3], dataName[3]		,   &dataSize[3], cfINT32,
			keyName[4],		&keyLen[4], dataName[4]		,	&dataSize[4], cfINT32,

			keyName[5],		&keyLen[5], dataName[5]		,   &dataSize[5], cfIpAddress,
			keyName[6],		&keyLen[6], dataName[6]		,	&dataSize[6], cfIpAddress,
			keyName[7],		&keyLen[7], dataName[7]		,   &dataSize[7], cfIpAddress,
			keyName[8],		&keyLen[8], dataName[8]		,	&dataSize[8], cfIpAddress,
			keyName[9],		&keyLen[9], dataName[9]		,   &dataSize[9], cfIpAddress,

			keyName[10],	&keyLen[10], dataName[10]		,	&dataSize[10], cfString,
			keyName[11],	&keyLen[11], dataName[11]		,   &dataSize[11], cfString,
			keyName[12],	&keyLen[12], dataName[12]		,	&dataSize[12], cfString,
			keyName[13],	&keyLen[13], dataName[13]		,	&dataSize[13], cfString
			);

		if (0 != res															||
			0 != strncmp(keyName[0], 	CS_KEY_IF_NAME 			, keyLen[0])		||
			0 != strncmp(keyName[1], 	CS_KEY_IF_STATE   		, keyLen[1])  	||
			0 != strncmp(keyName[2], 	CS_KEY_VLAN_SUPPORT 	, keyLen[2])	||
			0 != strncmp(keyName[3], 	CS_KEY_VLAN_PRIORITY   	, keyLen[3]) 	||
			0 != strncmp(keyName[4], 	CS_KEY_VLAN_ID 			, keyLen[4])		||
			0 != strncmp(keyName[5], 	CS_KEY_IPv4_ADDRESS     , keyLen[5]) 	||
			0 != strncmp(keyName[6], 	CS_KEY_IPv4_DEFAULT_GATEWAY , keyLen[6])	||
			0 != strncmp(keyName[7], 	CS_KEY_IPv4_SUBNET_MASK     , keyLen[7]) ||
			0 != strncmp(keyName[8], 	CS_KEY_IPv4_MTU 		    , keyLen[8])	||
			0 != strncmp(keyName[9], 	CS_KEY_IPv6_6to4_RELAY_ADDRESS, keyLen[9]) ||
			0 != strncmp(keyName[10], 	CS_KEY_IPv6_ADDRESS 		, keyLen[10]) ||
			0 != strncmp(keyName[11], 	CS_KEY_IPv6_SUBNET_MASK   	, keyLen[11]) ||
			0 != strncmp(keyName[12], 	CS_KEY_IPv6_CONFIG_TYPE 	, keyLen[12]) ||
			0 != strncmp(keyName[13], 	CS_KEY_IP_INTERFACE_TYPE 	, keyLen[13])
			)
		{
			PASSERTMSG(1, "CSimCSEndpointsModule::OnCsCommonParamReq  bad parameters");
		}

		//ipv4
		const DWORD *ptrCSIp = (const DWORD *)dataName[5];
		const DWORD CSIp = *ptrCSIp;

		char buff[256];
		SystemDWORDToIpString(CSIp, buff);

		DWORD CSIpReverse = SystemIpStringToDWORD(buff, eNetwork);
		char buffReverse[256];
		SystemDWORDToIpString(CSIpReverse, buffReverse);

		//TRACEINTO << "CSimCSEndpointsModule::OnCsCommonParamReq: " << keyName[5] << ": " << buff;
		TRACEINTO << "CSimCSEndpointsModule::OnCsCommonParamReq: " << keyName[5] << ": " << buffReverse;

		::GetEpSystemCfg()->SetCSIpAddress(buffReverse);

		//ipv6
		const char* ptrCSIpv6 = (const char*)dataName[10];

		TRACEINTO << "CSimCSEndpointsModule::OnCsCommonParamReq: IPv6 " << keyName[10] << ": " << ptrCSIpv6;

		::GetEpSystemCfg()->SetCSIpV6Address(ptrCSIpv6);

		DEALLOCBUFFERS(keyName ,NumOfParams);
		DEALLOCBUFFERS(dataName,NumOfParams);
	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_SUPPORTED_PROTOCOLS))
	{
		const int NumOfParams	= 2;

		int   keyLen	[NumOfParams] = {ParamLen,ParamLen};
		int	  dataSize	[NumOfParams] = {ParamLen,ParamLen};

		ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
		ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

		res = ::CfgGetParamsContinue(
			&commonRequestHandle,
			NumOfParams,
			commonRequestHandle.pCurBuf,
			keyName[0],	&keyLen[0], dataName[0]		,	&dataSize[0], cfBOOL,
			keyName[1],	&keyLen[1], dataName[1]		,   &dataSize[1], cfBOOL
			);

		if (0 != res													||
			0 != strncmp(keyName[0], CS_KEY_H323 , keyLen[0])	||
			0 != strncmp(keyName[1], CS_KEY_SIP   , keyLen[1]))
		{
			PASSERTMSG(1, "CSimCSEndpointsModule::OnCsCommonParamReq  bad parameters");
		}
		DEALLOCBUFFERS(keyName ,NumOfParams);
		DEALLOCBUFFERS(dataName,NumOfParams);

	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_DNS_CFG))
	{
		const int NumOfParams	= 2;

		int   keyLen	[NumOfParams] = {ParamLen,ParamLen};
		int	  dataSize	[NumOfParams] = {ParamLen,ParamLen};

		ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
		ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

		res = ::CfgGetParamsContinue(
			&commonRequestHandle,
			NumOfParams,
			commonRequestHandle.pCurBuf,
			keyName[0],	&keyLen[0], dataName[0]		,	&dataSize[0], cfBOOL,
			keyName[1],	&keyLen[1], dataName[1]		,   &dataSize[1], cfBOOL
			);

		if (0 != res													||
			0 != strncmp(keyName[0], CS_KEY_USE 			, keyLen[0])	||
			0 != strncmp(keyName[1], CS_KEY_GET_FROM_DHCP   , keyLen[1]))
		{
			PASSERTMSG(1, "CSimCSEndpointsModule::OnCsCommonParamReq  bad parameters CS_KEY_DNS_CFG");
		}
		DEALLOCBUFFERS(keyName ,NumOfParams);
		DEALLOCBUFFERS(dataName,NumOfParams);

	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_DNS_CALLS))
	{
		const int NumOfParams	= 3;

		int   keyLen	[NumOfParams] = {ParamLen,ParamLen,ParamLen};
		int	  dataSize	[NumOfParams] = {ParamLen,ParamLen,ParamLen};

		ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
		ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

		res = ::CfgGetParamsContinue(
			&commonRequestHandle,
			NumOfParams,
			commonRequestHandle.pCurBuf,
			keyName[0],	&keyLen[0], dataName[0]		,	&dataSize[0], cfBOOL,
			keyName[1],	&keyLen[1], dataName[1]		,   &dataSize[1], cfBOOL,
			keyName[2],	&keyLen[2], dataName[2]		,   &dataSize[2], cfString
			);

		if (0 != res													||
			0 != strncmp(keyName[0], CS_KEY_ACCEPT_DNS_CALLS , keyLen[0])	||
			0 != strncmp(keyName[1], CS_KEY_REGISTER_TO_DNS   , keyLen[1]) ||
			0 != strncmp(keyName[2], CS_KEY_HOST_NAME     , keyLen[2]))
		{
			PASSERTMSG(1, "CSimCSEndpointsModule::OnCsCommonParamReq  bad parameters CS_KEY_DNS_CALLS");
		}
		DEALLOCBUFFERS(keyName ,NumOfParams);
		DEALLOCBUFFERS(dataName,NumOfParams);

	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_DNS_NAME))
	{
		const int NumOfParams	= 3;

		int   keyLen	[NumOfParams] = {ParamLen,ParamLen,ParamLen};
		int	  dataSize	[NumOfParams] = {ParamLen,ParamLen,ParamLen};

		ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
		ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

		res = ::CfgGetParamsContinue(
			&commonRequestHandle,
			NumOfParams,
			commonRequestHandle.pCurBuf,
			keyName[0],	&keyLen[0], dataName[0]		,	&dataSize[0], cfBOOL,
			keyName[1],	&keyLen[1], dataName[1]		,   &dataSize[1], cfString,
			keyName[2],	&keyLen[2], dataName[2]		,   &dataSize[2], cfString
			);

		if (0 != res													||
			0 != strncmp(keyName[0], CS_KEY_GET_FROM_DHCP , keyLen[0])	||
			0 != strncmp(keyName[1], CS_KEY_DOMAIN_NAME   , keyLen[1]) ||
			0 != strncmp(keyName[2], CS_KEY_HOST_NAME     , keyLen[2]))
		{
			PASSERTMSG(1, "CSimCSEndpointsModule::OnCsCommonParamReq  bad parameters CS_KEY_DNS_CFG");
		}
		DEALLOCBUFFERS(keyName ,NumOfParams);
		DEALLOCBUFFERS(dataName,NumOfParams);

	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_AUTHENTICATION_ENTRY))
	{
		const int NumOfParams	= 7;

		int   keyLen	[NumOfParams] = {ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen};
		int	  dataSize	[NumOfParams] = {ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen,ParamLen};

		ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
		ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

		res = ::CfgGetParamsContinue(
			&commonRequestHandle,
			NumOfParams,
			commonRequestHandle.pCurBuf,
			keyName[0],	&keyLen[0], dataName[0]		,	&dataSize[0], cfString,
			keyName[1],	&keyLen[1], dataName[1]		,   &dataSize[1], cfString,
			keyName[2],	&keyLen[2], dataName[2]		,   &dataSize[2], cfString,
			keyName[3],	&keyLen[3], dataName[3]		,	&dataSize[3], cfString,
			keyName[4],	&keyLen[4], dataName[4]		,   &dataSize[4], cfString,
			keyName[5],	&keyLen[5], dataName[5]		,   &dataSize[5], cfString,
			keyName[6],	&keyLen[6], dataName[6]		,   &dataSize[6], cfIpAddress
			);

		if (0 != res													||
			0 != strncmp(keyName[0], CS_KEY_ACTION , keyLen[0])	||
			0 != strncmp(keyName[1], CS_KEY_PROTOCOL   , keyLen[1]) 	||
			0 != strncmp(keyName[2], CS_KEY_USER     , keyLen[2]) 	||
			0 != strncmp(keyName[3], CS_KEY_PASSWORD , keyLen[3])	||
			0 != strncmp(keyName[4], CS_KEY_DOMAIN_NAME   , keyLen[4]) 	||
			0 != strncmp(keyName[5], CS_KEY_SERVER_NAME     , keyLen[5])	||
			0 != strncmp(keyName[6], CS_KEY_SERVER_IP_ADDRESS , keyLen[6])
			)
		{
			PASSERTMSG(1, "CSimCSEndpointsModule::OnCsCommonParamReq  bad parameters CS_KEY_AUTHENTICATION_ENTRY");
		}
		DEALLOCBUFFERS(keyName ,NumOfParams);
		DEALLOCBUFFERS(dataName,NumOfParams);

	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_SIP_SPAN_TYPE))
	{
		const int NumOfParams	= 2;

		int   keyLen	[NumOfParams] = {ParamLen,ParamLen};
		int	  dataSize	[NumOfParams] = {ParamLen,ParamLen};

		ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
		ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

		res = ::CfgGetParamsContinue(
			&commonRequestHandle,
			NumOfParams,
			commonRequestHandle.pCurBuf,
			keyName[0],	&keyLen[0], dataName[0]		,	&dataSize[0], cfString,
			keyName[1],	&keyLen[1], dataName[1]		,   &dataSize[1], cfString
			);

		if (0 != res													||
			0 != strncmp(keyName[0], CS_KEY_TYPE 			, keyLen[0])	||
			0 != strncmp(keyName[1], CS_KEY_NAME   , keyLen[1]))
		{
			PASSERTMSG(1, "CSimCSEndpointsModule::OnCsCommonParamReq  bad parameters CS_KEY_SIP_SPAN_TYPE");
		}
		DEALLOCBUFFERS(keyName ,NumOfParams);
		DEALLOCBUFFERS(dataName,NumOfParams);

	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_DHCP))
	{
		const int NumOfParams	= 3;

		int   keyLen	[NumOfParams] = {ParamLen,ParamLen,ParamLen};
		int	  dataSize	[NumOfParams] = {ParamLen,ParamLen,ParamLen};

		ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
		ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

		res = ::CfgGetParamsContinue(
			&commonRequestHandle,
			NumOfParams,
			commonRequestHandle.pCurBuf,
			keyName[0],	&keyLen[0], dataName[0]		,	&dataSize[0], cfBOOL,
			keyName[1],	&keyLen[1], dataName[1]		,   &dataSize[1], cfBOOL,
			keyName[2],	&keyLen[2], dataName[2]		,   &dataSize[2], cfBOOL
			);

		if (0 != res																	||
			0 != strncmp(keyName[0], CS_KEY_DHCP_ENABLED 				, keyLen[0])	||
			0 != strncmp(keyName[1], CS_KEY_GET_SIP_PROXY_FROM_DHCP     , keyLen[1]) 	||
			0 != strncmp(keyName[2], CS_KEY_GET_GK_IP_FROM_DHCP 		, keyLen[2])
			)
		{
			PASSERTMSG(1, "CSimCSEndpointsModule::OnCsCommonParamReq  bad parameters CS_KEY_DHCP");
		}
		DEALLOCBUFFERS(keyName ,NumOfParams);
		DEALLOCBUFFERS(dataName,NumOfParams);
	}
	else
	{
		PASSERTMSG(1, "Illegal section name");
	}

	// Section ok indication to CSMngr (for all common commands)
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnCsNewServiceInitReq - CS_NEW_SERVICE_INIT_IND");


	char tmp[REQ_SIM_BUF_SIZE];
	memset( tmp, 0, REQ_SIM_BUF_SIZE );
	char *fastBuf = tmp;
	fastBuf += sizeof(DWORD);

	int allocBufSize=0;
    res = ::CfgSetParams(	1,
						fastBuf,
						REQ_SIM_BUF_SIZE-sizeof(DWORD),
						&allocBufSize,
						CS_KEY_OK_SECTION,
						CS_KEY_SECTION_NAME, rcvSectionName, cfString);
	if(0 != res)
	{
		 PTRACE2INT(eLevelInfoHigh,"CCommServiceService::SupportedEndServiceInitReq: - failed with status: ", res);
		 return;
	}
	fastBuf -= sizeof(DWORD);
	*((DWORD*)fastBuf) = (DWORD)allocBufSize;

	CMplMcmsProtocol* pMplProt = new CMplMcmsProtocol;

	CsFillCsProtocol( pMplProt, CS_COMMON_PARAM_IND,(BYTE*)(fastBuf), allocBufSize + sizeof(DWORD));

	SendToCsApi(*pMplProt);

	POBJDELETE(pMplProt);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnCsEndServiceInitReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnCsEndServiceInitReq - CS_END_SERVICE_INIT_REQ");

	BYTE *req = (BYTE*)pMplProtocol->GetData();
	req += sizeof(DWORD);

	cfgBufHndl  commonRequestHandle;
	memset( (BYTE*)&commonRequestHandle, 0, sizeof(cfgBufHndl) );
	char rcvSectionName[STR_LEN];
	memset( rcvSectionName, 0, STR_LEN );

	int			inBufSize		= 0;
	int         numOfKeys       = 0;
	int			inSecNameSize	= STR_LEN;
	const int 	ParamLen		= 256;

	int res = ::CfgUnpackBuf(
		&commonRequestHandle,
		(char*) req,
		rcvSectionName,
		&inSecNameSize,
		&inBufSize,
		&numOfKeys);

	if(0 != res)
	{
		PASSERTMSG(1, "FAILED to unpack a message from CSMngr");
	}
	else if(0 == strcmp(rcvSectionName, CS_KEY_END_SERVICE))
	{
		const int NumOfParams	= 1;

		DWORD serviceId = 0;

		int   keyLen	[NumOfParams] = {ParamLen};
		int	  dataSize	[NumOfParams] = {ParamLen};

		ALLOCBUFFERS(keyName ,ParamLen,NumOfParams);
		ALLOCBUFFERS(dataName,ParamLen,NumOfParams);

		res = ::CfgGetParamsContinue(
			&commonRequestHandle,
			NumOfParams,
			commonRequestHandle.pCurBuf,
			keyName[0],	&keyLen[0], dataName[0]		,	&dataSize[0], cfString
			);

		if (0 != res													||
			0 != strncmp(keyName[0], CS_KEY_SERVICE_NAME , keyLen[0]))
		{
			PASSERTMSG(1, "CSimCSEndpointsModule::OnCsEndServiceInitReq  bad parameters");
		}
		DEALLOCBUFFERS(keyName ,NumOfParams);
		DEALLOCBUFFERS(dataName,NumOfParams);
	}
	else
	{
		PASSERTMSG(1, "Illegal section name");
	}

	// sends ind
	bool isSendOk = true; // false ; //
	if(isSendOk)
	{
		SendEndIpServiceInitInd();
	}
	else
	{
		SendErrorEndServiceInd();
		return;
	}


	// send h323(GK) info to CSMngr.
	/////////////////////////////////////
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnCsNewServiceInitReq - send h323(GK) info");

	char tmp[REQ_SIM_BUF_SIZE + sizeof(DWORD)];
	memset( tmp, 0, sizeof(tmp));
	char *fastBuf = tmp;
	fastBuf += sizeof(DWORD);

	int allocBufSize=0;
    res = ::CfgSetParams(	6,
						fastBuf,
						REQ_SIM_BUF_SIZE,
						&allocBufSize,
						"H323_INFO",
						CS_KEY_SERVICE_ID	, m_ServiceId		, cfINT32,
						CS_KEY_NUM_OF_GKS	, 2					, cfINT32,
						CS_KEY_GK_1_IP		, 0x01020304		, cfIpAddress,
						CS_KEY_GK_1_NAME	, "Sim-GK_Name1"	, cfString,
						CS_KEY_GK_2_IP		, 0x05060708		, cfIpAddress,
						CS_KEY_GK_2_NAME	, "Sim-GK_Name2"	, cfString);

	if(0 != res)
	{
		 PTRACE2INT(eLevelInfoHigh,"CCommServiceService::SupportedEndServiceInitReq: - failed with status: ", res);
		 return;
	}
	fastBuf -= sizeof(DWORD);
	*((DWORD*)fastBuf) = (DWORD)allocBufSize;

	CMplMcmsProtocol* pMplProt = new CMplMcmsProtocol;

	CsFillCsProtocol( pMplProt, CS_COMMON_PARAM_IND,(BYTE*)(fastBuf), allocBufSize + sizeof(DWORD));

	SendToCsApi(*pMplProt);

	POBJDELETE(pMplProt);
	//DEALLOCBUFFER(fastBuf);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::SendEndIpServiceInitInd()
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::SendEndIpServiceInitInd - CS_END_SERVICE_INIT_IND");

	char tmp[REQ_SIM_BUF_SIZE + sizeof(DWORD)];
	memset( tmp, 0, sizeof(tmp));
	char *fastBuf = tmp;
	fastBuf += sizeof(DWORD);

	int allocBufSize = 0;
    int res = ::CfgSetParams(	1,
						fastBuf,
						REQ_SIM_BUF_SIZE,
						&allocBufSize,
						CS_KEY_OK_END_SERVICE,
						CS_KEY_SERVICE_NAME	, m_ipServiceName.c_str(), cfString
						);

	if(0 != res)
	{
		 PTRACE2INT(eLevelError,"CCommServiceService::SupportedEndServiceInitReq: - failed with status: ", res);
		 return;
	}
	fastBuf -= sizeof(DWORD);
	*((DWORD*)fastBuf) = (DWORD)allocBufSize;


	CMplMcmsProtocol* pMplProtInd = new CMplMcmsProtocol;

	CsFillCsProtocol( pMplProtInd, CS_END_SERVICE_INIT_IND,(BYTE*)(fastBuf), allocBufSize + sizeof(DWORD));

	SendToCsApi(*pMplProtInd);

	POBJDELETE(pMplProtInd);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::SendErrorEndServiceInd()
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::SendErrorEndServiceInd - CS_END_SERVICE_INIT_IND");

	char tmp[REQ_SIM_BUF_SIZE + sizeof(DWORD)];
	memset( tmp, 0, sizeof(tmp));
	char *fastBuf = tmp;
	fastBuf += sizeof(DWORD);

	int allocBufSize = 0;
    int res = ::CfgSetParams(	4,
						fastBuf,
						REQ_SIM_BUF_SIZE,
						&allocBufSize,
						CS_KEY_ERROR_END_SERVICE,
						CS_KEY_SECTION_NAME	, "EndServiceInd"	, cfString,
						CS_KEY_KEY			, "key"				, cfString,
						CS_KEY_DATA			, "data"			, cfString,
						CS_KEY_REASON		, "Simulation"		, cfString
						);

	if(0 != res)
	{
		 PTRACE2INT(eLevelError,"CSimCSEndpointsModule::SendErrorEndServiceInd: - failed with status: ", res);
		 return;
	}
	fastBuf -= sizeof(DWORD);
	*((DWORD*)fastBuf) = (DWORD)allocBufSize;


	CMplMcmsProtocol* pMplProtInd = new CMplMcmsProtocol;

	CsFillCsProtocol( pMplProtInd, CS_END_SERVICE_INIT_IND,(BYTE*)(fastBuf), allocBufSize + sizeof(DWORD));

	SendToCsApi(*pMplProtInd);

	POBJDELETE(pMplProtInd);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::SaveSrcIpAddress( APIU32 srcIpAddress, APIU32 port )
{
	//SaveSrcIpAddress( srcIpAddress );
	// find empty place

	// save IP address (remove it upon disconnecting)
	int i=0;
	for (i = 0; i < CS_SIM_MAX_CALL_PORTS; i++)
	{
		if (m_list[i].srcIpAddress == 0)
			break;
	}

	if (i < CS_SIM_MAX_CALL_PORTS)
	{
		m_list[i].srcIpAddress = srcIpAddress;
		m_list[i].port = port;
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::ReleaseSrcIpAddress( APIU32 srcIpAddress, APIU32 port )
{
	// release IP address (remove it upon disconnecting)
	int i=0;
	for (i = 0; i < CS_SIM_MAX_CALL_PORTS; i++)
	{
		if ((m_list[i].srcIpAddress == srcIpAddress) &&
			(m_list[i].port == port))
		{
			m_list[i].srcIpAddress = 0;
			m_list[i].port = 0;
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::SendCsSigGetPortInd( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::SendCsSigGetPortInd - H323_CS_SIG_GET_PORT_IND ");

	// create basic ind
	mcIndGetPort *ind = new mcIndGetPort;
	WORD indLen = sizeof(mcIndGetPort);
	memset( ind, 0, indLen);	// zeroing the struct

	// XML type and size
	ind->srcCallSignalAddress.unionProps.unionType =  eIpVersion4;
	ind->srcCallSignalAddress.unionProps.unionSize =  sizeof(ipAddressIf);

	// type
	ind->srcCallSignalAddress.transAddr.ipVersion = eIpVersion4;

	// ip-type (v4 or v6)

	// ip v4
	ind->srcCallSignalAddress.transAddr.addr.v4.ip = DONOT_CARE; //0;//DONOT_CARE;				// don't care

	// port
	ind->srcCallSignalAddress.transAddr.port = m_portCall;					// don't care (above 6000)'
	m_portCall++;	// for the next port

	// distribution
	ind->srcCallSignalAddress.transAddr.distribution = 0;//(eCmDistributionUnicast) //DONOT_CARE;
	//ind->srcCallSignalAddress.transAddr.distribution = DONOT_CARE;

	CMplMcmsProtocol* pCsProt = new CMplMcmsProtocol(*pMplProtocol);
	// send the struct to CSAPI
	CsFillCsProtocol( pCsProt,H323_CS_SIG_GET_PORT_IND,(BYTE*)(&(*ind)),sizeof(mcIndGetPort));

	SendToCsApi(*pCsProt);

	POBJDELETE(pCsProt);

//	POBJDELETE(pMplProtocol);
/*	CSegment *pMsg = new CSegment;
	*pMsg << (DWORD)H323_CS_SIG_GET_PORT_IND;
	*pMsg << (WORD)indLen;
	pMsg->Put( (BYTE*)ind, indLen );

	OnReceiveCommandToCSAPI( pMsg );

	POBJDELETE( pMsg );*/

	delete ind;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int CSimCSEndpointsModule::SendNewCSInd()
{
	// send New CS IND in order to start the MCMS CS startup
	PTRACE(eLevelInfoNormal, "CSimCSEndpointsModule::SendNewCSInd - CS_NEW_IND");

	// prepare & send CS_NEW indication
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	CS_New_Ind_S newCSInd;

	newCSInd.id = 1234;
	newCSInd.version.ver_release  = 1;
	newCSInd.version.ver_major    = 2;
	newCSInd.version.ver_minor    = 3;
	newCSInd.version.ver_internal = 4;

	CsFillCsProtocol(pMplProtocol,
                     CS_NEW_IND,
                     (BYTE*)(&newCSInd),
                     sizeof newCSInd);

	SendToCsApi(*pMplProtocol);

	POBJDELETE(pMplProtocol);

	StartTimer( TIMER_CS_NEW_IND, SECOND);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int CSimCSEndpointsModule::OnNewCSReq()
{
	PTRACE(eLevelInfoNormal, "CSimCSEndpointsModule::OnNewCSReq - CS_NEW_REQ");

	// nothing to do
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int CSimCSEndpointsModule::SendConfigParamInd()
{
	PTRACE(eLevelInfoNormal, "CSimCSEndpointsModule::SendConfigParamInd - CS_CONFIG_PARAM_IND");

	// prepare & send CS_CONFIG indication
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	CS_Config_Ind_S configInd;

	configInd.result = STATUS_OK;

	strncpy( configInd.str_section_name, "Section_Name", STR_LEN);
	configInd.str_section_name[STR_LEN - 1] = '\0';

	strncpy( configInd.str_key_name, "Key_Name", STR_LEN);
	configInd.str_key_name[STR_LEN - 1] = '\0';

	strncpy( configInd.str_error_reason, "No Reason", STR_LEN);
	configInd.str_error_reason[STR_LEN - 1] = '\0';

	CsFillCsProtocol(pMplProtocol,
                     CS_CONFIG_PARAM_IND,
                     (BYTE*)(&configInd),
                     sizeof configInd);

	SendToCsApi(*pMplProtocol);

	POBJDELETE(pMplProtocol);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int CSimCSEndpointsModule::SendEndConfigParamInd()
{
	PTRACE(eLevelInfoNormal, "CSimCSEndpointsModule::SendEndConfigParamInd - CS_END_CONFIG_PARAM_IND");

	// prepare & send CS_END_CONFIG indication
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	CS_End_Config_Ind_S endConfigInd;

	endConfigInd.result = STATUS_OK;
	strncpy( endConfigInd.str_error_reason, "No Reason", STR_LEN);
	endConfigInd.str_error_reason[STR_LEN - 1] = '\0';

	CsFillCsProtocol(pMplProtocol,
                     CS_END_CONFIG_PARAM_IND,
                     (BYTE*)(&endConfigInd),
                     sizeof endConfigInd);

	SendToCsApi(*pMplProtocol);

	POBJDELETE(pMplProtocol);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int CSimCSEndpointsModule::SendEndCSStartupInd()
{
	PTRACE(eLevelInfoNormal, "CSimCSEndpointsModule::SendEndCSStartupInd - CS_END_CS_STARTUP_IND");

	// prepare & send CS_END_STARTUP indication
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	CS_End_StartUp_Ind_S endStartupInd;

	endStartupInd.result = STATUS_OK;
	strncpy( endStartupInd.str_error_reason, "No Reason", STR_LEN);
	endStartupInd.str_error_reason[STR_LEN - 1] = '\0';

	CsFillCsProtocol(pMplProtocol,
                     CS_END_CS_STARTUP_IND,
                     (BYTE*)(&endStartupInd),
                     sizeof endStartupInd);

	SendToCsApi(*pMplProtocol);

	POBJDELETE(pMplProtocol);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::CsFillCsProtocol(CMplMcmsProtocol* pMplProt,
                                             const DWORD       opcode,
                                             const BYTE*       pData,
                                             const DWORD       nDataLen) const
{
	::FillCsProtocol(pMplProt, GetCSID(), opcode, pData, nDataLen);

/*
	DWORD payloadLen = sizeof(COMMON_HEADER_S)
                         + sizeof(MESSAGE_DESCRIPTION_HEADER_S)
                         + sizeof(CENTRAL_SIGNALING_HEADER_S)
                         + sizeof(PORT_DESCRIPTION_HEADER_S) * pMplProt->getPortDescriptionHeaderCounter()
			 + nDataLen;

	DWORD payloadOffset = sizeof(COMMON_HEADER_S);

	pMplProt->AddCommonHeader(opcode, MPL_PROTOCOL_VERSION_NUM, 0,
                                  (BYTE)eCentral_signaling, (BYTE)eMcms,
                                  ::SystemGetTickCount().GetIntegerPartForTrace(),
                                  payloadLen, payloadOffset, (DWORD)eHeaderMsgDesc,
                                  sizeof(MESSAGE_DESCRIPTION_HEADER_S));

	pMplProt->AddMessageDescriptionHeader(123, (DWORD)eCentral_signaling,
                                              ::SystemGetTickCount().GetIntegerPartForTrace(),
                                              (DWORD)eHeaderCs,
                                              sizeof(CENTRAL_SIGNALING_HEADER_S));

	pMplProt->AddCSHeader(GetCSID(),
                              pMplProt->getCentralSignalingHeaderDestUnitId(),
                              pMplProt->getCentralSignalingHeaderSrcUnitId());

	pMplProt->AddData(nDataLen, (const char*)pData);
*/
}

//#define SIP_CS_SIG_REGISTER_REQ 777

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
WORD CSimCSEndpointsModule::GetTargetTask( const OPCODE opcode ) const
{
	// return SIP or H323 according to message

	if( opcode >= SIP_CS_SIG_FIRST_REQ  && opcode <= SIP_CS_SIG_LAST_REQ )
		return ENDPOINTS_TARGET_TASK;
	else if( opcode >= H323_CS_SIG_FIRST_REQ  && opcode <= H323_CS_SIG_LAST_REQ )
		return ENDPOINTS_TARGET_TASK;
	else if( opcode >= PROXY_TO_CS_FIRST_OPCODE_IN_RANGE && opcode <= PROXY_TO_CS_LAST_OPCODE_IN_RANGE )
		return PROXY_TARGET_TASK;
	else if( opcode >= GATE_KEEPER_MNGR_TO_CS_FIRST_OPCODE_IN_RANGE && opcode <= GATE_KEEPER_MNGR_TO_CS_LAST_OPCODE_IN_RANGE)
		return GK_TARGET_TASK;

	switch( opcode )
	{
//		case SIP_CS_SIG_REGISTER_REQ:
	//	{
		//	return PROXY_TARGET_TASK;
//		}

		default:
		{
			return ILLEGAL_TARGET_TASK;
		}
	}

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::SendMsgToTargetTask( CMplMcmsProtocol* pMplProtocol, const WORD targetTask ) const
{
	if (targetTask == ENDPOINTS_TARGET_TASK)
	{ // send to H323 Task

	    // CS ID of the task should be the same as at the MPL header
	    if(GetCSID() != pMplProtocol->getCentralSignalingHeaderCsId())
	    {
            TRACEINTO << "CS[" << GetCSID()
                      << "] doesn't match to MPL["
                      << pMplProtocol->getCentralSignalingHeaderCsId()
                      << "]";

            // TODO (drabkin) remove temporary fix CS ID in the header
            const CENTRAL_SIGNALING_HEADER_S& hdr = pMplProtocol->GetCSHeaderConst();
            ((CENTRAL_SIGNALING_HEADER_S&)hdr).cs_id = GetCSID();
	    }

		CSegment* pMsg = new CSegment;
		pMplProtocol->Serialize(*pMsg, CS_API_TYPE);
		if(m_pEndpointsApi)
		{
			m_pEndpointsApi->SendMsg(pMsg,SIM_CS_RCV_MSG);
		}
		else
		{
			DBGPASSERT(pMplProtocol->getOpcode());
			POBJDELETE(pMsg);
		}
	}
	else if (targetTask == PROXY_TARGET_TASK)
	{ // send to Proxy Task
		CSegment* pMsg =  new CSegment;
		pMplProtocol->Serialize( *pMsg, CS_API_TYPE );

		CProxyTaskApi api;
		if(api.CreateOnlyApi() >= 0)
		    api.SendMsg(pMsg, CSAPI_TO_PROXY);

//		if (m_pProxyApi)
//			m_pProxyApi->SendMsg(pMsg,CSAPI_TO_PROXY);
//		else {
//			DBGPASSERT(pMplProtocol->getOpcode());
//			POBJDELETE(pMsg);
//		}
	}
	else if (targetTask == GK_TARGET_TASK) {		// send to GK Task
		CSegment* pMsg =  new CSegment;
		pMplProtocol->Serialize( *pMsg, CS_API_TYPE );

		CGKTaskApi api;
        if(api.CreateOnlyApi() >= 0)
            api.SendMsg(pMsg, SIM_CS_RCV_MSG);

//		if (m_pGKeeperApi)
//			m_pGKeeperApi->SendMsg(pMsg,SIM_CS_RCV_MSG);
//		else {
//			DBGPASSERT(pMplProtocol->getOpcode());
//			POBJDELETE(pMsg);
//		}
	}
	else {
		// POBJDELETE(pMsg); amir, checking valgrind error
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnReceiveMbxEndpointsIdle(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnReceiveMbxEndpointsIdle");
	COsQueue RcvMbx;
	RcvMbx.DeSerialize( *pMsg );
	m_pEndpointsApi->CreateOnlyApi(RcvMbx);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/*void CSimCSEndpointsModule::OnReceiveMbxH323(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnReceiveMbxH323");
	COsQueue RcvMbxH323;
	RcvMbxH323.DeSerialize( *pMsg );
	m_pH323Api->CreateOnlyApi(RcvMbxH323);
}*/

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/*void CSimCSEndpointsModule::OnReceiveMbxSIP(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnReceiveMbxSIP");
	COsQueue RcvMbxSIP;
	RcvMbxSIP.DeSerialize( *pMsg );
	m_pSIPApi->CreateOnlyApi(RcvMbxSIP);
}*/

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnReceiveMbxProxy(CSegment* pMsg)
{
	//PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnReceiveMbxProxy");
	//COsQueue RcvMbxProxy;
	//RcvMbxProxy.DeSerialize( *pMsg );
	//m_pProxyApi->CreateOnlyApi(RcvMbxProxy);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//void CSimCSEndpointsModule::OnReceiveMbxGK(CSegment* pMsg)
//{
//	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnReceiveMbxGK");
//	COsQueue RcvMbxGK;
//	RcvMbxGK.DeSerialize( *pMsg );
//	m_pGKeeperApi->CreateOnlyApi(RcvMbxGK);
//}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnReceiveCommandToCSAPI(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnReceiveCommandToCSAPI");

	// prepare & send CS_NEW indication
/*	DWORD	opcode;
	WORD	bufferLen;

	// fills command to send
	*pMsg >> opcode;		// opcode to CSAPI
	*pMsg >> bufferLen;		// size of indication data
	if (0 == bufferLen) {
		PTRACE(eLevelError,"CSimCSEndpointsModule::OnReceiveCommandToCSAPI - Illegal buffer length ");
		return;
	}
*/
	// gets indication struct
//	BYTE *bufferToSend = new BYTE[bufferLen];
//	pMsg->Get( bufferToSend, bufferLen );

	// fills the command to send to the CS-API
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
//	FillCsProtocol( pMplProtocol, opcode, bufferToSend, bufferLen);

	pMplProtocol->DeSerialize(*pMsg,CS_API_TYPE);

	SendToCsApi(*pMplProtocol);

	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnCsTerminalCommandReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnCsTerminalCommandReq - TERMINAL_COMMAND");

	char *data = pMplProtocol->getpData();
	data += sizeof(DWORD);

	CIstrStream istr(data);
	CTerminalCommand command;
	command.DeSerialize(istr);

	const string &terminalName = command.GetTerminalName();
	const string &commandName = command.GetCommandName();

	CProcessBase *process = CProcessBase::GetProcess();
	COstrStream answer;
	answer << "***** CS Simulation *****\n";
	PTRACECOMMAND(terminalName.c_str(), commandName.c_str(), answer.str().c_str());
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnCsGetKeepAliveReq( CMplMcmsProtocol* pMplProtocol )
{
//	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnCsGetKeepAliveReq");

/*
static int tempCounter=-1;
tempCounter++;
if (tempCounter>=2)
	tempCounter=0;
*/

   csKeepAliveSt keepAliveStructIND;
   for ( int i=0;i<NumOfComponents ;i++)
    {
         if(i<3)
            keepAliveStructIND.componentTbl[i].type=(compTypes)i;
         else
            keepAliveStructIND.componentTbl[i].type=(compTypes)0;

         keepAliveStructIND.componentTbl[i].bActive=0;
         keepAliveStructIND.componentTbl[i].id=0;
         keepAliveStructIND.componentTbl[i].reason=0;
         keepAliveStructIND.componentTbl[i].status=/*(compStatuses)tempCounter*/emCompOk;
    }

    WORD  bufferLen=sizeof(csKeepAliveSt);
	CMplMcmsProtocol* pMplProtocol1 = new CMplMcmsProtocol;
	CsFillCsProtocol( pMplProtocol1, CS_KEEP_ALIVE_IND, (BYTE*)(& keepAliveStructIND), bufferLen);

	SendToCsApi(*pMplProtocol1);

	POBJDELETE(pMplProtocol1);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnCsPingReq(CMplMcmsProtocol* pMplProtocol )
{
    CS_Ping_ind_S pingStruct;
    pingStruct.pingStatus = ePingStatus_ok;

    WORD  bufferLen=sizeof(pingStruct);
	CMplMcmsProtocol* pMplProtocol1 = new CMplMcmsProtocol;
	CsFillCsProtocol( pMplProtocol1, CS_PING_IND, (BYTE*)(& pingStruct), bufferLen);

	SendToCsApi(*pMplProtocol1);

	POBJDELETE(pMplProtocol1);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimCSEndpointsModule::OnCsProxyNotifyReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnCsProxyNotifyReq");

	CSegment* pMsg =  new CSegment;
	pMplProtocol->Serialize( *pMsg, CS_API_TYPE );

	const COsQueue* pProcessMngrMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessEndpointsSim,eManager);

	CTaskApi api;
	api.CreateOnlyApi(*pProcessMngrMbx);
	api.SendMsg(pMsg,SC_MSG_TO_MNGR);
	api.DestroyOnlyApi();
}
