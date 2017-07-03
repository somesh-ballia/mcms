// IPMCInterfaceProcess.h: interface for the CIPMCInterfaceProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_DEMOPROCESS_H__)
#define _DEMOPROCESS_H__

#include "ProcessBase.h"

class CIPMCInterfaceProcess : public CProcessBase  
{
CLASS_TYPE_1(CIPMCInterfaceProcess,CProcessBase )
public:
	friend class CTestIPMCInterfaceProcess;

	CIPMCInterfaceProcess();
	virtual ~CIPMCInterfaceProcess();
	virtual eProcessType GetProcessType() {return eProcessIPMCInterface;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();
    virtual BOOL GivesAwayRootUser() {return FALSE;}
};

#endif // !defined(_DEMOPROCESS_H__)
