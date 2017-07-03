// ConfPartyMonitor.h: interface for the ConfPartyMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CONFPARTYMONITOR__)
#define _CONFPARTYMONITOR__

#include "MonitorTask.h"
#include "Macros.h"
#include "CommConfDBGet.h"
#include "CommConfSpecific.h"
#include "ConfPartySpecific.h"
#include "Request.h"
#include "IVRServiceListGet.h"
#include "FileListGet.h"
#include "SNMPDefines.h"

// Timers opcodes
#define GET_CONF_INFO_TIMER                ((WORD)200)
#define GET_CONF_INFO_TIMER_TIME_OUT_VALUE 2*SECOND
#define GET_CONF_INFO_TIMER_DISABLE_STATE_TIME_OUT_VALUE 30*SECOND

class CConfPartyMonitor : public CMonitorTask
{
CLASS_TYPE_1(CConfPartyMonitor,CMonitorTask )
public:
	CConfPartyMonitor();
	virtual ~CConfPartyMonitor();

	virtual const char* NameOf() const { return "CConfPartyMonitor";}
	STATUS OnServerConfList(CRequest* pGetRequest);
	STATUS OnServerConfListFull(CRequest* pGetRequest);
	STATUS OnServerConfSpecific(CRequest* pGetRequest);
	STATUS OnServerPartySpecific(CRequest* pGetRequest);
	STATUS OnServerProfileList(CRequest* pGetRequest);
	STATUS OnServerGetProfileReq(CRequest* pGetRequest);
	STATUS OnServerIVRServiceList(CRequest* pGetRequest);
	STATUS OnServerFileList(CRequest* pGetRequest);
	STATUS OnServerReqMRList(CRequest* pGetRequest)    ;
	STATUS OnServerGetMeetingRoomReq(CRequest* pGetRequest);
	STATUS OnServerRecordingLinkList(CRequest* pGetRequest);
	STATUS OnServerConfTemplateList(CRequest* pGetRequest);
	STATUS OnServerGetConfTemplateReq(CRequest* pGetRequest);
    STATUS OnServerGetResolutionThreshold(CRequest* pSetRequest);
	STATUS OnServerGetCustomizeDisplayForConf(CRequest* pSetRequest);
	STATUS OnServerGetContentRatingTableForCascade(CRequest* pGetRequest);

    STATUS OnServerGetConversionStatus(CRequest* pGetRequest);
    
	//	STATUS OnServerGetConfRelayMediaInfo(CRequest* pGetRequest);
	STATUS OnServerGetFixedContentRatingTableForCascade(CRequest* pGetRequest);

	virtual void InitTask();
	void OnTimerGetConfInformationForSNMP();
	
protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CONFPARTYMONITOR__)

