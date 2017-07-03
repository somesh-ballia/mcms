#ifndef APPENDER_H_
#define APPENDER_H_

#include "ModuleContent.h"

#include <vector>
#include <string>
using namespace std;


class CAppenderConfiguration : public CSerializeObject
{
CLASS_TYPE_1(CAppenderConfiguration,CSerializeObject)
public:

    //Constructors
    
	CAppenderConfiguration();                                                                            
	CAppenderConfiguration(const CAppenderConfiguration &other);
	CAppenderConfiguration& operator=(const CAppenderConfiguration& other);
	virtual ~CAppenderConfiguration();
	virtual CSerializeObject* Clone() {return new CAppenderConfiguration();}  
	virtual void   SerializeXml(CXMLDOMElement*& pParentNode) const;
	virtual int    DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action);

	const char*  NameOf() const {return "CAppenderConfiguration";}  

	void								SetEnabled(BYTE bEnabled);
	BYTE							IsEnabled();
	void								SetIP(string	ip_addr);
	string							GetIP();
	void								SetPort(DWORD port);
	DWORD							GetPort();
	vector<CModuleContent*>&			GetModuleList();
	void								SetMoudleList(vector<CModuleContent*> moduleList);
	void								AddModule(CModuleContent* module);
	CModuleContent*					GetModuleByName(const char* moduleName);
	virtual void						CopyValue(CAppenderConfiguration* pOther);
	DWORD							GetIPIntValue();
            
protected:

	BYTE								m_isEnabled;
	DWORD								m_cliIP;
	DWORD								m_cliPort;
	vector<CModuleContent*>				m_moduleList;
	
  
};

/////////////////////////////////////////////////////////////////////////
#endif /*MODULECONTENT_H_*/



