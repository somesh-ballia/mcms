// Rename.cpp: implementation of the CRename class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//25/08/05		Yuriy W			  Class for Renaming directories and files
//========   ==============   ============================================

#include "Rename.h"
#include "psosxml.h"
#include "ApacheModuleEngine.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CRename::CRename()
{
	m_strInitialVirtualPath = "";
	m_strNewVirtualPath = "";
}
/////////////////////////////////////////////////////////////////////////////
CRename::CRename(const CRename &other) : CSerializeObject(other)
{
	*this = other;
}
/////////////////////////////////////////////////////////////////////////////
CRename& CRename::operator = (const CRename &other)
{
	m_strInitialVirtualPath = other.m_strInitialVirtualPath;
	m_strNewVirtualPath = other.m_strNewVirtualPath;	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CRename::~CRename()
{
}

///////////////////////////////////////////////////////////////////////////
void CRename::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
}

/////////////////////////////////////////////////////////////////////////////
int CRename::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pActionNode,"PATH",m_strInitialVirtualPath,_1_TO_NEW_FILE_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"NEW_NAME",m_strNewVirtualPath,_1_TO_NEW_FILE_NAME_LENGTH);	
		
	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
std::string CRename::GetInitialVirtualPath()
{
	return m_strInitialVirtualPath;
}

/////////////////////////////////////////////////////////////////////////////	
std::string CRename::GetNewVirtualPath()
{
	return m_strNewVirtualPath;	
}
