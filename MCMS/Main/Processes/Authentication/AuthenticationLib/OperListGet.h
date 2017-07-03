#ifndef __OPER_LIST_GET_H__
#define __OPER_LIST_GET_H__

#include "SerializeObject.h"


class CXMLDOMElement;


class COperListGet : public CSerializeObject

{

public:

	COperListGet();

	COperListGet(const COperListGet &other,WORD nReqAuthorization=ANONYMOUS);

	COperListGet& operator= (const COperListGet &other);

	~COperListGet();

	const char * NameOf(void) const {return "COperListGet";}

	void SerializeXml(CXMLDOMElement*& pActionsNode) const;

	int DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction);

	CSerializeObject* Clone() {return new COperListGet(*this);}

private:
	WORD m_nReqAuthorization;
};

#endif  // __OPER_LIST_GET_H__
