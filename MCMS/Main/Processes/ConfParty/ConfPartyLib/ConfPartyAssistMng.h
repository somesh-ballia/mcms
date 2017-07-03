#ifndef _CONF_PARTY_ASSIST_MANAGER
#define _CONF_PARTY_ASSIST_MANAGER

#include <set>

#include "TaskApi.h" 
#include "ManagerTask.h"
#include "ConfPartyGlobals.h"
#include "ConfPartyProcess.h"
#include "GKManagerStructs.h"


class CBridgePartyVideoUniDirection;

extern "C" void ConfPartyAssistMngEntryPoint(void* appParam);
const WORD PRINT_PROCESS_MEMORY_INFO_TOUT = 60 * SECOND;


typedef std::map<std::pair<DWORD, DWORD>, ALLOC_STATUS_PER_UNIT_S> VIDEO_PARTIES_MAP;


class CConfPartyAssistMng : public CTaskApp //CManagerTask
{
	CLASS_TYPE_1(CConfPartyAssistMng, CTaskApp)
public:

	virtual const char* NameOf() const { return "CConfPartyAssistMng"; }

    CConfPartyAssistMng();
    virtual ~CConfPartyAssistMng();
	virtual void Create(CSegment& appParam, WORD limited=FALSE); // called from entry point
	virtual eTaskRecoveryPolicyAfterSeveralRetries GetTaskRecoveryPolicyAfterSeveralRetries() const {return eCreateNewTask;}
	BOOL  IsSingleton() const { return YES; }
	const char* GetTaskName() const;
	void  InitTask() {;}

	void SendAllocatedResourcesMapToMPL(stBoardUnitParams unitParams);
	void OnCheckIfAllAcksRecievedTout(CSegment* pParam);
	void OnProcessMemInfoPrint(CSegment* pParam);
	void OnProcessMemInfoPrintTout(CSegment* pParam);

	void SendAllVideoParamsToMPL(ALLOC_STATUS_PER_UNIT_S * allocStatusPerUnit,BYTE boxId,BYTE boardId,BYTE unitId);
	void CleanConf(CSegment* pSeg);
	void CleanParty(CSegment* pSeg);
	void DelConfToSipProxy(const char* confName, DWORD confId);
	STATUS OnConfPartyListInd(CSegment* pParam);
	void   SendDataToVideoBridge( CONF_PARTY_LIST_S* pParamData);
	void   Dump(VB_PARTY_VIDEO_LIST_S* pPartyList);
	void   SendPartyListReq(DWORD boardId ,DWORD unitId );
	void   OnGetPartyVideoListInd(CSegment* pParam);

 protected:
	void OnKillPortAck(CSegment* pSeg);
	void OnAllocStatusPerUnitAck(CSegment* pSeg);

 private:

	CConfPartyProcess*       m_pProcess;

	BYTE m_faultyUnitsTable [MAX_NUM_OF_BOARDS][MAX_NUM_OF_UNITS];

	VIDEO_PARTIES_MAP m_videoParamsPerUnit;

	WORD m_printTimeout;

	std::set<stBoardUnitParams> m_faultyUnitsWaitingForAck;

	std::vector<string> m_psColumnNamesVec;

/* 	PDECLAR_TERMINAL_COMMANDS */
	PDECLAR_MESSAGE_MAP
};

#endif
