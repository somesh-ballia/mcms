// ApacheModuleLib.cpp
//
//////////////////////////////////////////////////////////////////////


#include "ApacheModuleEngine.h"
#include "ap_mpm.h"
#include "http_config.h"


#ifdef _WINNT_
#define INTVOID int
#else
#define INTVOID void
#endif


#ifndef _WINNT_

#define CDECLARE

extern "C" {

#else

#define CDECLARE "C"

#endif

int CDECLARE HandleResponsePhase(request_rec* pRequestRec)
{
	return CApacheModuleEngine::HandleResponsePhase(pRequestRec);
}

int CDECLARE HandleLogTransaction(request_rec* pRequestRec)
{
	return CApacheModuleEngine::HandleLogTransaction(pRequestRec);
}


apr_status_t CDECLARE CleanConnection(void *pData)
{
	return CApacheModuleEngine::CleanConnection((apr_pool_t*)pData);
}

int CDECLARE HandleFatalException(ap_exception_info_t *ei)
{
	return CApacheModuleEngine::HandleFatalException(ei->sig);

}

int CDECLARE InitConnection(conn_rec *pConnRec, void *pCsd)
{
	CApacheModuleEngine::InitConnection(pConnRec);
	apr_pool_cleanup_register(pConnRec->pool,(void*)pConnRec->pool,CleanConnection,NULL);
    return DECLINED;
}

apr_status_t CDECLARE CleanModule(void *pConfigPool)
{
	CApacheModuleEngine::Clean();

    return APR_SUCCESS;
}

INTVOID CDECLARE InitModule(apr_pool_t *pConfigPool, server_rec *pServerRec)
{
	CApacheModuleEngine::Initialize(pConfigPool,pServerRec);
	apr_pool_cleanup_register(pConfigPool,(void*)pConfigPool,CleanModule,NULL);

	#ifdef _WINNT_
	return DECLINED;
	#endif

}

const char* CDECLARE ReadDocumentRoot(cmd_parms *cmd, void *config, const char *pszPhysical)
{
	char ch[2];
	strcpy(ch, "/");
	CApacheModuleEngine::SetPhysicalPath(ch, pszPhysical);

	return DECLINE_CMD;
}

const char* CDECLARE ReadAlias(cmd_parms *cmd, void *config, const char *pszVirtual, const char *pszPhysical)
{
	CApacheModuleEngine::SetPhysicalPath((char*)pszVirtual, pszPhysical);

	return DECLINE_CMD;
}

int CDECLARE BasicAuthentication(request_rec* pRequestRec)
{
	return CApacheModuleEngine::BasicAuthentication(pRequestRec);
}

const char* CDECLARE SetDirectoryAccess(cmd_parms *cmd, void *config, const char *pszAccess)
{
	CApacheModuleEngine::SetDirectoryAccess(cmd->path, (char*)pszAccess);
	return NULL;
}

const char* CDECLARE SetCreateDirectory(cmd_parms *cmd, void *config, const char *pszCreateDir)
{
	CApacheModuleEngine::SetCreateDirectory(cmd->path, (char*)pszCreateDir);
	return NULL;
}

const char* CDECLARE SetAuditabilityOfDirectory(cmd_parms *cmd, void *config, const char *pszAuditability)
{
	CApacheModuleEngine::SetAuditabilityOfDirectory(cmd->path, (char*)pszAuditability);
	return NULL;
}

const char* CDECLARE SetInformProcessOfDirectory(cmd_parms *cmd, void *config, const char *pszProcessName)
{
	CApacheModuleEngine::SetInformProcessOfDirectory(cmd->path, (char*)pszProcessName);
	return NULL;
}

#define MAXRANGEHEADERS 5

apr_status_t  CDECLARE ap_requestCheck(request_rec* pRequestRec)
{
	//return HTTP_RANGE_NOT_SATISFIABLE;
	//return DECLINED;
	  const char *hdrs[2] = {"Range", "Request-Range"};
	    int i, count;
	    int maxChar = 1024;
	    for (i = 0; i < 2; i++) {
		const char *pRangeHeader = apr_table_get(pRequestRec->headers_in, hdrs[i]);

		if (!pRangeHeader)
		    continue;
		int charIndex =0;
		for (count = 1; *pRangeHeader; pRangeHeader++,charIndex++) {

			if(charIndex > maxChar)
				return HTTP_RANGE_NOT_SATISFIABLE;

		    if (*pRangeHeader == ',')
		    	count++;
			}
		if(count > MAXRANGEHEADERS)
			return HTTP_RANGE_NOT_SATISFIABLE;

	    }

	    return DECLINED;
}

#ifndef _WINNT_

}

#endif



void UnitTestAssert(const char * text)
{
	#ifdef _UNIT_TESTS
	CPPUNIT_ASSERT_MESSAGE( text, false );

	#endif //_UNIT_TESTS
}

bool IsUnitTest()
{
	return false;
}



