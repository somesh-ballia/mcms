#ifndef _CONF_TEMPLATE_DB_GET_H_
#define _CONF_TEMPLATE_DB_GET_H_

#include "SerializeObject.h"


class CConfTemplateDBGet : public CSerializeObject
{
CLASS_TYPE_1(CConfTemplateDBGet, CSerializeObject)
public:
	//Constructors
	CConfTemplateDBGet();
	CConfTemplateDBGet(const CConfTemplateDBGet &other);
	CConfTemplateDBGet& operator = (const CConfTemplateDBGet& other);
	virtual ~CConfTemplateDBGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
	CSerializeObject* Clone() {return new CConfTemplateDBGet();}
		
	const char * NameOf() const {return "CConfTemplateDBGet";}
protected:
};

#endif 
