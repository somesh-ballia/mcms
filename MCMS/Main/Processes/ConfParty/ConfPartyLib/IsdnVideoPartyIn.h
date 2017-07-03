#ifndef _ISDN_VIDEO_PARTY_IN_H
#define _ISDN_VIDEO_PARTY_IN_H

#include "IsdnVideoParty.h"
#include "LobbyApi.h"

extern "C" void PartyIsdnVideoInEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//                        CIsdnVideoPartyIn
////////////////////////////////////////////////////////////////////////////
class CIsdnVideoPartyIn : public CIsdnVideoParty
{
	CLASS_TYPE_1(CIsdnVideoPartyIn, CIsdnVideoParty)

public:
	             CIsdnVideoPartyIn();
	virtual     ~CIsdnVideoPartyIn();
	virtual void Create(CSegment& appParam);

	const char*  NameOf() const { return "CIsdnVideoPartyIn"; }

	void         OnLobbyTransferSetup(CSegment* pParam);

	// connect
	void         OnLobbyNetIdentIdle(CSegment* pParam);
	void         OnConfEstablishCallSetup(CSegment* pParam);
	void         OnNetConnectSetUp(CSegment* pParam);
	void         OnConfAddNetDescSetup(CSegment* pParam);
	void         OnBondConnectSetUp(CSegment* pParam);
	void         OnMuxEndH221ConSetUp(CSegment* pParam);
	void         OnMuxSyncInitCahnl(CSegment* pParam);
	void         OnConfReallocateRtmAck(CSegment* pParam);

	void         OnBondReqChnlSetUp(CSegment* pParam);
	void         OnBondEndnegotiationSetup(CSegment* pParam);
	void         OnTestTimer(CSegment* pParam);

	// disconnect
	void         OnBondDisConnect(CSegment* pParam);
	void         OnBondDistroy(CSegment* pParam);
	void         OnNetDisconnectSetUp(CSegment* pParam);
	void         OnTimerEndCallProceedingSetUp(CSegment* pParam);

	// Move
	void         OnConfExportChangeMode(CSegment* pParam);

protected:
	CLobbyApi*   m_pLobbyApi;
	WORD         m_numOfChannels;        // rons
	WORD         m_numChannelsConnected; // rons
	WORD         m_DownSpeedStatus;      // rons

private:

	PDECLAR_MESSAGE_MAP
};

#endif /* _ISDN_VIDEO_PARTY_IN_H */
