// IceProcess.h: interface for the CIceProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_IcePROCESS_H__)
#define _IcePROCESS_H__

#include "ProcessBase.h"
#include "IceServiceManager.h"

class CIceProcess : public CProcessBase
{
CLASS_TYPE_1(CIceProcess,CProcessBase )
public:
	//friend class CTestIceProcess;

	CIceProcess();
	virtual const char* NameOf() const { return "CIceProcess";}
	virtual ~CIceProcess();
	virtual eProcessType GetProcessType() {return eProcessIce;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();
    virtual BOOL HasMonitorTask() {return FALSE;}
    void AddServiceTask(CIceServiceManager*);
    CIceServiceManager* GetServiceTask( DWORD serviceId );
	std::vector< CIceServiceManager * > m_services;
	
	virtual int GetProcessAddressSpace() {return 96 * 1024 * 1024;}

};

#endif // !defined(_IcePROCESS_H__)

