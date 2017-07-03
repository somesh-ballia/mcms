#include "ConfTemplateDBGet.h"
#include "CommResDB.h"
#include "psosxml.h"
#include "InitCommonStrings.h"

extern CCommResDB * GetpConfTemplateDB();

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CConfTemplateDBGet::CConfTemplateDBGet()
{
	
}

CConfTemplateDBGet::CConfTemplateDBGet(const CConfTemplateDBGet &other)
:CSerializeObject(other)
{
	
}

/////////////////////////////////////////////////////////////////////////////
CConfTemplateDBGet& CConfTemplateDBGet::operator = (const CConfTemplateDBGet &other)
{
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CConfTemplateDBGet::~CConfTemplateDBGet()
{

}

/////////////////////////////////////////////////////////////////////////////
int CConfTemplateDBGet::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pResNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	
	return nStatus;
}
///////////////////////////////////////////////////////////////////////////
void CConfTemplateDBGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	if(::GetpConfTemplateDB() != NULL)
		(::GetpConfTemplateDB())->SerializeXml(pFatherNode,m_updateCounter); 
}

//////////////////////////////////////////////////////////////////////////


