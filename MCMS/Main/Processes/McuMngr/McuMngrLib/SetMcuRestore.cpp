#include "SetMcuRestore.h"
#include "psosxml.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"

CSetMcuRestore::CSetMcuRestore()
{
	m_McuRestoreType = eMcuRestoreNone;
}

CSetMcuRestore::~CSetMcuRestore()
{
}

void CSetMcuRestore::SerializeXml(CXMLDOMElement*& thisNode) const
{
	PASSERTMSG(TRUE, "CSetMcuRestore::SerializeXml should not be called");
}

int	CSetMcuRestore::DeSerializeXml(CXMLDOMElement *pNode,char *pszError, const char* action)
{
	STATUS nStatus = STATUS_OK;
	
	BYTE tmp = (BYTE)m_McuRestoreType;
	GET_VALIDATE_CHILD(pNode, "RESTORE_TYPE", &tmp, MCU_RESTORE_TYPE_ENUM);
	m_McuRestoreType = (eMcuRestoreType)tmp;
	
	return nStatus;
}
