//============================================================================
// Name        : SubscriptionsManager.h
// Author      : Vasily
// Version     :
// Copyright   : Copyright (c) Polycom Inc 2012
// Description : Subscription Data type Notification Service
//============================================================================

#ifndef __SUBSCRIPTIONSMANAGER_H__
#define __SUBSCRIPTIONSMANAGER_H__

#include "SerializeObject.h"
#include "SubscriptionItem.h"
#include <iostream>
#include <list>
#include <sstream>
#include <map>


const unsigned int MAXIMAL_SUBSCRIPTIONS_NUMBER = 100;

enum EResourceEventType {
	eResourceEventStatus = 0,
	eResourceEventAlarms,
	eResourceEventHeartbeat,
	eResourceEventUnknown /* the last */
};

struct TEventInfo
{
	EResourceEventType  opcode;
	const char* pszName;
};

class SubscriptionsManager : public CSerializeObject
{
  // function members
public:
	SubscriptionsManager(/*IFileOperationsProvider* fileOperationsProvider*/);
	SubscriptionsManager(const SubscriptionsManager &other);
	virtual ~SubscriptionsManager();

	// Overrides
	virtual const char*  NameOf() const { return "Subscription"; }
	virtual void Dump(std::ostream& stream) const;
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	virtual CSerializeObject* Clone() {return new Subscription;}

	SubscriptionsManager& operator=(const SubscriptionsManager &other);
	bool operator==(const SubscriptionsManager &) const;
	bool operator!=(const SubscriptionsManager &) const;

	int ReadXmlFile() { return CSerializeObject::ReadXmlFile(m_pszSubscriptionsFileName); }
	void WriteXmlFile() { CSerializeObject::WriteXmlFile(m_pszSubscriptionsFileName,"NOTIFICATION_MNGR_SUBSCRIPTIONS"); }

 	// API
	STATUS AddSubscription(const Subscription& subsc,const std::string& sUserId,std::string& sSubscriptionUId);
	STATUS GetSubscription(const std::string& sSubscrId,Subscription& subsc);
	STATUS DeleteSubscription(const std::string& sSubscrId);
	STATUS GetSubscriptionsList(const std::string& sResourceName,std::list<Subscription>& subscriptionsList) const;
	STATUS SendUpdateNotification(const std::string& sSubscrId);

public:
	// Utilities
	static unsigned int GetMaxSubscriptionsNumber() { return MAXIMAL_SUBSCRIPTIONS_NUMBER; }
	unsigned int GetSubscriptionsNum() const { return m_subscriptions.size(); }
	void CheckExpiration();
	static bool ValidateResourceType(const std::string& sResource);
	static const char* GetResourceName(const EResourceEventType eventType);
	void HandleEvent(const EResourceEventType resourceEvent, const std::string& sDataXml);

protected:
	// Utilities
	bool IsEqualSubscriptionData(const SubscriptionItem& first, const SubscriptionItem& second) const;
	static void SetSubscriptionExpirationTime(SubscriptionItem& subscription, const unsigned int ttl);
	std::list<SubscriptionItem>::iterator Find(const std::string& sId);
	BOOL PrepareNotificationCommand(const Subscription& subscription, const std::string& sData, std::string& notificationCommand);
	void SendNotification(SubscriptionItem& subscriptionItem, const std::string& sDataXml);
	bool Save() const;
	bool Load();

protected:
	static const struct TEventInfo m_taAvailableResourceEvents[];
	static const char* m_pszSubscriptionsFileName;
	std::string        m_sSubcriptionsFileName;
	std::list <SubscriptionItem>  m_subscriptions;
	std::map <std::string,std::string> m_lastResourceDataStringsMap;
};



#endif /* __SUBSCRIPTIONSMANAGER_H__ */
