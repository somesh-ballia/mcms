// OperListGet.cpp: implementation of the COperListGet class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//25/08/05		Yuriy W			  Class for Get XML Operator List
//========   ==============   =====================================================================

#include "ConnectionListGet.h"
#include "psosxml.h"
#include "ApacheModuleEngine.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CConnectionListGet::CConnectionListGet()
{
	m_updateCounter = 0;
}
/////////////////////////////////////////////////////////////////////////////
CConnectionListGet::CConnectionListGet(const CConnectionListGet &other) : CSerializeObject(other)
{
	*this = other;
}
/////////////////////////////////////////////////////////////////////////////
CConnectionListGet& CConnectionListGet::operator = (const CConnectionListGet &other)
{
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CConnectionListGet::~CConnectionListGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CConnectionListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CApacheModuleEngine::GetConnectionList()->SerializeXml(pActionsNode,m_updateCounter);
}

/////////////////////////////////////////////////////////////////////////////
int CConnectionListGet::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pActionNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
		
	return nStatus;
}
