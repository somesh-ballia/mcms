#ifndef UTILITY_MANAGER_H__
#define UTILITY_MANAGER_H__

////////////////////////////////////////////////////////////////////////////
#include "ManagerTask.h"

#include "FileDownloaderTask.h"

#include "TcpDumpEntity.h"

#include "Macros.h"
#include "CardsStructs.h"

#include "SlideConvertTask.h"

#include "SharedMcmsCardsStructs.h"
////////////////////////////////////////////////////////////////////////////
class CUtilityProcess;

////////////////////////////////////////////////////////////////////////////
extern void UtilityMonitorEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
class CUtilityManager : public CManagerTask
{
	CLASS_TYPE_1(CUtilityManager, CManagerTask)

public:

	CUtilityManager();

	STATUS ClearTcpDumpStorage();
	STATUS SendStartTcpDumpToConfigurator(CTcpDumpEntity* ent, char* startTimeStr, char* ipStr, DWORD allocatedUnits);
	STATUS SendStopTcpDumpToConfigurator();

private: // prohibit copying

	CUtilityManager(const CUtilityManager&);
	void operator =(const CUtilityManager&);

private: // overridden

	virtual void ManagerPostInitActionsPoint();

	virtual TaskEntryPoint GetMonitorEntryPoint()
	{ return UtilityMonitorEntryPoint; }

	virtual int GetTaskMbxBufferSize() const
	{ return 1024*1024-1; }

private:

	STATUS StopTcpDump(eTcpDumpState tcpDumpState);
	STATUS SendStartTcpDumpToCardsMngr(DWORD boardId, const char* param);
	STATUS SendStopTcpDumpToCardsMngr(DWORD boardId);

private: // transactions

	STATUS HandleStartTcpDump(CRequest* pRequest);
	STATUS HandleStopTcpDump(CRequest* pRequest);
	STATUS HandleTcpDumpClearStorage(CRequest* pRequest);

private: // actions

	void OnCSTCPDumpIPServiceParamInd(CSegment* seg);

	void HandleNoStartTcpDumpIndicationReceivedTimer(CSegment* = NULL);
	void HandleNoStopTcpDumpIndicationReceivedTimer(CSegment* = NULL);
	void HandleRequestIPServiceTimer(CSegment* = NULL);

	void OnStopTcpDump(CSegment* = NULL);
	void OnStartTcpDump(CSegment* seg);

	void OnStartTcpDumpTimer(CSegment* = NULL);

	void OnClearTcpDumpStorage(CSegment* = NULL);

	void OnStopTcpDumpStatusInd(CSegment* seg);
	void OnStartTcpDumpStatusInd(CSegment* seg);

	void OnMediaIpParamsFromCsInd(CSegment* seg);

	void OnDownloadFile(CSegment* seg);
	void OnConvertSlide(CSegment* seg);

	void ForwardTraceToLogger(CSegment* seg);

	void OnSystemCardsModeInd(CSegment* pMsg);

	void SendSystemBasedModeReqToCardMngr();

	void  OnRtmLanAndIsdnSlotInd(CSegment* pMsg);

private: // helpers

	string ConvertIpHostToInterface(APIU32 boardId, DWORD ipaddress);

	void MoveToRunningState();

	void BuildCommandLine(std::string& commandLine, DWORD ipaddress, CTcpDumpEntity* ent, const char* startTimeStr);
	void StartTcpDumpDurationTimer(eMaxCaptureDurationType maxCaptureDutration);

	STATUS TestFilters();

private:

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS

private:

	FileDownloaderTask m_downloaderTask;

	CUtilityProcess* m_proc;
	IP_INTERFACE_SHORT_S   m_interfacesList[MAX_NUM_OF_BOARDS * MAX_NUM_OF_PQS];

	// VNGR-21782
	std::string m_startTimeStr;

	DWORD m_tcpdumpCalls;
	DWORD m_maxCaptureSize; // in units of -C option
	DWORD m_totalTcpdumpUnits;

	bool m_bShouldNoStartTimerStart;
	bool m_bNoStartTimerStarted;

	SLOTS_CONFIGURATION_S m_mfaBoardCnfArray[MAX_NUM_OF_SLOTS];

    CSlideConvertTask m_slideConvertTask;
};

////////////////////////////////////////////////////////////////////////////
#endif  // UTILITY_MANAGER_H__
