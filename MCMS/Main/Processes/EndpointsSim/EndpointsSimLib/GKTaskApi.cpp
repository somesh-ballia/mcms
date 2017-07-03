#include "GKTaskApi.h"

CGKTaskApi::CGKTaskApi(void)
{ }

CGKTaskApi::CGKTaskApi(DWORD csID) :
    CCSIDTaskApi(csID)
{ }

CGKTaskApi::~CGKTaskApi(void)
{ }

/*
 * virtual
 */
eEPSimTaskType CGKTaskApi::GetTaskType(void) const
{
    return eGKTypeTaskType;
}

