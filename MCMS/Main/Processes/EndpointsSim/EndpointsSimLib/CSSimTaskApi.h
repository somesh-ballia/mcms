/*
 * CSSimTaskApi.h
 */

#ifndef _CSSIMTASKAPI_
#define _CSSIMTASKAPI_

#include "CSIDTaskApi.h"

class CCSSimTaskApi : public CCSIDTaskApi
{
CLASS_TYPE_1(CCSSimTaskApi, CCSIDTaskApi)
public:
    CCSSimTaskApi(void);
    CCSSimTaskApi(DWORD csID);
    ~CCSSimTaskApi(void);

    eEPSimTaskType GetTaskType(void) const;
};

#endif /* _CSSIMTASKAPI_ */
