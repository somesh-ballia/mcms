#ifndef CONF_API_H__
#define CONF_API_H__

#include "PObject.h"
#include "TaskApi.h"
#include "CommRes.h"
#include "ConfPartyDefines.h"
#include "LPRData.h"
#include "IvrApiStructures.h"
#include "IpChannelParams.h"
#include "PartyPreviewDrv.h"
#include "COP_video_mode.h"
#include "ICEParams.h"
#include "RvCommonDefs.h"
#include "RelayIntraData.h"
#include "ScpHandler.h"
#include "ConfPartyOpcodes.h"

const WORD AUDIOSRC                          = 1;
const WORD LSDSRC                            = 2;
const WORD HSDSRC                            = 3;
const WORD CHAIRSRC                          = 4;
const WORD PARTYSTATE                        = 5;
const WORD NETCHNL                           = 6;
const WORD H221                              = 7;
const WORD DISCAUSE                          = 8;
const WORD DISQ931                           = 9;
// Mute (not block) message
const WORD MUTE_STATE                        = 10;
const WORD LOOPSTATE                         = 11;
const WORD AUDCON                            = 12;
const WORD VIDCON                            = 13;
const WORD LSDCON                            = 14;
const WORD HSDCON                            = 15;
const WORD VIDMOD                            = 16;
const WORD RMOTCAP                           = 17;
const WORD CURCOMMODE                        = 18;
const WORD REMOTECOMMODE                     = 19;
const WORD CPCONFLAYOUT                      = 20;
const WORD CPPARTYLAYOUT                     = 21;
const WORD T120CON                           = 22;
const WORD NUMRETRY                          = 23;
const WORD RSRCDETAILS                       = 24;
const WORD NETRSRCDETAILS                    = 25;
const WORD MONITORPHONENUM                   = 26;
const WORD MONITORPHONENUM_T1CAS             = 27;
const WORD CHAIROWNER                        = 28;
const WORD CASCADESTATUS                     = 29;
const WORD PARTYAVSTATUS                     = 30;
const WORD CONFCASCADEMODE                   = 31;
const WORD PARTYCASCADEINFO                  = 32;
const WORD MASTERSRC                         = 33;
const WORD RMOT323CAP                        = 34;
const WORD RMOT323COMMODE                    = 35;
const WORD LOCAL323COMMODE                   = 36;
const WORD RECEIVEBAUDRATE                   = 37;
const WORD TRANSMITBAUDRATE                  = 38;
const WORD SNMPCONNECTIONSTATS               = 39;
const WORD PARTYDOWNSPEEDSTATUS              = 40;
const WORD SETPARTYCHNLNUM                   = 41;
const WORD SETBONDINGMODE                    = 42;
const WORD SNMPCHANNELSTATS                  = 43;
const WORD SETISLEADER                       = 44;
const WORD OPERASSISTTOCONF                  = 45;
const WORD STARTVOTE                         = 46;
const WORD NEWVOTE                           = 47;
const WORD STOPVOTE                          = 48;
const WORD VOTE                              = 49;
const WORD CANCELVOTE                        = 50;
const WORD ADD_VOTEPARTY                     = 51;
const WORD REMOVE_VOTEPARTY                  = 52;
const WORD NOISE_DETECTION                   = 53;
const WORD TONE_REMOVAL                      = 54;
const WORD STOP_PLAYING_MUSIC                = 55;
const WORD CONTENTCON                        = 56;
const WORD SIPPRIVATEEXTENSION               = 57;
// Invite opcodes
const WORD PREPARE_TO_INVITE                 = 60;
const WORD CLEAR_FROM_INVITE                 = 61;
const WORD MOVE_PARTY_TO_GROUP               = 62;
const WORD WAIT_PARTY_IN_GROUP               = 63;
const WORD PLAY_TONE                         = 64;
const WORD PARTY_PASSED_IVR                  = 65;
const WORD CONNECT_DIAL_OUT                  = 66;
const WORD ENDPLAYMESSAGEONCE                = 67;
// Block (not mute) message
const WORD BLOCK_STATE                       = 68;
// Private layout
const WORD PRIVATEON                         = 70;
const WORD CONTENTPROVIDER                   = 71;
const WORD CONTENTSRC                        = 72;
// Party status
const WORD PARTYSTATUS                       = 80;
// Secondary cause
const WORD SECONDARYCAUSE                    = 81;
const WORD REMOTEIP                          = 82;
// Lsd status
const WORD IPFECCSTATUS                      = 83;
// Update visual party name
const WORD UPDATEVISUALNAME                  = 84;
const WORD NOAUDIOSRC                        = 85;
// observerUpdate - CONTACT_INFO event
const WORD CONTACT_INFO                      = 86;
// Lpr indications
const WORD LPR_SYNC                          = 87;
const WORD UPDATE_EXCLUSIVE_CONTENT          = 88; // Restricted content
const WORD PARTY_REQUEST_TO_SPEAK            = 89;
const WORD PARTY_INTRA_SUPPRESS              = 90;
const WORD EVENT_MODE_LEVEL                  = 91;
// TelePresence indications
const WORD PARTYTELEPRESENCEMODE             = 92;
const WORD CHANNELSWITHLPRPAYLOAD            = 93;
// High Profile
const WORD PARTYHIGHPROFILE                  = 94;
// TIP Main profile
const WORD PARTYMAINPROFILE                  = 95;
const WORD UPDATE_EXCLUSIVE_CONTENT_MODE     = 97;  // Restricted content
const WORD UPDATE_MUTE_INCOMING_LECTURE_MODE = 96;
const WORD UPDATE_MUTE_ALL_AUDIO_EXCEPT_LEADER = 98;
// SoftMCU Media
const WORD MEDIA                             = 111;
const WORD MEDIA_REMOVE                      = 112;

// //~~~~~~~~~~~~~~~ invite opcodes ~~~~~~~~~~`
const WORD UPDATEDB                          = 5060;
const WORD UPDATECDR                         = 5061;
const WORD SETCAUSE                          = 5071;

// node types
const WORD TERMINAL                          = 0;
const WORD SLAVE                             = 2;
const WORD MASTER                            = 1;
const WORD NAGOTIATED                        = 3;
const WORD CONFLICT                          = 4;
const WORD MCU                               = 5;

#define EVENT_MODE_INTRA_SUPPRESS PARTY_INTRA_SUPPRESS   // for backward compatibility

////////////////////////////////////////////////////////////////////////////
enum LyncConnType
{
	No_Lync = 0,
	Lync_Addon,
	Lync,
	Lync2013,
	AvMcuLync2013Main,
	AvMcuLync2013Slave,
};

////////////////////////////////////////////////////////////////////////////
class CBridge;

class CParty;
class CCapH323;
class CComModeH323;
class CSecondaryParams;
class CH323NetSetup;
class CNetSetup;
class CIsdnNetSetup;
class CSipNetSetup;
class CSipCaps;
class CSipChanDifArr;
class CSipComMode;
class CComMode;
class CCapH320;
class CAutoScanOrder;

typedef CComModeH323 CIpComMode;

////////////////////////////////////////////////////////////////////////////
//                        CConfApi
////////////////////////////////////////////////////////////////////////////
class CConfApi : public CTaskApi
{
	CLASS_TYPE_1(CConfApi, CTaskApi)

public:
	CConfApi(DWORD ConfId = DEFAULT_CONF_ID);
	virtual ~CConfApi();

	// Initializations
	void Create(void (* entryPoint)(void*), COsQueue& creatorRcvMbx, DWORD confId, char* name);

	// Operations
	virtual const char* NameOf() const;

	void ForceKill();
	void UpdateDB(CTaskApp* pParty, WORD updType, DWORD param, WORD externFlag = 0, CSegment* pParamSeg = NULL);
	void UpdateChannelsLprHeader(CTaskApp* pParty, BYTE isChannelWithLprHeader);
	void ScpRequestFromEp(PartyRsrcID PartyId, CMrmpScpStreamsRequestStructWrap& aScpStreamReq);
	void ScpNotificationIndFromEp(PartyRsrcID PartyId, CScpNotificationWrapper& aScpStreamNotificationInd);
	void HandleTerminalEvent(WORD req, CSegment* Command = NULL);

	// Updating the CDR
	void UpdatePartyStateInCdr(const CTaskApp* pParty);
	void UpdateGkCallIdInCdr(const CTaskApp* pParty, BYTE* gkCallId);
	void UpdateNewRateInfoInCdr(const CTaskApp* pParty, DWORD currentRate);
	void UpdateCallInfoInCdr(const CTaskApp* pParty, DWORD maxBitRate, EFormat format, WORD maxFrameRate);
	void  UpdateSvcSipPartyInfoInCdr(const CTaskApp* pParty, const std::list<StreamDesc>* pStreams, ECodecSubType eAudioCodec, DWORD dwBitRateOut, DWORD dwBitRateIn);

	// api for mcumngr
	virtual void StartConf(const CCommRes& rsrv);
	virtual void ReconnectParty(const char* partyName, BYTE bUseDelay = 1);
	virtual void DropParty(const char* partyName, WORD mode = 0, WORD discCause = 0);

	void AddParty(const CRsrvParty& rsrvParty, DWORD undefId = 0, DWORD partyType = 0);
	void DropPartyViolent(const char* partyName, WORD mode = 0, WORD discCause = 0, DWORD taskId = 0);

	WORD ImportParty(void* pConfPartyCntl, void* pConfPartyDesc, void* pDestName, CSegment& rspMsg, DWORD tout, OPCODE& rspOpcode);
	void SetEndTime(const CStructTm& tm);
	void PartyDisConnect(WORD cause, CTaskApp* pParty, const char* alternativeAddrStr = NULL, DWORD MipErrorNumber = 0);
	void PartyChangeVidMode(CTaskApp* pParty, WORD isSetToSecondary);
	void InformConfOnPartyReCap(CTaskApp* pParty);

	// api for recording list
	void AddAndConnectRecordingLink(BYTE muteVideo = YES);
	void RejectAddRecordingLinkParty(DWORD rStatus);
	void DisconnectRecordingLink();
	void ActivateAvMcuAutoTerminationCheck();
	// api for party multipoint (add,del,changemode) controls
	void EndAddParty(CTaskApp* pParty, WORD status);
	void EndDelParty(CTaskApp* pParty, WORD status);
	void EndExportParty(PartyRsrcID PartyId, WORD status);//
	void EndImportParty(PartyRsrcID PartyId, WORD status);//

	WORD IsDestConfReadyForMove(CSegment& rspMsg, DWORD tout, OPCODE& rspOpcode);

	void RedailParty(CTaskApp* pParty, WORD IsHotBackupRedial = FALSE);

	void MoveParty(PartyMonitorID monitorPartyId, ConfMonitorID monitorDestConfId, EMoveType eMoveType = eMoveDefault);//

	WORD AddInH323Party(CH323NetSetup* pH323NetSetup, CTaskApp* pParty, COsQueue& partyRcvMbx, char* name, DWORD tout, CSegment& rspMsg);
	WORD AddInParty(CIsdnNetSetup* pNetSetup, CTaskApp* pParty, COsQueue& partyRcvMbx, char* name, DWORD tout, CSegment& rspMsg);

	void ConnectDialOutPartyByNumb(BYTE bNumbParty);
	void InviteParty(DWORD partyRsrcID, DWORD partyMonitorID, char* invitePartyDailingString, const map<WORD, WORD>& dailingOrder);
	void DisconnectInvitedParticipant(DWORD partyRsrcID);

	void InitiateFlowControlForConference(DWORD newVidRate);
	void InitiateFlowControlForParty(const CTaskApp* pParty);

	void PartyConnect(PartyRsrcID PartyId, WORD status);

	void PartyEndDisConnect(PartyRsrcID PartyId, WORD status);
	void PartyIncreaseDisconnctingTimer(PartyRsrcID PartyId);
	void PartyEndNetChnlDisConnect(PartyRsrcID PartyId, WORD status);

	void PartyEndChangeModeIp(PartyRsrcID PartyId, const CIpComMode& pCurrentScm, DWORD status, WORD reason = SECONDARY_CAUSE_DEFAULT, CSecondaryParams* pSecParamps = NULL);
	void SendPartyReceiveReCapsToPartyControl(PartyRsrcID PartyId, CCapH323* remoteCap);
	void SendPartyInConfIndToPartyCntl(PartyRsrcID PartyId);//
	void SendDisconnectBridgesToPartyControl(PartyRsrcID PartyId, WORD isDisconnectAudio, WORD isDisconnectVideo);//
	void SendConnectBridgesToPartyControl(PartyRsrcID PartyId, WORD isConnectAudio, WORD isConnectVideo,
											CIpComMode* pNewMode, CSipCaps* pRemoteCaps,
											unsigned int incomingVideoChannelHandle, unsigned int outgoingVideoChannelHandle);//

	void PartyExport(PartyRsrcID PartyId, WORD status);//

	void UpdateMessageOverlay(CMessageOverlayInfo* pMessageOverlayInfo);
	void UpdateSiteName(CSiteNameInfo* pSiteNameInfo);
	void RefreshLayout();
	void UpdateMessageOverlayParty(const char* SrcPartyName, CMessageOverlayInfo* pMessageOverlayInfo);

	// VNGR-26449 - unencrypted conference message
	void UpdateSecureMesaageToConnectedParty(const char* PartyName);
	void UpdateSecureMesaageToAllParties();
	void StopSecureMessageToParty(const char* PartyName);

	void PartyBridgeResponseMsg(const CTaskApp* pParty, OPCODE brdgOpcode, WORD responseOpcode, WORD status, BOOL isPartyCntlMsg, EMediaDirection eMediaDirection = eMediaInAndOut, CSegment* pParams = NULL);
	void PartyBridgeResponseMsgById(PartyRsrcID partyID, OPCODE brdgOpcode, WORD responseOpcode, WORD status, BOOL isPartyCntlMsg, EMediaDirection eMediaDirection = eMediaInAndOut, CSegment* pParams = NULL);	void SendResponseMsg(const CTaskApp* pParty, OPCODE brdgOpcode, WORD responseOpcode, WORD status, BOOL isPartyCntlMsg, CSegment* pParams = NULL);

	void SendLocalBrdgMsg(CSegment* msg, OPCODE brdgOpcode);
	// ----------------------------------
	// IVR & CAM API
	// ----------------------------------
	// The function starts play a message for a specific party
	// If the party currently is in other mode , then MSG mode
	// then go to the MSG mode .Elsewere ( if the party currently
	// in mode MSG , just change the message
	void StartMessage(PartyRsrcID PartyId, const char* msgFullPath, WORD msgDuration, WORD msgCheckSum, WORD wMessMode, WORD priority, WORD repeated_time_interval = 30);
	void StartMessageTipAux(PartyRsrcID PartyId, CSegment* pParam);
	// The message stops message playing , but leaves the party
	// in MSG mode . For passing the party in mode INCONF use
	void StopMessage(PartyRsrcID PartyId, DWORD mediaType = IVR_MEDIA_TYPE_AUDIO);

	void StartSequenceMessages(PartyRsrcID PartyId, const IVRMsgDescriptor* ArrayOfMessages, WORD SubMessagesNumber, WORD publicOrPrivate, WORD PlayMode = 1);

	void StartRecordMessage(PartyRsrcID PartyId, const IVRMsgDescriptor* ArrayOfMessages, WORD SubMessagesNumber, WORD cachePriority, bool isRecordOnly);
	void StartStopPLC(PartyRsrcID PartyId, WORD onOff);
	void NotifyCAM(PartyRsrcID PartyId, DWORD opcode, DWORD uniqueNumber);
	void NotifyCAMTimer(PartyRsrcID PartyId, DWORD opcode, DWORD featureType, DWORD uniqueNumber, DWORD originalOpcode);
	void PlcAction(PartyRsrcID PartyId, DWORD opcode, DWORD layoutCode, BYTE cellNumber = 0);
	void StartStopVenus(PartyRsrcID PartyId, WORD onOff);
	void VenusAction(PartyRsrcID PartyId, DWORD opcode, DWORD layoutCode, BYTE cellNumber = 0);
	void StopRollCallRecording(PartyRsrcID PartyId, DWORD status);
	void StopRollCallRecordingAckTimer(PartyRsrcID PartyId);

	void LayoutChangedNotification(const char* pPartyName);

	void IvrPartyNotification(
		DWORD partyID,
		CTaskApp* pParty,
		const char* pPartyName,
		OPCODE notifyOpcode,
		CSegment* pSeg,
		EMediaDirection eMediaDirection = eMediaInAndOut,
		const char* externalIvrFile = NULL);

	void IvrConfNotification(OPCODE notifyOpcode, CSegment* pParam);

	void UpdatePartyIvrStatus(PartyRsrcID PartyId, WORD status);

	// for IVR
	void MuteAllButX(DWORD PartyId, BYTE yesNo);
	void LockConference(BYTE yesNo);
	void SecureConf(CTaskApp* pParty, BYTE SecureFlag);
	void ShowParticipants();
	void ShowParticipantsToConf(DWORD partyRsrcID, DWORD permission, DWORD actionSrc);
	void ShowGathering(DWORD partyRsrcID, DWORD permission, DWORD actionSrc);

	// Roll Call
	void EnableRollCall(BYTE byteEnable);

	void PartyPassedIVR_EntranceProcedures(const char* szPartyName);

	void UpdatePartyNoiseDetection(PartyRsrcID PartyId, WORD noiseDetection, WORD noiseDetectionThreshold, WORD updateAudioDsp);

	void SendCamIvrCntlInd();

	// flow control
	void UpdatePartyControlOnNewRate(PartyRsrcID PartyId, DWORD newBitRate, WORD channelDir, WORD roleLabel);

	// Cop
	void CopVideoInChangeMode(PartyRsrcID PartyId, BYTE changeModeParam, ECopChangeModeType changeModeType = eCop_DecoderParams);
	void CopVideoOutChangeMode(PartyRsrcID PartyId, BYTE encoderIndex);
	void CopCascadeLinkLectureMode(PartyRsrcID PartyId, BYTE isActive);
	void CopStartCascadeLinkAsLecturerPendingMode(PartyRsrcID PartyId);

	// api for video and audio bridge control
	void MuteMedia(const char* partyName, const EOnOff onOff, DWORD mediaMask);
	void BlockMedia(const char* partyName, const EOnOff onOff, DWORD mediaMask);
	void SendBrdgPartyCntlMsg(const CTaskApp* pParty, OPCODE brdgOpcode, WORD opCode, WORD status, EMediaDirection eMediaDirection = eMediaInAndOut, CSegment* pParams = NULL);
	void SendBrdgPartyCntlMsgById(PartyRsrcID partyID, OPCODE brdgOpcode, WORD opCode, WORD status, EMediaDirection eMediaDirection = eMediaInAndOut, CSegment* pParams = NULL);

	// api for audio bridge control
	void EndAudBrdgConnect(WORD status);
	void EndAudBrdgDisConnect(WORD status);
	void AudioMute(PartyRsrcID PartyId, WORD onOff);
	void SetAudioVolume(const char* partyName, BYTE volume, EMediaDirection bVolumeInOut);
	void UpdateAGCExecFlag(const char* partyName, const WORD onOff);
	void ReSendOpenConf(WORD new_card_board_id);
	void StartLookForActiveSpeaker();

	// api for video bridge control
	void PartyVideoInSynced(const CTaskApp* pParty, OPCODE brdgOpced, WORD status);
	void PartyVideoBridgeImageInCellZeroChanged(OPCODE brdgOpcode, const CTaskApp* pParty, const CTaskApp* pSrcParty);

	void EndVidBrdgConnect(WORD status);
	void EndVidBrdgDisConnect(WORD status);
	void AudioSpeakerChanged(CTaskApp* pParty);
	void SpeakersChanged(CTaskApp* pPartyVideo, CTaskApp* pPartyAudio, DWORD partyDominantpeakerMSI);
	void VideoMute(PartyRsrcID PartyId, WORD onOff);
	void VideoRefresh(PartyRsrcID PartyId);
	void NotifyVbOnNetworkQualityChange(const CTaskApp* pParty, eRtcpPacketLossStatus networkStatePerCell, eRtcpPacketLossStatus networkStatePerLayout);
	void EventModeIntraPreviewReq(PartyRsrcID PartyId);
	void UpdateLectureMode(CLectureModeParams* pLectureMode);
	void UpdateContentLectureMode(BOOL yesNo);

	void SetVideoConfLayoutSeeMeAll(CVideoLayout& layout, BYTE bAnyway = 0);
	void SetVideoConfLayoutSeeMeParty(const char* SrcPartyName, CVideoLayout& layout);
	void UpdateAutoLayout(BYTE onOff);
	void SetVideoPrivateLayout(const char* SrcPartyName, CVideoLayout& layout, BOOL isMcmsAction = FALSE);
	void SetVideoPrivateLayoutButtonOnly(const char* SrcPartyName, WORD isPrivate);
	void UpdateVideoClarity(BYTE onOff);
	void SetAutoRedial(const BYTE bAutoRedial);
	void SetAutoScanInterval(WORD intervalValue);
	void SetAutoScanOrder(CAutoScanOrder* pAutoScanOrder);
	// LEGACY
	void ContenetDecoderRsrcAllocFailure(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status);
	void ContentDecoderResetFailStatus(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status);
	void ContentDecoderVideoInSynced(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status);
	void ContenetDecoderSyncLost(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status);
	void ContentDecoderAllocFail();
	void ContentDecoderResetFailStatus();
	void ContentDecoderDisconnected(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status);

	// api for party FECC bridge control
	void EndFECCBrdgConnect(WORD status);
	void EndFECCBrdgDisConnect(WORD status);
	void DataTokenRequest(CTaskApp* pParty, WORD bitRate, WORD isCameraControl = 1);
  	void  EndXCodeBrdgConnect(WORD status);
  	void  EndXCodeBrdgDisConnect(WORD status);
	void DataTokenRequestAccept(CTaskApp* pParty);
	void DataTokenRequestReject(CTaskApp* pParty);
	void DataTokenRelease(CTaskApp* pParty, WORD isCameraControl = 0);
	void DataTokenReleaseAndFree(CTaskApp* pParty, WORD isCameraControl = 0);
	void DataTokenWithdraw(CTaskApp* pParty);
	void DataTokenWithdrawAndFree();

	void FeccKeyMsg(DWORD partyRsrcId, WORD msg);

	// Faulty MFA
	void SendFaultyMfaNoticeToPartyCntl(PartyRsrcID PartyId, DWORD reason, BYTE mipHwConn = 0, BYTE mipMedia = 0, BYTE mipDirect = 0, BYTE mipTimerStat = 0, BYTE mipAction = 0);

	// api for H323 party app (task)
	void H323PartyConnect(PartyRsrcID PartyId, WORD status, BYTE bIsCascade, int MSDStatus, unsigned int channelHandle, CCapH323* pRemoteCap = NULL,
	                      CComModeH323* pCurrentScm = NULL, BYTE bLateReleaseResourcesInConfCntl = FALSE);
	void H323PartyConnectAll(PartyRsrcID PartyId, CComModeH323* pCurrentScm, DWORD videoRate, CCapH323* remoteCap, BYTE isCodianVcr, unsigned int channelHandle);

	void AddProtocolToH323Party(PartyRsrcID PartyId, CSegment* pParam);
	void RemoveProtocolFromH323Party(PartyRsrcID PartyId, CSegment* pParam);
	void UpdateLocalCapsInConfLevel(PartyRsrcID PartyId, CCapH323& localCaps);

	void SendSiteAndVisualNamePlusProductIdToPartyControl(const CTaskApp* pParty, BYTE bIsVisualName, DWORD len, char* siteName,
														  BYTE bIsProductId, DWORD len2, char* productId, BYTE bIsVersionId, DWORD len3, char* VersionId,
														  eTelePresencePartyType eLocalTelePresencePartyType,
														  BYTE isRemoteCopMCU=FALSE, RemoteIdent eRemoteVendorIdent=Regular);
	void SiteAndVisualName(PartyMonitorID PartyId, char* siteName);

	void PartyMoveToSecondery(PartyRsrcID PartyId, WORD reason, CSecondaryParams* pSecParamps = NULL);
	void SetPartySecondaryCause(PartyRsrcID PartyId, WORD cause, CSecondaryParams& secParams);
	void SendCloseChannelToConfLevel(PartyRsrcID PartyId, WORD type, WORD direction, WORD roleLabel);

	// Multiple links for ITP in cascaded conference feature:
	void UpdateMainPartyOnITPSpeaker(PartyRsrcID PartyId, DWORD numOfActiveLinks, eTelePresencePartyType itpType);
	void UpdateMainPartyOnITPSpeakerAck(PartyRsrcID PartyId);

	// Ip
	void IpMuteMedia(PartyRsrcID PartyId, BYTE audioInMute, BYTE audioOutMute, BYTE VideoInMute, BYTE VideoOutMute, BYTE ContentInMute, BYTE ContentOutMute, BYTE FeccInMute, BYTE FeccOutMute);
	void IpUpdateBridges(PartyRsrcID PartyId, CIpComMode* pIpScm, cmCapDataType mediaType = cmCapEmpty, cmCapDirection mediaDirection = cmCapReceiveAndTransmit);

	// Sip
	void SipPartyChannelsConnected(PartyRsrcID PartyId, CIpComMode* pCurrentScm, unsigned int* channelHandle);
	void SipPartyRemoteConnected(PartyRsrcID PartyId, CIpComMode* pCurrentScm, BYTE bIsEndConfChangeMode);
	void SipPartySendChannelHandle(PartyRsrcID PartyId, unsigned int incomingVideoChannelHandle, unsigned int outgoingVideoChannelHandle);
	void SipPartySendFallBackToSIP(PartyRsrcID PartyId, CIpComMode* pCurrentScm);

	WORD AddInSipParty( CSipNetSetup* pNetSetup, CSipCaps* pRemoteCaps, CTaskApp* pParty, COsQueue& partyRcvMbx, char* name, DWORD tout,
						CSegment& rspMsg, BYTE IsOfferer, BYTE bIsMrcHeader, BYTE bIsWebRtcCall, LyncConnType lyncEpType, RemoteIdent epType, eIsUseOperationPointsPreset isUseOperationPointesPresets, BOOL bIsRemoteSlave, BYTE  initialLayout = 0);

	void SipPartyRemoteCapsRecieved(PartyRsrcID PartyId, CSipCaps* pRemoteCaps, CIpComMode* pCurrentScm);

	// API for ECS - H323:
	void SendPartyReceiveEcsToPartyControl(PartyRsrcID PartyId);

	// SIP Re cap
	void SendPartyReceiveReCapsToPartyControl(PartyRsrcID PartyId, CSipCaps* pCaps, CIpComMode* pBestMode,WORD bisFallBack = FALSE);
	void UpdatePartyControlOnRemoteCaps(PartyRsrcID PartyId,CSipCaps* pCaps);

	void ReleaseReservedPartyFromLobby(DWORD listId, DWORD dwPartyId);
	void EndOfIVRSession(PartyRsrcID PartyId, BYTE isLeader);
	void SendCAMGeneralActionCommand(PartyRsrcID PartyId, DWORD confOrPartyAction, DWORD opcode, DWORD param);
	void SendCAMAddFeatureToList(PartyRsrcID PartyId, DWORD confOrPartyAction, DWORD opcode, DWORD param);//AT&T
	void SendCAMRemoveFeatureFromList(PartyRsrcID PartyId, DWORD confOrPartyAction, DWORD opcode, DWORD param);//AT&T
	void SendCAMGeneralActionCommandSeg(PartyRsrcID PartyId, DWORD confOrPartyAction, DWORD opcode, CSegment* pSeg);
	void SendCAMGeneralNotifyCommand(PartyRsrcID PartyId, DWORD opcode, CSegment* pParam);
	void SendCAMGeneralNotifyCommand(PartyRsrcID PartyId, DWORD opcode, DWORD param);
	void SetPartyAsLeader(const char* pPartyName, EOnOff eOnOff);
	void AddPartyFeatureToWaitList(PartyRsrcID PartyId, DWORD uniqueNumber, DWORD opcode);

	void SendStartFeature(PartyRsrcID PartyId, DWORD confOrPartyAction, DWORD opcode);
	void SendStopFeature(PartyRsrcID PartyId, DWORD confOrPartyAction, DWORD opcode);
	void SendDtmfFromParty(PartyRsrcID PartyId, const char* dtmfString);
	void SendDtmfFromClient(CSegment* pParam, const char* partyName);   // for Call Generator

	void SendIpMonitoringEventToParty(DWORD partyID, const char* pPartyName);

	// API FOR PCM
	void ChairpersonEnteredConf(PartyRsrcID PartyId, const COsQueue& pPartyMbx);
	void ChairpersonLeftConf(PartyRsrcID PartyId);
	void ChairpersonStartMoveFromConf(PartyRsrcID PartyId);
	void PCMPartyNotification(PartyRsrcID PartyId, OPCODE notifyOpcode, CSegment* pParam = NULL);
	void ForwardDtmfToPCM(PartyRsrcID PartyId, CSegment* pParam);
	void PCMClickAndView(PartyRsrcID PartyId);

	// API FROM PCM
	void PCMInviteParty(const char* dialString, int networkType);

	// PCM --> PartyCntl
	void InformPartyControlOnPcmState(PartyRsrcID PartyId, BYTE isConnected);

	// API FOR Content Bridge control
	void EndContentBrdgConnect(WORD status);
	void EndContentBrdgDisConnect(WORD status);
	void StartContent(BYTE currContentProtocol,BYTE isContentSnatching = NO);
	void StopContent();
	void LinkTryToConnect(CSegment* pParam);
	void LinkTryToDisconnect(CSegment* pParam);

	// API for Content party control
	void PartyContentBrdgConnect(PartyRsrcID partyID, OPCODE brdgOpcode, WORD status);
	void PartyContentBrdgDisConnect(PartyRsrcID partyID, OPCODE brdgOpcode, WORD status);

	void PartyContentRateChanged(const CTaskApp* pParty, const BYTE parameterID, const DWORD newContentRate);
	void MasterContentMessage(const DWORD opcode, const CTaskApp* pParty, BYTE mcuNUM, BYTE termNum, const DWORD newContentRate);

	void PartyContentBrdgEndSwitch(const CTaskApp* pParty, const CBridge* pBrdg, WORD status);
	void UpdatePartyH323VideoBitRate(PartyRsrcID PartyId, DWORD newBitRate, WORD channelDirection, WORD eRole); // rons
	void UpdateVideoBridgeFlowControlRate(PartyRsrcID PartyId, DWORD newBitRate, WORD channelDirection, WORD eRole, BYTE bIsCascade, CSegment* pLprParams);
	void UpdateRemoteUseSmartSwitchAccordingToVendor(const CTaskApp* pParty);
	void CleanBitRateLimitation(PartyRsrcID PartyId);

	// API for CONTENT
	void SendContentTokenMessage(DWORD subOpcode, CTaskApp* pParty, BYTE MCUNumber, BYTE terminalNumber, BYTE label, BYTE randomNum = 0);

	void ContentTokenRoleProviderMessage(CTaskApp* pParty, BYTE MCUNumber, BYTE terminalNumber, BYTE label, BYTE size, BYTE* pData);
	void SendBfcpContentTokenQueryMessage(CTaskApp* pParty, BYTE MCUNumber, BYTE terminalNumber, BYTE label, BYTE size, BYTE* pData);

	void ContentMediaProducerStatus(CTaskApp* pParty, BYTE channelID, BYTE status);
	void H239FlowControlReleaseReq(CTaskApp* pParty, BYTE channelId, WORD bitRate);
	void HWConetntOnOffAck(CTaskApp* pParty, eContentState ePState);
         void  ContentVideoRefresh(BYTE controlID,BYTE addIntraRequester = NO , const CTaskApp* pParty = NIL(CTaskApp),WORD destination = CONTENTCNTL_MSG);
	void ContentFreezePic();
	void ContentFreezePic(BYTE controlID);


	void NewContentTokenHolder(BYTE mcuNum, BYTE terminalNum);
	void ForwardContentTokenMsgToMaster(BYTE mcuNum, BYTE terminalNum, BYTE randomNum, OPCODE opcode);
	void NoContentTokenHolder();
	void ContentNoRoleProvider();

	void UpdatePresentationRes(PartyRsrcID PartyId);
	void UpdateNetChannel(PartyRsrcID PartyId, BYTE boardId, DWORD spanId, DWORD connectionID, WORD numChnl);

	void VideoFreeze(const CTaskApp* pParty);

	// H.320
	void PartyEndInitCom(PartyRsrcID PartyId);
	void EndInitCom(PartyRsrcID PartyId);
	void IsdnPartyConnect(PartyRsrcID PartyId, CCapH320& remoteCap, CComMode& currentScm, WORD status, BYTE isEncryptionSetupDone = TRUE);
	void PartyRemoteXferMode(PartyRsrcID PartyId, CComMode& remoteScm);

	void PartyRemoteCap(PartyRsrcID PartyId, CCapH320& remoteCap);
	void PartyReceivedFullCapSet(PartyRsrcID PartyId, CCapH320& remoteCap, CComMode& currentRmtScm);
	void AudioActive(PartyRsrcID PartyId, WORD onOff, WORD srcRequest);

	void AddPartyChannelDesc(CNetSetup* pNetSetup, char* name, WORD channel_num);
	void ReallocateBondingChannels(PartyRsrcID PartyId, DWORD number_of_channels);
	void ReallocateOnBoardFull(PartyRsrcID PartyId, DWORD conn_id);
	void OnSmartRecovery(PartyRsrcID PartyId);

	// API for call generator
	void SendCGStartContent(const char* partyName);
	void SendCGStopContent(const char* partyName);

	// ICE
	void UpdatePartyCntlOnICEParams(PartyRsrcID PartyId, BYTE IsICECall, CIceParams* IceParams);
	void SipPartySendFallbackFromIceToSip(PartyRsrcID PartyId);
	void SendFallbackFromIceToSipPartyToConf(const char* name);

	void SendDataCntlMsg(CTaskApp* pParty, WORD opCode, WORD isCameraControl = 0);

	void UpdateRecordingControl(WORD wRecordingControl, CTaskApp* pPartyToReply = NULL);

	// LPR
	void UpdatePartyLprVideoBitRate(PartyRsrcID PartyId, DWORD newPeopleRate, WORD channelDirection, DWORD lossProtection, DWORD mtbf, DWORD congestionCeiling, DWORD fill, DWORD modeTimeout, DWORD totalVideoRate, DWORD RemoteIdent = Regular);

	void SetDialStringForGW(const char* dialString);
	void SetDialOutServiceNameForGW(const char* isdnServiceName);
	void EndGWPartySetup(DWORD partyId, DWORD timeout, BYTE allowOverride, const char* msgStr);
	void VideoInSinched(CTaskApp* pParty);

	void SendVideoPreferencesToPartyControl(PartyRsrcID PartyId, DWORD Width, DWORD Height, DWORD FR, DWORD BitRate);
	void SendVideoUpdateAfterVsrMsgToPartyControl(PartyRsrcID PartyId,  CIpComMode* pNewMode);
	void SendLastTargetModeMsgToPartyControl(PartyRsrcID PartyId,  CIpComMode* pNewMode);

	void UpdatePartyControlRegardingChangeOfTxVideoCausedByFecOrRed(CIpComMode* pNewMode, PartyRsrcID PartyId, DWORD newPeopleRate, DWORD type=cmCapVideo, BYTE isVidoOutMuted=FALSE); //2=cmCapVideo
	void ContentTokenWithdraw(BYTE DummyIsImmediate = 0);

	// api for operator
	void SetVideoSrc(const char* partyName, const char* partyName_2 = NULL);
	void VoiceActivateVideo(const char* partyName = NULL);
	void SetVideoSrcCP(const char* partyName, const char* partyName_2, WORD sub_image, WORD reqSrc = OPERATOR, WORD localMsg = 1, BYTE confForceType = 0);
	void VoiceActivateVideoCP(const char* partyName, WORD sub_image, WORD reqSource = OPERATOR, BYTE localMsg = 1);
	void VoiceActivateConfForce(const char* partyName, WORD sub_image, WORD reqSource, BYTE localMsg = 1);
	void FreeTmpPhoneNumber(PartyRsrcID PartyId, char* partyName);
	void SetNodeType(PartyRsrcID PartyId, WORD nodeType);

	// H230 C&I
	void VideoActive(const CTaskApp* pParty, WORD onOff, WORD srcRequest, BYTE localMsg = 1);

	// api for chair application
	void ChairTokenRequest(const CTaskApp* pParty);
	void ChairTokenRelease(const CTaskApp* pParty);
	void ChairTokenRequestAccept(const CTaskApp* pParty);
	void ChairTokenRequestReject(const CTaskApp* pParty);
	void ChairDropTerminal(const CTaskApp* pParty, BYTE mcu, BYTE terminal);
	void TerminalPersonalIdentifier(const CTaskApp* pParty, BYTE mcu, BYTE terminal);
	void ChairDropConf(const CTaskApp* pParty);
	void PartyFloorRequest(const CTaskApp* pParty);
	void PartyIndicateEndOfString(const CTaskApp* pParty);
	void PartyInfoIndString(const CTaskApp* pParty, const char* identName);
	void ActiveTerminalUpdateListRequest(const CTaskApp* pParty);
	void ChairVideoBroadcast(const CTaskApp* pParty, BYTE mcu, BYTE terminal);
	void ChairCancelVideoBroadcast(const CTaskApp* pParty);
	void McuMultiCmdConf(const CTaskApp* pParty);
	void McuCancelMultiIndMaster(const CTaskApp* pParty);
	void McuMultiIndMaster(const CTaskApp* pParty);
	void RandomNumber(const CTaskApp* pParty, BYTE rand);
	void AssignTerminalNum(const CTaskApp* pParty, WORD Bridge, WORD mcu, WORD terminal);
	void IndicateTerminalNum(const CTaskApp* pParty, BYTE mcu, BYTE terminal);
	void IndicateDroppedTerminalNum(const CTaskApp* pParty, BYTE mcu, BYTE terminal);
	void IndicateVideoSrcNum(const CTaskApp* pParty, BYTE mcu, BYTE terminal);
	void VideoOnAirIndication(const CTaskApp* pParty);
	void CancelVideoOnAirIndication(const CTaskApp* pParty);
	void OperatorAssistance(DWORD partyId, BYTE action, BYTE mode, BYTE init_by);
	void ForwardMCVToChair(CTaskApp* pParty, WORD onOff);
	void TokenCommandAssociation(const CTaskApp* pParty);
	void SetMIHCap(const CTaskApp* pParty, WORD onOff);
	void SendVCSMessageToChair(const CTaskApp* pParty, BYTE mcu, BYTE terminal);
	void SendVCSCancelMessageToChair(const CTaskApp* pParty);

	void SendStartPreviewReqToParty(CPartyPreviewDrv* pPartyPreviewDrv);
	void SendStopPreviewReqToParty(const char* partyName, cmCapDirection VideoDirection);
	void SendRequestVideoPreviewIntra(const char* partyName, cmCapDirection VideoDirection);

	void VideoRefreshBeforeRecording(const char* partyName, WORD direction);

	// Operations
	void SetExclusiveContentOn(const char* partyName);     // Restricted content

	// mix mode
	void SetConfAvcSvcMode(eConfMediaState confState, ConfMonitorID confId);
	void SetPartyAvcSvcMode(eConfMediaState confState, ConfMonitorID confId, PartyMonitorID partyId);

	void RemoveExclusiveContent();     // Restricted content
	void AddSlaveParty(PartyRsrcID PartyId, DWORD tipPartyType);
	void SendAddSlavePartyToConf(const CRsrvParty& rsrvParty, DWORD undefId, DWORD tipPartyType, DWORD room_Id, DWORD pPartyRsrcID, DWORD masterVideoPartyType, CIpComMode* pMasterScm);
	void SendAddMsSlavePartyToConf(PartyRsrcID mainPartyRsrcId, const CRsrvParty& rsrvSlaveParty, eAvMcuLinkType AvMcuLinkType, DWORD msSlaveIndex, DWORD msSsrcRangeStart, CSipCaps* remoteCaps, CVidModeH323  *pLocalSdesCap = NULL);
	void SendMsSlaveToMainAck(PartyRsrcID mainPartyRsrcId, DWORD opcode, DWORD status, PartyRsrcID msSlavePartyRsrcId, eAvMcuLinkType AvMcuLinkType, DWORD msSlaveIndex);
	void SendAllMsOutSlavesConnected(PartyRsrcID mainPartyRsrcId, DWORD status, DWORD accBandwidth, mcMuxLync2013InfoReq* msSvcMuxMsg);
	void SendAllMsInSlavesConnected(PartyRsrcID mainPartyRsrcId, DWORD status, DWORD numOfConnectedInSlaves);
	void SendAllMsSlavesDeleted(PartyRsrcID mainPartyRsrcId, DWORD status);
	void DeleteMsSlave(PartyRsrcID mainPartyRsrcId, PartyRsrcID deletedSlavePartyRsrcId);

	// Multiple links for ITP in cascaded conference feature:
	void AddSubLinksAfterMainConnected(PartyRsrcID PartyId);
	void SendAddSubLinksPartiesToConf(WORD cascadedLinksNumber, WORD room_Id, char* partyName);

	void SendAckFromSlaveToMaster(PartyRsrcID PartyId, DWORD rStatus, PartyRsrcID PartyIdPeer, WORD tipPartyType, WORD allocated_resolution);

	// MSSlave-Flora Yao-2013/11/04- MSSlave Flora Comment: Just for integration, Anat will work on this interface later
	void SendAckFromMsSlaveToMain(PartyRsrcID PartyId, DWORD rStatus, PartyRsrcID PartyIdPeer, WORD msSlaveIndex, WORD allocated_resolution);
	void SendMessageFromSlaveToMaster(PartyRsrcID PartyId, WORD srcTipPartyType, DWORD opcode, CSegment* pMsg);
	void SendMessageFromMasterToSlave(PartyRsrcID PartyId, DWORD opcode, CSegment* pMsg);
	void PartyCntlToPartyMsgFromMasterToSlave(PartyRsrcID PartyId, DWORD opcode, CSegment* pMsg);
	void PartyCntlToPartyCntlMsgFromSlaveToMaster(PartyRsrcID PartyId, DWORD opcode, CSegment* pMsg);
	void PartyMsgFromMasterToSlave(CTaskApp* pParty, WORD destTipPartyType, DWORD opcode, CSegment* pMsg);
	void PartyMsgFromSlaveToMaster(CTaskApp* pParty, DWORD opcode, CSegment* pMsg);
	void PartytoPartyCntlMsgFromMasterToSlave(PartyRsrcID PartyId, WORD destTipPartyType, DWORD opcode, CSegment* pMsg);
	void PartyToPartyCntlMsgFromSlaveToMaster(PartyRsrcID PartyId, WORD srcTipPartyType, DWORD opcode, CSegment* pMsg);
	void SendDisconnectMessageFromMasterPartyControlToAllToSlaves(CTaskApp* pParty);

	void SendFallBckToReglarPartyToConf(const char* name, CIpComMode* pCurrentScm);

	void SetExclusiveContentMode(BOOL yesNo);    // Restricted content
	void SetMuteIncomingLectureMode(BOOL yesNo);

	// VideoRelay
	void AddVideoRelayImageToMix(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status);
	void UpdateOnRelayImageSvcToAvcTranslated(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status);
	void UpdateOnNonRelayImageAvcToSvcTranslated(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status);

	void HandleMrmpRtcpFirInd(PartyRsrcID PartyId, RelayIntraParam* pIntraParam);
	void SendScpNotificationReqToPartyCntl(PartyRsrcID PartyId, CSegment* pMsg);
	void ScpNotificationAckFromEP(PartyRsrcID PartyId, DWORD channelHandle, DWORD remoteSequenseNumber, DWORD bIsAck);
	void SendScpPipesMappingNotificationReqToPartyCntl(PartyRsrcID PartyId, CSegment* pMsg);
	void SendScpIvrShowSlideToPartyCntl(PartyRsrcID PartyId);
	void SendScpIvrStopShowSlideToPartyCntl(PartyRsrcID PartyId);
	void ConfAuthStatusNotify(PartyRsrcID PartyId, BYTE status);
	void UpdateArtOnTranslateVideoSsrcAck(PartyRsrcID PartyId, WORD status);
	void UpdateArtOnTranslateVideoSSRC( PartyRsrcID PartyId, DWORD ssrc);
	void ReplayUpgradeSvcAvcTranslate( PartyRsrcID PartyId, DWORD mediaType, EStat status);
	void ReplayDowngradeSvcAvcTranslate( PartyRsrcID PartyId, DWORD mediaType, EStat status);
	void PartyUpgradeToMixChannelsUpdated(PartyRsrcID PartyId,unsigned int channelHandle);
	void PartyEscalateUpgradeToMixChannelsUpdated(PartyRsrcID PartyId,unsigned int channelHandle);
	void GetPartyVideoDataReq(CSegment* pMsg);
	void SendAvcToSvcArtTranslatorConnectedToPartyControl(const CTaskApp* pParty, STATUS status);
	void SendAvcToSvcArtTranslatorDisconnectedToPartyControl(const CTaskApp* pParty, STATUS status);
    void SendUpdateToAudioBridgeOnSeenImage(PartyRsrcID idOfPartyToUpdate, PartyRsrcID idOfSeenParty);


	void ChangeVideoOutForTipPolycom(PartyRsrcID PartyId); //_t_p_

	void UpdateMuteAllAudioExceptLeader(BYTE muteAllButLeader, BYTE isMuteAllButLeaderForJustIncoming);
	void UpdateUnMuteAllAudioExceptLeader(BYTE muteAllButLeader, BYTE isMuteAllButLeaderForJustIncoming);

	void UpdateMuteAllVideoExceptLeader(BYTE onOff);
	void SetPartyAsLeaderForVB(const char* pPartyName, EOnOff eOnOf);
	void UpdateMuteAllIncomingVideoExceptLeader(BYTE onOff);

	void UpdateMrmpStreamIsMust( PartyRsrcID PartyId, DWORD ssrc, DWORD channelID, BOOL bIsMust);
	void UpdateMrmpStreamIsMustAck( PartyRsrcID PartyId, WORD status);

	void UpdateVidBrdgTelepresenseEPInfo(PartyRsrcID PartyId , CTelepresenseEPInfo* pTelepresenseEPInfo); //_e_m_
	void   SendRecordingControlAckToConf(PartyRsrcID PartyId, BYTE status);
	void  SendLayoutControlToConf(PartyRsrcID PartyId, BYTE layout);

	void ReleaseAVMCUParty(PartyMonitorID partyId, CSipNetSetup* pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, DWORD sdpLen);

	void MSOrganizerEndConnection(PartyRsrcID PartyId, DWORD status, char* FocusUri, char* MsConversationId,BOOL IsCallThroughDma);
	void MSFocusEndDisconnection(PartyRsrcID PartyId);
	void MSFocusEndConnection(PartyRsrcID PartyId, DWORD status, BOOL IsMS2013Server, char* strToAddr = NULL);
	void MSSubscriberEndDisconnection(PartyRsrcID PartyId);
	void MSSubscriberEndConnection(PartyRsrcID PartyId, DWORD status, BOOL IsMS2013Server, const char* uri);
	void SendAvmcu2013Detected(PartyRsrcID PartyId);
	void SendAvMcuAllMsInSlavesConnected(PartyRsrcID PartyId, DWORD numOfConnected);
	void SendAvMcuLocalRMXMsi(PartyRsrcID PartyId, DWORD RmxLocalAudioMSI, DWORD RmxLocalVideoMSI);

	void SendVsrMsgIndToSlavesController(PartyRsrcID PartyId, CSegment* pParam);
	void SendSingleVsrMsgInd(PartyRsrcID PartyId, const ST_VSR_SINGLE_STREAM* vsr);
	void SendSingleFecOrRedMsgFromSlavesControllerToPartyControl(PartyRsrcID PartyRsrcId, DWORD mediaType, DWORD newFecRedPercent); //LYNC2013_FEC_RED
	void SendMsgToSlavesController(PartyRsrcID PartyId, DWORD opcode, CSegment* pParam);
	void SendFullPacsiInd(PartyRsrcID PartyId,const MsFullPacsiInfoStruct* fullPacsiInfo, BYTE isReasonFecOrRed=FALSE);
	void SendSingleUpdatePacsiInfoToPartyControl(PartyRsrcID PartyId, CIpComMode* targetMode, BYTE isMute);

	void PartyToPartyCntlMsgFromMSSlaveToMain(PartyRsrcID PartyId, DWORD opcode, CSegment* pMsg);

	void SendMsSlaveToMainMsg(PartyRsrcID mainPartyRsrcId,DWORD opcode,
                                    CSegment* pMsg);
	/*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
	void SendByeToConf(const char* pPartyName);
	/*End:added by Richer for BRIDGE-12062 ,2014.3.3*/
    // VNFGE-8204
    void ChangePartyContentBitRateByLpr(PartyRsrcID PartyId, DWORD newContentRate, DWORD isDba);
	void UpdateConfTelepresenceLayoutMode(ETelePresenceLayoutMode newLayoutMode);// TELEPRESENCE_LAYOUTS
	
};

#endif //CONF_API_H__
