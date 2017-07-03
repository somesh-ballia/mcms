// DemoManager.h

#ifndef DEMO_MANAGER_H_
#define DEMO_MANAGER_H_

#include "ManagerTask.h"
#include "Macros.h"

void DemoManagerEntryPoint(void* appParam);

class CDemoManager : public CManagerTask, CNonCopyable
{
  CLASS_TYPE_1(CDemoManager, CManagerTask)

 public:
                 CDemoManager();
  virtual       ~CDemoManager();
  void           ManagerPostInitActionsPoint();
  TaskEntryPoint GetMonitorEntryPoint();

 private:
  PDECLAR_MESSAGE_MAP
  PDECLAR_TRANSACTION_FACTORY
  PDECLAR_TERMINAL_COMMANDS
};

#endif

