// SipProxyProcess.h: interface for the CSipProxyProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_SipProxyPROCESS_H__)
#define _SipProxyPROCESS_H__

#include "ProcessBase.h"
#include "SipProxyServiceManager.h"

class CSipProxyProcess : public CProcessBase  
{
CLASS_TYPE_1(CSipProxyProcess,CProcessBase )
public:
	friend class CTestSipProxyProcess;

	CSipProxyProcess();
	virtual const char* NameOf() const { return "CSipProxyProcess";}
	virtual ~CSipProxyProcess();
	virtual eProcessType GetProcessType() {return eProcessSipProxy;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();
    virtual BOOL HasMonitorTask() {return FALSE;}
    void AddServiceTask(CSipProxyServiceManager*);
    CSipProxyServiceManager* GetServiceTask( DWORD serviceId );
	std::vector< CSipProxyServiceManager * > m_services;
	
	virtual int GetProcessAddressSpace() {return 96 * 1024 * 1024;}

};

#endif // !defined(_SipProxyPROCESS_H__)

