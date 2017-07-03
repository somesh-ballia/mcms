#ifndef LOG4CXXCONFIGURATIONDBGET_H_
#define LOG4CXXCONFIGURATIONDBGET_H_



#include "LoggerProcess.h"


class CLog4cxxConfigurationDBGet : public CSerializeObject
{
CLASS_TYPE_1(CLog4cxxConfigurationDBGet,CSerializeObject)
public:

    //Constructors
    
	CLog4cxxConfigurationDBGet();                                                                            
	CLog4cxxConfigurationDBGet(const CLog4cxxConfigurationDBGet &other);
	CLog4cxxConfigurationDBGet& operator = (const CLog4cxxConfigurationDBGet& other);
	virtual ~CLog4cxxConfigurationDBGet();
	virtual CSerializeObject* Clone() {return new CLog4cxxConfigurationDBGet();}  
	virtual void   SerializeXml(CXMLDOMElement*& pParentNode) const;
	virtual int    DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action);

	const char*  NameOf() const {return "CLog4cxxConfigurationDBGet";}  

	

	
            
protected:

	CLoggerProcess*			m_loggerProcess;
  
};

/////////////////////////////////////////////////////////////////////////
#endif /*MODULECONTENT_H_*/



