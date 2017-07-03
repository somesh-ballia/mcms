// CollectorUnit.cpp

#include "CollectorUnit.h"

#include <limits>
#include <errno.h>
#include <fstream>
#include <stdlib.h>

#include "TaskApp.h"
#include "InterProcessStruct.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "Macros.h"
#include "TraceStream.h"
#include "ManagerApi.h"
#include "Request.h"
#include "StatusesGeneral.h"
#include "InternalProcessStatuses.h"
#include "OsFileIF.h"
#include "CollectorStructs.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "Native.h"
#include "limits.h"
#include "time.h"


#define PROCESS_INFO_SUMMURY_FILE_PATH "ProcessesInfo.txt"

#define LIST_OF_FILES_TO_COLLECTOR_FILE_PATH ((std::string)(MCU_TMP_DIR+"/files_to_collector.txt"))

#define FAULTS_FILE_PATH ((std::string)(MCU_MCMS_DIR + "/Faults/Faults_Collection.txt")) 

#define MAX_LONG std::numeric_limits<time_t>::max()

PBEGIN_MESSAGE_MAP(CCollectorUnit)
  ONEVENT(COLLECT_INFO_REQ, ANYCASE, CCollectorUnit::OnCollectRequest)
  ONEVENT(COLLECT_PROCESSES_INFO_TIMER, ANYCASE, CCollectorUnit::OnCollectProcessesInfoTimer)
  ONEVENT(CDR_XML_READY, ANYCASE, CCollectorUnit::OnCdrXmlReady)
  ONEVENT(CDR_XML_COLLECTION_DONE, ANYCASE, CCollectorUnit::CollectEndIndication)


PEND_MESSAGE_MAP(CCollectorUnit, CAlarmableTask);

void collectorUnitEntryPoint(void* appParam)
{
  CCollectorUnit* collectorUnitTask = new CCollectorUnit;
  collectorUnitTask->Create(*(CSegment*)appParam);
}

CCollectorUnit::CCollectorUnit()
{
  m_pCurrInfoTimeInterval = new CInfoTimeInterval;
  m_processes_count = NUM_OF_PROCESS_TYPES;
  m_pCollectorProcess = dynamic_cast<CCollectorProcess*>(CProcessBase::GetProcess());
  m_pCollectorProcess->SetCollectFilesStarted(FALSE);
  m_isCdrXmlReady = false;
  m_isCdrXmlCollectionDone = true;
}

CCollectorUnit::~CCollectorUnit()
{
  POBJDELETE(m_pCurrInfoTimeInterval);
}

// Virtual
void CCollectorUnit::InitTask()
{
  // Defined in base class
}

void CCollectorUnit::OnCollectRequest(CSegment* pSeg)
{
  m_flist.str("");
  m_flist.clear();

  // 1. Get start and end time from Req.

  pSeg->Get((BYTE*)m_pCurrInfoTimeInterval, sizeof(CInfoTimeInterval));

  // 2. Perform collecting processes' specific info..................

  // prepare empty folder for response
  std::string temp_output;
  std::string cmd = "rm -Rf "+MCU_TMP_DIR+"/Collect LogFiles/*.tgz; mkdir -p "+MCU_TMP_DIR+"/Collect";
  SystemPipedCommand(cmd.c_str(), temp_output);

  m_pCollectorProcess->SetCollectingFileName("");

  CHECK_ABORT;

  if (m_pCurrInfoTimeInterval->GetIsMarkForCollection(eCollectingType_processInfo))
  {
    // StartTimer for processes info
    StartTimer(COLLECT_PROCESSES_INFO_TIMER, SECOND*60);

    // Request processes info
    RequestProcessesInfo();
  }
  else  // no need to wait, since process info don't need to be collected
  {
    OnCollectProcessesInfoTimer(NULL);
  }

  // CollectZipFiles() called when timer COLLECT_PROCESSES_INFO_TIMER ends
  // or collection of processes info is done - and finishes the job.
}

void CCollectorUnit::CollectZipFiles()
{
  STATUS ret = STATUS_OK;
  CHECK_ABORT;

  // 1. Perform collecting files

  DeleteTimer(CDR_XML_COLLECTION_DONE);
  m_isCdrXmlReady = false;
  m_isCdrXmlCollectionDone = true;

  ret = CollectFiles(m_pCurrInfoTimeInterval->GetStartTime(),
                     m_pCurrInfoTimeInterval->GetEndTime());

  CHECK_ABORT;

  if (ret != STATUS_OK)
  {
    // 2. Send collecting failed indication to Manager
    	  ReturnFailResponse();
  }
  else if (m_pCollectorProcess->GetCollectingStatus() == eCollectingStatus_collecting)
  {
    // 3. Send collecting end indication to Manager
	  CollectEndIndication();
  }
}

void CCollectorUnit::CollectEndIndication()
{
	 CHECK_ABORT
	if (m_pCurrInfoTimeInterval->GetIsMarkForCollection(eCollectingType_cdr))
	{

		if(m_isCdrXmlReady)
		{
			CollectFolderContentByTime(LONG_MIN, LONG_MAX, MCU_TMP_DIR+"/CdrXml/", m_flist);
			TRACESTR(eLevelInfoNormal) << "\nCCollectorUnit::CollectEndIndicatio CDR is ready";
			m_isCdrXmlReady = false;
			m_isCdrXmlCollectionDone = true;
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCollectorUnit::CollectEndIndication CDR is not ready starting timer";
			StartTimer(CDR_XML_COLLECTION_DONE, SECOND*5);
			return;
		}
	}

	if(GetCollectedFiles() == STATUS_OK)
	{
		PTRACE(eLevelInfoNormal, "OnTimerCollect succeeded");
		CSegment* pMsg = new CSegment;
		const COsQueue* pMngrMbx =
				CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCollector,
						eManager);
		pMngrMbx->Send(pMsg, COLLECT_INFO_END_IND);
	}
	else
	{
		PTRACE(eLevelInfoNormal, "OnTimerCollect failed call");
		ReturnFailResponse();
	}
}

STATUS CCollectorUnit::GetCollectedFiles()
{

	std::string commandPathName = "(tar cz --files-from ";
        std::string runTarWithSleep = " & while true; do sleep 1; kill -STOP $! 2>/dev/null || break; sleep 1; kill -CONT $! 2>/dev/null || break; done)&";

	// Writes results to file
	 std::string path = LIST_OF_FILES_TO_COLLECTOR_FILE_PATH;
	std::ofstream file(path.c_str(),
			ios_base::out | ios_base::trunc);
	if (file)
	{
		file << m_flist.str();
	}
	else
	{
		PASSERTSTREAM(true,
				"ofstream: " << LIST_OF_FILES_TO_COLLECTOR_FILE_PATH
				<< ": " << strerror(errno) << " (" << errno << ")");
	}

	file.flush();

	// 8. creating and zipping info
	commandPathName += LIST_OF_FILES_TO_COLLECTOR_FILE_PATH;
	commandPathName += m_targetPathName;
	commandPathName += runTarWithSleep;

	TRACEINTOFUNC << "Collect command: " << commandPathName.c_str();

	std::string ans;
	STATUS      stat = SystemPipedCommand(commandPathName.c_str(), ans);
	PASSERTSTREAM(STATUS_OK != stat,
			"SystemPipedCommand : " << commandPathName.c_str()
			<< ": " << ans);

	m_pCollectorProcess->SetCollectFilesStarted(FALSE);

	return stat;
}


STATUS CCollectorUnit::CollectFiles(CStructTm start, CStructTm end)
{
  m_pCollectorProcess->SetCollectFilesStarted(TRUE);
  std::string commandPathName = "tar cz --files-from ";
  //printf("config time :mon :%d , day :%d , hour : %d , min : %d", start.m_mon, start.m_day, start.m_hour, start.m_min);


  char szStartTime[256];
  int Year = start.m_year;
  if (Year < 1900)
    Year += 1900;

  sprintf(szStartTime,
          "%d%2d%2d%2d%2d",
          Year,
          start.m_mon,
          start.m_day,
          start.m_hour,
          start.m_min);

  int len = strlen(szStartTime);
  for (int i = 0; i < len; i++)
  {
    if (szStartTime[i] == ' ')
      szStartTime[i] = '0';
  }

  char szEndTime[256];

  Year = end.m_year;
  if (Year < 1900)
    Year += 1900;

  sprintf(szEndTime,
          "%d%2d%2d%2d%2d",
          Year,
          end.m_mon,
          end.m_day,
          end.m_hour,
          end.m_min);

  len = strlen(szEndTime);
  for (int i = 0; i < len; i++)
  {
    if (szEndTime[i] == ' ')
      szEndTime[i] = '0';
  }

  m_targetPathName = " -f "+MCU_MCMS_DIR+"/";
  std::string targetFileName = "LogFiles/CollectInfo_";
  targetFileName += szStartTime;
  targetFileName += "-";
  targetFileName += szEndTime;
  targetFileName += ".tgz";
  m_targetPathName += targetFileName;
  m_pCollectorProcess->SetCollectingFileName(targetFileName);


  char* oldTZ = getenv("TZ");
  putenv("TZ=UTC");
  tzset();
  time_t start_abs = time_t(start);
  time_t end_abs = time_t(end);



  // *** Restore previous TZ
  if(oldTZ == NULL)
  {
	 putenv("TZ=");
  }
  else
  {
	 char buff[255];
	 memset(buff,sizeof(buff),0);
	 if(strlen(oldTZ) <= sizeof(buff) - 4)
	 {
	 	snprintf(buff,sizeof(buff)-1,"TZ=%s",oldTZ);
	 	putenv(buff);
	 }
	 else
	 	PASSERT(1);
  }
  tzset();


  //std::ostringstream m_flist;

  BOOL isJITCOn = IsJitcMode();

  if (IsHardDiskOk() != FALSE) // collect things from disk only if disk ok, fix 17/01
  {
    // These Items are collected only when system is not under ULTRA_SECURE_MODE (JITC)
    if (isJITCOn == FALSE)
    {
      if (m_pCurrInfoTimeInterval->GetIsMarkForCollection(eCollectingType_logs))
      {
        CollectFolderContentByTime(start_abs, end_abs, "LogFiles/", m_flist);
        CollectFolderContentByTime(start_abs, end_abs, "LocalTracer/", m_flist,TRUE);

        // Collecting startup logs
        CollectFolderContentByTime(start_abs, end_abs, MCU_TMP_DIR+"/startup_logs/", m_flist);

        // Collecting startup logs
         CollectFolderContentByTime(start_abs, end_abs, MCU_TMP_DIR+"/mcu_custom_config/", m_flist);

         // Collecting Versoins.xml
         TRACEINTOFUNC << "Adding Versions.xml" ;
         string versionDir = MCU_TMP_DIR + "/mcms/Versions.xml";
         m_flist << versionDir << std::endl;


        CHECK_ABORT_CLOSE_FILE;
      }

      if (m_pCurrInfoTimeInterval->GetIsMarkForCollection(eCollectingType_cdr))
      {

        CollectFolderContentByTime(start_abs, end_abs, "CdrFiles/", m_flist);
	    m_isCdrXmlCollectionDone = false;
        CreateXmlCdrFiles(start, end);
        //CollectFolderContent(start_abs, end_abs, MCU_TMP_DIR+"/CdrXml/", m_flist);
        CHECK_ABORT_CLOSE_FILE;
      }

      if (m_pCurrInfoTimeInterval->GetIsMarkForCollection(eCollectingType_faults))
      {

    	  SendRequestToPrepareFaultsFile(FAULTS_FILE_PATH,
        								m_flist);

    	  CHECK_ABORT_CLOSE_FILE;
      }

      if (m_pCurrInfoTimeInterval->GetIsMarkForCollection(eCollectingType_audit))
      {
        CollectFolderContentByTime(start_abs, end_abs, "Audit/", m_flist);
        CHECK_ABORT_CLOSE_FILE;
      }

      if (m_pCurrInfoTimeInterval->GetIsMarkForCollection(
            eCollectingType_coreDump))
      {
        CollectFolderContentByTime(start_abs, end_abs, MCU_OUTPUT_DIR+"/core/", m_flist);
        CHECK_ABORT_CLOSE_FILE;
      }

      if (m_pCurrInfoTimeInterval->GetIsMarkForCollection(eCollectingType_network_traffic_capture))
      {
        CollectFolderContentByTime(start_abs, end_abs, MCU_OUTPUT_DIR+"/tcp_dump/", m_flist);
        CHECK_ABORT_CLOSE_FILE;
      }

      eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
      if ((curProductType == eProductTypeSoftMCU || curProductType == eProductTypeSoftMCUMfw || curProductType == eProductTypeEdgeAxis
	  	|| curProductType == eProductTypeGesher|| curProductType == eProductTypeNinja))
	  {
		  if(curProductType == eProductTypeNinja)
		  {
		       CollectFolderContentByTime(start_abs, end_abs, MCU_OUTPUT_DIR+"/diagnostic/", m_flist);
		  	   CHECK_ABORT_CLOSE_FILE;
		  }
	  }

      if (m_pCurrInfoTimeInterval->GetIsMarkForCollection(eCollectingType_participants_recordings))
      {
        if (IsTarget())
        {
          CollectFolderContentByTime(start_abs,
                               end_abs,
                               (MCU_OUTPUT_DIR+"/media_rec/share/").c_str(),
                               m_flist);
        }
        else
        {
          CollectFolderContentByTime(start_abs,
                               end_abs,
                               (MCU_MCMS_DIR+"/MediaRecording/share/").c_str(),
                               m_flist);
        }

        CHECK_ABORT_CLOSE_FILE;
      }

      if (m_pCurrInfoTimeInterval->GetIsMarkForCollection(eCollectingType_cfg))
      {
        CollectFolderContentByTime(0, MAX_LONG, MCU_CONFIG_MCMS_DIR, m_flist);
        CollectFolderContentByTime(0, MAX_LONG, MCU_CONFIG_EMA_DIR, m_flist);
        CollectFolderContentByTime(0, MAX_LONG, MCU_CONFIG_LINKS_DIR, m_flist);
        CHECK_ABORT_CLOSE_FILE;
      }

      // Adds mandatories files
      CollectFolderContentByTime(0, MAX_LONG, MCU_TMP_DIR+"/Collect/", m_flist);
      CHECK_ABORT_CLOSE_FILE;

      std::string files[] = {
        MCU_TMP_DIR+"/mcu_daemon.log",
        MCU_OUTPUT_TMP_DIR+"/mcu_daemon.log"
      };

      for (std::string* name = files; name != ARRAYEND(files); ++name)
      {
         if (!IsFileExists(name->c_str()))
         {
            TRACEINTO << "File not found: " << name->c_str();
            continue;
         }

         m_flist << name->c_str() << std::endl;
         TRACEINTO << "Added " << name->c_str();
      }

      CHECK_ABORT_CLOSE_FILE;
    }

    // NIDS files are available when system is under ULTRA_SECURE_MODE (JITC)
    if (m_pCurrInfoTimeInterval->GetIsMarkForCollection(eCollectingType_nids))
    {
      CollectFolderContentByTime(start_abs, end_abs, MCU_OUTPUT_DIR+"/nids/", m_flist);
      CHECK_ABORT_CLOSE_FILE;
    }
  }


  return STATUS_OK;
}


STATUS CCollectorUnit::RequestProcessesInfo()
{
  STATUS ret_status = STATUS_OK;
  STATUS cur_status;

  m_processes_count = NUM_OF_PROCESS_TYPES;   // reset processes counter

  for (int i = (int) eProcessMcmsDaemon;
       i < NUM_OF_PROCESS_TYPES;
       i++)
  {
    eProcessType type = eProcessType(i);

    CManagerApi api(type);
    cur_status = api.SendMsg(NULL,
                             COLLECTOR_PROCESS_INFO_REQ);

    if (STATUS_OK != cur_status)
    {
      PTRACE2(eLevelError,
              "Failed to send collect request to process: ",
              CProcessBase::GetProcessName(type));
      ret_status = cur_status;
      m_processes_count--;//if process is not even started then no need to include in process counter
    }
  }

  return ret_status;
}

void CCollectorUnit::OnCollectProcessInfoInd(CSegment* pSeg)
{
  TRACECOND_AND_RETURN(!m_processes_count,
      "Error or late process answer to collect info");

  m_processes_count--;

  // get info (as string) from each process and write it to spec. file
  std::string returnedInfo;
  *pSeg >> returnedInfo;

  string processesInfofilePath = PROCESS_INFO_SUMMURY_FILE_PATH;

  FILE* file = fopen(processesInfofilePath.c_str(), "a");
  // Check if the file could be opened
  if (NULL == file)
  {
    PTRACE2(eLevelInfoNormal, "CCollectorUnit::OnCollectProcessInfoInd",
            processesInfofilePath.c_str());
    return;
  }

  ULONG size = returnedInfo.size();
  ULONG bytesIn = fwrite(returnedInfo.c_str(), sizeof(BYTE), size, file);
  ULONG error = ferror(file);

  if (bytesIn != size || error) // check that all data was written and there is no error
  {
    PTRACE2INT(eLevelInfoNormal, "OnCollectProcessInfoInd : cannot write to file.",
               error);
  }

  error = fclose(file);

  if (error)
  {
    PTRACE2INT(eLevelInfoNormal, "OnCollectProcessInfoInd : Cannot close file",
               error);
  }

  if (!m_processes_count)     // collecting info for all processes done.
  {
    // Delete Timer
    DeleteTimer(COLLECT_PROCESSES_INFO_TIMER);

    // call rest of routine ( collect files and zipping )
    CollectZipFiles();
  }
}

void CCollectorUnit::OnCollectProcessesInfoTimer(CSegment* pSeg)
{
  // block further processes info collecting
  m_processes_count = 0;

  // call rest of routine ( collect files and zipping )
  CollectZipFiles();
}

void CCollectorUnit::OnCdrXmlReady(CSegment* pSeg)
{
	TRACEINTOFUNC << "xml is ready ";
	m_isCdrXmlReady = true;

}

void CCollectorUnit::OnCollectEstimateSizeRequest(CSegment* pSeg)
{
  // TODO - in the next phase, add estimate size
}

void CCollectorUnit::SendRequestToPrepareFaultsFile(const std::string& faultFile,
									 std::ostringstream& out) const
{
	DeleteFile(faultFile);

	CSegment *pMsgSeg = new CSegment;

	*pMsgSeg << faultFile;

	CManagerApi api(eProcessFaults);

	OPCODE respOpcode;
    CSegment ret_seg;

    STATUS status = api.SendMessageSync(pMsgSeg, DUMP_FAULTS_FILE_REQ, 4 * SECOND, respOpcode, ret_seg);

    if(STATUS_OK != status || STATUS_OK != respOpcode)
    {
        PASSERTSTREAM(TRUE,
        		"Failed dump faults file, status " << (int)status << " respOpcode " << (int)respOpcode << "\n");

    }
    else
    {
    	out << faultFile << std::endl;
    	TRACEINTOFUNC << "Faults file was created successfully\n";

    }
}


void CCollectorUnit::CollectFolderContentByTime(time_t begin,
                                          time_t end,
                                          const std::string& path,
                                          std::ostringstream& out,
                                          BOOL is_IceLogs) const
{
  std::vector<FDStruct> files;

  // Gets all files in folder
  BOOL res = GetDirectoryContents(path.c_str(), files);
  PASSERTSTREAM_AND_RETURN(!res,
    "GetDirectoryContents: " << path
    << ": " << strerror(errno) << " (" << errno << ")");

  TRACECOND_AND_RETURN(files.empty(),
    "Directory " << path << " doesn't contain files");

  TRACEINTOFUNC << "Directory " << path << "size= "<<files.size();


   /*std::string ans;// = "-0400";


  	STATUS stat = SystemPipedCommand("date +%z", ans, TRUE, FALSE);
  	//CProcessBase *pProcess = CProcessBase::GetProcess();

  if (STATUS_OK == stat)
  {
		//begin += atoi(ans.c_str())*36;
		//end += atoi(ans.c_str())*36;
		//offset = atoi(ans.c_str());

  }*/

  for (std::vector<FDStruct>::const_iterator file = files.begin();
       file != files.end();
       file++)
  {
    // Builds file name
    std::string fname = path + "/" +  file->name;
    TRACEINTOFUNC << "fname =" << fname ;
    // Calls recursive on directory
    if ('D' == file->type)
    {
      // Appends slash to the directory name
      fname += "/";
      CollectFolderContentByTime(begin, end, fname, out);
      continue;
    }
    if(is_IceLogs &&
    		((file->name=="AFEngine0.log") ||(file->name=="AFEngine1.log"))
       )
    {
    	 TRACEINTOFUNC << "Added " << fname ;

    	  out << fname << std::endl;
    }
    else
    {
		// Does not quit on error
		time_t lmt = GetLastModified(fname);

		PASSERTSTREAM(-1 == lmt,
		  "GetLastModified: " << fname << ": "
		  << strerror(errno) << " (" << errno << ")");

		if (lmt > begin && lmt < end)
		{
		  TRACEINTOFUNC << "Added " << fname
						<< ", " << begin << " < " << lmt << " > " << end;
		  out << fname << std::endl;
		}
		else
		{
		  TRACEINTOFUNC << "Passed " << fname << " " << lmt << " out of ("
						<< begin << ", " << end << ")";
		}
    }
    // Quits if the status was changed
    if (m_pCollectorProcess->GetCollectingStatus() != eCollectingStatus_collecting)
    {
    	TRACEINTOFUNC << "eCollectingStatus_collecting ";
      break;
    }
  }
}

void CCollectorUnit::CollectFolderContent(const std::string& path, std::ostringstream& out) const
{
  std::vector<FDStruct> files;

  // Gets all files in folder
  BOOL res = GetDirectoryContents(path.c_str(), files);
  PASSERTSTREAM_AND_RETURN(!res,
    "GetDirectoryContents: " << path
    << ": " << strerror(errno) << " (" << errno << ")");

  TRACECOND_AND_RETURN(files.empty(),
    "Directory " << path << " doesn't contain files");

  TRACEINTOFUNC << "Directory " << path << "size= "<<files.size();

  for (std::vector<FDStruct>::const_iterator file = files.begin();
       file != files.end();
       file++)
  {
    // Builds file name
    std::string fname = path + file->name;
    TRACEINTOFUNC << "fname =" << fname ;
    // Calls recursive on directory
    if ('D' == file->type)
    {
      // Appends slash to the directory name
      fname += "/";
      CollectFolderContent(fname, out);
      continue;
    }

      TRACEINTOFUNC << "Added " << fname;
      out << fname << std::endl;

    // Quits if the status was changed
    if (m_pCollectorProcess->GetCollectingStatus() != eCollectingStatus_collecting)
    {
    	TRACEINTOFUNC << "eCollectingStatus_collecting ";
      break;
    }
  }
}

BOOL CCollectorUnit::IsJitcMode() const
{
  BOOL        bJitcMode = FALSE;
  CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
  sysConfig->GetBOOLDataByKey("ULTRA_SECURE_MODE", bJitcMode);
  return bJitcMode;
}

void CCollectorUnit::CreateXmlCdrFiles(CStructTm start, CStructTm end)
{

	CManagerApi cdrMngrApi(eProcessCDR);
	CSegment* seg = new CSegment;

	//*seg << (WORD)start<< (WORD)end;
	//*seg << (WORD)1<< (WORD)2;
	CStructTmDrv* pStartTimeDrv = (CStructTmDrv*)&start;
	CStructTmDrv* pActualDurationDrv = (CStructTmDrv*)&end;
	pStartTimeDrv->Serialize(NATIVE, *seg);
	pActualDurationDrv->Serialize(NATIVE, *seg);

	STATUS res  = cdrMngrApi.SendMsg(seg, CDR_CREATE_XML_FOLDER);
}


void         CCollectorUnit::ReturnFailResponse()
{
	TRACEINTOFUNC << "CCollectorUnit::ReturnFailResponse ";
	CSegment* pMsg = new CSegment;
	const COsQueue* pMngrMbx =   CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCollector,       eManager);
	pMngrMbx->Send(pMsg, COLLECT_INFO_FAILED_IND);
}
