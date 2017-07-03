// LdapModuleManager.h: interface for the CLdapModuleManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_LDAPMODULE_MANAGER_H__)
#define _LDAPMODULE_MANAGER_H__

#include "ManagerTask.h"
#include "Macros.h"
#include "LdapModuleClientControl.h"

class CLdapModuleCfg;

///     EXTERNALS
// Ldap module configuration
extern CLdapModuleCfg* GetLdapModuleCfg();
extern void SetLdapModuleCfg(CLdapModuleCfg* pCfg);


void LdapModuleManagerEntryPoint(void* appParam);

class CLdapModuleManager : public CManagerTask
{
CLASS_TYPE_1(CLdapModuleManager,CManagerTask )
public:
	CLdapModuleManager();
	virtual ~CLdapModuleManager();

	void ManagerPostInitActionsPoint();

	TaskEntryPoint GetMonitorEntryPoint();

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS

	STATUS HandleSetActiveDirCfg(CRequest *pRequest);
	void OnCheckAdServerAvailablilty(CSegment* pParam);
	void UpdateLdapCfgParams(CLdapModuleCfg* pNewLdapCfg);

	void OnAuthenticationProcessLdapConfigurationReq(CSegment* pMsg);
	void OnAuthenticationLoginReq(CSegment* pMsg);
	void OnLdapClientAdServerAvailableInd(CSegment* pMsg);
	void OnLdapClientOCSPResponderAvailableInd(CSegment* pMsg);
	void OnLdapClientAuthenticationLoginInd(CSegment* pMsg);
	void OnLdapSecurityPKICfg(CSegment* pMsg);
	STATUS HandleSecureConnection(CSegment * seg,std::ostream& answer);
	STATUS HandleSetLdapNormalMode(CSegment * seg,std::ostream& answer);
	STATUS HandleSetLdapDebugMode(CSegment * seg,std::ostream& answer);
	STATUS HandleMultipleOUs(CSegment * seg,std::ostream& answer);
protected:
	//Action functions:
	void OnCheckStartupEndTimerAnycase();
	void OnLdapAddActiveAlarm(CSegment* pMsg);

	//Operation functions:
	BOOL IsStartupFinished() const;
	void InformAuthenticationProcessOnLdapConfiguration();
	void HandleActiveAlarm(BOOL bIsAdServerAvailable,string &errorMsg);
	void CreateNatRuleIfNeeded();

private:
	DWORD ConvertExtDBStatus(DWORD status);

	CLdapModuleClientControl*	m_pLdapModuleClientControl;
	CLdapModuleCfg*		m_pLdapModuleCfg;
	std::string 		m_delNatCmd;
};

#endif // !defined(_LDAPMODULE_MANAGER_H__)
