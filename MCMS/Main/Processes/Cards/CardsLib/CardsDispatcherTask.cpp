// CardsDispatcherTask.cpp: implementation of the CCardsDispatcherTask class.
//
//////////////////////////////////////////////////////////////////////


#include "CardsDispatcherTask.h"
#include "CardsProcess.h"
#include "ProcessBase.h"
//#include "CardsDefines.h"
#include "SystemFunctions.h"
#include "MplMcmsProtocol.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsShelfMngr.h"
#include "OpcodesMcmsInternal.h"
#include "Segment.h"
#include "TraceStream.h"
#include "Trace.h"
#include "MplMcmsStructs.h"
#include "SwitchApi.h"
#include "MfaApi.h"
#include "RtmIsdnApi.h"
#include "MplMcmsProtocolTracer.h"
#include "OpcodesMcmsCardMngrMaintenance.h"

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////


extern "C" void CardsDispatcherEntryPoint(void* appParam)
{  
	CCardsDispatcherTask * cardsDispatcherTask = new CCardsDispatcherTask;
	cardsDispatcherTask->Create(*(CSegment*)appParam);
}

//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CCardsDispatcherTask* GetpConfPartyRoutingTable( void );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CCardsDispatcherTask::CCardsDispatcherTask()
        :CDispatcherTask(FALSE)
{
	m_pProcess = (CCardsProcess*)CCardsProcess::GetProcess();
	
	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for(int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
			m_isAssertForThisMfaTask[i][j] = NO;
	}
    m_Thread_Group = eTaskGroupRegular;
	
}
/////////////////////////////////////////////////////////////////////
CCardsDispatcherTask::~CCardsDispatcherTask()
{

}

/////////////////////////////////////////////////////////////////////////////
//ALL SPECIFIC HANDLING OF EVENTS IN CARDS PROCESS SHOULD BE ADDED IN THIS FUNCTION
BOOL CCardsDispatcherTask::TaskHandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{               
	void* ptr = NULL;        
	switch ( opCode ) 
	{						   
	case MPLAPI_MSG:  
		{
			HandleMPLEvent(pMsg);
			break;
		}
	default:
		{
			// was put in commrnt due to leak problems
			// TRACESTR(eLevelInfoNormal) << "CCardsDispatcherTask::HandleEvent : invalid opcode";
			return FALSE;
			break;
		}
	}
	return TRUE;
  
}              

/////////////////////////////////////////////////////////////////////////////
void CCardsDispatcherTask::HandleMPLEvent(CSegment *pSeg)
{
	STATUS res = STATUS_OK;
	
	CMplMcmsProtocol* pProtocol = new CMplMcmsProtocol;
	pProtocol->DeSerialize(*pSeg);
	CMplMcmsProtocolTracer(*pProtocol).TraceMplMcmsProtocol("CARDS_DISPATCHER_RECEIVED_FROM_MPL");

	//extract info from segment
	OPCODE opcd				= pProtocol->getOpcode();
	BYTE   msgBoardId		= pProtocol->getPhysicalInfoHeaderBoard_id(),
		   msgSubBoardId	= pProtocol->getPhysicalInfoHeaderSub_board_id();


	// send the segment to the relevant MFA
	// ===== 1. get Board's Mbx
	CCardsProcess* pProcess = (CCardsProcess*)CCardsProcess::GetProcess();
	COsQueue* queue = pProcess->GetMfaMbx(msgBoardId, msgSubBoardId);

	if (!queue)
	{
		// ASSERT + log (only once for each absent queue)
		if ( NO == GetIsAssertForThisMfaTask(msgBoardId, msgSubBoardId) )
		{
			if (opcd != SM_FATAL_FAILURE_IND ) //vngr-21480
			   PASSERT(1);
			
			string opcdStr = m_pProcess->GetOpcodeAsString(opcd);
    		TRACESTR(eLevelInfoNormal)
    			<< "\nCCardsDispatcherTask::HandleMPLEvent. Opcode: "  << opcdStr.c_str()
				<< "\nNo Mbx for BoardId: " << (WORD)msgBoardId << ", SubBoardId: " << (WORD)msgSubBoardId;
				
			SetIsAssertForThisMfaTask(YES, msgBoardId, msgSubBoardId);
		}


		// if a msg received from Switch before Authentication succeeded, then restarting Authentication procedure
		//     (and resetting all MFA boards) should be performed
		//     [note: we look for a msg from Switch since SwitchBoardId is needed in order to send the request to MPL].

		if ( (NO == m_pProcess->GetIsAuthenticationSucceeded()) && (SM_CARD_MNGR_RECONNECT_IND == opcd) )
		{
	    	TRACESTR(eLevelInfoNormal)
	    		<< "\nCCardsDispatcherTask::HandleMPLEvent. SM_CARD_MNGR_RECONNECT_IND received before Authentication succeeded";
			
			// 09.05.07: upon Switch guys request - Mcms will initiate system reset
			//SendRestartAuthenticationProcedureReqToCardsMngr(msgBoardId);
	    	
	    	// removed from the code according to VNGR-11894
			//m_pProcess->SendResetReqToDaemon("ShelfMngr sent reconnect indication before Authentication succeeded");
			
		}
		
		else
		{
			/*********************************************************************************/
			/* 21.7.10 VNGR-16271 added by Rachel Cohen                                      */
			/* We send to MPL ,in same second ,2 msgs : MFA_TASK_NOT_CREATED_REQ and         */
			/* CARDS_NOT_READY_REQ and that cause MPL to send                                */
			/* CM_CARD_MNGR_LOADED_IND twice one for each msg. That cause us to open         */
			/* and destroy mfaTask in an endless loop.                                       */
			/* We send OLD_SYSCFG_PARAMS_IND in initMfaTask and get OLD_SYSCFG_PARAMS_IND    */
			/* from MPL and that cause us to send MFA_TASK_NOT_CREATED_REQ to MPL.           */
			/*********************************************************************************/
            if ((opcd != OLD_SYSCFG_PARAMS_IND) && (msgBoardId != SWITCH_BBOARD_ID ))// 12.8.10 in case of board id 5 ,switch ,mfa_not_created is not the right msg. Rachel Cohen
            {
    			/***********************************************************************************/
    			/* 29.7.11 VNGR-21841 added by Rachel Cohen                                        */
            	/* when card pass hot swap it do not know that it is going to shut down . so it    */
            	/* keep sending the regular msgs like Ethernet_setting_IND and Keep_Alive_IND      */
            	/* mcms sending back MfaNotCreated msg and that cause the card to send MNGR_LOADED */
            	/* indication and to start the startup flow between MFA and MCMS                   */
            	/* on that fix if the card passed  hotswap mcms will wait 10 seconds before sending*/
            	/* MfaNotCreated msg.                                                              */
            	/***********************************************************************************/
               if (m_pProcess->GetCardHotSwapStatus(msgBoardId) == NO)
			       SendMfaNotCreatedToMplApi(msgBoardId, msgSubBoardId);
               else
            	   TRACESTR(eLevelInfoNormal)
            	   	    		<< "\nCCardsDispatcherTask::HandleMPLEvent.  received from MFA boardid " << msgBoardId << " msg with opcode "<<opcd
            	   	    		<<" but the card is passing hot swap";
            }

		}
	}
	
	else // queue ok
	{
		SetIsAssertForThisMfaTask(NO, msgBoardId, msgSubBoardId); // if connection will fail next time, an ASSERT will be produced
		
		if ( eMfaTaskZombie != pProcess->GetMfaTaskState(msgBoardId, msgSubBoardId) )
		{
			// ===== 2. prepare the data to be sent
			CSegment* pSegToBoard = new CSegment;
			if( pProtocol->getDataLen() )
			{
				pSegToBoard->Put( (unsigned char*)pProtocol->GetData(), pProtocol->getDataLen() );
			}
		

			// ===== 3. send the message
			BYTE switchBoardId = pProcess->GetAuthenticationStruct()->switchBoardId;
			if ( msgBoardId == switchBoardId ) 				// -----> Switch board
			{
				CSwitchApi switchApi;
				switchApi.CreateOnlyApi(*queue);
				res = switchApi.SendMsg(pSegToBoard, opcd);
				pSegToBoard = NULL;
			}
			else
			{
				int mainSubBoardId = FIRST_SUBBOARD_ID;
				if ( msgSubBoardId == mainSubBoardId )		// -----> MPM (Mfa) board
				{
					CMfaApi mfaApi;
					mfaApi.CreateOnlyApi(*queue);
					
					if(opcd == ART_FIPS_140_IND)
					{
						CSegment* pSeg = new CSegment();
						pProtocol->Serialize(*pSeg);
						res = mfaApi.SendMsg(pSeg, opcd);
					}
					else
					{
						res = mfaApi.SendMsg(pSegToBoard, opcd);
						pSegToBoard = NULL;
					}
				}
				else										// -----> Rtm entity
				{
					CRtmIsdnApi rtmIsdnApi;
					rtmIsdnApi.CreateOnlyApi(*queue);
					res = rtmIsdnApi.SendMsg(pSegToBoard, opcd);
					pSegToBoard = NULL;
				}
			}

			delete pSegToBoard;
		} // end if (task != zombie)
		else
		{
	    	TRACESTR(eLevelInfoNormal) << "\nCCardsDispatcherTask::HandleMPLEvent. Task is zombie!";
		}
	} // end else (queue ok)

	delete pProtocol;
}


/////////////////////////////////////////////////////////////////////////////
void CCardsDispatcherTask::SendRestartAuthenticationProcedureReqToCardsMngr(BYTE switchBoardId)
{
	TRACESTR(eLevelInfoNormal) << "CCardsDispatcherTask::SendRestartAuthenticationProcedureReqToCardsMngr";

	// ===== 1. insert switchBoardId to a segment
    CSegment* pRetParam = new CSegment;
    *pRetParam << switchBoardId;

	// ===== 2. send to CardsMngr
	const COsQueue* pCardsMngrMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCards, eManager);

	STATUS res = pCardsMngrMbx->Send(pRetParam, CARDS_RESTART_AUTHENTICATION_PROCEDURE_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void CCardsDispatcherTask::SendMfaNotCreatedToMplApi(BYTE boardId, BYTE subBoardId)
{
	TRACESTR(eLevelInfoNormal) << "CCardsDispatcherTask::SendMfaNotCreatedToMplApi";

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(MFA_TASK_NOT_CREATED_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, boardId, subBoardId);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}


/////////////////////////////////////////////////////////////////////////////
void CCardsDispatcherTask::SendCardsNotReadyToMplApi(BYTE boardId, BYTE subBoardId)
{
	TRACESTR(eLevelInfoNormal) << "CCardsDispatcherTask::SendCardsNotReadyToMplApi";

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(CARDS_NOT_READY_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, boardId, subBoardId);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}


//////////////////////////////////////////////////////////////////////////
void CCardsDispatcherTask::SetIsAssertForThisMfaTask(BOOL isAssert, DWORD boardId, DWORD subBoardId)
{
	if ( (MAX_NUM_OF_BOARDS <= boardId) || (MAX_NUM_OF_SUBBOARDS <= subBoardId) )
	{
		char buff[TEN_LINE_BUFFER_LEN];
		sprintf(buff, "CCardsDispatcherTask::SetIsAssertForThisMfaTask - Illegal: boardId: %d, subBoardId: %d", boardId, subBoardId);
		PASSERTMSG(1, buff);
	}

	m_isAssertForThisMfaTask[boardId][subBoardId] = isAssert;
}

//////////////////////////////////////////////////////////////////////////
BOOL CCardsDispatcherTask::GetIsAssertForThisMfaTask(DWORD boardId, DWORD subBoardId)
{
	if ( (MAX_NUM_OF_BOARDS <= boardId) || (MAX_NUM_OF_SUBBOARDS <= subBoardId) )
	{
		char buff[TEN_LINE_BUFFER_LEN];
		sprintf(buff, "CCardsDispatcherTask::GetIsAssertForThisMfaTask - Illegal: boardId: %d, subBoardId: %d", boardId, subBoardId);
		PASSERTMSG_AND_RETURN_VALUE(1, buff, TRUE);
	}

	return m_isAssertForThisMfaTask[boardId][subBoardId];
}

