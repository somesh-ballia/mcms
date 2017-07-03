// OperListGet.cpp: implementation of the CAudibleAlarmGet class.
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
#include "AudibleAlarmGet.h"
#include "Trace.h"
#include "TraceStream.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CAudibleAlarmGet::CAudibleAlarmGet()
{
	m_updateCounter = 0;
	m_user_name = "";
}
/////////////////////////////////////////////////////////////////////////////
CAudibleAlarmGet::CAudibleAlarmGet(const CAudibleAlarmGet &other) : CSerializeObject(other)
{
	*this = other;
}
/////////////////////////////////////////////////////////////////////////////
CAudibleAlarmGet& CAudibleAlarmGet::operator = (const CAudibleAlarmGet &other)
{
	m_updateCounter = other.m_updateCounter;
	m_user_name   =   other.m_user_name;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CAudibleAlarmGet::~CAudibleAlarmGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CAudibleAlarmGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	STATUS status = STATUS_OK;
	COperatorList* pOperList = GetOperatorList();
	FTRACESTR(eLevelError) << "\n CACAudibleAlarmGet::SerializeXml user name=" << m_user_name << "...";
	COperator* pOperInDB =  pOperList->GetOperatorByLogin(m_user_name);
	if (pOperInDB != NULL)
	{
		CAudibleAlarm* pAudibleAlarm = new CAudibleAlarm();
		*pAudibleAlarm = *(pOperInDB->GetAudibleAlarm());

		pAudibleAlarm->SerializeXml(pActionsNode);
		PDELETE(pAudibleAlarm);
	}

}

/////////////////////////////////////////////////////////////////////////////
int CAudibleAlarmGet::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;

	//GET_CHILD_NODE(pActionNode, "USER_NAME", pAudibleAlarmUserName);

	GET_VALIDATE_CHILD(pActionNode,"USER_NAME",m_user_name,_0_TO_OPERATOR_NAME_LENGTH);
	FTRACESTR(eLevelError) << "\n CAudibleAlarmGet::DeSerializeXml user name=" << m_user_name;

	return nStatus;
}
