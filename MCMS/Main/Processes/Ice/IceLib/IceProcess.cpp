// IceProcess.cpp: implementation of the CIceProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "IceProcess.h"

extern void IceManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CIceProcess();
}

//////////////////////////////////////////////////////////////////////
const CIceServiceManager*  GetServiceTask(DWORD serviceId)
{

	CIceProcess* pCPProcess = (CIceProcess*)CIceProcess::GetProcess();
	return (CIceServiceManager*)pCPProcess->GetServiceTask( serviceId );
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CIceProcess::GetManagerEntryPoint()
{
	return IceManagerEntryPoint;
}


//////////////////////////////////////////////////////////////////////
CIceProcess::CIceProcess()
{

}

//////////////////////////////////////////////////////////////////////
CIceProcess::~CIceProcess()
{

}

void CIceProcess::AddServiceTask(CIceServiceManager* pServiceTask)
{
    m_services.push_back( pServiceTask );
}

CIceServiceManager* CIceProcess::GetServiceTask( DWORD serviceId )
{
	if(serviceId < m_services.size())
		return m_services.at( serviceId );

	return NULL;
}

