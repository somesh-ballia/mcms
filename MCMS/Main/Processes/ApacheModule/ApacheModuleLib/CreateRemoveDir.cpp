// CreateRemoveDir.cpp: implementation of the CCreateRemoveDir class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//25/08/05		Yuriy W			  Class for Creating/Removing directories
//========   ==============   ===========================================

#include "CreateRemoveDir.h"
#include "psosxml.h"
#include "ApacheModuleEngine.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCreateRemoveDir::CCreateRemoveDir()
{
	m_strVirtualPath = "";
}
/////////////////////////////////////////////////////////////////////////////
CCreateRemoveDir::CCreateRemoveDir(const CCreateRemoveDir &other) : CSerializeObject(other)
{
	*this = other;
}
/////////////////////////////////////////////////////////////////////////////
CCreateRemoveDir& CCreateRemoveDir::operator = (const CCreateRemoveDir &other)
{
	m_strVirtualPath = other.m_strVirtualPath;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCreateRemoveDir::~CCreateRemoveDir()
{
}

///////////////////////////////////////////////////////////////////////////
void CCreateRemoveDir::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
}

/////////////////////////////////////////////////////////////////////////////
int CCreateRemoveDir::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;

    GET_VALIDATE_ASCII_CHILD(pActionNode,"PATH",m_strVirtualPath,_1_TO_NEW_FILE_NAME_LENGTH);
//	GET_VALIDATE_CHILD(pActionNode,"PATH",m_strVirtualPath,_1_TO_NEW_FILE_NAME_LENGTH);
	
	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
std::string CCreateRemoveDir::GetVirtualPath()
{
	return m_strVirtualPath;
}
