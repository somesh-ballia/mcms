// CommFaultsDBGet.h: interface for the CCommFaultsDBGet class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//19/10/05		Vasily			  Class for Get XML Faults List
//========   ==============   =====================================================================


#ifndef __COMMFAULTSDBGET_H_
#define __COMMFAULTSDBGET_H_


#include "SerializeObject.h"


class CCommFaultsDBGet : public CSerializeObject
{
CLASS_TYPE_1(CCommFaultsDBGet, CSerializeObject)
public:

	//Constructors
	CCommFaultsDBGet();
	CCommFaultsDBGet& operator = (const CCommFaultsDBGet& other);
	virtual ~CCommFaultsDBGet();

	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pFaultListNode,char *pszError, const char* action);
	CSerializeObject* Clone() { return new CCommFaultsDBGet(); }

	
  
protected:
//	CCardsProcess* m_pProcess;
	DWORD	m_id;
};


class CCommFaultsShortDBGet : public CSerializeObject
{
CLASS_TYPE_1(CCommFaultsShortDBGet, CSerializeObject)
public:

	//Constructors
	CCommFaultsShortDBGet();
	CCommFaultsShortDBGet& operator = (const CCommFaultsShortDBGet& other);
	virtual ~CCommFaultsShortDBGet();

	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pFaultListNode,char *pszError, const char* action);
	CSerializeObject* Clone() { return new CCommFaultsShortDBGet(); }

	
  
protected:
//	CCardsProcess* m_pProcess;
	DWORD	m_id;
};



#endif // !defined(__COMMFAULTSDBGET_H_)

