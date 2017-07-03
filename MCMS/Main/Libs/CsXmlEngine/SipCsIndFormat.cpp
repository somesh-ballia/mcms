// SipCsIndFormat.cpp
// Avner Lami

// Pre-definitions:
// ----------------
#define SRC	"SipCsIndFormat"

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
extern genXmlFormat mcXmlTransportAddressFormat;
extern genXmlFormat bfcpMessageFormat;
extern genXmlFormat sipContentAndHeadersFormat;
//-S- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------//
extern genXmlFormat sipContentAndHeadersFormat_Part;
//-E- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------//
//===========================================================
// Indication formats
//===========================================================

// CS --> SIP Party
///////////////////

// SIP_CS_SIG_INVITE_IND

genXmlFormat mcIndInviteFormat[] = {
	{parentElemType,	tagVarType,		"mcIndInvite",	3,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"bIsFocus",		0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,			1,	(int)&sipSdpAndHeadersFormat, 0, 0, 0}
};

genXmlFormat spDsIndInviteFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_INVITE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndInviteFormat, 0, 0, 0}
};

//_mccf_
genXmlFormat mccfIndInviteFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_MCCF_SIG_INVITE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndInviteFormat, 0, 0, 0}
};

// cccp 
genXmlFormat cccpIndInviteFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_CCCP_SIG_INVITE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndInviteFormat, 0, 0, 0}
};



//typedef struct {
//	APIS32					status;
//	APIU32					bIsFocus;
//	sipSdpAndHeadersSt		sipSdpAndHeaders;
//} mcIndInvite;

//===========================================================
// SIP_CS_SIG_REINVITE_IND

genXmlFormat mcIndReinviteFormat[] = {
	{parentElemType,	tagVarType,		"mcIndReinvite",	3,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"bIsFocus",		0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,			1,	(int)&sipSdpAndHeadersFormat, 0, 0, 0}
};

genXmlFormat spDsIndReinviteFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_REINVITE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndReinviteFormat, 0, 0, 0}
};

//typedef mcIndInvite mcIndReInvite;

//===========================================================
// SIP_CS_SIG_INVITE_ACK_IND

genXmlFormat mcIndInviteAckFormat[] = {
	{parentElemType,	tagVarType,	"mcIndInviteAck",	2,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",		0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,			1,	(int)&sipSdpAndHeadersFormat, 0, 0, 0}
};

genXmlFormat spDsIndInviteAckFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_INVITE_ACK_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndInviteAckFormat, 0, 0, 0}
};

//_mccf_
genXmlFormat mccfIndInviteAckFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_MCCF_SIG_INVITE_ACK_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndInviteAckFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					status;
//	sipSdpAndHeadersSt		sipSdpAndHeaders;
//} mcIndInviteAck;

//===========================================================
// SIP_CS_SIG_INVITE_RESPONSE_IND

genXmlFormat mcIndInviteResponseFormat[] = {
	{parentElemType,	tagVarType,		"mcIndInviteResponse",	2,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",	0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,		1,	(int)&sipSdpAndHeadersFormat, 0, 0, 0}
};

genXmlFormat spDsIndInviteResponseFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_INVITE_RESPONSE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndInviteResponseFormat, 0, 0, 0}
};

//cccp
genXmlFormat cccpIndAddUserInviteResponseFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_CCCP_SIG_ADD_USER_INVITE_RESPONSE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndInviteResponseFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					status;
//	sipSdpAndHeadersSt		sipSdpAndHeaders;
//} mcIndInviteResponse;

//===========================================================
// SIP_CS_SIG_PROV_RESPONSE_IND

genXmlFormat mcIndProvResponseFormat[] = {
	{parentElemType,	tagVarType,	"mcIndProvResponse",	1,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",			0,	0, 0, 0, 0}
};

genXmlFormat spDsIndProvResponseFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_PROV_RESPONSE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndProvResponseFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					status;
//} mcIndProvResponse;

//===========================================================
// SIP_CS_SIG_BYE_IND

genXmlFormat mcIndByeFormat[] = {
	{parentElemType,	tagVarType,	"mcIndBye",	1,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",			0,	0, 0, 0, 0}
};

genXmlFormat spDsIndByeFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_BYE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndByeFormat, 0, 0, 0}
};

genXmlFormat mccfIndByeFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_MCCF_SIG_BYE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndByeFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					status;
//} mcIndBye;

//===========================================================
// SIP_CS_SIG_BYE_200_OK_IND

genXmlFormat mcIndBye200OkFormat[] = {
	{parentElemType,	tagVarType,	"mcIndBye200Ok",	1,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",			0,	0, 0, 0, 0}
};

genXmlFormat spDsIndBye200OkFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_BYE_200_OK_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndBye200OkFormat, 0, 0, 0}
};

genXmlFormat mccfIndBye200OkFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_MCCF_SIG_BYE_200_OK_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndBye200OkFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					status;
//} mcIndBye200Ok;

//===========================================================
// SIP_CS_SIG_CANCEL_IND

genXmlFormat mcIndCancelFormat[] = {
	{parentElemType,	tagVarType,	"mcIndCancel",	1,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",			0,	0, 0, 0, 0}
};

genXmlFormat spDsIndCancelFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_CANCEL_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndCancelFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					status;
//} mcIndCancel;

//===========================================================
// SIP_CS_SIG_VIDEO_FAST_UPDATE_IND

genXmlFormat spDsIndVideoFastUpdateFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_VIDEO_FAST_UPDATE_IND",	1,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0}
};

//typedef mcIndBase mcIndVideoFastUpdate;

//===========================================================
// SIP_CS_SIG_DTMF_DIGIT_IND

genXmlFormat mcIndDtmfDigitFormat[] = {
	{parentElemType,	tagVarType,	"mcIndDtmfDigit",	1,	0, 0, 0, 0},
		{childElemType,		charVarType,	"digit",		0,	0, 0, 0, 0}
};

genXmlFormat spDsIndDtmfDigitFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_DTMF_DIGIT_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndDtmfDigitFormat, 0, 0, 0}
};

//typedef struct {
//	APIS8					digit;
//} mcIndDtmfDigit;

//===========================================================
// SIP_CS_SIG_INSTANT_MESSAGE_IND

genXmlFormat mcIndInstantMessageFormat[] = {
	{parentElemType,	tagVarType,		"mcIndInstantMessage",	2,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",	0,	0, 0, 0, 0},
		{siblingElemType,	stringVarType,	"message",	0,	MaxLengthOfNonSdpContent, 0, 0, 0}
};

genXmlFormat spDsIndInstantMessageFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_INSTANT_MESSAGE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndInstantMessageFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					status;
//	APIS8					message[MaxLengthOfNonSdpContent];
//} mcIndInstantMessage;

//===========================================================
// SIP_CS_SIG_OPTIONS_IND

genXmlFormat mcIndOptionsFormat[] = {
	{parentElemType,	tagVarType,	"mcIndOptions",	3,	0, 0, 0, 0},
		{childElemType,		longVarType,	"pAppCall",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"remoteCseq",	0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,			1,	(int)&sipMessageHeadersFormat, 0, 0, 0}
};

genXmlFormat spDsIndOptionsFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_OPTIONS_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndOptionsFormat, 0, 0, 0}
};

//typedef struct {
//	void					*pAppCall;
//	APIU32					remoteCseq;
//	sipMessageHeaders		sipHeaders;
//} mcIndOptions;

//===========================================================
// SIP_CS_SIG_REFER_IND

genXmlFormat mcIndReferFormat[] = {
	{parentElemType,	tagVarType,		"mcIndRefer",	4,	0, 0, 0, 0},
		{childElemType,		longVarType,	"pAppCall",			0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"remoteCseq",		0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"transportAddress",	1,	(int)&mcXmlTransportAddressFormat, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,				1,	(int)&sipMessageHeadersFormat, 0, 0, 0}
};

genXmlFormat spDsIndReferFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_REFER_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndReferFormat, 0, 0, 0}
};

//typedef struct {
//	void					*pAppCall;
//	APIU32					remoteCseq;
//	mcXmlTransportAddress	transportAddress;
//	sipMessageHeaders		sipHeaders;
//} mcIndRefer;

//===========================================================
// SIP_CS_SIG_SESSION_TIMER_EXPIRED_IND

genXmlFormat spDsIndSessionTimerExpiredFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_SESSION_TIMER_EXPIRED_IND",	1,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0}
};

//_mccf_
genXmlFormat mccfIndSessionTimerExpiredFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_MCCF_SIG_SESSION_TIMER_EXPIRED_IND",	1,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0}
};

//typedef mcIndBase mcIndSessionTimerExpired;

//===========================================================
// SIP_CS_SIG_SESSION_TIMER_REINVITE_IND

genXmlFormat spDsIndSessionTimerReinviteFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_SESSION_TIMER_REINVITE_IND",	1,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0}
};

//typedef mcIndBase mcIndSessionTimerReinvite;

//===========================================================
// SIP_CS_PARTY_KEEP_ALIVE_IND

genXmlFormat mcIndKeepAliveFormat[] = {
	{parentElemType,	tagVarType,	"mcIndKeepAlive",	1,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",			0,	0, 0, 0, 0}
};

genXmlFormat spDsIndKeepAliveFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_PARTY_KEEP_ALIVE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndKeepAliveFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					status;
//} mcIndKeepAlive;

//===========================================================
// SIP_CS_SIG_TRACE_INFO_IND

genXmlFormat mcIndTraceInfoFormat[] = {
	{parentElemType,	tagVarType,	"mcIndTraceInfo",	1,	0, 0, 0, 0},
		{childElemType,		stringVarType,	"sErrMsg",	0,	MaxErrorMessageSize, 0, 0, 0}
};

genXmlFormat spDsIndTraceInfoFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_TRACE_INFO_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndTraceInfoFormat, 0, 0, 0}
};

//typedef struct {
//	APIS8					sErrMsg[MaxErrorMessageSize];
//} mcIndTraceInfo;

//===========================================================
// SIP_CS_SIG_BAD_STATUS_IND

genXmlFormat mcIndBadStatusFormat[] = {
	{parentElemType,	tagVarType,		"mcIndBadStatus",	2,	0, 0, 0, 0},
		{childElemType,		stringVarType,	"sErrMsg",				0,	MaxErrorMessageSize, 0, 0, 0},
		{siblingElemType,	longVarType,	"FailedrequestOpcode",	0,	0, 0, 0, 0}
};

genXmlFormat spDsIndBadStatusFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_BAD_STATUS_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndBadStatusFormat, 0, 0, 0}
};

//_mccf_
genXmlFormat mccfIndBadStatusFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_MCCF_SIG_BAD_STATUS_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndBadStatusFormat, 0, 0, 0}
};

//typedef struct {
//	APIS8					sErrMsg[MaxErrorMessageSize];
//	APIU32					FailedrequestOpcode;
//} mcIndBadStatus;

//===========================================================
// SIP_CS_SIG_TRANSPORT_ERROR_IND

genXmlFormat mcIndTransportErrorFormat[] = {
	{parentElemType,	tagVarType,		"mcIndTransportError",	3,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"expectedReq",	0,	0, 0, 0, 0},
		{siblingElemType,	stringVarType,	"sErrMsg",		0,	MaxErrorMessageSize, 0, 0, 0}
};

genXmlFormat spDsIndTransportErrorFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_TRANSPORT_ERROR_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndTransportErrorFormat, 0, 0, 0}
};

//_mccf_
genXmlFormat mccfIndTransportErrorFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_MCCF_SIG_TRANSPORT_ERROR_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndTransportErrorFormat, 0, 0, 0}
};


//typedef struct {
//	APIS32					status;
//	APIS32					expectedReq;
//	APIS8					sErrMsg[MaxErrorMessageSize];
//} mcIndTransportError;

//===========================================================
// SIP_CS_SIG_CARD_DATA_IND

genXmlFormat mcIndCardDataFormat[] = {
	{parentElemType,	tagVarType,		"mcIndCardData",		2,	0, 0, 0, 0},
		{childElemType,		longVarType,	"cardDataCodes",		0,	0, 0, 0, 0},
		{siblingElemType,	stringVarType,	"cardValueDetailed",	0,	MaxErrorMessageSize, 0, 0, 0}
};

genXmlFormat spDsIndCardDataFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_CARD_DATA_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndCardDataFormat, 0, 0, 0}
};

//typedef struct {
//	APIU32					cardDataCodes;
//	APIS8					cardValueDetailed[MaxErrorMessageSize];
//} mcIndCardData;

//===========================================================
// SIP_CS_SIG_SUBSCRIBE_IND

genXmlFormat mcIndSubscribeFormat[] = {
	{parentElemType,	tagVarType,		"mcIndSubscribe",	5,	0, 0, 0, 0},
		{childElemType,		longVarType,	"pAppCall",			0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"expires",			0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"remoteCseq",		0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"transportAddress",	1,	(int)&mcXmlTransportAddressFormat, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,				1,	(int)&sipMessageHeadersFormat, 0, 0, 0}
};

genXmlFormat spDsIndSubscribeFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_SUBSCRIBE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndSubscribeFormat, 0, 0, 0}
};

//typedef struct {
//	void					*pAppCall;
//	APIS32					expires;
//	APIU32					remoteCseq;
//	mcXmlTransportAddress	transportAddress;
//	sipMessageHeaders		sipHeaders;
//} mcIndSubscribe;

//===========================================================
// SIP_CS_SIG_INFO_IND

genXmlFormat mcIndInfoBuffFormat[] = {
    {parentElemType,    tagVarType, "buff",  2,  0, 0, 0, 0},
        {childElemType,     longVarType,		"length",   0,  0, 0, 0, 0},
        {siblingElemType,   charArrayVarType,	"buff",  0,  0, 0, 0, 0}
};
genXmlFormat mcIndInfoFormat[] = {
	{parentElemType,	tagVarType,		"mcIndInfo",	3,		0, 	0, 0, 0},
		{childElemType,		longVarType,		"dTxnId",		0,	0, 0, 0, 0},
			{childElemType,		longVarType,		"subOpcode",	0,	0, 0, 0, 0},
		{parentElemType,	dynamicDataVarType,	NULL,			1,	(int)&mcIndInfoBuffFormat, 0, 0, 0}
};

	genXmlFormat spDsIndInfoFormat[] = {
		{rootElemType,		tagVarType,		"SIP_CS_SIG_INFO_IND",	2,	0, 0, 0, 0},
			{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
			{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndInfoFormat, 0, 0, 0}
};

//typedef struct
//{
//	APIU32					dTxnId;
//	APIU32					subOpcode; // enGeneralInfoOpcode
//	APIU32					dynamicLen;
//	APIU8					buff[1];	// mcIndVideoFastUpdate | mcIndVideoFastUpdateV2 | mcIndSipLPRModeChange | mcIndFlowControl
//} mcIndInfo;
genXmlFormat mcIndInfoRespFormat[] = {
	{parentElemType,	tagVarType,		"mcIndInfoResp",	4,		0, 	0, 0, 0},
		{childElemType,		longVarType,	"status",		0,	0, 0, 0, 0},
		{childElemType,		longVarType,	"dTxnId",		0,	0, 0, 0, 0},
		{childElemType,		longVarType,	"subOpcode",	0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,			1,	(int)&sipMessageHeadersFormat, 0, 0, 0}
};
	genXmlFormat spDsIndInfoRespFormat[] = {
		{rootElemType,		tagVarType,		"SIP_CS_SIG_INFO_RESP_IND",	2,	0, 0, 0, 0},
			{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
			{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndInfoRespFormat, 0, 0, 0}
};

//Sync API
//===========================================================
// VideoFastUpdateV2
//typedef struct {
//	APIS8					label[32]; // label string
//
//} mcIndVideoFastUpdateV2;

genXmlFormat mcIndVideoFastUpdateV2Format[] = {
	{parentElemType,		tagVarType,		"mcIndVideoFastUpdateV2",		1,	0, 0, 0, 0},
		{childElemType,		stringVarType,		"label",			0,	32, 0, 0, 0}
};
//===========================================================
// mcIndSipLPRModeChange
//typedef struct  {
//	APIU32					lossProtection;
//	APIU32					mtbf;
//	APIU32					congestionCeiling;
//	APIU32					fill;
//	APIU32					modeTimeout;
//
//} mcIndSipLPRModeChange;
genXmlFormat mcIndSipLPRModeChangeFormat[] = {
	{parentElemType,	tagVarType,		"mcIndSipLPRModeChange",		5,	0, 0, 0, 0},
		{childElemType,		longVarType,	"lossProtection",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"mtbf",				0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"congestionCeiling",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"fill",				0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"modeTimeout",			0,	0, 0, 0, 0}
};


//===========================================================
// mcIndFlowControl
//typedef struct  {
//	APIS8					label[32]; // label string
//	APIU32					mediaDirection; // enMediaDirection
//	APIU32					rate;
//
//} mcIndFlowControl;
genXmlFormat mcIndFlowControlFormat[] = {
	{parentElemType,		tagVarType,		"mcIndFlowControl",		3,	0, 0, 0, 0},
		{childElemType,		stringVarType,		"label",			0,	32, 0, 0, 0},
		{siblingElemType,	longVarType,		"mediaDirection",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,		"rate",				0,	0, 0, 0, 0},

};

//===========================================================
// mcIndFlowControl
//typedef struct  {
//	APIS8					pwd[64]; // pwd string
//} mcIndConfPwdInfo;
genXmlFormat mcIndConfPwdInfoFormat[] = {
	{parentElemType,		tagVarType,		"mcIndConfPwdInfo",		1,	0, 0, 0, 0},
		{childElemType,		stringVarType,		"pwd",			0,	64, 0, 0, 0}
};
//end of sync

//===========================================================
// SIP_CS_BFCP_MESSAGE_IND

genXmlFormat spDsIndBfcpMessageFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_BFCP_MESSAGE_IND",	3,	0, 0, 0, 0},
		{parentElemType,	structVarType,		"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{childElemType,		longVarType,		"status",	0,	0,						  0, 0, 0},
		{parentElemType, 	dynamicDataVarType,	NULL,		1,  (int) &bfcpMessageFormat, 0, 0,	0}
};

//===========================================================
// SIP_CS_BFCP_TRANSPORT_IND

genXmlFormat spDsIndBfcpTransportFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_BFCP_TRANSPORT_IND",	4,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",		1,	(int)&commonHeaderFormat, 0, 0, 0},
		{childElemType,		longVarType,	"status",		0,	0,						  0, 0, 0},
		{parentElemType,	structVarType,	"hostAddress",	1,	(int)&mcXmlTransportAddressFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"remoteAddress",1,	(int)&mcXmlTransportAddressFormat, 0, 0, 0}
};
//===========================================================
// SIP_CS_SIG_NOTIFY_RESPONSE_IND

genXmlFormat mcIndNotifyRespFormat[] = {
	{parentElemType,	tagVarType,	"mcIndNotifyResp",	2,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",	0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"id",		0,	0, 0, 0, 0}
};

genXmlFormat spDsIndNotifyRespFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_NOTIFY_RESPONSE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndNotifyRespFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					status;
//	APIS32					id;
//} mcIndNotifyResp;




//===========================================================

//  CS --> SIP Proxy
////////////////////

// SIP_CS_PROXY_REGISTER_RESPONSE_IND

genXmlFormat mcIndRegisterRespFormat[] = {
	{parentElemType,	tagVarType,		"mcIndRegisterResp",	4,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",	0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"id",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"expires",	0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,		1,	(int)&sipMessageHeadersFormat, 0, 0, 0}
};

genXmlFormat prxDsIndRegisterRespFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_PROXY_REGISTER_RESPONSE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndRegisterRespFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					status;
//	APIS32					id;
//	APIS32					expires;
//	sipMessageHeaders		sipHeaders;
//} mcIndRegisterResp;

//===========================================================
// SIP_CS_PROXY_SUBSCRIBE_RESPONSE_IND

genXmlFormat mcIndSubscribeRespFormat[] = {
	{parentElemType,	tagVarType,		"mcIndSubscribeResp",	5,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",	0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"id",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"expires",	0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,	"subOpcode",0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,		1,	(int)&sipMessageHeadersFormat, 0, 0, 0}
};

genXmlFormat prxDsIndSubscribeRespFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_PROXY_SUBSCRIBE_RESPONSE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndSubscribeRespFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					status;
//	APIS32					id;
//	APIS32					expires;
//	sipMessageHeaders		sipHeaders;
//} mcIndSubscribeResp;

//===========================================================
// SIP_CS_PROXY_NOTIFY_IND

genXmlFormat mcIndNotifyFormat[] = {
	{parentElemType,	tagVarType,	"mcIndNotify",	1,	0, 0, 0, 0},
		//{childElemType,		stringVarType,	"callId",	0,	MaxLengthOfSipCallId, 0, 0, 0},
		//{siblingElemType,	charArrayVarType,"message",	0,	MaxLengthOfNonSdpContent, 0, 0, 0}
		{parentElemType,	structVarType,	NULL,		1,	(int)&sipMessageHeadersFormat, 0, 0, 0}
};

genXmlFormat prxDsIndNotifyFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_PROXY_NOTIFY_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndNotifyFormat, 0, 0, 0}
};

//typedef struct {
//	APIS8					callId[MaxLengthOfSipCallId];
//	APIS8					message[MaxLengthOfNonSdpContent];
//} mcIndNotify;

//===========================================================
//SIP_CS_PROXY_SERVICE_RESPONSE_IND,

genXmlFormat mcIndServiceRespFormat[] = {
	{parentElemType,	tagVarType,		"mcIndServiceResp",	4,	0, 0, 0, 0},
		{childElemType,		longVarType,	"status",	0,	0, 0, 0, 0},
		{childElemType,		longVarType,	"id",	0,	0, 0, 0, 0},
		{childElemType,		longVarType,    "subOpcode",	0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,		1,	(int)&sipMessageHeadersFormat, 0, 0, 0}
};

genXmlFormat prxDsIndServiceRespFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_PROXY_SERVICE_RESPONSE_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndServiceRespFormat, 0, 0, 0}
};


//===========================================================
// SIP_CS_PROXY_TRACE_INFO_IND

genXmlFormat prxDsIndTraceInfoFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_PROXY_TRACE_INFO_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndTraceInfoFormat, 0, 0, 0}
};

//===========================================================
// SIP_CS_PROXY_BAD_STATUS_IND

genXmlFormat prxDsIndBadStatusFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_PROXY_BAD_STATUS_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndBadStatusFormat, 0, 0, 0}
};

//===========================================================
// SIP_CS_PROXY_TRANSPORT_ERROR_IND

genXmlFormat prxDsIndTransportErrorFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_PROXY_TRANSPORT_ERROR_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndTransportErrorFormat, 0, 0, 0}
};

//===========================================================
// SIP_CS_PROXY_CARD_DATA_IND

genXmlFormat prxDsIndCardDataFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_PROXY_CARD_DATA_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndCardDataFormat, 0, 0, 0}
};

//===========================================================
// SIP_CS_SIG_DIALOG_RECOVERY_IND

genXmlFormat mcIndDialogRecoveryFormat[] = {
  {parentElemType,  tagVarType, "mcIndDialogRecovery", 1,  0, 0, 0, 0},
    {childElemType,   longVarType,  "status",     0,  0, 0, 0, 0}
};

genXmlFormat spDsIndDialogRecoveryFormat[] = {
  {rootElemType,    tagVarType,   "SIP_CS_SIG_DIALOG_RECOVERY_IND", 2,  0, 0, 0, 0},
    {parentElemType,  structVarType,  "csHdrs", 1,  (int)&commonHeaderFormat, 0, 0, 0},
    {parentElemType,  structVarType,  "spMsg",  1,  (int)&mcIndDialogRecoveryFormat, 0, 0, 0}
};

//===========================================================
// SIP_CS_SIG_SOCKET_ACTIVITY_REQ/SIP_CS_SIG_SOCKET_ACTIVITY_IND

genXmlFormat mcIndSocketStatisticsFormat[] = {
  {parentElemType,  tagVarType,   "mcReqIndSocketStatistics", 2,  0, 0, 0, 0},
  {childElemType,   longVarType,  "dwSilenceTimeRecv_mSec", 0,  0, 0, 0, 0},
  {siblingElemType, longVarType,  "dwSilenceTimeSend_mSec", 0, 0, 0, 0, 0},
};

genXmlFormat spDsIndSocketStatisticsFormat[] = {
  {rootElemType,    tagVarType,   "SIP_CS_SIG_SOCKET_ACTIVITY_IND", 2,  0, 0, 0, 0},
    {parentElemType,  structVarType,  "csHdrs", 1,  (int)&commonHeaderFormat, 0, 0, 0},
    {parentElemType,  structVarType,  "spMsg",  1,  (int)&mcIndSocketStatisticsFormat, 0, 0, 0}
};

//===========================================================
// SIP_CS_SIG_CRLF_ERR_IND

genXmlFormat mcIndSigCrlfErrorFormat[] = {
  {parentElemType,  tagVarType,   "mcIndSigCrlfError", 1,  0, 0, 0, 0},
  {childElemType,   longVarType,  "eCrlfSendingErrorCode", 0,  0, 0, 0, 0},
};

genXmlFormat spDsIndSigCrlfErrorFormat[] = {
  {rootElemType,    tagVarType,   "SIP_CS_SIG_CRLF_ERR_IND", 2,  0, 0, 0, 0},
    {parentElemType,  structVarType,  "csHdrs", 1,  (int)&commonHeaderFormat, 0, 0, 0},
    {parentElemType,  structVarType,  "spMsg",  1,  (int)&mcIndSigCrlfErrorFormat, 0, 0, 0}
};

//===========================================================
// SIP_CS_SIG_CRLF_ERR_IND

genXmlFormat mcIndProxyCrlfErrorFormat[] = {
  {parentElemType,  tagVarType,   "mcIndProxyCrlfError", 2,  0, 0, 0, 0},
  {childElemType,   longVarType,  "eCrlfSendingErrorCode", 0,  0, 0, 0, 0},
  {siblingElemType, longVarType,  "nConfId", 0, 0, 0, 0, 0},
};

genXmlFormat prxDsIndProxyCrlfErrorFormat[] = {
  {rootElemType,    tagVarType,   "SIP_CS_PROXY_CRLF_ERR_IND", 2,  0, 0, 0, 0},
    {parentElemType,  structVarType,  "csHdrs", 1,  (int)&commonHeaderFormat, 0, 0, 0},
    {parentElemType,  structVarType,  "spMsg",  1,  (int)&mcIndProxyCrlfErrorFormat, 0, 0, 0}
};

//===========================================================
// SIP_CS_PROXY_BENOTIFY_IND

genXmlFormat mcIndBenotifyFormat[] = {
	{parentElemType,	tagVarType,		"mcIndBenotify",	2,	0, 0, 0, 0},
		{childElemType,		longVarType,	"expires",	0,	0, 0, 0, 0},
		{parentElemType,	structVarType,	NULL,				1,	(int)&sipContentAndHeadersFormat, 0, 0, 0}
};

genXmlFormat spDsIndBenotifyFormat[] = {
	{rootElemType,		tagVarType,		"SIP_CS_SIG_BENOTIFY_IND",	2,	0, 0, 0, 0},
		{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
		{parentElemType,	structVarType,	"spMsg",	1,	(int)&mcIndBenotifyFormat, 0, 0, 0}
};

//typedef struct {
//	APIS32					expires;
//	sipContentAndHeadersSt	sipContentAndHeaders;
//
//} mcIndBenotify;

//===========================================================

//-S- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------//
//===========================================================
// SIP_CS_PROXY_BENOTIFY_IND  (by PARTLS)
genXmlFormat mcIndBenotifyFormat_Part[] = {
{parentElemType,	tagVarType,		"mcIndBenotify_Part",	2,	0, 0, 0, 0},
{childElemType,		longVarType,	"expires",	0,	0, 0, 0, 0},
{parentElemType,	structVarType,	NULL,				1,	(int)&sipContentAndHeadersFormat_Part, 0, 0, 0}
};

genXmlFormat spDsIndBenotifyFormat_Part[] = {
{rootElemType,		tagVarType,		"SIP_CS_SIG_BENOTIFY_IND",	2,	0, 0, 0, 0},
{parentElemType,	structVarType,	"csHdrs",	1,	(int)&commonHeaderFormat, 0, 0, 0},
{parentElemType,	structVarType,	"spMsg_Part",	1,	(int)&mcIndBenotifyFormat_Part, 0, 0, 0}
};

//typedef struct _mcIndBenotify_Part
//{
//	APIS32						expires;
//	sipContentAndHeadersSt_Part	sipContentAndHeadersPart;

//} mcIndBenotify_Part;

//===========================================================
//-S- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------//
