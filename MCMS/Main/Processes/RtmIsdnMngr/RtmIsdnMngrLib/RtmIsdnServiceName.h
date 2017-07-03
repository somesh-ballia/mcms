#ifndef RTMISDNSERVICEDEL_H_
#define RTMISDNSERVICEDEL_H_


#include "SerializeObject.h"


class CRtmIsdnServiceName : public CSerializeObject
{
CLASS_TYPE_1(CRtmIsdnServiceName, CSerializeObject)

public:
	CRtmIsdnServiceName();
	CRtmIsdnServiceName(const CRtmIsdnServiceName &other);
	virtual ~CRtmIsdnServiceName();


	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	int	 DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone(){return new CRtmIsdnServiceName;}

	const char* GetName() const;
	void SetName(const char *serviceName);

private:
	std::string m_name;
};


#endif /*RTMISDNSERVICEDEL_H_*/
