//+========================================================================+
//                  EpSimEndpointsTask.cpp                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpSimEndpointsTask.cpp                                      |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+


#include "Macros.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"

#include "TaskApi.h"
#include "MplMcmsProtocol.h"

#include "SimApi.h"
#include "EndpointsSim.h"
#include "EndpointsGuiApi.h"
#include "EpSimEndpointsList.h"
#include "EpSimCapSetsList.h"
#include "EpSimH323BehaviorList.h"
#include "EpSimEndpointsList.h"

#include "EpSimEndpointsTask.h"

#include "Bonding.h"
#include "OpcodesMcmsBonding.h"
#include "OpcodesMcmsMux.h"
#include "OpcodesMcmsInternal.h"

/////////////////////////////////////////////////////////////////////////////
//
//   STATES:
//
//const WORD  IDLE        = 0;        // default state -  defined in base class
const WORD  SETUP         = 1;
const WORD  CONNECT       = 2;
//const WORD  ANYCASE     = 0xFFFF;   // any other state -  defined in base class


/////////////////////////////////////////////////////////////////////////////
//
//   EXTERNALS:
//



/////////////////////////////////////////////////////////////////////////////
//
//   SimEndpointsTask - Unified endpoints Task
//
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSimEndpointsTask)
//	ONEVENT( UPDATE_CS_MBX,		IDLE,	CSimEndpointsTask::OnReceiveMbxCsIdle)
	ONEVENT( SIM_API_AUDIO_MSG,	IDLE,   CSimEndpointsTask::OnGideonArtMsgAll)
	ONEVENT( SIM_API_MUX_MSG,	IDLE,   CSimEndpointsTask::OnGideonArtMsgAll)
	ONEVENT( SIM_API_ISDN_MSG,	IDLE,   CSimEndpointsTask::OnGideonIsdnMsgAll)
	ONEVENT( SIM_API_MRM_MSG,	IDLE,   CSimEndpointsTask::OnGideonMRMMsgAll)
	ONEVENT( SIM_CS_RCV_MSG,	IDLE,   CSimEndpointsTask::OnCSMsgAll)
	ONEVENT( BATCH_COMMAND,		IDLE,   CSimEndpointsTask::OnBatchCommandAll)
	ONEVENT( GUI_TO_ENDPOINTS,	IDLE,   CSimEndpointsTask::OnGUIMsgAll)
PEND_MESSAGE_MAP(CSimEndpointsTask,CTaskApp);


/////////////////////////////////////////////////////////////////////////////
//  task creation function
void epSimEndpointsTaskEntryPoint(void* appParam)
{
	CSimEndpointsTask*  pEndpointsTask = new CSimEndpointsTask;
	pEndpointsTask->Create(*(CSegment*)appParam);
}


/////////////////////////////////////////////////////////////////////////////
CSimEndpointsTask::CSimEndpointsTask()      // constructor
{
//	m_pCSApi		= new CTaskApi;
	m_pCapList		= new CCapSetsList;
	m_pBehaviorList	= new CH323BehaviorList;
	m_pEpList 		= new CEndpointsList( NULL /*m_pCSApi*/,m_pCapList,m_pBehaviorList );
}

/////////////////////////////////////////////////////////////////////////////
CSimEndpointsTask::~CSimEndpointsTask()     // destructor
{
	POBJDELETE (m_pCapList);
	POBJDELETE (m_pBehaviorList);
//	POBJDELETE (m_pCSApi);
	POBJDELETE (m_pEpList);
}

/////////////////////////////////////////////////////////////////////////////
void* CSimEndpointsTask::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CSimEndpointsTask::Create(CSegment& appParam)
{
	CTaskApp::Create(appParam);
}

/////////////////////////////////////////////////////////////////////////////
void CSimEndpointsTask::SelfKill() //Override
{
    DeleteAllTimers();
	CTaskApp::SelfKill();
}


/////////////////////////////////////////////////////////////////////////////
//void CSimEndpointsTask::OnReceiveMbxCsIdle(CSegment* pParam)
//{
//	PTRACE(eLevelInfoNormal,"CSimEndpointsTask::OnReceiveMbxCsIdle - MBX of CS saved.");
//
//	COsQueue RcvMbxCS;
//	RcvMbxCS.DeSerialize( *pParam );
//	m_pCSApi->CreateOnlyApi(RcvMbxCS);
//}

/////////////////////////////////////////////////////////////////////////////
// Endpoints Manager: received message from Gideon Sim process
/////////////////////////////////////////////////////////////////////////////
void CSimEndpointsTask::OnGideonArtMsgAll(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSimEndpointsTask::OnGideonArtMsgAll");

	DWORD opcode = 0xFFFFFFFF;

	*pParam >> opcode;

	switch( opcode )
	{
		case TB_MSG_OPEN_PORT_REQ:
		case TB_MSG_CLOSE_PORT_REQ:
		case IVR_PLAY_MESSAGE_REQ:
		case IVR_RECORD_ROLL_CALL_REQ:
		case IVR_STOP_PLAY_MESSAGE_REQ:
		case IVR_PLAY_MUSIC_REQ:
		case IVR_STOP_PLAY_MUSIC_REQ:
		case MOVE_RSRC_REQ:
		case BND_CONNECTION_INIT:
		case SEND_H_230:
		case IVR_STOP_RECORD_ROLL_CALL_REQ:
		case SET_ECS:
		{
			m_pEpList->HandleArtGideonMsg(opcode,pParam);
			break;
		}
		default:
		{
			DBGPASSERT(1000+opcode);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Endpoints Manager: received message from Gideon Sim process
/////////////////////////////////////////////////////////////////////////////
void CSimEndpointsTask::OnGideonIsdnMsgAll(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSimEndpointsTask::OnGideonIsdnMsgAll");

	DWORD opcode = 0xFFFFFFFF;

	*pParam >> opcode;

	m_pEpList->HandleRtmGideonMsg(opcode, pParam);

//	switch( opcode )
//	{
//		case NET_SETUP_REQ:
//		case NET_CLEAR_REQ:
//		case NET_ALERT_REQ:
//		case NET_CONNECT_REQ:
//		{

//			break;
//		}
//		default:
//		{
//			DBGPASSERT(1000+opcode);
//			break;
//		}
//	}
}

/////////////////////////////////////////////////////////////////////////////
// Endpoints Manager: received message from Gideon Sim process
/////////////////////////////////////////////////////////////////////////////
void CSimEndpointsTask::OnGideonMRMMsgAll(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSimEndpointsTask::OnGideonMRMMsgAll");

	DWORD opcode = 0xFFFFFFFF;

	*pParam >> opcode;

	m_pEpList->HandleMRMGideonMsg(opcode, pParam);

}

/////////////////////////////////////////////////////////////////////////////
// Endpoints Manager: received message from CS-API
/////////////////////////////////////////////////////////////////////////////
void CSimEndpointsTask::OnCSMsgAll(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSimEndpointsTask::OnCSMsgAll");

	//  gets the protocol
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pParam, CS_API_TYPE);

	// Handle the event in the list
	m_pEpList->HandleCsEvent( pMplProtocol );

	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
// Endpoints Manager: received message from batch script
/////////////////////////////////////////////////////////////////////////////
void CSimEndpointsTask::OnBatchCommandAll(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSimEndpointsTask::OnBatchCommandAll");

	DWORD opcode = 0;
	*pParam >> opcode;

	switch( opcode)
	{
		case BATCH_ADD_PARTY:
		case BATCH_DEL_PARTY:
		case BATCH_CONNECT_PARTY:
		case BATCH_DISCONNECT_PARTY:
		case BATCH_DTMF_PARTY:
		case BATCH_ACTIVE_SPEAKER:
		case BATCH_AUDIO_SPEAKER:
		case BATCH_MUTE:
		case BATCH_UNMUTE:
		case BATCH_FECC_TOKEN_REQUEST:
		case BATCH_FECC_TOKEN_RELEASE:
		case BATCH_CHANGEMODE_PARTY:
		case BATCH_H239_TOKEN_REQUEST:
		case BATCH_H239_TOKEN_RELEASE:
		case BATCH_CHANNELS_UPDATE:
		case BATCH_LPR_MODE_CHANGE_REQUEST:
		case BATCH_FECC_KEY_REQUEST:
		case BATCH_SCP_STREAMS_REQUEST:
		{
			m_pEpList->HandleBatchEvent(opcode, pParam);
			break;
		}
		case BATCH_SHOW_ALL_PARTIES_REQ:
		{
			m_pEpList->GetFullEpListToTerminal();
			break;
		}
		case BATCH_ADD_CAPSET:
		case BATCH_DEL_CAPSET:
		{
			m_pCapList->HandleBatchEvent(opcode, pParam);//AddCapFromGui(pParam);
			break;
		}
		default:
		{
			DBGPASSERT(1000+opcode);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Endpoints Manager: received message from GUI
/////////////////////////////////////////////////////////////////////////////
void CSimEndpointsTask::OnGUIMsgAll(CSegment* pParam)
{
	DWORD opcode = 0;
	*pParam >> opcode;

	switch( opcode ) {
		case ADD_CAP_REQ:
		{
			m_pCapList->AddCapFromGui( pParam );
			break;
		}
		case DELETE_CAP_REQ:
		{
			m_pCapList->DeleteCapFromGui( pParam );
			break;
		}
		case UPDATE_CAP_REQ:
		{
			m_pCapList->UpdateCapFromGui( pParam );
			break;
		}
		case GET_CAP_LIST_REQ:
		{
			m_pCapList->GetFullCapListToGui(pParam);
			break;
		}
		case GET_ONE_CAP_REQ:
		{
		//	m_capList->HandleCapEvent( opcode, pParam );
			break;
		}

		case ADD_BEHAVIOR_REQ:
		{
			m_pBehaviorList->AddNewBehavior( pParam );
			break;
		}

		case DELETE_BEHAVIOR_REQ:
		{
			m_pBehaviorList->DeleteBehavior( pParam );
			break;
		}
		case UPDATE_BEHAVIOR_REQ:
		case GET_BEHAVIOR_LIST_REQ:
		{
			m_pBehaviorList->GetFullBehaviorListToGui(pParam);
			break;
		}

		case GET_EP_LIST_REQ:
		{
			m_pEpList->GetFullEpListToGui(pParam);
			break;
		}
		case ADD_EP_REQ:
		{
			m_pEpList->AddEpFromGui( pParam );
			break;
		}
		case GUI_ADD_EP_RANGE_REQ:
		{
			m_pEpList->AddEpRangeFromGui( pParam );
			break;
		}
		case DELETE_EP_REQ:
		{
			m_pEpList->DeleteEpFromGui( pParam );
			break;
		}
		case UPDATE_EP_REQ:
		{
			m_pEpList->UpdateEpFromGui( pParam );
			break;
		}

		case SEND_DTMF_REQ:
		{
			m_pEpList->HandleDtmf(pParam);
			break;
		}
		case EP_CONNECT_REQ:
		{
			m_pEpList->HandleConnect(pParam);
			break;
		}
		case EP_DISCONNECT_REQ:
		{
			m_pEpList->HandleDisconnect(pParam);
			break;
		}
		case ADD_SCRIPT_REQ:
		{
			break;
		}
		case PLAY_SOUND_REQ_REQ:
		{
			m_pEpList->playSoundReqReq(pParam);
			break;
		}
		case EP_MUTE_REQ:
		case EP_UNMUTE_REQ:
		case EP_ACTIVE_SPEAKER_REQ:
		case EP_AUDIO_SPEAKER_REQ:
		case EP_FECC_ASK_TOKEN_REQ:
		case EP_FECC_RELEASE_TOKEN_REQ:
		case EP_H239_ASK_TOKEN_REQ:
		case EP_H239_RELEASE_TOKEN_REQ:
		case EP_DETAILS_REQ:
		case EP_ENDPOINT_CHANNELS_REQ:
		case EP_SIP_CS_SIG_REINVITE_IND:
		case EP_FECC_KEY_REQ:
		{
			m_pEpList->HandleGuiEpMessage(opcode,pParam);
			break;
		}

		default:
		{
			DBGPASSERT(1000+opcode);
			break;
		}
	}
}






