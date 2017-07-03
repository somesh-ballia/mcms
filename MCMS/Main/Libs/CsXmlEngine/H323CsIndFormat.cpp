// H323CsIndFormat.cpp
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

// External variables
//------------------
extern dynamicTableStruct capDynamicTbl;

//	External Routines:
//--------------------
extern genXmlFormat mcXmlTransportAddressFormat[]; //done
extern genXmlFormat partyAddressListFormat[];	//?????
extern genXmlFormat encTokensHeaderStructFormat[];
extern genXmlFormat ctCapabilitiesStructFormat[];
extern genXmlFormat xmlDynamicPropertiesFormat[];
extern genXmlFormat h460AvayaFeVndrIndStFormat[];
extern genXmlFormat h460AvayaMaxNonAudioBitRateIndStFormat[];
extern genXmlFormat ctNonStandardParameterStFormat[];
// Forward Declarations:
//----------------------

//-------------------------------------------------------------
//H323_CS_SIG_GET_PORT_IND
//typedef struct {
//	mcXmlTransportAddress			srcCallSignalAddress;	//The new RV stack struct(srcCallSignalAddress)
//							  
//} mcIndGetPort;

genXmlFormat mcIndGetPortFormat[] = {
	{parentElemType,	tagVarType,	"mcIndGetPort",	1,	0,0,0,0},
		{parentElemType,	structVarType,  "srcCallSignalAddress",   1,	(int) &mcXmlTransportAddressFormat,0,0,0},
};
//-------------------------------------------------------------
//H323_CS_SIG_CALL_DIAL_TONE_IND		// 323 Setup IND from RV stack (Inner ind within SC) - Mcms not involved
//H323_CS_SIG_CALL_PROCEEDING_IND		// The first answer the EP gives to Setup_req is this - Meaning got your call and I'm handling - Mcms not involved
//H323_CS_SIG_CALL_RING_BACK_IND		// EP sent alerting (there is a ringback tone) - Mcms not involved.
//typedef struct  {
//	mcXmlTransportAddress			h245IpAddress;	//The new RV stack struct
//
//} mcIndCallReport;

genXmlFormat mcIndCallReportFormat[] = {
	{parentElemType,	tagVarType,	"mcIndCallReport",	1,	0,0,0,0},
		{parentElemType,	structVarType,  "h245IpAddress",   1,	(int) &mcXmlTransportAddressFormat,0,0,0},
};
//-------------------------------------------------------------
// H323_CS_SIG_CALL_OFFERING_IND
//typedef struct  {
//	partyAddressList			srcPartyAliases;			// src Aliases list.(content changed for Ipv6).
//	partyAddressList			destPartyAliases;			// dest Aliases list.(content changed for Ipv6).
//	mcXmlTransportAddress		srcIpAddress;				// Changed for Ipv6.
//	mcXmlTransportAddress		destIpAddress;				// Changed for Ipv6.
//	
//	APIU32						rate;						// Party rate
//	APIS8						sDisplay[MaxDisplaySize];
//	APIS8						userUser[MaxUserUserSize];	
//	APIS32						userUserSize;
//
//	APIS8						conferenceId[MaxConferenceIdSize];				
//	APIS8						callId[Size16];				
//	APIU32						conferenceGoal; 	// create || envite || join		
//	APIS32						referenceValue; 			
//	APIU32						type;				// P2P || One2N || N2N || N2One						
//	APIS32						bIsActiveMc;		// Active multipoint controller
//	APIS32						bIsOrigin; 			// Always false		
//	APIS32						bH245Establish;		// H245 connection ? yes/no
//	APIU32						srcEndpointType;			
//	APIS32						localH225Port;
//	APIS32						indexTblAuth;	// ?? Avaya ?
//
//	encTokensHeaderStruct		encryTokens;	// must be last.
//
//} mcIndCallOffering;

genXmlFormat mcIndCallOfferingFormat[] = {
	{parentElemType,	tagVarType,	"mcIndCallOffering",	21,	0,0,0,0},
		{childElemType,		stringVarType,  "srcPartyAliases",  0,	MaxAddressListSize,0,0,0},
		{siblingElemType,	stringVarType,  "destPartyAliases",	0,	MaxAddressListSize,0,0,0},
		{parentElemType,	structVarType,  "srcIpAddress",	1, (int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "destIpAddress",1, (int) &mcXmlTransportAddressFormat,0,0,0},		
		{siblingElemType,	longVarType,	"rate",			0,	0,0,0,0},
		{siblingElemType,	stringVarType,  "sDisplay",		0,	MaxDisplaySize,0,0,0},
		{siblingElemType,	stringVarType,  "userUser",		0,	MaxUserUserSize,0,0,0},
		{siblingElemType,	longVarType,	"userUserSize", 0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"conferenceId", 0,	MaxConferenceIdSize,0,0,0},
		{siblingElemType,	charArrayVarType,  "callId",		0,	Size16,0,0,0},
		{siblingElemType,	longVarType,	"conferenceGoal",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"referenceValue",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"type",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"bIsActiveMc",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"bIsOrigin",	0,	0,0,0,0},
		{parentElemType,	structVarType,  "h245IpAddress",	1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{siblingElemType,	longVarType,	"bH245Establish",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"srcEndpointType",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"localH225Port",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"indexTblAuth",		0,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,			1,	(int) &encTokensHeaderStructFormat,0,0,0},
};
//-------------------------------------------------------------
//typedef struct
//{
//    RvUint8     t35CountryCode;
//    RvUint8     t35Extension;
//    RvUint16    manufacturerCode;
//} cmNonStandard;

genXmlFormat cmNonStandardFormat[] = {
	{parentElemType,	tagVarType,	"cmNonStandard",	3,	0,0,0,0},
		{childElemType,		charVarType,	"t35CountryCode",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"t35Extension",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"manufacturerCode",	0,	0,0,0,0},
};
//-------------------------------------------------------------

//typedef struct
//{
//    cmNonStandard   info;
//    int             productLen;
//    int             versionLen;
//    char            productID[256];
//    char            versionID[256];
//} cmVendor;

genXmlFormat cmVendorFormat[] = {
	{parentElemType,	tagVarType,	"cmVendor",	5,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &cmNonStandardFormat,0,0,0},
		{childElemType,		longVarType,	"productLen",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"versionLen",	0,	0,0,0,0},
		{siblingElemType,	stringVarType,	"productID",	0,	256,0,0,0},
		{siblingElemType,	stringVarType,	"versionID",	0,	256,0,0,0},
};
//-------------------------------------------------------------


//H323_CS_SIG_CALL_CONNECTED_IND
//typedef struct  {
//	mcXmlTransportAddress		h225remote;	//The new RV stack struct (Remote)
//	mcXmlTransportAddress		h225local;	//The new RV stack struct (Local)
//	mcXmlTransportAddress		h245remote;	//The new RV stack struct
//	mcXmlTransportAddress		h245local;	//The new RV stack struct
//	APIS8						sDisplay[MaxDisplaySize];
//	APIS8						userUser[MaxUserUserSize];
//	APIS32						userUserSize;
//	APIU32						remoteEndpointType;		
//	APIS32						h225RemoteVersion;
//	APIU32						endPointNetwork;
//	BOOL						bAuthenticated;
//	h460AvayaFeVndrIndSt	    avfFeVndIdInd;
//	cmVendor					remoteVendor;
//	
//	encTokensHeaderStruct		encryTokens;	// must be last.
//
//} mcIndCallConnected;

genXmlFormat mcIndCallConnectedFormat[] = {
	{parentElemType,	tagVarType,	"mcIndCallConnected",	14,	0,0,0,0},
		{parentElemType,	structVarType,  "h225remote",   1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "h225local",    1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "h245remote",   1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "h245local",    1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{childElemType,		stringVarType,  "sDisplay",		0,	MaxDisplaySize,0,0,0},
		{siblingElemType,	stringVarType,	"userUser",		0,	MaxUserUserSize,0,0,0},
		{siblingElemType,	longVarType,	"userUserSize", 0,	0,0,0,0},
		{siblingElemType,	longVarType,	"remoteEndpointType",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"h225RemoteVersion",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"endPointNetwork",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"bAuthenticated",		0,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,					1,	(int) &h460AvayaFeVndrIndStFormat,0,0,0},
		{parentElemType,	structVarType,	NULL,					1,	(int) &cmVendorFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,					1,	(int) &encTokensHeaderStructFormat,0,0,0},
};
//-------------------------------------------------------------

//H323_CS_SIG_CALL_NEW_RATE_IND
//typedef struct  {
//	APIU32						rate;
//} mcIndCallNewRate;

genXmlFormat mcIndCallNewRateFormat[] = {
	{parentElemType,	tagVarType,	"mcIndCallNewRate",	1,	0,0,0,0},
		{childElemType,	longVarType,  "rate",   0,	0,0,0,0},
};
//-------------------------------------------------------------

//H323_CS_SIG_CAPABILITIES_IND

//typedef struct  {
//	APIBOOL						capabilitiesSize;
//	ctCapabilitiesStruct		capabilities;
//} mcIndCapabilities;

genXmlFormat mcIndCapabilitiesFormat[] = {
	{parentElemType,	tagVarType,	"mcIndCapabilities",	2,	0,0,0,0},
		{childElemType,		longVarType,	"capabilitiesSize",	0,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &ctCapabilitiesStructFormat,0,0,0},
};

//-------------------------------------------------------------
//H323_CS_SIG_CALL_CNTL_CONNECTED_IND
//typedef struct  {
//	mcXmlTransportAddress		remoteH245Address;	//The new RV stack struct
//	APIU32						masterSlaveStatus;
//	APIS8						userUser[MaxUserUserSize];	
//	APIS32						userUserSize;
//	mcXmlTransportAddress		localH245Address;	//(Local)//The new RV stack struct
//} mcIndCallControlConnected;

genXmlFormat mcIndCallControlConnectedFormat[] = {
	{parentElemType,	tagVarType,	"mcIndCallControlConnected",	5,	0,0,0,0},
		{parentElemType,	structVarType,	"remoteH245Address",	1,	(int)&mcXmlTransportAddressFormat,0,0,0},
		{childElemType,		longVarType,	"masterSlaveStatus",	0,	0,0,0,0},
		{siblingElemType,	stringVarType,	"userUser",				0,	MaxUserUserSize,0,0,0},
		{siblingElemType,	longVarType,	"userUserSize",			0,	0,0,0,0},
		{parentElemType,	structVarType,	"localH245Address",		1,	(int)&mcXmlTransportAddressFormat,0,0,0},
};
//-------------------------------------------------------------

//H323_CS_SIG_CALL_BAD_SPONTAN_IND
//typedef struct {
//  BadSpontanIndReason			status; //the value set according to BadSpontanIndReason enum
//
//} mcIndBadSpontan;

genXmlFormat mcIndBadSpontanFormat[] = {
	{parentElemType,	tagVarType,	"mcIndBadSpontan",	1,	0,0,0,0},
		{childElemType,		longVarType,	"status",	0,	0,0,0,0},
};
//-------------------------------------------------------------

//H323_CS_SIG_CALL_ROLE_TOKEN_IND
//typedef struct  {
//	
//	APIU32						subOpcode; // ERoleTokenOpcode
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
//} mcIndRoleToken;

genXmlFormat mcIndRoleTokenFormat[] = {
	{parentElemType,	tagVarType,	"mcIndRoleToken",	9,	0,0,0,0},
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
//-------------------------------------------------------------

//H323_CS_SIG_CALL_AUTHENTICATION_IND
//typedef struct {
//	BOOL						bAuthenticated;
//
//} mcIndAuthentication;

genXmlFormat mcIndAuthenticationFormat[] = {
	{parentElemType,		tagVarType,	"mcIndAuthentication",	1,	0,0,0,0},
		{childElemType,		longVarType,	"bAuthenticated",			0,	0,0,0,0},
};
//-------------------------------------------------------------

//H323_CS_SIG_INCOMING_CHANNEL_IND

//typedef struct  {
//
//	APIU32						payloadType;
//	APIU32						dataType;				// audio || video || data || non standard ...
//	APIU32						channelIndex;
//	APIU32						channelDirection;				// 0 - In / 1 - Out
//	mcXmlTransportAddress		rmtRtpAddress; // New field ,provide here remote rtp Address/ rtcp is odd port incremental
//  APIS8						channelName[ChannelNameSize];	
//	APIU8						capTypeCode;
//	APIU8						filler[3];
//
//	APIU32						rate;
//	
//	BOOL						bIsActive;						// Is this an active channel
//	APIU32						sameSessionChannelIndex;
//	APIS32						sessionId;
//
//	APIS8						dynamicPayloadType;
//
//	APIU8						bIsEncrypted;
//	APIU8						encryptionAlgorithm;			// EenMediaType
//	APIU8						EncryptedSession235Key[sizeOf128Key];
//	APIU8						bUsedH263Plus;
//  APIS32						bIsLPR;
//
//	APIU16						sizeOfChannelParams;
//	APIU16						filler2;
//	xmlDynamicProperties		xmlDynamicProps;
//	APIS8						channelSpecificParams[1];
//
//	
//} mcIndIncomingChannel;

genXmlFormat mcIndIncomingChannelFormat[] = {
	{parentElemType,	tagVarType,	"mcIndIncomingChannel",	22,	0,0,0,0},
		{childElemType,		longVarType,	"payloadType",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"dataType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelIndex",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelDirection",	0,	0,0,0,0},
		{parentElemType,	structVarType,	"rmtRtpAddress",	1,	(int)&mcXmlTransportAddressFormat,0,0,0},
		{siblingElemType,	stringVarType,	"channelName",		0,	ChannelNameSize,0,0,0},
		{siblingElemType,	charVarType,	"capTypeCode",		0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"filler",			0,	3,0,0,0},
		{siblingElemType,	longVarType,	"rate",				0,	0,0,0,0},
		{siblingElemType,	longVarType,	"bIsActive",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sameSessionChannelIndex",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sessionId",			0,	0,0,0,0},
		{siblingElemType,	charVarType,	"dynamicPayloadType",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"bIsEncrypted",			0,	0,0,0,0},
		{siblingElemType,	charVarType,	"encryptionAlgorithm",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"EncryptedSession235Key",	0,	sizeOf128Key,0,0,0},
		{siblingElemType,	charVarType, 	"bUsedH263Plus",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"bIsLPR", 0, 0,0,0,0},
		{siblingElemType,	shortVarType,	"sizeOfChannelParams",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"filler2",				0,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,					1,	(int)	&xmlDynamicPropertiesFormat,0,0,0},
		{parentElemType,	dynamicVarType,	"channelSpecificParams",NumOfDynamicCaps,	(int) &capDynamicTbl,0,0,0},	//Dynamic!!!!!!
};
//-------------------------------------------------------------
//H323_CS_SIG_INCOMING_CHNL_CONNECTED_IND 
//typedef struct  {
//
//	APIU32						channelType;					// audio || video || data || non standard ...
//	APIU32						channelIndex;
//	APIU32						channelDirection;				// 0 - In / 1 - Out
//	APIS32						sessionId;						// Session Id of channel	
//	APIU32						sameSessionChannelIndex;		// Remove ? (From MCMS + RTP related)
//	APIU32						associatedChannelIndex;			// Remove ? (From MCMS + RTP related)
//
//} mcIndIncomingChannelConnected;

genXmlFormat mcIndIncomingChannelConnectedFormat[] = {
	{parentElemType,	tagVarType,	"mcIndIncomingChannelConnected",	6,	0,0,0,0},
		{childElemType,		longVarType,	"channelType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelIndex",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelDirection",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sessionId",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sameSessionChannelIndex",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"associatedChannelIndex",	0,	0,0,0,0},
};
//-------------------------------------------------------------

//H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND
//typedef struct  {
//	APIU32						channelType;					// audio || video || data || non standard ...
//	APIU32						channelIndex;
//	APIU32						channelDirection;				// 0 - In / 1 - Out
//	APIS32						sessionId;						// Session Id of channel
//
//	APIU32						payloadType;				// Confirmation from the EP.
//	APIU32						sameSessionChannelIndex;	// ?? - RTP associated	
//	APIU32						associatedChannelIndex;		// ?? - RTP associated
//	mcXmlTransportAddress		destRtpAddress;				// Added provided on OLCA 
//	APIS8						dynamicPayloadType;
//	APIU8						filler[1];
//	APIU8						bIsEncrypted;
//	APIU8						encryptionAlgorithm;
//	APIU8						EncryptedSession235Key[sizeOf128Key];

//} mcIndOutgoingChannelResponse; 

genXmlFormat mcIndOutgoingChannelResponseFormat[] = {
	{parentElemType,	tagVarType,	"mcIndOutgoingChannelResponse",	13,	0,0,0,0},
		{childElemType,		longVarType,	"channelType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelIndex",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelDirection",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sessionId",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"payloadType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sameSessionChannelIndex",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"associatedChannelIndex",	0,	0,0,0,0},
		{parentElemType,	structVarType,	"destRtpAddress",	1,	(int)&mcXmlTransportAddressFormat,0,0,0},
		{siblingElemType,	charVarType,	"dynamicPayloadType",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"filler", 			0, 1,0,0,0},
		{siblingElemType,	charVarType,	 "bIsEncrypted",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	 "encryptionAlgorithm",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"EncryptedSession235Key",	0,	sizeOf128Key,0,0,0},
};
//-------------------------------------------------------------

//H323_CS_SIG_CHAN_NEW_RATE_IND
//typedef struct  {
//	APIU32						channelType;					// audio || video || data || non standard ...
//	APIU32						channelIndex;
//	APIU32						channelDirection;				// 0 - In / 1 - Out
//
//	APIU32						rate;
//} mcIndChannelNewRate;

genXmlFormat mcIndChannelNewRateFormat[] = {
	{parentElemType,	tagVarType,	"mcIndChannelNewRate",	4,	0,0,0,0},
		{childElemType,		longVarType,	"channelType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelIndex",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelDirection",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"rate",				0,	0,0,0,0},
};
//-------------------------------------------------------------

//H323_CS_SIG_START_CHANNEL_CLOSE_IND
//typedef struct  {
//	APIU32						channelType;					// audio || video || data || non standard ...
//	APIU32						channelIndex;
//	APIU32						channelDirection;				// 0 - In / 1 - Out
//
//} mcIndStartChannelClose; 

genXmlFormat mcIndStartChannelCloseFormat[] = {
	{parentElemType,	tagVarType,	"mcIndStartChannelClose",	3,	0,0,0,0},
		{childElemType,		longVarType,	"channelType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelIndex",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelDirection",	0,	0,0,0,0},
};
//-------------------------------------------------------------

//H323_CS_SIG_CHANNEL_CLOSE_IND
//typedef struct  {
//	APIU32						channelType;					// audio || video || data || non standard ...
//	APIU32						channelIndex;
//	APIU32						channelDirection;				// 0 - In / 1 - Out
//
//} mcIndChannelClosed;

genXmlFormat mcIndChannelClosedFormat[] = {
	{parentElemType,	tagVarType,	"mcIndChannelClosed",	3,	0,0,0,0},
		{childElemType,		longVarType,	"channelType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelIndex",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelDirection",	0,	0,0,0,0},
};
//-------------------------------------------------------------

//H323_CS_SIG_CHAN_MAX_SKEW_IND
//typedef struct  {
//	APIU32						secondChannelIndex;
//	APIU32						skew;
//} mcIndChannelMaxSkew;

genXmlFormat mcIndChannelMaxSkewFormat[] = {
	{parentElemType,	tagVarType,	"mcIndChannelMaxSkew",	2,	0,0,0,0},
		{childElemType,		longVarType,	"secondChannelIndex",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"skew",					0,	0,0,0,0},
};
//-------------------------------------------------------------

//H323_CS_SIG_FLOW_CONTROL_IND_IND
//typedef struct  {
//	APIU32						channelType;					// audio || video || data || non standard ...
//	APIU32						channelIndex;
//	APIU32						channelDirection;				// 0 - In / 1 - Out	
//	APIU32						rate;
//} mcIndFlowControlIndication;

genXmlFormat mcIndFlowControlIndicationFormat[] = {
	{parentElemType,	tagVarType,	"mcIndFlowControlIndication",	4,	0,0,0,0},
		{childElemType,		longVarType,	"channelType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelIndex",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channelDirection",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"rate",				0,	0,0,0,0},
};
//-------------------------------------------------------------

// H232_CS_SIG_DBC2_COMMAND_CT_ON_IND
// H323_CS_SIG_DBC2_COMMAND_CT_OFF_IND
//typedef struct  {
//	APIU32						refreshRate;
//	APIU32						interLeave;
//	APIU32						mpiLimit;
//	APIU32						motionVector;
//	APIU32						noEncapsulation;
//	APIU32						overlap;
//
//} mcIndDBC2Command;

genXmlFormat mcIndDBC2CommandFormat[] = {
	{parentElemType,	tagVarType,	"mcIndDBC2Command",	6,	0,0,0,0},
		{childElemType,		longVarType,	"refreshRate",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"interLeave",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"mpiLimit",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"motionVector",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"noEncapsulation",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"overlap",			0,	0,0,0,0},
};
//-------------------------------------------------------------
// H323_CS_DTMF_INPUT_IND
//typedef struct {
//	APIS32	sourceType;
// 	APIU8	dtmfBuffer[DtmfBuffLen];	
//}mcIndDtmfBuff;
genXmlFormat mcIndDtmfBuffFormat[] = {
	{parentElemType,	tagVarType,	"mcIndDtmfBuff", 2,	0,0,0,0},
	{childElemType,		longVarType, "sourceType",	 0,	0,0,0,0},
	{siblingElemType,	charArrayVarType, "dtmfBuffer",	0, DtmfBuffLen,0,0,0},
};

// H323_CS_FACILITY_IND
//typedef struct {
//	h460AvayaFeVndrIndSt    		 avfFeVndIdInd;
//	h460AvayaFeMaxNonAudioBitRateInd avfFeMaxNonAudioBitRateInd;
//	
//} mcIndFacility;
genXmlFormat mcIndFacilityBuffFormat[] = {
		{parentElemType,	tagVarType,	"mcIndFacilityBuffFormat", 2,	0,0,0,0},
		{parentElemType,	structVarType,  "avfFeVndIdInd",	1, (int) &h460AvayaFeVndrIndStFormat,0,0,0},
		{parentElemType,	structVarType,  "avfFeMaxNonAudioBitRateInd", 1, (int) &h460AvayaMaxNonAudioBitRateIndStFormat,0,0,0},		
};

// H323_CS_SIG_LPR_MODE_CHANGE_IND
//typedef struct  {
//	APIU32					lossProtection;
//	APIU32					mtbf;
//	APIU32					congestionCeiling;
//	APIU32					fill;
//	APIU32					modeTimeout;
//
//} mcIndLPRModeChange;
genXmlFormat mcIndLPRModeChangeFormat[] = {
	{parentElemType,	tagVarType,	"mcIndLPRModeChange", 5, 0,0,0,0},
		{childElemType,		longVarType,  "lossProtection",		0, 0,0,0,0},
		{siblingElemType,	longVarType,  "mtbf", 				0, 0,0,0,0},		
		{siblingElemType,	longVarType,  "congestionCeiling", 	0, 0,0,0,0},		
		{siblingElemType,	longVarType,  "fill", 				0, 0,0,0,0},		
		{siblingElemType,	longVarType,  "modeTimeout", 		0, 0,0,0,0},		
};

//added by Jason for ITP-Multiple channels
// H323_CS_SIG_NEW_ITP_SPEAKER_IND
//typedef struct  {
//	APIU32					ITPType;
//	APIU32					numOfActiveLinks;
//
//} mcIndNewITPSpeaker;
genXmlFormat mcIndNewITPSpeakerFormat[] = {
	{parentElemType,	tagVarType,	"mcIndNewITPSpeaker", 2, 0,0,0,0},
		{childElemType,		longVarType,  "ITPType",		0, 0,0,0,0},
		{siblingElemType,	longVarType,  "numOfActiveLinks", 	0, 0,0,0,0},			
};

//Strat of sync
//-------------------------------------------------------------
//H323_CS_CONFERENCE_REQ_IND 
//typedef struct{
//	APIU32				ConReqOpcode;
//	APIU32				val1;
//	APIU32				val2;
//}mcIndConferenceReq;
genXmlFormat mcIndConferenceReqFormat[] = {
	{parentElemType,	tagVarType,	"mcIndConferenceReq",	3,	0,0,0,0},
		{childElemType,		longVarType,	"ConReqOpcode",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val1",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val2",		0,	0,0,0,0},

};

//-------------------------------------------------------------
//H323_CS_CONFERENCE_COM_IND
//typedef struct{
//	APIU32				ConComOpcode;	//from ConferenceCommandEnum
//	APIU32				val1;
//	APIU32				val2;
//	char				conID[16];
//}mcIndConferenceCom;
genXmlFormat mcIndConferenceComFormat[] = {
	{parentElemType,	tagVarType,	"mcIndConferenceCom",	4,	0,0,0,0},
		{childElemType,		longVarType,	"ConComOpcode",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val1",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val2",		0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"conID", 	0, 	16,0,0,0},

};

//-------------------------------------------------------------
//H323_CS_CONFERENCE_RES_IND
//typedef struct{
//	APIU32				ConResOpcode;	// from ConferenceResponseEnum
//	APIU32				val1;
//	APIU32				val2;
//	char				strID[128];
//}mcIndConferenceRes;
genXmlFormat mcIndConferenceResFormat[] = {
	{parentElemType,	tagVarType,	"mcIndConferenceRes",	4,	0,0,0,0},
		{childElemType,		longVarType,	"ConResOpcode",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val1",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val2",		0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"strID", 	0, 	128,0,0,0},

};

//-------------------------------------------------------------
//H323_CS_CONFERENCE_IND_IND
//typedef struct{
//	APIU32				ConIndOpcode;	//from ConferenceIndicationEnum
//	APIU32				val1;
//	APIU32				val2;
//}mcIndConferenceInd;
genXmlFormat mcIndConferenceIndFormat[] = {
	{parentElemType,	tagVarType,	"mcIndConferenceInd",	3,	0,0,0,0},
		{childElemType,		longVarType,	"ConIndOpcode",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val1",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val2",		0,	0,0,0,0},

};

//-------------------------------------------------------------
//typedef struct {
//	ctNonStandardParameterSt	nonStandardData;
//} mcIndNonStandard;
genXmlFormat mcIndNonStandardFormat[] = {
	{parentElemType,	tagVarType,	"mcIndNonStandard",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,		1,	(int) &ctNonStandardParameterStFormat,0,0,0},
		
};

	
// end of sync
// EOF

