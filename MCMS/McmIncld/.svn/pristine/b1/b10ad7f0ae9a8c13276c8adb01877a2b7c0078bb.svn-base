//+========================================================================+
//                       SipCsInd.h	                                       |
//            Copyright 2005 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:		SipCsInd.h	                                               |
// SUBSYSTEM:	CS/ConfParty                                               |
// PROGRAMMER:	Avner Lami												   |
// Date:		11/7/05													   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#ifndef __SIPCSIND_H__
#define __SIPCSIND_H__

#include "SipHeadersList.h"
#include "SipDefinitions.h"
#include "SipStructures.h"
#include "IpCommonDefinitions.h"
#include "IpAddressDefinitions.h"
#include "ChannelParams.h"

typedef struct {
	//TBD
} mcIndBase;


// CS --> SIP Party
///////////////////

// SIP_CS_SIG_INVITE_IND
typedef struct {
	APIS32					status;
	APIU32					bIsFocus;
	sipSdpAndHeadersSt		sipSdpAndHeaders;

} mcIndInvite;


// SIP_CS_SIG_REINVITE_IND
typedef mcIndInvite mcIndReInvite;


// SIP_CS_SIG_INVITE_ACK_IND
typedef struct {
	APIS32					status;
	sipSdpAndHeadersSt		sipSdpAndHeaders;

} mcIndInviteAck;


// SIP_CS_SIG_INVITE_RESPONSE_IND
typedef struct {
	APIS32					status;
	sipSdpAndHeadersSt		sipSdpAndHeaders;

} mcIndInviteResponse;


// SIP_CS_SIG_PROV_RESPONSE_IND
typedef struct {
	APIS32					status;

} mcIndProvResponse;


// SIP_CS_SIG_BYE_IND
typedef struct {
	APIS32					status;

} mcIndBye;


// SIP_CS_SIG_BYE_200_OK_IND
typedef struct {
	APIS32					status;

} mcIndBye200Ok;


// SIP_CS_SIG_CANCEL_IND
typedef struct {
	APIS32					status;

} mcIndCancel;


// SIP_CS_SIG_VIDEO_FAST_UPDATE_IND
typedef mcIndBase mcIndVideoFastUpdate;


// SIP_CS_SIG_DTMF_DIGIT_IND
typedef struct {
	APIS8					digit;

} mcIndDtmfDigit;


// SIP_CS_SIG_INSTANT_MESSAGE_IND
typedef struct {
	APIS32					status;
	APIS8					message[MaxLengthOfNonSdpContent];

} mcIndInstantMessage;


// SIP_CS_SIG_OPTIONS_IND
typedef struct {
	void					*pCallObj;
	APIU32					remoteCseq;
	sipMessageHeaders		sipHeaders;

} mcIndOptions;


// SIP_CS_SIG_REFER_IND
typedef struct {
	void					*pCallObj;
	APIU32					remoteCseq;
	mcXmlTransportAddress	transportAddress;
	sipMessageHeadersBase	headersBase;

} mcIndReferBase;

typedef struct {
	void					*pCallObj;
	APIU32					remoteCseq;
	mcXmlTransportAddress	transportAddress;
	sipMessageHeaders		sipHeaders;

} mcIndRefer;


// SIP_CS_SIG_SESSION_TIMER_EXPIRED_IND
typedef mcIndBase mcIndSessionTimerExpired;

// SIP_CS_SIG_SESSION_TIMER_REINVITE_IND
typedef mcIndBase mcIndSessionTimerReinvite;


// SIP_CS_PARTY_KEEP_ALIVE_IND
typedef struct {
	APIS32					status;
} mcIndKeepAlive;


// SIP_CS_SIG_TRACE_INFO_IND, SIP_CS_PROXY_TRACE_INFO_IND
typedef struct {
	APIS8					sErrMsg[MaxErrorMessageSize];

} mcIndTraceInfo;


// SIP_CS_SIG_BAD_STATUS_IND, SIP_CS_PROXY_BAD_STATUS_IND
typedef struct {
	APIS8					sErrMsg[MaxErrorMessageSize];
	APIU32					FailedrequestOpcode;

} mcIndBadStatus;


// SIP_CS_SIG_TRANSPORT_ERROR_IND, SIP_CS_PROXY_TRANSPORT_ERROR_IND
typedef struct {
	APIS32					status;
	APIS32					expectedReq;
	APIS8					sErrMsg[MaxErrorMessageSize];

} mcIndTransportError;


// SIP_CS_SIG_CARD_DATA_IND, SIP_CS_PROXY_CARD_DATA_IND
typedef struct {
	APIU32					cardDataCodes;
	APIS8					cardValueDetailed[MaxErrorMessageSize];

} mcIndCardData;


// SIP_CS_SIG_SUBSCRIBE_IND
typedef struct {
	void					*pCallObj;
	APIS32					expires;
	APIU32					remoteCseq;
	mcXmlTransportAddress	transportAddress;
	sipMessageHeadersBase	headersBase;

} mcIndSubscribeBase;

typedef struct {
	void					*pCallObj;
	APIS32					expires;
	APIU32					remoteCseq;
	mcXmlTransportAddress	transportAddress;
	sipMessageHeaders		sipHeaders;

} mcIndSubscribe;

// SIP_CS_SIG_INFO_IND
typedef struct
{
	APIU32					dTxnId;
	APIU32					subOpcode; // enGeneralInfoOpcode
	APIU32					dynamicLen;
	APIU8					buff[1];	// mcIndVideoFastUpdate | mcIndVideoFastUpdateV2 | mcIndSipLPRModeChange | mcIndFlowControl

} mcIndInfo;

// SIP_CS_SIG_INFO_RESP_IND
typedef struct
{
	APIS32					status;
	APIU32					dTxnId;
	APIU32					subOpcode; // enGeneralInfoOpcode
	sipMessageHeaders		sipHeaders;

} mcIndInfoResp;

typedef struct {
	APIS8					label[32]; // label string

} mcIndVideoFastUpdateV2;

typedef struct  {
	APIU32					lossProtection;
	APIU32					mtbf;
	APIU32					congestionCeiling;
	APIU32					fill;
	APIU32					modeTimeout;

} mcIndSipLPRModeChange;

typedef struct  {
	APIS8					label[32]; // label string
	APIU32					mediaDirection; // enMediaDirection
	APIU32					rate;

} mcIndFlowControl;

typedef struct  {
	APIS8					pwd[64]; // pwd string
} mcIndConfPwdInfo;


//for eFeatureRssDialin
typedef struct  {
	APIU32					status; //0: failed, 1: started, 2: stopped, 3: paused
} mcIndSrsIndicaton;
typedef struct  {
	APIU32					layout; //0: auto, 1: single view 1X1, 2: dual view, 1X2
} mcIndSrsVideolayout;


typedef union
{
	mcIndVideoFastUpdate 				uMcIndVideoFastUpdate;
	mcIndVideoFastUpdateV2 				uMcIndVideoFastUpdateV2;
	mcIndSipLPRModeChange				uMcIndLPRModeChange;
	mcIndFlowControl					uMcIndFlowControl;


} SIP_CS_SIG_INFO_IND_UNION;

//SIP_CS_SIG_NOTIFY_RESPONSE_IND
typedef struct {
	APIS32					status;
	APIS32					id;

} mcIndNotifyResp;

// SIP_CS_PROXY_BENOTIFY_IND
typedef struct {
	APIS32					expires;		
	sipContentAndHeadersBaseSt		contentAndHeadersBase;

} mcIndBenotifyBase;

typedef struct {
	APIS32					expires;	
	sipContentAndHeadersSt			sipContentAndHeaders;

} mcIndBenotify;

//-S- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------//
typedef struct _mcIndBenotify_Part
{
	APIS32						expires;	
	sipContentAndHeadersSt_Part	sipContentAndHeadersPart;

} mcIndBenotify_Part;
//-S- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------//

// CS --> SIP Proxy
///////////////////

// SIP_CS_PROXY_REGISTER_RESPONSE_IND
typedef struct {
	APIS32					status;
	APIS32					id;
	APIS32					expires;
	sipMessageHeadersBase	headersBase;

} mcIndRegisterRespBase;

typedef struct {
	APIS32					status;
	APIS32					id;
	APIS32					expires;
	sipMessageHeaders		sipHeaders;

} mcIndRegisterResp;


// SIP_CS_PROXY_SUBSCRIBE_RESPONSE_IND
typedef struct {
	APIS32					status;
	APIS32					id;
	APIS32					expires;
	APIS32					subOpcode;
	sipMessageHeadersBase	headersBase;

} mcIndSubscribeRespBase;

typedef struct {
	APIS32					status;
	APIS32					id;
	APIS32					expires;
	APIS32					subOpcode;
	sipMessageHeaders		sipHeaders;

} mcIndSubscribeResp;



// CS --> SIP Proxy
///////////////////

// SIP_CS_PROXY_NOTIFY_IND
typedef struct {
	//APIS8					callId[MaxLengthOfSipCallId];
	//APIS8					message[MaxLengthOfNonSdpContent];
	sipMessageHeaders		sipHeaders;

} mcIndNotify;

//SIP_CS_PROXY_SERVICE_RESPONSE_IND
typedef struct {

	APIS32					status;
	APIS32					id;
	APIS32					subOpcode;
	sipMessageHeaders		sipHeaders;

} mcIndServiceResp;

// SIP_CS_SIG_DIALOG_RECOVERY_IND 
typedef struct {
	APIS32					status; // reserved
} mcIndDialogRecovery;

//SIP_CS_SIG_SOCKET_ACTIVITY_IND
//See: mcReqIndSocketStatistics
typedef struct  _mcIndSocketStatistics
{
    unsigned int   dwSilenceTimeRecv_mSec; 	// SILENCE of SOSKET on RECV
    unsigned int   dwSilenceTimeSend_mSec; 	// SILENCE of SOSKET on SEND
} 
mcIndSocketStatistics;


#ifdef __BFCP_CS_CONNECTION_ENABLED__
// SIP_CS_BFCP_MESSAGE_IND
typedef struct {
	int 	status;
	int		length;
	char	buffer[1];

} mcIndBfcpMessage;

// SIP_CS_BFCP_TRANSPORT_IND
typedef struct {
	int 	status;
	mcXmlTransportAddress	hostAddress;
	mcXmlTransportAddress	remoteAddress;

} mcIndBfcpTransport;
#endif //__BFCP_CS_CONNECTION_ENABLED__

// SIP_CS_SIG_CRLF_ERR_IND   
typedef struct _mcIndSigCrlfError
{
    unsigned int 	eCrlfSendingErrorCode;//0:Undef; 1: Socket is unavailable; 2: write to socket failed
} 
mcIndSigCrlfError;

// SIP_CS_ PROXY_CRLF_ERR_IND   
typedef struct _mcIndProxyCrlfError
{
    unsigned int 	eCrlfSendingErrorCode;//0:Undef; 1: Socket is unavailable; 2: write to socket failed
    unsigned int    nConfId              ;
} 
mcIndProxyCrlfError;



/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
typedef union
{
	mcIndInvite							uMcIndInvite;
	mcIndReInvite						uMcIndReInvite;
	mcIndInviteAck						uMcIndInviteAck;
	mcIndInviteResponse					uMcIndInviteResponse;
	mcIndProvResponse					uMcIndProvResponse;
	mcIndBye							uMcIndBye;
	mcIndBye200Ok						uMcIndBye200Ok;
	mcIndCancel							uMcIndCancel;
	mcIndVideoFastUpdate				uMcIndVideoFastUpdate;
	mcIndDtmfDigit						uMcIndDtmfDigit;
	mcIndInstantMessage					uMcIndInstantMessage;
	mcIndOptions						uMcIndOptions;
	mcIndRefer							uMcIndRefer;
	mcIndSessionTimerExpired			uMcIndSessionTimerExpired;
	mcIndSessionTimerReinvite			uMcIndSessionTimerReinvite;
	mcIndTraceInfo						uMcIndTraceInfo;
	mcIndBadStatus						uMcIndBadStatus;
	mcIndTransportError					uMcIndTransportError;
	mcIndCardData						uMcIndCardData;
	mcIndRegisterResp					uMcIndRegisterResp;
	mcIndSubscribeResp					uMcIndSubscribeResp;
	mcIndSubscribe						uMcIndSubscribe;
	mcIndNotify							uMcIndNotify;
	mcIndNotifyResp						uMcIndNotifyResp;
	mcIndServiceResp					uMcIndServiceResp;
	mcIndDialogRecovery					uMcIndDialogRecovery;
    mcIndSocketStatistics               uMcIndSocketActivity;
    mcIndSigCrlfError                   uMcIndSigCrlfError  ;
    mcIndProxyCrlfError                 uMcIndProxyCrlfError;
#ifdef __BFCP_CS_CONNECTION_ENABLED__
	mcIndBfcpMessage					uMcIndBfcpMessage;
	mcIndBfcpTransport				    uMcIndBfcpTransport;
#endif //__BFCP_CS_CONNECTION_ENABLED__
} SIP_CS_IND_UNION;

#endif //__SIPCSIND_H__
