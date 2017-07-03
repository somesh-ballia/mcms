/*
 * CertificateGet.cpp
 */


#include "CertificateGet.h"

#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "psosxml.h"
#include "TraceStream.h"
#include "ApiStatuses.h"
#include "CertificateInfo.h"
#include "CertificateList.h"
#include "CertMngrProcess.h"

//////////////////////////////////////////////////////////////////////
CCertificateGet::CCertificateGet(void) :
    m_type(eCertificateTypeUnknown)
{ }

///////////////////////////////////////////////////////////////////////////
// virtual
const char* CCertificateGet::NameOf(void) const
{
    return GetCompileType();
}

///////////////////////////////////////////////////////////////////////////
// virtual
CSerializeObject* CCertificateGet::Clone(void)
{
    CCertificateGet* ret = new CCertificateGet;

    ret->m_issuer = m_issuer;
    ret->m_serial = m_serial;
    ret->m_type   = m_type;

    return ret;
}

void CCertificateGet::SetListType(eCertificateType type)
{
    m_type = type;
}

///////////////////////////////////////////////////////////////////////////
const char* CCertificateGet::GetSerialNumber(void) const
{
    return m_serial.c_str();
}

///////////////////////////////////////////////////////////////////////////
const char* CCertificateGet::GetIssuerName(void) const
{
    return m_issuer.c_str();
}

///////////////////////////////////////////////////////////////////////////
void CCertificateGet::SerializeXml(CXMLDOMElement*& parent) const
{
    CCertMngrProcess* proc = ((CCertMngrProcess*)CCertMngrProcess::GetProcess());
    PASSERTMSG_AND_RETURN(!proc, "Unable to get current process");

	const CCertificateList* list = proc->ReadList(m_type);
	PASSERTMSG_AND_RETURN(!list, "Unable to get certificate list");
	
	const CCertificateInfo* cer = list->GetCertificate(m_issuer.c_str(),
                                                        m_serial.c_str());
	PASSERTMSG_AND_RETURN(!cer, "Unable to get certificate details");

    CXMLDOMElement* node = parent->AddChildNode("CERTIFICATE_DETAILS");
    PASSERTMSG_AND_RETURN(!node, "Unable to add a node to XML");

    node->AddChildNode("CERTIFICATE_DETAILS_STRING", cer->GetDetails().c_str());
}

/////////////////////////////////////////////////////////////////////////////
int CCertificateGet::DeSerializeXml(CXMLDOMElement* pActionNode,
                                    char* pszError, const char* strAction)
{
    int nStatus;

	GET_VALIDATE_CHILD(pActionNode, "SERIAL_NUMBER",
                       m_serial, ONE_LINE_BUFFER_LENGTH);
	GET_VALIDATE_CHILD(pActionNode, "ISSUED_BY",
	                   m_issuer, ONE_LINE_BUFFER_LENGTH);
	
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
bool CCertificateGet::IsCertificateExist(void) const
{
    CCertMngrProcess* proc = ((CCertMngrProcess*)CCertMngrProcess::GetProcess());
    PASSERTMSG_AND_RETURN_VALUE(!proc, "Unable to get current process", false);

    const CCertificateList* list = proc->ReadList(m_type);
    PASSERTMSG_AND_RETURN_VALUE(!list, "Unable to get certificate list", false);

    const CCertificateInfo* info = list->GetCertificate(m_issuer.c_str(),
                                                        m_serial.c_str());

	return NULL != info;
}
