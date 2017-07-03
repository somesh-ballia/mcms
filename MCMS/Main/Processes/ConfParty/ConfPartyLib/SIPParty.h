//+========================================================================+
//                            SIPParty.h		                           |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPParty.h												   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 15/11/05   | This file contains								   |
//     |            |                                                      |
//+========================================================================+

#ifndef __SIPPARTY__
#define __SIPPARTY__

#include "IPParty.h"
#include "IPUtils.h"
#include "SipCall.h"
#include "ConfPartyGlobals.h"
#include "SIPTransaction.h"
#include "H323Util.h"
#include "OpcodesMcmsCardMngrICE.h"
#include "TokenMsg.h"
#include "TokenMsgMngr.h"
#include "TipStructs.h"
#include "PartyCntl.h"
#include "SipConfPartyDefinitions.h"


class CSipCntl;

// SIP Party states for state-machine:
const WORD   sPARTY_WAITFORCONFINITCALL		= 10;
const WORD   sPARTY_CONNECTING				= 11;
const WORD   sPARTY_ALLOCATE_TRANSLATOR_ARTS	= 12;

const WORD MAKE_ICE_CANDIDATES_TIMER	= 30*SECOND;
const WORD ICE_PORTS_RETRY_TIMER	= 5*SECOND;


typedef enum
{
	// No active transaction:
	kSipTransNone = 0,
	// Local initiation transactions:
	kSipTransInviteNoSdpReq,
	kSipTransInviteWithSdpReq,
	kSipTransInviteMrcSlaveWithSdpReq,
	kSipTransReInviteNoSdpReq,
	kSipTransReInviteWithSdpReq,
	kSipTransUpgradeSvcOnlyToMixReq,
	kSipTransUpgradeAvcOnlyToMixReq,
	// Remote initiation transactions:
	kSipTransInviteNoSdpInd,
	kSipTransInviteWithSdpInd,
	kSipTransReInviteNoSdpInd,
	kSipTransReInviteWithSdpInd,
	kSipTransRTCPVideoUpdateInd,
	kSipTransInviteMrcWithSdpInd,
//	kSipTransReInviteMrcWithSdpInd, // was united with kSipTransReInviteWithSdpInd
	kSipTransInviteMrcNoSdpInd,
	kSipTransReInviteMrcWithSdpReq,  //FSN-613: Dynamic Content for SVC/Mix Conf
	kSipTransRTCPVsrInd,
	kSipTransInviteWebRtcWithSdpInd,

} ESipTransactionType;

typedef enum
{
	eChangeModeMask_None = 0x00,
	eChangeModeMask_FlowControl = 0x01,
	eChangeModeMask_Incoming = 0x02,
	eChangeModeMask_ContentChangeRate = 0x04,
	eChangeModeMask_ContentChangeProtocol = 0x08,
	eChangeModeMask_UpgradeToMixed = 0x10
} EChangeModeMask;

typedef enum
{
	eUnKnown,
	eConf,
	eParty,
} EDisconnectInitiator;

typedef enum
{
	eSipPartyCntlDefult,
	eSipPartyCntlWebRtc
} ESipPartyCntlType;

inline EChangeModeMask& operator|= (EChangeModeMask& left, const EChangeModeMask right)
{
	int temp = (int)left;
	temp |= (int)right;
	left = (EChangeModeMask)temp;
	return left;
}

#define VSW_FLOW_CONTROL_RATE_THRESHOLD 0.50  // Percent from original video rate

//#define MIX_MODE_SUPPORT_FOR_AVC

#define MAX_NUM_RECV_STREAMS_FOR_MIX_AVC_VIDEO        2
#define MAX_NUM_RECV_STREAMS_FOR_VSW_RELAY            2
#define MAX_NUM_RECV_STREAMS_FOR_FIR_FILTER		      3

typedef struct {
	BYTE active;
	unsigned int ssrc;
	TICKS lastFirTime;
} FIR_FILTER_ST;


typedef std::map<unsigned int, CStructTm> FirTreatmentMap;



class CSipParty: public CIpParty
{
CLASS_TYPE_1(CSipParty, CIpParty)
public:
	// Constructors
	CSipParty(ESipPartyCntlType sipCntlType = eSipPartyCntlDefult);
	virtual ~CSipParty();
	virtual const char* NameOf() const { return "CSipParty";}

	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	virtual void  Create(CSegment& appParam);
	virtual void  HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);
	virtual BOOL  DispatchEvent(OPCODE event,CSegment* pParam = NULL);

	ESipDialState GetDialState() const {return m_eDialState;}
	void SetDialState(ESipDialState state) {m_eDialState = state;}
	char* GetTransactionTypeAsString(ESipTransactionType type);
	BYTE IsActiveTransaction() const;
	BYTE IsNeedToPendNewTransaction() const;
	//CDR_MCCF:
	virtual void ContinueToCloseTIPcallReq();

	// any case state
	void OnVidBrdgRefreshAnycase(CSegment* pParam);
	void OnVidBrdgVideoInSyncAnycase(CSegment* pParam);
	void OnMcuMngrPartyMonitoringReqAnycase(CSegment* pParam);
	virtual void OnMcuMngrPartyMonitoringReq(CSegment* pParam);
	virtual void OnBridgeNumberingMessageAnycase(CSegment* pParam);
	virtual void OnPartyUpdateConfRsrcIdForInterfaceAnycase(CSegment* pParam);
	void OnPartySendFaultyMfaToPartyCntlAnycase(CSegment *pParam);
	void OnPartyDifferentPayloadAnycase(CSegment* pParam);

//	void OnPartyChannelsDisconnectedAnycase(CSegment* pParam);
	void OnPartyDtmfIndAnycase(CSegment* pParam);
	// channel connect
	void OnSipLogicalChannelConnect(CSegment* pParam);
	// info about channels
	void OnSipPartyMonitoring(CSegment* pParam);

	virtual void OnConfCloseCall(CSegment* pParam) = 0;
	virtual void OnConfReadyToCloseCall(CSegment* pParam);
	virtual void OnConfDisconnectChannels(CSegment* pParam);

	//disconnecting
	void OnPartyDisconnectToutDisconnecting(CSegment* pParam);

	virtual void OnPartyCallFailed(CSegment* pParam);
	virtual void OnPartyCallReinvite(CSegment* pParam);
	virtual void OnPartyCallFailedDisconnecting(CSegment* pParam);
	virtual void OnPartyBadStatus(CSegment* pParam);
	virtual void OnPartyBadStatusConnecting(CSegment* pParam) = 0;
	virtual void OnPartyBadStatusDisconnecting(CSegment* pParam);
	virtual void OnPartyRemoteCloseCall(CSegment* pParam);
	virtual void OnPartyCallClosed(CSegment* pParam);
	virtual void OnPartyReInviteResponseDisconnecting(CSegment* pParam);
	virtual void OnPartyRemoteH230(CSegment* pParam);
	void    OnPartySinglIntraAvMcu(CSegment* pParam);
	virtual void OnPartyForwardRemoteH230(CSegment* pParam);
	virtual void OnPartyTransportError(CSegment* pParam);
	virtual void OnPartyTransportErrorDisconnecting(CSegment* pParam);
	virtual void OnPartyReceivedAckDisconnecting(CSegment* pParam);
	void SendSiteAndVisualNamePlusProductIdToPartyControl(CSegment* pParam);
	void OnPartyConnectTout(CSegment* pParam);
	void OnPartyConnectToutDisconnecting(CSegment* pParam);
	virtual void OnPartyChannelsConnected(CSegment* pParam);
	virtual void OnPartyChannelsDisConnected(CSegment* pParam);
	virtual void SerializeNetSetup(DWORD channelType,CSegment* pSeg);
	virtual void SetPartyToSecondary(WORD reason,CSecondaryParams *pSecParamps = NULL);
	virtual void OnConfBridgesUpdated(CSegment * pParam);
	void OnConfPartyReceiveVidBridgeConnectedAnycase(CSegment* pParam);
	virtual void CleanUp();
	BYTE IsMediaContaining(cmCapDataType dataType, cmCapDirection direction);
    void GetIpCallIdentifiers (IP_EXT_DB_STRINGS* ipStringsStruct);
    BYTE IsOfferer() const;
    // Move:
    void OnConfSetMoveParams(CSegment* pParam);


    // Data token messages from fecc bridge:
    void OnFeccBridgeTokenRequest(CSegment* pParam); // not in use
    void OnFeccBridgeTokenAccept(CSegment* pParam);
    void OnFeccBridgeTokenReject(CSegment* pParam);
    void OnFeccBridgeTokenWithdraw(CSegment* pParam);
    void OnFeccBridgeTokenReleaseRequest(CSegment* pParam);
    void OnFeccBridgeTokenRelease(CSegment* pParam);
    // Token request/release event
    void OnIpDataTokenMsg(CSegment* pParam);
    void OnIpFeccKeyMsg(CSegment* pParam);

    // Sip Transaction functions:
    void  StartTransaction(ESipTransactionType eTransactionType, OPCODE opcode = DEFAULT_OPCODE, CSegment * pParam = NULL);
    ESipTransactionType EndTransaction(EPendingTransType &bPendingTrns);
    void  EndTransactionByPartyIfNeeded();
   	void  OnPartyReceivedReInviteConnected(CSegment* pParam);
    // Sip Transaction handling messages functions:
    void  OnTransEndTransactionAnycase(CSegment* pParam);
    void  OnTransMuteMediaAnycase(CSegment* pParam);
    void  OnTransReceiveReCapAnycase(CSegment* pParam);
    void  OnTransUptateChannelsStatusAnycase(CSegment* pParam);
    void  OnTransChannelsConnectedAnycase(CSegment* pParam);
    void  OnTransRemoteCapsReceivedAnycase(CSegment* pParam);
    void  OnTransRemoteConnectedAnycase(CSegment* pParam);
    void  OnTransSendChannelHandle(CSegment* pParam);
    void  OnTransOriginalRemoteCapsAnycase(CSegment* pParam);
    void  OnTransSiteVisualNameAnycase(CSegment* pParam);
    void  OnTransUpdateEncStatusAnycase(CSegment* pParam);
    void  SendFlowControlMessage(APIU32 videoType,cmCapDirection mediaDirection,APIU32 rate);
    void  SendFlowControlToCs(CSegment* pParam);
    void  OnRemoveSelfFlowControlConstraint(CSegment* pParam);
    void  OnTransUpdateDbAnycase(CSegment* pParam);
    void  OnTransDisconnectBridgesAnycase(CSegment* pParam);
    void  OnTransConnectBridgesAnycase(CSegment* pParam);
    void  OnTransSendTokenReleaseForHoldAnycase(CSegment* pParam);
    ESipTransactionType  UniteConfAndTransModes(BYTE bSendFlowControl);
    BYTE  IsCurrentModeIncludedInCapsWithoutRate(CSipCaps* pCaps);
    void OnChangeRate(CSegment* pParam);
    BYTE HandleAtomicAction(CSipCaps* pCurRemoteCaps , CIpComMode* pBestMode);


    //Lpr
    void  UpdatePartySipLprVideoBitRate(CSegment* pParam);
    void  SendLprFlowControlToCs(CSegment* pParam);
    BOOL  CheckFlowControlDetails(DWORD newVidRate, BYTE direction);
    void SendFlowControlMessageIfPossible(DWORD newVidRate);
    void OnLprUpdatedPartySIPVideoBitRate(CSegment* pParam);
    void UpdatePartyVideoBitRate(CSegment* pParam);
    ESipTransactionType IsNeedToSendReInvite(BOOL& iceChange);
    void HandleIceFallbacks();
    BYTE IsNeedToSendReInviteAfterTIPFallback();
    void  OnConfChangeModeConnect(CSegment* pParam);
    void  AddToChangeModeMask(EChangeModeMask newChangeMode);
    // chair (for filtering fecc in COP)
    void  OnSetPartyToLeader(CSegment* pParam);
    ESipTransactionType  SendReInviteAfterEndTransactionIfNeeded();
    void  SetGlareStatus(BYTE bIsGlareStatus) {m_bIsGlareStatus = bIsGlareStatus;}
    BYTE  IsGlareStatus();
    void  HandleGlareStatus();
    void  SetSendIntraOnEndOfTransaction(BYTE setValue) {m_bSendIntraOnEndOfTransaction = setValue;}
    BYTE  GetSendIntraOnEndOfTransaction() {return m_bSendIntraOnEndOfTransaction;}
    void  OnSendIntraAfterTransactionTout();
    void OnTransHandleIceConnectivityCheckComplete (CSegment * pParam);
    ESipTransactionType GetActiveTransactionType() {return m_eActiveTransactionType;}
    // ppc
    void  OnPartyCntlUpdatePresentationOutStream(CSegment* pParam);
    void  OnPartyUpdatedPresentationOutStream(CSegment* pParam);
    void  OnPartyBfcpMsgInd(CSegment* pParam);
    void  OnPartyBfcpMsgDelayAckTimeout();//BRIDGE-13253
    void  OnPartyBfcpMsgDelayAck();
    void  SendIntraToEP();
    void  OnPartyBfcpTransportInd(CSegment* pParam);
    void  OnTipCntlContentMsgInd(CSegment* pParam);
    void  OnContentBrdgTokenMsg(CSegment* pParam);
    void  SpreadAllH239Msgs(CSegment* pParam, EMsgDirection direction, BYTE isOldTokenMsg, DWORD opcode, EMsgStatus msgStat = eMsgFree);
    EHwStreamState SetCorrectStreamStateForTMM();
    void  SendContentOnOff();
    void  HandleTMMList(CTokenMsgMngr* tokenMsgList);
    void  DisconnectPartyDueToProblemsWithH239RtpStream();
    void  HandleRtpProblemsDuringClosingContentStream();
    void  OnRtpAckForContentOnOff(CSegment* pParam);
    void  OnRtpAckForEvacuateContentStream(CSegment* pParam);
    void  OnRtpAckForContentTout(CSegment* pParam);
    void  OnRtpAckForEvacuateTout(CSegment* pParam);
    void  OnContentBrdgSimulateIndRefreshVideo(CSegment * pParam);
    void  OnContentBrdgRefreshVideo(CSegment * pParam);
    void  ContentIntraRequestFiltering();
    void  OnConfChangeModeContentRate(DWORD newContentRate);
    void  OnConfChangeModeContentProtocol(CIpComMode *pNewMode);
    void  OnContentProviderIdentityTimer(CSegment* pParam);
    void  DeleteProviderIdentityTimerIfNeeded(DWORD opcode, BYTE mcuNum, BYTE terminalNum);
    void OnSipConfNIDivrProviderEQ (CSegment * pParam);
    void SipConfNIDConfirmationInd(DWORD sts);
    //CSipTransaction* GetSipTransaction (){return m_pSipTransaction;}
    //CConfApi* GetConfApi (){return m_pConfApi;}
    void  OnSipPartyUnMuteClosingChannel(CSegment* pParam);
    void SetBfcpConnected(BOOL isConnected){ m_bBfcpConnected = isConnected;}
    BYTE GetBfcpConnected(){ return m_bBfcpConnected == TRUE;}
    
    void	SetBfcpCachedMsgType(DWORD bfcpMsg = 0) {m_BfcpMsgCachedType = bfcpMsg;}
    DWORD  	GetBfcpCachedMsgType() {return m_BfcpMsgCachedType;}
    void     SetBfcpCachedMsgTime();
    DWORD	GetBfcpCachedMsgTime(){return m_BfcpMsgCachedTime;}

    void OnPartyReceivedVideoPreference(CSegment* pParam);
    void OnConfVidBrdgUpdatedWithNewRes(CSegment* pParam);
    void OnTransChangeVideoResAnycase(CSegment* pParam);
    void OnPartyCntlStartVideoPreference(CSegment * pParam);
    
    void OnPartyReceivedVsrInd(CSegment* pParam);
    void OnTransChangeVideoAfterVsrMsgAnycase(CSegment* pParam);
    void  OnPartyMsftOutslavesCreated(CSegment* pParam);
    void OnPartyReceivedSingleVsrInd(CSegment* pParam);
    void OnPartyReceivedAvMcuSingleVsrInd(CSegment* pParam);

    void MuteMediaIfNeeded(cmCapDirection eDirection, BYTE bIsTipForceMuteOutChannels, BYTE channelsMask=0xFF);
    void MuteMediaByParty(BYTE bIsMute,cmCapDataType eMedia,cmCapDirection eDirection, ERoleLabel eRole);
	void SendTipCallEndMsgAndDisconnectToSlaves();

    // TIP
	void SetIsTipCall(BYTE);
	virtual bool GetIsTipCall() const { return m_bIsTipCall; }
	void UnmuteMediaAndSendReinviteForTipIfNeeded();
	void OnReceivedTipLastAck();
	void SetTipPartyOnHold(BOOL isOnHold);
	BOOL GetTipPartyOnHold();
	DWORD GetSlaveRsrcIdAccordingToSlaveType(DWORD tipType);
    void OnTransLastTargetModeMsgAnycase(CSegment* pParam);

	/////////////////////////////////////////////////////////////////////////////

	virtual ETipPartyTypeAndPosition GetTipPartyType() const { return m_tipPartyType; }
	void SetTipPartyTypeAndPosition(ETipPartyTypeAndPosition);
	ETipPartyTypeAndPosition  GetTipPartyTypeAndPosition();
	
	/*  MSSlave-Flora Yao-2013/10/23- MSSlave Flora Comment: */
	virtual eAvMcuLinkType GetAvMcuLinkType() const { return eAvMcuLinkNone; }
	virtual DWORD GetMSSlavePartyIndex() const { return 0;}

	void OnTipCntlNegotiationResult(CSegment * pParam);
	void OnSlaveSendRtcpFastUpdate(CSegment* pParam);
	void SendMessageFromMasterToSlave(WORD destTipPartyType, DWORD opcode, CSegment *pMsg);
	void SendMessageFromMasterToSlaveByID(DWORD rsrcId, DWORD opcode, CSegment *pMsg);
	void SendMessageFromSlaveToMaster(WORD srcTipPartyType, DWORD opcode, CSegment *pMsg);
	void AddSlaveParty(DWORD srcTipPartyType);
	void OnMessageFromMasterToSlave(CSegment * pParam);
	void OnMessageFromSlaveToMaster(CSegment * pParam);
	void OnSendNewNameWithRoomId(CSegment * pParam);
	void SetSlavePartyRsrcId(CSegment * pParam);
	void FallBackToRegularSip(RemoteIdent rmtIdent);
	void OnSlaveSendMonitoringReq(CSegment* pParam);
	void SendMuteToSlaves();
	void ChangeVideoOutForPolycomTip(); //_t_p_
	void OnTimerContentSpeakerIntraRequest(CSegment* pParam);
	void UpdateVidBrdgTelepresenseEPInfo(eTelePresencePartyType telePresencePartyType); //_e_m_

	//_dtls_
	void UpdateDtlsCapsForAllSlaves();
	void UpdateDtlsCapsForSlave(ETipPartyTypeAndPosition slaveType,  CSegment *pSeg);
	void SendUpdateChannelsRtpIfNeeded();
	void OnConfSetCapsAccordingToNewAllocation(CSegment* pParam);
	void UpdateBridgesForTipNegotiation();
	void SetScmAndChannelsForTipNegotiation();
	void SetContentEncryptionForTipVideoAux(CSipComMode* pNewMediaMode);
	void AddTipSlaves();
	void ResumeMediaAndSendReinviteForTip();
	void RemoveDtlsFromTargetModeIfNeeded( CIpComMode* pBestMode);

	void SetIsTipNegotiationActive(BYTE bIsActive);
	BYTE GetIsTipNegotiationActive() const {return m_bHandlingTipNegotiationResult;}
	void OnTimerCreateSlaveAck(CSegment* pParam);
	void OnTimerDisconnectSlaveParty(CSegment* pParam);
	void OnTimerSlaveRecapAck(CSegment* pParam);
	WORD TipGetNumOfStreams();
	BYTE TipGetIsAudioAux() {return m_bIsAudioAux;}
	BYTE TipGetIsVideoAux() {return m_bIsVideoAux;}

	void OnIceBandwidthInd(CSegment* pParam);
	void OnSipBandwidthAllocationStatus(CSegment* pParam);
	BYTE GetIsPartialConnectionForVSW() const {return m_IsPartialConnectionForVSW;}
	void SetIsPartialConnectionForVSW(BYTE bIsPartialConnectionForVSW) {m_IsPartialConnectionForVSW = bIsPartialConnectionForVSW;}

	void OnSipBandwidthReInviteNeeded(CSegment* pParam);
	void OnIceInsufficientBandwidthEvent(CSegment* pParam);

	void IceFallbackToSip();

	CIpComMode* GetTargetModeMaxAllocation() const { return m_pTargetModeMaxAllocation; }

	// BFCP
	void OnBfcpStartReestablishConnection(CSegment* pParam);
	void OnBfcpEndReestablishConnection(CSegment* pParam);
	void HandleBfcpScmAndCaps();

	void CloseTipSessionAndSendMuxDisconnect();
	void ForwardEventToTipSlaves(CSegment* pParam, OPCODE opCode); // overriding function from CParty

	//FEC
	void UpdateVideoRateForFECorRED(CSegment* pParam);
	void UpdateVideoRateForMSrtvFEC();
	void VideoRateForFecOrRedUpdated(CSegment* pParam);

	//LYNC2013_FEC_RED:
	void  UpdateVideoRateForMSsvcFEC(DWORD newFecPercent);
	void  UpdateVideoRateForRED();
	void  SetMoreDetailsForMsSvcIfNeeded(DWORD newPeopleRate);
	DWORD CalcVideoRateForMSsvcFEC(DWORD videoRateBeforeChange, DWORD newFecPercent, DWORD CurrentFrameRate);
	DWORD CalcVideoRateForRED(DWORD videoRateBeforeChange);
	void  OnPartyReceivedAvMcuSingleFecOrRedMsg(CSegment* pParam);
	DWORD CalcVideoRateForMSrtvFEC(DWORD CurrentVideoRate,DWORD frameRate);

	void OnPartyUpdateMuteIcon(CSegment* pParam);

	BOOL ParseIncomingPrecedenceInfo(const sipMessageHeaders* const pHeaders, CQoS& qos, BOOL* pRequireHeaderFailure = NULL) const;

	void OnRelayAskEndpointForIntra(CSegment* pParam);
	void OnMrmpRtcpFirInd(CSegment* pParam);
	void OnFirPeopleTimer(CSegment* pParam);
	void OnFirTreatmentMapTimer(CSegment* pParam);

	DWORD GetSSRcIdsForAvc(int ind, cmCapDirection direction, cmCapDataType aDataType);
    void  SetConfMediaType(eConfMediaType aConfMediaType);

    //FIR Filter
    BYTE FirFilter(unsigned int ssrc, TICKS curTimer);
	FirTreatmentMap m_firTreatmentMap;


	//LyncCCS
	void  OnConfPwdInd(CSegment* pParam);
	void OnPartyPwdReq(CSegment* pParam);
	void OnPartyUpgradeToMixChannelsUpdated(CSegment* pParam);
	//CDR_MCCF:
	void SetDisconnectInitiator(EDisconnectInitiator bDisconnectInitiator) {m_disconnectInitiator = bDisconnectInitiator;}
	EDisconnectInitiator GetDisconnectInitiator() const {return m_disconnectInitiator;}
	void OnSendMrmpStreamIsMustReq(CSegment* pParam);
	void OnMrmpStreamIsMustAck();
	
	void SetNonTipPartyOnHold(enMediaOnHold  eMediaOnHold) {m_eNonTipOnHold = eMediaOnHold;}
	enMediaOnHold GetNonTipPartyOnHold() {return m_eNonTipOnHold;}
	void SetNeedToStartIVRAfterFallbackFromTip(BYTE  needToStart) {m_bIsNeedToStartIVRAfterFallbackFromTip = needToStart;}
	
	void SendAvcToSvcArtTranslatorDisconnectedToPartyControl(STATUS status);

	BOOL IsNeedToSendRtcpVideoPreference() const;
	void OnSendVideoPreferenceReqTout() ;
	DWORD GetRtcpLyncPreferenseReqIntervalFromFlag();

	//Begin:modified by Richer for 8072
	void ClearNumOfBfcpReestablish() {m_numOfBfcpReestablish = 0;}
	//Begin:modified by Richer for 8072
	void OnVideoBridgeMultiVSRReq(CSegment * pParam);

	virtual void ForwardRemoteH230ToMsSlavesControllerIfNeeded(CSegment* pParam);
	void  OnPartyStreamsIntraReq(CSegment* pParam);

	void  OnTransSendSingleUpdatePacsiInfoAnycase(CSegment* pParam);
	void OnRefreshAnycase(WORD ignore_filtering,DWORD remoteSSRC, DWORD priorityID, DWORD msslaveIndex = 0, DWORD isSlaveRTV = FALSE);
	void OnInSyncAnycase(DWORD msslaveIndex = 0);
	void OnMSSlaveRefreshAnycase(CSegment* pParam);
	void OnMSSlaveVideoInSyncAnycase(CSegment* pParam);
	void SendMessageFromMSSlaveToMain(DWORD opcode, CSegment *pMsg);


	//////////////////////////////////////
	BOOL GetIsActiveContent(){return m_isActiveContentForTip;};
	BOOL  IsThereActiveInviteReq() const;

	void OnTokenRecapCollisionEndAnycase(CSegment* pParam);

	/////////////////////////////////
	void OnConfRecordingControlConnected(CSegment* pParam);
	void OnConfRecordingControlAnycase(CSegment* pParam);
	void OnPartyRecordingControlStatusAnycase(CSegment* pParam);
	void  OnPartyLayoutControlAnycase(CSegment* pParam);
	DWORD GetPartyContentRate() {return m_PartyContentRate;}
	void SetPartyContentRate(DWORD contentRate) { m_PartyContentRate = contentRate;}
	void SetIceCheckCompleteState(BYTE);
	void PartyUpdatePartyControlOnRemoteCaps();
	void StopWaitForContentReInvite();
	/*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
       void SendByeToPartyAnycase(CSegment* pParam);
	/*End:added by Richer for BRIDGE-12062 ,2014.3.3*/
    BOOL GetBOOLDataByKey(const std::string& key); //BRIDGE-13154

    void CheckIfIncomingNewFIRisPending(RelayIntraParam& currentIntraParam);
    void SaveIncomingIntraRequest(CSegment* pParam);
    BOOL IsSsrcEqual(RelayIntraParam& currentIntraParam);

private:

	//AT&T
    virtual void        CollectDigitsExternalIvr(DialogState& state, const CollectElementType& collect);
	virtual void        PlayFileExternalIvr(DialogState& state, const MediaElementType& media);
	virtual void        ExternalIvrStartDialog(DialogState& state, DWORD mcmsDelayInMsecs = 0);

	BOOL 				IsPayloadTypeIsForFecc(WORD payloadType); //BRIDGE-15265


protected:
	CSipCntl*				m_pSipCntl;
	ESipDialState			m_eDialState;
	BYTE					m_bIsAdvancedVideoFeatures;
	char*					m_alternativeAddrStr;  //If forward a call to other address
	ESipTransactionType		m_eActiveTransactionType;
	CSipTransaction*		m_pSipTransaction;
	CIpComMode*             m_pUpdateTargetMode; //this target mode will hold all new changes that maybe made in the time of an active trans
    CSegment*               m_pUpgradeParams;
	EChangeModeMask        	m_maskRequiredChangeMode; // used when a change mode or flow control request from conference arrived during an active transaction
	APIU32					m_lastConstraintRate;
	DWORD					m_minValForGlareTimer; // In ticks units
	DWORD					m_maxValForGlareTimer; // In ticks units
	BYTE					m_bIsGlareStatus;
	BYTE					m_bIsConfChangeMode;
	BYTE					m_bSendIntraOnEndOfTransaction;
	BYTE					m_bIsCallConnected;
	BYTE					m_bIsRcvCheckCompleteAndNeedToStartIVR;
	CIpComMode*				m_pTargetModeMaxAllocation;//For DPA
	CTokenMsgMngr*			m_pH239TokenMsgMngr;
	BYTE					m_bBfcpConnected;
	WORD					m_ProviderMessageCounter;
	DWORD					m_BfcpMsgCachedType; 
	DWORD					m_BfcpMsgCachedTime;
	BYTE					m_numOfIframesSent;

	DWORD					m_repeatIntra;
	DWORD					m_repeatIntraNumber;
	DWORD					m_repeatContentIntra;
	DWORD					m_repeatContentIntraNumber;
	WORD					m_lastActiveContentSenderID;
	DWORD					m_contentIntraInterval;

	DWORD 					m_RTVLastVideoPreferenceWidth;
	DWORD 					m_RTVLastVideoPreferenceHeigh;
	BYTE					m_bIsRtvPreferenceMsgSent;

	BOOL					m_bMsftRecevVsrInActiveTrans; // did we received MSFT RTCP VSR indication while we were in active trnsaction
	

	// TIP
	BYTE   					m_bIsTipCall;
	BYTE					m_bIsPolycomFromRTCP; /* If RTCP CNAME contains Polycom in the string value will be true */
	ETipPartyTypeAndPosition m_tipPartyType;
    DWORD               	m_SlaveAuxRsrcId;
    DWORD               	m_SlaveLeftRsrcId;
    DWORD               	m_SlaveRightRsrcId;
    WORD 					m_NumOfSlaves;
    BYTE 					m_NumOfAckFromSlaves;

    WORD 					m_TipNumOfStreams;
    BYTE 					m_bIsAudioAux;
    BYTE 					m_bIsVideoAux;

    WORD					m_nSlavesReturnRecapAck;
    BYTE					m_bIsNeedTipNegotiation;
    BYTE				    m_bHandlingTipNegotiationResult;
    BYTE                    m_IsPartialConnectionForVSW;//VNGR-21599

    int 					m_TipNotSentIntraResponseCounter;
	BYTE					m_bIsNeedToStartIVRAfterFallbackFromTip;

	DWORD					m_bIsNeedTipReinviteAfterNegotiation;

	BOOL					m_bTipLastAckReceived;
	BOOL					m_bTipEndSuccessSent;
	BOOL					m_bIsTipResumed;
	BOOL                    m_bIsTipPutOnHold;

	//CDR_MCCF:
	WORD                    m_oldstate;
	EDisconnectInitiator    m_disconnectInitiator;

	CSegment*				m_pFirPeopleSegnent;
	int						m_firPeopleCounter;
	RelayIntraParam 		m_incomingIntraParam;
	BOOL					m_bIsIncomingFIR;

	WORD					m_numOfBfcpReestablish;

	bool                    m_bIsNeedToUpdateContentOnTheTransactionEnd;
//    DWORD                   m_SsrcIdsForAvcParty[MAX_NUM_RECV_STREAMS_FOR_MIX_AVC_VIDEO];
	CSvcPartyIndParamsWrapper  m_SsrcIdsForAvcParty;
	BYTE					m_bTransactionSetContentOn; // Used at the first reinvite-req in dial-out calls, that content is set to on by transaction if local and remote support bfcp

	FIR_FILTER_ST          	m_FirFilter[MAX_NUM_RECV_STREAMS_FOR_FIR_FILTER];
	BYTE					m_bIsFirFilterCreated;

	CStructTm				m_lastTimeFirTreated;

	// This flag prevents suggesting content to EP in an endless loop. Added by Efi
	BYTE 					m_isContentSuggested;
	BYTE 					m_isNeedToSendReInviteAfterTIPFallBack;
	BYTE                    m_isActiveContentForTip;
	WORD 					m_num_content_intra_filtered;

	DWORD					m_nIntraCounter;
	DWORD					m_nIntraAfterTimer;
	DWORD					m_nIntraAfterTransaction;
	
	enMediaOnHold  m_eNonTipOnHold;
	eChangeModeState 		m_changeModeState;
	DWORD m_PartyContentRate;
	ST_VSR_SINGLE_STREAM*	m_pLastTxVsr;
	BOOL    				m_isAllOutSlavesConnected;

	// Token/Recap Collision Detection
	//==================================
	CSegment				m_pendedToken;
	DWORD					m_pendedTokenOpcode;
	EMsgDirection			m_pendedTokenDirection;
	BYTE 					m_pendedTokenIsOldToken;
	bool m_bIsDelayedOnce; //if the message arrived in eMsgDelayed state

//	void HandleNewTss(cmCapDataType eMediaType,CTimeSlotStream* pTss);
//	void DisconnectTss(cmCapDataType eMediaType);
	void TellConfOnDisconnecting(int reason,const char* alternativeAddrStr=NULL,DWORD MipErrorNumber = 0);
	void UpdateDbOnChannelsConnected();
	void UpdateDbOnChannelsDisconnected();
	void UpdateDbScm();
	void UpdateDbOnSipPrivateExtension();
	void DisconnectChannels();
	virtual void DestroyPartyTask();
	virtual BOOL inline IsPartyIn() const {return FALSE;}
	void ConnectCall(BYTE isInitialConnection = YES);
	void DispatchPartyCallClosed();
	void InformConfRemoteConnect();

	void IntraRequestFiltering(WORD videoSyncLost=3, BYTE bIsGradualIntra = FALSE, DWORD remoteSSRC = NON_SSRC, DWORD priorityID = INVALID, DWORD msSlavePartyIndex = 0);
	// Notice!!! - if you change SendFastUpdateReq prototype, you MUST change it in SipSlaveParty.h/.c as well.
	virtual void SendFastUpdateReq(ERoleLabel eRole, DWORD remoteSSRC = NON_SSRC, DWORD priorityID = INVALID, DWORD msSlavePartyIndex = 0);
	void OnMcuMngrStartPartyPreviewReq(CSegment* pParam);
	void OnMcuMngrStopPartyPreviewReq(CSegment* pParam);
	void OnMcuMngrIntraPreviewReq(CSegment* pParam);
	void StopAllPreviews();
	DWORD GetTimerValueForGlare();
	void OnSipGlareTimer(CSegment* pParam);
	void OnGradualTout();
	void OnWaitToStartTipFallBackTout(CSegment* pParam);
	void OnWaitToStartReinviteForContentTout(CSegment* pParam);
	void UpdateCurrentRcvModeAccordingToTarget();
	void IceMakeAnswerIndWhileDisconnecting();
//	BYTE GetIsMrcCall() const {return m_bIsMrcCall;}
//	void SetIsMrcCall(BYTE bIsMrcCall) {m_bIsMrcCall = bIsMrcCall;}
    void OnScpRequestFromEp(CSegment* pParam);
    void OnScpNotificationIndFromEp(CSegment* pParam);
	void OnAckForScpStreamsReq(CSegment* pParam);
	void OnScpNotificationToEpReq(CSegment* pParam);
	void OnScpIvrStateNotificationReqToEp(CSegment* pParam);
	void OnSipPartyNotificationAckFromEP(CSegment* pParam);
	void OnAckForScpNotificationInd(CSegment* pParam);
	void OnScpPipesMappingNotificationToEpReq(CSegment* pParam);
	void OnUpdateScmStreams(CSegment* pParam);
	void OnRemoveAvcToSvcArtTranslatorAnycase(CSegment* pParam);

	DWORD GetFirChannelHandle();
	void DoSendRelayAskEndpointForIntra(CSegment* pParam, DWORD channelHandle);

	void HandleContentScmAndCaps();
	BOOL IsTIPContentEnable() const;

	void FirFilterCreate();
	bool FirShouldBeHandledForSpecSsrc(unsigned int specSsrc);
	void OnUpdateArtWithSsrc(CSegment* pParam);
	void  HandleMsftVsrOnEndTransaction(bool isNeedToSendReinvite,ESipTransactionType endedTransType);
	void OnConfActiveMediaForAVMCULync();

	PDECLAR_MESSAGE_MAP
};

#endif

