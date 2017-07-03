#ifndef RTMISDNSERVICELISTGET_H_
#define RTMISDNSERVICELISTGET_H_


#include "SerializeObject.h"

class CRtmIsdnMngrProcess;


class CRtmIsdnServiceListGet : public CSerializeObject
{
CLASS_TYPE_1(CRtmIsdnServiceListGet, CSerializeObject)
public:
	CRtmIsdnServiceListGet();
	CRtmIsdnServiceListGet(const CRtmIsdnServiceListGet &other);
	CRtmIsdnServiceListGet& operator = (const CRtmIsdnServiceListGet& other);
	virtual ~CRtmIsdnServiceListGet();


	virtual void SerializeXml(CXMLDOMElement*& pActionsNode) const;
	int		DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CRtmIsdnServiceListGet;}


protected:
 	CRtmIsdnMngrProcess* m_pProcess;
 };


#endif /*RTMISDNSERVICELISTGET_H_*/
