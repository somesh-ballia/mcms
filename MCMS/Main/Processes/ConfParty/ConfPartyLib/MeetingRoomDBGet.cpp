#include "MeetingRoomDBGet.h"
#include "CommResDB.h"
#include "psosxml.h"
#include "InitCommonStrings.h"

extern CCommResDB * GetpMeetingRoomDB();

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMeetingRoomDBGet::CMeetingRoomDBGet()
{
	
}

CMeetingRoomDBGet::CMeetingRoomDBGet(const CMeetingRoomDBGet &other)
:CSerializeObject(other)
{
	
}

/////////////////////////////////////////////////////////////////////////////
CMeetingRoomDBGet& CMeetingRoomDBGet::operator = (const CMeetingRoomDBGet &other)
{
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMeetingRoomDBGet::~CMeetingRoomDBGet()
{

}

/////////////////////////////////////////////////////////////////////////////
int CMeetingRoomDBGet::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pResNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	
	return nStatus;
}
///////////////////////////////////////////////////////////////////////////
void CMeetingRoomDBGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	if(::GetpMeetingRoomDB() != NULL)
		(::GetpMeetingRoomDB())->SerializeXml(pFatherNode,m_updateCounter); 
}

//////////////////////////////////////////////////////////////////////////


