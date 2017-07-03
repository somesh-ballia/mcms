// AuditorManager.cpp: implementation of the CAuditorManager class.
//
//////////////////////////////////////////////////////////////////////



#include "AuditorManager.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "AuditorApi.h"
#include "TraceStream.h"
#include "ListenSocketApi.h"
#include "AuditorRxSocket.h"
#include "psosxml.h"
#include "AuditFileManager.h"
#include "FaultsDefines.h"
#include "AuditEventContainer.h"
#include "AuditorProcess.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"



#define AUDIT_FILE_WARNING_TIMEOUT     1000  // (10 seconds)
#define AUDIT_FILE_WARNING_IDLE_TIMEOUT 15*60*100 // (15 minutes)



////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CAuditorManager)
    ONEVENT( AUDIT_EVENT_MCMS                   ,ANYCASE  ,  CAuditorManager::HandleAuditEventMcms )
    ONEVENT( AUDIT_EVENT_OUTSIDER               ,ANYCASE  ,  CAuditorManager::HandleAuditEventOutsider )
    ONEVENT( CLOSE_SOCKET_CONNECTION            ,ANYCASE  ,  CAuditorManager::HandleCloseConnection )
    ONEVENT( OPEN_SOCKET_CONNECTION             ,ANYCASE  ,  CAuditorManager::HandleOpenConnection )
    ONEVENT( FLUSH_BUFFER_REQ                   ,ANYCASE  ,  CAuditorManager::HandleFlushBufferRequest )
    ONEVENT( TIMER_MID_NIGHT                    ,ANYCASE  ,  CAuditorManager::HandleMidNightTimer )
    ONEVENT( AUDIT_FILE_SYSTEM_WARNING_TIMER	,ANYCASE  ,  CAuditorManager::OnTimerFileSystemWarningTest )
PEND_MESSAGE_MAP(CAuditorManager,CManagerTask);


////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CAuditorManager)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CAuditorManager::HandleOperLogin)
END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CAuditorManager)
//  ONCOMMAND("ping",CAuditorManager::HandleTerminalPing,"test terminal commands")
    ONCOMMAND("dump_files",CAuditorManager::HandleTerminalDumpFiles,"Dump file list")
    ONCOMMAND("dump_events",CAuditorManager::HandleTerminalDumpEvents,"Dump event list")
    ONCOMMAND("flush",CAuditorManager::HandleTerminalFlush,"Flush buffer file to EMA file")
    ONCOMMAND("next",CAuditorManager::HandleTerminalNext,"Flush buffer and skip to next file")
END_TERMINAL_COMMANDS

extern void AuditorMonitorEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void AuditorManagerEntryPoint(void* appParam)
{
	CAuditorManager * pAuditorManager = new CAuditorManager;
	pAuditorManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CAuditorManager::GetMonitorEntryPoint()
{
	return AuditorMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CAuditorManager::CAuditorManager()
{
    m_pListenSocketApi = NULL;
    m_pRcvMbx = NULL;
    m_pAuditFileManager = NULL;
    m_fileSystemWarning = FALSE;
    

}

//////////////////////////////////////////////////////////////////////
CAuditorManager::~CAuditorManager()
{
    PDELETE(m_pRcvMbx);
    PDELETE(m_pListenSocketApi);
    PDELETE(m_pAuditFileManager);
}

//////////////////////////////////////////////////////////////////////
void CAuditorManager::ManagerPostInitActionsPoint()
{
    pProcess = dynamic_cast<CAuditorProcess*>(CProcessBase::GetProcess());

    m_pAuditFileManager = new CAuditFileManager;
    m_pAuditFileManager->Init();

    // creation of socket interface
    OpenSocketInterface();

    DWORD timeUntilMidNight = GetTickNumUntilMidNight();
    StartTimer(TIMER_MID_NIGHT, timeUntilMidNight);
    StartTimer(AUDIT_FILE_SYSTEM_WARNING_TIMER, AUDIT_FILE_WARNING_TIMEOUT);
    
}

////////////////////////////////////////////////////////////////////////////
void CAuditorManager::OpenSocketInterface()
{
	const char* MplApiIp = "0.0.0.0";
	if (IsTarget())
	{
		switch (CProcessBase::GetProcess()->GetProductFamily())
		{
		case eProductFamilyRMX:

			MplApiIp = "169.254.128.10";
			break;

		case eProductFamilyCallGenerator:
			MplApiIp = "127.0.0.1";
			break;

		default:
			PASSERTSTREAM(true, "Illegal product family " << CProcessBase::GetProcess()->GetProductFamily());
		} // switch
	}
	else
		if(eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
			MplApiIp = "127.0.0.1";

    m_pListenSocketApi = new CListenSocketApi(AuditorSocketRxEntryPoint,
                                              NULL,
                                              10010,
                                              MplApiIp);

    m_pListenSocketApi->SetCreateConnectionMode(eRxConnection);
    m_pListenSocketApi->SetMaxNumConnections(1);

    m_pRcvMbx = new COsQueue;

    m_pListenSocketApi->Create(GetRcvMbx());
}

////////////////////////////////////////////////////////////////////////////
void CAuditorManager::SelfKill()
{
    m_pListenSocketApi->Destroy();

    if(NULL != m_pAuditFileManager) // file system maybe disabled
    {
        m_pAuditFileManager->Shutdown();
    }

	CManagerTask::SelfKill();
}

//////////////////////////////////////////////////////////////////////
DWORD CAuditorManager::GetTickNumUntilMidNight()const
{
    CStructTm now;
    SystemGetTime(now);

    CStructTm midNight(now);
    midNight.m_hour = 24;
    midNight.m_min = 0;
    midNight.m_sec = 0;

    CStructTm timeDelta = midNight.GetTimeDelta(now);

    DWORD timeinterval = 60 * 60 * timeDelta.m_hour * 100;
    timeinterval += 60 * timeDelta.m_min * 100;
    timeinterval += timeDelta.m_sec * 100;

    TRACEINTO << "\nCAuditorManager::GetTickNumUntilMidNight - now : " << now
              << ", delta : " << timeDelta << "\n"
              << "sum : " << now + timeDelta;

    return timeinterval;
}

//////////////////////////////////////////////////////////////////////
void CAuditorManager::HandleAuditEventMcms(CSegment *pSeg)
{
    OnAuditIncomingEvent(pSeg);
}

//////////////////////////////////////////////////////////////////////
void CAuditorManager::HandleAuditEventOutsider(CSegment *pSeg)
{
    OnAuditIncomingEvent(pSeg);
}

//////////////////////////////////////////////////////////////////////
void CAuditorManager::HandleCloseConnection(CSegment *pSeg)
{
    PTRACE(eLevelInfoNormal, "HandleCloseConnection");
}

//////////////////////////////////////////////////////////////////////
void CAuditorManager::HandleOpenConnection(CSegment *pSeg)
{
    PTRACE(eLevelInfoNormal, "HandleOpenConnection");
}

//////////////////////////////////////////////////////////////////////
void CAuditorManager::HandleFlushBufferRequest(CSegment *pSeg)
{
    STATUS status = STATUS_OK;
    if(NULL == m_pAuditFileManager) // file system was disabled
    {
        status = FLUSH_BUFFER_IND_FAIL;
    }
    else
    {
        status = m_pAuditFileManager->Flush();
    }
    OPCODE respOpcode = (STATUS_OK == status
                         ?
                         FLUSH_BUFFER_IND_OK : FLUSH_BUFFER_IND_FAIL);
    ResponedClientRequest(respOpcode, NULL);
}

//////////////////////////////////////////////////////////////////////
void CAuditorManager::HandleMidNightTimer(CSegment *pSeg)
{
    TRACEINTO << "\nCAuditorManager::HandleMidNightTimer";

    DWORD timeInterval = 24 * 60 * 60 * 100; // 24 hours in TICKs
    StartTimer(TIMER_MID_NIGHT, timeInterval);

    if(NULL != m_pAuditFileManager) // file system was disabled
    {
        m_pAuditFileManager->MoveToNextFile();
    }
}

//////////////////////////////////////////////////////////////////////
void CAuditorManager::OnAuditIncomingEvent(CSegment *pSeg)
{
//    TRACEINTO << "\nCAuditorManager::OnAuditIncomingEvent";

    AUDIT_EVENT_HEADER_S auditHdr;
    CFreeData freeData;

	memset(&auditHdr, 0, sizeof(auditHdr));
	
    CAuditHdrWrapper auditWrapper(auditHdr, freeData);
    auditWrapper.DeSerialize(0, pSeg);

    // add event to memory
    pProcess->GetAuditEventContainer()->AddEvent(auditWrapper);

    // add event to file
    if(NULL != m_pAuditFileManager) // means that StopFileSystem was called already
    {
        STATUS status = m_pAuditFileManager->HandleEvent(auditWrapper);
        if(STATUS_OK != status)
        {
            StopFileSystem();
        }
    }
}

//////////////////////////////////////////////////////////////////////
STATUS CAuditorManager::HandleTerminalDumpFiles(CTerminalCommand &command, std::ostream& answer)
{
    if(NULL != m_pAuditFileManager)
    {
        m_pAuditFileManager->DumpFileList(answer);
    }

    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CAuditorManager::HandleTerminalDumpEvents(CTerminalCommand &command, std::ostream& answer)
{
    CXMLDOMElement * pFatherNode = NULL;
    pProcess->GetAuditEventContainer()->SerializeXml(pFatherNode);

    char *szResultString = NULL;
    pFatherNode->DumpDataAsLongStringEx(&szResultString, TRUE);

    answer << szResultString;

    delete [] szResultString;
    delete pFatherNode;

    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CAuditorManager::HandleTerminalFlush(CTerminalCommand &command, std::ostream& answer)
{
    STATUS status = (NULL != m_pAuditFileManager
                     ?
                     m_pAuditFileManager->Flush() : STATUS_FAIL);
    answer << "Flush was done : " << CProcessBase::GetProcess()->GetStatusAsString(status);

    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CAuditorManager::HandleTerminalNext(CTerminalCommand &command, std::ostream& answer)
{

    TRACEINTO << "\nCAuditorManager::HandleMidNightNext";
    
    if(NULL != m_pAuditFileManager) // file system was disabled
    {
        m_pAuditFileManager->MoveToNextFile();
        answer << "Auditor moved to next file.";    
    }
    return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CAuditorManager::StopFileSystem()
{
    POBJDELETE(m_pAuditFileManager);
    AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                            BAD_FILE_SYSTEM,
                            MAJOR_ERROR_LEVEL,
                            "The Audit File System is disabled. No Auditing is being done.",
                            true,
                            true
                            );
}

/////////////////////////////////////////////////////////////////////
void CAuditorManager::InformHttpGetFile(const std::string & file_name)
{
    if (m_pAuditFileManager)
    {
        m_pAuditFileManager->InformAuditFileRetrieved(file_name);
    }
    
    StartTimer(AUDIT_FILE_SYSTEM_WARNING_TIMER,
               AUDIT_FILE_WARNING_TIMEOUT);
    

}

//////////////////////////////////////////////////////////////////////
void CAuditorManager::OnTimerFileSystemWarningTest()
{
    if (m_pAuditFileManager)
    {
        BOOL make_alarm = FALSE;
        CProcessBase::GetProcess()->GetSysConfig()->
            GetBOOLDataByKey(CFG_KEY_ENABLE_CYCLIC_FILE_SYSTEM_ALARMS,make_alarm);

        BOOL warning_flag = m_pAuditFileManager->TestForFileSystemWarning();
        if (warning_flag && !m_fileSystemWarning)
        {
            if (make_alarm)
            {
                
                AddActiveAlarm(	FAULT_GENERAL_SUBJECT, 
                                BACKUP_OF_AUDIT_FILES_IS_REQUIRED,
                                MAJOR_ERROR_LEVEL, 
                                "Administrator has to backup old audit files before system will delete them",
                                true,
                                true);
                m_fileSystemWarning = TRUE;
            }    
        }
        
        if (m_fileSystemWarning && !warning_flag)
        {
            RemoveActiveAlarmByErrorCode(BACKUP_OF_AUDIT_FILES_IS_REQUIRED);
            m_fileSystemWarning = FALSE;
        }
    }

    StartTimer(AUDIT_FILE_SYSTEM_WARNING_TIMER, AUDIT_FILE_WARNING_IDLE_TIMEOUT);

    
}
