#include "CommResDBGet.h"
#include "CommResDB.h"
#include "psosxml.h"
#include "InitCommonStrings.h"

extern CCommResDB * GetpProfilesDB();

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCommResDBGet::CCommResDBGet()
{
	
}
/////////////////////////////////////////////////////////////////////
CCommResDBGet::CCommResDBGet(const CCommResDBGet &other)
:CSerializeObject(other)
{
	
}
/////////////////////////////////////////////////////////////////////////////
CCommResDBGet& CCommResDBGet::operator = (const CCommResDBGet &other)
{
	m_updateCounter = other.m_updateCounter;
	return *this;
}
/////////////////////////////////////////////////////////////////////////////
CCommResDBGet::~CCommResDBGet()
{

}
/////////////////////////////////////////////////////////////////////////////
int CCommResDBGet::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int nStatus = STATUS_OK;
	GET_VALIDATE_CHILD(pResNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	return nStatus;
}
///////////////////////////////////////////////////////////////////////////
void CCommResDBGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	if(::GetpProfilesDB() != NULL)
		(::GetpProfilesDB())->SerializeXml(pFatherNode,m_updateCounter); 
}

//////////////////////////////////////////////////////////////////////////

