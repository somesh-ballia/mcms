// SystemMonitoringManager.h

#ifndef SYSTEM_MONITOR_MANAGER_H_
#define SYSTEM_MONITOR_MANAGER_H_

#include <vector>
#include <list>

#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "TaskApi.h"
#include "DefinesGeneral.h"
#include "ManagerTask.h"
#include "Macros.h"
#include "OsProcessUtils.h"
#include "McuMngrInternalStructs.h"
#include "McmsAuthentication.h"

class CpuTemperatureControl;

struct CoreFileDescriptor
{
  CoreFileDescriptor(std::string& name, time_t modified, off_t size) :
    m_fileName(name), m_modified(modified), m_size(size)
  {}

  std::ostream& operator<<(std::ostream& ostr);
  bool operator<(const CoreFileDescriptor& r);

  std::string m_fileName;
  time_t m_modified;
  off_t m_size;
};

bool operator==(const CoreFileDescriptor& r, const CoreFileDescriptor& l);

bool operator!=(const CoreFileDescriptor& r, const CoreFileDescriptor& l);

struct CoreFileSortByTime
{
	CoreFileSortByTime(std::string& process, time_t modified, unsigned short usCoreCount) :
		m_processName(process), m_modified(modified), m_coreCount(usCoreCount)
	{}

	bool operator<(const CoreFileSortByTime& r);

	std::string m_processName;
	time_t m_modified;
	unsigned short m_coreCount;
};

typedef std::list < CoreFileDescriptor >   CORE_LIST;
typedef std::map< std::string , CORE_LIST >   CORE_MAP;

void SystemMonitoringManagerEntryPoint(void* appParam);

struct CFileSystemMonitoringData
{
    const string m_FileSystemPath;
    const DWORD m_Id;

    DWORD m_TotalSpace;
    DWORD m_FreeSpace;
    DWORD m_MinFreeSpace; // %

public:
    CFileSystemMonitoringData(const string & path, DWORD total, DWORD free, DWORD minFree, DWORD id)
        :m_FileSystemPath(path), m_Id(id), m_TotalSpace(total), m_FreeSpace(free)
        {
            SetMinFreeSpace(minFree);
            SetFreeSpace(free);
        }

    bool IsEnoughFreeSpace()const
        {
            DWORD res = GetFreeSpacePercentage();
            return res >=  m_MinFreeSpace;
        }

    void SetTotalSpace(DWORD total) {m_TotalSpace = total;}
    void SetFreeSpace(DWORD free) {m_FreeSpace = (0 == free ? 1 : free);}
    void SetMinFreeSpace(DWORD minFree) {m_MinFreeSpace = (minFree > 100 ? 100 : minFree);}

    DWORD GetFreeSpacePercentage()const {return (DWORD)((double)(m_FreeSpace * 100.0) / m_TotalSpace);}
    DWORD GetMinFreeSpace()const{return (DWORD)((double)(m_TotalSpace * m_MinFreeSpace) / 100);}
};

typedef vector<CFileSystemMonitoringData*> CFileSystemMonitorVector;

class SProcessStat: public CPObject
{
	CLASS_TYPE_1(SProcessStat,CPObject )

	public:

	SProcessStat();
	SProcessStat(const PS_PROC& Stat);
	virtual ~SProcessStat();
	virtual const char*  NameOf() const;

	std::ostream &  operator << (std::ostream  & ostr);
	bool operator < ( const SProcessStat  & r);
	virtual void Dump(std::ostream& oss) const;

	PS_PROC m_stat;
	static ePS_PROC m_sortType;
};

bool operator == ( const SProcessStat  & r,const SProcessStat  & l);

typedef std::list<SProcessStat> PROCESSES_STAT;
typedef std::list<PROCESSES_STAT> PROCESSES_STAT_LIST;

class CSystemMonitoringManager: public CManagerTask
{
CLASS_TYPE_1(CSystemMonitoringManager, CManagerTask)
public:
	CSystemMonitoringManager();
  virtual ~CSystemMonitoringManager();

  void ManagerPostInitActionsPoint();
  void OnTimerSmartMonitor(CSegment* pSeg);
  void OnTimerRunSmartShortSelftest(CSegment* pSeg);
  void OnTimerMonitorCoreDumpDir(CSegment* pSeg);
  void OnExternalNewCoreDumpInd(CSegment* pSeg);
  void OnTimerSystemMemoryUsage(CSegment* pSeg);
  void OnTimerSystemCpuUsage(CSegment* pSeg);
  void OnReqSystemCpuUsage(CSegment *pSeg);
  void OnTimerProcessStat(CSegment *pSeg);
  void OnMcuMngrAuthenticationStruct(CSegment* pSeg);
  void OnTimerTempSensors(CSegment* pSeg);
  void OnTimerEthSettingsMonitoring(CSegment *pSeg);
  void OnEthSettingClearMaxCountersReq(CSegment *pSeg);
  void OnShmCompSlotIdInd(CSegment* pMsg);
  void OnTimerSystemIsStartupFinished(CSegment* msg);
  void OnTimerCheckMediaRecordingFolderSizeTout(CSegment* pSeg);
  void OnTimerCheckCdrPersistenceQueueFolderSizeTout(CSegment* pSeg);
  void OnTimerSystemDiskSpace(CSegment *pSeg);
  void EnableDisableRemoteCdrTimer(CSegment *pSeg);
  void OnTimerSystemSchedLatency(CSegment *pSeg);
  void DisableActiveAlarmRemoteCdrFeature();
  virtual void           SelfKill(void);

  void PrintProcessStat(const PROCESSES_STAT& ProcessStat, std::ostream& oss) const;
  void PrintAllProcessStat(std::ostream& oss) const;
  void CalculateAndFilterProcessesStat(PROCESSES_STAT* pCalculatedProcessesState,
                                       ePS_PROC sortType);
  WORD GetNumberOfPrintStat();
  std::string GetLoadAverageValues() const;
  std::string GetTotalMemSize() const;

  MCMS_AUTHENTICATION_S* GetAuthenticationStruct();
  void HandleEthSettingsMonitoring(const DWORD portId,
                                   const string &theCaller,
                                   const bool isClearMaxCounters = false);

  STATUS SampleProcessStat(PROCESSES_STAT* pProcesses_stat);
  STATUS HandleTerminalMemoryUsage(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleTerminalScanHD(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleTerminalCheckHDAvail(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleCpuUsage(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleTerminalProcessStat(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleTerminalTemperature(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleTerminalLoadLogger(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleTerminalIncreaseTemperature(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleDumpMcuStatus(CTerminalCommand & command,std::ostream& answer);
  BOOL IsPortwell();
  BOOL IsFintec();

protected:
  TaskEntryPoint GetMonitorEntryPoint();
  WORD DetectingNewCoreDumps();
  void DumpOSStartupLogs();
  void CpuComEInfo();

  void ManageNinjaCoreDump();

  CORE_MAP* m_pCurrentCoreDumpsMap;
  CORE_MAP* m_pPreviousCoreDumpsMap;
  PROCESSES_STAT_LIST* m_pProcessStatMap;
  float m_fCoreDumpSizeLimitation;

private:
  void ReadLoadAverage(float& min1, float& min5, float& min15) const;
  float GetLoadAverage() const;
  void OnScanMemoryTimer(CSegment* seg);
  void InitFileSystemMonitor();
  void FreeFileSystemMonitor();
  bool ScanFileSystem(string &);
  void CheckForOverflowVector();
  void CheckForOverflowSingle(const CFileSystemMonitoringData & monitorData);
  void PrintFileUsage(const string thePath);
  void PrintOutFileSystemMonitoringVector(std::ostream &ostr);
  void CheckHardDisk();
  void CheckSystemCpuUsage();
  void TraceCurrentProcessStat(ePS_PROC type);
  void CheckBiosVersion();
  STATUS GetTemperature(int & cpu_sensor, int & hd_sensor);
  void OnLoggerLoadTout(CSegment* pMsg);
  void GetCpuType();
  void CheckSystemScheLatency();
  bool IsCDRUp();
  void IsRemoteCdrEnable();


  bool m_IsSystemPhysicalMemoryExhausted;
  bool m_IsSystemDiskSpaceExhausted;
  BOOL m_IsSystemCpuUsageExhausted;
  BOOL m_IsPrintCPUStat;
  BOOL m_IsPrintRSSStat;
  BOOL m_isHDDSupportsTemperature;
  MCMS_AUTHENTICATION_S* m_pAuthenticationStruct;

  CFileSystemMonitorVector m_FileSystemMonitorVector;
  eCpuManufacturerType GetCpuManufacturerType();
  eProductType GetProductTypeAccordingCpuType();

  eSystemRamSize m_ramSize;

  BYTE m_cpuBoardId;
  BYTE m_cpuSubBoardId;
  BOOL m_disable_IPMC_usage;
  WORD m_tracesPerSec;
  WORD m_tracesPerBatch;
  eCpuManufacturerType m_cpuManufacturerType;
  CCpuTemperatureControl*  m_cpuTemperatureControl;
  std::string m_cpuType;
  eIpType m_curIpType;
  bool m_isPersistenceQueueEnabled;
  bool m_isRemoteCdrEnabled;




  CTaskApi  m_PipeToSystemMonitoringTaskApi;

  PDECLAR_MESSAGE_MAP;
  PDECLAR_TRANSACTION_FACTORY;
  PDECLAR_TERMINAL_COMMANDS;
};

#endif  // SYSTEM_MONITOR_MANAGER_H_
