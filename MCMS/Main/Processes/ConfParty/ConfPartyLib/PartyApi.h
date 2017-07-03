#ifndef _PARTYAPI
#define _PARTYAPI

#include "TaskApi.h"
#include "NetSetup.h"
#include "SipNetSetup.h"
#include "RsrcParams.h"
#include "AllocateStructs.h"
#include "H221.h"
#include "OpcodesMcmsInternal.h"
#include "LPRData.h"
#include "BondingPhoneNumbers.h"
#include "H320ComMode.h"
#include "Q931Structs.h"
#include "SIPTransaction.h"
#include "CopVideoTxModes.h"
#include "PartyPreviewDrv.h"
#include "ConfPartyStructs.h"
#include "SvcPartyIndParamsWrapper.h"
#include "IpRtpReq.h"
#include "SipVsrControl.h"

#define NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS 2
#include "DialogStart.h"
#include "MscIvr.h"

#include "MccfHelper.h"

////////////////////////////////////////////////////////////////////////////
class CH323NetSetup;
class CSipNetSetup;
class CComModeH323;
typedef CComModeH323 CIpComMode;
class CSipComMode;
class CCapH323;
class CCapH320;
class CSipCaps;
class CSipChanDifArr;
class CSecondaryParams;
class CIsdnNetSetup;
class CIsdnPartyRsrcDesc;
class CQoS;
class CComMode;
class CTerminalNumberingManager;
class CMrmpScpStreamsRequestStructWrap;
class CScpNotificationWrapper;
class CScpPipeMappingNotification;

enum
{
	NO_CHANGE_MODE      = 0,
	REGULAR_CHANGE_MODE = 1,
	VIDEO_ASYMMETRIC_CHANGE_MODE,
	VIDEO_ASYMMETRIC_CHANGE_MODE_WAIT_OTHER,
	VIDEO_ASYMMETRIC_CHANGE_MODE_RMT_RCV,
	VIDEO_ASYMMETRIC_CHANGE_MODE_WAIT_OTHER_RMT_RCV
};

// #define MIX_MODE_SUPPORT_FOR_AVC

////////////////////////////////////////////////////////////////////////////
//                        CPartyApi
////////////////////////////////////////////////////////////////////////////
class CPartyApi : public CTaskApi
{
	CLASS_TYPE_1(CPartyApi, CTaskApi)

public:
	CPartyApi();
	~CPartyApi();

	// Initializations
	void Create(void (* entryPoint)(void*),
	            COsQueue& creatorRcvMbx,
	            COsQueue& confRcvMbx,
	            DWORD partyRsrcId,
	            DWORD partyMonitorId,
	            DWORD confMonitorId,
	            const char* numericConfId,
	            const char* partyName,
	            const char* confName,
	            WORD serviceId = 0,
	            WORD termNum = 1,
	            WORD mcuNum = 1,
	            const char* password = "",
	            WORD voice = 0,
	            WORD isChairEnabled = 1,
	            BYTE isGateWay = 0,
	            BYTE isRecording = 0,
	            BYTE isAutoVidBitRate = 1,
	            BYTE isNoVideoRsrcForVideoParty = FALSE);
	// Operations
	virtual const char* NameOf() const;

	void ForceKill();
	void HandlePartyExternalEvent(CSegment* pMsg, OPCODE opCode);

	// API for conf
	WORD EstablishCallPstn(PartyRsrcID partyRsrcID, CNetSetup* pNetSetup, CRsrcParams* pNetRsrcParams, CIsdnPartyRsrcDesc* desc, WORD room_id);
	WORD EstablishVideoCall(PartyRsrcID partyRsrcID, CNetSetup* pNetSetup,
	                        CRsrcParams* pNetRsrcParams, CIsdnPartyRsrcDesc* desc, WORD chnlNum,
	                        CComMode* pConfScm, CCapH320* pLocalCaps, WORD mcuNum, WORD terminalNum, WORD room_id);
	void SetMonitorPartyId(DWORD PartyId);

	void DelNetChannel(WORD seqnum);
	void LogicalDelNetChannel(WORD discause = causDEFAULT_VAL);
	void ChangeModeH323(CComModeH323* pScmH323, WORD changeModeState = 0, CCapH323* pLocalCapH323 = NULL, CRsrcParams** avcToSvcTranslatorRsrcParams = NULL,CRsrcParams* pMrmpRsrcParams = NULL);
	void SendInfoToRss(BYTE isStreaming, char* pExchangeConfId);
	void ChangeContentMode(CComModeH323* pScmH323, WORD changeModeState, BYTE bIsSpeaker);
	void TokenRecapCollisionEnded();
	void ChangeModeIp(CIpComMode* pIpComMode, BYTE changeModeState, BYTE bIsContentSpeaker, CSipCaps* pLocalCapSip = NULL,CRsrcParams** avcToSvcTranslatorRsrcParams = NULL,CRsrcParams* pMrmpRsrcParams=NULL,BYTE IsASSIPContentEnable = FALSE);
	void BridgesUpdated(CIpComMode* pIpComMode, UdpAddresses sUdpAddrressesParams, DWORD status, BYTE bIsContentSpeaker,BYTE bUpdateMixModeResources, CRsrcParams** avcToSvcTranslatorRsrcParams = NULL,CRsrcParams* pMrmpRsrcParams = NULL, BYTE bShouldPartyRemoveContent = FALSE);

	void SendVIN(WORD mcuNumber, WORD terminalNumber, PartyRsrcID partyId);

	void AssignTerminalNum(WORD mcuNum, WORD termNum);

	void Export(COsQueue* pDestConfMbx, void* pConfPartyCntl, void* pConfPartyDesc, EMoveType eCurMoveType = eMoveDefault);

	void SetMoveDestConfParams(WORD confType, WORD mcuNumber, WORD terminalNumber, CCopVideoTxModes* pCopVideoTxModes, CVideoOperationPointsSet* pVideoOperationPointsSet);
	void SetRsrcConfIdForInterface(DWORD destResourceConfId);

	void VideoRefresh(WORD ignore_filtering = FALSE,DWORD remoteSSRC = NON_SSRC, DWORD priorityID = INVALID);
	void SendH239VideoCaps();

	void H323EndMediaChannelsConnect(CCapH323& rmtCap, CComModeH323& currentScm, WORD status, BYTE bLateReleaseResourcesInConfCntl = FALSE, BYTE bOnlyAudioConnected = FALSE,
	                                 BYTE isCodianVcr = 0);

	// API for data brdg
	void DataTokenRequest(WORD bitRate);
	void DataTokenRelease();
	void DataTokenReleaseAndFree();
	void DataTokenAccept(WORD isCameraControl);
	void DataTokenReject();
	void DataTokenWithdraw();
	void DataTokenReleaseRequest();
	void OnIpDataTokenMsg(WORD msgType, WORD bitRate = LSD_6400, WORD isCameraControl = 0);
	void OnIpFeccKeyMsg(WORD keyType);

	// API for lobby
	void Transfer(WORD mode);

	void RejectCall(CNetSetup* pNetSetUp);
	void LobbyDestroy();
	// API for net control
	void NetConnect(WORD seqNum, WORD status, BYTE cause = 0);
	void NetDisConnect(WORD seqNum, BYTE cause);

	void UpdateEncryptionCurrentStateInDB(BYTE encryptionCurrentState);
	void EncryptionDisConnect(WORD cause);
	void EndNetDisConnect(WORD seqNum, WORD status);

	void SendLeaderStatus(BYTE isLeader = 0);
	void PartyActionsOnLeaderChanged(BYTE isLeader);
	void InformArtFeccPartyType();

	// API for H323
	void H323EstablishCall(PartyRsrcID partyRsrcID, CCapH323* pLocalCap, CH323NetSetup* pH323NetSetup,
	                       WORD cascadeMode, WORD nodeType, DWORD vidRate, CQoS* pQos,
	                       CComModeH323* pInitiateScm, WORD encAlg, WORD halfKeyType,
					        CRsrcParams* pMfaRsrcParams, CRsrcParams* pMrmpRsrcParams, CRsrcParams* pCsRsrcParams,CRsrcParams** avcToSvcTranslatorRsrcParams ,UdpAddresses sUdpAddressesParams,
					   		WORD mcuNum, WORD terminalNum, CCopVideoTxModes* pCopVideoTxModes,BYTE bNoVideRsrcForVideoParty,
					   		CTerminalNumberingManager* pTerminalNumberingManager, WORD room_Id ,eTypeOfLinkParty linkType,
					   		CSvcPartyIndParamsWrapper& aSsrcIdsForAvcParty);

	void EndH323DisConnect(WORD status);
	void IncreaseDisconnctingTimerInPartyCntl();
	void SendAudioBridgeConnected();
	void SendVideoBridgeConnected();
	void SendFeccBridgeConnected();
	void SendBridgesDisconnected();
	void SendBridgesConnected();

	void SendCsResourceAllocated(CRsrcParams* pCsRsrcParams);

	void IpRmtH230(CSegment* ipSeg);
	void IpSingleIntraForAvMcu(CSegment* ipSeg);
	void ForwardIpRmtH230(CSegment* ipSeg, WORD tipPosition);
	void IpRmtSpecificStreamsFastUpdateReq(CSegment* ipSeg);
	void MuteVideo(WORD onOff);
	void UpdateMuteIcon(WORD onOff);
	void sendPartyArqInd(CSegment* pParam, WORD opcode, WORD localFlag);

	void sendPartyDTMFInd(unsigned char* buffer, DWORD length, DWORD dtmfOpcode = SIGNALLING_DTMF_INPUT_IND);

	void RejectH323Call(CH323NetSetup* pH323NetSetUp, int reason);
	void RejectSipCall(CSipNetSetup* pNetSetup, int reason, WORD sdpLen,
	                   const struct sipSdpAndHeaders* pSdpAndHeaders, char* pAltAddress = "", STATUS status = 0);
	void IdentifyCall(CH323NetSetup* pH323NetSetup, DWORD bIsEncrypted
	                  , int indexTblAuth, BYTE* pSid, BYTE* pAuthKey, BYTE* pEncKey,
	                  DWORD authLen, DWORD encLen);
	void IdentifySip(CSipNetSetup* pNetSetup, const struct sipSdpAndHeaders* pSdpAndHeaders, DWORD sdpLen);
	void H323PartyDisConnect(WORD cause);
	void SetSecondaryCause(WORD reason, CSecondaryParams& secParams);

	void IpDisconnectMediaChannel(WORD channelType, WORD channelDirection, WORD roleLabel);
	void InActivateChannel(WORD channelType, WORD channelDirection, WORD roleLabel);

	void IpLogicalChannelUpdate(DWORD channelType, CH323NetSetup* pH323NetSetup, DWORD vendorType);
	void IpLogicalChannelConnect(CPrtMontrBaseParams* pPrtMonitor, DWORD channelType, DWORD vendorType);
	void IpLogicalChannelDisConnect(DWORD channelType, WORD type, BYTE bTransmitting, WORD roleLabel);
	void H323GateKeeperStatus(BYTE gkState, DWORD reqBandwidth, DWORD allocBandwidth, WORD requestInfoInterval, BYTE gkRouted);
	void IpPartyMonitoringUpdateDB(BYTE channelType, BYTE intraSyncFlag,
	                               BYTE videoBCHSyncFlag, WORD bchOutOfSyncCount,
	                               BYTE protocolSyncFlag, WORD protocolOutOfSyncCount);

	void Rmt323CommModeUpdateDB(const CComModeH323* pCurrMode);

	void SendRemovedProtocolToConfLevel(WORD NuRemovedProtocols, WORD ProtocolPayLoadType, WORD bIsCapEnum = FALSE);
	void UpdateLocalCapsInConfLevel(CCapH323& localCaps);
	void UpdatePartyH323VideoBitRate(APIU32 newBitRate, cmCapDirection channelDirection, ERoleLabel eRole);
	void Send2ChannelsFlowControlToCard(DWORD OutgoingChannelRate, DWORD IncomingChannelRate, DWORD TdmRate);
	void RemoveSelfFlowControlConstraint();

	void SendPartyMonitoringReq(CTaskApp* pTaskApp);

	void SendFlowControlToCs(DWORD newVidRate, BYTE outChannel, CLPRParams* lprParams = NULL);

	void SetServiceProvider(char* ServiceProvider);

	void SendSiteAndVisualNamePlusProductIdToPartyControl(BYTE bIsVisualName, char* siteName,
														  BYTE bIsProductId, char* productId, BYTE bIsVersionId, char* VersionId,
														  eTelePresencePartyType eLocalTelePresencePartyType,
														  BYTE bIsCopMcu = FALSE, RemoteIdent eRemoteVendorIdent=Regular);
	void SendCloseChannelToConfLevel(WORD channelType, WORD direction, WORD roleLabel);

	void H323ConnectFeccControl(int rmtType, WORD casacdeMode);

	// Gateway Party Conference Api Functions
	void SendRemoteCapabilities(CCapH323& rmtCap);

	void H323NonStandardInd(CSegment* pMess);
	void H323RmtCI(CSegment* pMess);

	void H323DBC2Command(CSegment* pMess);
	void H323DBC2CommandInd(DWORD opcode, CSegment* pMess);

	// API for IVR
	void SendOpcodeToIvrSubFeature(DWORD opcode);
	void SendReceivedDtmfToParty(CSegment* pParam);

	void UpdateIVRGeneralOpcode(WORD opcode);

	void SendContentMediaProducerStatusToConfLevel(BYTE channelID, BYTE status);

	// API for ECS:
	// Api from party to conf
	void SendECSToPartyControl();
	// Api from conf to party

	// API For HIGHEST COMMON:
	// Api from party to conf
	void SendH323LogicalChannelReject(WORD channelType, WORD channelDirection, WORD roleLabel);
	void SendIpStreamViolation(WORD cardStatus, BYTE reason, CSecondaryParams& secParams);
	void SendReCapsToPartyControl(CCapH323& rmtCapH323);

	void SendIpDifferentPayload(WORD channelType, WORD channelDirection, WORD payload, WORD roleLabel);
	void DowngradeToSecondary(WORD reason);

	// Multiple links for ITP in cascaded conference feature:
	void SendNewITPSpeakerInd(eTelePresencePartyType itpType, DWORD numOfActiveLinks);
	void SendNewITPSpeakerAckInd();
	void CreateNewITPSpeakerAckReq();
	void CreateNewITPSpeakerReq(DWORD numOfActiveLinks, BYTE itpType);

	// Api from conf to party
	void ReConnectStream(WORD dataType);
	void SetCapsValuesAccordingToNewAllocation(H264VideoModeDetails h264VidModeDetails,MsSvcVideoModeDetails msSvcDetails ,BYTE cif4Mpi, BYTE bIsAudioOnly, BYTE bIsRtv = FALSE,BYTE bIsMsSvc = FALSE,DWORD videoRate = 0);

	// Api for faulty resources
	void SetFaultyResourcesToPartyControlLevel(DWORD reason, MipHardWareConn mipHwConn = eMipNoneHw, MipMedia mipMedia = eMipNoneMedia, MipDirection mipDirect = eMipNoneDirction,
	                                           MipTimerStatus mipTimerStat = eMpiNoTimerAndStatus, MipAction mipAction = eMipNoAction);
	void UpdateGkCallIdInCdr(BYTE* gkCallId);

	// API for Sip:
	void SendSipTransMsg(OPCODE event, CSegment* pSeg);

	void SipConfEstablishCall(CRsrcParams** avcToSvcTranslatorRsrcParams,CRsrcParams* pMrmpRsrcParams, CRsrcParams* pMfaRsrcParams, CRsrcParams* pCsRsrcParams, UdpAddresses sUdpAddressesParams,
	                          CSipNetSetup* pNetSetup, CSipCaps* pLocalCaps, CIpComMode* pInitialMode,
	                          CQoS* pQos, BYTE bIsAdvancedVideoFeatures, const char* strConfParamInfo,
	                          BYTE eTransportType, WORD McuNumber, WORD TerminalNumber, CCopVideoTxModes* pCopVideoTxModes,
	                          const char* alternativeAddrStr, BYTE bNoVideRsrcForVideoParty,
							  CIpComMode* pTargetModeMaxAllocation,CSipCaps* MaxLocalCaps, WORD room_Id,
							  CSvcPartyIndParamsWrapper& SSRCIdsForAvcParty, BYTE bIsMrcCall,BYTE IsAsSipContentEnable, DWORD partyContentRate);
	void SipConfConnectCall();
	void SipConfDisconnectChannels(CSipComMode* pNewTargetMode);
	void SipPartyCallClosed(CSipComMode* pCurrentMode);
	void SipPartyRemoteCloseCall(DWORD reason);
	void SipPartyCallFailed(DWORD reason, DWORD MipErrorNumber = 0);
	void SipPartyCallReinvite(DWORD reason);
	void SipPartyChannelsConnected(CSipComMode* pCurrentMode);
	void SipPartyStatisticsInfo(); //CDR_MCCF
	void UpdatePartyVideoBitRate(DWORD rate, WORD direction, WORD role);
	void SipPartyChannelsUpdated(CSipComMode* pCurrentMode);
	void H323PartyChannelsUpdated(CComModeH323* pCurrentMode);
	void SipPartyChannelsDisconnected(CSipComMode* pCurrentMode);
	void SipPartyReceived200Ok(BYTE bRemovedAudio, BYTE bRemovedVideo, BYTE isUpdateAnatIpType); //add param for ANAT
	void SipPartyReInviteResponse(DWORD status, BYTE bRemovedAudio, BYTE bRemovedVideo, CSipChanDifArr* pChanDifArr);
	void SipPartyOriginalRemoteCaps(CSipCaps* pRemoteCaps);
	void SipPartyReceivedAck(DWORD status, DWORD isSdp, BYTE bRemovedAudio, BYTE bRemovedVideo, CSipChanDifArr* pChanDifArr);
	void SipPartyBadStatus(DWORD opcode, int len, BYTE* strDescription);
	void SipPartyReceivedReInvite(CSipChanDifArr* pChanDifArr, BYTE isReInviteWithSdp, BYTE isRejectReInvite);
	void SipPartyConnectTout();
	void SipPartyDisconnectTout();
	void DnsResolvingTout();
	void SipTransportError(DWORD expectedReq);
	void SipPartyDnsResAck(DWORD status);
	void SipPartyUnMuteClosingChannel(DWORD chanType);
	void SendBFCPMessageInd(APIS32 opCode, APIU32 Status, BYTE mcuId, BYTE terminalId);
	void SendBFCPTransportInd(APIU32 Status);
	void TipNegotiationResult(DWORD status, WORD numOfStreams, BYTE bIsAudioAux, BYTE bIsVideoAux, DWORD doVideoReInvite);
	void TipContentMessageInd(APIS32 opCode, APIU32 Status, BYTE mcuId, BYTE terminalId);
	void TipLastAckReceived();

	void StartH230CommandSeq();
	// API for audio brdg
	void AudioValidation(WORD onOff);
	void AudioActive(WORD onOff, WORD srcRequest, DWORD mediaMask);
	void SendMMStoParty(WORD onOff);
	void SendMCCtoParty(WORD onOff);

	// API for content token
	void SendContentMessage(OPCODE subOpcode, BYTE mcuNum, BYTE terminalNum, BYTE randomNum = 0, const BYTE isSpeakerChange = FALSE);
	void SendContentTokenRoleProviderIdentity(const BYTE mcuNum,
	                                          const BYTE terminalNum, const BYTE label, const BYTE dataSize, const BYTE* pData);
	void SendContentTokenMediaProducerStatus(const BYTE channelId, const BYTE status);
	void SendContentTokenNoRoleProvider(const BYTE mcuNum, const BYTE terminalNum);

	// API for AMSC rate change
	void SendPresentationRateChange(DWORD newRate, BYTE bIsSpeaker);
	void MfaUpdatedPresentationOutStream();

	void SendH239ContentVideoMode(CContentMode* pContentMode);
	void SendH239LogicalChannelInactive(BYTE controlID);
	void SendH239MCS(BYTE controlID);
	void SendH239FlowControlReleaseRes(const BYTE isAck, const WORD bitRate);

	// API for freeze picture and video refresh
	void SendContentVideoRefresh(WORD ignore_filtering = FALSE);
	void SendEPCContentFreezePicture();
	void CGSendContent();   // for Call Generator
	void CGStopContent();   // for Call Generator

	// API for mlp brdg
	void IdentifyNetChannel(CIsdnNetSetup* pNetSetUp);

	// API for mux control
	void TimeOutMuxEndH320Connect(CCapH320& rmtCap, CComMode& currentScm, WORD reasonTimeOut);
	void MuxEndH320Connect(CCapH320& rmtCap, CComMode currentScm, WORD status, BYTE isEncryptionSetupDone = TRUE);
	void MuXEndSetXmitComMode(WORD status);
	void MuxRmtCap(CCapH320& rmtCap, WORD status);
	void MuxRmtH230(CSegment* h221Str);
	void MuxSync(WORD localRemote, WORD lostRegain, WORD chnlNum = 0);
	void MuxSyncTimer();
	void MuxRmtXfrMode(CComMode& rmtxfr);
	// Eitan - new API MUXCNTL --> PARTY indicate to conf level that remote sent all its caps
	void MuxReceivedPartyFullCapSet(CCapH320& rmtCap, CComMode& currentRmtScm);

	void PartyVideoFreeze();
	void ChangeMode(CComMode* pComMode);
	void ExchangeCap(CCapH320* pNewLocalCap);
	void UpdateLocalCaps(CCapH320* pNewLocalCap);

	// API for bonding control
	void BondingEndNegotiation(BYTE need_reallocate, DWORD number_of_channels_to_reallocate);
	void BondAligned();
	void BondingFailed(DWORD cause);

	void MuxSyncInitChnl(CCapH320* pRmtCap);

	void SendAddedProtocolToConfLevel(WORD NoAddedProtocols, WORD ProtocolCapEnum);

	// API for content token
	void SendTokenMessageToConfLevel(DWORD subOpcode, BYTE MCUNumber, BYTE terminalNumber, BYTE label, BYTE randomNumber = 0,
	                                 DWORD size = 0, CSegment* pParam = NULL);
	void SendTokenMessageToCallGenerator(DWORD subOpcode, BYTE isAck);    // for Call Generator
	void SendEndChangeContentToConfLevel(const CComModeH323* pCurrentMode, int status);
	void SendContentOnOffAck(DWORD status, eContentState eTempContentInState);
	void SendContentEvacuateAck(DWORD status);
	void UpdatePresentationOutStream();
	void UpdatePartyCapabilitiesAndAudioRate(BYTE audioRate, CCapH323* pLocalCapH323 = NULL);
	void StopPartyIVR(DWORD restartIVR);

	void OnRsrcAllocatorReAllocateRTMAck(CIsdnPartyRsrcDesc* desc, BYTE bAllocationFailed);
	void OnRsrcAllocatorUpdateRTMChannelAck(DWORD monitor_conf_id, DWORD monitor_party_id, DWORD connection_id, DWORD status, DWORD channelNum, CIsdnNetSetup* pNetSetup);
	void OnRsrcAllocatorBoardFullAck(CIsdnPartyRsrcDesc* desc, BYTE bAllocationFailed);
	void OnSmartRecovery();
	void VideoReadyForSlide();

	// H239 Cascade
	void SendContentRateChangeDone();
	void NotifyConfOnMasterActionsRegardingContent(DWORD wOpcode, DWORD rate);

	// LPR
	void UpdatePeopleLprRate(DWORD newPeopleRate, cmCapDirection channelDirection
	                         , DWORD lossProtection, DWORD mtbf, DWORD congestionCeiling, DWORD fill, DWORD modeTimeout, DWORD totalVideoRate, DWORD bLprOnContent, DWORD newContentRate);
	void SetLprModeForVideoOutChannels(WORD status, DWORD lossProtection, DWORD mtbf, DWORD congestionCeiling, DWORD fill, DWORD modeTimeout);
	void SendFlowControlAndLprToCard(DWORD newVidRate, BYTE outChannel, lPRModeChangeParams* pLprChangeModeParams);
	void UpdateLprDB(WORD updType, DWORD param, WORD externFlag);
	void UpdateChannelLprHeaderDB(BYTE ChannelWithLprHeader);
	void SendContentFlowControlToConfLevel(DWORD newContentRate, DWORD isDba);


	// Gateway Since V7.0 used also for Conf DTMF FWD
	void SetDTMFForwarding(BYTE bForwardDtmfs, WORD opcode);
	void SendDTMFForwarding(const char* dtmfString, WORD opcode);

	// PCM
	void PartyConnectedToPCM();
	void PartyDisconnectedFromPCM();
	void ForwardDtmfForPCM(char* dtmf);

	// Party preview
	void SendPartyStartPreviewReq(CPartyPreviewDrv* pPartyPreviewDrv);
	void SendPartyStopPreviewReq(WORD Direction);
	void SendPartyIntraPreviewReq(WORD Direction);

	// ICE
	void UpdatePartyOnMakeAnswerInd(WORD status, int rate);
	void UpdatePartyOnMakeOfferInd(WORD status);
	void UpdatePartyOnReinviteDataInd(DWORD status);
	void UpdatePartyOnProcessAnswerInd(DWORD status);
	void UpdatePartyOnModifyOfferInd(DWORD Status);
	void UpdatePartyOnModifyAnswerInd(DWORD Status, int rate);
	void UpdatePartyIceConnectivityCheckComplete(CSegment* pParam);
	void UpdateTransIceConnectivityCheckComplete(CSegment* pParam);
	void UpdatePartyOnCloseIceSession(DWORD status);
	void UpdateNoResourcesForVideoParty(BYTE bNoResourcesForVideoParty);

	void UpdatePartyOnIceBandwidthEventInd(DWORD videoRate);

	// RTCP
	void UpdatePartyOnVideoPreference(DWORD Width, DWORD Height, DWORD BitRate, DWORD FrameRate);
	void VideoBridgeUpdatedWithNewResolution(APIU32 Status);
	void SendVideoPreferenceToSipParty();
	
	void SendVsrMsgIndToSipParty(ST_VSR_SINGLE_STREAM* vsr);
	void SendMultipeVsrToSipParty(ST_VSR_MUTILPLE_STREAMS* multipleVsr);
	void SendMsftOutSlaveCreatedToSipParty(CSegment* pParam);
	void SendSingleVsrMsgIndToSipParty(CSegment* pParam);
	void SendMsSlaveVidRefreshToSipParty(CSegment* pParam);
	void SendMsSlaveVideoInSyncedToSipParty(CSegment* pParam);

	void ReqConfNIDConfirmationSIP(const char* numericConfId);  // IvrProviderEQ
	void SendMediaDisconnectionByCAMToParty(CSegment* pParam); // External IVR - disconnection on cap negotioation scenario
	void SendMediaConnectedByCAMToParty(CSegment* pParam); // External IVR - connection notification to allow the ivr cntl to verify it can begin

	// send mute video
	void PartySendMuteVideo(BYTE isActive);

	// TIP
	void TipNegotiationCompleted(DWORD status, WORD numOfStreams, BYTE bIsAudioAux, BYTE bIsVideoAux);

	// TIP slave
	void SipConfEstablishSlaveCall(CRsrcParams* pMrmpRsrcParams, CRsrcParams* pCsRsrcParams,
	                               UdpAddresses sUdpAddressesParams, CSipNetSetup* pNetSetup,
	                               CSipCaps* pLocalCaps, CIpComMode* pInitialMode,
	                               BYTE bIsAdvancedVideoFeatures, const char* strConfParamInfo,
	                               BYTE eTransportType, WORD McuNumber, WORD TerminalNumber,
	                               BYTE bNoVideRsrcForVideoParty, WORD room_Id, DWORD tipPartyType);
	void SendMessageFromMasterToSlave(WORD destTipPartyType, DWORD opcode, CSegment* pMsg);
	void SendMessageFromSlaveToMaster(WORD srcTipPartyType, DWORD opcode, CSegment* pMsg);
	void PartyCntlToPartyMessageMasterToSlave(CSegment* pParam);
	void PartyCntlToPartyMessageSlaveToMaster(CSegment* pParam);
	void SendNewNameWithRoomId(char* newName, const char* confName);
	void SetSlavePartyRsrcId(DWORD tipPartyType, DWORD peerRsrcId);
	void SlaveEndChangeMode();
	void SipBandwidthAllocationStatus(DWORD reqBandwidth, DWORD allocBandwidth);
	void SipBandwidthReInviteNeeded();
	void ICEInsufficientBandwidthEvent();

	// BFCP
	void BfcpStartReestablishConnection();
	void BfcpEndReestablishConnection();

	void SendPartyInviteResultInd(eGeneralDisconnectionCause disconnectionCause,
	                              BOOL bRedial, BOOL bIsGwInvite);

	//LYNC2013_FEC_RED:
	void UpdateRateWithFECorRED(CSegment* seg);
	void VideoOutChannelsUpdatedForFecOrRed(WORD status, DWORD type);
	void SendSingleFecOrRedMsgToSipParty(DWORD mediaType,DWORD newFecRedPercent);

	void AskRelayEndPointForIntra(CSegment* pSeg);
	void HandleMrmpRtcpFirInd(CSegment* pParam);

	// FOR SCP
	void SipPartyScpRequestFromEP(CSegment* pParam);
	void SendAckForScpReq(unsigned int aSequenceNumber);
	void SipPartyScpNotificationIndFromEP(CSegment* pParam);
	void SendAckForScpNotificationInd(CSegment* pParam);
	void SipPartyScpNotificationAckFromEP(APIU32 channelHandle, APIU32 remoteSeqNumber, APIUBOOL bIsAck);
	void SendScpNotificationToMrmpReq(CScpNotificationWrapper* apNotifyStruct);
	void SendScpPipesMappingNotificationToMrmpReq(CScpPipeMappingNotification* pPipesMapNotifyStruct);
	void UpdateScmStreams(CIpComMode* pInitialMode);
	void SendScpIvrStateNotificationReqToParty(eIvrState ivrState, DWORD localSeqNumber);
	void SendConfEntryPassword(const char* pwd);
	void SendLocalAuthStatus(BYTE  status);
	void UpdateArtOnTranslateVideoSSRC(DWORD aSsrc);
	void IPPartyInternalArtsConnected();
	void IPPartyInternalArtsDisconnected();
//        void IPPartyInternalVideoArtDisconnected();
	void StartAvcToSvcArtTranslator(DWORD ssrc, WORD roomId, CRsrcParams &artTranslatorRsrcParams, CRsrcParams &mrmpArtTranslatorRsrcParams);
	void DisconnectAvcToSvcArtTranslator();
    void DisconnectAvcToSvcArtTranslator(CIpComMode* pInitialMode);

	//AT&T
	void SendDialogStart(DialogState& state);
	void ReceivedAckOnIvrPlayMessageReq();
	void ReceivedAckOnIvrPlayRecordReq(); //IVR for TIP
	void ReceivedAckOnIvrShowSlideReq();
	void SendEndVideoUpgradeToMix();

	void SipPartyDtlsStatus(APIU32 status);
	void SipPartyTipEarlyPacketInd();
	void NotifySipPendingTransaction(EPendingTransType ePendTrans);  //BRIDGE-15745
	void SipPartyDtlsChannelsDisconnected(CSipComMode* pCurrentMode);
	void MrmpStreamIsMustReq(CSegment* pSeg);
	void MrmpStreamIsMustAck();	
	/*  MSSlave-Flora Yao-2013/10/28- MSSlave Flora Comment: */
	void SipConfEstablishMSSlaveCall(CRsrcParams* pMrmpRsrcParams, CRsrcParams* pCsRsrcParams,
                                          UdpAddresses sUdpAddressesParams, CSipNetSetup* pNetSetup,
                                          CSipCaps* pLocalCaps, CIpComMode* pInitialMode,
                                          BYTE bIsAdvancedVideoFeatures, const char* strConfParamInfo,
                                          BYTE eTransportType, WORD McuNumber, WORD TerminalNumber,
                                          BYTE bNoVideRsrcForVideoParty, WORD room_Id, eAvMcuLinkType AvMcuLinkType,
                                          DWORD msSlavePartyIdx,CSipCaps* pRemoteCaps,
                                          CSipCaps* m_MaxLocalCaps/*,CIpComMode* m_pTargetModeMaxAllocation*/);

	void ActiveMediaForAvMcuLync();

	void SendRecordingControlInfoToParty(CSegment* pParam);
	void  SendRecordingControlStatus(BYTE status);
	void  SendLayoutControlLocal(BYTE layout);

	void SendWebRtcPartyEstablishCallIdle();
	void SendWebRtcConnectFailure();
	void SendWebRtcConnectTout();

	/*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
       void SendByeToParty(PartyRsrcID PartyId) ;
	/*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

protected:
	// Attributes

	// Operations
};

#endif /* _PARTYAPI */
