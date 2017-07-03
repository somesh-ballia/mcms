

#include "TraceStream.h"
#include "Trace.h"
#include "ProcessBase.h"
#include "TaskApi.h"
#include "LogFileTask.h"
#include "LoggerProcess.h"


extern char* ProcessTypeToString(eProcessType processType);

static int m_TaskIndex = 0;

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CLogFileTask)
PEND_MESSAGE_MAP(CLogFileTask, CTaskApp);


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void LogFileTaskEntryPoint(void* appParam)
{
    CLogFileTask * pLogFileTask = new CLogFileTask;
    pLogFileTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CLogFileTask::CLogFileTask()
{
    m_TaskIndex++;
    char szTmp[50];
    snprintf(szTmp, sizeof(szTmp), "%d", m_TaskIndex);
    string strTmp = szTmp;
    m_TaskName = "LogFileTask_" + strTmp;
    m_pProcess = (CLoggerProcess*)CLoggerProcess::GetProcess();
}

//////////////////////////////////////////////////////////////////////
CLogFileTask::~CLogFileTask()
{
}

//////////////////////////////////////////////////////////////////////
void CLogFileTask::InitTask()
{
    TRACESTR(eLevelInfoNormal) << "CLogFileTask - InitTask";
}
//////////////////////////////////////////////////////////////////////
void CLogFileTask::SelfKill()
{
//  TRACESTR(eLevelInfoNormal) << "CSwitchTask::SelfKill";
    // ===== 1.  clear entry in array
    COsQueue& rcvMbx = GetRcvMbx();
    // ===== 2.  destroy timer
    //DestroyTimer();

    // ===== 3. call father's SelfKill
    CTaskApp::SelfKill();
}
