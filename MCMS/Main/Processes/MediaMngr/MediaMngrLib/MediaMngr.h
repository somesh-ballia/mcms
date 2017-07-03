#ifndef MEDIAMNGR_H_
#define MEDIAMNGR_H_

#include "DataTypes.h"
#include "VideoApiDefinitions.h"
#include "VideoApiDefinitionsStrings.h"
#include "MplMcmsProtocol.h"


using namespace std;

////////////////////////////////////////////////////////////////////////////
///
///     DECLARATIONS
///
////////////////////////////////////////////////////////////////////////////

class CMediaMngrCfg;
class CMediaRepository;

class CDtmfAlgDB;



 
////////////////////////////////////////////////////////////////////////////
///
///     GLOBALS
///
////////////////////////////////////////////////////////////////////////////

static CMediaMngrCfg* g_pMediaMngrCfg = NULL;
static CMediaRepository* g_pMediaRepository = NULL;


//static BOOL g_bAudioRecording = FALSE;
//static BOOL g_bVideoRecording = FALSE;



////////////////////////////////////////////////////////////////////////////
///
///     EXTERNALS
///
////////////////////////////////////////////////////////////////////////////

	//media manager configuration
extern CMediaMngrCfg* GetMediaMngrCfg();
extern void SetMediaMngrCfg(CMediaMngrCfg* pCfg);


	//media and channels
extern char* ResourceTypeToString(APIU32 resourceType);
extern char* MediaTypeAndDirectionToString(APIU32 channelIndex);


	//media repository (video, audio, dtmf)
extern CMediaRepository* GetMediaRepository();
extern void SetMediaRepository(CMediaRepository* pMediaRepository);


	//dtmf
extern char GetTone(unsigned long toneId);


	//Recording Audio/Video
extern BOOL GetAudioRecording();
extern void SetAudioRecording(BOOL audioRecording);

extern BOOL GetVideoRecording();
extern void SetVideoRecording(BOOL videoRecording);

	//Checking Sequence Number Audio/Video
extern BOOL GetAudioCheckSeqNum();
extern void SetAudioCheckSeqNum(BOOL audioCheckSeqNum);

extern BOOL GetVideoCheckSeqNum();
extern void SetVideoCheckSeqNum(BOOL videoCheckSeqNum);
extern BOOL GetDetectIntra();
extern void SetDetectIntra(BOOL detectIntra);



////////////////////////////////////////////////////////////////////////////
///
///     DEFINES
///
////////////////////////////////////////////////////////////////////////////

#define MAX_NUM_OF_NIC_SLOTS     8
#define MAX_NUM_OF_ENDPOINTS     800

#define MAX_NUM_OF_BYTES_IN_UDP_DATAGRAM	8192

#define MAX_PACKETS_PER_FRAME	        77
#define MAX_PACKET_SIZE					MAX_NUM_OF_BYTES_IN_UDP_DATAGRAM

#define MAX_PACKET_SIZE_ENCRYPTION		MAX_PACKET_SIZE+128

#define MAX_FILE_BUFFER_SIZE		    100000000

#define MEDIA_WRITE_BUFFER_SIZE		    200000
#define MAX_MEDIA_WRITE_FILE_SIZE		100000000


#define NUM_OF_MEDIA_CHANNELS	 		8

#define VIDEO_OUT_CHANNEL_INDEX	 		0
#define VIDEO_IN_CHANNEL_INDEX	 		1
#define AUDIO_OUT_CHANNEL_INDEX	 		2
#define AUDIO_IN_CHANNEL_INDEX	 		3
#define CONTENT_OUT_CHANNEL_INDEX	 	4
#define CONTENT_IN_CHANNEL_INDEX	 	5
#define FECC_OUT_CHANNEL_INDEX	 		6
#define FECC_IN_CHANNEL_INDEX	 	 	7



#define MEDIA_DIRECTION_IN				 1
#define MEDIA_DIRECTION_OUT				 2
#define MEDIA_DIRECTION_IN_OUT			 3


#define MAX_DTMF_LEN 					100


//TIMERS

#define TIMER_SEND_MEDIA 				100
#define TIMER_RECV_MEDIA 				200
#define TIMER_ENABLE_INTRA 				300
#define GLOBAL_TX_TIMER 				400
#define GLOBAL_SWITCH_TIMER 			500


#define MAX_EVENT_MESSAGE_STACK_SIZE	50


//STATUS
#define STATUS_OK                 		0
#define STATUS_ERROR               	   -1
#define STATUS_IN_PROGRESS              1



//VIDEO endpoint connection rate
#define VIDEO_64_KBPS		"64"
#define VIDEO_128_KBPS		"128"
#define VIDEO_192_KBPS		"192"
#define VIDEO_256_KBPS		"256"	//new from V7
#define VIDEO_320_KBPS		"320"
#define VIDEO_448_KBPS		"448"
#define VIDEO_704_KBPS		"704"
#define VIDEO_768_KBPS		"768"	//new from V7
#define VIDEO_960_KBPS		"960"
#define VIDEO_1088_KBPS		"1088"	//new from V7
#define VIDEO_1216_KBPS		"1216"	//new from V7
#define VIDEO_1408_KBPS		"1408"
#define VIDEO_1472_KBPS		"1472"
#define VIDEO_1664_KBPS		"1664"	//new from V7
#define VIDEO_1856_KBPS		"1856"
#define VIDEO_1984_KBPS		"1984"	//new from V7
#define VIDEO_2496_KBPS		"2496"	//new from V7
#define VIDEO_3008_KBPS		"3008"	//new from V7
#define VIDEO_3520_KBPS		"3520"	//new from V7
#define VIDEO_4032_KBPS		"4032"
#define VIDEO_6080_KBPS		"6080"
#define VIDEO_8128_KBPS		"8128"	//new from V7



//MACRO BLOCK   (1 MacroBlock = 16*16 pixel = 256 pixels)
#define MACRO_BLOCK					256
#define MACRO_BLOCK_PER_SECOND		500



//RESOLUTION - Frame size in MB for specific resolution (from table)
#define FRAME_SIZE_MB_RES_CIF		396
#define FRAME_SIZE_MB_RES_2CIF		792
#define FRAME_SIZE_MB_RES_4CIF		1584
#define FRAME_SIZE_MB_RES_HD720		3600
#define FRAME_SIZE_MB_RES_HD1080	8160



//ENUMS

//MM Video Resolution
typedef enum
{
    E_MM_VIDEO_RES_DUMMY,	
    E_MM_VIDEO_RES_QCIF,
	E_MM_VIDEO_RES_CIF,       // for less than 2 Macro Block                                   
	E_MM_VIDEO_RES_2CIF,      // for less than 5 MB  more than 2 Macro Block                                     
	E_MM_VIDEO_RES_4CIF,		// for less than 14 MB  more than 5 Macro Block
	E_MM_VIDEO_RES_HD720,		// for less than 31 MB more than 14 Macro Block
	E_MM_VIDEO_RES_HD1080,		// for more than 31 Macro Block
	E_MM_VIDEO_RES_SIF,
	E_MM_VIDEO_RES_2SIF,
	E_MM_VIDEO_RES_4SIF,
	E_MM_VIDEO_RES_LAST
} EMMVideoResolution;

static const char * EMMVideoResolutionNames[] = 
{
	"E_MM_VIDEO_RES_DUMMY",
	"E_MM_VIDEO_RES_QCIF",
	"E_MM_VIDEO_RES_CIF",
	"E_MM_VIDEO_RES_2CIF",
	"E_MM_VIDEO_RES_4CIF",
	"E_MM_VIDEO_RES_HD720",
	"E_MM_VIDEO_RES_HD1080",
	"E_MM_VIDEO_RES_SIF",
	"E_MM_VIDEO_RES_2SIF",
	"E_MM_VIDEO_RES_4SIF",
	"E_MM_VIDEO_RES_LAST"
};

static const char * EShortMMVideoResolutionNames[] = 
{
	"DUMMY",
	"QCIF",
	"CIF",
	"2CIF",
	"4CIF",
	"HD",
	"HD1080",
	"SIF",
	"2SIF",
	"4SIF",
	"LAST"
};


//MM Video Frame Rate
typedef enum
{
	E_MM_VIDEO_FPS_DUMMY,
	E_MM_VIDEO_30_FPS,
	E_MM_VIDEO_25_FPS,
	E_MM_VIDEO_20_FPS,
	E_MM_VIDEO_15_FPS,
	E_MM_VIDEO_12_5_FPS,
	E_MM_VIDEO_10_FPS,
	E_MM_VIDEO_7_5_FPS,	
	E_MM_VIDEO_FPS_LAST
} EMMVideoFrameRate;


static const char * EShortMMVideoFrameRateNames[] = 
{
	"FPS_DUMMY",
	"30",
	"25",
	"20",
	"15",
	"12.5",
	"10",
	"7.5",
	"FPS_LAST"
};

//MM Video Profile Type
typedef enum
{
	E_MM_VIDEO_PROFILE_DUMMY,
	E_MM_VIDEO_PROFILE_BASELINE,
	E_MM_VIDEO_PROFILE_HIGH,
	E_MM_VIDEO_PROFILE_LAST
} EMMVideoProfileType;


static const char * EShortMMVideoProfileTypeNames[] =
{
	"PROFILE_DUMMY",
	"BL",
	"HI",
	"PROFILE_LAST"
};




//Media Repository
typedef enum
{
    E_DB_MEDIA_VIDEO,
    E_DB_MEDIA_AUDIO,
    E_DB_MEDIA_CONTENT,
    E_DB_MEDIA_LAST
} EDBMedia;

static const char * EDBMediaNames[] = 
{
	"DB_MEDIA_VIDEO",
	"DB_MEDIA_AUDIO",
	"DB_MEDIA_CONTENT",
	"DB_MEDIA_LAST"
};


//MM Incoming channel params
//Header Type
typedef enum
{
    E_MM_NO_HEADER,	
    E_MM_CARD_MANAGER_HEADER
} EMMIncomingChannelHeader;

////////////////////////////////////////////////////////////////////////////
///
///     CONSTANTS
///
////////////////////////////////////////////////////////////////////////////

const BYTE CHANNEL_TYPE_AUDIO 	= 	0x1;
const BYTE CHANNEL_TYPE_VIDEO 	= 	0x2;
const BYTE CHANNEL_TYPE_CONTENT = 	0x4;
const BYTE CHANNEL_TYPE_FECC	=	0x8;

////////////////////////////////////////////////////////////////////////////
///
///     EVENT OPCODES
///
////////////////////////////////////////////////////////////////////////////

// 10001 - 10100
const OPCODE LIST_EVENT_MESSAGE						= 10001;
const OPCODE ACTIVATE_EVENT_MESSAGE					= 10002;
const OPCODE VIDEO_OUT_PARAM_EVENT_MESSAGE			= 10003;
const OPCODE VIDEO_OUT_UPDATE_PARAM_EVENT_MESSAGE	= 10004;
const OPCODE VIDEO_OUT_RESET_EVENT_MESSAGE			= 10005;
const OPCODE AUDIO_OUT_RESET_EVENT_MESSAGE			= 10006;
const OPCODE VIDEO_IN_UPDATE_PIC_EVENT_MESSAGE		= 10007;
//const OPCODE CHANNEL_IN_PARAM_EVENT_MESSAGE			= 10008;
const OPCODE VIDEO_IN_PARAM_EVENT_MESSAGE			= 10009;


const OPCODE INIT_UDP_SOCKET_MSG 					= 10010;
const OPCODE SET_PARAM_UDP_SOCKET_MSG				= 10011;
const OPCODE SEND_DATA_TO_UDP_SOCKET_MSG 			= 10012;
const OPCODE RECV_DATA_FROM_UDP_SOCKET_MSG 			= 10013;
const OPCODE CLOSE_UDP_SOCKET_MSG 					= 10014;

const OPCODE START_WRITE_STEAM_UDP_SOCKET_MSG 		= 10015;
const OPCODE STOP_WRITE_STEAM_UDP_SOCKET_MSG 		= 10016;

//const OPCODE CONNECT_TX_SOCKET_MSG 	= 10015;
//const OPCODE SEND_DATA_TX_SOCKET_MSG	= 10016;
//const OPCODE DISCONNECT_TX_SOCKET_MSG	= 10017;

const OPCODE VIDEO_OUT_ENCODER_UPDATE_PARAM_EVENT_MESSAGE = 10018;


const OPCODE AUDIO_OUT_UPDATE_CHANNEL_EVENT_MESSAGE	= 100019;



////////////////////////////////////////////////////////////////////////////
///
///     STRUCTURES
///
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// Video Parameters
////////////////////////////////////////////////////////////////////////////
typedef struct
{
	APIS32					nVideoBitRate;
	EMMVideoResolution		eVideoResolution;
	EMMVideoFrameRate		eVideoFrameRate;
	EMMVideoProfileType		eVideoProfileType;	//High Profile (for H264 only)
} TVideoParam;

////////////////////////////////////////////////////////////////////////////
//monitoring ID end point struct
////////////////////////////////////////////////////////////////////////////

typedef struct 
{
	DWORD monitorPartyId;
	DWORD monitorConfId;
	CSegment* 	segment;
} TMonitoringIDEP;

////////////////////////////////////////////////////////////////////////////
//Event message struct
////////////////////////////////////////////////////////////////////////////

typedef struct 
{
	string		  	  str_event_opcode;
	OPCODE			  event_opcode;
	CMplMcmsProtocol* mcms_protocol;
} TEventMessage;

////////////////////////////////////////////////////////////////////////////
// Incoming channels params
////////////////////////////////////////////////////////////////////////////

typedef struct
{
	BYTE					bReadFlag;
	
	//packet sequence number
	BYTE					bCheckSeqNumber;
	BYTE					bIntraSeqNumber;
	
	//intra detection
	BYTE					bDetectIntra;
	
	//packet time stamp
	BYTE					bCheckTimeStamp;
	BYTE					bIntraTimeStamp;
	
	//encryption
	BYTE					bDecryptMedia;
	
	//write media to file
	BYTE					bWriteMedia;
	
	//Header type
	EMMIncomingChannelHeader	eHeaderType;
} TIncomingChannelParam;



/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// structures for CardManager-ART proprietary header >> used here to define media packets'
// beginning & end (the media recording files contain this header - junctions #51 & #91)
/////////////////////////////////////////////////////////////////////

typedef struct
{
	INT32 	enPacketSrcType;
	INT32	enMediaType;
	INT32 	nPartyId; 					// Unique number per port
	INT32 	nPayloadSize;				// Rtp (Header + Payload) size in bytes
	INT32 	nInternalSequenceNumber; 	// Packet numbering
	UINT32  unInternalTimestamp;
} TMediaHeaderCommon;

typedef struct
{
	INT32 	nRtpSequenceNumber; 	//no need to check	// DSP input. Recovery use
	INT32 	nUnitNumber; 			//no need to check	// DSP input. Recovery use
	UINT32 	unRtpTimestamp; 		//no need to check	// DSP input. Recovery use
	UINT32 	unSyncSource; 								// DSP input. Recovery use	
} TIpNetworkHeader;

typedef struct
{
	TMediaHeaderCommon	tCommonHeader;
	TIpNetworkHeader	tIpNetworkHeader;
} TMediaHeaderIpNetwork;


// RTP header
typedef struct
{
	UINT8	firstByte;
	UINT8	markerPlusPayloadType;
	UINT16	usSequenceNumber;
	INT32	unTimeStamp;
	UINT32	unSSRC;
} TRtpHeader;

//typedef struct
//{
// Uint16   usSequenceNumber; 
// UINT32    unTimeStamp; 
// Uint32    unSSRC; //synchronization source (SSRC) identifier   
// Bool      bMarker; 
// Uint8     ucPayloadType; 
// Bool      bPadding; 
// Uint32    unStartByte;
// Int32   nLength;
//} TRtpHeaderParams;


//////////////////////////////////
//// More Globals
//////////////////////////////////

// Incoming audio channel param flags
static TIncomingChannelParam g_tIncomingAudioChannelParam = 
		{FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,E_MM_CARD_MANAGER_HEADER};

// Incoming video channel param flags
static TIncomingChannelParam g_tIncomingVideoChannelParam = 
		{FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,E_MM_CARD_MANAGER_HEADER};

#endif /*MEDIAMNGR_H_*/


