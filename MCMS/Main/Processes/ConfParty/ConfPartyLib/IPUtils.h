//+========================================================================+
//                            IPUTILS.H                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IPUTILS.H                                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Atara Avidan                                                |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 17/2/03    | Utilities fot IP parties		                       |
//+========================================================================+

#ifndef _IP_UTILITIES
#define _IP_UTILITIES

#include <time.h>
#include <math.h>
#include <limits.h>
#include "PObject.h"
#include "IpCommonDefinitions.h"
#include "IpChannelParams.h"
#include "IpRtpFeccRoleToken.h"
#include "IpCsContentRoleToken.h"
#include "ConfPartyDefines.h"
#include "RvCommonDefs.h"
#include "ChannelParams.h"
#include "ConfStructs.h"
#include "SipStructures.h"
#include "RoomControl.h"
#include "TelepresenseEPInfo.h"
#include "VideoOperationPoint.h"


//#include <bigdiv.h>

#include "Capabilities.h"

#include "IpPartyMonitorDefinitions.h"
#include "PartyMonitor.h"
#include "H264.h"

#ifndef _K_
#define _K_ (1000)
#endif

#ifndef NA
#define NA (-1) //not available
#endif

#define MINUTE 60 //seconds
#define IP_LIMIT_ADDRESS_CHAR_LEN 256

// IP Control states:
const WORD  IP_DISCONNECTED		= 0x0001; //as IDLE  state
const WORD  IP_CONNECTING		= 0x0002; //as SETUP state
const WORD  IP_CHANGEMODE		= 0x0003; //as CHANGEMODE state
const WORD  IP_CONNECTED		= 0x0004; //as CONNECT state
const WORD  IP_DISCONNECTING	= 0x0005; //as DISCONNECTING state
const WORD	IP_MEDIA_OPENNING	= 0x0006;
// Media IP mismatch handling state
const WORD	IP_CHANGEMEDIA  	= 0x0007; // As changing udp media ports state

const APIU16 ITP_ZOOM_IN_MODE   			= 0x0008;
const APIU16 ITP_ZOOM_OUT_MODE   			= 0x0010;
const APIU16 ITP_RPX            			= 0x0002;
const APIU16 ITP_FLEX            			= 0x0004;
const APIU16 ITP_MCU            			= 0x0001;
const APIU16 CAM_CAP_MASK					= 0x00E0;
//const APIU16 EP_ZOOM_IN_STITCHED_SEG_VIEW = 0x0060;
//const APIU16 EP_ZOOM_OUT_ROOM_VIEW  	    = 0x00A0;
//const APIU16 EP_SUPPRT_ZOOM_IN_OUT_BOTH  	= 0x00E0;

// Timeout for ACK-s etc. from MFA
#define MFA_RESPONSE_TIME				20

#define MAX_INTERNAL_CHANNELS (5) // avc->svc mode - 2 ART video Tx + 1 ART audio Tx + 1 MRMP video Rx + 1 MRMP audio Rx
                                  // vsw - 2 channels

const BYTE CP		  = 2;
const BYTE VSW_AUTO   = 1;
const BYTE VSW		  = 0;

#define  E1Rate					1920000
#define  MaxCPRate              8192000 //only for MPMx //4096000
#define  MaxE1Rate				1843200
#define  MaxE1MixLinkEnvRate	1787900
#define  MaxE1MixWithEncryptionLinkEnvRate	1787100

#define  rate6144K (6144*_K_)
#define  rate4096K (4096*_K_)
#define  rate1920K (1920*_K_)
#define  rate1472K (1472*_K_)
#define  rate1152K (1152*_K_)
#define  rate1024K (1024*_K_)
#define  rate768K (768*_K_)
#define  rate512K (512*_K_)
#define  rate384K (384*_K_)
#define  rate320K (320*_K_)
#define  rate256K (256*_K_)
#define  rate192K (192*_K_)
#define  rate128K (128*_K_)
#define  rate96K  (96*_K_)
#define  rate64K  (64*_K_)
#define  rate56K  (56*_K_)
#define  rate48K  (48*_K_)
#define  rate32K  (32*_K_)
#define  rate24K  (24*_K_)
#define  rate16K  (16*_K_)
#define  rate8K   (8*_K_)
#define  rate4K	  (4*_K_)

class CCapSetInfo;

typedef enum {
	kH323,
	kSip
} EIpProtocol;

typedef enum {
	kCardManagerUnitId		= 0,
	kStackControllerUnitId	= 1,
	kStartOfRtpUnitId		= 2,
	kEndOfRtpUnitId			= 4
} EUnitId;

typedef enum{
	eNoInitiator = 0,
	ePartyInitiator,
	eConfInitiator,
	ePartyControlInitiator,
	eFallBackFromIce,
	eFallBackFromTip
} eChangeModeInitiator;

// The stream status
typedef enum {
	kStreamOffState,
	kStreamOnState,
	kMuteByRemote,
	kStreamUpdate
//	kStreamDisconnectingState
} EStreamState;

typedef enum {
	Regular, //the remote doesn't need special treatment
	PolycomMGC,
	PolycomRMX,
	PolycomNPG,
	RvMCU,
	RvGWOrProxy,
	RvEp,
	RVTestapplication,
	VconEp,
	TandbergEp,
	VIU,
	NetMeeting,
	EricssonVIG,
	EricssonVigSip,
	SonyEp,
    DstH323Mcs,
    PolycomEp,
    MicrosoftEP_R1,
    MicrosoftEP_R2,
    MicrosoftEP_Lync_R1,
    MicrosoftEP_Lync_2013,
    MicrosoftEP_MAC,
    MicrosoftEP_MAC_Lync,
    AvistarGW,
    CiscoGW,
    LifeSizeEp,
    IbmSametimeEp,
    IbmSametimeEp_Legacy, //BRIDGE-11137
    IbmSametime9_Q42015_AndLater, //VNGSWIBM-1609 - Client which supports video escalation after allocating audio only resources
    PolycomQDX,
    PolycomVVX,
    AvayaEP,
    CiscoCucm,
    MicrosoftMediationServer,
    Microsoft_AV_MCU,
    Microsoft_AV_MCU2013,
    MicrosoftEP_Lync_CCS,
    MicrosoftwPolycomEP_Colab_EP1,
    Polycom_Lync_CCS_Gw,
} RemoteIdent;

typedef enum {
	eSyncLost=1,
	eTransmitIntra,
	eReceiveIntra
} IntraDirection;

typedef enum
{
	MEDIA_DETECTION_AUDIO_RTP 				= 0,
	MEDIA_DETECTION_AUDIO_RTCP 				= 1,
	MEDIA_DETECTION_VIDEO_RTCP 				= 2,
	MEDIA_DETECTION_VIDEO_RTP 				= 3,

	MEDIA_DETECTION_MAX_CONNECTION			= 4,
}enMediaDetectionConnectionType;

typedef struct __MediaDetectionInfo
{
	DWORD	detectTimeLen;// timer length
	time_t	lastIndTime[MEDIA_DETECTION_MAX_CONNECTION];
	int		hasVideo;
	int		isSVCOpened;

	//=====================================================================================================
	// Checking if in the configured last X seconds, we got a media disconnection signal for this channel
	//=====================================================================================================
	inline bool MediaDisconnectedRecently(int connId, time_t currTime = 0)
	{
		time_t 	now = currTime? currTime : time((time_t*)NULL);
		return (now - lastIndTime[connId]) <= (int) detectTimeLen;
	}

	//============================================================================================================
	// Optional extra handling when receiving a media disconnection indication from embedded -
	// checking if we consider this media in timeout (in which case, for instance, a video cell may be removed).
	// PLEASE KEEP THIS IMPLEMENTATION SEPARATE FROM THE CALL DISCONNECTION IMPLEMENTATION OF THIS STRUCT.
	//============================================================================================================
	bool CheckForMediaTimeout(int connId);
	void MediaResumed(int connId);
	__MediaDetectionInfo();
	time_t	timeoutStarted[MEDIA_DETECTION_MAX_CONNECTION];
	time_t	prevIndTime[MEDIA_DETECTION_MAX_CONNECTION];
	bool	inTimeout[MEDIA_DETECTION_MAX_CONNECTION];
	static const DWORD MinimumMuteDuration;

}stMediaDetectionInfo;



#define	MAX_VIDEO_FORMATS 12 //not including formats only for monitoring use
#define	MAX_VIDEO_FORMATS_INCLUDE_MONITORING 10
#define TLS_PORT  5061

extern const char* g_typeStrings[];
extern const char* g_roleLabelStrings[];
extern const char* g_directionStrings[];
extern const char* g_ipChanTypeStrings[];

/*
#ifndef __MCMS_UNITTEST__
extern const char * g_dtmfSourceStrings[dtmfMaxNumber];
inline const char * GetDtmfSourceStr(dtmfSources_e d){return (d<dtmfMaxNumber)? g_dtmfSourceStrings[d]: "Unknown";}
#endif*/ //should not be here - only for SIP!!!

// Operators
inline EFormat operator++(EFormat& e, int)
{
	EFormat eTemp = e;
	e = (EFormat)(e+1);
	return eTemp;
}

inline EFormat operator--(EFormat& e, int)
{
	EFormat eTemp = e;
	e = (EFormat)(e-1);
	return eTemp;
}

inline CapEnum operator++(CapEnum& e, int)
{
	CapEnum eTemp = e;
	e = (CapEnum)(e+1);
	return eTemp;
}

// End of operators
inline cmCapDirection GetDirection(EIpChannelType eChanType) {return ((eChanType&0x01)? cmCapTransmit: cmCapReceive);}
inline cmCapDataType  GetMediaType (EIpChannelType eChanType) {return ((cmCapDataType)(eChanType/2));}
inline ERoleLabel	  GetRoleLable(EIpChannelType eChanType) {return ((ERoleLabel)(eChanType/6));}
inline cmCapDirection  CalcCmCapDirection(BOOL bIsTransmitting) {return (bIsTransmitting? cmCapTransmit : cmCapReceive);}
inline BYTE  CalcIsTransmiting(cmCapDirection direction) {return ((direction == cmCapTransmit) ? TRUE : FALSE);}
cmCapDirection CalcOppositeDirection(cmCapDirection d);

inline const char* GetRoleStr(ERoleLabel r){return (r<=kRoleLiveOrPresentation)?g_roleLabelStrings[r]:"Unknown";}
//inline const char* GetFormatStr(EFormat f){return (f != kUnknownFormat && f< kLastFormat)? g_formatStrings[f]:"Unknown Format";}
inline const char* GetTypeStr(cmCapDataType t){return (t <= cmCapBfcp)? g_typeStrings[t]:"Unknown";}
inline const char* GetDirectionStr(cmCapDirection d){return (d<=cmCapReceiveAndTransmit)?g_directionStrings[d]:"Unknown";}
inline const char* GetIpChanTypeStr(EIpChannelType t){return (t<=BFCP_OUT)? g_ipChanTypeStrings[t]:"Unknown";}

const char* GetMediaLineSubTypeStr(eMediaLineSubType subType);
const char* GetPartyStateStr(WORD partyState);
bool IsMrmpOpcode(DWORD opcode);
BYTE GetCodecNumberOfChannels(CapEnum CapCode);
const char* GetResourceLevelStr(RelayResourceLevelUsage resourceLevel);

const char* GetBfcpSetupStr(eBfcpSetup setup);
const char* GetBfcpConnectionStr(eBfcpConnection connection);
const char* GetBfcpFloorCtrlStr(eBfcpFloorCtrl floorcntl);

WORD IsValidPort(const char* pPort);
WORD IsValidIPAddress(const char* pIP);
EIpChannelType CalcChannelType(cmCapDataType eType,BOOL bTransmit,ERoleLabel eRole,CapEnum capCode=eUnknownAlgorithemCapCode);
int GetPayloadType(CCapSetInfo &capInfo);
BYTE  IsDynamicPayloadType(DWORD payload);
BYTE  IsValidPayloadType(DWORD payload);
kChanneltype DataTypeToChannelType(cmCapDataType dataType, ERoleLabel eRole = kRolePeople);
cmCapDataType ChannelTypeToDataType( kChanneltype eChannelType, ERoleLabel& eRole);
int  AllocateUniqueString(char* sDest, DWORD signalingIpAddress);
void RoundAudioRate(DWORD &actualRate, DWORD channelRate);
void VideoSyncParamsParser(DWORD streamVideoSyncParams, BYTE & rIntraSyncFlag,
						   BYTE & rVideoBCHSyncFlag, WORD & bchOutOfSyncCount,
						   BYTE & rProtocolSyncFlag, WORD & rProtocolOutOfSyncCount);
void BuildCapMatrix(ctCapabilitiesStruct* pCap,int numOfAudioCaps, int numOfVideoCaps, int numOfDataCaps);
void  UpdatePartyMonitoring(TRtpCommonChannelMonitoring *pChannelMonitoring,
							CPrtMontrBaseParams *pPrtMonitrParams,
							DWORD	*pOldMediaBytesArr,
							CapEnum capCode,
							EIpChannelType channelType,
							DWORD channelIndex,
							DWORD callIndex,
							DWORD *pOldFrames,
							DWORD rate,
							char *pChannelParams,
							mcTransportAddress* partyAddr=NULL,
							mcTransportAddress* mcuAddr=NULL,
							BYTE IsIce = 0,
							mcTransportAddress* IcePartyAddr=NULL,
							mcTransportAddress* IceMcuAddr=NULL,
							EIceConnectionType IceConnectionType = kNone);

void  SetPartyMonitorParams(CPrtMontrGlobalParams *pMonitorBase, TAdvanceMonitoringResultsSt *pAdvanceMonitoring);
void  SetPartyMonitorBaseParams(CPrtMontrBaseParams *pPrtMonitrParams,DWORD channelType,DWORD rate=0xFFFFFFFF,
								mcTransportAddress* partyAdd=NULL,mcTransportAddress* mcuAdd=NULL,
								DWORD protocol = (DWORD)eUnknownAlgorithemCapCode,DWORD pmIndex=0,BYTE IsIce = 0,
								mcTransportAddress* IcePartyAdd=NULL,mcTransportAddress* IceMcuAdd=NULL,EIceConnectionType IceConnectionType = kNone);
void SetAdvanceInfo(CAdvanceChInfo& pPrt, TAdvanceInfoSt& advanceInfo,TAdvanceMonitoringResultsSt *pAdvanceMonitoring,
					BOOL isCalculateAccumulated = TRUE, BOOL isCalculateInterval = TRUE, BOOL isOutOfOrderCalculation = FALSE);
void  SetRtpInfo(CRtpInfo *pPrtMonitrRtpParams, TAdvanceMonitoringResultsSt *pAdvanceMonitoring);
BYTE ConvertCapEnumToReservationProtocol(CapEnum capCode,APIU16 profile = H264_Profile_BaseLine);

DWORD  GetMediaDetectionTimer(const std::string& key);

static const char MAX_INT_DIGITS = static_cast<char>(ceil(log10(static_cast<float>(LONG_MAX))));
inline int AtoiBounded(const char* szNumeric)
{
	FPASSERTMSG_AND_RETURN_VALUE(!szNumeric, "AtoiBounded - received a NULL string to convert", 0);
	FPASSERTMSG_AND_RETURN_VALUE(!memchr(szNumeric, 0, MAX_INT_DIGITS + 1), "AtoiBounded - received a string too long to convert to int", 0);

	return atoi(szNumeric);
}

class CQoS : public CPObject
{
	CLASS_TYPE_1(CQoS,CPObject )
public:
    // constructors / destructor
    CQoS();
    CQoS(const QOS_S &rQos);
	virtual const char* NameOf() const { return "CQoS";}
    //CQoS(QOS_S rQos);
    virtual ~CQoS();

    BOOL  IsDynamicRPriorityEnabled() const;
    void  DeSerialize(WORD format,CSegment & seg);
	void  Serialize(WORD format,CSegment& seg) const;
	void  AssembleValFromPrecedence(const QOS_S& rQos);
	void  AssembleValFromRPrio(const char* szPrecedDomain, const BYTE precedRPrio);
	CQoS& operator=(const CQoS& other);
	void  AdjustToService(const QOS_S& qos);

    // ip service fields
    BYTE    m_bIpAudio;
    BYTE    m_bIpVideo;
    BYTE    m_bIpRtcp;
    BYTE    m_bIpSignaling;
    // atm service fields
    BYTE    m_bAtmAudio;
    BYTE    m_bAtmVideo;

protected:
    BYTE	m_audioFromService;
    BYTE	m_videoFromService;
};


class CConf;
class CH323NetSetup;
class CComModeH323;
class CCapH323;
class CAudioBridgeInterface;
class CVideoBridgeInterface;
class CConfAppMngrInterface;
class CFECCBridge;
class CContentBridge;
class CTerminalNumberingManager;
class COsQueue;
class CTaskApp;
class CCopVideoTxModes;

typedef struct {
		CConf*									pConf;
		CH323NetSetup*							pH323NetSetUp;
		CComModeH323* 							pH323Scm;
		CCapH323*								pH323Caps;
		COsQueue*								pConfRcvMbx;
		CAudioBridgeInterface*					pAudioBridgeInterface;
		CVideoBridgeInterface*					pVideoBridgeInterface;
		CConfAppMngrInterface*					pConfAppMngrInterface;
		CFECCBridge*							pFECCBridge;
		CContentBridge*							pContentBridge;
		CTerminalNumberingManager*				pTerminalNumberingManager;
		COsQueue*								pPartyRcvMbx;
		CTaskApp*								pParty;
		CCopVideoTxModes*						pCopVideoTxModes;
		WORD									termNum;
        WORD									type;
        const char*								pPartyName;
        const char*								pConfName;
        const char*								pPassword;
        DWORD									monitorConfId;
        DWORD									monitorPartyId;
        WORD									TcMode;
        CQoS*									pQos;
        WORD									cascadeMode;
        WORD									serviceId;
        WORD									nodeType; // = 0
        BYTE									voice; // = 0,
        WORD									stby; // = 0,
        DWORD									connectDelay; // = 0,
        DWORD									h323VideoRate; // = 0
        const char*								pAV_service_name; // = NULL,
        WORD									welcome_msg_time; // = 0
        WORD									bIsAudioOnly; // = FALSE,
        BYTE									IsGateWay; // = 0,
        BYTE									IsRecording; // = NO
        BYTE									IsStreaming; // = NO
        const char*								pExchangeServerConfId; // = NULL,
        WORD									sourceId; // = 0,
        eSubCPtype								bySubCPtype; // = eSubCPtypeClassic,
        WORD									isUndefParty; // = NO,
        BYTE									isAutoBitRate; // = 1,
        eTelePresencePartyType					eTelePresenceMode; // = eTelePresencePartyNone

        //Multiple links for ITP in cascaded conference feature:
        eTypeOfLinkParty                        linkType;
        DWORD                                   roomID;

} stH323AddPartyCreateParam;


#endif //_IPUTILITIES
