// EncryptionKeyServerManager.h

#if !defined(_ENCMANAGER_H__)
#define _ENCMANAGER_H__

#include <map>
#include <vector>

#include "ManagerTask.h"
#include "Macros.h"
#include "DHTable.h"
#include "StatusesGeneral.h"
#include "EncryptionCommon.h"


//FIPS140 simulation values
enum eEncryptionFipsSimulationMode
{
	eInactiveSimulation,
	eFailDeterministicFipsTest,
	eFailPoolGenerationFipsTest
};

void EncryptionKeyServerManagerEntryPoint(void* appParam);

// Used to be 25 - changed to 50
#define MIN_FILLING_PRECENTAGE_THRESHOLD 50

typedef struct
{
	BYTE  bToFillTable;
	DWORD dwStartFillingTableTimer;
	DWORD dwStartFillingTableTimerValue;
	BYTE  bIsTimerStarted;
	BYTE  bIsGenKeyFromStartup;
	DWORD dwMinTableSize;
} FillingTablePolicy;

class CDHApi;

class CEncryptionKeyServerManager : public CManagerTask
{
CLASS_TYPE_1(CEncryptionKeyServerManager,CManagerTask )
public:

	typedef std::map< DWORD , DWORD > MaxTablesSizePerGenerator;
	typedef std::map<DWORD, FillingTablePolicy> FillingTablePolicyPerGenerator;
	typedef std::vector< DWORD > TablesFillingOrder;

	void ManagerPostInitActionsPoint();
	CEncryptionKeyServerManager();
	virtual ~CEncryptionKeyServerManager();
	virtual void  SelfKill();

	TaskEntryPoint GetMonitorEntryPoint();

private:

	void CreateDHTask();
	void GetSystemCfgParams();
		
	void SendFailReq(STATUS reason=STATUS_FAIL);
		
	void InitTablesFillingOrder();
	DWORD GetNumberOfKeys();
	eEncryptionFipsSimulationMode TranslateSysConfigDataToEnum(std::string & data);
	void InitTablesFillingPolicy();

	void OnStartCreateKeysReq(CSegment* pSeg);
	void OnDiffeiHelmanRcvNewKey(CSegment* pSeg);
	
	void OnTimerStartFillingEncryKeyTbl(CSegment* pSeg);
	void OnTimerStartGenerateAnotherEncKey(CSegment* pSeg);
	
	void OnTimerStartCheckMinThEncKeys();
		
	void FillKeyIfNeeded(BOOL bIsGenKeyFromStartup);
	
	STATUS HandleTerminalGetNumKeys(CTerminalCommand& command, std::ostream& ans);
	
	
	
	MaxTablesSizePerGenerator m_maxTablesSizePerGenerator      ;  //holds for each gen the maxSize
	FillingTablePolicyPerGenerator m_fillingTablePolicyPerGenerator;
	TablesFillingOrder m_tablesFillingOrder                    ; //holds the order of the SetGenerator logic
	CDHApi* m_pDHApi                                           ;//DH task api
	
	DWORD	m_delayBetweenGenerateEnc;
	
		
	static const DWORD DelayBetweenGenerateEncDefaultVal;
	
	EncyptedSharedMemoryTables*  m_encyptedSharedMemoryTables;
	
	BOOL	m_tablesWereFilled;
	
	BOOL m_needToCallFillKeyIfNeededOnHandler;
	// This is are  for checking. In case we missed response from DHTable task - timer handler will identify it and call  FillKeyIfNeeded even if it doesnt need
	DWORD m_lastVisitInFillKeyIfNeeded_Id;
	DWORD m_lastVisitInOnTimerStartCheckMinThEncKeys_Id;	
	DWORD m_isFirstTimeTimerCalled;
	

	PDECLAR_MESSAGE_MAP;
	PDECLAR_TRANSACTION_FACTORY;
	PDECLAR_TERMINAL_COMMANDS;
	
	
};

#endif // !defined(_ENCMANAGER_H__)
