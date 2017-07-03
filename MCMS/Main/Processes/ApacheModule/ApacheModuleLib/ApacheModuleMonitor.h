// ApacheModuleMonitor.h: interface for the CApacheModuleMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CApacheModuleMONITOR__)
#define _CApacheModuleMONITOR__

#include "MonitorTask.h"

class CApacheModuleMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CApacheModuleMonitor();
	virtual ~CApacheModuleMonitor();
	STATUS HandleGetConnectionList(CRequest *pRequest);
	STATUS HandleGetVirtualDirectory(CRequest *pRequest);
	STATUS HandleGetVirtualDirectoryRecursive(CRequest *pRequest);

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	
private:
	STATUS GetVirtualDirectory(CRequest *pRequest, WORD bNested);
};

#endif // !defined(_CApacheModuleMONITOR__)

