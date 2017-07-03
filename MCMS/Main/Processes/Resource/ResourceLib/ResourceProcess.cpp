// ResourceProcess.cpp: implementation of the CResourceProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "ResourceProcess.h"
#include "CRsrcDetailGet.h"
#include "RsrvManager.h"

extern void ResourceManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CResourceProcess;
}


//////////////////////////////////////////////////////////////////////
TaskEntryPoint CResourceProcess::GetManagerEntryPoint()
{
	return ResourceManagerEntryPoint;
}

//////////////////////////////////////////////////////////////////////
CResourceProcess::CResourceProcess()
{
	m_pMoveManager = NULL;
	m_pSystemResources = NULL;
	m_pCentralConferencesDB = NULL;	
	m_pRsrvManager = NULL;
	m_bSNMPEnabled = FALSE;
}

//////////////////////////////////////////////////////////////////////
CResourceProcess::~CResourceProcess()
{
  //POBJDELETE(m_pSystemResources);
}
////////////////////////////////////////////////////////////////////////
//***override SetUp() function for allocating memory to Resources DB ***
////////////////////////////////////////////////////////////////////////
void CResourceProcess::SetUpProcess()
{
	//CProcessBase::SetUp();

	m_pSystemResources = new CSystemResources;
	
	m_pCentralConferencesDB = new CCentralConferencesDB();
	
	m_pRsrvManager = NULL;
	
	
	m_pMoveManager     = new CMoveManager;
}
//////////////////////////////////////////////////////////////////////
void CResourceProcess::AddExtraStringsToMap()
{
	CStringsMaps::AddItem(COP_PORT_TYPE_ENUM,TYPE_NONE,"none");
	CStringsMaps::AddItem(COP_PORT_TYPE_ENUM,TYPE_COP,"conf_on_port");
	CStringsMaps::AddItem(COP_PORT_TYPE_ENUM,TYPE_COP_PRIMERY,"conf_on_port_primary");
	CStringsMaps::AddItem(COP_PORT_TYPE_ENUM,TYPE_COP_SECONDARY,"conf_on_port_secondary");
	CStringsMaps::AddItem(COP_PORT_TYPE_ENUM,TYPE_ART,"art");
	CStringsMaps::AddItem(COP_PORT_TYPE_ENUM,TYPE_VIDEO,"video");
	
	CStringsMaps::AddItem(CHANGE_STATUS_ENUM,CONF_NOT_CHANGED,"none");
	CStringsMaps::AddItem(CHANGE_STATUS_ENUM,CONF_FAST_PLUS_SLOW_INFO,"update");
	CStringsMaps::AddItem(CHANGE_STATUS_ENUM,CONF_COMPLETE_INFO,"new");

}

/////////////////////////////////////////////////////////////////////////
int CResourceProcess::TearDown()
{
	CSelfConsistency::TearDown();
	//SystemSleep(50); 

	POBJDELETE(m_pSystemResources);

	POBJDELETE(m_pCentralConferencesDB);

	
	POBJDELETE(m_pMoveManager);
	
    CProcessBase::TearDown();
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////
CReservator* CResourceProcess::GetReservator() const
{
	
	if(!m_pRsrvManager)
	{
		PASSERT(1);
		return NULL;
	}
	
	return m_pRsrvManager->GetReservator();
}

///////////////////////////////////////////////////////////////////////
bool CResourceProcess::IsFailoverBlockTransaction_SlaveMode(string sAction)
{
	return true; // all Resource SET transactions are blocked in Slave mode
}
