//+========================================================================+
//                       H323MfaReq.h	                                   |
//            Copyright 1995 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       IpCmReq.h	                                               |
// SUBSYSTEM:  MFA/ConfParty                                                |
// PROGRAMMER: Guy D,													   |
// Date : 15/5/05														   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+                        
/*

This header file contains all mutual request structures related to 
ConfParty + MFA entities within the Carmel.
In this document we set the common structures.

*/

#ifndef __IPCMREQ_H__
#define __IPCMREQ_H__

#include "DataTypes.h"
//#include "IpCommonParams.h"
#include "IpAddressDefinitions.h"
#include "IpQosDefinitions.h"
#include "PhysicalResource.h"
#include "OpcodesMcmsCardMngrIpMedia.h"
#include "SrtpStructsAndDefinitions.h"
#include "IpChannelParams.h"

#define  MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS        5//4

typedef enum {
	eUdp,           // CM should use UDP as the transport type
	eTcpActive, 	// CM should connect to remote that is listening
	eTcpPassive, 	// CM is the listener and should wait to remote to open connection
	eInvalidTransportType
} eConnectionTransportMode;

// H323_CM_OPEN_UDP_PORT_REQ
// H323_CM_UPDATE_UDP_ADDR_REQ
// SIP_CM_OPEN_UDP_PORT_REQ
// SIP_CM_UPDATE_UDP_ADDR_REQ
typedef struct  {
	APIU32						channelType;		// kChanneltype
	APIU32						channelDirection;	// cmCapDirection.
	mcTransportAddress			CmLocalUdpAddressIp;			// UDP IP address  
	APIU32						LocalRtcpPort;
	mcTransportAddress			CmRemoteUdpAddressIp;			// UDP IP address 
	APIU32						RemoteRtcpPort;
        APIU32                                          uRtpKeepAlivePeriod;         // RTP keepalive period, in seconds
	APIU32						tosValue[NumberOfTosValues]; // QOS information
	//this is the ice channel ID which we get from the ICE stack instead of socket. 
	// it can be:
	//	0 - meaning this is NOT an ICE connection - hence we need to open the socket ourselves
	//	!= 0 (1-1000) - meaning that this is an ICE connection and the ICE stack has opened the socket. 
	// we use the channel ID for networking operations with the stack
	APIS32						ice_channel_rtp_id;
	APIS32						ice_channel_rtcp_id;	
	APIS32						capProtocolType;       //CapEnum
	sdesCapSt					sdesCap;
	//TIP
	APIU32                      bIsTipEnable; //always false in H323, true in SIP + TIP, false in SIP + NO TIP
	APIU16                      RtcpCnameMask; //this is the WORd used for the SDES CNAME mask

	// BFCP
	eConnectionTransportMode	connMode;
	APIU32 			ulDetectionTimerLen;
	APIU32          uMsftType;   // according to MSF_CLIENT_ENUM

}mcReqCmOpenUdpPortOrUpdateUdpAddr;


typedef struct  {
	MCMS_MPL_PHYSICAL_RESOURCE_INFO_S    physicalPort;
	mcReqCmOpenUdpPortOrUpdateUdpAddr    tCmOpenUdpPortOrUpdateUdpAddr;
}TOpenUdpPortOrUpdateUdpAddrMessageStruct;

// H323_CM_CLOSE_UDP_PORT_REQ
// SIP_CM_CLOSE_UDP_PORT_REQ
typedef struct  {
	APIU32						channelType;	// kChanneltype
	APIU32						channelDirection;
	mcTransportAddress			CmLocalUdpAddressIp;			// UDP IP address + UDP port 
	APIU32						LocalRtcpPort;					
	mcTransportAddress			CmRemoteUdpAddressIp;			// UDP IP address + UPD port  
	APIU32						RemoteRtcpPort;					
	//this is the ice channel ID which we get from the ICE stack instead of socket. 
	// it can be:
	//	0 - meaning this is NOT an ICE connection - hence we need to open the socket ourselves
	//	!= 0 (1-1000) - meaning that this is an ICE connection and the ICE stack has opened the socket. 
	// we use the channel ID for networking operations with the stack
	APIS32						ice_channel_rtp_id;
	APIS32						ice_channel_rtcp_id;
}mcReqCmCloseUdpPort;

typedef struct  {
	MCMS_MPL_PHYSICAL_RESOURCE_INFO_S    physicalPort;
	mcReqCmCloseUdpPort					 tCmCloseUdpPort;
}TCloseUdpPortMessageStruct;

typedef struct  {
	APIU32 udp_ports[MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS];
    APIU32 rtcp_ports[MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS];
	APIU32 ulIsIceConn;
}mcKillUdpPortRequestStruct;

typedef struct  {
	MCMS_MPL_PHYSICAL_RESOURCE_INFO_S    physicalPort;
	mcKillUdpPortRequestStruct           tCmKillUdpPort;
}TKillUdpPortMessageStruct;

// IP_CM_START_PREVIEW_CHANNEL
typedef struct{
    APIU32                      channelType;               // kChanneltype
    APIU32                      channelDirection;
    mcTransportAddress          remotePreviewAddress;
    previewStreamPayloadType    payloadType;
} mcReqCmStartPreviewChannel;

/*
How to fill example:

	if(CmLocalUdpAddressIp.ipVersion == eIpVersion4)// equel to 0
	{// IpV4
		CmLocalUdpAddressIp.addr.v4.ip = 0x23ac1b4f;//an example
	}
	else if(CmLocalUdpAddressIp.ipVersion == eIpVersion6)// equel to 4
	{
		CmLocalUdpAddressIp.addr.v6.ip = ;//array of 16 char
		CmLocalUdpAddressIp.addr.v6.scopeId = ????; // 
	}
	else
	{// the values cmTransportTypeIPStrictRoute, cmTransportTypeIPLooseRoute, cmTransportTypeNSAP
	 // currently are not in use.
	}

	CmLocalUdpAddressIp.port = 60234;// an example. Port is in WORD limit.
	CmLocalUdpAddressIp.distribution = ;// Don't Care.// unicast or multicast
	CmLocalUdpAddressIp.transportType = eTransportTypeUdp;// an example
	tosValue[MEDIA_TOS_VALUE_PLACE] = m_pQos->m_bIpVideo;
	tosValue[RTCP_TOS_VALUE_PLACE] = m_pQos->m_bIpRtcp;
	

*/

// IP_CM_DTLS_START_REQ
typedef struct{
	APIU32						channelType;		// kChanneltype
	APIU32						channelDirection;	// cmCapDirection.
	mcTransportAddress			CmLocalUdpAddressIp;// UDP IP address
	APIU32						LocalRtcpPort;
	mcTransportAddress			CmRemoteUdpAddressIp;// UDP IP address
	APIU32						RemoteRtcpPort;

	sdesCapSt					sdesCap;			// local sdes caps

} mcReqCmDtlsStart;

typedef struct  {
	MCMS_MPL_PHYSICAL_RESOURCE_INFO_S    	physicalPort;
	mcReqCmDtlsStart           		tCmDtlsStart;
}TDtlsStartStruct;

// IP_CM_DTLS_CLOSE_REQ
typedef struct{
	APIU32						channelType;		// kChanneltype
	APIU32						channelDirection;	// cmCapDirection.
	mcTransportAddress			CmLocalUdpAddressIp;// UDP IP address
	APIU32						LocalRtcpPort;
	mcTransportAddress			CmRemoteUdpAddressIp;// UDP IP address
	APIU32						RemoteRtcpPort;

} mcReqCmDtlsClose;

typedef struct  {
	MCMS_MPL_PHYSICAL_RESOURCE_INFO_S    	physicalPort;
	mcReqCmDtlsClose           		tCmDtlsClose;
}TDtlsCloseStruct;

//-------- Lync 2013 related opcodes ----------

#define MAX_STREAM_AVMCU_CONN       9 //3 for xmit, 5 for recv, 1 dummy
#define MAX_STREAM_BASIC_LYNC_CONN  1
#define MAX_STREAM_MUX_LYNC_CONN    3
#define MAX_STREAM_DMUX_LYNC_CONN   6
#define MAX_STREAM_LYNC_2013_CONN   6 //max stream per connection is the DMUX stream+1 for the dummy

typedef struct
{
      UINT32  bIsActive; 	// bIsActive = TRUE only if there is a valid DSP this is for mux/demux VSR shouldn't use this
      UINT32  unPartyId;
      UINT32  unDspNum;
      UINT32  unPortNum;
      UINT32  remoteSSRCRange[2]; //the first location should be the lower range, the second will be the higher range - Video Rx channel -DeMux
      UINT32  localSSRCRange[2];  //the first location should be the lower range, the second will be the higher range - Video Tx channel -Mux
      UINT32  lyncMSI;
      UINT32  localMSI;
	  UINT32  bUpdateArtInfo;     // If true, need to update Art encryption info (seqNum and ROC)

}SingleStreamDesc;


// CONFPARTY_CM_INIT_ON_LYNC_CALL_REQ
typedef struct
{
    UINT32      				bIsAVMCU; //sould be 0
	UINT32      				bIsIceParty;
    SingleStreamDesc 			connectedParties[MAX_STREAM_BASIC_LYNC_CONN];

} mcBasicLync2013InfoReq;

//  CONFPARTY_CM_INIT_ON_AVMCU_CALL_REQ
typedef struct
{
   UINT32						bIsAVMCU; //should be 1
   UINT32						bIsIceParty; 
   SingleStreamDesc				ConnectedParties[MAX_STREAM_LYNC_2013_CONN];
   // Encryption support:
   UINT32 bIsEncrypted;         //Only if bIsEncrypted is on, the next 2 arrays (localEncryptionParam & RemoteEncryptionParam) are valid
   sdesCapSt localEncryptionParam [MAX_STREAM_LYNC_2013_CONN];  //Key info for media and RTCP Tx Streams for encryption
   sdesCapSt RemoteEncryptionParam[MAX_STREAM_LYNC_2013_CONN];  //Key info for media and RTCP Rx Streams for decryption
} mcAvMcuLync2013InfoReq;


// CONFPARTY_CM_MUX_ON_AVMCU_CALL_REQ
typedef struct
{
   UINT32      					bIsAVMCU;//should be 1
   UINT32      					bIsIceParty; 
   SingleStreamDesc 			txConnectedParties[MAX_STREAM_MUX_LYNC_CONN];

} mcMuxLync2013InfoReq;


// CONFPARTY_CM_DMUX_ON_AVMCU_CALL_REQ
typedef struct
{
    UINT32	    				bIsAVMCU;//should be 1
	UINT32      				bIsIceParty; 
    SingleStreamDesc 			rxConnectedParties[MAX_STREAM_DMUX_LYNC_CONN];

} mcDmuxLync2013InfoReq;

// RTP_PARTY_MONITORING_AV_MCU_REQ 
typedef struct
{
	UINT32  unPartyId;
    UINT32  unDspNum;
    UINT32  unPortNum;
}DspInfoForPartyMonitoringReq;

typedef struct
{
   UINT32      			         bIsIceParty; 
   DspInfoForPartyMonitoringReq  DSPInfoList[MAX_STREAM_AVMCU_CONN]; //The first element in the struct in main party. If partyid=0 the ART is not active.

} cmPartyMonitoringAvMcuReq;

#endif


