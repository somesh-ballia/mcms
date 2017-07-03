// FileListGet.cpp: implementation of the CFileListGet class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//25/08/05		Yuriy W			  Class for Get Directory Contents
//========   ==============   ====================================

#include "FileListGet.h"
#include "ApacheModuleEngine.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFileListGet::CFileListGet()
{
	m_strVirtualPath = "";
	m_strPhysicalPath = "";	
	m_pFileList = NULL;
}
/////////////////////////////////////////////////////////////////////////////
CFileListGet::CFileListGet(const CFileListGet &other) : CSerializeObject(other)
{
	m_pFileList = NULL;
	*this = other;
}
/////////////////////////////////////////////////////////////////////////////
CFileListGet& CFileListGet::operator = (const CFileListGet &other)
{
	m_strVirtualPath = other.m_strVirtualPath;
	m_strPhysicalPath = other.m_strPhysicalPath;
	
	if(other.m_pFileList != NULL)
	{
		POBJDELETE(m_pFileList);
		m_pFileList = new CFileList(*(other.m_pFileList));
	}
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CFileListGet::~CFileListGet()
{
	POBJDELETE(m_pFileList);
}

///////////////////////////////////////////////////////////////////////////
void CFileListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	if(m_pFileList != NULL)
		m_pFileList->SerializeXml(pActionsNode);
}

/////////////////////////////////////////////////////////////////////////////
int CFileListGet::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pActionNode,"PATH",m_strVirtualPath,_1_TO_NEW_FILE_NAME_LENGTH);
		
	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
int CFileListGet::FillFileList(WORD bNested)
{
	if(!CApacheModuleEngine::GetPhysicalPath(m_strVirtualPath,m_strPhysicalPath))
		return STATUS_NOT_FOUND;
		
	if(m_strPhysicalPath[m_strPhysicalPath.length()-1] != '/')
		m_strPhysicalPath += "/";
	
	POBJDELETE(m_pFileList);
	m_pFileList = new CFileList;
	
	int nRet = m_pFileList->FillFileList(m_strPhysicalPath.c_str(),bNested,TRUE);

	return nRet;
}
