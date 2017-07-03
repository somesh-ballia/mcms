//+========================================================================+
//                   GideonSimManager.cpp                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimManager.cpp                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

// GideonSimManager.cpp: implementation of the CGideonSimManager class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#include <memory>
#include <stdlib.h>
#include "Macros.h"
#include "Trace.h"
#include "LoggerDefines.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsNetQ931.h"

#include "Request.h"
#include "ListenSocketApi.h"
#include "SystemFunctions.h"
#include "OutsideEntities.h"
#include "DummyEntry.h"

#include "SimApi.h"
#include "GideonSim.h"
#include "GideonGuiApi.h"
#include "GideonSimConfig.h"
#include "SimGuiRxSocket.h"
#include "SimGuiTxSocket.h"
#include "SimPcmRxSocket.h"
#include "SimPcmTxSocket.h"
#include "EpGuiPcmRxSocket.h"
#include "EpGuiPcmTxSocket.h"
#include "CommGideonSimSet.h"

#include "GideonSimLoggerRxSocket.h"
#include "GideonSimLoggerTxSocket.h"
#include "GideonSimCardAckStatusList.h"

#include "GideonSimManager.h"

#include "CardsStructs.h"
#include "TraceStream.h"
#include "OpcodesMcmsAudioCntl.h"
#include "TerminalCommand.h"
#include "GideonSimLogicalParams.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "IpMfaOpcodes.h"
#include "IpCmReq.h"
#include "IpRtpReq.h"
#include "IpRtpInd.h"


#include "McmsProcesses.h"
#include "ManagerApi.h"

#include "MplMcmsProtocol.h"
#include "OpcodesMcmsPCM.h"
#include "OsFileIF.h"

extern void GideonSimMonitorEntryPoint(void* appParam);

static DWORD           switchVersionBurnRate = 50;
static DWORD           mediaVersionBurnRate = 20;
static DWORD           switchIPMCBurnRate = 50;
static DWORD           mediaIPMCBurnRate = 150;

extern DWORD GetBurnRate(eBurnTypes burnTypes);

DWORD mcuInternalOpcode = 0;
eLogicalResourceTypes mcuInternalResType = eLogical_res_none;
BYTE statusOrTimer = 0;

extern DWORD GetMcuInternalOpcode() {return mcuInternalOpcode;}
extern eLogicalResourceTypes GetMcuInternalResType() { return mcuInternalResType;}
extern BYTE GetStatusOrTimer() {return statusOrTimer;}


//added by huiyu
// tx mailbox of PCM request
COsQueue g_pcmTxMbx;

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CGideonSimManager)
	ONEVENT( OPEN_SOCKET_CONNECTION,  ANYCASE,  CGideonSimManager::OnListenSocketOpenConnection)
	ONEVENT( CLOSE_SOCKET_CONNECTION, ANYCASE,  CGideonSimManager::OnListenSocketCloseConnection)
	ONEVENT( XML_REQUEST,        IDLE,        CGideonSimManager::HandlePostRequest )
	ONEVENT( SIM_WAIT_TIMER,         IDLE,        CGideonSimManager::OnTimerWait)
	ONEVENT( SIM_PCM_TIMER,         IDLE,         CGideonSimManager::OnTimerPcmTransfer)	
		// GideonSim messages from EndpointsSim
	ONEVENT( SIM_API_ISDN_MSG,   ANYCASE,     CGideonSimManager::OnEpIsdnMsgAnycase)
	ONEVENT( SIM_API_AUDIO_MSG,  ANYCASE,     CGideonSimManager::OnEpAudioMsgAnycase)
	ONEVENT( SIM_API_MUX_MSG,  	 ANYCASE,     CGideonSimManager::OnEpMuxMsgAnycase)
	ONEVENT( SIM_API_MRM_MSG,  ANYCASE,     CGideonSimManager::OnEpScpMsgAnycase)
		// MM messages
	ONEVENT( MM_INDICATIONS_MSG, ANYCASE,  	  CGideonSimManager::OnMMIndicationsMsgAnycase)
#ifndef __DISABLE_ICE__
	ONEVENT( MM_GS_ICE_MSG, ANYCASE,  	  CGideonSimManager::OnMMIceMsgAnycase)
#endif	//__DISABLE_ICE__
		// GUI messages
	ONEVENT( SIM_GUI_SOCKET_RCV_MSG, ANYCASE,     CGideonSimManager::OnGuiMsgAnycase)
	ONEVENT( SIM_PCM_SOCKET_RCV_MSG, ANYCASE,     CGideonSimManager::OnPcmMsgAnycase)
	ONEVENT( EPGUI_PCM_SOCKET_RCV_MSG, ANYCASE,     CGideonSimManager::OnEpGuiPcmMsgAnycase)
		// Logger socket messages
	ONEVENT( SOCKET_CONNECTED,   ANYCASE,     CGideonSimManager::OnLoggerSocketConnectedAnycase)
	ONEVENT( SOCKET_FAILED,      ANYCASE,     CGideonSimManager::OnLoggerSocketFailedAnycase)
	ONEVENT( SOCKET_DROPPED,     ANYCASE,     CGideonSimManager::OnLoggerSocketDroppedAnycase)
	ONEVENT( SOCKET_RCV_MSG,     ANYCASE,     CGideonSimManager::OnLoggerSocketRcvMsgAnycase)
		// Forward message to logger socket
	ONEVENT( SIM_MESSAGE_FOR_LOGGER, ANYCASE,     CGideonSimManager::OnForwardLoggerMsgAnycase)
PEND_MESSAGE_MAP(CGideonSimManager,CManagerTask)


////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CGideonSimManager)
  //ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CGideonSimManager::HandleOperLogin)
  //ON_TRANS("TRANS_MCU",			"LOGOUT",			CDummyEntry, 						CGideonSimManager::HandleOperLogout)
	//ON_TRANS("TRANS_GIDEON_SIM",	"SOCKET_ACTION",	CCommSetDisconnectOrConnectSockets, CGideonSimManager::HandleSocketAction)
	ON_TRANS("TRANS_SIMULATION",	"SOCKET_DISCONNECT",CCommSetDisconnectOrConnectSockets, CGideonSimManager::HandleSocketDisconnectAction)
	ON_TRANS("TRANS_SIMULATION",	"SOCKET_CONNECT",	CCommSetDisconnectOrConnectSockets, CGideonSimManager::HandleSocketConnectAction)
	ON_TRANS("TRANS_SIMULATION",	"SET_UNIT_STATUS_FOR_KEEP_ALIVE",	CSetUnitStatus,     CGideonSimManager::HandleSetUnitForKeepConnectAction)
	ON_TRANS("TRANS_SIMULATION", 	"CARD_ACK_STATUS", 	CCommSetCardAckStatus, 				CGideonSimManager::HandleCardAckStatusAction )
	ON_TRANS("TRANS_SIMULATION", 	"ISDN_TIMERS", 		CCommSetIsdnTimers, 				CGideonSimManager::HandleIsdnTimersAction )
	ON_TRANS("TRANS_SIMULATION", 	"ISDN_ENABLE_DISABLE_PORTS", CCommSetEnableDisableRtmPorts, CGideonSimManager::HandleIsdnEnableDisablePortsAction )

	ON_TRANS("TRANS_SIMULATION", "INSERT_CARD_EVENT", CCommInsertCardEvent, CGideonSimManager::HandleInsertCardEventAction)
	ON_TRANS("TRANS_SIMULATION", "REMOVE_CARD_EVENT", CCommRemoveCardEvent, CGideonSimManager::HandleRemoveCardEventAction)
	
END_TRANSACTION_FACTORY




////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CGideonSimManager)
  ONCOMMAND("set_build_burn_rate",CGideonSimManager::SetVersionBurnRate,"set burn rate [board id] [burn type] [rate]")
  ONCOMMAND("handle_burning_actions",CGideonSimManager::HandleBurningActions,"handle burning actions [board id] [burn type] [action]")
  ONCOMMAND("set_mcu_internal",CGideonSimManager::HandleSetMcuInternal,"handle set mcu internal [mcuInternalCode]")
  ONCOMMAND("send_user_ldap_req", CGideonSimManager::HandleSendLdapUserReq, "handle send user ldap req [user name] [user password]")
  ONCOMMAND("sim_card_reset",CGideonSimManager::HandleSimCardReset,"simulate card reset [board id] [is_problematic_socket=true/false]")
END_TERMINAL_COMMANDS




////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void GideonSimManagerEntryPoint(void* appParam)
{
	CGideonSimManager * pGideonSimManager = new CGideonSimManager;
	pGideonSimManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CGideonSimManager::GetMonitorEntryPoint()
{
	return GideonSimMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CGideonSimManager::CGideonSimManager()
{
	m_pSimConfig       = NULL;
	m_ppSimCardApiArr  = NULL;
	m_wMaxCards        = 0;
	m_pListenSocketApi = NULL;
	m_pAckStatusList   = new CGideonSimCardAckStatusList();
	m_pLoggerSocketConnection = NULL;
	m_boadSearchDirection = 0;
	m_pcmData          = NULL;
	m_pListenSocketPcmApi = NULL;
	m_pListenSocketEpGuiPcmApi = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimManager::~CGideonSimManager()
{
	POBJDELETE(m_pListenSocketApi);
	POBJDELETE(m_pListenSocketPcmApi);
	POBJDELETE(m_pListenSocketEpGuiPcmApi);
	POBJDELETE(m_pSimConfig);
	POBJDELETE(m_pAckStatusList);

	if( m_ppSimCardApiArr ) {
		for( WORD i=0; i<m_wMaxCards; i++ )
			POBJDELETE(m_ppSimCardApiArr[i]);
		PDELETEA(m_ppSimCardApiArr);
	}
	POBJDELETE(m_pLoggerSocketConnection);
}

/////////////////////////////////////////////////////////////////////////////
void*  CGideonSimManager::GetMessageMap()
{
	return (void*)m_msgEntries;
}

extern DWORD GetBurnRate(eBurnTypes burnTypes)
{
	/*switch (burnTypes)
	{
		case eSwitchIpmcBurnRate:
		{
			return switchIPMCBurnRate;
			break;
		}
		case eSwitchVersionBurnRate:
		{
			return switchVersionBurnRate;
			break;
		}
		case eMediaCardIpmcBurnRate:
		{
			return mediaIPMCBurnRate;
			break;
		}
		case eMediaCardVersionBurnRate:
		{
			return mediaVersionBurnRate;
			break;
		}
	}*/

	return STATUS_FAIL;


}



///////////////////////////////////////////////////////
STATUS CGideonSimManager::HandleBurningActions(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CGideonSimManager::HandleBurningActions ");

	DWORD numOfParams = command.GetNumOfParams();

	if(3 != numOfParams)
	{
		answer << "error: missing arguments ";
		answer << "usage: handle burning actions [board id] [burn type] [action]";
		return STATUS_FAIL;
	}

	const string &sBoardId = command.GetToken(eCmdParam1);
	const string &burnType = command.GetToken(eCmdParam2);
	const string &action   = command.GetToken(eCmdParam3);

	DWORD boardId = atoi(sBoardId.c_str());

	if( boardId < m_wMaxCards &&  m_ppSimCardApiArr[boardId] != NULL )
	{
		CCardCfg* pCurrCardCfg = m_pSimConfig->GetCardCfg(boardId);
		if( !pCurrCardCfg )
		{
			answer << "usage: no card data for this board id";
			return STATUS_FAIL;
		}

		eBurnActionTypes burnActionType;
		if (action == "stop")
			burnActionType = eStopBurn;
		else if (action == "start")
			burnActionType = eStartBurn;
		else if (action == "pause")
			burnActionType = ePauseBurn;
		else if (action == "resume")
			burnActionType = eResumeBurn;
		else
		{
			answer << "usage: invalid burn action. please use stop, start, pause";
			return STATUS_FAIL;
		}

		eBurnTypes burnTypes;
		if (burnType == "version")
			burnTypes = eVersionBurnType;
		else if (burnType == "ipmc")
			burnTypes = eIpmcBurnType;
		else
		{
			answer << "usage: invalid burn type. please use version or ipmc";
			return STATUS_FAIL;
		}
		if(pCurrCardCfg->GetCardType() == eCardBarak || pCurrCardCfg->GetCardType() == eCardSwitch)
		{
			m_ppSimCardApiArr[boardId]->SendBurnAction(burnActionType,burnTypes);
		}
	}
	else
	{
		answer << "usage: invalid board id";
		return STATUS_FAIL;
	}


	return STATUS_OK;

}
///////////////////////////////////////////////////////
STATUS CGideonSimManager::HandleSetMcuInternal(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CGideonSimManager::HandleSetMcuInternal ");

	DWORD numOfParams = command.GetNumOfParams();

	if(1 != numOfParams)
	{
		answer << "error: missing arguments ";
		answer << "usage: handle set mcu internal [mcuInternalCode]";
		return STATUS_FAIL;
	}

		const string &sMcuInternalNumber = command.GetToken(eCmdParam1);

		if (sMcuInternalNumber.length() != 5)
		{
			answer << "error: mcuInternalCode must include exactly 5 digits";
			return STATUS_FAIL;
		}
		char mipHardWareConn = sMcuInternalNumber[0];
		char mipMedia = sMcuInternalNumber[1];
		char mipDirection = sMcuInternalNumber[2];
		char mipTimerStatus = sMcuInternalNumber[3];
		char mipAction = sMcuInternalNumber[4];


		switch(mipHardWareConn)
		{
		case('1'):
		{
			mcuInternalResType = eLogical_rtp;
			switch(mipAction)
			{
			case('2'):
			case('4'):
			{
				mcuInternalOpcode = CONFPARTY_CM_OPEN_UDP_PORT_REQ;
				answer << "Opcode: CONFPARTY_CM_OPEN_UDP_PORT_REQ ";
				break;
			}
			case('3'):
			case('5'):
			{
				mcuInternalOpcode = CONFPARTY_CM_CLOSE_UDP_PORT_REQ;
				answer << "Opcode: CONFPARTY_CM_CLOSE_UDP_PORT_REQ ";
				break;
			}
			}
			break;
		}
		case('2'):
		{
			mcuInternalResType = eLogical_rtp;
			switch(mipAction)
			{
				case('2'):
				case('4'):
				{
					mcuInternalOpcode = H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ;
					answer << "Opcode: H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ ";
					break;
				}
			}
			break;
		}
		case('3'):
		{
			switch(mipAction)
			{
				case('1'):
				{
					mcuInternalOpcode = TB_MSG_CONNECT_REQ;
					answer << "Opcode: TB_MSG_CONNECT_REQ ";
					break;
				}
				case('2'):
				{
					mcuInternalOpcode = TB_MSG_OPEN_PORT_REQ;
					answer << "Opcode: TB_MSG_OPEN_PORT_REQ ";
					break;
				}
				case('3'):
				{
					mcuInternalOpcode = TB_MSG_DISCONNECT_REQ;
					answer << "Opcode: TB_MSG_DISCONNECT_REQ ";
					break;
				}
				case('5'):
				{
					mcuInternalOpcode = TB_MSG_CLOSE_PORT_REQ;
					answer << "Opcode: TB_MSG_CLOSE_PORT_REQ ";
					break;
				}
			}
			if (mipMedia == '1')
			{
				if (mipDirection == '1')
				{
					mcuInternalResType = eLogical_audio_decoder;
				}
				else
				{
					mcuInternalResType = eLogical_audio_encoder;
				}
			}
			else if (mipMedia == '2')
			{
				if (mipDirection == '1')
				{
					mcuInternalResType = eLogical_video_decoder;
				}
				else
				{
					mcuInternalResType = eLogical_video_encoder;
				}
			}
			break;
		}
	}
		if (mipTimerStatus == '1')
		{
			statusOrTimer = 1; //timer
			answer << " timer - don't reply to the opcode";
		}
		else
		{
			statusOrTimer = 2; //status
			answer << " status - reply with status_fail to the opcode";
		}

		return STATUS_OK;

}

///////////////////////////////////////////////////////

STATUS CGideonSimManager::HandleSendLdapUserReq(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CGideonSimManager::HandleSendLdapUserReq ");

	DWORD numOfParams = command.GetNumOfParams();

	if(2 != numOfParams)
	{
		answer << "error: missing arguments";
		answer << "usage: send user ldap req [user name] [user password]";
		return STATUS_FAIL;
	}
	CSegment*  pSeg = new CSegment();

	const string &userName 		= command.GetToken(eCmdParam1);
	const string &userPassword = command.GetToken(eCmdParam2);

	*pSeg << userName
		<< userPassword;

	DWORD switchBoardId = 0;

	if (m_ppSimCardApiArr[switchBoardId] != NULL )
	{
		m_ppSimCardApiArr[switchBoardId]->SendMsg(pSeg,  SIM_USER_LDAP_LOGIN );
        answer << "Sent message user LDAP request for user " << userName << " (assumption switchBoardId is 0)";
	}
	else {
		PASSERT(switchBoardId+1000);
	}
	return STATUS_OK;
}

///////////////////////////////////////////////////////
STATUS CGideonSimManager::SetVersionBurnRate(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CGideonSimManager::SetVersionBurnRate ");

	DWORD numOfParams = command.GetNumOfParams();

	if(3 != numOfParams)
	{
		answer << "error: missing arguments";
		answer << "usage: set burn rate [board id] [burn type] [rate]";
		return STATUS_FAIL;
	}

	const string &sBoardId = command.GetToken(eCmdParam1);
	const string &burnType = command.GetToken(eCmdParam2);
	const string &sRate	   = command.GetToken(eCmdParam3);

	DWORD boardId = atoi(sBoardId.c_str());
	DWORD rate    = atoi(sRate.c_str());

	if (rate >= 10000)
	{
		answer << "error: rate range is invalid";
		answer << "usage: rate range must be between 0 and 10000";
		return STATUS_FAIL;
	}

	if( boardId < m_wMaxCards &&  m_ppSimCardApiArr[boardId] != NULL )
	{
		CCardCfg* pCurrCardCfg = m_pSimConfig->GetCardCfg(boardId);
		if( !pCurrCardCfg )
		{
			answer << "usage: no card data for this board id";
			return STATUS_FAIL;
		}
		eBurnTypes burnTypes;
		if (burnType == "version")
			burnTypes = eVersionBurnType;
		else if (burnType == "ipmc")
			burnTypes = eIpmcBurnType;
		else
		{
			answer << "usage: invalid burn type. please use version or ipmc";
			return STATUS_FAIL;
		}

		if(pCurrCardCfg->GetCardType() == eCardBarak || pCurrCardCfg->GetCardType() == eCardSwitch)
		{
			//if ( static_cast <int>( sizeof(m_ppSimCardApiArr) / sizeof(m_ppSimCardApiArr[0]) ) >= static_cast <int>(boardId.))
			m_ppSimCardApiArr[boardId]->UpdateBurnRate(rate,burnTypes);
		}
	}
	else
	{
		answer << "usage: invalid board id";
		return STATUS_FAIL;
	}

	/*
	if (cardType == "switch" && burnType == "version")
		switchVersionBurnRate = atoi(rate.c_str()) ;
	else if(cardType == "switch" && burnType == "ipmc")
		switchIPMCBurnRate = atoi(rate.c_str()) ;
	else if(cardType == "media" && burnType == "version")
		mediaVersionBurnRate = atoi(rate.c_str()) ;
	else if(cardType == "media" && burnType == "ipmc")
		mediaIPMCBurnRate = atoi(rate.c_str()) ;
	else
		answer << "error: one of params is not valid";*/

	return STATUS_OK;

}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CGideonSimManager::HandleSimCardReset(CTerminalCommand & command, std::ostream& answer)
{
	TRACEINTO << ".";

	DWORD numOfParams = command.GetNumOfParams();

	if (2 > numOfParams)
	{
		answer << "error: missing arguments, check help";
		return STATUS_FAIL;
	}
	CSegment*  pSeg = new CSegment();

	const string& sBoardId = command.GetToken(eCmdParam1);
	const string& sIsProblematicSocket = command.GetToken(eCmdParam2);

	WORD boardId = atoi(sBoardId.c_str());
	bool isProblematic = (sIsProblematicSocket == "true" || sIsProblematicSocket == "yes") ? true : false;

	//DWORD switchBoardId = 0;
	*pSeg << (WORD)isProblematic;

	if (boardId < m_wMaxCards && m_ppSimCardApiArr[boardId] != NULL )
	{
		m_ppSimCardApiArr[boardId]->SendMsg(pSeg, SIM_RESET_CARD );
        answer << "start reset card " << boardId << " with" << (isProblematic == false ? "out " : " ") << "problems.";
	}
	else
	{
		PASSERT(boardId+1000);
		answer << "error: board " << boardId << " not found!";
	}
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::SelfKill()
{
	if( TRUE == CPObject::IsValidPObjectPtr(m_pLoggerSocketConnection) )
	{
		//m_pLoggerSocketConnection->Disconnect();
		m_pLoggerSocketConnection->Destroy();
	}
	//SystemSleep(100);

	if ( TRUE == CPObject::IsValidPObjectPtr(m_pListenSocketApi) ) {
		m_pListenSocketApi->SyncDestroy();
		POBJDELETE(m_pListenSocketApi);
		//SystemSleep(5);
	}

	if( m_ppSimCardApiArr ) {
		for( WORD i=0; i<m_wMaxCards; i++ ) {
			if( CPObject::IsValidPObjectPtr(m_ppSimCardApiArr[i]) ) {
				m_ppSimCardApiArr[i]->SyncDestroy();
				POBJDELETE(m_ppSimCardApiArr[i]);
				//SystemSleep(5);
			}
		}
	}
	PDELETEA(m_ppSimCardApiArr);


//	DestroyTimer();
	DeleteAllTimers();
	CManagerTask::SelfKill();
}

//////////////////////////////////////////////////////////////////////
void CGideonSimManager::ManagerPostInitActionsPoint()
{
	// this function is called just before WaitForEvent

	// init timers
//	InitTimer(GetRcvMbx());

	// init application with XML configuration file
	m_pSimConfig = new CGideonSimSystemCfg;

	// Create logger socket
	char   szIpAddress[IP_ADDRESS_STR_LEN];
	strncpy(szIpAddress,::GetGideonSystemCfg()->GetMplApiIpAddress(),sizeof(szIpAddress) - 1);
	szIpAddress[sizeof(szIpAddress) - 1] = '\0';

	POBJDELETE(m_pLoggerSocketConnection);
	m_pLoggerSocketConnection = new CClientSocket(this,GideonSimLoggerRxEntryPoint,GideonSimLoggerTxEntryPoint);
	m_pLoggerSocketConnection->Init(szIpAddress,LOGGER_LISTEN_SOCKET_PORT);
	m_pLoggerSocketConnection->Connect();

	// create cards by configuration
	CCardCfg* pCurrCardCfg = NULL;

	m_wMaxCards  = m_pSimConfig->GetMaxCardSlots();

	m_ppSimCardApiArr = new CSimCardApi* [m_wMaxCards];
	for( WORD i=0; i<m_wMaxCards; i++ ) {
		m_ppSimCardApiArr[i] = NULL;

		pCurrCardCfg = m_pSimConfig->GetCardCfg(i);
		if( CPObject::IsValidPObjectPtr(pCurrCardCfg) ) {
			// create card task
			m_ppSimCardApiArr[i] = new CSimCardApi;
			m_ppSimCardApiArr[i]->Create(*m_pRcvMbx,pCurrCardCfg);
		}
	}

	// Create Listen Socket task for GUI negotiation
	m_pListenSocketApi = new CListenSocketApi(SimGuiRxEntryPoint,SimGuiTxEntryPoint,
											::GetGideonSystemCfg()->GetGuiPortNumber());
	m_pListenSocketApi->Create(*m_pRcvMbx);

	// Create Listen Socket task for PCM negotiation
	m_pListenSocketPcmApi = new CListenSocketApi(SimPcmRxEntryPoint,SimPcmTxEntryPoint,
											::GetGideonSystemCfg()->GetPcmPortNumber());
	m_pListenSocketPcmApi->Create(*m_pRcvMbx);


	// Create Listen Socket task for GUI showing PCM negotiation
	m_pListenSocketEpGuiPcmApi = new CListenSocketApi(EpGuiPcmRxEntryPoint,EpGuiPcmTxEntryPoint,
											::GetGideonSystemCfg()->GetPcmFramePortNumber());
	m_pListenSocketEpGuiPcmApi->Create(*m_pRcvMbx);
	
	StartTimer(SIM_WAIT_TIMER,1*SECOND);

	if (IsFileExists("Bin/pcm"))
	    StartTimer(SIM_PCM_TIMER,1*SECOND);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnTimerWait(CSegment* pParam)
{
	for( WORD i=0; i<m_wMaxCards; i++ ) {
		if( CPObject::IsValidPObjectPtr(m_ppSimCardApiArr[i]) )
			m_ppSimCardApiArr[i]->Connect();
	}
}

#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include "ZipCompressor.h"
#include <stdio.h>
/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnTimerPcmTransfer(CSegment* pParam)
{
	WORD PCM_FRAME_WIDTH  = 1024;
	WORD PCM_FRAME_HEIGHT = 576;
	WORD OFFSET           = 64;

	if(m_pcmData == NULL)
	{
		int displayId = 0;
	
		int shmId;
		key_t key;
		char szName[64];
		std::string fname = MCU_TMP_DIR+"/RMX1000_TVUI_MEM_";
		snprintf(szName, sizeof(szName), "%s%02d", fname.c_str(), displayId);
		key = ftok(szName, 'v');
		
		if(key == -1)
			fprintf(stderr, "RMXScreen mem ftok error %x\n", errno);
		
		shmId = shmget(key, PCM_FRAME_WIDTH * PCM_FRAME_HEIGHT * 4 + OFFSET, 0666);
		
		if(shmId < 0)
		{
            char szTmp[100];
            snprintf(szTmp, sizeof(szTmp), "shmget error code= %d reason=%s\n",errno,strerror(errno));
			static int ii = 0;
			if(ii>10)
			{
				fprintf(stderr,"%s",szTmp);
				ii = 0;
			}
			ii++;
            m_pcmData = NULL;
		}
		else
		{
			m_pcmData = (unsigned char *)shmat(shmId,NULL,0);
			if(m_pcmData == NULL)
			{
				fprintf(stderr, "RMXScreen data == NULL\n");
			}
			else
			{
				fprintf(stderr,"CGideonSimManager::OnTimerPcmTransfe(),get share memroy successully.\n");
			}
		}
	}
	else
	{
		static int nCounter = 0;
		int nTemp;
		memcpy(&nTemp,m_pcmData,4);
		if(nCounter >= nTemp)
		{
			StartTimer(SIM_PCM_TIMER,3*SECOND);
			return;
		}
		nCounter = nTemp;
		
		BYTE	*m_pFileBuffer;			// will hold data before writing to HD.
		ULONG   m_lFileBufferSize;     	// size of buffer in bytes
		m_lFileBufferSize = 80000;
	    m_pFileBuffer = new BYTE[m_lFileBufferSize];
		
		CZipCompressor *m_pZipCompressor 	= new CZipCompressor; AUTO_DELETE(m_pZipCompressor);
		int lErr;
		// Init Compressor
		/* The following is the init procedure for zlib compression*/
		lErr = m_pZipCompressor->Init(m_pFileBuffer, m_lFileBufferSize);
		if (lErr!=Z_OK)
		{
			//FATAL_ERROR_VOID("CGideonSimManager::OnTimerPcmTransfer : Cannot init zip deflate",lErr);
			fprintf(stderr,"CGideonSimManager::OnTimerPcmTransfer : Cannot init zip deflate\n");			
		}
		
		m_pZipCompressor->SetNextIn((BYTE*)(m_pcmData + OFFSET), PCM_FRAME_WIDTH * PCM_FRAME_HEIGHT * 4);
		
		lErr = m_pZipCompressor->Deflate(Z_NO_FLUSH);	
		if (lErr != Z_OK)
		{
			//FATAL_ERROR_BOOL("CGideonSimManager::OnTimerPcmTransfer : deflate failed",lErr);
			fprintf(stderr,"CGideonSimManager::OnTimerPcmTransfer : deflate failed\n");			
		}

		m_pZipCompressor->SetNextIn(NULL, 0);

		// Compressor
		lErr = m_pZipCompressor->Deflate(Z_FINISH);
		if (lErr != Z_STREAM_END)
		{
			//FATAL_ERROR_BOOL("CGideonSimManager::OnTimerPcmTransfer : compressed data buffer not flushed at limit",lErr);
			fprintf(stderr,"CGideonSimManager::OnTimerPcmTransfer : compressed data buffer not flushed at limit\n");			
		}

		if (m_pZipCompressor->GetTotalOut() > m_lFileBufferSize)
		{
			//FATAL_ERROR_BOOL("CGideonSimManager::OnTimerPcmTransfer : File limit crossed. MAX_INTERNAL_COMPRESSION_BUFFER is too small.",0);
			fprintf(stderr,"CGideonSimManager::OnTimerPcmTransfer : File limit crossed. MAX_INTERNAL_COMPRESSION_BUFFER is too small.\n");			
		}
		
		CSegment*  pMsg = new CSegment;
		pMsg->Put((BYTE*)m_pFileBuffer,m_pZipCompressor->GetTotalOut());
		
		CTaskApi api;
		api.CreateOnlyApi(m_EpGuiPcmTxMbx);
		api.SendMsg(pMsg,SOCKET_WRITE);
		api.DestroyOnlyApi();
	}

    StartTimer(SIM_PCM_TIMER,3*SECOND);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnListenSocketOpenConnection(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CGideonSimManager::OnListenSocketOpenConnection - GUI connection opened.");
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnListenSocketCloseConnection(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CGideonSimManager::OnListenSocketCloseConnection - GUI connection closed.");
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnEpIsdnMsgAnycase(CSegment* pParam)
{
	WORD  boardId = 0xFFFF;//, unitId;
	DWORD opcode = 0;

	CSegment*  pCopySeg = new CSegment(*pParam);

	*pParam >> opcode
			>> boardId;

	if ( boardId > 10)
	{
		TRACEINTO << "CGideonSimManager::OnEpIsdnMsgAnycase boardId" << boardId << " opcode=" << opcode;
	}

	if( boardId < m_wMaxCards &&  m_ppSimCardApiArr[boardId] != NULL )
	{
		m_ppSimCardApiArr[boardId]->SendMsg(pCopySeg, SIM_API_ISDN_MSG);
	}
	else if( NET_SETUP_IND == opcode  &&  0xFFFF == boardId )
	{
		// find last media card (MFA/Barak) with RTM sub-board
		CCardCfg* pCurrCardCfg = NULL;

		if (0 == m_boadSearchDirection)
		{
			for( int i=m_wMaxCards; i>0; i-- )
			{
				if( CPObject::IsValidPObjectPtr(m_ppSimCardApiArr[i-1]) == TRUE )
				{
					pCurrCardCfg = m_pSimConfig->GetCardCfg(i-1);
					if( CPObject::IsValidPObjectPtr(pCurrCardCfg) == TRUE )
					{
						if((pCurrCardCfg->IsMediaCard()) &&
							( TRUE == (dynamic_cast<CBaseMediaCardCfg*>(pCurrCardCfg))->GetRtmAttached() ))
						{
							m_ppSimCardApiArr[i-1]->SendMsg(pCopySeg,SIM_API_ISDN_MSG);
							m_boadSearchDirection = 1;
							return;
						}
					}
				}
			}
		}
		else
		{
			for( int i=1; i<=m_wMaxCards; i++ )
			{
				if( CPObject::IsValidPObjectPtr(m_ppSimCardApiArr[i-1]) == TRUE )
				{
					pCurrCardCfg = m_pSimConfig->GetCardCfg(i-1);
					if( CPObject::IsValidPObjectPtr(pCurrCardCfg) == TRUE )
					{
						if((pCurrCardCfg->IsMediaCard()) &&
							( TRUE == (dynamic_cast<CBaseMediaCardCfg*>(pCurrCardCfg))->GetRtmAttached() ))
						{
							m_ppSimCardApiArr[i-1]->SendMsg(pCopySeg,SIM_API_ISDN_MSG);
							m_boadSearchDirection = 0;
							return;
						}
					}
			
				}
			}
		}

		// no RTM in configuration
		PASSERT(m_wMaxCards+1000);
	}
	else
	{
		TRACEINTO << "CGideonSimManager::OnEpIsdnMsgAnycase boardId=" << boardId << " opcode=" << opcode;

		// opcode different from NET_SETUP_IND arrived to unknown board
		PASSERT(boardId+1000);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnEpAudioMsgAnycase(CSegment* pParam)
{
	WORD  boardId = 0xFFFF;//, unitId;
	DWORD opcode = 0;

	CSegment*  pCopySeg = new CSegment(*pParam);

	*pParam >> opcode
			>> boardId;
			
	if( boardId < m_wMaxCards &&  m_ppSimCardApiArr[boardId] != NULL )
	{
		m_ppSimCardApiArr[boardId]->SendMsg(pCopySeg,SIM_API_AUDIO_MSG);
	}
	else
		PASSERT(boardId+1000);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnEpMuxMsgAnycase(CSegment* pParam)
{
	WORD  boardId = 0xFFFF;//, unitId;
	DWORD opcode = 0;

	CSegment*  pCopySeg = new CSegment(*pParam);

	*pParam >> opcode
			>> boardId;

	if( boardId < m_wMaxCards &&  m_ppSimCardApiArr[boardId] != NULL )
		m_ppSimCardApiArr[boardId]->SendMsg(pCopySeg,SIM_API_MUX_MSG);
	else
		PASSERT(boardId+1000);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnEpScpMsgAnycase(CSegment* pParam)
{

	WORD  boardId = 0xFFFF;
	WORD  subBoardId = 0xFFFF;
	WORD  unitId = 0xFFFF;
	WORD  portId = 0xFFFF;
	DWORD opcode = 0xFFFF;


	DWORD confId = 0;
	DWORD partyId = 0;
	DWORD connId = 0;
	DWORD channelId = 0;
	WORD  numOfStreams = 0;


	CSegment*  pCopySeg = new CSegment(*pParam);

	*pParam >> opcode
			>> boardId
	        >> subBoardId
	        >> unitId
	        >> portId
	        >> confId
	        >> partyId
	        >> connId
	        >> channelId
	        >> numOfStreams;

	TRACEINTO << "CGideonSimManager::OnEpScpMsgAnycase boardId=" << boardId
			<< " opcode=" << opcode
			<<  " connId=" << connId
			<<  " numOfStreams=" << numOfStreams ;

	if( boardId < m_wMaxCards &&  m_ppSimCardApiArr[boardId] != NULL )
	{
		m_ppSimCardApiArr[boardId]->SendMsg(pCopySeg,SIM_API_MRM_MSG);
	}
	else
		PASSERT(boardId+1000);

}


////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnMMIndicationsMsgAnycase(CSegment* pParam)
{
	DWORD opcode = 0;
	DWORD boardId = (DWORD)-1;

	CSegment*  pCopySeg = new CSegment(*pParam);

	*pParam >> opcode
			>> boardId;

	if( boardId < m_wMaxCards &&  m_ppSimCardApiArr[boardId] != NULL )
		m_ppSimCardApiArr[boardId]->SendMsg(pCopySeg,MM_INDICATIONS_MSG);
	else//commented by Victor: memory leak here!!! Who release pCopySeg?
		PASSERT(boardId+1000);
}

#ifndef __DISABLE_ICE__
void CGideonSimManager::OnMMIceMsgAnycase(CSegment* pParam)
{
	DWORD boardId = (DWORD)-1;

	std::auto_ptr<CSegment>  pCopySeg(new CSegment(*pParam));
	std::auto_ptr<CMplMcmsProtocol> pMplProtocol(new CMplMcmsProtocol);
	
	pMplProtocol->DeSerialize(*pParam);

	if ( pMplProtocol->getPhysicalInfoHeaderCounter() != 1 ) {
		PASSERT(100+pMplProtocol->getPhysicalInfoHeaderCounter());
		return;
	}
	
	boardId=pMplProtocol->getPhysicalInfoHeaderBoard_id();

	if( boardId < m_wMaxCards &&  m_ppSimCardApiArr[boardId] != NULL )
		m_ppSimCardApiArr[boardId]->SendMsg(pCopySeg.release(),MM_GS_ICE_MSG);
	else
		PASSERT(boardId+1000);
}
#endif	//__DISABLE_ICE__

// Forward message to logger socket
/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnForwardLoggerMsgAnycase(CSegment* pParam)
{
	if( TRUE != CPObject::IsValidPObjectPtr(m_pLoggerSocketConnection) )
		return;

	CSegment* pMsg = new CSegment; // pMsg will delete inside

	// put parameters to message
	*pMsg  << *pParam;

	m_pLoggerSocketConnection->Send(pMsg);

	POBJDELETE(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnLoggerSocketConnectedAnycase(CSegment* pParam)
{
	//PTRACE(eLevelInfoNormal,"CGideonSimManager::OnLoggerSocketConnectedAnycase - LOGGER SOCKET connected, VASILYLOG");
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnLoggerSocketFailedAnycase(CSegment* pParam)
{
	//PTRACE(eLevelInfoNormal,"CGideonSimManager::OnLoggerSocketFailedAnycase - LOGGER SOCKET failed, VASILYLOG");
	// endless loop
	if( TRUE == CPObject::IsValidPObjectPtr(m_pLoggerSocketConnection) )
		m_pLoggerSocketConnection->Connect();
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnLoggerSocketDroppedAnycase(CSegment* pParam)
{
	//PTRACE(eLevelInfoNormal,"CGideonSimManager::OnLoggerSocketDroppedAnycase - LOGGER SOCKET dropped, VASILYLOG");
	// reestablish connection
	if( TRUE == CPObject::IsValidPObjectPtr(m_pLoggerSocketConnection) ) {
		m_pLoggerSocketConnection->Disconnect();
		m_pLoggerSocketConnection->Connect();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnLoggerSocketRcvMsgAnycase(CSegment* pParam)
{
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnGuiMsgAnycase(CSegment* pParam)
{

	// get Mbx of response socket
	COsQueue  rTxSocketMbx;
	rTxSocketMbx.DeSerialize(*pParam);

	// get parameters
	DWORD  reqType = 0;
	*pParam >> reqType;

	if( reqType == SIM_GUI_MSG_TYPE_GET ) {
		ProcessGetRequest(rTxSocketMbx,pParam);
	} else if( reqType == SIM_GUI_MSG_TYPE_SET ) {
		ProcessSetRequest(rTxSocketMbx,pParam);
	} else
		PASSERT(1000+reqType);

//	// create response message
//	CSegment* pMsgSeg = new CSegment;
//
//	// send response
//	CTaskApi api;
//	api.CreateOnlyApi(*pTxSocketQueue);
//	STATUS a = api.SendMsg(pMsgSeg,SOCKET_WRITE);
//	api.DestroyOnlyApi();
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnPcmMsgAnycase(CSegment* pParam)
{
	//only test added by huiyu
    // PCM command
    // get Mbx of response socket
	g_pcmTxMbx.DeSerialize(*pParam);

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pParam);

	DWORD  opcode = pMplProtocol->getOpcode();	
	switch( opcode ) {
		case PCM_COMMAND:
		{
			char* pData = pMplProtocol->GetData();			
			if (strstr(pData,"COMMAND")!=NULL)
			{
				PTRACE2(eLevelError,"CGideonSimManager::OnPcmMsgAnycase()\n", pData);	
				int barakBoardId = 0;
				CCardCfg* pCardCfg = NULL;
				eCardTypes cardType;
				for (int i=0; i<m_wMaxCards; i++)
				{
					pCardCfg = m_pSimConfig->GetCardCfg(i);
					
					if(pCardCfg)
					{
						cardType = pCardCfg->GetCardType();
						if (cardType == eCardBarak)
						{
							barakBoardId = i;
							break;
						}
					}
				}
				
				CSegment* pParamSeg = new CSegment;
				pMplProtocol->Serialize(*pParamSeg);
				m_ppSimCardApiArr[barakBoardId]->SendMsg(pParamSeg, (DWORD)PCM_MSG_TO_MPLAPI);				
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::OnEpGuiPcmMsgAnycase(CSegment* pParam)
{
	fprintf(stderr,"CGideonSimManager::OnEpGuiPcmMsgAnycase(),receive message from EpGui\n");
	// get Mbx of response socket
	m_EpGuiPcmTxMbx.DeSerialize(*pParam);
	
	//COsQueue  rTxSocketMbx;
	//rTxSocketMbx.DeSerialize(*pParam);
/*
	// get parameters
	DWORD  reqType = 0;
	*pParam >> reqType;

	if( reqType == SIM_GUI_MSG_TYPE_GET ) {
		ProcessGetRequest(rTxSocketMbx,pParam);
	} else if( reqType == SIM_GUI_MSG_TYPE_SET ) {
		ProcessSetRequest(rTxSocketMbx,pParam);
	} else
		PASSERT(1000+reqType);
*/		
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::ProcessGetRequest(const COsQueue& rTxSocketMbx,CSegment* pParam)
{
	DWORD  action = 0;

	*pParam >> action;

	switch( action ) {
		case GET_CONFIG: {
			// prepare response
			CSegment* pMsgSeg = new CSegment;
			m_pSimConfig->Serialize(*pMsgSeg);
			// send response
			CTaskApi api;
			api.CreateOnlyApi(rTxSocketMbx);
			STATUS a = api.SendMsg(pMsgSeg,SOCKET_WRITE);
			api.DestroyOnlyApi();
			break;
					 }
		default: {
			PASSERT(1000+action);
			break;
				 }
	}
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimManager::ProcessSetRequest(const COsQueue& rTxSocketMbx,CSegment* pParam)
{
	DWORD  action = 0;

	*pParam >> action;

	switch( action ) {
		case SET_CONFIG: {
			CGideonSimSystemCfg* pConfig = new CGideonSimSystemCfg;
			pConfig->DeSerialize(*pParam);
			if( !m_pSimConfig->IsEqual(*pConfig) ) {
				*m_pSimConfig = *pConfig;
				m_pSimConfig->WriteXmlFile();
			}
			POBJDELETE(pConfig);
			break;
						 }
		default: {
			PASSERT(1000+action);
			break;
				 }
	}
}


/////////////////////////////////////////////////////////////////////////////
//  Connect / Disconnect Socket from Card
STATUS CGideonSimManager::HandleSocketDisconnectAction(CRequest *pRequest)
{
    CCommSetDisconnectOrConnectSockets *pPartySet = (CCommSetDisconnectOrConnectSockets*)pRequest->GetRequestObject();
    PTRACE(eLevelInfoNormal,"Shlomit++++++++++++++++++++++++++++++++++++++++++++++++++++.");
	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_SOCKETS_ACTION
			<< pPartySet->GetSlots();

	int boardId=pPartySet->GetSlots();
	//for (boardId = 0; boardId < m_wMaxCards; boardId++)
	//{
		//if (pPartySet->IsSlotInList( boardId ))
		//{
			if( m_ppSimCardApiArr[boardId] != NULL )
			{
				CSegment*  pMsgToSend = new CSegment(*pMsg);
				m_ppSimCardApiArr[boardId]->SendMsg( pMsgToSend, (DWORD)PAUSE_SOCKET/*BATCH_GIDEON_SIM_COMMAND*/ );
			}
		//}
	//}

	POBJDELETE( pMsg );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
//  Connect / Disconnect Socket from Card
/////////////////////////////////////////////////////////////////////////////

//  Connect / Disconnect Socket from Card
STATUS CGideonSimManager::HandleSocketConnectAction(CRequest *pRequest)
{
	CCommSetDisconnectOrConnectSockets *pPartySet = (CCommSetDisconnectOrConnectSockets*)pRequest->GetRequestObject();
PTRACE(eLevelInfoNormal,"Shlomit++++++++++++++++++++++++++++++++++++++++++++++++++++.");
	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_SOCKETS_ACTION
			<< pPartySet->GetSlots();

	// sends to all requested slots (cards)
	int boardId=pPartySet->GetSlots();
	//for (boardId = 0; boardId < m_wMaxCards; boardId++)
	//{
		//if (pPartySet->IsSlotInList( boardId ))
		//{
			if( m_ppSimCardApiArr[boardId] != NULL )
			{
				CSegment*  pMsgToSend = new CSegment(*pMsg);
				m_ppSimCardApiArr[boardId]->SendMsg( pMsgToSend, (DWORD)RESUME_SOCKET );
			}
		//}
	//}

	POBJDELETE( pMsg );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CGideonSimManager::HandleSetUnitForKeepConnectAction(CRequest *pRequest)
{

	CSetUnitStatus *pUnitSet = (CSetUnitStatus*)pRequest->GetRequestObject();
    PTRACE(eLevelInfoNormal,"Shlomit++++++++++++HandleSetUnitForKeepConnectAction++++++++++++++++++++++++++++++++++++++++.");
	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)SET_UNIT_STATUS_FOR_KEEP_ALIVE_NEW//BATCH_SOCKETS_ACTION
			<< pUnitSet->GetSlots()
			<< pUnitSet->GetUnit()
			<< pUnitSet->GetUnitStatus();

	// sends to all requested slots (cards)
	int boardId=pUnitSet->GetSlots();
	//boardId=1;
	if( m_ppSimCardApiArr[boardId] != NULL )
	{
		CSegment*  pMsgToSend = new CSegment(*pMsg);
		m_ppSimCardApiArr[boardId]->SendMsg( pMsgToSend, (DWORD)SET_UNIT_STATUS_FOR_KEEP_ALIVE_NEW/*RESUME_SOCKET*/ );
		//DispatchEvent(SET_UNIT_STATUS_FOR_KEEP_ALIVE_NEW,pMsg);
	}

	POBJDELETE( pMsg );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
//  Connect / Disconnect Socket from Card
STATUS CGideonSimManager::HandleCardAckStatusAction(CRequest *pRequest)
{
    CCommSetCardAckStatus* pAck = (CCommSetCardAckStatus*)pRequest->GetRequestObject();

	// add status to list
	m_pAckStatusList->NewStatusReceived(pAck->GetOpcode(),pAck->GetIsSendAck(),pAck->GetStatus());

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
//  Set end Bonding Timer
STATUS CGideonSimManager::HandleIsdnTimersAction(CRequest *pRequest)
{
    CCommSetIsdnTimers* pIsdnTimers = (CCommSetIsdnTimers*)pRequest->GetRequestObject();

	// Send parameters to media card (MFA/Barak)

	DWORD bndRmtLclAlignment = pIsdnTimers->GetBndRmtLclAlignment();
	DWORD opcode = SIM_API_ISDN_SET_TIMERS;

	CSegment*  pCopySeg = new CSegment;
	*pCopySeg 	<< opcode
				<< bndRmtLclAlignment;

	CCardCfg* pCurrCardCfg = NULL;
	for( int i=0; i < m_wMaxCards; i++ )
	{
		if( CPObject::IsValidPObjectPtr( m_ppSimCardApiArr[i] ) == TRUE )
		{
			pCurrCardCfg = m_pSimConfig->GetCardCfg( i );
			if( CPObject::IsValidPObjectPtr( pCurrCardCfg ) == TRUE )
			{
				if (pCurrCardCfg->IsMediaCard())
				{
					m_ppSimCardApiArr[i]->SendMsg( pCopySeg, SIM_API_MEDIA_CARD_SET_PARAMS );
					break;
				}
			}
		}
	}

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);

	return STATUS_OK;

}



/////////////////////////////////////////////////////////////////////////////
//  Set end Bonding Timer
STATUS CGideonSimManager::HandleIsdnEnableDisablePortsAction(CRequest *pRequest)
{
    CCommSetEnableDisableRtmPorts* pRtmPorts = (CCommSetEnableDisableRtmPorts*)pRequest->GetRequestObject();

	// Send parameters to media card (MFA/Barak)

	DWORD card 		= pRtmPorts->GetCard();
	DWORD span 		= pRtmPorts->GetSpan();
	DWORD firstPort = pRtmPorts->GetFirstPort();
	DWORD numPorts 	= pRtmPorts->GetNumPorts();
	DWORD action 	= pRtmPorts->GetAction();

	if (card >= m_wMaxCards)
	{
	    PTRACE(eLevelError,"CGideonSimManager::HandleIsdnEnableDisablePortsAction - Illegal card number");
	    return STATUS_FAIL;
	}

	DWORD opcode = SIM_API_RTM_ENABLE_DISABLE_PORTS;

	CSegment*  pCopySeg = new CSegment;
	*pCopySeg 	<< opcode
				<< span
				<< firstPort
				<< numPorts
				<< action;

	CCardCfg* pCurrCardCfg = NULL;
	if( CPObject::IsValidPObjectPtr( m_ppSimCardApiArr[card] ) == TRUE )
	{
		pCurrCardCfg = m_pSimConfig->GetCardCfg( card );
		if( CPObject::IsValidPObjectPtr( pCurrCardCfg ) == TRUE )
		{
			if((pCurrCardCfg->IsMediaCard()) &&
				( TRUE == (dynamic_cast<CBaseMediaCardCfg*>(pCurrCardCfg))->GetRtmAttached() ))
			{
				m_ppSimCardApiArr[card]->SendMsg( pCopySeg, SIM_API_MEDIA_CARD_SET_PARAMS );
			}
		}
	}

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);

	return STATUS_OK;

}



/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//  Insert card from slot
STATUS CGideonSimManager::HandleInsertCardEventAction(CRequest *pRequest)
{
	CCommInsertCardEvent* pAck = (CCommInsertCardEvent*)pRequest->GetRequestObject();

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);


	BYTE boardId = pAck->GetBoadrId();
	BYTE subBoardId = pAck->GetSubBoadrId();
	BOOL bIsRtm = (subBoardId==1) ? FALSE : TRUE;

	TRACEINTO << "CGideonSimManager::HandleInsertCardEventAction - boardId: " << (int)boardId << " subBoardId: " << (int)subBoardId;

	if (boardId >= m_pSimConfig->GetMaxCardSlots())
	{
		PASSERTMSG(TRUE, "CGideonSimManager::HandleInsertCardEventAction - Board Id is larger than max slot number");
		return STATUS_FAIL;
	}

	//slot is empty - insert new card to slot
	if (m_ppSimCardApiArr[boardId] == NULL)
	{
		//if new card is RTM - assert & return
		if (bIsRtm)
		{
			TRACEINTO << "CGideonSimManager::HandleInsertCardEventAction - Currently no MFA allocated.  boardId: " << (int)boardId << " subBoardId: " << (int)subBoardId;
			PASSERTMSG(TRUE, "CGideonSimManager::HandleInsertCardEventAction - Currently no MFA allocated");
			return STATUS_OK;
		}

		TRACEINTO << "CGideonSimManager::HandleInsertCardEventAction - Add new MFA. slot: " << (int)boardId;


		// create cards by configuration
		CCardCfg* pCurrCardCfg;
		eCardType cardType = (eCardType)pAck->GetCardType();
		  TRACEINTO << "CGideonSimManager::HandleInsertCardEventAction - cardType: " << (int)cardType;
		if(cardType == eMpmPlus_20 || cardType == eMpmPlus_40 || cardType == eMpmPlus_80)
			pCurrCardCfg = new CCardCfgBarak(boardId, "9707000", cardType, false);
		else if(cardType == eMpmx_80 || cardType == eMpmx_40 || cardType == eMpmx_20)
			pCurrCardCfg = new CCardCfgBreeze(boardId, "9707000", cardType, false);
		else if(cardType == eMpmRx_Full || cardType == eMpmRx_Half )
		    pCurrCardCfg = new CCardCfgMpmRx(boardId, "9707000", cardType, false);
		else
			pCurrCardCfg = new CCardCfgMfa(boardId, "9707000", cardType, false);
		// add card config to Cfg
		m_pSimConfig->SetCardCfg(boardId, pCurrCardCfg);

		//add card api to card array
		m_ppSimCardApiArr[boardId] = new CSimCardApi();
		//create card task
		m_ppSimCardApiArr[boardId]->Create(*m_pRcvMbx, pCurrCardCfg);
		//connect the task to mpl-api
		m_ppSimCardApiArr[boardId]->Connect();

		/*************************************************************************************************/
		/* 15.6.10 VNGR 15414 changed by Rachel Cohen                                                    */
		/* In MFA card or RTM card we need to send FatalFauilureInd to Cards (switchtask)                */
		/* When card is pushed in status field will set to eSmComponentNotExist (= 2 ) with bitmask 0x20 */
		/* When card is pushed out status field will set to eSmComponentOk (= 0 ) with bitmask 0x20      */
		/*************************************************************************************************/

		CSegment*  segInertCard = new CSegment;
					*segInertCard << boardId;
					*segInertCard << subBoardId;



		int switchBoardId = 0;
		CCardCfg* pCardCfg = NULL;
		eCardTypes cardTypeTmp;
		for (int i=0; i<m_wMaxCards; i++)
		{
			pCardCfg = m_pSimConfig->GetCardCfg(i);
			if( pCardCfg )
			{
				cardTypeTmp = pCardCfg->GetCardType();
				if (cardTypeTmp == eCardSwitch)
				{
					switchBoardId = i;
					break;
				}
			}
		}


		m_ppSimCardApiArr[switchBoardId]->SendMsg( segInertCard, (DWORD)SWITCH_SEND_INSERT_CARD_IND );




	}
	else
	{//slot is occupied

		//if new card is RTM
		if (bIsRtm)
		{
			CCardCfg* pCurrCardCfg = m_pSimConfig->GetCardCfg(boardId);
			CBaseMediaCardCfg* pMediaCard = dynamic_cast<CBaseMediaCardCfg*>(pCurrCardCfg);
			BOOL bCurrCardIsRtmAttached = pMediaCard->GetRtmAttached();

			if (bCurrCardIsRtmAttached)
			{
				// rtm slot is occupied
				TRACEINTO << "CGideonSimManager::HandleInsertCardEventAction - Occupied RTM PSTN. boardId: " << (int)boardId << " subBoardId: " << (int)subBoardId;
				PASSERTMSG(TRUE, "CGideonSimManager::HandleInsertCardEventAction - Occupied RTM PSTN");
				return STATUS_OK;
			}
			else
			{
				//insert RTM PSTN to slot
				/////////////////////////
				TRACEINTO << "CGideonSimManager::HandleInsertCardEventAction - Add new RTM. slot: " << (int)boardId;

				//1. update card in cfg with RTM attached
				pMediaCard->SetRtmAttached(TRUE);

				/*int switchBoardId = 0;
				CCardCfg* pCardCfg = NULL;
				eCardTypes cardType;
				for (int i=0; i<m_wMaxCards; i++)
				{
					pCardCfg = m_pSimConfig->GetCardCfg(i);
					cardType = pCardCfg->GetCardType();
					if (cardType == eCardSwitch)
					{
						switchBoardId = i;
						break;
					}
				}*/

				//2. create rtm logic & send to mpl-api
				CSegment*  seg = new CSegment;
				*seg << boardId;
				*seg << subBoardId;

				if(pCurrCardCfg->GetCardType() == eCardBarak || pCurrCardCfg->GetCardType() == eCardBreeze || pCurrCardCfg->GetCardType() == eCardMpmRx)
				{
					m_ppSimCardApiArr[boardId]->SendMsg( seg, (DWORD)BARAK_OR_BREEZE_INSERT_SUB_CARD );					
				}
				else
				{
					m_ppSimCardApiArr[boardId]->SendMsg( seg, (DWORD)MFA_INSERT_SUB_CARD );
			}
		}
		}
		else
		{
			// new card is mfa and slot allready is occupied with mfa
			TRACEINTO << "CGideonSimManager::HandleInsertCardEventAction - Occupied MFA board Id. boardId: " << (int)boardId << " subBoardId: " << (int)subBoardId;
			PASSERTMSG(TRUE, "CGideonSimManager::HandleInsertCardEventAction - Occupied MFA board Id");
		}
	}



	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//  Remove card from slot
STATUS CGideonSimManager::HandleRemoveCardEventAction(CRequest *pRequest)
{
	CCommRemoveCardEvent* pAck = (CCommRemoveCardEvent*)pRequest->GetRequestObject();

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);


	BYTE boardId = pAck->GetBoadrId();
	BYTE subBoardId = pAck->GetSubBoadrId();
	BOOL bIsRtm = (subBoardId==1) ? FALSE : TRUE;

	TRACEINTO << "CGideonSimManager::HandleRemoveCardEventAction - boardId: " << (int)boardId << " subBoardId: " << (int)subBoardId;


	if (boardId >= m_pSimConfig->GetMaxCardSlots())
	{
		PASSERTMSG(TRUE, "CGideonSimManager::HandleRemoveCardEventAction - Board Id is larger than max slot number");
		return STATUS_FAIL;
	}

	//MFA exists
	if (m_ppSimCardApiArr[boardId] != NULL)
	{
		CSegment*  seg = new CSegment;
		*seg << boardId;
		*seg << subBoardId;


		if (bIsRtm)
		{
			//card removed is RTM-PSTN
			//////////////////////////
			TRACEINTO << "CGideonSimManager::HandleRemoveCardEventAction - Remove RTM-PSTN card. slot: " << (int)boardId;

			CCardCfg* pCurrCardCfg = m_pSimConfig->GetCardCfg(boardId);
			CBaseMediaCardCfg* pCurrCard = dynamic_cast<CBaseMediaCardCfg*>(pCurrCardCfg);
			BOOL bCurrCardIsRtmAttached = pCurrCard->GetRtmAttached();

			//rtm-pstn
			if (!bCurrCardIsRtmAttached)
			{
				PASSERTMSG(TRUE, "CGideonSimManager::HandleRemoveCardEventAction - RTM-PSTN is not attached on board");
				return STATUS_FAIL;
			}

			//1. update card in cfg with RTM detached
			pCurrCard->SetRtmAttached(FALSE);

			//2. send msg to switch, only when remove card was done appropriatley
			int switchBoardId = 0;
			CCardCfg* pCardCfg = NULL;
			eCardTypes cardType;
			for (int i=0; i<m_wMaxCards; i++)
			{
				pCardCfg = m_pSimConfig->GetCardCfg(i);
				if( pCardCfg )
				{
					cardType = pCardCfg->GetCardType();
					if (cardType == eCardSwitch)
					{
						switchBoardId = i;
						break;
					}
				}
			}
			m_ppSimCardApiArr[switchBoardId]->SendMsg( seg, (DWORD)SWITCH_SEND_REMOVE_CARD_IND );

			return STATUS_OK;

		}


		//if card is removed via handle
		if (pAck->GetIsHandleRemove())
		{
			TRACEINTO << "CGideonSimManager::HandleRemoveCardEventAction - Handle is removed";

			//send msg to switch, only when remove card was done appropriatley

			int switchBoardId = 0;
			CCardCfg* pCardCfg = NULL;
			eCardTypes cardType;
			for (int i=0; i<m_wMaxCards; i++)
			{
				pCardCfg = m_pSimConfig->GetCardCfg(i);
				if(pCardCfg)
				{
					cardType = pCardCfg->GetCardType();
				}
				else
				{
					PASSERTMSG(1, "GetCardCfg return NULL !!!");
					break; 
				}
				if (cardType == eCardSwitch)
				{
					switchBoardId = i;
					break;
				}
			}

			m_ppSimCardApiArr[switchBoardId]->SendMsg( seg, (DWORD)SWITCH_SEND_REMOVE_CARD_IND );
		}
		else
		{
			TRACEINTO << "CGideonSimManager::HandleRemoveCardEventAction - Card is Dead";
		}




		//card removed is MFA
		/////////////////////

		TRACEINTO << "CGideonSimManager::HandleRemoveCardEventAction - Removing existing MFA Card. slot: " << (int)boardId;

		//destroy card task
		m_ppSimCardApiArr[boardId]->Destroy("CGideonSimManager::HandleRemoveCardEventAction - Removing existing Card");

		//delete card api from card array
		POBJDELETE(m_ppSimCardApiArr[boardId]);

		//clear card from Cfg
		m_pSimConfig->ClearCardCfg(boardId);

			TRACEINTO << "CGideonSimManager::HandleRemoveCardEventAction - Remove RTM-PSTN card. slot: " << (int)boardId;

	}
	else
	{
		PASSERTMSG(TRUE, "CGideonSimManager::HandleRemoveCardEventAction - Board Id does not exist");
		return STATUS_FAIL;
	}

	return STATUS_OK;

}


/////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////
//    GLOBAL FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CGideonSimSystemCfg* GetGideonSystemCfg()
{
	return g_pGideonSystemCfg;
}

/////////////////////////////////////////////////////////////////////////////
void SetGideonSystemCfg(CGideonSimSystemCfg* p)
{
	g_pGideonSystemCfg = p;
}

/////////////////////////////////////////////////////////////////////////////
CGideonSimCardAckStatusList* GetCardAckStatusList()
{
	return g_pCardAckStatusList;
}

/////////////////////////////////////////////////////////////////////////////
void SetCardAckStatusList(CGideonSimCardAckStatusList* p)
{
	g_pCardAckStatusList = p;
}


/////////////////////////////////////////////////////////////////////////////
void SendIsdnMessageToEndpointsSimApp(CSegment& rParam)
{
	CSegment*  pMsg = new CSegment(rParam);

    CManagerApi api(eProcessEndpointsSim);
	api.SendMsg(pMsg,SIM_API_ISDN_MSG);

/*	const COsQueue* pEpsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessEndpointsSim,eManager);

	CTaskApi api;
	api.CreateOnlyApi(*pEpsMbx);
	api.SendMsg(pMsg,SIM_API_ISDN_MSG);
	api.DestroyOnlyApi();
*/
}

/////////////////////////////////////////////////////////////////////////////
void SendAudioMessageToEndpointsSimApp(CSegment& rParam)
{
	CSegment*  pMsg = new CSegment(rParam);

	CManagerApi api(eProcessEndpointsSim);
	api.SendMsg(pMsg,SIM_API_AUDIO_MSG);

/*	const COsQueue* pEpsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessEndpointsSim,eManager);

	CTaskApi api;
	api.CreateOnlyApi(*pEpsMbx);
	api.SendMsg(pMsg,SIM_API_AUDIO_MSG);
	api.DestroyOnlyApi();
*/
}

/////////////////////////////////////////////////////////////////////////////
void SendMuxMessageToEndpointsSimApp(CSegment& rParam)
{
	CSegment*  pMsg = new CSegment(rParam);

    CManagerApi api(eProcessEndpointsSim);
	api.SendMsg(pMsg,SIM_API_MUX_MSG);

/*	const COsQueue* pEpsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessEndpointsSim,eManager);

	CTaskApi api;
	api.CreateOnlyApi(*pEpsMbx);
	api.SendMsg(pMsg,SIM_API_MUX_MSG);
	api.DestroyOnlyApi();
*/
}

/////////////////////////////////////////////////////////////////////////////
void SendMRMMessageToEndpointsSimApp(CSegment& rParam)
{
	CSegment*  pMsg = new CSegment(rParam);

    CManagerApi api(eProcessEndpointsSim);
	api.SendMsg(pMsg,SIM_API_MRM_MSG);
}


/////////////////////////////////////////////////////////////////////////////
void SendMessageToMediaMngrApp(CSegment& rParam)
{
    if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
    {
    	FPTRACE(eLevelInfoNormal,"SendMessageToMediaMngrApp - ERROR: tried to send message to MediaMngr process - system is not CG!!");
        return;
    }

	CSegment*  pMsg = new CSegment(rParam);

    CManagerApi api(eProcessMediaMngr);
	api.SendMsg(pMsg,SIM_API_H323_MSG);

}


/////////////////////////////////////////////////////////////////////////////
void SendMessageToLogger
			(
				const WORD level,
				const CPObject * pObj,
				const char *message1,
				const char *message2,
				const DWORD topic_id,
 				const char * terminalName,
				const DWORD unit_id,
				const DWORD conf_id,
				const DWORD party_id,
				const OPCODE opcode,
				const char * str_opcode
			)
{
	WORD messageLen = strlen(message1);
	if (NULL != message2)
	{
		messageLen += strlen(message2);
	}

		// Fill COMMON header
	COMMON_HEADER_S	tCommonHead;
	memset(&tCommonHead,0,sizeof(COMMON_HEADER_S));

//	tCommonHead.protocol_version   = MPL_PROTOCOL_VERSION_NUM;
//	tCommonHead.option =
	tCommonHead.src_id     = (BYTE)eMpl;
	tCommonHead.dest_id    = (BYTE)eMcms;
//	tCommonHead.opcode =
	tCommonHead.time_stamp = SystemGetTickCount().GetIntegerPartForTrace();
//	tCommonHead.sequence_num =
	tCommonHead.payload_len        = sizeof(COMMON_HEADER_S) + sizeof(TRACE_HEADER_S) + messageLen;
	tCommonHead.payload_offset     = sizeof(COMMON_HEADER_S);
	tCommonHead.next_header_offset = sizeof(TRACE_HEADER_S);
	tCommonHead.next_header_type   = eHeaderTrace;

		// Fill TRACE header
	TRACE_HEADER_S	tTraceHead;
	memset(&tTraceHead,0,sizeof(TRACE_HEADER_S));

	static DWORD lMessageCounter=0;
    lMessageCounter++; // count each message

	static CProcessBase *process = CProcessBase::GetProcess();

	tTraceHead.m_level 			= level;
	tTraceHead.m_topic_id		= topic_id;
	tTraceHead.m_unit_id 		= unit_id;
	tTraceHead.m_conf_id 		= conf_id;
	tTraceHead.m_party_id 		= party_id;
	tTraceHead.m_opcode 		= opcode;
//	tTraceHead.m_processType 	= process->GetProcessType();
	tTraceHead.m_processType 	= eTheOneTheOnlyMplProcess;
	tTraceHead.m_messageLen   	= messageLen;
	tTraceHead.m_sourceId		= (DWORD)pObj;
	tTraceHead.m_systemTick		= SystemGetTickCount().GetIntegerPartForTrace();
	tTraceHead.m_terminalName[0]= '\0';
	tTraceHead.m_taskName[0]  	= '\0';
    tTraceHead.m_objectName[0]	= '\0';
	tTraceHead.m_str_opcode[0]	= '\0';
	tTraceHead.m_processMessageNumber = lMessageCounter;

	const CTaskApp *currentTask = process->GetCurrentTask();
	if(NULL != currentTask)
	{
		strncpy(tTraceHead.m_taskName, currentTask->GetTaskName(),sizeof(tTraceHead.m_taskName) - 1);
		tTraceHead.m_taskName[sizeof(tTraceHead.m_taskName) - 1] = '\0';
	}

	if(NULL != str_opcode)
	{
		memcpy(tTraceHead.m_str_opcode, str_opcode, strlen(str_opcode) + 1);
	}

	if(0 != tTraceHead.m_sourceId)
	{
		if (CPObject::IsValidPObjectPtr(pObj))
		{
			strncpy(tTraceHead.m_objectName,
					pObj->NameOf(),
					sizeof(tTraceHead.m_objectName) - 1);
			tTraceHead.m_objectName[sizeof(tTraceHead.m_objectName) - 1] = '\0';
		}
	}

	if(NULL != terminalName)
	{
	  strncpy(tTraceHead.m_terminalName, terminalName, sizeof(tTraceHead.m_terminalName)-1);
	  tTraceHead.m_terminalName[sizeof(tTraceHead.m_terminalName)-1] = '\0';
	}

		// Put all data to segment
	CSegment*  pMsg = new CSegment();
	pMsg->Put((BYTE*)(&tCommonHead), sizeof(COMMON_HEADER_S));
	pMsg->Put((BYTE*)(&tTraceHead), sizeof(TRACE_HEADER_S));
	pMsg->Put((BYTE*)message1,strlen(message1));
	//*pMsg << message1;
	if( NULL != message2 )
		pMsg->Put((BYTE*)message2,strlen(message2));
		//*pMsg << message2;

	// Send message to Manager task
	CManagerApi api(eProcessGideonSim);
	api.SendMsg(pMsg, SIM_MESSAGE_FOR_LOGGER);

/*	const COsQueue* pMngrMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessGideonSim,eManager);

		// Send message to Manager task
	CTaskApi api;
	api.CreateOnlyApi(*pMngrMbx);
	api.SendMsg(pMsg, SIM_MESSAGE_FOR_LOGGER);
	api.DestroyOnlyApi();
*/
}

/////////////////////////////////////////////////////////////////////////////
void SendMessageToPCMSimApp(CSegment& rParam)
{
	CSegment*  pMsg = new CSegment(rParam);

	CTaskApi api;
	api.CreateOnlyApi(g_pcmTxMbx);
	api.SendMsg(pMsg,SOCKET_WRITE);
	api.DestroyOnlyApi();
}


