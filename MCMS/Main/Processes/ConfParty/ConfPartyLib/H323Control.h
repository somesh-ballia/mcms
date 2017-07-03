//+========================================================================+
//                            H323Control.h                                |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323Control.h                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Michel                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 4/16/95     |                                                     |
//+========================================================================+
// GuyD| 12/7/05     | Revision for Carmel                                 |
//+========================================================================+

#ifndef _H323Control
#define _H323Control

#include "StateMachine.h"
#include "GateKeeperCommonParams.h"
#include "H323PartyControl.h"
#include "IpRtpFeccRoleToken.h"
#include "IpCsContentRoleToken.h"
#include "H323Util.h"
#include "CallAndChannelInfo.h"
#include "RsrcParams.h"
#include "H323CsReq.h"
#include "IpCommonDefinitions.h"
#include "IpRtpReq.h"
#include "IpCmReq.h"
#include "H323CsInd.h"
#include "CsInterface.h"
#include "HardwareInterface.h"
#include "H323Authentication.h"
#include "GKManagerStructs.h"
#include "EncryptionKey.h"
#include "GkCsInd.h"
#include "IpControl.h"


const DWORD H323_CALL_SETUP_NEW_REQ		= 23001;

//Should be equal with 'callCloseReasonNoAnswer' in CallTaskCallBack.c of CS module!!
#define   callCloseReasonNoAnswer        8


class  CParty;
class  CRsrcTbl;
class  CPartyApi;
class  CH323;
class  IpRsrcDesc;
class  CH323PartyOut;
class  CH323NetSetup;
class  CCapH221;
class  CComMode;
class  CCapH323;
class  CComModeH323;
class  CIPSpan;
class  CConfParty;
//class  CH323LoadMngrConnector;
class  CH323Party;


typedef struct
{
	APIU8	     countryCode;
	APIU8        t35Extension;
	APIU16       manufactorCode;
	char*        productId;
	char*        versionId;
	APIU8		 isAvayaSipCm;
	APIU8        isCopMcu;
} RemoteVendorSt;

typedef struct
{
	APIU32		remoteEndpointType;
	APIS32		h225RemoteVersion;
	APIU32		endPointNetwork;
} RemoteInfoSt;

typedef enum
{
    kInitialCapNegotiation  = 0x00,
    kRemoteCapsRecieved     = 0x01,
    kLocalCapsAcknowledged  = 0x02,
    kCompleteCapNegotiation = 0x03
} ECapNegotiation;

#define NUM_TOKEN_MSG 6
#define IS_FECC_MSG(subOpcode) ((subOpcode >=0) && (subOpcode < NUM_TOKEN_MSG))

#define MGC_CASCADE_NEW_VID_RATE 0.94  // 94% from current video rate because of RTP protection
#define NUM_OF_MEDIA_TYPES 4
#define AUDIO_AVC_TO_SVC_TRANSLATOR_INDEX (0)

extern const char *g_roleTokenOpcodeStrings[];
extern const char* g_H239GeneicOpcodeStrings[];
extern const char *g_feccTokenOpcodeStrings[];
extern const char* g_badSpontanIndReasonStrings[];

const char* GetRoleTokenOpcodeStr(ERoleTokenOpcode o);
inline const char *GetFeccTokenOpcodeStr(feccTokenEnum o){return(IS_FECC_MSG(o))?g_feccTokenOpcodeStrings[o]:"Unknown Opcode";}
inline BYTE IsRoleTokenOpcodeH239Type(ERoleTokenOpcode opcode) {return (opcode > kStartH239TokenOpcodes);}

class CH323Cntl : public CIpCntl
{
CLASS_TYPE_1(CH323Cntl,CIpCntl)
public:
	// Constructors
	CH323Cntl(CTaskApp* pOwnerTask);
	virtual ~CH323Cntl();

	// Initializations
	void  CreateForReject(CH323Party* pParty, CH323NetSetup& pH323NetSetup, CCapH323& capH323,
				 CComModeH323* pInitialModeH323 = NULL,WORD isLocalCap = 1,char *pSid=NULL,
				 DWORD serviceId = 1,BYTE isAutoVidBitRate = 1, WORD room_id = 0xFFFF, eTypeOfLinkParty linkType = eRegularParty);
	void  CreateForAccept(CH323Party* pParty, CH323NetSetup& pH323NetSetup, CCapH323& capH323,
				 CComModeH323* pInitialModeH323 = NULL,WORD isLocalCap = 1,char *pSid=NULL,
				 DWORD serviceId = 1,BYTE isAutoVidBitRate = 1, WORD room_id = 0xFFFF, eTypeOfLinkParty linkType = eRegularParty);
	void  Create(CH323Party* pParty, CH323NetSetup& pH323NetSetup, CCapH323& capH323,
				 CComModeH323* pInitialModeH323 = NULL,WORD isLocalCap = 1,char *pSid=NULL,
				 DWORD serviceId = 1,BYTE isAutoVidBitRate = 1, WORD room_id = 0xFFFF, eTypeOfLinkParty linkType = eRegularParty);
	BYTE  IsCreated() const {return (m_pParty != NULL);}
	void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	

	int	      SetCurrentChannel(APIU32 channelIndex,APIU32 mcChannelIndex, CChannel **ppChannel, APIS8 *pIndex) const;
	CChannel* FindChannelInList(DWORD index) const;
	CChannel* FindChannelInList(cmCapDataType eType, BYTE bIsTransmiting, ERoleLabel eRole = kRolePeople, bool aIsExternal = true) const;
	DWORD     GetChannelIndexInList(bool aIsExternal, cmCapDataType eType, BYTE bIsTransmiting, ERoleLabel eRole = kRolePeople) const;
	DWORD     GetChannelIndexInList(CChannel* pChannel) const;
	DWORD	  GetLclPort(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
	unsigned int GetMrmpChannelHandle(cmCapDataType eType) const;

	void  BuildCapabilities();
	void  SetRtpUnitId(WORD unitid) {m_rtpUnitId = unitid;};
	WORD  GetRtpUnitId() { return m_rtpUnitId;};
	EResult BuildOpenChannelStruct(CCapSetInfo capInfo,ERoleLabel eRole,BYTE bMCMSOrigin,DWORD bitRate,channelSpecificParameters *pOutChannelParams, BYTE bIsNeedToImprove = TRUE);
	void  CloseIncomingChannel(cmCapDataType channelTypeH323, ERoleLabel eRole = kRolePeople);
	void  CloseOutgoingChannel(cmCapDataType channelTypeH323, ERoleLabel eRole = kRolePeople);

	ConnectionStatus	GetH323ControlConnectionStatus(){ return m_CallConnectionState;};

    // Operations
	virtual const char* NameOf() const { return "CH323Cntl";}
	virtual void*	GetMessageMap();
	void  Dump(std::ostream & msg) const;
	void   DumpRemoteVendorSt(RemoteVendorSt& stRemoteVendor);
	void   PrintDHToTrace(int len, BYTE *pDhDetail) const;
	BOOL   HandleEncryptionSession(encryptionToken *pEncToken,APIU32 numOfTokens);
	void   SetEncrAlgType(EenMediaType encAlg);
	void   SetNonEncPartyInEncrConf();
	void   OnPartyCallSetupReq(char *tempGkPrefixAlias = NULL,
								APIU32 *pDestExtraCallInfoTypes = NULL,
								char *pDestInfo = NULL, char* destExtraCallInfo = NULL,
								char* remoteExtensionAddress = NULL);

	//Multiple links for ITP in cascaded conference feature:
	int   SetUserUserFieldForMultipleLinksForITPcascadedConf(char *pUserUserString, char *pAddedString);
	void  CreateStrForITPcascade(char* pNewStr, eTypeOfLinkParty linkType, const char* partyName,DWORD cascadedLinksNumber,DWORD unrsrvMainLinkDialInNumber);

	void  OnH323CallOfferingInd(CSegment* pParam);
	void  OnPartyCallAnswerReq(int reason = -1, BYTE isGkRouted = 0);
	void  OnH323CallNewRateInd(CSegment* pParam);
	void  OnH323CallConnectedInd(CSegment* pSeg);
	void  OnH323CallDialToneInd(CSegment* pParam);
	void  OnH323CallProceedingInd(CSegment* pParam);
	void  OnH323CallRingBackInd(CSegment* pParam);
	void  OnPartyCallDropReq();
	void  UpdateCallString(CSegment* pParam);
	void  ChangeCapsAndCreateControl(DWORD vidRate);
	void  OnPartyCreateControl(BYTE bIsReCap = FALSE, BYTE capTout = FALSE);
	void  SendMultipointModeComTerminalID() const;
	void  OnPartyMediaProducerStatusReq(BYTE bIsActive = YES,cmCapDataType eType = cmCapVideo) const;
	void  OnPartyRoleTokenReq(DWORD eRoleTokenOpcode,BYTE mcuNum = 0,BYTE terminalNum = 0,BYTE randomNum = 0) ;
	void  UpdatePresentationOutStream();
    void  UpdateOutStream(ERoleLabel eRole);
    BYTE  UpdateVideoOutgoingChannelAccordingToTargetScm();
	void  OnPartyContentRateChange(APIU32 newRate, BYTE bIsSpeaker);
	void  OnPartyContentSpeakerChange(BYTE bIsSpeaker, DWORD curConfContRate = 0);
	void  OnH323FlowControlIndInd(CSegment* pParam);
//	void  OnH323MediaProducerStatusInd(CSegment* pParam);
	void  OnPartyFeccTokenReq(feccTokenEnum eFeccTokenOpcode,WORD isCameraControl = 0);
	void  SetVideoParamInCaps(H264VideoModeDetails h264VidModeDetails, BYTE cif4Mpi, DWORD videoRate);
	void  SetLocalCapToAudioOnly();
	void  SendMuteForVideoChannel(BYTE bIsActive = YES,cmCapDataType eType = cmCapVideo) const;
	void  SetITPRtcpMask();
	APIU16 RetriveCnameInfoFromEpIfPosible(char *pCnameString);

	// H239
	void SendContentOnOffReqForRtp();
	void SendCGContentOnOffReqForRtp(BYTE isOn);	// for Call Generator
	eContentState GetContentInStreamState() { return m_eContentInState;}
	void SetContentInStreamState(eContentState contentInStreamState) { m_eContentInState = contentInStreamState;}
	void SendEvacuateReqForRtpOnH239Stream();
	void SendFlowCntlInCaseStopingBeingTheSpeaker();
	BYTE IsContentSpeaker () {return m_bIsContentSpeaker;}
    DWORD GetCurrentPeopleRate () {return m_curPeopleRate;}

	void  HandleCapIndication(BYTE bPrevCapsAreFull,BYTE bPrevCapsHaveAudio, BYTE bAreAllChannelsConnected );
    void  CloseUnSupportedChannels ();
    void  InitAudioCapsAndInformRemote(BYTE bIsRecap, BYTE informRemote );
    WORD  GetBitRateFromAudioMode(BYTE audMode);
	void  OnH323RtpVideoUpdatePicInd(CSegment* pParam);
	void  OnH323CapIndication(CSegment* pParam);
	void  OnH323FacilityIndSetup(CSegment* pParam);
	void  OnH323FacilityIndConnect(CSegment* pParam);
	void  OnH323CapResponseInd(CSegment* pParam);
	void  OnH323ConfConnectedInd(CSegment* pParam);
	void  OnH323IncomingChnlInd(CSegment* pParam);
	BYTE  RejectIncomingChannelIfNeeded(mcIndIncomingChannel *pInChnlInd, APIS32 status, APIU32 channelIndex, APIU32 mcChannelIndex);
	int   InitMcChannelParams(BOOL bIsTransmit, CChannel *pMcChannel, CCapSetInfo capInfo,ERoleLabel eRole,
								DWORD rate, mcIndIncomingChannel *pinChnlInd, APIS32 status, APIU32 CsChannelIndex, APIU16 isLpr);
	int   UpdateMcChannelParams(CChannel *pMcChannel, CCapSetInfo capInfo, DWORD rate);
	void  SetOutAAL5Params(CCapSetInfo capInfo, mcReqOutgoingChannel* pOutChnlReq);
	BOOL  AllocateAndSetChannelParams(capBuffer *pCapBuffer, CChannel *pMcChannel);
	BOOL  AllocateAndSetChannelParams(int size,char *pData, CChannel *pMcChannel);

	void  sendFlowControlIfRateExceeded();
	void  OnPartyIncomingChnlResponseReq(CChannel* pChannel, BYTE bRejectChannel);
	void  OnPartyIncomingChnlResponseReq(DWORD array_index, cmCapDataType eType, BYTE rejectChannel,ERoleLabel label,APIU32 channelIndexForReject = 0);
	void  OnH323IncomingChnlConnectedInd(CSegment* pParam);
	void  OnPartyVideoUpdatePicReq(CSegment* pParam);
	void  OnH323VideoUpdatePicInd(CSegment* pParam);
	void  OnH323PacketLostStatusConnected(CSegment* pParam);
	void  OnH323PacketLostStatusImproperState(CSegment* pParam);
	void  PropagatePacketLostStatus(const eRtcpPacketLossStatus InLoss, const eRtcpPacketLossStatus OutLoss, const BYTE InLpr, const BYTE OutLpr);
	void  OnH323IncomingMediaInd(CSegment* pParam = NULL);
	void  OnH323CallCntlInd(CSegment* pParam);
	//void  SendStreamOffReq(CChannel *pChannel);
	void  OnPartyStreamOffReq(BYTE bIsStreamOffWithoutDisconnection = FALSE);
	void  OnH323ChannelCloseInd(CSegment* pParam);
	void  OnPartyCallCloseConfirmReq();
	void  OnPartyCallCloseConfirmReqIfNeeded();
	void  OnH323CallIdleInd(CSegment* pParam);
	void  OnPartyOutgoingChannelReq(CapEnum h323CapCode,ERoleLabel eRole,BYTE isMCMSOpenChannel = 0, BYTE bIsNeedToImprove = FALSE,BYTE isNeedToIntersec = FALSE,BYTE isEnable4cif = TRUE);
   	void  OnH323ForwardDisconnect(CSegment* pParam = NULL);
	void  OnH323RtpBadSpontaneuosInd(CSegment* pParam);
	void  OnH323CsBadSpontaneuosInd(CSegment* pParam);

	void  OnH323StartChannelCloseInd(CSegment* pParam);
	void  CloseStream(CChannel *pChannel);
	void  StartCloseChannel(CChannel *pChannel, BYTE bRejectByCs);
	void  SendChannelDropReq(CChannel* pChannel);
	void  HandleContentChannel(CChannel* pChannel,APIS32 chanStatus);
	void  CloseChannel(CChannel* pChannel,APIS8 arrayIndex,APIS32 chanStatus,BYTE isNeedToCloseUdp = 0);
	void  PrintChannelDetails(CChannel *pChannel, BYTE isCloseStream = 0);
	void  RejectChannel(CChannel *pChannel, APIS32 status,APIU32 channelIndex, APIU32 csChannelIndex,BYTE arrayIndex);

	void  OnH323OutgoingChnlResponseInd(CSegment* pParam);
	void  OnH323ChannelNewRateInd(CSegment* pParam);
	void  OnH323ChannelMaxSkewInd(CSegment* pParam);
	void  OnPartyGetPortReq();
	void  OnH323GetPortInd(CSegment* pParam);
	void  OnH323PartyMonitoringInd(CSegment* pParam);
	void  OnCmPacketLossInd(CSegment* pParam);
	void  RetriveCNAMEInfoIfNeeded(CSegment* pParam);

    void  OnConfOrPartyDisconnectMediaChannel(WORD channelType,cmCapDirection channelDirection, ERoleLabel eRole);
	void  OnConfStreamOffMediaChannel(WORD channelType, WORD channelDirection, ERoleLabel eRole = kRolePeople);
	void  FastUpdate(ERoleLabel eRole = kRolePeople);
	void  OnAudioConnectTimeOutSetup(CSegment* pParam);
	void  OnAudioConnectTimeOutConnect(CSegment* pParam);
	void  OnOtherMediaConnectTimeOutSetup(CSegment* pParam);
	void  OnOtherMediaConnectTimeOutConnect(CSegment* pParam);
	void  OnH323ChannelOffInd(CSegment* pParam);
	void  OnH323ChannelOnInd(CSegment* pParam);
	void  ConnectPartyToConf(void);
	BYTE  IsPartyConnectedOnlyAudioMedia();
	BYTE  IsCallConnectedAudioOnly();

	void  NoActionFunc();
	void  RemoveAndDisconnectCall(int status = 0);
	WORD  disconnectCauseEnum2Opcode(long param); //add by Uri
	void  OnCallReleasePortReq();
	void  UpdateLocalCapH323(CCapH323& capH323); //ADD BY URI
	void  UpdateH320TargetCM(CComMode &pTargetComMode);
	void  OnPartyCallDropDialIn() ;
	void  SetState(WORD state) { m_state = state;}
	void  OnPartyCallAnswerDialIn();
	void  OnPartyCallAnswerDialInFailure(int reason, char* alternativeAlias = NULL);
	void  LogicalChannelUpdate(DWORD channelType, DWORD vendorType = (DWORD)Regular);
	void  LogicalChannelConnect(CPrtMontrBaseParams *pPrtMonitor,DWORD channelType, DWORD vendorType = (DWORD)Regular);
	void  LogicalChannelDisConnect(DWORD channelType, cmCapDataType eType = cmCapEmpty, BYTE bTransmitting = TRUE, ERoleLabel eRole = kRolePeople);
	void  GatekeeperStatus(BYTE gkState=0xFF, DWORD reqBandwidth=0xFFFFFFFF, DWORD allocBandwidth=0xFFFFFFFF, WORD requestInfoInterval=0xFFFF, BYTE gkRouted=0xFF);
	// Error handling Party CS keep alive timers
	void  OnPartyCsErrHandleKeepAliveFirstTout(CSegment* pParam);
	void  OnPartyCsErrHandleKeepAliveSecondTout(CSegment* pParam);
	void  OnPartyCsErrHandleKeepAliveInd(CSegment* pParam);
	void  StartCsPartyErrHandlingLoop();
	//GK
	void  OnH323GkBRQInd(CSegment* pParam);
	void  OnH323GkDRQInd(CSegment* pParam);
	void  OnH323BRQInd(CSegment* pParam);
	void  OnH323GkIRQInd(CSegment* pParam);
	void  OnH323GKFailInd(CSegment* pParam);
	void  OnRtpDtmfInputInd(CSegment* pSeg);
	void  OnH323ARQInd(CSegment* pParam);
	void  OnIrrTimeout(CSegment* pParam = NULL);
	void  OnGkManagerRemoveCallReq(CSegment* pSeg);
	void  OnGkMangerStopIrrTimer(CSegment* pSeg);
	void  OnGkMangerSendDrqIndOrFail(CSegment* pSeg);
	void  OnGkManagerStopIrrTimer(CSegment* pSeg);
	void  OnGkManagerResendGkReq(CSegment* pSeg);
	void  OnGkManagerHoldGkReq(CSegment* pSeg);
	void  OnGkManagerKeepAliveReq(CSegment* pSeg);
	void  OnDrqTimer(CSegment* pParam);
	void  SendBrqIfNeeded();

	void  SetDialIn_mcCall();
	void  RecieveInfoFor_mcCall();
	void  OnH323GetPortFailed(CSegment* pParam = NULL);
	void  OnPartyConnectingTimeout(CSegment* pParam = NULL);
	void  OnTimerOpenChannelFromMcms(CSegment* pParam = NULL);
	void  OpenChannelFromMcmsIfNeeded();
	void  OpenChannelFromMcms();
	void  OnTimerOpenDataChannelFromMcms(CSegment* pParam = NULL);
	void  OpenDataChannelFromMcms();
	void  SetGWcallFlag(WORD setFlag) { m_onGatewayCall = setFlag ;	}
//	void  SendDelayCapAtGatewayCallReq(CComMode& pTargetMode, CCapH323& capH323);
	void  SetMaxRateAtNetsetup( DWORD confRate );
	void  SetTotalVideoRate(DWORD rate );

    void  SendConferenceResponse(ConferenceResponseEnum response);
	void  SendConferenceIndReq(WORD opcode, WORD mcuNum, WORD terminalNum, PartyRsrcID partyId);
    void  OnH323DTMFInd(CSegment* pParam);
    //
    void  OnH323ConferenceReqInd(CSegment* pParam);
    void  OnH323ConferenceComInd(CSegment* pParam);
    void  OnH323ConferenceResInd(CSegment* pParam);
    void  OnH323ConferenceIndInd(CSegment* pParam);
//	void  SendCapabilitiesAck(APIS32 lastSequenceNumber);

    // generic NonStandard events
    void  OnH323NonStandardReqInd(CSegment* pParam);
    void  OnH323NonStandardComInd(CSegment* pParam);
    void  OnH323NonStandardResInd(CSegment* pParam);
    void  OnH323NonStandardIndInd(CSegment* pParam);
    void  OnH323NonStandard(CSegment* pParam);
	void  OnTimerClear(CSegment* pParam = NULL);
	void  OnContentChangeModeTimeOut(CSegment* pParam);
	void  OnTimerWaitForUpdateRate(CSegment* pParam);
	//DBC2
	void  OnH323DBC2CommandInd(CSegment* pParam);
	void  OnConfDBC2Command(DWORD opcode,DWORD refreshRate,DWORD interLeave,DWORD mpiLimit,DWORD motionVector,
							DWORD noEncapsulation,DWORD overlap);
	BYTE  IsSupportingDBC2(CChannel* pChannel);


	int   SetSrcPartyAddress (mcReqCallSetup& pCallSetupReq);
	int   SetDstPartyAddressForAnswer (mcReqCallAnswer& pCallAnswer);
	void  SetDestPartyAddress (mcReqCallSetup& pCallSetupReq, char *pDestInfo);
	BOOL  IsChannelConnected(cmCapDataType dataType,cmCapDirection direction, ERoleLabel eRole);
	DWORD IsSymmetricProtocol(CCapSetInfo capInfo, ERoleLabel eRole, channelSpecificParameters* pChannelParams);
	void  OpenOutgoingChannel(cmCapDataType dataType, ERoleLabel eRole = kRolePeople);
    void  UpdateNetSetUp(CH323NetSetup& pH323NetSetup);// {*m_pH323NetSetup = pH323NetSetup;}

	CBaseVideoCap* FindIntersect(CapEnum incomingProtocol,CapEnum scmProtocol,BYTE isopen4cif = TRUE);
	CBaseVideoCap* FindIntersectForHDCPAsymmetricModes();
	CBaseVideoCap* FindIntersectForNetMeeting(CapEnum incomingProtocol,CapEnum scmProtocol);

	//Prty monitoring request from the operator
	void  PartyMonitoringReq();
	BOOL  CheckCascadeParams();

	WORD GetConnectionId() const;

	void SetOrign(BOOL isOrigin);
	void RemoveCsControllerResource();
	void SetConnectionIdForReject(DWORD DinRejectConnectionId);
	void SetMcCallNetSetupParams();
	void SetConfType(EConfType confType);
	CCall* GetCallParams() { return m_pmcCall;};
	WORD GetMcuNumFromMaster(){return m_mcuNumFromMaster;};		// for Call Generator
	WORD GetTerminalNumFromMaster(){return m_terminalNumFromMaster;};	// for Call Generator

	// Media detection, imported from SIP
    void    OnMediaDisconnectDetectionInd(CSegment* pParam);
    void    OnMediaDisconnectDetectionIndInAnycase(CSegment* pParam);

    void OnMediaResume(CSegment* pParam);
    void MediaDisconnectionTimerKick();
    
	// LPR
	void OnRtpLprChangeModeInd(CSegment* pParam);
	void OnH323LprChangeModeAckInd(CSegment* pParam);
	void OnH323LprChangeModeInd(CSegment* pParam);
	void OnH323LprChangeModeIndSetup(CSegment* pParam);
	void OnH323LprTout(CSegment* pParam);
	void CancelLpr();
	void SendLprReqToMfa(WORD status, DWORD lossProtection, DWORD mtbf, DWORD newPeopleRate, DWORD fill, DWORD modeTimeout, BYTE isReset = 0);
	void SendLprAckToParty();
	void MfaH323LprAck();
	void UpdateDbLprSyncStatus(BYTE isSynced, BYTE isLocal);
	void OnRtpLprTout(CSegment* pParam);
	void UpdateRtpWithLprInfo();
	void UpdateLprModeReqToRtp(cmCapDataType dataType, BYTE bOutDirection, ERoleLabel eRole);
	bool SetNewPeopleRateLpr(DWORD& newPeopleRate, DWORD& newTotalVideoRate, DWORD& newContentRate, bool reCalcContent);
	bool DynamicContentLprHandling(mcIndLPRModeChange& lprChangeModeInd, BYTE& bLprOnContent, BYTE& bSetPeopleRate, DWORD& newPeopleRate, DWORD& newTotalVideoRate, DWORD& newContentRate);
	bool DynamicContentLprHandlingInCascade(mcIndLPRModeChange& lprChangeModeInd, BYTE& bSetPeopleRate, DWORD& newPeopleRate, DWORD& newTotalVideoRate, DWORD newContentRate);
	bool DynamicContentLprHandlingInSingleConf(BYTE& bLprOnContent, DWORD& newPeopleRate, DWORD& newTotalVideoRate, DWORD& newContentRate);
	void CalcLprInfluanceNewContentRateWithThreshold(bool& bIsForceThreshold, bool& bImproveContent, DWORD newTotalVideoRate, DWORD& newContentRate, DWORD& newPeopleRate, DWORD& dwContentThresholdRate);


	//added by Jason for ITP-Multiple channels
	void SendNewITPSpeakerToParty(eTelePresencePartyType  itpType, DWORD numOfActiveLinks);
	void SendNewITPSpeakerAckToParty();
	void OnH323NewITPSpeakerAckInd(CSegment* pParam);
	void OnH323NewITPSpeakerInd(CSegment* pParam);
	
	void OnCapabilitiesTout(CSegment* pParam);
	H264VideoModeDetails GetH264ModeAccordingToRemoteVideoPartyType(eVideoPartyType videoPartyType) const;

	void SetRssInfo(CSegment* pMsg);
	void SendRssRequest();
	void UpdateRtpOnLeaderStatus(BYTE isLeader);
	// Cop
	void SetCopVideoTxModes(CCopVideoTxModes* pCopVideoModes);

	void OnH323RtpSelfVideoUpdatePicReq(CSegment* pParam);
	void RemoveTransmitCaps();
	BYTE IsNeedToSendTrasmitCap();
	BYTE IsNeedToCreateRemoteTxMode();
	BYTE IsNeedToOPenAccordingToRemoteTxModes();
	void AdjustRMX1000levelsToRMX2000levels();
	BYTE IsRemoteRmx1000or500();
	BYTE  IsRemoteIsRVpresentation() const;
	BOOL CheckAndMakeH323CallOnGKFail(); //EXT-4632, VNGR-20237
	void InitSpeakerParams();


	//TIP
	BOOL IsPartyLegacyForTipContent (DWORD partyContentRate);
	BOOL IsRemoteHasContentXGA();

	virtual CPartyApi*	GetPartyApi() {return m_pTaskApi;}
	virtual CParty*		GetParty() {return (CParty*)m_pParty;}
	virtual eConfMediaType GetTargetConfMediaType() {return m_pTargetModeH323->GetConfMediaType(); }

	// Attribute
//-------------------------------------
	CCapH323*		m_pLocalCapH323;
	CCapH323*		m_pRmtCapH323;
	CComModeH323*	m_pTargetModeH323;	// desirable mode
	CComModeH323*	m_pCurrentModeH323;	// current mode (currently opened channels)
//-------------------------------------
	CH323Party*									m_pParty;
	CPartyApi*									m_pTaskApi;

	CCopVideoTxModes*							m_pCopVideoModes;
	CCopVideoTxModes*                           m_pCopRemoteVideoModes;
	CH323NetSetup*								m_pH323NetSetup;
	CSegment*									m_pNewInChanSeg[NUM_OF_MEDIA_TYPES];
	CCall*										m_pmcCall;
	DWORD										m_combineOpcode;
	BYTE										m_channelTblIndex;
	mcIndGetPort								m_getPortInd;
	WORD										m_rtpUnitId;

	EenMediaType								m_encAlg;		//In dialOut-local alg, dialIn-rmt alg

	CIpDHKeyManagement*							m_pDHKeyManagement;  //For Encryption

	BOOL										m_bIsAvaya;
	RemoteInfoSt								m_remoteInfo;
	RemoteVendorSt								m_remoteVendor;
	char										m_sId[Pf1SessionIdSize];
	APIU8										m_nonceArq[Pf1NonceSize];
	char										*m_pTempGkPrefixAlias;
	APIU32										*m_pDestExtraCallInfoTypes;
	char										*m_pDestInfo;
	char										*m_pDestExtraCallInfo;
	char										*m_pRemoteExtensionAddress;

	APIU32										m_pDestUnitId;
	APIU32										m_callIndex;
	DWORD										m_serviceId;
	mcTransportAddress							m_gkRouteAddress;						// GK route address for ARJ
	APIU32										m_numOfGkRoutedAddres;

	PDECLAR_MESSAGE_MAP

//-------------------------------------
		// internal use
	BYTE				m_isAudioOutgoingChannelConnected;
	BYTE				m_isVideoOutgoingChannelConnected;
	BYTE                m_isVideoContentOutgoingChannelConnected;
	BYTE				m_isDataOutgoingChannelConnected;
	BYTE				m_isIncomingAudioHasDisconnectedOnce;//for reopen audio in channel
	BYTE				m_isOutgoingAudioHasDisconnectedOnce;//for reopen audio out channel

	BYTE				m_isCallDropRequestWaiting;

	BYTE				m_isH245Connected;
	BYTE				m_isCallConnetIndArrived;
	BYTE				m_isReceiveCallDropMessage;
	BYTE				m_isReceiveCallIdleMessage;
	BYTE				m_isCloseConfirm;
	BYTE				m_isCallAnswerReject;
	ConnectionStatus	m_CallConnectionState;
	DWORD*      		m_oldMediaBytes;
	DWORD*      		m_oldFrames;
	BYTE				m_McmsOpenChannels;
	//BYTE				m_OneOfTheMediaChannelWasConnected;
	BYTE				m_onGatewayCall;
	BYTE				m_bIsRemoveGeneric;

	BOOL				m_isAudioConnected;


	BYTE 				m_CapabilityNegotiation;//type ECapNegotiation

//	WORD				m_numOfPartyMonitoring;
	WORD				m_speakerMcuNum;
	WORD				m_speakerTermNum;
	PartyRsrcID			m_speakerPartyId;

	WORD	  			m_maxCallChannel;

	WORD				m_isLprModeOn;
	WORD				m_LprModeTimeout;
	BYTE                m_ChannelsWithLprPayload;

	//Multiple links for ITP in cascaded conference feature:
	eTypeOfLinkParty    m_linkType;

	DWORD IsNeedToSendFlowControl();
	RemoteIdent GetRemoteIdent() const {return m_remoteIdent;}
//	void UpdateRateIfNeeded(CChannel *pCurrentChannel,BOOL bIsAfterReCpas);
	void OnChannelConnected(CChannel *pCurrentChannel);
	void UpdateCurrentScmH323(CChannel *pCurrentChannel);
	BYTE CreateTargetComMode(BYTE bIsDataOnly);
	BYTE AreAllChannelsConnected(void);
	BYTE IsChannelInDisconnectProcessByMcms();
	void StoreRemoteVendorInfo(h460AvayaFeVndrIndSt& avfFeVndIdInd);
	void DuplicateRemoteVendorInfo(RemoteVendorSt& stRemoteVendor);
	void FillRemoteVendorInfoAvaya(RemoteVendorSt& stRemoteVendor, mcIndCallConnected* pCallConnectedInd);
	void CheckAndUpdateRemoteVendor(RemoteVendorSt& stRemoteVendor, mcIndCallConnected* pCallConnectedInd);
	void ChangeCapabilitySetAccordingToVendor(RemoteVendorSt& stRemoteVendor, BYTE& isCascadeToPreviousVer);
	void ChangeCapabilitySetAccordingToVendorId();
	bool ChangeCapabilitySetAccordingToVendorId(mcIndCallConnected* pCallConnectedInd, BYTE& isCascadeToPreviousVer);
	void RemoveGenericCapAndUpdateConfLevel();
	BYTE OnConfFlowControlReq(DWORD newVidRate, BYTE outChannel, lPRModeChangeParams* pLprChangeModeParams=NULL);
	BYTE SendFlowControlReq(cmCapDataType eType,BYTE bIsTransmit,ERoleLabel eRole,DWORD newRate);

	BOOL CheckFlowControlDetails(DWORD newVidRate, CChannel *pChannel);

	void SendStopAllProcessorsToCard(mcReqStopAllProcesses* pStopAllProcessors, CChannel *pMcChannel);
	void UpdatePartyVideoRate();
	void UpdateRemoteCapsAccordingToRemoteType(CCapH323 &pTmpRmtCaps);

//Highest Common functions:
    void OnH323StreamStatusInd(CSegment* pParam);
	void OnTimerRopenContentIn(CSegment* pParam);

		//Operations:
	BYTE UpdateTargetMode(CComModeH323* pNewScm, cmCapDataType type = cmCapEmpty, cmCapDirection direction = cmCapReceiveAndTransmit, ERoleLabel eRole = kRolePeople);
	void UpdateLocalCapsFromTargetMode(BYTE bIsVideoCapEqualScm);
	void OpenVideoOutChannelFromMcms(BYTE bOpenWithoutCheck = TRUE);
	void OpenContentOutChannelFromMcms();
	void OnPartyNewChannelModeReq(DWORD specificVideoRate = 0);
	void BuildNewCapsFromNewTargetModeAndCaps( CCapH323* pCaps,const CCapH323* pNewLocalCaps=NULL);
	WORD GetChangeFromNewScmMode(CComModeH323* pNewScm,cmCapDirection direction,cmCapDataType dataType,ERoleLabel eRole = kRolePeople) const;
	CComModeH323* GetTargetMode()  {return m_pTargetModeH323;}
	CComModeH323* GetCurrentMode() {return m_pCurrentModeH323;}
	CCapH323*     GetLocalCaps()   {return m_pLocalCapH323;}
	WORD GetFeccMediaToDeclare(CapEnum &feccMedia);
	void SendRemoteNumbering();
	BYTE IsDelayedIvr() {return m_isDelayedIvr;}

    // Quality Of Service
	void  SetQualityOfService(const CQoS& qos) {*m_pQos = qos;}
	DWORD GetCurConfContRate() const {return m_curConfContRate;}

	DWORD RoundUpConfRate(int CallRate);

	// MCU INTERNAL PROBLEM
	void TranslateAckToMipErrorNumber(MipHardWareConn& mipHwConn, MipMedia& mipMedia, MipDirection& mipDirect,
									   MipTimerStatus& mipTimerStat, MipAction& mipAction, ACK_IND_S* pAckIndStruct = NULL,
									   CChannel* pChannel = NULL,BYTE isClearTimer = 0);

	void  CheckOpenOutgoingVideoChannel();

	//26.12.2006 Changes by VK. Stress Test
	void StartStressTestTimerOnce();
	void OnStressTestTimeout(CSegment* pParam);
	void StressTestCase(CapEnum eAudioCap, CapEnum eVideoCap, BOOL bAudioClose, BOOL bVideoClose, BOOL bRegTimer);
	void CloseSpecificOutgoingChannels(BOOL bAudioClose, BOOL bVideoClose);
    void SendAVFFacilityEndOfMove(const char * sNumericID);
	BOOL IsIncomingVideoExist();
	// Cascade H239 MIH
    BYTE  IsSlaveCascadeModeForH239() const;

	// VNGFE-787
	void SetIsCodianVcr(BYTE isCodianVcr);
	BYTE GetIsCodianVcr();
	void OnCodianVcrVidChannelTimeout(CSegment* pParam);
	BYTE AreAudioVideoChannelsConnected();
	BYTE ConvertChannelTypeEnumToArrayIndex(cmCapDataType eType,ERoleLabel eRole );
	DWORD GetCallRateAllowedByRemote();

    void SetLprMode(WORD lprOnOff);

    void SetH264VideoParams(H264VideoModeDetails h264VidModeDetails, APIS32 sar, cmCapDirection direction);


    //Flow Control constraint
    void SetConfPeopleFlowControlConstraint (DWORD val) {m_confPeopleFlowControlConstraint = val;}
    DWORD GetConfPeopleFlowControlConstraint () {return m_confPeopleFlowControlConstraint;}

    void SetH264ModeInLocalCapsAndScmAccordingToVideoPartyType(eVideoPartyType videoPartyType,APIS8 cif4Mpi);
    void SetH264ModeInLocalCapsForTandEP(RemoteVendorSt& stRemoteVendor);
	// IpV6
	enIpVersion CheckForMatchBetweenPartyAndUdp(enIpVersion eIpVer,eIpType eipType);
	void SetSrcSigAddressAccordingToDestAddress();
    void SendStartPartyPreviewReqToCM(DWORD RemoteIPAddress,WORD VideoPort,cmCapDirection Direction,CapEnum capEnum);
    void SendStopPartyPreviewReqToCM(cmCapDirection Direction);
    BYTE IsContentRejected() {return m_bIsContentRejected;}
    BYTE IsWaitForContentOutToBeOpen();
    
    BYTE  IsRemoteIsSlaveMGCWithContent() const;
    BYTE  IsRemoteIsSlaveMGC() const;
	void SendReqToGkManager(CSegment *pSeg, OPCODE opcode);

	virtual int OpenInternalArts(ENetworkType networkType);
    BYTE UpgradeAvcChannelReq(CComModeH323* pTargetMode);
    void ConnectPartyToConfWhenUpgradeToMix();
    void Deescalate(CComModeH323* pTargetMode);
    void DeescalateChannelsByMedia(cmCapDataType aDataType);
    void UpdateMrmpInternalChannelIfNeeded();
    /*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
    void VideoRecoverSendCallDrop(){SendCallDrop();}
    /*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/

protected:
    CQoS* m_pQos;
	BYTE  m_bVideoInRejected;
	BYTE  m_bVideoOutRejected;
	BYTE  m_bIsDataInRejected;
	BYTE  m_bIsDataOutRejected;
	RemoteIdent m_remoteIdent;

	//gk:
//	PathNavigatorStates m_PNGkState;
	int					m_irrFrequency;
	BYTE				m_isCallingThroughGk;
	int  				m_gkRequestedBrqBw;

	int  m_NumOfGetPortMsg;

	void DisconnectForCallWithoutSetup();
	BYTE CalculateNewBandwidth(DWORD& channelsTotalRate);
	void OnPartyFlowControlReq(CChannel *pChannel,DWORD newRate);
/*	BYTE ChangeCapsForSpecialVSWCascades(const char* user);
	BYTE ChangeCapsForSpecialCP2VSWCascades(const char* userUser);*/
	void FindSiteAndVisualNamePlusProductIdAndSendToConfLevel(char* sCallConnectedDisplay);

	BYTE DecideOnUpdatingOutRateAccordingToRemoteType(CapEnum h323CapCode);
	BYTE UpdateVideoOutRates(CapEnum h323CapCode);
    DWORD ChangeVideoRateInCp(DWORD oldVideoRate);

	// Highest Common:
	void UpdateParamsBeforeConnectingToConf(BYTE bUpdateVidParams);
	BYTE SaveIncomingChannelForFurtherUse(mcIndIncomingChannel *pInChnlInd,APIU32 callIndex,APIU32 channelIndex,APIU32 mcChannelIndex,APIU32 stat1,APIU16 srcUnitId);
	void SetH264Mbps(channelSpecificParameters* pChannelParams);

	//ECS
    void DisconnectCallBecauseBadCaps();
	BYTE ECSDecideOnOpeningOutChannels();
//	void OnH323NotifyInd(CSegment* pParam);
	void OnChannelsECSTimer();
	//Content:
	void SendEndChangeContentToConfLevel(EStat status = statOK);
	// Encryption:
	void SetEncryptionInStructToZero(encTokensHeaderStruct& encryTokens);

	void SetIsH263PlusForOutgoingChannel(CChannel* pMcChannel);

	void SendCallDropIfNeeded();
	void SendCallDrop();

	//MFA & CM
	void AllocateDynamicPayloadType(CChannel* pMcChannel, CCapSetInfo capInfo);
	BYTE Rtp_FillAndSendUpdatePortOpenRtpStruct(CChannel *pMcChannel, BYTE isUpdate = 0, BOOL isInternal = FALSE, BYTE index = 0);
	void Rtp_FillUpdatePortOpenRtpChannelStruct(TUpdateRtpSpecificChannelParams* pUpdateRtpSpecificChannelParams, CChannel *pMcChannel);
	void Rtp_FillAndSendUpdateRtpChannelStruct(CChannel *pMcChannel);
	void Rtp_FillAndSendUpdateRtpChannelRateStruct(CChannel *pMcChannel);
	void SendUpdateMtPairReq();
    virtual DWORD ShouldInitTimerForSendMsg(DWORD opcode);
    virtual void InitAllChannelsSeqNum();
	void Cm_FillAndSendOpenUdpPortOrUpdateUdpAddrStruct(CChannel *pMcChannel, mcTransportAddress rmtAddress);
	void OpenSvcChannel(CChannel *pChannel, mcTransportAddress &rmtAddress, BYTE isUpdate = NO);
	void CloseSvcChannel(CChannel *pChannel);
	int OpenInternalChannels(EConnectionState iChannelsInState);
	int OpenInternalChannelsByMedia(cmCapDataType aDataType, DWORD bitRate, EConnectionState iChannelsInState /*-1*/);
	void CloseInternalChannels(cmCapDataType aDataType);
	void Cm_FillAndSendCloseUdpPortStruct(CChannel *pMcChannel);
	void OnMfaReqToutAnycase(CSegment* pParam);
	void OnMfaAck(CSegment* pParam);
	void OnMfaAckDisconnectInternalArt(CSegment* pParam);
	void OnH323RoleTokenInd(CSegment* pParam);
	void OnH323FeccTokenInd(CSegment *pParam);
	void OnH323FeccKeyInd(CSegment *pParam);
	//Content translation - EPC - H239 - PPC(API)
	DWORD TranslateH239OpcodeToPPCOpcode(mcIndRoleToken *pRoleToken);
	DWORD TranslateEPCOpcodeToPPCOpcode(mcIndRoleToken *pRoleToken);
	void  TranslatePPCOpcodeToEPCOpcode(DWORD EPCOpcode,mcReqRoleTokenMessage **pReq);
	void  TranslatePPCOpcodeToH239Opcode(DWORD EPCOpcode,mcReqRoleTokenMessage **pReq);
	void  HandleContentOpcodes(DWORD Opocde,mcIndRoleToken *pRoleToken);

	void  FillIpAndPort2McCall_callSignalAddress();

	//GK:
	HeaderToGkManagerStruct SetHeaderToGkManagerStruct(APIS32 status = STATUS_OK);
	void  InsertLocalAlias();
	void  InsertRemoteAlias();
	void  HandleBandwidth(BOOL &bWithoutVideo, BOOL bChangeAudio = TRUE);
	void  HandleBandwidthAvaya(gkIndBRQFromGk* pBRQfromGKIndSt);
	BYTE  IsRejectBRQ(gkIndBRQFromGk* pBRQfromGKIndSt
                     ,int           * par_pMinMaxPossibleRate);//HOMOLOGATION. A negative value indicates MIN-rate from required (from GK); Positive: MAX-rate
	void  CreateAndSendARQReq();
	void  CreateAndSendBRQReq(int newBandwidth);
	void  CreateAndSendIrrReq(DWORD opcode, int hsRas = 0);
	void  CreateAndSendDRQReq(BYTE disengageReason);
	void  UpdateGkCallIdInCdr(char* callId);
	void  DisconnectCallBecauseInsufficientBandwidth();
	void  CloseAllChannelsBesideAudio();
    void  HandleBCFAsFlowControl (DWORD gkBcfRate);

	// for content

	CapEnum GetCommonContentMode(ERoleLabel &eRole);
	void  SetContentTargetTransmitMode(CCapSetInfo capInfo, ERoleLabel eRole);
	void  SetNewVideoRateInH239Call(DWORD newContentTdmRate);
    BYTE  isContentRequest (ERoleTokenOpcode eRoleTokenOpcode) const;
    DWORD GetRmtPossibleContentRate() const;

    BYTE  IsRemoteACascadedMcuWithH239Enabled();
    DWORD m_LastContentRateFromMaster; // for the content coming from the master side (slave)
    DWORD m_lastContentRateFromMasterForThisToken; // for the content coming from our side (slave)

    //General
    char *GetRejectReasonAsString(APIS32 rjReason);

    //H263 4CIF
    BYTE IsH2634CifSupported();
    CBaseVideoCap* SetScmWith4Cif();
    BYTE checkIsh263preffered(CBaseVideoCap* Intersect,BYTE bIsCp);

    BYTE IsLateReleaseOfVideoResources () const;
    BOOL IsTIPContentEnable() const;
    BOOL IsCallGeneratorConf() const;

    void OnMrmpRtcpFirInd(CSegment* pParam);
    APIU32	GetMasterSlaveTerminalType();

	BYTE  m_bIsContentSpeaker;
	DWORD m_curConfContRate; //no bch
	DWORD m_targetConfContRate;
	DWORD m_curPeopleRate; // no bch
//	DWORD m_curPeopleTdmRate;

	BYTE  m_bWaitForFlowCntlIndIndOnPeople;
	BYTE  m_bWaitForFlowCntlIndIndOnContent;
	BYTE  m_bNeedToAnswerToMasterOnPeople;
	BYTE  m_bNeedToAnswerToMasterOnContent;
	BYTE  m_bFirstFlowControlIndIndOnContent;
	BYTE  m_bLinkFirstFlowControlIndIndOnContent; //For cascade P&C
	BYTE  m_bIsStreamOffContentNeeded;
	BYTE  m_bIsOutContentChanReject;	//when receive outgoingChanResponse with status!=0
	BYTE  m_bIsContentRejected;		//After closeing In and out content channels.
	BYTE  m_bContentInClosedWhileChangeVidMode;
//	CH323LoadMngrConnector *m_pLoadMngrConnector;
	eContentState m_eContentInState;

	BYTE						m_remoteCapIndNotHandle;
	BYTE						m_bPrevCapsAreFull;
	BYTE						m_bPrevCapsHaveAudio;

 	WORD						m_keepAliveTimerCouter;
 	WORD						m_isKeepAliveIndArrived;
 	BYTE 						m_isAutoVidBitRate;
 	BYTE						m_isH239FlowCntlSent;
 	BYTE						m_isDelayedIvr;
    //H239 cascade - Slave disguised as endpoint mode
    BYTE                        m_bDisguiseAsEPMode;
    WORD                        m_mcuNumFromMaster;
    WORD                        m_terminalNumFromMaster;
 	// VNGFE-787
 	BYTE						m_isCodianVcr;
 	BYTE						m_isRealIncVidChanSentFromCodianVcr;
    DWORD                       m_confPeopleFlowControlConstraint;

    DWORD 						m_realLprRate;
    BYTE						m_isLprContentForceReductionTo64;
    lPRModeChangeParams 		m_lprModeChangeData;
    BYTE						m_isCameraControl; //some of the token indication are not real for fecc
    BYTE						m_useRtcp;

    // Rss info parameters
    BYTE	m_isStreaming;
    BYTE*	m_pExchangeConfId;
    BYTE    m_isAlreadySentMultipointToMGC;
    BYTE    m_FixVideoRateAccordingToType;

	APIU16   m_RtcpCnameMask;
	BYTE m_IsNeedToExtractInfoFromRtcpCname;

private:
	std::string GetCloudIp() const;

	eRtcpPacketLossStatus m_cmInboundPacketLossStatus;
	eRtcpPacketLossStatus m_cmOutboundPacketLossStatus;
	eRtcpPacketLossStatus m_adjInboundPacketLossStatus;
	eRtcpPacketLossStatus m_adjOutboundPacketLossStatus;
	BYTE m_inboundLprActive;
	BYTE m_outboundLprActive;
	BYTE 	m_isContentOn;

	BOOL m_isAudioMuted;
	BOOL m_isVideoMuted;
};




#endif /* _H323CNTL  */
