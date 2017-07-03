#ifndef RTMISDNPHONENUMBERRANGESET_H_
#define RTMISDNPHONENUMBERRANGESET_H_


#include "SerializeObject.h"
#include "RtmIsdnPhoneNumberRange.h"



class CRtmIsdnPhoneNumberRangeSet : public CSerializeObject
{
CLASS_TYPE_1(CRtmIsdnPhoneNumberRangeSet, CSerializeObject)

public:
	CRtmIsdnPhoneNumberRangeSet();
	CRtmIsdnPhoneNumberRangeSet(const CRtmIsdnPhoneNumberRangeSet &other);
	virtual ~CRtmIsdnPhoneNumberRangeSet();


	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	int	 DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone(){return new CRtmIsdnPhoneNumberRangeSet;}

	const char* GetServiceName() const;
	void SetServiceName(const char *serviceName);
	
	const char* GetFirstPhoneNumber() const;
	CRtmIsdnPhoneNumberRange* GetPhoneNumRange() const;


private:
	std::string m_serviceName;
	CRtmIsdnPhoneNumberRange *m_pPhoneNumRange;
};














#endif /*RTMISDNPHONENUMBERRANGESET_H_*/
