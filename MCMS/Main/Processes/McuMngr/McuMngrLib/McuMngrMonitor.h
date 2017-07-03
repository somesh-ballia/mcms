// McuMngrMonitor.h: interface for the CMcuMngrMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CMcuMngrMONITOR__)
#define _CMcuMngrMONITOR__


#include "MonitorTask.h"
#include "PrecedenceSettings.h"
//#include "Macros.h"

//class CRequest;
class CIPService;
class CSysState;


class CMcuMngrMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CMcuMngrMonitor();
	virtual ~CMcuMngrMonitor();

	virtual const char* NameOf() const { return "CMcuMngrMonitor";}


	STATUS OnGetMngmntNetwork(CRequest* pGetRequest);
	STATUS OnGetMcuState(CRequest* pGetRequest);
	STATUS OnGetActiveAlarmsList(CRequest* pGetRequest);
	STATUS OnGetMcuTime(CRequest* pGetRequest);
	STATUS OnGetCfs(CRequest* pGetRequest);
	STATUS OnGetEthernetSettingsConfigList(CRequest* pGetRequest);

    STATUS OnGetIpmiEntityList(CRequest * pGetRequest);
    STATUS OnGetIpmiEntityFanInfo(CRequest * pGetRequest);
    STATUS OnGetIpmiEntityFanLevel(CRequest * pGetRequest);
    STATUS OnGetIpmiEntityEventLog(CRequest * pGetRequest);
    STATUS OnGetIpmiFru(CRequest * pGetRequest);
    STATUS OnGetIpmiSensorList(CRequest * pGetRequest);
    STATUS OnGetIpmiSensorReadingList(CRequest * pGetRequest);
    STATUS OnGetLanPortList(CRequest * pGetRequest);
    STATUS OnGetLanPort(CRequest * pGetRequest);

	STATUS OnTimerLogMemory(CRequest* pGetRequest);
    STATUS OnTimerHardwareMonitoring(CRequest* pGetRequest);
    void SendShutdownRequest();
    STATUS OnShutdownRequest(CRequest* pGetRequest);
    bool IsPlatformOfNewArch() const;
	void  InitTask();


protected:

	STATUS SendMngmntNetworkParamsToMcuMngrManager(CIPService* pNetworkParams);

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY


private:
	STATUS HandleGetCfg(CRequest *pGetRequest);
	STATUS HandleGetPrecedenceSettings(CRequest* pGetRequest);
	void OnRequestPrecedenceSettings(CSegment* pMsg);
	bool ReadValidateCfgTmpFiles();
	void PrintSensorRead();

    CSysState *m_pCpuMemUsage;
    eProductType m_mcuMngrProductType;
    pthread_t m_sensorCacheTid;
    int m_bQuit;
    bool m_bShutdown;
};

#endif // !defined(_CMcuMngrMONITOR__)

