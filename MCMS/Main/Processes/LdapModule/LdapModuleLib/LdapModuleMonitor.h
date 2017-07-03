// LdapModuleMonitor.h: interface for the CLdapModuleMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CLdapModuleMONITOR__)
#define _CLdapModuleMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CLdapModuleMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CLdapModuleMonitor();
	virtual ~CLdapModuleMonitor();

	STATUS OnGetActiveDirCfg(CRequest* pGetRequest);

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

private:
	STATUS OnUpdateAdServerAvailabilityStatus(CSegment* pParam);
	STATUS OnGetAdServerAvailabilityStatus(CRequest* pGetRequest);

	DWORD m_adServerAvailabilityStatus;

};

#endif // !defined(_CLdapModuleMONITOR__)
