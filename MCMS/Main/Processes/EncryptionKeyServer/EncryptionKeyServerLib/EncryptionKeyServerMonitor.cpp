// EncryptionKeyServerMonitor.cpp: implementation of the EncryptionKeyServerMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "EncryptionKeyServerMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CEncryptionKeyServerMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CEncryptionKeyServerMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CEncryptionKeyServerMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CEncryptionKeyServerMonitor)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CEncryptionKeyServerMonitor::HandleOperLogin)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void EncryptionKeyServerMonitorEntryPoint(void* appParam)
{  
	CEncryptionKeyServerMonitor *monitorTask = new CEncryptionKeyServerMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEncryptionKeyServerMonitor::CEncryptionKeyServerMonitor()
{

}

CEncryptionKeyServerMonitor::~CEncryptionKeyServerMonitor()
{

}

