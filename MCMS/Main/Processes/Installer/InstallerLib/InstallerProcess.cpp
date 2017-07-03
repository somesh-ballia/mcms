// InstallerProcess.cpp: implementation of the CInstallerProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "InstallerProcess.h"
#include "SystemFunctions.h"
#include "ApiStatuses.h"
#include "StringsMaps.h"
#include "InstallerPreviousVersion.h"

extern void InstallerManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CInstallerProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CInstallerProcess::GetManagerEntryPoint()
{
	return InstallerManagerEntryPoint;
}


//////////////////////////////////////////////////////////////////////
CInstallerProcess::CInstallerProcess()
  :m_installPhaseList(READ_WRITE)
{
    m_InslationStatus = STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
CInstallerProcess::~CInstallerProcess()
{

}

//////////////////////////////////////////////////////////////////////
void CInstallerProcess::AddExtraStatusesStrings()
{
}

//////////////////////////////////////////////////////////////////////
void CInstallerProcess::AddExtraStringsToMap()
{
	CProcessBase::AddExtraStringsToMap();
	
	CStringsMaps::AddItem(VERSION_TYPE_ENUM, eVersionTypeFallback,  "fallback");
	CStringsMaps::AddItem(VERSION_TYPE_ENUM, eVersionTypeFactory,   "factory");	
}

//////////////////////////////////////////////////////////////////////
void CInstallerProcess::ResetSoftwareInstall()
{
  m_installPhaseList.InitToZeroState();
}

//////////////////////////////////////////////////////////////////////
void CInstallerProcess::UpdateSoftwareInstallProgress(eInstallPhaseType type, 
						      int progress)
{
  CInstallPhase phase;
  m_installPhaseList.m_phaseList.Get((int)type,phase);
  phase.m_progress = progress;
  m_installPhaseList.m_phaseList.Update(phase);

}
 
//////////////////////////////////////////////////////////////////////
void CInstallerProcess::UpdateSoftwareInstallStatus(eInstallPhaseType type, eInstallPhaseStatus state)
{

  CInstallPhase phase;
  m_installPhaseList.m_phaseList.Get((int)type,phase);
  phase.m_status = state;
  m_installPhaseList.m_phaseList.Update(phase);

}
