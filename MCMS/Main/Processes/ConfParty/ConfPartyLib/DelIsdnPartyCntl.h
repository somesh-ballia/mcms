#ifndef DELISDNPARTYCNTL_H_
#define DELISDNPARTYCNTL_H_

#include "IsdnPartyCntl.h"

////////////////////////////////////////////////////////////////////////////
//                        CDelIsdnPartyCntl
////////////////////////////////////////////////////////////////////////////
class CDelIsdnPartyCntl : public CIsdnPartyCntl
{
	CLASS_TYPE_1(CDelIsdnPartyCntl, CPartyCntl)

public:
	                    CDelIsdnPartyCntl();
	virtual            ~CDelIsdnPartyCntl();

	CDelIsdnPartyCntl&  operator=(const CDelIsdnPartyCntl& other);

	virtual const char* NameOf() const  { return "CDelIsdnPartyCntl"; }
	virtual void*       GetMessageMap() { return (void*)m_msgEntries; }

	// Operations
	void                Disconnect(WORD mode = 0);
	void                DisconnectISDN(WORD mode = 0, DWORD disconnectionDelay = 0);
	void                UpdateConfEndDisconnect(WORD status);

	BOOL                GetIsViolentDestroy()               { return m_isViolentDestroy; }
	void                SetIsViolentDestroy(BOOL isViolent) { m_isViolentDestroy = isViolent; }
	DWORD               GetPartyTaskId()                    { return m_partyTaskId; }
	void                SetPartyTaskId(DWORD taskId)        { m_partyTaskId = taskId; }

protected:
	// action functions
	void                OnAudBrdgDisconnect(CSegment* pParam);
	void                OnVidBrdgDisconnect(CSegment* pParam);
	int                 OnContentBrdgDisconnected(CSegment* pParam);
	void                OnXCodeBrdgDisconnected(CSegment* pParam);
	void                OnFeccBridgeDisConnect(CSegment* pParam);

	void                OnPartyPcmStateChangedIdle(CSegment* pParam);

	void                OnPartyDisconnectDestroyParty(CSegment* pParam);
	void                OnMplAckDeleteFromHw(CSegment* pParam);
	void                OnRsrcDeallocatePartyRspDeallocate(CSegment* pParam);

	void                OnTimerBridgesDisconnect(CSegment* pParam);
	void                OnTimerPcmDisconnect(CSegment* pParam);
	void                OnTimerPartyDisconnectDestroyParty(CSegment* pParam);
	void                OnTimerMplDisconnectDeleteFromHw(CSegment* pParam);
	void                OnTimerRsrcAllocatorDisconnect(CSegment* pParam);

	void                OnPartyDelayDisconnectIdle(CSegment* pParam);

	// Operations
	void                BridgeDisconnetCompleted();
	void                DestroyParty();
	void                DeletePartyFromHW();
	void                DeallocatePartyResources();

	void                EndPartyDisconnect();
	void                PcmDisconnectionCompleted();

	BOOL                m_isViolentDestroy;
	DWORD               m_partyTaskId;

	PDECLAR_MESSAGE_MAP
};

#endif
