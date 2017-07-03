// ConfiguratorProcess.cpp: implementation of the CConfiguratorProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "ConfiguratorProcess.h"
#include "SystemFunctions.h"

extern void ConfiguratorManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CConfiguratorProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CConfiguratorProcess::GetManagerEntryPoint()
{
	return ConfiguratorManagerEntryPoint;
}


//////////////////////////////////////////////////////////////////////
CConfiguratorProcess::CConfiguratorProcess()
{

}

//////////////////////////////////////////////////////////////////////
CConfiguratorProcess::~CConfiguratorProcess()
{

}

