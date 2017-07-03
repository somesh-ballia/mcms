// MCCFMngrMonitor.h: interface for the CMCCFMngrMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CMCCFMngrMONITOR__)
#define _CMCCFMngrMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CMCCFMngrMonitor : public CMonitorTask
{
	CLASS_TYPE_1(CMCCFMngrMonitor, CMonitorTask)

public:

	CMCCFMngrMonitor();
	virtual ~CMCCFMngrMonitor();

protected:

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
};

#endif // !defined(_CMCCFMngrMONITOR__)
