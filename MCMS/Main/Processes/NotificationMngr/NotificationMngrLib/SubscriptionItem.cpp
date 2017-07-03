//============================================================================
// Name        : SubscriptionItem.cpp
// Author      : Vasily
// Version     :
// Copyright   : Copyright (c) Polycom Inc 2012
// Description : Subscription Data type Notification Service
//============================================================================


#include "SubscriptionItem.h"
#include "psosxml.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
SubscriptionItem::SubscriptionItem()
:m_expirationTime(0)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
SubscriptionItem::SubscriptionItem(const SubscriptionItem &other)
:CSerializeObject(other)
,m_subscription(other.m_subscription)
,m_uid(other.m_uid)
,m_userId(other.m_userId)
,m_expirationTime(other.m_expirationTime)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
SubscriptionItem::~SubscriptionItem()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
SubscriptionItem& SubscriptionItem::operator=(const SubscriptionItem &other)
{
	if (this == &other)
		return *this;

	//subscription
	m_subscription = other.m_subscription;

	//uid
	m_uid = other.m_uid;

	//user id
	m_userId = other.m_userId;

	//expiration time
	m_expirationTime = other.m_expirationTime;

	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool SubscriptionItem::operator==(const SubscriptionItem& other) const
{

	//subscription
	if (m_subscription != other.m_subscription)
		return false;

	//uid
	if (m_uid != other.m_uid)
		return false;

	//user id
	if (m_userId != other.m_userId)
		return false;

	//expiration time
	if (m_expirationTime != other.m_expirationTime)
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool SubscriptionItem::operator!=(const SubscriptionItem& other) const
{
	return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SubscriptionItem::Dump(std::ostream& stream) const
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SubscriptionItem::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	DBGPASSERT_AND_RETURN(pFatherNode == NULL);

	CXMLDOMElement* pSubscriptionNode = pFatherNode->AddChildNode("SUBSCRIPTION_ITEM");
	DBGPASSERT_AND_RETURN(pSubscriptionNode == NULL);

	pSubscriptionNode->AddChildNode("UID",m_uid);
	pSubscriptionNode->AddChildNode("USER_ID",m_userId);
	pSubscriptionNode->AddChildNode("EXPIRATION_TIME",(DWORD)m_expirationTime);

	m_subscription.SerializeXml(pSubscriptionNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int SubscriptionItem::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pSubscriptionNode;
	char* ParentNodeName;

	pActionNode->get_nodeName(&ParentNodeName);
	if(!strcmp(ParentNodeName, "SUBSCRIPTION_ITEM"))
	{
 		pSubscriptionNode = pActionNode;
	}
	else
		GET_MANDATORY_CHILD_NODE(pActionNode, "SUBSCRIPTION_ITEM", pSubscriptionNode);

	if ( pSubscriptionNode )
	{
		GET_VALIDATE_CHILD(pSubscriptionNode,"UID",m_uid,_0_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pSubscriptionNode,"USER_ID",m_userId,_0_TO_H243_NAME_LENGTH);
		DWORD dw = 0;
		GET_VALIDATE_CHILD(pSubscriptionNode,"EXPIRATION_TIME",&dw,_0_TO_DWORD);
		m_expirationTime = dw;

		CXMLDOMElement* pApiSubNode = NULL;
		GET_MANDATORY_CHILD_NODE(pSubscriptionNode, "SUBSCRIPTION", pApiSubNode);
		m_subscription.DeSerializeXml(pApiSubNode,pszError,action);
	}

	return nStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SubscriptionItem::SetUID(const std::string& uid)
{
	m_uid = uid;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SubscriptionItem::SetUserId(const std::string& userId)
{
	m_userId = userId;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SubscriptionItem::SetSubscription(const Subscription& subscription)
{
	m_subscription = subscription;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SubscriptionItem::SetExpirationTime(const long expirationTime)
{
	m_expirationTime = expirationTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SubscriptionItem::SetTTL(const unsigned int ttl)
{
	m_subscription.SetTTL(ttl);
}








