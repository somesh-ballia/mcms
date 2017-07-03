// McmsDaemonProcess.h: interface for the CMcmsDaemonProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_McmsDaemonPROCESS_H__)
#define _McmsDaemonPROCESS_H__

#include "ProcessBase.h"
#include "ProcessPolicy.h"
#include <errno.h>
#include "StatusesGeneral.h"
#include <bitset>

class CMcmsDaemonProcess : public CProcessBase
{
	friend class CTestMcmsDaemonProcess;

	CLASS_TYPE_1(CMcmsDaemonProcess, CProcessBase)

public:

	CMcmsDaemonProcess();

	virtual const char* NameOf() const { return "CMcmsDaemonProcess"; }

	virtual ~CMcmsDaemonProcess();
	virtual eProcessType GetProcessType() { return eProcessMcmsDaemon; }
	virtual BOOL UsingSockets() { return true; }
	virtual TaskEntryPoint GetManagerEntryPoint();

	virtual BOOL HasWatchDogTask()       { return false; }
	virtual BOOL HasDispatcherTask()     { return false; }
	virtual BOOL HasSyncDispathcerTask() { return false; }
	virtual BOOL HasMonitorTask()        { return false; }
	virtual BOOL GivesAwayRootUser()     { return false; }

	virtual int SetUp();
	virtual int GetProcessAddressSpace() { return 0; }

	virtual void DumpProcessStatistics(std::ostream& answer, CTerminalCommand & command) const;

	void SetAllProcessesStatus(eMonitorProcessStatus from, eMonitorProcessStatus to);
	void ResetProcessMonitoringInfo();

	CProcessPolicy* GetProcessMonitoringArray() { return m_ProcessMonitoringInfo; }

	void SetIsSafeMode(bool isSafe) { m_isSafeMode = isSafe; }
	bool GetIsSafeMode() const      { return m_isSafeMode; }

	bool GetSystemCfgActiveAlarm() const { return m_systemCfgActiveAlarm; }

	void printMap(std::ostream& answer);

	eProcessType ConvertStrToProcessType(const std::string& processName);
	bool IsFlexeraLicenseInSysFlag();

public:

	static bool IsLaunched(eProcessType process);
	static bool IsWatched(eProcessType process);
	static bool IsProcessExists(eProcessType process);
	static bool IsCruicial(eProcessType process);

	static void UpdateProcessWatchState(eProcessType processType, bool state);

private:

	STATUS VerifyXmlFile(const char* fileName);

private:

	CProcessPolicy m_ProcessMonitoringInfo[NUM_OF_PROCESS_TYPES];

	bool m_isSafeMode;
	bool m_systemCfgActiveAlarm;

	typedef std::bitset<NUM_OF_PROCESS_TYPES> ProcessBitMap;

	static ProcessBitMap m_processWatchMap;

	static void CreateProcessMap();
};


#endif // !defined(_McmsDaemonPROCESS_H__)

