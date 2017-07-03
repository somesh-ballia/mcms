#include "CdrSettingsGet.h"

#include "psosxml.h"
#include "CDRDetal.h"
#include "CDRShort.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"

CCdrSettingsGet::CCdrSettingsGet()
{
	m_pProcess = (CCDRProcess*)CCDRProcess::GetProcess();
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CCdrSettingsGet& CCdrSettingsGet::operator = (const CCdrSettingsGet &other)
{
	if (&other == this) 
		return *this;
	
	m_pProcess = (CCDRProcess*)CCDRProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;
	m_apiFormat = other.m_apiFormat;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCdrSettingsGet::~CCdrSettingsGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CCdrSettingsGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{

}

////////////////////////////////////////////////////////////////////////////////////////////////
int CCdrSettingsGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	WORD tmp = 0;
	GET_VALIDATE_CHILD(pNode,"API_FORMAT",&tmp,API_FORMAT_TYPE_ENUM);
	if(tmp)
	{
		m_apiFormat = (eApiFormat)tmp;
	}
	return STATUS_OK;
}
