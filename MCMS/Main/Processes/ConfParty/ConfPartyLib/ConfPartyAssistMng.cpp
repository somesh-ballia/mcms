#include "ConfPartyAssistMng.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "BridgePartyVideoUniDirection.h"
#include "SharedMemoryMapCommon.h"
#include "TerminalCommand.h"
#include "ConfPartyMplMcmsProtocolTracer.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"
#include "SipProxyManagerApi.h"
#include "RsrvManagerApi.h"
#include "GkTaskApi.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include "OsTask.h"
#include <sys/signal.h>

extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );
extern CIpServiceListManager*   GetIpServiceListMngr();
extern char* LogicalResourceTypeToString(APIU32 logicalResourceType);

const WORD CHECK_IF_ALL_ACKS_RECEIVED = 104;
const WORD CHECK_IF_ALL_ACKS_RECEIVED_TOUT = 5 * SECOND;

const WORD PRINT_PROCESS_MEMORY_INFO = 105;
//const WORD PRINT_PROCESS_MEMORY_INFO_TOUT = 60 * SECOND;

PBEGIN_MESSAGE_MAP(CConfPartyAssistMng)

  ONEVENT(KILL_PORT_REQ,                                      ANYCASE, CConfPartyAssistMng::OnKillPortAck)
  ONEVENT(ALLOC_STATUS_PER_UNIT_REQ,                          ANYCASE, CConfPartyAssistMng::OnAllocStatusPerUnitAck)
  ONEVENT(CHECK_IF_ALL_ACKS_RECEIVED,                         ANYCASE, CConfPartyAssistMng::OnCheckIfAllAcksRecievedTout)
  ONEVENT(PROCESS_MEMINFO_PRINT,             		          ANYCASE, CConfPartyAssistMng::OnProcessMemInfoPrint)
  ONEVENT(PRINT_PROCESS_MEMORY_INFO,             		      ANYCASE, CConfPartyAssistMng::OnProcessMemInfoPrintTout)
  ONEVENT(CLEAN_FAILED_CONFERENCE,             		     	  ANYCASE, CConfPartyAssistMng::CleanConf)
  ONEVENT(CLEAN_FAILED_PARTY,             		     	      ANYCASE, CConfPartyAssistMng::CleanParty)
  ONEVENT(GET_CONFS_AND_PARTIES_LIST_IND,                     ANYCASE, CConfPartyAssistMng::OnConfPartyListInd)
  ONEVENT(GET_PARTY_VIDEO_DATA_IND ,                          ANYCASE, CConfPartyAssistMng::OnGetPartyVideoListInd)


PEND_MESSAGE_MAP(CConfPartyAssistMng,CTaskApp);




void ConfPartyAssistMngEntryPoint(void* appParam)
{
	CConfPartyAssistMng* pAssistTaskApp = new CConfPartyAssistMng;
	pAssistTaskApp->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CConfPartyAssistMng::CConfPartyAssistMng()
{
	m_pProcess = (CConfPartyProcess*) CProcessBase::GetProcess();
	m_printTimeout = PRINT_PROCESS_MEMORY_INFO_TOUT;

	for (int board = 0; board < MAX_NUM_OF_BOARDS; board++) {
		for (int unit = 0; unit < MAX_NUM_OF_UNITS; unit++)
			m_faultyUnitsTable[board][unit] = FALSE;
	}
	m_psColumnNamesVec.push_back("pid");
	m_psColumnNamesVec.push_back("name");
	m_psColumnNamesVec.push_back("status");
	m_psColumnNamesVec.push_back("pcpu");
	m_psColumnNamesVec.push_back("resident mem");
	m_psColumnNamesVec.push_back("virtual mem");
	m_psColumnNamesVec.push_back("start_time");
	m_psColumnNamesVec.push_back("end_time");
}

CConfPartyAssistMng::~CConfPartyAssistMng()
{
	m_psColumnNamesVec.clear();
}
/////////////////////////////////////////////////////////////////////////////
void CConfPartyAssistMng::Create(CSegment& appParam, WORD limited)
{
	PTRACE(eLevelInfoNormal, "I am the Conference Party Assist Manager - my goal is to handle DSP ports map");
    CTaskApp::Create( appParam, limited );
}

const char* CConfPartyAssistMng::GetTaskName() const
{
	return "ConfPartyAssistMng";
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyAssistMng::SendAllocatedResourcesMapToMPL(stBoardUnitParams unitParams)
{

	CMedString cstr;
	cstr << "CConfPartyAssistMng::SendAllocatedResourcesMapToMPL  board: " << unitParams.m_boardId << " unit: " << unitParams.m_unitId << "\n";
    PTRACE(eLevelInfoNormal,cstr.GetString());

    BOOL isUnitDBsyncOn = YES;
    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
    sysConfig->GetBOOLDataByKey("UNIT_DB_SYNC", isUnitDBsyncOn);

    isUnitDBsyncOn = NO; //temp to fix core dump BRIDGE-4318
    if (isUnitDBsyncOn == YES)
	    SendPartyListReq(unitParams.m_boardId , unitParams.m_unitId  );
}


void CConfPartyAssistMng::OnProcessMemInfoPrint(CSegment* pParam)
{
	BYTE enbl = NO;
	DWORD interval_sec = 0;
	if( pParam )
	{
		*pParam >> enbl;
		*pParam >> interval_sec;
	}
	if( enbl ) {
		m_printTimeout = ( interval_sec ? interval_sec*SECOND : m_printTimeout);
		TRACEINTO << "CConfPartyAssistMng::OnProcessMemInfoPrint : start the timer for " << interval_sec << " sec";
		StartTimer(PRINT_PROCESS_MEMORY_INFO, m_printTimeout);
	}
	else if ( IsValidTimer(PRINT_PROCESS_MEMORY_INFO) ) {
		TRACEINTO << "CConfPartyAssistMng::OnProcessMemInfoPrint : delete the timer PRINT_PROCESS_MEMORY_INFO ";
		DeleteTimer(PRINT_PROCESS_MEMORY_INFO);
	}
}

void CConfPartyAssistMng::OnProcessMemInfoPrintTout(CSegment* pParam)
{
	std::string result, cmd, info;
	std::vector<string> psOutputVec; // Create vector to hold our words
	std::string buf; // Have a buffer string
	STATUS stat =STATUS_OK;
	//cmd = "ps -eTo pid,comm,rss,vsz,time,etime | grep ConfParty | grep -v grep";
	if( IsTarget() && (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily()) )
	{
		cmd = "ps -eo pid,ppid,pgid,comm,nice,vsz,etime | grep ConfParty";
		stat = SystemPipedCommand(cmd.c_str(), result);
		info = "CConfPartyAssistMng::OnProcessMemInfoPrintTout :\n"  + result;
	}
	else
	{
		cmd = "ps -eLo pid,comm,stat,pcpu,rss,vsz,start_time,etime | grep ConfParty | awk 'NR==2' ";
		stat = SystemPipedCommand(cmd.c_str(), result);
		std::stringstream ss(result); // Insert the string into a stream
		while (ss >> buf)
	    	psOutputVec.push_back(buf);

		info = "CConfPartyAssistMng::OnProcessMemInfoPrintTout :\n";
	    for(unsigned int i = 0; i<psOutputVec.size(); i++)
		{
	    	info += m_psColumnNamesVec[i] + ": " + psOutputVec[i] + "\n";
		}
	}
	TRACEINTO << info.c_str();
	StartTimer(PRINT_PROCESS_MEMORY_INFO, m_printTimeout);
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyAssistMng::OnKillPortAck(CSegment* pSeg)
{
	CSmallString cstr;
	cstr << "CConfPartyAssistMng::OnKillPortAck ";
	DWORD boardId = 0;
	// default subBoardId set to 1: since 'm_faultyUnitsTable' below is used to monitor only video resources,
	// and units alocated to video are always on the MFA, and not the RTM. so the sub board is 1 (MFA)
	DWORD subBoardId = 1;
	DWORD unitId = 0;

	*pSeg >> boardId  >> unitId;

	stBoardUnitParams unitParams;
	unitParams.m_boardId = boardId;
	unitParams.m_subBoardId = subBoardId;
	unitParams.m_unitId = unitId;

	cstr << " board id: " << boardId << " sub_board id: " << subBoardId << " unit id: " << unitId;

	if (boardId >= MAX_NUM_OF_BOARDS) {
		cstr << " --> ILLEGAL BOARD ID!!!";
		PTRACE(eLevelInfoNormal,cstr.GetString());
		PASSERT_AND_RETURN(boardId);
	}
	if (unitId >= MAX_NUM_OF_UNITS)
	{
		cstr << " --> ILLEGAL UNIT ID!!!";
		PTRACE(eLevelInfoNormal,cstr.GetString());
		PASSERT_AND_RETURN(unitId);
	}

	if (m_faultyUnitsTable[boardId][unitId] == TRUE)
	{
		cstr << " unit is already marked as faulty";
		PTRACE(eLevelInfoNormal,cstr.GetString());
		return;
	}
	else
	{
		cstr << " send allocated resources map to MPL";
		PTRACE(eLevelInfoNormal,cstr.GetString());
		m_faultyUnitsTable[boardId][unitId] = TRUE;
		SendAllocatedResourcesMapToMPL(unitParams);
	}

}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyAssistMng::OnAllocStatusPerUnitAck(CSegment* pSeg)
{
	CSmallString cstr;
	cstr << "CConfPartyAssistMng::OnAllocStatusPerUnitAck ";
	DWORD boardId = 0;
	DWORD unitId = 0;

	*pSeg >> boardId >> unitId;

	cstr << " board id: " << boardId << " unit id: " << unitId;
	PTRACE(eLevelInfoNormal,cstr.GetString());
	if (boardId >= MAX_NUM_OF_BOARDS) {
		PASSERT_AND_RETURN(boardId);
	}

	if (unitId >= MAX_NUM_OF_UNITS) {
	    PASSERT_AND_RETURN(unitId);
	}

	m_faultyUnitsTable[boardId][unitId] = FALSE;
}
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************/
/* ConfPartyAssistMng::CleanConf(CSegment* pSeg)		   		   */
/* The function is called when a conference terminated unexpectedly*/
/* Clean the resources, CS, SipProxy, Gk, kill all the tasks 	   */
/* of the parties and conference.							       */
/*******************************************************************/
void CConfPartyAssistMng::CleanConf(CSegment* pSeg)
{
	DWORD monitorConfId;
	DWORD taskId;
	*pSeg >> monitorConfId >> taskId;

	// Set Self-Kill for all the parties in the conference
	CCommConf* currentConf = ::GetpConfDB()->GetCurrentConf(monitorConfId);
	PASSERTMSG_AND_RETURN(!currentConf, "CConfPartyAssistMng::CleanConf - Cannot find conf in DB!");
	CConfParty* pTmpConfParty = currentConf->GetFirstParty();
	while (IsValidPObjectPtr(pTmpConfParty))
	{
		if (!IsValidPObjectPtr(pTmpConfParty->GetTask()))
		{
			PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf - party task not valid (party not connected)");
			pTmpConfParty = currentConf->GetNextParty();
			continue;
		}

		PTRACE2INT(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf - Clean party task id=", pTmpConfParty->GetTask()->GetTaskId());

		if ((pTmpConfParty->GetNetInterfaceType() == ISDN_INTERFACE_TYPE) && (pTmpConfParty->GetVoice()))
		{
			PTRACE2INT(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf - Kill task voice only, net_type=", pTmpConfParty->GetNetInterfaceType());
			COsTask::SendSignal(pTmpConfParty->GetTask()->GetTaskId(), SIGHUP);
		}
		else
		{
			PTRACE2INT(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf - Set self kill for task, net_type=", pTmpConfParty->GetNetInterfaceType());
			pTmpConfParty->GetTask()->SetSelfKill();
		}

		pTmpConfParty = currentConf->GetNextParty();
	}

	PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf - Before DelConfToSipProxy");
	// Delete conference from sipProxy
    DelConfToSipProxy(currentConf->GetName(), monitorConfId);
    PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf - After DelConfToSipProxy");

	// Clean conference and parties entries from ConfParty routing table
    // and send cleanup message to GK manager
	CManagerApi rsrcManagerApi(eProcessResource);
	BOOL isPartiesListNeeded = TRUE;
	CSegment rspMsg;
	OPCODE   rspOpcode;
	CSegment *pConfPartiesIdsReq = new CSegment;
	*pConfPartiesIdsReq << monitorConfId;
	*pConfPartiesIdsReq << isPartiesListNeeded;
	STATUS responseStatus = rsrcManagerApi.SendMessageSync(pConfPartiesIdsReq, GET_CONF_AND_PARTIES_RSRC_IDS_REQ, CONF_RSRC_REQ_TOUT, rspOpcode, rspMsg);

	if (STATUS_OK == responseStatus)
	{
		if(rspMsg.GetLen() > 0)
		{
			DWORD rsrcConfId = 0;
			WORD numOfParties = 0;
			DWORD rsrcPartyId = 0;

			rspMsg >> rsrcConfId;
			rspMsg >> numOfParties;

			CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
			if(pRoutingTbl)
			{
				PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: BEFORE CLEAN");
				pRoutingTbl->DumpTable();

				for (WORD i = 0; i < numOfParties; i++)
				{
					rspMsg >> rsrcPartyId;

					// clean current party from ConfParty routing table
					PTRACE2INT(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: RemoveAllPartyRsrcs rsrcPartyId=", rsrcPartyId);
					pRoutingTbl->RemoveAllPartyRsrcs(rsrcPartyId);
				}

				if(!currentConf->GetEntryQ())
				{
					// clean conference from ConfParty routing table
					PTRACE2INT(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: RemoveConfFromRoutingTbl rsrcConfId=", rsrcConfId);
					pRoutingTbl->RemoveConfFromRoutingTbl(rsrcConfId);
				}

				PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: AFTER CLEAN");
				pRoutingTbl->DumpTable();
			}
			else
			{
				PTRACE(eLevelInfoNormal, "CConfPartyAssistMng::CleanConf : Cannot find ConfPartyRoutingTable!");
			}

			// Send cleanup message to GK manager
			CSegment *pGKCleanupReq = new CSegment;
			*pGKCleanupReq << rsrcConfId;
			CGatekeeperTaskApi gatekeeperApi(0);
			STATUS status = gatekeeperApi.SendMsg(pGKCleanupReq, CP_GK_CONF_ID_CLEAN_UP);
		}
		else // no content in segment
		{
			PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: GET_CONF_AND_PARTIES_RSRC_IDS_REQ No Msg Content!");
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: GET_CONF_AND_PARTIES_RSRC_IDS_REQ TIME OUT!");
	}


	if (currentConf->GetIsCOP())
	{
		// Clean COP resources from ConfParty routing table
		rspMsg.Reset();
		CSegment *pCopRsrcIdsReq = new CSegment;
		*pCopRsrcIdsReq << monitorConfId;
		responseStatus = rsrcManagerApi.SendMessageSync(pCopRsrcIdsReq, GET_CONF_COP_RSRC_IDS_REQ, CONF_RSRC_REQ_TOUT, rspOpcode, rspMsg);
		if (STATUS_OK == responseStatus)
		{
			if(rspMsg.GetLen() > 0)
			{
				CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
				if(pRoutingTbl)
				{
					PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: COP BEFORE CLEAN 2");
					pRoutingTbl->DumpTable();

					ALLOCATED_COP_RSRC_PARAM_S currCopRsrcParams;
					DWORD rsrcConfId = 0;
					DWORD numOfRsrcs = 0;

					memset(&currCopRsrcParams, 0, sizeof(ALLOCATED_COP_RSRC_PARAM_S));

					rspMsg >> rsrcConfId;
					rspMsg >> numOfRsrcs;

					PTRACE2INT(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: COP numOfRsrcs=", numOfRsrcs);

					for (WORD i = 0; i < numOfRsrcs; i++)
					{
						rspMsg.Get((BYTE*)(&currCopRsrcParams),sizeof(ALLOCATED_COP_RSRC_PARAM_S));

						// clean current COP resource from ConfParty routing table
						CLargeString str;
						str << " rsrcEntityId=" << currCopRsrcParams.rsrcEntityId;
						str << " logicalRsrcType=" << LogicalResourceTypeToString(currCopRsrcParams.logicalRsrcType);
						PTRACE2(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: COP RemoveAllPartyRsrcs", str.GetString());
						if (currCopRsrcParams.logicalRsrcType != eLogical_PCM_manager)
						{
							pRoutingTbl->RemoveAllPartyRsrcs(currCopRsrcParams.rsrcEntityId);
						}
						else // eLogical_PCM_manager
						{
							CRsrcParams PcmRsrsParams;
							CRsrcDesc   PcmRsrsDesc;
							PcmRsrsParams.SetLogicalRsrcType(eLogical_PCM_manager);
							PcmRsrsParams.SetConfRsrcId(rsrcConfId);
							PcmRsrsParams.SetConnectionId(currCopRsrcParams.connectionId);
							PcmRsrsDesc.SetConnectionId(currCopRsrcParams.connectionId);
							PcmRsrsDesc.SetLogicalRsrcType(eLogical_PCM_manager);
							PcmRsrsParams.SetRsrcDesc(PcmRsrsDesc);
							pRoutingTbl->RemovePartyRsrc(PcmRsrsParams);
						}
					}

					PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: COP AFTER CLEAN 2");
					pRoutingTbl->DumpTable();
				}
				else
				{
					PTRACE(eLevelInfoNormal, "CConfPartyAssistMng::CleanConf : COP Cannot find ConfPartyRoutingTable!");
				}
			}
			else // no content in segment
			{
				PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: COP GET_CONF_AND_PARTIES_RSRC_IDS_REQ No Msg Content!");
			}
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: COP GET_CONF_AND_PARTIES_RSRC_IDS_REQ TIME OUT!");
		}
	}

    // Clean conf + parties resources in Resource Allocator
	rspMsg.Reset();
	CSegment* pDeallocPartiesReq = new CSegment;
	*pDeallocPartiesReq << monitorConfId;
	responseStatus = rsrcManagerApi.SendMessageSync(pDeallocPartiesReq, FORCE_DEALLOCATE_ALL_PARTIES_IN_CONF_RSRC_REQ, CONF_RSRC_REQ_TOUT, rspOpcode, rspMsg);
	if (STATUS_OK == responseStatus)
	{
		PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: FORCE_DEALLOCATE_ALL_PARTIES_IN_CONF_RSRC_REQ finished!");

		if(!currentConf->GetEntryQ())
		{
			rspMsg.Reset();

			CONF_RSRC_REQ_PARAMS_S* pConfRsrcReqParams = new CONF_RSRC_REQ_PARAMS_S;
			memset(pConfRsrcReqParams, 0, sizeof(CONF_RSRC_REQ_PARAMS_S));
			pConfRsrcReqParams->monitor_conf_id = monitorConfId;
			pConfRsrcReqParams->status = STATUS_FAIL;	// Use STATUS_FAIL to indicate a kill port request later
			CSegment *pDeallocConfReq = new CSegment;
			pDeallocConfReq->Put((BYTE*)(pConfRsrcReqParams),sizeof(CONF_RSRC_REQ_PARAMS_S));
			PDELETE(pConfRsrcReqParams);

			responseStatus = rsrcManagerApi.SendMessageSync(pDeallocConfReq, TERMINATE_CONF_RSRC_REQ, CONF_RSRC_REQ_TOUT, rspOpcode, rspMsg);
			if (STATUS_OK == responseStatus)
			{
				PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: TERMINATE_CONF_RSRC_REQ finished!");
			}
			else
			{
				PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: TERMINATE_CONF_RSRC_REQ TIME OUT!");
			}
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::CleanConf: FORCE_DEALLOCATE_ALL_PARTIES_IN_CONF_RSRC_REQ TIME OUT!");
	}


	// Clean reservator manager resources
	rspMsg.Reset();
	OPCODE opcode;
	if (currentConf->IsMeetingRoom())
		opcode = SYNC_DEACTIVATE_MR_REQ;
	else
		opcode = SYNC_DEL_RSRV_CONF_REQ;
	CRsrvManagerApi rsrvManagerApi;
	CSegment *seg = new CSegment;
	*seg << monitorConfId;
	responseStatus = rsrvManagerApi.SendMessageSync(seg, opcode, CONF_RSRC_REQ_TOUT, rspOpcode, rspMsg);
	if (STATUS_OK == responseStatus)
	{
		CMedString str;
		str << "CConfPartyAssistMng::CleanConf: " << (opcode == SYNC_DEL_RSRV_CONF_REQ ? "SYNC_DEL_RSRV_CONF_REQ" : "SYNC_DEACTIVATE_MR_REQ") << " finished!";
		PTRACE(eLevelInfoNormal, str.GetString());
	}
	else
	{
		CMedString str;
		str << "CConfPartyAssistMng::CleanConf: " << (opcode == SYNC_DEL_RSRV_CONF_REQ ? "SYNC_DEL_RSRV_CONF_REQ" : "SYNC_DEACTIVATE_MR_REQ") << " TIME OUT!";
		PTRACE(eLevelInfoNormal, str.GetString());
	}

	// Update CDR with terminate conf reason
	currentConf->EndConference(UNKNOWN_ERROR);

	//in case of Meeting Room Conf , update the MR dB
	if (currentConf->IsMeetingRoom())
	{
		const char* mrName = currentConf->GetName();
		if (currentConf->GetEntryQ())
		{
			mrName = ::GetpMeetingRoomDB()->GetOrigionEqReservationName(mrName, monitorConfId);
		}
		CCommRes* pMeetingRoom =::GetpMeetingRoomDB()->GetCurrentRsrv(mrName);
		if (pMeetingRoom)
		{
			pMeetingRoom->SetMeetingRoomState(MEETING_ROOM_PASSIVE_STATE);
			::GetpMeetingRoomDB()->Update(*pMeetingRoom);
			POBJDELETE(pMeetingRoom);
		}
		else
		{
			PTRACE(eLevelError,"CConfPartyAssistMng::CleanConf: MR not found!");
		}
	 }

	// Delete the Conf Queue which was allocated in the ConfPartyManager
	COsQueue * pConfRcvMailBox = currentConf->GetRcvMbx();
	PDELETE(pConfRcvMailBox);
	currentConf->SetRcvMbx(NULL);

    // Remove conference from ConfDB
	if (::GetpConfDB()->FindId(monitorConfId) != NOT_FIND)
	{
	    ::GetpConfDB()->Cancel(monitorConfId);
	}

	// Kill the thread of the conference
	COsTask::SendSignal(taskId, SIGHUP);

	// Print message to short fault list
	CMedString str;
	str << "Cleanup completed for Conference ID=" << monitorConfId;
	CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, TASK_FAILED, MAJOR_ERROR_LEVEL, str.GetString(), FALSE);
}

void CConfPartyAssistMng::CleanParty(CSegment* pSeg)
{
	PartyMonitorID monitoryPartyId;
	DWORD taskId;
	*pSeg >> monitoryPartyId >> taskId;

	PTRACE2INT(eLevelInfoNormal,"CCConfPartyAssistMng::CleanParty, monitorPartId", monitoryPartyId);
	PTRACE2INT(eLevelInfoNormal,"CCConfPartyAssistMng::CleanParty, taskId", taskId);

	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pCommConf = pCommConfDB->GetFirstCommConf();
	CConfParty* pConfParty = NULL;
	BOOL conf_found = FALSE;

	//get conference
	while(IsValidPObjectPtr(pCommConf)  && conf_found == FALSE)
	{
		pConfParty = pCommConf->GetFirstParty();
		while(pConfParty)
		{
			if(monitoryPartyId == pConfParty->GetPartyId())
			{
				conf_found = TRUE;
				break;
			}
			pConfParty = pCommConf->GetNextParty();
		}

		if(conf_found == FALSE)
			pCommConf = pCommConfDB->GetNextCommConf();
	}

	PASSERT_AND_RETURN(NULL == pCommConf);
	PASSERT_AND_RETURN(NULL == pConfParty);

	CConfApi confApi;
	confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));

	confApi.DropPartyViolent(pConfParty->GetName(), 0/*delete*/,0/*discCause*/ , taskId);

	//////////////////////////////////////////////////////////////////

    // send cleanup message to GK manager
	CManagerApi rsrcManagerApi(eProcessResource);
	BOOL isPartiesListNeeded = TRUE;
	CSegment rspMsg;
	OPCODE   rspOpcode;
	DWORD rsrcPartyId = 0;
	CSegment *pConfPartiesIdsReq = new CSegment;

	*pConfPartiesIdsReq << pCommConf->GetMonitorConfId();
	*pConfPartiesIdsReq << monitoryPartyId;
	STATUS responseStatus = rsrcManagerApi.SendMessageSync(pConfPartiesIdsReq, GET_PARTY_RSRC_ID_REQ, CONF_RSRC_REQ_TOUT, rspOpcode, rspMsg);
	PTRACE2INT(eLevelInfoNormal,"CCConfPartyAssistMng::CleanParty, responseStatus = ", responseStatus);
	//PTRACE2(eLevelInfoNormal,"CCConfPartyAssistMng::CleanParty, rspMsg = ", rspMsg);
	if (STATUS_OK == responseStatus)
	{
		PTRACE2INT(eLevelInfoNormal,"CCConfPartyAssistMng::CleanParty, rspMsg.GetLen() = ", rspMsg.GetLen());
		if(rspMsg.GetLen() > 0)
		{

			rspMsg >> rsrcPartyId;
			PTRACE2INT(eLevelInfoNormal,"CCConfPartyAssistMng::CleanParty,   rsrcPartyId= ", rsrcPartyId);
		}

	// Send cleanup message to GK manager
		CSegment *pGKCleanupReq = new CSegment;
		*pGKCleanupReq << rsrcPartyId;
		CGatekeeperTaskApi gatekeeperApi(0);
		STATUS status = gatekeeperApi.SendMsg(pGKCleanupReq, CP_GK_PARTY_ID_CLEAN_UP);
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CCConfPartyAssistMng::CleanParty, party not exists = ", monitoryPartyId);
	}
	//////////////////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////////////////
void  CConfPartyAssistMng::DelConfToSipProxy(const char* confName, DWORD confId)
{
	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	for( int i=0; i<NUM_OF_IP_SERVICES;i++ )
	{
	    CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(i);
	    if( pServiceParams == NULL )
	       continue;
	    CSipProxyManagerApi SipProxyApi;
	    SipProxyApi.DelConference(i, confName, confId);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyAssistMng::SendPartyListReq(DWORD boardId ,DWORD unitId )
{
	PTRACE(eLevelInfoNormal,"CConfPartyAssistMng::SendPartyListReq");

	CSegment*  pRetParam = new CSegment;
	*pRetParam << (DWORD)boardId;
	*pRetParam << (DWORD)unitId;

	const COsQueue* pResourceMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResource, eManager);

	GetRcvMbx().Serialize(*pRetParam);

	STATUS res = pResourceMbx->Send(pRetParam,GET_CONFS_AND_PARTIES_LIST_REQ);
}

//////////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyAssistMng::OnConfPartyListInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnConfPartyListInd");
	PASSERT_AND_RETURN_VALUE(!pParam, STATUS_FAIL);

	std::set<DWORD> monitorConfIdSet;

	CONF_PARTY_LIST_S* pParamData = new CONF_PARTY_LIST_S;

	pParam->Get((BYTE*)( &(pParamData->list_size) ),sizeof(DWORD));
	pParam->Get((BYTE*)( &(pParamData->boardId) ),sizeof(DWORD));
	pParam->Get((BYTE*)( &(pParamData->unitId) ),sizeof(DWORD));

	DWORD listSize = pParamData->list_size;
	DWORD boardId  = pParamData->boardId;
	DWORD unitId   = pParamData->unitId;

	pParamData->conf_party_list = new CONF_PARTY_ELEMENTS_S[listSize];
	pParam->Get( (BYTE*)( pParamData->conf_party_list ),sizeof(CONF_PARTY_ELEMENTS_S)*listSize );

	TRACEINTO <<"CConfPartyAssistMng::OnConfPartyListInd boardId " << boardId << " unitId " << unitId;

	if (pParamData->list_size == 0)
	{

		TRACEINTO <<"CConfPartyAssistMng::OnConfPartyListInd send to MPL empty struct" ;

		ALLOC_STATUS_PER_UNIT_S allocStatusPerUnit;
		memset(&allocStatusPerUnit,0,sizeof(ALLOC_STATUS_PER_UNIT_S));
		SendAllVideoParamsToMPL(&allocStatusPerUnit,1,(BYTE) boardId,(BYTE)unitId);

		delete [] pParamData->conf_party_list;
		delete pParamData;
		return STATUS_OK;
	}

	for(WORD j=0; j< pParamData->list_size;j++)
	{
		TRACEINTO <<"CConfPartyAssistMng::OnConfPartyListInd index " <<j
				<< " Monitor Conf Id " << pParamData->conf_party_list[j].monitor_conf_id
				<< " Rsrc Conf Id " << pParamData->conf_party_list[j].rsrc_conf_id
				<< " Rsrc Party Id " << pParamData->conf_party_list[j].rsrc_party_id
				<< " Port Id " << pParamData->conf_party_list[j].port_id
				<< " accelerator Id " << pParamData->conf_party_list[j].acceleratorId
				<< " Logical Rsrc Type " << pParamData->conf_party_list[j].logicalRsrcType;

		monitorConfIdSet.insert(pParamData->conf_party_list[j].monitor_conf_id);
	}

	for(std::set<DWORD>::iterator monitorConfId = monitorConfIdSet.begin();
			monitorConfId != monitorConfIdSet.end();   ++monitorConfId)
	{
		TRACEINTO << "CConfPartyAssistMng::OnConfPartyListInd MonitorConfId " << *monitorConfId;

		CCommConfDB* pCommConfDB = ::GetpConfDB();
		CCommConf* pRequestedConf = pCommConfDB->GetCurrentConf(*monitorConfId);

		if (!IsValidPObjectPtr(pRequestedConf))
		{
			TRACEINTO <<"CConfPartyAssistMng::sendConfPartyListToVB: error: Monitor Conf " << *monitorConfId << " does not exist in DB.";
			continue ;
		}

		CConfApi confApi;
		confApi.CreateOnlyApi(*(pRequestedConf->GetRcvMbx()));

		CSegment*  pSeg = new CSegment;
		//send my mail box parameters that the reply will arrive back to confPartyAssist task.
		GetRcvMbx().Serialize(*pSeg);

		*pSeg << *pParam;

		confApi.GetPartyVideoDataReq(pSeg);
	}

	delete [] pParamData->conf_party_list;
	delete pParamData;

	StartTimer(CHECK_IF_ALL_ACKS_RECEIVED,CHECK_IF_ALL_ACKS_RECEIVED_TOUT);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyAssistMng::OnGetPartyVideoListInd(CSegment* pParam)
{
	TRACEINTO << "CConfPartyAssistMng::OnGetPartyVideoListInd";

	//get data from segment
	VB_PARTY_VIDEO_LIST_S* pParamData = new VB_PARTY_VIDEO_LIST_S;

	pParam->Get((BYTE*)( &(pParamData->list_size) ),sizeof(DWORD));

	DWORD listSize = pParamData->list_size;

	pParamData->party_video_list = new ALLOC_STATUS_PER_PORT_S[listSize];
	pParam->Get( (BYTE*)( pParamData->party_video_list ),sizeof(ALLOC_STATUS_PER_PORT_S)*listSize );

	Dump(pParamData);

	DWORD boardId = pParamData->party_video_list[0].tPortPhysicalId.physical_id.physical_unit_params.board_id;
	DWORD unitId  = pParamData->party_video_list[0].tPortPhysicalId.physical_id.physical_unit_params.unit_id;

	VIDEO_PARTIES_MAP::iterator itMap;
	ALLOC_STATUS_PER_UNIT_S * tmp;
	ALLOC_STATUS_PER_PORT_S tmpst;

	itMap = m_videoParamsPerUnit.find(std::make_pair(boardId, unitId));
	if(itMap != m_videoParamsPerUnit.end())
	{
		TRACEINTO << "CConfPartyAssistMng::OnGetPartyVideoListInd entry  exist";
		tmp = &itMap->second;


		for(WORD i = 0; i < pParamData->list_size; i++)
		{
			tmpst = pParamData->party_video_list[i];
			tmp->atPortsStatus[pParamData->party_video_list[i].tPortPhysicalId.physical_id.port_id] = tmpst;
		}
	}
	else
	{
		TRACEINTO << "CConfPartyAssistMng::OnGetPartyVideoListInd entry not exist";

		ALLOC_STATUS_PER_UNIT_S * allocStatusPerUnit = new ALLOC_STATUS_PER_UNIT_S ;
		memset(allocStatusPerUnit,0,sizeof(ALLOC_STATUS_PER_UNIT_S));

		for(WORD i = 0; i < pParamData->list_size; i++)
		{
			tmpst = pParamData->party_video_list[i];
			allocStatusPerUnit->atPortsStatus[pParamData->party_video_list[i].tPortPhysicalId.physical_id.port_id] = tmpst;

		}

		m_videoParamsPerUnit[std::make_pair(boardId, unitId)] = *allocStatusPerUnit;
		PDELETE(allocStatusPerUnit);
	}

	delete[] pParamData->party_video_list;
	delete pParamData;
}

void CConfPartyAssistMng::Dump(VB_PARTY_VIDEO_LIST_S* pPartyList)
{
	std::ostringstream msg;
	msg << "CConfPartyAssistMng::Dump"
			<< "\nVB_PARTY_VIDEO_LIST_S:"
			<< "\n  Number of ports              :" << pPartyList->list_size;

	for(WORD i = 0; i < pPartyList->list_size; i++)
	{
		msg << "\n  List index                    :" << i
			<< " boxid " << (DWORD )pPartyList->party_video_list[i].tPortPhysicalId.physical_id.physical_unit_params.box_id
			<< " boardid " << (DWORD )pPartyList->party_video_list[i].tPortPhysicalId.physical_id.physical_unit_params.board_id
			<< " unitid " << (DWORD )pPartyList->party_video_list[i] .tPortPhysicalId.physical_id.physical_unit_params.unit_id ;
		if (pPartyList->party_video_list[i].tPortPhysicalId.physical_id.resource_type == eLogical_video_decoder)
		{
			msg << " DecoderParam.nProtocol " << pPartyList->party_video_list[i].tDecoderParam.nProtocol;
		}
		else if (pPartyList->party_video_list[i].tPortPhysicalId.physical_id.resource_type == eLogical_video_encoder)
		{
			msg << " EncoderParam.nProtocol " << pPartyList->party_video_list[i].tEncoderParam.nProtocol;
		}
	}
	PTRACE(eLevelInfoNormal, msg.str().c_str());
}



/////////////////////////////////////////////////////////////////////////////
void CConfPartyAssistMng::OnCheckIfAllAcksRecievedTout(CSegment* pParam)
{
    CSmallString cstr;
    cstr << "CConfPartyAssistMng::OnCheckIfAllAcksRecievedTout ";
    PTRACE(eLevelInfoNormal,cstr.GetString());

 /*   for (set<stBoardUnitParams>::iterator unitIter = m_faultyUnitsWaitingForAck.begin(); unitIter != m_faultyUnitsWaitingForAck.end(); unitIter++)
    {
        if (unitIter->m_boardId >= MAX_NUM_OF_BOARDS || unitIter->m_unitId >= MAX_NUM_OF_UNITS)
            PASSERT(1); //break??
        else
        {
            stBoardUnitParams faultyUnit = *unitIter;
            SendAllocatedResourcesMapToMPL(faultyUnit);
        }
    }

    m_faultyUnitsWaitingForAck.clear();
    */

    ALLOC_STATUS_PER_UNIT_S * tmp;
    ALLOC_STATUS_PER_PORT_S tmpst;
    std::ostringstream msg;


    for (VIDEO_PARTIES_MAP::iterator unitIter = m_videoParamsPerUnit.begin(); unitIter != m_videoParamsPerUnit.end(); unitIter++)
    {
    	std::pair<DWORD, DWORD> it = unitIter->first;
        DWORD boardIdIterator = it.first;
        DWORD unitIdIterator  = it.second;

        TRACEINTO <<"CConfPartyAssistMng::OnCheckIfAllAcksRecievedTout boardIdIterator " << boardIdIterator << " unitIdIterator " << unitIdIterator;

        BYTE boxId   = 0  ;
    	BYTE boardId =0 ;
    	BYTE unitId   ;

    	tmp = &unitIter->second;
    	for (int i=0;i<MAX_VIDEO_PORTS_PER_UNIT;i++)
    	{
    		tmpst = tmp->atPortsStatus[i];

    		msg << "\nboxid " << (DWORD )tmpst.tPortPhysicalId.physical_id.physical_unit_params.box_id
    		<< " boardid " << (DWORD )tmpst.tPortPhysicalId.physical_id.physical_unit_params.board_id
    		<< " unitid " << (DWORD )tmpst.tPortPhysicalId.physical_id.physical_unit_params.unit_id
    		<< " portId " << (DWORD )tmpst.tPortPhysicalId.physical_id.port_id
    		<< " resourceType " << (DWORD )tmpst.tPortPhysicalId.physical_id.resource_type;


    		if (tmpst.tPortPhysicalId.physical_id.physical_unit_params.board_id != 0)
    		{
    			boxId      = tmpst.tPortPhysicalId.physical_id.physical_unit_params.box_id ;
    			boardId    = tmpst.tPortPhysicalId.physical_id.physical_unit_params.board_id;
    			unitId     = tmpst.tPortPhysicalId.physical_id.physical_unit_params.unit_id;

    			TRACEINTO <<"CConfPartyAssistMng::OnCheckIfAllAcksRecievedTout inside port list boardId  " << (DWORD) boardId << " unitId " << (DWORD)unitId;
    			if (boardId != (BYTE)boardIdIterator )
    				TRACEINTO <<"CConfPartyAssistMng::OnCheckIfAllAcksRecievedTout an error in DB boardId different from boardIdIterator ";


    		}

    	}

    	//send ALLOC_STATUS_PER_UNIT_S to MPL
    	SendAllVideoParamsToMPL(tmp,boxId,(BYTE) boardIdIterator,(BYTE)unitIdIterator);
    }

    PTRACE(eLevelInfoNormal, msg.str().c_str());

    m_videoParamsPerUnit.clear();
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyAssistMng::SendAllVideoParamsToMPL(ALLOC_STATUS_PER_UNIT_S * allocStatusPerUnit,BYTE boxId,BYTE boardId,BYTE unitId)
{
	TRACEINTO << "CConfPartyAssistMng::SendAllVideoParamsToMPL ";

	BYTE subBoardId = 1 ;

	CMplMcmsProtocol *pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->AddCommonHeader(ALLOC_STATUS_PER_UNIT_REQ);
	pMplMcmsProtocol->AddMessageDescriptionHeader();
	pMplMcmsProtocol->AddPhysicalHeader(boxId,boardId,subBoardId,unitId);
	pMplMcmsProtocol->AddPortDescriptionHeader(DUMMY_PARTY_ID,DUMMY_CONF_ID);

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(allocStatusPerUnit),sizeof(ALLOC_STATUS_PER_UNIT_S));
	//pMsg->DumpHex();
	pMplMcmsProtocol->AddData(pMsg->GetWrtOffset(),(const char*)pMsg->GetPtr());
	POBJDELETE(pMsg);

	CConfPartyMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("***CHardwareInterface::SendMsgToMPL");
	STATUS status = pMplMcmsProtocol->SendMsgToMplApiCommandDispatcher();
	PASSERT(status);
	POBJDELETE(pMplMcmsProtocol);
}
