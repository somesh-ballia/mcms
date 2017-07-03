#ifndef _PARTYISDN
#define _PARTYISDN

#include  "Party.h"
#include "IsdnNetSetup.h"
#include "NetChnlCntl.h"

typedef struct
{
	CRsrcParams* pMfaRsrcParams;
	CRsrcParams* pMrmpRsrcParams;
	DWORD ssrc;
	PartyRsrcID partyRsrcId;
	ConfRsrcID confRsrcId;
	WORD roomId;

} CREATE_AVC_TO_SVC_ART_TRANSLATOR_S;

class CHardwareInterface;
class CRsrcParams;
class CAvcToSvcArtTranslator;

////////////////////////////////////////////////////////////////////////////
//                        CIsdnParty
////////////////////////////////////////////////////////////////////////////
class CIsdnParty : public CParty
{
	CLASS_TYPE_1(CIsdnParty, CParty)

public:
	                    CIsdnParty();
	virtual            ~CIsdnParty();

	virtual const char* NameOf() const { return "CIsdnParty";}

	void                SetPartyResource(CRsrcParams* NetRsrcParams);
	void                OnEndNetDisconnect(CSegment* pParam); // rons
	void                OnStartAvcToSvcArtTranslator(CSegment* pParam);
	void                OnRemoveAvcToSvcArtTranslatorAnycase(CSegment* pParam);
	void                AvcToSvcArtTranslatorConnected(STATUS status);
	void                AvcToSvcArtTranslatorDisconnected(STATUS statusOnCloseMrmpChannel, STATUS statusOnCloseArt);

protected:
	void                DisconnectNetChnlCntl(WORD chnl);

	WORD                NetDisconnectCompleted();
	virtual void        UpdateNetChnlStatus(WORD chnl, WORD status);
	void                MonitorPartyPhoneNumbers(BYTE channel, CNetSetup* pNetSetUp, BYTE isDailOut);
	void                UpdatePartyPhoneNumbers(BYTE channel, const char* calledNumber, const char* callingNumber);
	void                UpdateQ931DisCause(const BYTE cause);
	void                OnNetDisconnectSetUp(CSegment* pParam);
	void                OnNetDisconnectConnect(CSegment* pParam);
	void                OnNetDisconnect(CSegment* pParam);
	void                OnPartySendFaultyMfaToPartyCntlAnycase(CSegment* pParam);
	void                OnConfEstablishCall(CSegment* pParam);
	void                CleanUp(WORD mode = 0);
	void                RemoveNetChannel(WORD channel_index);

	void                OnConfDelLogicalNetCntl(CSegment* pParam);
	void                ReplayToNetWhenNoResource(DWORD opcode, CIsdnNetSetup& netSetup, WORD discasue = causDEFAULT_VAL);



	virtual void        PartySpecfiedIvrDelay();

	CIsdnNetSetup*      m_pNetSetUp;
	CRsrcParams*        m_pNetRsrcParams;
	CNetChnlCntl*       m_pNetChnlCntl[MAX_CHNLS_IN_PARTY];
	BYTE                m_byQ931Cause;
	WORD                m_retryNetCall[MAX_CHNLS_IN_PARTY];
	BYTE                m_conf_has_been_notifyed;
	CAvcToSvcArtTranslator *m_pAvcToSvcArtTranslator;

	PDECLAR_MESSAGE_MAP
};

#endif /* _PARTYV */

