#include "GetConfTempSpecific.h"
#include "CommRes.h"
#include "StatusesGeneral.h"
#include "psosxml.h"
#include "CommResDB.h"


extern CCommResDB * GetpConfTemplateDB();

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CConfTempSpecific::CConfTempSpecific():
m_confTemplateID(0xFFFFFFFF)
{
	
}
//////////////////////////////////////////////////////////////////////
CConfTempSpecific::CConfTempSpecific(const CConfTempSpecific &other):
CSerializeObject(other),
m_confTemplateID(other.m_confTemplateID)
{

}
//////////////////////////////////////////////////////////////////////
CConfTempSpecific& CConfTempSpecific::operator=(const CConfTempSpecific &other)
{
	m_confTemplateID = other.m_confTemplateID;
	m_updateCounter = other.m_updateCounter;
			
	return *this;
}
//////////////////////////////////////////////////////////////////////
CConfTempSpecific::~CConfTempSpecific()
{
	
}
/////////////////////////////////////////////////////////////////////////////
int CConfTempSpecific::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pResNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pResNode,"ID",&m_confTemplateID,_0_TO_DWORD);
	
	return nStatus;

}

///////////////////////////////////////////////////////////////////////////
void CConfTempSpecific::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	STATUS status=STATUS_OK;
	
	CCommRes * pCommRes = ::GetpConfTemplateDB()->GetCurrentRsrv(m_confTemplateID);
	if(pCommRes)
		pCommRes->SerializeXml(pFatherNode,m_updateCounter); 
	
	POBJDELETE(pCommRes);
}

//////////////////////////////////////////////////////////////////////////
DWORD  CConfTempSpecific::GetConfTemplateID()const
{
	return m_confTemplateID;
}
