// CommFaultsDBGet.cpp: interface for the CCommFaultsDBGet class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//19/10/05		Vasily			  Class for Get XML Faults List
//========   ==============   =====================================================================

#include "StatusesGeneral.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "FaultsProcess.h"
#include "HlogList.h"
#include "CommFaultsDBGet.h"
#include "ApiStatuses.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCommFaultsDBGet::CCommFaultsDBGet()
{
	m_id = 0;
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CCommFaultsDBGet& CCommFaultsDBGet::operator = (const CCommFaultsDBGet &other)
{
	m_id = other.m_id;
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCommFaultsDBGet::~CCommFaultsDBGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CCommFaultsDBGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CHlogList* pFaultsDB = ((CFaultsProcess*)CFaultsProcess::GetProcess())->GetFaultsListDB();

	if ( pFaultsDB )
	{
		//pFaultsDB->SerializeXml(pActionsNode, m_updateCounter , m_id);
		pFaultsDB->SerializeXmlAllVector(pActionsNode, m_updateCounter , m_id);
	}
}

/////////////////////////////////////////////////////////////////////////////
int CCommFaultsDBGet::DeSerializeXml(CXMLDOMElement* pFaultListNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pFaultListNode,"ID",&m_id,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pFaultListNode,"OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD);

	return STATUS_OK;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCommFaultsShortDBGet::CCommFaultsShortDBGet()
{
	m_id = 0;
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CCommFaultsShortDBGet& CCommFaultsShortDBGet::operator = (const CCommFaultsShortDBGet &other)
{
	m_id = other.m_id;
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCommFaultsShortDBGet::~CCommFaultsShortDBGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CCommFaultsShortDBGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CHlogList* pFaultsDB = ((CFaultsProcess*)CFaultsProcess::GetProcess())->GetFaultsShortListDB();

	if ( pFaultsDB )
		//pFaultsDB->SerializeXml(pActionsNode, m_updateCounter, m_id);
		pFaultsDB->SerializeXmlAllVector(pActionsNode, m_updateCounter, m_id);/////////////////////////
}

/////////////////////////////////////////////////////////////////////////////
int CCommFaultsShortDBGet::DeSerializeXml(CXMLDOMElement* pFaultListNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pFaultListNode,"ID",&m_id,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pFaultListNode,"OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD);

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
