// EncryptionKeyServerManager.cpp: implementation of the CEncryptionKeyServerManager class.
//
//////////////////////////////////////////////////////////////////////

#include "EncryptionKeyServerManager.h"
#include "OpcodesMcmsInternal.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "DHTask.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "TraceStream.h"
#include "Trace.h"
#include "FipsMode.h"
#include "McuMngrInternalStructs.h"
#include "FaultsDefines.h"
#include "ManagerApi.h"
#include "TerminalCommand.h"
#include <stdlib.h>

#define START_FILLING_ENCRYP_KEY_POLYCOM_DH_GEN_TABLE_TOUT_VALUE      10*SECOND
#define START_FILLING_ENCRYP_KEY_TANBERG_H323_DH_GEN_TABLE_TOUT_VALUE 10*SECOND
#define START_FILLING_ENCRYP_KEY_TANBERG_H320_DH_GEN_TABLE_TOUT_VALUE 10*SECOND


#define START_DELAY_BETWEEN_ENC_KEY_CHECK_TH_TOUT_VALUE				5
#define ENC_TO_MCUMNGR_IS_NTP_SYNC_REQ_TOUT_VALUE				3


////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CEncryptionKeyServerManager)
  ONEVENT(DH_IND_ON_NEW_KEY   	,ANYCASE    ,  CEncryptionKeyServerManager::OnDiffeiHelmanRcvNewKey )
  // ONEVENT(GET_HALF_KEY_REQ    	,ANYCASE    ,  CEncryptionKeyServerManager::OnReqForKeys )
  ONEVENT(START_CREATE_KEYS_REQ	,ANYCASE    ,  CEncryptionKeyServerManager::OnStartCreateKeysReq )
  ONEVENT(START_FILLING_ENCRYP_KEY_POLYCOM_DH_GEN_TABLE_TOUT, ANYCASE, CEncryptionKeyServerManager::OnTimerStartFillingEncryKeyTbl)
  ONEVENT(START_FILLING_ENCRYP_KEY_TANBERG_H320_DH_GEN_TABLE_TOUT,  ANYCASE, CEncryptionKeyServerManager::OnTimerStartFillingEncryKeyTbl)
  ONEVENT(START_FILLING_ENCRYP_KEY_TANBERG_H323_DH_GEN_TABLE_TOUT, ANYCASE, CEncryptionKeyServerManager::OnTimerStartFillingEncryKeyTbl)
  ONEVENT(DELAY_BETWEEN_GENERATE_ENC_KEY_TOUT, ANYCASE, CEncryptionKeyServerManager::OnTimerStartGenerateAnotherEncKey)
  ONEVENT(DELAY_BETWEEN_ENC_KEY_CHECK_TH_TOUT, ANYCASE, CEncryptionKeyServerManager::OnTimerStartCheckMinThEncKeys)

PEND_MESSAGE_MAP(CEncryptionKeyServerManager,CManagerTask);


////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CEncryptionKeyServerManager)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CEncryptionKeyServerManager::HandleOperLogin)
  ONCOMMAND("get_num_keys",CEncryptionKeyServerManager::HandleTerminalGetNumKeys,"get number of encryption Keys in the table")
  //ONCOMMAND("queue_info",CEncryptionKeyServerManager::HandleTerminalGetEncryptedQueueInfo,"get general encrypted shared mempry queue information")

END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CEncryptionKeyServerManager)
//  ONCOMMAND("ping",CEncryptionKeyServerManager::HandleTerminalPing,"test terminal commands")
END_TERMINAL_COMMANDS

extern void EncryptionKeyServerMonitorEntryPoint(void* appParam);



const DWORD CEncryptionKeyServerManager::DelayBetweenGenerateEncDefaultVal = 50;



////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void EncryptionKeyServerManagerEntryPoint(void* appParam)
{
	CEncryptionKeyServerManager * pEncryptionKeyServerManager = new CEncryptionKeyServerManager;
	pEncryptionKeyServerManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CEncryptionKeyServerManager::GetMonitorEntryPoint()
{
	return EncryptionKeyServerMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CEncryptionKeyServerManager::CEncryptionKeyServerManager()
{
	m_lastVisitInFillKeyIfNeeded_Id = 0;
	m_lastVisitInOnTimerStartCheckMinThEncKeys_Id = 0;
	m_needToCallFillKeyIfNeededOnHandler = YES;
	m_isFirstTimeTimerCalled = YES;
}

//////////////////////////////////////////////////////////////////////
CEncryptionKeyServerManager::~CEncryptionKeyServerManager()
{
	// No need to delete from shared memory
	//DeleteAllMembers();
	POBJDELETE(m_encyptedSharedMemoryTables);

}

//////////////////////////////////////////////////////////////////////
void CEncryptionKeyServerManager::CreateDHTask()
{
	m_pDHApi = new CDHApi;
	COsQueue dummyDHMbx;
	CreateTask(m_pDHApi, DHEntryPoint, &dummyDHMbx);

	PTRACE(eLevelInfoNormal, "CEncryptionKeyServerManager::CreateDHTas DH Task is Running");
}

//////////////////////////////////////////////////////////////////////
void CEncryptionKeyServerManager::ManagerPostInitActionsPoint()
{
	// Process startup tests
    ENCRYPTIONKEYSERVER_FIPS_140_TEST_INFO_S lStruct;
    lStruct.result  = TestAndEnterFipsMode();

	PTRACE(eLevelInfoNormal, "CEncryptionKeyServerManager::ManagerPostInitActionsPoint ");


	m_tablesWereFilled = NO;

    // Initialize shared memory queue
    m_encyptedSharedMemoryTables = new EncyptedSharedMemoryTables();



	// InitDHTables()          ;   //init the Tables
	CreateDHTask()          ;  //Create the DHTask
	GetSystemCfgParams()    ; //Read parmas
	InitTablesFillingOrder();//Set the filling order
	InitTablesFillingPolicy();


	CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();

    // ===== 1. get the unSimulationErrCode from SysConfig
	std::string eSimValue;
	pSysConfig->GetDataByKey(CFG_KEY_FIPS140_SIMULATE_ENCRYPTION_PROCESS_ERROR, eSimValue);
	eEncryptionFipsSimulationMode fips140SimulationEncryptionError = eInactiveSimulation;
	fips140SimulationEncryptionError = TranslateSysConfigDataToEnum(eSimValue);

	if(fips140SimulationEncryptionError == eFailDeterministicFipsTest)
	{
		PTRACE(eLevelError,
               "CEncryptionKeyServerManager::ManagerIdleActionsPoint - simulate TEST_FAILED");
		lStruct.result = STATUS_FAIL;
	}

	CSegment *pSeg = new CSegment;

	pSeg->Put((BYTE*)&lStruct, sizeof(ENCRYPTIONKEYSERVER_FIPS_140_TEST_INFO_S));

	CManagerApi apiMcuMngr(eProcessMcuMngr);


	apiMcuMngr.SendMsg(pSeg,
                       ENCRYPTION_KEY_SERVER_FIPS_140_TEST_RESULT_IND);


	CSegment rspMsg;
    OPCODE rspOpcode;
    STATUS responseStatus = STATUS_FAIL;
    if(CProcessBase::IsProcessAlive(eProcessMcuMngr))  // check if McuMngr is up don't send messgae if not
    	responseStatus  = apiMcuMngr.SendMessageSync(NULL,ENC_TO_MCUMNGR_IS_NTP_SYNC_REQ,ENC_TO_MCUMNGR_IS_NTP_SYNC_REQ_TOUT_VALUE*SECOND, rspOpcode,rspMsg);
	if (STATUS_OK == responseStatus ) //Check if Synch call success
    {
		PASSERTMSG(rspMsg.GetLen() <= 0, "ENC_TO_MCUMNGR_IS_NTP_SYNC_REQ returned empty result");
		BOOL   isNtpSyncLegal;
		rspMsg >> isNtpSyncLegal;
		if (isNtpSyncLegal == YES)
		{
			PTRACE(eLevelInfoNormal, "ENC_TO_MCUMNGR_IS_NTP_SYNC_REQ received isNtpSyncLegal YES - starting filling the table ");
			m_tablesWereFilled = YES;
			// start small timer for the first time
			StartTimer( DELAY_BETWEEN_ENC_KEY_CHECK_TH_TOUT, START_DELAY_BETWEEN_ENC_KEY_CHECK_TH_TOUT_VALUE*SECOND);

		}
		else
		{
			PTRACE(eLevelInfoNormal, "ENC_TO_MCUMNGR_IS_NTP_SYNC_REQ received isNtpSyncLegal NO - waiting to create message from McuMngr ");
		}
    }
	else
	{
		PTRACE(eLevelInfoNormal, "ENC_TO_MCUMNGR_IS_NTP_SYNC_REQ timeout - didn't receive isNtpSyncLegal from Mcu Manager ");
	}

}

/////////////////////////////////////////////////////////////////////////////
void CEncryptionKeyServerManager::GetSystemCfgParams()
{
	PTRACE(eLevelInfoNormal, "CEncryptionKeyServerManager::GetSystemCfgParams ");
	// get system.cfg values
	CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string key = "";
	std::string dataString = "";
	DWORD data = 0;
	BOOL isSystemTarget = NO;
	isSystemTarget = IsTarget();

	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();

	BOOL bIsSoftMCU = (eProductFamilySoftMcu == curProductFamily);

	//Filling Pizza values of the tables
	if (isSystemTarget == NO && !bIsSoftMCU)
	{
	  TRACESTR(eLevelInfoNormal) << "CEncryptionKeyServerManager::GetSystemCfgParams Running on Pizza sizes: G2= " << PIZZA_G2 << ",G3= " << PIZZA_G3 << ",G5 = " << PIZZA_G5<<std::endl;
	  m_maxTablesSizePerGenerator[POLYCOM_DH_GENERATOR] = PIZZA_G2;
	  m_maxTablesSizePerGenerator[TANBERG_H323_DH_GENERATOR] = PIZZA_G3;
	  m_maxTablesSizePerGenerator[TANBERG_H320_DH_GENERATOR] = PIZZA_G5;
	  return;
	}



	//filling G2 Data
	key = "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_POLYCOM";
	pSysConfig->GetDWORDDataByKey(key, data);
	m_maxTablesSizePerGenerator[POLYCOM_DH_GENERATOR] = data;
	TRACESTR(eLevelInfoNormal) << "CEncryptionKeyServerManager::GetSystemCfgParams Read: Size of Table G2, is " << int(data);

	//Filling G3 DB
	key = "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_TANDBERG_IP";
	pSysConfig->GetDWORDDataByKey(key, data);
	m_maxTablesSizePerGenerator[TANBERG_H323_DH_GENERATOR] = data;
	TRACESTR(eLevelInfoNormal) << "CEncryptionKeyServerManager::GetSystemCfgParams Read: Size of Table G3, is " << int(data);

	//Filling G5 DB
	key = "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_TANDBERG_ISDN";
	pSysConfig->GetDWORDDataByKey(key, data);
	m_maxTablesSizePerGenerator[TANBERG_H320_DH_GENERATOR] = data;
	TRACESTR(eLevelInfoNormal) << "CEncryptionKeyServerManager::GetSystemCfgParams Read: Size of Table G5, is " << int(data);

	m_delayBetweenGenerateEnc = DelayBetweenGenerateEncDefaultVal;
	CSysConfig* cfg = CProcessBase::GetProcess()->GetSysConfig();
	BOOL res = cfg->GetDWORDDataByKey(CFG_KEY_DELAY_BETWEEN_GENERATE_ENC_KEY, m_delayBetweenGenerateEnc);
	PASSERTSTREAM(!res, "CSysConfig::GetDWORDDataByKey: " << CFG_KEY_DELAY_BETWEEN_GENERATE_ENC_KEY);
	TRACESTR(eLevelInfoNormal) << "CEncryptionKeyServerManager::GetSystemCfgParams Rm_delayBetweenGenerateEnc " << m_delayBetweenGenerateEnc;
}

///////////////////////////////////////////////////////////////////////////////
void CEncryptionKeyServerManager::OnStartCreateKeysReq(CSegment * pSeg)
{
	if (m_tablesWereFilled == NO)
	{
		PTRACE(eLevelInfoNormal,"CEncryptionKeyServerManager::OnStartCreateKeysReq filling the table");
		//Start the keys poolling
		BYTE bIsGenKeyFromStartup = YES;
		FillKeyIfNeeded(bIsGenKeyFromStartup);


		// start small timer for the first time
		StartTimer( DELAY_BETWEEN_ENC_KEY_CHECK_TH_TOUT, START_DELAY_BETWEEN_ENC_KEY_CHECK_TH_TOUT_VALUE*SECOND);

	}
	else {
		PTRACE(eLevelInfoNormal,"CEncryptionKeyServerManager::OnStartCreateKeysReq -  table was already filled in");
	}
}




///////////////////////////////////////////////////////////////////////////////
void CEncryptionKeyServerManager::OnDiffeiHelmanRcvNewKey(CSegment * pSeg)
{
	DWORD fips140Status;
	DWORD generator;
	BYTE bIsGenKeyFromStartup = NO;

	*pSeg >> fips140Status;
	*pSeg >> bIsGenKeyFromStartup;

	CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	// ===== 1. get the unSimulationErrCode from SysConfig

	std::string eSimValue;
	pSysConfig->GetDataByKey(CFG_KEY_FIPS140_SIMULATE_ENCRYPTION_PROCESS_ERROR, eSimValue);
	eEncryptionFipsSimulationMode fips140SimulationEncryptionError = eInactiveSimulation;

	fips140SimulationEncryptionError = TranslateSysConfigDataToEnum(eSimValue);

	if(fips140Status == STATUS_OK &&
       fips140SimulationEncryptionError == eFailPoolGenerationFipsTest &&
       GetNumberOfKeys() == 7)
	{
		PTRACE(eLevelError,
               "CEncryptionKeyServerManager::OnDiffeiHelmanRcvNewKey - simulate TEST_FAILED");

		fips140Status = STATUS_FAIL;
	}

	if(fips140Status != STATUS_OK)// what should be the condition for checking - if the fips140 is on and the return value equal to ??
	{	// what should be the response to the RMX manager?
		PTRACE2INT(eLevelError,
                   "OnRcvNewKey Fips140 test has failed !!!! Number of avialable keys - "
                   , GetNumberOfKeys());

		PASSERT(fips140Status);// also convert the returned error value to string and print it.

		if(bIsGenKeyFromStartup)
		{// only if we are in process startup mode we need to operate active alarm

			COstrStream msg;
			msg << "Failed to create Half Key in CEncryptionKeyServerManager, Current Keys number is - " << GetNumberOfKeys();
			CProcessBase *pProcess = CProcessBase::GetProcess();

			pProcess->AddActiveAlarmFromProcess (FAULT_GENERAL_SUBJECT,
                                                 AA_ENCRYPTION_SERVER_ERROR,
                                                 MAJOR_ERROR_LEVEL,
                                                 msg.str().c_str(),
                                                 true,
                                                 true);
		}
		return;
	}

	*pSeg >> generator;

	EncryptedKey encryptedKey;
	encryptedKey.DeSerialize(*pSeg);


	// TRACESTR(eLevelInfoHigh) << " before  QueueEncryptedKey " << encryptedKey;


	const std::map< DWORD , SharedMemoryEncryptedKeyQueue* >& sharedMemoryEncryptedKeyQueueMap = m_encyptedSharedMemoryTables->GetSharedMemoryEncryptedKeyQueueMap();
	std::map< DWORD , SharedMemoryEncryptedKeyQueue* >::const_iterator itQueue = sharedMemoryEncryptedKeyQueueMap.find(generator);

	if(itQueue == sharedMemoryEncryptedKeyQueueMap.end())
	{
		TRACESTR(eLevelError) <<  "OnDiffeiHelmanRcvNewKey  no table for generator " <<generator;
	}

	if (itQueue != sharedMemoryEncryptedKeyQueueMap.end() && itQueue->second->GetNumOfEntries() < m_maxTablesSizePerGenerator[generator])
	{
		if (m_encyptedSharedMemoryTables->QueueEncryptedKey(generator, encryptedKey)  != STATUS_OK)
		{
			TRACESTR(eLevelError) << "OnDiffeiHelmanRcvNewKey: Failed Queue encrypted key generator " << generator;
		}
	}
	if (bIsGenKeyFromStartup)
	{


		FillKeyIfNeeded(bIsGenKeyFromStartup);
	}
	else
	{
		// TRACESTR(eLevelError) << "After filling key start timer  " ;

		StartTimer(DELAY_BETWEEN_GENERATE_ENC_KEY_TOUT, m_delayBetweenGenerateEnc);

	}

}

///////////////////////////////////////////////////////////////////////////////

void CEncryptionKeyServerManager::OnTimerStartGenerateAnotherEncKey(CSegment*)
{
	 FillKeyIfNeeded(NO);
}

///////////////////////////////////////////////////////////////////////////////
void CEncryptionKeyServerManager::OnTimerStartCheckMinThEncKeys()
{
	TRACESTR(eLevelInfoNormal) <<  "EncryptionKeyServerManager::OnTimerStartCheckMinThEncKey m_needToCallFillKeyIfNeededOnHandler " <<  (int)m_needToCallFillKeyIfNeededOnHandler;

	if (m_needToCallFillKeyIfNeededOnHandler)
	{
		FillKeyIfNeeded(m_isFirstTimeTimerCalled);
	}
	else
	{
		// to be on the safe  side. In case FillKeyIfNeeded stopped working because for example response didnt return from task.
		if (m_lastVisitInOnTimerStartCheckMinThEncKeys_Id <  m_lastVisitInOnTimerStartCheckMinThEncKeys_Id)
		{
			// This mean we FillKeyIfNeeded was never called since m_needToCallFillKeyIfNeededOnHandler became false => maybe there is problem so call it anyway!
			TRACESTR(eLevelInfoNormal) << "It seems that FillKeyIfNeeded was never called since m_needToCallFillKeyIfNeededOnHandler became false => maybe there is problem so call it anyway";
			FillKeyIfNeeded(NO);
		}
		// else all seems to be OK
		++m_lastVisitInOnTimerStartCheckMinThEncKeys_Id;
	}

	if (m_isFirstTimeTimerCalled )
	{
		m_isFirstTimeTimerCalled = NO;
	}

	m_lastVisitInFillKeyIfNeeded_Id =  m_lastVisitInOnTimerStartCheckMinThEncKeys_Id;
	StartTimer( DELAY_BETWEEN_ENC_KEY_CHECK_TH_TOUT, START_DELAY_BETWEEN_ENC_KEY_CHECK_TH_TOUT_VALUE*SECOND);
}

void CEncryptionKeyServerManager::FillKeyIfNeeded(BOOL bIsGenKeyFromStartup)
{
	++m_lastVisitInFillKeyIfNeeded_Id;


	const std::map< DWORD , SharedMemoryEncryptedKeyQueue* >& sharedMemoryEncryptedKeyQueueMap = m_encyptedSharedMemoryTables->GetSharedMemoryEncryptedKeyQueueMap();
	std::map< DWORD , SharedMemoryEncryptedKeyQueue* >::const_iterator  itQueue;


	for (unsigned int i=0; i < m_tablesFillingOrder.size(); ++i)
	{
		DWORD generatorTable = m_tablesFillingOrder[i];
		itQueue = sharedMemoryEncryptedKeyQueueMap.find(generatorTable);

		if (itQueue == sharedMemoryEncryptedKeyQueueMap.end() ||  m_fillingTablePolicyPerGenerator.find(generatorTable) == m_fillingTablePolicyPerGenerator.end())
		{
			m_needToCallFillKeyIfNeededOnHandler = TRUE;
			PASSERTMSG(TRUE, "UnExpected error occurred - Invalid generatorTable id");
			return;
		}

		DWORD tableSize = itQueue->second->GetNumOfEntries();

		//Sometimes print - so we can see what happens...
		if (m_lastVisitInFillKeyIfNeeded_Id % 50 <=2 )
		{
			TRACESTR(eLevelInfoNormal) << " Number of keys in table " << generatorTable << " : " <<  tableSize;
		}

		if (tableSize < m_maxTablesSizePerGenerator[generatorTable])
		{
			if (tableSize <=  m_fillingTablePolicyPerGenerator[generatorTable].dwMinTableSize )
			{
				// TRACESTR(eLevelInfoNormal) << " OnTimerStartCheckMinThEncKeys " << generatorTable << "to file " <<  (int)m_fillingTablePolicyPerGenerator[generatorTable].bToFillTable;
				m_needToCallFillKeyIfNeededOnHandler = FALSE;
				m_lastVisitInFillKeyIfNeeded_Id =  m_lastVisitInOnTimerStartCheckMinThEncKeys_Id;
				if(!m_fillingTablePolicyPerGenerator[generatorTable].bToFillTable)
				{
					TRACESTR(eLevelInfoNormal) << " DeleteTimer min the of " << generatorTable;
					m_fillingTablePolicyPerGenerator[generatorTable].bToFillTable = YES;
					DeleteTimer(m_fillingTablePolicyPerGenerator[generatorTable].dwStartFillingTableTimer); // we dont need the timer to start filling the table, the number keys is less than the min threshold
				}
				m_pDHApi->GetNewKey(generatorTable, bIsGenKeyFromStartup); //Rew for more keys of this type
				return;
			}
			// else in this case m_fillingTablePolicyPerGenerator[generatorTable].bIsTimerStarted should be YES
		}
	}

	for (unsigned int i=0; i < m_tablesFillingOrder.size(); ++i)
	{
		DWORD generatorTable = m_tablesFillingOrder[i];
		itQueue = sharedMemoryEncryptedKeyQueueMap.find(generatorTable);

		DWORD tableSize = itQueue->second->GetNumOfEntries();

		if (tableSize < m_maxTablesSizePerGenerator[generatorTable])
		{
			if ( m_fillingTablePolicyPerGenerator[generatorTable].bToFillTable )
			{
				// TRACESTR(eLevelInfoNormal) << "OnTimerStartCheckMinThEncKeys " << generatorTable << "to file " << (int) m_fillingTablePolicyPerGenerator[generatorTable].bToFillTable;
				m_needToCallFillKeyIfNeededOnHandler = FALSE;
				m_lastVisitInFillKeyIfNeeded_Id =  m_lastVisitInOnTimerStartCheckMinThEncKeys_Id;
				if(!m_fillingTablePolicyPerGenerator[generatorTable].bToFillTable)
				{
					m_fillingTablePolicyPerGenerator[generatorTable].bToFillTable = YES;
					DeleteTimer(m_fillingTablePolicyPerGenerator[generatorTable].dwStartFillingTableTimer); // we dont need the timer to start filling the table, the number keys is less than the min threshold
				}
				m_pDHApi->GetNewKey(generatorTable, bIsGenKeyFromStartup); //Rew for more keys of this type
				return;
			}
			// else in this case m_fillingTablePolicyPerGenerator[generatorTable].bIsTimerStarted should be YES
		}

		if (m_fillingTablePolicyPerGenerator[generatorTable].bIsGenKeyFromStartup )
		{
			m_fillingTablePolicyPerGenerator[generatorTable].bIsGenKeyFromStartup = NO;
		}

		// Actually one of the conditions will do
		if (m_fillingTablePolicyPerGenerator[generatorTable].bToFillTable || !m_fillingTablePolicyPerGenerator[generatorTable].bIsTimerStarted)
		{
			TRACESTR(eLevelInfoNormal) << "Stop filling tables "  ;
			m_fillingTablePolicyPerGenerator[generatorTable].bToFillTable = FALSE;
			m_fillingTablePolicyPerGenerator[generatorTable].bIsTimerStarted = YES;
			CSegment *tableInfo = new CSegment;
			* tableInfo << generatorTable;
			StartTimer(m_fillingTablePolicyPerGenerator[generatorTable].dwStartFillingTableTimer, m_fillingTablePolicyPerGenerator[generatorTable].dwStartFillingTableTimerValue, tableInfo);
		}
	}
	// If we reached here we didint call GetNewKey and we will not reach here again otherwise
	m_needToCallFillKeyIfNeededOnHandler = TRUE;

}


///////////////////////////////////////////////////////////////////////////////
void  CEncryptionKeyServerManager::SelfKill()
{
	m_pDHApi->SyncDestroy();
	POBJDELETE(m_pDHApi);
	// no need to delete from shared memory
	// DeleteAllMembers();
	CManagerTask::SelfKill();
}
///////////////////////////////////////////////////////////////////////////////
void  CEncryptionKeyServerManager::SendFailReq(STATUS reason)
{
	CSegment * pRetParam = new CSegment;
	HALF_KEY_IND_S resParams;

	memset(&resParams,0,sizeof(HALF_KEY_IND_S));
	resParams.status = reason;

	pRetParam->Put( (BYTE *)&resParams,sizeof(HALF_KEY_IND_S));

	ResponedClientRequest(GET_HALF_KEY_IND, pRetParam);
}

///////////////////////////////////////////////////////////////////////////////

DWORD  CEncryptionKeyServerManager::GetNumberOfKeys()
{
	DWORD keysNumber = 0;

	const std::map< DWORD , SharedMemoryEncryptedKeyQueue* >& sharedMemoryEncryptedKeyQueueMap = m_encyptedSharedMemoryTables->GetSharedMemoryEncryptedKeyQueueMap();
	for(std::map< DWORD , SharedMemoryEncryptedKeyQueue* >::const_iterator it =sharedMemoryEncryptedKeyQueueMap.begin() ;
			it != sharedMemoryEncryptedKeyQueueMap.end(); ++it)
	{
		keysNumber += it->second->GetNumOfEntries();
	}
	return keysNumber;
}

// ///////////////////////////////////////////////////////////////////////////////
// char* CEncryptionKeyServerManager::ConvertFipsRvOpcodes2String(int rval)
// {
// 	if(rval == E_FIPS_RV_OK)
// 		return "E_FIPS_RV_OK";
// 	else if(rval == E_FIPS_RV_ERROR_PRG_INIT_FAILURE)
// 		return "E_FIPS_RV_ERROR_PRG_INIT_FAILURE";
// 	else if(rval == E_FIPS_RV_ERROR_RANDOMIZATION_FAILURE)
// 		return "E_FIPS_RV_ERROR_RANDOMIZATION_FAILURE";
// 	else if(rval == E_FIPS_RV_ERROR_SAME_SEED_DIFFERENT_RES)
// 		return "E_FIPS_RV_ERROR_SAME_SEED_DIFFERENT_RES";
// 	else if(rval == E_FIPS_RV_ERROR_SEED_VALUE_FAILURE)
// 		return "E_FIPS_RV_ERROR_SEED_VALUE_FAILURE";
// 	else if(rval == E_FIPS_RV_ERROR_SHA1_FAILURE)
// 		return "E_FIPS_RV_ERROR_SHA1_FAILURE";

// 	return "E_FIPS_RV_END";
// }

///////////////////////////////////////////////////////////////////////////////
void CEncryptionKeyServerManager::InitTablesFillingOrder()
{
	PTRACE(eLevelInfoNormal, "CEncryptionKeyServerManager::InitTablesFillingOrder setting the order of the tables filling ");

	//This is the order of the filling, only this table will be filled
	m_tablesFillingOrder.push_back(POLYCOM_DH_GENERATOR);
	m_tablesFillingOrder.push_back(TANBERG_H323_DH_GENERATOR);
	m_tablesFillingOrder.push_back(TANBERG_H320_DH_GENERATOR);
}
///////////////////////////////////////////////////////////////////////////////
void CEncryptionKeyServerManager::InitTablesFillingPolicy()
{
	///POLYCOM GENERATOR
	m_fillingTablePolicyPerGenerator[POLYCOM_DH_GENERATOR].bToFillTable = YES;
	m_fillingTablePolicyPerGenerator[POLYCOM_DH_GENERATOR].dwStartFillingTableTimer = START_FILLING_ENCRYP_KEY_POLYCOM_DH_GEN_TABLE_TOUT;
	m_fillingTablePolicyPerGenerator[POLYCOM_DH_GENERATOR].dwStartFillingTableTimerValue = START_FILLING_ENCRYP_KEY_POLYCOM_DH_GEN_TABLE_TOUT_VALUE;
	m_fillingTablePolicyPerGenerator[POLYCOM_DH_GENERATOR].bIsTimerStarted = NO;
	m_fillingTablePolicyPerGenerator[POLYCOM_DH_GENERATOR].bIsGenKeyFromStartup = YES;
	m_fillingTablePolicyPerGenerator[POLYCOM_DH_GENERATOR].dwMinTableSize = m_maxTablesSizePerGenerator[POLYCOM_DH_GENERATOR]*MIN_FILLING_PRECENTAGE_THRESHOLD/100;

	TRACESTR(eLevelInfoNormal) << "POLYCOM_DH_GENERATOR dwMinTableSize = " << m_fillingTablePolicyPerGenerator[POLYCOM_DH_GENERATOR].dwMinTableSize ;


	///TANDBERG H323 GENERATOR
	m_fillingTablePolicyPerGenerator[TANBERG_H323_DH_GENERATOR].bToFillTable = YES;
	m_fillingTablePolicyPerGenerator[TANBERG_H323_DH_GENERATOR].dwStartFillingTableTimer = START_FILLING_ENCRYP_KEY_TANBERG_H323_DH_GEN_TABLE_TOUT;
	m_fillingTablePolicyPerGenerator[TANBERG_H323_DH_GENERATOR].dwStartFillingTableTimerValue = START_FILLING_ENCRYP_KEY_TANBERG_H323_DH_GEN_TABLE_TOUT_VALUE;
	m_fillingTablePolicyPerGenerator[TANBERG_H323_DH_GENERATOR].bIsTimerStarted = NO;
	m_fillingTablePolicyPerGenerator[TANBERG_H323_DH_GENERATOR].bIsGenKeyFromStartup = YES;
	m_fillingTablePolicyPerGenerator[TANBERG_H323_DH_GENERATOR].dwMinTableSize = m_maxTablesSizePerGenerator[TANBERG_H323_DH_GENERATOR]*MIN_FILLING_PRECENTAGE_THRESHOLD/100;

	TRACESTR(eLevelInfoNormal) << "TANBERG_H323_DH_GENERATOR dwMinTableSize = " << m_fillingTablePolicyPerGenerator[TANBERG_H323_DH_GENERATOR].dwMinTableSize ;


	///TANDBERG H320 GENERATOR
	m_fillingTablePolicyPerGenerator[TANBERG_H320_DH_GENERATOR].bToFillTable = YES;
	m_fillingTablePolicyPerGenerator[TANBERG_H320_DH_GENERATOR].dwStartFillingTableTimer = START_FILLING_ENCRYP_KEY_TANBERG_H320_DH_GEN_TABLE_TOUT;
	m_fillingTablePolicyPerGenerator[TANBERG_H320_DH_GENERATOR].dwStartFillingTableTimerValue = START_FILLING_ENCRYP_KEY_TANBERG_H320_DH_GEN_TABLE_TOUT_VALUE;
	m_fillingTablePolicyPerGenerator[TANBERG_H320_DH_GENERATOR].bIsTimerStarted = NO;
	m_fillingTablePolicyPerGenerator[TANBERG_H320_DH_GENERATOR].bIsGenKeyFromStartup = YES;
	m_fillingTablePolicyPerGenerator[TANBERG_H320_DH_GENERATOR].dwMinTableSize = m_maxTablesSizePerGenerator[TANBERG_H320_DH_GENERATOR]*MIN_FILLING_PRECENTAGE_THRESHOLD/100;

	TRACESTR(eLevelInfoNormal) << "TANBERG_H320_DH_GENERATOR dwMinTableSize = " << m_fillingTablePolicyPerGenerator[TANBERG_H320_DH_GENERATOR].dwMinTableSize ;
}

///////////////////////////////////////////////////////////////////////////////////////////
void CEncryptionKeyServerManager::OnTimerStartFillingEncryKeyTbl(CSegment * pSeg)
{
	DWORD generatorTable;
	*pSeg >> generatorTable;
	BYTE bIsGenKeyFromStartup = NO;

	if(generatorTable)
	{
	// 	PTRACE2INT(eLevelInfoNormal, "CEncryptionKeyServerManager::OnTimerStartFillingEncryKeyTblgeneratorTable = ", generatorTable);

		const std::map< DWORD , SharedMemoryEncryptedKeyQueue* >& sharedMemoryEncryptedKeyQueueMap = m_encyptedSharedMemoryTables->GetSharedMemoryEncryptedKeyQueueMap();
		std::map< DWORD , SharedMemoryEncryptedKeyQueue* >::const_iterator itQueue = sharedMemoryEncryptedKeyQueueMap.find(generatorTable);

		if(itQueue != sharedMemoryEncryptedKeyQueueMap.end())
		{
			if (itQueue->second->GetNumOfEntries() <  m_maxTablesSizePerGenerator[generatorTable])
			{
				m_fillingTablePolicyPerGenerator[generatorTable].bToFillTable = YES;
				m_fillingTablePolicyPerGenerator[generatorTable].bIsTimerStarted = NO;
				// Instead of calling this - wait for the check timer to do so.
				// FillKeyIfNeeded(NO);

				return;
			}
		}
		else
		{
			TRACESTR(eLevelError) <<  "OnTimerStartFillingEncryKeyTbl failed get queue for generator " << generatorTable;
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CEncryptionKeyServerManager::OnTimerStartFillingEncryKeyTbl generatorTable null");

	}
	// start timer again
	m_fillingTablePolicyPerGenerator[generatorTable].bIsTimerStarted = YES;
	CSegment *tableInfo = new CSegment;
	* tableInfo << generatorTable;

	StartTimer(m_fillingTablePolicyPerGenerator[generatorTable].dwStartFillingTableTimer, m_fillingTablePolicyPerGenerator[generatorTable].dwStartFillingTableTimerValue, tableInfo);

}


//////////////////////////////////////////////////////////////////////
eEncryptionFipsSimulationMode CEncryptionKeyServerManager::TranslateSysConfigDataToEnum(std::string & data)
{
	eEncryptionFipsSimulationMode eSimValue = eInactiveSimulation;

	if(data == "FAIL_DETERMINISTIC_TEST")
		eSimValue = eFailDeterministicFipsTest;
	if(data == "FAIL_POOL_GENERATION_TEST")
		eSimValue = eFailPoolGenerationFipsTest;

	return eSimValue;
}

//////////////////////////////////////////////////////////////////////

STATUS CEncryptionKeyServerManager::HandleTerminalGetNumKeys(CTerminalCommand& command, std::ostream& ans)
{
     const string &strTableID = command.GetToken(eCmdParam1);
     int iTableID = atoi(strTableID.c_str());
     //if (""==strTableID || iTableID <0 || iTableID > 2)
     if (""==strTableID || iTableID <0 || iTableID >= (int)m_tablesFillingOrder.size())
     {
         ans << "error: Please enter a valid Tabel ID betweene 0 to " << m_tablesFillingOrder.size() << "\n";
         ans << "usage: Bin/McuCmd get_num_keys [TableID]\n";
         return STATUS_FAIL;
     }

     const SharedMemoryEncryptedKeyQueue*  sharedMemoryEncryptedKeyQueue = m_encyptedSharedMemoryTables->GetSharedMemory(m_tablesFillingOrder[iTableID]);


     if (sharedMemoryEncryptedKeyQueue == NULL)
     {
    	 ans << "No shared memory table for generator " << m_tablesFillingOrder[iTableID] << "\n";
    	 return STATUS_FAIL;
     }
 	 DWORD tableSize = sharedMemoryEncryptedKeyQueue->GetNumOfEntries();

     ans << "Number of Keys are (iTableID) " << iTableID <<  " (generator) " << m_tablesFillingOrder[iTableID] << ": " << tableSize <<"\n";

     return STATUS_OK;
}
