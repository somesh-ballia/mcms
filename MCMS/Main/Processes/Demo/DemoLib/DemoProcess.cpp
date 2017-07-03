// DemoProcess.cpp

#include "DemoProcess.h"
#include "SystemFunctions.h"

extern void DemoManagerEntryPoint(void* appParam);

CProcessBase* CreateNewProcess()
{
  return new CDemoProcess;
}

TaskEntryPoint CDemoProcess::GetManagerEntryPoint()
{
  return DemoManagerEntryPoint;
}

CDemoProcess::CDemoProcess()
{}

CDemoProcess::~CDemoProcess()
{}
