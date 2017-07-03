//+========================================================================+
//                   GideonSimCardTask.cpp                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimCardTask.cpp                                       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+
#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#include "Macros.h"
#include "Trace.h"
#include "CardsStructs.h"
#include "Segment.h"
#include "ProcessBase.h"
#include "MplMcmsProtocol.h"
#include "SystemFunctions.h"
#include "ClientSocket.h"
#include "SimApi.h"
#include "GideonSim.h"
#include "GideonSimConfig.h"
#include "GideonSimLogicalModule.h"
#include "GideonSimCardTask.h"
#include "StatusesGeneral.h"
#include "NStream.h"
#include "ObjString.h"
#include "TraceStream.h"
#include "CommGideonSimSet.h"
#include "Request.h"
#include "TranEntry.h"
#include "TerminalCommand.h"
#include "GideonSimSwitchLogical.h"
#include "GideonSimMfaLogical.h"
#include "GideonSimRtmLogical.h"
#include "OpcodesMcmsBonding.h"
#include "GideonSimBarakLogical.h"
#include "GideonSimLogicalParams.h"


/////////////////////////////////////////////////////////////////////////////
//
//   STATES:
//
//const WORD  IDLE        = 0;        // default state -  defined in base class
//const WORD  SETUP         = 1;
//const WORD  STARTUP       = 2;
//const WORD  CONFIG        = 3;
//const WORD  CONNECT       = 4;
//const WORD  PAUSE         = 5;
//const WORD  ANYCASE     = 0xFFFF;   // any other state -  defined in base class

///
///     ERROR CODES
///
const DWORD ERR_CONF_MPL_PARTY_ALREADY_CONNECTED	= 1001;
const DWORD ERR_CONF_MPL_PARTY_NOT_CONNECTED		= 1002;


/////////////////////////////////////////////////////////////////////////////
//
//   EXTERNALS:
//
extern "C" void SimCardRxEntryPoint(void* appParam);
extern "C" void SimCardTxEntryPoint(void* appParam);


/////////////////////////////////////////////////////////////////////////////
//
//   SimBasicCard - base class (abstract) for all simulation cards
//
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSimBasicCard)
//  ONEVENTA( XML_REQUEST,  IDLE,    (AFUNC)CSimBasicCard::NullActionFunction,  "XML_REQUEST", "IDLE" )
PEND_MESSAGE_MAP(CSimBasicCard,CStateMachine);


/////////////////////////////////////////////////////////////////////////////
//  no task creation function (abstract class)

/////////////////////////////////////////////////////////////////////////////
CSimBasicCard::CSimBasicCard()      // constructor
{
	m_wBoardId = 0xFFFF;
	m_wSubBoardId = 0xFFFF;
	m_szSerialNumber[0] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
CSimBasicCard::~CSimBasicCard()     // destructor
{
}


/////////////////////////////////////////////////////////////////////////////
void*  CSimBasicCard::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  CSimBasicCard::Create(CSegment& appParam)
{
	CTaskApp::Create(appParam);

	appParam	>> m_wBoardId
				>> m_szSerialNumber;

//	InitTimer(GetRcvMbx());
}

/////////////////////////////////////////////////////////////////////////////
void CSimBasicCard::SelfKill() //Override
{
//	DestroyTimer();
	DeleteAllTimers();
	CTaskApp::SelfKill();
}

/////////////////////////////////////////////////////////////////////////////
const char* CSimBasicCard::GetTaskName() const
{
	return "SimBasicCardTask";
}

/////////////////////////////////////////////////////////////////////////////
void CSimBasicCard::FillMplProtocol( CMplMcmsProtocol* pMplProt,
		const DWORD opcode,const BYTE* pData,const DWORD nDataLen ) const
{
	DWORD  payloadLen   =  sizeof(COMMON_HEADER_S)
						+  sizeof(MESSAGE_DESCRIPTION_HEADER_S)
						+  sizeof(PHYSICAL_INFO_HEADER_S)
						+  nDataLen;
	DWORD payloadOffset =  sizeof(COMMON_HEADER_S);

	pMplProt->AddCommonHeader(opcode,MPL_PROTOCOL_VERSION_NUM,0,
				(BYTE)eMpl,(BYTE)eMcms,SystemGetTickCount().GetIntegerPartForTrace(),
				payloadLen, payloadOffset,
				(DWORD)eHeaderMsgDesc,sizeof(MESSAGE_DESCRIPTION_HEADER_S));

	pMplProt->AddMessageDescriptionHeader(123/*requestId*/,(DWORD)eMpl/*entity_type*/,
				SystemGetTickCount().GetIntegerPartForTrace(),eHeaderPhysical,sizeof(PHYSICAL_INFO_HEADER_S));

// 	pMplProt->AddPortDescriptionHeader(0/*partyId*/,0/*confId*/ /*,connId,lrt1,lrt2,0,0,0,0*/);

	pMplProt->AddPhysicalHeader(0/*box_id*/,(BYTE)m_wBoardId,(BYTE)m_wSubBoardId);

	pMplProt->AddData(nDataLen,(const char*)pData);
}


/////////////////////////////////////////////////////////////////////////////
void CSimBasicCard::OnTerminalCommand(CMplMcmsProtocol* pMplProtocol)const
{
	char *data = pMplProtocol->getpData();

	CIstrStream istr(data);
	CTerminalCommand command;
	command.DeSerialize(istr);

	const string &terminalName = command.GetTerminalName();
	const string &commandName = command.GetCommandName();

	CProcessBase *process = CProcessBase::GetProcess();
	COstrStream answer;
	answer << "***** " << process->GetProcessName(process->GetProcessType()) << " *****\n";
	PTRACECOMMAND(terminalName.c_str(), commandName.c_str(), answer.str().c_str());
}



/////////////////////////////////////////////////////////////////////////////
//
//   SimSocketedCard - simulation card class (abstract) with socket connection
//
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSimSocketedCard)
	// Manager: connect card
	ONEVENT( CONNECT_CARD,           IDLE,    CSimSocketedCard::OnMngrConnectSocketIdle)
	// Socket: connection established
	ONEVENT( SOCKET_CONNECTED,       SETUP,   CSimSocketedCard::OnSocketConnectedSetup)
	// Socket: connection failed
	ONEVENT( SOCKET_FAILED,          SETUP,   CSimSocketedCard::OnSocketFailedSetup)
	// Socket: connection dropped
	ONEVENT( SOCKET_DROPPED,         SETUP,   CSimSocketedCard::OnSocketDroppedSetup)
	ONEVENT( SOCKET_DROPPED,         CONNECT, CSimSocketedCard::OnSocketDroppedConnect)

	ONEVENT( RESUME_SOCKET,          PAUSE,   CSimSocketedCard::OnSocketResumePause)
	ONEVENT( PAUSE_SOCKET,           CONNECT, CSimSocketedCard::OnSocketPauseConnect)


	// Socket : message received
	ONEVENT( SOCKET_RCV_MSG,         IDLE,     CSimSocketedCard::OnSocketRcvIdle)
 	ONEVENT( SOCKET_RCV_MSG,         SETUP,    CSimSocketedCard::OnSocketRcvSetup)
	ONEVENT( SOCKET_RCV_MSG,         CONNECT,  CSimSocketedCard::OnSocketRcvConnect)
	// Logical module: forward message to MPL-API
	ONEVENT( FORWARD_MSG_MPLAPI,     ANYCASE,  CSimSocketedCard::OnLogicModuleForwardMsgAnycase)

	// Simulate reset card
	ONEVENT( SIM_RESET_CARD,         IDLE,     CSimSocketedCard::OnMngrResetCardIdle)
	ONEVENT( SIM_RESET_CARD,         SETUP,    CSimSocketedCard::OnMngrResetCardSetup)
	ONEVENT( SIM_RESET_CARD,         CONNECT,  CSimSocketedCard::OnMngrResetCardConnect)

PEND_MESSAGE_MAP(CSimSocketedCard,CSimBasicCard);

/////////////////////////////////////////////////////////////////////////////
//  no task creation function (abstract class)

/////////////////////////////////////////////////////////////////////////////
CSimSocketedCard::CSimSocketedCard()      // constructor
{
	m_pSocketConnection = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CSimSocketedCard::~CSimSocketedCard()     // destructor
{
	POBJDELETE(m_pSocketConnection);
}


/////////////////////////////////////////////////////////////////////////////
void*  CSimSocketedCard::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  CSimSocketedCard::Create(CSegment& appParam)
{
	CSimBasicCard::Create(appParam);

	char  szIpAddress[100];
	WORD  port=0;

	appParam	>> szIpAddress
				>> port;

	POBJDELETE(m_pSocketConnection);
	m_pSocketConnection = new CClientSocket(this,SimCardRxEntryPoint,SimCardTxEntryPoint);
	m_pSocketConnection->Init(szIpAddress,port);
}

/////////////////////////////////////////////////////////////////////////////
const char* CSimSocketedCard::GetTaskName() const
{
	return "SimSocketedCardTask";
}

/////////////////////////////////////////////////////////////////////////////
// temp func for Talya
void CSimSocketedCard::SendToSocket(CSegment& paramSegment) const
{
	// paramSegment should contain COMMON_HEADER, PHISYCAL_HEADER and data
	if( !CPObject::IsValidPObjectPtr(m_pSocketConnection) ) {
		PASSERT(1);
		return;
	}

//	WORD  headerSize = sizeof(TPKT_HEADER_S);
//
//	// fill LEVEL1 header information
//	TPKT_HEADER_S   tTpktStruct;
//	tTpktStruct.version_num      = TPKT_VERSION_NUM;
//	tTpktStruct.payload_len      = (WORD)(headerSize + paramSegment.GetWrtOffset());
//
	CSegment* pMsg = new CSegment; // pMsg will delete inside
//
//	// put LEVEL1 header to message
//	pMsg->Put((BYTE*)(&tTpktStruct),headerSize);

	// put parameters to message
	*pMsg  << paramSegment;

	m_pSocketConnection->Send(pMsg); // pMsg will delete inside
}

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::SendToMplApi(CMplMcmsProtocol& rMplProtocol) const
{
	if( !CPObject::IsValidPObjectPtr(m_pSocketConnection) ) {
		PASSERT(1);
		return;
	}

	CSegment  rParamSegment;
	rMplProtocol.Serialize(rParamSegment);

	SendToMplApi(rParamSegment);
}

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::SendToMplApi(CSegment& rParam) const
{
	if( !CPObject::IsValidPObjectPtr(m_pSocketConnection) ) {
		PASSERT(1);
		return;
	}

//	WORD  headerSize = sizeof(TPKT_HEADER_S);

//	// fill LEVEL1 header information
//	TPKT_HEADER_S   rTpktStruct;
//	rTpktStruct.version_num      = TPKT_VERSION_NUM;
//	rTpktStruct.payload_len      = (WORD)(headerSize + rParam.GetWrtOffset());

	CSegment* pMsg = new CSegment; // pMsg will delete inside

//	// put LEVEL1 header to message
//	pMsg->Put((BYTE*)(&rTpktStruct),headerSize);

	// put parameters to message
	*pMsg  << rParam;

	m_pSocketConnection->Send(pMsg);

	POBJDELETE(pMsg);
}


/////////////////////////////////////////////////////////////////////////////
// App Manager: establish connection with MPL-API

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::OnMngrConnectSocketIdle(CSegment* pMsg)
{
	if( !CPObject::IsValidPObjectPtr(m_pSocketConnection) )
		return;

	m_state = SETUP;

 	m_pSocketConnection->Connect();
}


/////////////////////////////////////////////////////////////////////////////
// Socket: connection established

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::OnSocketConnectedSetup(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimSocketedCard::OnSocketConnectedSetup");

	m_state = CONNECT;
}


/////////////////////////////////////////////////////////////////////////////
// Socket: connection failed

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::OnSocketFailedSetup(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimSocketedCard::OnSocketFailedSetup");

	// endless loop
	if( CPObject::IsValidPObjectPtr(m_pSocketConnection) )
		m_pSocketConnection->Connect();
}


/////////////////////////////////////////////////////////////////////////////
// Socket: connection dropped
//

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::OnSocketDroppedSetup(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimSocketedCard::OnSocketDroppedSetup");
	DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::OnSocketDroppedConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimSocketedCard::OnSocketDroppedConnect");
	OnSocketDropped(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::OnSocketDropped(CSegment* pMsg)
{
	m_state = SETUP;

	if( CPObject::IsValidPObjectPtr(m_pSocketConnection) ) {
		m_pSocketConnection->Disconnect();
		m_pSocketConnection->Connect();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::OnSocketPauseConnect(CSegment* pMsg)
{
	m_state = PAUSE;
    PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnSocketPause m_state = PAUSE");
	if( CPObject::IsValidPObjectPtr(m_pSocketConnection) ) {
		m_pSocketConnection->Disconnect();
		}
}
/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::OnSocketResumePause(CSegment* pMsg)
{
/*	//m_state = SETUP;
    PTRACE(eLevelInfoNormal,"CSimCSEndpointsModule::OnSocketResume m_state = SETUP");
	if( CPObject::IsValidPObjectPtr(m_pSocketConnection) ) {
		m_pSocketConnection->Connect();
		}
		*/
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Socket: message received
//

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::OnSocketRcvIdle(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimSocketedCard::OnSocketRcvIdle");
	DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::OnSocketRcvSetup(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimSocketedCard::OnSocketRcvSetup");
	DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::OnSocketRcvConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimSocketedCard::OnSocketRcvConnect");
	DispatchMcmsMsg(pMsg);
}


/////////////////////////////////////////////////////////////////////////////
// Logical module: forward message to MPL-API
//

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::OnLogicModuleForwardMsgAnycase(CSegment* pMsg)
{
	//PTRACE(eLevelInfoNormal,"CSimSocketedCard::OnLogicModuleForwardMsgAnycase");

	CSegment* pCopySeg = new CSegment(*pMsg);
	SendToMplApi(*pCopySeg);
	POBJDELETE(pCopySeg);
}

/////////////////////////////////////////////////////////////////////////////
// Socket: simulate card reset with/out problems
//

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::OnMngrResetCardIdle(CSegment* pMsg)
{
	TRACEINTO << "Do nothing";
	//DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::OnMngrResetCardSetup(CSegment* pMsg)
{
	TRACEINTO << "Do nothing";
	//DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CSimSocketedCard::OnMngrResetCardConnect(CSegment* pMsg)
{
	WORD isProblem = 0;
	*pMsg >> isProblem;

	TRACEINTO << "Reset card with problem=" << isProblem;

	m_state = SETUP;

	if( CPObject::IsValidPObjectPtr(m_pSocketConnection) ) {
		m_pSocketConnection->Disconnect();
		m_pSocketConnection->Connect();
	}
}





/////////////////////////////////////////////////////////////////////////////
//
//   SimSwitchCard - SWITCH card simulation class implements SWITCH card
//
/////////////////////////////////////////////////////////////////////////////
//
//	STATES:
//		IDLE      - default state immediately after creation
//		SETUP     - from Connect command until socket connection will be established
//		STARTUP   - socket connected, start configure switch card
//		CONNECT   - switch card started up
//		RECOVERY (?)
//		RESET    (?)

PBEGIN_MESSAGE_MAP(CSimSwitchCard)
	// Socket: connection established
	ONEVENT( SOCKET_CONNECTED,   SETUP,     CSimSwitchCard::OnSocketConnectedSetup)
	// Switch: connected
 	ONEVENT( SWITCH_CONNECTED,   CONNECT,   CSimSwitchCard::OnSwitchEndConnectConnect)
 	ONEVENT( SET_UNIT_STATUS_FOR_KEEP_ALIVE_NEW,  CONNECT,   CSimSwitchCard::OnSetUnitStatustSimSwitchForKeepAliveIndConnect)
 	ONEVENT( SWITCH_SEND_REMOVE_CARD_IND,   CONNECT,   CSimSwitchCard::OnSwitchSendRemoveCardIndConnect)
	ONEVENT( SWITCH_SEND_INSERT_CARD_IND,   CONNECT,   CSimSwitchCard::OnSwitchSendInsertCardIndConnect)
 	ONEVENT( UPDATE_BURN_RATE,			    ANYCASE,	 CSimSwitchCard::OnSetBurnRate)
 	ONEVENT( SET_BURN_ACTIONS,				ANYCASE,	 CSimSwitchCard::OnSetBurnAction)
 	ONEVENT( SIM_USER_LDAP_LOGIN,	CONNECT,   CSimSwitchCard::OnUserLdapLogin)
 	
PEND_MESSAGE_MAP(CSimSwitchCard,CSimSocketedCard);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void gideonSimSwitchCardEntryPoint(void* appParam)
{
	CSimSwitchCard*  pSimSwitchCard = new CSimSwitchCard;
	pSimSwitchCard->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CSimSwitchCard::CSimSwitchCard()      // constructor
{
	m_pSwitchLogics = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CSimSwitchCard::~CSimSwitchCard()     // destructor
{
	POBJDELETE(m_pSwitchLogics);
}


/////////////////////////////////////////////////////////////////////////////
void  CSimSwitchCard::Create(CSegment& appParam)
{
	CSimSocketedCard::Create(appParam);

	DWORD  temp=0;

	appParam >> temp; // additional flag, not in use

	PTRACE(eLevelInfoNormal,"CSimSwitchCard::Create - Creating SWITCH module.");
	//LOGGER_TRACE(eLevelInfoNormal,"CSimSwitchCard::Create - Creating SWITCH module. VASILYLOG");
	m_pSwitchLogics = new CGideonSimSwitchLogical(this,m_wBoardId,1);
}

/////////////////////////////////////////////////////////////////////////////
void*  CSimSwitchCard::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
const char* CSimSwitchCard::GetTaskName() const
{
	return "SimSwitchCardTask";
}

/////////////////////////////////////////////////////////////////////////////
void CSimSwitchCard::DispatchMcmsMsg(CSegment* pMsg) const
{
	CSegment* pCopySeg = new CSegment(*pMsg);
	//  deserialize protocol
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pCopySeg);

	if( pMplProtocol->getCommonHeaderOpcode() == TERMINAL_COMMAND )
	{
		OnTerminalCommand(pMplProtocol);
	}
	else if ( pMplProtocol->getPhysicalInfoHeaderCounter() == 1 ) {
		// get copy of Physical Header
		PHYSICAL_INFO_HEADER_S  rPhysicalStruct;
		if ( pMplProtocol->GetPhysicalInfoHeaderCopy(0,&rPhysicalStruct) == STATUS_OK ) {
			if( rPhysicalStruct.board_id == m_wBoardId ) {
				m_pSwitchLogics->ProcessMcmsMsg(pMsg);
			} else
				PASSERT(100+rPhysicalStruct.board_id);
		}
	} else
	{
// disabled temporary until VNGSW-277 is fixed
//		PASSERT(100+pMplProtocol->getPhysicalInfoHeaderCounter());
//		PASSERT(pMplProtocol->getCommonHeaderOpcode());
	}
	POBJDELETE(pMplProtocol);
	POBJDELETE(pCopySeg);
}

/////////////////////////////////////////////////////////////////////////////
// Socket: connection established

/////////////////////////////////////////////////////////////////////////////
void CSimSwitchCard::OnSocketConnectedSetup(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimSwitchCard::OnSocketConnectedSetup");

	m_state = CONNECT;

	m_pSwitchLogics->Startup();
}

/////////////////////////////////////////////////////////////////////////////
// Switch logical: start up completed

/////////////////////////////////////////////////////////////////////////////
void CSimSwitchCard::OnSwitchEndConnectConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimSwitchCard::OnSwitchEndConnectConnect - SWITCH MODULE finished start-up.");
	//LOGGER_TRACE(eLevelInfoNormal,"CSimSwitchCard::OnSwitchEndConnectConnect - SWITCH MODULE finished start-up.VASILYLOG");
}

/////////////////////////////////////////////////////////////////////////////
void CSimSwitchCard::OnSetUnitStatustSimSwitchForKeepAliveIndConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimSwitchCard::OnSetUnitStatustSimSwitchForKeepAliveIndConnect");
	DWORD opcode;
	WORD  boardId, UnitId ,UnitStatus, dummy,UnitStatus3,UnitStatus4;

	CSegment* pCopySeg  = new CSegment(*pMsg);
	*pCopySeg	>> opcode
				>> boardId
				>> dummy
				>> UnitId
				>> dummy
				>> UnitStatus
				>> dummy;
	POBJDELETE(pCopySeg);

// print to trace
	CLargeString kaStr = "CSimSwitchCard::OnSetUnitStatustSimSwitchForKeepAliveIndConnect get the XML Results:\n==================\n";
			kaStr << " \n opcode     : " << opcode  ;
			kaStr << " \n boardId    : " << boardId  ;
			kaStr << " \n UnitId     : " << UnitId ;
			kaStr << " \n UnitStatus : " << UnitStatus ;


	TRACEINTO << kaStr.GetString();

m_pSwitchLogics->SetUnitStatustSwitchForKeepAliveInd(UnitId,UnitStatus);

}

/////////////////////////////////////////////////////////////////////////////

void CSimSwitchCard::OnSwitchSendRemoveCardIndConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimSwitchCard::OnSwitchSendRemoveCardIndConnect");

	BYTE  boardId;
	BYTE  subBoardId;

	*pMsg >> boardId
	      >> subBoardId;

	TRACEINTO << "CSimSwitchCard::OnSwitchSendRemoveCardIndConnect boardId: " << (int)boardId  << " subBoardId: " << (int)subBoardId;

	//send to Swich Logical Module
	m_pSwitchLogics->SetUnitStatusSwitchForMFARemoveInd(boardId, subBoardId);
}

/////////////////////////////////////////////////////////////////////////////

void CSimSwitchCard::OnSwitchSendInsertCardIndConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimSwitchCard::OnSwitchSendInsertCardIndConnect");

	BYTE  boardId;
	BYTE  subBoardId;

	*pMsg >> boardId
	      >> subBoardId;

	TRACEINTO << "CSimSwitchCard::OnSwitchSendInsertCardIndConnect boardId: " << (int)boardId  << " subBoardId: " << (int)subBoardId;

	//send to Swich Logical Module
	m_pSwitchLogics->SetUnitStatusSwitchForMFAInsertInd(boardId, subBoardId);
}

/////////////////////////////////////////////////////////////////////////////
void CSimSwitchCard::OnSetBurnRate(CSegment* pMsg)
{
	DWORD  rate;
	DWORD burnType;
	*pMsg >> rate
		  >>burnType;

	TRACEINTO << "CSimSwitchCard::OnSetBurnRate rate: " << (int)rate;

	if(burnType == eVersionBurnType)
		m_pSwitchLogics->SetVersionBurnRate(rate);
	else if(burnType == eIpmcBurnType)
		m_pSwitchLogics->SetIPMCBurnRate(rate);

}

/////////////////////////////////////////////////////////////////////////////
void CSimSwitchCard::OnSetBurnAction(CSegment* pMsg)
{
	DWORD burnActionType;
	DWORD burnType;

	*pMsg >>burnActionType;
	*pMsg >>burnType;

	TRACEINTO << "CSimSwitchCard::OnSetBurnAction ";

	m_pSwitchLogics->SetBurnAction((eBurnActionTypes)burnActionType,(eBurnTypes)burnType);

}

/////////////////////////////////////////////////////////////////////////////
//
//   SimMfaCard - MFA card simulation class implements MFA card
//
/////////////////////////////////////////////////////////////////////////////
//
//	STATES:
//		IDLE      - default state immediately after creation
//		SETUP     - from Connect command until socket connection will be established
//		CONNECT   - socket connected, start configure switch card
//		RECOVERY (?)
//		RESET    (?)

PBEGIN_MESSAGE_MAP(CSimMfaCard)
	// Socket: connection established
	ONEVENT( SOCKET_CONNECTED,						SETUP,     CSimMfaCard::OnSocketConnectedSetup)
	// MFA logical: start up completed
 	ONEVENT( MFA_CONNECTED,							CONNECT,   CSimMfaCard::OnMfaEndConnectConnect)
 	// MFA logical: insert RTM-PSTN sub board
 	ONEVENT( MFA_INSERT_SUB_CARD,				  CONNECT,   CSimMfaCard::OnMfaInsertSubCardConnect)
	// Endpoints Sim: message received to this card
  	ONEVENT( SIM_API_ISDN_MSG,						CONNECT,   CSimMfaCard::OnEndpointsSimIsdnMsgConnect)
	ONEVENT( SIM_API_AUDIO_MSG,						CONNECT,   CSimMfaCard::OnEndpointsSimAudioMsgConnect)
	ONEVENT( SIM_API_MUX_MSG,                   	CONNECT,   CSimMfaCard::OnEndpointsSimMuxMsgConnect)
	ONEVENT( SET_UNIT_STATUS_FOR_KEEP_ALIVE_NEW, 	CONNECT,   CSimMfaCard::OnSetUnitStatustSimMfaForKeepAliveIndConnect)
	ONEVENT( SIM_API_MEDIA_CARD_SET_PARAMS,		  CONNECT,   CSimMfaCard::OnSetMediaCardParams)
PEND_MESSAGE_MAP(CSimMfaCard,CSimSocketedCard);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void gideonSimMfaCardEntryPoint(void* appParam)
{
	CSimMfaCard*  pSimMfaCard = new CSimMfaCard;
	pSimMfaCard->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CSimMfaCard::CSimMfaCard()      // constructor
{
	m_pMfaLogics = NULL;
	m_pRtmLogics = NULL;
	m_isRtmAttached = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
CSimMfaCard::~CSimMfaCard()     // destructor
{
	POBJDELETE(m_pMfaLogics);
	POBJDELETE(m_pRtmLogics);
}


/////////////////////////////////////////////////////////////////////////////
void*  CSimMfaCard::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  CSimMfaCard::Create(CSegment& appParam)
{
	CSimSocketedCard::Create(appParam);

	WORD  isRtm = 0;
	appParam	>> isRtm;



	m_isRtmAttached = (isRtm) ? TRUE : FALSE;

	WORD  cardType = 0;
	appParam	>> cardType;
	m_RtmCardType = (eCardType)cardType;

	WORD  subBoardId = 1;

		// mfa create
	m_pMfaLogics = new CGideonSimMfaLogical(this,m_wBoardId,subBoardId++);
	PTRACE(eLevelInfoNormal,"CSimMfaCard::Create - Creating MFA module.");
	//LOGGER_TRACE(eLevelInfoNormal,"CSimMfaCard::Create - Creating MFA module.VASILYLOG");
		// rtm create only if available in config.xml
	if( m_isRtmAttached )
	{
		m_pRtmLogics = new CGideonSimRtmLogical(this,m_wBoardId,subBoardId++,m_RtmCardType);
		PTRACE(eLevelInfoNormal,"CSimMfaCard::Create - Creating RTM module.");
		//LOGGER_TRACE(eLevelInfoNormal,"CSimMfaCard::Create - Creating RTM module.VASILYLOG");
	}
}

/////////////////////////////////////////////////////////////////////////////
const char* CSimMfaCard::GetTaskName() const
{
	return "SimMfaCardTask";
}

				///vb TEMP start
#include "OpcodesMcmsNetQ931.h"
				///vb TEMP end

/////////////////////////////////////////////////////////////////////////////
void CSimMfaCard::DispatchMcmsMsg(CSegment* pMsg) const
{
	CSegment* pCopySeg = new CSegment(*pMsg);
	//  deserialize protocol
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pCopySeg);

	if( pMplProtocol->getCommonHeaderOpcode() == TERMINAL_COMMAND )
	{
		OnTerminalCommand(pMplProtocol);
	}
	else if ( pMplProtocol->getPhysicalInfoHeaderCounter() == 1 )
	{
		// get copy of Physical Header
		PHYSICAL_INFO_HEADER_S  rPhysicalStruct;
		if ( pMplProtocol->GetPhysicalInfoHeaderCopy(0,&rPhysicalStruct) == STATUS_OK )
		{
			if( rPhysicalStruct.board_id == m_wBoardId )
			{
				///vb TEMP start
				if( (pMplProtocol->getOpcode()>=NET_SETUP_REQ && pMplProtocol->getOpcode()<=NET_CONNECT_REQ) /*||
					//(pMplProtocol->getOpcode()>=BND_CONNECTION_INIT && pMplProtocol->getOpcode()<=BND_END_DISCONNECT) )
					(pMplProtocol->getOpcode() == BND_CONNECTION_INIT)*/)
				{
					m_pRtmLogics->ProcessMcmsMsg(pMsg);
				}
				else
				{
					///vb TEMP end
					if ( rPhysicalStruct.sub_board_id == m_pMfaLogics->GetSubBoardId() )
					{
						m_pMfaLogics->ProcessMcmsMsg(pMsg);
					}
					else if ( rPhysicalStruct.sub_board_id == m_pRtmLogics->GetSubBoardId() )
					{
						m_pRtmLogics->ProcessMcmsMsg(pMsg);
					}
					else
					{
						PASSERT(100+rPhysicalStruct.sub_board_id);
					}
				}
			}
			else
			{
				PASSERT(100+rPhysicalStruct.board_id);
			}
		}
		else
		{
			PASSERT(100);
		}
	}
	else
	{
		PASSERT(100+pMplProtocol->getPhysicalInfoHeaderCounter());
	}

	POBJDELETE(pMplProtocol);
	POBJDELETE(pCopySeg);
}


/////////////////////////////////////////////////////////////////////////////
// Socket: connection failed

/////////////////////////////////////////////////////////////////////////////
void CSimMfaCard::OnSocketFailedSetup(CSegment* pParam)
{
 	CSimSocketedCard::OnSocketFailedSetup(pParam);

//	Vasily::TEMP -   for Amir
//	int a = 5;
//
//	DWORD  opcode = 11;
//	CSegment*  pMsg = new CSegment;
//
//	*pMsg	<< m_wBoardId
//			<< (WORD)5  // unit id
//			<< opcode;
//
// 	::SendMessageToEndpointsSimApp(pMsg);
//	Vasily::TEMP -   for Amir

}


/////////////////////////////////////////////////////////////////////////////
// Socket: connection established

/////////////////////////////////////////////////////////////////////////////
void CSimMfaCard::OnSocketConnectedSetup(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimMfaCard::OnSocketConnectedSetup");

	m_state = CONNECT;
	m_pMfaLogics->Startup();
	if( m_pRtmLogics )
		m_pRtmLogics->Startup();
}

/////////////////////////////////////////////////////////////////////////////
void CSimMfaCard::OnMfaEndConnectConnect(CSegment* pMsg)
{
//	WORD boardId = 0, subBoardId = 0;
//	*pMsg	>> boardId
//			>> subBoardId;

	char szMessage[64];
	sprintf(szMessage,"BoardId <%d>",m_wBoardId);//(int)boardId,(int)subBoardId);
	PTRACE2(eLevelInfoNormal,"CSimMfaCard::OnMfaEndConnectConnect - MFA MODULE finished start-up: ",szMessage);
	//LOGGER_TRACE2(eLevelInfoNormal,"CSimMfaCard::OnMfaEndConnectConnect - VASILYLOG - MFA MODULE finished start-up: ",szMessage);
}

/////////////////////////////////////////////////////////////////////////////
void CSimMfaCard::OnMfaInsertSubCardConnect(CSegment* pMsg)
{
	BYTE  boardId;
	BYTE  subBoardId;

	*pMsg >> boardId
	      >> subBoardId;

	TRACEINTO << "CSimMfaCard::OnMfaInsertSubCardConnect boardId: " << (int)boardId << " subBoardId: " << (int)subBoardId;

	m_isRtmAttached = true;

	if (m_pRtmLogics == NULL)
		m_pRtmLogics = new CGideonSimRtmLogical(this, boardId, subBoardId);

	//send to RTM Logical Module
	m_pRtmLogics->Startup();
}


/////////////////////////////////////////////////////////////////////////////
// Endpoints Sim: PSTN message received
/////////////////////////////////////////////////////////////////////////////
void CSimMfaCard::OnEndpointsSimIsdnMsgConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimMfaCard::OnEndpointsSimIsdnMsgConnect - Received message from EP.");

	DWORD opcode=0;
	WORD  boardId = 0xFFFF, subBoardId = 0xFFFF;

	CSegment* pCopySeg  = new CSegment(*pMsg);

	*pCopySeg	>> opcode
				>> boardId
				>> subBoardId;

	POBJDELETE(pCopySeg);

	// NET_SETUP_IND with unknown boardId / subBoardId
	if( NET_SETUP_IND == opcode && 0xFFFF == boardId && 0xFFFF == subBoardId )
	{
		boardId    = m_wBoardId;
		subBoardId = m_pRtmLogics->GetSubBoardId();
	}

	if( boardId != m_wBoardId ) {
		PASSERT_AND_RETURN(1000+boardId);
	}

	if( m_pRtmLogics  &&  subBoardId == m_pRtmLogics->GetSubBoardId() )
		m_pRtmLogics->ProcessEndpointsSimMsg(pMsg);

/*	for( BYTE i=0; i<MAX_MFA_LOGICS_IN_LITE_CARD; i++ ) {
		if( m_pRtmLogicsArr[i] && subBoardId == m_pRtmLogicsArr[i]->GetSubBoardId() )
			m_pRtmLogicsArr[i]->ProcessEndpointsSimMsg(pMsg);
	}*/
}

/////////////////////////////////////////////////////////////////////////////
// Endpoints Sim: Audio message received

/////////////////////////////////////////////////////////////////////////////
void CSimMfaCard::OnSetUnitStatustSimMfaForKeepAliveIndConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimMfaCard::OnSetUnitStatustSimMfaForKeepAliveIndConnect");
	DWORD opcode=0;
	WORD  boardId = 0xFFFF, UnitId = 0xFFFF, UnitStatus = 0xFFFF, dummy = 0, UnitStatus3 = 0xFFFF, UnitStatus4 = 0xFFFF;

	CSegment* pCopySeg  = new CSegment(*pMsg);
	*pCopySeg	>> opcode
				>> boardId
				>> dummy
				>> UnitId
				>> dummy
				>> UnitStatus
				>> dummy;
	POBJDELETE(pCopySeg);

// print to trace
	CLargeString kaStr = "CSimMfaCard::OnSetUnitStatustSimMfaForKeepAliveIndConnect get the XML Results:\n==================\n";
			kaStr << " \n opcode     : " << opcode  ;
			kaStr << " \n boardId    : " << boardId  ;
			kaStr << " \n UnitId     : " << UnitId ;
			kaStr << " \n UnitStatus : " << UnitStatus ;


	TRACEINTO << kaStr.GetString();

	m_pMfaLogics->SetUnitStatustSimMfaForKeepAliveInd(UnitId,UnitStatus);

}


/////////////////////////////////////////////////////////////////////////////
void CSimMfaCard::OnSetMediaCardParams(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimMfaCard::OnSetMediaCardParams ");

	DWORD opcode=0;
	*pMsg >> opcode;

	switch( opcode )
	{
		case SIM_API_ISDN_SET_TIMERS:
		{
			DWORD bndRmtLclAlignment = 0;
			*pMsg >> bndRmtLclAlignment;
			if (m_pMfaLogics)
				m_pMfaLogics->SetBndLclAlignmentTimer( bndRmtLclAlignment );
			break;
		}

		case SIM_API_RTM_ENABLE_DISABLE_PORTS:
		{
			DWORD span=0;
			DWORD firstPort=0;
			DWORD numPorts=0;
			DWORD action=0;

			*pMsg >> span >> firstPort >> numPorts >> action;

			if (m_pRtmLogics)
				m_pRtmLogics->SetRtmEnableDisablePorts( span, firstPort, numPorts, action );
			break;
		}

		default:
			return;
	}

}


/////////////////////////////////////////////////////////////////////////////
void CSimMfaCard::OnEndpointsSimAudioMsgConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimMfaCard::OnEndpointsSimAudioMsgConnect - Received message from EP.");

	DWORD opcode=0;
	WORD  boardId = 0xFFFF, subBoardId = 0xFFFF;

	CSegment* pCopySeg  = new CSegment(*pMsg);

	*pCopySeg	>> opcode
				>> boardId
				>> subBoardId;

	POBJDELETE(pCopySeg);

	if( boardId != m_wBoardId ) {
		PASSERT_AND_RETURN(1000+boardId);
	}

	m_pMfaLogics->ProcessEndpointsSimMsg(pMsg);
/*	for( BYTE i=0; i<MAX_MFA_LOGICS_IN_LITE_CARD; i++ ) {
		if( m_pMfaLogicsArr[i] && subBoardId == m_pMfaLogicsArr[i]->GetSubBoardId() )
			m_pMfaLogicsArr[i]->ProcessEndpointsSimMsg(pMsg);
	}*/
}


/////////////////////////////////////////////////////////////////////////////
void CSimMfaCard::OnEndpointsSimMuxMsgConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimMfaCard::OnEndpointsSimMuxMsgConnect - Received message from EP.");

	DWORD opcode=0;
	WORD  boardId = 0xFFFF, subBoardId = 0xFFFF;

	CSegment* pCopySeg  = new CSegment(*pMsg);

	*pCopySeg	>> opcode
				>> boardId
				>> subBoardId;

	POBJDELETE(pCopySeg);

	if( boardId != m_wBoardId ) {
		PASSERT_AND_RETURN(1000+boardId);
	}

	m_pMfaLogics->ProcessEndpointsSimMsg(pMsg);
/*	for( BYTE i=0; i<MAX_MFA_LOGICS_IN_LITE_CARD; i++ ) {
		if( m_pMfaLogicsArr[i] && subBoardId == m_pMfaLogicsArr[i]->GetSubBoardId() )
			m_pMfaLogicsArr[i]->ProcessEndpointsSimMsg(pMsg);
	}*/
}


/////////////////////////////////////////////////////////////////////////////
void CSimMfaCard::PartyReaction( CMplMcmsProtocol* pMplProt )
{
	COMMON_HEADER_S					rCommonH;
	pMplProt->GetCommonHeaderCopy(&rCommonH);

	MESSAGE_DESCRIPTION_HEADER_S	rMsgDescH;
	pMplProt->GetMsgDescHeaderCopy(&rMsgDescH);

	PORT_DESCRIPTION_HEADER_S		rPortDescH;
	pMplProt->GetPortDescHeaderCopy(&rPortDescH);

	DWORD foundIndex = MAX_LOGICAL_PARTIES_PER_RTM;
	DWORD firstEmpty = MAX_LOGICAL_PARTIES_PER_RTM;

	for( DWORD  i=0; i<MAX_LOGICAL_PARTIES_PER_RTM; i++ ) {
		if( rPortDescH.conf_id == m_LogicalPartiesArr[i].m_confId &&
				rPortDescH.party_id == m_LogicalPartiesArr[i].m_partyId )
			foundIndex = i;
		if( MAX_LOGICAL_PARTIES_PER_RTM == firstEmpty &&
				0 == m_LogicalPartiesArr[i].m_confId &&
				0 == m_LogicalPartiesArr[i].m_partyId )
			firstEmpty = i;
	}

	switch( rCommonH.opcode )
	{
		case CONF_MPL_CREATE_PARTY_REQ: {
			if( foundIndex < MAX_LOGICAL_PARTIES_PER_RTM ) {
				Bad(rCommonH,rMsgDescH,rPortDescH,ERR_CONF_MPL_PARTY_ALREADY_CONNECTED);
			} else {
				m_LogicalPartiesArr[firstEmpty].AddNew(rPortDescH.conf_id,rPortDescH.party_id);
				Ack(rCommonH,rMsgDescH,rPortDescH);
			}
			break;
		}
		case TB_MSG_OPEN_PORT_REQ: {
			if( foundIndex < MAX_LOGICAL_PARTIES_PER_RTM )
				Ack(rCommonH,rMsgDescH,rPortDescH);
			else
				Bad(rCommonH,rMsgDescH,rPortDescH,ERR_CONF_MPL_PARTY_NOT_CONNECTED);
			break;
		}
		case TB_MSG_CLOSE_PORT_REQ: {
			if( foundIndex < MAX_LOGICAL_PARTIES_PER_RTM )
				Ack(rCommonH,rMsgDescH,rPortDescH);
			else
				Bad(rCommonH,rMsgDescH,rPortDescH,ERR_CONF_MPL_PARTY_NOT_CONNECTED);
			break;
		}
		case CONF_MPL_DELETE_PARTY_REQ: {
			if( foundIndex < MAX_LOGICAL_PARTIES_PER_RTM ) {
				m_LogicalPartiesArr[foundIndex].CleanUp();
				Ack(rCommonH,rMsgDescH,rPortDescH);
			} else
				Bad(rCommonH,rMsgDescH,rPortDescH,ERR_CONF_MPL_PARTY_NOT_CONNECTED);
			break;
		}
		case TB_MSG_CONNECT_REQ: {
			if( foundIndex < MAX_LOGICAL_PARTIES_PER_RTM )
				Ack(rCommonH,rMsgDescH,rPortDescH);
			else
				Bad(rCommonH,rMsgDescH,rPortDescH,ERR_CONF_MPL_PARTY_NOT_CONNECTED);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// temp func for Talya
void CSimMfaCard::Ack( COMMON_HEADER_S tCommonH,MESSAGE_DESCRIPTION_HEADER_S tMsgDescH,
			PORT_DESCRIPTION_HEADER_S tPortDescH)
{
	ACK_IND_S  tAckStruct;
	memset(&tAckStruct,0,sizeof(ACK_IND_S));
	tAckStruct.ack_base.ack_opcode = tCommonH.opcode;
	tAckStruct.ack_base.status = STATUS_OK;

	tCommonH.opcode  = ACK_IND;
	tCommonH.src_id  = eMpl;
	tCommonH.dest_id = eMcms;
	tCommonH.payload_len = sizeof(COMMON_HEADER_S)
						+  sizeof(MESSAGE_DESCRIPTION_HEADER_S)
						+  sizeof(PORT_DESCRIPTION_HEADER_S)
						+  sizeof(ACK_IND_S);

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tCommonH),sizeof(COMMON_HEADER_S));
	pMsg->Put((BYTE*)(&tMsgDescH),sizeof(MESSAGE_DESCRIPTION_HEADER_S));
	pMsg->Put((BYTE*)(&tPortDescH),sizeof(PORT_DESCRIPTION_HEADER_S));
 	pMsg->Put((BYTE*)(&tAckStruct),sizeof(ACK_IND_S));

	SendToSocket(*pMsg);
	POBJDELETE(pMsg);
// 	SendToConfParty(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// temp func for Talya
void CSimMfaCard::Bad( COMMON_HEADER_S tCommonH,MESSAGE_DESCRIPTION_HEADER_S tMsgDescH,
			PORT_DESCRIPTION_HEADER_S tPortDescH,DWORD error)
{
	ACK_IND_S  tAckStruct;
	memset(&tAckStruct,0,sizeof(ACK_IND_S));
	tAckStruct.ack_base.ack_opcode = tCommonH.opcode;
	tAckStruct.ack_base.status = STATUS_FAIL;

	tCommonH.opcode  = ACK_IND;
	tCommonH.src_id  = eMpl;
	tCommonH.dest_id = eMcms;
	tCommonH.payload_len = sizeof(COMMON_HEADER_S)
						+  sizeof(MESSAGE_DESCRIPTION_HEADER_S)
						+  sizeof(PORT_DESCRIPTION_HEADER_S)
						+  sizeof(ACK_IND_S);

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tCommonH),sizeof(COMMON_HEADER_S));
	pMsg->Put((BYTE*)(&tMsgDescH),sizeof(MESSAGE_DESCRIPTION_HEADER_S));
	pMsg->Put((BYTE*)(&tPortDescH),sizeof(PORT_DESCRIPTION_HEADER_S));
 	pMsg->Put((BYTE*)(&tAckStruct),sizeof(ACK_IND_S));

	SendToSocket(*pMsg);
	POBJDELETE(pMsg);
// 	SendToConfParty(pMsg);
}
/////////////////////////////////////////////////////////////////////////////
void CSimMfaCard::OnSocketResumePause(CSegment* pMsg)
{
	m_state = SETUP;
    PTRACE(eLevelInfoNormal,"CSimMfaCard::OnSocketResumePause m_state = SETUP!!!!!!!!!!!!");
	if( CPObject::IsValidPObjectPtr(m_pSocketConnection) ) {
		m_pSocketConnection->Reconnect();
		}

}




/////////////////////////////////////////////////////////////////////////////
//
//   SimBarakCard - BARAK card simulation class implements BARAK card
//
/////////////////////////////////////////////////////////////////////////////
//
//	STATES:
//		IDLE      - default state immediately after creation
//		SETUP     - from Connect command until socket connection will be established
//		CONNECT   - socket connected, start configure switch card
//		RECOVERY (?)
//		RESET    (?)

PBEGIN_MESSAGE_MAP(CSimBarakCard)
	// Socket: connection established
	ONEVENT( SOCKET_CONNECTED,                    SETUP,     CSimBarakCard::OnSocketConnectedSetup)
	// BARAK logical: start up completed
 	ONEVENT( BARAK_CONNECTED,                     CONNECT,   CSimBarakCard::OnBarakEndConnectConnect)
 	// Barak logical: insert RTM-PSTN sub board
 	ONEVENT( BARAK_OR_BREEZE_INSERT_SUB_CARD,	  CONNECT,   CSimBarakCard::OnBarakInsertSubCardConnect)
	// Endpoints Sim: message received to this card
  	ONEVENT( SIM_API_ISDN_MSG,                    CONNECT,   CSimBarakCard::OnEndpointsSimIsdnMsgConnect)
	ONEVENT( SIM_API_AUDIO_MSG,                   CONNECT,   CSimBarakCard::OnEndpointsSimAudioMsgConnect)
	ONEVENT( SIM_API_MUX_MSG,                     CONNECT,   CSimBarakCard::OnEndpointsSimMuxMsgConnect)
	ONEVENT( SIM_API_MRM_MSG,                     CONNECT,   CSimBarakCard::OnEndpointsSimScpMsgConnect)
	ONEVENT( SET_UNIT_STATUS_FOR_KEEP_ALIVE_NEW,  CONNECT,   CSimBarakCard::OnSetUnitStatustSimBarakForKeepAliveIndConnect)
	ONEVENT( SIM_API_MEDIA_CARD_SET_PARAMS,		  CONNECT,   CSimBarakCard::OnSetMediaCardParams)
	ONEVENT( UPDATE_BURN_RATE,					  ANYCASE,	 CSimBarakCard::OnSetBurnRate)
	ONEVENT( SET_BURN_ACTIONS,					  ANYCASE,	 CSimBarakCard::OnSetBurnAction)


	//PCM
	ONEVENT( PCM_MSG_TO_MPLAPI,		              ANYCASE,   CSimBarakCard::OnPCMMsgToMplApi)
	// MRM messages
	ONEVENT( MM_INDICATIONS_MSG,				  CONNECT,   CSimBarakCard::OnMMIndicationsMsgConnect)
#ifndef __DISABLE_ICE__
	ONEVENT( MM_GS_ICE_MSG,					CONNECT,	    CSimBarakCard::OnMMIceMsgConnect)
#endif	//__DISABLE_ICE__
	
PEND_MESSAGE_MAP(CSimBarakCard,CSimSocketedCard);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void gideonSimBarakCardEntryPoint(void* appParam)
{
	CSimBarakCard*  pSimBarakCard = new CSimBarakCard;
	pSimBarakCard->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CSimBarakCard::CSimBarakCard()      // constructor
{
	m_pBarakLogics = NULL;
	m_pRtmLogics = NULL;
	m_isRtmAttached = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
CSimBarakCard::~CSimBarakCard()     // destructor
{
	POBJDELETE(m_pBarakLogics);
	POBJDELETE(m_pRtmLogics);
}


/////////////////////////////////////////////////////////////////////////////
void*  CSimBarakCard::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  CSimBarakCard::Create(CSegment& appParam)
{
	CSimSocketedCard::Create(appParam);

	WORD  isRtm = 0;
	appParam	>> isRtm;

	m_isRtmAttached = (isRtm) ? TRUE : FALSE;

	WORD  cardType = 0;
	appParam	>> cardType;
	m_RtmCardType = (eCardType)cardType;

	WORD  subBoardId = 1;

		// Barak create
	m_pBarakLogics = new CGideonSimBarakLogical(this,m_wBoardId,subBoardId++);
	PTRACE(eLevelInfoNormal,"CSimBarakCard::Create - Creating Barak module.");
	//LOGGER_TRACE(eLevelInfoNormal,"CSimBarakCard::Create - Creating Barak module.VASILYLOG");
		// rtm create only if available in config.xml
	if( m_isRtmAttached ) 
	{
		m_pRtmLogics = new CGideonSimRtmLogical(this,m_wBoardId,subBoardId++,m_RtmCardType);
		PTRACE(eLevelInfoNormal,"CSimBarakCard::Create - Creating RTM module.");
		//LOGGER_TRACE(eLevelInfoNormal,"CSimBarakCard::Create - Creating RTM module.VASILYLOG");
	}
}

/////////////////////////////////////////////////////////////////////////////
const char* CSimBarakCard::GetTaskName() const
{
	return "SimBarakCardTask";
}

				///vb TEMP start
#include "OpcodesMcmsNetQ931.h"
				///vb TEMP end
/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::DispatchMcmsMsg(CSegment* pMsg) const
{
	CSegment* pCopySeg = new CSegment(*pMsg);
	//  deserialize protocol
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pCopySeg);

	if( pMplProtocol->getCommonHeaderOpcode() == TERMINAL_COMMAND )
	{
		OnTerminalCommand(pMplProtocol);
	}
	else if ( pMplProtocol->getPhysicalInfoHeaderCounter() == 1 ) {
		// get copy of Physical Header
		PHYSICAL_INFO_HEADER_S  rPhysicalStruct;
		if ( pMplProtocol->GetPhysicalInfoHeaderCopy(0,&rPhysicalStruct) == STATUS_OK )
		{
			if( rPhysicalStruct.board_id == m_wBoardId )
			{
				///vb TEMP start
				if( pMplProtocol->getOpcode() >= NET_SETUP_REQ  &&
					pMplProtocol->getOpcode() <= NET_CONNECT_REQ )
				{
					m_pRtmLogics->ProcessMcmsMsg(pMsg);
				}
				else
				///vb TEMP end
				if ( rPhysicalStruct.sub_board_id == m_pBarakLogics->GetSubBoardId() )
				{
					m_pBarakLogics->ProcessMcmsMsg(pMsg);
				}
				else if ( rPhysicalStruct.sub_board_id == m_pRtmLogics->GetSubBoardId() )
				{
					m_pRtmLogics->ProcessMcmsMsg(pMsg);
				}
				else
				{
						PASSERT(100+rPhysicalStruct.sub_board_id);
				}
			}
			else
				PASSERT(100+rPhysicalStruct.board_id);
		}
		else
			PASSERT(100);
	}
	else
	{

	// disabled temporary until VNGSW-277 is fixed	
//		PASSERT(100+pMplProtocol->getPhysicalInfoHeaderCounter());
//		PASSERT(pMplProtocol->getCommonHeaderOpcode());
	}
	
	POBJDELETE(pMplProtocol);
	POBJDELETE(pCopySeg);
}


/////////////////////////////////////////////////////////////////////////////
// Socket: connection failed

/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::OnSocketFailedSetup(CSegment* pParam)
{
 	CSimSocketedCard::OnSocketFailedSetup(pParam);

//	Vasily::TEMP -   for Amir
//	int a = 5;
//
//	DWORD  opcode = 11;
//	CSegment*  pMsg = new CSegment;
//
//	*pMsg	<< m_wBoardId
//			<< (WORD)5  // unit id
//			<< opcode;
//
// 	::SendMessageToEndpointsSimApp(pMsg);
//	Vasily::TEMP -   for Amir

}


/////////////////////////////////////////////////////////////////////////////
// Socket: connection established

/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::OnSocketConnectedSetup(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimBarakCard::OnSocketConnectedSetup");

	m_state = CONNECT;
	m_pBarakLogics->Startup();
	if( m_pRtmLogics )
		m_pRtmLogics->Startup();
}

/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::OnBarakEndConnectConnect(CSegment* pMsg)
{
//	WORD boardId = 0, subBoardId = 0;
//	*pMsg	>> boardId
//			>> subBoardId;

	char szMessage[64];
	sprintf(szMessage,"BoardId <%d>",m_wBoardId);//(int)boardId,(int)subBoardId);
	PTRACE2(eLevelInfoNormal,"CSimBarakCard::OnBarakEndConnectConnect - BARAK MODULE finished start-up: ",szMessage);
	//LOGGER_TRACE2(eLevelInfoNormal,"CSimBarakCard::OnBarakEndConnectConnect - VASILYLOG - BARAK MODULE finished start-up: ",szMessage);
}

/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::OnBarakInsertSubCardConnect(CSegment* pMsg)
{
	BYTE  boardId;
	BYTE  subBoardId;

	*pMsg >> boardId
	      >> subBoardId;

	TRACEINTO << "CSimBarakCard::OnBarakInsertSubCardConnect boardId: " << (int)boardId << " subBoardId: " << (int)subBoardId;

	m_isRtmAttached = true;

	if (m_pRtmLogics == NULL)
		m_pRtmLogics = new CGideonSimRtmLogical(this, boardId, subBoardId);

	//send to RTM Logical Module
	m_pRtmLogics->Startup();
}

/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::OnSetBurnRate(CSegment* pMsg)
{
	DWORD  rate;
		DWORD burnType;
		*pMsg >> rate
			  >>burnType;

		TRACEINTO << "CSimBarakCard::OnSetBurnRate rate: " << (int)rate;

		if(burnType == eVersionBurnType)
			m_pBarakLogics->SetVersionBurnRate(rate);
		else if(burnType == eIpmcBurnType)
			m_pBarakLogics->SetIPMCBurnRate(rate);

}

/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::OnSetBurnAction(CSegment* pMsg)
{
	DWORD burnActionType;
	DWORD burnType;

	*pMsg >>burnActionType;
	*pMsg >>burnType;

	TRACEINTO << "CSimBarakCard::OnSetBurnAction ";

	m_pBarakLogics->SetBurnAction((eBurnActionTypes)burnActionType,(eBurnTypes)burnType);


}
/////////////////////////////////////////////////////////////////////////////
// Endpoints Sim: PSTN message received

/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::OnEndpointsSimIsdnMsgConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimBarakCard::OnEndpointsSimIsdnMsgConnect - Received message from EP.");

	DWORD opcode=0;
	WORD  boardId = 0xFFFF, subBoardId = 0xFFFF;

	CSegment* pCopySeg  = new CSegment(*pMsg);

	*pCopySeg	>> opcode
				>> boardId
				>> subBoardId;

	POBJDELETE(pCopySeg);

	// NET_SETUP_IND with unknown boardId / subBoardId
	if( NET_SETUP_IND == opcode && 0xFFFF == boardId && 0xFFFF == subBoardId )
	{
		boardId    = m_wBoardId;
		subBoardId = m_pRtmLogics->GetSubBoardId();
	}

	if( boardId != m_wBoardId ) {
		PASSERT_AND_RETURN(1000+boardId);
	}

	if( m_pRtmLogics  &&  subBoardId == m_pRtmLogics->GetSubBoardId() )
		m_pRtmLogics->ProcessEndpointsSimMsg(pMsg);

/*	for( BYTE i=0; i<MAX_MFA_LOGICS_IN_LITE_CARD; i++ ) {
		if( m_pRtmLogicsArr[i] && subBoardId == m_pRtmLogicsArr[i]->GetSubBoardId() )
			m_pRtmLogicsArr[i]->ProcessEndpointsSimMsg(pMsg);
	}*/
}

/////////////////////////////////////////////////////////////////////////////
// Endpoints Sim: Audio message received

/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::OnSetUnitStatustSimBarakForKeepAliveIndConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimBarakCard::OnSetUnitStatustSimBarakForKeepAliveIndConnect");
	DWORD opcode=0;
	WORD  boardId = 0xFFFF, UnitId = 0xFFFF, UnitStatus = 0xFFFF, dummy = 0, UnitStatus3 = 0xFFFF, UnitStatus4 = 0xFFFF;

	CSegment* pCopySeg  = new CSegment(*pMsg);
	*pCopySeg	>> opcode
				>> boardId
				>> dummy
				>> UnitId
				>> dummy
				>> UnitStatus
				>> dummy;
	POBJDELETE(pCopySeg);

// print to trace
	CLargeString kaStr = "CSimBarakCard::OnSetUnitStatustSimBarakForKeepAliveIndConnect get the XML Results:\n==================\n";
			kaStr << " \n opcode     : " << opcode  ;
			kaStr << " \n boardId    : " << boardId  ;
			kaStr << " \n UnitId     : " << UnitId ;
			kaStr << " \n UnitStatus : " << UnitStatus ;


	TRACEINTO << kaStr.GetString();

	m_pBarakLogics->SetUnitStatustSimBarakForKeepAliveInd(UnitId,UnitStatus);

}


/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::OnSetMediaCardParams(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimBarakCard::OnSetMediaCardParams ");

	DWORD opcode=0;
	*pMsg >> opcode;

	switch( opcode )
	{
		case SIM_API_ISDN_SET_TIMERS:
		{
			DWORD bndRmtLclAlignment = 0;
			*pMsg >> bndRmtLclAlignment;
			if (m_pBarakLogics)
				m_pBarakLogics->SetBndLclAlignmentTimer( bndRmtLclAlignment );
			break;
		}

		case SIM_API_RTM_ENABLE_DISABLE_PORTS:
		{
			DWORD span=0;
			DWORD firstPort=0;
			DWORD numPorts=0;
			DWORD action=0;

			*pMsg >> span >> firstPort >> numPorts >> action;

			if (m_pRtmLogics)
				m_pRtmLogics->SetRtmEnableDisablePorts( span, firstPort, numPorts, action );
			break;
		}

		default:
			return;
	}

}


/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::OnEndpointsSimAudioMsgConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimBarakCard::OnEndpointsSimAudioMsgConnect - Received message from EP.");

	DWORD opcode=0;
	WORD  boardId = 0xFFFF, subBoardId = 0xFFFF;

	CSegment* pCopySeg  = new CSegment(*pMsg);

	*pCopySeg	>> opcode
				>> boardId
				>> subBoardId;

	POBJDELETE(pCopySeg);

	if( boardId != m_wBoardId ) {
		PASSERT_AND_RETURN(1000+boardId);
	}

	m_pBarakLogics->ProcessEndpointsSimMsg(pMsg);
/*	for( BYTE i=0; i<MAX_MFA_LOGICS_IN_LITE_CARD; i++ ) {
		if( m_pMfaLogicsArr[i] && subBoardId == m_pMfaLogicsArr[i]->GetSubBoardId() )
			m_pMfaLogicsArr[i]->ProcessEndpointsSimMsg(pMsg);
	}*/
}


/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::OnEndpointsSimMuxMsgConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimBarakCard::OnEndpointsSimMuxMsgConnect - Received message from EP.");

	DWORD opcode=0;
	WORD  boardId = 0xFFFF, subBoardId = 0xFFFF;

	CSegment* pCopySeg  = new CSegment(*pMsg);

	*pCopySeg	>> opcode
				>> boardId
				>> subBoardId;

	POBJDELETE(pCopySeg);

	if( boardId != m_wBoardId ) {
		PASSERT_AND_RETURN(1000+boardId);
	}

	m_pBarakLogics->ProcessEndpointsSimMsg(pMsg);
/*	for( BYTE i=0; i<MAX_MFA_LOGICS_IN_LITE_CARD; i++ ) {
		if( m_pMfaLogicsArr[i] && subBoardId == m_pMfaLogicsArr[i]->GetSubBoardId() )
			m_pMfaLogicsArr[i]->ProcessEndpointsSimMsg(pMsg);
	}*/
}

/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::OnEndpointsSimScpMsgConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimBarakCard::OnEndpointsSimScpMsgConnect - Received message from EP.");

	DWORD opcode=0;
	WORD  boardId = 0xFFFF, subBoardId = 0xFFFF;

	CSegment* pCopySeg  = new CSegment(*pMsg);

	*pCopySeg	>> opcode
				>> boardId
				>> subBoardId;

	POBJDELETE(pCopySeg);

	if( boardId != m_wBoardId ) {
		PASSERT_AND_RETURN(1000+boardId);
	}

	m_pBarakLogics->ProcessEndpointsSimMsg(pMsg);
}

#ifndef __DISABLE_ICE__
void CSimBarakCard::OnMMIceMsgConnect(CSegment* pMsg)
{
	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator) {
		PTRACE(eLevelInfoNormal,"CSimBarakCard::OnMMIceMsgConnect - ERROR: Received message from MediaMngr process??? - system is not CG!!");
		return;
	}

	PTRACE(eLevelInfoNormal,"CSimBarakCard::OnMMIceMsgConnect - Received message from MM");

	SendToMplApi(*pMsg);//forward it to CM
}
#endif	//__DISABLE_ICE__

/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::OnPCMMsgToMplApi(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimSocketedCard::OnPCMMsgToMplApi");
	CSegment* pCopySeg = new CSegment(*pMsg);
	SendToMplApi(*pCopySeg);
	POBJDELETE(pCopySeg);
}

/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::OnMMIndicationsMsgConnect(CSegment* pMsg)
{
    if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
    	PTRACE(eLevelInfoNormal,"CSimBarakCard::OnMMIndicationsMsgConnect - ERROR: Received message from MediaMngr process??? - system is not CG!!");
        return;
    }

    PTRACE(eLevelInfoNormal,"CSimBarakCard::OnMMIndicationsMsgConnect - Received message from MM");

	DWORD opcode  = 0;
	DWORD boardId = (DWORD)-1;

	CSegment* pCopySeg  = new CSegment(*pMsg);
	*pCopySeg	>> opcode
				>> boardId;
	POBJDELETE(pCopySeg);

	if( boardId != (DWORD)m_wBoardId ) {
		PASSERT_AND_RETURN(1000+boardId);
	}

	m_pBarakLogics->ProcessMMIndicationMsg(pMsg);
}



/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::PartyReaction( CMplMcmsProtocol* pMplProt )
{
	COMMON_HEADER_S					rCommonH;
	pMplProt->GetCommonHeaderCopy(&rCommonH);

	MESSAGE_DESCRIPTION_HEADER_S	rMsgDescH;
	pMplProt->GetMsgDescHeaderCopy(&rMsgDescH);

	PORT_DESCRIPTION_HEADER_S		rPortDescH;
	pMplProt->GetPortDescHeaderCopy(&rPortDescH);

	DWORD foundIndex = MAX_LOGICAL_PARTIES_PER_RTM;
	DWORD firstEmpty = MAX_LOGICAL_PARTIES_PER_RTM;

	for( DWORD  i=0; i<MAX_LOGICAL_PARTIES_PER_RTM; i++ ) {
		if( rPortDescH.conf_id == m_LogicalPartiesArr[i].m_confId &&
				rPortDescH.party_id == m_LogicalPartiesArr[i].m_partyId )
			foundIndex = i;
		if( MAX_LOGICAL_PARTIES_PER_RTM == firstEmpty &&
				0 == m_LogicalPartiesArr[i].m_confId &&
				0 == m_LogicalPartiesArr[i].m_partyId )
			firstEmpty = i;
	}

	switch( rCommonH.opcode )
	{
		case CONF_MPL_CREATE_PARTY_REQ: {
			if( foundIndex < MAX_LOGICAL_PARTIES_PER_RTM ) {
				Bad(rCommonH,rMsgDescH,rPortDescH,ERR_CONF_MPL_PARTY_ALREADY_CONNECTED);
			} else {
				m_LogicalPartiesArr[firstEmpty].AddNew(rPortDescH.conf_id,rPortDescH.party_id);
				Ack(rCommonH,rMsgDescH,rPortDescH);
			}
			break;
		}
		case TB_MSG_OPEN_PORT_REQ: {
			if( foundIndex < MAX_LOGICAL_PARTIES_PER_RTM )
				Ack(rCommonH,rMsgDescH,rPortDescH);
			else
				Bad(rCommonH,rMsgDescH,rPortDescH,ERR_CONF_MPL_PARTY_NOT_CONNECTED);
			break;
		}
		case TB_MSG_CLOSE_PORT_REQ: {
			if( foundIndex < MAX_LOGICAL_PARTIES_PER_RTM )
				Ack(rCommonH,rMsgDescH,rPortDescH);
			else
				Bad(rCommonH,rMsgDescH,rPortDescH,ERR_CONF_MPL_PARTY_NOT_CONNECTED);
			break;
		}
		case CONF_MPL_DELETE_PARTY_REQ: {
			if( foundIndex < MAX_LOGICAL_PARTIES_PER_RTM ) {
				m_LogicalPartiesArr[foundIndex].CleanUp();
				Ack(rCommonH,rMsgDescH,rPortDescH);
			} else
				Bad(rCommonH,rMsgDescH,rPortDescH,ERR_CONF_MPL_PARTY_NOT_CONNECTED);
			break;
		}
		case TB_MSG_CONNECT_REQ: {
			if( foundIndex < MAX_LOGICAL_PARTIES_PER_RTM )
				Ack(rCommonH,rMsgDescH,rPortDescH);
			else
				Bad(rCommonH,rMsgDescH,rPortDescH,ERR_CONF_MPL_PARTY_NOT_CONNECTED);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// temp func for Talya
void CSimBarakCard::Ack( COMMON_HEADER_S tCommonH,MESSAGE_DESCRIPTION_HEADER_S tMsgDescH,
			PORT_DESCRIPTION_HEADER_S tPortDescH)
{
	ACK_IND_S  tAckStruct;
	memset(&tAckStruct,0,sizeof(ACK_IND_S));
	tAckStruct.ack_base.ack_opcode = tCommonH.opcode;
	tAckStruct.ack_base.status = STATUS_OK;

	tCommonH.opcode  = ACK_IND;
	tCommonH.src_id  = eMpl;
	tCommonH.dest_id = eMcms;
	tCommonH.payload_len = sizeof(COMMON_HEADER_S)
						+  sizeof(MESSAGE_DESCRIPTION_HEADER_S)
						+  sizeof(PORT_DESCRIPTION_HEADER_S)
						+  sizeof(ACK_IND_S);

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tCommonH),sizeof(COMMON_HEADER_S));
	pMsg->Put((BYTE*)(&tMsgDescH),sizeof(MESSAGE_DESCRIPTION_HEADER_S));
	pMsg->Put((BYTE*)(&tPortDescH),sizeof(PORT_DESCRIPTION_HEADER_S));
 	pMsg->Put((BYTE*)(&tAckStruct),sizeof(ACK_IND_S));

	SendToSocket(*pMsg);
	POBJDELETE(pMsg);
// 	SendToConfParty(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// temp func for Talya
void CSimBarakCard::Bad( COMMON_HEADER_S tCommonH,MESSAGE_DESCRIPTION_HEADER_S tMsgDescH,
			PORT_DESCRIPTION_HEADER_S tPortDescH,DWORD error)
{
	ACK_IND_S  tAckStruct;
	memset(&tAckStruct,0,sizeof(ACK_IND_S));
	tAckStruct.ack_base.ack_opcode = tCommonH.opcode;
	tAckStruct.ack_base.status = STATUS_FAIL;

	tCommonH.opcode  = ACK_IND;
	tCommonH.src_id  = eMpl;
	tCommonH.dest_id = eMcms;
	tCommonH.payload_len = sizeof(COMMON_HEADER_S)
						+  sizeof(MESSAGE_DESCRIPTION_HEADER_S)
						+  sizeof(PORT_DESCRIPTION_HEADER_S)
						+  sizeof(ACK_IND_S);

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(&tCommonH),sizeof(COMMON_HEADER_S));
	pMsg->Put((BYTE*)(&tMsgDescH),sizeof(MESSAGE_DESCRIPTION_HEADER_S));
	pMsg->Put((BYTE*)(&tPortDescH),sizeof(PORT_DESCRIPTION_HEADER_S));
 	pMsg->Put((BYTE*)(&tAckStruct),sizeof(ACK_IND_S));

	SendToSocket(*pMsg);
	POBJDELETE(pMsg);
// 	SendToConfParty(pMsg);
}
/////////////////////////////////////////////////////////////////////////////
void CSimBarakCard::OnSocketResumePause(CSegment* pMsg)
{
	m_state = SETUP;
    PTRACE(eLevelInfoNormal,"CSimBarakCard::OnSocketResumePause m_state = SETUP!!!!!!!!!!!!");
	if( CPObject::IsValidPObjectPtr(m_pSocketConnection) ) {
		m_pSocketConnection->Reconnect();
		}

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


void gideonSimBreezeCardEntryPoint(void* appParam)
{
	CSimBreezeCard*  pSimBreezeCard = new CSimBreezeCard;
	pSimBreezeCard->Create(*(CSegment*)appParam);
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CLogicalParty::CleanUp()
{
	m_partyId = 0;
	m_confId  = 0;
}


/////////////////////////////////////////////////////////////////////////////
//
//   SimGideonLiteCard - Gideon Lite configuration, has SWITCH module and 2 MFA modules
//
/////////////////////////////////////////////////////////////////////////////
//
//	STATES:
//		IDLE      - default state immediately after creation
//		SETUP     - from Connect command until socket connection will be established
//		CONNECT   - card started up
//		RECOVERY (?)
//		RESET    (?)

PBEGIN_MESSAGE_MAP(CSimGideonLiteCard)
	// Socket: connection established
	ONEVENT( SOCKET_CONNECTED,   SETUP,     CSimGideonLiteCard::OnSocketConnectedSetup)
	// Switch: connected
 	ONEVENT( SWITCH_CONNECTED,   CONNECT,   CSimGideonLiteCard::OnSwitchEndConnectConnect)
	// MFA: connected
 	ONEVENT( MFA_CONNECTED,      CONNECT,   CSimGideonLiteCard::OnMfaEndConnectConnect)
	// Endpoints Sim: message received to this card
	ONEVENT( SIM_API_ISDN_MSG,   CONNECT,   CSimGideonLiteCard::OnEndpointsSimIsdnMsgConnect)
	ONEVENT( SIM_API_AUDIO_MSG,  CONNECT,   CSimGideonLiteCard::OnEndpointsSimAudioMsgConnect)
	ONEVENT( SIM_API_MUX_MSG,  	 CONNECT,   CSimGideonLiteCard::OnEndpointsSimMuxMsgConnect)
PEND_MESSAGE_MAP(CSimGideonLiteCard,CSimSwitchCard);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void gideonSimGideonLiteCardEntryPoint(void* appParam)
{
	CSimGideonLiteCard*  pSimLiteCard = new CSimGideonLiteCard;
	pSimLiteCard->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CSimGideonLiteCard::CSimGideonLiteCard()      // constructor
{
	m_pSwitchLogics = NULL;

	for( int i=0; i<MAX_MFA_LOGICS_IN_LITE_CARD; i++ ) {
		m_pMfaLogicsArr[i] = NULL;
		m_pRtmLogicsArr[i] = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
CSimGideonLiteCard::~CSimGideonLiteCard()     // destructor
{
	POBJDELETE(m_pSwitchLogics);

	for( int i=0; i<MAX_MFA_LOGICS_IN_LITE_CARD; i++ ) {
		POBJDELETE(m_pMfaLogicsArr[i]);
		POBJDELETE(m_pRtmLogicsArr[i]);
	}
}


/////////////////////////////////////////////////////////////////////////////
void*  CSimGideonLiteCard::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  CSimGideonLiteCard::Create(CSegment& appParam)
{
	PTRACE(eLevelInfoNormal,"CSimGideonLiteCard::Create - Creating CARD.");

	CSimSwitchCard::Create(appParam);

	WORD  isRtm = 0;
	appParam	>> isRtm;

	WORD  subBoardId = 0;

	m_pSwitchLogics = new CGideonSimSwitchLogical(this,m_wBoardId,subBoardId++);

	for( BYTE i=0; i<MAX_MFA_LOGICS_IN_LITE_CARD; i++ ) {
		// mfa create
		m_pMfaLogicsArr[i] = new CGideonSimMfaLogical(this,m_wBoardId,subBoardId++);
		PTRACE(eLevelInfoNormal,"CSimGideonLiteCard::Create - Creating MFA module.");
		// rtm create only if available in config.xml
		if( isRtm ) {
			m_pRtmLogicsArr[i] = new CGideonSimRtmLogical(this,m_wBoardId,subBoardId++);
			PTRACE(eLevelInfoNormal,"CSimGideonLiteCard::Create - Creating RTM module.");
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
const char* CSimGideonLiteCard::GetTaskName() const
{
	return "CSimGideonLiteCard";
}

/////////////////////////////////////////////////////////////////////////////
void CSimGideonLiteCard::DispatchMcmsMsg(CSegment* pMsg) const
{
	CSegment* pCopySeg = new CSegment(*pMsg);
	//  deserialize protocol
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pCopySeg);

	if ( pMplProtocol->getPhysicalInfoHeaderCounter() == 1 ) {
		// get copy of Physical Header
		PHYSICAL_INFO_HEADER_S  rPhysicalStruct;
		if ( pMplProtocol->GetPhysicalInfoHeaderCopy(0,&rPhysicalStruct) == STATUS_OK ) {
			if( rPhysicalStruct.board_id == m_wBoardId ) {
				if ( rPhysicalStruct.sub_board_id == m_pSwitchLogics->GetSubBoardId() ) {
					m_pSwitchLogics->ProcessMcmsMsg(pMsg);
				} else {
					int found = FALSE;
					for ( int i=0; i<MAX_MFA_LOGICS_IN_LITE_CARD; i++ ) {
						if ( rPhysicalStruct.sub_board_id == m_pMfaLogicsArr[i]->GetSubBoardId() ) {
							m_pMfaLogicsArr[i]->ProcessMcmsMsg(pMsg);
							found = TRUE;
							break;
						}
						if ( m_pRtmLogicsArr[i] && rPhysicalStruct.sub_board_id == m_pRtmLogicsArr[i]->GetSubBoardId() ) {
							m_pRtmLogicsArr[i]->ProcessMcmsMsg(pMsg);
							found = TRUE;
							break;
						}
					}
					if( !found )
						PASSERT(100+rPhysicalStruct.sub_board_id);
				}
			} else
				PASSERT(100+rPhysicalStruct.board_id);
		}
	} else
		PASSERT(100+pMplProtocol->getPhysicalInfoHeaderCounter());

	POBJDELETE(pMplProtocol);
	POBJDELETE(pCopySeg);
}


/////////////////////////////////////////////////////////////////////////////
void CSimGideonLiteCard::OnSwitchEndConnectConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimGideonLiteCard::OnSwitchEndConnectConnect - SWITCH MODULE finished start-up.");

	for( int i=0; i<MAX_MFA_LOGICS_IN_LITE_CARD; i++ ) {
		m_pMfaLogicsArr[i]->Startup();
		if( m_pRtmLogicsArr[i] )
			m_pRtmLogicsArr[i]->Startup();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSimGideonLiteCard::OnMfaEndConnectConnect(CSegment* pMsg)
{
	WORD boardId = 0, subBoardId = 0;
	*pMsg	>> boardId
			>> subBoardId;

	char szMessage[64];
	sprintf(szMessage,"BoardId <%d>, SubBoardId <%d>",(int)boardId,(int)subBoardId);
	PTRACE2(eLevelInfoNormal,"CSimGideonLiteCard::OnMfaEndConnectConnect - MFA MODULE finished start-up: ",szMessage);
}

/////////////////////////////////////////////////////////////////////////////
// Socket: connection established

/////////////////////////////////////////////////////////////////////////////
void CSimGideonLiteCard::OnSocketConnectedSetup(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimGideonLiteCard::OnSocketConnectedSetup - CARD's socket connected.");

	m_state = CONNECT;

	m_pSwitchLogics->Startup();
}

/////////////////////////////////////////////////////////////////////////////
// Endpoints Sim: PSTN message received

/////////////////////////////////////////////////////////////////////////////
void CSimGideonLiteCard::OnEndpointsSimIsdnMsgConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimGideonLiteCard::OnEndpointsSimIsdnMsgConnect - Received message from EP.");

	DWORD opcode = 0;
	WORD  boardId = 0, subBoardId = 0;

	CSegment* pCopySeg  = new CSegment(*pMsg);

	*pCopySeg	>> opcode
				>> boardId
				>> subBoardId;

	POBJDELETE(pCopySeg);

	if( boardId != m_wBoardId ) {
		PASSERT_AND_RETURN(1000+boardId);
	}

	for( BYTE i=0; i<MAX_MFA_LOGICS_IN_LITE_CARD; i++ ) {
		if( m_pRtmLogicsArr[i] && subBoardId == m_pRtmLogicsArr[i]->GetSubBoardId() )
			m_pRtmLogicsArr[i]->ProcessEndpointsSimMsg(pMsg);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Endpoints Sim: Audio message received

/////////////////////////////////////////////////////////////////////////////
void CSimGideonLiteCard::OnEndpointsSimAudioMsgConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimGideonLiteCard::OnEndpointsSimAudioMsgConnect - Received message from EP.");

	DWORD opcode = 0;
	WORD  boardId = 0, subBoardId = 0;

	CSegment* pCopySeg  = new CSegment(*pMsg);

	*pCopySeg	>> opcode
				>> boardId
				>> subBoardId;

	POBJDELETE(pCopySeg);

	if( boardId != m_wBoardId ) {
		PASSERT_AND_RETURN(1000+boardId);
	}

	for( BYTE i=0; i<MAX_MFA_LOGICS_IN_LITE_CARD; i++ ) {
		if( m_pMfaLogicsArr[i] && subBoardId == m_pMfaLogicsArr[i]->GetSubBoardId() )
			m_pMfaLogicsArr[i]->ProcessEndpointsSimMsg(pMsg);
	}
}


/////////////////////////////////////////////////////////////////////////////
void CSimGideonLiteCard::OnEndpointsSimMuxMsgConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimGideonLiteCard::OnEndpointsSimMuxMsgConnect - Received message from EP.");

	DWORD opcode = 0;
	WORD  boardId = 0, subBoardId = 0;

	CSegment* pCopySeg  = new CSegment(*pMsg);

	*pCopySeg	>> opcode
				>> boardId
				>> subBoardId;

	POBJDELETE(pCopySeg);

	if( boardId != m_wBoardId ) {
		PASSERT_AND_RETURN(1000+boardId);
	}

	for( BYTE i=0; i<MAX_MFA_LOGICS_IN_LITE_CARD; i++ ) {
		if( m_pMfaLogicsArr[i] && subBoardId == m_pMfaLogicsArr[i]->GetSubBoardId() )
			m_pMfaLogicsArr[i]->ProcessEndpointsSimMsg(pMsg);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSimSwitchCard::OnUserLdapLogin(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CSimMfaCard::OnUserLoginLdap");

	string userName;
	string userPassword;

	*pMsg >> userName
			>> userPassword;

	CLargeString kaStr = "CSimMfaCard::OnUserLdapLogin:\n==================\n";
			kaStr << " \n userName     : " << userName  ;
			kaStr << " \n userPassword    : " << userPassword  ;
	TRACEINTO << kaStr.GetString();


	m_pSwitchLogics->SendUserLdapLogin(userName,userPassword);

}


