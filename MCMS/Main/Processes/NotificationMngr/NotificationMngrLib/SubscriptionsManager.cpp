//============================================================================
// Name        : SubscriptionsManager.cpp
// Author      : Vasily
// Version     :
// Copyright   : Copyright (c) Polycom Inc 2012
// Description : Subscription Data type Notification Service
//============================================================================

#include "SubscriptionsManager.h"

//#include <fstream>
#include <time.h>
#include <iomanip>
#include <sstream>
#include "psosxml.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"
#include "Trace.h"
#include "TraceStream.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "Utils.h"
#include "EncodeHelper.h"
#include "SystemFunctions.h"


// STATIC fields initialization

const struct TEventInfo
SubscriptionsManager::m_taAvailableResourceEvents[] =
{
	{ eResourceEventStatus, "/event/status" },
	{ eResourceEventAlarms, "/event/alarms" },
	{ eResourceEventHeartbeat, "/event/heartbeat" },
};

const char*
SubscriptionsManager::m_pszSubscriptionsFileName = "Cfg/NotifMngrSubscriptions.xml";


////////////////////////////////////////////////////////////////////////////////
SubscriptionsManager::SubscriptionsManager()
: CSerializeObject()
{
}

///////////////////////////////////////////////////
SubscriptionsManager::SubscriptionsManager(const SubscriptionsManager &other)
: CSerializeObject(other)
{
	//subscriptions
	m_subscriptions.assign(other.m_subscriptions.begin(),
				other.m_subscriptions.end());
	m_lastResourceDataStringsMap.insert(other.m_lastResourceDataStringsMap.begin(),
				other.m_lastResourceDataStringsMap.end());
}

///////////////////////////////////////////////////
SubscriptionsManager::~SubscriptionsManager()
{
	//subscriptions
	m_subscriptions.clear();
	//lastResourceDataStringsMap
	m_lastResourceDataStringsMap.clear();
}

///////////////////////////////////////////////////
SubscriptionsManager& SubscriptionsManager::operator=(const SubscriptionsManager &other)
{
	if (this == &other)
		return *this;

	//subscriptions
	m_subscriptions.clear();
	m_subscriptions.assign(other.m_subscriptions.begin(), other.m_subscriptions.end());
	//lastResourceDataStringsMap
	m_lastResourceDataStringsMap.clear();
	m_lastResourceDataStringsMap.insert(other.m_lastResourceDataStringsMap.begin(), other.m_lastResourceDataStringsMap.end());

	return *this;
}

///////////////////////////////////////////////////
bool SubscriptionsManager::operator==(const SubscriptionsManager& other) const
{
	//subscriptions
	if (m_subscriptions.size() != other.m_subscriptions.size() )
		return false;

	if (!std::equal(m_subscriptions.begin(),
					m_subscriptions.end(),
					other.m_subscriptions.begin()))
		 return false;

	if (!std::equal(m_lastResourceDataStringsMap.begin(),
					m_lastResourceDataStringsMap.end(),
					other.m_lastResourceDataStringsMap.begin()))
		 return false;

	return true;
}

///////////////////////////////////////////////////
bool SubscriptionsManager::operator!=(const SubscriptionsManager& other) const
{
	return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////
void SubscriptionsManager::Dump(std::ostream& ostr) const
{
	ostr << " Subscriptions List. Num of elements: " << (int)m_subscriptions.size() << std::endl;
	ostr << "-----------------------------------------------------------------------------------------------------------------" << std::endl;
	ostr    << std::setw(37) << std::left << "Subscription Id"
			<< std::setw(10) << std::left << "User Id"
			<< std::setw(18) << std::left << "Resource"
			<< std::setw(5)  << std::left << "TTL"
			<< std::setw(18) << std::left << "user:password"
			                 << std::left << "Consumer URL"
//			<< std::setw(7)  << std::left << "Locked"
//			<< std::setw(7)  << std::left << "Disabled"
			<< std::endl;
	ostr << "-----------------------------------------------------------------------------------------------------------------" << std::endl;
	std::list<SubscriptionItem>::const_iterator it = m_subscriptions.begin();
	while (it != m_subscriptions.end())
	{
		const SubscriptionItem* pItem = &(*it);
		std::string sUserPassword = EncodeHelper::base64_decode(pItem->GetSubscription().GetUsername());
		sUserPassword.append(":");
		sUserPassword.append(EncodeHelper::base64_decode(pItem->GetSubscription().GetPassword()));

		ostr    << std::setw(37) << std::left << pItem->GetUID()
				<< std::setw(10) << std::left << pItem->GetUserId()
				<< std::setw(18) << std::left << pItem->GetSubscription().GetResourceUrl()
				<< std::setw(5)  << std::left << pItem->GetSubscription().GetTTL()
				<< std::setw(18) << std::left << sUserPassword
				                 << std::left << pItem->GetSubscription().GetConsumerUrl()
				<< std::endl;

		it++;
	}
	ostr << "-----------------------------------------------------------------------------------------------------------------" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
bool SubscriptionsManager::ValidateResourceType(const std::string& sResource)
{
	unsigned int len = sizeof(m_taAvailableResourceEvents)/sizeof(m_taAvailableResourceEvents[0]);
	for( unsigned int i=0; i<len; i++ )
	{
		if( sResource == m_taAvailableResourceEvents[i].pszName )
			return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
const char* SubscriptionsManager::GetResourceName(const EResourceEventType eventType)
{
	unsigned int len = sizeof(m_taAvailableResourceEvents)/sizeof(m_taAvailableResourceEvents[0]);
	for( unsigned int i=0; i<len; i++ )
	{
		if( eventType == m_taAvailableResourceEvents[i].opcode )
			return m_taAvailableResourceEvents[i].pszName;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SubscriptionsManager::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pMainNode;

	if(!pFatherNode)
		return;

	CXMLDOMElement* pListNode = pFatherNode->AddChildNode("SUBSCRIPTIONS_LIST");

	// run over all subscriptions and serialize
	std::list<SubscriptionItem>::const_iterator it = m_subscriptions.begin();
	while (it != m_subscriptions.end())
	{
		const SubscriptionItem* pObj = &(*it);
		pObj->SerializeXml(pListNode);
		it++;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int SubscriptionsManager::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;

	m_subscriptions.clear();

	CXMLDOMElement* pMngrNode;
	char* pszParentNodeName;

	pActionNode->get_nodeName(&pszParentNodeName);
	if(!strcmp(pszParentNodeName, "NOTIFICATION_MNGR_SUBSCRIPTIONS"))
	{
		pMngrNode = pActionNode;
	}
	else
		GET_MANDATORY_CHILD_NODE(pActionNode, "NOTIFICATION_MNGR_SUBSCRIPTIONS", pMngrNode);

	if ( pMngrNode )
	{
		CXMLDOMElement* pSubscriptionsListNode = NULL;
		GET_MANDATORY_CHILD_NODE(pMngrNode, "SUBSCRIPTIONS_LIST", pSubscriptionsListNode);

		if( pSubscriptionsListNode )
		{
			CXMLDOMElement* pSubscriptionNode = NULL;

			GET_FIRST_CHILD_NODE(pSubscriptionsListNode, "SUBSCRIPTION_ITEM", pSubscriptionNode);

			while (pSubscriptionNode)
			{
				SubscriptionItem subscriptionItem;

				nStatus = subscriptionItem.DeSerializeXml(pSubscriptionNode, pszError, action);

				if (nStatus != STATUS_OK)
					return nStatus;

				m_subscriptions.push_back(subscriptionItem);

				GET_NEXT_CHILD_NODE(pSubscriptionsListNode, "SUBSCRIPTION_ITEM", pSubscriptionNode);
			}
		}
	}

	return nStatus;
}

////////////////////////////////////////////////////////////////////////////////
//
//     APIs
//

////////////////////////////////////////////////////////////////////////////////
STATUS SubscriptionsManager::AddSubscription(const Subscription& subsc,
		const std::string& sUserId, std::string& sSubscriptionUId)
{
	// check resource type
	if( false == ValidateResourceType(subsc.GetResourceUrl()) )
		return STATUS_NOTIF_RESOURCE_NOT_SUPPORTED;

	Subscription sub = subsc;
	sub.SetUsername(EncodeHelper::base64_encode((const unsigned char*)subsc.GetUsername().c_str(),subsc.GetUsername().length()));
	sub.SetPassword(EncodeHelper::base64_encode((const unsigned char*)subsc.GetPassword().c_str(),subsc.GetPassword().length()));

	SubscriptionItem subscriptionItem;
	subscriptionItem.SetSubscription(sub);
	subscriptionItem.SetUserId(sUserId);

	// if such subscription already exists -> update data fields & expiration time
	std::list<SubscriptionItem>::iterator it = m_subscriptions.begin();
	while (it != m_subscriptions.end())
	{
		SubscriptionItem* pObj = &(*it);
		if( IsEqualSubscriptionData(subscriptionItem,*pObj) )
		{
			sSubscriptionUId = pObj->GetUID();
			*pObj = subscriptionItem;
			pObj->SetUID(sSubscriptionUId);
			SetSubscriptionExpirationTime(*pObj,sub.GetTTL());
			WriteXmlFile();
			return STATUS_OK;
		}
		it++;
	}

	// Check if Maximal users number reached
	if( GetMaxSubscriptionsNumber() <= m_subscriptions.size() )
		return STATUS_NOTIF_MAXIMAL_SUBSCRIPTIONS_NUMBER_REACHED;

	// not found -> append new to list

	//  generate UID
	std::string sUid;
	bool found = false;
	unsigned int count = 0;
	// check if UID exists
	do
	{
		found = false;
		// generate uid
		sUid = Utils::GenerateUid();

		// check if exists
		it = m_subscriptions.begin();
		while (!found && it != m_subscriptions.end())
		{
			if( sUid == it->GetUID() )
				found = true;
			it++;
		}

		// next try
		count++;
		if( count > 100 )
		{
			return STATUS_NOTIF_ID_GENERATION_FAILED;
		}
	} while( found == true );

	subscriptionItem.SetUID(sUid);
	sSubscriptionUId = sUid;

	//  set expiration time
	SetSubscriptionExpirationTime(subscriptionItem,subscriptionItem.GetSubscription().GetTTL());

	//  append
	m_subscriptions.push_back(subscriptionItem);
	WriteXmlFile();

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
STATUS SubscriptionsManager::GetSubscription(const std::string& sSubscrId,Subscription& subsc)
{
	std::list<SubscriptionItem>::iterator it = Find(sSubscrId);
	if( m_subscriptions.end() == it )
		return STATUS_NOTIF_SUBSCRIPTION_NOT_FOUND;

	subsc = it->GetSubscription();
	subsc.SetUsername(EncodeHelper::base64_decode(subsc.GetUsername()));
	subsc.SetPassword(EncodeHelper::base64_decode(subsc.GetPassword()));

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
STATUS SubscriptionsManager::DeleteSubscription(const std::string& sSubscrId)
{
	std::list<SubscriptionItem>::iterator it = Find(sSubscrId);
	if( m_subscriptions.end() == it )
		return STATUS_NOTIF_SUBSCRIPTION_NOT_FOUND;

	m_subscriptions.erase(it);
	WriteXmlFile();

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
STATUS SubscriptionsManager::GetSubscriptionsList(const std::string& sResourceName,
		std::list<Subscription>& subscriptionsList) const
{
	subscriptionsList.clear();

	std::list<SubscriptionItem>::const_iterator it = m_subscriptions.begin();
	while (it != m_subscriptions.end())
	{
		Subscription subsc = it->GetSubscription();
		if( sResourceName.empty() ||  subsc.GetResourceUrl() == sResourceName )
		{
			subsc.SetUsername(EncodeHelper::base64_decode(subsc.GetUsername()));
			subsc.SetPassword(EncodeHelper::base64_decode(subsc.GetPassword()));
			subscriptionsList.push_back(subsc);
		}
		it++;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
STATUS SubscriptionsManager::SendUpdateNotification(const std::string& sSubscrId)
{
	std::list<SubscriptionItem>::iterator it = Find(sSubscrId);
	if( m_subscriptions.end() == it )
		return STATUS_NOTIF_SUBSCRIPTION_NOT_FOUND;

	Subscription subscription = it->GetSubscription();
	if( m_lastResourceDataStringsMap.find(subscription.GetResourceUrl()) != m_lastResourceDataStringsMap.end() )
	{
		SendNotification(*it,m_lastResourceDataStringsMap[subscription.GetResourceUrl()]);
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"SubscriptionsManager::SendUpdateNotification - no saved data on resource: ",subscription.GetResourceUrl().c_str());
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
//     UTILITIES
//

////////////////////////////////////////////////////////////////////////////////
bool SubscriptionsManager::IsEqualSubscriptionData(const SubscriptionItem& first, const SubscriptionItem& second) const
{
	if( first.GetUserId() != second.GetUserId() )
		return false;
	if( first.GetSubscription().GetConsumerUrl() != second.GetSubscription().GetConsumerUrl() )
		return false;
	if( first.GetSubscription().GetResourceUrl() != second.GetSubscription().GetResourceUrl() )
		return false;
	if( first.GetSubscription().GetUsername() != second.GetSubscription().GetUsername() )
		return false;
//	if( first.GetSubscription().m_password != second.GetSubscription().m_password )
//		return false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void SubscriptionsManager::SetSubscriptionExpirationTime(SubscriptionItem& subscription, const unsigned int ttl)
{
	time_t now = time(NULL);
	subscription.SetExpirationTime(now + ttl);

	subscription.SetTTL(ttl);
}

////////////////////////////////////////////////////////////////////////////////
void SubscriptionsManager::CheckExpiration()
{
	bool isChanged = false;
	time_t now = time(NULL);

	std::list<SubscriptionItem>::iterator it = m_subscriptions.begin();
	while (it != m_subscriptions.end())
	{
		if( now > it->GetExpirationTime() )
		{
			PTRACE2(eLevelInfoNormal,"Subscription expired. UID=", it->GetUID().c_str());
			isChanged = true;
			m_subscriptions.erase(it++);
			continue;
		}
		it++;
	}
	if( isChanged )
		WriteXmlFile();
}

////////////////////////////////////////////////////////////////////////////////
std::list<SubscriptionItem>::iterator SubscriptionsManager::Find(const std::string& sId)
{
	std::list<SubscriptionItem>::iterator it = m_subscriptions.begin();
	while (it != m_subscriptions.end())
	{
		if( sId == it->GetUID() )
			return it;
		it++;
	}

	return m_subscriptions.end();
}

////////////////////////////////////////////////////////////////////////////////
void SubscriptionsManager::HandleEvent(const EResourceEventType resourceEvent, const std::string& sDataXml)
{
	const char* szResourceName = GetResourceName(resourceEvent);
	if (NULL == szResourceName)
		return;
	std::string sResource = szResourceName;
	if( !ValidateResourceType(sResource) )
		return;

	if( sDataXml == m_lastResourceDataStringsMap[sResource] && resourceEvent != eResourceEventHeartbeat )
		return;

	m_lastResourceDataStringsMap[sResource] = sDataXml;

	std::list<SubscriptionItem>::iterator it = m_subscriptions.begin();
	while (it != m_subscriptions.end())
	{
		Subscription subsc = it->GetSubscription();
		if( subsc.GetResourceUrl() == sResource )
			SendNotification(*it,sDataXml);

		it++;
	}

	return;
}

////////////////////////////////////////////////////////////////////////////////
void SubscriptionsManager::SendNotification(SubscriptionItem& subscriptionItem, const std::string& sDataXml)
{
	PTRACE(eLevelInfoNormal,"SubscriptionsManager::SendNotification");
	std::string command_in;
	std::string command_out;

	// real DMA for test
	// "curl -k https://admin:admin@10.226.10.15:8443/api/rest/users -s -S -d \"<some_data>data</some_data>\"";
	//
	// my command
	// "curl -k https://admin:admin@10.253.66.82:8445/event/listener -s -S -d \"<some_data>data</some_data>\" 2>&1";
	//

	Subscription subscription = subscriptionItem.GetSubscription();
	if( PrepareNotificationCommand(subscription,sDataXml,command_in) )
	{
		//std::cout << "Command_IN:" << /*std::endl << command_in << */std::endl;
		SystemPipedCommand(command_in.c_str(),command_out);
		//std::cout << "Command_OUT:" << std::endl << command_out << std::endl;

		// update last error field in subscription if needed
		int  newStatus = STATUS_OK;

		std::string::size_type start_pos = command_out.find("curl: (");
		if( std::string::npos != start_pos)
			newStatus = STATUS_FAIL;

		//std::cout << ">>> OldStatus: " << subscription.GetLastErrorCode() << "; NewStatus: " << newStatus << "; Out: " << command_out << std::endl;
		if( subscription.GetLastErrorCode() != newStatus || subscription.GetLastErrorDescription() != command_out )
		{
			TRACEINTO << "\n SubscriptionsManager::SendNotification - save new LAST_ERROR to file. OldStatus: "
					<< subscription.GetLastErrorCode() << "; NewStatus: " << newStatus << ".";
			//std::cout << ">>> write" << std::endl << std::endl;
			subscription.SetLastErrorStatus(newStatus,command_out);
			subscriptionItem.SetSubscription(subscription);
			WriteXmlFile();
		}
		else
		{
			TRACEINTO << "\n SubscriptionsManager::SendNotification - Status: " << newStatus << ".";
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
BOOL SubscriptionsManager::PrepareNotificationCommand(const Subscription& subscription, const std::string& sData,
		std::string& notificationCommand)
{
	std::stringstream ss;
	std::string sUrl = subscription.GetConsumerUrl();
	std::string sUsername = subscription.GetUsername().length() > 0 ? EncodeHelper::base64_decode(subscription.GetUsername()) : "";
	std::string sPassword = subscription.GetPassword().length() > 0 ? EncodeHelper::base64_decode(subscription.GetPassword()) : "";

	// if user/password declared in subscription -> substitute it in URL
	if( sUsername.length() && sPassword.length() )
	{
		size_t nEndProtocolPos = sUrl.find_first_of("://");
		if( std::string::npos != nEndProtocolPos )
		{
			// move to first char after "://"
			nEndProtocolPos += 3;
		}
		else
		{
			PTRACE(eLevelInfoNormal,"SubscriptionsManager::PrepareNotificationCommand - No protocol in subscription's consumer url!");
			nEndProtocolPos = 0;
		}
		size_t nAddressPos = sUrl.find_first_of("@");
		if( std::string::npos != nAddressPos )
		{
			// move to first char after "@"
			nAddressPos += 1;
		}
		else
		{
			// no username/password in URL, like
			//   https://domain.com/address
			nAddressPos = nEndProtocolPos;
		}
		std::string sNewUrl = sUrl.substr(0,nEndProtocolPos);
		sNewUrl.append(sUsername);
		sNewUrl.append(":");
		sNewUrl.append(sPassword);
		sNewUrl.append("@");
		sNewUrl.append(sUrl.substr(nAddressPos));

		sUrl = sNewUrl;
	}
	ss << "curl -k ";
	ss << sUrl;
	ss << " -s -S -d \"" << sData << "\" 2>&1";

	notificationCommand = ss.str();

	return TRUE;
}














