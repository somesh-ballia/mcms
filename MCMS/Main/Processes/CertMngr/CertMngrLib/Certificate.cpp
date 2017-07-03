// Certificate.cpp: implementation of the CCertificate class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//14/12/06		Judith M			  Class for Certificate
//========   ==============   ===========================================

#include "Certificate.h"
#include "StatusesGeneral.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"
#include <string.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCertificate::CCertificate()
{
	m_strCertificate = "";
}
/////////////////////////////////////////////////////////////////////////////
CCertificate::CCertificate(const CCertificate &other) : CSerializeObject(other)
{
	*this = other;
}
/////////////////////////////////////////////////////////////////////////////
CCertificate& CCertificate::operator = (const CCertificate &other)
{
	m_strCertificate = other.m_strCertificate;
	m_strServiceName = other.m_strServiceName;
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCertificate::~CCertificate()
{
}

///////////////////////////////////////////////////////////////////////////
void CCertificate::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
}

/////////////////////////////////////////////////////////////////////////////
int CCertificate::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;	
	
	GET_VALIDATE_CHILD(pActionNode,"CERTIFICATE", m_strCertificate, FIFTY_LINE_BUFFER_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"SERVICE_NAME", m_strServiceName, NET_SERVICE_PROVIDER_NAME_LENGTH);
	
	//TBD - JUD to remove
	if (m_strServiceName == "")
		m_strServiceName = "Default IP Service";

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
const std::string CCertificate::GetCertificate()
{
	return m_strCertificate;
}
/////////////////////////////////////////////////////////////////////////////
const std::string CCertificate::GetServiceName()
{
	return m_strServiceName;
}
//////////////////////////////////////////////////////////////////////

CCertificateStatus::CCertificateStatus(std::string uploadStatus)
{
	m_uploadStatus = uploadStatus;
}

CCertificateStatus::CCertificateStatus(const CCertificateStatus &other) : CSerializeObject()
{
	this->m_uploadStatus = other.m_uploadStatus;
}

CCertificateStatus& CCertificateStatus::operator= (const CCertificateStatus &other)
{
	this->m_uploadStatus = other.m_uploadStatus;
	return *this;
}

void CCertificateStatus::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	//CXMLDOMElement* node = pActionsNode->AddChildNode("FINISH_UPLOAD_CERTIFICATE");
	 //node->AddChildNode();
	pActionsNode->AddChildNode("CERTIFICATE_UPLOAD_STATUS",      m_uploadStatus);
}

