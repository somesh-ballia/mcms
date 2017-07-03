// FileListGet.cpp: implementation of the CFileListGet class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Anatg			  Class for getting files List 
//========   ==============   =====================================================================

#include "NStream.h"
#include "FileListGet.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "StringsMaps.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFileListGet::CFileListGet()
{
	m_Path[0] = '\0';
	m_FileList = new CFileList;
	
}
/////////////////////////////////////////////////////////////////////////////
CFileListGet& CFileListGet::operator = (const CFileListGet &other)
{
	strncpy( m_Path, other.m_Path, MAX_FULL_PATH_LEN );
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CFileListGet::~CFileListGet()
{
	POBJDELETE (m_FileList);
}


///////////////////////////////////////////////////////////////////////////
void CFileListGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	m_FileList->SerializeXml(pFatherNode);
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CFileListGet::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char * strAction)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pActionNode, "PATH", m_Path, _0_TO_MAX_FULL_PATH_LENGTH);
	return nStatus;
}

//////////////////////////////////////////////////////////////////////////
char* CFileListGet::GetPath()
{
	return m_Path;
}


//////////////////////////////////////////////////////////////////////////
CFileList* CFileListGet::GetFileList()
{
	return m_FileList;
}




