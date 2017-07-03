// H323CsReqFormat.cpp
// Anat Elia

// Includes files
//----------------
#include "XmlDefs.h"
#include "XmlFormat.h"
#include "IpCsSizeDefinitions.h"
#include "IpChannelParams.h"

// Macros
//--------

#undef	SRC
#define SRC	"H323CsIndFormat"

//	Static variables
//-------------------

// Global variables
//------------------
extern dynamicTableStruct capDynamicTbl;
// External variables
//------------------
extern genXmlFormat h460AvayaFeVndrReqStFormat[];
extern genXmlFormat mcXmlTransportAddressFormat[];
extern genXmlFormat ctCapabilitiesStructFormat[];
extern genXmlFormat xmlDynamicPropertiesFormat[];
extern genXmlFormat encTokensHeaderStructFormat[];
extern genXmlFormat mcXmlCallGeneratorParamsFormat[];
//	External Routines:
//--------------------

// Forward Declarations:

//typedef struct {				
//	APIS8				sDisplay[MaxDisplaySize];		
//	APIS8				userUser[MaxUserUserSize];
//	APIBOOL				sDisplaySize;
//	APIBOOL				userUserSize;
//
//} mcCallTransient;

genXmlFormat mcCallTransientFormat[] = {
	{parentElemType,	tagVarType,	"mcCallTransient",	4,	0,0,0,0},
		{childElemType,		stringVarType,	"sDisplay",		0,	MaxDisplaySize,0,0,0},
		{siblingElemType,	stringVarType,  "userUser",		0,	MaxUserUserSize,0,0,0},
		{siblingElemType,	longVarType,  "sDisplaySize",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"userUserSize", 0,	0,0,0,0},
};
//-----------------------------------------------------------------------------------

//H323_CS_SIG_GET_PORT_REQ
//typedef struct {
//	APIU32 srcIpAddress;//replace the haCall from the common headar
//} mcReqGetPort;

genXmlFormat mcReqGetPortFormat[] = {
	{parentElemType,	tagVarType,	"mcReqGetPort",	1,	0,0,0,0},
		{childElemType,		longVarType,	"srcIpAddress",		0,	0,0,0,0},
};
//-----------------------------------------------------------------------------------

//H323_CS_SIG_RELEASE_PORT_REQ 
//typedef struct  {
//	APIU16							port;
//  APIU16							filler;
//	void							*haCall;
//} mcReqReleasePort;

genXmlFormat mcReqReleasePortFormat[] = {
	{parentElemType,	tagVarType,	"mcReqReleasePort",	3,	0,0,0,0},
		{childElemType,		shortVarType, "port",		0,	0,0,0,0},
		{siblingElemType,	shortVarType, "filler",		0,	0,0,0,0},
		{siblingElemType,	longVarType,  "haCall",		0,	0,0,0,0},
};
//-----------------------------------------------------------------------------------

//H323_CS_SIG_CALL_SETUP_REQ
//typedef struct {
//	APIS8						srcPartyAliases[MaxAddressListSize];	// src Aliases list.(content changed for Ipv6).
//	APIS8						destPartyAliases[MaxAddressListSize];	// dest Aliases list.(content changed for Ipv6).	
//	mcXmlTransportAddress		srcIpAddress;							// Changed for Ipv6.
//	mcXmlTransportAddress		destIpAddress;							// Changed for Ipv6.
//	APIS8						conferenceId[MaxConferenceIdSize];		// Conference ID for the card 
//	APIS8						callId[Size16];							// Unique parameter per H225 call 	
//	APIS32						referenceValue; 						// Unique parameter per Q931 call
//	APIU32						maxRate;			// Max conference rate (In conf. parameters).
//	APIU32						minRate;			// Min conf. rate (=0).
//	mcCallTransient				callTransient;		// display field values
//	APIU32						localEndpointType;  // MCU || GW || Terminal		
//	APIU32						type;				// P2P || One2N || N2N || N2One // Always N2N ?
//	APIU32						conferenceType;		// SWCP | VSW | CP | COP | H239 |P&C version | FECC
//	APIS32						bIsActiveMc;		// According to H225 Std. - Mandatory
//	
//	APIS32						callGoal;			// cmCreate, cmJoin, cmInvite.
//	h460AvayaFeVndrReqSt        avfFeVndIdReq;		// Extension of H.225...
//	void						*haCall;
//	// -- canMapAlias -- // 
//	APIU32						destExtraCallInfoTypes[MaxNumberOfAliases]; // ??? 
//	APIS8						destExtraCallInfo[MaxAddressListSize];	//size = 100 canMapAlias
//	APIS8						remoteExtensionAddress[MaxAliasLength];		//size = 128 canMapAlias
//	// -----------------
//	APIS32						bAuthentication;
//	APIU8						sid[Pf1SessionIdSize];	// len = 11 Session Id for authentication of Avaya		
//	APIU8						filler[1];	
//	APIU8						authKey[sizeOf128Key];	//len = 128 // Encryption For Control channel?
//	APIU32						bIsCascade;	
//	APIU32						epIdentLength;			//New fields for encryption
//	APIS8						endpointIdent[MaxIdentifierSize];
//
//  	mcCallGeneratorParams		callGeneratorParams;	// Call Generator SoftMCU CS parameters
//	encTokensHeaderStruct		encryTokens;	// must be last. // SC uses these params for H245 security signaling
//
//} mcReqCallSetup;
//


genXmlFormat mcReqCallSetupFormat[] = {
	{parentElemType,	tagVarType,	"mcReqCallSetup",	30,	0,0,0,0},
		{childElemType,		stringVarType,	"srcPartyAliases",		0,	MaxAddressListSize,0,0,0},
		{siblingElemType,	stringVarType,  "destPartyAliases",		0,	MaxAddressListSize,0,0,0},		
		{parentElemType,	structVarType,  "srcIpAddress", 1, (int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "destIpAddress", 1, (int) &mcXmlTransportAddressFormat,0,0,0},
		{siblingElemType,	charArrayVarType,  "conferenceId",			0,	MaxConferenceIdSize,0,0,0},
		{siblingElemType,	charArrayVarType,  "callId",				0,	Size16,0,0,0},
		{siblingElemType,	longVarType,	"referenceValue",		0,	0,0,0,0},
		{siblingElemType,	longVarType,  "maxRate",				0,	0,0,0,0},
		{siblingElemType,	longVarType,  "minRate",				0,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,			1,	(int) &mcCallTransientFormat,0,0,0},
		{siblingElemType,	longVarType,	"localEndpointType",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"type",					0,	0,0,0,0},
		{siblingElemType,	longVarType,	"conferenceType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"bIsActiveMc",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"callGoal",				0,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,					1,	(int) &h460AvayaFeVndrReqStFormat,0,0,0},
		{siblingElemType,	longVarType,	"haCall",				0,	0,0,0,0},
		{siblingElemType,	longListVarType,"destExtraCallInfoTypes",	MaxNumberOfAliases,	0,0,0,0},
		{siblingElemType,	stringVarType,  "destExtraCallInfo",		0,	MaxAddressListSize,0,0,0},
		{siblingElemType,	stringVarType,  "remoteExtensionAddress",	0,	MaxAliasLength,0,0,0},
		{siblingElemType,	longVarType,	"bAuthentication",				0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"sid",					0,	Pf1SessionIdSize,0,0,0},
		{siblingElemType,	charArrayVarType,  "filler",				0,	1,0,0,0},
		{siblingElemType,	charArrayVarType,  "authKey",			0,	sizeOf128Key,0,0,0},
		{siblingElemType,	longVarType,		"bIsCascade",		0,	0,0,0,0},
		{siblingElemType,	longVarType,		"length",		0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,	"endpointIdent",		0,	MaxIdentifierSize,0,0,0},
		{siblingElemType,	longVarType,		"localTerminalType",		0,	0,0,0,0},
		{parentElemType,	structVarType,	"callGeneratorParams",		1,	(int)&mcXmlCallGeneratorParamsFormat, 0, 0, 0},
		{parentElemType,	structVarType,			NULL,			1,	(int) &encTokensHeaderStructFormat,0,0,0},
};
//-----------------------------------------------------------------------------------

//H323_CS_SIG_CALL_ANSWER_REQ
//typedef struct {
//	mcXmlTransportAddress		h245Address;		// The new RV stack struct
//	mcXmlTransportAddress		remoteAddress; 		// Changed for Ipv6.
//	APIS8						conferenceId[MaxConferenceIdSize];	// Conference ID for the card 
//	mcCallTransient				callTransient;		// display field values
//	APIU32						localEndpointType;	// MCU || GW || Terminal	
//	APIU32						maxRate;			// Max conference rate (In conf. parameters).
//	APIU32						rejectCallReason;	// cmReasonType
//	APIU32						conferenceType;		// SWCP | VSW | CP | COP | H239 |P&C version | DUO | FECC | T120
//	
//	APIS32						bAuthentication;	// Avaya....
//	APIU8						sid[Pf1SessionIdSize];	// Session Id for authentication of Avaya	
//	APIU8						filler[1];	
//	APIU8						authKey[sizeOf128Key];	//len = 128 // Encryption For Control channel?
//	void						*haCall;
//	APIU32						bIsCascade;	
//	APIU32						epIdentLength;			//New fiels for encryption
//	APIS8						endpointIdent[MaxIdentifierSize];
//
//	mcCallGeneratorParams		callGeneratorParams;	// Call Generator SoftMCU CS parameters
//	encTokensHeaderBasicStruct	encryTokens;		// must be last.
//} mcReqCallAnswerBase;

genXmlFormat mcReqCallAnswerFormat[] = {
	{parentElemType,	tagVarType,	"mcReqCallAnswer",	20,	0,0,0,0},
		{parentElemType,	structVarType,  "h245Address",			1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "remoteAddress", 1, (int) &mcXmlTransportAddressFormat,0,0,0},
		{siblingElemType,	charArrayVarType,  "conferenceId",			0,	MaxConferenceIdSize,0,0,0},
		{parentElemType,	structVarType,  NULL,					1,	(int) &mcCallTransientFormat,0,0,0},
		{siblingElemType,	longVarType,	"localEndpointType",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxRate",				0,	0,0,0,0},
		{siblingElemType,	longVarType,	"rejectCallReason",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"conferenceType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"bAuthentication",		0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,  "sid",					0,	Pf1SessionIdSize,0,0,0},
		{siblingElemType,	charArrayVarType,  "filler",				0,	1,0,0,0},
		{siblingElemType,	charArrayVarType,  "authKey",				0,	sizeOf128Key,0,0,0},
		{siblingElemType,	longVarType,	"haCall",				0,	0,0,0,0},
		{siblingElemType,	longVarType,		"bIsCascade",		0,	0,0,0,0},
		{siblingElemType,	longVarType,		"length",		0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,	"endpointIdent",		0,	MaxIdentifierSize,0,0,0},
		{siblingElemType,	charArrayVarType,	"ConnectedAddressAliases",		0,	MaxAddressListSize,0,0,0},
		{siblingElemType,	longVarType,		"localTerminalType",		0,	0,0,0,0},
		{parentElemType,	structVarType,	"callGeneratorParams",		1,	(int)&mcXmlCallGeneratorParamsFormat, 0, 0, 0},
		{parentElemType,	structVarType,			NULL,					1,	(int) &encTokensHeaderStructFormat,0,0,0},
};
//-----------------------------------------------------------------------------------

// H323_CS_SIG_CREATE_CNTL_REQ
// H323_CS_SIG_RE_CAPABILITIES_REQ
//typedef struct {
//	mcXmlTransportAddress		h245IpAddress;		//The new RV stack struct
//	BOOL						masterSlaveTerminalType;
//	BOOL						capabilitiesStructLength;
//	ctCapabilitiesStruct		capabilities;
//} mcReqCreateControl;

genXmlFormat mcReqCreateControlFormat[] = {
	{parentElemType,	tagVarType,	"mcReqCreateControl",	4,	0,0,0,0},
		{parentElemType,	structVarType,	"h245IpAddress",				1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{childElemType,		longVarType,	"masterSlaveTerminalType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"capabilitiesStructLength",		0,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,							1,	(int) &ctCapabilitiesStructFormat,0,0,0},
};
//-----------------------------------------------------------------------------------

// H323_CS_SIG_INCOMING_CHNL_RESPONSE_REQ
//typedef struct {
//
//	APIU32						channelIndex;
//	APIU32						dataType;	  // audio || video || data || non standard ...
//	APIU32						channelDirection; // 0 - In / 1 - Out
//	mcXmlTransportAddress		localRtpAddressIp;			// UDP IP address to receive the incoming channel stream 
//	APIU8						dynamicPayloadType;				// Need to keep
//	APIU8						bIsEncrypted;					// Keep 
//	APIU8						encryptionAlgorithm;			// EenMediaType - Keep
//	APIS8						filler[1];						// Do we still need to keep after the DBC2 ? 
//	APIU8						EncryptedSession235Key[sizeOf128Key];		// Keep 
//} mcReqIncomingChannelResponse;

genXmlFormat mcReqIncomingChannelResponseFormat[] = {
	{parentElemType,	tagVarType,	"mcReqIncomingChannelResponse",	9,	0,0,0,0},
		{childElemType,		longVarType,	"channelIndex",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"dataType",				0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelDirection",		0,	0,0,0,0},
		{parentElemType,	structVarType,  "localRtpAddressIp",	1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{siblingElemType,	charVarType,	"dynamicPayloadType",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"bIsEncrypted",			0,	0,0,0,0},
		{siblingElemType,	charVarType,	"encryptionAlgorithm",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,  "filler",				0,	1,0,0,0},
		{siblingElemType,	charArrayVarType,  "EncryptedSession235Key",	0,	sizeOf128Key,0,0,0},
};
//-----------------------------------------------------------------------------------

// H323_CS_SIG_OUTGOING_CHNL_REQ
//typedef struct {
//	
//	mcXmlTransportAddress		SourceRtpAddressIp;				// UDP IP address to transmit the outgoing channel stream 
//
//	APIU32						payloadType;					// _PCMU || _G722 || _Siren14 || ...
//	APIU32						channelType;					// audio || video || data || non standard ...
//	APIU32						channelIndex;
//	APIU32						channelDirection;				// 0 - In / 1 - Out
//
//	APIS8						channelName[ChannelNameSize];	// Keep
//	APIU8						capTypeCode;					// g711Alaw64kCapCode || h263CapCode || ...
//	APIU8						filler1[3];
//	
//	APIU8						dynamicPayloadType;				// Keep
//	APIU8						bIsEncrypted;					// Keep
//	APIU8						encryptionAlgorithm;				// Keep
//	APIS8						filler4[1];						
//	APIU8						EncryptedSession235Key[sizeOf128Key];		// Keep
//	APIS32						bIsLPR;							// Indication LPR is supported on the Channel.
//
//	APIU16						sizeOfChannelParams;			// Need to remain
//	APIS8						filler5[2];
//	xmlDynamicProperties		xmlDynamicProps;
//	APIS8						channelSpecificParams[1];		// The channel (algorith//m) parameters
//
//} mcReqOutgoingChannel;

genXmlFormat mcReqOutgoingChannelFormat[] = {
	{parentElemType,	tagVarType,	"mcReqOutgoingChannel",	18,	0,0,0,0},
		{parentElemType,	structVarType,  "SourceRtpAddressIp",	1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{childElemType,		longVarType,	"payloadType",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelType",				0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelIndex",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelDirection",		0,	0,0,0,0},
		{siblingElemType,	stringVarType,  "channelName",	0,	ChannelNameSize,0,0,0},
		{siblingElemType,	charVarType,	"capTypeCode",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,  "filler1",	0,	3,0,0,0},
		{siblingElemType,	charVarType,	"dynamicPayloadType",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"bIsEncrypted",			0,	0,0,0,0},
		{siblingElemType,	charVarType,	"encryptionAlgorithm",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,  "filler4",				0,	1,0,0,0},
		{siblingElemType,	charArrayVarType,  "EncryptedSession235Key",	0,	sizeOf128Key,0,0,0},
		{siblingElemType,	longVarType,	"bIsLPR", 0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"sizeOfChannelParams",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,	"filler5",				0,	2,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&xmlDynamicPropertiesFormat,0,0,0},
		{parentElemType,	dynamicVarType,	"channelSpecificParams",NumOfDynamicCaps,	(int) &capDynamicTbl,0,0,0},	//Dynamic!!!!!!
};

//-----------------------------------------------------------------------------------

// H323_CS_SIG_CHAN_NEW_RATE_REQ
//typedef struct  {
//	APIU32						channelType;					// audio || video || data || non standard ...
//	APIU32						channelIndex;
//	APIU32						channelDirection;				// 0 - In / 1 - Out
//	APIU32						rate;
//} mcReqChannelNewRate;

genXmlFormat mcReqChannelNewRateFormat[] = {
	{parentElemType,	tagVarType,	"mcReqChannelNewRate",	4,	0,0,0,0},
		{childElemType,		longVarType,	"channelType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelIndex",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelDirection",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"rate",				0,	0,0,0,0},
};
//-----------------------------------------------------------------------------------

// H323_CS_SIG_CHAN_MAX_SKEW_REQ
//typedef struct  {
//	APIU32						secondChannelIndex;
//	APIU32						channelType;					// audio || video || data || non standard ...
//	APIU32						channelIndex;
//	APIU32						channelDirection;				// 0 - In / 1 - Out
//	APIU32						skew;
//} mcReqChannelMaxSkew;

genXmlFormat mcReqChannelMaxSkewFormat[] = {
	{parentElemType,	tagVarType,	"mcReqChannelMaxSkew",	5,	0,0,0,0},
		{childElemType,		longVarType,	"secondChannelIndex",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelIndex",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelDirection",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"skew",				0,	0,0,0,0},
};
//-----------------------------------------------------------------------------------

//H323_CS_SIG_CHANNEL_OFF_REQ	  
//typedef struct  {
//	APIU32						channelType;					// audio || video || data || non standard ...
//
//} mcReqChannelOff;

genXmlFormat mcReqChannelOffFormat[] = {
	{parentElemType,	tagVarType,	"mcReqChannelOff",	1,	0,0,0,0},
		{childElemType,		longVarType,	"channelType",		0,	0,0,0,0},
};
//-----------------------------------------------------------------------------------

//H323_CS_SIG_MULTIPOINTMODECOM_TERMINALID_REQ
//typedef struct  {
//	APIU32						mcuID;
//	APIU32						terminalID;	
//}mcReqMultipointModeComTerminalIDMessage;

genXmlFormat mcReqMultipointModeComTerminalIDMessageFormat[] = {
	{parentElemType,	tagVarType,	"mcReqMultipointModeComTerminalIDMessage",	2,	0,0,0,0},
		{childElemType,		longVarType,	"mcuID",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"terminalID",	0,	0,0,0,0},
};
//-----------------------------------------------------------------------------------

//H323_CS_SIG_CHANNEL_ON_REQ	  
//typedef struct  {
//	APIU32						channelType;					// audio || video || data || non standard ...
//
//} mcReqChannelOn;

genXmlFormat mcReqChannelOnFormat[] = {
	{parentElemType,	tagVarType,	"mcReqChannelOn",	1,	0,0,0,0},
		{childElemType,		longVarType,	"channelType",		0,	0,0,0,0},
};
//-----------------------------------------------------------------------------------

// H323_CS_SIG_ROLE_TOKEN_REQ
//typedef struct  {
//	APIU32						subOpcode;	// ERoleTokenOpcode
//	APIU32						mcuID;
//	APIU32						terminalID;
//	APIU32						randNumber;
//	//only for EPC:
//	APIU32                       contentProviderInfo;
//	APIU32                       label;
//	//only for H239
//	APIU16				    	bitRate;
//	APIU8					    bIsAck;
//  APIU8						filler;
//}mcReqRoleTokenMessage;

genXmlFormat mcReqRoleTokenMessageFormat[] = {
	{parentElemType,	tagVarType,	"mcReqRoleTokenMessage",	9,	0,0,0,0},
		{childElemType,		longVarType,	"subOpcode",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"mcuID",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"terminalID",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"randNumber",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"contentProviderInfo",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"label",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"bitRate",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"bIsAck",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"filler",		0,	0,0,0,0},

};
//-----------------------------------------------------------------------------------

//H323_CS_SIG_UNEXPECTED_MESSAGE_REQ
//typedef struct {
//	APIU32						badOpcode;
//	APIU32						sizeMessage;
//} mcReqUnexpectedMessageBase;

genXmlFormat mcReqUnexpectedMessageBaseFormat[] = {
	{parentElemType,	tagVarType,	"mcReqUnexpectedMessageBase",	2,	0,0,0,0},
		{childElemType,		longVarType,	"badOpcode",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sizeMessage",		0,	0,0,0,0},
};
//-----------------------------------------------------------------------------------

//H323_CS_SIG_CALL_DROP_TIMER_EXPIRED_REQ
//typedef struct {
//	APIU32						rejectCallReason;
//	ctTimerExpiredReason		reason;
//} mcReqCallDropTimerExpired;

genXmlFormat mcReqCallDropTimerExpiredFormat[] = {
	{parentElemType,	tagVarType,	"mcReqCallDropTimerExpired",	2,	0,0,0,0},
		{childElemType,		longVarType,	"rejectCallReason",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"reason",				0,	0,0,0,0},
};
//-----------------------------------------------------------------------------------

// H323_CS_SIG_CHNL_DROP_REQ
//typedef struct {
//	APIU32						channelType;					// audio || video || data || non standard ...
//	APIU32						channelIndex;
//	APIU32						channelDirection;				// 0 - In / 1 - Out
//	
//} mcReqChannelDrop;

genXmlFormat mcReqChannelDropFormat[] = {
	{parentElemType,	tagVarType,	"mcReqChannelDrop",	3,	0,0,0,0},
		{childElemType,		longVarType,	"channelType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelIndex",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelDirection",	0,	0,0,0,0},
};
//-----------------------------------------------------------------------------------

//H323_CS_SIG_CALL_DROP_REQ
//typedef struct {
//	APIU32						rejectCallReason;	// cmReasonType
//} mcReqCallDrop;

genXmlFormat mcReqCallDropFormat[] = {
	{parentElemType,	tagVarType,	"mcReqCallDrop",	1,	0,0,0,0},
		{childElemType,		longVarType,	"rejectCallReason",		0,	0,0,0,0},
};
//-----------------------------------------------------------------------------------

// H323_CS_SIG_DBC2_COMMAND_CT_ON_REQ
// H323_CS_SIG_DBC2_COMMAND_CT_OFF_REQ

//typedef struct  {
//	APIU32					refreshRate;
//	APIU32					interLeave;
//	APIU32					mpiLimit;
//	APIU32					motionVector;
//	APIU32					noEncapsulation;
//	APIU32					overlap;
//
//} mcReqDBC2Command;

genXmlFormat mcReqDBC2CommandFormat[] = {
	{parentElemType,	tagVarType,	"mcReqDBC2Command",	6,	0,0,0,0},
		{childElemType,		longVarType,	"refreshRate",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"interLeave",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"mpiLimit",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"motionVector",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"noEncapsulation",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"overlap",			0,	0,0,0,0},
};
//-----------------------------------------------------------------------------------
//H323_CS_SIG_CT_AUTHENTICATION_REQ
//typedef struct 
//{
//	int						indexTblAuth;
//	APIU8					authKey[sizeOf128Key];	//len = 128
//} mcReqAuthentication;

genXmlFormat mcReqAuthenticationFormat[] = {
	{parentElemType,	tagVarType,	"mcReqAuthentication",	2,	0,0,0,0},
		{childElemType,		longVarType,	"indexTblAuth",		0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,	"authKey",			0,	sizeOf128Key,0,0,0},
};

//-------------------------------------------------------------
// H323_CS_DTMF_INPUT_REQ
//typedef struct {
//	APIS32	sourceType;
// 	APIU8	dtmfBuffer[DtmfBuffLen];	
//}mcReqDtmfBuff;
genXmlFormat mcReqDtmfBuffFormat[] = {
	{parentElemType,	tagVarType,	"mcReqDtmfBuff", 2,	0,0,0,0},
	{childElemType,		longVarType, "sourceType",	 0,	0,0,0,0},
	{siblingElemType,	charArrayVarType, "dtmfBuffer",	0, DtmfBuffLen,0,0,0},
};


// H323_CS_FACILITY_REQ
//typedef struct {
//	UINT32	 	avfStandardId;
//	char 		e164ConferenceId[MaxAliasLength];
//}mcReqFafility;

genXmlFormat mcReqFacilityFormat[] = {
	{parentElemType,	tagVarType,	"mcReqFacility", 2,	0,0,0,0},
	{childElemType,		longVarType, "avfStandardId",	 0,	0,0,0,0},
	{siblingElemType,	stringVarType, "e164ConferenceId",	0, MaxAliasLength,0,0,0},
};

// H323_CS_SIG_LPR_MODE_CHANGE_REQ
//typedef struct  {
//	APIU32					lossProtection;
//	APIU32					mtbf;
//	APIU32					congestionCeiling;
//	APIU32					fill;
//	APIU32					modeTimeout;
//
//} mcReqLPRModeChange;
genXmlFormat mcReqLPRModeChangeFormat[] = {
	{parentElemType,	tagVarType,	"mcReqLPRModeChange",	5,	0,0,0,0},
		{childElemType,		longVarType,	"lossProtection",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"mtbf",					0,	0,0,0,0},
		{siblingElemType,	longVarType,	"congestionCeiling",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"fill",					0,	0,0,0,0},
		{siblingElemType,	longVarType,	"modeTimeout",			0,	0,0,0,0},
};

//added by Jason for ITP-Multiple channels
// H323_CS_SIG_NEW_ITP_SPEAKER_REQ
//typedef struct  {
//	APIU32					ITPType;
//	APIU32					numOfActiveLinks;
//
//} mcReqNewITPSpeaker;
genXmlFormat mcReqNewITPSpeakerFormat[] = {
	{parentElemType,	tagVarType,	"mcReqNewITPSpeaker",	2,	0,0,0,0},
		{childElemType,	longVarType,	"ITPType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"numOfActiveLinks",	0,	0,0,0,0},
};



//EOF
