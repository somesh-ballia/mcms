// SIPTransaction.h

#ifndef SIPTRANSACTION_
#define SIPTRANSACTION_

#include "StateMachine.h"
#include "IpScm.h"
#include "SipCall.h"

// SIP transaction states for state-machine:
const WORD   sTRANS_OPENINCHANNELS		 	= 110;
const WORD   sTRANS_CHANNELSCONNETED     	= 111;
const WORD   sTRANS_OPENBRIDGES				= 112;
const WORD   sTRANS_CONNECTING			    = 113;
const WORD   sTRANS_CHANGECHANNELS		    = 114;
const WORD   sTRANS_RECOVERY				= 115;
const WORD   sTRANS_INITREINVITEUPDATEIN	= 116;
const WORD   sTRANS_INITREINVITE			= 117;
const WORD   sTRANS_INITREINVITECLOSECHAN	= 118;
const WORD   sTRANS_OPENOUTCHANNELS			= 119;
const WORD   sTRANS_RMTCONNECTED			= 120;
const WORD   sTRANS_OPENCHANNELS			= 121;
const WORD   sTRANS_RECREINVITE				= 122;
const WORD   sTRANS_RECREINVITECLOSECHANN	= 123;
const WORD   sTRANS_RECREINVITEUPDATECHANN	= 124;
const WORD   sTRANS_RECREINVITEOPENCHANN	= 125;
const WORD   sTRANS_RECREINVITEUPDATEBRIDGE	= 126;
const WORD   sTRANS_RECREINVITEREJECTED		= 127;
const WORD   sTRANS_RESPONSEREINVITE		= 128;
const WORD   sTRANS_INITREINVITEUPDATECHANN = 129;
const WORD   sTRANS_END_WAS_SENT			= 130;
const WORD   sTRANS_INITREINVITEUPDATEBRIDGE= 131;
const WORD   sTRANS_INITEINVITEOPENCHANN	= 132;
const WORD	 sTRANS_REINVITEUPDATECHANNELS	= 133;
const WORD   sTRANS_WAITFORICECANDIDATES	= 134;
const WORD	 sTRANS_REINVITEOPENINCHANNELS	= 135;
const WORD 	 sTRANS_RECVIDEOPREFEUPDATEBRIDGE = 136;
const WORD 	 sTRANS_200OK_NO_MEDIA_SENT		= 137;
const WORD   sTRANS_RECREINVITEDISCONNECTBRIDGE	= 138;
const WORD   sTRANS_RECREINVITECONNECTBRIDGE	= 139;
//const WORD   sTRANS_ICE_WAIT_FOR_CHOSEN_CANDIDATES	= 140;
const WORD   sTRANS_SAVESTATISTICINFOBEFORECLOSECHANNEL  = 141;
const WORD   sTRANS_WAITFORUPDATECHANN	= 142;
const WORD   sTRANS_CHANNSUPDATEDWAITFORBRIDGESUPGRADE	= 143;


const WORD 	 sTRANS_REOPENINCHANNELS		= 144;  //added for ANAT
const WORD	 sTRANS_DTLS_STARTED			= 145;
const WORD	 sTRANS_DTLS_UPDATED_CHAN		= 146;
const WORD	 sTRANS_DTLS_CLOSED_CHAN_AFTER_DTLS_FAILURE		= 147;
const WORD	 sTRANS_DTLS_CLOSE_BEFORE_CLOSING_SIP_CHANNEL		= 148;
const WORD	 sTRANS_CLOSE_CHANNELS_BEFORE_PROCESS_ANSWER = 150;

const WORD   sTRANS_TRANSLATOR_ARTS_DISCONNECTING = 151;

const WORD 	 sTRANS_WAIT_FOR_REINVITE		= 152;
//const WORD 	 sTRANS_WAIT_FOR_FIXCONTENT		= 153;  //FSN-613: Dynamic Content for SVC/Mix Conf


// Timers:
const WORD OPENBRIDGESTOUT	 = 200;
const WORD UPDATEBRIDGESTOUT = 201;
const WORD ICEGENERALTOUT    = 202;
const WORD ICEOFFERTOUT      = 203;
const WORD ICEMODIFYTOUT    = 204;
const WORD ICECOMPLETETOUT  = 205;
const WORD ICEPORTSRETRYTOUT  = 206;
const WORD DISCONNECTBRIDGESTOUT = 207;
const WORD CONNECTBRIDGESTOUT = 208;
const WORD DTLSTOUT			= 210;
//const WORD FIXCONTENTTOUT			= 211; //FSN-613: Dynamic Content for SVC/Mix Conf



#define BRIDGES_TIME 15 // Time to open or update bridges

typedef enum
{
	// Dial in and out
	kNotInDialState = 0,	//not Connecting 0
	kBadStatusArrived,
	kConnectTimer,
	kDisconnectTimer,
	kBadStatusArrivedAfterOk,
	kChannelsFailedToOpen,
	kCapsDontMatch,
	kTerminateByConf,
	kTerminateByRemote,
	kReInviteArrived,
	kReInviteRejected,//10
	kReInviteAccepted,
	kTransportErrorArrived,
	kReInviteSent,
	kResponseToReInviteArrived,
	kByeArrived,
	kInternalProblem,

	// Dial out only
	kBeforeInvite,
	kInviteSent,
	kCancelInvite,
	kRejectArrived, //20  // Can be in Invite Req or ReInvite Req
	kDnsResolveTimer,

	// kOkArrived cases
	kGuessSucceeded,	// we only use 2 state now: kNoRecovery or kGuessSucceeded (kNoRecoveryForVideo is a private case of kGuessSucceeded)
						// if we can establish media connection (with or without recovery), we set the state to kGuessSucceeded
	kInternalRecovery,	//GuessFailed
	kExternalRecovery,	//GuessFailed
	kNoRecovery,		//GuessFailed (also for re-invite)
	kNoRecoveryForVideo,//GuessFailed
	// end of kOkArrived cases

	// Dial in only
	kRejectedByLobby,
	kFailedInLobby,
	kDisconnectTimerAfterRejectedByLobby,
	kTransferFailed,//30
	kFailedInConf,
	kCancelArrived,
	kBeforeOk,
	kBeforeOkInConf,
	kOkSent,
	kBadStatusAckArrived,
	kLastDialState,
    kICETimeOut,
    kWebRtcConnectFailure,
    kWebRtcConnectTimeOut

} ESipDialState;

class CPartyApi;
class CSipCntl;
class CSipParty;

class CSipTransaction : public CStateMachine
{
CLASS_TYPE_1(CSipTransaction, CStateMachine)
public:
	// Constructors:
	explicit CSipTransaction(CTaskApp* pOwnerTask);
	virtual ~CSipTransaction();

	// Initializations and settings:
	virtual const char* NameOf() const {return "CSipTransaction";}
	virtual BOOL  DispatchEvent(OPCODE event,CSegment* pParam = NULL);
	virtual void HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode) {}
	BOOL HandleSipPartyEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);
	virtual void InitTransaction(CSipParty* pParty, CSipCntl* pPartySipCntl, CIpComMode* pPartyCurrentMode, CIpComMode* pPartyTargetMode, ESipDialState* pPartyEDialState, char* partyConfName, WORD voice, char* alternativeAddrStr,CIpComMode* pTargetModeMaxAllocation, BYTE bTransactionSetContentOn = FALSE, BYTE isContentSuggested = FALSE, BYTE isFallbackFromTipToSipFlow = FALSE,  BYTE bIsGlare = FALSE); //BRIDGE-12961 bIsGlare
	void EndTransaction(DWORD retStatus = STATUS_OK);
	void EndTransactionByParty();
	void CleanTransaction();
	ESipDialState GetDialState() const;
	void SetDialState(ESipDialState eSipDialState);
	WORD GetIsVoice() const {return m_voice;}
	BYTE IsNeedReInviteForReAlloc() const {return m_bNeedReInviteForReAlloc;}
	BYTE IsNeedReInviteForSecondary() const {return m_bNeedReInviteForSecondary;}
	BYTE IsNeedReInviteForSecondaryContent() const {return m_bNeedReInviteForSecondaryContent;}
	BYTE IsNeedReInviteForIce() const {return m_bNeedReInviteForIce;}
	BOOL IsNeedToSendReInviteWithFullAudioCaps() const {return m_bNeedToSendReInviteWithFullAudioCaps;}
	BYTE IsNeedReInviteForAddContent();// const {return m_bNeedReInviteForAddContent;}
	BYTE IsNeedReInviteToFixContentAlg() const {return m_bNeedReInviteToFixContentAlg;}

	// TIP
	BYTE IsNeedReInviteForNoneTipCall() const {return m_bNeedReInviteForSwitchToNoneTipCall;}
	BYTE IsNeedToDropCallForLowBitRateForTip() const {return m_bIsNeedToDropCall;}

	BYTE IsContentRateWasChanged() const {return m_bContentRateWasChanged;}
	BYTE IsContentProtocolWasChanged() const {return m_bContentProtocolWasChanged;}
  	BYTE IsVideoFormatChangeOnly() const {return m_bChangeInVideoFormatOnly;}
	// Operations:
	void UpdateDbChannelsStatus(CSegment* pParam, BYTE bIsConnected /* TRUE or FALSE = update connected or disconnected channels*/);
	virtual void RequiredChannelsActionDone(DWORD opcode);
	void CheckChangingInCurrentMode(BYTE bCheckUpgradeReceive = FALSE);
	void UpdateChannelsIfNeeded();
	void OpenChannelsIfNeededForReInvite(BYTE isAnsweringToNewCap, cmCapDirection eDirection, BYTE bIsToOpenAudio=FALSE, BYTE bIsToOpenVideo=FALSE);
	void OpenChannelsIfNeededForReInviteOpenChannels(BYTE isAnsweringToNewCap, cmCapDirection eDirection, WORD isToOpenAudioChannel, WORD isToOpenVideoChannel);
	void CloseChannelsIfNeededForReceiveReInvite();
	virtual void InformChannelsConnectedOpenBridges(CSegment* pParam);
	void HandleChannelsDisconnectedStartRecovery(CSegment* pParam);
	void HandleChannelsUpdatedDuringRecovery(CSegment* pParam);
	virtual void InternalRecoveryCompleted() {};
	void HandleOutChannelsConnected(CSegment* pParam);
	void OnPartyChannelsConnectedAnycase(CSegment* pParam);

	//CDR_MCCF:
    void OnPartyRecStatisticInfo(CSegment* pParam);
    BYTE CheckTxChannelsVideoAndContentForCdrMCCF();

	void OnPartyChannelsUpdatedAnycase(CSegment* pParam);
	virtual void HandleBridgeConnectedInd(DWORD status){};
	void OnConfPartyReceiveAudBridgeConnected();
	void OnConfPartyReceiveVidBridgeConnected();
	void OnConfPartyReceiveFeccBridgeConnected();
	void SetCapsAccordingToNewAllocation(CSegment* pParam);
	void OnPartyOriginalRemoteCaps(CSegment* pParam);
	BYTE IsNoRecovery(CSipComMode* pBestMode);
	BYTE IsNoRecoveryForVideo(CSipComMode* pBestMode);
	BYTE IsNoRecoveryForData(CSipComMode* pBestMode);
	BYTE IsNoRecoveryForBfcp(CSipComMode* pBestMode);
	void CloseChannelsIfNeeded( BYTE bIsCloseChannels, //video
								BYTE bIsCloseDataChannels=FALSE,
								BYTE bIsCloseBfcpChannels=FALSE,
								BYTE bIsCloseAudioChannels=FALSE,
								BYTE bIsCloseContentChannels=FALSE,
								BYTE bIsUpdateAnatIpType=FALSE); //add param for ANAT
	void InformPartyRemoteConnect();
	void OpenOutChannels(BYTE isAnsweringToNewCap = FALSE);
	void OpenInAndOutChannelsIfNeeded(BYTE isAnsweringToNewCap = FALSE);
	virtual void PartyConnectCall() {}
	void OnPartySendSiteAndVisualNamePlusProductIdToPartyControl(CSegment* pParam);
	virtual void OnPartyConnectToutConnecting(CSegment* pParam);
	virtual void RollbackTransaction(); // Rollback transaction is case of reinvite transaction reject or error
	void GetProductIdAndSendToConfLevel();
	void SetIsNeedToSendReInviteforFullAudioCapsAccordingToUserAgentAndVersion();
 	virtual BYTE IsNeedToUpdateFlowControlInVideoBridge() {return FALSE;}; //Return true only if it is reinvite indication because then it is the remote who initiating the flow control
 	BYTE IsOfferer() const {return m_bIsOfferer;}
 	BYTE IsReInvite() const {return m_bIsReInvite;}
 	BYTE IsNeedToPendOtherTransaction() const {return m_bIsNeedToPendOtherTransaction;}

	//  Messages from Sip-Transaction to Sip-Party:
	void SendMessageToParty(OPCODE event,CSegment* pSeg);
	void SendMuteMediaToParty(cmCapDirection eDirection);// instead of void  SendMuteMediaToParty(BYTE audioInMute, BYTE audioOutMute, BYTE VideoInMute, BYTE VideoOutMute, BYTE ContentInMute, BYTE ContentOutMute, BYTE FeccInMute, BYTE FeccOutMute);
 	void SendEndTransactionToParty(DWORD retStatus);
 	void SendReCapsReceivedToParty(CSipCaps* pCaps,CIpComMode * pBestMode,WORD isFallBckFromTip = FALSE, WORD bIsGlare = FALSE); //bridge-12961 bIsGlare
 	void SendUpdateDbToParty(DWORD partyState);
 	void SendDisconnectBridgesToParty(WORD isDisconnectAudio, WORD isConnectVideo);
 	void SendConnectBridgesToParty(WORD isConnectAudio, WORD isConnectVideo, unsigned int* channelHandle);
 	void SendUpdateDbChannelsStatusToParty(BYTE bIsConnected);
 	void SendChannelsConnectedToParty();
 	void SendRemoteCapsReceivedToParty();
 	void SendRemoteConnectedToParty();
 	void SendOriginalRemoteCapsToParty(CSipCaps * pRemoteCaps);
 	void SendSiteAndVisualNamePlusProductIdToParty(CSegment* pParam);
 	void SendUpdateDbEncryptionStatusToParty(WORD bIsEncrypted);
 	void SendVideoChangeResolutionToParty(DWORD Width,DWORD Height,DWORD FrameRate,DWORD BitRate);
	void SendStartVideoPreferenceToParty();
	void SendChannelHandleToParty();
 	void CheckChangesInVideo();
 	BOOL IsNeedToSendRtcpVideoPreference();
 	BYTE IsNeedReinviteForRemoveRtv();
 	BYTE RemoveRtvCapsIfNeeded(CIpComMode* pScmMode);
 	DWORD CalcualteNewRateAccordingToBestModeForCOP( DWORD vidRateBefore ,CIpComMode*  pBestMode);
 	void  SendVideoChangeAfterVsrMsgToParty();

	//ICE
 	WORD SendIceMgsReqAccordingToTargetMode(DWORD Opcode);
 	WORD SendIceMgsReqAccordingToTargetModeAndDiffArr(DWORD Opcode,BYTE bChangeInMedia = TRUE);
 	WORD SendIceMgsReqAccordingToTargetModeAndCurrentMode(DWORD Opcode);
 	void SendHandleIceConnectivityCheckCompleteToParty(CSegment* pParam);
 	BYTE IsNeedToReInviteToFallbackFromIceToSip() const;
 	// ppc
 	void CheckContentChangesInConfResponse(BYTE bIsContentSpeaker, CapEnum oldContentProtocol = eUnknownAlgorithemCapCode);
 	void RemoveBfcpAccordingToRemoteIdent();
	void	  RemoveBFCPIfTransportUnsupported();
	void RemoveBfcpIfNecessaryForANAT(); //added for ANAT

 	// TIP
 	//BYTE CheckIfRemoteSdpIsTipCompatible();
 	BYTE CheckIfRemoteVideoRateIsTipCompatible();
 	BYTE GetIsMuteForTip();
 	void SetIsMuteForTip(BYTE bIsMute = FALSE);
 	void SetIsNeedToWaitForSlavesEndChangeMode(BYTE bIsEndChangeMode = FALSE);
 	void SetIsTipResumeMedia(BYTE);
 	BYTE GetIsTipResumeMedia() const;
	BYTE IsNeedReInviteForBandwidth() const {return m_bNeedReInviteForBandwidth;}
	void SetNeedReInviteForBandwidth(BYTE b) { m_bNeedReInviteForBandwidth = b;}
 	void UpdateIfCallIsResumed();
 	void FallbackFromTipToNoneTip();
 	virtual BOOL IsNeedToAddContentForFallbackFromTip() {return TRUE;}

 	// BFCP
 	void SetNeedReInviteForBfcp(BYTE bIsReinviteBfcp) { m_bNeedReInviteForBfcp = bIsReinviteBfcp;}
 	BYTE IsNeedReInviteForBfcp() const {return m_bNeedReInviteForBfcp;}
 	void BfcpDecisionCenter(CSipComMode* pBestMode);
 	BYTE IsNeedToToCloseBfcpAndContentWithoutReinvite() {return m_bNeedToCloseBfcpAndContentWithoutReinvite;}

 	void CheckIfNeedToFixContentAlgAndSendReInvite(const CSipCaps *pLocalCaps);//, CSipCaps *pRemoteCaps);
 	void StartFECIfNeeded();
 	void StartREDIfNeeded();
 	// DTLS
 	BYTE CloseDtlsChannelsBeforeSipChannelsIfNeeded();
 	BYTE StartDtlsIfNeeded();
 	void OnPartyDtlsEndInd(CSegment* pParam);
 	void UpdateDtlsChannels();
 	void CloseDtlsChannels();
 	void OnDtlsTout(CSegment* pParam);
 	void DisconnectOnDtlsEncryptFail();
 	void TipEarlyPacketDtlsNotNeeded();

 	// N.A. SDES
 	BYTE SetMediaSdesChangesIfNeeded(CSipComMode* pBestMode, BYTE bChangeInMediaIn = NO);


 	// HOLD-RESUME
 	BOOL IsCallResumed();
 	BOOL IsCallHold();

 	void RestoreStreamGroups(CIpComMode* pComMode);
 	void RestoreRecvStreamGroup(CIpComMode* pComMode);
	void RestoreSendStreamGroup(CIpComMode* pComMode);

	// can be remnoved
 	virtual void RestoreStreamGroups(CIpComMode* pSrcTarget, CIpComMode* pDestTarget, WORD isToOpenAudioChannel, WORD isToOpenVideoChannel){ return; }
 	virtual void RestoreRecvStreamGroup(CIpComMode* pSrcTarget, CIpComMode* pDestTarget, WORD isToOpenAudioChannel, WORD isToOpenVideoChannel){ return; }
	virtual void RestoreSendStreamGroup(CIpComMode* pSrcTarget, CIpComMode* pDestTarget, WORD isToOpenAudioChannel, WORD isToOpenVideoChannel){ return; }

	BYTE IsContentSuggested() {return m_isContentSuggested;}; // Added by Efi

	void AddToChannelsMask(DWORD& channelsMask, EIpChannelType eChanType);

	BYTE GetRtpStatisticsIfNeededForReinvite();

    void OnPartyVideoArtDisconnectedDoNothing();
    void OnRemoveAvcToSvcArtTranslatorAnycase(CSegment* pParam);
    void OnPartyTranslatorArtsDisconnected(CSegment* pParam);

    BOOL GetIsRemoteIdentMicrosoft();
	BYTE RemoveEncryptionFromScmAndCapsWhenTipResumedIfNeeded(CSipComMode* pBestMode);

	BOOL RejectDTLSEncIfNeeded(BYTE	bIsDisconnectOnEncFailure);

	void SetNeedReInviteToActiveMedia(BYTE bIsReinvitActiveMedia) { m_bNeedReInviteToActiveMedia = bIsReinvitActiveMedia;}
	BYTE IsNeedReInviteToActiveMedia() const {return m_bNeedReInviteToActiveMedia;}
	void CheckEncryptionAndDisconnectIfNeeded(BOOL bIsUndefinedParty, BYTE bIsDisconnectOnEncFailure, BYTE bIsWhenAvailableEncMode, DWORD isEncrypted );

	BOOL GetIsUseMkiForLocalCaps(CSipComMode* pBestMode, cmCapDataType mediaType , ERoleLabel eRole);	//BRIDGE-10820
	void UpdateLocalCapsWithEncryptionParameters(CSipComMode* pBestMode, cmCapDataType mediaType , ERoleLabel eRole);	//BRIDGE-10820
	void SendLastTargetModeToParty();

	void OnSipNotifyPendingTransactionDtlsStarted(CSegment* pParam); //BRIDGE-15745

protected:
	virtual void UserAgentAndVersionUpdated(const char* cUserAgent, const char* pVersionId){}
	virtual bool ShouldKeepTargetTxStreams() {return false;}

	CPartyApi*		m_pPartyApi;
	BYTE					m_isVideoBridgeConnected;
	BYTE					m_isAudioBridgeConnected;
	BYTE					m_isFeccBridgeConnected;
	CSipChanDifArr* 		m_pChanDifArr;
	WORD					m_voice;
	BYTE					m_bNeedReInviteForReAlloc;   // New caps from conf were received during transaction (as a result of ReAlloc)
	BYTE					m_bNeedReInviteForSecondary; // Need to send ReInvite for removing the video.
	BYTE					m_bNeedReInviteForSecondaryContent; // Need to send ReInvite for removing the content.
	BYTE					m_bNeedReInviteForIce;
	BYTE					m_bNeedReInviteForAddContent;// Need to add content (first offer is without content).
	BYTE					m_bNeedReInviteToFixContentAlg;

	BYTE					m_bContentRateWasChanged; // True if content rate was changed by conf as part of transaction flow
	BYTE					m_bContentProtocolWasChanged; // True if content protocol was changed by conf as part of transaction flow

	BYTE 					m_bChangeInVideoFormatOnly;
	BYTE					m_bIsOfferer; 		// Is Offerer transaction
	BYTE					m_bIsReInvite; 		// Is ReInvite transaction (all types of reinvite: req/ind sdp/no-sdp)
	BYTE					m_bIsNeedToPendOtherTransaction; // Is need to pend other transaction during current transaction

	// Pointers from SipParty (including Party or IpParty). Don't allocate/delete them in Transaction:
	char*					m_pPartyConfName;
	CIpComMode* 			m_pCurrentMode;
	CIpComMode* 			m_pTargetMode;
	CSipCntl*				m_pSipCntl;
	CSipParty*				m_pParty;
	ESipDialState*			m_pEDialState;
	char*					m_pAlternativeAddrStr;
	BOOL 					m_bNeedCloseIceChannels;
//	BOOL					m_bNeedUpdateIceChannels; // For SVC (Relay) channels we use Update instead of close and reopen that is used for AVC.
	BOOL					m_bNeedCloseSrtpChannels;
	BOOL					m_bNeedUpdateSrtpChannels; // For SVC (Relay) channels we use Update instead of close and reopen that is used for AVC.
	BOOL 					m_bIsChangeInICEChannels;
	CIpComMode*             m_pTargetModeMaxAllocation;
	BOOL					m_bNeedToSendReInviteWithFullAudioCaps;
	BYTE					m_bNeedReinviteForRemoveRtv;

	BYTE					m_bNeedToCloseBfcpAndContentWithoutReinvite;

	// TIP
	BYTE					m_IceMakeOfferAnswerCounter;
	BYTE					m_bNeedReInviteForSwitchToNoneTipCall;
	BYTE					m_bIsNeedToDropCall;
	DWORD                    m_Transation_Reason;
	BYTE					m_bIsTipMute;
	BYTE					m_needToWaitForSlavesEndChangeMode;
	BYTE 					m_bIsResumeMedia;

	BYTE 					m_isNeedToEndTransAfterTipSlavesAck;

	BYTE					m_isFallbackFromTipToSipFlow;
	// CAC
	BYTE					m_bNeedReInviteForBandwidth;
	BYTE					m_bIsNeedToFallbackFromIceToSip;

	// BFCP
	BYTE					m_bNeedReInviteForBfcp;

	BYTE 					m_bTransactionSetContentOn;
	//CDR_MCCF:
	WORD                    m_oldstate;

	// This flag prevents suggesting content to EP in an endless loop. Added by Efi
	BYTE 					m_isContentSuggested;
        bool                                    m_bNeedToCloseInternalVideoArt;
    unsigned int m_incomingVideoChannelHandle;
    unsigned int m_outgoingVideoChannelHandle;

    WORD m_state_deescalation;

    //Lync 2013
    BYTE 					m_bNeedReInviteToActiveMedia;
    BYTE					m_bTipEarlyPacket;

    BYTE 					m_bIsGlare; //BRIDGE-12961

	PDECLAR_MESSAGE_MAP;
};

#endif  // SIPTRANSACTION_
