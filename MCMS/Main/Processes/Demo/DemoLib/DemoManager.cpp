// DemoManager.cpp

#include "DemoManager.h"

#include "Trace.h"
//#include "FipsMode.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCommon.h"

PBEGIN_MESSAGE_MAP(CDemoManager)
  ONEVENT(XML_REQUEST, IDLE, CDemoManager::HandlePostRequest)
PEND_MESSAGE_MAP(CDemoManager, CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY(CDemoManager)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CDemoManager)
END_TERMINAL_COMMANDS

extern void DemoMonitorEntryPoint(void* appParam);

void DemoManagerEntryPoint(void* appParam)
{
  CDemoManager* mngr = new CDemoManager;
  mngr->Create(*(CSegment*)appParam);
}

TaskEntryPoint CDemoManager::GetMonitorEntryPoint()
{
  return DemoMonitorEntryPoint;
}

CDemoManager::CDemoManager()
{}

CDemoManager::~CDemoManager()
{}

void CDemoManager::ManagerPostInitActionsPoint()
{
  // put task init code here
  // this function is called just before WaitForEvent
//  TestAndEnterFipsMode();
}
