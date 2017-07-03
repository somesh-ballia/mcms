// MediaMngrMonitor.h: interface for the CMediaMngrMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CMediaMngrMONITOR__)
#define _CMediaMngrMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CMediaMngrMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CMediaMngrMonitor();
	virtual ~CMediaMngrMonitor();

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CMediaMngrMONITOR__)
