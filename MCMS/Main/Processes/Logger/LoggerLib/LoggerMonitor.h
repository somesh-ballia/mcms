// LoggerMonitor.h

#ifndef LOGGER_MONITOR_H_
#define LOGGER_MONITOR_H_

#include "MonitorTask.h"

class CLoggerMonitor : public CMonitorTask
{
CLASS_TYPE_1(CLoggerMonitor, CMonitorTask)
public:
	CLoggerMonitor(void);
	virtual const char* NameOf(void) const;

private:
	STATUS OnServerLoggerFileList(CRequest* pGetRequest);
	STATUS OnServerGetLoggerConfiguration(CRequest* pGetRequest);
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
};

#endif  // LOGGER_MONITOR_H_
