// DemoMonitor.h

#ifndef DEMO_MONITOR_H_
#define DEMO_MONITOR_H_

#include "MonitorTask.h"
#include "Macros.h"

class CDemoMonitor : public CMonitorTask, CNonCopyable
{
  CLASS_TYPE_1(CDemoMonitor, CMonitorTask)

 public:
           CDemoMonitor();
  virtual ~CDemoMonitor();

 private:
  PDECLAR_MESSAGE_MAP
  PDECLAR_TRANSACTION_FACTORY
};

#endif

