// CertMngrMonitor.h: interface for the CCertMngrMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CCertMngrMONITOR__)
#define _CCertMngrMONITOR__

#include "MonitorTask.h"
#include "Macros.h"
#include "CertMngrDefines.h"

class CCertMngrMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor, CMonitorTask)
public:
	STATUS HandleGetList(CRequest* req);
	STATUS HandleGetCTL(CRequest* req);
	STATUS HandleGetPersonal(CRequest* req);
	STATUS HandleGetCRL(CRequest* req);

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

	STATUS HandleListRequest(CRequest* req, eCertificateType type);
};

#endif // !defined(_CCertMngrMONITOR__)
