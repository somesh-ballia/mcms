#include "GetMRSpecific.h"
#include "CommRes.h"
#include "StatusesGeneral.h"
#include "psosxml.h"
#include "CommResDB.h"

extern CCommResDB * GetpMeetingRoomDB();
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGetMRSpecific::CGetMRSpecific():
m_MRID(0xFFFFFFFF)
{
	
}

CGetMRSpecific::CGetMRSpecific(const CGetMRSpecific &other):
CSerializeObject(other),
m_MRID(other.m_MRID)
{

}
 
/////////////////////////////////////////////////////////////////////////////
CGetMRSpecific& CGetMRSpecific::operator = (const CGetMRSpecific  &other)
{
	m_MRID = other.m_MRID;
	m_updateCounter = other.m_updateCounter;
		
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CGetMRSpecific::~CGetMRSpecific()
{

}

/////////////////////////////////////////////////////////////////////////////
int CGetMRSpecific::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pResNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pResNode,"ID",&m_MRID,_0_TO_DWORD);
	
	return nStatus;

}

///////////////////////////////////////////////////////////////////////////
void CGetMRSpecific::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	STATUS status=STATUS_OK;
	
	CCommRes * pCommRes = ::GetpMeetingRoomDB()->GetCurrentRsrv(m_MRID);
	if(pCommRes)
		pCommRes->SerializeXml(pFatherNode,m_updateCounter); 
	
	POBJDELETE(pCommRes);
}

//////////////////////////////////////////////////////////////////////////
DWORD  CGetMRSpecific::GetMRID()const
{
	return m_MRID;
}




