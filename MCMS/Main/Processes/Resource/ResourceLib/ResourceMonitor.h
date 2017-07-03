// ResourceMonitor.h: interface for the CResourceMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CResourceMONITOR__)
#define _CResourceMONITOR__

#include "MonitorTask.h"
#include "Macros.h"
#include "MplMcmsProtocol.h"
#include "CRsrcDetailGet.h"
#include "Request.h"
#include "InitCommonStrings.h"
#include "SystemResources.h"

#include "CMediaRecordingGet.h"

#include "DummyEntry.h"
#include "OpcodesMcmsCardMngrRecording.h"
#include "RecordingRequestStructs.h"
#include "SNMPDefines.h"

// Timers opcodes
#define GET_RESOURCE_INFO_TIMER                ((WORD)200)
#define GET_RESOURCE_INFO_TIMER_TIME_OUT_VALUE 2*SECOND

class CResourceMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CResourceMonitor();
	virtual ~CResourceMonitor();

	virtual const char* NameOf() const { return "CResourceMonitor";}
    STATUS OnServerCardDetail(CRequest* pGetRequest);
	STATUS OnServerCardUnitDetail(CRequest* pGetRequest);

	STATUS OnServerCardDetailNinja(CRequest* pGetRequest);
	STATUS OnServerCardUnitDetailNinja(CRequest* pGetRequest);
	
	//RR
	STATUS OnServerResourceReport(CRequest* pGetRequest);
	STATUS OnServerConfResourceReport(CRequest* pGetRequest);
	
	//Recording
	STATUS OnGetRecordingJunctionsList(CRequest* pGetRequest);
	
	// Ports Configuration
	STATUS OnGetConfigurationList(CRequest* pGetRequest);
	STATUS OnGetEnhancedConfiguration(CRequest* pGetRequest);
	STATUS OnCheckEnhancedPortConfiguration(CRequest* pSetRequest);
	
	//Allocation mode (for pure mode)
	STATUS OnGetAllocationMode(CRequest* pSetRequest);
	
	//Reservation list
	STATUS OnGetReservationList(CRequest* pGetRequest);
	STATUS OnServerGetResReq(CRequest* pGetRequest);

	STATUS OnGetPartyPortsInfo(CRequest* pRequest);

	STATUS OnServerServicesResourceReport(CRequest* pGetRequest);	
	virtual void InitTask();
	void OnTimerGetResourceInformationForSNMP();
	void SendMessageToSNMP(eTelemetryType eType, DWORD value);

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	
	CJunctionsList* m_pJunctionsList;
};

#endif // !defined(_CResourceMONITOR__)

