/*
 * CommApiSubscription.h
 *
 *  Created on: Jun 4, 2012
 *      Author: Vasily
 */

#ifndef __COMMAPISUBSCRIPTION_H__
#define __COMMAPISUBSCRIPTION_H__


#include "SerializeObject.h"
#include "Subscription.h"


class CommApiSubscription : public CSerializeObject
{
CLASS_TYPE_1(CommApiSubscription,CSerializeObject)
public:
	CommApiSubscription();
	CommApiSubscription(const CommApiSubscription& other);
	virtual ~CommApiSubscription();
	CommApiSubscription& operator =(const CommApiSubscription& other);
	virtual CSerializeObject* Clone() { return new CommApiSubscription; }
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

//	std::string GetUid() const { return m_sUid; }

//protected:
	std::string  m_sUid;
	Subscription m_subscription;
};



#endif /* __COMMAPISUBSCRIPTION_H__ */
