#include "AuthenticationProcess.h"


static COperatorList* GetOperatorList()
{
	CAuthenticationProcess* pProcess = (CAuthenticationProcess*) CAuthenticationProcess::GetProcess();
	return pProcess->GetOperatorList();
};

static COperatorList* GetLdapOperatorList()
{
	CAuthenticationProcess* pProcess = (CAuthenticationProcess*) CAuthenticationProcess::GetProcess();
	return pProcess->GetLdapOperatorList();
};

