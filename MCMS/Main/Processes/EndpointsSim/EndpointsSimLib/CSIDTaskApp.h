/*
 * CSIDTaskApp.h
 */

#ifndef _CSIDTASKAPP_
#define _CSIDTASKAPP_

#include "DataTypes.h"
#include "TaskApp.h"
#include "EndpointsSimProcess.h"

class CSegment;

class CCSIDTaskApp : public CTaskApp
{
CLASS_TYPE_1(CCSIDTaskApp, CTaskApp)
public:
    CCSIDTaskApp(void);
    ~CCSIDTaskApp(void);
    void Create(CSegment& appParam);
    DWORD GetCSID(void) const;

    virtual eEPSimTaskType GetTaskType(void) const = 0;

    static DWORD GetCurrentTaskCSID(void);
    static const DWORD INVALID_CS_ID;

private:
    DWORD m_csID;
    eEPSimTaskType m_type;
};

#endif /* _CSIDTASKAPP_ */
