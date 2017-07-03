// LoggerProcess.h

#ifndef LOGGER_PROCESS_H_
#define LOGGER_PROCESS_H_

#include <vector>
#include "ProcessBase.h"
#include "LoggerFileList.h"
#include "LoggerSocketStatus.h"

#define		MCMS_MODULE_NAME		"Mcms"
#define 		EMA_MODULE_NAME			"EMA"
#define		MEDIACARD_MODULE_NAME 	"MediaCard"

class CTraceStatistics;
class CLog4cxxConfiguration;

class CLoggerProcess : public CProcessBase
{
CLASS_TYPE_1(CLoggerProcess, CProcessBase)
public:
	friend class CTestLoggerProcess;

	enum eSender
	{
	    eSenderOut,
	    eSenderMCMS,
	    eSenderEMA,

	    eSenderQuantity
	};
	enum eUniteLogLevel
	{
		eTRACE=0,
		eDEBUG,
		eINFO,
		eWARN,
		eERROR,
		eFATAL,
		eOFF,
		eLevelNum
		
	};

	enum eRMXALLProcesses
	{
		eInavlidProcess=0,
		eMcmsDaemon=1,
		eConfigurator,
		eLogger,
		eAuditor,
		eFaults,
		eIPMCInterface,
		eMcmsNetwork,
		eSNMP,
		eCSMngr,
		eCertMngr,
		eMcuMngr,
		eLicenseServer,
		eConfParty,
		eCards,
		eCDR,
		eResource,
		eSipProxy,
		eDNSAgent,    
		eGatekeeper,
		eQAAPI,
		eExchangeModule,
		eEncryptionKeyServer,
		eAuthentication,	
		eInstaller,
		eRtmIsdnMngr,
		eBackupRestore,
		eMplApi,
		eCSApi,
		eCollector,
		eSystemMonitoring,
		eMediaMngr,
		eFailover,
		eLdapModule,
		eMCCFMngr,
		eNotificationMngr,
		eIce,
		eApacheModule,
		eGideonSim,
		eEndpointsSim,
		eDemo,
		eTestClient,
		eMcuCmd,
		eClientLogger,
		eDiagnostics,
		eUtility,
		eEMA,
		eOnlyMplProcess,
		eMfaCardManager,
		eSwitchCardManager,
		eVideoDsp,
		eArtDsp,
		eEmbeddedApacheModule,
		eIceManager,
		eAMP,
		eVMP,
		eMPProxy,
		eCsModule,
		eMfaLauncher,
		eMediaCardNA,
		
	};

	enum eSendDuration
	{
		e1Hour,
		e2Hour,
		e4Hour,
		e6Hour,
		e8Hour,
		e12Hour,
		e16Hour,
		e24Hour,
		e48Hour
		
	};
	
	CLog4cxxConfiguration* GetLog4cxxConfiguration() const;
	static const char* SenderToStr(eSender sender);
	static const char* GetProcessNameByMainEntity(const eMainEntities mainEntityType,
    											  const DWORD processType);
	static int	GetProcessIndexByMainEntity(const eMainEntities mainEntityType,
											const DWORD processType);

	CLoggerProcess(void);
	virtual ~CLoggerProcess(void);
	virtual const char* NameOf(void) const;

	virtual eProcessType GetProcessType(void);
	virtual TaskEntryPoint GetManagerEntryPoint(void);
	virtual BOOL UsingSockets(void);
	virtual void DumpProcessStatistics(std::ostream& answer,
                                       CTerminalCommand& command) const;
	virtual int GetProcessAddressSpace(void);
	virtual DWORD GetMaxTimeForIdle(void) const;

	void SetSocketStatus(eSocketStatus status) { m_SocketStatus = status; }
	eSocketStatus GetSocketStatus(void) const { return m_SocketStatus; }

	void AddTraceToStatistics(const eMainEntities source, const DWORD processType, const DWORD size);
	void ResetStatistics(void);
	void IncrementCntNotSentTrace(void) { m_CntNotSentTrace++; }

	CLoggerFileList* GetLoggerFileList(void) const { return m_loggerFileList; }

	bool GetDropMessageFlagAndIncrease(const eSender sender);
	bool GetDropMessageFlag(void) const;
	void SetDropMessageFlag(const bool val);
	std::vector<unsigned long long> GetDropMessageStat(void) const;
	ULONGLONG	GetNumOfAllTraces() const;
	WORD GetMaxNumberOfFiles() const;
	const LOGGER_LICENSING_S* GetLicensingData();
	void  SetLicensingData(LOGGER_LICENSING_S& licensingData);

private:
	virtual void AddExtraStringsToMap();
	virtual void AddExtraStatusesStrings();
	void AddLog4cxxLogLevelStringsToMap() const;
	void AddAllRMXProcessesToMap() const;

	bool m_IsDropMessage;
	DWORD m_CntNotSentTrace;
	eSocketStatus m_SocketStatus;
	CTraceStatistics* m_pTraceStatistics;
	CLoggerFileList* m_loggerFileList;
	std::vector<unsigned long long> m_CntDroppedMessages;
	CLog4cxxConfiguration*	m_pLog4cxxConfiguration;
	LOGGER_LICENSING_S*		m_pLicensing;
};

#endif  // LOGGER_PROCESS_H_
