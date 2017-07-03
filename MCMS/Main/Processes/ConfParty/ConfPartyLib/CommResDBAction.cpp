#include "CommResDBAction.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCommResDBAction::CCommResDBAction()
:m_confID(0xFFFFFFFF)
{
	m_transitEQ[0] = '\0';
	
}

CCommResDBAction::CCommResDBAction(const CCommResDBAction &other)
:CSerializeObject(other),
m_confID(0xFFFFFFFF)
{
	strncpy(m_transitEQ,other.m_transitEQ,H243_NAME_LEN);	
}
/////////////////////////////////////////////////////////////////////////////
CCommResDBAction& CCommResDBAction::operator = (const CCommResDBAction &other)
{
	CSerializeObject::operator =(other);
	m_confID = other.m_confID;
	strncpy(m_transitEQ,other.m_transitEQ,H243_NAME_LEN);
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCommResDBAction::~CCommResDBAction()
{
}

/////////////////////////////////////////////////////////////////////////////
int CCommResDBAction::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{
	STATUS nStatus=STATUS_OK;
	
	GET_VALIDATE_CHILD(pResNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pResNode,"ID",&m_confID,_0_TO_DWORD);

	
	std::string stringAction(strAction);	
	if(!strncmp("SET_DEFAULT_EQ",strAction,14))
	{
		GET_VALIDATE_CHILD(pResNode,"NAME",m_transitEQ,_0_TO_H243_NAME_LENGTH);
	}	
	else if(!strncmp("CANCEL_DEFAULT_EQ",strAction,17))
	{
		GET_VALIDATE_CHILD(pResNode,"NAME",m_transitEQ,_0_TO_H243_NAME_LENGTH);		
	}
	
	return nStatus;
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CCommResDBAction::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	pFatherNode->AddChildNode("OBJ_TOKEN",m_updateCounter,_0_TO_DWORD);
	pFatherNode->AddChildNode("ID",m_confID,_0_TO_DWORD);
	
}






