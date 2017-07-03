// BackupRestoreManager.h

#ifndef BACKUP_RESTORE_MANAGER_H_
#define BACKUP_RESTORE_MANAGER_H_

#include "ManagerTask.h"
#include "Macros.h"
#include "DefinesGeneral.h"
#include "Segment.h"
#include "BackupRestoreProcess.h"
#include "CommonStructs.h"

#define TIME_FORMAT "%d-%m-%Y_%H-%M-%S"

void BackupRestoreManagerEntryPoint(void* appParam);

class CBackupRestoreManager : public CManagerTask
{
  CLASS_TYPE_1(CBackupRestoreManager, CManagerTask)
public:
  static STATUS  SendProgressTypeToMcuMngr(OPCODE action, BYTE type);
  static STATUS  SendMsgToInstaller(OPCODE action, BYTE type = eBRIdle);

  CBackupRestoreManager();
  virtual ~CBackupRestoreManager(){ }

  const char*    NameOf() const {return "CBackupRestoreManager";}
  TaskEntryPoint GetMonitorEntryPoint();

  STATUS         HandleBackUpStart(CRequest* pRequest);
  STATUS         OnBackUpStart();
  STATUS         HandleBackUpFinish(CRequest* pRequest);
  STATUS         HandleRestoreStart(CRequest* pRequest);
  STATUS         HandleRestoreFinish(CRequest* pRequest);
  STATUS         FinishAction(OPCODE actionOP,
                              const char* dump,
                              BYTE stateType,
                              char* dirToRemove = NULL);

  void           OnBackupTimeout();
  void           OnRestoreTimeout();
  short          GetProcessAction()               { return m_processAction; }
  void           SetProcessAction(short inAction) { m_processAction = inAction; }

  // Ask McuMngr for system version
  void           AskMcuMngrForSysVersion();
  void           OnGetMcuVersionInd(CSegment* pSeg);
  void           CreateBackupName(char* name, int len);
  bool           CreateBackupVersionFile();
  void           GetTime(char* timeStr, int timeLen, char* format = TIME_FORMAT);
  STATUS         EncryptBackupFile(const char* fileName);
  STATUS         DecryptBackupFile(const char* fileName,bool isSHA256Dec=true);
  STATUS 		 DecryptBackupFileSha1(const char* fileName);
  BYTE           IsJitcMode();

protected:
  void           OnInstallStartInd();
  void           OnInstallFinishInd();
  STATUS         OnBackupRestoreProgressTimeout();
  void           OnBackupIdleTimer();

  bool m_installInProgress;

private:
  STATUS         HandleTerminalDecrypt(CTerminalCommand& cmd, std::ostream& ans);
  STATUS         HandleTerminalEncryptionDecryption(CTerminalCommand& command,
                                                    std::ostream& answer);
  virtual void   ManagerStartupActionsPoint();
  void           SendAuditEvent(const std::string strEvent,
                                const std::string strDescription) const;
  STATUS         CopyFile(const char* fileNameSource,
                          const char* fileNameDestination);

  VERSION_S m_mcuVer;

  short  m_processAction;
  string m_strBackupFileName;

  PDECLAR_MESSAGE_MAP;
  PDECLAR_TRANSACTION_FACTORY;
  PDECLAR_TERMINAL_COMMANDS;
};

#endif  // ifndef BACKUP_RESTORE_MANAGER_H_
