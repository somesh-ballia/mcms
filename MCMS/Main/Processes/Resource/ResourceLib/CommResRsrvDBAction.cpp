#include "CommResRsrvDBAction.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCommResRsrvDBAction::CCommResRsrvDBAction()
:m_confID(0xFFFFFFFF)
{
	
}

CCommResRsrvDBAction::CCommResRsrvDBAction(const CCommResRsrvDBAction &other)
:CSerializeObject(other),
m_confID(0xFFFFFFFF)
{
}
/////////////////////////////////////////////////////////////////////////////
CCommResRsrvDBAction& CCommResRsrvDBAction::operator = (const CCommResRsrvDBAction &other)
{
	CSerializeObject::operator =(other);
	m_confID = other.m_confID;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCommResRsrvDBAction::~CCommResRsrvDBAction()
{
}

/////////////////////////////////////////////////////////////////////////////
int CCommResRsrvDBAction::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{
	STATUS nStatus=STATUS_OK;
	
	std::string stringAction(strAction);
	if(!strncmp("CANCEL_REPEATED",strAction,15))
	{
		GET_VALIDATE_CHILD(pResNode,"REPEATED_ID",&m_confID,_0_TO_DWORD);
	}
	else
	{
		GET_VALIDATE_CHILD(pResNode,"ID",&m_confID,_0_TO_DWORD);
	}
	
	return nStatus;
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CCommResRsrvDBAction::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	
}






