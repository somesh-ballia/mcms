// CertificateListGet.cpp

#include "CertificateListGet.h"

#include "TraceStream.h"
#include "CertMngrProcess.h"
#include "psosxml.h"
#include "FaultsDefines.h"
#include "InitCommonStrings.h"

// Virtual
CSerializeObject* CCertificateListGet::Clone(void)
{
    return new CCertificateListGet(*this);
}

// Virtual
void CCertificateListGet::SerializeXml(CXMLDOMElement*& father) const
{
    CCertMngrProcess* proc = (CCertMngrProcess*)CCertMngrProcess::GetProcess();
    PASSERTMSG_AND_RETURN(NULL == proc, "Unable to continue");

    CAlarmableTask* task = (CAlarmableTask*)proc->GetCurrentTask();
    PASSERTMSG_AND_RETURN(NULL == task, "Unable to continue");

	CXMLDOMElement* cert = father->AddChildNode("CERTIFICATES_LIST_CONTENT");

	CXMLDOMElement* list = cert->AddChildNode("TRUSTED_CA_LIST");
	proc->ReadList(eCertificateTrust)->SerializeXml(list, m_updateCounter);

	list = cert->AddChildNode("PERSONAL_CERTIFICATE_LIST");

	const CCertificateList* trustedCertificateList = proc->ReadList(eOCS);

	proc->ReadList(eCertificatePersonal)->SerializeXml(list, m_updateCounter, trustedCertificateList);


	list = cert->AddChildNode("UNTRUSTED_CERTIFICATE_LIST");
	proc->ReadList(eCertificateRevocation)->SerializeXml(list, m_updateCounter);

	WORD aa = AA_CERTIFICATE_REPOSITORY_NOT_UPDATED;
	DWORD val = task->IsActiveAlarmExistByErrorCode(aa) ? YES : NO;
	cert->AddChildNode("REPOSITORY_CHANGED_BUT_NOT_UPDATED", val, _BOOL);
}

// Virtual
int CCertificateListGet::DeSerializeXml(CXMLDOMElement*, char*, const char*)
{
	return STATUS_OK;
}



