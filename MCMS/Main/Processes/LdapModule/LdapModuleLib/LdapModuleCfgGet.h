// CLdapModuleCfgGet.h: interface for the CLdapModuleCfgGet class.
//
//////////////////////////////////////////////////////////////////////

#ifndef LDAP_MODULE_CONFIGURATION_GET_H_
#define LDAP_MODULE_CONFIGURATION_GET_H_



#include "SerializeObject.h"
#include "LdapModuleProcess.h"
#include "psosxml.h"



///     EXTERNALS
// Ldap module configuration
extern CLdapModuleCfg* GetLdapModuleCfg();
extern void SetLdapModuleCfg(CLdapModuleCfg* pCfg);



class CLdapModuleCfgGet : public CSerializeObject
{
CLASS_TYPE_1(CLdapModuleCfgGet, CSerializeObject)
public:

	//Constructors
	CLdapModuleCfgGet();
	CLdapModuleCfgGet(const CLdapModuleCfgGet &other);
	CLdapModuleCfgGet& operator = (const CLdapModuleCfgGet& other);
	virtual ~CLdapModuleCfgGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CLdapModuleCfgGet();}
	const char * NameOf() const {return "CLdapModuleCfgGet";}
	
protected:
	CLdapModuleProcess* m_pProcess;
};




#endif /*LDAP_MODULE_CONFIGURATION_GET_H_*/
