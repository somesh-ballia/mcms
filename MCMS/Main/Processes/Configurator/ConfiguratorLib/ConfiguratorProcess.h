// ConfiguratorProcess.h: interface for the CConfiguratorProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_DEMOPROCESS_H__)
#define _DEMOPROCESS_H__

#include "ProcessBase.h"

class CConfiguratorProcess : public CProcessBase  
{
CLASS_TYPE_1(CConfiguratorProcess,CProcessBase )
public:
	friend class CTestConfiguratorProcess;

	CConfiguratorProcess();
	virtual ~CConfiguratorProcess();
	virtual eProcessType GetProcessType() {return eProcessConfigurator;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();
    virtual BOOL GivesAwayRootUser() {return FALSE;}
    virtual BOOL HasMonitorTask() {return FALSE;}
    virtual int GetProcessAddressSpace() {return 0;}
};

#endif // !defined(_DEMOPROCESS_H__)
