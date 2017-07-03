// NewOperator.cpp: implementation of the CNewOperator class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//25/08/05		Yuriy W			  Class for Get XML Operator List
//========   ==============   =====================================================================

#include "NewOperator.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "Operator.h"
#include "ApiStatuses.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ProcessBase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CNewOperator::CNewOperator()
{
	m_pOperator = NULL;
}
//exactly the same as the constructor above + an optiont to init the internal Member
CNewOperator::CNewOperator(std::string user, std::string pwd)
{
	m_pOperator = new COperator();
	m_pOperator->SetLogin(user);
	m_pOperator->SetPassword(pwd);
}
/////////////////////////////////////////////////////////////////////////////
CNewOperator::CNewOperator(const CNewOperator &other) : CSerializeObject(other)
{
	m_pOperator = NULL;

	if(other.m_pOperator)
	{
		m_pOperator = new COperator(*(other.m_pOperator));

		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		std::string key = CFG_KEY_JITC_MODE;
		BOOL bJitcMode;
		sysConfig->GetBOOLDataByKey(key, bJitcMode);

		if(bJitcMode)
			m_pOperator->SetForceChangePwd(TRUE);
	}
}
/////////////////////////////////////////////////////////////////////////////
CNewOperator::~CNewOperator()
{
	if(m_pOperator)
		POBJDELETE(m_pOperator);
}

///////////////////////////////////////////////////////////////////////////
void CNewOperator::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
}

/////////////////////////////////////////////////////////////////////////////
int CNewOperator::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pOperatorNode = NULL;

	GET_MANDATORY_CHILD_NODE(pActionNode, "OPERATOR", pOperatorNode);

	m_pOperator = new COperator;
	nStatus = m_pOperator->DeSerializeXml(pOperatorNode,pszError);

	return nStatus;
}
