#ifndef COMMRESDBGET_H_
#define COMMRESDBGET_H_

#include "SerializeObject.h"


class CCommResDBGet : public CSerializeObject
{
CLASS_TYPE_1(CCommResDBGet, CSerializeObject)
public:

	//Constructors
	CCommResDBGet();
	virtual const char* NameOf() const { return "CCommResDBGet";}
	CCommResDBGet(const CCommResDBGet &other);
	CCommResDBGet& operator = (const CCommResDBGet& other);
	virtual ~CCommResDBGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
	CSerializeObject* Clone() {return new CCommResDBGet();}
		
protected:
};

#endif /*COMMRESDBGET_H_*/
