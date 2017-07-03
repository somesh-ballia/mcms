// SNMPTask.h

#ifndef SNMPTASK_H_
#define SNMPTASK_H_

#include "TaskApp.h"

class CSNMPTask : public CTaskApp
{
  CLASS_TYPE_1(CSNMPTask, CTaskApp)
public:
  CSNMPTask();
  virtual ~CSNMPTask();
  virtual const char* NameOf() const      { return "CSNMPTask";}

  BOOL                IsSingleton() const {return YES;}
  const char*         GetTaskName() const {return "SNMPTask";}
  void                WaitForEvent();

protected:
  void                InitTask();
  virtual void        RegisterOID() const { }

  PDECLAR_MESSAGE_MAP;
};

#endif

