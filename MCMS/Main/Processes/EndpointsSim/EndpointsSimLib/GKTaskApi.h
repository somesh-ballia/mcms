/*
 * GKTaskApi.h
 */

#ifndef _GKTASKAPI_
#define _GKTASKAPI_

#include "CSIDTaskApi.h"

class CGKTaskApi : public CCSIDTaskApi
{
CLASS_TYPE_1(CGKTaskApi, CCSIDTaskApi)
public:
    CGKTaskApi(void);
    CGKTaskApi(DWORD csID);
    ~CGKTaskApi(void);

    eEPSimTaskType GetTaskType(void) const;
};

#endif /* _GKTASKAPI_ */
