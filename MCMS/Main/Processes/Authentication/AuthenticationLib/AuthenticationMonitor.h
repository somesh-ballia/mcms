// AuthenticationMonitor.h: interface for the CAuthenticationMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CAuthenticationMONITOR__)
#define _CAuthenticationMONITOR__

#include "MonitorTask.h"

class CAuthenticationMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CAuthenticationMonitor();
	virtual ~CAuthenticationMonitor();
	STATUS HandleGetOperList(CRequest *pRequest);
	STATUS GetOperatorAudibleALarm(CRequest *pRequest);
protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CAuthenticationMONITOR__)

