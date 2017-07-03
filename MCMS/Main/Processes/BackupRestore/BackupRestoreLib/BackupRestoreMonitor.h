// BackupRestoreMonitor.h: interface for the CBackupRestoreMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CBackupRestoreMONITOR__)
#define _CBackupRestoreMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CBackupRestoreMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CBackupRestoreMonitor();
	virtual ~CBackupRestoreMonitor();

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CBackupRestoreMONITOR__)
