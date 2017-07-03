#ifndef ADDISDNPARTYCNTL_H_
#define ADDISDNPARTYCNTL_H_

#include "IsdnPartyCntl.h"

//template class std::pair<WORD, CIsdnNetSetup>;
//template class std::map<WORD, CIsdnNetSetup>;

typedef std::map<WORD, CIsdnNetSetup> ADDITIONAL_CHANNELS;

////////////////////////////////////////////////////////////////////////////
//                        CAddIsdnPartyCntl
////////////////////////////////////////////////////////////////////////////
class CAddIsdnPartyCntl : public CIsdnPartyCntl
{
	CLASS_TYPE_1(CAddIsdnPartyCntl, CIsdnPartyCntl)

public:
	                    CAddIsdnPartyCntl();
	virtual            ~CAddIsdnPartyCntl();
	CAddIsdnPartyCntl&  operator=(const CAddIsdnPartyCntl& other);

	virtual const char* NameOf() const;
	virtual void*       GetMessageMap();

	// Initializations
	void                Create(CConf* pConf, CIsdnNetSetup* pNetSetUp, CCapH320* pCap, CComMode* pScm,
	                           CComMode* pTransmitScm, COsQueue* pConfRcvMbx, CAudioBridgeInterface* pAudioBridgeInterface,
	                           CVideoBridgeInterface* pVideoBridgeInterface, CConfAppMngrInterface* pConfAppMngrInterface,
	                           CFECCBridge* pFECCBridge, CContentBridge* pContentBridge, CTerminalNumberingManager* pTerminalNumberingManager,
	                           COsQueue* pPartyRcvMbx, CTaskApp* pParty, WORD termNum, BYTE chnlWidth, WORD numChnl, WORD type,
	                           const char* partyName, const char* confName, DWORD monitorConfId, DWORD monitorPartyId,
	                           const char* service_provider, ENetworkType networkType, WORD nodeType, BYTE voice, WORD stby,
	                           DWORD connectDelay, eTelePresencePartyType eTelePresenceMode, eSubCPtype bySubCPtype = eSubCPtypeClassic, WORD isUndefParty = NO);

	void                Reconnect(const char* confName, CCapH320* pCap, CComMode* pScm,
	                           CComMode* pTransmitScm, COsQueue* pConfRcvMbx,
	                           WORD termNum, BYTE isRecording, WORD redialInterval, WORD connectDelay);

	// to add additional channel to specific party (in bonding dial in lobby->conf->partycntl)
	void                AddPartyChannel(CIsdnNetSetup& netSetUp, WORD channelNum);

protected:
	// action functions
	void                OnPartyConnectDelayIdle(CSegment* pParam);

	void                OnRsrcAllocatePartyRspAllocate(CSegment* pParam);
	void                OnMplAckCreate(CSegment* pParam);
	void                OnMplAckConnectAudio(CSegment* pParam);
	void                OnAudConnectConnectAudio(CSegment* pParam);
	void                OnPartyEndInitComSetup(CSegment* pParam);
	void                OnPartyConnectChangeAudio(CSegment* pParam);
	void                OnPartyConnectAnycase(CSegment* pParam);
	void                OnPartyReceivedFullCapSet(CSegment* pParam);

	void                OnConnectToutAllocate(CSegment* pParam);
	void                OnConnectToutCreate(CSegment* pParam);
	void                OnConnectToutConnectAudio(CSegment* pParam);
	void                OnConnectToutSetup(CSegment* pParam);
	void                OnConnectToutChangeAudio(CSegment* pParam);
	void                OnConnectToutAudioConnected(CSegment* pParam);
	void                OnTimerWaitForRsrcAndAskAgainAllocate(CSegment* pParam);

	virtual void        SetPartyTypeRelevantInfo(ALLOC_PARTY_REQ_PARAMS_S& allocatePartyParams);

	// update RsrcAlloc on chnl status
	void                OnPartyUpdateRTMChannel(CSegment* pParam);
	void                OnRsrcAllocatorUpdateRTMChannelAck(CSegment* pParam);
	void                AddNetConnectionIdToRsrcRoutingTable(DWORD net_connection_id);

	// ask from RsrcAlloc to Re-Allocate party resources
	void                OnPartyReAllocateRTM(CSegment* pParam);
	void                OnRsrcAllocatorUpdateReAllocateRTMAck(CSegment* pParam);

	void                OnPartyBoardFull(CSegment* pParam);
	void                OnRsrcReallocateBoardFull(CSegment* pParam);

	void                HandleReallocateResponse(BYTE bAllocationFailed, BYTE bLocalCapsChanged);

private:
	// Operations
	void                DialOut(WORD redialInterval = 0); // WORD redialInterval??
	void                StartPartyConnection();
	void                SendConnectAudioNet();
	void                ConnectPartyToAudioBridge();
	void                SendPartyEstablishCall();
	void                AllocatePartyResources();
	void                UnMuteAndUpdateDB();

	// Attributes
	WORD                m_isNetAudioConnected;
	BYTE                m_isUnMuted;

	// 'MCMS' level audio mute is used both for:
	// 1. the ISDN/PSTN connection mute (to avoid noises during bonding/cap negotiation
	// 2. in case 'mute_all_but_me' activated by a chairperson.
	// this flag is to differentiate between the two cases.
	BYTE                m_isIncomingAudioMutedByAudioBridge;

	ADDITIONAL_CHANNELS additionalChannels;  // for storing the dial in net_setup

	PDECLAR_MESSAGE_MAP
};

#endif /*ADDISDNPARTYCNTL_H_*/
