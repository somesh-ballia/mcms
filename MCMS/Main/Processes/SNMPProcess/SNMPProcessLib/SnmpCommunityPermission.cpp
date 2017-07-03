#include "SnmpCommunityPermission.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "StringsLen.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"

//////////////////////////////////////////////////////////////////////////
CSnmpCommunityPermission::CSnmpCommunityPermission()
{
    m_permission = 2;
}
//////////////////////////////////////////////////////////////////////////

CSnmpCommunityPermission::CSnmpCommunityPermission(const string &communityName, WORD permission)
{
	Init();
	m_communityName = communityName;
	m_permission = permission;
	
}

void CSnmpCommunityPermission::Init()
{
	m_communityName = "public";
	m_permission = 2;

}
void CSnmpCommunityPermission::InitDefaults()
{
	m_snmpV3Params.IniDefaults();
	Init();
}

//////////////////////////////////////////////////////////////////////////
CSnmpCommunityPermission::CSnmpCommunityPermission(const CSnmpCommunityPermission &other) :
	CSerializeObject(other)
{
    *this = other;
}

//////////////////////////////////////////////////////////////////////////
const char* CSnmpCommunityPermission::NameOf() const
{
    return "CSnmpCommunityPermission";
}

//////////////////////////////////////////////////////////////////////////
CSnmpCommunityPermission& CSnmpCommunityPermission::operator = (const CSnmpCommunityPermission& other)
{
    m_permission = other.m_permission;
    m_communityName = other.m_communityName;
    m_snmpV3Params = other.m_snmpV3Params;
    return *this;
}

//////////////////////////////////////////////////////////////////////////
CSnmpCommunityPermission::~CSnmpCommunityPermission()
{ }

// Virtual
CSerializeObject* CSnmpCommunityPermission::Clone(void)
{
	return new CSnmpCommunityPermission;
}

const string& CSnmpCommunityPermission::GetCommunityName(void) const
{
	return m_communityName;
}

//////////////////////////////////////////////////////////////////////////
void CSnmpCommunityPermission::SerializeXml(CXMLDOMElement*& pParentNode,bool isToEma)const
{
	CXMLDOMElement* pSnmpCommunityPermissionNode =
        pParentNode->AddChildNode("COMMUNITY_PERMISSION");

	pSnmpCommunityPermissionNode->AddChildNode("COMMUNITY_PERMISSION_SPECIFIC",
                                               m_permission,
                                               COMMUNITY_PERMISSION_ENUM);
    
	pSnmpCommunityPermissionNode->AddChildNode("COMMUNITY_NAME",
                                               m_communityName);

	m_snmpV3Params.SerializeXml(pSnmpCommunityPermissionNode,isToEma);
}

//////////////////////////////////////////////////////////////////////////
void CSnmpCommunityPermission::SerializeXml(CXMLDOMElement*& pParentNode)const
{
	CXMLDOMElement* pSnmpCommunityPermissionNode =
        pParentNode->AddChildNode("COMMUNITY_PERMISSION");

	pSnmpCommunityPermissionNode->AddChildNode("COMMUNITY_PERMISSION_SPECIFIC",
                                               m_permission,
                                               COMMUNITY_PERMISSION_ENUM);

	pSnmpCommunityPermissionNode->AddChildNode("COMMUNITY_NAME",
                                               m_communityName);

	m_snmpV3Params.SerializeXml(pSnmpCommunityPermissionNode);
}

/////////////////////////////////////////////////////////////////////////////
int CSnmpCommunityPermission::DeSerializeXml(CXMLDOMElement* pCommunityPermissionNode,
                                             char *pszError,
                                             const char* action)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pCommunityPermissionNode,
                       "COMMUNITY_PERMISSION_SPECIFIC",
                       &m_permission,
                       COMMUNITY_PERMISSION_ENUM);
    
	GET_VALIDATE_ASCII_CHILD(pCommunityPermissionNode,
                             "COMMUNITY_NAME",
                             m_communityName,
                             _0_TO_SNMP_STRING_LENGTH);

	if (STATUS_OK == nStatus)
		nStatus = m_snmpV3Params.DeSerializeXml(pCommunityPermissionNode,
												pszError, action);

	return nStatus;
}

const CSnmpV3SecurityParams& CSnmpCommunityPermission::GetSnmpV3Param(void) const
{
	return m_snmpV3Params;
}
