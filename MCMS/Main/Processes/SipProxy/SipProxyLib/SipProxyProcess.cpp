// SipProxyProcess.cpp: implementation of the CSipProxyProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "SipProxyProcess.h"

extern void SipProxyManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CSipProxyProcess();
}

//////////////////////////////////////////////////////////////////////
const CSipProxyServiceManager*  GetServiceTask(DWORD serviceId)
{

	CSipProxyProcess* pCPProcess = (CSipProxyProcess*)CSipProxyProcess::GetProcess();
	return (CSipProxyServiceManager*)pCPProcess->GetServiceTask( serviceId );
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CSipProxyProcess::GetManagerEntryPoint()
{
	return SipProxyManagerEntryPoint;
}


//////////////////////////////////////////////////////////////////////
CSipProxyProcess::CSipProxyProcess()
{

}

//////////////////////////////////////////////////////////////////////
CSipProxyProcess::~CSipProxyProcess()
{

}

void CSipProxyProcess::AddServiceTask(CSipProxyServiceManager* pServiceTask)
{
    m_services.push_back( pServiceTask );
}

CSipProxyServiceManager* CSipProxyProcess::GetServiceTask( DWORD serviceId )
{
    return m_services.at( serviceId );
}

