#ifndef _EXPORT_ISDN_PARTYCNTL_
#define _EXPORT_ISDN_PARTYCNTL_

#include "IsdnPartyCntl.h"

// Self timers for Export process
#define EXPORT_PARTY_TOUT    ((WORD)300)
#define EXPORT_RSRC_TOUT     ((WORD)301)
#define EXPORT_MPL_TOUT      ((WORD)302)
#define END_EXPORT_RSRC_TOUT ((WORD)303)
#define EXPORT_BRIDGES_TOUT  ((WORD)304)
#define EXPORT_FAILED_TOUT   ((WORD)305)

////////////////////////////////////////////////////////////////////////////
//                        CExportIsdnPartyCntl
////////////////////////////////////////////////////////////////////////////
class CExportIsdnPartyCntl : public CIsdnPartyCntl
{
	CLASS_TYPE_1(CExportIsdnPartyCntl, CIsdnPartyCntl)

public:
	CExportIsdnPartyCntl();
	virtual ~CExportIsdnPartyCntl();

	CExportIsdnPartyCntl& operator=(const CExportIsdnPartyCntl& other);

	// Operations
	virtual const char* NameOf() const;
	virtual void*       GetMessageMap();

	// Initializations
	BYTE UpdateVidBrdgStateBeforeExport();
	void Transfer(COsQueue* pDestRcvMbx, void* pComConf, DWORD destConfId, DWORD destPartyId, EMoveType eCurMoveType = eMoveDefault);
	void Transfer();
	void ChangeContent();

	// state machine events
	void OnEndResourceAllocatorStartMove(CSegment* pParam);
	void OnMplApiMoveAck(CSegment* pParam);
	void OnResourceAllocatorEndMove(CSegment* pParam);
	void OnAudioBridgeExported(CSegment* pParam);
	void OnVideoBridgeExported(CSegment* pParam);
	// TOOREN - is needed
	void OnAudBrdgDisconnect(CSegment* pParam);
	void OnVidBrdgDisconnect(CSegment* pParam);
	void OnContentBridgeDisConnect(CSegment* pParam);
	void OnVidBrdgDisconnectUpdateState(CSegment* pParam);
	void OnPartyPcmStateChangedExportRsrc(CSegment* pParam);

	void OnPartyExport(CSegment* pParam);
	void OnRmtCapsAnycase(CSegment* pParam);

	void  OnXCodeBrdgDisconnect(CSegment* pParam);
	// Timer events
	void OnTimerPcmDisconnect(CSegment* pParam);
	void OnTimerRAStartMove(CSegment* pParam);
	void OnTimerMPLExport(CSegment* pParam);
	void OnTimerRAEndMove(CSegment* pParam);
	void OnTimerExportBridges(CSegment* pParam);
	void OnTimerExport(CSegment* pParam);
	void OnTimerExportFailed(CSegment* pParam);

	void BridgeExportCompleted();
	void MfaAcksCompleted();
	WORD CheckIfLogicalRsrcAckAccepted(eLogicalResourceTypes lrt);

protected:
	STATUS TestReturnedParams(DWORD status, DWORD targetMonitorConfId, DWORD targetResourceConfId,
	                          DWORD monitorPartyId, DWORD rsrcPartyId, DWORD expectedMonitorPartyId);

	WORD m_numOfActiveLogicalRsrc;
	WORD m_activeLogicalRsrc[NUM_OF_LOGICAL_RESOURCE_TYPES];
	WORD m_ackTable[NUM_OF_LOGICAL_RESOURCE_TYPES];


	PDECLAR_MESSAGE_MAP
};

#endif
