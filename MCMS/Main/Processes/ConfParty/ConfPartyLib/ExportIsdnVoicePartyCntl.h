#ifndef _EXPORT_ISDN_VOICE_PARTYCNTL_
#define _EXPORT_ISDN_VOICE_PARTYCNTL_

#include "IsdnPartyCntl.h"
#include <map>

// Self timers for Export process
#define EXPORT_PARTY_TOUT    ((WORD)300)
#define EXPORT_RSRC_TOUT     ((WORD)301)
#define EXPORT_MPL_TOUT      ((WORD)302)
#define END_EXPORT_RSRC_TOUT ((WORD)303)
#define EXPORT_BRIDGES_TOUT  ((WORD)304)

////////////////////////////////////////////////////////////////////////////
//                        CExportIsdnVoicePartyCntl
////////////////////////////////////////////////////////////////////////////
class CExportIsdnVoicePartyCntl : public CIsdnPartyCntl
{
	CLASS_TYPE_1(CExportIsdnVoicePartyCntl, CIsdnPartyCntl)

public:
	typedef std::map< eLogicalResourceTypes, bool > AcksTable;

	                    CExportIsdnVoicePartyCntl();
	virtual            ~CExportIsdnVoicePartyCntl();

	// Operations
	virtual const char* NameOf() const;
	virtual void*       GetMessageMap();

	void                Transfer(COsQueue* pDestRcvMbx, void* pComConf, DWORD destConfId, DWORD destPartyId, EMoveType eCurMoveType = eMoveDefault);
	void                OnEndResourceAllocatorStartMove(CSegment* pParam);
	void                OnResourceAllocatorEndMove(CSegment* pParam);
	void                OnAudioBridgeExported(CSegment* pParam);
	void                OnMplApiMoveAck(CSegment* pParam);
	void                OnPartyExport(CSegment* pParam);
	void                BridgeExportCompleted();
	void                MfaAcksCompleted();
	WORD                CheckIfLogicalRsrcAckAccepted(eLogicalResourceTypes lrt);
	virtual BYTE        IsRemoteAndLocalCapSetHasContent(eToPrint toPrint = eToPrintNo) const { return NO;}

	// Timer events
	void                OnTimerRAStartMove(CSegment* pParam);
	void                OnTimerMPLExport(CSegment* pParam);
	void                OnTimerRAEndMove(CSegment* pParam);
	void                OnTimerExportBridges(CSegment* pParam);
	void                OnTimerExport(CSegment* pParam);

protected:
	STATUS              TestReturnedParams(DWORD status, DWORD targetMonitorConfId, DWORD targetResourceConfId,
	                                       DWORD monitorPartyId, DWORD rsrcPartyId, DWORD expectedMonitorPartyId);

	WORD                m_numOfActiveLogicalRsrc;
	WORD                m_activeLogicalRsrc[NUM_OF_LOGICAL_RESOURCE_TYPES];
	AcksTable           m_ackTable;

	PDECLAR_MESSAGE_MAP
};

#endif // ifndef _EXPORT_ISDN_VOICE_PARTYCNTL_

