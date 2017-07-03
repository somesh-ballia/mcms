//============================================================================
// Name        : Subscription.h
// Author      : Vasily
// Version     :
// Copyright   : Copyright (c) Polycom Inc 2012
// Description : Subscription Data type Notification Service
//============================================================================

#ifndef __SUBSCRIPTION_H_
#define __SUBSCRIPTION_H_

#include "ApiStatuses.h"
#include "StatusesGeneral.h"
#include "SerializeObject.h"


struct ReturnStatus
{
	int          m_id;
	std::string  m_description;
};


class Subscription : public CSerializeObject
{
  CLASS_TYPE_1(Subscription, CSerializeObject)
  // function members
  public:
	Subscription();
	Subscription(const Subscription &other);
	virtual ~Subscription();

	// Overrides
	virtual const char*  NameOf() const { return "Subscription"; }
	virtual void Dump(std::ostream& stream) const;
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	virtual CSerializeObject* Clone() {return new Subscription;}

	Subscription& operator=(const Subscription &other);
	bool operator==(const Subscription& other) const;
	bool operator!=(const Subscription& other) const;

  public:
	std::string GetConsumerUrl() const { return m_consumer_url; }
	std::string GetResourceUrl() const { return m_subject_resource_url; }
	std::string GetUsername() const { return m_username; }
	std::string GetPassword() const { return m_password; }
	unsigned int GetTTL() const { return m_ttl; }
	std::string GetPassback() const { return m_passback; }
	std::string GetPassthru() const { return m_passthru; }
	int         GetLastErrorCode() const { return m_last_error.m_id; }
	std::string GetLastErrorDescription() const { return m_last_error.m_description; }

	void SetUsername(const std::string& username);
	void SetPassword(const std::string& password);
	void SetTTL(const unsigned int ttl);
	void SetLastErrorStatus(const int status=STATUS_OK, const std::string& description="");

  // data members
  protected:
	std::string m_consumer_url;
	std::string m_subject_resource_url;
	std::string m_username;
	std::string m_password;
	unsigned int m_ttl;
	ReturnStatus m_last_error;
	std::string m_passback;
	std::string m_passthru;
};

#endif // __SUBSCRIPTION_H_
