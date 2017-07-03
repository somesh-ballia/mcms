#include "CommResRsrvDBGet.h"
#include "CRsrvDB.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "HelperFuncs.h"
#include "ApiStatuses.h"
#include "Reservator.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCommResRsrvDBGet::CCommResRsrvDBGet()
{

}
/////////////////////////////////////////////////////////////////////
CCommResRsrvDBGet::CCommResRsrvDBGet(const CCommResRsrvDBGet &other)
:CSerializeObject(other)
{

}
/////////////////////////////////////////////////////////////////////////////
CCommResRsrvDBGet& CCommResRsrvDBGet::operator = (const CCommResRsrvDBGet &other)
{
	m_updateCounter = other.m_updateCounter;
	return *this;
}
/////////////////////////////////////////////////////////////////////////////
CCommResRsrvDBGet::~CCommResRsrvDBGet()
{

}
/////////////////////////////////////////////////////////////////////////////
int CCommResRsrvDBGet::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{
	int nStatus = STATUS_OK;
	GET_VALIDATE_CHILD(pResNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	return nStatus;
}
///////////////////////////////////////////////////////////////////////////
void CCommResRsrvDBGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CReservator* pReservator = CHelperFuncs::GetReservator();
    PASSERT_AND_RETURN(pReservator == NULL);
    if( pReservator->IsInternalReservator() == FALSE )
       return;

    CRsrvDB* pRsrvDb = CHelperFuncs::GetRsrvDB();
	if( pRsrvDb != NULL )
       pRsrvDb->SerializeXml(pFatherNode,m_updateCounter);
}

//////////////////////////////////////////////////////////////////////////


