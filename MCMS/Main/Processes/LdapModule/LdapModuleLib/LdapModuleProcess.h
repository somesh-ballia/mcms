// LdapModuleProcess.h

#ifndef LDAP_MODULE_PROCESS_H_
#define LDAP_MODULE_PROCESS_H_

#include "ProcessBase.h"

class CLdapModuleCfg;

class CLdapModuleProcess : public CProcessBase  
{
CLASS_TYPE_1(CLdapModuleProcess, CProcessBase)
public:
	friend class CTestLdapModuleProcess;

	CLdapModuleProcess();
	virtual ~CLdapModuleProcess();
	virtual eProcessType GetProcessType() {return eProcessLdapModule;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();
	BOOL GetLdapCfg(CLdapModuleCfg &ldapModuleCfg) const;
	void SetLdapCfg(const CLdapModuleCfg &ldapModuleCfg);
	virtual int 	GetProcessAddressSpace() {return 96 * 1024 * 1024;};
	virtual DWORD GetMaxTimeForIdle(void) const
	{
	  return 12000;
	}
	//virtual int GetProcessAddressSpace() {return 64 * 1024 * 1024;}

protected:
	CLdapModuleCfg* m_pLdapModuleCfg;
};

#endif  // LDAP_MODULE_PROCESS_H_
