// OperListGet.cpp: implementation of the COperListGet class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//25/08/05		Yuriy W			  Class for Get XML Operator List
//========   ==============   =====================================================================

#include "OperListGet.h"
#include "GlobalDataAccess.h"
#include "psosxml.h"
#include "OperatorList.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
COperListGet::COperListGet()
{
	m_nReqAuthorization = ANONYMOUS;
	m_updateCounter = 0;
}
/////////////////////////////////////////////////////////////////////////////
COperListGet::COperListGet(const COperListGet &other,WORD nReqAuthorization/*=ANONYMOUS*/) : CSerializeObject(other)
{
	*this = other;
	m_nReqAuthorization = nReqAuthorization;
}
/////////////////////////////////////////////////////////////////////////////
COperListGet& COperListGet::operator = (const COperListGet &other)
{
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
COperListGet::~COperListGet()
{

}

///////////////////////////////////////////////////////////////////////////
void COperListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	GetOperatorList()->SerializeXml(pActionsNode,m_updateCounter,m_nReqAuthorization);
}

/////////////////////////////////////////////////////////////////////////////
int COperListGet::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);

	return nStatus;
}
