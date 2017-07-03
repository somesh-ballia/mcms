//+========================================================================+
//                       SipCsReq.h	                                       |
//            Copyright 2005 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:		SipCsReq.h	                                               |
// SUBSYSTEM:                                                              |
// PROGRAMMER:	Avner Lami												   |
// Date:		11/7/05													   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#ifndef __SIPCSREQ_H__
#define	__SIPCSREQ_H__

#include "IpCommonDefinitions.h"
#include "SipDefinitions.h"
#include "SipStructures.h"
#include "SipHeadersList.h"
#include "IpAddressDefinitions.h"
#include "ChannelParams.h"

typedef struct {
	//TBD
} mcReqBase;


// SIP Party --> CS
///////////////////

//SIP_CS_SIG_SOCKET_ACTIVITY_REQ   
//SIP_CS_SIG_SOCKET_ACTIVITY_IND 
//    Request and report of SOCKET's statistics
//    for MS-KEEP-ALIVE ; Role: UAS 
typedef struct  _mcReqIndSocketStatistics
{
    unsigned int   dwSilenceTimeRecv_mSec; 	// SILENCE of SOSKET on RECV
    unsigned int   dwSilenceTimeSend_mSec; 	// SILENCE of SOSKET on SEND
} 
mcReqIndSocketStatistics;

//SIP_CS_SIG_SEND_CRLF_REQ
//SIP_CS_PROXY_SEND_CRLF_REQ
//    Request for CRLF+CRLF sending.
//    for MS-KEEP-ALIVE ; Role: UAC 
typedef struct  _mcReqSendCrlf
{
    mcXmlTransportAddress	registrarTransportAddr  ; // for SIP_CS_PROXY_SEND_CRLF_REQ only
    unsigned int            dwMsKepAliveTimeOut_Sec ; // 	
    unsigned int            dwConfId                ; // for SIP_CS_PROXY_SEND_CRLF_REQ only  
} 
mcReqSendCrlf;


// SIP_CS_SIG_INVITE_REQ
typedef struct {
	APIU32					bIsOutboundProxyInUse;
	APIU32					bIsAVMCU;
	mcXmlTransportAddress 	transportAddress;	
	APIS8					domainName[MaxLengthOfSingleUrl];
	mcCallGeneratorParams	callGeneratorParams;	// Call Generator SoftMCU CS parameters
	sipSdpAndHeadersSt		sipSdpAndHeaders;

} mcReqInvite;


// SIP_CS_SIG_REINVITE_REQ
typedef struct {
	sipSdpAndHeadersSt		sipSdpAndHeaders;

} mcReqReInvite;


// SIP_CS_SIG_INVITE_ACK_REQ
typedef mcReqBase mcReqInviteAck;


// SIP_CS_SIG_INVITE_RESPONSE_REQ
typedef struct {
	APIS32					status;
	APIU32					bIsFocus;
	mcCallGeneratorParams	callGeneratorParams;	// Call Generator SoftMCU CS parameters
	sipSdpAndHeadersSt		sipSdpAndHeaders;

} mcReqInviteResponse;


// SIP_CS_SIG_RINGING_REQ
typedef struct {
	sipMessageHeaders		sipHeaders;

} mcReqRinging;


// SIP_CS_SIG_CANCEL_REQ
typedef mcReqBase mcReqCancel;


// SIP_CS_SIG_BYE_REQ
typedef struct {
	APIS8					sReason[MaxErrorMessageSize];

} mcReqBye;


// SIP_CS_SIG_BYE_200_OK_REQ
typedef mcReqBase mcReqBye200Ok;


// SIP_CS_SIG_VIDEO_FAST_UPDATE_REQ
typedef mcReqBase mcReqVideoFastUpdate;

typedef mcReqBase mcReqVideoFastUpdateSametime;


// SIP_CS_SIG_DEL_NEW_CALL_REQ
typedef mcReqBase mcReqDelNewCall;


// SIP_CS_SIG_INSTANT_MESSAGE_REQ
typedef struct {
	APIS8					contentType[MaxLengthOfNonSdpContentType];
	APIS8					content[MaxLengthOfNonSdpContent];

} mcReqInstantMessage;


// SIP_CS_SIG_REDIRECT_DATA_REQ
typedef struct {
	APIS32					maxCalls;
	APIS8					contact[MaxLengthOfSingleUrl];

} mcReqRedirectData;


// SIP_CS_SIG_OPTIONS_RESP_REQ
typedef struct {
	APIS32					status;
	void					*pCallObj;
	APIU32					remoteCseq;
	sipSdpAndHeadersSt		sipSdpAndHeaders;

} mcReqOptionsResp;


// SIP_CS_SIG_REFER_RESP_REQ
typedef struct {
	APIS32					status;
	APIU32					remoteCseq;
	void					*pCallObj;

} mcReqReferResp;


// SIP_CS_PARTY_KEEP_ALIVE_REQ
typedef mcReqBase mcReqKeepAlive;


// SIP_CS_SIG_SUBSCRIBE_RESP_REQ
typedef struct {
	APIS32					status;
	void					*pCallObj;
	APIS32					expires;
	APIU32					remoteCseq;

} mcReqSubscribeResp;

// SIP_CS_SIG_INFO_REQ
typedef struct
{
	APIU32					dTxnId;
	APIU32					subOpcode; // enGeneralInfoOpcode
	APIU32					dynamicLen;
	APIU8					buff[1];	// mcReqVideoFastUpdate | mcReqVideoFastUpdateV2 | mcReqVideoFastUpdateSametime | mcReqSipLPRModeChange | mcReqFlowControl

} mcReqInfo;

// SIP_CS_SIG_INFO_RESP_REQ
typedef struct
{
	APIS32					status;
	APIU32					dTxnId;
	APIU32					subOpcode; // enGeneralInfoOpcode
} mcReqInfoResp;

typedef struct {
	APIS8					label[32]; // label string

} mcReqVideoFastUpdateV2;
// SIP_CS_SIG_LPR_MODE_CHANGE_REQ
typedef struct  {
	APIU32					lossProtection;
	APIU32					mtbf;
	APIU32					congestionCeiling;
	APIU32					fill;
	APIU32					modeTimeout;

} mcReqSipLPRModeChange;


typedef struct  {
	APIS8					label[32]; // label string
	APIU32					mediaDirection; // enMediaDirection
	APIU32					rate;

} mcReqFlowControl;

typedef struct  {
	APIU8					digits[MaxLengthOfDigits];

} mcReqPlcmIvr;

// SIP_CS_SIG_DIALOG_RECOVERY_REQ 
typedef struct {
	APIS32					status; // 0 - break flow, 1 - Continue
} mcReqDialogRecovery;

//CCS plugin pwd
typedef struct{
	APIU32 					status; // 0--failed, 1--success
}mcReqCcsConfPwd;

//For eFeatureRssDialin
typedef struct{
	APIU32 					command; // 1: start, 2: stop, 3: pause
}mcReqSrsCmd;


typedef union
{
	mcReqVideoFastUpdate				uMcReqVideoFastUpdate;
	mcReqVideoFastUpdateV2 				uMcReqVideoFastUpdateV2;
	mcReqVideoFastUpdateSametime		uMcReqVideoFastUpdateSametime;
	mcReqSipLPRModeChange				uMcReqLPRModeChange;
	mcReqFlowControl					uMcReqFlowControl;
	mcReqPlcmIvr							uMcReqPlcmIvr;

} SIP_CS_SIG_INFO_REQ_UNION;

// SIP Proxy --> CS
///////////////////

// SIP_CS_PROXY_REGISTER_REQ
typedef struct {
	APIS32					id;
	APIS32					expires;
	APIU32					subOpcode;
	mcXmlTransportAddress	proxyTransportAddr;
	mcXmlTransportAddress	registrarTransportAddr;
	sipMessageHeadersBase	headersBase;

} mcReqRegisterBase;

typedef struct {
	APIS32					id;
	APIS32					expires;
	APIU32					subOpcode;
	mcXmlTransportAddress	proxyTransportAddr;
	mcXmlTransportAddress	registrarTransportAddr;
	sipMessageHeaders		sipHeaders;

} mcReqRegister;


// SIP_CS_PROXY_SUBSCRIBE_REQ
typedef struct {
	APIS32					id;
	APIS32					expires;
	APIU32					subOpcode;
	APIS8                   domainName[MaxLengthOfSingleUrl];
	mcXmlTransportAddress	transportAddr;
	sipMessageHeadersBase	headersBase;

} mcReqSubscribeBase;

typedef struct {
	APIS32					id;
	APIS32					expires;
	APIU32					subOpcode;
    APIS8                   domainName[MaxLengthOfSingleUrl];
	mcXmlTransportAddress	transportAddr;
	sipMessageHeaders		sipHeaders;

} mcReqSubscribe;


// SIP_CS_PROXY_UNKNOWN_METHOD_REQ
typedef struct {
	mcXmlTransportAddress	remoteTransportAddress;	//Either UAS or Proxy
	APIS8					localAddress[MaxLengthOfSingleUrl];
	APIS8					methodName[MaxLengthOfSipMethosName];
	APIS8					contentType[MaxLengthOfNonSdpContentType];
	APIS8					content[MaxLengthOfNonSdpContent];

} mcReqUnknownMethod;

// SIP_CS_PROXY_NOTIFY_REQ
typedef struct {
	APIU32					cseq;
	mcXmlTransportAddress	transportAddress;
	sipContentAndHeadersBaseSt	contentAndHeadersBase;

} mcReqNotifyBase;

typedef struct {
	APIU32					cseq;
	mcXmlTransportAddress	transportAddress;
	sipContentAndHeadersSt	sipContentAndHeaders;

} mcReqNotify;

// SIP_CS_PROXY_SERVICE_REQ
typedef struct {
	APIU32					subOpcode;
	APIS32					id;
	mcXmlTransportAddress	transportAddress;
	sipMessageHeadersBase	headersBase;

} mcReqServiceBase;

typedef struct {
	APIU32					subOpcode;
	APIS32					id;
	mcXmlTransportAddress	transportAddress;
	sipMessageHeaders		sipHeaders;

} mcReqService;

#ifdef __BFCP_CS_CONNECTION_ENABLED__
// SIP_CS_BFCP_MESSAGE_REQ
typedef struct {
	int		status;
	int		length;
	char	buffer[1];
	
} mcReqBfcpMessage;
#endif //__BFCP_CS_CONNECTION_ENABLED__


/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
typedef union
{
	mcReqInvite							uMcReqInvite;
	mcReqReInvite						uMcReqReInvite;
	mcReqInviteAck						uMcReqInviteAck;
	mcReqInviteResponse					uMcReqInviteResponse;
	mcReqRinging						uMcReqRinging;
	mcReqCancel							uMcReqCancel;
	mcReqBye							uMcReqBye;
	mcReqBye200Ok						uMcReqBye200Ok;
	mcReqVideoFastUpdate				uMcReqVideoFastUpdate;
	mcReqVideoFastUpdateSametime		uMcReqVideoFastUpdateSametime;
	mcReqDelNewCall						uMcReqDelNewCall;
	mcReqInstantMessage					uMcReqInstantMessage;
	mcReqRedirectData					uMcReqRedirectData;
	mcReqOptionsResp					uMcReqOptionsResp;
	mcReqReferResp						uMcReqReferResp;
	mcReqRegister						uMcReqRegister;
	mcReqSubscribe						uMcReqSubscribe;
	mcReqSubscribeResp					uMcReqSubscribeResp;
	mcReqUnknownMethod					uMcReqUnknownMethod;
	mcReqNotify							uMcReqNotify;
	mcReqService						uMcReqService;
	mcReqDialogRecovery					uMcReqDialogRecovery;
    mcReqSendCrlf                       uMcReqSendCrlf;
	//mcReqBfcpMessage					uMcReqBfcpMessage;
} SIP_CS_REQ_UNION;

#endif // __SIPCSREQ_H__
