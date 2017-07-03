// LicenseServerMonitor.h

#ifndef LICENSE_SERVER_MONITOR_H_
#define LICENSE_SERVER_MONITOR_H_

#include "MonitorTask.h"
#include "Macros.h"

class CLicenseServerMonitor : public CMonitorTask, CNonCopyable
{
  CLASS_TYPE_1(CLicenseServerMonitor, CMonitorTask)

 public:
           CLicenseServerMonitor();
  virtual ~CLicenseServerMonitor();

 private:
  PDECLAR_MESSAGE_MAP
  PDECLAR_TRANSACTION_FACTORY
};

#endif

