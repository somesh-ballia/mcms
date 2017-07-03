#ifndef _ISDN_VIDEO_PARTY_OUT_H
#define _ISDN_VIDEO_PARTY_OUT_H

#include  "IsdnVideoParty.h"

class CIsdnPartyRsrcDesc;

// additional channel dialing methods
#define DIALING_METHOD_BY_TIMER   1
#define DIALING_METHOD_SEQUENTIAL 2

// ===== Public Functions Declarations =====
extern "C" void PartyVideoOutEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//                        CIsdnVideoPartyOut
////////////////////////////////////////////////////////////////////////////
class CIsdnVideoPartyOut : public CIsdnVideoParty
{
	CLASS_TYPE_1(CIsdnVideoPartyOut, CIsdnVideoParty)

public:
	             CIsdnVideoPartyOut();
	virtual     ~CIsdnVideoPartyOut();
	virtual void Create(CSegment& appParam);

	const char*  NameOf() const { return "CIsdnVideoPartyOut"; }

protected:
	void         OnConfEstablishCallIdle(CSegment* pParam);
	void         OnNetConnectSetUp(CSegment* pParam);
	void         OnUpdateChannelAck(CSegment* pParam);
	void         EstablishOutCall(WORD chnl);
	void         DialNextChannel();
	void         DestroyNetConnection(WORD chnl);
	void         OnConfReallocateRtmAck(CSegment* pParam);

	void         OnBondEndnegotiationSetup(CSegment* pParam);
	void         OnBondAlignedSetup(CSegment* pParam);

	void         OnBoardFullAck(CSegment* pParam);

	void         OnTimerDialAdditionalChannel();

	void         OnBondDisConnect(CSegment* pParam);
	void         OnTestTimer(CSegment* pParam);

	// channels dialing data members
	WORD         m_numOfChannels;
	WORD         m_numChannelsConnected;
	WORD         m_lastChannelDial;

private:
	void         DumpIsdnPartyRsrcDesc(CIsdnPartyRsrcDesc& pPartyRsrcDesc);

	// net channels dialing
	void         DialChannel();
	void         DialNextAdditionalChannel();
	void         StopDial();
	DWORD        TimerToWaitBeforeDialAdditinalChannl(WORD channel_number);

	void         SetBondingAdditionalPhoneNumbers();

	// net channels dialing - get system config
	WORD         GetDialingMethod();
	DWORD        GetChannelDelay();
	DWORD        GetGroupDelay();
	DWORD        GetNumChannelsInGroup();

	PDECLAR_MESSAGE_MAP
};

#endif /* _ISDN_VIDEO_PARTY_OUT_H */
