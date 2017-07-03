//+========================================================================+
//                       H323CsInd.h	                                   |
//            Copyright 1995 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       H323CsInd.h	                                               |
// SUBSYSTEM:  CS/ConfParty                                                |
// PROGRAMMER: Guy D,													   |
// Date : 15/5/05														   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+                        
/*

This header file contains all mutual Indication structures related to 
ConfParty + CS entities within the Carmel.
In this document we set the common structures.

*/

#ifndef __H323CSIND_H__
#define __H323CSIND_H__


#include "ChannelParams.h"
#include "ConfMultimediaControlDefinitions.h"
#include "NonStandard.h"
#include "IpAddressDefinitions.h"
#include "IpEncryptionDefinitions.h" 
#include "H460_1.h"
#include "Capabilities.h"
#include "RvCommonDefs.h"


//H323_CS_SIG_GET_PORT_IND
typedef struct {
	mcXmlTransportAddress			srcCallSignalAddress;	//The new RV stack struct(srcCallSignalAddress)
							  
} mcIndGetPort;


//H323_CS_SIG_CALL_DIAL_TONE_IND		// 323 Setup IND from RV stack (Inner ind within SC) - Mcms not involved
//H323_CS_SIG_CALL_PROCEEDING_IND		// The first answer the EP gives to Setup_req is this - Meaning got your call and I'm handling - Mcms not involved
//H323_CS_SIG_CALL_RING_BACK_IND		// EP sent alerting (there is a ringback tone) - Mcms not involved.
typedef struct  {
	mcXmlTransportAddress			h245IpAddress;	//The new RV stack struct

} mcIndCallReport;
//Comments: Mcms does not get these IND


//H323_CS_SIG_CALL_OFFERING_IND
typedef struct  {
	partyAddressList			srcPartyAliases;			// src Aliases list.(content changed for Ipv6).
	partyAddressList			destPartyAliases;			// dest Aliases list.(content changed for Ipv6).
	mcXmlTransportAddress		srcIpAddress;				// Changed for Ipv6.
	mcXmlTransportAddress		destIpAddress;				// Changed for Ipv6.
	
	APIU32						rate;						// Party rate
	APIS8						sDisplay[MaxDisplaySize];
	APIS8						userUser[MaxUserUserSize];	
	APIS32						userUserSize;

	APIS8						conferenceId[MaxConferenceIdSize];	
	APIS8						callId[Size16];					
	APIU32						conferenceGoal; 	// create || envite || join		
	APIS32						referenceValue; 			
	APIU32						type;				// P2P || One2N || N2N || N2One						
	APIS32						bIsActiveMc;		// Active multipoint controller
	APIS32						bIsOrigin; 			// Always false		
	mcXmlTransportAddress		h245IpAddress;		//The new RV stack struct				
				
	APIS32						bH245Establish;		// H245 connection ? yes/no
	APIU32						srcEndpointType;			
	APIS32						localH225Port;
	APIS32						indexTblAuth;		// ?? Avaya ?
	
	encTokensHeaderBasicStruct	encryTokens;		// must be last
	
} mcIndCallOfferingBase;

typedef struct  {
	partyAddressList			srcPartyAliases;			// src Aliases list.(content changed for Ipv6).
	partyAddressList			destPartyAliases;			// dest Aliases list.(content changed for Ipv6).
	mcXmlTransportAddress		srcIpAddress;				// Changed for Ipv6.
	mcXmlTransportAddress		destIpAddress;				// Changed for Ipv6.
	
	APIU32						rate;						// Party rate
	APIS8						sDisplay[MaxDisplaySize];
	APIS8						userUser[MaxUserUserSize];	
	APIS32						userUserSize;

	APIS8						conferenceId[MaxConferenceIdSize];				
	APIS8						callId[Size16];				
	APIU32						conferenceGoal; 	// create || envite || join		
	APIS32						referenceValue; 			
	APIU32						type;				// P2P || One2N || N2N || N2One						
	APIS32						bIsActiveMc;		// Active multipoint controller
	APIS32						bIsOrigin; 			// Always false		
	mcXmlTransportAddress		h245IpAddress;		//The new RV stack struct				
	APIS32						bH245Establish;		// H245 connection ? yes/no
	APIU32						srcEndpointType;			
	APIS32						localH225Port;
	APIS32						indexTblAuth;	// ?? Avaya ?

	encTokensHeaderStruct		encryTokens;	// must be last.

} mcIndCallOffering;


//H323_CS_SIG_CALL_CONNECTED_IND
typedef struct  {
	mcXmlTransportAddress		h225remote;	//The new RV stack struct (Remote)
	mcXmlTransportAddress		h225local;	//The new RV stack struct (Local)
	mcXmlTransportAddress		h245remote;	//The new RV stack struct
	mcXmlTransportAddress		h245local;	//The new RV stack struct
	APIS8						sDisplay[MaxDisplaySize];
	APIS8						userUser[MaxUserUserSize];
	APIS32						userUserSize;
	APIU32						remoteEndpointType;		
	APIS32						h225RemoteVersion;
	APIU32						endPointNetwork;
	APIS32						bAuthenticated;
	h460AvayaFeVndrIndSt	    avfFeVndIdInd;
	cmVendor					remoteVendor;
	
	encTokensHeaderBasicStruct	encryTokens;	// must be last.

} mcIndCallConnectedBase;

typedef struct  {
	mcXmlTransportAddress		h225remote;	//The new RV stack struct (Remote)
	mcXmlTransportAddress		h225local;	//The new RV stack struct (Local)
	mcXmlTransportAddress		h245remote;	//The new RV stack struct
	mcXmlTransportAddress		h245local;	//The new RV stack struct
	APIS8						sDisplay[MaxDisplaySize];
	APIS8						userUser[MaxUserUserSize];
	APIS32						userUserSize;
	APIU32						remoteEndpointType;		
	APIS32						h225RemoteVersion;
	APIU32						endPointNetwork;
	APIS32						bAuthenticated;
	h460AvayaFeVndrIndSt	    avfFeVndIdInd;
	cmVendor					remoteVendor;
	
	encTokensHeaderStruct		encryTokens;	// must be last.

} mcIndCallConnected;


//H323_CS_SIG_CALL_NEW_RATE_IND
typedef struct  {
	APIU32						rate;
} mcIndCallNewRate;


//H323_CS_SIG_BASE_CAPABILITIES_IND
typedef struct  {
	APIS32						capabilitiesSize;
} mcIndCapabilitiesBase;

//H323_CS_SIG_CAPABILITIES_IND
typedef struct  {
	APIS32						capabilitiesSize;
	ctCapabilitiesStruct		capabilities;
} mcIndCapabilities;


//H323_CS_SIG_CAP_RESPONSE_IND;(Empty)
//typedef struct  {
//} mcIndCapResponse;


//H323_CS_SIG_CALL_CNTL_CONNECTED_IND
typedef struct  {
	mcXmlTransportAddress		remoteH245Address;	//The new RV stack struct
	APIU32						masterSlaveStatus;
	APIS8						userUser[MaxUserUserSize];	
	APIS32						userUserSize;
	mcXmlTransportAddress		localH245Address;	//(Local)//The new RV stack struct
} mcIndCallControlConnected;

//H323_CS_SIG_CALL_BAD_SPONTAN_IND
typedef struct {
  BadSpontanIndReason			status; //the value set according to BadSpontanIndReason enum

} mcIndBadSpontan;


//H323_CS_SIG_CALL_ROLE_TOKEN_IND
typedef struct  {
	
	APIU32						subOpcode; // ERoleTokenOpcode
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

} mcIndRoleToken;


//H323_CS_SIG_CALL_AUTHENTICATION_IND
typedef struct {
	APIS32						bAuthenticated;

} mcIndAuthentication;

//H323_CS_SIG_INCOMING_CHANNEL_IND
typedef struct  {

	APIU32						payloadType;
	APIU32						dataType;				// audio || video || data || non standard ...
	APIU32						channelIndex;
	APIU32						channelDirection;				// 0 - In / 1 - Out
	mcXmlTransportAddress		rmtRtpAddress; // New field ,provide here remote rtp Address/ rtcp is odd port incremental
	APIS8						channelName[ChannelNameSize];	
	APIU8						capTypeCode;
	
	APIU8						filler[3];

	APIU32						rate;	
	APIS32						bIsActive;						// Is this an active channel
	APIU32						sameSessionChannelIndex;
	APIS32						sessionId;

	APIS8						dynamicPayloadType;
	
	APIU8						bIsEncrypted;
	APIU8						encryptionAlgorithm;			// EenMediaType
	APIU8						EncryptedSession235Key[sizeOf128Key];
	APIU8						bUsedH263Plus;
	APIS32						bIsLPR;

	APIU16						sizeOfChannelParams;
	APIU16						fille2;
	xmlDynamicProperties		xmlDynamicProps;
} mcIndIncomingChannelBase;

typedef struct  {

	APIU32						payloadType;
	APIU32						dataType;				// audio || video || data || non standard ...
	APIU32						channelIndex;
	APIU32						channelDirection;				// 0 - In / 1 - Out
	mcXmlTransportAddress		rmtRtpAddress; // New field ,provide here remote rtp Address/ rtcp is odd port incremental
	APIS8						channelName[ChannelNameSize];	
	APIU8						capTypeCode;
	APIU8						filler[3];

	APIU32						rate;
	
	APIS32						bIsActive;						// Is this an active channel
	APIU32						sameSessionChannelIndex;
	APIS32						sessionId;

	APIS8						dynamicPayloadType;
	
	APIU8						bIsEncrypted;
	APIU8						encryptionAlgorithm;			// EenMediaType
	APIU8						EncryptedSession235Key[sizeOf128Key];
	APIU8						bUsedH263Plus;
	APIS32						bIsLPR;

	APIU16						sizeOfChannelParams;
	APIU16						fille2;
	xmlDynamicProperties		xmlDynamicProps;
	APIS8						channelSpecificParams[1];

	
} mcIndIncomingChannel;


//H323_CS_SIG_INCOMING_CHNL_CONNECTED_IND
typedef struct  {

	APIU32						channelType;					// audio || video || data || non standard ...
	APIU32						channelIndex;
	APIU32						channelDirection;				// 0 - In / 1 - Out
	APIS32						sessionId;						// Session Id of channel

	
	APIU32						sameSessionChannelIndex;		// Remove ? (From MCMS + RTP related)
	APIU32						associatedChannelIndex;			// Remove ? (From MCMS + RTP related)

} mcIndIncomingChannelConnected;
//Comments: This is the Ack saying that the channel is connected EP wise.


//H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND
typedef struct  {
	APIU32						channelType;					// audio || video || data || non standard ...
	APIU32						channelIndex;
	APIU32						channelDirection;				// 0 - In / 1 - Out
	APIS32						sessionId;						// Session Id of channel

	APIU32						payloadType;				// Confirmation from the EP.
	APIU32						sameSessionChannelIndex;	// ?? - RTP associated	
	APIU32						associatedChannelIndex;		// ?? - RTP associated
	mcXmlTransportAddress		destRtpAddress;			// Added provided on OLCA 

	APIS8						dynamicPayloadType;
	APIU8						filler[1];
	APIU8						bIsEncrypted;					// Keep 
	APIU8						encryptionAlgorithm;			// EenMediaType - Keep
	APIU8						EncryptedSession235Key[sizeOf128Key];		// Keep 
	

} mcIndOutgoingChannelResponse; 


//H323_CS_SIG_CHAN_NEW_RATE_IND
typedef struct  {
	APIU32						channelType;					// audio || video || data || non standard ...
	APIU32						channelIndex;
	APIU32						channelDirection;				// 0 - In / 1 - Out

	APIU32						rate;
} mcIndChannelNewRate;



//H323_CS_SIG_CALL_IDLE_IND(Empty)
//typedef struct  {
//} mcIndCallIdle;
//Comments: All is closed with remote.


//H323_CS_SIG_START_CHANNEL_CLOSE_IND
typedef struct  {
	APIU32						channelType;					// audio || video || data || non standard ...
	APIU32						channelIndex;
	APIU32						channelDirection;				// 0 - In / 1 - Out

} mcIndStartChannelClose; 
//Comments: EP asked to close channel - Need to inform RTP (Drop_Chnl_req).


//H323_CS_SIG_CHANNEL_CLOSE_IND
typedef struct  {
	APIU32						channelType;					// audio || video || data || non standard ...
	APIU32						channelIndex;
	APIU32						channelDirection;				// 0 - In / 1 - Out

} mcIndChannelClosed;

//H323_CS_SIG_CHAN_MAX_SKEW_IND
typedef struct  {
	APIU32						secondChannelIndex;
	APIU32						skew;
} mcIndChannelMaxSkew;


//H323_CS_SIG_CHANNEL_OFF_IND
//typedef struct  {
//
//} mcIndChannelOff;

//H323_CS_SIG_CHANNEL_ON_IND
//typedef struct  {
//	
//} mcIndChannelOn;


//H323_CS_SIG_FLOW_CONTROL_IND_IND
typedef struct  {
	APIU32						channelType;					// audio || video || data || non standard ...
	APIU32						channelIndex;
	APIU32						channelDirection;				// 0 - In / 1 - Out
	
	APIU32						rate;
} mcIndFlowControlIndication;


// H232_CS_SIG_DBC2_COMMAND_CT_ON_IND
// H323_CS_SIG_DBC2_COMMAND_CT_OFF_IND

typedef struct  {
	APIU32						refreshRate;
	APIU32						interLeave;
	APIU32						mpiLimit;
	APIU32						motionVector;
	APIU32						noEncapsulation;
	APIU32						overlap;

} mcIndDBC2Command;
//Comments: This is for Indications coming from the EP.

//IP_CS_VIDEO_UPDATE_PIC_IND
//typedef struct  {
//}mcVideoUpdatePicture;

//H323_CS_CONFERENCE_REQ_IND 
typedef struct{
	APIU32				ConReqOpcode;
	APIU32				val1;
	APIU32				val2;
}mcIndConferenceReq;

//H323_CS_CONFERENCE_COM_IND
typedef struct{
	APIU32				ConComOpcode;	//from ConferenceCommandEnum
	APIU32				val1;
	APIU32				val2;
	char				conID[16];
}mcIndConferenceCom;

//H323_CS_CONFERENCE_RES_IND
typedef struct{
	APIU32				ConResOpcode;	// from ConferenceResponseEnum
	APIU32				val1;
	APIU32				val2;
	char				strID[128];
}mcIndConferenceRes;

//H323_CS_CONFERENCE_IND_IND
typedef struct{
	APIU32				ConIndOpcode;	//from ConferenceIndicationEnum
	APIU32				val1;
	APIU32				val2;
}mcIndConferenceInd;

// H323_CS_NON_STANDARD_REQ_IND
// H323_CS_NON_STANDARD_COM_IND
// H323_CS_NON_STANDARD_RES_IND
// H323_CS_NON_STANDARD_IND_IND

typedef struct {
	ctNonStandardParameterSt	nonStandardData;
} mcIndNonStandard;

// H323_CS_DTMF_INPUT_IND
typedef struct {
	APIS32	sourceType;
 	APIU8	dtmfBuffer[DtmfBuffLen];	
}mcIndDtmfBuff;

// H323_CS_PARTY_KEEP_ALIVE_IND
//typedef struct {
//}mcIndPartyKeepAlive;

// H323_CS_FACILITY_IND
typedef struct {
	h460AvayaFeVndrIndSt    		 avfFeVndIdInd;
	h460AvayaFeMaxNonAudioBitRateInd avfFeMaxNonAudioBitRateInd;
	
} mcIndFacility;

// H323_CS_SIG_LPR_MODE_CHANGE_IND
typedef struct  {
	APIU32					lossProtection;
	APIU32					mtbf;
	APIU32					congestionCeiling;
	APIU32					fill;
	APIU32					modeTimeout;

} mcIndLPRModeChange;

//H323_CS_SIG_LPR_MODE_CHANGE_RES_IND(Empty)
//typedef struct {
//} mcIndLPRModeChangeRes;

//added by Jason for ITP-Multiple channels
// H323_CS_SIG_NEW_ITP_SPEAKER_IND
typedef struct  {
	APIU32					ITPType;
	APIU32					numOfActiveLinks;

} mcIndNewITPSpeaker;
//----------------------------------------------------------------------
typedef union  
{   
	mcIndGetPort								uMcIndGetPort;
	mcIndCallReport								uMcIndCallReport;
	mcIndCallOffering							uMcIndCallOffering;
	mcIndCallConnected							uMcIndCallConnected;
	mcIndCallNewRate							uMcIndCallNewRate;
	mcIndCapabilities							uMcIndCapabilities;
	/*tmDsIndCapResponse						uTmDsIndCapResponse;*/
	mcIndCallControlConnected					uMcIndCallControlConnected;
	mcIndIncomingChannel						uMcIndIncomingChannel;
	mcIndIncomingChannelConnected				uMcIndIncomingChannelConnected;
	mcIndOutgoingChannelResponse				uMcIndOutgoingChannelResponse;
	mcIndChannelNewRate							uMcIndChannelNewRate;
	/*tmDsIndCallIdle							uTmDsIndCallIdle;*/
	mcIndStartChannelClose						uMcIndStartChannelClose;
	mcIndChannelClosed							uMcIndChannelClosed;
	mcIndFlowControlIndication					uMcIndFlowControlIndication;
	mcIndDBC2Command							uMcIndDBC2Command;
	mcIndConferenceReq							uMcIndConferenceReq;
	mcIndConferenceCom							uMcIndConferenceCom;
	mcIndConferenceRes							uMcIndConferenceRes;
	mcIndConferenceInd							uMcIndConferenceInd;
	mcIndNonStandard							uMcIndNonStandard;
//	mcIndPartyKeepAlive							uMcIndPartyKeepAlive;
	
}	H323_CS_IND_UNION;


#endif

