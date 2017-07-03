// LicenseServerProcess.cpp

#include "LicenseServerProcess.h"
#include "SystemFunctions.h"

extern void LicenseServerManagerEntryPoint(void* appParam);

CProcessBase* CreateNewProcess()
{
  return new CLicenseServerProcess;
}

TaskEntryPoint CLicenseServerProcess::GetManagerEntryPoint()
{
  return LicenseServerManagerEntryPoint;
}

CLicenseServerProcess::CLicenseServerProcess()
{
	const char * flexeraLicense = getenv("FLEXERA_LICENSE");

	  if ((NULL != flexeraLicense)  && (strcmp(flexeraLicense ,"YES") == 0))
		  m_isFlexeraSimulationMode=TRUE;
	  else
	      m_isFlexeraSimulationMode=FALSE;
}

CLicenseServerProcess::~CLicenseServerProcess()
{}

BOOL CLicenseServerProcess::IsFlexeraSimulationMode()
{
	return m_isFlexeraSimulationMode;
}
