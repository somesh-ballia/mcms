// ApacheModuleProcess.h: interface for the CApacheModuleProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_ApacheModulePROCESS_H__)
#define _ApacheModulePROCESS_H__

#include "ProcessBase.h"
#include "McuMngrInternalStructs.h"
#include "McmsAuthentication.h"



class CApacheModuleProcess : public CProcessBase  
{
CLASS_TYPE_1(CApacheModuleProcess,CProcessBase )
public:
	friend class CTestApacheModuleProcess;

	CApacheModuleProcess();
	virtual ~CApacheModuleProcess();
	const char * NameOf(void) const {return "CApacheModuleProcess";}
	virtual eProcessType GetProcessType() {return eProcessApacheModule;}
	virtual TaskEntryPoint GetManagerEntryPoint();
	virtual BOOL UsingSockets() {return NO;}
    virtual BOOL GivesAwayRootUser() {return FALSE;}    
    virtual BOOL HasErrorHandlerTask() {return FALSE;}
    virtual BOOL HasWatchdogTask() {return YES;}
    
    virtual int GetProcessAddressSpace() {return 500 * 1024 * 1024;}
    void AddExtraStringsToMap();
    

	void						SetAuthenticationStruct(MCMS_AUTHENTICATION_S* authentStruct);
	MCMS_AUTHENTICATION_S*		GetAuthenticationStruct() const;
	
	void						SetLicensingStruct(APACHEMODULE_LICENSING_S* licensingStruct);
	APACHEMODULE_LICENSING_S*	GetLicensingStruct() const;
	void                        SetLicensingStructLicenseMode(int LicenseMode);
	
	void SetIsAuthenticationStructureAlreadyReceived(BOOL isReceived);
	BOOL GetIsAuthenticationStructureAlreadyReceived();
	
    static BOOL IsFederalOn();
	void SetWaitingLicensingInd(BOOL bWaitingLicensingInd);
	BOOL GetWaitingLicensingInd();

    void SetBlockRequest(BOOL blockState){m_bBlockRequests = blockState;};
    BOOL IsBlockRequests(){return m_bBlockRequests;};
protected:
	MCMS_AUTHENTICATION_S*		m_pAuthenticationStruct;
	APACHEMODULE_LICENSING_S*	m_pLicensingStruct;
	BOOL					m_isAuthenticationStructureAlreadyReceived;
	BOOL m_bWaitingLicensingInd;
	BOOL m_bBlockRequests;
};

#endif // !defined(_ApacheModulePROCESS_H__)

