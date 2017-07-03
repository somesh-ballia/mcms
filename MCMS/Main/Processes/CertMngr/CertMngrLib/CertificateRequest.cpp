// CertificateRequest.cpp: implementation of the CCertificateRequest class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//14/12/06		Judith M			  Class for Certificate Request
//========   ==============   ===========================================

#include "CertificateRequest.h"

#include "psosxml.h"
#include "StatusesGeneral.h"

#include "InitCommonStrings.h"
#include "ApiStatuses.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCertificateRequest::CCertificateRequest()
{
	m_strHashMethod = eHashMethodSHA1;
	m_strCommonName = "";
	m_strCountryName = "";
	m_strLocality = "";
	m_strOrganization = "";
	m_strOrganizationUnit = "";
	m_strStateOrProvince = "";
	m_strCertificateRequest = "";
	m_strServiceName = "";
	m_strSubjectAlternateName = "";
}
/////////////////////////////////////////////////////////////////////////////
CCertificateRequest::CCertificateRequest(const CCertificateRequest &other) : CSerializeObject(other)
{
	*this = other;
}
/////////////////////////////////////////////////////////////////////////////
CCertificateRequest& CCertificateRequest::operator = (const CCertificateRequest &other)
{
	m_strHashMethod = other.m_strHashMethod;
	m_strCommonName = other.m_strCommonName;
	m_strCountryName = other.m_strCountryName;
	m_strLocality = other.m_strLocality;
	m_strOrganization = other.m_strOrganization;
	m_strOrganizationUnit = other.m_strOrganizationUnit;
	m_strStateOrProvince = other.m_strStateOrProvince;
	m_strCertificateRequest = other.m_strCertificateRequest;
	m_strServiceName = other.m_strServiceName;
	m_strSubjectAlternateName = other.m_strSubjectAlternateName;
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCertificateRequest::~CCertificateRequest()
{
}

///////////////////////////////////////////////////////////////////////////
void CCertificateRequest::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	pActionsNode->AddChildNode("CERTIFICATE_REQUEST", m_strCertificateRequest.c_str());
}

/////////////////////////////////////////////////////////////////////////////
int CCertificateRequest::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement* pCertReqNode = NULL;
	
	GET_MANDATORY_CHILD_NODE(pActionNode, "CERTIFICATE_DATA", pCertReqNode);
	
	GET_VALIDATE_MANDATORY_CHILD(pCertReqNode,"COUNTRY_NAME", m_strCountryName, ONE_LINE_BUFFER_LENGTH);
	GET_VALIDATE_MANDATORY_CHILD(pCertReqNode,"STATE_OR_PROVINCE_NAME", m_strStateOrProvince, ONE_LINE_BUFFER_LENGTH);
	GET_VALIDATE_MANDATORY_CHILD(pCertReqNode,"LOCALITY_NAME", m_strLocality, ONE_LINE_BUFFER_LENGTH);
	GET_VALIDATE_MANDATORY_CHILD(pCertReqNode,"ORGANIZATION_NAME", m_strOrganization, ONE_LINE_BUFFER_LENGTH);
	GET_VALIDATE_MANDATORY_CHILD(pCertReqNode,"ORGANIZATION_UNIT_NAME", m_strOrganizationUnit, ONE_LINE_BUFFER_LENGTH);
	GET_VALIDATE_MANDATORY_CHILD(pCertReqNode,"COMMON_NAME", m_strCommonName, ONE_LINE_BUFFER_LENGTH);
	GET_VALIDATE_CHILD(pCertReqNode,"SUBJECT_ALTERNATIVE_NAME", m_strSubjectAlternateName, FIFTY_LINE_BUFFER_LENGTH);


	 DWORD tmp = 0;
	 GET_VALIDATE_CHILD(pCertReqNode, "HASH_METHOD", &tmp, HASH_METHOD_ENUM);
	 m_strHashMethod = (eHashMethod)tmp;
	
	GET_VALIDATE_CHILD(pActionNode,"SERVICE_NAME", m_strServiceName, NET_SERVICE_PROVIDER_NAME_LENGTH);
	
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
std::string CCertificateRequest::GetCountryName()
{
	return m_strCountryName;
}

/////////////////////////////////////////////////////////////////////////////
eHashMethod CCertificateRequest::GetHashMethod()
{
	return m_strHashMethod;
}
/////////////////////////////////////////////////////////////////////////////
std::string CCertificateRequest::GetCommonName()
{
	return m_strCommonName;
}
/////////////////////////////////////////////////////////////////////////////
std::string CCertificateRequest::GetLocality()
{
	return m_strLocality;
}
/////////////////////////////////////////////////////////////////////////////
std::string CCertificateRequest::GetOrganization()
{
	return m_strOrganization;
}
/////////////////////////////////////////////////////////////////////////////
std::string CCertificateRequest::GetOrganizationUnit()
{
	return m_strOrganizationUnit;
}
/////////////////////////////////////////////////////////////////////////////
std::string CCertificateRequest::GetStateOrProvince()
{
	return m_strStateOrProvince;
}
//////////////////////////////////////////////////////////////////////
void CCertificateRequest::SetCertificateRequest(std::string cert_req)
{
	m_strCertificateRequest = cert_req;
}
//////////////////////////////////////////////////////////////////////
std::string CCertificateRequest::GetServiceName()
{
	return m_strServiceName;
}

/////////////////////////////////////////////////////////////////////////////
std::string CCertificateRequest::GetSubjectAlternateName()
{
	return m_strSubjectAlternateName;
}
