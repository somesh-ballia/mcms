#include "ProxyTaskApi.h"

CProxyTaskApi::CProxyTaskApi(void)
{
}

CProxyTaskApi::CProxyTaskApi(DWORD csID) :
    CCSIDTaskApi(csID)
{
}

CProxyTaskApi::~CProxyTaskApi(void)
{
}

/*
 * virtual
 */
eEPSimTaskType CProxyTaskApi::GetTaskType(void) const
{
    return eProxyTaskType;
}
