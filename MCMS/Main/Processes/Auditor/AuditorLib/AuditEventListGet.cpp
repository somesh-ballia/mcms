#include "AuditEventListGet.h"
#include "AuditorProcess.h"
#include "AuditEventContainer.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"


CAuditEventListGet::CAuditEventListGet()
{
}

CAuditEventListGet::~CAuditEventListGet()
{
    
}

void CAuditEventListGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CAuditorProcess *pProcess = dynamic_cast<CAuditorProcess*>(CProcessBase::GetProcess());
    pProcess->GetAuditEventContainer()->SerializeXml(pFatherNode, m_updateCounter);
}

int CAuditEventListGet::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    int nStatus = STATUS_OK;
    
	GET_VALIDATE_CHILD(pActionNode,"OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD);

	return STATUS_OK;
}
