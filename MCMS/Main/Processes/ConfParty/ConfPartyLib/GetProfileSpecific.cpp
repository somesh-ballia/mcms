#include "GetProfileSpecific.h"
#include "CommRes.h"
#include "StatusesGeneral.h"
#include "psosxml.h"
#include "CommResDB.h"

extern CCommResDB * GetpProfilesDB();
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGetProfileSpecific::CGetProfileSpecific():
m_ConfID(0xFFFFFFFF)
{
	
}

CGetProfileSpecific::CGetProfileSpecific(const CGetProfileSpecific &other):
CSerializeObject(other),
m_ConfID(other.m_ConfID)
{

}
 
/////////////////////////////////////////////////////////////////////////////
CGetProfileSpecific& CGetProfileSpecific::operator = (const CGetProfileSpecific  &other)
{
	m_ConfID = other.m_ConfID;
	m_updateCounter = other.m_updateCounter;
		
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CGetProfileSpecific::~CGetProfileSpecific()
{

}

/////////////////////////////////////////////////////////////////////////////
int CGetProfileSpecific::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pResNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pResNode,"ID",&m_ConfID,_0_TO_DWORD);
	
	return nStatus;

}

///////////////////////////////////////////////////////////////////////////
void CGetProfileSpecific::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	STATUS status=STATUS_OK;
	
	CCommRes * pCommRes = ::GetpProfilesDB()->GetCurrentRsrv(m_ConfID);
	if(pCommRes)
		pCommRes->SerializeXml(pFatherNode,m_updateCounter); 
	
	POBJDELETE(pCommRes);
}

//////////////////////////////////////////////////////////////////////////
DWORD  CGetProfileSpecific::GetConfID()const
{
	return m_ConfID;
}




