/*
 * CSIDTaskApi.h
 */

#ifndef _CSIDTASKAPI_
#define _CSIDTASKAPI_

#include "DataTypes.h"
#include "TaskApi.h"
#include "EndpointsSimProcess.h"

class COsQueue;

class CCSIDTaskApi : public CTaskApi
{
CLASS_TYPE_1(CCSIDTaskApi, CTaskApi)
public:
    ~CCSIDTaskApi(void);
    void Create(void (*entryPoint)(void*), const COsQueue& creatorRcvMbx);

    int CreateOnlyApi(void);
    DWORD GetCSID(void) const;

    virtual eEPSimTaskType GetTaskType(void) const = 0;

    static DWORD GetCSIDFromTaskApi(const CTaskApi* api);

protected:
    CCSIDTaskApi(void);
    CCSIDTaskApi(DWORD csID);

    DWORD m_csID;
};

#endif /* _CSIDTASKAPI_ */
