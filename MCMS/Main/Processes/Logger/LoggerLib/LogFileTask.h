/*
 * LogFileTask.h
 *
 *  Created on: Feb 7, 2012
 *      Author: kobi
 */

#ifndef LOGFILETASK_H_
#define LOGFILETASK_H_

#include "TaskApp.h"

extern "C" void LogFileTaskEntryPoint(void* appParam);
class CLoggerProcess;

class CLogFileTask : public CTaskApp
{
public:
    CLogFileTask();
    virtual ~CLogFileTask();
    virtual const char* NameOf() const { return "LogFileTask";}
    void InitTask();
    void SelfKill();

    BOOL        IsSingleton() const {return NO;}
    const char* GetTaskName() const {return m_TaskName.c_str();}

private:
    CLoggerProcess*  m_pProcess;
    std::string     m_TaskName;
    PDECLAR_MESSAGE_MAP
};

#endif /* LOGFILETASK_H_ */
