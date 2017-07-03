//============================================================================
// Name        : SubscriptionItem.h
// Author      : Vasily
// Version     :
// Copyright   : Copyright (c) Polycom Inc 2012
// Description : Subscription Data type Notification Service
//============================================================================

#ifndef __SUBSCRIPTIONITEM_H_
#define __SUBSCRIPTIONITEM_H_

#include "SerializeObject.h"
#include "Subscription.h"


class SubscriptionItem : public CSerializeObject
{
  CLASS_TYPE_1(SubscriptionItem, CSerializeObject)
  // function members
  public:
	SubscriptionItem();
	SubscriptionItem(const SubscriptionItem &other);
	virtual ~SubscriptionItem();

	// Overrides
	virtual const char*  NameOf() const { return "SubscriptionItem"; }
	virtual void Dump(std::ostream& stream) const;
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	virtual CSerializeObject* Clone() {return new SubscriptionItem;}

	SubscriptionItem& operator=(const SubscriptionItem &other);
	bool operator==(const SubscriptionItem &) const;
	bool operator!=(const SubscriptionItem &) const;

  public:
	std::string GetUID() const { return m_uid; }
	std::string GetUserId() const { return m_userId; }
	const Subscription& GetSubscription() const { return m_subscription; }
	long GetExpirationTime() const { return m_expirationTime; }

	void SetUID(const std::string& uid);
	void SetUserId(const std::string& userId);
	void SetSubscription(const Subscription& subscription);
	void SetExpirationTime(const long expirationTime);
	void SetTTL(const unsigned int ttl);

  // data members
  protected:
	Subscription m_subscription;
	std::string m_uid;
	std::string m_userId;
	long m_expirationTime;
};
#endif // __SUBSCRIPTIONITEM_H_
