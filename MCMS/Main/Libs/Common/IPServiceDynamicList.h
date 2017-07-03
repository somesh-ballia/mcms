// IPServiceDynamicList.h: interface for the CIPServiceDynamicList class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		does serialization of dynamic properties of ip services
//========   ==============   =====================================================================


#ifndef __IPSERVICEDYNAMICLIST_H__
#define __IPSERVICEDYNAMICLIST_H__


#include "SerializeObject.h"

class CIPServiceList;


class CIPServiceFullList : public CSerializeObject  
{
public:
	CIPServiceFullList();
	CIPServiceFullList(const CIPServiceFullList & other);
	virtual ~CIPServiceFullList();
	
	virtual const char* NameOf() const { return "CIPServiceFullList";}
	virtual CSerializeObject* Clone();
	
	virtual void SerializeXml(CXMLDOMElement* pFatherNode,DWORD ObjToken)const;
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int DeSerializeXml(class CXMLDOMElement *,char *,const char *);
	
	void SetIPServiceList(CIPServiceList *ipServiceList);
	
private:
	CIPServiceList *m_IPServiceList;
};

#endif // __IPSERVICEDYNAMICLIST_H__
