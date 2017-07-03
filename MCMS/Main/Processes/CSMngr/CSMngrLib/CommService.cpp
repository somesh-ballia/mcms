// ConfigParamService.cpp: implementation of the CCommService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Base class for all classes who need to communicate with other processes
// 								within MCMS and outside modules like CS Module
//========   ==============   =====================================================================

#include "CommService.h"
#include "IpService.h"
#include "CSMngrProcess.h"
#include "TraceStream.h"

#define STR_NO	"NO"
#define STR_YES "YES"



/*--------------------------------------------------------------------
	Static methods
--------------------------------------------------------------------*/

int CCommService::MoveNext(int & param)
{
	param = param + 1;
	return param;
}

//////////////////////////////////////////////////////////////////////
int CCommService::MoveBack(int & param)
{
	param = param - 1;
	return param;
}




/*--------------------------------------------------------------------
	Public methods
--------------------------------------------------------------------*/

CCommService::CCommService()
{
	m_IpService 	= NULL;
	m_IsConnected 	= false;
	m_pCSProcess = dynamic_cast<CCSMngrProcess*>(CProcessBase::GetProcess());
}

//////////////////////////////////////////////////////////////////////
CCommService::~CCommService()
{}



//////////////////////////////////////////////////////////////////////
void CCommService::SetIpService(CIPService *ipService)
{
	m_IpService = ipService;
}

//////////////////////////////////////////////////////////////////////
bool CCommService::GetIsConnected()
{
	return m_IsConnected;
}

//////////////////////////////////////////////////////////////////////
void CCommService::SetIsConnected(bool val)
{
	m_IsConnected = val;
}

//////////////////////////////////////////////////////////////////////
CIPServiceList* CCommService::GetIpServiceList()
{
	static CCSMngrProcess *pProcess = (CCSMngrProcess*)(CProcessBase::GetProcess()); 
	CIPServiceList *pList = pProcess->GetIpServiceListDynamic();
	const eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	return pList;
}
