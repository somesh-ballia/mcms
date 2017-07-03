// McuMngrMonitor.cpp: implementation of the McuMngrMonitor class.
//
//////////////////////////////////////////////////////////////////////


#include "McuMngrMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "TraceStream.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"
#include "SysConfig.h"
#include "SysConfigGet.h"
#include "ProcessBase.h"
#include "Request.h"
#include "IpService.h"
#include "McuMngrStructs.h"
#include "MplMcmsProtocol.h"
#include "IpInterfacesListGet.h"
#include "IpParameters.h"
#include "DummyEntry.h"
#include "McuStateGet.h"
#include "ActiveAlarmsListGet.h"
#include "McuTimeGet.h"
#include "McuIpmiEntityListGet.h"
#include "McuIpmiEntityFanInfoGet.h"
#include "McuIpmiEntityFanLevelGet.h"
#include "McuIpmiEntityEventLogGet.h"
#include "McuIpmiFruGet.h"
#include "McuIpmiSensorListGet.h"
#include "McuIpmiSensorReadingListGet.h"
#include "McuLanPortListGet.h"
#include "McuLanPortGet.h"
#include "ApacheDefines.h"
#include "SystemFunctions.h"
#include "LicensingGet.h"
#include "EthernetSettingsConfigListGet.h"
#include "GetCpuMemUsage.h"
#include "sensor_read.h"
#include "sensor_read_cache.h"
#include "IpmiSensorDescrToType.h"
#include "FaultsDefines.h"
#include "PrecedenceSettingsGet.h"
#include "PlcmTimeConfig.h"

#include "SysConfigKeys.h"

#define MEMORY_REPORT_TIMER 301
#define HARDWARE_MONITORING_TIMER 302
#define SHUTDOWN_TIMER  303
////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CMcuMngrMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CMcuMngrMonitor::HandlePostRequest)
  ONEVENT(MEMORY_REPORT_TIMER ,IDLE , CMcuMngrMonitor::OnTimerLogMemory)
  ONEVENT(HARDWARE_MONITORING_TIMER, IDLE, CMcuMngrMonitor::OnTimerHardwareMonitoring)
  ONEVENT(SHUTDOWN_TIMER,	ANYCASE,  CMcuMngrMonitor::OnShutdownRequest )	
PEND_MESSAGE_MAP(CMcuMngrMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CMcuMngrMonitor)
	ON_TRANS("TRANS_CFG"					, "GET_CFG"						, CSysConfigGet					, CMcuMngrMonitor::HandleGetCfg)
	ON_TRANS("TRANS_IP_SERVICE_LIST"		, "GET_MANAGEMENT_NETWORK_LIST"	, CIpInterfacesListGet			, CMcuMngrMonitor::OnGetMngmntNetwork)
	ON_TRANS("TRANS_ACTIVE_ALARMS_LIST"		, "GET"							, CActiveAlarmsListGet			, CMcuMngrMonitor::OnGetActiveAlarmsList)
	ON_TRANS("TRANS_MCU"					, "GET_STATE"					, CMcuStateGet					, CMcuMngrMonitor::OnGetMcuState)
	ON_TRANS("TRANS_MCU"					, "GET_TIME"					, CMcuTimeGet					, CMcuMngrMonitor::OnGetMcuTime)
ON_TRANS("TRANS_MCU" , "GET_PRECEDENCE_SETTINGS" , CPrecedenceSettingsGet , CMcuMngrMonitor::HandleGetPrecedenceSettings)
	ON_TRANS("TRANS_MCU"					, "GET_CFS"				        , CLicensingGet					, CMcuMngrMonitor::OnGetCfs)
	ON_TRANS("TRANS_ETHERNET_SETTINGS_LIST"	, "GET"							, CEthernetSettingsConfigListGet, CMcuMngrMonitor::OnGetEthernetSettingsConfigList)
	ON_TRANS("TRANS_IPMI_ENTITY_LIST"	    , "GET"							, CMcuIpmiEntityListGet, CMcuMngrMonitor::OnGetIpmiEntityList)
	ON_TRANS("TRANS_IPMI_ENTITY"	        , "GET_FAN_INFO"				, CMcuIpmiEntityFanInfoGet, CMcuMngrMonitor::OnGetIpmiEntityFanInfo)
	ON_TRANS("TRANS_IPMI_ENTITY"	        , "GET_FAN_LEVEL"				, CMcuIpmiEntityFanLevelGet, CMcuMngrMonitor::OnGetIpmiEntityFanLevel)
	ON_TRANS("TRANS_IPMI_ENTITY"	        , "GET_EVENT_LOG"				, CMcuIpmiEntityEventLogGet, CMcuMngrMonitor::OnGetIpmiEntityEventLog)
	ON_TRANS("TRANS_IPMI_FRU"	            , "GET"							, CMcuIpmiFruGet, CMcuMngrMonitor::OnGetIpmiFru)
	ON_TRANS("TRANS_IPMI_SENSOR_LIST"	    , "GET"							, CMcuIpmiSensorListGet, CMcuMngrMonitor::OnGetIpmiSensorList)
	ON_TRANS("TRANS_IPMI_SENSOR_READING_LIST"	, "GET"						, CMcuIpmiSensorReadingListGet, CMcuMngrMonitor::OnGetIpmiSensorReadingList)
	ON_TRANS("TRANS_LAN_PORT_LIST"	        , "GET"							, CMcuLanPortListGet, CMcuMngrMonitor::OnGetLanPortList)
	ON_TRANS("TRANS_LAN_PORT"	        , "GET"							, CMcuLanPortGet, CMcuMngrMonitor::OnGetLanPort)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void McuMngrMonitorEntryPoint(void* appParam)
{
	CMcuMngrMonitor *monitorTask = new CMcuMngrMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMcuMngrMonitor::CMcuMngrMonitor()
    : m_pCpuMemUsage(new CSysState())
{
    m_mcuMngrProductType = CProcessBase::GetProcess()->GetProductType();
    m_sensorCacheTid = (pthread_t)0;
    m_bQuit = 0;
    m_bShutdown = false;
}

bool CMcuMngrMonitor::IsPlatformOfNewArch() const
{
	return 0
		|| (eProductTypeGesher==m_mcuMngrProductType)
		|| (eProductTypeNinja==m_mcuMngrProductType)
		;
}

//////////////////////////////////////////////////////////////////////
CMcuMngrMonitor::~CMcuMngrMonitor()
{
    if (IsPlatformOfNewArch())
    {
        WaitForSensorReadCache(m_sensorCacheTid, m_bQuit);
        DestroySensorsCacheLock();
        DeleteTimer(HARDWARE_MONITORING_TIMER);
    }
    
    delete m_pCpuMemUsage;
    m_pCpuMemUsage = 0;
}

void CMcuMngrMonitor::InitTask()
{
    CMonitorTask::InitTask();
    StartTimer(MEMORY_REPORT_TIMER,5*SECOND);

    if (IsPlatformOfNewArch())
    {
        StartTimer(HARDWARE_MONITORING_TIMER, 5*SECOND);
        InitSensorsCacheLock();
        StartSensorReadCache(m_sensorCacheTid, m_bQuit);
    }

//    ReadValidateCfgTmpFiles();
}

//////////////////////////////////////////////////////////////////////
STATUS CMcuMngrMonitor::HandleGetCfg(CRequest *pGetRequest)
{

	if (pGetRequest->GetAuthorization() != SUPER &&
        pGetRequest->GetAuthorization() != ORDINARY &&
        pGetRequest->GetAuthorization() != AUTH_OPERATOR &&
        pGetRequest->GetAuthorization() != ADMINISTRATOR_READONLY)
	{
		pGetRequest->SetConfirmObject(new CDummyEntry());
		TRACESTR(eLevelInfoNormal) << "CMcuMngrMonitor::HandleGetCfg: No permission to get system.cfg files";
		pGetRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CSysConfigGet *pSysConfigGet = (CSysConfigGet*)pGetRequest->GetRequestObject();
	eCfgParamType fileType = pSysConfigGet->GetFileType();
	CSysConfigEma *pSysConfigConfirm = new CSysConfigEma();
	pSysConfigConfirm->LoadFromFile(fileType);
	pSysConfigConfirm->RemoveParamsInFileNonVisible();
	pGetRequest->SetConfirmObject(pSysConfigConfirm);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrMonitor::OnGetMngmntNetwork(CRequest* pGetRequest)
{
	CIpInterfacesListGet* pIpInterfacesListGet = new CIpInterfacesListGet();
	*pIpInterfacesListGet  = *(CIpInterfacesListGet*)pGetRequest->GetRequestObject();
	pGetRequest->SetConfirmObject(pIpInterfacesListGet);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CMcuMngrMonitor::OnGetActiveAlarmsList(CRequest* pGetRequest)
{
	CActiveAlarmsListGet* pActiveAlarmsListGet = new CActiveAlarmsListGet;
	*pActiveAlarmsListGet = *(CActiveAlarmsListGet*)pGetRequest->GetRequestObject() ;
	pGetRequest->SetConfirmObject(pActiveAlarmsListGet);

 	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrMonitor::OnGetMcuState(CRequest* pGetRequest)
{
	CMcuStateGet* pMcuStateGet = new CMcuStateGet();
	*pMcuStateGet  = *(CMcuStateGet*)pGetRequest->GetRequestObject();
	pGetRequest->SetConfirmObject(pMcuStateGet);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrMonitor::OnGetMcuTime(CRequest* pGetRequest)
{
	CMcuTimeGet* pMcuTimeGet = new CMcuTimeGet();
	*pMcuTimeGet  = *(CMcuTimeGet*)pGetRequest->GetRequestObject();
	pGetRequest->SetConfirmObject(pMcuTimeGet);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrMonitor::HandleGetPrecedenceSettings(CRequest* pGetRequest) {
	TRACESTR(eLevelInfoNormal) << "CMcuMngrMonitor::HandleGetPrecedenceSettings " << PRECEDENCE_SETTINGS_FILE;

	CPrecedenceSettings* pPrecedenceSettings = new CPrecedenceSettings();

	CMcuMngrProcess* pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	*pPrecedenceSettings = *(pProcess->GetPrecedenceSettings());

	pGetRequest->SetConfirmObject(pPrecedenceSettings);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrMonitor::OnGetCfs(CRequest* pGetRequest) {
	CLicensingGet* pLicensingGet = new CLicensingGet();
	*pLicensingGet  = *(CLicensingGet*)pGetRequest->GetRequestObject();
	pGetRequest->SetConfirmObject(pLicensingGet);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CMcuMngrMonitor::OnGetEthernetSettingsConfigList(CRequest* pGetRequest)
{
	CEthernetSettingsConfigListGet* pEthernetSettingsConfigListGet = new CEthernetSettingsConfigListGet;
	*pEthernetSettingsConfigListGet = *(CEthernetSettingsConfigListGet*)pGetRequest->GetRequestObject() ;
	pGetRequest->SetConfirmObject(pEthernetSettingsConfigListGet);

 	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

template <typename RequestGetType>
RequestGetType * OnGetRequest(CRequest * pGetRequest)
{
    RequestGetType * const pGet = new RequestGetType;
    *pGet = *(RequestGetType*)pGetRequest->GetRequestObject();
    pGetRequest->SetConfirmObject(pGet);
    return pGet;
}

STATUS CMcuMngrMonitor::OnGetIpmiEntityList(CRequest * pGetRequest)
{
    OnGetRequest<CMcuIpmiEntityListGet>(pGetRequest);

    return STATUS_OK;
}

STATUS CMcuMngrMonitor::OnGetIpmiEntityFanInfo(CRequest * pGetRequest)
{
    OnGetRequest<CMcuIpmiEntityFanInfoGet>(pGetRequest);

    return STATUS_OK;
}

STATUS CMcuMngrMonitor::OnGetIpmiEntityFanLevel(CRequest * pGetRequest)
{
    OnGetRequest<CMcuIpmiEntityFanLevelGet>(pGetRequest);

    return STATUS_OK;
}

STATUS CMcuMngrMonitor::OnGetIpmiEntityEventLog(CRequest * pGetRequest)
{
    OnGetRequest<CMcuIpmiEntityEventLogGet>(pGetRequest);

    return STATUS_OK;
}

STATUS CMcuMngrMonitor::OnGetIpmiFru(CRequest * pGetRequest)
{
    OnGetRequest<CMcuIpmiFruGet>(pGetRequest);

    return STATUS_OK;
}

STATUS CMcuMngrMonitor::OnGetIpmiSensorList(CRequest * pGetRequest)
{
    CMcuIpmiSensorListGet * const pGet = OnGetRequest<CMcuIpmiSensorListGet>(pGetRequest);
    pGet->SetCpuMemUsageGetter(m_pCpuMemUsage);

    return STATUS_OK;
}

STATUS CMcuMngrMonitor::OnGetIpmiSensorReadingList(CRequest * pGetRequest)
{
    CMcuIpmiSensorReadingListGet * const pGet = OnGetRequest<CMcuIpmiSensorReadingListGet>(pGetRequest);
    pGet->SetCpuMemUsageGetter(m_pCpuMemUsage);

    return STATUS_OK;
}

STATUS CMcuMngrMonitor::OnGetLanPortList(CRequest * pGetRequest)
{
    OnGetRequest<CMcuLanPortListGet>(pGetRequest);

    return STATUS_OK;
}

STATUS CMcuMngrMonitor::OnGetLanPort(CRequest * pGetRequest)
{
    OnGetRequest<CMcuLanPortGet>(pGetRequest);

    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrMonitor::OnTimerLogMemory(CRequest* pGetRequest)
{
    std::string ps_result;
    std::string meminfo;
    BOOL bJitcMode = FALSE;
    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
    sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);
    if (bJitcMode)
        return STATUS_OK;



    STATUS res = SystemPipedCommand("ps",ps_result);

    if (res == STATUS_OK)
    {
        TRACESTR(eLevelInfoNormal) << "*** PROCESS MEMORY REPORT ***\n"
                             << ps_result;
    }

    res = SystemPipedCommand("cat /proc/meminfo",meminfo);

    if (res == STATUS_OK)
    {
        TRACESTR(eLevelInfoNormal) << "*** SYSTEM MEMORY REPORT ***\n"
                               << meminfo;
    }

    StartTimer(MEMORY_REPORT_TIMER,30*60*SECOND); // every 30 minutes

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
namespace
{ 
    #define MAX_TEMP_SHUTDOWN_TIMES     6
    struct TTempThreholdTimes
    {
        unsigned int cpu;
        unsigned int fpga;
        unsigned int dsp1;
        unsigned int dsp2;
        unsigned int dsp3;
    } s_tempTimes = {0, 0, 0, 0, 0};

    unsigned int * FindTempCounter(sensor_t const & elem)
    {
        if(0 == strcmp(elem.SensorName,"CPU Temp"))
        {
            return &(s_tempTimes.cpu);
        }
        else if(0 == strcmp(elem.SensorName,"FPGA Temp"))
        {
            return &(s_tempTimes.fpga);
        }
        else if(0 == strcmp(elem.SensorName,"DSP 1 Temp"))
        {
            return &(s_tempTimes.dsp1);
        }
        else if(0 == strcmp(elem.SensorName,"DSP 2 Temp"))
        {
            return &(s_tempTimes.dsp2);
        }
        else if(0 == strcmp(elem.SensorName,"DSP 3 Temp"))
        {
            return &(s_tempTimes.dsp3);
        }

        return NULL;   
    }

    unsigned int IncreaseTempShutdownCounter(sensor_t const & elem)
    {
        unsigned int * pCurrentCount = FindTempCounter(elem);

        if(NULL == pCurrentCount)
        {  
            return 0;
        }

        (*pCurrentCount)++;

        if((*pCurrentCount) >= MAX_TEMP_SHUTDOWN_TIMES)
        {
            (*pCurrentCount) = 0;
            return MAX_TEMP_SHUTDOWN_TIMES;
        }

        return (*pCurrentCount);
    }

    void ZeroTempShutdownCounter(sensor_t const & elem)
    {
        unsigned int * pCurrentCount = FindTempCounter(elem);

        if(NULL == pCurrentCount)
        {  
            return;
        }

        (*pCurrentCount) = 0;
    }    
}

namespace
{
    #define MAX_SENSOR_READ_NUM 8
    std::vector<std::string> s_sensorRead;
    void AddSensorRead(std::string & szSensorRead)
    {
        s_sensorRead.push_back(szSensorRead);
        if(s_sensorRead.size() > MAX_SENSOR_READ_NUM)
        {
            s_sensorRead.erase (s_sensorRead.begin(), s_sensorRead.begin() + s_sensorRead.size() - MAX_SENSOR_READ_NUM);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrMonitor::OnTimerHardwareMonitoring(CRequest* pGetRequest)
{
    unsigned int counter = 0;

    m_pCpuMemUsage->GetCPUUsage();

    vector<sensor_t> sensors;
    std::string szIpmiSensorLog;
    sensor_read_with_log(sensors, &szIpmiSensorLog);
    AddSensorRead(szIpmiSensorLog);
        
    {
        int const count = sensors.size();
        for (int i=0; i<count; ++i)
        {
            sensor_t const & elem = sensors[i];
            AlarmInfo const info = GetAlarmInfo(elem);
            if (-1==info.base) continue;

            if (ALARM_OFFSET_NORMAL==info.offset)
            {
                if(info.base == AA_TEMPERATURE_MAJOR_PROBLEM && eProductTypeNinja == m_mcuMngrProductType)
                {
                    ZeroTempShutdownCounter(elem);
                }
            }
            else
            {
                SetAlarmReportFlag(info.base, info.offset, elem);
                
                if(info.base == AA_TEMPERATURE_MAJOR_PROBLEM && info.offset >= ALARM_OFFSET_CRITICAL && info.isLow == 0 && false == m_bShutdown && eProductTypeNinja == m_mcuMngrProductType)
                {
                    counter = IncreaseTempShutdownCounter(elem);
                    TRACESTR(eLevelInfoNormal) << "\nIncreaseTempShutdownCounter - SensorName: [" << elem.SensorName << "]  CurrentVal: "<< elem.CurrentVal <<" Counter: " << counter;
                    if(counter >= MAX_TEMP_SHUTDOWN_TIMES) //After temperature reach the critical 6 times continuously (30 seconds), alarm and shutdown the system.
                    {
                        SendShutdownRequest();
                    }
                }
                else if(info.base == AA_TEMPERATURE_MAJOR_PROBLEM && info.offset < ALARM_OFFSET_CRITICAL && info.isLow == 0 && false == m_bShutdown && eProductTypeNinja == m_mcuMngrProductType)
                {
                    ZeroTempShutdownCounter(elem);
                }
            }
        }
    }

    {   //For Hardware Monitor Active Alarm
        HWAlaramEntity * pAlarmActions = NULL;
        unsigned int size = 0;
        bool didAction = false;
        GetAlarmAction(pAlarmActions, size);
        if(NULL != pAlarmActions)
        {
            for(unsigned int i=0; i<size; i++)
            {
                switch(pAlarmActions[i].action)
                {
                    case 1:     //add alarm
                        AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT, pAlarmActions[i].fault, MAJOR_ERROR_LEVEL, pAlarmActions[i].curDesc, true, false);
                        AddActiveAlarmFaultOnlySingleton(FAULT_GENERAL_SUBJECT, pAlarmActions[i].fault, MAJOR_ERROR_LEVEL, pAlarmActions[i].curDesc);
                        didAction = true;
                        break;
                    case 2:     //remove alarm
                        RemoveActiveAlarmByErrorCode(pAlarmActions[i].fault); 
                        didAction = true;
                        break;
                    case 3:     //Re-Add
                        RemoveActiveAlarmByErrorCode(pAlarmActions[i].fault);
                        AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT, pAlarmActions[i].fault, MAJOR_ERROR_LEVEL, pAlarmActions[i].curDesc, true, false);
                        AddActiveAlarmFaultOnlySingleton(FAULT_GENERAL_SUBJECT, pAlarmActions[i].fault, MAJOR_ERROR_LEVEL, pAlarmActions[i].curDesc);
                        didAction = true;
                        break;
                }
            }
        }

        if(didAction)
        {
            PrintSensorRead();
        }

        FlushAlarmFlag();
    }
    
    if (IsPlatformOfNewArch())
    {
        StartTimer(HARDWARE_MONITORING_TIMER, 5*SECOND); // every 5 seconds
    }

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
bool CMcuMngrMonitor::ReadValidateCfgTmpFiles()
{
	CSysConfig sysCnfg;
	sysCnfg.LoadFromFile(eCfgParamUser);
	sysCnfg.LoadFromFile(eCfgParamDebug);
	sysCnfg.LoadFromFile(eCfgCustomParam);
    return true;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrMonitor::SendShutdownRequest()
{
    BOOL isDisableTemperatureControl = NO;

    CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
    if (pSysConfig)
    {
        pSysConfig->GetBOOLDataByKey(CFG_KEY_DISABLE_TEMPERATURE_CONTROL, isDisableTemperatureControl);
    }

    if(isDisableTemperatureControl)
    {
        m_bShutdown = false;
        TRACESTR(eLevelInfoNormal) << "CMcuMngrMonitor::SendShutdownRequest: DISABLE_TEMPERATURE_CONTROL is YES. Skip shut down the system.";
    }
    else
    {
        m_bShutdown = true;
        TRACESTR(eLevelInfoNormal) << "CMcuMngrMonitor::SendShutdownRequest: A temperature sensor reached and passed the upper critical. Shut down the system.";
        AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                    TEMPERATURE_CRITICAL_SHUTDOWN,
                    MAJOR_ERROR_LEVEL,
                    "A temperature sensor reached and passed the upper critical. Shut down the system.",
                    true, true
        );
        StartTimer(SHUTDOWN_TIMER, 1*SECOND);
    }
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrMonitor::OnShutdownRequest(CRequest* pGetRequest)
{
    TRACESTR(eLevelInfoNormal) << "CMcuMngrMonitor::OnShutdownRequest: A temperature sensor reached and passed the upper critical. Shut down the system.";
    system("sudo /sbin/poweroff");

    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrMonitor::PrintSensorRead()
{
    TRACESTR(eLevelInfoNormal) << "CMcuMngrMonitor::PrintSensorRead\n\n*************************\n\n\tIPMI SENSOR READ LOGS\n\n*************************\n\n";
    for(unsigned int i = 0; i < s_sensorRead.size(); i++)
    {
        TRACESTR(eLevelInfoNormal) << "*** IPMITOOL SENSOR LOG "<< i <<" ***\n"
                               << s_sensorRead[i] << "\n*****************************\n";
    }
    TRACESTR(eLevelInfoNormal) << "CMcuMngrMonitor::PrintSensorRead\n\n*******************************\n\n\tEND OF IPMI SENSOR READ LOGS\n\n*******************************\n\n";
}
