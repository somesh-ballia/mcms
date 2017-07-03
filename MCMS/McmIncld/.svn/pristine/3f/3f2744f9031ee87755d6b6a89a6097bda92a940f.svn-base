//+========================================================================+
//                       H323CsReq.h	                                   |
//            Copyright 1995 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       H323CsReq.h	                                               |
// SUBSYSTEM:  CS/ConfParty                                                |
// PROGRAMMER: Guy D,													   |
// Date : 15/5/05														   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+                        
/*

This header file contains all mutual request structures related to 
ConfParty + CS entities within the Carmel.
In this document we set the common structures.

*/

#ifndef __H323CSREQ_H__
#define __H323CSREQ_H__


#include "H460_1.h"
#include "ChannelParams.h"
#include "IpEncryptionDefinitions.h"
#include "GateKeeperCommonParams.h"


//H323_CS_SIG_GET_PORT_REQ
typedef struct {
	APIU32 srcIpAddress;//replace the haCall from the common headar
} mcReqGetPort;
//Comments: The allocation will remain in CS.The port is a number which will be the same for IpV6/4


//H323_CS_SIG_RELEASE_PORT_REQ 
typedef struct  {
	APIU16							port;
	APIU16							filler;
	void							*haCall;

} mcReqReleasePort;
//Comments: The port is a number which will be the same for IpV6/4.

typedef struct {
	APIS8						srcPartyAliases[MaxAddressListSize];	// src Aliases list.(content changed for Ipv6).
	APIS8						destPartyAliases[MaxAddressListSize];	// dest Aliases list.(content changed for Ipv6).	
	mcXmlTransportAddress		srcIpAddress;							// Changed for Ipv6.
	mcXmlTransportAddress		destIpAddress;							// Changed for Ipv6.
	APIS8						conferenceId[MaxConferenceIdSize];		// Conference ID for the card 
	APIS8						callId[Size16];							// Unique parameter per H225 call 	
	APIS32						referenceValue; 						// Unique parameter per Q931 call
	APIU32						maxRate;			// Max conference rate (In conf. parameters).
	APIU32						minRate;			// Min conf. rate (=0).
	mcCallTransient				callTransient;		// display field values
	APIU32						localEndpointType;  // MCU || GW || Terminal		
	APIU32						type;				// P2P || One2N || N2N || N2One // Always N2N ?
	APIU32						conferenceType;		// SWCP | VSW | CP | COP | H239 |P&C version | FECC
	APIS32						bIsActiveMc;		// According to H225 Std. - Mandatory
	APIU32						callGoal;			// cmCreate, cmJoin, cmInvite.
	h460AvayaFeVndrReqSt        avfFeVndIdReq;		// Extension of H.225...
	void						*haCall;
	// -- canMapAlias -- // 
	APIU32						destExtraCallInfoTypes[MaxNumberOfAliases]; // ??? 
	APIS8						destExtraCallInfo[MaxAddressListSize];	//canMapAlias
	APIS8						remoteExtensionAddress[MaxAliasLength];		//canMapAlias
	// -----------------
	APIS32						bAuthentication;
	APIU8						sid[Pf1SessionIdSize];	// Session Id for authentication of Avaya		
	APIU8						filler[1];	
	APIU8						authKey[sizeOf128Key];	//len = 128 // Encryption For Control channel?
	
	APIU32						bIsCascade;					
	APIU32						epIdentLength;			//New fields for encryption
	APIS8						endpointIdent[MaxIdentifierSize];

	encTokensHeaderBasicStruct		encryTokens;	// must be last. // SC uses these params for H245 security signaling

} mcReqCallSetupBase;


//H323_CS_SIG_CALL_SETUP_REQ
typedef struct {
	APIS8						srcPartyAliases[MaxAddressListSize];	// src Aliases list.(content changed for Ipv6).
	APIS8						destPartyAliases[MaxAddressListSize];	// dest Aliases list.(content changed for Ipv6).	
	mcXmlTransportAddress		srcIpAddress;							// Changed for Ipv6.
	mcXmlTransportAddress		destIpAddress;							// Changed for Ipv6.
	APIS8						conferenceId[MaxConferenceIdSize];		// Conference ID for the card 
	APIS8						callId[Size16];							// Unique parameter per H225 call 	
	APIS32						referenceValue; 						// Unique parameter per Q931 call
	APIU32						maxRate;			// Max conference rate (In conf. parameters).
	APIU32						minRate;			// Min conf. rate (=0).
	mcCallTransient				callTransient;		// display field values
	APIU32						localEndpointType;  // MCU || GW || Terminal		
	APIU32						type;				// P2P || One2N || N2N || N2One // Always N2N ?
	APIU32						conferenceType;		// SWCP | VSW | CP | COP | H239 |P&C version | FECC
	APIS32						bIsActiveMc;		// According to H225 Std. - Mandatory
	
	APIS32						callGoal;			// cmCreate, cmJoin, cmInvite.
	h460AvayaFeVndrReqSt        avfFeVndIdReq;		// Extension of H.225...
	void						*haCall;
	// -- canMapAlias -- // 
	APIU32						destExtraCallInfoTypes[MaxNumberOfAliases]; // ??? 
	APIS8						destExtraCallInfo[MaxAddressListSize];	//size = 100 canMapAlias
	APIS8						remoteExtensionAddress[MaxAliasLength];		//size = 128 canMapAlias
	// -----------------
	APIS32						bAuthentication;
	APIU8						sid[Pf1SessionIdSize];	// len = 11 Session Id for authentication of Avaya		
	APIU8						filler[1];	
	APIU8						authKey[sizeOf128Key];	//len = 128 // Encryption For Control channel?
	
	APIU32						bIsCascade;	
	APIU32						epIdentLength;			//New fields for encryption
	APIS8						endpointIdent[MaxIdentifierSize];
	//-------------------
	APIU32						localTerminalType; // used for MSD 

	mcCallGeneratorParams		callGeneratorParams;	// Call Generator SoftMCU CS parameters

	encTokensHeaderStruct		encryTokens;	// must be last. // SC uses these params for H245 security signaling

} mcReqCallSetup;
/*Comments: each type of parameter need to have a setting function and not in the general function, like conferenceType. 
IpV6 will pass to CS the same way (In the string), the only difference will be an additional identifier in the begining of the string.
Encryption must stay - CS will use these params for H225 security signaling.*/
 

//H323_CS_SIG_CALL_ANSWER_REQ
typedef struct {
	mcXmlTransportAddress		h245Address;		// The new RV stack struct
	mcXmlTransportAddress		remoteAddress; 		// Changed for Ipv6. forward address.
	APIS8						conferenceId[MaxConferenceIdSize];	// Conference ID for the card 
	mcCallTransient				callTransient;		// display field values
	APIU32						localEndpointType;	// MCU || GW || Terminal	
	APIU32						maxRate;			// Max conference rate (In conf. parameters).
	APIU32						rejectCallReason;	// cmReasonType
	APIU32						conferenceType;		// SWCP | VSW | CP | COP | H239 |P&C version | DUO | FECC | T120
	
	APIS32						bAuthentication;	// Avaya....
	APIU8						sid[Pf1SessionIdSize];	// Session Id for authentication of Avaya	
	APIU8						filler[1];	
	APIU8						authKey[sizeOf128Key];	//len = 128 // Encryption For Control channel?
	void						*haCall;
	APIU32						bIsCascade;
	APIU32						epIdentLength;			//New fiels for encryption
	APIS8						endpointIdent[MaxIdentifierSize];
	APIS8						ConnectedAddressAliases[MaxAddressListSize];
	encTokensHeaderBasicStruct	encryTokens;		// must be last.
} mcReqCallAnswerBase;


typedef struct {
	mcXmlTransportAddress		h245Address;		// The new RV stack struct.
	mcXmlTransportAddress		remoteAddress; 		// Changed for Ipv6. forward address.
	APIS8						conferenceId[MaxConferenceIdSize];	// Conference ID for the card 
	mcCallTransient				callTransient;		// display field values
	APIU32						localEndpointType;	// MCU || GW || Terminal	
	APIU32						maxRate;			// Max conference rate (In conf. parameters).
	APIU32						rejectCallReason;	// cmReasonType
	APIU32						conferenceType;		// SWCP | VSW | CP | COP | H239 |P&C version | DUO | FECC | T120
	
	APIS32						bAuthentication;	// Avaya....
	APIU8						sid[Pf1SessionIdSize];	// Session Id for authentication of Avaya	
	APIU8						filler[1];	
	APIU8						authKey[sizeOf128Key];	//len = 128 // Encryption For Control channel?
	void						*haCall;
	APIU32						bIsCascade;
	APIU32						epIdentLength;			//New fiels for encryption
	APIS8						endpointIdent[MaxIdentifierSize];
	APIS8						ConnectedAddressAliases[MaxAddressListSize];
	APIU32						localTerminalType; // used for MSD
	mcCallGeneratorParams		callGeneratorParams;	// Call Generator SoftMCU CS parameters
	encTokensHeaderStruct		encryTokens;		// must be last.
} mcReqCallAnswer;
/*Comments: each type of parameter need to have a setting function and not in the general function, like conferenceType. 
IpV6 will pass to CS the same way (In the string), the only difference will be an additional identifier in the begining of the string.
Encryption must stay - CS will use these params for H225 security signaling.*/


// H323_CS_SIG_CREATE_CNTL_REQ
// H323_CS_SIG_RE_CAPABILITIES_REQ
typedef struct {
	mcXmlTransportAddress		h245IpAddress;		//The new RV stack struct
	APIU32						masterSlaveTerminalType;
	APIU32						capabilitiesStructLength;
} mcReqCreateControlBase;


typedef struct {
	mcXmlTransportAddress		h245IpAddress;		//The new RV stack struct
	APIS32						masterSlaveTerminalType;
	APIS32						capabilitiesStructLength;
	ctCapabilitiesStruct		capabilities;
} mcReqCreateControl;


// H323_CS_SIG_INCOMING_CHNL_RESPONSE_REQ
typedef struct {

	APIU32						channelIndex;
	APIU32						dataType;	  // audio || video || data || non standard ...
	APIU32						channelDirection; // 0 - In / 1 - Out
	mcXmlTransportAddress		localRtpAddressIp;			// UDP IP address to receive the incoming channel stream 
	APIU8						dynamicPayloadType;				// Need to keep
	APIU8						bIsEncrypted;					// Keep 
	APIU8						encryptionAlgorithm;			// EenMediaType - Keep
	APIS8						filler[1];						// Do we still need to keep after the DBC2 ? 
	APIU8						EncryptedSession235Key[sizeOf128Key];		// Keep 
} mcReqIncomingChannelResponse;
//Comments: DynamicPayloadType logic will move to MCMS Party (2 stages: 1. Identification of the caps. 2. Giving dynamic number.).  


// H323_CS_SIG_OUTGOING_CHNL_REQ
typedef struct {
	
	mcXmlTransportAddress		SourceRtpAddressIp;				// UDP IP address to transmit the outgoing channel stream 

	APIU32						payloadType;					// _PCMU || _G722 || _Siren14 || ...
	APIU32						channelType;					// audio || video || data || non standard ...
	APIU32						channelIndex;
	APIU32						channelDirection;				// 0 - In / 1 - Out

	APIS8						channelName[ChannelNameSize];	// Keep
	APIU8						capTypeCode;					// g711Alaw64kCapCode || h263CapCode || ...
	APIU8						filler1[3];
	
	APIU8						dynamicPayloadType;				// Keep
	APIU8						bIsEncrypted;					// Keep
	APIU8						encryptionAlgorithm;				// Keep
	APIS8						filler4[1];						
	APIU8						EncryptedSession235Key[sizeOf128Key];		// Keep
	APIS32						bIsLPR;							// Indication LPR is supported on the Channel.

	APIU16						sizeOfChannelParams;			// Need to remain
	APIS8						filler5[2];
	xmlDynamicProperties		xmlDynamicProps;
	APIS8						channelSpecificParams[1];		// The channel (algorithm) parameters

} mcReqOutgoingChannel;


// H323_CS_SIG_CHAN_NEW_RATE_REQ
typedef struct  {
	APIU32						channelType;					// audio || video || data || non standard ...
	APIU32						channelIndex;
	APIU32						channelDirection;				// 0 - In / 1 - Out
	APIU32						rate;
} mcReqChannelNewRate;



// H323_CS_SIG_CHAN_MAX_SKEW_REQ
typedef struct  {
	APIU32						secondChannelIndex;
	APIU32						channelType;					// audio || video || data || non standard ...
	APIU32						channelIndex;
	APIU32						channelDirection;				// 0 - In / 1 - Out
	APIU32						skew;
} mcReqChannelMaxSkew;



//H323_CS_SIG_VIDEO_UPDATE_PIC_REQ (Empty )
//typedef struct  {
//} mcReqVideoUpdatePicture;
//Comments: This is a request for the EP regarding Intra demand.
// In case RTP will detect a sync loss - It will inform the MCMS and MCMS will forward this REQ to EP. 


//H323_CS_SIG_CHANNEL_OFF_REQ	  
typedef struct  {
	APIU32						channelType;					// audio || video || data || non standard ...

} mcReqChannelOff;
//Comments: Channel INactive (Tell Ep to mute the channel)


//H323_CS_SIG_MULTIPOINTMODECOM_TERMINALID_REQ
typedef struct  {
	APIU32						mcuID;
	APIU32						terminalID;	
}mcReqMultipointModeComTerminalIDMessage;


//Comments:Non standard indication message, Interval request that the content is ON.

//H323_CS_SIG_CHANNEL_ON_REQ	  
typedef struct  {
	APIU32						channelType;					// audio || video || data || non standard ...

} mcReqChannelOn;
//Comments: Channel Active (Tell Ep to unmute the channel)


// H323_CS_SIG_ROLE_TOKEN_REQ
typedef struct  {
	APIU32						subOpcode;	// ERoleTokenOpcode
	APIU32						mcuID;
	APIU32						terminalID;
	APIU32						randNumber;
	//only for EPC:
	APIU32                      contentProviderInfo;
	APIU32                      label;
	//only for H239
	APIU16				    	bitRate;
	APIU8					    bIsAck;
	APIU8						filler;
}mcReqRoleTokenMessage;



//H323_CS_SIG_UNEXPECTED_MESSAGE_BASE_REQ
typedef struct {
	APIU32						badOpcode;
	APIU32						sizeMessage;
} mcReqUnexpectedMessageBase;

//H323_CS_SIG_UNEXPECTED_MESSAGE_REQ
typedef struct {
	APIU32						badOpcode;
	APIU32						sizeMessage;
	char						message[1];
} mcReqUnexpectedMessage;


//H323_CS_SIG_CALL_DROP_TIMER_EXPIRED_REQ
typedef struct {
	APIU32						rejectCallReason;
	ctTimerExpiredReason		reason;
} mcReqCallDropTimerExpired;


// disconnect messages
// H323_CS_SIG_CHNL_DROP_REQ
typedef struct {
	APIU32						channelType;					// audio || video || data || non standard ...
	APIU32						channelIndex;
	APIU32						channelDirection;				// 0 - In / 1 - Out
	
} mcReqChannelDrop;
/*Comments: Remains the same ? - Is going only towards the EP or for the RTP too?
** Now in case the channel is in IDLE (Already closed) SC sends closeChannel to the RTP
** Means - In Carmel, We need to send to the RTP close channel + streamOff (?) parallely.
** In case we got H323_CHANNEL_CLOSE_IND (Meaning the EP closed its channel) need to send close channel + streamOff (?) 
** to the RTP + this opcode to SC for Ack.*/ 



//H323_CS_SIG_CALL_DROP_REQ
typedef struct {
	APIU32						rejectCallReason;	// cmReasonType
} mcReqCallDrop;
//Comments: Need to send same opCode to RTP in order to make it close+release all its channels.



//H323_CS_SIG_CALL_CLOSE_CONFIRM_REQ(Empty)
//typedef struct {
//} mcReqCallCloseConfirm;



// H323_CS_SIG_DBC2_COMMAND_CT_ON_REQ
// H323_CS_SIG_DBC2_COMMAND_CT_OFF_REQ

typedef struct  {
	APIU32					refreshRate;
	APIU32					interLeave;
	APIU32					mpiLimit;
	APIU32					motionVector;
	APIU32					noEncapsulation;
	APIU32					overlap;

} mcReqDBC2Command;
//Comments: Requests to the EP.

//H323_CS_SIG_CT_AUTHENTICATION_REQ
typedef struct 
{
	int						indexTblAuth;
	APIU8					authKey[sizeOf128Key];	//len = 128
} mcReqAuthentication;

// IP_CS_SIG_STOP_ALL_PROCESSES_REQ
typedef struct  {
	APIS32					stopAllProcessorsReason;
} mcReqStopAllProcesses;

//H323_CS_SIG_CONFERENCE_RES_REQ
typedef struct{
	APIU32				ConResOpcode;	// from ConferenceResponseEnum
	APIU32				val1;
	APIU32				val2;
	char				strID[128];
}mcReqConferenceRes;

//H323_CS_SIG_CONFERENCE_IND_REQ
typedef struct{
	APIU32				ConIndOpcode;	//from ConferenceIndicationEnum
	APIU32				val1;
	APIU32				val2;
}mcReqConferenceInd;

// H323_CS_DTMF_INPUT_REQ
typedef struct {
	APIS32	sourceType;
 	APIU8	dtmfBuffer[DtmfBuffLen];	
}mcReqDtmfBuff;

//H323_CS_PARTY_KEEP_ALIVE_REQ(Empty)
//typedef struct {
//} mcReqPartyKeepAlive;

//H323_CS_SIG_CAPABILITIES_RES_REQ(Empty)
//typedef struct {
//} mcReqCapbabilitiesRes;

// H323_CS_FACILITY_REQ
typedef struct {
	UINT32	 	avfStandardId;
	char 		e164ConferenceId[MaxAliasLength];
}mcReqFacility;


// H323_CS_SIG_LPR_MODE_CHANGE_REQ
typedef struct  {
	APIU32					lossProtection;
	APIU32					mtbf;
	APIU32					congestionCeiling;
	APIU32					fill;
	APIU32					modeTimeout;

} mcReqLPRModeChange;

typedef struct 
{
    APIU32  length;
    char    paramBuffer[1];
} CS_RssCommon_Param_Req_S;


// H323_CS_SIG_RSS_CMD_REQ
typedef struct  {
	APIU32						subOpcode; // eRSSCommandRequestEnum
	CS_RssCommon_Param_Req_S 	data;
} mcReqRssCommand;

//H323_CS_SIG_LPR_MODE_CHANGE_RES_REQ(Empty)
//typedef struct {
//} mcReqLPRModeChangeRes;

//added by Jason for ITP-Multiple channels
// H323_CS_SIG_NEW_ITP_SPEAKER_REQ
typedef struct  {
	APIU32					ITPType;
	APIU32					numOfActiveLinks;

} mcReqNewITPSpeaker;
/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
typedef union  
{   
	mcReqGetPort								uMcReqGetPort;
	mcReqReleasePort							uMcReqReleasePort;
	mcReqCallSetup								uMcReqCallSetup;
	mcReqCallAnswer								uMcReqCallAnswer;
	mcReqCreateControl							uMcReqCreateControl;
	mcReqIncomingChannelResponse				uMcReqIncomingChannelResponse;
	mcReqOutgoingChannel						uMcReqOutgoingChannel;
	mcReqChannelNewRate							uMcReqChannelNewRate;
	/*mcReqVideoUpdatePicture					uMcReqVideoUpdatePicture;*/
	mcReqChannelOff								uMcReqChannelOff;
	mcReqMultipointModeComTerminalIDMessage		uMcReqMultipointModeComTerminalIDMessage;
	mcReqRoleTokenMessage						uMcReqRoleTokenMessage;
	mcReqUnexpectedMessageBase					uMcReqUnexpectedMessageBase;
	mcReqCallDropTimerExpired					uMcReqCallDropTimerExpired;
	mcReqChannelDrop							uMcReqChannelDrop;
	mcReqCallDrop								uMcReqCallDrop;
	/*mcReqCallCloseConfirm						uMcReqCallCloseConfirm;*/
	mcReqDBC2Command							uMcReqDBC2Command;
	mcReqStopAllProcesses						uMcReqStopAllProcesses;
	mcReqRssCommand								uMcReqRssCommand;
	
}	H323_CS_REQ_UNION;

#endif
