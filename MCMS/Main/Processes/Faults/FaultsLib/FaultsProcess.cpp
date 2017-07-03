// FaultsProcess.cpp

#include "FaultsProcess.h"

#include "HlogList.h"

extern void FaultsManagerEntryPoint(void* appParam);

CProcessBase* CreateNewProcess()
{
  return new CFaultsProcess;
}

TaskEntryPoint CFaultsProcess::GetManagerEntryPoint()
{
  return FaultsManagerEntryPoint;
}

CFaultsProcess::CFaultsProcess() :
  m_pFaultsList(NULL),
  m_pFaultsShortList(NULL),
  m_IsHardDiskOk(YES)
{}

// Virtual
CFaultsProcess::~CFaultsProcess()
{
  delete m_pFaultsList;
  delete m_pFaultsShortList;
}

// Virtual
eProcessType CFaultsProcess::GetProcessType()
{
  return eProcessFaults;
}

// Virtual
BOOL CFaultsProcess::UsingSockets()
{
  return NO;
}

// Virtual
DWORD CFaultsProcess::GetMaxTimeForIdle() const
{
  return 12000;
}

// Virtual
int CFaultsProcess::GetProcessAddressSpace()
{
  return 96 * 1024 * 1024;
}

int CFaultsProcess::SetUp()
{
  CProcessBase::SetUp();
  return 0;
}

CHlogList* CFaultsProcess::GetFaultsListDB() const
{
  return m_pFaultsList;
}

CHlogList* CFaultsProcess::GetFaultsShortListDB() const
{
  return m_pFaultsShortList;
}

void CFaultsProcess::SetFaultList(CHlogList *pFaultsList)
{
  m_pFaultsList = pFaultsList;
}

void CFaultsProcess::SetShortFaultList(CHlogList *pFaultsList)
{
  m_pFaultsShortList = pFaultsList;
}

void CFaultsProcess::SetIsHardDiskOk(BOOL isOk)
{
  m_IsHardDiskOk = isOk;
}

BOOL CFaultsProcess::GetIsHardDiskOk()
{
  return m_IsHardDiskOk;
}
