// SipCsReqFormat.cpp
// Avner Lami

// Pre-definitions:
// ----------------
#define SRC	"SipCsReqFormat"

// Includes files
//----------------
#include <IpCommonDefinitions.h>
#include <XmlFormat.h>
#include <SipDefinitions.h>


// External variables:
// -------------------
extern genXmlFormat commonHeaderFormat;
extern genXmlFormat sipSdpAndHeadersFormat;
extern genXmlFormat sipMessageHeadersFormat;
extern genXmlFormat sipContentAndHeadersFormat;
extern genXmlFormat mcXmlTransportAddressFormat;
extern genXmlFormat bfcpMessageFormat;
extern genXmlFormat mcXmlCallGeneratorParamsFormat;

//===========================================================
// Request formats
//===========================================================

// SIP Party --> CS
///////////////////

// SIP_CS_SIG_INVITE_REQ

genXmlFormat mcReqInviteFormat[] = {
	{parentElemType,	tagVarType,		"mcReqInvite",	6,	0, 0, 0, 0},
		{childElemType,		longVarType,	"bIsOutboundProxyInUse",	0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"bIsAVMCU",					0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"transportAddress",			1,	(int)&mcXmlTransportAddressFormat, 0, 0, 0},
		{siblingElemType,	stringVarType,	"domainName",				0,	MaxLengthOfSingleUrl, 0, 0, 0},
		{parentElemType,	structVarType,	"callGeneratorParams",			1,	(int)&mcXmlCallGeneratorParamsFormat, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,						1,	(int)&sipSdpAndHeadersFormat, 0, 0, 0}
};

genXmlFormat mcDsReqInviteFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_INVITE_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqInviteFormat, 0, 0, 0}
};

//typedef struct {
//	APIU32					bIsOutboundProxyInUse;
//	APIU32					bIsAVMCU;
//	mcXmlTransportAddress 	transportAddress;
//	APIS8					domainName[MaxLengthOfSingleUrl];
//	sipSdpAndHeadersSt		sipSdpAndHeaders;
//} mcReqInvite;

//===========================================================
// SIP_CS_SIG_REINVITE_REQ

genXmlFormat mcReqReInviteFormat[] = {
	{parentElemType,	tagVarType,		"mcReqReInvite",	1,	0, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,	1,	(int)&sipSdpAndHeadersFormat, 0, 0, 0}
};

genXmlFormat mcDsReqReInviteFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_REINVITE_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqReInviteFormat, 0, 0, 0}
};

//typedef struct {
//	sipSdpAndHeadersSt		sipSdpAndHeaders;
//} mcReqReInvite;

//===========================================================
// SIP_CS_SIG_INVITE_ACK_REQ

genXmlFormat mcDsReqInviteAckFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_INVITE_ACK_REQ",	1,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0}
};

//typedef mcReqBase mcReqInviteAck;

//===========================================================
// SIP_CS_SIG_INVITE_RESPONSE_REQ

genXmlFormat mcReqInviteResponseFormat[] = {
	{parentElemType,	tagVarType,		"mcReqInviteResponse",	4,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",	0,	0, 0, 0, 0},
		{childElemType,		longVarType,	"bIsFocus",	0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"callGeneratorParams",			1,	(int)&mcXmlCallGeneratorParamsFormat, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,		1,	(int)&sipSdpAndHeadersFormat, 0, 0, 0}
};

genXmlFormat mcDsReqInviteResponseFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_INVITE_RESPONSE_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqInviteResponseFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					status;
//	sipSdpAndHeadersSt		sipSdpAndHeaders;
//} mcReqInviteResponse;

//===========================================================
// SIP_CS_SIG_RINGING_REQ

genXmlFormat mcReqRingingFormat[] = {
	{parentElemType,	tagVarType,		"mcReqRinging",	1,	0, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,	1,	(int)&sipMessageHeadersFormat, 0, 0, 0}
};

genXmlFormat mcDsReqRingingFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_RINGING_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqRingingFormat, 0, 0, 0}
};

//typedef struct {
//	sipMessageHeaders		sipHeaders;
//} mcReqRinging;

//===========================================================
// SIP_CS_SIG_CANCEL_REQ

genXmlFormat mcDsReqCancelFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_CANCEL_REQ",	1,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0}
};

//typedef mcReqBase mcReqCancel;

//===========================================================
// SIP_CS_SIG_BYE_REQ

genXmlFormat mcReqByeFormat[] = {
	{parentElemType,	tagVarType,		"mcReqBye",	1,	0, 0, 0, 0},
		{childElemType,		stringVarType,	"sReason",	0,	MaxErrorMessageSize, 0, 0, 0}
};

genXmlFormat mcDsReqByeFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_BYE_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqByeFormat, 0, 0, 0}
};

//_mccf_
genXmlFormat mccfDsReqByeFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_MCCF_SIG_BYE_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqByeFormat, 0, 0, 0}
};

//typedef struct {
//	APIS8					sReason[MaxErrorMessageSize];
//} mcReqBye;

//===========================================================
// SIP_CS_SIG_BYE_200_OK_REQ

genXmlFormat mcDsReqBye200OkFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_BYE_200_OK_REQ",	1,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0}
};

//===========================================================
// SIP_CS_MCCF_SIG_BYE_200_OK_REQ
genXmlFormat mccfDsReqBye200OkFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_MCCF_SIG_BYE_200_OK_REQ",	1,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0}
};


//typedef mcReqBase mcReqBye200Ok;

//===========================================================
// SIP_CS_SIG_VIDEO_FAST_UPDATE_REQ

genXmlFormat mcDsReqVideoFastUpdateFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_VIDEO_FAST_UPDATE_REQ",	1,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0}
};

//typedef mcReqBase mcReqVideoFastUpdate;

//===========================================================
// SIP_CS_SIG_DEL_NEW_CALL_REQ

genXmlFormat mcDsReqDelNewCallFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_DEL_NEW_CALL_REQ",	1,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0}
};

//typedef mcReqBase mcReqDelNewCall;


//===========================================================
// SIP_CS_SIG_INSTANT_MESSAGE_REQ

genXmlFormat mcReqInstantMessageFormat[] = {
	{parentElemType,	tagVarType,		"mcReqInstantMessage",	2,	0, 0, 0, 0},
		{childElemType,		stringVarType,	"contentType",		0,	MaxLengthOfNonSdpContentType, 0, 0, 0},
		{siblingElemType,	charArrayVarType,"content",			0,	MaxLengthOfNonSdpContent, 0, 0, 0}
};

genXmlFormat mcDsReqInstantMessageFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_INSTANT_MESSAGE_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqInstantMessageFormat, 0, 0, 0}
};

//typedef struct {
//	APIS8					contentType[MaxLengthOfNonSdpContentType]; 
//	APIS8					content[MaxLengthOfNonSdpContent]; 
//} mcReqInstantMessage;

//===========================================================
// SIP_CS_SIG_REDIRECT_DATA_REQ

genXmlFormat mcReqRedirectDataFormat[] = {
	{parentElemType,	tagVarType,	"mcReqRedirectData",	2,	0, 0, 0, 0},
		{childElemType,		longVarType,	"maxCalls",	0,	0, 0, 0, 0},
		{siblingElemType,	stringVarType,	"contact",	0,	MaxLengthOfSingleUrl, 0, 0, 0}
};

genXmlFormat mcDsReqRedirectDataFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_REDIRECT_DATA_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqRedirectDataFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					maxCalls;
//	APIS8					contact[MaxLengthOfSingleUrl];
//} mcReqRedirectData;

//===========================================================
// SIP_CS_SIG_OPTIONS_RESP_REQ

genXmlFormat mcReqOptionsRespFormat[] = {
	{parentElemType,	tagVarType,	"mcReqOptionsResp",	4,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"pAppCall",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"remoteCseq",	0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,			1,	(int)&sipSdpAndHeadersFormat, 0, 0, 0},
};

genXmlFormat mcDsReqOptionsRespFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_OPTIONS_RESP_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqOptionsRespFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					status;
//	void					*pAppCall;
//	APIU32					remoteCseq;
//	sipSdpAndHeadersSt		sipSdpAndHeaders;
//} mcReqOptionsResp;

//===========================================================
// SIP_CS_SIG_REFER_RESP_REQ

genXmlFormat mcReqReferRespFormat[] = {
	{parentElemType,	tagVarType,		"mcReqReferResp",	3,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"remoteCseq",	0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"pAppCall",		0,	0, 0, 0, 0}
};

genXmlFormat mcDsReqReferRespFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_REFER_RESP_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqReferRespFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					status;
//	APIU32					remoteCseq;
//	void					*pAppCall;
//} mcReqReferResp;

//===========================================================
// SIP_CS_PARTY_KEEP_ALIVE_REQ

genXmlFormat mcDsReqSipKeepAliveFormat[] = {
	{rootElemType,	tagVarType,	"SIP_CS_PARTY_KEEP_ALIVE_REQ",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};

//typedef mcReqBase mcDsReqKeepAlive;

//===========================================================
// SIP_CS_SIG_SUBSCRIBE_RESP_REQ

genXmlFormat mcReqSubscribeRespFormat[] = {
	{parentElemType,	tagVarType,	"mcReqSubscribeResp",	4,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"pAppCall",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"expires",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"remoteCseq",	0,	0, 0, 0, 0}
};

genXmlFormat mcDsReqSubscribeRespFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_SUBSCRIBE_RESP_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqSubscribeRespFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					status;
//	void					*pAppCall;
//	APIS32					expires;
//	APIU32					remoteCseq;
//} mcReqSubscribeResp;

//===========================================================
// SIP_CS_SIG_INFO_REQ

genXmlFormat mcReqInfoBuffFormat[] = {
    {parentElemType,    tagVarType, "buff",  2,  0, 0, 0, 0},
        {childElemType,     longVarType,		"length",   0,  0, 0, 0, 0},
        {siblingElemType,   charArrayVarType,	"buff",  0,  0, 0, 0, 0}
};
genXmlFormat mcReqInfoFormat[] = {
	{parentElemType,	tagVarType,		"mcReqInfo",	3,		0, 	0, 0, 0},
			{childElemType,		longVarType,		"dTxnId",		0,	0, 0, 0, 0},
			{childElemType,		longVarType,		"subOpcode",	0,	0, 0, 0, 0},
			{parentElemType,	dynamicDataVarType,	NULL,			1,	(int)&mcReqInfoBuffFormat, 0, 0, 0}
};

	genXmlFormat mcDsReqInfoFormat[] = {
		{rootElemType,		tagVarType,		"SIP_CS_SIG_INFO_REQ",	2,	0, 0, 0, 0},
			{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
			{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqInfoFormat, 0, 0, 0}
};		

//typedef struct
//{
//	APIU32					dTxnId;
//	APIU32								subOpcode; // enGeneralInfoOpcode
//	APIU32								dynamicLen;
//	APIS8					buff[1];	
//	
//} mcReqInfo;

genXmlFormat mcReqInfoRespFormat[] = {
	{parentElemType,	tagVarType,		"mcReqInfoResp",	3,		0, 	0, 0, 0},
		{childElemType,		longVarType,	"status",		0,	0, 0, 0, 0},
		{childElemType,		longVarType,	"dTxnId",		0,	0, 0, 0, 0},
		{childElemType,		longVarType,	"subOpcode",	0,	0, 0, 0, 0},
};
genXmlFormat mcDsReqInfoRespFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_INFO_RESP_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqInfoRespFormat, 0, 0, 0}
};		

//===========================================================

// SIP Proxy --> CS
///////////////////

// SIP_CS_PROXY_REGISTER_REQ

genXmlFormat mcReqRegisterFormat[] = {
	{parentElemType,	tagVarType,		"mcReqRegister",	6,	0, 0, 0, 0},
		{childElemType,		longVarType,	"id",					0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"expires",				0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"subOpcode",			0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"proxyTransportAddr",	1,	(int)&mcXmlTransportAddressFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"registrarTransportAddr",1,	(int)&mcXmlTransportAddressFormat, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,					1,	(int)&sipMessageHeadersFormat, 0, 0, 0}
};

genXmlFormat mcDsReqRegisterFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_PROXY_REGISTER_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqRegisterFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					id;
//	APIS32					expires;
//	mcXmlTransportAddress	proxyTransportAddr;
//	mcXmlTransportAddress	registrarTransportAddr;
//	sipMessageHeaders		sipHeaders;
//} mcReqRegister;

//===========================================================
// SIP_CS_PROXY_SUBSCRIBE_REQ

genXmlFormat mcReqSubscribeFormat[] = {
	{parentElemType,	tagVarType,	"mcReqSubscribe",	6,	0, 0, 0, 0},
		{childElemType,		longVarType,	"id",				0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"expires",			0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"subOpcode",		0,	0, 0, 0, 0},
		{siblingElemType,	stringVarType,	"domainName",		0,	MaxLengthOfSingleUrl, 0, 0, 0},
		{parentElemType,	structVarType,	"transportAddr",	1,	(int)&mcXmlTransportAddressFormat, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,				1,	(int)&sipMessageHeadersFormat, 0, 0, 0}
};

genXmlFormat mcDsReqSubscribeFormat[] = {
	{rootElemType,		tagVarType,	"SIP_CS_PROXY_SUBSCRIBE_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqSubscribeFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					id;
//	APIS32					expires;
//	mcXmlTransportAddress	transportAddr;
//	sipMessageHeaders		sipHeaders;
//} mcReqSubscribe;

//===========================================================
// SIP_CS_PROXY_UNKNOWN_METHOD_REQ

genXmlFormat mcReqUnknownMethodFormat[] = {
	{parentElemType,	tagVarType,		"mcReqUnknownMethod",	5,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"remoteTransportAddress",	1,	(int)&mcXmlTransportAddressFormat, 0, 0, 0},
		{childElemType,		stringVarType,	"localAddress",				0,	MaxLengthOfSingleUrl, 0, 0, 0},
		{siblingElemType,	stringVarType,	"methodName",				0,	MaxLengthOfSipMethosName, 0, 0, 0},
		{siblingElemType,	stringVarType,	"contentType",				0,	MaxLengthOfNonSdpContentType, 0, 0, 0},
		{siblingElemType,	charArrayVarType,"content",					0,	MaxLengthOfNonSdpContent, 0, 0, 0}
};

genXmlFormat mcDsReqUnknownMethodFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_PROXY_UNKNOWN_METHOD_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqUnknownMethodFormat, 0, 0, 0}
};

//typedef struct {
//	mcXmlTransportAddress	remoteTransportAddress;	//Either UAS or Proxy
//	APIS8					localAddress[MaxLengthOfSingleUrl]; 
//	APIS8					methodName[MaxLengthOfSipMethosName];
//	APIS8					contentType[MaxLengthOfNonSdpContentType]; 
//	APIS8					content[MaxLengthOfNonSdpContent]; 
//} mcReqUnknownMethod;

//===========================================================
// SIP_CS_PROXY_NOTIFY_REQ

genXmlFormat mcReqNotifyFormat[] = {
	{parentElemType,	tagVarType,		"mcReqNotify",	3,	0, 0, 0, 0},
		{childElemType,		longVarType,	"cseq",				0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"transportAddress",	1,	(int)&mcXmlTransportAddressFormat, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,				1,	(int)&sipContentAndHeadersFormat, 0, 0, 0}
};

genXmlFormat mcDsReqNotifyFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_PROXY_NOTIFY_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqNotifyFormat, 0, 0, 0}
};

//typedef struct {
//	APIU32					cseq;
//	mcXmlTransportAddress	transportAddress;
//	sipContentAndHeadersSt	sipContentAndHeaders;
//} mcReqNotify;

//===========================================================
// SIP_CS_PROXY_SERVICE_REQ

genXmlFormat mcReqServiceFormat[] = {
	{parentElemType,	tagVarType,		"mcReqService",	4,	0, 0, 0, 0},
	{childElemType,		longVarType,	"subOpcode", 0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"id", 0, 0, 0, 0, 0},
		{parentElemType,	structVarType,	"transportAddress",	1,	(int)&mcXmlTransportAddressFormat, 0, 0, 0},		
		{parentElemType,	structVarType,	NULL,				1,	(int)&sipMessageHeadersFormat, 0, 0, 0}
		
};

genXmlFormat mcDsReqServiceFormat[] = {
	{rootElemType,		tagVarType,	"SIP_CS_PROXY_SERVICE_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqServiceFormat, 0, 0, 0}
};


//typedef struct {
//	APIS32					id;
//	APIS32					expires;
//	mcXmlTransportAddress	transportAddress;
//	sipMessageHeaders		sipHeaders;
//} mcReqService;


//===========================================================
// SIP_CS_BFCP_MESSAGE_REQ

genXmlFormat mcDsReqBfcpMessageFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_BFCP_MESSAGE_REQ",	3,	0, 0, 0, 0},
		{parentElemType,	structVarType,		"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{childElemType,		longVarType,		"status",	0,	0,						  0, 0, 0},
		{parentElemType, 	dynamicDataVarType,	NULL,		1,  (int) &bfcpMessageFormat, 0, 0,	0}
};

//===========================================================
// SIP_CS_SIG_DIALOG_RECOVERY_REQ

genXmlFormat mcReqDialogRecoveryFormat[] = {
  {parentElemType,  tagVarType,   "mcReqDialogRecovery", 1,  0, 0, 0, 0},
  {childElemType,   longVarType,    "status", 0,  0,              0, 0, 0},
};

genXmlFormat mcDsReqDialogRecoveryFormat[] = {
  {rootElemType,    tagVarType,   "SIP_CS_SIG_DIALOG_RECOVERY_REQ", 2,  0, 0, 0, 0},
    {parentElemType,  structVarType,  "csHdrs", 1,  (int)&commonHeaderFormat, 0, 0, 0},
    {parentElemType,  structVarType,  "spMsg",  1,  (int)&mcReqDialogRecoveryFormat, 0, 0, 0}
};


//Sync API
//CCS plugin pwd
//typedef struct{
//	APIU32 					status; // 0--failed, 1--success
//}mcReqCcsConfPwd;
genXmlFormat mcReqCcsConfPwdFormat[] = {
	{parentElemType,	tagVarType,		"mcReqCcsConfPwd",	1,		0, 	0, 0, 0},
		{childElemType,		longVarType,	"status",		0,	0, 0, 0, 0},
};
//end of sync

//===========================================================
//SIP_CS_SIG_SEND_CRLF_REQ
//SIP_CS_PROXY_SEND_CRLF_REQ
//typedef struct  _mcReqSendCrlf
//{
//    unsigned int            dwMsKepAliveTimeOut_Sec ; // 	
//    mcXmlTransportAddress	  registrarTransportAddr  ; //
//} 
//mcReqSendCrlf;

genXmlFormat mcReqSendCrlfFormat[] = {
  {parentElemType,  tagVarType,   "mcReqSendCrlf", 3,  0, 0, 0, 0},
  {parentElemType,  structVarType,  "registrarTransportAddr",1, (int)&mcXmlTransportAddressFormat, 0, 0, 0},
  {childElemType,   longVarType,    "dwMsKepAliveTimeOut_Sec", 0,  0, 0, 0, 0},
  {siblingElemType,   longVarType,    "dwConfId", 0,  0, 0, 0, 0},
};

genXmlFormat mcDsReqSendCrlfFormat[] = {
  {rootElemType,    tagVarType,   "SIP_CS_SIG_SEND_CRLF_REQ", 2,  0, 0, 0, 0},
    {parentElemType,  structVarType,  "csHdrs", 1,  (int)&commonHeaderFormat, 0, 0, 0},
    {parentElemType,  structVarType,  "spMsg",  1,  (int)&mcReqSendCrlfFormat, 0, 0, 0}
};

//SIP_CS_PROXY_SEND_CRLF_REQ
//genXmlFormat mcDsReqSendCrlfRegisterFormat[] = {
//    {rootElemType,    tagVarType,   "SIP_CS_PROXY_SEND_CRLF_REQ", 2,  0, 0, 0, 0},
//    {parentElemType,  structVarType,  "csHdrs", 1,  (int)&commonHeaderFormat, 0, 0, 0},
//    {parentElemType,  structVarType,  "spMsg",  1,  (int)&mcReqSendCrlfFormat, 0, 0, 0}
//};

//===========================================================
// SIP_CS_SIG_SOCKET_ACTIVITY_REQ

genXmlFormat mcReqSocketStatisticsFormat[] = {
  {parentElemType,  tagVarType,   "mcReqIndSocketStatistics", 2,  0, 0, 0, 0},
  {childElemType,   longVarType,  "dwSilenceTimeRecv_mSec", 0,  0,              0, 0, 0},
  {siblingElemType, longVarType,  "dwSilenceTimeSend_mSec", 0, 0, 0, 0, 0},
};

genXmlFormat mcDsReqSocketStatisticsFormat[] = {
  {rootElemType,    tagVarType,   "SIP_CS_SIG_SOCKET_ACTIVITY_REQ", 2,  0, 0, 0, 0},
    {parentElemType,  structVarType,  "csHdrs", 1,  (int)&commonHeaderFormat, 0, 0, 0},
    {parentElemType,  structVarType,  "spMsg",  1,  (int)&mcReqSocketStatisticsFormat, 0, 0, 0}
};


//===========================================================
// SIP_CS_MCCF_SIG_INVITE_RESPONSE_REQ

genXmlFormat mccfDsReqInviteResponseFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_MCCF_SIG_INVITE_RESPONSE_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqInviteResponseFormat, 0, 0, 0}
};

//sync
//typedef struct {
//	APIS8					label[32]; // label string
//
//} mcReqVideoFastUpdateV2;
genXmlFormat mcReqVideoFastUpdateV2Format[] = {
  {parentElemType,  tagVarType,   "mcReqVideoFastUpdateV2", 1,  0, 0, 0, 0},
  {childElemType,   charVarType,    "label", 0,  32, 0, 0, 0},

};


// SIP_CS_SIG_LPR_MODE_CHANGE_REQ
//typedef struct  {
//	APIU32					lossProtection;
//	APIU32					mtbf;
//	APIU32					congestionCeiling;
//	APIU32					fill;
//	APIU32					modeTimeout;
//
//} mcReqSipLPRModeChange;
genXmlFormat mcReqSipLPRModeChangeFormat[] = {
  {rootElemType,  tagVarType,   "SIP_CS_SIG_LPR_MODE_CHANGE_REQ", 5,  0, 0, 0, 0},
  	{childElemType,   longVarType,    "lossProtection", 0,  0, 0, 0, 0},
	{siblingElemType,   longVarType,    "mtbf", 0,  0, 0, 0, 0},
	{siblingElemType,   longVarType,    "congestionCeiling", 0,  0, 0, 0, 0},
	{siblingElemType,   longVarType,    "fill", 0,  0, 0, 0, 0},
	{siblingElemType,   longVarType,    "modeTimeout", 0,  0, 0, 0, 0},
};


//typedef struct  {
//	APIS8					label[32]; // label string
//	APIU32					mediaDirection; // enMediaDirection
//	APIU32					rate;
//
//} mcReqFlowControl;
genXmlFormat mcReqFlowControlFormat[] = {
  {parentElemType,  tagVarType,   "mcReqFlowControl", 3,  0, 0, 0, 0},
  	{childElemType,   charVarType,    "label", 0,  32, 0, 0, 0},
	{siblingElemType,   longVarType,    "mediaDirection", 0,  0, 0, 0, 0},
	{siblingElemType,   longVarType,    "rate", 0,  0, 0, 0, 0},

};

//typedef struct  {
//	APIU8					digits[MaxLengthOfDigits];
//
//} mcReqPlcmIvr;
genXmlFormat mcReqPlcmIvrFormat[] = {
  {parentElemType,  tagVarType,   "mcReqPlcmIvr", 1,  0, 0, 0, 0},
  	{childElemType,   charVarType,    "digits", 0,  MaxLengthOfDigits, 0, 0, 0},

};

// SIP_CS_CCCP_SIG_INVITE_REQ

genXmlFormat cccpInviteReqFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_CCCP_SIG_INVITE_REQ",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcReqInviteFormat, 0, 0, 0}
};


//end of sync

