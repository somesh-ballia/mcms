
#include "RtmIsdnServiceName.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "InitCommonStrings.h"
#include "psosxml.h"



//////////////////////////////////////////////////////////////////////////////
CRtmIsdnServiceName::CRtmIsdnServiceName()
{
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnServiceName::~CRtmIsdnServiceName()
{

}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnServiceName::CRtmIsdnServiceName(const CRtmIsdnServiceName &other)
:CSerializeObject(other)
{
	m_name = other.m_name;
}


///////////////////////////////////////////////////////////////////////////
void CRtmIsdnServiceName::SerializeXml(CXMLDOMElement*& thisNode) const
{
		
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnServiceName::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	
	char buff[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN];
	memset(buff, 0, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN);

	GET_VALIDATE_CHILD(pNode,"NAME", buff, _0_TO_RTM_ISDN_SERVICE_PROVIDER_NAME_LENGTH);

	m_name = buff;
	
	return nStatus;
}

///////////////////////////////////////////////////////////////////////////////////////////////
const char* CRtmIsdnServiceName::GetName() const
{
	return m_name.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnServiceName::SetName(const char *serviceName)
{
	m_name = serviceName;
}
