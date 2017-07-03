// BackupRestoreProcess.h: interface for the CBackupRestoreProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_DEMOPROCESS_H__)
#define _DEMOPROCESS_H__

#include "ProcessBase.h"

class CBackupRestoreProcess : public CProcessBase  
{
CLASS_TYPE_1(CBackupRestoreProcess,CProcessBase )
public:
	friend class CTestBackupRestoreProcess;

	CBackupRestoreProcess();
	virtual ~CBackupRestoreProcess();
	const char * NameOf() const {return "CBackupRestoreProcess";}
	virtual eProcessType GetProcessType() {return eProcessBackupRestore;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();
};

#endif // !defined(_DEMOPROCESS_H__)
