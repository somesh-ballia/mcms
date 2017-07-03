

#ifndef __MRC_STRUCTS__
#define __MRC_STRUCTS__

#include "DataTypes.h"
#include "IpAddressDefinitions.h"
#include "IpQosDefinitions.h"
#include "PhysicalResource.h"
#include "OpcodesMcmsCardMngrIpMedia.h" // SHOULD BE REMOVED EYALN
#include "OpcodesMrmcCardMngrMrc.h"
#include "SrtpStructsAndDefinitions.h"
#include "IpChannelParams.h"


#define MAX_SSRC_PER_INCOMING_CHANNEL 5
#define MAX_NUMBER_OF_PHYSICAL_RESOURCES 3


typedef struct
{

	APIU32 m_ssrc;
	APIS32 m_dependencyId;
}IncomingSsrcInfo;


typedef struct {
	APIU32			ulID;		/* In: Stream ID */
	APIU32			ulSize;		/* In: Size of stream window */
	APIU32			ulType;		/* In: Unicast... Multicast... */
	APIU32			ulBase;		/* In: Physical Base Address   */
} MrmpTStreamInfo;


typedef struct {
	APIU8			majorVer;
	APIU8			middleVer;
	APIU8			minorVer;
}  MrdVersionStruct;



typedef struct  {
	APIU32						m_channelType;		// kChanneltype
	APIU32						m_channelDirection;	// cmCapDirection.

	mcTransportAddress			m_localAddress;			// UDP IP address
	mcTransportAddress			m_remoteAddress;			// UDP IP address

	mcTransportAddress			m_localRtcpAddress;			// UDP IP address
	mcTransportAddress			m_remoteRtcpAddress;			// UDP IP address

	APIS32						m_capTypeCode;       //CapEnum
	APIU32						m_PayloadType;			// the channel negotiated PayLoad type (dynamic or standard)
	APIU32						m_dtmfPayloadType;			// the channel negotiated PayLoad type (dynamic or standard)
	APIU32 						m_operationPointsSetId; // this is actually confId
	APIU32 						m_partyId;
	IncomingSsrcInfo			m_ssrcInfo[MAX_SSRC_PER_INCOMING_CHANNEL];
	APIU32 						m_videoFlag; // audio only or audio+video call
	APIU32 						m_fLegacy;

	APIU32 						m_allocatedPhysicalResources;
    MCMS_MPL_PHYSICAL_RESOURCE_INFO_S   physicalId[MAX_NUMBER_OF_PHYSICAL_RESOURCES];  // the physical info for audio encoder or audio decoder when opening audio channel -or  physical info of video decoder or encoder for the AVC/SVC mix

	APIU32						m_maxVideoBR; // For MBA usage: upper bit rate limit for video for given participant. For video channel only


	APIU32						uRtpKeepAlivePeriod;         // RTP keepalive period, in seconds
	APIU32						tosValue[NumberOfTosValues]; // QOS information

	//this is the ice channel ID which we get from the ICE stack instead of socket.
	// it can be:
	//	0 - meaning this is NOT an ICE connection - hence we need to open the socket ourselves
	//	!= 0 (1-1000) - meaning that this is an ICE connection and the ICE stack has opened the socket.
	// we use the channel ID for networking operations with the stack
	APIS32						ice_channel_rtp_id;
	APIS32						ice_channel_rtcp_id;
	APIU32						bIsIceEnable;
 	sdesCapSt                   m_sDesCapSt;    // Standard data required for SRTP usage
	// Mix mode 
	APIS32						m_lMrmpOpeningConnID;
	MrmpTStreamInfo				m_XmitStreamInfo;
 	APIU32 					  	ulDetectionTimerLen; 
 	MrdVersionStruct	        mrdVersion;
 	APIU32						bIsLinkParty; // Set to 1 for link party, set to 0 for regular party
} MrmpOpenChannelRequestStruct;


typedef struct  {
	APIS32						channelHandle; //(alternatively we need party_id conf_id media_type media_direction)

	APIU32						m_channelType;		// kChanneltype
	APIU32						m_channelDirection;	// cmCapDirection.

	mcTransportAddress			m_localAddress;			// UDP IP address
	mcTransportAddress			m_remoteAddress;			// UDP IP address

	mcTransportAddress			m_localRtcpAddress;			// UDP IP address
	mcTransportAddress			m_remoteRtcpAddress;			// UDP IP address

    APIS32						ice_channel_rtp_id;
	APIS32						ice_channel_rtcp_id;
}MrmpCloseChannelRequestMessage;



typedef struct  {
	APIU32 								m_allocatedPhysicalResources;
	MCMS_MPL_PHYSICAL_RESOURCE_INFO_S   physicalId[MAX_NUMBER_OF_PHYSICAL_RESOURCES]; 

	MrmpCloseChannelRequestMessage      tMrmpCloseChannelRequestMessage;
}MrmpCloseChannelRequestStruct;



typedef struct
{
	APIU32    unChannelHandle;
	APIU32    unSyncSource;
	APIU32    unLayerId;
	APIU32    unPipeId;
	APIU32    unTId;
	APIUBOOL  bIsSpeaker;
}VideoRealySourceParams;


typedef struct
{
	APIU32 unChannelHandle;
	APIU32 unSequenseNumber;
	APIU32 unSourceOperationPointSetId;
	APIS32 nNumberOfSourceParams;
	VideoRealySourceParams tVideoRelaySourceParamsArray[1];
} VideoRelaySourcesParams;


//SCP structs
	typedef struct
	{
		APIU32   unPipeId;
		APIU32   unNotifyType;  // eScpNotificationType
		APIU32   unReason;     // eScpNotificationReasons
		APIUBOOL bIsPermanent;
	} MrmpScpPipe;

	typedef struct
	{
		APIU32      unChannelHandle;
		APIU32      unSequenseNumber;
		APIU32      unRemoteSequenseNumber;
		APIS32      nNumberOfScpPipes;
	} MrmpScpStreamsNotificationStructBase;

	typedef struct
	{
		APIU32      unChannelHandle;
		APIU32      unSequenseNumber;
		APIU32      unRemoteSequenseNumber;
		APIS32      nNumberOfScpPipes;
		MrmpScpPipe scpPipe[1];
	} MrmpScpStreamsNotificationStruct;

	typedef struct
	{
		APIU32 unChannelType;		// kChanneltype
		APIU32 unPayloadType;
		APIU32 unSpecificSourceSsrc;
		APIU32 unBitRate;
		APIU32 unFrameRate;
		APIU32 unHeight;
		APIU32 unWidth;
		APIU32 unPipeIdSsrc;
		APIU32 unSourceIdSsrc;
		APIU32 unPriority;
	}MrmpStreamDesc;

	typedef struct
	{
		APIU32 		   unChannelHandle;
		APIU32 	   	   unSequenseNumber;
		APIS32 	   	   nNumberOfMediaStream;
	} MrmpScpStreamsRequestStructBase;


	typedef struct
	{
		APIU32 		   unChannelHandle;
		APIU32 	   	   unSequenseNumber;
		APIS32 	   	   nNumberOfMediaStream;
		MrmpStreamDesc mediaStreams[1];
	} MrmpScpStreamsRequestStruct;


	typedef struct
	{
		APIU32 		   unChannelHandle;
		APIU32 	   	   unRemoteSequenseNumber;
		APIUBOOL   	   bIsAck;
	} MrmpScpAckStruct;

//FIR structs
	typedef struct
	{
		APIU32 		   unChannelHandle;
		APIU32   	   unSequenseNumber;
		APIS32 	   	   nNumberOfSyncSources;
		APIUBOOL	   bIsGdr;
	} MrmpRtcpFirStructBase;

	typedef struct
	{
		APIU32 		   unChannelHandle;
		APIU32   	   unSequenseNumber;
		APIS32 	   	   nNumberOfSyncSources;
		APIUBOOL	   bIsGdr;
		APIU32		   syncSources[1];
	} MrmpRtcpFirStruct;

	typedef struct
	{
		APIU32      unChannelHandle;
		APIU32      unSequenseNumber;
		APIU32    	unIvrState;
	} MrmpScpIvrStateNotificationStruct;

	typedef struct
	{
		APIU32   unPipeId;
		APIU32   unCsrc;
	} MrmpScpPipeMapping;

	typedef struct
	{
		APIU32      unChannelHandle;
		APIU32      unSequenseNumber;
		APIU32      unRemoteSequenseNumber;
		APIS32      nNumberOfScpPipes;
		MrmpScpPipeMapping scpPipeMapping[1];
	} MrmpScpPipesMappingNotificationStruct;


	////////////////////////////////////
	//Party monitoring request
	// CONF_PARTY_MRMP_PARTY_MONITORING_REQ
	////////////////////////////////////
	typedef struct
	{
		APIU32      unPartyID;
	} MrmpPartyMonitoringReq;


	typedef struct
	{
		APIU32     unChannelHandle;  //incoming channel
		APIU32     unSyncSource;
		APIUBOOL   bIsMust;   //true means MCU won't send TSPR to this sssr
	} MrmpStreamIsMustStruct;



#endif
