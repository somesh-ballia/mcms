// GatekeeperProcess.h: interface for the CGatekeeperProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_GKPROCESS_H__)
#define _GKPROCESS_H__

#include "GKServiceManager.h"
#include "ProcessBase.h"

class CGKProcess : public CProcessBase  
{
CLASS_TYPE_1(CGKProcess,CProcessBase )
public:
	friend class CTestGKProcess;

	CGKProcess();
	virtual const char* NameOf() const { return "CGKProcess";}
	virtual ~CGKProcess();
	virtual eProcessType GetProcessType() {return eProcessGatekeeper;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();    
    virtual BOOL HasMonitorTask() {return FALSE;}
    void AddServiceTask(CGKServiceManager*);
    CGKServiceManager* GetServiceTask( DWORD serviceId );
	std::vector< CGKServiceManager * > m_services;
	
	virtual int GetProcessAddressSpace() {return 50 * 1024 * 1024;}
	BOOL GetIsFailoverEndCfg();
	void SetIsFailoverEndcfg(BOOL bFlag);
private:
	 BOOL     m_isFailoverEndcfg;
};

#endif // !defined(_GKPROCESS_H__)

