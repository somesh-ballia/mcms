// InstallerProcess.h

#ifndef INSTALLER_PROCESS_H_
#define INSTALLER_PROCESS_H_

#include "ProcessBase.h"
#include "InstallPhaseCommon.h"

class CInstallerProcess : public CProcessBase  
{
CLASS_TYPE_1(CInstallerProcess, CProcessBase)
public:
	friend class CTestInstallerProcess;

	CInstallerProcess();
	virtual ~CInstallerProcess();
	virtual eProcessType GetProcessType() {return eProcessInstaller;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();
	virtual void AddExtraStringsToMap();

    void SetInstallationStatus(STATUS status){m_InslationStatus = status;}
    STATUS GetInstallationStatus()const{return m_InslationStatus;}
    

    void UpdateSoftwareInstallProgress(eInstallPhaseType type, int progress);
    void UpdateSoftwareInstallStatus(eInstallPhaseType type, eInstallPhaseStatus state);
    void ResetSoftwareInstall();

private:
    virtual void AddExtraStatusesStrings();
    
    STATUS m_InslationStatus;
    CInstallPhaseList m_installPhaseList;
};

#endif  // INSTALLER_PROCESS_H_
