// AuditorProcess.cpp

#include "AuditorProcess.h"

#include "SystemFunctions.h"
#include "AuditFileList.h"
#include "AuditEventContainer.h"
#include "AuditorDefines.h"

extern void AuditorManagerEntryPoint(void* appParam);

CProcessBase* CreateNewProcess()
{
  return new CAuditorProcess;
}

TaskEntryPoint CAuditorProcess::GetManagerEntryPoint()
{
  return AuditorManagerEntryPoint;
}

CAuditorProcess::CAuditorProcess() :
  m_IsDownload(false),
  m_pAuditFileList(new CAuditFileList),
  m_pAuditEventContainer(new CAuditEventContainer(MAX_NUM_AUDIT_EVENT_IN_MEMORY))
{
}

CAuditorProcess::~CAuditorProcess()
{
  delete m_pAuditFileList;
  delete m_pAuditEventContainer;
}

// Virtual
const char* CAuditorProcess::NameOf() const
{
  return GetCompileType();
}

// Virtual
eProcessType CAuditorProcess::GetProcessType()
{
  return eProcessAuditor;
}

// Virtual
BOOL CAuditorProcess::UsingSockets()
{
  return NO;
}

// Virtual
DWORD CAuditorProcess::GetMaxTimeForIdle() const
{
  return 12000;
}

// Virtual
int CAuditorProcess::GetProcessAddressSpace()
{
  return 96 * 1024 * 1024;
}

CAuditFileList* CAuditorProcess::GetAuditFileList()
{
  return m_pAuditFileList;
}

CAuditEventContainer* CAuditorProcess::GetAuditEventContainer()
{
  return m_pAuditEventContainer;
}
