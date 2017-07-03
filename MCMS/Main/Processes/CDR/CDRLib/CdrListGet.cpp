#include "CdrListGet.h"
#include "CDRShort.h"
#include "StatusesGeneral.h"
#include "psosxml.h"


CCdrListGet::CCdrListGet(CCdrList *cdrList)
:m_CdrList(cdrList)
{
}

CCdrListGet::CCdrListGet()
:m_CdrList(NULL)
{
	
}

CCdrListGet::~CCdrListGet()
{
}

CSerializeObject* CCdrListGet::Clone()
{
	CCdrListGet *enstance = new CCdrListGet(m_CdrList);
	return enstance;
}

void CCdrListGet::SerializeXml(CXMLDOMElement *& pFatherNode) const
{
	m_CdrList->SerializeXml(pFatherNode);
}

int CCdrListGet::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	return STATUS_OK;
}
