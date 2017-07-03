#include "CSIDTaskApp.h"

#include "Trace.h"
#include "TraceStream.h"
#include "Segment.h"

const DWORD CCSIDTaskApp::INVALID_CS_ID = (DWORD)-1;

CCSIDTaskApp::CCSIDTaskApp(void) :
    m_csID(INVALID_CS_ID),
    m_type(eUnknownTaskType)
{ }

CCSIDTaskApp::~CCSIDTaskApp(void)
{
    if(INVALID_CS_ID == m_csID)
        return;

    PASSERT_AND_RETURN(eUnknownTaskType == m_type);

    CEndpointsSimProcess* process =
            (CEndpointsSimProcess*)CProcessBase::GetProcess();
    PASSERT_AND_RETURN(process == NULL);

    process->UnregisterQueue(m_csID, m_type);

    // TODO (drabkin) clear up, crash on trace
//    TRACEINTO << EpSimTaskTypeToStr(m_type)
//              << "["                 << m_csID
//              << "] down, Task ID: " << GetTaskId();
}

DWORD CCSIDTaskApp::GetCSID(void) const
{
    PASSERTSTREAM(
        INVALID_CS_ID == m_csID,
        "Invalid CS ID for " << EPSimTaskTypeToStr(GetTaskType())
            << ", Task ID: " << GetTaskId()
            << ", Name: "    << GetTaskName());

    return m_csID;
}

void CCSIDTaskApp::Create(CSegment& appParam)
{
    CEndpointsSimProcess* process =
           (CEndpointsSimProcess*)CProcessBase::GetProcess();
    PASSERT_AND_RETURN(process == NULL);

    CTaskApp::Create(appParam);

    appParam >> m_csID;

    PASSERTSTREAM_AND_RETURN(
        INVALID_CS_ID == m_csID,
        "Invalid CS ID for " << EPSimTaskTypeToStr(GetTaskType())
            << ", Task ID: " << GetTaskId()
            << ", Name: "    << GetTaskName());

    m_type = GetTaskType();
    process->RegisterQueue(m_csID, m_type, GetRcvMbx());

    TRACEINTO << EPSimTaskTypeToStr(m_type)
              << "["               << m_csID
              << "] up, Task ID: " << GetTaskId()
              << ", Name: "        << GetTaskName();
}

// static
DWORD CCSIDTaskApp::GetCurrentTaskCSID(void)
{
    CProcessBase* process = CProcessBase::GetProcess();
    FPASSERT_AND_RETURN_VALUE(process == NULL, INVALID_CS_ID);

    CTaskApp* task = process->GetCurrentTask();
    FPASSERT_AND_RETURN_VALUE(task == NULL, INVALID_CS_ID);

    FPASSERTSTREAM_AND_RETURN_VALUE(
        !task->IsTypeOf(CCSIDTaskApp::GetCompileType()),
        "Invalid class type " << task->GetRTType()
            << ", should be " << CCSIDTaskApp::GetCompileType(),
        INVALID_CS_ID);

    return ((CCSIDTaskApp*)task)->GetCSID();
}
