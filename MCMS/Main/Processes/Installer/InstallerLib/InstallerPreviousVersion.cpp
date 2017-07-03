#include "ApiStatuses.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InstallerPreviousVersion.h"

CInstallPreviousVersion::CInstallPreviousVersion()
{
	m_VersionType = eVersionTypeFallback;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CInstallPreviousVersion::~CInstallPreviousVersion()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CInstallPreviousVersion::SerializeXml(CXMLDOMElement*& thisNode) const
{
	PASSERTMSG(TRUE, "CInstallPreviousVersion::SerializeXml should not be called");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int	CInstallPreviousVersion::DeSerializeXml(CXMLDOMElement *pNode,char *pszError, const char* action)
{
	STATUS nStatus = STATUS_OK;
	
	BYTE tmp = (BYTE)m_VersionType;
	GET_VALIDATE_CHILD(pNode, "VERSION_TYPE", &tmp, VERSION_TYPE_ENUM);
	m_VersionType = (eVersionType)tmp;
	
	return nStatus;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CInstallPreviousVersion& CInstallPreviousVersion::operator=(const CInstallPreviousVersion& other)
{
	m_VersionType = other.m_VersionType;

	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
