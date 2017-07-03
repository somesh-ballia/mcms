// NewOperator.cpp: implementation of the CNewOperator class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//25/08/05		Yuriy W			  Class for Get XML Operator List
//========   ==============   =====================================================================

#include "RenameOperator.h"
#include "Operator.h"
#include "StatusesGeneral.h"
#include "psosxml.h"
#include "ApiStatuses.h"
#include "TraceStream.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CRenameOperator::CRenameOperator()
{
	m_pOperator = NULL;
}
/////////////////////////////////////////////////////////////////////////////
CRenameOperator::CRenameOperator(const CRenameOperator &other) : CSerializeObject(other)
{
	if (other.m_pOperator)
		m_pOperator = new COperator(*(other.m_pOperator));
	else
		m_pOperator = NULL;	
}
/////////////////////////////////////////////////////////////////////////////
CRenameOperator::~CRenameOperator()
{
	if(m_pOperator)
		POBJDELETE(m_pOperator);
}

///////////////////////////////////////////////////////////////////////////
void CRenameOperator::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
}

/////////////////////////////////////////////////////////////////////////////
int CRenameOperator::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	
	CXMLDOMElement *pOperatorNode = NULL;

	GET_MANDATORY_CHILD_NODE(pActionNode, "OPERATOR", pOperatorNode);

	m_pOperator = new COperator;
	nStatus = m_pOperator->DeSerializeXml(pOperatorNode,pszError,"RENAME_OPERATOR");

	return nStatus;
}
