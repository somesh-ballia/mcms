#ifndef LOCALLOGAPPEDNERCONFIGURATION_H_
#define LOCALLOGAPPEDNERCONFIGURATION_H_

#include "AppenderConfiguration.h"

class CLocalLogAppenderConfiguration : public CAppenderConfiguration
{
CLASS_TYPE_1(CLocalLogAppenderConfiguration,CAppenderConfiguration)
public:

    //Constructors
    
	CLocalLogAppenderConfiguration();                                                                            
	CLocalLogAppenderConfiguration(const CLocalLogAppenderConfiguration &other);
	CLocalLogAppenderConfiguration& operator= (const CLocalLogAppenderConfiguration& other);
	virtual ~CLocalLogAppenderConfiguration();
	virtual CSerializeObject* Clone() {return new CLocalLogAppenderConfiguration();}  
	virtual void   SerializeXml(CXMLDOMElement*& pParentNode) const;
	virtual int    DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action);

	const char*  NameOf() const {return "CLocalLogAppenderConfiguration";}  

	
  
};

/////////////////////////////////////////////////////////////////////////
#endif /*MODULECONTENT_H_*/



