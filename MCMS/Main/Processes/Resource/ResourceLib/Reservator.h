#ifndef RESERVATOR_H_
#define RESERVATOR_H_

#include "PObject.h"
#include "RsrvResources.h"
#include "SystemResources.h"
#include <map>
#include "StateMachine.h"
#include "ReservationCalculator.h"
#include "EnumsAndDefines.h"
#include "RepeatedReservationWriter.h"

const WORD START_CONFERENCE     = 3001;
const WORD NUM_SECS_IN_INTERVAL = 300;

class CResourceProcess;
class CCentralConferencesDB;
class CSystemResources;
class CComResRepeatDetails;
class CCommResRecurrenceResponse;

////////////////////////////////////////////////////////////////////////////
//                        ConfRsrvObj
////////////////////////////////////////////////////////////////////////////
class ConfRsrvObj
{
public:
	ConfRsrvObj (DWORD time, DWORD monitorId, BOOL isStart, WORD minParties, BOOL isVSW) :
		m_confTime(time), m_monitorConfId(monitorId), m_isConfStartTime(isStart), m_minNumParties(minParties), m_isVSW(isVSW) { }

	friend WORD operator==(const ConfRsrvObj&, const ConfRsrvObj&);
	friend bool operator<(const ConfRsrvObj&, const ConfRsrvObj&);

	DWORD               m_confTime;
	DWORD               m_monitorConfId;
	BOOL                m_isConfStartTime;
	WORD                m_minNumParties;
	BOOL                m_isVSW;
};


////////////////////////////////////////////////////////////////////////////
//                        CReservator
////////////////////////////////////////////////////////////////////////////
class CReservator : public CStateMachine
{
	CLASS_TYPE_1(CReservator, CStateMachine)

public:
	CReservator();
	virtual ~CReservator();
	const char*                NameOf() const                          { return "CReservator"; }
	virtual void*              GetMessageMap();
	void                       HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

	void                       SetDongleRestriction(DWORD num_parties) { m_numPartiesInDongle = num_parties;}
	DWORD                      GetDongleRestriction()                  { return m_numPartiesInDongle;}
	BOOL                       IsInternalReservator()                  {return m_is_internal_reservator;}
	BOOL                       IsRMode2C()                             { return m_is_2C_mode; }
	void                       SetRMode2C(BOOL mode)                   { m_is_2C_mode = mode; }
	void                       TimeChanged();

	void                       SetRPMode(eRPMode rpMode)               { m_RPMode = rpMode;}
	eRPMode                    GetRPMode()                             { return m_RPMode;}

	void                       ActuallyDeleteRepeatedRes();
	void                       ActuallyWriteRepeatedRes();

	void                       SetLogicalResources(WORD logical_num_parties[NUM_OF_PARTY_RESOURCE_TYPES], BOOL bRecalculateReservationPartyResources);

	const CConfRsrvRsrc*       GetConfRsrvRsrcById(DWORD monitorConfId);

	STATUS                     CreateRsrvFromAPIRequest(CCommResApi* pCommResApi, bool& bForwardTransaction, DWORD* pProblemConfId = NULL);
	STATUS                     StartSlaveOngReq(CCommResApi* pCommResApi);
	STATUS                     CreateRepeatedRsrvFromAPIRequest(CCommResApi* pCommResApi, CComResRepeatDetails* pCommResRepeated, CCommResRecurrenceResponse* pResRecurrenceResponse);
	STATUS                     CreateRsrvFromMRRequest(CCommResApi* pCommResApi);
	STATUS                     CreateAdHocRequest(CCommResApi* pCommResApi, BOOL bUseOldConfId);
	STATUS                     CreateReservationFromStartupOfResDB(CCommResApi* pCommResApi);
	STATUS                     UpdateRsrvFromAPIRequest(CCommResApi* pCommResApi);
	STATUS                     DeleteRepeatedRes(DWORD repeatedId);
	STATUS                     ProfileInRsrv(DWORD profileID);
	void                       ProfileChangedInd(DWORD profileID, BOOL bReReserveAll = TRUE);
	STATUS                     DeleteRes(DWORD monitorId);
	STATUS                     DeleteAllRes();
	STATUS                     DeleteResAllTypes(DWORD monitorId, eRsrcConfType* conf_type, BOOL bReReserveOthers = TRUE);
	STATUS                     SetConferenceEndTimeRequest(const DWORD monitorid, const CStructTm* pStructTm);

	void                       SetStartConfId(DWORD lastConfId);

	STATUS                     AllocateNumericConfId(char* numConfId, BYTE is_Startup = FALSE);
	BOOL                       NumericIdExist(const char* numConfId);
	BOOL                       NumericIdExistInSleepingConfs(const char* numConfId);
	char*                      GetNumericIdbyMonitorId(DWORD monitorId);
	size_t                     NumericIdCount(WORD length = 0);

	BYTE                       MonitorIdExist(DWORD monitorId);
	DWORD                      AllocateMntrConfId(DWORD mntrConfId = 0);
	STATUS                     DeAllocateMntrConfId(DWORD mntrConfId);
	void                       UpdateMntrConfId(DWORD monitorId);

	const CConfRsrvRsrc*       GetRegularConference(const DWORD monitorId);
	STATUS                     AddRegularConference(CCommResApi* pCommResApi);

	const CSleepingConference* GetSleepingConference(DWORD monitorId);
	STATUS                     AddSleepingConference(const char* numConfId, DWORD monitorId, eRsrcConfType conf_type);
	STATUS                     UpdateSleepingConference(const char* numConfId, DWORD monitorId, eRsrcConfType conf_type);

	WORD                       GetNumConfPerType(eRsrcConfType c_type);
	void                       PlusNumConfPerType(eRsrcConfType c_type);
	void                       MinusNumConfPerType(eRsrcConfType c_type);

	DWORD                      GetMaxNumbRsrv()   { return m_MAX_NUMBER_OF_RESERVATIONS;}
	DWORD                      GetMaxNumbMR()     { return m_MAX_NUMBER_OF_MRS;}
	DWORD                      GetMaxNumbEQ()     { return m_MAX_NUMBER_OF_EQS;}
	DWORD                      GetMaxNumbSipFct() { return m_MAX_NUMBER_OF_SIP_FACTORIES;}

	STATUS                     InitInternalReservator();
	void                       InitProductType(eProductType pProductType);
	eProductType               GetProductType()   { return ((m_MAX_NUMBER_OF_RESERVATIONS == MAX_RSRV_IN_LIST_AMOS) ? eProductTypeRMX4000 : eProductTypeRMX2000);}
	size_t                     StructTm2IntervalForStart(const CStructTm* timeStruct);
	size_t                     StructTm2IntervalForEnd(const CStructTm* timeStruct);

	DWORD                      StartConferenceTimer(BOOL bStartTimerAlways=FALSE);
	void                       DeleteConferenceTimer();

	STATUS                     AllocateBondingTemporaryNumber_WithSchedule(CServicePhoneStr* pPhoneStr, const char* pSimilarToThisString);
	STATUS                     AllocationPhoneMR_WithSchedule(CCommResApi* pCommResApi, BOOL isUpdate = FALSE);
	STATUS                     CreateOrUpdateSlaveRsrv(CCommResApi* pCommResApi);
	WORD                       GetMaxNumberOngoingConferences2C(BOOL accordingToCards = TRUE) const;
	STATUS                     CheckReservationSysMode(CCommResApi& i_CommResApi);
	void                       Rereserve(eRereserveType reReserveType, DWORD start = 0, DWORD end = 0);
	void                       ShiftStartTimeAndRereserve(WORD hour_shift, WORD min_shift, WORD sign);

	void                       AddIpServiceCalc(DWORD ipServiceId, float service_factor, BOOL round_up);
	void                       DumpCalculatorTotals() const;
	DWORD                      GetIpServiceIdForMinParties(CCommResApi* pCommResApi);

	void						CreateRsrvContinue1();
	CCommResApi*               StartConference(bool bTransactionContinue, DWORD monitorConfId = 0);
	void                       IncActiveForwardTransaction();
	DWORD                      DecActiveForwardTransaction();

	PDECLAR_MESSAGE_MAP

private:
	size_t                     StructTm2Interval(const CStructTm* timeStruct, int offset);

	void                       NumericIdConfigInit();

	STATUS                     CreateReserveInterval(DWORD start, DWORD end, DWORD monitorConfIdNotToAdd = 0, DWORD onlyMonitorConfIdSmallerThan = 0, BOOL bAlsoCheckNIDsOfMRs = TRUE, DWORD ipServiceId = ID_ALL_IP_SERVICES);

	void                       RecalculateReservationPartyResources();

	void                       SendResToConfParty(CCommResApi* pCommResApi, BOOL isMeetingRoom, bool bTransactionContinue);
	void                       StartTimerForNextConference();
	void                       OnTimerStartConference(CSegment* pMsg);
	void                       SetStartTimeIfStartImmediately(CCommResApi* pCommResApi, BOOL is_ong = FALSE);

	STATUS                     CreateConfFromAPI_NoSchedule(CCommResApi* pCommResApi);
	STATUS                     CreateRsrvFromAPI_WithSchedule(CCommResApi* pCommResApi, bool& bForwardTransaction, BOOL bForUpdate = FALSE, BOOL bForce = FALSE, BOOL bWriteNowToDisk = TRUE, BOOL bAlsoCheckNIDsOfMRs = TRUE);
	STATUS                     CreatePermanent_WithSchedule(CCommResApi* pCommResApi, bool& bForwardTransaction, DWORD* pProblemConfId, BOOL bForUpdate = FALSE, BOOL bForce = FALSE, BOOL bWriteNowToDisk = TRUE, BOOL bAlsoCheckNIDsOfMRs = TRUE);
	STATUS                     CreateConfFromMR_NoSchedule(CCommResApi* pCommResApi);
	STATUS                     CreateRsrvFromMR_WithSchedule(CCommResApi* pCommResApi);
	STATUS                     RsrvFromMR_WithSchedule(CCommResApi* pCommResApi);
	STATUS                     CreateAdHoc_NoSchedule(CCommResApi* pCommResApi);
	STATUS                     CreateAdHoc_WithSchedule(CCommResApi* pCommResApi, BOOL bUseOldConfId);

	eRsrcConfType              GetConfType(CCommResApi* pCommResApi);

	STATUS                     CheckIfOneMoreConferenceCanBeAdded(eRsrcConfType e_type, BOOL isOng = FALSE);
	STATUS                     CheckConfNameInResDB(CCommResApi* pCommResApi);
	STATUS                     CommonAllocationProcedureForNewConferencesOfAllTypes(CCommResApi* pCommResApi, eRsrcConfType c_type, BOOL bFromAPI, BOOL isOng = FALSE, BOOL bUseOldConfId = FALSE);
	STATUS                     AllocationProcedureForAPIConferences_NoSchedule(CCommResApi* pCommResApi, eRsrcConfType c_type);

	STATUS                     AllocateNonSpecificNumericId(char* numConfId);
	STATUS                     CheckNIDValidity(char* numConfId);
	STATUS                     FillRandomNID(char numConfId[NUMERIC_CONF_ID_MAX_LEN+1]);
	STATUS                     NIDCheckExist(CCommResApi* pCommResApi, eRsrcConfType c_type);
	STATUS                     NIDAllocation_NoSchedule(CCommResApi* pCommResApi, eRsrcConfType c_type);
	STATUS                     NIDAllocationAndResourcesCheckForConferences_WithSchedule(CConfRsrvRsrc* pconf, CCommResApi* pCommResApi, eRsrcConfType c_type, BOOL is_Startup = FALSE, BOOL bAlsoCheckNIDsOfMRs = TRUE);
	STATUS                     NIDAllocationAndResourcesCheckForPermanent(CConfRsrvRsrc* pconf, CCommResApi* pCommResApi, eRsrcConfType c_type,
	                                                                      BOOL is_Startup = FALSE, BOOL bAlsoCheckNIDsOfMRs = TRUE,
	                                                                      DWORD* problemConfId = NULL);
	STATUS                     NIDAllocationForSleepingConferences_WithSchedule(CCommResApi* pCommResApi, eRsrcConfType c_type);
	STATUS                     FindFreeNID(char numConfId[NUMERIC_CONF_ID_MAX_LEN+1]);
	STATUS                     CreateReserveConference_WithSchedule(CCommResApi* pCommResApi, CConfRsrvRsrc* pConfRsrvRsrc, eRsrcConfType c_type);
	STATUS                     ReReserveOneConference(CConfRsrvRsrc* pConfRsrvRsrc);

	void                       RollbackAndPrintStatus(char* strFromWhere, STATUS status, BOOL bRollBackMonitorConfId, BOOL bRemoveConf, CCommResApi* pCommResApi, eRsrcConfType c_type);

	void                       DumpStartTimesOfRecurrences(std::set<CStructTm>* pTimeRepeatSched);
	STATUS                     AddRecurrence(CCommResApi* pCommResApi, CStructTm& startTime, int index, BOOL& firstFound);

	void                       BuildRecurrenceName(char* name, const char* originalName, int index);
	void                       UpdateRoutingNameWithNID(CCommResApi* pCommResApi);

	void                       InitRsrvCalculatorPhoneList(CConfRsrvRsrc* pConf);

	STATUS                     CalculateMaxWeightOfAllReservation(WORD min_parties_permanent, WORD max_num_conf, BOOL isVSW, DWORD& problemConfId);

	size_t                     m_genesis;
	CReservationCalculator     m_ReservationCalculator;

	DWORD                      m_numPartiesInDongle; // dongle's restriction

	DWORD                      m_MAX_NUMBER_OF_RESERVATIONS;
	DWORD                      m_MAX_NUMBER_OF_MRS;
	DWORD                      m_MAX_NUMBER_OF_EQS;
	DWORD                      m_MAX_NUMBER_OF_SIP_FACTORIES;

	BOOL                       m_is_internal_reservator;
	BOOL                       m_is_2C_mode;

	eRPMode                    m_RPMode;

	CStructTm*                 m_pNearTime;
	DWORD                      m_NearConfId;
	DWORD                      m_numActiveForwardTransaction;

	WORD                       m_numMR;
	WORD                       m_numEQ;
	WORD                       m_numSipFact;

	DWORD                      m_num_conf_id_len;
	DWORD                      m_num_conf_id_max_len;
	DWORD                      m_num_conf_id_min_len;
	DWORD                      m_lastConfId;

	// ***monitor conference id - alocated to all reservations
	DWORD                      m_nextMntrConfId;

	std::set<DWORD>            m_monitorIdsToDelete;

	CCentralConferencesDB*     m_pCentralConferencesDB;

	CRepeatedReservationWriter m_RepeatedReservationWriter;

	friend class CSelfConsistency;
};

#endif /*RESERVATOR_H_*/
