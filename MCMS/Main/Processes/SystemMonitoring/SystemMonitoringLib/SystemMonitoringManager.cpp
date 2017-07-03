// SystemMonitoringManager.cpp

#include "SystemMonitoringManager.h"

#include <sstream>
#include <stdlib.h>
#include <sys/vfs.h>
#include "time.h"
#include "iomanip"
#include "sstream"
#include <set>

#include "Trace.h"
#include "HlogApi.h"
#include "OsFileIF.h"
#include "AlarmStrTable.h"
#include "OpcodesMcmsCommon.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsInternal.h"
#include "ConfigManagerApi.h"
#include "ApiStatuses.h"
#include "FaultsDefines.h"
#include "ObjString.h"
#include "FaultsDefines.h"
#include "TraceStream.h"
#include "TerminalCommand.h"
#include "ProcessSettings.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "IPMCInterfaceApi.h"
#include "SystemMonitoringProcess.h"
#include "EthernetSettingsMonitoring.h"
#include "CpuTemperatureControl.h"


#define MAX_NUM_OF_PRINTED_PROCESSES_STAT 20
#define MAX_NUM_OF_CORE_DUMPS_PER_PROCESS 3
#define MAX_NUM_OF_PROCESSES_STAT_RECORDS 4
#define DEFAULT_TEMPRETURE                30
#define LOGGER_LOAD_TOUT                  25

const char* GetSystemRamSizeStr(eSystemRamSize theSize);
extern char* ProductTypeToString(APIU32 productType);
extern const char* ShelfMngrComponentTypeToString(APIU32 compType);
extern char* IpTypeToString(APIU32 ipType, bool caps = false);

extern void        pipeToSystemMonitoringEntryPoint(void* appParam);

static DWORD logger_load_counter = 0;

#define ERROR -1

const DWORD SCAN_HD_FLUSH_INTERVAL = SECOND * 60;
const DWORD PROCESS_STAT_INTERVAL =  SECOND * 60 * 5;
const DWORD MEMORY_USAGE_INTERVAL = SECOND*60;
const DWORD SMART_MONITOR_INTERVAL = SECOND * 60 * 30;
const DWORD SMART_SELFTEST_INTERVAL = SECOND * 60 * 60 * 24;
const DWORD CORE_DUMP_MANAGER_INTERVAL = SECOND * 60;
const DWORD TEMPERATURE_SENSORS_INTERVAL = SECOND * 60;
const DWORD ETHERNET_SETTINGS_MONITORING_INTERVAL = SECOND * 8;
const DWORD CHECK_MEDIA_RECORDING_FOLDER_SIZE_INTERVAL = 120 * SECOND;
const DWORD CHECK_PERSISTENCE_QUEUE_FOLDER_SIZE_INTERVAL = /*60*/5 * SECOND;
const DWORD DISK_SPACE_INTERVAL = SECOND*120; // 2 minutes
const string cpuTypePentium = "Pentium";
const string cpuTypeCeleron = "Celeron";
const string cpuTypeCore2Duo = "Core";
const string cpuTypeAtom = "Atom";

static const int sched_check_interval = SECOND * 60;

ePS_PROC SProcessStat::m_sortType = eCPUTime;

PBEGIN_MESSAGE_MAP(CSystemMonitoringManager)
  ONEVENT(XML_REQUEST, IDLE, CSystemMonitoringManager::HandlePostRequest )
  ONEVENT(SMART_MONITOR_TIMER, ANYCASE, CSystemMonitoringManager::OnTimerSmartMonitor )
  ONEVENT(SMART_SELFTEST_TIMER, ANYCASE, CSystemMonitoringManager::OnTimerRunSmartShortSelftest )
  ONEVENT(CORE_DUMP_MANAGER_TIMER, ANYCASE, CSystemMonitoringManager::OnTimerMonitorCoreDumpDir )
  ONEVENT(SYSTEM_MEMORY_USAGE_TIMER, ANYCASE, CSystemMonitoringManager::OnTimerSystemMemoryUsage )
  ONEVENT(NEW_CORE_DUMP_IND, ANYCASE, CSystemMonitoringManager::OnExternalNewCoreDumpInd )
  ONEVENT(SYSTEM_MEMORY_SCAN_TIMER, ANYCASE, CSystemMonitoringManager::OnScanMemoryTimer )
  ONEVENT(SYSTEM_SYSTEM_CPU_USAGE_TIMER, ANYCASE, CSystemMonitoringManager::OnTimerSystemCpuUsage )
  ONEVENT(SYSTEM_CHECK_SYSTEM_CPU_USAGE_REQ, ANYCASE, CSystemMonitoringManager::OnReqSystemCpuUsage )
  ONEVENT(SYSTEM_PROCESS_STAT_TIMER, ANYCASE, CSystemMonitoringManager::OnTimerProcessStat )
  ONEVENT(MCUMNGR_AUTHENTICATION_STRUCT_REQ, ANYCASE, CSystemMonitoringManager::OnMcuMngrAuthenticationStruct )
  ONEVENT(SYSTEM_PROCESS_TEMP_SENSORS_TIMER, ANYCASE, CSystemMonitoringManager::OnTimerTempSensors )
  ONEVENT(ETHERNET_SETTINGS_MONITORING_TIMER, ANYCASE, CSystemMonitoringManager::OnTimerEthSettingsMonitoring )
  ONEVENT(ETHERNET_SETTINGS_CLEAR_MAX_COUNTERS_REQ, ANYCASE, CSystemMonitoringManager::OnEthSettingClearMaxCountersReq )
  ONEVENT(ETHERNET_SETTINGS_SYSTEM_MONITORING_REQ, ANYCASE, CSystemMonitoringManager::OnTimerEthSettingsMonitoring )
  ONEVENT(SM_COMP_SLOT_ID_IND, ANYCASE, CSystemMonitoringManager::OnShmCompSlotIdInd )
  ONEVENT(SYSTEM_IS_STARTUP_FINISHED_TIMER, ANYCASE, CSystemMonitoringManager::OnTimerSystemIsStartupFinished)
  ONEVENT(LOGGER_LOAD_TOUT, ANYCASE, CSystemMonitoringManager::OnLoggerLoadTout )
  ONEVENT(SYSTEM_CHECK_MEDIA_RECORDING_FOLDER_SIZE_TIMER, ANYCASE, CSystemMonitoringManager::OnTimerCheckMediaRecordingFolderSizeTout )
  ONEVENT(CDR_PERSISTENCE_QUEUE_FOLDER_SIZE_TIMER, ANYCASE, CSystemMonitoringManager::OnTimerCheckCdrPersistenceQueueFolderSizeTout )
  ONEVENT(CDR_ENABLE_DISABLE_REMOTE_CDR_TIMER, ANYCASE, CSystemMonitoringManager::EnableDisableRemoteCdrTimer )
  ONEVENT(SYSTEM_CHECK_FREE_DISK_SPACE_TIMER , ANYCASE,   CSystemMonitoringManager::OnTimerSystemDiskSpace )
    //ONEVENT(SYSTEM_CHECK_FREE_DISK_SPACE_TIMER , ANYCASE,   CSystemMonitoringManager::OnTimerSystemDiskSpace )
  ONEVENT(SYSTEM_SCHED_LATENCY_TIMER, ANYCASE, CSystemMonitoringManager::OnTimerSystemSchedLatency)
PEND_MESSAGE_MAP(CSystemMonitoringManager,CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY(CSystemMonitoringManager)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CSystemMonitoringManager)
  ONCOMMAND("Memory_Usage", CSystemMonitoringManager::HandleTerminalMemoryUsage, "Get Physical Memory Usage")
  ONCOMMAND("scan_memory", CSystemMonitoringManager::HandleTerminalScanHD, "Get Physical Memory Usage")
  ONCOMMAND("check_hd_avail", CSystemMonitoringManager::HandleTerminalCheckHDAvail, "Check if the HD is available")
  ONCOMMAND("process_stat", CSystemMonitoringManager::HandleTerminalProcessStat, "Display process stat: command's format [all,cpu[all[cal]],rss[all[cal]]]")
  ONCOMMAND("temperature", CSystemMonitoringManager::HandleTerminalTemperature, "Display hardware sensors temperature")
  ONCOMMAND("load_logger", CSystemMonitoringManager::HandleTerminalLoadLogger, "Simulate logger load")
  ONCOMMAND("increase_temperature", CSystemMonitoringManager::HandleTerminalIncreaseTemperature, "increase cpu's temperature for testing")
  ONCOMMAND("dump_mcu_status", CSystemMonitoringManager::HandleDumpMcuStatus, "Dump SoftMcuStatus output to log")
END_TERMINAL_COMMANDS

extern void SystemMonitoringMonitorEntryPoint(void* appParam);

void SystemMonitoringManagerEntryPoint(void* appParam)
{
  CSystemMonitoringManager* pSystemMonitoringManager = new CSystemMonitoringManager;
  pSystemMonitoringManager->Create(*(CSegment*)appParam);
}

TaskEntryPoint CSystemMonitoringManager::GetMonitorEntryPoint()
{
  return SystemMonitoringMonitorEntryPoint;
}

CSystemMonitoringManager::CSystemMonitoringManager() :
  m_ramSize(eSystemRamSize_illegal)
{
  m_pCurrentCoreDumpsMap = NULL;
  m_pProcessStatMap = new PROCESSES_STAT_LIST;
  SProcessStat::m_sortType = eCPUTime;

	m_pAuthenticationStruct = new MCMS_AUTHENTICATION_S;
	m_pAuthenticationStruct->productType		= CProcessBase::GetProcess()->GetProductType();
	m_pAuthenticationStruct->switchBoardId		= 0;
	m_pAuthenticationStruct->switchSubBoardId	= 0;

	m_cpuBoardId	= 0;
	m_cpuSubBoardId	= 0;
	m_disable_IPMC_usage = FALSE;

	m_tracesPerSec = 0;
	m_tracesPerBatch = 0;

	m_curIpType = (eIpType)0;
	m_isPersistenceQueueEnabled = true;
	m_isRemoteCdrEnabled = false;

	m_fCoreDumpSizeLimitation = 0;
	m_IsSystemPhysicalMemoryExhausted = false;
	m_IsSystemDiskSpaceExhausted = false;
	m_isHDDSupportsTemperature = FALSE;
	m_cpuTemperatureControl = NULL;
	m_IsPrintCPUStat = FALSE;
	m_IsPrintRSSStat = FALSE;
	m_pPreviousCoreDumpsMap = NULL;
	m_cpuManufacturerType = eCpuManufacturerTypeUnknown;
	m_IsSystemCpuUsageExhausted = FALSE;

}

CSystemMonitoringManager::~CSystemMonitoringManager()
{
	delete m_pCurrentCoreDumpsMap;
	delete m_pProcessStatMap;
	delete m_pAuthenticationStruct;

	FreeFileSystemMonitor();

}

////////////////////////////////////////////////////////////////////////////////
void CSystemMonitoringManager::ManagerPostInitActionsPoint()
{
  m_IsSystemPhysicalMemoryExhausted = FALSE;
  m_IsSystemCpuUsageExhausted = FALSE;
  m_IsPrintCPUStat = FALSE;
  m_IsPrintRSSStat = FALSE;
  m_isHDDSupportsTemperature = TRUE;



  CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
  sysConfig->GetBOOLDataByKey("DISABLE_IPMC_USAGE", m_disable_IPMC_usage);

  StartTimer(SYSTEM_MEMORY_USAGE_TIMER, MEMORY_USAGE_INTERVAL);

  // Activates first "core dumps" check - to remove the extra  ones.
  DispatchEvent(CORE_DUMP_MANAGER_TIMER, NULL);

  InitFileSystemMonitor();

  //Rachel Cohen 24.1.13 create a task handeling a pipe for running wpa_action.sh ( 802.1x feature)
  eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
  if(eProductTypeRMX1500 == curProductType || eProductTypeRMX2000 == curProductType || eProductTypeRMX4000 == curProductType)
     CreateTask(&m_PipeToSystemMonitoringTaskApi, pipeToSystemMonitoringEntryPoint, m_pRcvMbx);

  // Starts System CPU usage checks after startup
  StartTimer(SYSTEM_IS_STARTUP_FINISHED_TIMER, 30 * SECOND);
  StartTimer(SYSTEM_MEMORY_SCAN_TIMER, SCAN_HD_FLUSH_INTERVAL);

  // Checks MediaRecording folder Size < 1.5Giga each 2 Seconds
  StartTimer(SYSTEM_CHECK_MEDIA_RECORDING_FOLDER_SIZE_TIMER, CHECK_MEDIA_RECORDING_FOLDER_SIZE_INTERVAL);


  // Checks Persistenece queue folder Size
  IsRemoteCdrEnable();

  StartTimer(SYSTEM_PROCESS_STAT_TIMER, PROCESS_STAT_INTERVAL);
  m_cpuManufacturerType = GetCpuManufacturerType();


  if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
  {
    StartTimer(SYSTEM_PROCESS_TEMP_SENSORS_TIMER, TEMPERATURE_SENSORS_INTERVAL);
    StartTimer(SMART_MONITOR_TIMER, SECOND * 60 * 4);
    StartTimer(SMART_SELFTEST_TIMER, SECOND * 60 * 2);
  }

  //eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
  eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();

  if ( eProductFamilySoftMcu == curProductFamily )
  {
	  StartTimer(SYSTEM_CHECK_FREE_DISK_SPACE_TIMER, DISK_SPACE_INTERVAL);
	  
  }

 /* if ((curProductFamily != eProductFamilyCallGenerator)
      && (curProductFamily != eProductFamilySoftMcu) )
    StartTimer(ETHERNET_SETTINGS_MONITORING_TIMER, ETHERNET_SETTINGS_MONITORING_INTERVAL);*/


  GetCpuType();
  m_cpuTemperatureControl = InitializeCpuTemperatureControl(curProductType,m_cpuManufacturerType, m_cpuType);
  TRACESTR(eLevelInfoNormal) << "m_cpuTemperatureControl: " << m_cpuTemperatureControl->GetName();
  
  CheckHardDisk();
  // don't check BIOS in SoftwareMcu
  if( curProductFamily != eProductFamilySoftMcu )
     CheckBiosVersion();
  m_ramSize = GetRamSizeAccordingToTotalMemory("CSystemMonitoringManager::InitTask");

  DumpOSStartupLogs();
  CpuComEInfo();

  BOOL checkSchedLatency = FALSE;

  BOOL res = sysConfig->GetBOOLDataByKey(CFG_KEY_ENBLE_OS_LATENCY_DETECTION, checkSchedLatency);
  PASSERTSTREAM_AND_RETURN(!res, "GetBOOLDataByKey: " << CFG_KEY_ENBLE_OS_LATENCY_DETECTION);

  if( curProductType == eProductTypeEdgeAxis && checkSchedLatency == TRUE )
  {
      StartTimer(SYSTEM_SCHED_LATENCY_TIMER, sched_check_interval);
  }

  std::string answer;
  SystemPipedCommand("uname -a", answer, FALSE, FALSE);

  TRACESTR(eLevelInfoNormal) << "Control Unit Kernel Information: " << answer;

  CProcessBase* proc = CProcessBase::GetProcess();
  proc->m_NetSettings.LoadFromFile();
  m_curIpType = proc->m_NetSettings.m_iptype;

}

bool CSystemMonitoringManager::IsCDRUp()
{
	bool   is_alive = FALSE;
	STATUS status = IsProcAlive("CDR", is_alive);
	FTRACECOND_AND_RETURN_VALUE(STATUS_OK != status,
			"Assumed that SystemMonitoring is down",
			TRUE);
	return is_alive;
}

void CSystemMonitoringManager::IsRemoteCdrEnable()
{
	if( IsCDRUp() == FALSE )
	{
		PTRACE(eLevelInfoNormal,"CCDRManager::IsRemoteCdrEnable - CDR is not alive yet");
		return;
	}

	CSegment* seg = new CSegment;
    OPCODE opcode;
    CSegment ret_seg;
    CManagerApi api(eProcessCDR);
    STATUS stat = api.SendMessageSync(seg,
    		CDR_IS_REMOTE_CDR_ENABLE,
                                      5 * SECOND,
                                      opcode,
                                      ret_seg);
    BOOL isRemote = 0;
    if(STATUS_OK == stat)
    {
    	// verify response message
    	if(0 < ret_seg.GetLen())
    	{

    		ret_seg >> isRemote;
    	}
    }
    if(isRemote == TRUE)
    {
    	if(m_isRemoteCdrEnabled == false)
    	{
    		TRACESTR(eLevelInfoNormal) << "\nCSystemMonitoringManager::EnableDisableRemoteCdrTimer - m_isRemoteCdrEnabled == false";
    		m_isRemoteCdrEnabled = true;
    		StartTimer(CDR_PERSISTENCE_QUEUE_FOLDER_SIZE_TIMER, CHECK_PERSISTENCE_QUEUE_FOLDER_SIZE_INTERVAL);
    	}
    }

}

void CSystemMonitoringManager::GetCpuType()
{

	CConfigManagerApi api;
	STATUS stat = api.GetCPUType(m_cpuType);
}

void CSystemMonitoringManager::InitFileSystemMonitor()
{
    if(FALSE == IsTarget())
    {
        CFileSystemMonitoringData *pSimData = new CFileSystemMonitoringData(".", 1, 1, 50, 0);
        m_FileSystemMonitorVector.push_back(pSimData);
        return;
    }

    DWORD minFreeSpacePercentage = 0;
    int id = 0;
    CFileSystemMonitoringData *pData = NULL;

    minFreeSpacePercentage = 5;
    id = m_FileSystemMonitorVector.size();
    pData = new CFileSystemMonitoringData(MCU_DATA_DIR, 1, 1, minFreeSpacePercentage, id);
    m_FileSystemMonitorVector.push_back(pData);

    minFreeSpacePercentage = 20;
    id = m_FileSystemMonitorVector.size();
    pData = new CFileSystemMonitoringData(MCU_CONFIG_DIR, 1, 1, minFreeSpacePercentage, id);
    m_FileSystemMonitorVector.push_back(pData);

    minFreeSpacePercentage = 20;
    id = m_FileSystemMonitorVector.size();
    pData = new CFileSystemMonitoringData(MCU_OUTPUT_DIR, 1, 1, minFreeSpacePercentage, id);
    m_FileSystemMonitorVector.push_back(pData);

    switch (CProcessBase::GetProcess()->GetProductFamily())
    {
        case eProductFamilyRMX:
        case eProductFamilySoftMcu:
        {
		    minFreeSpacePercentage = 10;
		    id = m_FileSystemMonitorVector.size();
		    std::string fname = MCU_MCMS_DIR+"/IVRX/RollCall";
		    pData = new CFileSystemMonitoringData(fname.c_str(), 1, 1, minFreeSpacePercentage, id);
		    m_FileSystemMonitorVector.push_back(pData);
            break;
        }
        case eProductFamilyCallGenerator:
        {
            // do nothing
            break;
        }
        default:
        {
            PASSERTMSG(1, "bad product type");
        }
    }
}

void CSystemMonitoringManager::FreeFileSystemMonitor()
{
    CFileSystemMonitorVector::iterator iTer = m_FileSystemMonitorVector.begin();
    CFileSystemMonitorVector::iterator iEnd = m_FileSystemMonitorVector.end();
    for( ; iTer < iEnd ; iTer++)
    {
        CFileSystemMonitoringData *pData = *iTer;
        POBJDELETE(pData);
    }
    m_FileSystemMonitorVector.clear();
}

void CSystemMonitoringManager::OnTimerSmartMonitor(CSegment* pSeg)
{
  std::string msg;
  STATUS status = CConfigManagerApi().GetSMARTErrors(msg);

  PASSERTSTREAM(STATUS_SMART_REPORT_ERRORS == status,
      "SMART errors: " << msg.c_str());
  TRACECOND(status != STATUS_OK && STATUS_SMART_REPORT_ERRORS != status,
      "SMART check failed: " << msg);

  StartTimer(SMART_MONITOR_TIMER, SMART_MONITOR_INTERVAL);
}

void CSystemMonitoringManager::OnTimerRunSmartShortSelftest(CSegment* pSeg)
{
  std::string errors;
  CConfigManagerApi api;
  api.RunSMARTSelftest(TRUE);

  StartTimer(SMART_SELFTEST_TIMER, SMART_SELFTEST_INTERVAL);
}

void CSystemMonitoringManager::OnExternalNewCoreDumpInd(CSegment* pSeg)
{
	//We wait 5 seconds to make sure the new core dump file was completely created.
	StartTimer(CORE_DUMP_MANAGER_TIMER, SECOND * 5);
}

void CSystemMonitoringManager::OnTimerMonitorCoreDumpDir(CSegment* pSeg)
{
	StartTimer(CORE_DUMP_MANAGER_TIMER, CORE_DUMP_MANAGER_INTERVAL);

	const char * path = "Cores/";
	DIR * dirp;
	struct dirent * dp;
	off_t  total_size = 0;
    m_pPreviousCoreDumpsMap = m_pCurrentCoreDumpsMap;
    m_pCurrentCoreDumpsMap = new CORE_MAP;

    dirp = opendir (path);
    if (dirp)
    {
        while ((dp = readdir(dirp)))
        {
            if ( (strcmp(dp->d_name,".")!=0) && (strcmp(dp->d_name , "..")!=0))
            {
	            struct stat temp_struct;
	            std::string name = path;
	            name += dp->d_name;

                if (stat(name.c_str(),& temp_struct) == -1)
                {
            		perror("stat failed");
                }
                else
                {
                	time_t modification = temp_struct.st_mtime;
                	off_t  size = temp_struct.st_size;
                	total_size+= size;
                    if(IsTarget())
                    { // change owner of root core files on target
                        uid_t user_id =  temp_struct.st_uid;
                        gid_t group_id = temp_struct.st_gid;

                        if (user_id != 200 ||
                            group_id != 200)
                        {
                            CConfigManagerApi api;
                            api.TakeCoreFileOwnership(dp->d_name);
                        }
                    }


                	int place = name.find_first_of('.');
                	std::string process_name = name.substr(0,place);
                	(*m_pCurrentCoreDumpsMap)[process_name].push_back(CoreFileDescriptor(name,modification,size));
                }
            }
        }

        closedir(dirp);

        if(DetectingNewCoreDumps())
        {
    		CORE_MAP::iterator CoreMapitr;

    		for (CoreMapitr = m_pCurrentCoreDumpsMap->begin() ; CoreMapitr != m_pCurrentCoreDumpsMap->end() ; CoreMapitr++)
    		{
    			CoreMapitr->second.sort();

    			while (CoreMapitr->second.size() > MAX_NUM_OF_CORE_DUMPS_PER_PROCESS)
    			{
    				int res = unlink(CoreMapitr->second.begin()->m_fileName.c_str());

    	      	 	if(res)
						PASSERT(res);
					else //Remove the deleted file from the list
						CoreMapitr->second.pop_front();
    			}
    		}

    		if ( eProductTypeGesher == CProcessBase::GetProcess()->GetProductType() ||
    				eProductTypeNinja == CProcessBase::GetProcess()->GetProductType() )
    		{
    			ManageNinjaCoreDump();
    		}

   		 }
    }
    else
 	   PTRACE(eLevelInfoNormal,"CSystemMonitoringManager::OnTimerMonitorCoreDumpDir : An error occured while opening the directory");

    delete m_pPreviousCoreDumpsMap;
}
/* 2014-07-30 by Penrod Li BRIDGE-13630
 * Because there are many processes in Ninja, even if every process can remain 3 core dump file,
 * it may occupy more than size of disk.
 * Solution:
 * 1.	Take the less one between below options as defined size limitation of core dump files   (Currently, there are 2 kinds of HD  40G and 120G, so the define size will be 20G and 78G.)
 *  		a.	2/3 * size of output folder
 *  		b.	Size of output folder minus 2* MIN_SYSTEM_DISK_SPACE_TO_ALERT ( 2G)
 * 2.	If the size of core dump files is bigger than define size, then delete the earliest one until the size is smaller than define
 * 3.	Try best to remain one core dump per process. If there some core dump in same time, at first delete most core dumps by process.
 *
 */
bool CoreFileSortByTime::operator<(const CoreFileSortByTime& r)
{
  if (m_modified < r.m_modified || (m_modified == r.m_modified && m_coreCount > r.m_coreCount))
	  return true;
  return false;
}
void CSystemMonitoringManager::ManageNinjaCoreDump()
{

	// When first invoked, calculate the define size limitation
	if ( 0 == m_fCoreDumpSizeLimitation)
	{
		CProcessBase *pProcess = CProcessBase::GetProcess();
		DWORD cfg_minimum_disk_space;
		CSysConfig* sysConfig =	pProcess->GetSysConfig();
		sysConfig->GetDWORDDataByKey(CFG_KEY_MIN_SYSTEM_DISK_SPACE_TO_ALERT, cfg_minimum_disk_space);
		cfg_minimum_disk_space = cfg_minimum_disk_space * 2;

		float disk_space;
		std::string ans;
		std::string cmd;

		cmd = "df -m -l -x tmpfs | grep -E ' "+MCU_OUTPUT_DIR+"$' | tr -s ' ' | cut -d' ' -f4";
		STATUS stat = SystemPipedCommand(cmd.c_str(), ans, TRUE, FALSE);
		float fDiskSize;
		if (STATUS_OK == stat)
		{
			fDiskSize = atoi(ans.c_str());
		}
		else
		{
			TRACEINTOLVLERR << "Can't get the size of output, return is "<< ans;
			return;
		}
		float fLimitation1 = fDiskSize * 2 / 3;
		float fLimitation2 = fDiskSize - cfg_minimum_disk_space;
		m_fCoreDumpSizeLimitation = fLimitation1 < fLimitation2 ? fLimitation1 : fLimitation2;
		TRACEINTOFUNC << "size limitation of all core dump is "<< m_fCoreDumpSizeLimitation << " MB.";
	}

	//calculate the size of all core dump file and get the earliest one
	CORE_MAP::iterator CoreMapitr;
	off_t  total_size = 0;
	std::list<CoreFileSortByTime> SortListByTime;
	std::string strProcess;
	for (CoreMapitr = m_pCurrentCoreDumpsMap->begin() ; CoreMapitr != m_pCurrentCoreDumpsMap->end() ; ++CoreMapitr)
	{
		if(CoreMapitr->second.empty())
		{
			continue;
		}
		CORE_LIST::iterator CoreListitr = CoreMapitr->second.begin();
		unsigned short usCoreCount = CoreMapitr->second.size();
		strProcess = CoreMapitr->first;
		for ( CoreListitr=CoreMapitr->second.begin(); CoreListitr != CoreMapitr->second.end(); ++CoreListitr)
		{
			time_t modification = CoreListitr->m_modified;
			total_size += CoreListitr->m_size;
			SortListByTime.push_back(CoreFileSortByTime(strProcess, modification, usCoreCount));
		}
	}
	off_t limitation = static_cast<off_t>(m_fCoreDumpSizeLimitation * 1024 * 1024);
	if( total_size > limitation )
	{
		SortListByTime.sort();
		std::list<CoreFileSortByTime>::iterator it;
		for(it = SortListByTime.begin(); it != SortListByTime.end(); ++it)
		{
			strProcess = it->m_processName;
			CoreMapitr = m_pCurrentCoreDumpsMap->find(strProcess);
			if( CoreMapitr != m_pCurrentCoreDumpsMap->end() )
			{
				total_size -= CoreMapitr->second.begin()->m_size;
				int res = unlink(CoreMapitr->second.begin()->m_fileName.c_str());

				if(res)
					PASSERT(res);
				else //Remove the deleted file from the list
					CoreMapitr->second.pop_front();
				if( total_size < limitation )
				{
					break;
				}
			}
			//it = SortListByTime.erase(it);
		}
	}
	SortListByTime.clear();

}

WORD CSystemMonitoringManager::DetectingNewCoreDumps()
{
	WORD IsNewCoreDump = NO;

	//Current policy:
	//Core dumps that were created before CSystemMonitoringManager was initiated
	//will not be detected.
	if(m_pPreviousCoreDumpsMap)
	{
		CORE_MAP::iterator CurrentItr;
		CORE_MAP::iterator PreviousItr;

		//We check if m_pPreviousCoreDumpsMap comprises m_pCurrentCoreDumpsMap.
		for (CurrentItr = m_pCurrentCoreDumpsMap->begin() ; CurrentItr != m_pCurrentCoreDumpsMap->end() ; CurrentItr++)
    		{
				BOOL IsNewForProcess = NO;
    			PreviousItr = m_pPreviousCoreDumpsMap->find(CurrentItr->first);

    			if(PreviousItr != m_pPreviousCoreDumpsMap->end())
    			{
    				//We sort both lists - because operator == of "list" compares the lists' respective elements
    				CurrentItr->second.sort();
    				PreviousItr->second.sort();


    				if (CurrentItr->second == PreviousItr->second)
    				{
    					continue;
    				}
    				else
    				{

    					IsNewCoreDump = YES;
    					IsNewForProcess = YES;
    				}
    			}
    			else
    			{
    				IsNewCoreDump = YES;
    				IsNewForProcess = YES;
    			}

    			if (IsNewForProcess)
    			{

    				std::set<std::string> coreList;

    				CORE_LIST::iterator itOld ;

    				bool prevExist =(PreviousItr != m_pPreviousCoreDumpsMap->end());

    				if (prevExist)
					{
    					itOld = PreviousItr->second.begin();
					}

    				CORE_LIST::iterator itNew =  CurrentItr->second.begin();

    				for (; itNew != CurrentItr->second.end(); ++itNew)
    				{

    					if (!prevExist || itOld == PreviousItr->second.end())
    					{
    						coreList.insert(itNew->m_fileName);
    					}
    					else
    					{
    						if (*itNew != *itOld)
    						{
    							coreList.insert(itNew->m_fileName);
    						}
    						++itOld;
    					}

    				}
    				std::string coresStr = "";
    				std::string prefix = "Cores/";
    				std::string coreProcess;

    				std::set<std::string>::iterator itCore = coreList.begin();
    				for(; itCore != coreList.end(); ++itCore)
    				{
    					if(itCore->substr(0, prefix.size()) == prefix)
    					{
    						coreProcess = itCore->substr(prefix.size());
    					}
    					else
    					{
    						coreProcess =  *itCore;

    					}

    					if (coresStr != "")
    					{
    						coresStr += ", ";
    					}

    					coresStr += coreProcess;
    				}
    				std::string msg;
    				if (coreList.size() > 1)
    				{
    					msg = "New core files were detected: " + coresStr;
    				}
    				else if (coreList.size() == 1)
    				{
    					msg = "New core file was detected:" + coresStr;
    				}
    				else
    				{
    					TRACEINTOLVLERR << "Error: Core file list is empty. \n";
    				}


					CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
							AA_CORE_FILE_DETECTED, SYSTEM_MESSAGE,
							msg.c_str(), TRUE);

					TRACEINTOFUNC << msg.c_str();

    			}
    		}

    	if (YES == IsNewCoreDump)
    	{
			//Notify McuMngr regarding the new core dump.
			CManagerApi api(eProcessMcuMngr);
			api.SendMsg(NULL,NEW_CORE_DUMP_IND);
    	}
	}

	else
	{
		//Relate to the first check as YES so we will delete
		//extra files (more then MAX_NUM_OF_CORE_DUMPS_PER_PROCESS).
		IsNewCoreDump = YES;
	}

	return IsNewCoreDump;
}

std::ostream& CoreFileDescriptor::operator<< (std::ostream& ostr)
{
	ostr << m_fileName << " " << m_modified << " " << m_size << std::endl;
	return ostr;
}

void CSystemMonitoringManager::OnTimerSystemCpuUsage(CSegment*)
{
  CheckSystemCpuUsage();

  StartTimer(SYSTEM_SYSTEM_CPU_USAGE_TIMER, kMaxTraceLevelTimeout);

  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN(NULL == proc);

  TRACEINTOFUNC << "Timer "
                << proc->GetOpcodeAsString(SYSTEM_SYSTEM_CPU_USAGE_TIMER)
                << " will fire in "
                << kMaxTraceLevelTimeout / SECOND << " seconds";
}

void CSystemMonitoringManager::CheckSystemScheLatency()
{
        static const int sleep_interval = 10 * 1000000;
        static const int nintrvls = 100;

	CProcessBase* proc = CProcessBase::GetProcess();
	PASSERT_AND_RETURN(NULL == proc);

	CSysConfig* cfg = proc->GetSysConfig();
	PASSERT_AND_RETURN(NULL == cfg);

        DWORD lat_low_thr;
        DWORD lat_high_thr;
	BOOL res;

	res = cfg->GetDWORDDataByKey(CFG_KEY_OS_LATENCY_LOW_THRESHOLD, lat_low_thr);
	PASSERTSTREAM_AND_RETURN(!res, "GetDWORDDataByKey: " << CFG_KEY_OS_LATENCY_LOW_THRESHOLD);

	res = cfg->GetDWORDDataByKey(CFG_KEY_OS_LATENCY_HIGH_THRESHOLD, lat_high_thr);
	PASSERTSTREAM_AND_RETURN(!res, "GetDWORDDataByKey: " << CFG_KEY_OS_LATENCY_HIGH_THRESHOLD);

        struct timespec t_sleep;
        struct timespec t_rem;
        t_sleep.tv_sec=0;
        t_sleep.tv_nsec = sleep_interval;

        struct timespec t_start;
        struct timespec t_end;

        int loop_count;

        float latency=0.0;
        float awaken_after;
        float sleep_intrvl;
        // make sure that interrupts are correctly handled
        for (loop_count=0; loop_count < nintrvls; ++loop_count)
        {
            clock_gettime(CLOCK_REALTIME, &t_start);
            if ( 0 == nanosleep(&t_sleep, &t_rem) )
            {
               clock_gettime(CLOCK_REALTIME, &t_end);
               awaken_after = ((t_end.tv_sec + 1e-9 * t_end.tv_nsec) - (t_start.tv_sec + 1e-9 * t_start.tv_nsec)) * 1000.0;
               sleep_intrvl = (t_sleep.tv_sec + 1e-9 * t_sleep.tv_nsec) * 1000.0;
               if(latency < (awaken_after-sleep_intrvl))
                     latency=(awaken_after-sleep_intrvl);
            }
        }

        if (latency >= (lat_high_thr/1000.0))
        {
            std::stringstream alarm_text;
            alarm_text << "Operating System Latency Threshold Exceeded [" << latency << " ms]" ;

            AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                                    AA_HIGH_SCHEDULER_LATENCY,
                                    MAJOR_ERROR_LEVEL,
                                    alarm_text.str(),
                                    true, true);
        }
        else
        {
            if ( latency <= (lat_low_thr/1000.0))
                RemoveActiveAlarmByErrorCode(AA_HIGH_SCHEDULER_LATENCY);
        }
    return;
}

void CSystemMonitoringManager::OnTimerSystemSchedLatency(CSegment*)
{
  CheckSystemScheLatency();

  StartTimer(SYSTEM_SCHED_LATENCY_TIMER, sched_check_interval);
}

void CSystemMonitoringManager::OnTimerSystemDiskSpace(CSegment*)
{
	float disk_space;
	std::string ans;
	std::string sub;
	std::string cmd;
	CProcessBase *pProcess = CProcessBase::GetProcess();
	
	eProductType prodType = pProcess->GetProductType();

	if(eProductTypeGesher == prodType || eProductTypeNinja == prodType)
	{
		cmd = "df -m -l -x tmpfs | grep -E ' "+MCU_OUTPUT_DIR+"$' | tr -s ' ' | cut -d' ' -f4";
	}
	else
	{
		cmd = "df -m -l -x tmpfs | grep -E ' /$' | tr -s ' ' | cut -d' ' -f4";
	}
	
	STATUS stat = SystemPipedCommand(cmd.c_str(), ans, TRUE, FALSE);
	DWORD cfg_minimum_disk_space;
	if (STATUS_OK == stat)
	{
		disk_space = atoi(ans.c_str());
		CSysConfig* sysConfig =	pProcess->GetSysConfig();
		sysConfig->GetDWORDDataByKey(CFG_KEY_MIN_SYSTEM_DISK_SPACE_TO_ALERT, cfg_minimum_disk_space);
		if (disk_space < cfg_minimum_disk_space)
		{
			if (m_IsSystemDiskSpaceExhausted == FALSE)
			{
				disk_space /= 1000;
				std::stringstream out;
				out << disk_space;
				string sDesc = "The System has only " + out.str() + "GB free disk space";

				m_IsSystemDiskSpaceExhausted = TRUE;
				pProcess->AddActiveAlarmFromProcess( FAULT_GENERAL_SUBJECT,
													 LOW_SYSTEM_DISK_SPACE,
													 MAJOR_ERROR_LEVEL,
													 sDesc.c_str(),
													 true,		//isForEma
													 true);		//inForFaults

			}
		}
		else
		{
			m_IsSystemDiskSpaceExhausted = FALSE;
			pProcess->RemoveActiveAlarmFromProcess(LOW_SYSTEM_DISK_SPACE);

		}

	}
	else
	{
		// Assumes high CPU on failure
		TRACEWARN << "disk check failed";

	}

  StartTimer(SYSTEM_CHECK_FREE_DISK_SPACE_TIMER, DISK_SPACE_INTERVAL);

}

void CSystemMonitoringManager::OnTimerSystemIsStartupFinished(CSegment*)
{
  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN(NULL == proc);

  OPCODE opcode;
  unsigned int timeout;

  // Runs System Load Average verification in 2 minutes after startup
  eMcuState state = proc->GetSystemState();
  if (state != eMcuState_Invalid && state != eMcuState_Startup)
  {
    timeout = 1 * 60 * SECOND;
    opcode = SYSTEM_SYSTEM_CPU_USAGE_TIMER;
  }
  else
  {
    timeout = 30 * SECOND;
    opcode = SYSTEM_IS_STARTUP_FINISHED_TIMER;
  }

  StartTimer(opcode, timeout);
  TRACEINTOFUNC << "MCU state " << GetMcuStateName(state)
                << ", timer " << proc->GetOpcodeAsString(opcode)
                << " will fire in " << timeout / SECOND << " seconds";
}

void CSystemMonitoringManager::OnReqSystemCpuUsage(CSegment *pSeg)
{
    CheckSystemCpuUsage();
}

void CSystemMonitoringManager::OnTimerSystemMemoryUsage(CSegment *pSeg)
{
    int FreeMemoryPercentage = GetFreeMemoryPercentage();
    CProcessBase *pProcess = CProcessBase::GetProcess();

    if (FreeMemoryPercentage < 10 )
    {
        if (m_IsSystemPhysicalMemoryExhausted==FALSE)
        {
            string sDesc = "The System surpassed 90% physical memory usage ";
            sDesc += "(system's RAM size: ";
            sDesc += ::GetSystemRamSizeStr(m_ramSize);
            sDesc += ")";


        	pProcess->AddActiveAlarmFaultOnlyToProcess( FAULT_GENERAL_SUBJECT,
                                                 LOW_SYSTEM_MEMORY_ALERT,
                                                 MAJOR_ERROR_LEVEL,
                                                 sDesc.c_str());

        	m_IsSystemPhysicalMemoryExhausted=TRUE;
            m_IsPrintRSSStat = TRUE;

            TraceCurrentProcessStat(eRSS);
        }
    }
    else if (FreeMemoryPercentage >30 && m_IsSystemPhysicalMemoryExhausted==TRUE)
    {
        m_IsSystemPhysicalMemoryExhausted=FALSE;

        pProcess->RemoveActiveAlarmFaultOnlyFromProcess(LOW_SYSTEM_MEMORY_ALERT);
        //Remove Active Alarm
    }

    StartTimer(SYSTEM_MEMORY_USAGE_TIMER, MEMORY_USAGE_INTERVAL);
}

/////////////////////////////////////////////////////////////////////
STATUS CSystemMonitoringManager::HandleTerminalMemoryUsage(CTerminalCommand & command,std::ostream& answer)
{
    int FreeMemoryPercentage = GetFreeMemoryPercentage();
    int OccupiedMemoryPercentage = 100 - FreeMemoryPercentage;
    answer << "The percent of Occupied Memory is " << OccupiedMemoryPercentage;

	return STATUS_OK;
}

STATUS CSystemMonitoringManager::HandleTerminalCheckHDAvail(CTerminalCommand & command,std::ostream& answer)
{
    const char *result = (IsHardDiskOk() == TRUE ? "" : "NOT");
    answer << "Hard Disk is " << result << "\n" ;

    return STATUS_OK;
}

STATUS CSystemMonitoringManager::HandleCpuUsage(CTerminalCommand& command,
                                                std::ostream& answer)
{
  STATUS retStatus = STATUS_FAIL;

  std::string pid;

  pid = command.GetToken(eCmdParam1);

  if (pid == "Invalide Token")
  {
    answer << "\nCSystemMonitoringManager::HandleCpuUsage(): default value";
    pid = "0";
  }
  else
  {
    answer << "\nCSystemMonitoringManager::HandleCpuUsage(): pid = " << pid;
  }

  pid_t i = atoi(pid.c_str());

  char r[20];
  sprintf(r, "%d", i);

  answer << "\nCSystemMonitoringManager::HandleCpuUsage(): i value = " << r;

  clockid_t clock_id;
  int x = clock_getcpuclockid(i, &clock_id);

  if (x)
    answer << "\nCSystemMonitoringManager::HandleCpuUsage(): errno = "
           << strerror(errno);

  char buffer[32];
  sprintf(buffer, "%d", clock_id);
  std::string s_clock_id = buffer;

  answer << "\nCSystemMonitoringManager::HandleCpuUsage(): clock_id = "
         << s_clock_id.c_str();

  struct timespec tp;
  clock_gettime(clock_id, &tp);

  sprintf(buffer, "%ld", tp.tv_sec);
  std::string s_clock_gettime1 = buffer;

  sprintf(buffer, "%ld", tp.tv_nsec);
  std::string s_clock_gettime2 = buffer;

  answer << "\nCSystemMonitoringManager::HandleCpuUsage(): clock_gettime sec = "
         << s_clock_gettime1.c_str()
         << "\nCSystemMonitoringManager::HandleCpuUsage(): clock_gettime nsec = "
         << s_clock_gettime2.c_str();

  return retStatus;
}

void CSystemMonitoringManager::OnScanMemoryTimer(CSegment * seg)
{
  StartTimer(SYSTEM_MEMORY_SCAN_TIMER, SCAN_HD_FLUSH_INTERVAL);

  std::string fileSystem;
  bool res = ScanFileSystem(fileSystem);
  if (false == res)
  {
    string message = "FAILED to scan ";
    message += fileSystem;
    AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                            FILE_SYSTEM_FAILED_TO_SCAN,
                            MAJOR_ERROR_LEVEL,
                            message.c_str(),
                            true, true);
    return;
  }

  bool isExist = IsActiveAlarmExistByErrorCode(FILE_SYSTEM_FAILED_TO_SCAN);
  if (true == isExist)
    RemoveActiveAlarmByErrorCode(FILE_SYSTEM_FAILED_TO_SCAN);

  if (IsTarget())
    CheckForOverflowVector();
}

bool CSystemMonitoringManager::ScanFileSystem(string &outFileSystem)
{
    CFileSystemMonitorVector::iterator iTer = m_FileSystemMonitorVector.begin();
    CFileSystemMonitorVector::iterator iEnd = m_FileSystemMonitorVector.end();
    for( ; iTer < iEnd ; iTer++)
    {
        struct statfs buf;
        CFileSystemMonitoringData *pData = *iTer;
        int res = statfs(pData->m_FileSystemPath.c_str(), &buf);
        if(-1 == res)
        {
            int errCode = errno;
            outFileSystem = pData->m_FileSystemPath;
            CSmallString str = "CSystemMonitoringManager::ScanFileSystem\n";
            str << "FAILED to scan " << outFileSystem.c_str() << "\n"
                << "Errno : " << errCode;
            PASSERTMSG(TRUE, str.GetString());

            return false;
        }

        pData->SetTotalSpace(buf.f_blocks);
        pData->SetFreeSpace(buf.f_bfree);
    }
    return true;
}

/////////////////////////////////////////////////////////////////////
void CSystemMonitoringManager::CheckForOverflowVector()
{
    CFileSystemMonitorVector::iterator iTer = m_FileSystemMonitorVector.begin();
    CFileSystemMonitorVector::iterator iEnd = m_FileSystemMonitorVector.end();
    for( ; iTer < iEnd ; iTer++)
    {
        CFileSystemMonitoringData *pData = *iTer;
        CheckForOverflowSingle(*pData);
    }
}
/////////////////////////////////////////////////////////////////////
void CSystemMonitoringManager::CheckForOverflowSingle(const CFileSystemMonitoringData & monitorData)
{
	bool isAlertExist = IsActiveAlarmFaultOnlyExistByErrorCodeUserId(FILE_SYSTEM_OVERFLOW, monitorData.m_Id);

    if ( true == monitorData.IsEnoughFreeSpace() )
    {
        if (true == isAlertExist)
        {
        	RemoveActiveAlarmFaultOnlyByErrorCode(FILE_SYSTEM_OVERFLOW);
        }
        return;
    }

    if (true == isAlertExist)
    {
        return;
    }

    CLargeString message;
    message << " Overflow in  " << monitorData.m_FileSystemPath.c_str();
    message << " ; ";
    message << "Free space : " << monitorData.GetFreeSpacePercentage() << "% "
            << "(" <<monitorData.m_FreeSpace << " Blocks) - "
            << "Min Free space : " << monitorData.m_MinFreeSpace << "% "
            << "(" << monitorData.GetMinFreeSpace()  << " Blocks)";



    AddActiveAlarmFaultOnlySingleton(FAULT_GENERAL_SUBJECT,
                            FILE_SYSTEM_OVERFLOW,
                            MAJOR_ERROR_LEVEL,
                            message.GetString(),
                            true,
                            true,
                            monitorData.m_Id);

	PrintFileUsage(monitorData.m_FileSystemPath);
}

void CSystemMonitoringManager::ReadLoadAverage(float& min1,
                                               float& min5,
                                               float& min15) const
{
  int res;
  std::string ans;
  std::string sub;
  STATUS stat = SystemPipedCommand("uptime", ans, TRUE, FALSE);
  if (STATUS_OK == stat)
  {
    size_t index = ans.find("load average:", 0);
    if (std::string::npos != index)
    {
      sub = ans.substr(index + 13);
    }
    else
    {
      // Assumes high CPU on failure
      TRACEWARN << "uptime failed, assume load average: 1.56, 1.56, 1.56";
      goto error;    }
  }
  else
  {
    // Assumes high CPU on failure
    TRACEWARN << "uptime failed, assume load average: 1.56, 1.56, 1.56";
    goto error;
  }

  res = sscanf(sub.c_str(), "%f, %f, %f,", &min1, &min5, &min15);
  switch (res)
  {
  case 3:
    // Success
    return;

  case EOF:
    TRACEWARN << "sscanf: " << strerror(errno) << " (" << errno << ")";
    goto error;

  default:
    TRACEWARN << "sscanf read " << res << " elements instead of 3";
    goto error;
  }

error:
  min1 = min5 = min15 = 1.56f;
}

float CSystemMonitoringManager::GetLoadAverage() const
{
  float min1, min5, min15;
  ReadLoadAverage(min1, min5, min15);

  unsigned int minutes;
  const char key[] = CFG_KEY_LOAD_AVERAGE_MINUTES;

  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN_VALUE(NULL == proc, min1);

  CSysConfig* cfg = proc->GetSysConfig();
  PASSERT_AND_RETURN_VALUE(NULL == cfg, min1);

  BOOL res = cfg->GetDWORDDataByKey(key, minutes);
  PASSERTSTREAM_AND_RETURN_VALUE(!res,
      "GetDWORDDataByKey: " << key,
      min1);

  switch (minutes)
  {
  case 1:
    return min1;

  case 5:
    return min5;

  case 15:
    return min15;

  default:
    PASSERTSTREAM(true, "Invalid value of " << key << ": " << minutes);
    return min1;
  }

  return min1;
}

void CSystemMonitoringManager::CheckSystemCpuUsage()
{
	CProcessBase* proc = CProcessBase::GetProcess();
	PASSERT_AND_RETURN(NULL == proc);
	
	static unsigned int upper = 0 ;
	static unsigned int lower = 0;
	
	// Calculates the threshold values only first time
	if (0 == upper)
	{
		unsigned int threshold = 480;
		//VNGSW-644 - increase cpu usage trash hold for soft mcu products
		if (eProductFamilySoftMcu == proc->GetProductFamily())
			threshold = 180;

		CSysConfig* cfg = proc->GetSysConfig();
		PASSERT_AND_RETURN(NULL == cfg);

		BOOL res = cfg->GetDWORDDataByKey(CFG_KEY_LOAD_AVERAGE_ALARM_LEVEL, threshold);
		PASSERTSTREAM_AND_RETURN(!res, "GetDWORDDataByKey: " << CFG_KEY_LOAD_AVERAGE_ALARM_LEVEL);

		STATUS stat = GetCPUUsageThreshold(threshold, upper, lower);
		if (STATUS_OK != stat)
			return;
	}
	
	// Takes load for last 1 minute by default and errors
	unsigned int load =
		static_cast<unsigned int>(GetLoadAverage() * 100.f + .5f);
	
	if (load > upper)
	{
	  if (!m_IsSystemCpuUsageExhausted)
	  {
		TRACEWARN << "load =" << load << ", upper limit=" << upper;
		proc->AddActiveAlarmFaultOnlyToProcess(FAULT_GENERAL_SUBJECT,
											   SYSTEM_CPU_USAGE_ALERT,
											   MAJOR_ERROR_LEVEL,
											   GetAlarmDescription(SYSTEM_CPU_USAGE_ALERT));
	
		m_IsSystemCpuUsageExhausted = TRUE;
		m_IsPrintCPUStat = TRUE;
	
		TraceCurrentProcessStat(eCPUTime);
	  }
	}
	else if (load < lower)
	{
	  if (m_IsSystemCpuUsageExhausted)
	  {
		proc->RemoveActiveAlarmFaultOnlyFromProcess(SYSTEM_CPU_USAGE_ALERT);
		m_IsSystemCpuUsageExhausted = FALSE;
	  }
	}
	
	// Sends asynchronous message to Logger with CPU load
	CSegment* msg = new CSegment;
	*msg << load;
	
	CTaskApi::SendMsgWithTrace(eProcessLogger,
							   eManager,
							   msg,
							   LOGGER_SYSTEM_LOAD_AVERAGE);

}

void CSystemMonitoringManager::PrintFileUsage(const string thePath)
{
  std::string ans;
  std::string cmd = "du -h " + thePath;

  STATUS stat = SystemPipedCommand(cmd.c_str(), ans);
  TRACEINTOFUNC << ans.c_str();

  ans = "";
  cmd = "df" ;

  stat = SystemPipedCommand(cmd.c_str(), ans);
  TRACEINTOFUNC << ans.c_str();

}

STATUS CSystemMonitoringManager::HandleTerminalScanHD(CTerminalCommand& command,
                                                      std::ostream& answer)
{
    string fileSystem;
    bool res = ScanFileSystem(fileSystem);
    if(false == res)
    {
        answer << "FAILED to scan file system : " << fileSystem;
        return STATUS_FAIL;
    }

    PrintOutFileSystemMonitoringVector(answer);

	return STATUS_OK;
}

void CSystemMonitoringManager::PrintOutFileSystemMonitoringVector(std::ostream &ostr)
{
    CFileSystemMonitorVector::iterator iTer = m_FileSystemMonitorVector.begin();
    CFileSystemMonitorVector::iterator iEnd = m_FileSystemMonitorVector.end();
    for( ; iTer < iEnd ; iTer++)
    {
        CFileSystemMonitoringData *pData = *iTer;
        ostr << pData->m_FileSystemPath << " - "
             << "Total : " << pData->m_TotalSpace << " Blocks"
             << " , Free : " << pData->GetFreeSpacePercentage() << "% "
             << "(" << pData->m_FreeSpace << " MB) , "
             << "Min Free : " << pData->m_MinFreeSpace << "% "
             << "(" << pData->GetMinFreeSpace()  << " Blocks)"
             << "\n";
    }
}

void CSystemMonitoringManager::CheckHardDisk()
{
  if (IsHardDiskOk())
    return;

  AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                          BAD_HARD_DISK,
                          MAJOR_ERROR_LEVEL,
                          "Hard disk not responding",
                          true, true);
}

void CSystemMonitoringManager::CheckBiosVersion()
{
  BYTE bJitcMode = FALSE;
  CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
  if (sysConfig)
    sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);

  std::string date;
  GetBiosDate(date);

  TRACECOND(date == "05/29/2008", "A new BIOS version must be installed");

  if (bJitcMode)
  {
    bool valid = false;
    std::string vers;
    GetBiosVersion(vers);

    switch (CProcessBase::GetProcess()->GetProductType())
    {
      case eProductTypeRMX1500:
        if (("12/23/2010" == date && "PA10" == vers) ||
            ("01/21/2010" == date && "PP10" == vers))
         valid = true;
        break;

      case eProductTypeRMX2000:
        if (("12/09/2008" == date) ||
            ("10/01/2008" == date) ||
            ("12/28/2010" == date && "PP02" == vers) ||
            ("06/10/2010" == date && "PA01" == vers) ||
            ("11/07/2012" == date && "PP02" == vers) ||
            ("02/22/2013" == date && "PA02" == vers) ||
            ("12/11/2013" == date && "PP03" == vers) ||
            ("01/15/2014" == date && "PD02" == vers)  )
          valid = true;
        break;

      case eProductTypeRMX4000:
        if (("05/24/2010" == date && "PC02" == vers) ||
            ("06/10/2010" == date && "PA01" == vers) ||
            ("11/07/2012" == date && "PP02" == vers) ||
            ("02/22/2013" == date && "PA02" == vers) ||
            ("12/11/2013" == date && "PP03" == vers) ||
            ("01/15/2014" == date && "PD02" == vers)  )
          valid = true;
        break;

      default:
        PASSERTMSG(1, "Product type is not suitable");
    }

    if (!valid)
    {
      // Checks if BIOS alias starts with PX1 (JITC BIOS version alias)
      if (vers.find("PA1") != string::npos ||
          vers.find("PC1") != string::npos ||
          vers.find("PP1") != string::npos)
        valid = true;
    }

    if (!valid)
    {
      std::ostringstream msg;
      msg << "BIOS version " << vers << " and date " << date
          << " are unsuitable for Ultra Secure Mode";
      AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                              NON_JITC_BIOS_VERSION,
                              MAJOR_ERROR_LEVEL,
                              msg.str(),
                              true,
                              true
                              );
    }
  }
}

bool CoreFileDescriptor::operator<(const CoreFileDescriptor& r)
{
  return m_modified < r.m_modified;
}

bool operator ==(const CoreFileDescriptor & r, const CoreFileDescriptor & l)
{
  return (l.m_fileName == r.m_fileName && l.m_modified == r.m_modified);
}

bool operator!=(const CoreFileDescriptor& r, const CoreFileDescriptor& l)
{
	return !(r==l);
}

void CSystemMonitoringManager::OnTimerTempSensors(CSegment* pSeg)
{
    if (m_disable_IPMC_usage == FALSE)
    {
      StartTimer(SYSTEM_PROCESS_TEMP_SENSORS_TIMER,TEMPERATURE_SENSORS_INTERVAL);
    }



    int cpu_sensor = 0;
	int hd_sensor = 0;

	GetTemperature(cpu_sensor,hd_sensor);

	TRACESTR(eLevelInfoNormal)  << "\nOnTimerTempSensors::cpuTemp - cpu_sensor = "
						<< cpu_sensor;
	CIPMCInterfaceApi api;


	api.SendCpuTemperature(cpu_sensor);

	api.SendHdTemerature(hd_sensor);

  return;
}

STATUS CSystemMonitoringManager::GetTemperature(int& cpu_sensor, int& hd_sensor)
{
	std::string cpuTemp;
	std::string hd;

	if (m_cpuTemperatureControl->GetCpuTemperature(cpuTemp) == STATUS_OK)	
	{		
		// TRACESTR(eLevelInfoNormal)  << "AFTER GetCpuTemperature  " << cpuTemp;
		cpu_sensor = atoi(cpuTemp.c_str());
	}
	else
	{
		TRACESTR(eLevelInfoNormal)  << "\nFailed to get temperature. Use  default " << cpuTemp;		
		// cpuTemp gets default value anyway : 40
		cpu_sensor = atoi(cpuTemp.c_str());
	}

	if(m_isHDDSupportsTemperature)
	{
		CConfigManagerApi api;
		STATUS stat	= api.GetHDTemperature(hd);

		/*If hd is "" It means there is no HD - default value*/
		if(hd != "")
		{
			hd_sensor = atoi(hd.c_str());
		}
		else
		{
			hd_sensor = DEFAULT_TEMPRETURE;
		}
		if(stat != STATUS_OK)
		{
			/*m_isHDDSupportsTemperature = FALSE;*/
			TRACESTR(eLevelInfoNormal)  << "\nCSystemMonitoringManager::GetHDTemperaturet - stat = "
			<< stat;
		}

	}
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
void CSystemMonitoringManager::OnTimerProcessStat(CSegment* pSeg)
{
  StartTimer(SYSTEM_PROCESS_STAT_TIMER,PROCESS_STAT_INTERVAL);

  PROCESSES_STAT* pProcesses_stat = new PROCESSES_STAT; AUTO_DELETE(pProcesses_stat);
  if(SampleProcessStat(pProcesses_stat) == STATUS_OK)
  {
        m_pProcessStatMap->push_back(*pProcesses_stat);
        PDELETE(pProcesses_stat);

       /***********************************************************************/
       /* 28.6.10 VNGR-15657 added by Rachel Cohen                            */
       /* to keep the 4 latest processes I replaced the "if" with "while"     */
        /**********************************************************************/

    //We keep the 4 latest processes stat records.
   	int loops = 0;
           while (m_pProcessStatMap->size() > MAX_NUM_OF_PROCESSES_STAT_RECORDS)
   	  {
          		m_pProcessStatMap->pop_front();
   		loops++;
   		if (loops > 1000)
   		  {
   		    PASSERT(1);
   		    break;
   		  }
   	  }
       }
       else
    	   PTRACE(eLevelInfoNormal,"CSystemMonitoringManager::OnTimerProcessStat : An error occured while Sampling Process Stat");

    WORD NumOfPrintStat = GetNumberOfPrintStat();

    for(int i = 0 ; i < NumOfPrintStat ; i++)
    {
      PROCESSES_STAT* pCalculatedProcessStat = new PROCESSES_STAT;

      ostringstream* oss = new ostringstream;

      //We print the CalculateAndFilterProcessesStat
      //according to the following flags.
      if(m_IsPrintCPUStat)
      {
        *oss << "\n" << GetLoadAverageValues() << "\n";
        CalculateAndFilterProcessesStat(pCalculatedProcessStat,eCPUTime);
        m_IsPrintCPUStat = FALSE;
      }
      else if(m_IsPrintRSSStat)
      {
        *oss << "\n" << GetTotalMemSize() << "\n";
        CalculateAndFilterProcessesStat(pCalculatedProcessStat,eRSS);
        m_IsPrintRSSStat = FALSE;
      }

      PrintProcessStat(*pCalculatedProcessStat,*oss);
      TRACESTR(eLevelInfoHigh) << oss->str();

      delete oss;
      delete pCalculatedProcessStat;
    }
}

void CSystemMonitoringManager::OnMcuMngrAuthenticationStruct(CSegment* pSeg)
{
	// ===== 1. get the parameters from the structure received into process's attribute
	pSeg->Get( (BYTE*)m_pAuthenticationStruct, sizeof(MCMS_AUTHENTICATION_S) );

	// ===== 2. print the data received
    TRACESTR(eLevelInfoNormal)  << "\nCSystemMonitoringManager::OnMcuMngrAuthenticationStruct -"
                            << *m_pAuthenticationStruct;
}

/////////////////////////////////////////////////////////////////////
WORD CSystemMonitoringManager::GetNumberOfPrintStat()
{
	WORD number = 0;

	if(m_IsPrintCPUStat)
		number++;

	if(m_IsPrintRSSStat)
		number++;

	return number;
}
/////////////////////////////////////////////////////////////////////
STATUS CSystemMonitoringManager::HandleTerminalProcessStat(CTerminalCommand & command,std::ostream& answer)
{
	std::string filter,filter2,filter3;

	filter = command.GetToken(eCmdParam1);
	filter2 = command.GetToken(eCmdParam2);
	filter3 = command.GetToken(eCmdParam3);

	PROCESSES_STAT* pProcesses_stat = NULL;

	if(filter == "all") //Prints all non sorted history table for all filters
	{
		//CPU filterd
		SProcessStat::m_sortType = eCPUTime;
		PrintAllProcessStat(answer);

		SProcessStat::m_sortType = eRSS;
		PrintAllProcessStat(answer);

	}
	else if(filter == "cpu")
	{
		SProcessStat::m_sortType = eCPUTime;

		if(filter2 == "all") //Prints all history table of cpu
		{
			if(filter3 == "cal")
			{
				pProcesses_stat = new PROCESSES_STAT;
    			CalculateAndFilterProcessesStat(pProcesses_stat,eCPUTime);
    			answer << "\n" << GetLoadAverageValues() << "\n";
    			PrintProcessStat(*pProcesses_stat,answer);
    			delete pProcesses_stat;
			}
			else
			{
    			PrintAllProcessStat(answer);
			}
		}
		else //Prints sorted current stat of cpu
		{
			pProcesses_stat = new PROCESSES_STAT;

			if(SampleProcessStat(pProcesses_stat) == STATUS_OK)
			{
				pProcesses_stat->sort();
				PrintProcessStat(*pProcesses_stat,answer);
			}

			delete pProcesses_stat;
		}
	}
	else if(filter == "rss")
	{
		SProcessStat::m_sortType = eRSS;

		if(filter2 == "all") //Prints all history table of rss
		{
			if(filter3 == "cal")
			{
				pProcesses_stat = new PROCESSES_STAT;
    			CalculateAndFilterProcessesStat(pProcesses_stat,eRSS);
    			answer << "\n" << GetTotalMemSize() << "\n";
    			PrintProcessStat(*pProcesses_stat,answer);
    			delete pProcesses_stat;
			}
			else
			{
    			PrintAllProcessStat(answer);
			}
		}
		else //Prints sorted current stat of rss
		{
			pProcesses_stat = new PROCESSES_STAT;

			if(SampleProcessStat(pProcesses_stat) == STATUS_OK)
			{
				pProcesses_stat->sort();
				PrintProcessStat(*pProcesses_stat,answer);
			}

			delete pProcesses_stat;
		}
	}
	else	//Prints current stat of all filters
	{
		pProcesses_stat = new PROCESSES_STAT;

		if(SampleProcessStat(pProcesses_stat) == STATUS_OK)
		{
			//CPU filterd
			SProcessStat::m_sortType = eCPUTime;
			pProcesses_stat->sort();
			PrintProcessStat(*pProcesses_stat,answer);

			SProcessStat::m_sortType = eRSS;
			pProcesses_stat->sort();
			PrintProcessStat(*pProcesses_stat,answer);
		}

		delete pProcesses_stat;
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
STATUS CSystemMonitoringManager::HandleTerminalTemperature(CTerminalCommand & command,std::ostream& answer)
{
	int cpu_sensor = 0;
	int hd_sensor = 0;
	GetTemperature(cpu_sensor,hd_sensor);
	answer << "CPU sensor: " << cpu_sensor
		   << " HD sensor: " << hd_sensor;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
STATUS CSystemMonitoringManager::HandleTerminalLoadLogger(CTerminalCommand & command,std::ostream& answer)
{
	std::string stStartStop, stTracesPerSecond, stTracesPerBatch;

	stStartStop 		= command.GetToken(eCmdParam1);
	stTracesPerSecond 	= command.GetToken(eCmdParam2);
	stTracesPerBatch	= command.GetToken(eCmdParam3);

	m_tracesPerSec	 = atoi(stTracesPerSecond.c_str());
	m_tracesPerBatch = atoi(stTracesPerBatch.c_str());

	if(stStartStop == "start" && m_tracesPerSec && m_tracesPerSec <= 100 && m_tracesPerBatch)
	{
		TRACESTR(eLevelInfoNormal)  << "CSystemMonitoringManager::HandleTerminalLoadLogger - start";

		for(int counter = 0; counter < m_tracesPerBatch; counter++)
		{
			TRACESTR(eLevelInfoNormal)  << "CSystemMonitoringManager::HandleTerminalLoadLogger - loading logger with traces, trace no." << logger_load_counter;
			logger_load_counter++;
		}
		StartTimer(LOGGER_LOAD_TOUT, (100 / m_tracesPerSec));

	}

	if(stStartStop == "stop")
	{
		TRACESTR(eLevelInfoNormal)  << "CSystemMonitoringManager::HandleTerminalLoadLogger - stop";
		logger_load_counter = 0;
		DeleteTimer(LOGGER_LOAD_TOUT);
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
void CSystemMonitoringManager::OnLoggerLoadTout(CSegment* pMsg)
{
	for(int counter = 0; counter < m_tracesPerBatch; counter++)
	{
		TRACESTR(eLevelInfoNormal)  << "CSystemMonitoringManager::HandleTerminalLoadLogger - loading logger with traces, trace no." << logger_load_counter;
		logger_load_counter++;
	}
	StartTimer(LOGGER_LOAD_TOUT, (100 / m_tracesPerSec));
}

/////////////////////////////////////////////////////////////////////
void CSystemMonitoringManager::TraceCurrentProcessStat(ePS_PROC traceType)
{
  TRACEINTO << "CSystemMonitoringManager::TraceCurrentProcessStat : " << GetPsProcEnumName(traceType);

  PROCESSES_STAT* pProcesses_stat = new PROCESSES_STAT;
	COstrStream answer;

    if(SampleProcessStat(pProcesses_stat) == STATUS_OK)
    {
        SProcessStat::m_sortType = traceType;
        pProcesses_stat->sort();
        PrintProcessStat(*pProcesses_stat,answer);
        PTRACE(eLevelInfoNormal, answer.str().c_str());
    }
    else
    {
        PASSERTMSG(TRUE, "FAILED to sample process memory");
    }

    PDELETE(pProcesses_stat);
}

/////////////////////////////////////////////////////////////////////
STATUS CSystemMonitoringManager::SampleProcessStat(PROCESSES_STAT* pProcesses_stat)
{
	if(!pProcesses_stat)
		return STATUS_FAIL;

	const char * path = "/proc";
	DIR * dirp;
	struct dirent * dp;

	dirp = opendir (path);

    if (dirp)
    {
    	PS_PROC* psp = new PS_PROC;

    	while ((dp = readdir(dirp)))
        {
        	if ( (strcmp(dp->d_name,".")!=0) && (strcmp(dp->d_name , "..")!=0))
            {
	            int pid = atoi(dp->d_name);

	            memset(psp, 0, sizeof(PS_PROC));
                int res = process_readstat(pid, *psp);

                if(res == ERROR) //Not a process
       				continue;
                else
                {
                	SProcessStat * pSProcessStat = new SProcessStat(*psp);
                	pProcesses_stat->push_back(*pSProcessStat);
                    delete pSProcessStat;
                }
            }
        }

        delete psp;
        closedir(dirp);
    }
    else
    {
 	   PTRACE(eLevelInfoNormal,"CSystemMonitoringManager::OnTimerProcessStat : An error occured while opening the directory");
    	return STATUS_FAIL;
    }

    return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////
void CSystemMonitoringManager::PrintAllProcessStat(std::ostream& oss) const
{
	PROCESSES_STAT_LIST::iterator ProcessesStatListItr;
	WORD ListNumber = 1;
	char strNum[10];

	oss << "\n-------------------PrintAllProcessStat-------------------";

	for (ProcessesStatListItr = m_pProcessStatMap->begin() ; ProcessesStatListItr != m_pProcessStatMap->end() ; ProcessesStatListItr++)
    {
    	snprintf(strNum, sizeof(strNum), "%d",ListNumber);

    	oss <<  "\n\n              Process list num " << strNum;
    	oss <<  "\n            ----------------------";

    	PrintProcessStat(*ProcessesStatListItr,oss);

    	ListNumber++;
    }
}

void CSystemMonitoringManager::PrintProcessStat(const PROCESSES_STAT& ProcessStat,
                                                std::ostream& oss) const
{
  oss.setf(std::ios::left, std::ios::adjustfield);
  oss << "\n" << "Process name          " << "ID          ";
  oss << GetPsProcEnumName(SProcessStat::m_sortType);
  oss << "\n"
      << "-------------------------------------------------------------";

  PROCESSES_STAT::const_iterator ProcessesStatItr;

  for (ProcessesStatItr = ProcessStat.begin();
      ProcessesStatItr != ProcessStat.end(); ProcessesStatItr++)
    oss << *ProcessesStatItr;
}

void CSystemMonitoringManager::CalculateAndFilterProcessesStat(PROCESSES_STAT* pCalculatedProcessesState,ePS_PROC sortType)
{
	SProcessStat::m_sortType = sortType;

	PROCESSES_STAT* pProcessesStatLast = NULL;
	PROCESSES_STAT* pProcessesStatFirst = NULL;

	PROCESSES_STAT::iterator LastProcessesStatItr;
	PROCESSES_STAT::iterator FirstProcessesStatItr;

	//Our reference is the LAST list of processes, under the assumption that
	//if we have a process who caused a problem it will appear on the last list.
	if(m_pProcessStatMap->size())
		pProcessesStatLast = &(m_pProcessStatMap->back());
	else
		return;

    switch(SProcessStat::m_sortType)
   	{
   		case eCPUTime:
   		{
   			pProcessesStatFirst = &(m_pProcessStatMap->front());

    		if(pProcessesStatLast != pProcessesStatFirst)
   			 {
    			SProcessStat* pCalculatedProcessStat = NULL;
    			PS_PROC* pCalculatestat = NULL;
    			int OldCPUUserUsageTime = 0;
    			int OldCPUKernalUsageTime = 0;

				for (LastProcessesStatItr = pProcessesStatLast->begin() ; LastProcessesStatItr != pProcessesStatLast->end() ; LastProcessesStatItr++)
    			{
    				//find the old process stat.
    				for (FirstProcessesStatItr = pProcessesStatFirst->begin() ; FirstProcessesStatItr != pProcessesStatFirst->end() ; FirstProcessesStatItr++)
   						if(LastProcessesStatItr->m_stat == FirstProcessesStatItr->m_stat)
   							break;

   					pCalculatestat = new PS_PROC;

   					switch(SProcessStat::m_sortType)
   					{
   						case eCPUTime:
   						{
   							int OldCPUUserUsageTime = 0;
    						int OldCPUKernalUsageTime = 0;
    						int OldCPUChildrenUserUsageTime = 0;
    						int OldCPUChildrenKernalUsageTime = 0;

   							//We found the old process stat.
   							if(FirstProcessesStatItr != pProcessesStatFirst->end())
   							{
   								OldCPUUserUsageTime = FirstProcessesStatItr->m_stat.utime;
   								OldCPUKernalUsageTime =  FirstProcessesStatItr->m_stat.stime;
   								OldCPUChildrenUserUsageTime = FirstProcessesStatItr->m_stat.cutime;
   								OldCPUChildrenKernalUsageTime = FirstProcessesStatItr->m_stat.cstime;
   							}

   							pCalculatestat->utime = LastProcessesStatItr->m_stat.utime - OldCPUUserUsageTime;
	   						pCalculatestat->stime = LastProcessesStatItr->m_stat.stime - OldCPUKernalUsageTime;
	   						pCalculatestat->cutime = LastProcessesStatItr->m_stat.cutime - OldCPUChildrenUserUsageTime;
	   						pCalculatestat->cstime = LastProcessesStatItr->m_stat.cstime - OldCPUChildrenKernalUsageTime;
	   						//Insert Process name

	  					 	strncpy(pCalculatestat->cmd,LastProcessesStatItr->m_stat.cmd,cmdLen-1);
	   						pCalculatestat->cmd[cmdLen-1] = '\0';

	   						//Insert ID
	   						pCalculatestat->pid = LastProcessesStatItr->m_stat.pid;

	   						pCalculatedProcessStat = new SProcessStat(*pCalculatestat);
	   						pCalculatedProcessesState->push_back(*pCalculatedProcessStat);
                            PDELETE(pCalculatedProcessStat);

	   						break;
   						}

   						default:
   						{
   						}
   					}
                    PDELETE(pCalculatestat);
    			}
   			 }
   		}

   		case eRSS:
   		{
   			*pCalculatedProcessesState = *pProcessesStatLast;
   			break;
   		}

   		default:
   		{
   		}
   	}

	pCalculatedProcessesState->sort();

  if (pCalculatedProcessesState->size() > MAX_NUM_OF_PRINTED_PROCESSES_STAT)
  {
    PROCESSES_STAT::iterator CalculatedItr;
    CalculatedItr = pCalculatedProcessesState->begin();

    for (int i = 0; i < MAX_NUM_OF_PRINTED_PROCESSES_STAT; i++)
      CalculatedItr++;

    pCalculatedProcessesState->erase(CalculatedItr,
        pCalculatedProcessesState->end());
  }
}

std::string CSystemMonitoringManager::GetLoadAverageValues() const
{
	CProcessBase *pProcess = CProcessBase::GetProcess();
    std::string answer,sub;

    STATUS res = SystemPipedCommand("uptime",answer,TRUE,FALSE);

    if (res == STATUS_OK)
    {
    	int index = answer.find("load average:",0);
    	sub = answer.substr(index);
    }

    return sub;
}
/////////////////////////////////////////////////////////////////////
std::string CSystemMonitoringManager::GetTotalMemSize() const
{
	CProcessBase *pProcess = CProcessBase::GetProcess();
    std::string meminfo,sub;

    STATUS res = SystemPipedCommand("cat /proc/meminfo",meminfo,TRUE,FALSE);

    if (res == STATUS_OK)
    {
    	//First line contains TotalMemSize.
    	int index = meminfo.find("kB",0);
    	sub = meminfo.substr(0,index+2);// +2 for "kB"
    }

    return sub;
}

////////////////////////////////////////////////////////////////////////////
MCMS_AUTHENTICATION_S*  CSystemMonitoringManager::GetAuthenticationStruct()
{
	return m_pAuthenticationStruct;
}


////////////////////////////////////////////////////////////////////////////
void CSystemMonitoringManager::OnTimerEthSettingsMonitoring(CSegment* pSeg)
{
	const string sTheCaller("CSystemMonitoringManager::OnTimerEthSettingsMonitoring");

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if(eProductTypeRMX4000 == curProductType)
	{
	HandleEthSettingsMonitoring(ETH_SETTINGS_PORT_MANAGEMENT,	sTheCaller);
	HandleEthSettingsMonitoring(ETH_SETTINGS_PORT_SIGNALING,	sTheCaller);
	}

	if(eProductTypeRMX1500 == curProductType)
	{
	HandleEthSettingsMonitoring(ETH_SETTINGS_CPU_MNGMNT_PORT_ON_SWITCH_BOARD_RMX1500,	sTheCaller);
	HandleEthSettingsMonitoring(ETH_SETTINGS_CPU_MNGMNTB_PORT_ON_SWITCH_BOARD_RMX1500,	sTheCaller);
	}


   // StartTimer(ETHERNET_SETTINGS_MONITORING_TIMER,ETHERNET_SETTINGS_MONITORING_INTERVAL);
}

////////////////////////////////////////////////////////////////////////////
void CSystemMonitoringManager::OnEthSettingClearMaxCountersReq(CSegment* pSeg)
{
	DWORD portId = 0;

	*pSeg >> portId;


	const string sTheCaller("CSystemMonitoringManager::OnEthSettingClearMaxCountersReq");
	HandleEthSettingsMonitoring(portId, sTheCaller, true);
}

////////////////////////////////////////////////////////////////////////////
void CSystemMonitoringManager::HandleEthSettingsMonitoring(const DWORD portId, const string &theCaller, const bool isClearMaxCounters/*=false*/)
{
    TRACESTR(eLevelInfoNormal)  << "\nCSystemMonitoringManager::HandleEthSettingsMonitoring (caller: " << theCaller << ")";

    // ===== 1. get current eth settings
	CConfigManagerApi configApi;

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	eConfigInterfaceType ifType = ConvertEthPortToConfigInterfaceType(portId,curProductType);
	ETH_SETTINGS_S* pCurEthStruct = new ETH_SETTINGS_S;


	 if (eProductTypeRMX4000 == curProductType || eProductTypeRMX1500 == curProductType)
	   configApi.EthernetSettingsMonitoring( *pCurEthStruct, GetInterfaceNum(ifType,m_curIpType) );
	else
		configApi.EthernetSettingsMonitoring( *pCurEthStruct, GetInterfaceNum(ifType,eIpType_IpV4) );


	// ===== 2. update local list (for maintaining the 'Max' attributes)
	ETH_SETTINGS_SPEC_S* pEthSettings	= new ETH_SETTINGS_SPEC_S;
	
	pEthSettings->portParams.slotId		= (DWORD)FIXED_DISPLAY_BOARD_ID_SWITCH;
	
	pEthSettings->portParams.portNum	= portId;
	memcpy( &(pEthSettings->monitoringParams), pCurEthStruct, sizeof(ETH_SETTINGS_S) );

	delete pCurEthStruct;
	TRACESTR(eLevelInfoNormal)  << "\nCSystemMonitoringManager::HandleEthSettingsMonitoring slotid " << pEthSettings->portParams.slotId << " portid "<<portId;

	CSystemMonitoringProcess *pProcess = (CSystemMonitoringProcess*)CSystemMonitoringProcess::GetProcess();
	pProcess->SetSpecEthernetSettingsStructWrapper(pEthSettings);

	if (false == isClearMaxCounters)
	{
		pProcess->UpdateSpecEthernetSettingsMaxCounters(portId);
	}
	else
	{
		pProcess->ClearSpecEthernetSettingsMaxCounters(portId);
	}

	ETH_SETTINGS_SPEC_S* pTmpEthSettings = pProcess->GetSpecEthernetSettingsStructWrapper(portId);
	if (pTmpEthSettings != NULL)
		memcpy( pEthSettings, pTmpEthSettings, sizeof(ETH_SETTINGS_SPEC_S) );
	else
		  TRACESTR(eLevelInfoNormal) << "\nCSystemMonitoringManager::HandleEthSettingsMonitoring failed to get GetSpecEthernetSettingsStructWrapper(portId)\n";

	// ===== 3. send to Cards process
    CSegment* pRetSeg = new CSegment;
    pRetSeg->Put( (BYTE*)pEthSettings, sizeof(ETH_SETTINGS_SPEC_S) );

	delete pEthSettings;

	CManagerApi api(eProcessCards);
	api.SendMsg(pRetSeg, SYSMONITOR_CARDS_ETHERNET_SETTINGS_MONITORING_IND);
}

void CSystemMonitoringManager::OnShmCompSlotIdInd(CSegment* pMsg)
{
	RemoveActiveAlarmByErrorCode(AA_UNKNOWN_CPU_SLOT_ID);

    DWORD tmpCpuBoardId		= 0,
    	  tmpCpuSubBoardId	= 0,
          tmpCompType		= 0;

    eShmComponentType compType = eShmComp_Illegal;

    *pMsg >> tmpCpuBoardId
          >> tmpCpuSubBoardId
          >> tmpCompType;

    m_cpuBoardId	= (BYTE)tmpCpuBoardId;
    m_cpuSubBoardId	= (BYTE)tmpCpuSubBoardId;
    compType		= (eShmComponentType)tmpCompType;

    TRACESTR(eLevelInfoNormal) << "\nCSystemMonitoringManager::OnShmCompSlotIdInd"
                           << "\nBoardId:    " << (int)m_cpuBoardId
                           << "\nSubBoardId: " << (int)m_cpuSubBoardId
                           << "\nCmpnntType: " << ::ShelfMngrComponentTypeToString(compType);
}

static bool is_not_logfile(const std::string& s)
{
  const char pat[] = ".log";
  size_t pos = s.rfind(pat);
  if (std::string::npos == pos)
    return true;

  if (s.length()-ARRAYSIZE(pat)+1 != pos)
    return true;

  return false;
}

void CSystemMonitoringManager::DumpOSStartupLogs()
{
  std::ostringstream err;
  std::vector<std::string> files;
  std::string osStartupLogsPath = OS_STARTUP_LOGS_PATH;
  if (!GetDirectoryContents(osStartupLogsPath.c_str(), files, err))
    return;

  // Filters only *.log files.
  std::vector<std::string>::iterator old_end = files.end();
  std::vector<std::string>::iterator new_end =
    std::remove_if (files.begin(), files.end(), is_not_logfile);
  files.erase(new_end, old_end);

  TRACEINTO << "There are " << files.size() << " startup logfiles";

  std::string remove_cmd,answer;

  std::vector<std::string>::const_iterator i;
  for (i = files.begin(); i != files.end(); ++i)
  {
    std::stringstream buf;
    buf << OS_STARTUP_LOGS_PATH << *i;
    ::DumpFile(buf.str());

    remove_cmd = "rm -f " + buf.str();
    SystemPipedCommand(remove_cmd.c_str(), answer);
  }

  std::vector<std::string> watcher_files;
  string watcher_path = "WatcherLogs/";
  string fullFilepath =OS_STARTUP_LOGS_PATH + watcher_path;
  if (!GetDirectoryContents(fullFilepath.c_str(), watcher_files, err))
       return;

  for( std::vector<string>::const_iterator i = watcher_files.begin(); i != watcher_files.end(); ++i)
  {
	  std::stringstream buf;
	  buf << fullFilepath << *i;
	  STATUS stat = DumpFile(buf.str());
	  if (stat == STATUS_OK)
	  {
		  remove_cmd= "rm -f " + buf.str();
		  SystemPipedCommand(remove_cmd.c_str(), answer);
	  }
  }

}

SProcessStat::SProcessStat()
{}

SProcessStat::SProcessStat(const PS_PROC &Stat)
:m_stat(Stat)
{}

SProcessStat::~SProcessStat()
{}

bool SProcessStat::operator< ( const SProcessStat  & r)
{
	switch(m_sortType)
	{
		case eCPUTime:
		  return (m_stat.utime +  m_stat.stime + m_stat.cutime +  m_stat.cstime) >
				     (r.m_stat.utime + r.m_stat.stime + r.m_stat.cutime + r.m_stat.cstime);

		case eRSS:
		  return m_stat.rss > r.m_stat.rss;

		default:
		  return (m_stat.utime +  m_stat.stime + m_stat.cutime +  m_stat.cstime) >
					   (r.m_stat.utime + r.m_stat.stime + r.m_stat.cutime + r.m_stat.cstime);
	}
}

const char* SProcessStat::NameOf() const
{
	return "SProcessStat";
}

void SProcessStat::Dump(std::ostream& oss) const
{
	oss << "\n" << std::setw(23) << m_stat.cmd;

	char ID[11];
	snprintf(ID, sizeof(ID), "%d",m_stat.pid);
CProcessBase *pProcess = CProcessBase::GetProcess();
        std::string answer;
        SystemPipedCommand("uptime",answer,TRUE,FALSE);
        int index = answer.find("load average:",0);
        std::string sub = answer.substr(index + 13);
        float min1,min5,min15;
        sscanf(sub.c_str(),"%f, %f, %f,",&min1,&min5,&min15);	oss << std::setw(12) << ID;

	switch(m_sortType)
	{
		case eCPUTime:
		{
			char r[20];
			sprintf(r,"%d",m_stat.utime + m_stat.stime + m_stat.cutime + m_stat.cstime);
			oss << std::setw(10) << r;
			break;
		}

		case eRSS:
		{
			char r[20];
			sprintf(r,"%d",m_stat.rss * 4); //The size of each page is 4KB
			oss << std::setw(10) << r;
			break;
		}

		default:
		{
		}
	}
}

void CSystemMonitoringManager::CpuComEInfo()
{
  CConfigManagerApi api;
  std::string hd_size;
  STATUS stat = api.GetHDSize(hd_size);
  TRACECOND(stat != STATUS_OK, "GetHDSize failed, status=" << stat);

  std::string hd_details;
  stat = api.GetHDModel(hd_details);
  TRACECOND(stat != STATUS_OK, "GetHDModel failed, status=" << stat);

  std::string flash_size;
  stat = api.GetFlashSize(flash_size);
  TRACECOND(stat != STATUS_OK, "GetFlashSize failed, status=" << stat);

  std::string flash_model_details;
  stat = api.GetFlashModel(flash_model_details);
  TRACECOND(stat != STATUS_OK, "GetFlashModel failed, status=" << stat);

  std::string cpu_type;
  stat = api.GetCPUType(cpu_type);
  TRACECOND(stat != STATUS_OK, "GetCPUType failed, status=" << stat);

  std::string ram_size;
  stat = api.GetRAMSize(ram_size);
  TRACECOND(stat != STATUS_OK, "GetRAMSize failed, status=" << stat);

  std::string hd_firmware;
  stat = api.GetHDFirmware(hd_firmware);
  TRACECOND(stat != STATUS_OK, "GetHDFirmware failed, status=" << stat);

	std::string bios_version = "--";
	std::string bios_date = "--";
  // bios data not relevant for SoftwareMcu
  eProductType prodType = CProcessBase::GetProcess()->GetProductType();
  eProductFamily prodFamily = CProcessBase::GetProcess()->GetProductFamily();
  if( eProductFamilySoftMcu != prodFamily )
  {
    std::string cmd_bios_version = "echo -n `cat /sys/class/dmi/id/bios_version`";
    stat = SystemPipedCommand(cmd_bios_version.c_str(), bios_version);
    TRACECOND(stat != STATUS_OK, "cmd_bios_version failed, status=" << stat);

    std::string cmd_bios_date = "echo -n `cat /sys/class/dmi/id/bios_date`";
    stat = SystemPipedCommand(cmd_bios_date.c_str(), bios_date);
    TRACECOND(stat != STATUS_OK, "cmd_bios_date failed, status=" << stat);
  }

  TRACEINTO << "\nHD Size = " <<hd_size
            << "\nHD Firmware Version = " <<hd_firmware
            <<"\nHD model details = " <<hd_details
            <<"\nFlash size = " <<flash_size
            <<"\nFlash model details = " <<flash_model_details
            <<"\nRAM Size = " <<ram_size
            <<"\nBios version: " <<bios_version
            <<"\nBios date: " <<bios_date
            <<"\nCPU type:\n" <<cpu_type;

  std::string cpuInfo; 
     
 TRACEINTO <<"prodType " << prodType;
    if(eProductTypeGesher == prodType || eProductTypeNinja == prodType)
    {

        cpuInfo = "HD Size = " + hd_size + "\nHD Firmware Version = " + hd_firmware
         + "\nHD model details = " + hd_details + "\n";
 	 cpuInfo += "RAM Size = "+ram_size
         +"\nBios version: " + bios_version + "\nBios date: "+bios_date + "\nCPU type:"+cpu_type;
    }
    else
    {
      cpuInfo ="HD Size = " + hd_size + "\nHD Firmware Version = " + hd_firmware
      + "\nHD model details = " + hd_details + "\nFlash size = "+flash_size
      + "\nFlash model details = "+flash_model_details + "\nRAM Size = "+ram_size
      +"\nBios version: " + bios_version + "\nBios date: "+bios_date + "\nCPU type:"+cpu_type;
    }
  

  CSegment*  pParamSeg = new CSegment;
  *pParamSeg << cpuInfo;
  CManagerApi apiMngr(eProcessMcuMngr);
  apiMngr.SendMsg(pParamSeg, SYSTEM_MONITORING_MCUMNGR_CPU_INFO);
}

bool operator ==(const SProcessStat& r, const SProcessStat& l)
{
  return (l.m_stat.pid) == (r.m_stat.pid);
}

STATUS CSystemMonitoringManager::HandleTerminalIncreaseTemperature(CTerminalCommand & command,std::ostream& answer)
{
	string cpuTemp;
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	m_cpuTemperatureControl->GetCpuTemperature(cpuTemp) ;
	answer << "Start test: increase cpu temperature, temp = " << cpuTemp;

	int counter = 1;
	while(counter < 9000)
	{
		counter ++;
	}
	m_cpuTemperatureControl->GetCpuTemperature(cpuTemp) ;
	
	answer << "\nEnd test:  temp = " << cpuTemp;
	return STATUS_OK;
}

STATUS CSystemMonitoringManager::HandleDumpMcuStatus(CTerminalCommand & command,std::ostream& answer)
{
	std::string ans;
	std::string cmd = "."+MCU_MCMS_DIR+"/Scripts/SoftMcuStatus.sh 2>&1";

 	STATUS stat = SystemPipedCommand(cmd.c_str(), ans);
	PASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != stat, 
					cmd << ": " << ans << ": " << stat, 
					STATUS_FAIL);
	TRACEINTO << ans;	
	return STATUS_OK;
}

BOOL CSystemMonitoringManager::IsFintec()
{
  STATUS stat;
  std::string ans_first_register, ans_second_register;
  STATUS res;

  CConfigManagerApi api;
  stat = api.ReadValueFromRegisterBySensor(ans_first_register, "0x2d", "0x5d");
  stat = api.ReadValueFromRegisterBySensor(ans_second_register, "0x2d", "0x5e");

  //Fintec at 0x5d and 0x5e is 0x1934
  return (("0x19" == ans_first_register) && ("0x34" == ans_second_register));

}

BOOL CSystemMonitoringManager::IsPortwell()
{
	STATUS stat;
	std::string ans;
	CConfigManagerApi api;

	stat = api.ReadValueFromRegisterBySensor(ans, "0x4c", "0xfe");

	TRACEINTO << "IsPortwell str_0xfe =" << ans;

	//Portwell at 0xfe is 0x41
	return ("0x41" == ans);

}

eCpuManufacturerType CSystemMonitoringManager::GetCpuManufacturerType()
{
	eCpuManufacturerType cpuManufacturerType = eCpuManufacturerTypeUnknown;

	if(FALSE == IsTarget())
	{
		cpuManufacturerType = eCpuManufacturerTypePortwell;
		return cpuManufacturerType;
	}

	if(IsPortwell())
	{
		cpuManufacturerType = eCpuManufacturerTypePortwell;
	}
	else if (IsFintec())
	{
		cpuManufacturerType = eCpuManufacturerTypeFintec;
	}

	return cpuManufacturerType;
}

////////////////////////////////////////////////////////////////////////////
void CSystemMonitoringManager::OnTimerCheckMediaRecordingFolderSizeTout(CSegment* pSeg)
{
    std::string ans;
    std::string cmd = "du -s "+MCU_MCMS_DIR+"/MediaRecording/share/ | awk '{print $1}'";

    STATUS status = SystemPipedCommand(cmd.c_str(), ans,TRUE,FALSE);

    if(STATUS_OK != status)
    {
       TRACESTR(eLevelInfoNormal) << "\nFailed to execute command: " << cmd.c_str();
    }
    long folderSize = atol(ans.c_str());
    if (folderSize > MEDIA_RECORDING_MAX_FOLDER_SIZE) // Bigger then 1.5Giga
    {
        //Send Stop All Media Recording Command
        CManagerApi api(eProcessResource);

        STATUS res = api.SendOpcodeMsg(SYSMNTR_TO_RSRC_STOP_ALL_MEDIA_RECORDING_REQ);

        if(STATUS_OK != res)
        {
            PASSERT(1);
        }
    }
    StartTimer(SYSTEM_CHECK_MEDIA_RECORDING_FOLDER_SIZE_TIMER, 120 * SECOND);
}

////////////////////////////////////////////////////////////////////////////
void CSystemMonitoringManager::OnTimerCheckCdrPersistenceQueueFolderSizeTout(CSegment* pSeg)
{
	TRACEINTO << "\nCSystemMonitoringManager::OnTimerCheckCdrPersistenceQueueFolderSizeTout";
	if(m_isRemoteCdrEnabled == false)
	{
		DisableActiveAlarmRemoteCdrFeature();
		return;
	}
    std::string ans;
    std::string cmd;
    //if boolean member is false return
    if (IsRmxSimulation())
    	 cmd = "du -s "+MCU_OUTPUT_TMP_DIR+"/cdr/ | awk '{print $1}'";
    else
    	 cmd = "du -s " + MCU_TMP_DIR+"/cdr/ | awk '{print $1}'";

    STATUS status = SystemPipedCommand(cmd.c_str(), ans,TRUE,FALSE);

    if(STATUS_OK != status)
    {
       StartTimer(CDR_PERSISTENCE_QUEUE_FOLDER_SIZE_TIMER, CHECK_PERSISTENCE_QUEUE_FOLDER_SIZE_INTERVAL);
       return;
    }

    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

    DWORD	 cdrServiceQueueSizeFromFlag;
    sysConfig->GetDWORDDataByKey(CFG_KEY_CDR_SERVICE_QUEUE_SIZE, cdrServiceQueueSizeFromFlag);
    DWORD	 cdrServiceQueueSize = cdrServiceQueueSizeFromFlag*1024*1024;

    DWORD	 cdrServiceFullThreashold;
    sysConfig->GetDWORDDataByKey(CFG_KEY_CDR_SERVICE_QUEUE_FULL_THRESHOLD, cdrServiceFullThreashold);

    DWORD folderSize = atol(ans.c_str());
    //not posiible to write to the queue
    if (folderSize > cdrServiceQueueSize)
    {
    	// check if not already disabled
    	if(m_isPersistenceQueueEnabled == true)
    	{
    		//Send Stop CDR persistence queue
    		CManagerApi api(eProcessCDR);

    		STATUS res = api.SendOpcodeMsg(CDR_PERSISTENCE_QUEUE_DISABLE);

    		if(STATUS_OK != res)
    		{
    			PASSERT(1);
    			return;
    		}
    		m_isPersistenceQueueEnabled = false;
    	}
    }
    //posiible to write to the queue
    else
    {
    	// check if not already enabled
    	if(m_isPersistenceQueueEnabled == false)
    	{
    		CManagerApi api(eProcessCDR);

    		STATUS res = api.SendOpcodeMsg(CDR_PERSISTENCE_QUEUE_ENABLE);

    		if(STATUS_OK != res)
    		{
    			PASSERT(1);
    			return;
    		}
    		m_isPersistenceQueueEnabled = true;
    	}
    }


    if((cdrServiceQueueSize*cdrServiceFullThreashold/100)< folderSize && folderSize < cdrServiceQueueSize)
    {
    	if(!IsActiveAlarmExistByErrorCode(AA_CDR_CHECK_CONNECTION_TO_CDR_SERVER ))
        		AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
        				AA_CDR_CHECK_CONNECTION_TO_CDR_SERVER ,
        				MAJOR_ERROR_LEVEL,
        				"Check connection to CDR server. CDR events buffer will soon be full",
        				true,
        				true);
    }
    else if (folderSize > cdrServiceQueueSize)
    {
    	if(!IsActiveAlarmExistByErrorCode(AA_CDR_INFORMATION_TO_CDR_SERVER_LOST))
    		AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
    				AA_CDR_INFORMATION_TO_CDR_SERVER_LOST,
    				MAJOR_ERROR_LEVEL,
    				"CDR information to remote CDR Service is being lost",
    				true,
    				true);
    }
    TRACEINTO << "\nCSystemMonitoringManager::AA_CDR_INFORMATION_TO_CDR_SERVER_LOST cdrServiceQueueSize = "<<cdrServiceQueueSize;
    TRACEINTO << "\nCSystemMonitoringManager::AA_CDR_INFORMATION_TO_CDR_SERVER_LOST 95*cdrServiceQueueSize/100 = "<<95*cdrServiceQueueSize/100;
    if(folderSize < (95*cdrServiceQueueSize/100))
    {
    	if(IsActiveAlarmExistByErrorCode(AA_CDR_INFORMATION_TO_CDR_SERVER_LOST))
    		RemoveActiveAlarmByErrorCode(AA_CDR_INFORMATION_TO_CDR_SERVER_LOST);
    }

    if(folderSize < ((cdrServiceFullThreashold-5)*cdrServiceQueueSize/100))
    {
    	if(IsActiveAlarmExistByErrorCode(AA_CDR_CHECK_CONNECTION_TO_CDR_SERVER))
    		RemoveActiveAlarmByErrorCode(AA_CDR_CHECK_CONNECTION_TO_CDR_SERVER);
    }

    StartTimer(CDR_PERSISTENCE_QUEUE_FOLDER_SIZE_TIMER, CHECK_PERSISTENCE_QUEUE_FOLDER_SIZE_INTERVAL);

}

//////////////////////////////////////////////////////////////////////
void CSystemMonitoringManager::EnableDisableRemoteCdrTimer(CSegment *pSeg)
{
	if(!pSeg)
	{
		TRACEINTO << "\nCSystemMonitoringManager::EnableDisableRemoteCdrTimer: Empty segment";
		return;
	}
	BYTE isRemoteCdrEnable = 0;
	*pSeg >> isRemoteCdrEnable;

	if(isRemoteCdrEnable == true)
	{
		TRACESTR(eLevelInfoNormal) << "\nCSystemMonitoringManager::EnableDisableRemoteCdrTimer - true";
		if(m_isRemoteCdrEnabled == false)
		{
			TRACESTR(eLevelInfoNormal) << "\nCSystemMonitoringManager::EnableDisableRemoteCdrTimer - m_isRemoteCdrEnabled == false";
			m_isRemoteCdrEnabled = true;
			StartTimer(CDR_PERSISTENCE_QUEUE_FOLDER_SIZE_TIMER, CHECK_PERSISTENCE_QUEUE_FOLDER_SIZE_INTERVAL);
		}
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCSystemMonitoringManager::EnableDisableRemoteCdrTimer - false";
		m_isRemoteCdrEnabled = false;
	}
}

//////////////////////////////////////////////////////////////////////




void CSystemMonitoringManager::SelfKill(void)
{

  m_PipeToSystemMonitoringTaskApi.Destroy();

  CManagerTask::SelfKill();
}

void CSystemMonitoringManager::DisableActiveAlarmRemoteCdrFeature()
{
	if(IsActiveAlarmExistByErrorCode(AA_CDR_CHECK_CONNECTION_TO_CDR_SERVER))
	{
		RemoveActiveAlarmByErrorCode(AA_CDR_CHECK_CONNECTION_TO_CDR_SERVER);
	}

	if(IsActiveAlarmExistByErrorCode(AA_CDR_INFORMATION_TO_CDR_SERVER_LOST))
	{
		RemoveActiveAlarmByErrorCode(AA_CDR_INFORMATION_TO_CDR_SERVER_LOST);
	}

	if(IsActiveAlarmExistByErrorCode(AA_FAIL_TO_REGISTER_TO_CDR_SERVER))
	{
		RemoveActiveAlarmByErrorCode(AA_FAIL_TO_REGISTER_TO_CDR_SERVER);
	}
}
