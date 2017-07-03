/*
 * CommApiUid.cpp
 *
 *  Created on: Jun 4, 2012
 *      Author: Vasily
 */


#include "CommApiUid.h"
#include "psosxml.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"


//////////////////////////////////////////////////////////////////////
CommApiUid::CommApiUid()
{
	m_sUid = "JUNK_UID";
}

//////////////////////////////////////////////////////////////////////
CommApiUid::CommApiUid(const CommApiUid& other) : CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CommApiUid::~CommApiUid()
{
}

/////////////////////////////////////////////////////////////////////////////
CommApiUid& CommApiUid::operator= (const CommApiUid& other)
{
	if( this == &other )
		return *this;

	m_sUid = other.m_sUid;

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CommApiUid::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	pFatherNode->AddChildNode("UID",m_sUid);
}

//////////////////////////////////////////////////////////////////////
int CommApiUid::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"UID",m_sUid,_1_TO_H243_NAME_LENGTH);

	return nStatus;
}
