// CardsMonitor.h: interface for the CCardsMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CCardsMONITOR__)
#define _CCardsMONITOR__


#include "MonitorTask.h"



class CCardsMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CCardsMonitor();
	virtual ~CCardsMonitor();

	virtual const char* NameOf() const { return "CCardsMonitor";}
	STATUS OnServerCardsList(CRequest* pGetRequest);
	STATUS OnServerCardStatuses(CRequest* pGetRequest);

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CCardsMONITOR__)

