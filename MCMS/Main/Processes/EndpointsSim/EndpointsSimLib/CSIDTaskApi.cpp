#include "CSIDTaskApi.h"

#include "Trace.h"
#include "TraceStream.h"
#include "CSIDTaskApp.h"

/*
 * Static
 */
DWORD CCSIDTaskApi::GetCSIDFromTaskApi(const CTaskApi* api_const)
{
    FPASSERT_AND_RETURN_VALUE(api_const == NULL, CCSIDTaskApp::INVALID_CS_ID);

    // IsTypeOf() and GetRTType() is not declared as const
    CTaskApi* api = const_cast<CTaskApi*>(api_const);

    FPASSERTSTREAM_AND_RETURN_VALUE(
        !api->IsTypeOf(CCSIDTaskApi::GetCompileType()),
        "Invalid class type " << api->GetRTType()
            << ", should be " << CCSIDTaskApi::GetCompileType(),
        CCSIDTaskApp::INVALID_CS_ID);

    return static_cast<const CCSIDTaskApi*>(api_const)->GetCSID();
}

CCSIDTaskApi::CCSIDTaskApi(void) :
    m_csID(CCSIDTaskApp::GetCurrentTaskCSID())
{ }

CCSIDTaskApi::CCSIDTaskApi(DWORD csID) :
    m_csID(csID)
{ }

CCSIDTaskApi::~CCSIDTaskApi(void)
{ }

void CCSIDTaskApi::Create(void (*entryPoint)(void*),
                          const COsQueue& creatorRcvMbx)
{
    PASSERTSTREAM_AND_RETURN(
        CCSIDTaskApp::INVALID_CS_ID == m_csID,
        "Invalid CS ID for " << EPSimTaskTypeToStr(GetTaskType()));

    CTaskApi::Create(creatorRcvMbx);
    m_appParam << m_csID;
    LoadApp(entryPoint);
}

int CCSIDTaskApi::CreateOnlyApi(void)
{
    PASSERTSTREAM_AND_RETURN_VALUE(
        CCSIDTaskApp::INVALID_CS_ID == m_csID,
        "Invalid CS ID for " << EPSimTaskTypeToStr(GetTaskType()),
        -1);

    CEndpointsSimProcess* process =
            (CEndpointsSimProcess*)CProcessBase::GetProcess();
    PASSERT_AND_RETURN_VALUE(process == NULL, -2);

    COsQueue queue = process->RetrieveQueue(m_csID, GetTaskType());
    PASSERTSTREAM_AND_RETURN_VALUE(
        !queue.IsValid(),
        "The task "
            << EPSimTaskTypeToStr(GetTaskType())
            << "["
            << m_csID
            << "] is not started yet",
        -3);

    CTaskApi::CreateOnlyApi(queue);

    return 0;
}

DWORD CCSIDTaskApi::GetCSID(void) const
{
    PASSERTSTREAM(
        CCSIDTaskApp::INVALID_CS_ID == m_csID,
        "Invalid CS ID for " << EPSimTaskTypeToStr(GetTaskType()));

    return m_csID;
}
