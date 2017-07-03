#ifndef __IPPARTY__
#define __IPPARTY__
#include "Party.h"

#define TIME_FOR_IP_FRACTION_LOST 15 * SECOND
const WORD IP_FRACTION_LOST_TIMER = 101;

class CIpParty: public CParty
{
CLASS_TYPE_1(CIpParty, CParty)
public:

	CIpParty(CIpComMode* pCurrentMode, CIpComMode* pTargetMode);
	virtual ~CIpParty();

	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	virtual void NullActionFunction(CSegment* pParam);
	void InitTask();
	virtual const char* NameOf() const { return "CIpParty";}

	virtual void CleanUp() = 0; // only when local is initiator of closing (before fully connected)

	void Create(CSegment& appParam);

	CIpComMode* GetCurrentMode() const { return m_pCurrentMode; }
	CIpComMode* GetTargetMode()  const { return m_pTargetMode; }

	void LogicalChannelConnect(CPrtMontrBaseParams* pPrtMonitor, DWORD channelType,DWORD vendorType = 0);
	void LogicalChannelUpdate(DWORD channelType, DWORD vendorType = 0);

	void SetPartyMonitorBaseParamsAndConnectChannel(
								DWORD channelType,
								DWORD rate = 0xFFFFFFFF,
								mcTransportAddress* partyAdd = NULL,
								mcTransportAddress* mcuAdd = NULL,
								DWORD protocol = (DWORD) eUnknownAlgorithemCapCode,
								DWORD pmIndex = 0,
								DWORD vendorType = 0,
								BYTE IsIce = 0,
								mcTransportAddress* IcePartyAdd = NULL,
								mcTransportAddress* IceMcuAdd = NULL,
								EIceConnectionType IceConnectionType = kNone);

	virtual void LogicalChannelDisconnect(DWORD eChannelType);

	virtual void OnIpLogicalChannelConnect(CSegment* pParam);
	virtual void OnIpLogicalChannelDisconnect(CSegment* pParam);
	virtual void OnIpPartyMonitoring(CSegment* pParam);
	virtual void OnMcuMngrPartyMonitoringReq(CSegment* pParam) = 0;
	virtual void OnPartyRemoteH230(CSegment* pParam) = 0;
	virtual void OnPartyStreamViolation(CSegment* pParam);

	void  OnVidBrdgValidation(CSegment* pParam);
	void  OnAudBrdgValidation(CSegment* pParam);
	virtual void SerializeNetSetup(DWORD channelType,CSegment* pSeg) = 0;
	virtual void SetPartyToSecondary(WORD reason,CSecondaryParams *pSecParamps = NULL) = 0;

protected:

	PDECLAR_MESSAGE_MAP;

	CIpComMode* m_pCurrentMode;
	CIpComMode* m_pTargetMode;

	BYTE		m_bIsPreviewVideoIn;
	BYTE		m_bIsPreviewVideoOut;

	CPartyPreviewDrv* m_RcvPreviewReqParams;
	CPartyPreviewDrv* m_TxPreviewReqParams;

};

#endif
