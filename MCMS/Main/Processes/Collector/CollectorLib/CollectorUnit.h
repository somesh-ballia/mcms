// CollectorUnit.h

#ifndef COLLECTOR_UNIT_H_
#define COLLECTOR_UNIT_H_

#include "CollectorProcess.h"
#include "CollectorInfo.h"
#include "ProcessBase.h"
#include "StateMachine.h"
#include "TaskApp.h"
#include "TaskApi.h"
#include "SingleToneApi.h"
#include "AlarmableTask.h"
#include <sstream>


class CInfoTimeInterval;

extern "C" void collectorUnitEntryPoint(void* appParam);

#define COLLECTOR_FILES_TO_BRING  20


#define CHECK_ABORT \
  if (m_pCollectorProcess->GetCollectingStatus() == eCollectingStatus_aborting) \
  { \
    TRACESTR(eLevelInfoNormal) << "CHECK_ABORT - working"; \
    m_pCollectorProcess->HandleAbort(); \
    m_pCollectorProcess->SetCollectFilesStarted(FALSE); \
    m_isCdrXmlReady = false; \
    m_isCdrXmlCollectionDone = true;	\
    return; \
  } \
  else if (m_pCollectorProcess->GetCollectingStatus() == \
           eCollectingStatus_ready) {                                                                                 \
    TRACESTR(eLevelInfoNormal) << "CHECK_ABORT - collecting was already over";}

#define CHECK_ABORT_CLOSE_FILE \
  if (m_pCollectorProcess->GetCollectingStatus() == eCollectingStatus_aborting) \
  { \
    TRACEINTOFUNC << "CHECK_ABORT_CLOSE_FILE - working"; \
    m_pCollectorProcess->HandleAbort(); \
    m_pCollectorProcess->SetCollectFilesStarted(FALSE); \
    m_isCdrXmlReady = false; \
    m_isCdrXmlCollectionDone = true;	\
    return STATUS_FAIL; \
  } \
  else if (m_pCollectorProcess->GetCollectingStatus() == \
           eCollectingStatus_ready) \
  { \
    TRACEINTOFUNC << "CHECK_ABORT - collecting was already over"; \
  }

class CCollectorProcess;
class CCollectorUnit : public CAlarmableTask
{
  CLASS_TYPE_1(CCollectorUnit, CAlarmableTask)

public:
  CCollectorUnit();
  virtual ~CCollectorUnit();

  BOOL         IsSingleton() const {return YES;}
  const char*  GetTaskName() const {return "CollectorUnit";}
  virtual void InitTask();
  void         OnCollectRequest(CSegment* pSeg); // from Collector Manager Task
  void         OnCollectEstimateSizeRequest(CSegment* pSeg);

  STATUS       CollectFiles(CStructTm start, CStructTm end);
  STATUS       RequestProcessesInfo();
  void         OnCollectProcessInfoInd(CSegment* pSeg);
  void         OnCollectProcessesInfoTimer(CSegment* pSeg);
  void 		   OnCdrXmlReady(CSegment* pSeg);
  STATUS 	   AddAllCdrToXmlDirector();
  STATUS 	   AddCdrToXml(CSerializeObject* pResponse, string fileName);
  void         CollectZipFiles();
  BOOL         IsJitcMode() const;
  void 		   CreateXmlCdrFiles(CStructTm start, CStructTm end);  
  STATUS 	   GetCollectedFiles();  
  void 	   CollectEndIndication();

private:


  void 			SendRequestToPrepareFaultsFile(const std::string& faultFile,
  									 std::ostringstream& out) const;


  void         CollectFolderContentByTime(time_t start_abs,
                                    time_t end_abs,
                                    const std::string& path,
                                    std::ostringstream& out,
                                    BOOL is_IceLogs=FALSE) const;
  void 		   CollectFolderContent(const std::string& path,
				  std::ostringstream& out) const;
  void         ReturnFailResponse();

  WORD               m_processes_count;
  CInfoTimeInterval* m_pCurrInfoTimeInterval;
  CCollectorProcess* m_pCollectorProcess;
  std::ostringstream m_flist;
  std::string 		 m_targetPathName;
  BOOL				m_isCdrXmlReady;
  BOOL				m_isCdrXmlCollectionDone;
  

  PDECLAR_MESSAGE_MAP;
};

#endif
