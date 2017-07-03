
#include "RtmIsdnPhoneNumberRangeSet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "psosxml.h"
#include "ApiStatuses.h"


//////////////////////////////////////////////////////////////////////////////
CRtmIsdnPhoneNumberRangeSet::CRtmIsdnPhoneNumberRangeSet()
{
	m_pPhoneNumRange = NULL;
}


/////////////////////////////////////////////////////////////////////////////
CRtmIsdnPhoneNumberRangeSet::~CRtmIsdnPhoneNumberRangeSet()
{
	PDELETE(m_pPhoneNumRange);
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnPhoneNumberRangeSet::CRtmIsdnPhoneNumberRangeSet(const CRtmIsdnPhoneNumberRangeSet &other)
:CSerializeObject(other)
{
	m_serviceName = other.m_serviceName;

	if( NULL == other.m_pPhoneNumRange)
		m_pPhoneNumRange = NULL;
	else
		m_pPhoneNumRange = new CRtmIsdnPhoneNumberRange(*other.m_pPhoneNumRange);  
}


///////////////////////////////////////////////////////////////////////////
void CRtmIsdnPhoneNumberRangeSet::SerializeXml(CXMLDOMElement*& thisNode) const
{
		
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnPhoneNumberRangeSet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	
	char buff[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN];
	memset(buff, 0, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN);
	GET_VALIDATE_CHILD(pNode,"NAME", buff, _0_TO_RTM_ISDN_SERVICE_PROVIDER_NAME_LENGTH);
	m_serviceName = buff;
	
	CXMLDOMElement *pTempNode = NULL;

	GET_FIRST_CHILD_NODE(pNode,"NET_PHONE",pTempNode);
	if (pTempNode)
	{
		CRtmIsdnPhoneNumberRange* pPhoneNum = new CRtmIsdnPhoneNumberRange;
		nStatus = pPhoneNum->DeSerializeXml(pTempNode, pszError,"");

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pPhoneNum);
			return nStatus;
		}

		POBJDELETE(m_pPhoneNumRange);
		m_pPhoneNumRange = pPhoneNum;
	}

	return nStatus;
}

///////////////////////////////////////////////////////////////////////////////////////////////
const char* CRtmIsdnPhoneNumberRangeSet::GetServiceName() const
{
	return m_serviceName.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnPhoneNumberRangeSet::SetServiceName(const char *serviceName)
{
	m_serviceName = serviceName;
}

///////////////////////////////////////////////////////////////////////////////////////////////
const char* CRtmIsdnPhoneNumberRangeSet::GetFirstPhoneNumber() const
{
	return (char*)(m_pPhoneNumRange->GetFirstPhoneNumber());
}

///////////////////////////////////////////////////////////////////////////////////////////////
CRtmIsdnPhoneNumberRange* CRtmIsdnPhoneNumberRangeSet::GetPhoneNumRange() const
{
	return m_pPhoneNumRange;
}
