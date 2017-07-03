// MonitorTask.h: interface for the CMonitorTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MONITOR_TASK_H__)
#define _MONITOR_TASK_H__

#include "RequestHandler.h"
#include "DataTypes.h"

class CXMLDOMElement;
class CRequest;

extern "C" void monitorEntryPoint(void* appParam);

class CMonitorTask : public CRequestHandler
{
CLASS_TYPE_1(CMonitorTask,CRequestHandler )
public:
	CMonitorTask();
	virtual ~CMonitorTask();

	virtual const char* NameOf() const { return "CMonitorTask";}

	BOOL         IsSingleton() const {return YES;}
	const char*	 GetTaskName() const;

protected:

};

#endif // !defined(_MONITOR_TASK_H__)

