// EncryptionKeyServerMonitor.h: interface for the CEncryptionKeyServerMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CEncryptionKeyServerMONITOR__)
#define _CEncryptionKeyServerMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CEncryptionKeyServerMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CEncryptionKeyServerMonitor();
	virtual ~CEncryptionKeyServerMonitor();

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CEncryptionKeyServerMONITOR__)
