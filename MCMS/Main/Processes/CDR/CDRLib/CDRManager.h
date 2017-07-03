// CDRManager.h

#ifndef CDR_MANAGER_H_
#define CDR_MANAGER_H_

#include "ManagerTask.h"
#include "Macros.h"
#include "PersistItem.h"
#include "PersistenceList.h"
#include "PersistenceQueue.h"
#include "CDRSettings.h"
#include "CdrRegistrationApiDefines.h"
#include "PlcmCdrEventSource.h"
#include "PlcmCdrEventSourceId.h"
#include "CdrPersistApiEnums.h"
#include "EventData.h"
#include "PlcmCdrEvent.h"
#include "McmsAuthentication.h"
#include "PlcmCdrEventList.h"
#include "PlcmCdrEventConfRecord.h"
#include "PlcmCdrEventConfEnd.h"
#include "PlcmCdrEventConfDataUpdate.h"

class CCdrLog;
class CCDRProcess;
class CurlHTTP;




void CDRManagerEntryPoint(void* appParam);

class CCDRManager : public CManagerTask
{
  CLASS_TYPE_1(CCDRManager, CManagerTask)
public:
  CCDRManager();
  virtual ~CCDRManager();

  virtual const char* NameOf() const;

  virtual void        InformHttpGetFile(const std::string& file_name);
  virtual int         GetTaskMbxBufferSize() const;
  TaskEntryPoint      GetMonitorEntryPoint();
  void                OnTimerFileSystemWarningTest();
  static void* 		  StartSendingCdrToServer(void *arg);
  static void 		  SetTimer(struct timespec* abstime);
  void SelfKill();
private:
  virtual void        ManagerPostInitActionsPoint();
  STATUS              HandleCDRStartConf(CSegment* pSeg);
  STATUS 			  HandleSetCDRSettings(CRequest *pRequest);
  STATUS              HandleCDREvent(CSegment* pSeg);
  STATUS              HandleCDREndConf(CSegment* pSeg);
  STATUS              HandleGetLastConfId(CSegment* pSeg);
  STATUS              HandleHandleCDRRetrievedNotify(CSegment* pSeg);
  STATUS              HandleTerminalListCDR(CTerminalCommand& command,
                                            std::ostream& answer);
  STATUS              HandleTerminalGetLastConfId(CTerminalCommand& command,
                                                  std::ostream& answer);
  STATUS	   		  HandleTerminalTestPersistence(CTerminalCommand& command,
  											  std::ostream& answer);
  STATUS 			  HandleTerminalTestPersistencePopAfterPeek(CTerminalCommand& command,
            std::ostream& answer);

  STATUS HandleTerminalTestPersistenceAddItem(CTerminalCommand& command,
		  std::ostream& answer);
  STATUS HandleTerminalTestPersistenceAddMultipleItemsFromFile(CTerminalCommand& command,
 		  std::ostream& answer);
  STATUS HandleTerminalTestPersistenceAddMultipleItems(CTerminalCommand& command,
   		  std::ostream& answer);
  STATUS HandleTerminalTestXml2source(CTerminalCommand& command,
     		  std::ostream& answer);

  STATUS HandleTerminalTestPersistenceEnable(CTerminalCommand& command,
   		  std::ostream& answer);
  STATUS HandleTerminalTestPersistenceDisable(CTerminalCommand& command,
     		  std::ostream& answer);
  STATUS HandleTerminalTestPersistencePeekAndPopAll(CTerminalCommand& command,
       		  std::ostream& answer);

  STATUS HandleTerminalTestPersistenceCreateThread(CTerminalCommand& command,
       		  std::ostream& answer);

  STATUS HandleTerminalTestPersistenceAddItemSynced(CTerminalCommand& command,
           std::ostream& answer);
  STATUS HandleTerminalTestPersistenceAddMultipleItemsSynced(CTerminalCommand& command,
  		std::ostream& answer);
  STATUS HandleTerminalEnableReigsterHttps(CTerminalCommand& command,
  		std::ostream& answer);
  STATUS HandleTerminalReadNumOfErrorsFromServer(CTerminalCommand& command,
    		std::ostream& answer);


  STATUS HandlePersistenceDisable();
  STATUS HandlePersistenceEnable();
  static bool TrySendEventToServer(PlcmCdrEvent* itemFromfile);
  static bool SendEventToServer(std::string msgToServer);
  static std::string GetGmtTime();
  static std::string CreateMsgToServer(PlcmCdrEvent* itemFromfile);
  void EnableActiveAlarmCdrServerConeectionDown(CSegment * pSeg);
  void DisableActiveAlarmCdrServerConeectionDown(CSegment * pSeg);



  void                FormatTraceMessage(const string& location,
                                         const string& message,
                                         STATUS status);
  STATUS              IsCanPerformHDOperation() const;
  void 				  CreateConvertXmlTask();
  STATUS 			  HandleCDRCreateXmlFolder(CSegment* pSeg);
  STATUS 			  HandleAbortCreateXmlFolder();
  void  			  AskMcuMngrForParams();
  void 				  OnMcuMngrCdrParamsInd(CSegment* pSeg);
  std::string 		  RegisterToCdrServer(CCDRSettings* pCdrNew);
  void				  SaveCdrSettingsToFile(CCDRSettings* pCdr);
  const char* 			  GetModel(CCDRProcess* pProcess);
  void				  CreateCdrCommunicationThread();
  STATUS 			  HandlePersistenceAddCdr(CSegment* pSeg);
  void 				  EnableDisalbeRemoteCdrTimer(BOOL isEnable);
  void 				  EnableSystemMonitoringTimerIfAlive();
  bool 				  IsSystemMonitoringUp();
  STATUS 			  IsRemoteCdrEnable(CSegment* msg);

  CCDRProcess* m_CDRProcess;
  bool         m_IsHardDiskOk;
  BOOL         m_fileSystemWarning;
  CTaskApi*    m_pConvertXmlMngApi;
  MCMS_INFO_S  m_McmsInfoStruct;
  std::string  m_serialNum;
  std::string  m_ipv4;
  std::string  m_ipv6;
  std::string  m_name;
  std::string  m_hardwareVersion;
  CPersistenceQueue* m_psQDebug;
  CPersistenceQueue* m_persistenceQueue;
  bool m_isHttpsEnabled;

  PDECLAR_MESSAGE_MAP
  PDECLAR_TRANSACTION_FACTORY
  PDECLAR_TERMINAL_COMMANDS
  DISALLOW_COPY_AND_ASSIGN(CCDRManager);
};

#endif

