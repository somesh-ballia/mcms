/*
 *  ProxyTaskApi.h
 */

#ifndef _PROXYTASKAPI_
#define _PROXYTASKAPI_

#include "CSIDTaskApi.h"

class CProxyTaskApi : public CCSIDTaskApi
{
CLASS_TYPE_1(CProxyTaskApi, CCSIDTaskApi)
public:
    CProxyTaskApi(void);
    CProxyTaskApi(DWORD csID);
    ~CProxyTaskApi(void);

    eEPSimTaskType GetTaskType(void) const;
};

#endif /* _PROXYTASKAPI_ */
