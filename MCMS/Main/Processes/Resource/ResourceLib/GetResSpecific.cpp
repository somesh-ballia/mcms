#include "GetResSpecific.h"
#include "CommResApi.h"
#include "StatusesGeneral.h"
#include "psosxml.h"
#include "CRsrvDB.h"
#include "HelperFuncs.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGetResSpecific::CGetResSpecific():
	m_ResID(0xFFFFFFFF)
{
	
}

CGetResSpecific::CGetResSpecific(const CGetResSpecific &other):
CSerializeObject(other),
m_ResID(other.m_ResID)
{

}
 
/////////////////////////////////////////////////////////////////////////////
CGetResSpecific& CGetResSpecific::operator = (const CGetResSpecific  &other)
{
	m_ResID = other.m_ResID;
	m_updateCounter = other.m_updateCounter;
		
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CGetResSpecific::~CGetResSpecific()
{

}

/////////////////////////////////////////////////////////////////////////////
int CGetResSpecific::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pResNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pResNode,"ID",&m_ResID,_0_TO_DWORD);
	
	return nStatus;

}

///////////////////////////////////////////////////////////////////////////
void CGetResSpecific::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	STATUS status=STATUS_OK;
	CRsrvDB* pRsrvDb = CHelperFuncs::GetRsrvDB();

	if (pRsrvDb != NULL)
	{
		CCommResApi*  pRes= pRsrvDb->GetRsrv(m_ResID);
		if(pRes)
			pRes->SerializeXml(pFatherNode,m_updateCounter);
	
		POBJDELETE(pRes);
	}
}

//////////////////////////////////////////////////////////////////////////
DWORD  CGetResSpecific::GetResID()const
{
	return m_ResID;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CShiftTime::CShiftTime()
{
	m_hour = 0;
	m_min = 0;
	m_sign = 0;
}
//////////////////////////////////////////////////////////////////////////

void CShiftTime::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	FTRACEINTO << " CShiftTime::SerializeXml - shouldn't be called!";
}
//////////////////////////////////////////////////////////////////////////

int CShiftTime::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement* pShiftNode;
	GET_CHILD_NODE(pActionNode,"SHIFT_RESERVATIONS", pShiftNode);
	if( pShiftNode )
	{
		GET_VALIDATE_MANDATORY_CHILD(pShiftNode,"HOUR",&m_hour,_0_TO_WORD);
		GET_VALIDATE_MANDATORY_CHILD(pShiftNode,"MINUTE",&m_min,_0_TO_WORD);
		GET_VALIDATE_MANDATORY_CHILD(pShiftNode,"SIGN",&m_sign,SIGN_TYPE);
	}
	return nStatus;
}





