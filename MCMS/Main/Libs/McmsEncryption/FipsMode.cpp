// FipsMode.cpp

#include "FipsMode.h"

#include <time.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/fips.h>
#include <openssl/opensslv.h>

#include "SslFunc.h"
#include "ProcessBase.h"
#include "Trace.h"
#include "TraceStream.h"
#include "TaskApi.h"
#include "ConfigHelper.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"


bool GetSNMPFipsMode()
{
	CProcessBase* proc = CProcessBase::GetProcess();
	FPASSERT_AND_RETURN_VALUE(NULL == proc, false);

	CSysConfig* cfg = proc->GetSysConfig();
	FPASSERT_AND_RETURN_VALUE(NULL == cfg, false);

	BOOL val;
	BOOL res = cfg->GetBOOLDataByKey(CFG_SNMP_FIPS_MODE, val);


	FPASSERTSTREAM_AND_RETURN_VALUE(!res, "Unable to read " << CFG_SNMP_FIPS_MODE,  false);


	return val;
}

int TestAndEnterFipsMode(bool alwaysFIPS)
{
  if (!alwaysFIPS && !GetSNMPFipsMode())
  {
	  const char* pname = CProcessBase::GetProcessName(CProcessBase::GetProcess()->GetProcessType());

	  FTRACESTR(eLevelInfoNormal) << pname << " is not running in FIPS mode ";
	  return STATUS_OK;
  }

  CProcessBase* proc = CProcessBase::GetProcess();
  FPASSERT_AND_RETURN_VALUE(NULL == proc, STATUS_FAIL);

  const char* pname = CProcessBase::GetProcessName(proc->GetProcessType());
  FPASSERT_AND_RETURN_VALUE(NULL == pname, STATUS_FAIL);

  // Omits FIPS mode under Valgrind.
  FTRACECOND_AND_RETURN_VALUE(IsUnderValgrind(pname),
    pname << " runs under valgrind, FIPS mode is skipped",
    STATUS_OK);

  // Already in FIPS mode.
  FTRACECOND_AND_RETURN_VALUE(FIPS_mode() != 0,
    pname << " runs in FIPS mode already with " << OPENSSL_VERSION_TEXT
    << " and " << FIPS_module_version_text(),
    STATUS_OK);

  CTaskApp::Unlocker unlocker;
  const clock_t begin = clock();

  int res = FIPS_mode_set(1);

  // Sets FIPS mode.
  FTRACECOND_AND_RETURN_VALUE(0 != res,
    pname << " runs in FIPS mode with " << OPENSSL_VERSION_TEXT
    << " and " << FIPS_module_version_text()
    << ", operation duration: " << float(clock () - begin) / CLOCKS_PER_SEC << " s",
    STATUS_OK);

  FPASSERTSTREAM(true,
    "FIPS set failed for " << pname << " with " << OPENSSL_VERSION_TEXT
    << " and " << FIPS_module_version_text()
    << ": " << CSslFunctions::SSLErrMsg());

  exit(1);

  return STATUS_FAIL;
}
