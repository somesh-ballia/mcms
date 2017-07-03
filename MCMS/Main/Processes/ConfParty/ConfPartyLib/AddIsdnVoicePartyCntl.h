#ifndef _ADDISDNVOICEPARTYCNTL
#define _ADDISDNVOICEPARTYCNTL

#include "IsdnPartyCntl.h"

////////////////////////////////////////////////////////////////////////////
//                        CAddIsdnVoicePartyCntl
////////////////////////////////////////////////////////////////////////////
class CAddIsdnVoicePartyCntl : public CIsdnPartyCntl
{
	CLASS_TYPE_1(CAddIsdnVoicePartyCntl, CIsdnPartyCntl)

public:
	CAddIsdnVoicePartyCntl();
	virtual ~CAddIsdnVoicePartyCntl();

	virtual const char* NameOf() const;
	virtual void*       GetMessageMap();

	CAddIsdnVoicePartyCntl& operator=(const CAddIsdnVoicePartyCntl& other);

	// Initializations
	void         Create(CConf* pConf, CIsdnNetSetup* pNetSetUp, /*CCapH221* pCap,CComMode* pScm,*/
	                    COsQueue* pConfRcvMbx, CAudioBridgeInterface* pAudioBridgeInterface, CConfAppMngrInterface* pConfAppMngrInterface, COsQueue* pPartyRcvMbx,
	                    CTaskApp* pParty, WORD termNum, const char* telNum, WORD type, const char* partyName,
	                    const char* confName, const char* password, DWORD monitorConfId,
	                    DWORD monitorPartyId, ENetworkType networkType, BYTE voice, BYTE audioVolume,
	                    const char* service_provider, WORD stby, WORD connectDelay,
	                    const char* AV_service_name, WORD welcome_msg_time,
	                    BYTE IsRecording = 0);


	void         Reconnect(const char* confName, const char* password, COsQueue* pConfRcvMbx, WORD termNum, WORD WelcomMode, BYTE isRecording = 0, WORD redialInterval = 0);
	void         DialOut(const char* confName, const char* password, COsQueue* pConfRcvMbx, WORD termNum, BYTE isRecording = 0, WORD redialInterval = 0);


protected:
	// action functions
	void         OnPartyConnectDelayIdle(CSegment* pParam);

	void         OnAudConnectConnectAudio(CSegment* pParam);
	void         OnPartyConnectSetup(CSegment* pParam);

	void         OnRsrcAllocatePartyRspAllocate(CSegment* pParam);
	void         OnMplAckCreate(CSegment* pParam);
	void         OnMplAckConnectAudio(CSegment* pParam);

	void         OnConnectToutAllocate(CSegment* pParam);
	void         OnConnectToutCreate(CSegment* pParam);
	void         OnConnectToutConnectAudio(CSegment* pParam);
	void         OnConnectToutSetup(CSegment* pParam);
	void         OnConnectToutConnect(CSegment* pParam);
	void         OnTimerWaitForRsrcAndAskAgainAllocate(CSegment* pParam);
	void         OnEndAvcToSvcArtTranslatorConnectCreate(CSegment* pParam);

	virtual void SetPartyTypeRelevantInfo(ALLOC_PARTY_REQ_PARAMS_S& allocatePartyParams);

	// update RsrcAlloc on chnl status
	void         OnPartyUpdateRTMChannel(CSegment* pParam);

protected:
	// Operations
	void         StartPartyConnection();
	void         SendConnectAudioNet();
	void         ConnectPartyToAudioBridge();
	void         SendPartyEstablishCall();
	void         AllocatePartyResources();

	// Attributes
	WORD         m_isNetAudioConnected;
	// 'MCMS' level audio mute is used both for:
	// 1. the ISDN/PSTN connection mute (to avoid noises during bonding/cap negotiation
	// 2. in case 'mute_all_but_me' activated by a chairperson.
	// this flag is to differentiate between the two cases.
	BYTE         m_isIncomingAudioMutedByAudioBridge;

	PDECLAR_MESSAGE_MAP
};

#endif // ifndef _ADDISDNVOICEPARTYCNTL
