/*
 * LdapModuleClientControl.h
 *
 *  Created on: Aug 10, 2010
 *      Author: yael
 */


#ifndef LDAPMODULECLIENTCONTROL_H_
#define LDAPMODULECLIENTCONTROL_H_
/* memory.c */
	/* simple macros to realloc for now */
#define LDAP_MALLOC(s)		(ber_memalloc_x((s),NULL))
#define LDAP_CALLOC(n,s)	(ber_memcalloc_x((n),(s),NULL))
#define LDAP_REALLOC(p,s)	(ber_memrealloc_x((p),(s),NULL))
#define LDAP_FREE(p)		(ber_memfree_x((p),NULL))
#define LDAP_VFREE(v)		(ber_memvfree_x((void **)(v),NULL))
#define LDAP_STRDUP(s)		(ber_strdup_x((s),NULL))
#define LDAP_STRNDUP(s,l)	(ber_strndup_x((s),(l),NULL))

#define LDAP_MALLOCX(s,x)	(ber_memalloc_x((s),(x)))
#define LDAP_CALLOCX(n,s,x)	(ber_memcalloc_x((n),(s),(x)))
#define LDAP_REALLOCX(p,s,x)	(ber_memrealloc_x((p),(s),(x)))
#define LDAP_FREEX(p,x)		(ber_memfree_x((p),(x)))
#define LDAP_VFREEX(v,x)	(ber_memvfree_x((void **)(v),(x)))
#define LDAP_STRDUPX(s,x)	(ber_strdup_x((s),(x)))
#define LDAP_STRNDUPX(s,l,x)	(ber_strndup_x((s),(l),(x)))


#include "StateMachine.h"
#include <sasl.h>
#include <ldap.h>
#include <map>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
using namespace std;



typedef struct lutil_sasl_defaults_s {
	char *mech;
	char *realm;
	char *authcid;
	char *passwd;
	char *authzid;
	char *mechVersion;
	char **resps;
	int nresps;
} lutilSASLdefaults;


class CLdapModuleCfg;

///     EXTERNALS
// Ldap module configuration
extern CLdapModuleCfg* GetLdapModuleCfg();
extern void SetLdapModuleCfg(CLdapModuleCfg* pCfg);



class CLdapModuleClientControl: public CStateMachine
{
CLASS_TYPE_1(CLdapModuleClientControl,CStateMachine )
public:
				// Constructors
	CLdapModuleClientControl(CTaskApp* pTask);
	virtual ~CLdapModuleClientControl();
				// base class overriding
	virtual void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	virtual const char* NameOf() const { return "CLdapModuleClientControl"; }

	void OnClientCheckServerAvailabilityTimeout(CSegment* pSeg);

	void CheckAndReportAdServerAvailability();
///	void AuthenticationLoginReq(CSegment* pMsg);
	CSegment* AuthenticationLoginReq(CSegment* pMsg);
	BOOL CheckAdServerAvailability(CLdapModuleCfg* pLdapModuleCfg);
	BOOL CheckAdServerAvailability(CLdapModuleCfg* pLdapModuleCfg, BOOL bUseAddressForConnection);
	void StartCheckServerAvailablityTimer();
	void SecureConnectionTest(std::ostream& answer);
	void SetLdapDebugMode(std::ostream& answer);
	void SetLdapNormalMode(std::ostream& answer);
	void TestMultipleOUs(std::ostream& answer);
	BOOL set_ldap_tls_options();
	string get_last_error(){return m_last_error;};
protected:
	PDECLAR_MESSAGE_MAP

	BOOL CheckAdConfiguration();
	void ReportAvailability(BOOL bIsAdServerAvailable);
	void ReportOCSPAvailability(BOOL bIsOCSPResponder,char * pStrReason);
///	void ReportLoginInd(DWORD requestId, STATUS statusAuthenticated);

	int ldap_x_sasl_digest_md5_bind_s(LDAP *ld,char *user_name,char *password,char *baseDn,LDAPControl **serverctrls,LDAPControl **clientctrls);
	int ldap_x_sasl_ntlm_bind_s(LDAP *ld,char *user_name,char *password,char *baseDn,LDAPControl **serverctrls,LDAPControl **clientctrls, DWORD &authorizationLevel);
	void* lutil_sasl_defaults(LDAP *ld,char *mech,char *realm,char *authcid,char *passwd,char *authzid, char* mechVersion);
	static int lutil_sasl_interact1(LDAP *ld,unsigned flags,void *defaults,	void *in );
	static int ldap_charray_add(char	***a,const char *s);
	static int interaction(unsigned flags,	sasl_interact_t *interact,	lutilSASLdefaults *defaults );
	BOOL IsActiveAlarmExistByErrorCode(WORD errorCode);
	void BuildRealmFromBaseDn(char* baseDn, char* realm);
	map<string , string> m_primaryGroup;
	BOOL m_bIsAdServerAvailable;
	string m_last_error;
	void buildLdapUrl(char* url,CLdapModuleCfg* pLdapModuleCfg,BOOL bUseAddressForConnection);
	BOOL set_ldap_tls_options(LDAP *ld);
private:
	void releaseMemDefaultsStruct(void *defaults);
	BOOL check_set_option_status(std::string action ,STATUS istatus);
	int  check_secure_bind(CLdapModuleCfg* pLdapModuleCfg);
	BOOL check_ou_exists(int& count);
	std::string prepareBaseDNwithMultipleOus(int OuIndex);
};
#endif /* LDAPMODULECLIENTCONTROL_H_ */
