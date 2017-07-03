// EthernetSettingsConfigListGet.cpp: implementation of the CEthernetSettingsConfigListGet class.
//////////////////////////////////////////////////////////////////////////


#include "EthernetSettingsConfigListGet.h"
#include "EthernetSettingsConfig.h"
#include "McuMngrProcess.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CEthernetSettingsConfigListGet::CEthernetSettingsConfigListGet()
{
	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CEthernetSettingsConfigListGet::CEthernetSettingsConfigListGet(const CEthernetSettingsConfigListGet &other)
: CSerializeObject(other)
{
	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;
}

/////////////////////////////////////////////////////////////////////////////
CEthernetSettingsConfigListGet::~CEthernetSettingsConfigListGet()
{

}

/////////////////////////////////////////////////////////////////////////////
CEthernetSettingsConfigListGet& CEthernetSettingsConfigListGet::operator = (const CEthernetSettingsConfigListGet &other)
{
	if (&other == this)
		return *this;

	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;
	return *this;
}

///////////////////////////////////////////////////////////////////////////
void CEthernetSettingsConfigListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CEthernetSettingsConfigList* pEthernetSettingsConfigList = m_pProcess->GetEthernetSettingsConfigList();
	
	if (pEthernetSettingsConfigList)
	{
		pEthernetSettingsConfigList->SerializeXmlForEma(pActionsNode);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CEthernetSettingsConfigListGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	//GET_VALIDATE_CHILD(pNode, "OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD); Not implemented yet
	return nStatus;
}
