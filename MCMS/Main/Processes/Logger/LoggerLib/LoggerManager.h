// LoggerManager.h

#ifndef LOGGER_MANAGER_H_
#define LOGGER_MANAGER_H_

#include <string>

#include "TaskApi.h"
#include "TraceClass.h"
#include "XMLFileBuilder.h"
#include "TraceHeader.h"
#include "ManagerTask.h"
#include "LoggerDefines.h"
#include "LoggerProcess.h"
#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/xml/domconfigurator.h"


using namespace log4cxx;

class CEMATrace;
class CTraceTailer;
class CLoggerProcess;
class CLogFileManager;
class CListenSocketApi;
class CManDefinedString;
class CContainerClientLogger;
class CAppenderConfiguration;
class CModuleContent;
class CLog4cxxConfiguration;

class CLoggerManager : public CManagerTask
{
  CLASS_TYPE_1(CLoggerManager, CManagerTask)

public:
  CLoggerManager(void);
  virtual ~CLoggerManager(void);

  virtual const char*    NameOf(void) const;
  virtual void           ManagerPostInitActionsPoint(void);
  virtual void           SelfKill(void);
  virtual int            GetTaskMbxBufferSize(void) const;
  virtual int            GetTaskMbxThreshold(void) const;
  virtual TaskEntryPoint GetMonitorEntryPoint(void);
  virtual void           InformHttpGetFile(const std::string& file_name);

  void                   HandleOutTraceMessage(CSegment* pMsg, DWORD msgLen);
  void                   HandleTraceMessage(CSegment* pMsg, DWORD msgLen);
  void                   HandleSubscribeRequest(CSegment* pSeg);
  void                   HandleUnSubscribeRequest(CSegment* pSeg);
  void                   HandleFlush(CSegment* pSeg);
  void                   HandleLoggerDisconnect(CSegment* pSeg);
  void                   HandleLoggerConnection(CSegment* pSeg);
  void                   OnTimerFileSystemWarningTest(void);
  STATUS                 OnEmaTrace(CRequest* pRequest);
  STATUS                 OnEmaFlush(CRequest* pRequest);
  bool                   IsHighCPU(void) const;
  void                   OnUDPAppenderTimer();
  void                   OnMsgAvgCount();
  void                   OnLicensingInd(CSegment* pSeg);
  void                   OnUpdateCfgWriteAlternativeTout(CSegment* pSeg);

protected:
  virtual int            GetMaxLegitimateUsagePrecents(){return 80;}

private:
  static void            GenerateTitle(const COMMON_HEADER_S& commonHeader,
                                       const TRACE_HEADER_S& traceHeader,
                                       std::string& out);
  static STATUS          ManSetLevel(std::ostream& ans);

  STATUS HandleTerminalTail(CTerminalCommand& cmd, std::ostream& ans);
  STATUS HandleTerminalSetTail(CTerminalCommand& cmd, std::ostream& ans);
  STATUS HandleTerminalFlush(CTerminalCommand& cmd, std::ostream& ans);
  STATUS HandleTerminalSocket(CTerminalCommand& cmd, std::ostream& ans);
  STATUS HandleTerminalClearStat(CTerminalCommand& cmd, std::ostream& ans);
  STATUS HandleTerminalStopFileSystem(CTerminalCommand& cmd, std::ostream& ans);
  STATUS HandleTerminalAddAppender(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleTerminalShowLog4cxxSwitch(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleTerminalClearAllUDPAppenders(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleTerminalShowAllUDPAppenders(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleTerminalEnableLog4cxxSwitch(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleTerminalDisableLog4cxxSwitch(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleTerminalStopLogging(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleTerminalStartLogging(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleTerminalLoggingStatus(CTerminalCommand& command, std::ostream& answer);
  STATUS HandleTerminalSetLevel(CTerminalCommand& cmd, std::ostream& ans);
  STATUS HandleTerminalGetLogStatic(CTerminalCommand& cmd,std::ostream& ans);

  void   Stoplogging();
  void   Startlogging();
  eLogLevel            GetTraceLevelByLoad(unsigned int load,
                                             unsigned int threshold) const;
  virtual unsigned int   GetMaxLegitimateUsagePrecents(void) const;
  virtual void           AddFilterOpcodePoint(void);
  void                   OnDropMessageFlagOnTimer(CSegment* msg);
  void                   OnSystemLoadAverage(CSegment* msg);
  void                   DispatchTrace(const char* msg);
  bool                   BuildMcmsMessage(CSegment* pSeg, std::ostringstream& buf);
  bool                   BuildOutMessage(CSegment* pSeg, std::ostringstream& buf);
  void                   BuildEmaMessage(CEMATrace* trace, std::ostringstream& buf);
  void                   BuildGenericMessage(std::ostringstream& buf,
                                             const COMMON_HEADER_S&
                                             commonHeader,
                                             const PHYSICAL_INFO_HEADER_S&
                                             physicalHeader,
                                             TRACE_HEADER_S& traceHeader,
                                             const MCMS_TRACE_HEADER_S&
                                             mcmsHeader,
                                             const char* content);

  void                   FillBuffer(std::ostringstream& buf,
                                    const COMMON_HEADER_S& commonHeader,
                                    const PHYSICAL_INFO_HEADER_S&
                                    physicalHeader,
                                    const TRACE_HEADER_S& traceHeader,
                                    const MCMS_TRACE_HEADER_S& mcmsHeader,
                                    const char* content);
  STATUS 				 SendMinimumConfiguredLevelToMFA(CLog4cxxConfiguration* logCondig, eLogLevel system_level);
  void 				 	 CheckFullValidityForUDPAppender(CLog4cxxConfiguration* logCondig,STATUS &status);
  bool                   IsTraceLengthOver(char* content, char* header, DWORD& totalLen);
  void                   PrintCommandAnswerToTerminal(const char* terminalName,
                                                      const char* answer) const;
  STATUS                 GenericFlush(void);
  void                   StopFileSystem(void);
  void                   TraceToFile(const char* message);
  STATUS                 UpdateLogLevel(eLogLevel new_level,
                                          eLogLevel& old_level);

  BOOL                   CheckIfNewHeaderIsNeeded(const COMMON_HEADER_S& commonHeader,
                                                  const PHYSICAL_INFO_HEADER_S&
                                                  physicalHeader,
                                                  const TRACE_HEADER_S&
                                                  traceHeader,
                                                  const MCMS_TRACE_HEADER_S&
                                                  mcmsHeader);

  bool            GetLog4cxxLogLevelAccordingRMXLogLevel(int rmx_log_level, int& log4cxx_log_level);
  void            InitLog4cxxConfiguration();
  STATUS          OnServerSetLoggerConfiguration(CRequest* pRequest);
  STATUS 		  CheckDiskSpaceForMaxLogSize(CLog4cxxConfiguration &logconfig,bool updateMaxLogLevel=false);
  STATUS 		  OnServerStartCSLogs(CRequest* pRequest);
  STATUS 		  OnServerStopCSLogs(CRequest* pRequest);
  CModuleContent* GetModulePtrByProcessByMainEntity(CAppenderConfiguration* pAppender, eMainEntities mainEntityType);
  DWORD 		  GetMinimumLogLevel (CAppenderConfiguration *appender_config,DWORD currentMinimumLevel);
  int             IsThisMsgNeedToBeSentToLoggerType(int log_level,
                                                      int src_id,
                                                      int processType,
                                                      CAppenderConfiguration *appenderConfig);
  void            ApplyUDPAppenderConfiguration();
  void 			  ApplySysLogAppenderConfiguration();
  STATUS          HandleTerminalTestUDPConfiguration(CTerminalCommand& cmd, std::ostream& ans);
  void            DisableUDPAppender();
  void   		  DisableSysLogAppender();
  void            ApplyAppendersConfiguration();
  void            SendToAppenders(unsigned int src_id,
                                     unsigned int log_level,
                                     unsigned int process_type,
                                     const char* buf,
                                     LoggerPtr logger,
                                     CAppenderConfiguration *appenderConfig);
  string 		 GetProcessName(unsigned int processType);

  void UpdateCSLogTraceFlagsInSystemCfg(std::string level);

  void OnSetLoggerLevelExplicit(CSegment* pSeg);
  void UpdateExplicitLogLevel(eLogLevel new_level);

  CContainerClientLogger* m_ContainerClientLogger;
  CLogFileManager*        m_FileService;
  CTraceTailer*           m_TraceTailer;

  CListenSocketApi* m_pListenSocketApi;
  CTaskApi          m_PipeToLoggerTaskApi;

  BOOL            m_fileSystemWarning;
  CLoggerProcess* m_proc;

  COMMON_HEADER_S        m_prev_commonHeader;
  PHYSICAL_INFO_HEADER_S m_prev_physicalHeader;
  TRACE_HEADER_S         m_prev_traceHeader;
  MCMS_TRACE_HEADER_S    m_prev_mcmsHeader;
  std::string            m_title;

  XMLFileBuilder m_xml_builder;
  bool           m_enable_logging;
  bool           m_enable_update_level;
  ULONGLONG      m_udpAppenderStopTime;
  ULONGLONG      m_iMsgAvgTimerCount;
  int            m_logLevelMapping[CLoggerProcess::eLevelNum][4];
  BOOL	m_bStartLogStaticTimer;
  STATUS  m_bIsMfaUpdated;
  bool			m_cfgWriteToAlternativeFileEnabled;
  PDECLAR_MESSAGE_MAP
  PDECLAR_TRANSACTION_FACTORY
  PDECLAR_TERMINAL_COMMANDS

  DISALLOW_COPY_AND_ASSIGN(CLoggerManager);
};

#endif  // LOGGER_MANAGER_H_
