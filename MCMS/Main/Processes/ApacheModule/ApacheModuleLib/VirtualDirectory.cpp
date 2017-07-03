// VirtualDirectory.cpp: implementation of the CVirtualDirectory class.
//
//////////////////////////////////////////////////////////////////////

#include <string.h>
#include "VirtualDirectory.h"
#include "ProcessBase.h"

//////////////////////////////////////////////////////////////////////
// CVirtualDirectory class
//////////////////////////////////////////////////////////////////////


CVirtualDirectory::CVirtualDirectory()
{
	m_Access = -1;
	m_pVirtualDirectory = NULL;
	m_pPhysicalDirectory = NULL;
    m_isAuditable = true;
    m_InformProcess = eProcessTypeInvalid;
    m_CreateDir = -1;
    
}

CVirtualDirectory::~CVirtualDirectory()
{
	if (m_pVirtualDirectory)
		delete[] m_pVirtualDirectory;
	if (m_pPhysicalDirectory)
		delete[] m_pPhysicalDirectory;
}

void CVirtualDirectory::SetPhysicalPath(char* pszVirtual, const char* pszPhysicalPath)
{
	char* virtualDir;
	virtualDir = pszVirtual;
	int VirtualDirectoryLen = strlen(pszVirtual)+1;

	m_pVirtualDirectory = new char[VirtualDirectoryLen];
	
	if (virtualDir[VirtualDirectoryLen-2]=='/')
		virtualDir[VirtualDirectoryLen-2]='\0';
	if (virtualDir[0]=='/')
		virtualDir++;
	
	strcpy(m_pVirtualDirectory, virtualDir);
	
	int PhysicalDirectoryLen = strlen(pszPhysicalPath)+1;
	m_pPhysicalDirectory = new char[PhysicalDirectoryLen];
	strcpy(m_pPhysicalDirectory, pszPhysicalPath);
}

char* CVirtualDirectory::GetVirtualPath()
{
	return m_pVirtualDirectory;
}

char* CVirtualDirectory::GetPhysicalPath()
{
	return m_pPhysicalDirectory;
}

void CVirtualDirectory::SetDirectoryAccess(const int access)
{
	m_Access = access;
}

int CVirtualDirectory::GetDirectoryAccess()
{
	return m_Access;
}

void CVirtualDirectory::SetCreateDirectory(const int create_dir)
{
	m_CreateDir = create_dir;
}

int CVirtualDirectory::GetCreateDirectory()
{
	return m_CreateDir;
}

void CVirtualDirectory::SetAuditability(bool val)
{
    m_isAuditable = val;
}

bool CVirtualDirectory::GetAuditability()const
{
    return m_isAuditable;
}

void CVirtualDirectory::SetInformProcess( eProcessType processType)
{
    m_InformProcess = processType;
}

void CVirtualDirectory::SetInformProcess( const std::string & process_name)
{
    SetInformProcess(CProcessBase::GetProcessValueByString(process_name.c_str()));
}

eProcessType CVirtualDirectory::GetInformProcess() const
{
    return m_InformProcess;
    
}
