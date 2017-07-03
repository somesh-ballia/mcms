#ifndef COMMRESRSRVDBGET_H_
#define COMMRESRSRVDBGET_H_

#include "SerializeObject.h"


class CCommResRsrvDBGet : public CSerializeObject
{
CLASS_TYPE_1(CCommResRsrvDBGet, CSerializeObject)
public:

	//Constructors
	CCommResRsrvDBGet();
	CCommResRsrvDBGet(const CCommResRsrvDBGet &other);
	CCommResRsrvDBGet& operator = (const CCommResRsrvDBGet& other);
	virtual ~CCommResRsrvDBGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
	CSerializeObject* Clone() {return new CCommResRsrvDBGet();}
		
	const char * NameOf() const {return "CCommResRsrvDBGet";}
protected:
};

#endif /*COMMRESRSRVDBGET_H_*/
