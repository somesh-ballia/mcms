// InstallerMonitor.h: interface for the CInstallerMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CInstallerMONITOR__)
#define _CInstallerMONITOR__

#include "MonitorTask.h"
#include "Macros.h"




class CInstallerProcess;




class CInstallerMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CInstallerMonitor();
	virtual ~CInstallerMonitor();
	STATUS OnSetLastUpdateKeyStatus(CSegment* pParam);

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

private:
    STATUS HandleGetInstallationStatus(CRequest *pRequest);
	STATUS OnGetUpdateKeyStatus(CRequest* pGetRequest);
    
	int m_nLastSetUpdateKeyStatus;
	std::string m_LastSetUpdateKeyStatusDesc;


    const CInstallerProcess *m_pProcess;
};

#endif // !defined(_CInstallerMONITOR__)
