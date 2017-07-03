#ifndef SYSLOGAPPENDERCONFIGURATION_H_
#define SYSLOGAPPENDERCONFIGURATION_H_

#include "AppenderConfiguration.h"

#include <vector>
#include <string>

using namespace std;


class CSysLogAppenderConfiguration : public CAppenderConfiguration
{
CLASS_TYPE_1(CSysLogAppenderConfiguration,CAppenderConfiguration)
public:

    //Constructors
    
	CSysLogAppenderConfiguration();                                                                            
	CSysLogAppenderConfiguration(const CSysLogAppenderConfiguration &other);
	CSysLogAppenderConfiguration& operator = (const CSysLogAppenderConfiguration& other);
	virtual ~CSysLogAppenderConfiguration();
	virtual CSerializeObject* Clone() {return new CSysLogAppenderConfiguration();}  
	virtual void   SerializeXml(CXMLDOMElement*& pParentNode) const;
	virtual int    DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action);

	const char*  NameOf() const {return "CSysLogAppenderConfiguration";}  
	
  
};

/////////////////////////////////////////////////////////////////////////
#endif /*MODULECONTENT_H_*/



