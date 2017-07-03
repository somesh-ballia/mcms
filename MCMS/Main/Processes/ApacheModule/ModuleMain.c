
#define off64_t  __off64_t

#include "httpd.h"
#include "http_config.h"
#include "ap_mpm.h"
#include "http_protocol.h"
#include "http_request.h"
#include "http_connection.h"

#define INTVOID void


extern int HandleResponsePhase(request_rec* pRequestRec);
extern int HandleLogTransaction(request_rec* pRequestRec);
extern INTVOID InitModule(apr_pool_t *pConfigPool, server_rec *pServerRec);
extern int InitConnection(conn_rec *pConnRec, void *pCsd);
extern const char* ReadDocumentRoot(cmd_parms *cmd, void *config, const char *pszPhysical);
extern const char* ReadAlias(cmd_parms *cmd, void *config, const char *pszVirtual, const char *pszPhysical);
extern int BasicAuthentication(request_rec* pRequestRec);
extern const char* SetDirectoryAccess(cmd_parms *cmd, void *config, const char *pszAccess);
extern const char* SetCreateDirectory(cmd_parms *cmd, void *config, const char *pszCreateDir);
extern const char* SetAuditabilityOfDirectory(cmd_parms *cmd, void *config, const char *pszAuditability);
extern const char* SetInformProcessOfDirectory(cmd_parms *cmd, void *config, const char *pszProcessName);
extern apr_status_t ap_requestCheck(request_rec *pRequestRec);

extern int HandleFatalException(ap_exception_info_t *ei);

static int check_user_access(request_rec *r)
{
	return OK;
}
//extern int HandleFatalException(void *);

//module AP_MODULE_DECLARE_DATA carmel_module;


static const command_rec command_handlers[] =
{
	AP_INIT_TAKE1("DocumentRoot", ReadDocumentRoot, NULL, OR_ALL, NULL),
	AP_INIT_TAKE2("Alias", ReadAlias, NULL, OR_ALL, NULL),
	AP_INIT_TAKE1("Access", SetDirectoryAccess, NULL, ACCESS_CONF, NULL),
	AP_INIT_TAKE1("Create_directory", SetCreateDirectory, NULL, ACCESS_CONF, NULL),
	AP_INIT_TAKE1("Is_Auditable", SetAuditabilityOfDirectory, NULL, OR_ALL, NULL),
	AP_INIT_TAKE1("Inform_Process", SetInformProcessOfDirectory, NULL, OR_ALL, NULL),
	{NULL,{NULL},NULL,0,0,NULL}
	//{NULL,NULL,NULL,OR_ALL,0,NULL}
};


/*
 * This function is a callback and it declares what other functions
 * should be called for request processing and configuration requests.
 * This callback function declares the Handlers for other events.
 */
static void mod_carmel_register_hooks (apr_pool_t *p)
{
	static const char* const aszSuccessor[] = { "mod_proxy.c", NULL};
	ap_hook_child_init(InitModule, NULL, NULL, APR_HOOK_MIDDLE);
	ap_hook_pre_connection(InitConnection, NULL, NULL, APR_HOOK_MIDDLE);
	ap_hook_handler(HandleResponsePhase, NULL, aszSuccessor, APR_HOOK_FIRST);
	ap_hook_check_user_id(BasicAuthentication,NULL,NULL,APR_HOOK_FIRST);
	ap_hook_auth_checker(check_user_access,NULL,NULL,APR_HOOK_FIRST);
	ap_hook_fatal_exception(HandleFatalException,NULL,NULL, APR_HOOK_MIDDLE);
    ap_hook_log_transaction(HandleLogTransaction,NULL,NULL, APR_HOOK_FIRST);
    ap_hook_post_read_request(ap_requestCheck, NULL, NULL, APR_HOOK_FIRST);
 }


/*
 * Declare and populate the module's data structure.  The
 * name of this structure ('carmel_module') is important - it
 * must match the name of the module.  This structure is the
 * only "glue" between the httpd core and the module.
 */
module AP_MODULE_DECLARE_DATA carmel_module =
{
	STANDARD20_MODULE_STUFF,
	NULL,
	NULL,
	NULL,
	NULL,
	command_handlers,
	mod_carmel_register_hooks,			/* callback for registering hooks */
};

