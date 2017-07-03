// GatekeeperProcess.cpp: implementation of the CGatekeeperProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "GKProcess.h"

extern void GKManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CGKProcess();
}


const CGKServiceManager*  GetServiceTask(DWORD serviceId)
{

	CGKProcess* pCPProcess = (CGKProcess*)CGKProcess::GetProcess();
	return (CGKServiceManager*)pCPProcess->GetServiceTask( serviceId );
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CGKProcess::GetManagerEntryPoint()
{
	return GKManagerEntryPoint;
}


//////////////////////////////////////////////////////////////////////
CGKProcess::CGKProcess()
{
     m_isFailoverEndcfg = false;
}

//////////////////////////////////////////////////////////////////////
CGKProcess::~CGKProcess()
{
}

void CGKProcess::AddServiceTask(CGKServiceManager* pGkServiceTask)
{
    m_services.push_back( pGkServiceTask );
}

CGKServiceManager* CGKProcess::GetServiceTask( DWORD serviceId )
{
    return m_services.at( serviceId );
}

BOOL CGKProcess::GetIsFailoverEndCfg()
{
    return m_isFailoverEndcfg;
}

void CGKProcess::SetIsFailoverEndcfg(BOOL bFlag)
{
    m_isFailoverEndcfg = bFlag;
}
