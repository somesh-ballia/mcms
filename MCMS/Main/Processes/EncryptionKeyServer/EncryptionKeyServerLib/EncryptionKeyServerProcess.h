// EncryptionKeyServerProcess.h

#ifndef ENCRYPTION_KEY_SERVER_H_
#define ENCRYPTION_KEY_SERVER_H_

#include "ProcessBase.h"

class CEncryptionKeyServerProcess : public CProcessBase  
{
CLASS_TYPE_1(CEncryptionKeyServerProcess, CProcessBase)
public:
	friend class CTestEncryptionKeyServerProcess;

	CEncryptionKeyServerProcess();
	virtual ~CEncryptionKeyServerProcess();
	virtual eProcessType GetProcessType() {return eProcessEncryptionKeyServer;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();
  virtual BOOL HasMonitorTask() {return FALSE;}
  virtual void AddExtraOpcodesStrings();
  virtual DWORD GetMaxTimeForIdle(void) const
  {
    return 12000;
  }
};

#endif  // ENCRYPTION_KEY_SERVER_H_
