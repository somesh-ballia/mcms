#include "CSSimTaskApi.h"

CCSSimTaskApi::CCSSimTaskApi(void)
{
}

CCSSimTaskApi::CCSSimTaskApi(DWORD csID) :
    CCSIDTaskApi(csID)
{
}

CCSSimTaskApi::~CCSSimTaskApi(void)
{
}

/*
 * virtual
 */
eEPSimTaskType CCSSimTaskApi::GetTaskType(void) const
{
    return eCSTaskType;
}
