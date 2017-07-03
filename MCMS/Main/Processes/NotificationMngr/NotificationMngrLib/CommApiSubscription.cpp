/*
 * CommApiSubscription.cpp
 *
 *  Created on: Jun 4, 2012
 *      Author: Vasily
 */


#include "CommApiSubscription.h"
#include "psosxml.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"


//////////////////////////////////////////////////////////////////////
CommApiSubscription::CommApiSubscription()
{
	m_sUid = "JUNK_UID";
}

//////////////////////////////////////////////////////////////////////
CommApiSubscription::CommApiSubscription(const CommApiSubscription& other) : CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CommApiSubscription::~CommApiSubscription()
{
}

/////////////////////////////////////////////////////////////////////////////
CommApiSubscription& CommApiSubscription::operator= (const CommApiSubscription& other)
{
	if( this == &other )
		return *this;

	m_sUid = other.m_sUid;
	m_subscription = other.m_subscription;

	return *this;
}

//////////////////////////////////////////////////////////////////////
void CommApiSubscription::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	DBGPASSERT_AND_RETURN(pFatherNode == NULL);

	CXMLDOMElement* pSubscriptionNode = pFatherNode->AddChildNode("SUBSCRIPTION_RESPONSE_OBJ");
	DBGPASSERT_AND_RETURN(pSubscriptionNode == NULL);

	pSubscriptionNode->AddChildNode("UID",m_sUid);
	m_subscription.SerializeXml(pSubscriptionNode);
}

//////////////////////////////////////////////////////////////////////
int CommApiSubscription::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"UID",m_sUid,_1_TO_H243_NAME_LENGTH);

	return nStatus;
}
