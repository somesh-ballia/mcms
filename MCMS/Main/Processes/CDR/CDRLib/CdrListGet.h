#ifndef CDRLISTGET_H_
#define CDRLISTGET_H_

#include "SerializeObject.h"

class CCdrList;

class CCdrListGet : public CSerializeObject
{
CLASS_TYPE_1(CCdrListGet, CSerializeObject)
public:
	CCdrListGet();
	CCdrListGet(CCdrList *cdrList);
	virtual ~CCdrListGet();

	const char * NameOf(void) const {return "CCdrListGet";}
	virtual CSerializeObject* Clone();
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

private:
	CCdrList *m_CdrList;
};

#endif /*CDRLISTGET_H_*/
