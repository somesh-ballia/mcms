//============================================================================
// Name        : Subscription.cpp
// Author      : Vasily
// Version     :
// Copyright   : Copyright (c) Polycom Inc 2012
// Description : Subscription Data type Notification Service
//============================================================================


#include "Subscription.h"
#include "psosxml.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
Subscription::Subscription()
: m_ttl(0)
{
	m_last_error.m_id = STATUS_OK;
	m_last_error.m_description = "";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Subscription::Subscription(const Subscription &other)
:CSerializeObject(other)
,m_consumer_url(other.m_consumer_url)
,m_subject_resource_url(other.m_subject_resource_url)
,m_username(other.m_username)
,m_password(other.m_password)
,m_ttl(other.m_ttl)
,m_passback(other.m_passback)
,m_passthru(other.m_passthru)
{
	m_last_error.m_id      = other.m_last_error.m_id;
	m_last_error.m_description = other.m_last_error.m_description;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Subscription::~Subscription()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Subscription& Subscription::operator=(const Subscription &other)
{
	if (this == &other)
		return *this;

	//consumer-url
	m_consumer_url = other.m_consumer_url;

	//subject-resource-url
	m_subject_resource_url = other.m_subject_resource_url;

	//username
	m_username = other.m_username;

	//password
	m_password = other.m_password;

	//ttl
	m_ttl = other.m_ttl;

	//plcm-error
	m_last_error.m_id = other.m_last_error.m_id;
	m_last_error.m_description = other.m_last_error.m_description;

	//passback
	m_passback = other.m_passback;

	//passthru
	m_passthru = other.m_passthru;

	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool Subscription::operator==(const Subscription& other) const
{

	//consumer-url
	if (m_consumer_url != other.m_consumer_url)
		return false;

	//subject-resource-url
	if (m_subject_resource_url != other.m_subject_resource_url)
		return false;

	//username
	if (m_username != other.m_username)
		return false;

	//password
	if (m_password != other.m_password)
		return false;

	//ttl
	if (m_ttl != other.m_ttl)
		return false;

	//plcm-error
	if (m_last_error.m_id != other.m_last_error.m_id)
		return false;
	if (m_last_error.m_description != other.m_last_error.m_description)
		return false;

	//passback
	if (m_passback != other.m_passback)
		return false;

	//passthru
	if (m_passthru != other.m_passthru)
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool Subscription::operator!=(const Subscription& other) const
{
	return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Subscription::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	DBGPASSERT_AND_RETURN(pFatherNode == NULL);

	CXMLDOMElement* pSubscriptionNode = pFatherNode->AddChildNode("SUBSCRIPTION");
	DBGPASSERT_AND_RETURN(pSubscriptionNode == NULL);

	pSubscriptionNode->AddChildNode("CONSUMER_URL",m_consumer_url);
	pSubscriptionNode->AddChildNode("SUBJECT_RESOURCE_URL",m_subject_resource_url);
	pSubscriptionNode->AddChildNode("USERNAME",m_username);
	pSubscriptionNode->AddChildNode("PASSWORD",m_password);
	pSubscriptionNode->AddChildNode("TTL",m_ttl);
	pSubscriptionNode->AddChildNode("PASSBACK",m_passback);
	pSubscriptionNode->AddChildNode("PASSTHRU",m_passthru);

	CXMLDOMElement* pErrorNode = pSubscriptionNode->AddChildNode("LAST_ERROR");

	DBGPASSERT_AND_RETURN(pErrorNode == NULL);

	pErrorNode->AddChildNode("STATUS",m_last_error.m_id);
	pErrorNode->AddChildNode("DESCRIPTION",m_last_error.m_description);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int Subscription::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pSubscriptionNode;
	char* pParentNodeName;

	pActionNode->get_nodeName(&pParentNodeName);
	if(!strcmp(pParentNodeName, "SUBSCRIPTION"))
	{
 		pSubscriptionNode = pActionNode;
	}
	else
		GET_MANDATORY_CHILD_NODE(pActionNode, "SUBSCRIPTION", pSubscriptionNode);

	if ( pSubscriptionNode )
	{
		GET_VALIDATE_CHILD(pSubscriptionNode,"CONSUMER_URL",m_consumer_url,_0_TO_IP_LIMIT_ADDRESS_CHAR_LENGTH);
		GET_VALIDATE_CHILD(pSubscriptionNode,"SUBJECT_RESOURCE_URL",m_subject_resource_url,_0_TO_IP_LIMIT_ADDRESS_CHAR_LENGTH);
		GET_VALIDATE_CHILD(pSubscriptionNode,"USERNAME",m_username,_0_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pSubscriptionNode,"PASSWORD",m_password,_0_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pSubscriptionNode,"TTL",&m_ttl,_0_TO_3000_DECIMAL);
		GET_VALIDATE_CHILD(pSubscriptionNode,"PASSBACK",m_passback,_0_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pSubscriptionNode,"PASSTHRU",m_passthru,_0_TO_H243_NAME_LENGTH);

		if(STATUS_NODE_MISSING == nStatus)
			nStatus = STATUS_OK;

		CXMLDOMElement* pErrorNode = NULL;
		GET_CHILD_NODE(pSubscriptionNode, "LAST_ERROR", pErrorNode);
		if(ERR_FAIL == nStatus)
			return STATUS_OK;
		if( pErrorNode )
		{
			GET_VALIDATE_CHILD(pErrorNode,"STATUS",&m_last_error.m_id,_0_TO_100000_DECIMAL);
			GET_VALIDATE_CHILD(pErrorNode,"DESCRIPTION",m_last_error.m_description,_0_TO_H243_NAME_LENGTH);
		}
	}

	return nStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Subscription::Dump(std::ostream& stream) const
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Subscription::SetUsername(const std::string& username)
{
	m_username = username;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Subscription::SetPassword(const std::string& password)
{
	m_password = password;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Subscription::SetTTL(const unsigned int ttl)
{
	m_ttl = ttl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Subscription::SetLastErrorStatus(const int status, const std::string& description)
{
	m_last_error.m_id = status;
	m_last_error.m_description = description;
}


