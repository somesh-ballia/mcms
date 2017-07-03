// AuditorProcess.h

#ifndef AUDITOR_PROCESS_H_
#define AUDITOR_PROCESS_H_

#include "Macros.h"
#include "ProcessBase.h"

class CAuditFileList;
class CAuditEventContainer;

class CAuditorProcess : public CProcessBase, CNonCopyable
{
  CLASS_TYPE_1(CAuditorProcess, CProcessBase)
  friend class CTestAuditorProcess;

 public:
                         CAuditorProcess();
  virtual               ~CAuditorProcess();
  const char*            NameOf() const;
  virtual eProcessType   GetProcessType();
  virtual BOOL           UsingSockets();
  virtual TaskEntryPoint GetManagerEntryPoint();
  virtual DWORD          GetMaxTimeForIdle() const;
  virtual int            GetProcessAddressSpace();

  CAuditFileList*        GetAuditFileList();
  CAuditEventContainer*  GetAuditEventContainer();

 private:
  bool                  m_IsDownload;
  CAuditFileList*       m_pAuditFileList;  // Contains ready for EMA files
  CAuditEventContainer* m_pAuditEventContainer;
};

#endif

