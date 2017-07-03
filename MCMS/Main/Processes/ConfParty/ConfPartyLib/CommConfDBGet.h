// CommConfDBGet.h: interface for the CCommConfDBGet class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Add XML Reservation
//========   ==============   =====================================================================


#if !defined(_CommConfDBGet_H__)
#define _CommConfDBGet_H__


#include "psosxml.h"
#include "SerializeObject.h"
#include "CommConfDB.h"


class CCommConfDBGet : public CSerializeObject
{
CLASS_TYPE_1(CCommConfDBGet, CSerializeObject)
public:

	//Constructors
	CCommConfDBGet();
	virtual const char* NameOf() const { return "CCommConfDBGet";}
	CCommConfDBGet(const CCommConfDBGet &other);
	CCommConfDBGet& operator = (const CCommConfDBGet& other);
	virtual ~CCommConfDBGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError);
	CSerializeObject* Clone() {return new CCommConfDBGet();}

	//int convertStrActionToNumber(const char * strAction);

	
  
protected:

};


class CCommConfDBGetFull : public CSerializeObject
{
CLASS_TYPE_1(CCommConfDBGetFull, CSerializeObject)
public:

	//Constructors
	CCommConfDBGetFull();
	CCommConfDBGetFull(const CCommConfDBGetFull &other);
	CCommConfDBGetFull& operator = (const CCommConfDBGetFull& other);
	virtual ~CCommConfDBGetFull();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError);
	CSerializeObject* Clone() {return new CCommConfDBGetFull();}

	//int convertStrActionToNumber(const char * strAction);

	const char * NameOf() const {return "CCommConfDBGetFull";}
	
  
protected:

	

};

#endif // !defined(_CommConfDBGet_H__)

