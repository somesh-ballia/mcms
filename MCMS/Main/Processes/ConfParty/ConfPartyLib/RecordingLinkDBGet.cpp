// RecordingLinkDBGet.cpp: implementation of the CRecordingLinkDBGet class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//1/7/07		Keren			  Class for Get XML Recording Link List
//========   ==============   =====================================================================

#include "NStream.h"
#include "RecordingLinkDBGet.h"
#include "StatusesGeneral.h"

extern CRecordingLinkDB*      GetRecordingLinkDB();

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CRecordingLinkDBGet::CRecordingLinkDBGet()
{
	m_updateCounter = 0;
}
/////////////////////////////////////////////////////////////////////////////
CRecordingLinkDBGet& CRecordingLinkDBGet::operator = (const CRecordingLinkDBGet &other)
{
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CRecordingLinkDBGet::~CRecordingLinkDBGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CRecordingLinkDBGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{

  ::GetRecordingLinkDB()->SerializeXml(pActionsNode,m_updateCounter);

}



/////////////////////////////////////////////////////////////////////////////
int CRecordingLinkDBGet::DeSerializeXml(CXMLDOMElement* pResNode,char* pszError,const char* strAction)
{

	DeSerializeXml(pResNode,pszError);
	

	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CRecordingLinkDBGet::DeSerializeXml(CXMLDOMElement* pActionsNode, char* pszError)
{
		
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pActionsNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	
	return nStatus;
	
}

