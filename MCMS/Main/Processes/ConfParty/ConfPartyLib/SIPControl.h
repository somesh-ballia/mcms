//+========================================================================+
//                            SIPControl.h                                 |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPControl.h                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 15/11/05   | This file contains								   |
//     |            |                                                      |
//+========================================================================+

#ifndef __SIP_CONTROL__
#define __SIP_CONTROL__

#include "IpRtpReq.h"
#include "CsInterface.h"
#include "HardwareInterface.h"
#include "SipNetSetup.h"
#include "SdesManagment.h"
#include "ICEParams.h"
#include "IceCmInd.h"
#include "IceCmReq.h"
#include "IpCmReq.h"
#include "SipConfPartyDefinitions.h"
#include "TipRtcpControl.h"
#include "TipStructs.h"
#include "SipBfcpControl.h"
#include "MrcStructs.h" /* for softMcu eyaln_to_merge */

#include "RelayIntraData.h"
#include "ScpHandler.h"
#include "MrcApiDefinitions.h"
#include "ArtDefinitions.h"
#include "IpControl.h"
#include "SipVsrControl.h"

#define OK_VAL					200
#define LOW_REJECT_VAL  300
#define HIGH_REJECT_VAL	700
#define MIN_VIDEO_RATE	320
#define MAX_LYNC_AUDIO  256


//const WORD MFA_RESPONSE_TIME = 20; /* for softMcu eyaln_to_merge */

#define IMS_PSI_SOURCE_NONE			"NONE"
#define IMS_PSI_SOURCE_FACTORY		"FACTORY"
#define IMS_PSI_SOURCE_EQ			"EQ"


typedef struct {
	BOOL  isNeedToHandleNewData;
	DWORD fractionLossInPercent;
	DWORD mediaType;
	DWORD ssrc;
} PACKET_LOSS_STATUS;


enum enIMSSipPsiSource
{
	eIMSSourceNone = 0,
	eIMSSourceFactory,
	eIMSSourceEntryQ
};

enum enIceConnectivityCheckStatus
{
    eIceNotConnected,
    eIceConnectedLocal,
    eIceConnectedRemote
};
//LyncCCS
enum enPartyAuthState
{
	ePartyAuthStateIdle 				= 0,
	ePartyAuthStateWaitingRemote 	= 1,
	ePartyAuthStateWaitingLocal 		= 2,
	ePartyAuthStateTobeMatched 		= 3,
	ePartyAuthStateCompleted  		=4,
	ePartyAuthStateButt
};

enum  enPartyAuthRespStatus
{
	ePartyAuthRespFailed			=0,
	ePartyAuthRespSuccess			=1,
	ePartyAuthRespInvalid			=2, 
	ePartyAuthRespButt				
};

#define	DefaultOfDnsInterval 120
//#define BFCP_PORT_NONZERO 1
#define BFCP_IPV4_TCP_PORT 60002
#define BFCP_IPV6_TCP_PORT 60003
#define BFCP_TCP_RTCP_PORT 0
#define STREAM_LABEL_VIDEO "20"
#define STREAM_LABEL_CONTENT "21"
#define BFCP_FLOOR_ID_PPC "1"
#define BFCP_FLOOR_ID 	   	atoi(BFCP_FLOOR_ID_PPC)

#define MAX_LABEL_LENGTH 32
#define QOS_HEADER_PREFIX "Resource-Priority"
#define QOS_REQUIRE_STR "Require: resource-priority"
#define DTMF_FORWARDING_SOURCE_HEADER "plcm-dtmf-forwarding-source"
#define MRD_HEADER_STR "MRD:MRM;MRC-V:1.0.1"
#define MRD_SDP_HEADER_STR "MRD=MRM MRC-V=1.0.1"
#define MRC_VERSION_STR "MRC-V="



class CSipParty;
class CSipWebRtcCntl;

class CSipCntl : public CIpCntl
{
CLASS_TYPE_1(CSipCntl, CIpCntl)

public:

	CSipCntl(CTaskApp * pOwnerTask);
	virtual const char* NameOf() const { return "CSipCntl";}
	virtual ~CSipCntl();
	void Create(CSipParty* pParty, CSipNetSetup* pNetSetup, /*CIpRsrcDesc* pRsrcDesc,*/ BYTE bIsInitiator, DWORD serviceId, DWORD room_Id);

	BYTE IsWebRtcCntl() {return (GetWebRtcCntl() != NULL);}
	virtual CSipWebRtcCntl *GetWebRtcCntl() {return NULL;}
	virtual void DisconnectWebRtcCntl() {}
	virtual void AddInviteResponseReqHeader(CSipHeaderList& rHeaderList) {}
	virtual void OverwriteMonitoring(EIpChannelType channelType, mcTransportAddress &partyAddr, mcTransportAddress &mcuAddr, BYTE &IsIce, mcTransportAddress &partyIceAddr, mcTransportAddress &mcuIceAddr) {}

	virtual void* GetMessageMap() {return (void*)m_msgEntries;}
	virtual void  HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
	virtual BOOL  DispatchEvent(OPCODE event,CSegment* pParam);
	const CSipCaps*		GetLastRemoteCaps() const {return m_pLastRemoteCaps;}
	const CSipCaps*		GetLocalCaps() const {return m_pChosenLocalCap ;}
	CSipNetSetup*	    GetNetSetup() {return m_pNetSetup;}
	RemoteIdent			GetRemoteIdent() const {return m_remoteIdent;}
	APIU16				GetPlcmRequireMask() const {return m_plcmRequireMask;}
	void				SetPlcmRequireMask(APIU16 mask) {m_plcmRequireMask = mask;}
	eMediaLineInternalType	GetPlcmReqMaskMlineAtIndex(APIU16 index) { return m_plcmReqMaskMlineOrder[index]; }
	void					SetPlcmReqMaskMlineOrder(eMediaLineInternalType plcmReqMaskMlineOrder[]) { memcpy(m_plcmReqMaskMlineOrder, plcmReqMaskMlineOrder, sizeof(m_plcmReqMaskMlineOrder)); }
	const char*         GetUserAgent() const{return m_UserAgent;}
    BYTE		        IsRemoteMicrosoft () {return (m_remoteIdent == MicrosoftEP_R1 || m_remoteIdent == MicrosoftEP_R2 || m_remoteIdent == MicrosoftEP_Lync_R1 || m_remoteIdent == MicrosoftEP_MAC || (m_remoteIdent == MicrosoftEP_MAC_Lync) || MicrosoftEP_Lync_2013 == m_remoteIdent ||  Microsoft_AV_MCU2013 == m_remoteIdent || Microsoft_AV_MCU == m_remoteIdent || MicrosoftwPolycomEP_Colab_EP1 == m_remoteIdent) ;}
    BOOL		        IsMicrosof2013() const {return (m_remoteIdent == MicrosoftEP_Lync_2013 || m_remoteIdent == Microsoft_AV_MCU2013) ;}
	void 				SavePreviousRemoteCaps();
	void 				ReturnRemoteCapsToThePreviousCaps();
	const char*         GetSdpRemoteSessionInformation() const{return m_sdpRemoteSessionInformation;}
//	CIpRsrcDesc*	GetRsrcDesc() const {return m_pRsrcDesc;}
//	CIPService*		GetService() const {return m_pService;}
	WORD			GetConId() const;
	DWORD			GetCallIndex() const {return m_pCall->GetCallIndex();}
	CSipHeaderList* GetCdrHeaders() const;
	CConfIpParameters* GetServiceParams() const;
	const char*		GetForwardAddr() const;
	enTransportType GetTransportProtocol() const;
	DWORD		GetOutBoundProxyIp() const;
	BYTE		IsMedia(cmCapDataType eMediaType,cmCapDirection eDirection=cmCapReceiveAndTransmit, ERoleLabel eRole = kRolePeople) const;
	void		MuteChannels(BYTE bIsMute,int arrSize,EIpChannelType chanArr[]);
	BYTE		IsChannelMuted(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
	int			GetNumOfChannels() const;
	APIU8		GetChannelPayloadType(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
	CSipChannel *GetChannel(cmCapDataType eMediaType, cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
	EConnectionState GetChannelConnectionState(cmCapDataType eMediaType, cmCapDirection eDirection, ERoleLabel eRole) const;
	EConnectionState GetCallConnectionState() const;
	void		SetIpAddressToAllMedias(sipSdpAndHeadersSt* pSdpAndHeaders, BYTE isResultOfIpMediaMismatch = 0, BYTE isReinvite = 0); //add param for ANAT
	enIpVersion SetMediaIpAddress(mcXmlTransportAddress &mediaIp, unsigned int &rtcpPort, cmCapDataType mediaType, ERoleLabel eRole, cmCapDirection mediaDirection, BYTE isResultOfIpMediaMismatch = 0);
	enIpVersion SetSessionIpAddress(mcXmlTransportAddress &mediaIp, BYTE isResultOfIpMediaMismatch = 0);
	enIpVersion	SetLocalMediaIp(union ipAddressIf &sIpAddress,cmCapDataType mediaType, ERoleLabel eRole = kRolePeople);
	enIpVersion SetMediaIPFromIceParams(union ipAddressIf &sIpAddress,cmCapDataType mediaType, ERoleLabel eRole);
	void 		AddClosedMediaCapsToSdp(sipSdpAndHeadersSt* pSdpAndHeaders,int capSize,int &exactCapsSize);
	void		SetPortZeroForClosedMedias(sipSdpAndHeadersSt* pSdpAndHeaders);
	DWORD		GetLocalMediaIp() const;
	UdpAddresses GetUdpAddress();
	BOOL		IsRemoteMediaIpZero(cmCapDataType eMediaType, ERoleLabel eRole);
	// IpV6
	void        GetRemoteMediaIpAsTrAddr(cmCapDataType eMediaType, ERoleLabel eRole, mcTransportAddress& remoteIp) ;
	void		SetMaxRate(DWORD rate);

	void        GetLocalMediaIpAsTrAddr(mcTransportAddress& localIp);
	DWORD		GetRemoteMediaPort(cmCapDataType eMediaType, ERoleLabel eRole) const;
	BOOL 		IsRemoteSavp() const;
	DWORD		GetRemoteMediaRtcpPort(cmCapDataType eMediaType, ERoleLabel eRole) const;
	DWORD		GetPort(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
//	WORD		GetBoardId() const {return m_pRsrcDesc->m_boardId;}
	BYTE 		CheckIsMobilePhoneByHeader(BYTE bIsMcuInitiateTransactionq);
	void		SetQos(const CQoS& qos) {*m_pQos = qos;}
	void		SetCardCallIndex(DWORD cardCallIndex);
	void		SetLocalCaps(const CSipCaps& rCaps, APIU16 plcmRequireMask = 0); //BRIDGE-11697
	void        SetRemoteCaps(const CSipCaps& rCaps);
	void        SetMaxLocalCaps(const CSipCaps& rCaps);
	void		SetLastRemoteCaps(const CSipCaps& rCaps);
	void		SetRemoteSdp(const sipSdpAndHeadersSt & remoteSdp);
	void        SetLocalSdp(const sipSdpAndHeadersSt & localSdp);
	void        FillMrdVersion();
	void        Deescalate();
	void        DeescalateChannelsByMedia(cmCapDataType aDataType);

	void        FillRemoteInformationHeader();
	bool 		StoreRemoteVendorInfo();
	RemoteIdent GetFromIdent(const sipSdpAndHeadersSt* pSdpAndHeaders);
	sipSdpAndHeadersSt* GetRemoteSdp(void);
	sipMessageHeaders *GetRemoteCallLegHeaders();
	sipMessageHeaders*	GetRmtHeaders() {return m_pCall->GetRmtHeaders();}
	DWORD		SetDialInSessionTimerHeaders(CSipHeaderList& headerList);
	DWORD		SetDialOutSessionTimerHeaders(CSipHeaderList& headerList, DWORD rmtSessionTimerMinSec);
	DWORD		GetRmtSessionTimerMinSec();
	void		SetOutBoundProxyIp(DWORD ip);
	void		SetConfParamInfo(const char* strInfo);
	void		SetTransportType(enTransportType eTransportType);
	void		AddParamToConfParamInfo(const char* strParamInfo);
	void		CompleteRemoteCapDataFromLocalCap();
	void		SetCallLegAndCdrHeaders(const sipSdpAndHeadersSt& sdpAndHeaders);
	void		SetRemoteHeaders(const sipSdpAndHeadersSt* pSdpAndHeaders);
	void		SetRejectReason(enSipCodes eReason);
	void		GetOutboundSipProxy(char* pProxyAddress);
	void        SetRemoteTypeOnNetSetup(eVideoPartyType RemoteVideoPartyType);

	void        SetIceCallOnNetSetup(BOOL isEnableSipICE);

	DWORD		GetServiceId(void){return m_serviceId;};

	// Timers
	void		OnTimerChangeMode(CSegment* pParam);
	void		OnTimerFastUpdate(CSegment* pParam);
	void		OnTimerConnectCall(CSegment* pParam);
	void		OnTimerDisconnectCall(CSegment* pParam);
	void		OnTimerCloseAllChannels(CSegment* pParam);
	void		OnMfaReqToutDisconnecting(CSegment* pParam);
	void		OnMfaReqToutAnycase(CSegment* pParam);
	virtual void		GetMfaAckAndIdentifyChannel(CSegment* pParam, ACK_IND_S* pAckIndStruct, CSipChannel** pChannel, CObjString &cLog, ConnectionID &connectionId);
	void		EnterFunctionAndOpcode(CObjString &cMainLog, const char* pString, DWORD opcode);
	BYTE		HandleMfaAckStatus(CObjString &cMainLog, DWORD status, DWORD opcode, CSipChannel* pChannel);
	void		HandleMfaAckChannelNotFound(CObjString &cLog, const char* pString, ACK_IND_S* pAckIndStruct);

	//LPR
	void SetLastRateBeforeLpr(DWORD rate){m_LastRateBeforeLpr = rate;};
	DWORD GetLastRateBeforeLpr(){return m_LastRateBeforeLpr;};
	void SendLprReqToMfa(WORD status, DWORD lossProtection, DWORD mtbf, DWORD newPeopleRate, DWORD fill, DWORD modeTimeout, BYTE isReset = 0);
	void SetLprOfRemoteCapsCapStruct(kChanneltype channelType );
	void CancelLpr();
	void SetIsLprModeOn(WORD status){ m_isLprModeOn = status;};
	WORD GetIsLprModeOn(){return m_isLprModeOn;};

	// Msg from party
	void MakeANewCall(CSipComMode* pTargetMode, ETipPartyTypeAndPosition tipPartyType, BYTE bIsFirstOfferer = FALSE/* is MCU is the first offerer in the call*/, BYTE bStartSignalingOnly = FALSE /*start signaling without open channels*/);
	void CloseCall(BYTE bIsInitiator = NA,int reason = 0);
	void ViolentCloseCall();
	void RejectCall(int reason,int warning = STATUS_OK);
	void CancelCall(ECancelType eCancelType);
	void RejectCallAndProvideAlternativeAddrToCall(int reason,const char* alternativeAddrStr);
	void SetCallDisconnectedAndRemoveFromRsrcTbl();

	// class utils
	void SetConnectionId(DWORD connId);
	CSipComMode* FindBestModeToOpen(const CSipComMode& rPreferredMode, BYTE bTryUpgradeFromSecondary,BYTE isIntersectwithMaxCaps = FALSE); //alloc memory
//	CSipComMode* FindTargetMode(const CSipComMode& rPreferredMode) const; //alloc memory
	void FindSiteAndVisualNamePlusProductIdAndSendToConfLevel();// currently user agent header.
	BYTE IsMediaMuted(cmCapDataType eMediaType, cmCapDirection eDirection, const sipSdpAndHeadersSt* pSdpAndHeaders,
								const CSipCaps* pSdpCaps, ERoleLabel eRole = kRolePeople) const;
	BYTE SetMediaMuteState(cmCapDataType eMediaType, cmCapDirection eDirection, BYTE *muteChanges, ERoleLabel eRole) const;
	void SetRsrcParams(CRsrcParams *pCsRsrcParams);
	BYTE CheckRsrcLimitation(CSipComMode* pBestMode,CSipCaps* ptmpRemoteCaps,H264VideoModeDetails& h264VidModeDetails);

	// Inds from card
	void OnSipInviteAckIndConnecting(CSegment* pParam);
	void OnCsProvisunalResponseInd(CSegment* pParam);
	void OnSipInviteAckIndDisconnecting(CSegment* pParam);
	void OnSipCancelIndConnecting(CSegment* pParam);
	void OnSipCancelIndDisconnecting(CSegment* pParam);
	void OnSipInviteResponseIndConnecting(CSegment* pParam);
	void OnSipInviteResponseIndDisconnecting(CSegment* pParam);
	void HandleReInviteResponse(mcIndInviteResponse* pReInviteResponseMsg);
	BYTE IsRejectCall(DWORD status);
	void HandleSdpFromInviteAckInd(mcIndInviteAck* pInviteAckMsg, CSipChanDifArr* pDifArr, BYTE &bRemovedAudio, BYTE &bRemovedVideo);
	EPendingTransType CheckActiveAndPendingTransactions(CSegment* pParam);
	void OnSipByeInd(CSegment* pParam = NULL);
    void OnSipByeIndConnecting(CSegment* pParam = NULL);//RFC-3261 : [15]. HOMOLOGATION: SIP_CC_TE_CR_V_003
	void OnSipByeIndDisconnecting(CSegment* pParam);
	void OnSipBye200OkInd(CSegment* pParam);
	/*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
       void OnSipBye200OkInd_Avoid(CSegment* pParam);
	/*End:added by Richer for BRIDGE-12062 ,2014.3.3*/
	void OnSipChannelsOpenedIndConnecting(CSegment* pParam);
	void OnSipChannelsOpenedIndDisconnecting(CSegment* pParam);
	void OnSipChannelsClosedInd(CSegment* pParam);
	void OnSipBadStatusInd(CSegment* pParam);
	void OnSipTraceInfoInd(CSegment* pParam);
	void OnSipInstantMessageInd(CSegment* pParam);
	void OnSipReInviteIndConnecting(CSegment* pParam);
	void OnSipReInviteIndConnected(CSegment* pParam = NULL);
	void OnSipReInviteIndChangeMode(CSegment* pParam);
	void OnSipReInviteIndDisconnecting(CSegment* pParam);
	void OnIpVideoUpdatePictureInd(CSegment* pParam);
	void OnIpVideoUpdatePictureIndDisconnecting(CSegment* pParam);
	void OnPartyMonitoringInd(CSegment* pParam);
	void OnSipInviteAckIndChangeMode(CSegment* pParam);

	//CDR_MCCF:
	void OnPartyStatisticsIndStateDisconnecting(CSegment* pParam);
	void OnPartyInMiddleOfTransactionStatisticsInd(CSegment* pParam);
	void OnPartyStatisticsIndStateDisconnectingTout(CSegment* pParam);
	void OnPartyStatisticsIndAnycaseTout(CSegment* pParam);

	void OnPartyMrmpPartyMonitoringInd(CSegment* pParam);
	void OnCmRtcpMsSvcPliInd(CSegment* pParam);
	void OnCmRtcpAvMcuDshInd(CSegment* pParam);
	void OnCmRtcpRtpFBInd(CSegment* pParam);
	void OnCmRtcpMsgInd(CSegment* pParam);
	void OnPacketsFractionLostInd(CSegment* pParam);
	void OnCsDtmfInd(CSegment* pParam);
	void OnCsDtmfIndIgnore(CSegment* pParam);
	void OnSipTransportErrorInd(CSegment* pParam);
	void OnSipSessionTimerExpire(CSegment* pParam);
	void OnSipSessionTimerReinvite(CSegment* pParam);
	void OnRtpStreamStatusInd(CSegment* pParam);
	void OnRtpDifferentPayloadTypeInd(CSegment* pParam);
	void OnRtpFeccMuteInd(CSegment* pParam);
	void OnRtpFeccTokenInd(CSegment *pParam);
	void OnRtpFeccKeyInd(CSegment *pParam);
	void OnCmBfcpTcpTransportInd(CSegment *pParam);
//	void OnMfaBadSpontaneuosInd(CSegment* pParam);
//	void OnSIPCallDynamicPTInd(CSegment* pParam);

	//CDR_MCCF:
	void OnPartyInfoStatisticsOrMonitoring(TRtpChannelMonitoringInd* pChanMonitoring, APIU32 numOfChannels, BYTE isMonitoringIndMsg = TRUE);
	void SendStatisticsInfoOfThisEpToMCMSpolycomMixer();
	void UpdateChannelStatisticsInfoIfNeeded(TRtpCommonChannelMonitoring* pChanMonitoring ,CPrtMontrBaseParams* pPrtMonitrParams);
	void SetIpVideoChannelDetails(EIpChannelType channelType, BYTE connectionStatus,
	        DWORD actualRate, mcTransportAddress* partyAddr, mcTransportAddress* mcuAddr, BYTE IsIce,
	        mcTransportAddress* IcePartyAddr, mcTransportAddress* IceMcuAddr, EIceConnectionType IceConnectionType,
	        DWORD packetsCounterIn, DWORD packetsCounterUse, WORD frameRate, int videoResolution);
	void CompareChannelStatisticsWithCurrentDbStatisticsAndUpdate(CPrtMontrBaseParams* pPrtMonitrParams);
	BYTE SendRtpVideoChannelStatisticsReq(int partyid_to_send = 0);

	// Reqs to card
	virtual DWORD ShouldInitTimerForSendMsg(DWORD opcode);
    virtual void InitAllChannelsSeqNum();
	void CreateFastUpdateMsgToCM(APIU32 mdiaType, APIU32 msgType, APIU32 tipPosition,APIU32 SeqNum, APIU32 remoteSsrc = NON_SSRC);
	BYTE SendStreamOffReq(CSipChannel* pChannel);
	BYTE SendStreamOnReq(CSipChannel* pChannel);
	BYTE PartyMonitoringReq(int partyid_to_send = 0);
	BYTE PartyMonitoringForAvMcuReq(int partyid_to_send= 0);
	BYTE MrcPartyMonitoringReq(int partyid_to_send);
	BYTE SipNewCallReq(CSipComMode* pTargetMode, ETipPartyTypeAndPosition tipPartyType);//=eTipNone);
	BYTE SipDelNewCallReq(DWORD status);
	BYTE SipInviteReq(const char* alternativeAddrStr=NULL, DWORD rmtSessionTimerMinSec = 0, BYTE isResultOfIpMediaMismatch = 0);
	BYTE SipReInviteReq(DWORD reinviteReason = 0, BYTE bIsFirstOffer = FALSE);
	BYTE SipInviteAckReq(BYTE bCloseSession = YES);
	BYTE SipCancelReq();
	BYTE SipInviteResponseReq(DWORD status,int warning = STATUS_OK,const char* alternativeAddrStr=NULL, BYTE isDialInNoSdp = NO, BYTE isBwmForIBM = NO);
	/////////////-------------CODE REFACTORING------------------------------------------------------------------------------------------------------------------------------------------
	BYTE SipInviteResponseReqBuildHeadersList(CSipHeaderList& headerList, DWORD& status,int warning = STATUS_OK,const char* alternativeAddrStr=NULL);
	BYTE SipInviteResponseReqReject(CSipHeaderList& headerList, DWORD status);
	BYTE SipInviteResponseReqAccept(CSipHeaderList& headerList, BYTE isDialInNoSdp, BYTE isBwmForIBM);
	BYTE SipInviteResponseReqAcceptOfferer(CSipHeaderList &headerList, BYTE isBwmForIBM);
	BYTE SipInviteResponseReqAcceptAnswerer(CSipHeaderList &headerList, BYTE isBwmForIBM);
	BYTE SipInviteResponseReqSendMsg(mcReqInviteResponse* pInviteResponseMsg);
	void SipInviteResponseReqSetSdpHeaderStuctFields(sipSdpAndHeadersSt* pSdpAndHeaders, int mediaLinesOffset, int mediaLinesLength, int iceOffset,
														int iceLength, int headersOffset, int headerLength, int lenOfDynamicSection);
	void SipInviteResponseReqReject(mcReqInviteResponse* pInviteResponseMsg, int msgLen, DWORD status);
	DWORD SipInviteResponseReqSetResponseMsg(mcReqInviteResponse* pInviteResponseMsg, int msgLen, DWORD status);
	void SipInviteResponseReqAnswererSetIpAndPorts(mcReqInviteResponse* pInviteResponseMsg, CSipCaps* pTempRemoteCap, vector<eMediaLineInternalType> closedChannelsCopiedTypes, vector<int> closedChannelsCopiedIndexes, BYTE isBwmForIBM = NO );
	int  SipInviteResponseReqAnswererAddMediaCap(sipSdpAndHeadersSt* pSdpAndHeaders, CSipCaps* pTempRemoteCap, int capsBufferLens, cmCapDataType eMediaType,
															int leaveMediaCap, eMediaLineSubType bfcpType, ERoleLabel eRole= kRolePeople);
	int  SipInviteResponseReqAnswererAddMediaCapsNotFromChannels(sipSdpAndHeadersSt* pSdpAndHeaders, CSipCaps* pTempRemoteCap, int capsBufferLen, BYTE isBwmForIBM = NO);
	BYTE SipInviteResponseReqAcceptAnswererSetSdpAndCopyBuffers(mcReqInviteResponse* pInviteResponseMsg, int msgLen, BYTE isBwmForIBM = NO);
	BYTE SipInviteResponseReqAcceptOffererSetSdpAndCopyBuffers(mcReqInviteResponse* pInviteResponseMsg, int msgLen);
	void SipInviteResponseReqSetSDPGenericFields(mcReqInviteResponse* pInviteResponseMsg, BYTE isBwmForIBM);
	void SipInviteResponseReqSetMediaRatesInSDP(sipSdpAndHeadersSt* pSdpAndHeaders);
	void SipInviteResponseReqLeaveMedia(BYTE bIsAnswerer, CSipCaps* pTempRemoteCap, int& leaveAudioCaps, int& leaveVideoCaps, int& leaveDataCaps,
													int& leaveContentCaps, int& leaveBfcpCaps);
	void SipInviteResponseReqLeaveMediaAnswerer(CSipCaps* pTempRemoteCap, int& leaveAudioCaps, int& leaveVideoCaps, int& leaveDataCaps,
													int& leaveContentCaps, int& leaveBfcpCaps);
	void LeaveMedia(cmCapDataType eMediaType, int& leaveMediaCaps, eMediaLineInternalType intType, CSipCaps* pTempRemoteCap);
	/////////////---------------------------------------------------------------------------------------------------------------------------------------------------------------------
	BYTE SipByeReq();
	BYTE SipBye200OkReq();
	BYTE SipRingingReq();
	BYTE SipOpenChannelsReq(CSipComMode* pTargetMode,cmCapDirection eDirection=cmCapReceiveAndTransmit,BYTE bIsAnswering=FALSE, ETipPartyTypeAndPosition tipPartyType = eTipNone);
	BYTE OpenContentRecvChannelForSrtp(CSipComMode* pTargetMode);
	BYTE SipCloseChannelReq(EIpChannelType chanType);
	BYTE SipOpenBfcpChannelReq(cmCapDirection eDirection);
	BYTE SipCloseAllChannelsReq();
	BYTE SipUpdateChannelReq(CSipComMode* pTargetMode,EIpChannelType chanArr,
							  int eUpdate, BOOL bIsReplyDiffPayload = NO, BOOL bKeepCurrentPayload = NO,WORD payload = 0);
	BYTE SipInstantMessageReq(const char* instantMessageStr);
	APIU32 CalcRateForIBM();

	CSipCall* GetCallObj(void){ return m_pCall; };
	void SetVideoParamInCaps(H264VideoModeDetails h264VidModeDetails, BYTE cif4Mpi,BYTE bIsRtv,DWORD videoRate);
	void SetMsSvcVideoParamInCaps(MsSvcVideoModeDetails MsSvcModeDetails, BYTE cif4Mpi,BYTE bIsRtv,DWORD videoRate);
	void SetLocalCapToAudioOnly();
	void SendRemoteNumbering();
	void SendUpdateMtPairReq();
	void OnPartyFeccTokenReq(feccTokenEnum eFeccTokenOpcode,WORD isCameraControl = 0);
	void UpdateSeeingMcuTerminalNum(WORD mcuNum, WORD terminalNum, PartyRsrcID partyId);
	void RemoveChannel(CSipChannel* pChannel);
	void RemoveCapSet(const CCapSetInfo& capInfo, ERoleLabel eRole = kRolePeople);
	void AdjustLocalCapToNonTip(CIpComMode* pTargetMode);

	// Open and close MFA channels requests
	int  OpenMediaChannels(EConnectionState iChannelsInState = (EConnectionState)-1);
	int  OpenInternalChannels(CSipComMode* pTargetMode, CSipComMode* pCurrentMode, EConnectionState iChannelsInState /*-1*/);
	int  OpenInternalChannelsByMedia(cmCapDataType aDataType, CSipComMode* pTargetMode, CSipComMode* pCurrentMode, EConnectionState iChannelsInState /*-1*/);
	void CloseInternalChannels(EIpChannelType chanType);
	bool UpdateMrmpInternalChannelIfNeeded();
	void Rtp_FillAndSendUpdatePortOpenRtpStruct(CSipChannel* pChannel, BOOL isInternal=FALSE, BYTE index=0);
	void Rtp_FillUpdatePortOpenRtpChannelStruct(TUpdateRtpSpecificChannelParams* pUpdateRtpSpecificChannelParams, CSipChannel *pChannel);
	void Rtp_FillAndSendUpdateRtpChannelStruct(CSipChannel *pChannel);
	void Rtp_FillAndSendUpdateRtpChannelRateStruct(CSipChannel *pChannel);
	void Cm_FillAndSendOpenUdpPortOrUpdateUdpAddrStructOutChannels(CSipChannel *pChannel, BYTE isUpdate);
	void Cm_FillAndSendOpenUdpPortOrUpdateUdpAddrStructInChannels(CSipChannel *pChannel, BYTE isUpdate);
	void Cm_FillAndSendOpenUdpPortOrUpdateUdpAddrStruct(CSipChannel *pChannel, mcTransportAddress &rmtAddress, APIU32 rmtRtcpPort, BYTE isUpdate = NO);
	BYTE Cm_FillAndSendCloseUdpPortStruct(CSipChannel *pChannel);
	void SetNewUdpPorts(UdpAddresses sUdpAddressesParams);
	void Rtp_FillMsftSvcParamsOnRtpStruct(CSipChannel* pChannel, TUpdatePortOpenRtpChannelReq* pStruct);


	// MFA indications
	void OnMfaAckConnecting(CSegment* pParam);
	virtual void OnMfaAckConnectingWithParams(ACK_IND_S* pAckIndStruct, CSipChannel* pChannel, CMedString &cLog, ConnectionID connectionId);
	void OnMfaAckConnected(CSegment* pParam);
	void OnMfaAckChangeMode(CSegment* pParam);
	void OnMfaAckDisconnecting(CSegment* pParam);
	void OnRtpDtmfInd(CSegment* pParam);
	void OnRtpDtmfIndIgnore(CSegment* pParam);

	// Intra (video) commands
	void OnSipRtpVideoUpdatePicInd(CSegment* pParam);
	void OnPartyFastUpdateReqNotConnectedState(CSegment *pParam);
	void OnSipCsVideoUpdatePicInd(CSegment* pParam);
	void FastUpdateReq(ERoleLabel eRole, ETipVideoPosition tipPosition,DWORD remoteSSRC = NON_SSRC, DWORD priorityID = INVALID, DWORD msSlavePartyIndex = 0, DWORD isSlaveRTV = FALSE);
	void SendFastUpdateReq(CSegment *pParam);
	void CreateFastUpdateMsgToMrmp();

	//FLOW CONTROL
	void SendFlowControlReq(APIU32 videoType , cmCapDirection mediaDirection,APIU32 rate);
	void SendInfoFlowControlReq(APIU32 videoType , cmCapDirection mediaDirection,APIU32 rate);
	void SendFlowControlToCM(APIU32 mediaType ,APIU32 rate);
	void OnChangeRateInd(APIS8 *label , APIU32 mediaDirection,APIU32 rate);
	void ChangeLocalCapsAndMpCallPeopleRate(DWORD newVidRate, ERoleLabel eRole);
	BYTE IsChannelSupportLpr(CSipChannel* pChannel);
	void SendH230FastUpdateToParty(kChanneltype channelType, cmCapDirection channelDirection, BYTE bIsGradualIntra = FALSE, ETipVideoPosition tipPosition = eTipVideoPosLast);
	void SendH230FastUpdateToPartyMS2013(kChanneltype channelType, cmCapDirection channelDirection, BYTE bIsGradualIntra = FALSE, ETipVideoPosition tipPosition = eTipVideoPosLast, DWORD remoteSsrc = NON_SSRC, DWORD prID=INVALID );
	void OnTimerSendRtcpFlowControl();
	void SetPreferTIPFlowCtrlRateIfNeeded(APIU32& rate, APIU32 videoType);//BRIDGE-14087
	//void OnTimerSendMsgToSipProxy();

	//LPR
	void OnSipLprChangeModeInd(mcIndSipLPRModeChange* sLprModeChangelInd, APIU32 dTxId);
	void OnSIPLprTout(CSegment* pParam);
	void MfaSIPLprAck();
	void SendInfoLprReq(APIU32 lossProtection , APIU32 mtbf,APIU32 congestionCeiling ,APIU32 fill,APIU32 modeTimeout);
	void OnRtpLprChangeModeInd(CSegment* pParam);
	void OnRtpLprTout(CSegment* pParam);

	//LYNC2013_FEC_RED:
	void SendStartRedToCM();
	BYTE IsChannelSupportFec(CSipChannel* pChannel);
	BYTE IsChannelSupportRed(CSipChannel* pChannel);
	//void SetLastRateBeforeRED(DWORD rate){m_LastRateBeforeRED = rate;};
	//DWORD GetLastRateBeforeRED(){return m_LastRateBeforeRED;};
	//void SetBitRateWithRED(DWORD rate){m_BitRateWithRED = rate;};
	//DWORD GetBitRateWithRED(){return m_BitRateWithRED;};
	void SetTheChangeOfRateForRED(DWORD rate){m_ChangeOfRateRED = rate;};
	DWORD GetTheChangeOfRateForRED(){return m_ChangeOfRateRED;};
	BOOL GetIsRedOn(){return m_bIsRedOn;};
	void SetIsRedOn(BOOL bOnOff){m_bIsRedOn = bOnOff;};
	BOOL GetIsRedStarted(){return m_bIsCMStartRed;};
	void SetIsRedStarted(BOOL bOnOff){m_bIsCMStartRed = bOnOff;};
	BOOL GetIsActiveFecFlow(){return m_isActiveFecFlow;};
	void SetIsActiveFecFlow(BOOL bOnOff){m_isActiveFecFlow = bOnOff;};
	BOOL GetIsActiveRedFlow(){return m_isActiveRedFlow;};
	void SetIsActiveRedFlow(BOOL bOnOff){m_isActiveRedFlow = bOnOff;};
	void HandleRedPacketLossInd(DWORD newRedPercent,DWORD mediaType,DWORD ssrc);

	//FEC - (rtc or mssvc)
	void SendStartFecToCM(BOOL isFecStartedBecauseOfLync2013);
	void HandleFecPacketLossInd(DWORD newFecPercent,DWORD mediaType,DWORD ssrc);
	//void SetLastRateBeforeFEC(DWORD rate){m_LastRateBeforeFEC = rate;};
	//DWORD GetLastRateBeforeFEC(){return m_LastRateBeforeFEC;};
	//void SetBitRateWithFEC(DWORD rate){m_BitRateWithFEC = rate;};
	//DWORD GetBitRateWithFEC(){return m_BitRateWithFEC;};
	void SetTheChangeOfRateForFEC(DWORD rate){m_ChangeOfRateFEC = rate;};
	DWORD GetTheChangeOfRateForFEC(){return m_ChangeOfRateFEC;};
	void SendFecOrRedReqToART(DWORD type=cmCapVideo, WORD status=statOK);  //2=video
	BOOL GetIsFecOn(){return m_bIsFecOn;};
	void SetIsFecOn(BOOL bOnOff){m_bIsFecOn = bOnOff;};
	BOOL GetIsFecStarted(){return m_bIsCMStartFec;};
	void SetIsFecStarted(BOOL bOnOff){m_bIsCMStartFec = bOnOff;};
	void CloseFecOrRedDueToChannelClose(CSipChannel *pChannel);
	DWORD GetFecFractionLossInPercent() {return m_FecFractionLossInPercent;};
	void SetFecFractionLossInPercent(DWORD fecFractionLossInPercent) {m_FecFractionLossInPercent = fecFractionLossInPercent;};

	// Video quality indication
	void  OnSipPacketLostStatusConnected(CSegment* pParam);
	void  OnSipPacketLostStatusImproperState(CSegment* pParam);
	void  PropagatePacketLostStatus(const eRtcpPacketLossStatus InLoss, const eRtcpPacketLossStatus OutLoss, const BYTE InLpr, const BYTE OutLpr);

	//PLCM IVR
	void SendIvrProviderEQReq(const char* numericConfId);

	// Error handling Party CS keep alive timers
	void  OnPartyCsErrHandleKeepAliveFirstTout(CSegment* pParam);
	void  OnPartyCsErrHandleKeepAliveSecondTout(CSegment* pParam);
	void  OnPartyCsErrHandleKeepAliveInd(CSegment* pParam);
	void  StartCsPartyErrHandlingLoop();
//	void  OnSipCsBfcpMessageInd(CSegment* pParam);
//	void  OnSipCsBfcpTransportInd(CSegment* pParam);
	void  SendBfcpMessageReq(DWORD opcode, BYTE mcuId, BYTE terminalId);

	// MCU INTERNAL PROBLEM
	void TranslateAckToMipErrorNumber(MipHardWareConn& mipHwConn, MipMedia& mipMedia, MipDirection& mipDirect,
									   MipTimerStatus& mipTimerStat, MipAction& mipAction, ACK_IND_S* pAckIndStruct = NULL, CSipChannel* pChannel = NULL
									   ,BYTE isChanNotFound = 0);
	DWORD GetMipErrorNumber(ACK_IND_S* pAckIndStruct = NULL, CSipChannel* pChannel = NULL,BYTE isChanNotFound = 0);


	H264VideoModeDetails GetH264ModeAccordingToRemoteVideoPartyType(eVideoPartyType videoPartyType,DWORD partyRate, BYTE isRtv=FALSE)const ;
	eVideoPartyType GetMaxRemoteVideoPartyType(CSipCaps* pRemoteCaps,BYTE isRtv = FALSE)const;

	// IpV6
	enIpVersion CheckForMatchBetweenPartyAndUdp(enIpVersion eIpVer,eIpType eipType) const;
	enIpVersion GetIpAddrMatch() const;
	BYTE FindMatchingIpV6MediaAddressByScopeId();
	// Media IP mismatch
	void HandleMediaIpAddrVersionMismatch(BYTE isNeedToResetCallIndex = 0);
	BYTE checkForMediaIpVersionMismatch(const sipSdpAndHeadersSt* pSdpAndHeaders);

	// DNS
	void OnDNSResolveInd(CSegment* pParam);
	void OnDnsPollingTimeout(CSegment* pParam);
	void SendStartDnsTimer(DWORD serviceId);
	void ResolveDomainReq(DWORD serviceId, const char* pProxyAddress);
	void MakeDnsQuary();
//	void OnAllChannelsAreClosedInChangeMedia();
	int  ReOpenMediaChannels(EConnectionState iChannelsInState = kUnknown);
	void handleSwitchedMediaNewSession();
	// INFO MESSAGES
	void OnPartyInfoUnionInd(CSegment* pParam);

	void OnPartyInfoRespInd(CSegment* pParam);

	//handling payload types
	void UpdatePayloadTypeInRecieveMediasWhenAnswering();
	void SendUpdateChannelForDiffPayloadIfNeeded(CSipComMode* pTargetMode);

	//PartyPreview
	void SendStartPartyPreviewReqToCM(DWORD RemoteIPAddress,WORD VideoPort,cmCapDirection Direction,CapEnum capEnum);
	void SendStopPartyPreviewReqToCM(cmCapDirection Direction);

	void UpdateRtpOnLeaderStatus(BYTE isLeader);

	void SetCopVideoTxModes(CCopVideoTxModes* pCopVideoModes);
	void SetLocalVideoCapsExactlyAccordingToScm(CIpComMode* pScm);

	//SDES
	void SetLocalSdesKeysAndTag(CIpComMode *targetMode,CIpComMode *targetModeMax);
//	void UpdateLocalSdesTag(cmCapDataType dataType, uint tag, CIpComMode *targetMode, ERoleLabel eRole = kRolePeople);
	void UpdateLocalCapsSdesTag(CSipComMode *pTargetMode);
	void SetIsEnableICE(BYTE flag);
	BYTE GetIsEnableICE() const;

	WORD  SendRemoteICESdp(DWORD opcode,CSipChanDifArr* pIceChannelDifArr,BOOL IsChangeNeeded,BYTE IsSecondary,BOOL IsEncryptedCall);
    void  SipIceMakeOffer(DWORD Opcode,CSipChanDifArr* pIceChannelDifArr,BOOL IsChangeNeeded);
  	void  SetIceParams(const ICE_GENERAL_IND_S* pStruct);
  	DWORD HandleIceIndication(CSegment* pParam, bool is_ms_ordering=false);
	void  OnMfaICEAnswerInd(CSegment* pParam);
	void  OnMfaICEOfferInd(CSegment* pParam);
    void  OnMfaICEModifyAnswerInd(CSegment* pParam);
    void  OnMfaICEModifyOfferInd(CSegment* pParam);
    void  OnMfaICEBandwidthEventInd(CSegment* pParam);

    void  ContinueToEndCloseChannels();

	//void   GetLocalPortsFromSdp(char* sdp,DWORD sdpSize);
    void  OnMfaICEProcessAnswerInd(CSegment* pParam);
    void  OnMfaICEReinviteDataInd(CSegment* pParam);
    void UpdateIceChannelsIds(mcReqCmCloseUdpPort* pUdpSt,CSipChannel* pChannel);
    void UpdateIceChannelsIds(MrmpCloseChannelRequestMessage* pStruct, CSipChannel* pChannel);
    void CloseIceSession();
    CIceParams*	GetICEParams(){return m_pIceParams;}

    BYTE GetIsIceCall () {return m_IsEnableICE;}
    enIceConnectivityCheckStatus GetIceConnectivityStatus () { return m_IceConnectivityCheckStatus;}
    void SetIceConnectivityStatus (enIceConnectivityCheckStatus status) {m_IceConnectivityCheckStatus=status;}

    void SetNeedUpdateIceToNonIce(BYTE bNeedUpdateIceToNonIce) {m_bNeedUpdateIceToNonIce = bNeedUpdateIceToNonIce;}
    BYTE GetIsNeedUpdateIceToNonIce() const {return m_bNeedUpdateIceToNonIce;}

    void GetOriginalRmtIpAddress(cmCapDataType eMediaType, mcTransportAddress& remoteIp, ERoleLabel eRole = kRolePeople);
    void DumpUdpAddresses();
    UdpAddresses GetUdpAddressParams() {return m_UdpAddressesParams;};

    void  SetChosenCandidates(chosenCandidatesSt Candidates,cmCapDataType eType);
    char* GetMediaChosenRemoteIpAddress(cmCapDataType eType);
    DWORD GetMediaChosenRemotePort(cmCapDataType eType);
    EIceConnectionType GetMediaChosenRemoteType(cmCapDataType eType);
    char* GetMediaChosenLocalIpAddress(cmCapDataType eType);
    DWORD GetMediaChosenLocalPort(cmCapDataType eType);
    EIceConnectionType GetChannelChosenLocalType(cmCapDataType eType);



    EPendingTransType IsPendingTrns();
    void SetIsReInviteTransaction(BOOL isReinvite) {m_bIsReInviteTransaction=isReinvite;}
    void RemovePendingReInviteTrns();


	// ppc
	void SetMediaLinesInternalTypeForRmtSdp(sipSdpAndHeadersSt &sdp, const CCommConf* pCommConf = NULL);
	void SetCapsRolesForRmtSdp(sipSdpAndHeadersSt &sdp, eMediaLineInternalType mlineIntType, ERoleLabel eRole);
	void OrderMediaLines(sipSdpAndHeadersSt &sdp);
	void OrderMediaLinesByPlcmRequireTags(sipSdpAndHeadersSt &sdp);
	void AddNotSupportedMediaLines(int numberOfNotSupported, sipMediaLinesEntrySt* pCapabilities) const;
	void AddPanoramicMediaLines(int numberOfVideoMlines, sipMediaLinesEntrySt* pMediaLinesEntry) const;
	void OrderMediaLinesWithPanoramic(sipSdpAndHeadersSt &sdp);
	void SetFloorParamsInLocalCaps();
	void SetLabelsAndContentAttributesInLocalSdp(sipSdpAndHeadersSt &sdp);
	void SetLync2013SpecificMLinesAttributes(sipSdpAndHeadersSt &sdp);
	int GetPanoramicMLineIndex(const sipSdpAndHeadersSt& sdp) const;
	char* GetRemoteStreamLabel(ERoleLabel eRole) const;
	char* GetLocalStreamLabel(ERoleLabel eRole) const;
	void UpdatePresentationOutStream();
	BYTE HandleContentStreamStateForRtpAck(CSipChannel* pChannel);
	eContentState GetContentInStreamState() const { return m_eContentInState;}
	void SetContentInStreamState(eContentState contentInStreamState) { m_eContentInState = contentInStreamState;}
	void SendContentOnOffReqToRtp();
	void SendEvacuateReqForRtpOnH239Stream();
	void HandleContentOnOffAck(ACK_IND_S* pAckIndStruct);
	void HandleRtpEvacuateAck(ACK_IND_S* pAckIndStruct);
	void SetRateForLocalSdp(sipSdpAndHeadersSt &sdp, eMediaLineInternalType sipMediaLineIntType, DWORD bitRate, int mediaCap = -1);
	DWORD GetFullContentRate() const { return m_fullContentRate;}
	void SetFullContentRate(DWORD contentRate) { m_fullContentRate = contentRate;}
	void AddContentCapIfNeeded(CIpComMode *pScm, CCapSetInfo capInfo);
	void RemoveBfcpCaps();
	void RemoveFeccCaps();
	void RemoveContentCaps();
	void RemoveBfcpAndContentCaps();
	void SetIsContentSpeaker(BYTE bIsContentSpeaker) {m_bIsContentSpeaker = bIsContentSpeaker;}
	void SetIsNeedToReleaseTokenForHoldTip(BYTE bIsRelease) {m_isNeedToReleaseUponHoldForTip = bIsRelease;}
    BYTE GetIsContentSpeaker() const {return m_bIsContentSpeaker;}
    BYTE GetIsNeedToReleaseTokenForHoldTip() const {return m_isNeedToReleaseUponHoldForTip;}
    void OnSIPPartyContentSpeakerChange(BYTE bIsSpeaker, DWORD curConfContRate);

    void OnSipRtpSelfVideoUpdatePicReq(CSegment* pParam);

    //RTV RTCP
    void SendRtcpVideoPreferenceReq(DWORD Width,DWORD Height);
    void SendRTCPBandwidthLimitation(APIU32 rate);
    DWORD GetCopMaxCallRate();

    // TIP
//	void SetIsTipCall(BYTE);
	BYTE GetIsTipCall() const;
	BOOL IsTipRtcpMessage(OPCODE opCode);
	BOOL IsVsrRtcpMessage(OPCODE opCode) const {return (opCode == IP_CM_RTCP_VSR_IND);}
	BOOL IsVsrInitMessage(OPCODE opCode) const {return opCode == CONFPARTY_CM_INIT_ON_LYNC_CALL_REQ || opCode == CONFPARTY_CM_INIT_ON_AVMCU_CALL_REQ;}
	void StartTipNegotiation();
	void EndTipNegotiation(ETipNegStatus eStatus);
	void SendTipCallMessageToMPL(DWORD masterId, DWORD leftId, DWORD rightId, DWORD audioAuxId, DWORD opcode);
	void TipReInviteReq(WORD, BYTE);
	void HandleMuxTipAck(ACK_IND_S* pAckIndStruct);
	BYTE CheckPartyMonitoringReq();
	void CloseTipSessionIfNeeded();
	void SetITPRtcpMask();
	void RetriveCNAMEInfoIfNeeded(CSegment* pParam);
	APIU16 RetriveCnameInfoFromEpIfPosible(char *pCnameString);
	void BuildCnameStringFromMask(APIU16 mask,char *fullString);
	APIU16 RetriveMaskAndNameFromEpIfPosible(char *pCnameString,BYTE bIsProductId, char* productId, BYTE bIsVersionId, char* VersionId);
	void SendTokenMessageReq(DWORD opcode, BYTE mcuId, BYTE terminalId);
	void SetRemoteCapsTipAuxFPS(ETipAuxFPS tipAuxFPS);
	void AddTipNegotiationCapsToRemoteCaps();
	BYTE UpdateRemoteCapsForTipNegRes();
	void TipPrepareRatesInLocalCaps(WORD nStreams, BYTE bIsVideoAux);
	BOOL IsUndefinedParty();
	BOOL GetConfEncryptionMode();
	BOOL IsWhenAvailableEncryptionMode();
	BOOL IsEncryptAllMode();
	int GetContentCapsCountInSentSdp();
	BOOL GetRecevRingback();
	void SetRecevRingback(BOOL flag);
	void SetIsResuming(BOOL isResuming) {m_bIsResuming = isResuming;}
	BOOL GetIsResuming() {return m_bIsResuming; }

	// BFCP
	void CreateSipBfcpCtrl();
	void CreateAndInitSipBfcpCtrl();
	BOOL IsSipBfcpCtrlMessage(OPCODE opCode);
	eMediaLineSubType GetBfcpType() const;
	void UpdateBfcpTransportType(enTransportType transType, BOOL isUpdateBfcpCntl = TRUE);

	WORD GetBFCPcapConfIDfield() const {return m_BFCPcapConfIDfield;}
	WORD GetBFCPcapUserIDfield() const {return m_BFCPcapUserIDfield;}

	void SetBFCPcapConfIDfield(WORD BFCPcapConfIDfield) { m_BFCPcapConfIDfield = BFCPcapConfIDfield;}
	void SetBFCPcapUserIDfield(WORD BFCPcapUserIDfield) { m_BFCPcapUserIDfield = BFCPcapUserIDfield;}

	eBfcpMStreamType GetMStreamType();
	BOOL IsBfcpCtrlCreated() {return (m_pSipBfcpCtrl != NULL);}
    void SetBfcpCap(const CIpComMode* pScm);
    void AddBfcpOnFallback(CIpComMode* pPartyScm1, CIpComMode* pPartyScm2=NULL);
    void SetContentCap(const CIpComMode* pScm);

	void DeclareOnContentFromScmOnly(BYTE bIsToDeclareFromScmOnly);
	void UpdateReceivedBfcpProtocol();
	void RemoveH263FromRemoteIfNeeded(CSipChanDifArr* pDifArr);
	eSipBfcpMode4DialOut GetBfcpMode4DialOut() const;
	void SendReqToSipProxy(CSegment *pSeg, OPCODE opcode);
	void OnDialogRecoveryMessageInd(CSegment *pSeg);
	void OnDialogRecoveryFromProxy(CSegment *pSeg);
	void OnDialogRecoveryMessageIndDisconnecting(CSegment *pSeg);
	void SendDialogRecoveryMsgToCS(DWORD status);
	void OnDialogRecoveryTimeOut(CSegment *pSeg);
	DWORD GetRmtMsKeepAliveTimeOut();
    void GetKeppAliveParameters ();
	void OnMsKeepAliveClientTimeOut(CSegment *pSeg);
	void OnMsKeepAliveServerTimeOut(CSegment *pSeg);
	void OnMsSocketActivityInd(CSegment *pSeg);
	void OnMsKeepAliveErrInd(CSegment *pSeg);
    void vStoreTcpAliveParameters();
    void crlf_vGetKaParametersFromFormatedString(  char  * szMsKeepAlive_Data, DWORD * pKa_Frequency, DWORD * pnKa_PongTimeout, DWORD * pnKa_Behavior, DWORD * pnKa_Tipe);

	void SipBfcpReconnect();

	void SetRemoteIdent(RemoteIdent rmtIdent);

	BYTE GetIsICESessionIndex(void){return m_bIsIceHasSessionIndex;}

	void SetRtcpMaskIntoCall(APIS32 rtcpFeedback);
	// field 7091
	APIS32 GetRemoteRTCPMaskfromVideoCap(ERoleLabel role);

	BYTE CheckIfCallIsResumed(CSipChanDifArr* pDifArr);
	BYTE CheckIfCallIIsPutOnHold(CSipChanDifArr* pDifArr);
	enMediaOnHold CheckIfCallisResumedByMedia(CSipChanDifArr* pDifArr);
	enMediaOnHold CheckIfCallisPutOnHoldByMedia(CSipChanDifArr* pDifArr);

	BYTE GetIsMrcCall() const {return m_bIsMrcCall;}
	void SetIsMrcCall(BYTE bIsMrcCall) {m_bIsMrcCall = bIsMrcCall;}

	void OnMrmpRtcpFirInd(CSegment* pParam);
	void FillAndSendMrmpRtcpFirStruct(RelayIntraParam *pIntraParam, DWORD channelHandle);

	DWORD GetChannelHandle(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole) const;
	// SCP
	void OnScpStreamsRequest(CSegment* pParam);
	void OnScpStreamsNotificationFromEpInd(CSegment* pParam);
	void OnScpStreamNotificationAckInd(CSegment* pParam);
	void SendAckForScpReq(unsigned int aSequenceNumber);
	void SendAckFroScpNotificationInd(CSegment* pParam);

	// open/close SVC channel
	void OpenSvcChannel(CSipChannel *pChannel, mcTransportAddress &rmtAddress, APIU32 rmtRtcpPort,BYTE mode ,BYTE isUpdate = NO);
	void UpdateRelayChannel(CSipChannel *pChannel);

	CSipChannel* ShouldOpenMRMPAvcChannel(CSipChannel *pChannel,int& chnType);
	void CloseSvcChannel(CSipChannel *pChannel, BYTE mode); /* for softMcu eyaln_to_merge */

	//
	virtual void SendScpNotificationReq(CSegment* pParam);
	virtual void SendScpPipesMappingNotificationReq(CSegment* pParam);
	void SendScpIvrStateNotificationReq(CSegment* pParam);
	CSipChannel *GetChannelEx(cmCapDataType eMediaType, cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
	void SipCloseSingleChannelsReq(CSipChannel* pChannel,BYTE& bNoChannels);
	void InitSpeakerParams();
	void OnRelayTimerFastUpdateConnected(CSegment* pParam); //intra for VSW RPD=>HDX
	void OnRelayTimerFastUpdateNotConnected(CSegment* pParam); //intra for VSW RPD=>HDX

    BOOL GetQosParams(const char *& precedDomain, BYTE& precedRPrio) const;
    void  SetConfMediaType(eConfMediaType aConfMediaType);

	//LyncCCS: PartyAuth
	void 			OnSipPartyAuthInfoInd(mcIndConfPwdInfo *sConfPwdInfo , APIU32 dTxId);
	enPartyAuthState  GetPartyAuthState(){return m_ePartyAuthState;};
	BYTE 			SetPartyAuthState(enPartyAuthState  eState);
	BYTE			SetPartyAuthPwd(char* pPwd);
	BYTE			CheckPartyAuthPwd(char* pPwd);
	BYTE			SendPartyAuthRequired();
	BYTE			SendPartyAuthResponse(APIU32 transId, APIS32 status= SipCodesOk);
	BYTE			SendPartyAuthStatus(enPartyAuthRespStatus  authStatus = ePartyAuthRespInvalid);
	BYTE 			PartyAuthIndReceiveFromConf(char* pstrPwd);
	BYTE  			PartyAuthAckReceiveFromCS();
	//LyncCCS
	BYTE GetIsCCSPlugin() const {return m_bIsCCSPlugin;}
	void SetIsCCSPlugin(BYTE bIsCCSPlugin) {m_bIsCCSPlugin= bIsCCSPlugin;}


	//CAC
	int GetCacBwLimitation(){return m_AllocatedBandwidth;}

    void SetTargetModeBeforeDowngrade(CIpComMode* originalTargetMode) { *m_pTargetModeBeforeDowngrade = *originalTargetMode; }
    CIpComMode* GetTargetModeBeforeDowngrade() { return m_pTargetModeBeforeDowngrade; }
    void RemoveUnsupportedSdesCapFromLocalCaps(APIU16 cryptoSuite, BOOL bIsMkiInUse , cmCapDataType mediaType, ERoleLabel eRole);
    void UpdateSdesTagFromBestModeToLocalCaps(CSdesCap *sdesCap, cmCapDataType mediaType, ERoleLabel eRole);
    void RemoveUnsupportedSdesCapsForMrcCall();
    void RemoveUnsupportedSdesCapsFromAllLocalCaps(APIU16 supportedCryptoSuite, BOOL bSupportedMkiInUse);
    void RemoveUnsupportedSdesCapsForCiscoCallIfNeeded();
    void RemoveUnsupportedSdesCapsForRadVisionCallIfNeeded();
    void SetIsCiscoTagExist(BOOL bIsCiscoTagExist);
    BOOL GetIsCiscoTagExist();
	//added for ANAT begin
	void DecideAnatSelectedIpVersion(sipSdpAndHeadersSt* pSdpAndHeaders = NULL);	
	enIpVersion GetAnatSelectedIpVersion(){ return m_AnatSelectedIpVersion;}
	BYTE IsAnatSupported() const {return ((m_AnatSelectedIpVersion != enIpVersionMAX) ? TRUE : FALSE); }
	//added for ANAT end

    void SipStartDtlsChannelReq(cmCapDirection channelDirection/*EIpChannelType chanArr*/);
    void OnCmDtlsEndInd(CSegment* pParam);
    void SipCloseAllDtlsChannelsAfterDtlsFailureReq(/*EIpChannelType chanArr*/);
    void OnCmTipEarlyPacketInd(CSegment* pParam);
    void CopySdesCapsFromClassToStruct(sdesCapSt *pToSdesCap, CSdesCap *pFromSdesCap);
    BYTE CopySdesCapsFromStructToClass(CSdesCap *pToSdesCap, sdesCapSt *pFromSdesCap);
    void ReActivateContentNotificationForTip();
    void RemoveSdesCapFromLocalCaps(cmCapDataType mediaType, ERoleLabel eRole);
    void UpdateLocalCapsSdesUnencryptedSrtcp(CSipComMode * pBestMode, cmCapDataType eMediaType, ERoleLabel eRole);
    BYTE SipCloseDtlsChannelReq(CSipChannel* pChannel);

    const char* GetDtmfForwardSource();
    void    OnMediaDisconnectDetectionInd(CSegment* pParam);
    void    OnMediaDisconnectDetectionIndConnecting(CSegment* pParam);
    void    OnMediaDisconnectDetectionIndInAnycase(CSegment* pParam);

    void OnMediaResume(CSegment* pParam);
    void MediaDisconnectionTimerKick();
    
    void UpdateArtWithSsrc(DWORD ssrc);
    void HandleArtUpdateWithSsrcAck(ACK_IND_S* pAckIndStruct);
	bool GetMfaAckOnArtStatus(CSegment* pParam);
    void HandleInternalArtAck(ACK_IND_S *pAckIndStruct, ConnectionID connectionId);
    mediaModeEnum ConfigureMediaModeForParty(CSipChannel* pChannel,kChanneltype channelType);
	void SipUpgradeSvcChannelReq(CSipComMode* pTargetMode,EIpChannelType chanArr,
							  int eUpdate, BOOL bIsReplyDiffPayload = NO, BOOL bKeepCurrentPayload = NO,WORD payload = 0);
	BYTE SipUpgradeAvcChannelReq(CSipComMode* pTargetMode,CSipComMode* pCurrentMode);
	 void ResetReqCounter(){m_MfaReqCounter=0;}
	 void PrintChangeModeWithinTransactionValue();
     void ResetChangeModeWithinTransactionValue();
  
    BYTE GetASSIPContent(void) { return m_isASSIPContentEnable; }
    void SetASSIPContent(BYTE bisASSIPContentEnable) { m_isASSIPContentEnable = bisASSIPContentEnable; }

    BYTE IsASSIPContentDisabledinASSIPConf();
    BYTE IsAsSipConf();
    BYTE IsCallGeneratorConf();

    void  MrmpStreamIsMustReq(CSegment* pParam);
    void  GetSipUsernameFromUrl(char* sipUsername, char* sipUrl, APIU32 maxLen);
    void RemoveIceParamsObject();
    BYTE SipCloseAllDtlsChannelsReq();
    void SetLocalSdesKeysAndTagByRemote(int sdesTag, char *pSdesKeySymmetric, CIpComMode *targetMode, CIpComMode *targetModeMax, cmCapDataType eMediaType, ERoleLabel eRole);
	void SetLocalSdesKeysAndTagByHost(CIpComMode *targetMode, CIpComMode *targetModeMax, cmCapDataType eMediaType, ERoleLabel eRole);
	
	void  UpdateVideoInStreamsList(const std::list <StreamDesc>& streamsDescList) ;
    //MSFT 2013 SVC
    void initVsrCtrl();
    void initAVMcuVsrCtrl();
    void initLyncClentMcuVsrCtrl();
    inline enMsft2013ActivationState isMs2013Active(){return m_isMs2013Active;}
    void SetIsMs2013(enMsft2013ActivationState isMs2013);
    BOOL FillVsrInfo(ST_VSR_SINGLE_STREAM* vsrArr, DWORD vsrNum);
    void SendSingleVsr();
    void SendMultiVsr(ST_VSR_MUTILPLE_STREAMS& multiVsr);
    inline BOOL isVsrCtrlInitialized(){return m_bIsVsrCtrlInitialized;}
    void setMsftStreamOnState(BOOL isStreamOn){ m_bShouldMsftVideoTxStreamOn = isStreamOn;};
    void videoInSynched(DWORD partyIndex);
    void trigerMsftRcvVsr();
    void MsftSendMsgToMux(mcMuxLync2013InfoReq& msSvcMuxMsg);
    void SetMsftTotalBW(DWORD bw);
    void SetCallConnected();
    void CreateFakeIceCandidates();

    // BRIDGE-9740
    void IntersectSirenLPRLocalAndRemoteCaps(cmCapDataType eMediaType, WORD payload, ERoleLabel eRole);

    // Cascade:
    BYTE GetCascadeMode() const;
    BYTE IsRemoteSlave() const;
    BYTE IsRemoteMaster() const;
    BYTE  SendRecordingControlCmd(const char* szControlCmd);	
    enSrsVideoLayoutType   GetInitVideoLayoutForRL(sipSdpAndHeadersSt* pSdpAndHeaders = NULL);
   void   OnSipPartySrsStatusInd( mcIndSrsIndicaton *pstRssCmdStatus);
   void     OnSipPartySrsLayoutInd(mcIndSrsVideolayout *pstRssLayoutCmd);
   
    void SetStateOnEndOfTransaction();
    void SetIsOneDtlsArrived(BOOL set){ m_bIsOneDtlsArrived = set;};
    bool CheckAndStoreRemoteOriginVersionField();
    void FillRemoteCapsFromReInvite();

    bool    IsSameTimeEP() const {return ((m_remoteIdent == IbmSametimeEp) || (m_remoteIdent == IbmSametimeEp_Legacy));}

    inline void AllowMediaBasedDisconnectionInReinvite() {m_allowMediaBasedDisconnectionInReinvite = TRUE;}
    bool IsPartyInDiffPayLoadType();
    BOOL GetIsST852Resume() const {return m_IsST852Resume;}

protected:

	void SetCallDisconnecting();
	void OnAllChannelsAreClosed();

	BYTE IsInviteInitiator() const;
	void SetLocalSipHostAddress();
	void StartFastUpdateTimer();
	void AnalyzeDifferencesInSdp(const sipSdpAndHeadersSt* pSdpAndHeaders,const CSipCaps* pSdpCaps,CSipChanDifArr* pDifArr);
	void AnalyzeDifferencesInSdpCloseChannels(const sipSdpAndHeadersSt* pSdpAndHeaders, const CSipCaps* pSdpCaps,CSipChanDifArr* pDifArr);
	void AnalyzeDifferencesInSdpNewChannelsIncludingMute(const sipSdpAndHeadersSt* pSdpAndHeaders,const CSipCaps* pSdpCaps,CSipChanDifArr* pDifArr);
	void AnalyzeDifferencesInSdpUpdateChannelsIncludingMuteState(const sipSdpAndHeadersSt* pSdpAndHeaders,const CSipCaps* pSdpCaps,CSipChanDifArr* pDifArr);
	void AnalyzeDifferencesInSdpCmChanges(const sipSdpAndHeadersSt* pSdpAndHeaders, CSipChanDifArr* pDifArr);
	DWORD GetVideoRateOverflowFix(BYTE bCheckUserAgentHeader, CBaseVideoCap* cVideoCapForFix);
	BYTE GetSelfFlowControl(void) { return m_bSelfFlowControl; }
	void SetSelfFlowControl(BYTE bSelfFlowControl) { m_bSelfFlowControl = bSelfFlowControl; }
	DWORD GetOriginalVideoRate(void) {return m_wOriginalVideoRate;}
	void SetOriginalVideoRate(DWORD wOriginalVideoRate) {m_wOriginalVideoRate = wOriginalVideoRate;}
	BYTE ReduceVideoRate(BYTE bCheckUserAgentHeader, sipSdpAndHeadersSt *pSdpAndHeaders);
	BYTE RestoreVideoRate(sipSdpAndHeadersSt *pSdpAndHeaders);
	DWORD GetSIPMsgTimeout();
	void SendSIPMsgToCS(DWORD opcode, void* pMsg, size_t size);

	enIMSSipPsiSource CheckIMSPublicServiceIdentitySource();
	void BuildIMSAssertedHeader(CSipHeaderList& headerList, const char* strFromDisplay, const char* strFromAddr);
	void BuildIMSChargingHeader(CSipHeaderList& headerList, const char* strLocalAddr);
	void BuildIMSRouteHeader(CSipHeaderList& headerList, const char* strFromAddr);
	void BuildIMSHeaders(CSipHeaderList& headerList, const char* strFromDisplay, const char* strFromAddr, const char* strLocalAddr);
    void SetAlternativeTransportType(const char* strAltToAddr);
	void BuildRepresentableSiteName(char* siteName);
	void RemoveOpeningAndEndingInvertedCommas(char* siteName);
	void UpdateDbLprSyncStatus(BYTE isSynced, BYTE isLocal);
	void UpdatePayloadTypeInRecieveChannelsAccordingToRemoteCaps();
	void UpdateDtmf2833PayloadTypeAccordingToRemoteCap(CSipChannel* pChannel);


	// patch for Microsoft MOC R2
	int  GetNewMOCStateAccoringToSdp(sipSdpAndHeadersSt* pSdpAndHeaders);
	BYTE IsRenderingHeader(sipSdpAndHeadersSt* pSdpAndHeaders);
	void SaveAudioAndVideoParams(sipSdpAndHeadersSt* pSdpAndHeaders);
	void BuildCandidateList(ICE_GENERAL_REQ_S* pStruct,CSipChanDifArr*	pIceChannelDifArr);
	void OnMfaICECloseSessionInd(CSegment* pParam);
	void OnMfaICECloseSessionIndDisconnecting(CSegment* pParam);
    void CompleteDataFromOtherCapUsingUserAgent(CSipCaps* remoteCaps);
	DWORD BuildIceSessionPart(BYTE* Buffer);
	void OnMfaICEErrInd(CSegment* pParam);
    void OnMfaICEConnectivityCheckComplete (CSegment* pParam);
	BOOL GetDisplayNameFromIdentity(char* siteName);

	void LoggedNullActionFunction(CSegment*);

    //RTCP
    void OnCmRtcpVideoPreferenceInd(CSegment* pParam);

    void OnCmRtcpReceiverBandwidthInd(CSegment* pParam);
    void OnCmRtcpVideoPreferenceIndAnycase(CSegment* pParam);
    void BandwidthAllocationStatus(DWORD reqBandwidth, DWORD allocBandwidth );
    void OnMfaICEInsufficientBandwidthEventInd(CSegment* pParam);
    void OnMfaICESessionIndexInd(CSegment* pParam);

    BYTE CheckIfNeedToUpdateRateForTipCall();

    int AddClosedMediaLinesFromRemoteSDP(sipSdpAndHeadersSt* pSdpAndHeaders, vector<eMediaLineInternalType>* pCopiedTypes,BYTE isNotCopyFullRemovedContentLineTip  = FALSE) const;
	int GetClosedMediaSize(BYTE isNotCopyFullRemovedContentLineTip = FALSE) const;
	int CopyMLineFromRemoteSDP(sipMediaLinesEntrySt* pMediaLinesEntry, eMediaLineInternalType mediaType, int mediaLinePos, int index=-1,BYTE CopyMLineFromRemoteSDP = TRUE) const; //modified for ANAT, add a param:index
	void UpdatePortInClosedMedias(sipSdpAndHeadersSt* pSdpAndHeaders, vector<eMediaLineInternalType> copiedTypes) const;

	BYTE isSdpICECompatible(char *pSdpString, DWORD sdpSize);
    void HandleNewIceAllocatedBandwidth(int allocatedRateVideo, int allocatedRateAudio);
    void SetMaxLocalCapToAudioOnly();
    void SetLastRemoteCapToAudioOnly();

	void UpdateBfcpParametersByRemoteSdp(BOOL isToFixTransport = FALSE);

	eSipBfcpMode4DialOut CalcBfcpMode4DialOut();
	void SetIceMlineSubTypesIfNeeded(sipSdpAndHeadersSt* pSdpAndHeaders);

    void AddMrdHeader(CSipHeaderList &rHeaderList);
    void AddInfoHeader(CSipHeaderList &rHeaderList);
    void AddContactHeader(CSipHeaderList &rHeaderList);
    void AddIBMDestinationPublicIpHeader(CSipHeaderList &rHeaderList);
    void AddQosHeader(CSipHeaderList &rHeaderList, const char* domain, BYTE rPriority) const;

    BYTE IsContent(APIU32 channelType, DWORD opcode);
    void SetToAndFromTagsForRmtSdp(sipSdpAndHeadersSt& sdpAndHeaders); //_mccf_
    
    //_dtls_
    void AddParamsToSupportedHeader(CSipHeaderList& headerList);
    void AddXCiscoToSupportedHeader(CSipHeaderList& headerList);
    void AddPlcmIvrToSupportedHeader(CSipHeaderList& headerList);
    void AddParamsToContactHeader(char* strContact);
    void AddXCiscoToContactHeader(char* strContact);
    BOOL GetIsAllowSdesInSdp();
    BOOL IsForceAvpOnEncryptWhenPossible();
    BOOL IsForceSavpOnEncryptAll();

	//added for ANAT begin
	void SetMediaLinesInternalTypeForRmtSdpForAnat(sipSdpAndHeadersSt &sdp);
	void SetLabelsAndContentAttributesInLocalSdpForAnat(sipSdpAndHeadersSt &sdp);
	void UpdatePortInClosedMediasForAnat(sipSdpAndHeadersSt* pSdpAndHeaders, vector<int> copiedIndexes) const;
	int GetClosedMediaSizeForAnat() const;
	int AddClosedMediaLinesFromRemoteSDPForAnat(sipSdpAndHeadersSt* pSdpAndHeaders, vector<int>* pCopiedIndexes) const;
	int AddClosedMediaLinesFromRemoteSDPReinvteForAnat(sipSdpAndHeadersSt* pSdpAndHeaders, vector<int>* pCopiedIndexes) const;
	int CopyMLineFromLocalForAnat(sipSdpAndHeadersSt* pSdpAndHeaders, BYTE isReinvite = FALSE);
	BYTE UpdateAnatSelectedIpVersionIfNecessary(sipSdpAndHeadersSt* pSdpAndHeaders);
	BYTE checkForMediaIpVersionMismatchForPreAnat(const sipSdpAndHeadersSt* pSdpAndHeaders);
	void OrderMediaLinesForAnat(sipSdpAndHeadersSt &sdp);
	//added for ANAT end

	//  for ms lync2013
	int AddSimulcastVideoMLinesToSDPForLync2013(sipSdpAndHeadersSt* pSdpAndHeaders);
	void SetRecvOnlyAttributesToSimulcastVideoMLines(sipSdpAndHeadersSt &sdp);
	void OrderMediaLinesForMsft2013AVMCU(sipSdpAndHeadersSt &sdp);

	void BlockMessagesToCmDuringDtlsHandshake(BYTE bBlock) {m_bIsToBlockBecauseDtls = bBlock;}
	BYTE IsToBlockMessageToCmBecauseDtlsHandshake() {return m_bIsToBlockBecauseDtls;}
	
    BOOL EvaluateEncryptionForTypeNotSupported(const CCommConf* pCommConf, BYTE &fEncryption);
    void SetMediaLinesInternalTypeNotSupported(BYTE fEncryption, sipMediaLineSt *pMediaLine1, sipMediaLineSt *pMediaLine2, BOOL &fFirstVideo, BOOL &fCleanLast, BOOL &fContinue);


	void  CreateFastUpdateMsgToMsft2013(DWORD remoteSSRC, DWORD priorityID, DWORD msSlavePartyIndex = 0);

	virtual CPartyApi*	GetPartyApi() {return m_pPartyApi;}
	virtual CParty*		GetParty() {return (CParty*)m_pParty;}
	virtual eConfMediaType GetTargetConfMediaType();
	void UpdatePayloadTypeInRecieveChannelAccordingToRemoteCaps(CSipChannel * pChannel);
	BYTE IsMSSlaveParty();
	void RemoveOperationPointsFromSvcCap(sipSdpAndHeadersSt &pSdpAndHeaders);
	BYTE IsMSSlaveInParty();
	DWORD GetMSSlavePartyIndex();

	const char* StripGWDecorationFromHost(const char* decorated, char* undecorated) const;

	void PartyDisconnectionPrint(std::ostringstream& msg,const char* flow_str, bool isReject = false, DWORD rejectSipCode = SipCodesOk, const char* reject_str = "reject call",bool print = false);

	//BRIDGE-15745
	void NotifyTransactionOnPendingIfNeeded();
	BOOL IsTipCallTryToResumeAndPendingIsToHold(CSipChanDifArr* pDifArr, BOOL bIsReInviteWithSdp);
	
	PDECLAR_MESSAGE_MAP

	CSipParty*						m_pParty;

	CPartyApi*						m_pPartyApi;
	CSipNetSetup*					m_pNetSetup;

//	CIpRsrcDesc*					m_pRsrcDesc;
//	CIPService*						m_pService;

	APIU32							m_pDestUnitId;
	APIU32							m_callIndex;
	DWORD							m_serviceId;

	CQoS*							m_pQos;
	CSipCall*						m_pCall;

	CSipCaps*						m_pChosenLocalCap ;			// Pointer to current local caps (can be changed according to allocated resources or vendor).
	CSipCaps*						m_pFullLocalCaps;		// Full set of local caps (can be changed according to vendor).
	CSipCaps*						m_pPartialLocalCaps;	// Partial set of local caps (can be changed according to vendor).
	CSipCaps*                       m_pMaxLocalCaps;//for DPA
	CSipCaps*						m_pPreviousRemoteCaps;	// The saved remote caps, before the new transaction. It is needed in order to be able to return to previuos remote caps when re-invite failed
	CSipCaps*						m_pLastRemoteCaps;		// The new remote caps from the last invite transaction.
	sipSdpAndHeadersSt*				m_pRemoteSdp;			// the original before removing bad payloads or completing from local
	sipSdpAndHeadersSt*				m_pLocalSdp;			// the last sent local SDP. It needed to match media lines (content)

	DWORD							m_outboundProxyIp;
	enTransportType					m_transportType;
	CSmallString					m_strConfParamInfo;
	DWORD*							m_oldMediaBytesArr;
	DWORD*							m_oldFramesArr;
	BYTE							m_bIsReInviteTransaction;

	DWORD							m_wOriginalVideoRate;
	BYTE							m_bSelfFlowControl;
 	WORD							m_keepAliveTimerCouter;
 	WORD							m_isKeepAliveIndArrived;

 	BYTE							m_isNeedToResetCallIndex;



 	WORD							m_speakerMcuNum;
 	WORD							m_speakerTermNum;
 	PartyRsrcID						m_speakerPartyId;

	// For Microsoft MOC R2:
	BYTE 							m_bIsHold;	  // current hold state.
	BYTE							m_bPauseMyVideo;  // Is current state is pause my video
	APIU32 							m_savedAudioPort;
	APIU32 							m_savedVideoPort;
	APIU32 							m_savedVideoIp;

	RemoteIdent 					m_remoteIdent;
	char*							m_UserAgent;
	eMediaStreamAttr				m_eMediaStreamAttrType;
	eBfcpMStreamType				m_bfcpMStreamType;
	char*							m_sdpRemoteSessionInformation;

	mcXmlTransportAddress			m_dummyMediaIp;
	unsigned int 							m_dummyRtcpPort;
	BYTE							m_bChangeModeWithinTransaction;
	WORD                            m_isLprModeOn;
	DWORD                           m_LastRateBeforeLpr;
	WORD                            m_LprModeTimeout;
	CCopVideoTxModes*				m_pCopVideoModes;

    BYTE							m_isCameraControl; //some of the token indication are not real for fecc
    BYTE							m_bDeclareVideoFromScmOnly; // Declare the video from scm only, without adding video from local caps.

	//ICE
	BYTE							m_IsEnableICE;
	char*							m_LocalICESdp;
	CIceParams*						m_pIceParams;
	DWORD 							m_Ice_Session_Index;
    enIceConnectivityCheckStatus    m_IceConnectivityCheckStatus;
    BYTE 							m_bIsSentICEStackReq;
    BYTE							m_bIsIceSessionIsInClosingState;
	BYTE							m_bNeedUpdateIceToNonIce;

    CSegment*                       m_SavedTrans;
    EPendingTransType               m_PendTransType;
    BYTE                            m_ChannelsWithLprPayload;

    chosenCandidatesSt				m_chosenCandidates[NumOfMediaTypes-1];

    // patch for cancel in case of ICE
    CSegment*  						m_cancelIndParams;
    BYTE							m_bIsIceHasSessionIndex;
    BYTE							m_bNeedToCancelIceSession;

    APIU16 							m_lastFloorRequestID;

	eContentState					m_eContentInState;
	eContentState					m_eContentOutState;

	DWORD							m_fullContentRate; // The content rate that was received from conference in the original local caps.***ppc can be removed when dpa implemented.
	BYTE							m_bIsContentSpeaker;
	BYTE                            m_isNeedToReleaseUponHoldForTip;

//	mcTransportAddress				m_bfcpLocalAddress;
//	mcTransportAddress				m_bfcpRemoteAddress;

	// TIP
	DWORD							m_tipAudioRate;
	DWORD							m_tipVideoRate;
	CTipRtcpCntl*					m_pTipRtcpCntl;
//	ETipPartyTypeAndPosition 		m_tipPartyType;
	DWORD							m_tipPartySlaveIdToMonitor;
	APIU16							m_RtcpCnameMask;
	BYTE                            m_IsNeedToExtractInfoFromRtcpCname;
	int 							m_AllocatedBandwidth;
	BOOL							m_bRingback;
	BOOL							m_bIsOnHold;
	BOOL							m_bIsResuming;

	// BFCP
	CSipBfcpCtrl					*m_pSipBfcpCtrl;

	BYTE							m_bDeclareOnContentFromScmOnly;

	// Video quality indication
	eRtcpPacketLossStatus 			m_cmInboundPacketLossStatus;
	eRtcpPacketLossStatus 			m_cmOutboundPacketLossStatus;
	eRtcpPacketLossStatus 			m_adjInboundPacketLossStatus;
	eRtcpPacketLossStatus 			m_adjOutboundPacketLossStatus;

	BYTE 							m_inboundLprActive;
	BYTE 							m_outboundLprActive;
    //---TCP Keep Alive ------------------------------------------//
	DWORD 							m_dwKeepAliveFrequency_Sec  ;
	DWORD 							m_MsKeepAlivePongTimeOut    ;
    DWORD                           m_dwKeppAliveBehavior       ;
    DWORD                           m_dwKeepAliveType           ;   
    DWORD                           m_dwSocketActivityErrCount  ;
    //------------------------------------------------------------//

	WORD							m_bfcpChannelsAckCounter;
	BYTE							m_bBfcpReconnect;

	//LYNC2013_FEC_RED:
	DWORD							m_RedFractionLossInPercent;
	BOOL                            m_bIsRedOn;
	DWORD 							m_ChangeOfRateRED;
	//DWORD 						m_LastRateBeforeRED;
	//DWORD   						m_BitRateWithRED;
	BOOL							m_bIsCMStartRed;
	BOOL                            m_isActiveFecFlow;
	BOOL                            m_isActiveRedFlow;
	PACKET_LOSS_STATUS              m_savedFecPacketLossStatus;
	PACKET_LOSS_STATUS              m_savedRedPacketLossStatus;

	//MS RTV FEC or FEC for MsSvc
	BOOL 							m_bIsCMStartFec;
	BOOL							m_bIsFecOn;
	DWORD							m_FecFractionLossInPercent;
	//DWORD 						m_LastRateBeforeFEC;
	//DWORD 						m_BitRateWithFEC;
	DWORD 							m_ChangeOfRateFEC;

	APIU32 							m_LossPacketSeqNum;
	CIpComMode* 					m_pTargetModeBeforeDowngrade; // to keep 'original' target mode before 'Hold'/downgrade transaction
	BYTE                            m_bIsMrcCall;
    MrdVersionStruct                m_mrdVersion;
	APIU16							m_plcmRequireMask;
	eMediaLineInternalType 			m_plcmReqMaskMlineOrder[MaxMediaLinesPlcmReqTag];

	//added for ANAT
	enIpVersion						m_AnatSelectedIpVersion; 
	enIpVersion						m_LastAnatSelectedIpVersion;
	BYTE							m_isUpdateAnatIpType;
	//CDR_MCCF: monitoring
	CIpChannelDetails*             m_pIpVideoCdrChannelDetails[NUM_OF_CHANNELS_FOR_MCCF_CDR_INFO];
	CPrtMontrBaseParams*           m_pIpVideoCdrChannelMonitor[NUM_OF_CHANNELS_FOR_MCCF_CDR_INFO];
	
	//LyncCCS	
	BYTE							m_bIsCCSPlugin;
	enPartyAuthState					m_ePartyAuthState;
	char*							m_pAuthPwd;
	BYTE							m_AuthCounter;

	DWORD 							m_nCenterNacks;
	DWORD							m_nLeftToRightNacks;
	DWORD							m_nRightToLeftNacks;
	DWORD							m_nRtpSelfIntraReq;
	
	BYTE							m_bIsToBlockBecauseDtls;
	
	BYTE					        m_isASSIPContentEnable;

	BOOL							m_bShouldMsftVideoTxStreamOn; // mute msft tx video channel till we receive VSR with source != NONE 
	enMsft2013ActivationState       m_isMs2013Active; //this is TRUE only if this is MS 2013 Client/mcu + MS env + supported by HW for 2013 features (MS SVC is not supported in mpmx for example).
	CSipVsrCtrl*					m_pVsrControl;
	BOOL							m_bIsVsrCtrlInitialized;
	DWORD							m_msftPliRequestId[MAX_STREAM_LYNC_2013_CONN];
	

	WORD				m_BFCPcapConfIDfield;
	WORD				m_BFCPcapUserIDfield;

	BOOL							m_bIsCiscoTagExist;
    BYTE						m_useRtcp;
    BYTE						m_bIsOneDtlsArrived;
    std::string                    m_sLastOriginVersion;

    BOOL                           m_allowMediaBasedDisconnectionInReinvite;
    mcMuxLync2013InfoReq           m_LastAvMcuMux;

    BOOL m_IsST852Resume;
};


#endif



