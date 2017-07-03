// IpPartyMonitoring.h
// Assaf.

#ifndef __IP_PARTY_MONITORING__
#define __IP_PARTY_MONITORING__

#include "DataTypes.h"
#include "MplMcmsStructs.h"

//#define __PartyMonitoringDebug__
#define audioTimestampToMilli 8
#define videoTimestampToMilli 90

// Video Sync Macros:
// ------------------
#define ValidBitMask					0x80000000
#define BchSyncMask					    0x40000000 
#define BchCountMask					0x3FFF0000
#define IntraSyncBitMask				0x00008000
#define ProtocolSyncMask				0x00004000
#define ProtocolCountMask				0x00003FFF
#define SyncCountMask					0x00003FFF



// VSR Macros:
// -------------
#define VSR_MAX_ENTRIES           	 	8
#define VSR_SOURCE_NONE 	         	0xFFFFFFFF
#define VSR_SOURCE_ANY   		  		0xFFFFFFFE
#define VSR_NUM_OF_QUALITY_LEVELS       8
#define VSR_NUM_OF_BITRATE_RANGES       10

// DSH Macro:
// -------------
#define DSH_MAX_ENTRIES                 10  


////////////////////////////////////
//RTP party monitoring - structure building blocks
// 
// 
////////////////////////////////////
typedef struct{
	APIU32							unAccumulated;
	APIU32							unInterval;
	APIU32							unPeak;
}TAdvanceInfoSt;


typedef struct{
	TAdvanceInfoSt					tPacketsActualLoss;
	TAdvanceInfoSt					tPacketsOutOfOrder;
	TAdvanceInfoSt					tPacketsFragmented;
	TAdvanceInfoSt					tJitterBufferSize;
	TAdvanceInfoSt					tJitterLatePackets;
	TAdvanceInfoSt					tJitterOverFlows;
	TAdvanceInfoSt					tJitterSamplePacketInterval;
	TAdvanceInfoSt					tErrorResilienceRepairs;
	APIU32							unAccumulatedPacket;
	APIU32							unIntervalPacket;
}TRtpStatisticSt;


typedef struct  {
	APIU32							unRtcpSrCnt;
	APIU32							unRtcpRrCnt;
	APIU32							unRtcpRrReceptionBlockCnt;
	APIU32							unAccumulatedPacketLoss;
	APIU32							unAccumulatedPacket;
	APIU32							unIntervalJitter;
	APIU32							unLatency;
	APIU32							unIntervalPeakJitter;
	APIU32							unIntervalPeakFractionLoss;
	APIU32							unIntervalFractionLoss;
}TRtcpInfoSt;


typedef struct	{
	TRtcpInfoSt						tRtcpInfoSt;
	TRtpStatisticSt					tRtpStatisticSt;
}TAdvanceMonitoringResultsSt;

//COMMON
typedef struct	{
	APIU32							bunValidChannel;
	APIU32							unChannelType;				//Video/Audio/Content/FECC
	APIU32							unChannelDirection;
	APIU32							unTicksInterval;
	APIU32							unMediaBytes;
	APIU32							unProtocol;
}TRtpCommonChannelMonitoring;

//VIDEO
typedef struct {
	TRtpCommonChannelMonitoring	tRtpCommonChannelMonitoring;

	APIU32							unFrameRate;
	APIU32							unMaxFrameRate;
	APIU32							unMinFrameRate;
	APIU32							unVideoResolution;
	APIU32							unVideoMaxResolution;
	APIU32							unVideoMinResolution;
	APIU32                          unVideoWidth;
	APIU32                          unVideoMaxWidth;
	APIU32                          unVideoMinWidth;
	APIU32                          unVideoHeight;
	APIU32                          unVideoMaxHeight;
	APIU32                          unVideoMinHeight;
	APIU32							unAnnexes;

	APIU32							unStreamVideoSync;
	TAdvanceMonitoringResultsSt		tAdvanceMonitoringResultsSt;
} TRtpVideoChannelMonitoring;

//AUDIO
typedef struct {
	TRtpCommonChannelMonitoring		tRtpCommonChannelMonitoring;

	APIU32							unFramesPerPacket;
	TAdvanceMonitoringResultsSt		tAdvanceMonitoringResultsSt;
} TRtpAudioChannelMonitoring;

//FECC
typedef struct {
	TRtpCommonChannelMonitoring		tRtpCommonChannelMonitoring;

	TAdvanceMonitoringResultsSt		tAdvanceMonitoringResultsSt;
} TRtpFeccChannelMonitoring;


////////////////////////////////////
//RTP party monitoring indication union
// 
// 
////////////////////////////////////
typedef union {
	TRtpVideoChannelMonitoring		tRtpVideoChannelMonitoring;
	TRtpAudioChannelMonitoring		tRtpAudioChannelMonitoring;
	TRtpFeccChannelMonitoring		tRtpFeccChannelMonitoring;
}TRtpChannelMonitoringInd;

typedef enum {
	RTCP_PLI 	= 1,			//Picture Loss Indication
	RTCP_SLI 	= 2,			//Slice Lost Indication
	//RTCP_RPSI 	= 3,			//Reference Picture Selection Indication 
	RTCP_TMMBR 	= 3,			//Temporary Maximum Media Bit Rate request 
	RTCP_FIR 	= 4,			//Full Intra Request
	RTCP_TSTR 	= 5,			//Temporal-Spatial Trade-off Request
	RTCP_VBCM 	= 6, 			//Temporal-Spatial Trade-off Notification
	RTCP_TIP_IDR= 7, 		// Full intra request for TIP devices
	RTCP_TIP_GDR= 8, 		// Gradual intra request for TIP devices
	RTCP_TIP_PKT_LOSS = 9, 		// informing ragarding the remote EP reported packet loss (via TIP feedback packet)
	RTCP_APP_LAYER_FB = 15, 	//Application layer FB message
	RTCP_FUTURE_EXPANSION = 31, //reserved for future expansion of the number space
	RTCP_WEBRTC_TMMBR = 50,		// WebRTC TMMBR
	RTCP_WEBRTC_FIR =	51,		//Webrtc Intra
	RTCP_INTRA_RTV = 100        //RTV Full Intra   

} eRtcpMsgType;


//====================================================

//TPortMessagesHeader tPortMessagesHeader; includes hidden in message

typedef struct
{
	APIU32              uDspNumber;
	APIU32 				uPortNumber; 
} TCmRtcpHeader;

//IP_CM_RTCP_MSG_REQ
//IP_CM_RTCP_MSG_IND

typedef struct
{
	APIU32              uMediaType;
	APIU32 				uMsgType;   //enum rtcpMsgType
	APIU32	    		uTipPosition; // enum ETipVideoPosition. relevant only if message type is RTCP_TIP_IDR or RTCP_TIP_GDR
	APIU32      		uSeqNumber; // 0 for any type, except RTCP_INTRA_RTV
    APIU32              ulIce_rtcp_channel_id;
    APIU32              uSsrc;      //Remote ssrc for RTV intra and NON_SSRC (0xFFFFFFFF) otherwise 
} TCmRtcpMsgInfo;

typedef struct
{
	TCmRtcpHeader 		tCmRtcpHeader;
	TCmRtcpMsgInfo 		tCmRtcpMsgInfo;
} TCmRtcpMsg;

typedef struct
{
    	UINT32 MxTBRExp;
    	UINT32 MxTBRMantissa;
    	UINT32 MxTBRMeasuredOverhead;
    	APIU32 uMsgType;   //enum rtcpMsgType
	APIU32 uMediaType;
}TCmRtcpRTPFBInfo; // For TMMBR feedback

typedef struct
{
	TCmRtcpHeader 		tCmRtcpHeader;
	TCmRtcpRTPFBInfo 	tCmRtcpRTPFBInfo;
} TCmRtcpRTPFB; // For TMMBR feedback
//====================================================
// 
//IP_CM_RTCP_GET_ESTIMATE_BANDWIDTH_REQ			  
//IP_CM_RTCP_GET_ESTIMATE_BANDWIDTH_IND			  
 
//IP_CM_RTCP_POLICY_SERVER_BANDWIDTH_REQ          
//IP_CM_RTCP_POLICY_SERVER_BANDWIDTH_IND            

//IP_CM_RTCP_RECEIVER_BANDWIDTH_REQ               
//IP_CM_RTCP_RECEIVER_BANDWIDTH_IND                 

//IP_CM_RTCP_TURN_SERVER_BANDWIDTH_REQ            
//IP_CM_RTCP_TURN_SERVER_BANDWIDTH_IND              

typedef struct
{
    TCmRtcpHeader 			tCmRtcpHeader;   
	APIS32         			nBandwidth;
} TCmRtcpBandwidth;

//====================================================

//IP_CM_RTCP_VIDEO_PREFERENCE_REQ
//IP_CM_RTCP_VIDEO_PREFERENCE_IND

typedef struct
{
    APIU32         uFrameResWidth;
    APIU32         uFrameResHeight; 
    APIU32         uBitRate;
    APIU32         uFrameRate;

 } TCmRtcpVideoPreferenceInfo;

typedef struct
{
    TCmRtcpHeader 			   tCmRtcpHeader;
    TCmRtcpVideoPreferenceInfo tCmRtcpVideoPreferenceInfo;  

} TCmRtcpVideoPreference;

//====================================================

// IP_CM_RTCP_GET_PEER_EXCHANGE_IND				  

typedef struct
{
    APIU32         uInboundLinkBandwidth;   // maximum inbound bandwidth the host can support
    APIU32         uOutboundLinkBandwidth;  // maximum outbound bandwidth the host can support
    APIUBOOL       fNoCache;             	// indicates that it should not be used beyond this session ( not cached)      


} TCmRtcpReerExchangeInfo;

typedef struct
{
    TCmRtcpHeader           tCmRtcpHeader;   
    TCmRtcpReerExchangeInfo tCmRtcpReerExchangeInfo;
} TCmRtcpReerExchange;


// ====================================================
// ======== Lync 2013 START ===========================
// ====================================================


//====================================================
// IP_CM_RTCP_VSR_REQ 
// IP_CM_RTCP_VSR_IND 

typedef struct
{              
    APIU32                 minBitRate;
    APIU32                 bitRatePerLevel;
    APIU32                 frameRateBitmask;
    APIU32                 maximumNumberOfPixels;

    APIU16                 numberOfMustInstances;
    APIU16                 numberOfMayInstances;
    APIU16                 maxWidth;
    APIU16                 maxHeight;

    APIU16                 qualityReportHistogram[VSR_NUM_OF_QUALITY_LEVELS];
    APIU16                 bitRateHistogram[VSR_NUM_OF_BITRATE_RANGES];

    APIU8                  payloadType;
    APIU8                  flags;
    APIU8                  aspectRatioBitMask;
    APIU8                  dummyPadding;
} TCmRtcpVsrEntry;


typedef struct
{
    APIU32                 senderSSRC;
    APIU32                 MSI;
    APIU16                 requestId;
    APIU8                  keyFrame;
    APIU8                  numberOfEntries;   // Other than source_none:  MCMS -> CM should always be 2, CM -> MCMS: 2 for P2P, <=5 for AVMCU
    TCmRtcpVsrEntry        VSREntry[VSR_MAX_ENTRIES];
} TCmRtcpVsrInfo;

typedef struct
{
    TCmRtcpHeader          cmRtcpHeader;
    TCmRtcpVsrInfo         cmRtcpVsrInfo;
} TCmRtcpVsrMsg;



//====================================================
// IP_CM_RTCP_MS_SVC_PLI_REQ
// IP_CM_RTCP_MS_SVC_PLI_IND

typedef struct
{
    APIU32                 senderSSRC;
    APIU32                 mediaSourceSSRC;
    APIU16                 requestId;
    APIU8                  SFR0;
    APIU8                  SFR1;
    APIU8                  SFR2;
    APIU8                  SFR3;
    APIU8                  SFR4;
    APIU8                  SFR5;
    APIU8                  SFR6;
    APIU8                  SFR7;
    APIU16                 dummyPadding; 
} TCmRtcpMsSvcPLIInfo   ;

typedef struct
{
    TCmRtcpHeader          cmRtcpHeader;
    TCmRtcpMsSvcPLIInfo    cmRtcpMsSvcPLI;
} TCmRtcpMsSvcPLIMsg;


//====================================================
// IP_CM_RTCP_DSH_IND 

typedef struct
{
	APIU32                 dominantSpeakerMsi;
    APIU32                 DSHEntry[DSH_MAX_ENTRIES];
} TCmRtcpDshInfo   ;

typedef struct
{
    TCmRtcpHeader          cmRtcpHeader;
    TCmRtcpDshInfo         cmRtcpDsh;
} TCmRtcpDshMsg;


// ======== Lync 2013 END ===========================





#endif // __IP_PARTY_MONITORING__
