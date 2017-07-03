/*
 * LdapModuleClientControl.cpp
 *
 *  Created on: Aug 10, 2010
 *      Author: yael
 */

#include <lber.h>
#include <lber_pvt.h>
#include <stdlib.h>
#include <algorithm>



#include "LdapModuleClientControl.h"
#include "OpcodesMcmsInternal.h"
#include "TraceStream.h"
#include "ManagerApi.h"
#include "ProcessBase.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "LdapModuleCfg.h"
#include "FaultsDefines.h"
#include "TaskApi.h"
#include "ManagerApi.h"
#include "SystemTick.h"
#include "LdapModuleCfg.h"
//#include "TaskApp.cpp"
#include "OsTask.h"

#define RESET_ERROR_CODE -111111



const WORD CLIENT_CHECK_SERVER_AVAILABILITY_TIMEOUT	= 60 * SECOND;

extern char* LdapDirPortToString(int ldapDirPort);
extern char* LdapAuthenticationTypeToString(int ldapAuthenticationType);
/////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CLdapModuleClientControl)
	ONEVENT(LDAP_CLIENT_CHECK_SERVER_AVAILABILITY_TIMER,	ANYCASE,	CLdapModuleClientControl::OnClientCheckServerAvailabilityTimeout)

PEND_MESSAGE_MAP(CLdapModuleClientControl,CStateMachine);

/////////////////////////////////////////////////////////////////////////////
CLdapModuleClientControl::CLdapModuleClientControl(CTaskApp* pTask) // creator task		: CStateMachine(pTask)
{
	m_bIsAdServerAvailable = TRUE;
	m_last_error = "";
	set_ldap_tls_options();
	if (::GetLdapModuleCfg()->GetEnableDirServices())
			StartTimer(LDAP_CLIENT_CHECK_SERVER_AVAILABILITY_TIMER, CLIENT_CHECK_SERVER_AVAILABILITY_TIMEOUT);


	m_primaryGroup["500"]="Administrator";
	m_primaryGroup["501"]="Guest";
	m_primaryGroup["502"]="KRBTGT";
	m_primaryGroup["512"]="Domain Admins";
	m_primaryGroup["513"]="Domain Users";
	m_primaryGroup["514"]="Domain Guests";
	m_primaryGroup["515"]="Domain Computers";
	m_primaryGroup["516"]="Domain Controllers";
	m_primaryGroup["517"]="Cert Publishers";
	m_primaryGroup["518"]="Schema Admins";
	m_primaryGroup["519"]="Enterprise Admins";
	m_primaryGroup["520"]="Group Policy Creator Owners";
	m_primaryGroup["553"]="RAS and IAS Servers";
	m_primaryGroup["544"]="Administrators";
	m_primaryGroup["545"]="Users";
	m_primaryGroup["546"]="Guests";
	m_primaryGroup["547"]="Power Users";
	m_primaryGroup["548"]="Account Operators";
	m_primaryGroup["549"]="Server Operators";
	m_primaryGroup["550"]="Print Operators";
	m_primaryGroup["551"]="Backup Operators";
	m_primaryGroup["552"]="Replicators";


}

/////////////////////////////////////////////////////////////////////////////
CLdapModuleClientControl::~CLdapModuleClientControl()     // destructor
{
	sasl_done();
}

/////////////////////////////////////////////////////////////////////////////
void* CLdapModuleClientControl::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CLdapModuleClientControl::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode,pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CLdapModuleClientControl::OnClientCheckServerAvailabilityTimeout(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::OnClientCheckServerAvailabilityTimeout";
	if (::GetLdapModuleCfg()->GetEnableDirServices() == FALSE)
		return;
	//if(::GetLdapModuleCfg()->GetDirPort()==e636)
		
	CLdapModuleCfg* pLdapModuleCfg = ::GetLdapModuleCfg();
	BOOL bIsAdServerAvailable = CheckAdServerAvailability(pLdapModuleCfg);
	if (m_bIsAdServerAvailable != bIsAdServerAvailable)
	{
		m_bIsAdServerAvailable = bIsAdServerAvailable;
		ReportAvailability(bIsAdServerAvailable);
	}

	if (::GetLdapModuleCfg()->GetEnableDirServices())
		StartTimer(LDAP_CLIENT_CHECK_SERVER_AVAILABILITY_TIMER, CLIENT_CHECK_SERVER_AVAILABILITY_TIMEOUT);
}

/////////////////////////////////////////////////////////////////////////////
void CLdapModuleClientControl::CheckAndReportAdServerAvailability()
{
	TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::CheckAndReportAdServerAvailability";
	BOOL bIsAdServerConfigurationOK = CheckAdConfiguration();
	if (bIsAdServerConfigurationOK == FALSE)
	{
		m_bIsAdServerAvailable = bIsAdServerConfigurationOK;
		return;
	}

	CLdapModuleCfg* pLdapModuleCfg = ::GetLdapModuleCfg();
	BOOL bIsAdServerAvailable = CheckAdServerAvailability(pLdapModuleCfg);
	m_bIsAdServerAvailable = bIsAdServerAvailable;
	ReportAvailability(bIsAdServerAvailable);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CLdapModuleClientControl::CheckAdConfiguration()
{
	TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::CheckAdConfiguration";

	CLdapModuleCfg* pLdapModuleCfg = ::GetLdapModuleCfg();
	if (pLdapModuleCfg->GetEnableDirServices() == FALSE)
	{
		TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::CheckAdConfiguration - AD is not enabled";
		return FALSE;
	}

	// 1. Validate configuration - Add active alarm if not valid
	STATUS validationStatus = pLdapModuleCfg->ValidateConfiguration();
	if (STATUS_OK != validationStatus)
	{
		CManagerApi api(eProcessLdapModule);
		CSegment* pSeg = new CSegment();
		*pSeg << (DWORD)AA_INVALID_LDAP_MODULE_CONFIGURATION
			  << "Invalid Ldap Module configuration";
		api.SendMsg(pSeg, LDAP_ADD_ACTIVE_ALARM);

		return FALSE;
	}

	// 2. In case of Jitc mode, only one user (administrator) is allowed - check with Authentication process
	if (pLdapModuleCfg->IsJitcMode())
	{
		if (STATUS_OK != pLdapModuleCfg->CheckOperatorList())
		{
			CManagerApi api(eProcessLdapModule);
			CSegment* pSeg = new CSegment();
			*pSeg << (DWORD)AA_INVALID_LOCAL_USER_ACCOUNT_LIST_FOR_LDAP
				  << "When Active Directory enabled in Jitc mode only one user (administrator) is allowed";
			api.SendMsg(pSeg, LDAP_ADD_ACTIVE_ALARM);

			return FALSE;
		}
	}
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
BOOL CLdapModuleClientControl::CheckAdServerAvailability(CLdapModuleCfg* pLdapModuleCfg)
{
	BOOL bUseAddressForConnection = TRUE;
	BOOL bIsAdServerAvailable = CheckAdServerAvailability(pLdapModuleCfg, bUseAddressForConnection);
	if ((!bIsAdServerAvailable))
	{
		if (0 != strcmp(pLdapModuleCfg->GetDirNameAddress(), pLdapModuleCfg->GetDirNameAddressForConnection()))
		{
			// try the original DirNameAddress in case we added [] (IPv6)
			TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::CheckAdServerAvailability - try the original DirNameAddress in case we added [] (IPv6)";
			bUseAddressForConnection = FALSE;
			bIsAdServerAvailable = CheckAdServerAvailability(pLdapModuleCfg, bUseAddressForConnection);
			if (bIsAdServerAvailable)	// if it works, we will use the original address
				pLdapModuleCfg->SetDirNameAddressForConnection(pLdapModuleCfg->GetDirNameAddress());
		}
	}

	return bIsAdServerAvailable;
}


/////////////////////////////////////////////////////////////////////////////
BOOL CLdapModuleClientControl::CheckAdServerAvailability(CLdapModuleCfg* pLdapModuleCfg, BOOL bUseAddressForConnection)
{
	PTRACE(eLevelInfoNormal,"CLdapModuleClientControl::CheckAdServerAvailability");

	if (pLdapModuleCfg->GetEnableDirServices() == FALSE)
	{
		TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::CheckAdServerAvailability - AD is not enabled";
		return FALSE;
	}
	
	
	const eLdapDirPort dirPort = pLdapModuleCfg->GetDirPort();
	
	char baseDn[512] = "";
	strncpy(baseDn, (char*)pLdapModuleCfg->GetBaseDn(), sizeof(baseDn)-1);
	baseDn[sizeof(baseDn)-1] = '\0';
	eLdapAuthenticationType authenticationType = pLdapModuleCfg->GetAuthenticationType();

	STATUS iLdapResult = LDAP_OPT_SUCCESS;
	LDAP *ld;
	//SSL *ssl_session = NULL;
	//SSL_CTX* ssl_ctx = NULL;

	char url[80] = "";
	
	buildLdapUrl(url,pLdapModuleCfg,bUseAddressForConnection);
	TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::CheckAdServerAvailability URL=" << url;
	ldap_initialize(&ld, url);
	if (ld == NULL)
	{
		FPASSERT(1);
		return FALSE;
	}

	int ldapVersion;
	ldap_get_option( ld, LDAP_OPT_PROTOCOL_VERSION, &ldapVersion );

	if (ldapVersion == LDAP_VERSION2)
	{
		ldapVersion = LDAP_VERSION3;
		int iLdapSetResult = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldapVersion);
	}

	char strAuthenticationType[80] = "";
	if (eNTLM == authenticationType)
		strcpy(strAuthenticationType, "GSS-SPNEGO");
	else
	{
		
		char* authenTypeStr = LdapAuthenticationTypeToString(authenticationType);
		if(authenTypeStr)
		{
			strncpy(strAuthenticationType, authenTypeStr, sizeof(authenTypeStr)-1);
			strAuthenticationType[sizeof(strAuthenticationType)-1] = '\0';
		}
	}

	int bindTimeout = ::GetLdapModuleCfg()->GetCheckAvailabilityBindTimeout();
	ldap_set_option(ld, LDAP_OPT_NETWORK_TIMEOUT, &bindTimeout);

	set_ldap_tls_options(ld);
	int  error_code =RESET_ERROR_CODE; // reset error code
	ldap_set_option(NULL, LDAP_OPT_X_PKI_ERROR_CODE, &error_code);
	CTaskApp *pTaskApp = COsTask::GetTaskApp();
	if(pTaskApp)
		pTaskApp->UnlockRelevantSemaphore();

	iLdapResult = ldap_sasl_bind_s(ld,baseDn,strAuthenticationType,NULL,NULL,NULL,NULL);


	if(pTaskApp)
		pTaskApp->LockRelevantSemaphore();
	char *pPkiError = NULL;
	ldap_get_option(NULL,LDAP_OPT_X_PKI_ERROR_CODE,&error_code);
	ldap_get_option(NULL,LDAP_OPT_X_PKI_ERROR,(void*)&pPkiError);

	if (iLdapResult != LDAP_SUCCESS && iLdapResult != LDAP_SASL_BIND_IN_PROGRESS)
	{
		TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::CheckAdServerAvailability = ldap_bind_s error = "<< iLdapResult << "description = " << ldap_err2string( iLdapResult );
		if(pPkiError && (error_code != RESET_ERROR_CODE) )
		{
			TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s - ldap_sasl_interactive_bind_s - pki last error code  "
					                   << error_code << " error string =" << pPkiError;
			m_last_error = pPkiError;

		}
		else
		{
			m_last_error =	ldap_err2string( iLdapResult );
		}
		ldap_unbind_ext_s(ld,NULL,NULL);
		if(pPkiError)
			ldap_memfree(pPkiError);
		return FALSE;
	}
	else
	{
		if(pLdapModuleCfg->getRevocationMethodType() == eOcsp)
		{
			if(error_code < 0)
				ReportOCSPAvailability(false,pPkiError);
			else
				ReportOCSPAvailability(true,pPkiError);
		}
		TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::CheckAdServerAvailability = success";
	}

	if(pPkiError)
		ldap_memfree(pPkiError);

	ldap_unbind_ext_s(ld,NULL,NULL);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CLdapModuleClientControl::ReportAvailability(BOOL bIsAdServerAvailable)
{
	CManagerApi api(eProcessLdapModule);
	CSegment* pSeg = new CSegment();
	*pSeg << (WORD)bIsAdServerAvailable;
	*pSeg << m_last_error;
	api.SendMsg(pSeg, LDAP_MODULE_AD_SERVER_AVAILABE_IND);
}
//////////////////////////////////////////////////////////////////////////

void CLdapModuleClientControl::ReportOCSPAvailability(BOOL bIsOCSPResponder,char * pStrReason)
{
	CManagerApi api(eProcessLdapModule);
	CSegment* pSeg = new CSegment();
	*pSeg << (WORD)bIsOCSPResponder;
	if(pStrReason && !bIsOCSPResponder)
		*pSeg << pStrReason;
	else
		*pSeg << "empty";
	api.SendMsg(pSeg,LDAP_MODULE_OCSP_SERVER_AVAILABE_IND );
}
/////////////////////////////////////////////////////////////////////////////
void CLdapModuleClientControl::StartCheckServerAvailablityTimer()
{
	StartTimer(LDAP_CLIENT_CHECK_SERVER_AVAILABILITY_TIMER, CLIENT_CHECK_SERVER_AVAILABILITY_TIMEOUT);
}

/////////////////////////////////////////////////////////////////////////////
///void CLdapModuleClientControl::AuthenticationLoginReq(CSegment* pMsg)
CSegment* CLdapModuleClientControl::AuthenticationLoginReq(CSegment* pMsg)
{
	STATUS statusAuthenticated = EXT_APP_ILLEGAL_USER_NAME_OR_PASSWORD;
	CSegment* pResponseSeg = new CSegment();

	DWORD requestId;
	DWORD authorizationLevel = ANONYMOUS;
	*pMsg >> requestId;

	CLdapModuleCfg* pLdapModuleCfg = ::GetLdapModuleCfg();
	if (pLdapModuleCfg->GetEnableDirServices() == FALSE)
	{
		TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::AuthenticationLoginReq - AD is not enabled";
///		ReportLoginInd(requestId, statusAuthenticated);
		*pResponseSeg << (DWORD)requestId;
		*pResponseSeg << (DWORD)statusAuthenticated;
		*pResponseSeg << (DWORD)authorizationLevel;
		return pResponseSeg;
	}

	DWORD userLen;
	*pMsg >> userLen;

	ALLOCBUFFER(userStr, userLen+1);
	userStr[userLen] = '\0';
	*pMsg >> userStr;

	DWORD passwordLen;
	*pMsg >> passwordLen;

	ALLOCBUFFER(passwordStr, passwordLen+1);
	passwordStr[passwordLen] = '\0';
	*pMsg >> passwordStr;

	TRACESTR(eLevelInfoNormal) << "\nLdapModuleClientControl - AuthenticationLoginReq " << requestId << " " << userStr;


	char baseDn[512] = "";
  strncpy(baseDn, (char*)pLdapModuleCfg->GetBaseDn(), sizeof(baseDn)-1);
  baseDn[sizeof(baseDn)-1] = '\0';
	eLdapAuthenticationType authenticationType = pLdapModuleCfg->GetAuthenticationType();

	char url[80] = "";
	BOOL bUseAddressForConnection = TRUE;
	buildLdapUrl(url,pLdapModuleCfg,bUseAddressForConnection);
	TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::AuthenticationLoginReq URL=" << url;
	LDAP *ld;
	int ldapVersion = LDAP_VERSION3;
    //struct berval   cred;

    ldap_initialize(&ld,url);

    int iLdapSetResult = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldapVersion);

    //cred.bv_val = strdup(passwordStr);
	//cred.bv_len = strlen( passwordStr );

	char dnReq[512] = "";
	snprintf(dnReq, 512, "%s", baseDn);

	TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::AuthenticationLoginReq - ldap_sasl_bind_s dnReq =" << dnReq;

	STATUS iLdapResult = LDAP_OPT_SUCCESS;
	LDAPControl **servcred = NULL;
	LDAPControl **clientctrls=NULL;

	CTaskApp::Unlocker unlocker;
	if (eNTLM == authenticationType)
		iLdapResult = ldap_x_sasl_ntlm_bind_s(ld,userStr,passwordStr,baseDn,servcred,clientctrls,authorizationLevel);
	else
		//iLdapResult = ldap_x_sasl_digest_md5_bind_s(ld,userStr,passwordStr,baseDn,servcred,clientctrls);
		iLdapResult = LDAP_AUTH_METHOD_NOT_SUPPORTED;

	if (iLdapResult == LDAP_SUCCESS || iLdapResult == LDAP_SASL_BIND_IN_PROGRESS)
	{
		TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::AuthenticationLoginReq - ldap_sasl_bind_s,  iLdapResult= " << iLdapResult;
		statusAuthenticated = EXT_APP_STATUS_OK;
	}
	else if (iLdapResult == LDAP_INVALID_CREDENTIALS)
	{
		statusAuthenticated = EXT_APP_ILLEGAL_USER_NAME_OR_PASSWORD;
	}
	else
	{
		statusAuthenticated = EXT_APP_INTERNAL_ERROR;
	}

	ldap_unbind_ext_s(ld,NULL,NULL);
	sasl_client_init(NULL);
	TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::AuthenticationLoginReq - ldap_sasl_bind_s, user name: < " << userStr << " >, iLdapResult= " << iLdapResult << ",authentication level="<< authorizationLevel;
	cout << "CLdapModuleClientControl::AuthenticationLoginReq - ldap_sasl_bind_s,  iLdapResult= " << iLdapResult << ",authentication level="<< authorizationLevel;

	*pResponseSeg << (DWORD)requestId;
	*pResponseSeg << (DWORD)statusAuthenticated;
	*pResponseSeg << (DWORD)authorizationLevel;
 
	DEALLOCBUFFER(userStr);
	DEALLOCBUFFER(passwordStr);

	return pResponseSeg;
}



/////////////////////////////////////////////////////////////////////////////
int CLdapModuleClientControl::ldap_x_sasl_digest_md5_bind_s(LDAP *ld,char *user_name,char *password,char *baseDn,LDAPControl **serverctrls,LDAPControl **clientctrls)
{
	struct berval	*challenge = NULL;
	int		errnum;
	char		*digest = NULL;
	struct berval	resp;

	//LDAPDebug(LDAP_DEBUG_TRACE, "ldap_x_sasl_digest_md5_bind_s\n", 0, 0, 0);

	/* Add debug */
	if (ld == NULL || user_name == NULL || password==NULL)
		return (LDAP_PARAM_ERROR);
	int ldapVal = LDAP_VERSION3;
	ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldapVal);
	char* autId=user_name; ///*realm = "israel.polycom.com";

	// parse the Base DN string for building the realm string
	char realm[512] = "";
	BuildRealmFromBaseDn(baseDn, realm);

	char* mechVersion = "2";
	void* defaults;
	defaults = lutil_sasl_defaults(ld,"DIGEST-MD5",realm,autId,password,autId, mechVersion);

///	errnum = ldap_sasl_interactive_bind_s( ld, "dc=israel,dc=polycom,dc=com", "digest-md5",serverctrls,clientctrls, 0, &CLdapModuleClientControl::lutil_sasl_interact1, defaults );
	errnum = ldap_sasl_interactive_bind_s( ld, baseDn, "digest-md5",serverctrls,clientctrls, 0, &CLdapModuleClientControl::lutil_sasl_interact1, defaults );
    ber_memfree(defaults);
	return (errnum);
}

/////////////////////////////////////////////////////////////////////////////
int CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s(LDAP *ld,char *user_name,char *password,char *baseDn,LDAPControl **serverctrls,LDAPControl **clientctrls, DWORD &authorizationLevel)
{
	struct berval	*challenge = NULL;
	int		errnum;
	char		*digest = NULL;
	struct berval	resp;
	struct berval	**msgs = NULL;

	//LDAPDebug(LDAP_DEBUG_TRACE, "ldap_x_sasl_ntlm_bind_s\n", 0, 0, 0);

	if (ld == NULL || user_name == NULL || password==NULL)
		return (LDAP_PARAM_ERROR);
	int ldapVal = LDAP_VERSION3;
	ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldapVal);
	char* autId=user_name; ///*realm = "israel.polycom.com";

	// parse the Base DN string for building the realm string
	char realm[512] = "";
	BuildRealmFromBaseDn(baseDn, realm);

	char* mechVersion = "2";
	void* defaults;
	defaults = lutil_sasl_defaults(ld,"GSS-SPNEGO",realm,autId,password,autId, mechVersion);
///	errnum = ldap_sasl_interactive_bind_s( ld, " cn=Users,dc=israel,dc=polycom,dc=com", "GSS-SPNEGO",serverctrls,clientctrls, 0, &CLdapModuleClientControl::lutil_sasl_interact1, defaults );
	errnum = ldap_sasl_interactive_bind_s( ld, baseDn, "GSS-SPNEGO",serverctrls,clientctrls, 0, &CLdapModuleClientControl::lutil_sasl_interact1, defaults );
	//ber_memfree(defaults);
	if ( errnum != LDAP_SUCCESS )
	{
		TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s - ldap_sasl_interactive_bind_s - user name: < "
							   << user_name << " >  error value: < " << ldap_err2string( errnum ) << " >";
		char *pPkiError = NULL;
		int  error_code;
		ldap_get_option(NULL,LDAP_OPT_X_PKI_ERROR_CODE,&error_code);
		ldap_get_option(NULL,LDAP_OPT_X_PKI_ERROR,(void*)&pPkiError);
		if(pPkiError)
		{
			TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s - ldap_sasl_interactive_bind_s - pki last error code  "
					                   << error_code << " error string =" << pPkiError;
			ldap_memfree(pPkiError);
		}
		if ( NULL != defaults )
		{
		    releaseMemDefaultsStruct(defaults);
		}
		return( errnum );
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s - success";
	}
	int        msgid, parse_rc, finished = 0,i;
	struct timeval  zerotime;
	zerotime.tv_sec = zerotime.tv_usec = 0L;
	char      *matched_msg = NULL, *error_msg = NULL,*a, *dn;
	int        msgtype, num_entries = 0, num_refs = 0;
	LDAPMessage    *res = NULL, *msg,*e;
	BerElement     *ber;
	char           **referrals;
	struct berval	**vals = NULL;
	int sizeLim = 5000;
	static char *attrs[] = { "memberOf" ,"primaryGroupID"};
	  /* Perform the search operation. */
	string strUser = user_name;
	string filter = "(&(samAccountName="+strUser+"))";
//		string strBaseDnOrig = baseDn;
//		string strBaseDn="cn=Users," + strBaseDnOrig;
	int ouLength =0;
	if(check_ou_exists(ouLength))
	{
		TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s - ous exists if check original basedn and then parse ou to seperate paths";

		errnum = ldap_search_ext_s(ld, baseDn, LDAP_SCOPE_SUBTREE,filter.c_str(), attrs, 0, serverctrls, clientctrls,NULL, LDAP_NO_LIMIT, &res );
		if ( errnum != LDAP_SUCCESS )
		{
			for(int i=0;i< ouLength;i++)
			{
			
				string newBaseDN = prepareBaseDNwithMultipleOus(i);
				TRACESTR(eLevelInfoNormal) <<"CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s  new base DN="<<newBaseDN.c_str();
				errnum = ldap_search_ext_s(ld, newBaseDN.c_str(), LDAP_SCOPE_SUBTREE,filter.c_str(), attrs, 0, serverctrls, clientctrls,
										   NULL, LDAP_NO_LIMIT, &res );
				num_entries = ldap_count_entries( ld, res );
				if( errnum == LDAP_SUCCESS && num_entries>0)
					break;
			}
		}
	}
	else
	{
	  errnum = ldap_search_ext_s(ld, baseDn, LDAP_SCOPE_SUBTREE,filter.c_str(), attrs, 0, serverctrls, clientctrls,
							   NULL, LDAP_NO_LIMIT, &res );
	}
	if ( errnum != LDAP_SUCCESS )
	{
		TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s - ldap_search_ext_s - user name: < "
							   << user_name << " >  error value: < " << ldap_err2string( errnum ) << " >";
		fprintf( stderr, "ldap_search_ext_s: %s\n", ldap_err2string( errnum ) );
		if ( error_msg != NULL && *error_msg != '\0' ) {
			fprintf( stderr, "%s\n", error_msg );
		}
		if ( matched_msg != NULL && *matched_msg != '\0' ) {
			fprintf( stderr, "Part of the DN that matches an existing entry: %s\n", matched_msg );
		}
		return( errnum );
	}
	TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s after search" ;
	num_entries = ldap_count_entries( ld, res );
	num_refs = ldap_count_references( ld, res );

		  //fprintf( stderr, "num_entries=%s,num_refs=%s\n", num_entries, num_refs);
		  /* Iterate through the results. An LDAPMessage structure sent back from
		      a search operation can contain either an entry found by the search,
		      a search reference, or the final result of the search operation. */
//	while ((errnum = ldap_result( ld, LDAP_RES_ANY, LDAP_MSG_ALL,  NULL, &res )) > 0 )
//	{
	//cout << "primary group"<< m_primaryGroup[512];
		CLdapModuleCfg* pLdapModuleCfg = ::GetLdapModuleCfg();
		bool hasFoundRole = false;
		authorizationLevel = 0;		//initialize to the highest authorization level available as a staring value for comparison

		for ( msg = ldap_first_message( ld, res ); msg != NULL; msg = ldap_next_message( ld, msg ) )
		{
			/* Determine what type of message was sent from the server. */
			msgtype = ldap_msgtype( msg );

			switch( msgtype ) {
			/* If the result was an entry found by the search, get and print the
			   attributes and values of the entry. */
				case LDAP_RES_SEARCH_ENTRY:
					/* Get and print the DN of the entry. */
					if (( dn = ldap_get_dn( ld, res )) != NULL ) {
						printf( "dn: %s\n", dn );
						ldap_memfree( dn );
					}
					/* Iterate through each attribute in the entry. */
					for ( a = ldap_first_attribute( ld, res, &ber );
							a != NULL; a = ldap_next_attribute( ld, res, ber ) )
					{
						/* Get and print all values for each attribute. */
						if (( msgs = ldap_get_values_len( ld, res, a )) != NULL )
						{
							for ( i = 0; msgs[ i ] != NULL; i++ )
							{
								printf( "%s: %s\n", a, msgs[ i ]->bv_val );
								TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s - entry value: " << msgs[ i ]->bv_val;
								string attr=msgs[ i ]->bv_val;
								string strTemp, cnStr;
								size_t pos;
								//CLdapModuleCfg tmp;
								if (CLdapModuleCfg::ValidateBaseDn(attr) != STATUS_OK)//not a dn
								{
									TRACEINTO << "CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s - invalid Base DN";
									if (atoi(attr.c_str())!=0)//is a int..
									{
										TRACEINTO << "CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s - search result is an integer type - need to use m_primaryGroup array";
										cnStr = m_primaryGroup[attr];
									}
								}
								else //a memberOf
								{
									pos = attr.find("CN=")+3;
									strTemp = attr.substr (pos);
									pos = strTemp.find(",");
									cnStr=strTemp.substr(0,pos);
								}
								if ("" == cnStr)
								{
									TRACEINTO << "CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s - CN string (group) is empty!";
									continue;
								}

								// Check if the group of the current attribute is less privileged then the groups in the previous attributes
								// We should assign the user according to the least privileged group.
								string authorizationStr;

								if (pLdapModuleCfg->GetLdapAuditorRole() == cnStr)
								{
									authorizationLevel = AUDITOR;
									authorizationStr = "auditor";
									hasFoundRole = true;
								}
								else if (pLdapModuleCfg->GetLdapChairpersonRole() == cnStr)
								{
									if (authorizationLevel < AUTH_OPERATOR)
									{
										authorizationLevel = AUTH_OPERATOR;
										authorizationStr = "chairperson";
										hasFoundRole = true;
									}
								}
								else if (pLdapModuleCfg->GetLdapOperatorRole() == cnStr)
								{
									if (authorizationLevel < ORDINARY)
									{
										authorizationLevel = ORDINARY;
										authorizationStr = "operator";
										hasFoundRole = true;
									}
								}
								else if (pLdapModuleCfg->GetLdapAdministratorRole() == cnStr)
								{
									if (authorizationLevel <= SUPER)
									{
										authorizationLevel=SUPER;	//administrator
										authorizationStr = "administrator";
										hasFoundRole = true;
									}
								}

								// ADMINISTRATOR_READONLY is lower in level then AUDITOR, although it can also Backup Configuration in addition to
								//Pull audits and See all configuration , in terms of ldap if User belongs to Both Audit and Admin read only , the permission will be Admin Read Only
								else if (pLdapModuleCfg->GetLdapAdministratorReadOnlyRole() == cnStr)
								{
									if (authorizationLevel < ADMINISTRATOR_READONLY )
									{
										authorizationLevel= ADMINISTRATOR_READONLY;	//administrator read only
										authorizationStr = "administrator_readonly";
										hasFoundRole = true;
									}
								}
								
								if (hasFoundRole)
								{
									TRACEINTO << "CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s - "
											  << "a group matching RMX authorization level was found: \n "
											  << "authorizationLevel = " << authorizationStr << "\n"
											  << "cnStr = " << cnStr << "\n"
											  << "pLdapModuleCfg->GetLdapAdministratorRole() = " << pLdapModuleCfg->GetLdapAdministratorRole();
								}

							}
							ldap_value_free_len( msgs );
						}
						ldap_memfree( a );
					}
					if ( ber != NULL ) {
						ber_free( ber, 0 );
					}
					printf( "\n" );
					break;

				case LDAP_RES_SEARCH_REFERENCE:
					/* The server sent a search reference encountered during the
					 search operation. */
					/* Parse the result and print the search references.
					 Ideally, rather than print them out, you would follow the
					 references. */
					parse_rc = ldap_parse_reference( ld, msg, &referrals, NULL, 0 );
					if ( parse_rc != LDAP_SUCCESS ) {
						fprintf( stderr, "ldap_parse_result: %s\n", ldap_err2string( parse_rc ) );
						return( 1 );
					}
					if ( referrals != NULL ) {
						for ( i = 0; referrals[ i ] != NULL; i++ ) {
							printf( "Search reference: %s\n\n", referrals[ i ] );
						}
						delete referrals;
					}
					break;

				case LDAP_RES_SEARCH_RESULT:
					/* Parse the final result received from the server. Note the last
					 argument is a non-zero value, which indicates that the
					 LDAPMessage structure will be freed when done.  (No need
					 to call ldap_msgfree().) */
					parse_rc = ldap_parse_result( ld, msg, &errnum, &matched_msg, &error_msg, NULL, &serverctrls, 0 );
					if ( parse_rc != LDAP_SUCCESS ) {
						fprintf( stderr, "ldap_parse_result: %s\n", ldap_err2string( parse_rc ) );
						return( 1 );
					}
					/* Check the results of the LDAP search operation. */
					if ( errnum != LDAP_SUCCESS ) {
						fprintf( stderr, "ldap_search_ext: %s\n", ldap_err2string( errnum ) );
						if ( error_msg != NULL && *error_msg != '\0' ) {
							fprintf( stderr, "%s\n", error_msg );
						}
						if ( matched_msg != NULL && *matched_msg != '\0' ) {
							fprintf( stderr, "Part of the DN that matches an existing entry: %s\n", matched_msg );
						}
					} else {
						printf( "Search completed successfully.\n"
								"Entries found: %d\n"
								"Search references returned: %d\n",
								num_entries, num_refs );
					}
					break;

				default:
					break;

			}
		}
//	}
		TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s before the end " ;
	if (hasFoundRole)
		errnum = LDAP_SUCCESS;
	else
	{
		TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::ldap_x_sasl_ntlm_bind_s - ldap_search_ext_s - user name: < "
							   << user_name << " > -- ROLE WAS NOT FOUND";
		errnum = LDAP_INVALID_CREDENTIALS;
		authorizationLevel = ANONYMOUS;
	}
	ldap_msgfree( res );
	if ( NULL != defaults ) //to please klocwork ID : 4763
	{
		releaseMemDefaultsStruct(defaults);
	}
	return (errnum);
}
void CLdapModuleClientControl::releaseMemDefaultsStruct(void *defaults)
{
	lutilSASLdefaults *defs = (lutilSASLdefaults*)defaults;

	if (defs->mech) ber_memfree(defs->mech);
	if (defs->realm) ber_memfree(defs->realm);
	if (defs->authcid) ber_memfree(defs->authcid);
	if (defs->passwd) ber_memfree(defs->passwd);
	if (defs->authzid) ber_memfree(defs->authzid);
	if(defs->mechVersion) ber_memfree(defs->mechVersion);
	ber_memfree(defs);
}



/////////////////////////////////////////////////////////////////////////////
void* CLdapModuleClientControl::lutil_sasl_defaults(LDAP *ld,char *mech,char *realm,char *authcid,char *passwd,char *authzid, char* mechVersion)
{
	lutilSASLdefaults *defaults;

	defaults = (lutilSASLdefaults*)ber_memalloc(sizeof(lutilSASLdefaults));

	if (defaults == NULL)
		return NULL;

	defaults->mech = mech ? ber_strdup(mech) : NULL;
	defaults->realm = realm ? ber_strdup(realm) : NULL;
	defaults->authcid = authcid ? ber_strdup(authcid) : NULL;
	defaults->passwd = passwd ? ber_strdup(passwd) : NULL;
	defaults->authzid = authzid ? ber_strdup(authzid) : NULL;
	defaults->mechVersion = mechVersion ? ber_strdup(mechVersion) : NULL;

	if (defaults->mech == NULL)
	{
		ldap_get_option(ld, LDAP_OPT_X_SASL_MECH, &defaults->mech);
	}
	if (defaults->realm == NULL)
	{
		ldap_get_option(ld, LDAP_OPT_X_SASL_REALM, &defaults->realm);
	}
	if (defaults->authcid == NULL)
	{
		ldap_get_option(ld, LDAP_OPT_X_SASL_AUTHCID, &defaults->authcid);
	}
	if (defaults->authzid == NULL)
	{
		ldap_get_option(ld, LDAP_OPT_X_SASL_AUTHZID, &defaults->authzid);
	}

	defaults->resps = NULL;
	defaults->nresps = 0;

	return defaults;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
int CLdapModuleClientControl::interaction(unsigned flags, sasl_interact_t *interact, lutilSASLdefaults *defaults)
{
	const char *dflt = interact->defresult;
	char input[1024] = "";

	int noecho=0;
	int challenge=0;

	switch (interact->id)
	{
		case SASL_CB_GETREALM:
			if (defaults)
				dflt = defaults->realm;
			break;
		case SASL_CB_AUTHNAME:
			if (defaults) {
				dflt = defaults->authcid;
				//YossiG Klocwork NPD issue
				printf("SASL_CB_AUTHNAME = %s",(const char*)defaults->authcid);
			}
			break;
		case SASL_CB_PASS:
			if(defaults)
				dflt = defaults->passwd;
			noecho = 1;
			break;
		case SASL_CB_USER:
			if (defaults)
				dflt = defaults->authzid;
			break;
		case SASL_CB_MECHVERSION: //ntlm version 0x4040:
			if(defaults)
				dflt = defaults->mechVersion; //always 2 in the structure
			noecho = 1;
			break;
		case SASL_CB_NOECHOPROMPT:
			noecho = 1;
			challenge = 1;
			break;
		case SASL_CB_ECHOPROMPT:
			challenge = 1;
			break;
	}

	if (dflt && !*dflt)
		dflt = NULL;

	if (flags != LDAP_SASL_INTERACTIVE && ( dflt || interact->id == SASL_CB_USER ) )
	{
		goto use_default;
	}

	if (flags == LDAP_SASL_QUIET)
	{
		/* don't prompt */
		return LDAP_OTHER;
	}

	if (challenge)
	{
		if (interact->challenge)
		{
			cout << "Challenge: " << interact->challenge << endl;
		}
	}

	if (dflt)
	{
		cout << "Default:" <<  dflt << endl;
	}

	if (interact->prompt == NULL)
		snprintf( input, sizeof input, "%s: ", "Interact");
	else
		snprintf( input, sizeof input, "%s: ",(const char*)interact->prompt);

	if (noecho)
	{
		interact->result = (char*)getchar();
		if (interact->result==NULL)
			interact->len = 0;
		else
			interact->len = strlen((const char*)interact->result);
	}

	else
	{
		/* prompt user */
		fputs( input, stderr );

		/* get input */
		interact->result = fgets(input, sizeof(input), stdin);

		if (interact->result == NULL)
		{
			interact->len = 0;
			return LDAP_UNAVAILABLE;
		}

		/* len of input */
		interact->len = strlen(input);

		if (interact->len > 0 && input[interact->len - 1] == '\n')
		{
			/* input includes '\n', trim it */
			interact->len--;
			input[interact->len] = '\0';
		}
	}


	if (interact->len > 0)
	{
		/* duplicate */
		char *p = (char *)interact->result;
		if(defaults)
		{
			ldap_charray_add(&defaults->resps, (const char*)interact->result);
			interact->result = defaults->resps[defaults->nresps++];
		}
		/* zap */
		memset( p, '\0', interact->len );
	}

	else
	{
use_default:
		/* input must be empty */
		if (dflt!=NULL)
			interact->result = (void*)dflt;
		else
			interact->result = (void*)"";

		interact->len = strlen( (const char*)interact->result );
	}

	return LDAP_SUCCESS;
}

////////////////////////////////////////////////////////
int CLdapModuleClientControl::lutil_sasl_interact1(LDAP *ld,unsigned flags,void *defaults,	void *in )
{
	sasl_interact_t *interact = (sasl_interact_t *)in;

	if (ld == NULL)
		return LDAP_PARAM_ERROR;

	if (flags == LDAP_SASL_INTERACTIVE)
	{
		cout << "SASL Interaction\n";
	}

	while (interact->id != SASL_CB_LIST_END)
	{
		int rc = interaction( flags, interact, (lutilSASLdefaults*)defaults );

		if (rc)
			return rc;
		interact++;
	}

	return LDAP_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
int CLdapModuleClientControl::ldap_charray_add(char	***a,const char *s)
{
	int	n;

	if (*a == NULL)
	{
		*a = (char **) LDAP_MALLOC( 2 * sizeof(char *) );
		n = 0;

		if (*a == NULL)
		{
			return -1;
		}
	}

	else
	{
		char **new1;

		for (n = 0; *a != NULL && (*a)[n] != NULL; n++)
		{
			;	/* NULL */
		}

		new1 = (char **) LDAP_REALLOC( (char *) *a,
		    (n + 2) * sizeof(char *) );

		if (new1 == NULL)
		{
			/* caller is required to call ldap_charray_free(*a) */
			return -1;
		}

		*a = new1;
	}

	(*a)[n] = LDAP_STRDUP(s);

	if ((*a)[n] == NULL)
	{
		return 1;
	}

	(*a)[++n] = NULL;

	return 0;
}


/////////////////////////////////////////////////////////////////////////////
void CLdapModuleClientControl::BuildRealmFromBaseDn(char* baseDn, char* realm)
{
	char tmpBaseDn[512] = "";
	if (baseDn)
	{
	  strncpy(tmpBaseDn, baseDn, sizeof(tmpBaseDn)-1);
	  tmpBaseDn[sizeof(tmpBaseDn)-1] = '\0';
	}
	char* tmpStr = strtok(tmpBaseDn, "=");
	int cntr = 1;
	while (tmpStr != NULL)
	{
		if (cntr % 2 == 1)
		{
			tmpStr = (strtok(NULL, ","));
			if(tmpStr)
				strcat(realm, tmpStr);
			strcat(realm, ".");
		}
		else
		{
			tmpStr = (strtok(NULL, "="));
		}
		cntr++;
	}

	int length = strlen(realm);
	realm[length-1] = '\0';
}
// testing function from ca
void CLdapModuleClientControl::SecureConnectionTest(std::ostream& answer)
{
	STATUS statusAuthenticated = EXT_APP_ILLEGAL_USER_NAME_OR_PASSWORD;

	DWORD authorizationLevel = ANONYMOUS;
	CLdapModuleCfg* pLdapModuleCfg = ::GetLdapModuleCfg();
	if(NULL != pLdapModuleCfg)
	{
		answer << "get ldap configurations..." <<  endl;
		answer << pLdapModuleCfg->Dump();


		int iLdapSetResult = check_secure_bind(pLdapModuleCfg);


		if (iLdapSetResult == LDAP_SUCCESS || iLdapSetResult == LDAP_SASL_BIND_IN_PROGRESS)
		{
			answer << "ldap status  ldap success";
			TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::AuthenticationLoginReq - ldap_sasl_bind_s,  iLdapResult= " << iLdapSetResult;
			statusAuthenticated = EXT_APP_STATUS_OK;
		}
		else if (iLdapSetResult == LDAP_INVALID_CREDENTIALS)
		{
			answer << "ldap status  Invalid credentials";
			statusAuthenticated = EXT_APP_ILLEGAL_USER_NAME_OR_PASSWORD;
		}
		else
		{
			answer << "ldap status  internal error";
			statusAuthenticated = EXT_APP_INTERNAL_ERROR;
		}
		sasl_done();
		sasl_client_init(NULL);
		TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::AuthenticationLoginReq - ldap_sasl_bind_s " << " iLdapResult= " << iLdapSetResult << ",authentication level="<< authorizationLevel;
		cout << "CLdapModuleClientControl::AuthenticationLoginReq - ldap_sasl_bind_s,  iLdapResult= " << iLdapSetResult << ",authentication level="<< authorizationLevel;
	}
}

/////////////////////////////////////////////////////////////////////////
BOOL CLdapModuleClientControl::check_set_option_status(std::string action ,STATUS istatus)
{
	std::string header =  "CLdapModuleClientControl::check_set_option_status";
	if(LDAP_OPT_SUCCESS == istatus)
	{
		TRACESTR(eLevelInfoNormal) << header <<" set action = "<< action << "  success" ;
		return TRUE;
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << header <<" set action = "<< action << "  fail " << ldap_err2string(istatus);
		return FALSE;
	}
}

int  CLdapModuleClientControl::check_secure_bind(CLdapModuleCfg* pLdapModuleCfg)
{
		PTRACE(eLevelInfoNormal,"CLdapModuleClientControl::check_secure_bind");

		char baseDn[512] = {0};
		strncpy(baseDn, (char*)pLdapModuleCfg->GetBaseDn(),sizeof(baseDn)-1);
		eLdapAuthenticationType authenticationType = pLdapModuleCfg->GetAuthenticationType();

		STATUS iLdapResult = LDAP_OPT_SUCCESS;
		LDAP *ld;

		std::string url="ldaps://";

		const char* dirNameAddress = pLdapModuleCfg->GetDirNameAddressForConnection();
		url=url+ dirNameAddress;
		int iLdapSetResult  = ldap_initialize(&ld, url.c_str());
		if (ld == NULL)
		{
			FPASSERT(1);
			return iLdapSetResult;
		}

		int ldapVersion = LDAP_VERSION3;
		iLdapSetResult = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldapVersion);

		char strAuthenticationType[80] = {0};
		if (eNTLM == authenticationType)
			strcpy(strAuthenticationType, "GSS-SPNEGO");
		else
		{
			char* pStr = LdapAuthenticationTypeToString(authenticationType);
			if(pStr)
				strncpy(strAuthenticationType, pStr,sizeof(strAuthenticationType)-1);
		}

		int bindTimeout = ::GetLdapModuleCfg()->GetCheckAvailabilityBindTimeout();
		iLdapResult = ldap_set_option(ld, LDAP_OPT_NETWORK_TIMEOUT, &bindTimeout);

		CTaskApp *pTaskApp = COsTask::GetTaskApp();
		if(pTaskApp)
			pTaskApp->UnlockRelevantSemaphore();

		iLdapResult = ldap_sasl_bind_s(ld,baseDn,strAuthenticationType,NULL,NULL,NULL,NULL);

		if(pTaskApp)
			pTaskApp->LockRelevantSemaphore();


		if (iLdapResult != LDAP_SUCCESS && iLdapResult != LDAP_SASL_BIND_IN_PROGRESS)
		{
			TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::check_secure_bind = ldap_bind_s error = "<< iLdapResult;
			ldap_unbind_ext_s(ld,NULL,NULL);
			return iLdapSetResult;
		}
		else
			TRACESTR(eLevelInfoNormal) << "CLdapModuleClientControl::check_secure_bind = success";


		ldap_unbind_ext_s(ld,NULL,NULL);
		return iLdapSetResult;

}

void CLdapModuleClientControl::SetLdapDebugMode(std::ostream& answer)
{
	int debug_level = 7;
	int iLdapSetResult = ldap_set_option(NULL, LDAP_OPT_DEBUG_LEVEL, &debug_level);
	if(check_set_option_status("DEBUG_LEVEL",iLdapSetResult))
		answer << "LDAP is in Debug Mode" << endl;
	else
		answer << "FAILED: to set Ldap in Debug Mode" << endl;
}
void CLdapModuleClientControl::SetLdapNormalMode(std::ostream& answer)
{
	int debug_level = 0;
	int iLdapSetResult = ldap_set_option(NULL, LDAP_OPT_DEBUG_LEVEL, &debug_level);
	if(check_set_option_status("DEBUG_LEVEL",iLdapSetResult))
		answer << "LDAP is back to Normal Mode" << endl;
	else
		answer << "FAILED: to set LDAP to Normal Mode" << endl;
}

BOOL CLdapModuleClientControl::set_ldap_tls_options(LDAP *ld)
{
	BOOL status = TRUE;
	CLdapModuleCfg* pCfg = ::GetLdapModuleCfg();
	int iLdapSetResult =0;
	if(NULL != pCfg)
	{
		int val = LDAP_OPT_X_TLS_NEVER;
		if(pCfg->IsRequestPeerCertificate())
		{
			TRACESTR(eLevelInfoNormal) << "LDAP_OPT_X_TLS_REQUIRE_CERT - value = LDAP_OPT_X_TLS_DEMAND";
			 val = LDAP_OPT_X_TLS_DEMAND;

		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "LDAP_OPT_X_TLS_REQUIRE_CERT - value = LDAP_OPT_X_TLS_NEVER";
		}
		iLdapSetResult = ldap_set_option(ld,LDAP_OPT_X_TLS_REQUIRE_CERT,&val);
		status = check_set_option_status("TLS_REQUIRE_CERT",iLdapSetResult);

		if(pCfg->getRevocationMethodType() == eCrl)
		{
			    val = LDAP_OPT_REVOCATION_METHOD_CRL;
			    iLdapSetResult = ldap_set_option(NULL,LDAP_OPT_X_TLS_REV_METHOD,&val);
			    status = check_set_option_status("TLS_REV_METHOD",iLdapSetResult);
		    	//iLdapSetResult = ldap_set_option(ld,LDAP_OPT_X_TLS_CRLFILE,MCU_MCMS_DIR+"/CRL/ca.crl");
				//status = check_set_option_status("TLS_CRLFILE",iLdapSetResult);
				//crl
				//val = LDAP_OPT_X_TLS_CRL_NONE;
				val = LDAP_OPT_X_TLS_CRL_ALL;
				iLdapSetResult = ldap_set_option(ld,LDAP_OPT_X_TLS_CRLCHECK,&val);
				status = check_set_option_status("TLS_CRLCHECK",iLdapSetResult);
		}
		else
		{
			if(pCfg->getRevocationMethodType() == eOcsp)
			{
				BYTE bNONonceMode = TRUE;
				CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
				if(sysConfig != NULL)
				{
					sysConfig->GetBOOLDataByKey(CFG_OCSP_NO_NONCE, bNONonceMode);
					if(!bNONonceMode)
					{
						val = LDAP_OPT_OCSP_NONCE_YES;
						iLdapSetResult = ldap_set_option(NULL,LDAP_OPT_TLS_OCSP_NONCE,&val);
						status = check_set_option_status("TLS_OCSP_NONCE",iLdapSetResult);
					}
				}


				val = LDAP_OPT_REVOCATION_METHOD_OCSP;
				iLdapSetResult = ldap_set_option(NULL,LDAP_OPT_X_TLS_REV_METHOD,&val);
				status = check_set_option_status("TLS_REV_METHOD",iLdapSetResult);

				if(!pCfg->IsSkipValidationOcspCert())
				{
					val = LDAP_OPT_OCSP_CERTIFICATE_DEMAND;
				}
				else
				{
					val = LDAP_OPT_OCSP_CERTIFICATE_NEVER;
				}
				iLdapSetResult = ldap_set_option(NULL,LDAP_OPT_X_TLS_OCSP_DEMAND,&val);
				status = check_set_option_status("TLS_OCSP_DEMAND",iLdapSetResult);
				if(pCfg->IsUseResponderOcspUri())
				{
					val = LDAP_OPT_RESPONDER_OCSP_URI_YES;

				}
				else
				{
					val = LDAP_OPT_RESPONDER_OCSP_URI_NO;
				}
				iLdapSetResult = ldap_set_option(NULL,LDAP_OPT_X_TLS_OCSP_R_URI,&val);
				status = check_set_option_status("TLS_OCSP_R_URI",iLdapSetResult);
				const char* pStr = NULL;
				if(!(pCfg->GetOCSPGlobalResponderURI().empty()))
				{
					pStr = pCfg->GetOCSPGlobalResponderURI().c_str();

				}
				iLdapSetResult = ldap_set_option(NULL,LDAP_OPT_X_TLS_OCSP_URL,pStr);
				status = check_set_option_status("TLS_OCSP_URL",iLdapSetResult);
			}
		}
		if(pCfg->getRevocationMethodType() == eNoneMethod)
		{
			val = LDAP_OPT_REVOCATION_METHOD_NONE;
			iLdapSetResult = ldap_set_option(NULL,LDAP_OPT_X_TLS_REV_METHOD,&val);
			status = check_set_option_status("TLS_REV_METHOD",iLdapSetResult);
		}
		if(!pCfg->IsIncompleteRevocation())
		{
			val = LDAP_OPT_X_TLS_LOOSE_NEVER;
		}
		else
		{
			val = LDAP_OPT_X_TLS_LOOSE_ALLOW;
		}
		iLdapSetResult = ldap_set_option(NULL,LDAP_OPT_X_TLS_LOOSE,&val);
		status = check_set_option_status("LDAP_OPT_X_TLS_LOOSE",iLdapSetResult);

	}
	return status;
}

BOOL CLdapModuleClientControl::set_ldap_tls_options()
{
	BOOL status = TRUE;

	int iLdapSetResult = ldap_set_option(NULL,LDAP_OPT_X_TLS_CACERTFILE,(MCU_MCMS_DIR+"/CACert/ca-bundle-client.crt").c_str());
	status = check_set_option_status("TLS_CACERTFILE",iLdapSetResult);

	iLdapSetResult = ldap_set_option(NULL,LDAP_OPT_X_TLS_CACERTDIR,(MCU_MCMS_DIR+"/CACert").c_str());
	status = check_set_option_status("TLS_CACERTDIR",iLdapSetResult);

	iLdapSetResult = ldap_set_option(NULL,LDAP_OPT_X_TLS_CERTFILE,(MCU_MCMS_DIR+"/Keys/cert_off.pem").c_str());
	status = check_set_option_status("TLS_CERTFILE",iLdapSetResult);

	iLdapSetResult = ldap_set_option(NULL,LDAP_OPT_X_TLS_CRLFILE,(MCU_MCMS_DIR+"/CRL/ca.crl").c_str());
	status = check_set_option_status("TLS_CRLFILE",iLdapSetResult);

	if(!status)
		return status;

	int debug_level = 7;
	iLdapSetResult = ldap_set_option(NULL, LDAP_OPT_DEBUG_LEVEL, &debug_level);


	return status;
}

void CLdapModuleClientControl::buildLdapUrl(char* url,CLdapModuleCfg* pLdapModuleCfg,BOOL bUseAddressForConnection)
{
	char *ptr = url;
	const eLdapDirPort dirPort = pLdapModuleCfg->GetDirPort();
	//char url[80] = "";

	// Try to connect to the server
		// get parameters
	const char* dirNameAddress = '\0';
	if (bUseAddressForConnection)
		dirNameAddress = pLdapModuleCfg->GetDirNameAddressForConnection();
	else
		dirNameAddress = pLdapModuleCfg->GetDirNameAddress();
	std::string address(dirNameAddress);
	//std::remove(address.begin(), address.end(), ' ');
	address.erase(std::remove_if(address.begin(), address.end(),
													&::isspace), address.end());
	
	if(e389 ==dirPort )
	{
		strcpy(ptr, "ldap://");
		strcat(ptr, address.c_str());
		strcat(ptr, ":");
		char* strDirPort = LdapDirPortToString(dirPort);
		strcat(ptr, strDirPort);
	}
	else
	{
		strcpy(ptr, "ldaps://");
		strcat(ptr, address.c_str());
	}
}
void CLdapModuleClientControl::TestMultipleOUs(std::ostream& answer)
{
	answer << "Start TestMultipleOUs" << endl;
	CLdapModuleCfg* pLdapModuleCfg = ::GetLdapModuleCfg();
	if(pLdapModuleCfg)
	{
		int numberOu =0;
		char baseDn[512] = "";
		strncpy(baseDn, (char*)pLdapModuleCfg->GetBaseDn(), sizeof(baseDn)-1);
		baseDn[sizeof(baseDn)-1] = '\0';
		if(check_ou_exists(numberOu))
		{
			answer << "origibal base dn =" << baseDn;
			answer << "ous exists number="<<numberOu << endl;
			for(int i=0;i <numberOu;i++)
			{				
				string res = prepareBaseDNwithMultipleOus(i);
				answer << res << endl;
				
			}
		}
	}
	else
		answer << "ladp configuration is null" << endl;
		
	
}

BOOL CLdapModuleClientControl::check_ou_exists(int& count)
{
	CLdapModuleCfg* pLdapModuleCfg = ::GetLdapModuleCfg();
	BOOL stat = FALSE;
	if(pLdapModuleCfg)
	{
		string sBaseDn(pLdapModuleCfg->GetBaseDn());
		string token;
		size_t found,tokenPos=0;
		found=sBaseDn.find_first_of(",");
		while (found!=string::npos)
		{
			//make string low case
		   token =sBaseDn.substr(tokenPos,found-tokenPos);
		   tokenPos = found+1;
		   std::transform(token.begin(), token.end(), token.begin(), ::tolower);		   
		   //search for = 
		   size_t entityPos =  token.find('=');
		   string entity = token.substr(0,entityPos);
		   //trim string
		   std::remove(entity.begin(), entity.end(), ' ');		   
		   if(entity =="ou")
		   {
			   stat = TRUE;
			   count++;
		   }
		   
		   found=sBaseDn.find_first_of(",",found+1);
		}
	
	//last token
	token = sBaseDn.substr(tokenPos);
	// low case
	std::transform(token.begin(), token.end(), token.begin(), ::tolower);
	size_t entityPos =  token.find('=');
	string entity = token.substr(0,entityPos);
	// trim string
	std::remove(entity.begin(), entity.end(), ' ');
	 if(entity =="ou")
	 {
		 stat = TRUE;
		   count++;
	 }
	}
	return stat;

}

std::string CLdapModuleClientControl::prepareBaseDNwithMultipleOus(int OuIndex)
{	
	std::string newBaseDn ="";
	CLdapModuleCfg* pLdapModuleCfg = ::GetLdapModuleCfg();
	int count =0;
	if(pLdapModuleCfg)
	{
		string sBaseDn(pLdapModuleCfg->GetBaseDn());
		string token;
		size_t found,tokenPos=0;
		found=sBaseDn.find_first_of(",");
		while (found!=string::npos)
		{
			//make string low case
		    token =sBaseDn.substr(tokenPos,found-tokenPos);
		    tokenPos = found+1;		    
		    
		   //search for = 
		   size_t entityPos =  token.find('=');
		   string entity = token.substr(0,entityPos);
		   //trim string and to low case
		   std::transform(entity.begin(), entity.end(), entity.begin(), ::tolower);
		   std::remove(entity.begin(), entity.end(), ' ');		  
		   if(entity =="ou")
		   {					    
			   if(count==OuIndex)
			   {
				   if(newBaseDn =="")
					   newBaseDn  += token;
				   else
				   	   newBaseDn  += ","+token;
			   }
			   count++;
		   }
		   else
		   {
			   if(newBaseDn =="")
				   newBaseDn  += token;
			   else
				   newBaseDn  += ","+token;
		   }
		   found=sBaseDn.find_first_of(",",found+1);
		}
		newBaseDn  +="," + sBaseDn.substr(tokenPos);
	}		
	return newBaseDn;
}



