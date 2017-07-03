// LicenseServerManager.h

#ifndef LICENSE_SERVER_MANAGER_H_
#define LICENSE_SERVER_MANAGER_H_

#include "ManagerTask.h"
#include "Macros.h"
#include "HttpSAP.h"
#include "LicenseInfoSender.h"
#include "LicenseServerProcess.h"

void LicenseServerManagerEntryPoint(void* appParam);

class LicensingManager;

class CLicenseServerManager : public CManagerTask, CNonCopyable
{
  CLASS_TYPE_1(CLicenseServerManager, CManagerTask)

 public:
                 CLicenseServerManager();
  virtual       ~CLicenseServerManager();
  void           ManagerPostInitActionsPoint();
  void           OnRefreshLicenseInfo(CSegment*);
  void           SyncLicenseInfo(CRequest*);
  void           ProcessServerConfiguration(CSegment* msg);


  TaskEntryPoint GetMonitorEntryPoint();
  void           RefreshLicenseInfo(void);

 private:
  PDECLAR_MESSAGE_MAP
  PDECLAR_TRANSACTION_FACTORY
  PDECLAR_TERMINAL_COMMANDS

 private:
  bool              CheckServerType(void);
  bool              isRefreshing;
  STATUS HandleTerminalRefreshLicenseInfo(CTerminalCommand & command, std::ostream& answer);

  LicensingManager* lMgr;

  CLicenseServerProcess* m_pProcess ;
};

#endif

