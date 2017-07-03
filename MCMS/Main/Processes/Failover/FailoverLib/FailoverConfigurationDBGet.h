#ifndef _CONF_TEMPLATE_DB_GET_H_
#define _CONF_TEMPLATE_DB_GET_H_

#include "SerializeObject.h"
#include "FailoverConfiguration.h"

class CFailoverConfigurationDBGet : public CSerializeObject
{
CLASS_TYPE_1(CFailoverConfigurationDBGet, CSerializeObject)
public:
	//Constructors
	CFailoverConfigurationDBGet();
	CFailoverConfigurationDBGet(CFailoverConfiguration * pFailoverConfiguration);
	CFailoverConfigurationDBGet(const CFailoverConfigurationDBGet &other);
	CFailoverConfigurationDBGet& operator = (const CFailoverConfigurationDBGet& other);
	virtual ~CFailoverConfigurationDBGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
	CSerializeObject* Clone() {return new CFailoverConfigurationDBGet();}

	const char * NameOf() const {return "CFailoverConfigurationDBGet";}

private:
	//CFailoverConfiguration * m_pFailoverConfiguration;
	CFailoverProcess* m_pProcess;
};

#endif
