
#include "RtmIsdnSpanMapDel.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "InitCommonStrings.h"
#include "psosxml.h"



//////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMapDel::CRtmIsdnSpanMapDel()
{
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMapDel::~CRtmIsdnSpanMapDel()
{

}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMapDel::CRtmIsdnSpanMapDel(const CRtmIsdnSpanMapDel &other)
:CSerializeObject(other)
{
	m_boardId = other.m_boardId;
	m_spanId  = other.m_spanId;
}


///////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMapDel::SerializeXml(CXMLDOMElement*& thisNode) const
{
		
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnSpanMapDel::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pNode,	"SLOT_NUMBER",	&m_boardId,	_0_TO_WORD);
	GET_VALIDATE_CHILD(pNode,	"SPAN_ID",		&m_spanId,	_0_TO_WORD);

	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CRtmIsdnSpanMapDel::GetBoardId () const                 
{
    return m_boardId;
}

/////////////////////////////////////////////////////////////////////////////
void  CRtmIsdnSpanMapDel::SetBoardId(const WORD boardId)                 
{
	m_boardId = boardId;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CRtmIsdnSpanMapDel::GetSpanId () const                 
{
    return m_spanId;
}

/////////////////////////////////////////////////////////////////////////////
void  CRtmIsdnSpanMapDel::SetSpanId(const WORD spanId)                 
{
	m_spanId = spanId;
}

