#include "TraceStream.h"
#include "Reservator.h"
#include "InternalProcessStatuses.h"
#include "CommResRsrvShort.h"
#include "TaskApi.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "CommResApi.h"
#include "ResourceProcess.h"
#include "CRsrvDB.h"
#include "HelperFuncs.h"
#include "RepeatedScheduleCalc.h"
#include "ComResRepeatDetails.h"
#include "CommResRecurrenceResponse.h"
#include "CommResFailedRecurrence.h"
#include "DefinesGeneral.h"
#include "ManagerApi.h"
#include "HlogApi.h"
#include "SingleToneApi.h"

#undef min
#include "PrettyTable.h"

PBEGIN_MESSAGE_MAP(CReservator)
	ONEVENT(START_CONFERENCE, ANYCASE, CReservator::OnTimerStartConference)
PEND_MESSAGE_MAP(CReservator, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CReservator
////////////////////////////////////////////////////////////////////////////
CReservator::CReservator()
{
	m_numPartiesInDongle          = 0;
	m_MAX_NUMBER_OF_RESERVATIONS  = 0;
	m_MAX_NUMBER_OF_MRS           = 0;
	m_MAX_NUMBER_OF_EQS           = 0;
	m_MAX_NUMBER_OF_SIP_FACTORIES = 0;

	CStructTm curTime;
	PASSERT(SystemGetTime(curTime));

	m_genesis    = (time_t)curTime;
	m_genesis    =  m_genesis / NUM_SECS_IN_INTERVAL;
	m_genesis   *= NUM_SECS_IN_INTERVAL;
	m_pNearTime  = new CStructTm(1, 1, 3000, 1, 0, 0);  // far away
	m_NearConfId = 0;
	m_numActiveForwardTransaction = 0;

	// for new limitations
	m_numMR               = 0;
	m_numEQ               = 0;
	m_numSipFact          = 0;

	m_num_conf_id_len     = NUMERIC_CONF_ID_LEN;
	m_num_conf_id_max_len = NUMERIC_CONF_ID_MAX_LEN;
	m_num_conf_id_min_len = NUMERIC_CONF_ID_MIN_LEN;
	m_nextMntrConfId      = 1;
	m_lastConfId          = 0;                          // default, should be set upon req/ind from CDR

	// init numeric id length validity from SYSTEM.CFG
	NumericIdConfigInit();

	VALIDATEMESSAGEMAP

	m_pCentralConferencesDB = NULL;

	CResourceProcess* pProcess = (CResourceProcess*)CResourceProcess::GetProcess();
	PASSERT_AND_RETURN(!pProcess);
	m_pCentralConferencesDB = pProcess->GetCentralConferencesDB();
	PASSERT_AND_RETURN(!m_pCentralConferencesDB);

	m_is_internal_reservator = FALSE;
	m_is_2C_mode             = TRUE;
	m_RPMode                 = eRPRegularMode;

	CSysConfig* pSysConfig = pProcess->GetSysConfig();
	PASSERT_AND_RETURN(!pSysConfig);

	std::string key = CFG_KEY_INTERNAL_SCHEDULER;
	BOOL is_internal;
	pSysConfig->GetBOOLDataByKey(key, is_internal);
	m_is_internal_reservator = is_internal;
}

//--------------------------------------------------------------------------
CReservator::~CReservator()
{
	POBJDELETE(m_pNearTime);
}

//--------------------------------------------------------------------------
void CReservator::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	DispatchEvent(opCode, pMsg);
}

//--------------------------------------------------------------------------
void* CReservator::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
STATUS CReservator::CreateReserveInterval(DWORD start, DWORD end, DWORD monitorConfIdNotToAdd, DWORD onlyMonitorConfIdSmallerThan, BOOL bAlsoCheckNIDsOfMRs, DWORD ipServiceId)
{
	ReservedConferences* pConfRsrvRsrc = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrc, STATUS_FAIL);

	SleepingConferences* pSleepingConferences = m_pCentralConferencesDB->GetSleepingConferences();
	PASSERT_AND_RETURN_VALUE(!pSleepingConferences, STATUS_FAIL);

	STATUS status     = STATUS_OK;
	STATUS ret_status = STATUS_OK;

	DWORD  conf_start;
	DWORD  conf_end;
	BOOL   isFromPassive = FALSE;
	BOOL   isPermanent   = FALSE;

	ReservedConferences::iterator confRsrvIterator;
	for (confRsrvIterator = pConfRsrvRsrc->begin(); confRsrvIterator != pConfRsrvRsrc->end(); confRsrvIterator++)
	{
		CIntervalRsrvAmount confNeededAmount;
		isFromPassive = confRsrvIterator->GetFromPassive();
		conf_start    = confRsrvIterator->GetStartInterval();
		conf_end      = confRsrvIterator->GetEndInterval();
		isPermanent   = confRsrvIterator->IsPermanent();

		if (isPermanent)
		{
			conf_start = start;
			conf_end   = end;
		}

		if (conf_start > end || conf_end < start)
			continue;

		if (monitorConfIdNotToAdd == confRsrvIterator->GetMonitorConfId())
			continue;

		if (onlyMonitorConfIdSmallerThan != 0)
		{
			if (confRsrvIterator->GetMonitorConfId() >= onlyMonitorConfIdSmallerThan)
				continue;
		}

		((CConfRsrvRsrc)(*confRsrvIterator)).GetConferenceNeededAmount(confNeededAmount, TRUE);

		TRACEINTO << "MonitorConfId:" << confRsrvIterator->GetMonitorConfId();
		status = m_ReservationCalculator.ReserveAmount(max(start, conf_start), min(end, conf_end), confNeededAmount, isFromPassive, FALSE, ePhoneCheckAlloc, ipServiceId);

		if (status != STATUS_OK)
			ret_status = status;
	}

	CIntervalRsrvAmount confNeededAmount;
	if (bAlsoCheckNIDsOfMRs == TRUE)
	{
		// add NIDs from meeting rooms
		SleepingConferences::iterator sleepingConfsIterator;
		for (sleepingConfsIterator = pSleepingConferences->begin(); sleepingConfsIterator != pSleepingConferences->end(); sleepingConfsIterator++)
		{
			confNeededAmount.Init();
			if (sleepingConfsIterator->GetConfType() != eConf_type_none)
			{
				conf_start = start;
				conf_end   = end;
				sleepingConfsIterator->GetConferenceNeededAmount(confNeededAmount);

				TRACEINTO << "MonitorConfId:" << sleepingConfsIterator->GetMonitorConfId();

				status = m_ReservationCalculator.ReserveAmount(0, 0, confNeededAmount, FALSE, FALSE, ePhoneAlloc, ipServiceId);
			}
			else
			{
				DBGPASSERT(1);
			}

			if (status != STATUS_OK)
				ret_status = status;
		}
	}

	return ret_status;
}

//--------------------------------------------------------------------------
STATUS CReservator::ProfileInRsrv(DWORD profileID)
{
	STATUS status = STATUS_OK;
	if (TRUE == IsInternalReservator())
	{
		PASSERT_AND_RETURN_VALUE(m_pCentralConferencesDB->GetRsrvDB() == NULL, STATUS_FAIL);
		if (m_pCentralConferencesDB->GetRsrvDB()->IsRsrvUsingProfile(profileID))
			status = STATUS_PROFILE_IN_USE_CANNOT_BE_DELETED;
	}
	return status;
}

//--------------------------------------------------------------------------
const CConfRsrvRsrc* CReservator::GetConfRsrvRsrcById(DWORD monitorConfId)
{
	ReservedConferences* pConfRsrvRsrc = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrc, NULL);

	ReservedConferences::iterator i;

	for (i = pConfRsrvRsrc->begin(); i != pConfRsrvRsrc->end(); i++)
	{
		const CConfRsrvRsrc& conf = *i;
		if (conf.GetMonitorConfId() == monitorConfId)
			return (&(*i));
	}

	return NULL;
}

//--------------------------------------------------------------------------
void CReservator::SetLogicalResources(WORD logical_num_parties[NUM_OF_PARTY_RESOURCE_TYPES],BOOL bRecalculateReservationPartyResources)
{
	//   PTRACE(eLevelInfoNormal,"RSRV_LOG: CReservator::SetLogicalResources");

	m_ReservationCalculator.SetLogicalResources(logical_num_parties);

	if(bRecalculateReservationPartyResources)
		RecalculateReservationPartyResources();

	//recalculate all reservations, and check which one are faulty, or aren't faulty any more
	Rereserve(eRereserveAll);
}

//--------------------------------------------------------------------------
void CReservator::RecalculateReservationPartyResources()
{
	ReservedConferences* pConfRsrvRsrc = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN(!pConfRsrvRsrc);

	CRsrvDB* pRsrvDb = m_pCentralConferencesDB->GetRsrvDB();
	PASSERT_AND_RETURN(!pRsrvDb);

	// update all the reservations
	ReservedConferences::iterator i;
	CCommResRsrvShort*                pShortRes;
	for (i = pConfRsrvRsrc->begin(); i != pConfRsrvRsrc->end(); i++)
	{
		pShortRes = pRsrvDb->GetShortRsrv(i->GetMonitorConfId());
		if (pShortRes == NULL)
		{
			// this is OK, it means it's not a reservation, but an ongoing conf. Just continue
			continue;
		}

		// update inner structs
		((CConfRsrvRsrc*)(&(*i)))->CalculateReservationPartyResources();
	}
	}

//--------------------------------------------------------------------------
size_t CReservator::StructTm2IntervalForStart(const CStructTm* timeStruct)
{
	return StructTm2Interval(timeStruct, +5);
}

//--------------------------------------------------------------------------
size_t CReservator::StructTm2IntervalForEnd(const CStructTm* timeStruct)
{
	return StructTm2Interval(timeStruct, -5);
}

//--------------------------------------------------------------------------
size_t CReservator::StructTm2Interval(const CStructTm* timeStruct, int offset)
{
	DWORD tres1 = (size_t)(*timeStruct);

	if (tres1 < m_genesis)
	{
		FPTRACE(eLevelError, "Illegal reservation time");
		return 0;
	}

	tres1 -= m_genesis;

	DWORD  tres = (DWORD)((long)tres1 + offset);

	size_t ires = ((size_t)tres)/NUM_SECS_IN_INTERVAL;

	return ires;
}

//--------------------------------------------------------------------------
void CReservator::StartTimerForNextConference()
{
	PASSERT_AND_RETURN(m_pCentralConferencesDB->GetRsrvDB() == NULL);

	CCommResRsrvShort* pNextResShort = m_pCentralConferencesDB->GetRsrvDB()->GetNextReservation();
	if (pNextResShort != NULL)
	{
		*m_pNearTime = *(pNextResShort->GetStartTime());
		m_NearConfId = pNextResShort->GetConferenceId();
		TRACEINTO << "ConfName:" << pNextResShort->GetName() << " - Setting timer for next conference";
		StartConferenceTimer(TRUE);
	}
	else
	{
		m_NearConfId        = 0;
		m_pNearTime->m_year = 3000; // far away
		TRACEINTO << "No next timer";
	}
}

//--------------------------------------------------------------------------

void CReservator::CreateRsrvContinue1()
{
	StartConference(true);
}

void CReservator::OnTimerStartConference(CSegment* pMsg)
{
	StartConference(false);
}

//////////////////////////////////////////////////////////////////////////////////
CCommResApi* CReservator::StartConference(bool bTransactionContinue, DWORD monitorConfId)
{
	CCommResApi* pRetValCommResApi = NULL;
	CRsrvDB* pRsrvDb = m_pCentralConferencesDB->GetRsrvDB();
	PASSERT_AND_RETURN_VALUE(!pRsrvDb, NULL);

	if (monitorConfId)
		m_NearConfId = monitorConfId;

	if (m_NearConfId != 0)
	{
		TRACEINTO << "MonitorConfId:" << m_NearConfId;

		CCommResRsrvShort* pShortRes = pRsrvDb->GetShortRsrv(m_NearConfId);
		CCommResApi* pCommResApi = pRsrvDb->GetRsrv(m_NearConfId);
		if (pShortRes != NULL && pCommResApi != NULL)
		{
			eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();

			if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
			{
				TRACEINTO << "MonitorConfId:" << pCommResApi->GetMonitorConfId() << ", DisplayName:" << pCommResApi->GetDisplayName() << " - Became past due during startup and has been deleted from the Reservation list";

				std::ostringstream msg;
				msg << "Reservation \"" << pCommResApi->GetDisplayName() << "\" became past due during startup and has been deleted from the Reservation list.";
				CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, START_TIME_OF_RESERVED_CONFER_IS_OVER, SYSTEM_MESSAGE, msg.str().c_str(), FALSE);

				eRsrcConfType c_type;
				DeleteResAllTypes(m_NearConfId, &c_type);
			}
			else if (pShortRes->GetStatus() == STATUS_OK)
			{
				CStructTm now;
				SystemGetTime(now);
				const CStructTm* pStarttime = pCommResApi->GetStartTime();

				DWORD dif = 0;

				if (now < *pStarttime)
					dif = *pStarttime - now;
				else
					dif = now - *pStarttime;

				if (dif > 60)
				{
					TRACEINTO << "Diff:" << dif << ", Now:" << now << ", StartTime:" << *pStarttime << " - Big time difference (Diff in seconds)";
				}

				// if reservation is OK, send it to confparty
				if (eRPSlaveMode != GetRPMode())
				{
					if (bTransactionContinue)
						pRetValCommResApi = new CCommResApi(*pCommResApi);
					else
						SendResToConfParty(pCommResApi, FALSE, bTransactionContinue);
				}
			}
			else
			{
				// if it's a faulty reservation, don't start it, but delete it from the ConfRsrvRsrcs too
				TRACEINTO << "MonitorConfId:" << m_NearConfId << ", DisplayName:" << pCommResApi->GetDisplayName() << " - Cannot be started due to insufficient resources, delete reservation";

				std::ostringstream msg;
				msg << "Reservation \"" << pCommResApi->GetDisplayName() << "\" cannot be started due to insufficient resources.";
				CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, RESERVED_CONFER_HAS_NO_SUFFICIENT_RESOURCES, SYSTEM_MESSAGE, msg.str().c_str(), FALSE);

				eRsrcConfType c_type;
				DeleteResAllTypes(m_NearConfId, &c_type);
			}

			pRsrvDb->Cancel(m_NearConfId);
			POBJDELETE(pCommResApi);
		}
		else
			DBGPASSERT(1);
	}
	else
	{
		DBGPASSERT(1);
	}

	if (!m_numActiveForwardTransaction)
		StartTimerForNextConference();

	return pRetValCommResApi;
}

//--------------------------------------------------------------------------
void CReservator::TimeChanged()
{
	if (IsInternalReservator())
	{
		if (m_NearConfId != 0)
			DeleteConferenceTimer();

		StartTimerForNextConference();
	}
}

//--------------------------------------------------------------------------
STATUS CReservator::InitInternalReservator()
{
	BOOL is_internal_reservator = IsInternalReservator();

	TRACEINTO << "IsInternalReservator:" << (int)is_internal_reservator;

	STATUS status = STATUS_OK;
	if (TRUE == is_internal_reservator)  // create reservation DB
	{
		status = m_pCentralConferencesDB->CreateRsrvDB(this);

		if (status == STATUS_OK)
			StartTimerForNextConference();
	}

	return status;
}

//--------------------------------------------------------------------------
// 2 modes cop/cp
STATUS CReservator::CheckReservationSysMode(CCommResApi& i_CommResApi)
{
	if ((i_CommResApi.IsCOPReservation() && IsRMode2C()) ||       // COP
			(!i_CommResApi.IsCOPReservation() && !IsRMode2C()) ||     // CP
			i_CommResApi.GetIsHDVSW())                                // Olga - ask Carmit about VSW
		return STATUS_OK;

	return STATUS_ILLEGAL_SYS_MODE;
}

//--------------------------------------------------------------------------
STATUS CReservator::CreateReservationFromStartupOfResDB(CCommResApi* pCommResApi)
{
	DWORD bIsCordinateModes = STATUS_FAIL;

	ReservedConferences* pConfRsrvRsrcs = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrcs, STATUS_FAIL);

	CConfRsrvRsrc* pConfRsrvRsrc = new CConfRsrvRsrc(pCommResApi);

	STATUS status = CreateReserveConference_WithSchedule(pCommResApi, pConfRsrvRsrc, eConf_type_none);
	if (status == STATUS_OK)
	{
		eRsrcConfType c_type = eConf_type_none;
		{
			if (NIDAllocationAndResourcesCheckForConferences_WithSchedule(pConfRsrvRsrc, pCommResApi, c_type, TRUE) != STATUS_OK)
			{
				status = STATUS_RESERVATION_SUSPENDED;
				pConfRsrvRsrc->SetStatus(STATUS_RESERVATION_SUSPENDED);

				CReservator* pReservator = CHelperFuncs::GetReservator(); // Status Field
				if (NULL == pReservator)
					PASSERTMSG(1, "Reservator is not valid");
				else
					bIsCordinateModes = pReservator->CheckReservationSysMode(*pCommResApi);

				if (bIsCordinateModes != STATUS_OK)
					pCommResApi->SetResSts(eWrongSysMode); // Status Field
				else
					pCommResApi->SetResSts(eStsSUSPEND);
			}
		}
		pConfRsrvRsrcs->insert(*pConfRsrvRsrc);
		AllocateMntrConfId(pCommResApi->GetMonitorConfId());
	}

	POBJDELETE(pConfRsrvRsrc);

	return status;
}

//--------------------------------------------------------------------------
void CReservator::DeleteConferenceTimer()
{
	CTaskApp* pTaskApp = CProcessBase::GetProcess()->GetCurrentTask();
	if (NULL == pTaskApp || strcmp((pTaskApp->GetTaskName()), "RsrvManager") != 0)
	{
		// the timer should be set in the task RsrvManager (where this object was created), otherwise it will not be fired
		CSingleToneApi rsrvManager(eProcessResource, "RsrvManager");
		rsrvManager.SendMsg(NULL, RESOURCE_DELETE_CONFERENCE_TIMER);
		rsrvManager.CleanUp();
		return;
	}

	DeleteTimer(START_CONFERENCE);
}

//--------------------------------------------------------------------------
DWORD CReservator::StartConferenceTimer(BOOL bStartTimerAlways)
{
	CTaskApp* pTaskApp = CProcessBase::GetProcess()->GetCurrentTask();
	if (NULL == pTaskApp || strcmp((pTaskApp->GetTaskName()), "RsrvManager") != 0)
	{
		// the timer should be set in the task RsrvManager (where this object was created), otherwise it will not be fired
		CSingleToneApi rsrvManager(eProcessResource, "RsrvManager");
		rsrvManager.SendMsg(NULL, RESOURCE_START_CONFERENCE_TIMER);
		rsrvManager.CleanUp();
		return 0xFFFFFFFF;
	}

	CStructTm curTime;
	PASSERT(SystemGetTime(curTime));

	DWORD duration = 0;

	if (curTime < *m_pNearTime)
	{
		DWORD diffTime = *m_pNearTime - curTime;

		// to prevent integer overflow in the next multiply operation
		if (diffTime > 60 * 60 * 24 * 365)
			diffTime = 60 * 60 * 24 * 365;

		duration = diffTime*SECOND;
	}
	TRACEINTO << "TimerDuration:" << duration << " ms";
	if (duration > 0)
	{
		StartTimer(START_CONFERENCE, duration);
	}
	else if (bStartTimerAlways)
	{
		StartTimer(START_CONFERENCE, 0);
	}

	return duration;
}

//--------------------------------------------------------------------------
void CReservator::SendResToConfParty(CCommResApi* pCommResApi, BOOL isMeetingRoom, bool bTransactionContinue)
{
	CSegment* pRetParam = new CSegment();
	COstrStream ostr;
	pCommResApi->Serialize(NATIVE, ostr);
	ostr << STATUS_OK;
	(*pRetParam) << ostr.str().c_str();

	if (bTransactionContinue)
	{
		return;
	}
	else
	{
		// send back to  to ConfMngr
		OPCODE opcode = RSRC_START_CONF_IND;
		if (isMeetingRoom == TRUE)
			opcode = RSRC_ADD_MR_IND;

		CManagerApi api(eProcessConfParty);
		api.SendMsg(pRetParam, opcode);
	}
}

//--------------------------------------------------------------------------
STATUS CReservator::CreateConfFromAPI_NoSchedule(CCommResApi* pCommResApi)
{
	// ADDING PASSIVE MEETING ROOM ALSO PERFORMED IN THIS FUNCTION!!!
	PTRACE(eLevelInfoNormal,"CReservator::CreateConfFromAPI_NoSchedule");

	STATUS status = STATUS_OK;
	eRsrcConfType c_type = GetConfType(pCommResApi);

	// check number of conferences + name of conference in Reservation db
	// allocate monitor id
	// (when necessary) deal with min_parties

	BOOL isOng = FALSE;

	//VNGR-25136
	if (c_type == eConf_type_none)
	{
		CStructTm curTime;
		PASSERT(SystemGetTime(curTime));
		if (pCommResApi->GetStartTime()->m_year == 0 || *(pCommResApi->GetStartTime()) <= curTime)
			isOng = TRUE;
	}

	status = CommonAllocationProcedureForNewConferencesOfAllTypes(pCommResApi, c_type, TRUE, isOng);
	if (STATUS_OK != status)
		return status;

	// numeric id allocation
	status = NIDAllocation_NoSchedule(pCommResApi, c_type);
	if (status != STATUS_OK)
		return status;

	// PSTN phones allocation
	// PlusNumConfPerType
	status = AllocationProcedureForAPIConferences_NoSchedule(pCommResApi, c_type);
	if (STATUS_OK != status)
		return status;

	// Send To ConfParty process
	BYTE isMeetingRoom = FALSE;
	if (pCommResApi->IsMeetingRoom() || (pCommResApi->GetEntryQ() == YES))
		isMeetingRoom = TRUE;

//TODO //BG decide about 3-th parameter (bTransactionContinue);
	SendResToConfParty(pCommResApi, isMeetingRoom, false);

	return status;
}

//--------------------------------------------------------------------------
void CReservator::SetStartTimeIfStartImmediately(CCommResApi* pCommResApi, BOOL is_ong)
{
	if (pCommResApi->GetStartTime()->m_year == 0 || is_ong) // start immediately
	{
		// set start time as now
		CStructTm start;
		SystemGetTime(start);

		pCommResApi->SetStartTime(start);
	}
}

//--------------------------------------------------------------------------
STATUS CReservator::CreateRsrvFromAPI_WithSchedule(CCommResApi* pCommResApi, bool& bForwardTransaction, BOOL bForUpdate, BOOL bForce, BOOL bWriteNowToDisk, BOOL bAlsoCheckNIDsOfMRs)
{
	// RESERVATOR:

	// 0. number of conferences restriction check.
	// 1. monitor conf id.
	// 2. resource reservation.
	// 3. write to disk
	// 4. write short to RAM
	// 5. start timer

	// open issue : MR path ???

	TRACEINTO << "ConfName:" << pCommResApi->GetName();

	bForwardTransaction = false;

	PASSERT_AND_RETURN_VALUE(bForce == TRUE && bForUpdate == FALSE, STATUS_FAIL);  // force can meanwhile only be used when updating

	ReservedConferences* pConfRsrvRsrcs = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrcs, STATUS_FAIL);

	STATUS        status        = STATUS_OK;
	char*         numericId     = (char*)(pCommResApi->GetNumericConfId());
	eRsrcConfType c_type        = GetConfType(pCommResApi);
	BYTE          isMeetingRoom = FALSE;
	BOOL          isOng         = FALSE;

	if (c_type != eConf_type_none)
		isMeetingRoom = TRUE;

	if (c_type == eConf_type_none)
	{
		CStructTm curTime;
		PASSERT(SystemGetTime(curTime));
		if (pCommResApi->GetStartTime()->m_year == 0 || *(pCommResApi->GetStartTime()) <= curTime)
			isOng = TRUE;
	}

	SetStartTimeIfStartImmediately(pCommResApi, isOng);

	if (bForUpdate == FALSE)
	{
		status = CommonAllocationProcedureForNewConferencesOfAllTypes(pCommResApi, c_type, TRUE, isOng);
		if (STATUS_OK != status)
			return status;
	}

	if (isMeetingRoom != TRUE)
	{
		CConfRsrvRsrc* pConfRsrvRsrc = new CConfRsrvRsrc(pCommResApi);
		status = CreateReserveConference_WithSchedule(pCommResApi, pConfRsrvRsrc, c_type);
		if (STATUS_OK != status)
		{
			RollbackAndPrintStatus("CreateRsrvFromAPI_WithSchedule 1", status, TRUE, FALSE, pCommResApi, c_type);
			POBJDELETE(pConfRsrvRsrc);
			return status;
		}

		if (!bForce)
		{
			status = NIDAllocationAndResourcesCheckForConferences_WithSchedule(pConfRsrvRsrc, pCommResApi, c_type, FALSE, bAlsoCheckNIDsOfMRs);
			if (STATUS_OK != status)
			{
				RollbackAndPrintStatus("CreateRsrvFromAPI_WithSchedule 2", status, TRUE, FALSE, pCommResApi, c_type);
				POBJDELETE(pConfRsrvRsrc);
				return status;
			}
		}

		if (bForUpdate == FALSE && bWriteNowToDisk == TRUE)
		{
			if (m_pCentralConferencesDB->GetRsrvDB() != NULL)
				status = m_pCentralConferencesDB->GetRsrvDB()->Add(*pCommResApi); // both: writes to disc and creates short inside
			else
				status = STATUS_FAIL;

			if (STATUS_OK != status)
			{
				if (STATUS_RESERVATION_NAME_EXISTS != status)                     // STATUS_RESERVATION_NAME_EXISTS is a normal status
				{
					PTRACE(eLevelError, "CReservator::CreateRsrvFromAPI_WithSchedule Failed on Add Rsrv To DB");
					PASSERT(status);
				}

				RollbackAndPrintStatus("CreateRsrvFromAPI_WithSchedule 3", status, TRUE, FALSE, pCommResApi, c_type);
				POBJDELETE(pConfRsrvRsrc);
				return status;
			}
		}

		pConfRsrvRsrcs->insert(*pConfRsrvRsrc);
		POBJDELETE(pConfRsrvRsrc);
		pConfRsrvRsrc = NULL;

		if (bWriteNowToDisk == TRUE)
		{
			// check & start timer
			if (isOng || *(pCommResApi->GetStartTime()) <= *m_pNearTime || m_NearConfId == 0 || m_NearConfId == pCommResApi->GetMonitorConfId())
			{
				if (m_NearConfId != 0)
					DeleteConferenceTimer();

				if (m_NearConfId == pCommResApi->GetMonitorConfId() && (*(pCommResApi->GetStartTime()) != *m_pNearTime))    // VNGFE-2914
				{
					const CStructTm*   pStartTimeNew = pCommResApi->GetStartTime();
					CCommResRsrvShort* pShortRes     = m_pCentralConferencesDB->GetRsrvDB()->GetShortRsrv(m_NearConfId);
					pShortRes->SetStartTime(*pStartTimeNew);
					StartTimerForNextConference();
				}
				else
				{
					*m_pNearTime = *pCommResApi->GetStartTime();
					m_NearConfId = pCommResApi->GetMonitorConfId();
					TRACEINTO << "ConfName:" << pCommResApi->GetName() << " - Setting timer for next conference";
					DWORD dwDelay = StartConferenceTimer();

					if (dwDelay == 0)
					{
						if (bForUpdate) //BRIDGE-18068
						{
							StartTimer(START_CONFERENCE, 0);
						}
						else
						{
							bForwardTransaction = true;
							IncActiveForwardTransaction();
						}
					}
				}
			}
		}
	}
	else // if meeting room type
	{
		status = NIDAllocationForSleepingConferences_WithSchedule(pCommResApi, c_type);
		if (STATUS_OK != status)
			return status;

		status = AddSleepingConference(pCommResApi->GetNumericConfId(), pCommResApi->GetMonitorConfId(), c_type);
		if (STATUS_OK != status)
			return status;

		status = AllocationPhoneMR_WithSchedule(pCommResApi);
		if (STATUS_OK != status)
			return status;

		if (eRPSlaveMode != GetRPMode())
		{
		//TODO //BG decide about 3-th parameter (bTransactionContinue);
			//SendResToConfParty(pCommResApi, isMeetingRoom, false);
			bForwardTransaction = true;
		}
	}

	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::CreatePermanent_WithSchedule(CCommResApi* pCommResApi, bool& bForwardTransaction, DWORD* pProblemConfId, BOOL bForUpdate, BOOL bForce, BOOL bWriteNowToDisk, BOOL bAlsoCheckNIDsOfMRs)
{
	PASSERT_AND_RETURN_VALUE(bForce == TRUE && bForUpdate == FALSE, STATUS_FAIL);  // force can meanwhile only be used when updating

	ReservedConferences* pConfRsrvRsrcs = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrcs, STATUS_FAIL);

	STATUS        status = STATUS_OK;
	eRsrcConfType c_type = GetConfType(pCommResApi);
	BOOL          isOng  = TRUE; // VNGR-25136 change from FALSE Permanent conf is ongoing

	TRACEINTO << "ConfName:"          << pCommResApi->GetName()
	          << ", ConfType:"        << c_type
	          << ", NumericConfId:"   << pCommResApi->GetNumericConfId()
	          << ", bForUpdate:"      << (int)bForUpdate
	          << ", bForce:"          << (int)bForce
	          << ", bWriteNowToDisk:" << (int)bWriteNowToDisk
	          << ", bCheckNIDsOfMRs:" << (int)bAlsoCheckNIDsOfMRs;

	PASSERT_AND_RETURN_VALUE(c_type != eConf_type_none, STATUS_FAIL);

	SetStartTimeIfStartImmediately(pCommResApi, isOng);

	// Permanent Conf
	CStructTm currentTime;
	SystemGetTime(currentTime);
	const CStructTm* pStartTime = pCommResApi->GetStartTime();
	DWORD diff = 0;

	if (currentTime < *pStartTime)
		diff = *pStartTime - currentTime;
	else
		diff = currentTime - *pStartTime;

	if (diff)   // future fix - don't allow Reserved conf -- reject it.
	{
		TRACEINTO << "Diff:" << diff << ", currentTime:" << currentTime << ", StartTime:" << *pStartTime << " - Reserved conference can't be permanent";
	} // Permanent Conf-END

	if (bForUpdate == FALSE)
	{
		status = CommonAllocationProcedureForNewConferencesOfAllTypes(pCommResApi, c_type, TRUE, isOng);
		if (STATUS_OK != status)
			return status;
	}

	CConfRsrvRsrc* pConfRsrvRsrc = new CConfRsrvRsrc(pCommResApi);
	status = CreateReserveConference_WithSchedule(pCommResApi, pConfRsrvRsrc, c_type);
	if (STATUS_OK != status)
	{
		POBJDELETE(pConfRsrvRsrc);
		return status;
	}

	if (!bForce)
	{
		status = NIDAllocationAndResourcesCheckForPermanent(pConfRsrvRsrc, pCommResApi, c_type, FALSE, bAlsoCheckNIDsOfMRs, pProblemConfId);

		if (STATUS_OK != status)
		{
			POBJDELETE(pConfRsrvRsrc);
			return status;
		}
	}

	if (bForUpdate == FALSE && bWriteNowToDisk == TRUE)
	{
		if (m_pCentralConferencesDB->GetRsrvDB() != NULL)
			status = m_pCentralConferencesDB->GetRsrvDB()->Add(*pCommResApi); // both: writes to disc and creates short inside
		else
			status = STATUS_FAIL;

		if (STATUS_OK != status)
		{
			if (STATUS_RESERVATION_NAME_EXISTS != status) {                  // STATUS_RESERVATION_NAME_EXISTS is a normal status
				PASSERTSTREAM(1, "Failed on Add Rsrv To DB");
			}

			POBJDELETE(pConfRsrvRsrc);
			return status;
		}
	}

	pConfRsrvRsrcs->insert(*pConfRsrvRsrc);
	POBJDELETE(pConfRsrvRsrc);
	pConfRsrvRsrc = NULL;

	if (bWriteNowToDisk == TRUE)
	{
		// check & start timer
		if (m_NearConfId != 0)
			DeleteConferenceTimer();

		*m_pNearTime = *pCommResApi->GetStartTime();
		m_NearConfId = pCommResApi->GetMonitorConfId();
		bForwardTransaction = true;
		IncActiveForwardTransaction();
	}

	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::CreateConfFromMR_NoSchedule(CCommResApi* pCommResApi)
{
	PTRACE(eLevelInfoNormal,"CReservator::CreateConfFromMR_NoSchedule");

	eRsrcConfType c_type = eConf_type_none;

	// check number of conferences + name of conference in Reservation db
	// allocate monitor id
	// (when necessary) deal with min_parties
	STATUS status = CommonAllocationProcedureForNewConferencesOfAllTypes(pCommResApi, c_type, FALSE,TRUE); // VNGR-25136 conf from MR is ongoing
	if (STATUS_OK != status)
		return status;

	// numeric id allocation
	status = NIDCheckExist(pCommResApi, c_type);

	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::RsrvFromMR_WithSchedule(CCommResApi* pCommResApi)
{
	// 0. number of ongoing conferences in reservation - check.
	// 1. monitor conference id.
	PTRACE(eLevelInfoNormal,"CReservator::RsrvFromMR_WithSchedule");

	STATUS status = STATUS_OK;
	eRsrcConfType c_type = eConf_type_none;

	// set start time as now
	CStructTm start;
	SystemGetTime(start);
	pCommResApi->SetStartTime(start);

	ReservedConferences* pConfRsrvRsrcs = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrcs, STATUS_FAIL);

	char* numericId = (char*)(pCommResApi->GetNumericConfId());
	if (NumericIdExist(numericId) == FALSE) { // MR numeric id previously allocated and must exist
		PASSERTSTREAM_AND_RETURN_VALUE(STATUS_NUMERIC_CONFERENCE_ID_NOT_EXISTS, "CReservator::RsrvFromMR_WithSchedule - Failed, status:" << status, STATUS_NUMERIC_CONFERENCE_ID_NOT_EXISTS);
	}

	// monitoring ID alloc.
	status = CommonAllocationProcedureForNewConferencesOfAllTypes(pCommResApi, c_type, TRUE, TRUE); // VNGR-25136 conf from MR is ongoing
	if (STATUS_OK != status)
		return status;

	CConfRsrvRsrc* pConfRsrvRsrc = new CConfRsrvRsrc(pCommResApi, TRUE);
	status = CreateReserveConference_WithSchedule(pCommResApi, pConfRsrvRsrc, c_type);
	if (STATUS_OK != status)
	{
		RollbackAndPrintStatus("RsrvFromMR_WithSchedule 1", status, TRUE, FALSE, pCommResApi, c_type);
		POBJDELETE(pConfRsrvRsrc);
		return status;
	}

	if (!pConfRsrvRsrc->IsPermanent())
		status = NIDAllocationAndResourcesCheckForConferences_WithSchedule(pConfRsrvRsrc, pCommResApi, c_type);
	else
		status = NIDAllocationAndResourcesCheckForPermanent(pConfRsrvRsrc, pCommResApi, c_type, FALSE, FALSE);

	if (STATUS_OK != status)
	{
		RollbackAndPrintStatus("RsrvFromMR_WithSchedule 2", status, TRUE, FALSE, pCommResApi, c_type);
		POBJDELETE(pConfRsrvRsrc);
		return status;
	}

	pConfRsrvRsrcs->insert(*pConfRsrvRsrc);
	POBJDELETE(pConfRsrvRsrc);
	pConfRsrvRsrc = NULL;

	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::CreateRsrvFromMR_WithSchedule(CCommResApi* pCommResApi)
{
	// 0. number of conferences restriction check.
	// 1. monitor conf id.
	// 2. resource reservation.
	// 3. write to disk
	// 4. write short to RAM
	// 5. start timer
	// 6. return to EMA

	PTRACE(eLevelInfoNormal,"CReservator::CreateRsrvFromMR_WithSchedule");

	eRsrcConfType c_type = eConf_type_none;
	SetStartTimeIfStartImmediately(pCommResApi);

	// check number of conferences + name of conference in Reservation db
	// allocate monitor id
	// (when necessary) deal with min_parties


	//VNGR-25136
	BOOL isOng = FALSE;
	if (c_type == eConf_type_none)
	{
		CStructTm curTime;
		PASSERT(SystemGetTime(curTime));
		if (pCommResApi->GetStartTime()->m_year == 0 || *(pCommResApi->GetStartTime()) <= curTime)
			isOng = TRUE;
	}

	STATUS status = CommonAllocationProcedureForNewConferencesOfAllTypes(pCommResApi, c_type, FALSE,isOng);
	if (STATUS_OK != status)
		return status;

	// numeric id allocation
	status = NIDCheckExist(pCommResApi, c_type);
	if (STATUS_OK != status)
		return status;

	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::CreateAdHoc_NoSchedule(CCommResApi* pCommResApi)
{
	PTRACE(eLevelInfoNormal,"CReservator::CreateAdHoc_NoSchedule");

	eRsrcConfType c_type = eConf_type_none;

	// check number of conferences + name of conference in Reservation db
	// allocate monitor id
	// (when necessary) deal with min_parties
	STATUS status = CommonAllocationProcedureForNewConferencesOfAllTypes(pCommResApi, c_type, FALSE, TRUE);// VNGR-25136 AdHoc conf is ongoing
	if (STATUS_OK != status)
		return status;

	// (**)in EQ access conf must be specific numeric id, not for factory
	const char* numericId = pCommResApi->GetNumericConfId();
	if (!pCommResApi->IsSIPFactory())
	{
		if (!pCommResApi->GetIsGateway() && (numericId == NULL || strlen(numericId) == 0))
		{
			PASSERT(1);
			PTRACE(eLevelError, "CReservator::CreateAdHoc_NoSchedule: non-specific numeric Id for AddHoc sent");
			return STATUS_FAIL;
			// (**) meaningfull status - TBD
		}
	}
	else
		pCommResApi->SetSIPFactory(FALSE);

	// numeric id allocation
	status = NIDAllocation_NoSchedule(pCommResApi, c_type);
	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::CreateAdHoc_WithSchedule(CCommResApi* pCommResApi, BOOL bUseOldConfId)
{
	// (**) monitor allocation
	// (**) numeic Id allocation
	// (**) Fill in CCommRes.

	STATUS status = STATUS_OK;

	ReservedConferences* pConfRsrvRsrcs = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrcs, STATUS_FAIL);
	PTRACE(eLevelInfoNormal,"CReservator::CreateAdHoc_WithSchedule");

	eRsrcConfType c_type = eConf_type_none;
	SetStartTimeIfStartImmediately(pCommResApi);

	status = CommonAllocationProcedureForNewConferencesOfAllTypes(pCommResApi, c_type, FALSE, FALSE, bUseOldConfId);
	//status = CommonAllocationProcedureForNewConferencesOfAllTypes(pCommResApi, c_type, FALSE, TRUE);// VNGR-25136 AdHoc conf is ongoing
	if (STATUS_OK != status)
		return status;

	// (**)in EQ access conf must be specific numeric id, not for factory
	if (!pCommResApi->IsSIPFactory())
	{
		const char* numericId = pCommResApi->GetNumericConfId();
		if (!pCommResApi->GetIsGateway() && (numericId == NULL || strlen(numericId) == 0))
		{
			status = STATUS_FAIL;  // (**) meaningfull status - TBD
			DBGPASSERT(1);
			PTRACE(eLevelError, "CReservator::CreateAdHoc_WithSchedule: non-specific numeric Id for AddHoc sent");
			RollbackAndPrintStatus("CreateAdHoc_WithSchedule 1", status, TRUE, FALSE, pCommResApi, c_type);
			return status;
		}
	}
	else
		pCommResApi->SetSIPFactory(FALSE);

	CConfRsrvRsrc* pConfRsrvRsrc = new CConfRsrvRsrc(pCommResApi);
	status = CreateReserveConference_WithSchedule(pCommResApi, pConfRsrvRsrc, c_type);
	if (STATUS_OK != status)
	{
		RollbackAndPrintStatus("CreateAdHoc_WithSchedule 2", status, TRUE, FALSE, pCommResApi, c_type);
		POBJDELETE(pConfRsrvRsrc);
		return status;
	}

	status = NIDAllocationAndResourcesCheckForConferences_WithSchedule(pConfRsrvRsrc, pCommResApi, c_type);

	if (STATUS_OK != status)
	{
		RollbackAndPrintStatus("CreateAdHoc_WithSchedule 3", status, TRUE, FALSE, pCommResApi, c_type);
		POBJDELETE(pConfRsrvRsrc);
		return status;
	}

	pConfRsrvRsrcs->insert(*pConfRsrvRsrc);
	POBJDELETE(pConfRsrvRsrc);
	return status;
}

//--------------------------------------------------------------------------
eRsrcConfType CReservator::GetConfType(CCommResApi* pCommResApi)
{
	if (pCommResApi->IsSIPFactory())
		return eSipFact_type;
	else if (pCommResApi->GetEntryQ())
		return eEQ_type;
	else if (pCommResApi->IsMeetingRoom())
		return eMR_type;

	return eConf_type_none;
}

//--------------------------------------------------------------------------
STATUS CReservator::CheckConfNameInResDB(CCommResApi* pCommResApi)
{
	if (TRUE == IsInternalReservator())
	{
		PASSERT_AND_RETURN_VALUE(m_pCentralConferencesDB->GetRsrvDB() == NULL, STATUS_FAIL);
		if (m_pCentralConferencesDB->GetRsrvDB())
		{
			if (m_pCentralConferencesDB->GetRsrvDB()->NameExists(pCommResApi->GetName()))
				return STATUS_RESERVATION_NAME_EXISTS;

			if (m_pCentralConferencesDB->GetRsrvDB()->DisplayNameExists(pCommResApi->GetDisplayName()))
				return STATUS_RESERVATION_NAME_EXISTS;
		}
		else
		{
			PASSERT_AND_RETURN_VALUE(true, STATUS_FAIL);
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CReservator::CheckIfOneMoreConferenceCanBeAdded(eRsrcConfType c_type, BOOL isOng)
{
	STATUS status = STATUS_OK;
	WORD num_conf = GetNumConfPerType(c_type);

	switch (c_type)
	{
		case eMR_type:
			if (num_conf >= m_MAX_NUMBER_OF_MRS)
				status = STATUS_MAX_RESERVATIONS_EXCEEDED;
			TRACEINTO << "num_conf:" << num_conf << ", max_num_mrs:" << m_MAX_NUMBER_OF_MRS << ", status:" << status;
			break;

		case eEQ_type:
			if (num_conf >= m_MAX_NUMBER_OF_EQS)
				status = STATUS_MAX_RESERVATIONS_EXCEEDED;
			TRACEINTO << "num_conf:" << num_conf << ", max_num_eqs:" << m_MAX_NUMBER_OF_EQS << ", status:" << status;
			break;

		case eSipFact_type:
			if (num_conf >= m_MAX_NUMBER_OF_SIP_FACTORIES)
				status = STATUS_MAX_RESERVATIONS_EXCEEDED;
			TRACEINTO << "num_conf:" << num_conf << ", max_num_sip:" << m_MAX_NUMBER_OF_SIP_FACTORIES << ", status:" << status;
			break;

		case eConf_type_none:
		{
			if (FALSE == IsInternalReservator())
			{
				CConfRsrcDB* pConfRsrcDB = m_pCentralConferencesDB->GetConfRsrcDB();
				PASSERT_AND_RETURN_VALUE(!pConfRsrcDB, STATUS_FAIL);

				CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
				PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

				WORD num_ong_conf     = pConfRsrcDB->GetNumConfRsrcs();
				WORD max_num_of_conf  = pSystemResources->GetMaxNumberOfOngoingConferences();
				if (max_num_of_conf < num_ong_conf +1)
					status = STATUS_MAX_RESERVATIONS_EXCEEDED;

				TRACEINTO << "max_num_of_conf:" << max_num_of_conf << ", num_ong_conf:" << num_ong_conf << ", status:" << status;
			}
			else
			{
				// when it's with reservator, we don't have to check the MAX_NUMBER_OF_ONGOING_CONFERENCES
				// because this is being checked in the intervals
				// what we do have to check is the total number of reservations
				if (FALSE == isOng)
				{
					CRsrvDB* pRsrvDB = m_pCentralConferencesDB->GetRsrvDB();
					PASSERT_AND_RETURN_VALUE(!pRsrvDB, STATUS_FAIL);
					DWORD num_res = pRsrvDB->GetResNumber();
					if (m_MAX_NUMBER_OF_RESERVATIONS < (num_res + 1))
						status = STATUS_MAX_RESERVATIONS_EXCEEDED;

					TRACEINTO << "max_num_reservation:" << m_MAX_NUMBER_OF_RESERVATIONS << ", num_res:" << num_res << ", status:" << status;
				}
				else
				{
					CConfRsrcDB* pConfRsrcDB = m_pCentralConferencesDB->GetConfRsrcDB();
					PASSERT_AND_RETURN_VALUE(!pConfRsrcDB, STATUS_FAIL);

					CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
					PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

					WORD num_ong_conf     = pConfRsrcDB->GetNumConfRsrcs();
					WORD max_num_of_conf  = pSystemResources->GetMaxNumberOfOngoingConferences();
					if (max_num_of_conf < num_ong_conf +1)
						status = STATUS_MAX_RESERVATIONS_EXCEEDED;

					TRACEINTO << "max_num_of_conf:" << max_num_of_conf << ", num_ong_conf:" << num_ong_conf << ", status:" << status;
				}
			}
			break;
		}

		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	} // switch

	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::UpdateRsrvFromAPIRequest(CCommResApi* pCommResApi)
{
	BOOL is_internal_reservator = IsInternalReservator();

	PASSERT_AND_RETURN_VALUE(is_internal_reservator == FALSE, STATUS_FAIL);
	PASSERT_AND_RETURN_VALUE(m_pCentralConferencesDB->GetRsrvDB() == NULL, STATUS_FAIL);

	DWORD monitorConfId = pCommResApi->GetMonitorConfId();

	CCommResApi*  pOldResApi = m_pCentralConferencesDB->GetRsrvDB()->GetRsrv(monitorConfId);
	PASSERT_AND_RETURN_VALUE(pOldResApi == NULL, STATUS_RESERVATION_NOT_EXISTS);

	STATUS status = STATUS_OK;

	eRsrcConfType c_type;
	status = DeleteResAllTypes(monitorConfId, &c_type, FALSE);  // FALSE : don't rereserve while deleting, because we don't realy delete. Rereserving will be done at the end
	if (status != STATUS_OK)
	{
		POBJDELETE(pOldResApi);
		return status;
	}
	bool bForwardTransaction = false;
	//TODO //BG decide about bForwardTransaction
	status = CreateRsrvFromAPI_WithSchedule(pCommResApi, bForwardTransaction, TRUE);
	if (status != STATUS_OK)
	{
		CreateRsrvFromAPI_WithSchedule(pOldResApi, bForwardTransaction, TRUE, TRUE);   // reinsert old one, force this one, because maybe we are updating "faulty" reservation
		POBJDELETE(pOldResApi);
		return status;
	}

	if (m_pCentralConferencesDB->GetRsrvDB() != NULL)
		status = m_pCentralConferencesDB->GetRsrvDB()->Update(*pCommResApi);
	else
		status = STATUS_FAIL;

	if (status != STATUS_OK)
	{
		// tbd zoe ??? some kind of rollback????
		POBJDELETE(pOldResApi);
		return status;
	}

	// maybe now there are more resources, so rereserve
	const CStructTm* start    = pOldResApi->GetStartTime();
	DWORD            oldstart = StructTm2IntervalForStart(start);
	const CStructTm* end      = pOldResApi->GetEndTime();
	DWORD            oldend   = StructTm2IntervalForEnd(end);
	Rereserve(eRereserveFaulty, oldstart, oldend);

	POBJDELETE(pOldResApi);
	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CReservator::DumpStartTimesOfRecurrences(std::set<CStructTm>* pTimeRepeatSched)
{
	std::set<CStructTm>::iterator i;
	CStructTm startTime;

	CLargeString* pStrInd = new CLargeString;
	*pStrInd <<" CReservator::DumpStartTimesOfRecurrences \n"
					 << "number of recurrences   =   "<< pTimeRepeatSched->size() <<'\n';

	int j = 0;
	for (i = pTimeRepeatSched->begin(); i != pTimeRepeatSched->end() && j <= 50; i++) // we will print only the first 50 occurences
	{
		startTime = *(i);
		startTime.m_mon++;                                                              // the values of the months are zero-based!!!

		*pStrInd << "\nstart time " << j++ << " : " << startTime;
	}

	PTRACE(eLevelInfoNormal, pStrInd->GetString());
	POBJDELETE(pStrInd);
}

//--------------------------------------------------------------------------
STATUS CReservator::CreateRepeatedRsrvFromAPIRequest(CCommResApi* pCommResApi, CComResRepeatDetails* pCommResRepeated, CCommResRecurrenceResponse* pResRecurrenceResponse)
{
	if (FALSE == IsInternalReservator())
		return STATUS_FAIL;                                                             // relevant only with internal scheduler

	// check NID in sleeping confs, for specific NID
	if (strlen(pCommResApi->GetNumericConfId()) != 0)
	{
		if (!strcmp(pCommResApi->GetNumericConfId(), "7001"))
		{
			PTRACE(eLevelInfoNormal, "CReservator::CreateRepeatedRsrvFromAPIRequest - Cannot create a Meeting Room with NID 7001 because it is used by the SIP Factory");
			return STATUS_NUMERIC_CONF_ID_OCCUPIED_BY_SIP_FACTORY;
		}
		else
		{
			if (NumericIdExistInSleepingConfs(pCommResApi->GetNumericConfId()))
				return STATUS_NUMERIC_CONFERENCE_ID_OCCUPIED;
		}
	}

	BOOL                  nRet      = FALSE;
	STATUS                retStatus = STATUS_OK, tempStatus;
	CRepeatedScheduleCalc SchedulCalc;
	CStructTm             duration(*(pCommResApi->GetDuration()));
	CStructTm             start(*(pCommResApi->GetStartTime()));

	start.m_mon--;

	if (pCommResRepeated->GetEndBy())
	{
		CStructTm limitTime = pCommResRepeated->GetLimitTime();
		limitTime.m_mon--;
		pCommResRepeated->SetLimitTime(limitTime);
	}
	else
	{
		//VNGFE-8896 - m_MAX_NUMBER_OF_RESERVATIONS
		if (((DWORD)pCommResRepeated->GetOccurNumber()) > m_MAX_NUMBER_OF_RESERVATIONS)
			return STATUS_MAX_RESERVATIONS_EXCEEDED;
	}

	SchedulCalc.SetReservationData(start, duration);
	SchedulCalc.SetRepetedDetails(pCommResRepeated);

	nRet = SchedulCalc.Calculate();

	if (nRet)
	{
		if (m_RepeatedReservationWriter.IsWriting() == TRUE) // this should very rarely happen, since it's an UI based operation
			return STATUS_FAIL;

		m_RepeatedReservationWriter.StartWriting();

		DWORD repeatedId = AllocateMntrConfId();
		pCommResApi->SetRepSchedulingId(repeatedId);

		std::set<CStructTm>* pTimeRepeatSched = SchedulCalc.GetStartTimes();
		PASSERT(pTimeRepeatSched == NULL);

		DumpStartTimesOfRecurrences(pTimeRepeatSched);

		std::set<CStructTm>::iterator i;
		CStructTm startTime;
		int       index      = 1;
		WORD      ok_res_num = 0;
		BOOL      firstFound = FALSE;
		CRsrvDB*  pRsrvDB    = m_pCentralConferencesDB->GetRsrvDB();

		PASSERT_AND_RETURN_VALUE(!pRsrvDB, STATUS_FAIL);

		WORD num_res = pRsrvDB->GetResNumber();
		CStructTm curTime;
		PASSERT(SystemGetTime(curTime));

		BYTE isOng = TRUE;

		for (i = pTimeRepeatSched->begin(); i != pTimeRepeatSched->end(); i++)
		{
			startTime = *(i);
			startTime.m_mon++;

			if (startTime.m_year < 1900)
				startTime.m_year += 1900;

			// S+O
			if (isOng && startTime <= curTime)
				isOng = TRUE;
			else
				isOng = FALSE;

			// v4.1c <--> V6 merge toDo: decide what is the correct (in 4.1c was MAX_RSRV_IN_LIST)
			//VNGFE-8896 - m_MAX_NUMBER_OF_RESERVATIONS
			if (!isOng && (m_MAX_NUMBER_OF_RESERVATIONS < (DWORD)(num_res + ok_res_num + 1)))      // if more than 1 group of recurrences reservations are added then
			{																					  // there is a possibility to add more than 2000 reservations at all.
				tempStatus = STATUS_MAX_RESERVATIONS_EXCEEDED;
			}
			else
				tempStatus = AddRecurrence(pCommResApi, startTime, index, firstFound);

			if (tempStatus != STATUS_OK)
			{
				char m_failedName[H243_NAME_LEN];
				BuildRecurrenceName(m_failedName, pCommResApi->GetName(), index);
				CCommResFailedRecurrence* pCommResFailedRecurrence = new CCommResFailedRecurrence(startTime, m_failedName, tempStatus);
				pResRecurrenceResponse->AddFailedRecurrence(pCommResFailedRecurrence);
			}
			else
			{
				if (!isOng)
					ok_res_num++;
			}
			index++;
		}

		CSingleToneApi rsrvManager(eProcessResource, "RsrvManager");
		rsrvManager.SendMsg(NULL, RESOURCE_WRITE_REPEATED_IND);
		rsrvManager.CleanUp();
	}
	else
	{
		retStatus = STATUS_FAIL;
	}

	pResRecurrenceResponse->SetCommRes(pCommResApi);

	return retStatus;
}

//--------------------------------------------------------------------------
void CReservator::BuildRecurrenceName(char* name, const char* originalName, int index)
{
	if (index < 10)
		sprintf(name, "%s_0000%d", originalName, index);
	else if (index >= 10 && index < 100)
		sprintf(name, "%s_000%d", originalName, index);
	else if (index >= 100 && index < 1000)
		sprintf(name, "%s_00%d", originalName, index);
	else if (index >= 1000 && index < 10000)
		sprintf(name, "%s_0%d", originalName, index);
	else
		sprintf(name, "%s_%d", originalName, index);
}

//--------------------------------------------------------------------------
STATUS CReservator::AddRecurrence(CCommResApi* pCommResApi, CStructTm& startTime, int index, BOOL& firstFound)
{
	CCommResApi* pConfToAdd = new CCommResApi;
	*pConfToAdd = *pCommResApi;
	pConfToAdd->SetStartTime(startTime);

	if (pConfToAdd->GetName()[0] != '\0')
		BuildRecurrenceName((char*)pConfToAdd->GetName(), pConfToAdd->GetName(), index);

	BuildRecurrenceName((char*)pConfToAdd->GetDisplayName(), pConfToAdd->GetDisplayName(), index);

	BOOL   bIsFirstOccurence = (index == 1);
	BOOL   bIsNonSpecific    = (strlen(pCommResApi->GetNumericConfId()) == 0);

	bool bForwardTransaction = false;
	STATUS ret               = CreateRsrvFromAPI_WithSchedule(pConfToAdd, bForwardTransaction, FALSE, FALSE,
	                                                          FALSE,                                // don't write to disk now
	                                                          bIsFirstOccurence && bIsNonSpecific); // check NID of MRs only if it's the first occurrence and if it's non-specific (because if it's specific, we already checked it)
	if (bIsFirstOccurence)                                                                          // first occurrence, take it's NID as NID for all
	{
		// We will take the NID (in case of non-specific) of the first occurrence, and put it in the other once
		// This is not a full and perfect solution, because there might be a collision of the NID because of another occurrence.
		// But meanwhile this solution is good enough (and this was the solution in MGC) because:
		// many times recurrences have a specific NID, and because the random number usually should be good enough

		if (bIsNonSpecific)
		{
			pCommResApi->SetNumericConfId(pConfToAdd->GetNumericConfId());
			if (pConfToAdd->GetName()[0] != '\0')
			{
				UpdateRoutingNameWithNID(pCommResApi);
				BuildRecurrenceName((char*)pConfToAdd->GetName(), pCommResApi->GetName(), index);
			}
		}

		m_RepeatedReservationWriter.SetBaseReservation(pCommResApi);
	}

	// ISDN phone treatment
	CServicePhoneStr* pPhoneStr = pCommResApi->GetFirstServicePhone(); // if status is not OK
	if (pPhoneStr != NULL && pPhoneStr->m_netServiceName[0] != '\0' && 0 == pPhoneStr->GetNumPhoneNumbers())
	{
		if (STATUS_OK == ret && !firstFound)
		{
			firstFound = TRUE;
			pCommResApi->AddServicePhone(*pConfToAdd->GetFirstServicePhone());
			m_RepeatedReservationWriter.SetBaseReservation(pCommResApi);
		}
	} // ISDN phone treatment

	if (ret == STATUS_OK)
	{
		m_RepeatedReservationWriter.AddRecurrence(index, pConfToAdd);
	}

	POBJDELETE(pConfToAdd)

	return ret;
}

//--------------------------------------------------------------------------
STATUS CReservator::CreateRsrvFromAPIRequest(CCommResApi* pCommResApi, bool& bForwardTransaction, DWORD* pProblemConfId)
{
	// If Internal Reservator OFF - old path
	// for treatment of conferences / meeting rooms is performed.
	// Otherwise - performed new ( full ) reservation path.

	bForwardTransaction = false;

	eRsrcConfType c_type = GetConfType(pCommResApi);

	TRACEINTO << "ConfName:" << pCommResApi->GetName() << ", ConfType:" << c_type << ", IsPermanent:" << pCommResApi->IsPermanent();

	if (FALSE == IsInternalReservator())
		return CreateConfFromAPI_NoSchedule(pCommResApi);
	else
	{
		if (eConf_type_none == c_type && pCommResApi->IsPermanent())
			return CreatePermanent_WithSchedule(pCommResApi, bForwardTransaction, pProblemConfId);

		return CreateRsrvFromAPI_WithSchedule(pCommResApi, bForwardTransaction);
	}
}

//--------------------------------------------------------------------------
STATUS CReservator::CreateOrUpdateSlaveRsrv(CCommResApi* pCommResApi)
{
	PASSERT_AND_RETURN_VALUE(eRPSlaveMode != GetRPMode(), STATUS_FAIL);

	PASSERT_AND_RETURN_VALUE(!IsInternalReservator(), STATUS_FAIL);

	// We need to check : it's new new reservation or update
	CRsrvDB* pRsrvDB = CHelperFuncs::GetRsrvDB();
	PASSERT_AND_RETURN_VALUE(!pRsrvDB, STATUS_FAIL);

	CCommResApi* pCRApi  = pRsrvDB->GetRsrv(pCommResApi->GetMonitorConfId());
	if (pCRApi)  // it means that it should be update
	{
		delete pCRApi;
		return UpdateRsrvFromAPIRequest(pCommResApi);
	}
	else
	{
		bool bForwardTransaction = false;
		return CreateRsrvFromAPI_WithSchedule(pCommResApi, bForwardTransaction);
	}
}

//--------------------------------------------------------------------------
STATUS CReservator::StartSlaveOngReq(CCommResApi* pCommResApi)
{
	// If Internal Reservator OFF - old path
	// for treatment of conferences / meeting rooms is performed.
	// Otherwise - performed new ( full ) reservation path.

	TRACEINTO;

	if (FALSE == IsInternalReservator())
	{
		DBGPASSERT(1);
		return CreateConfFromAPI_NoSchedule(pCommResApi);
	}
	else
	{
		ReservedConferences* pConfRsrvRsrcs = m_pCentralConferencesDB->GetConfRsrvRsrcs();
		PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrcs, STATUS_FAIL);

		STATUS        status        = STATUS_OK;
		char*         numericId     = (char*)(pCommResApi->GetNumericConfId());
		eRsrcConfType c_type        = GetConfType(pCommResApi);
		BYTE          isMeetingRoom = FALSE;
		BOOL          isOng         = FALSE;

		if (c_type != eConf_type_none)
			isMeetingRoom = TRUE;

		if (c_type == eConf_type_none)
		{
			CStructTm curTime;
			PASSERT(SystemGetTime(curTime));
			isOng = TRUE;
		}

		status = CommonAllocationProcedureForNewConferencesOfAllTypes(pCommResApi, c_type, TRUE, isOng);
		if (STATUS_OK != status)
			return status;

		if (isMeetingRoom != TRUE)
		{
			CConfRsrvRsrc* pConfRsrvRsrc = new CConfRsrvRsrc(pCommResApi);
			status = CreateReserveConference_WithSchedule(pCommResApi, pConfRsrvRsrc, c_type);
			if (STATUS_OK != status)
			{
				RollbackAndPrintStatus("StartSlaveOngReq 1", status, TRUE, FALSE, pCommResApi, c_type);
				POBJDELETE(pConfRsrvRsrc);
				return status;
			}

			if (!pConfRsrvRsrc->IsPermanent())
				status = NIDAllocationAndResourcesCheckForConferences_WithSchedule(pConfRsrvRsrc, pCommResApi, c_type);
			else
				status = NIDAllocationAndResourcesCheckForPermanent(pConfRsrvRsrc, pCommResApi, c_type);

			if (STATUS_OK != status)
			{
				RollbackAndPrintStatus("StartSlaveOngReq 2", status, TRUE, FALSE, pCommResApi, c_type);
				POBJDELETE(pConfRsrvRsrc);
				return status;
			}

			pConfRsrvRsrcs->insert(*pConfRsrvRsrc);
			POBJDELETE(pConfRsrvRsrc);
			pConfRsrvRsrc = NULL;
		}
		else // if meeting room type
		{
			status = NIDAllocationForSleepingConferences_WithSchedule(pCommResApi, c_type);
			if (STATUS_OK != status)
				return status;

			AddSleepingConference(pCommResApi->GetNumericConfId(), pCommResApi->GetMonitorConfId(), c_type);
			status = AllocationPhoneMR_WithSchedule(pCommResApi);
			if (STATUS_OK != status)
				return status;
		}

		return status;
	}
}

//--------------------------------------------------------------------------
STATUS CReservator::CreateRsrvFromMRRequest(CCommResApi* pCommResApi)
{
	// If Internal Reservator OFF - old path
	// for treatment of conferences / meeting rooms is performed.
	// Otherwise - performed new ( full ) reservation path.

	if (FALSE == IsInternalReservator())
		return CreateConfFromMR_NoSchedule(pCommResApi);
	else
		return RsrvFromMR_WithSchedule(pCommResApi);
}

//--------------------------------------------------------------------------
STATUS CReservator::CreateAdHocRequest(CCommResApi* pCommResApi, BOOL bUseOldConfId)
{
	// If Internal Reservator OFF - old path
	// for treatment of conferences / meeting rooms is performed.
	// Otherwise - performed new ( full ) reservation path.

	if (FALSE == IsInternalReservator())
		return CreateAdHoc_NoSchedule(pCommResApi);
	else
		return CreateAdHoc_WithSchedule(pCommResApi, bUseOldConfId);
}

//--------------------------------------------------------------------------
STATUS CReservator::CommonAllocationProcedureForNewConferencesOfAllTypes(CCommResApi* pCommResApi, eRsrcConfType c_type, BOOL bFromAPI, BOOL isOng, BOOL bUseOldConfId)
{
	// check number of conferences + name of conference in Reservation db (only in resDb because all other reservation name checks are done in confparty)
	// allocate monitor id
	// (when necessary) deal with min_parties

	STATUS status = STATUS_OK;

	TRACEINTO << "ConfType:"  << c_type << ", bFromAPI:" << (WORD)bFromAPI << ", isOngoing:" << (WORD)isOng;
	if (m_pCentralConferencesDB)
		m_pCentralConferencesDB->DumpUsedSize();

	// check the appropriate number of conferences
	// if it's an EQ that goes to ongoing, allow it always
	if (!(bFromAPI == FALSE && pCommResApi->GetEntryQ() == YES))
	{
		status = CheckIfOneMoreConferenceCanBeAdded(c_type, isOng);
		if (STATUS_OK != status)
			return status;
	}

	status = CheckConfNameInResDB(pCommResApi);
	if (STATUS_OK != status)
		return status;

	// (**) monitor id allocation

	DWORD monitor_Id = ((eRPSlaveMode != GetRPMode()) && !bUseOldConfId) ? AllocateMntrConfId() : pCommResApi->GetMonitorConfId();

	if (eRPSlaveMode != GetRPMode() && monitor_Id == 0) // internal allocation - '0' is failure
	{
		status = STATUS_INSUFFICIENT_MNTR_CONF_ID;
		PTRACE2INT(eLevelInfoNormal, "CReservator::CommonAllocationProcedureForNewConferencesOfAllTypes, monitor_id problem. status:", status);
		return status;
	}

	if (eRPSlaveMode == GetRPMode())
		UpdateMntrConfId(monitor_Id);

	pCommResApi->SetMonitorConfId(monitor_Id);

	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::AllocationProcedureForAPIConferences_NoSchedule(CCommResApi* pCommResApi, eRsrcConfType c_type)
{
	// PSTN phones allocation
	// PlusNumConfPerType
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	STATUS status = STATUS_OK;
	DWORD  monitor_Id = pCommResApi->GetMonitorConfId();

	// *** PSTN phones allocation
	if (pCommResApi->IsEnableIsdnPstnAccess())
	{
		CSleepingConference* pConf     = NULL;
		CConfRsrvRsrc*       pRsrvConf = NULL;

		if (eEQ_type == c_type || eMR_type == c_type)
		{
			pConf = (CSleepingConference*)GetSleepingConference(monitor_Id);
		}
		else if (eConf_type_none == c_type)
		{
			pRsrvConf = (CConfRsrvRsrc*)GetRegularConference(monitor_Id);
		}

		if (pConf || pRsrvConf)
		{
			WORD count = 0;                                // for roll-back
			CServicePhoneStr* pPhoneStr = pCommResApi->GetFirstServicePhone();

			while (pPhoneStr != NULL && pPhoneStr->m_netServiceName[0] != '\0') // SPECIFIC AND NON SPECIFIC CASE
			{
				status = pSystemResources->AllocateServicePhones(*pPhoneStr); // roll-back

				if (status == STATUS_OK)
				{
					if (pConf)
						pConf->AddServicePhone(*pPhoneStr);
					else if (pRsrvConf)
						pRsrvConf->AddServicePhone(*pPhoneStr);
				}
				else                                                          // roll-back
				{
					pPhoneStr = pCommResApi->GetFirstServicePhone();

					while (count > 0)
					{
						pSystemResources->DeAllocateServicePhones(*pPhoneStr);
						pPhoneStr = pCommResApi->GetNextServicePhone();
						count--;
					}
					break;                                                      // exit prime loop
				} // roll-back end

				pPhoneStr = pCommResApi->GetNextServicePhone();
				count++; // for roll-back
			}

			// (**) rollback monitor id and numeric_Id
			if (status != STATUS_OK)
			{
				RollbackAndPrintStatus("AllocationProcedureForAPIConferences_NoSchedule", status, TRUE, TRUE, pCommResApi, c_type);
				return status;
			}
			// (**) rollback monitor id and numeric_Id END
		}
	}

	PlusNumConfPerType(c_type);
	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::AllocationPhoneMR_WithSchedule(CCommResApi* pCommResApi, BOOL isUpdate)
{
	eRsrcConfType c_type = GetConfType(pCommResApi);

	// NOTE: this function should be called only for MR and EQ
	// do PSTN phones allocation & PlusNumConfPerType
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	STATUS status = STATUS_OK;
	DWORD  monitor_Id = pCommResApi->GetMonitorConfId();

	if ((eEQ_type == c_type || eMR_type == c_type) && pCommResApi->IsEnableIsdnPstnAccess())  // NETWORK_H320_H323 == pCommResApi->GetNetwork() )
	{
		CSleepingConference* pConf = (CSleepingConference*)(GetSleepingConference(monitor_Id));
		if (!pConf)
			return STATUS_FAIL; // olga ???

		CServicePhoneStr* pPhoneStr = pCommResApi->GetFirstServicePhone();
		BOOL isSpecific = TRUE;

		if (pPhoneStr != NULL && pPhoneStr->m_netServiceName[0] != '\0') // SPECIFIC AND NON SPECIFIC CASE
		{
			char* serviceName = pPhoneStr->m_netServiceName;
			const CNetServiceRsrcs* pExistService = pSystemResources->findServiceByName(serviceName);
			if (pExistService)
			{
				// 1. Insert EQ/MR list + Service Name
				m_ReservationCalculator.SetNetServiceName(serviceName);
				m_ReservationCalculator.SetPhonesList(*pExistService->GetPhonesList());

				// 2. Check with existing EQ and MR( if SPECIFIC )
				Phone* phone = pPhoneStr->GetFirstPhoneNumber();
				isSpecific = (NULL == phone) ? FALSE :  TRUE;
				if (isSpecific)
				{
					status = m_ReservationCalculator.PhoneCheckOrAlloc(pPhoneStr, ePhoneCheck);
				}

				// 3. Add all reservations
				if (STATUS_OK == status)
				{
					ReservedConferences* pConfRsrvRsrc = m_pCentralConferencesDB->GetConfRsrvRsrcs();
					ReservedConferences::iterator confRsrvIterator;

					for (confRsrvIterator = pConfRsrvRsrc->begin(); confRsrvIterator != pConfRsrvRsrc->end(); confRsrvIterator++)
					{
						if (confRsrvIterator->GetStatus() != STATUS_OK || confRsrvIterator->GetFromPassive())
							continue;

						CIntervalRsrvAmount confNeededAmount;
						((CConfRsrvRsrc)(*confRsrvIterator)).GetConferenceNeededAmount(confNeededAmount, TRUE);

						CServicePhoneStr* pConfRsrvPhoneStr = (CServicePhoneStr*) confNeededAmount.GetServicePhone();

						status = m_ReservationCalculator.PhoneCheckOrAlloc(pConfRsrvPhoneStr, ePhoneCheckAlloc);  // temporary phone's allocation
						if (STATUS_OK != status)
							break;
					}
				}

				// 4. Allocate
				if (STATUS_OK == status)
				{
					if (!isSpecific)
					{
						status = m_ReservationCalculator.ReservePhoneIfNeeded(pPhoneStr, NULL);                   // NULL because CSleepingConference isn't CConfRsrvRsrc
					}
					else
					{
						status = m_ReservationCalculator.PhoneCheckOrAlloc(pPhoneStr, ePhoneCheck);
					}

					if (STATUS_OK == status)
					{
						status = pSystemResources->AllocateServicePhones(*pPhoneStr);                             // update CCOMRESAPI - NOTE: no need to check twice!!!
						if (STATUS_OK == status)
							pConf->AddServicePhone(*pPhoneStr);                                                     // update sleeping conf - if returned STATUS should be checked?
						}
				}
				m_ReservationCalculator.Cleanup();
			}
		}

		// (**) rollback monitor id and numeric_Id
		if (status != STATUS_OK && !isUpdate)
		{
			RollbackAndPrintStatus("AllocationPhoneMR_WithSchedule", status, TRUE, TRUE, pCommResApi, c_type);
			return status;
		} // (**) rollback monitor id and numeric_Id END
	}

	if (!isUpdate)
		PlusNumConfPerType(c_type);

	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::NIDAllocation_NoSchedule(CCommResApi* pCommResApi, eRsrcConfType c_type)
{
	STATUS status = STATUS_OK;

	const char* numericId = pCommResApi->GetNumericConfId();
	status = AllocateNumericConfId((char*)numericId);
	if (status == STATUS_OK)
	{
		if (c_type == eConf_type_none)
			AddRegularConference(pCommResApi);
		else
			AddSleepingConference(pCommResApi->GetNumericConfId(), pCommResApi->GetMonitorConfId(), c_type);
	}

	// (**) rollback monitor id
	if (status != STATUS_OK)
	{
		RollbackAndPrintStatus("NIDAllocation_NoSchedule", status, TRUE, FALSE, pCommResApi, c_type);
		return status;
	}

	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::NIDAllocationForSleepingConferences_WithSchedule(CCommResApi* pCommResApi, eRsrcConfType c_type)
{
	STATUS status = STATUS_OK;
	STATUS ret_status = STATUS_OK;

	ReservedConferences* pConfRsrvRsrc = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrc, STATUS_FAIL);

	SleepingConferences* pSleepingConferences = m_pCentralConferencesDB->GetSleepingConferences();
	PASSERT_AND_RETURN_VALUE(!pSleepingConferences, STATUS_FAIL);

	// add all reservation NIDs
	CIntervalRsrvAmount confNeededAmount;
	confNeededAmount.m_bIsRealConference = FALSE; // we don't check the number of conferences here, nor their time

	DWORD ipServiceIdForMinParties = GetIpServiceIdForMinParties(pCommResApi);
	for (ReservedConferences::iterator _ii = pConfRsrvRsrc->begin(); _ii != pConfRsrvRsrc->end(); ++_ii)
	{
		confNeededAmount.SetNID(_ii->GetNID());// we don't check the status here, because it might be that different conferences have the same NID
		m_ReservationCalculator.ReserveAmount(0, 0, confNeededAmount, _ii->GetFromPassive(), FALSE, ePhoneAlloc, ipServiceIdForMinParties);
	}

	// add NIDs from system resources, for meeting rooms and from ongoing conferences
	for (SleepingConferences::iterator _ii = pSleepingConferences->begin(); _ii != pSleepingConferences->end(); ++_ii)
	{
		confNeededAmount.SetNID(_ii->GetNumConfId());
		m_ReservationCalculator.ReserveAmount(0, 0, confNeededAmount, FALSE, FALSE, ePhoneAlloc, ipServiceIdForMinParties);
	}

	if (strlen(pCommResApi->GetNumericConfId()) == 0)
	{
		char* resNumConfId = new char[NUMERIC_CONF_ID_MAX_LEN+1];
		status = FindFreeNID(resNumConfId);
		if (status == STATUS_OK)
		{
			pCommResApi->SetNumericConfId(resNumConfId);
			UpdateRoutingNameWithNID(pCommResApi);
		}
		else
			ret_status = status;

		delete [] resNumConfId;
	}
	else
	{
		char* resNumConfId = (char*)pCommResApi->GetNumericConfId();
		status = CheckNIDValidity(resNumConfId);
		if (status != STATUS_OK)
		{
			ret_status = status;
		}
		else
		{
			confNeededAmount.SetNID(resNumConfId);
			DWORD ipServiceIdForMinParties = GetIpServiceIdForMinParties(pCommResApi);
			status = m_ReservationCalculator.ReserveAmount(0, 0, confNeededAmount, FALSE, TRUE, ePhoneAlloc, ipServiceIdForMinParties);
			if (status != STATUS_OK)
				ret_status = status;
		}
	}

	if (ret_status != STATUS_OK)
	{
		RollbackAndPrintStatus("NIDAllocationForSleepingConferences_WithSchedule", ret_status, TRUE, FALSE, pCommResApi, c_type);
	}

	m_ReservationCalculator.Cleanup();

	return ret_status;
}

//--------------------------------------------------------------------------
STATUS CReservator::ReReserveOneConference(CConfRsrvRsrc* pconf)
{
	STATUS status = STATUS_OK;
	eRsrcConfType c_type = eConf_type_none;

	DWORD start     = pconf->GetStartInterval();
	DWORD end       = pconf->GetEndInterval();
	DWORD monitorid = pconf->GetMonitorConfId();

	InitRsrvCalculatorPhoneList(pconf);

	BYTE  isMultipleService = CHelperFuncs::IsMultipleService();
	DWORD ipServiceId       = pconf->GetIpServiceIdForMinParties();
	if (isMultipleService)
	{
		status = CreateReserveInterval(start, end, monitorid, monitorid, TRUE, ipServiceId);
	}
	else
	{
		status = CreateReserveInterval(start, end, monitorid, monitorid);
	}

	if (status != STATUS_OK)
	{
		m_ReservationCalculator.Cleanup();
		return status;  // assert/trace?
	}

	CIntervalRsrvAmount confNeededAmount;
	pconf->GetConferenceNeededAmount(confNeededAmount);
	BOOL isFromPassive = pconf->GetFromPassive();
	status = m_ReservationCalculator.ReserveAmount(start, end, confNeededAmount, isFromPassive, FALSE, ePhoneAlloc, ipServiceId);

	m_ReservationCalculator.Cleanup();
	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::NIDAllocationAndResourcesCheckForConferences_WithSchedule(CConfRsrvRsrc* pconf, CCommResApi* pCommResApi, eRsrcConfType c_type, BOOL is_Startup, BOOL bAlsoCheckNIDsOfMRs)
{
	STATUS status = STATUS_OK;

	DWORD start = pconf->GetStartInterval();
	DWORD end   = pconf->GetEndInterval();

	TRACEINTO << "MonitorConfId:" << pconf->GetMonitorConfId() << ", Start:" << start << ", End:" << end << ", ConfType:" << c_type << ", IsStartup:" << (int)is_Startup << ", IsCheckNIDsOfMRs:" << (int)bAlsoCheckNIDsOfMRs;

	if (FALSE == is_Startup)
	{
		char* resNumConfId = (char*)pCommResApi->GetNumericConfId();
		if (NULL != resNumConfId && strlen(resNumConfId) != 0)
		{
			status = CheckNIDValidity(resNumConfId);
			if (status != STATUS_OK)
				return status;
		}
	}

	InitRsrvCalculatorPhoneList(pconf);

	BYTE  isMultipleService = CHelperFuncs::IsMultipleService();
	DWORD ipServiceId = GetIpServiceIdForMinParties(pCommResApi);
	if (isMultipleService)
	{
		status = CreateReserveInterval(start, end, 0, 0, bAlsoCheckNIDsOfMRs, ipServiceId);
	}
	else
	{
		status = CreateReserveInterval(start, end, 0, 0, bAlsoCheckNIDsOfMRs);
	}

	if (status != STATUS_OK)
	{
		m_ReservationCalculator.Cleanup();
		RollbackAndPrintStatus("NIDAllocationAndResourcesCheckForConferences_WithSchedule 1", status, TRUE, FALSE, pCommResApi, c_type);
		return status;                                                          // assert/trace?
	}

	CIntervalRsrvAmount confNeededAmount;
	pconf->GetConferenceNeededAmount(confNeededAmount);
	BOOL isFromPassive = pconf->GetFromPassive();
	status = m_ReservationCalculator.ReserveAmount(start, end, confNeededAmount, isFromPassive, TRUE, ePhoneAlloc, ipServiceId);
	if (status != STATUS_OK)
	{
		m_ReservationCalculator.Cleanup();
		RollbackAndPrintStatus("NIDAllocationAndResourcesCheckForConferences_WithSchedule 2", status, TRUE, FALSE, pCommResApi, c_type);
		return status;                                                          // assert/trace?
	}

	// if phone should be allocated => set the phone to pCommResApi
	CServicePhoneStr* pPhoneStr = pCommResApi->GetFirstServicePhone();
	status = m_ReservationCalculator.ReservePhoneIfNeeded(pPhoneStr, pconf);  // olga

	// if NID is empty, find a free one
	if (status == STATUS_OK && (pconf->GetNID() == NULL || strlen(pconf->GetNID()) == 0))
	{
		char* resNumConfId =  new char[NUMERIC_CONF_ID_MAX_LEN+1];
		status = FindFreeNID(resNumConfId);
		if (status == STATUS_OK)
		{
			pconf->SetNID(resNumConfId);
			pCommResApi->SetNumericConfId(resNumConfId);
			UpdateRoutingNameWithNID(pCommResApi);
		}

		delete [] resNumConfId;
	}

	m_ReservationCalculator.Cleanup();

	if (status != STATUS_OK)
	{
		RollbackAndPrintStatus("NIDAllocationAndResourcesCheckForConferences_WithSchedule 3", status, TRUE, FALSE, pCommResApi, c_type);
		return status;
	}

	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::NIDAllocationAndResourcesCheckForPermanent(CConfRsrvRsrc* pconf, CCommResApi* pCommResApi, eRsrcConfType c_type, BOOL is_Startup, BOOL bAlsoCheckNIDsOfMRs, DWORD* pProblemConfId)
{
	TRACEINTO << "ConfType:" << c_type << ", IsStartup:" << (int)is_Startup << ", IsCheckNIDsOfMRs:" << (int)bAlsoCheckNIDsOfMRs;

	STATUS status        = STATUS_OK;
	DWORD  problemConfId = 0;
	WORD   temp_weight   = 0;

	if (FALSE == is_Startup)
	{
		char* resNumConfId = (char*)pCommResApi->GetNumericConfId();
		if (NULL != resNumConfId && strlen(resNumConfId) != 0)
		{
			status = CheckNIDValidity(resNumConfId);
			if (status != STATUS_OK)
				return status;
		}
	}

	if (bAlsoCheckNIDsOfMRs)
		status = NIDAllocationForSleepingConferences_WithSchedule(pCommResApi, eConf_type_none); // c_type?

	if (STATUS_OK == status)
	{
		CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
		PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

		WORD min_num_parties_in_permanent_conf = pconf->GetMinNumVideoParties(), max_num_conf = 0;
		if (!CHelperFuncs::IsMode2C())
		{
			min_num_parties_in_permanent_conf = pconf->GetLogicalWeightOfMinNumVideoParties();
			max_num_conf = pSystemResources->GetMaxNumberOfOngoingConferences();
		}
		else
		{
			if (pCommResApi->GetIsHDVSW())
				max_num_conf = pSystemResources->GetMaxNumberVSWConferencesEventMode();
			else
				max_num_conf = GetMaxNumberOngoingConferences2C();
		}

		TRACEINTO << "min_permanent:" << min_num_parties_in_permanent_conf << ", max_conf_num:" << max_num_conf;

		status = CalculateMaxWeightOfAllReservation(min_num_parties_in_permanent_conf, max_num_conf, pCommResApi->GetIsHDVSW(), problemConfId);

		if (STATUS_OK == status)
			pconf->SetNID(pCommResApi->GetNumericConfId());
		else if (pProblemConfId)
			*pProblemConfId = problemConfId;
	}

	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::CreateReserveConference_WithSchedule(CCommResApi* pCommResApi, CConfRsrvRsrc* pConfRsrvRsrc, eRsrcConfType c_type)
{
	STATUS status = STATUS_OK;
	ConfMonitorID confId = pCommResApi->GetMonitorConfId();

	TRACEINTO << "ConfName:" << pCommResApi->GetDisplayName() << ", MonitorConfId:" << confId;

	CConfRsrvRsrc* pConf = (CConfRsrvRsrc*)GetConfRsrvRsrcById(confId);
	if (pConf != NULL)
	{
		PASSERTSTREAM(confId, "CReservator::CreateReserveConference_WithSchedule - Failed, Conference already exists, MonitorConfId:" << confId);
		status = STATUS_FAIL;
		RollbackAndPrintStatus("CreateReserveConference_WithSchedule 1", status, TRUE, FALSE, pCommResApi, c_type);
		return status;
	}

	status = pConfRsrvRsrc->FillFieldsNeededForReservator(pCommResApi);
	if (status != STATUS_OK)
	{
		RollbackAndPrintStatus("CreateReserveConference_WithSchedule 2", status, TRUE, FALSE, pCommResApi, c_type);
		return status;
	}

	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::NIDCheckExist(CCommResApi* pCommResApi, eRsrcConfType c_type)
{
	STATUS status = STATUS_OK;

	const char* numericId = pCommResApi->GetNumericConfId();
	if (NumericIdExist(numericId) == FALSE) // MR numeric id previously allocated and must exist
	{
		status = STATUS_NUMERIC_CONFERENCE_ID_NOT_EXISTS;
		PASSERTSTREAM(1, "CReservator::NIDCheckExist - Failed, status:" << status);
	}

	if (status != STATUS_OK)
	{
		RollbackAndPrintStatus("NIDCheckExist", status, TRUE, FALSE, pCommResApi, c_type);
		return status;
	}

	return status;
}

//--------------------------------------------------------------------------
void CReservator::RollbackAndPrintStatus(char* strFromWhere, STATUS status, BOOL bRollBackMonitorConfId, BOOL bRemoveConf, CCommResApi* pCommResApi, eRsrcConfType c_type)
{
	TRACEINTO << "ConfName:" << pCommResApi->GetName() << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status) << " - Called from " << strFromWhere;

	if (bRollBackMonitorConfId)
		DeAllocateMntrConfId(pCommResApi->GetMonitorConfId());

	if (bRemoveConf)
		DeleteResAllTypes(pCommResApi->GetMonitorConfId(), &c_type);
}

//--------------------------------------------------------------------------
STATUS CReservator::FindFreeNID(char resNumConfId[NUMERIC_CONF_ID_MAX_LEN+1])
{
	STATUS status = STATUS_OK;

	CIntervalRsrvAmount nidAmount;
	nidAmount.m_bIsRealConference = FALSE;

	WORD count;

	for (count = 0; count < NUM_NON_SPECIFIC_NUM_CONF_ID_ALLOC_TRY; count++)
	{
		status = FillRandomNID(resNumConfId);
		if (status != STATUS_OK)
			break;

		nidAmount.SetNID(resNumConfId);
		status = m_ReservationCalculator.ReserveAmount(0, 0, nidAmount, FALSE, TRUE);
		if (status == STATUS_OK)
			break;
		else
			TRACEINTO << "Failed, Non-specific numeric id allocation try failed for: " << resNumConfId;
	} // end for try

	if (count == NUM_NON_SPECIFIC_NUM_CONF_ID_ALLOC_TRY)
		status = STATUS_LACK_OF_AVAILABLE_NUMERIC_ID;

	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::CheckNIDValidity(char* numConfId)
{
	if (numConfId == NULL)
	{
		DBGPASSERT(1);
		return STATUS_ILLEGAL;
	}

	WORD length = strlen(numConfId);
	if ((length < m_num_conf_id_min_len || length > m_num_conf_id_max_len))
	{
		TRACEINTO << "Failed, illegal NID length:" << length;
		return STATUS_ILLEGAL_CONFERENCE_ID_LENGTH;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CReservator::AllocateNumericConfId(char* numConfId, BYTE is_Sturtup)
{
	// EXPECTED CALL PARAMETER

	// *** numConfId = NULL - illegal
	// *** numConfId empty - non-specific
	// *** numConfId filled - specific

	if (numConfId == NULL)
	{
		DBGPASSERT(1);
		return STATUS_ILLEGAL;
	}

	STATUS status = STATUS_OK;

	WORD length = strlen(numConfId);

	if (length != 0)                                    // specific numeric id
	{
		if (!is_Sturtup)
			status = CheckNIDValidity(numConfId);

		if (status == STATUS_OK)
		{
			if (NumericIdExist(numConfId) == TRUE)
				status = STATUS_NUMERIC_CONFERENCE_ID_OCCUPIED;
		}
	}
	else                                                // non-specific numeric id
		status = AllocateNonSpecificNumericId(numConfId); // fill info into numConfId

	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::DeleteResAllTypes(DWORD monitorId, eRsrcConfType* conf_type, BOOL bReReserveOthers)
{
	if (monitorId == 0xFFFFFF)
	{
		DBGPASSERT(1);
		return STATUS_ILLEGAL;
	}

	STATUS status = STATUS_FAIL;

	DeAllocateMntrConfId(monitorId);

	SleepingConferences* pSleepingConferences = m_pCentralConferencesDB->GetSleepingConferences();
	PASSERT_AND_RETURN_VALUE(!pSleepingConferences, STATUS_FAIL);

	ReservedConferences* pConfRsrvRsrcs = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrcs, STATUS_FAIL);

	for (SleepingConferences::iterator _ii = pSleepingConferences->begin(); _ii != pSleepingConferences->end(); _ii++)
	{
		if (_ii->GetMonitorConfId() == monitorId)  // found
		{
			TRACEINTO << "MonitorConfId:" << monitorId << ", ReReserveOthers:" << (int)bReReserveOthers << " - Found in CSleepingConference";

			*conf_type = (eRsrcConfType)(_ii->GetConfType());
			pSleepingConferences->erase(_ii);

			if (bReReserveOthers)
				// maybe now an occupied NID is free
				Rereserve(eRereserveFaulty);

			status = STATUS_OK;
			break;
		}
	}

	if (status != STATUS_OK)
	{
		for (ReservedConferences::iterator _ii = pConfRsrvRsrcs->begin(); _ii != pConfRsrvRsrcs->end(); _ii++)
		{
			if (_ii->GetMonitorConfId() == monitorId)  // found
			{
				TRACEINTO << "MonitorConfId:" << monitorId << ", ReReserveOthers:" << (int)bReReserveOthers << " - Found in CConfRsrvRsrc";

				*conf_type = eConf_type_none;
				DWORD start = _ii->GetStartInterval();
				DWORD end   = _ii->GetEndInterval();
				pConfRsrvRsrcs->erase(_ii);

				if (bReReserveOthers)
					// maybe now some resources are free
					Rereserve(eRereserveFaulty, start, end);

				status = STATUS_OK;
				break;
			}
		}
	}

	TRACEINTO << "MonitorConfId:" << monitorId << ", sleepingConferencesSize:" << pSleepingConferences->size() << ", confRsrvRsrcsSize:" << pConfRsrvRsrcs->size() << " - Failed";
	return status;
}

//--------------------------------------------------------------------------
BOOL CReservator::NumericIdExist(const char* numConfId)
{
	if (NumericIdExistInSleepingConfs(numConfId) == TRUE)
		return TRUE;

	ReservedConferences* pConfRsrvRsrcs = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrcs, FALSE);

	ReservedConferences::iterator confRsrvIterator;
	for (confRsrvIterator = pConfRsrvRsrcs->begin(); confRsrvIterator != pConfRsrvRsrcs->end(); confRsrvIterator++)
		if (strcmp(confRsrvIterator->GetNID(), numConfId) == 0) // identical
			return TRUE;

	return FALSE;
}

//--------------------------------------------------------------------------
BOOL CReservator::NumericIdExistInSleepingConfs(const char* numConfId)
{
	SleepingConferences* pSleepingConferences = m_pCentralConferencesDB->GetSleepingConferences();
	PASSERT_AND_RETURN_VALUE(!pSleepingConferences, FALSE);

	SleepingConferences::iterator i;
	for (i = pSleepingConferences->begin(); i != pSleepingConferences->end(); i++)
		if (strcmp(i->GetNumConfId(), numConfId) == 0)          // identical
			return TRUE;

	return FALSE;
}

//--------------------------------------------------------------------------
char* CReservator::GetNumericIdbyMonitorId(DWORD monitorId)
{
	SleepingConferences* pSleepingConferences = m_pCentralConferencesDB->GetSleepingConferences();
	PASSERT_AND_RETURN_VALUE(!pSleepingConferences, NULL);

	ReservedConferences* pConfRsrvRsrcs = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrcs, NULL);

	SleepingConferences::iterator i;
	for (i = pSleepingConferences->begin(); i != pSleepingConferences->end(); i++)
		if (i->GetMonitorConfId() == monitorId)                 // identical
			return i->GetNumConfId();

	ReservedConferences::iterator confRsrvIterator;
	for (confRsrvIterator = pConfRsrvRsrcs->begin(); confRsrvIterator != pConfRsrvRsrcs->end(); confRsrvIterator++)
		if (confRsrvIterator->GetMonitorConfId() == monitorId)  // identical
			return (char*)confRsrvIterator->GetNID();

	return NULL;
}

//--------------------------------------------------------------------------
const CConfRsrvRsrc* CReservator::GetRegularConference(const DWORD monitorId)
{
	ReservedConferences* pConfRsrvRsrcs = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrcs, NULL);

	ReservedConferences::iterator confRsrvIterator;
	for (confRsrvIterator = pConfRsrvRsrcs->begin(); confRsrvIterator != pConfRsrvRsrcs->end(); confRsrvIterator++)
		if (confRsrvIterator->GetMonitorConfId() == monitorId)  // identical
			return &(*confRsrvIterator);

	return NULL;
}

//--------------------------------------------------------------------------
const CSleepingConference* CReservator::GetSleepingConference(DWORD monitorId)
{
	SleepingConferences* pSleepingConferences = m_pCentralConferencesDB->GetSleepingConferences();
	PASSERT_AND_RETURN_VALUE(!pSleepingConferences, NULL);

	SleepingConferences::iterator i;
	for (i = pSleepingConferences->begin(); i != pSleepingConferences->end(); i++)
		if (i->GetMonitorConfId() == monitorId)                 // identical
			return &(*i);

	return NULL;
}

//--------------------------------------------------------------------------
STATUS CReservator::AddSleepingConference(const char* numConfId, DWORD monitorId, eRsrcConfType conf_type)
{
	SleepingConferences* pSleepingConferences = m_pCentralConferencesDB->GetSleepingConferences();
	PASSERT_AND_RETURN_VALUE(!pSleepingConferences, STATUS_FAIL);

	// check collision - both monitor and numeric;
	if (MonitorIdExist(monitorId) == TRUE || NumericIdExist(numConfId) == TRUE){
		PASSERTSTREAM_AND_RETURN_VALUE(1, "monitorId:" << monitorId << ", ConfId:" << numConfId, STATUS_ILLEGAL);
	}

	CSleepingConference* pConfId = new CSleepingConference();
	pConfId->SetMonitorConfId(monitorId);
	pConfId->SetNumConfId((char*)numConfId);
	pConfId->SetConfType(conf_type);

	pSleepingConferences->insert(*pConfId);

	POBJDELETE(pConfId);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CReservator::AddRegularConference(CCommResApi* pCommResApi)
{
	ReservedConferences* pConfRsrvRsrcs = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrcs, STATUS_FAIL);

	const char* numConfId = pCommResApi->GetNumericConfId();
	DWORD       monitorId = pCommResApi->GetMonitorConfId();
	// check collision - both monitor and numeric;
	if (MonitorIdExist(monitorId) == TRUE || NumericIdExist(numConfId) == TRUE)
	{
		DBGPASSERT(1);
		return STATUS_ILLEGAL;
	}

	CConfRsrvRsrc* pConfRsrvRsrc = new CConfRsrvRsrc(pCommResApi);
	pConfRsrvRsrcs->insert(*pConfRsrvRsrc);
	POBJDELETE(pConfRsrvRsrc);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CReservator::UpdateSleepingConference(const char* numConfId, DWORD monitorId, eRsrcConfType conf_type)
{
	// this function is called only when the NID has been changed...

	STATUS status  = STATUS_OK;

	CSleepingConference* pConfId = (CSleepingConference*)GetSleepingConference(monitorId);

	if (pConfId)
	{
		pConfId->SetNumConfId((char*)numConfId);
		pConfId->SetConfType(conf_type);
	}
	else
	{
		DBGPASSERT(monitorId);
		status = STATUS_FAIL;
	}

	// rereserve faulty reservations, because maybe now their NID is free
	Rereserve(eRereserveFaulty);
	return status;
}

//--------------------------------------------------------------------------
// Return the number of occupied numeric Ids
// Using length to give the number of occupied numeric Ids of a given length
size_t CReservator::NumericIdCount(WORD length)
{
	SleepingConferences* pSleepingConferences = m_pCentralConferencesDB->GetSleepingConferences();
	PASSERT_AND_RETURN_VALUE(!pSleepingConferences, 0);

	ReservedConferences* pConfRsrvRsrcs = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrcs, FALSE);

	size_t count = 0;

	if (0 != length)
	{
		SleepingConferences::iterator i;
		for (i = pSleepingConferences->begin(); i != pSleepingConferences->end(); i++)
			if (length == strlen(i->GetNumConfId()))
				count++;

		ReservedConferences::iterator confRsrvIterator;
		for (confRsrvIterator = pConfRsrvRsrcs->begin(); confRsrvIterator != pConfRsrvRsrcs->end(); confRsrvIterator++)
			if (length == strlen(confRsrvIterator->GetNID()))
				count++;
	}
	else
	{
		count +=  pSleepingConferences->size();
		count +=  pConfRsrvRsrcs->size();
	}

	return count;
}

//--------------------------------------------------------------------------
STATUS CReservator::AllocateNonSpecificNumericId(char* numConfId)
{
	// ***non-specific numeric id should be of m_num_conf_id_len

	if (numConfId == NULL)
	{
		DBGPASSERT(1);
		return STATUS_ILLEGAL;
	}

	char* resNumConfId = new char[NUMERIC_CONF_ID_MAX_LEN+1];

	STATUS status = STATUS_LACK_OF_AVAILABLE_NUMERIC_ID;
	STATUS tempStatus;

	for (WORD count = 0; count < NUM_NON_SPECIFIC_NUM_CONF_ID_ALLOC_TRY; count++)
	{
		tempStatus = FillRandomNID(resNumConfId);
		if (tempStatus != STATUS_OK)
		{
			status = tempStatus;
			break;
		}

		if (NumericIdExist(resNumConfId) == FALSE)
		{
			status = STATUS_OK;
			break;
		}
		else
		{
			TRACEINTO << "NumericConfId:" << resNumConfId << " - Non-specific numeric id allocation try";
		}
	}   // end for try

	// Add to DB should be performed in high level
	if (status == STATUS_OK)
	{
		WORD res_length = strlen(resNumConfId);
		DBGPASSERT(res_length != m_num_conf_id_len);

		TRACEINTO << "NumericConfId:" << resNumConfId;

		if (res_length <= NUMERIC_CONF_ID_MAX_LEN)
		{
			strncpy(numConfId, resNumConfId, res_length);
			numConfId[res_length] = '\0';
		}
		else
		{
			PASSERTMSG(1, "res_length exceed NUMERIC_CONF_ID_MAX_LEN + 1");
			numConfId[0] = '\0';
		}
	}
	else
		TRACEINTO << "Non-specific numeric id allocation failed, RequestedLength:" <<  m_num_conf_id_len;

	delete [] resNumConfId;

	return status;
}

//--------------------------------------------------------------------------
STATUS CReservator::FillRandomNID(char resNumConfId[NUMERIC_CONF_ID_MAX_LEN+1])
{
	int length = m_num_conf_id_len;

	if (length < 0 || length > NUMERIC_CONF_ID_MAX_LEN)
	{
		PTRACE(eLevelError, "Numeric Id length corrupted");
		return STATUS_FAIL;
	}

	// *** rand() return range 0-32767
	// *** prints in 16 symbols
	// to increase randomization we combine two randoms

	char* tempNumConfId  =  new char[NUMERIC_CONF_ID_MAX_LEN+1];
	char* tempNumConfId2 = new char[NUMERIC_CONF_ID_MAX_LEN+1];

	memset(resNumConfId, '\0', NUMERIC_CONF_ID_MAX_LEN+1);

	memset(tempNumConfId, '\0', NUMERIC_CONF_ID_MAX_LEN+1);
	sprintf(tempNumConfId, "%016d", rand());

	memset(tempNumConfId2, '\0', NUMERIC_CONF_ID_MAX_LEN+1);
	sprintf(tempNumConfId2, "%016d", rand());

	strncpy(resNumConfId, tempNumConfId + NUMERIC_CONF_ID_MAX_LEN  - length, std::min(length, NUMERIC_CONF_ID_MAX_LEN - 1));
	strncpy(resNumConfId, tempNumConfId2 + NUMERIC_CONF_ID_MAX_LEN  - length/2, std::min(length/2, NUMERIC_CONF_ID_MAX_LEN - 1));
	resNumConfId[std::min(length, NUMERIC_CONF_ID_MAX_LEN - 1)] = '\0';

	delete [] tempNumConfId;
	delete [] tempNumConfId2;

	return STATUS_OK;
}

//--------------------------------------------------------------------------
WORD CReservator::GetNumConfPerType(eRsrcConfType c_type)
{
	WORD num_conf = 0;
	switch (c_type)
	{
		case eMR_type     : { num_conf = m_numMR; break; }
		case eEQ_type     : { num_conf = m_numEQ; break; }
		case eSipFact_type: { num_conf = m_numSipFact; break; }
		default           : { break; }
	} // switch

	return num_conf;
}

//--------------------------------------------------------------------------
void CReservator::PlusNumConfPerType(eRsrcConfType c_type)
{
	switch (c_type)
	{
		case eMR_type     : { m_numMR++; break; }
		case eEQ_type     : { m_numEQ++; break; }
		case eSipFact_type: { m_numSipFact++; break; }
		default           : { return; }                    // return so the trace will not be printed unnecessary
	}

	TRACEINTO << "numMR:" << m_numMR << ", numEQ:" << m_numEQ << ", numSipFact:" << m_numSipFact << ", ConfType:" << c_type;
}

//--------------------------------------------------------------------------
void CReservator::MinusNumConfPerType(eRsrcConfType c_type)
{
	switch (c_type)
	{
		case eMR_type:
		{
			if (m_numMR)
				m_numMR--;
			else
				PASSERT(1);
			break;
		}

		case eEQ_type:
		{
			if (m_numEQ)
				m_numEQ--;
			else
				PASSERT(1);
			break;
		}

		case eSipFact_type:
		{
			if (m_numSipFact)
				m_numSipFact--;
			else
				PASSERT(1);
			break;
		}

		default:
		{
			PASSERT(1);
		}
	} // switch

	TRACEINTO << "numMR:" << m_numMR << ", numEQ:" << m_numEQ << ", numSipFact:" << m_numSipFact << ", ConfType:" << c_type;
}

//--------------------------------------------------------------------------
DWORD CReservator::AllocateMntrConfId(DWORD mntrConfId)   // circular , -1 - failure return value
{
	if (m_nextMntrConfId >= MAX_MNTR_CONF_IDS-1)
	{
		// PASSERT(1);
		PTRACE2INT(eLevelInfoNormal, "CReservator::AllocateMntrConfId: m_nextMntrConfId is reached the max value =", m_nextMntrConfId);
		m_nextMntrConfId = 1;
	}

	WORD  status = STATUS_OK;
	DWORD next   = 0;
	if (mntrConfId != 0)                                    // DB read, specific - start
	{
		if (mntrConfId > MAX_MNTR_CONF_IDS -1)
		{
			DBGPASSERT(1);
			status = STATUS_FAIL;
		}

		if (mntrConfId >= m_nextMntrConfId)
			m_nextMntrConfId = mntrConfId + 1;

		next = mntrConfId;
	} // DB read, specific - end

	else                                                    // non-specific - start
	{
		next = m_nextMntrConfId;
	}  // non-specific - end

	// promote m_nextMntrConfId only in non-specific case!!! becouse specific case - DB reading - should not
	// cause shift of m_nextMntrConfId back - for further non-specific allocations,
	// that MUST be all above last conf id which was taken from CDR at Startup.

	if ((status == STATUS_OK) && (mntrConfId == 0))
		m_nextMntrConfId++;

	return (status == STATUS_OK) ? next : 0;
}

//--------------------------------------------------------------------------
void CReservator::UpdateMntrConfId(DWORD mntrConfId)
{
	// only for slave case
	if (eRPSlaveMode != GetRPMode())
		return;

	if (m_nextMntrConfId >= MAX_MNTR_CONF_IDS-1)
	{
		PTRACE2INT(eLevelInfoNormal, "CReservator::AllocateMntrConfId: m_nextMntrConfId is reached the max value =", m_nextMntrConfId);
		m_nextMntrConfId = 1;
	}

	if (mntrConfId > MAX_MNTR_CONF_IDS -1)
	{
		DBGPASSERT(1);
		m_nextMntrConfId = 1;
		return;
	}

	if (mntrConfId >= m_nextMntrConfId)
		m_nextMntrConfId = mntrConfId + 1;
}

//--------------------------------------------------------------------------
STATUS CReservator::DeAllocateMntrConfId(DWORD mntrConfId)
{
	return STATUS_OK;
}

//--------------------------------------------------------------------------
BYTE CReservator::MonitorIdExist(DWORD monitorId)
{
	SleepingConferences* pSleepingConferences = m_pCentralConferencesDB->GetSleepingConferences();
	PASSERT_AND_RETURN_VALUE(!pSleepingConferences, FALSE);

	ReservedConferences* pConfRsrvRsrcs = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrcs, FALSE);

	SleepingConferences::iterator i;
	for (i = pSleepingConferences->begin(); i != pSleepingConferences->end(); i++)
		if (i->GetMonitorConfId() == monitorId)                 // identical
			return TRUE;

	ReservedConferences::iterator confRsrvIterator;
	for (confRsrvIterator = pConfRsrvRsrcs->begin(); confRsrvIterator != pConfRsrvRsrcs->end(); confRsrvIterator++)
		if (confRsrvIterator->GetMonitorConfId() == monitorId)  // identical
			return TRUE;

	return FALSE;
}

//--------------------------------------------------------------------------
void CReservator::SetStartConfId(DWORD lastConfId)
{
	if (lastConfId > MAX_MNTR_CONF_IDS)
	{
		DBGPASSERT(1);
		PTRACE2INT(eLevelInfoNormal, "CReservator::SetStartConfId: illegal last conf id from CDR :", lastConfId);
		m_lastConfId = 0;
	}

	else
	{
		m_lastConfId = lastConfId;                              // field kept for consistency & check

		if (lastConfId >= m_nextMntrConfId)
			m_nextMntrConfId = lastConfId+1;
	}
}

//--------------------------------------------------------------------------
void CReservator::NumericIdConfigInit()
{
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();

	if (!pSysConfig)                                          // SysConfig NOT FOUND!
	{
		PASSERT(1);                                             // +trace?
	}
	else
	{
		DWORD length = 0, max_length = 0, min_length = 0;

		std::string key = CFG_KEY_NUMERIC_CONF_ID_MAX_LEN;
		pSysConfig->GetDWORDDataByKey(key, max_length);

		key = CFG_KEY_NUMERIC_CONF_ID_MIN_LEN;
		pSysConfig->GetDWORDDataByKey(key, min_length);

		key = CFG_KEY_NUMERIC_CONF_ID_LEN;
		pSysConfig->GetDWORDDataByKey(key, length);

		if (max_length > NUMERIC_CONF_ID_MAX_LEN)
		{
			PASSERT(1); max_length = NUMERIC_CONF_ID_MAX_LEN;
		}

		if (min_length < NUMERIC_CONF_ID_MIN_LEN)
		{
			PASSERT(1); min_length = NUMERIC_CONF_ID_MIN_LEN;
		}

		if (max_length < min_length)
		{
			PASSERT(1); max_length = NUMERIC_CONF_ID_MAX_LEN; min_length = NUMERIC_CONF_ID_MIN_LEN;
		}

		if (length > NUMERIC_CONF_ID_MAX_LEN) // but may be greater than max_length
		{
			PASSERT(1); length = max_length;
		}

		if (length < NUMERIC_CONF_ID_MIN_LEN) // but may be less than min_length
		{
			PASSERT(1); length = min_length;
		}

		m_num_conf_id_len     = length;
		m_num_conf_id_max_len = max_length;
		m_num_conf_id_min_len = min_length;
	}
}

//--------------------------------------------------------------------------
STATUS CReservator::DeleteRepeatedRes(DWORD repeatedId)
{
	STATUS finalStatus = STATUS_OK;
	int    numDeleted  = 0;

	ReservedConferences* pConfRsrvRsrc = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrc, STATUS_FAIL);

	CConfRsrcDB* pConfRsrcsDB = m_pCentralConferencesDB->GetConfRsrcDB();
	PASSERT_AND_RETURN_VALUE(!pConfRsrcsDB, STATUS_FAIL);

	ReservedConferences::iterator i;
	for (i = pConfRsrvRsrc->begin(); i != pConfRsrvRsrc->end(); i++)
	{
		if (i->GetRepeatedConfId() == repeatedId) // found
		{
			// only delete reservations (and not ongoing conferences)
			if (pConfRsrcsDB->GetConfRsrc(i->GetMonitorConfId()) == NULL)
			{
				m_monitorIdsToDelete.insert(i->GetMonitorConfId());
				numDeleted++;
			}
		}
	}

	if (numDeleted == 0)
		finalStatus = STATUS_NOT_FOUND;
	else
	{
		finalStatus = STATUS_IN_PROGRESS;
		CSingleToneApi rsrvManager(eProcessResource, "RsrvManager");
		rsrvManager.SendMsg(NULL, RESOURCE_DELETE_REPEATED_IND);
		rsrvManager.CleanUp();
	}

	TRACEINTO << "numDeleted:" <<  numDeleted << ", repeatedId:" << repeatedId;

	return finalStatus;
}

//--------------------------------------------------------------------------
void CReservator::ActuallyDeleteRepeatedRes()
{
	STATUS tempStatus;
	int    numDeleted = 0;
	int    count      = 0;

	std::set<DWORD>::iterator monitorIdsToDeleteIterator;

	// we will start from the end, because else, at each deletion we will move the timer to the next conference...
	for (monitorIdsToDeleteIterator = m_monitorIdsToDelete.end(); monitorIdsToDeleteIterator != m_monitorIdsToDelete.begin();)
	{
		monitorIdsToDeleteIterator--;
		tempStatus = DeleteRes(*monitorIdsToDeleteIterator);  // rereserve done inside
		if (tempStatus != STATUS_OK)
		{
			TRACEINTO << "MonitorId:" << *monitorIdsToDeleteIterator << ", Status " << tempStatus << " - Failed, Couldn't delete repeated reservation";
		}
		else
			numDeleted++;

		count++;
		if (count%50 == 0)                                    // We go to sleep for a little while every 100 reservations.
		{                   // To allow other tasks to be performed in the Resource Process - especially answering the watchdog
			SystemSleep(20, TRUE);
		}
	}

	m_monitorIdsToDelete.clear();

	TRACEINTO << "numDeleted:" << numDeleted << " - Succeeded deleting of repeated reservations";
}

//--------------------------------------------------------------------------
void CReservator::ActuallyWriteRepeatedRes()
{
	STATUS           status;
	CCommResApi*     pConfToAdd;
	CRecurrenceData* pRecurrenceData;

	TRACEINTO << " Start writing reservations to disk " << " \n";
	PASSERT_AND_RETURN(m_pCentralConferencesDB->GetRsrvDB() == NULL);

	for (DWORD i = 0; i < m_MAX_NUMBER_OF_RESERVATIONS; i++)
	{
		pRecurrenceData = m_RepeatedReservationWriter.GetRecurrenceData(i);
		if (pRecurrenceData != NULL)
		{
			pConfToAdd = new CCommResApi(*(m_RepeatedReservationWriter.GetBaseReservation()));
			pRecurrenceData->CopyDataTo(pConfToAdd);

			status = m_pCentralConferencesDB->GetRsrvDB()->Add(*pConfToAdd); // both: writes to disc and creates short inside
			if (STATUS_OK != status)
			{
				PTRACE(eLevelError, "CReservator::ActuallyWriteRepeatedRes Failed on Add Rsrv To DB");
				PASSERT(status);
			}
			else
			{
				if (pRecurrenceData->m_StartTime <= *m_pNearTime || m_NearConfId == 0)
				{
					if (m_NearConfId != 0)
						DeleteConferenceTimer();

					*m_pNearTime = pRecurrenceData->m_StartTime;
					m_NearConfId = pRecurrenceData->m_MonitorConfId;
					PTRACE2(eLevelInfoNormal, "CReservator::ActuallyWriteRepeatedRes, setting timer for next conference = ", pRecurrenceData->m_Name);
					StartConferenceTimer();
				}
			}

			POBJDELETE(pConfToAdd);

			// Delete the current recurrence
			m_RepeatedReservationWriter.RemoveRecurrence(i);

			if ((i+1) % 25 == 0) 			// We go to sleep for a little while every 25 reservations.
			{								// To allow other tasks to be performed in the Resource Process - especially answering the watchdog
				TRACEINTO << " SystemSleep for 100ms, current reservation: " <<  i;
				SystemSleep(10, TRUE);	// Decrease the num of reservation from 50 to 25, and reduce the system sleep, to fix VNGFE-7957)
			}
		}
	}

	m_RepeatedReservationWriter.EndWriting();
}

//--------------------------------------------------------------------------
STATUS CReservator::DeleteAllRes()
{
	STATUS finalStatus = STATUS_OK;
	int    numDeleted  = 0;

	ReservedConferences* pConfRsrvRsrc = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrc, STATUS_FAIL);

	CConfRsrcDB* pConfRsrcsDB = m_pCentralConferencesDB->GetConfRsrcDB();
	PASSERT_AND_RETURN_VALUE(!pConfRsrcsDB, STATUS_FAIL);

	ReservedConferences::iterator i;
	for (i = pConfRsrvRsrc->begin(); i != pConfRsrvRsrc->end(); i++)
	{
		// only delete reservations (and not ongoing conferences)
		if (pConfRsrcsDB->GetConfRsrc(i->GetMonitorConfId()) == NULL)
		{
			m_monitorIdsToDelete.insert(i->GetMonitorConfId());
			numDeleted++;
		}
	}

	if (numDeleted == 0)
		finalStatus = STATUS_NOT_FOUND;
	else
	{
		finalStatus = STATUS_IN_PROGRESS;
		CSingleToneApi rsrvManager(eProcessResource, "RsrvManager");
		rsrvManager.SendMsg(NULL, RESOURCE_DELETE_ALL_RES_IND);
		rsrvManager.CleanUp();
	}

	TRACEINTO << "numDeleted:" << numDeleted;

	return finalStatus;
}

//--------------------------------------------------------------------------
STATUS CReservator::DeleteRes(DWORD monitorId)
{
	CRsrvDB* pRsrvDB = CHelperFuncs::GetRsrvDB();
	PASSERT_AND_RETURN_VALUE(pRsrvDB == NULL, STATUS_FAIL);

	eRsrcConfType conf_type   = eConf_type_none;
	CCommResApi*  pCommResApi = pRsrvDB->GetRsrv(monitorId);

	if (pCommResApi != NULL)
	{
		pRsrvDB->Cancel(monitorId);
		POBJDELETE(pCommResApi);

		TRACEINTO << "MonitorConfId:" << monitorId;

		STATUS status = DeleteResAllTypes(monitorId, &conf_type); // re-reserve done here
		PASSERT(conf_type != eConf_type_none);

		if (m_NearConfId == monitorId)                            // if it was next conference to start
		{
			DeleteConferenceTimer();
			StartTimerForNextConference();                          // find next conference to start
		}

		return status;
	}
	else
	{
		DBGPASSERT(1);
		return STATUS_NOT_FOUND;
	}
}

//--------------------------------------------------------------------------
STATUS CReservator::SetConferenceEndTimeRequest(const DWORD monitorid, const CStructTm* pStructTm)
{
	STATUS status = STATUS_OK;
	if (FALSE == IsInternalReservator())
		return status;

	CConfRsrvRsrc* pconf = (CConfRsrvRsrc*)GetRegularConference(monitorid);
	if (NULL == pconf)
	{
		return status; // Its's only temporary due to the fact that MR and EQ in active stage aren't inserted into list
	}

	BOOL  isFromPassive = pconf->GetFromPassive();

	DWORD start  = pconf->GetStartInterval();
	DWORD newend = StructTm2IntervalForEnd(pStructTm);

	InitRsrvCalculatorPhoneList(pconf);

	BYTE  isMultipleService = CHelperFuncs::IsMultipleService();
	DWORD ipServiceId = pconf->GetIpServiceIdForMinParties();
	if (isMultipleService)
	{
		status = CreateReserveInterval(start, newend, monitorid, 0, TRUE, ipServiceId);
	}
	else
	{
		status = CreateReserveInterval(start, newend, monitorid);
	}

	if (status != STATUS_OK)
	{
		m_ReservationCalculator.Cleanup();
		return status;
	}

	CIntervalRsrvAmount confNeededAmount;
	pconf->GetConferenceNeededAmount(confNeededAmount);
	status = m_ReservationCalculator.ReserveAmount(start, newend, confNeededAmount, isFromPassive, FALSE, ePhoneAlloc, ipServiceId);
	if (status != STATUS_OK)
	{
		m_ReservationCalculator.Cleanup();
		return status;
	}

	m_ReservationCalculator.Cleanup();
	DWORD oldend = pconf->GetEndInterval();
	pconf->SetEndInterval(newend);

	if (newend < oldend) // if conference was shortened
		Rereserve(eRereserveFaulty, start, oldend);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CReservator::Rereserve(eRereserveType rereserveType, DWORD start, DWORD end)
{
	// don't re-reserve during startup, not necessary
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
		return;

	if (IsInternalReservator() == FALSE)
		return;

	TRACEINTO << "rereserveType:" << rereserveType << ", start:" << start << ", end:" << end;

	ReservedConferences* pConfRsrvRsrc = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN(!pConfRsrvRsrc);

	CRsrvDB* pRsrvDb = m_pCentralConferencesDB->GetRsrvDB();
	//PASSERT_AND_RETURN(!pRsrvDb); //causes asserts while Soft MCU starting due to differences in flow of RMX & S.MCU
	if (!pRsrvDb)
	{
		TRACESTRFUNC(eLevelWarn) << "RsrvDB is not initialized";
		return;
	}

	eRsrcConfType c_type = eConf_type_none;
	DWORD conf_start, conf_end;
	DWORD reservationCount = 0;

	for (ReservedConferences::iterator i = pConfRsrvRsrc->begin(); i != pConfRsrvRsrc->end(); i++)
	{
		if (start != 0)
		{
			conf_end = i->GetEndInterval();
			if (conf_end < start)
				continue;
		}

		if (end != 0)
		{
			conf_start = i->GetStartInterval();
			if (conf_start > end)
				continue;
		}

		CCommResRsrvShort* pShortRes = pRsrvDb->GetShortRsrv(i->GetMonitorConfId());
		if (pShortRes == NULL)
		{
			// this is OK, it means it's not a reservation, but an ongoing conf. Just continue
			continue;
		}

		if (rereserveType == eRereserveFaulty)
		{
			if (pShortRes->GetStatus() == STATUS_OK)
				continue;
		}
		else if (rereserveType == eRereserveNotFaulty)
		{
			if (pShortRes->GetStatus() != STATUS_OK)
				continue;
		}

		if (ReReserveOneConference((CConfRsrvRsrc*)&(*i)) != STATUS_OK)
		{
			if (pShortRes->GetStatus() != eStsSUSPEND &&
					pShortRes->GetStatus() != eWrongSysMode) // Status Field
			{
				pShortRes->SetStatus(eStsSUSPEND);
				((CConfRsrvRsrc*)&(*i))->SetStatus(STATUS_RESERVATION_SUSPENDED);
				pRsrvDb->IncreaseSummaryUpdateCounter();
				pShortRes->SetSummeryUpdateCounter(pRsrvDb->GetSummaryUpdateCounter());
			}
		}
		else
		{
			if (pShortRes->GetStatus() != STATUS_OK)
			{
				pShortRes->SetStatus(STATUS_OK);
				((CConfRsrvRsrc*)&(*i))->SetStatus(STATUS_OK);
				pRsrvDb->IncreaseSummaryUpdateCounter();
				pShortRes->SetSummeryUpdateCounter(pRsrvDb->GetSummaryUpdateCounter());
			}
		}

		reservationCount++;
		// We go to sleep for a little while every 10 reservations.
		// To allow other tasks to be performed in the Resource Process - especially answering the watchdog
		// Note: This is a temp fix, it still shows Internal communication error when shifting time of 1000-2000 reservations, but the functionality is working.
		if (reservationCount % 10 == 0)
		{
			TRACEINTO << "reservationCount:" << reservationCount;
			SystemSleep(2, TRUE);
		}
	}
}

//--------------------------------------------------------------------------
void CReservator::UpdateRoutingNameWithNID(CCommResApi* pCommResApi)
{
	const char* currentRoutingName = pCommResApi->GetName();
	if (currentRoutingName[0] == '\0')
	{
		pCommResApi->SetName(pCommResApi->GetNumericConfId());
	}
}

//--------------------------------------------------------------------------
void CReservator::ProfileChangedInd(DWORD profileID, BOOL bReReserveAll)
{
	if (!IsInternalReservator())
		return;

	CRsrvDB* pRsrvDb = m_pCentralConferencesDB->GetRsrvDB();
	PASSERT_AND_RETURN(!pRsrvDb);

	// if there's no reservation that uses this profile, return
	if (!pRsrvDb->IsRsrvUsingProfile(profileID))
		return;

	ReservedConferences* pConfRsrvRsrc = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN(!pConfRsrvRsrc);

	// update all the reservations with the new profile id
	ReservedConferences::iterator _itr, _end = pConfRsrvRsrc->end();
	for (_itr = pConfRsrvRsrc->begin(); _itr != _end; ++_itr)
	{
		CCommResRsrvShort* pShortRes = pRsrvDb->GetShortRsrv(_itr->GetMonitorConfId());
		if (!pShortRes) // this is OK, it means it's not a reservation, this is an ongoing conference. Just continue
			continue;

		if (pShortRes->GetAdHocProfileId() == profileID)
		{
			// load full details from disk and update inner struts
			CCommResApi* pCommResApi = pRsrvDb->GetRsrv(_itr->GetMonitorConfId());
			const_cast<CConfRsrvRsrc&>(*_itr).FillFieldsNeededForReservator(pCommResApi);
			delete pCommResApi;
		}
	}

	if (bReReserveAll) // now re-reserve all reservations
		Rereserve(eRereserveAll);
}

//--------------------------------------------------------------------------
void CReservator::InitProductType(eProductType productType)
{
	if (productType == eProductTypeRMX4000)
	{
		m_MAX_NUMBER_OF_RESERVATIONS  = MAX_RSRV_IN_LIST_AMOS;
		m_MAX_NUMBER_OF_MRS           = MAX_NUMBER_OF_MEETING_ROOM_AMOS;
		m_MAX_NUMBER_OF_EQS           = MAX_NUMBER_OF_EQ_AMOS;
		m_MAX_NUMBER_OF_SIP_FACTORIES = MAX_NUMBER_OF_SIP_FACTORY_AMOS;
	}
	else if (eProductTypeGesher==productType)
	{
		m_MAX_NUMBER_OF_RESERVATIONS  = MAX_RSRV_IN_LIST_GESHER;
		m_MAX_NUMBER_OF_MRS           = MAX_NUMBER_OF_MEETING_ROOM_GESHER;
		m_MAX_NUMBER_OF_EQS           = MAX_NUMBER_OF_EQ_GESHER;
		m_MAX_NUMBER_OF_SIP_FACTORIES = MAX_NUMBER_OF_SIP_FACTORY_GESHER;
	}
	else if (eProductTypeNinja==productType)
	{
		m_MAX_NUMBER_OF_RESERVATIONS  = MAX_RSRV_IN_LIST_NINJA;
		m_MAX_NUMBER_OF_MRS           = MAX_NUMBER_OF_MEETING_ROOM_NINJA;
		m_MAX_NUMBER_OF_EQS           = MAX_NUMBER_OF_EQ_NINJA;
		m_MAX_NUMBER_OF_SIP_FACTORIES = MAX_NUMBER_OF_SIP_FACTORY_NINJA;
	}
	else
	{
		m_MAX_NUMBER_OF_RESERVATIONS  = MAX_RSRV_IN_LIST_RMX2000;
		m_MAX_NUMBER_OF_MRS           = MAX_NUMBER_OF_MEETING_ROOM_RMX2000;
		m_MAX_NUMBER_OF_EQS           = MAX_NUMBER_OF_EQ_RMX2000;
		m_MAX_NUMBER_OF_SIP_FACTORIES = MAX_NUMBER_OF_SIP_FACTORY_RMX2000;
	}
}

//--------------------------------------------------------------------------
void CReservator::InitRsrvCalculatorPhoneList(CConfRsrvRsrc* pConf)
{
	if (!pConf || pConf->GetFromPassive())
		return;

	CServicePhoneStr* pPhoneStr = pConf ? pConf->GetFirstServicePhone() : NULL;   // olga - first only?
	if (pPhoneStr)
	{
		char* serviceName = (char*)pPhoneStr->GetNetServiceName();

		CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
		PASSERT_AND_RETURN(!pSystemResources);

		const CNetServiceRsrcs* pExistService = pSystemResources->findServiceByName(serviceName);
		if (pExistService)
		{
			m_ReservationCalculator.SetNetServiceName(serviceName);
			m_ReservationCalculator.SetPhonesList(*pExistService->GetPhonesList());
		}
	}
}

//--------------------------------------------------------------------------
STATUS CReservator::AllocateBondingTemporaryNumber_WithSchedule(CServicePhoneStr* pPhoneStr, const char* pSimilarToThisString)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	STATUS status = STATUS_OK;

	if (pPhoneStr != NULL && pPhoneStr->m_netServiceName[0] != '\0') // SPECIFIC AND NON SPECIFIC CASE
	{
		char* serviceName  = pPhoneStr->m_netServiceName;
		const CNetServiceRsrcs* pExistService = pSystemResources->findServiceByName(serviceName);
		if (pExistService)
		{
			// 1. Insert EQ/MR list + Service Name
			m_ReservationCalculator.SetNetServiceName(serviceName);
			m_ReservationCalculator.SetPhonesList(*pExistService->GetPhonesList());

			// 2. Check with existing EQ and MR( if SPECIFIC )
			Phone* phone = pPhoneStr->GetFirstPhoneNumber();
			if (NULL != phone)
			{
				TRACEINTO << "Failed, phone isn't empty";
				status = STATUS_FAIL;
			}

			// 3. Add all reservations
			if (STATUS_OK == status)
			{
				ReservedConferences* pConfRsrvRsrc = m_pCentralConferencesDB->GetConfRsrvRsrcs();
				DWORD start_interval;

				CStructTm now;
				SystemGetTime(now);

				DWORD currentInterval = StructTm2IntervalForStart(&now);

				ReservedConferences::iterator confRsrvIterator;
				for (confRsrvIterator = pConfRsrvRsrc->begin(); confRsrvIterator != pConfRsrvRsrc->end(); confRsrvIterator++)
				{
					if (confRsrvIterator->GetStatus() != STATUS_OK || confRsrvIterator->GetFromPassive())
						continue;

					start_interval = confRsrvIterator->GetStartInterval();
					if (start_interval > currentInterval)
						continue;

					CIntervalRsrvAmount confNeededAmount;
					((CConfRsrvRsrc)(*confRsrvIterator)).GetConferenceNeededAmount(confNeededAmount, TRUE);

					CServicePhoneStr* pConfRsrvPhoneStr = (CServicePhoneStr*) confNeededAmount.GetServicePhone();

					status = m_ReservationCalculator.PhoneCheckOrAlloc(pConfRsrvPhoneStr, ePhoneCheckAlloc);        // temporary phone's allocation
					if (STATUS_OK != status)
						break;
				}

				// 4. Allocate
				if (STATUS_OK == status)
				{
					status = m_ReservationCalculator.ReservePhoneIfNeeded(pPhoneStr, NULL, pSimilarToThisString);   // NULL because CSleepingConference isn't CConfRsrvRsrc
				}

				if (STATUS_OK == status)
				{
					status = pSystemResources->AllocateServicePhones(*pPhoneStr);                                   // NOTE: no need to check twice!!!
				}

				m_ReservationCalculator.Cleanup();
			}
		}
	}

	return status;
}

//--------------------------------------------------------------------------
WORD operator==(const ConfRsrvObj& lhs, const ConfRsrvObj& rhs)
{
	return (lhs.m_confTime == rhs.m_confTime && lhs.m_monitorConfId == rhs.m_monitorConfId);
}

//--------------------------------------------------------------------------
bool operator<(const ConfRsrvObj& lhs, const ConfRsrvObj& rhs)
{
	if (lhs.m_confTime == rhs.m_confTime)
		return (lhs.m_monitorConfId < rhs.m_monitorConfId);

	return (lhs.m_confTime < rhs.m_confTime);
}

//--------------------------------------------------------------------------
STATUS CReservator::CalculateMaxWeightOfAllReservation(WORD min_num_parties_in_permanent_conf,
                                                       WORD max_num_conf,
                                                       BOOL isVSW,
                                                       DWORD& problemConfId)
{
	ReservedConferences* pConfRsrvRsrc = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrc, STATUS_INSUFFICIENT_RSRC);

	DWORD maxWeightPermanentConf = 0, conf_start, conf_end;

	ReservedConferences::iterator confRsrvIterator;
	std::set<ConfRsrvObj> confListToCheck;
	WORD num_perm = 0, num_concurrent = 0;
	for (confRsrvIterator = pConfRsrvRsrc->begin(); confRsrvIterator != pConfRsrvRsrc->end(); confRsrvIterator++)
	{
		if (confRsrvIterator->IsPermanent())
		{
			if (CHelperFuncs::IsMode2C())
				maxWeightPermanentConf += confRsrvIterator->GetMinNumVideoParties();
			else
				maxWeightPermanentConf += confRsrvIterator->GetLogicalWeightOfMinNumVideoParties();

			if ((isVSW && confRsrvIterator->IsAutoVSW()) || (!isVSW && !confRsrvIterator->IsAutoVSW()))
				num_perm++;
		}
		else
		{
			WORD minParties = confRsrvIterator->GetMinNumVideoParties();
			if (!CHelperFuncs::IsMode2C())
				minParties = confRsrvIterator->GetLogicalWeightOfMinNumVideoParties();

			conf_start =  confRsrvIterator->GetStartInterval();
			conf_end   =  confRsrvIterator->GetEndInterval();

			ConfRsrvObj startConfObj(conf_start, confRsrvIterator->GetMonitorConfId(), TRUE, minParties, confRsrvIterator->IsAutoVSW());
			ConfRsrvObj endConfObj(conf_end, confRsrvIterator->GetMonitorConfId(), FALSE, minParties, confRsrvIterator->IsAutoVSW());

			confListToCheck.insert(startConfObj);
			confListToCheck.insert(endConfObj);
		}
	}

	if (maxWeightPermanentConf + min_num_parties_in_permanent_conf > m_numPartiesInDongle)
	{
		TRACEINTO << "There is conflict with other permanent conferences";
		return STATUS_INSUFFICIENT_RSRC;  // maxWeightPermanentConf;
	}

	if (num_perm >= max_num_conf)
	{
		TRACEINTO << "Number of permanent conferences is exceeded";
		return STATUS_MAX_RESERVATIONS_EXCEEDED; // maxWeightPermanentConf;
	}

	if (confListToCheck.size() == 0)
		return STATUS_OK;  // maxWeightPermanentConf;

	DWORD maxWeight = m_numPartiesInDongle - maxWeightPermanentConf;
	DWORD tempMaxWeight = 0, currentWeight = 0;
	std::set<ConfRsrvObj>::iterator confListIter;
	for (confListIter = confListToCheck.begin(); confListIter != confListToCheck.end(); confListIter++)
	{
		BOOL isSameSessionType = ((isVSW && confListIter->m_isVSW) || (!isVSW && !confListIter->m_isVSW));
		if (confListIter->m_isConfStartTime)
		{
			currentWeight += confListIter->m_minNumParties;
			if (isSameSessionType)
				num_concurrent++;
		}
		else
		{
			currentWeight -= confListIter->m_minNumParties;
			if (isSameSessionType)
			{
				if (num_concurrent)
					num_concurrent--;
				else
					DBGPASSERT(1);
			}
		}

		if ((currentWeight + min_num_parties_in_permanent_conf) > maxWeight)
		{
			TRACEINTO << "MonitorConfId:" << confListIter->m_monitorConfId << " - There is conflict with Conference Monitor Id";

			problemConfId = confListIter->m_monitorConfId;
			return STATUS_INSUFFICIENT_RSRC;
		}

		if ((num_concurrent + num_perm) >= max_num_conf)
		{
			TRACEINTO << "num_of_permanent:" << num_perm << ", num_concurrent:" << num_concurrent << " - Max number of conferences is exceeded";
			problemConfId = confListIter->m_monitorConfId;
			return STATUS_MAX_RESERVATIONS_EXCEEDED;
		}

		if (currentWeight > tempMaxWeight)
			tempMaxWeight = currentWeight;
	}

	return STATUS_OK; // return tempMaxWeight;
}

//--------------------------------------------------------------------------
WORD CReservator::GetMaxNumberOngoingConferences2C(BOOL accordingToCards /*= TRUE*/) const
{
	if (CHelperFuncs::IsMode2C())
	{
		if (accordingToCards)
		{
			CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
			PASSERT_AND_RETURN_VALUE(!pSystemResources, 0);
			return pSystemResources->GetMaxNumOngoingConfPerCardsInEventMode();
		}

		// else we will calc per license
		if (m_numPartiesInDongle <= 60)
			return 4;
		else if (m_numPartiesInDongle <= 150)
			return 8;
		else if (m_numPartiesInDongle <= 240)
			return 16;
		else
			return 16;
	}
	else
	{
		CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
		PASSERT_AND_RETURN_VALUE(!pSystemResources, 0);
		return pSystemResources->GetMaxNumberOfOngoingConferences();
	}
}

//--------------------------------------------------------------------------
void CReservator::AddIpServiceCalc(DWORD ipServiceId, float service_factor, BOOL round_up)
{
	m_ReservationCalculator.AddIpServiceCalc(ipServiceId, service_factor, round_up);
}

//--------------------------------------------------------------------------
void CReservator::DumpCalculatorTotals() const
{
	m_ReservationCalculator.DumpTotals();
}

//--------------------------------------------------------------------------
DWORD CReservator::GetIpServiceIdForMinParties(CCommResApi* pCommResApi)
{
	DWORD serviceId = ID_ALL_IP_SERVICES;
	if (pCommResApi == NULL)
	{
		PASSERT(ID_ALL_IP_SERVICES);
		return serviceId;
	}

	const char* ipServiceNameForMinParties = pCommResApi->GetServiceNameForMinParties();

	if (0 == strncmp(ipServiceNameForMinParties, "[Default Service]", NET_SERVICE_PROVIDER_NAME_LEN) || strlen(ipServiceNameForMinParties) == 0)
	{
		CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
		PASSERT_AND_RETURN_VALUE(!pSystemResources, ID_ALL_IP_SERVICES);
		ipServiceNameForMinParties = pSystemResources->GetDefaultIpService(serviceId, DEFAULT_SERVICE_H323);
	}
	else
	{
		serviceId = CHelperFuncs::GetIpServiceId(ipServiceNameForMinParties);
	}

	TRACEINTO << "ServiceName:" << ipServiceNameForMinParties << ", ServiceId:" << serviceId;
	return serviceId;
}

//--------------------------------------------------------------------------
void CReservator::ShiftStartTimeAndRereserve(WORD hour_shift, WORD min_shift, WORD sign)
{
	if (!IsInternalReservator())
		return;

	DWORD sec_shift = hour_shift*3600 + min_shift*60;

	ReservedConferences* pConfRsrvRsrc = m_pCentralConferencesDB->GetConfRsrvRsrcs();
	PASSERT_AND_RETURN(!pConfRsrvRsrc);

	CRsrvDB* pRsrvDb = m_pCentralConferencesDB->GetRsrvDB();
	PASSERT_AND_RETURN(!pRsrvDb);

	CPrettyTable<ConfMonitorID, const char*, DWORD, DWORD, const char*> tbl("MonitorConfId", "StartTime", "StartInterval", "EndInterval", "FileName");

	ReservedConferences::iterator _itr, _end = pConfRsrvRsrc->end();
	for (_itr = pConfRsrvRsrc->begin(); _itr != _end; ++_itr)
	{
		ConfMonitorID monitorConfID = _itr->GetMonitorConfId();

		CCommResRsrvShort* pShortRes = pRsrvDb->GetShortRsrv(monitorConfID);
		if (pShortRes == NULL)
			continue;                                     // this is OK, it means it's not a reservation, but an ongoing conferencet continue
		else if (pShortRes->GetRepSchedulingId() == 0)  // if not recurring reservation, then do not change DST
			continue;

		const CStructTm* pStartTime = pShortRes->GetStartTime();
		char buffer[128];
		pStartTime->DumpToBuffer(buffer);

		time_t timeStartInSec = pStartTime->GetAbsTime(TRUE);
		time_t timeStartInSecShifted = sign ? (timeStartInSec + sec_shift) : (timeStartInSec - sec_shift);

		CStructTm newStructTime;
		newStructTime.SetAbsTime(timeStartInSecShifted);

		DWORD startInterval = StructTm2IntervalForStart(&newStructTime);
		((CConfRsrvRsrc*)&(*_itr))->SetStartInterval(startInterval);
		pShortRes->SetStartTime(newStructTime);
		DWORD endInterval = StructTm2IntervalForEnd(pShortRes->GetEndTime());
		((CConfRsrvRsrc*)&(*_itr))->SetEndInterval(endInterval);

		tbl.Add(monitorConfID, buffer, startInterval, endInterval, pShortRes->GetFileUniqueName().c_str());

		// need to save changes on disk
		CCommResApi* pOldResApi = pRsrvDb->GetRsrv(monitorConfID);
		if (IsValidPObjectPtr(pOldResApi))
		{
			pOldResApi->SetStartTime(newStructTime);
			pOldResApi->SetEndTime();
			pRsrvDb->Update(*pOldResApi);
			delete pOldResApi;
		}
		else
			PASSERTMSG(TRUE, "Reservation file is not found");
	}
	TRACEINTO << "HourShift:" << hour_shift << ", MinShift:" << min_shift << ", SecShift:" << sec_shift << tbl.Get();

	Rereserve(eRereserveAll);

	StartTimerForNextConference();
}

void CReservator::IncActiveForwardTransaction()
{
	m_numActiveForwardTransaction++;
}

DWORD CReservator::DecActiveForwardTransaction()
{
	if (m_numActiveForwardTransaction)
		m_numActiveForwardTransaction--;
	else
	{
		PASSERTMSG(TRUE, "no active forward transaction");
	}

	return m_numActiveForwardTransaction;
}
