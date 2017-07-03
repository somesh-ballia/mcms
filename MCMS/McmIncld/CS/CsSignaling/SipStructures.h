//SipStructures.h
//Uri Avni

#ifndef __SIP_STRUCTURES__
#define __SIP_STRUCTURES__


#include "IpAddressDefinitions.h"
#include "ChannelParams.h"
#include "SipDefinitions.h"

/*****Constants*****/
#define MAX_GROUP_ANAT_MEMBER_NUM 2 	//ANAT


////////////////////////////////////////////////////////////////////////
typedef enum  {

	eMediaLineTypeAudio,
	eMediaLineTypeVideo,
	eMediaLineTypeApplication,
	eMediaLineTypeNotSupported

} eMediaLineType;

/**
 * This following Enum is for all the unsupported media line types.
 * For example proprietary media line types.
 */
typedef enum {

	eMediaLineNotSupportedSubTypeNotRelevant,
	eMediaLineNotSupportedSubTypeControl

} eMediaLineNotSupportedSubType;


typedef enum  {

	eMediaLineSubTypeNull,
	eMediaLineSubTypeUnknown,
	eMediaLineSubTypeRtpAvp,
	eMediaLineSubTypeRtpSavp,
	eMediaLineSubTypeTcpBfcp,
	eMediaLineSubTypeTcpTlsBfcp,
	eMediaLineSubTypeUdpBfcp,
	eMediaLineSubTypeTcpRtpAvp,
	eMediaLineSubTypeTcpRtpSavp,	
	eMediaLineSubTypeTcpCfw
} eMediaLineSubType;

typedef enum  {
	// Keep the order of this enumeration!
	eMediaLineContentNull		= 0,
	eMediaLineContentUnknown	= 1,
	eMediaLineContentSlides		= 2,
	eMediaLineContentSl			= 3,
	eMediaLineContentAlt		= 4,
	eMediaLineContentSpeaker	= 5,
	eMediaLineContentMain		= 6,
	eMediaLineContentUserFloor	= 7
} eMediaLineContent;

typedef enum  {
	kMediaLineInternalTypeNone,	// must be the first
	kMediaLineInternalTypeAudio,
	kMediaLineInternalTypeVideo,
	kMediaLineInternalTypeFecc,
	kMediaLineInternalTypeContent,
	kMediaLineInternalTypeBfcp,
	kMediaLineInternalTypeNotSupported,
	kMediaLineInternalTypeLast	// must be the last
} eMediaLineInternalType;

typedef enum {
	eMediaLineIceRtp,
	eMediaLineIceRtcp,
	eMediaLineIceCandidate,
} eMediaLineIceAttr;

typedef enum {
	eSessionIcePwd,
	eSessionIceUfrag,
	eSessionIceOptions,
} eSessionIceAttr;

typedef enum {
	eMediaStreamNone,
	eMediaStream,
	eMediaStreamId,
} eMediaStreamAttr;

typedef struct{

	APIU32					numberOfIceAttr;
	APIU32					lenOfDynamicSection;

} sipIceAttrEntryBaseSt;

typedef struct{

	APIU32					numberOfIceAttr;
	APIU32					lenOfDynamicSection;
	APIS8 					iceAttr[1];

} sipIceAttrEntrySt;

typedef struct {
	APIU8					index;					// ice attribute index in sdp
	APIU8					type;					// eIceAttrType
	APIU16					attrLen;
	APIU32					lenOfDynamicSection;	// Aggregate of caps.

} sipIceAttrBaseSt;

typedef struct {
	APIU8					index;					// ice attribute index in sdp
	APIU8					type;					// eIceAttrType
	APIU16					attrLen;
	APIU32					lenOfDynamicSection;	// Aggregate of caps.
	APIS8 					data[1];
} sipIceAttrSt;

typedef struct {
	mcXmlTransportAddress	mediaIp;				// ip and port
 	APIU32 					rtcpPort;				// rtcp port
 	APIU8					intraFlag;				// intra
	APIU8					index;					// m-line index in sdp
	APIU8					type;					// eMediaLineType
	APIU8					subType;				// eMediaLineSubType
	APIU8					internalType;			// eMediaLineInternalType
	APIU32					notSupportedSubType;    // eMediaLineNotSupportedSubType
	APIU8					content;				// eMediaLineContent
	APIS8					label[32];				// Label string
	APIU32                  ssrcrange[2];           // start and end ssrc range	(in and out. For out check if not 0. If 0- not send attribute)
	APIU32                  msi;                    // MSI represents a contributing source (CSRC) from atribute a=x-source-streamid (in only)
	APIU32					rtcp_rsize;				// 1 for Lync 2013 and UP,0- all other
	APIU8                   mid;          			// added for ANAT
	APIU8 					midAnatGroup[MAX_GROUP_ANAT_MEMBER_NUM]; 	//added for ANAT, ENUM=3               
	APIU32					rssVideoLayout; //for eFeatureRssDialin -- 0: auto, 1: single view 1X1, 2: dual view, 1X2
	APIU32					numberOfCaps;			// numberof all caps in media line
	APIU32					lenOfDynamicSection;	// Aggregate of caps.

} sipMediaLineBaseSt;

typedef struct {
	mcXmlTransportAddress	mediaIp;				// ip and port
 	APIU32 					rtcpPort;				// rtcp port
 	APIU8					intraFlag;				// intra
 	APIU8					index;					// m-line index in sdp
	APIU8					type;					// eMediaLineType
	APIU8					subType;				// eMediaLineSubType
	APIU8					internalType;			// eMediaLineInternalType
	APIU32					notSupportedSubType;    // eMediaLineNotSupportedSubType
	APIU8					content;				// eMediaLineContent
	APIS8					label[32];				// Label string
	APIU32                  ssrcrange[2];           // start and end ssrc range	(in and out. For out check if not 0. If 0- not send attribute)
	APIU32                  msi;                    // MSI represents a contributing source (CSRC) from atribute a=x-source-streamid (in only)
	APIU32					rtcp_rsize;				// 1 for Lync 2013 and UP,0- all other
	APIU8                   mid;       				// added for ANAT
	APIU8 					midAnatGroup[MAX_GROUP_ANAT_MEMBER_NUM]; 	//added for ANAT, ENUM=3  
	APIU32					rssVideoLayout; //for eFeatureRssDialin -- 0: auto, 1: single view 1X1, 2: dual view, 1X2 
	APIU32					numberOfCaps;			// numberof all caps in media line
	APIU32					lenOfDynamicSection;	// Aggregate of caps.
	APIS8 					caps[1];

} sipMediaLineSt;

typedef struct{

	APIU32					bMainMediaIpPresent;	// Is Session level c-line present
	mcXmlTransportAddress	mainMediaIp;			// Session level c-line
	APIU32					numberOfMediaLines;
	APIU32					lenOfDynamicSection;

}sipMediaLinesEntryBaseSt;

typedef struct{

	APIU32					bMainMediaIpPresent;	// Is Session level c-line present
	mcXmlTransportAddress	mainMediaIp;			// Session level c-line
	APIU32					numberOfMediaLines;
	APIU32					lenOfDynamicSection;
	APIS8 					mediaLines[1];

}sipMediaLinesEntrySt;

typedef struct{
	APIS8                   cCname[CNAME_STRING_MAX_LEN +1];
	APIU32					callRate;
	APIU32                  msVideoRateTx;//in kbit unit max BW for Tx - for a=x-mediabw
    APIU32                  msVideoRateRx;//in kbit unit max BW for Rx - for a=x-mediabw
	APIU32                  msAudioRateTx;//in kbit unit max BW for Tx - for a=x-mediabw
    APIU32                  msAudioRateRx;//in kbit unit max BW for Rx - for a=x-mediabw
	APIU32					sipMediaLinesOffset;
	APIU32					sipMediaLinesLength;
	APIU32					sipHeadersOffset;
	APIU32					sipHeadersLength;
	APIU32					sipIceOffset;
	APIU32					sipIceLength;
	APIU32					rssConnectionType; //for eFeatureRssDialin -- 0: null, 1: recording, 2: playback	
	APIU32					lenOfDynamicSection;
}sipSdpAndHeadersBaseSt;

typedef struct sipSdpAndHeaders{
	APIS8                   cCname[CNAME_STRING_MAX_LEN +1];
	APIU32					callRate;
	APIU32                  msVideoRateTx;//in kbit unit max BW for Tx - for a=x-mediabw
    APIU32                  msVideoRateRx;//in kbit unit max BW for Rx - for a=x-mediabw
	APIU32                  msAudioRateTx;//in kbit unit max BW for Tx - for a=x-mediabw
    APIU32                  msAudioRateRx;//in kbit unit max BW for Rx - for a=x-mediabw
	APIU32					sipMediaLinesOffset;
	APIU32					sipMediaLinesLength;
	APIU32					sipHeadersOffset;
	APIU32					sipHeadersLength;
	APIU32					sipIceOffset;
	APIU32					sipIceLength;
	APIU32					rssConnectionType; //for eFeatureRssDialin -- 0: null, 1: recording, 2: playback
	APIU32					lenOfDynamicSection;
	APIS8					capsAndHeaders[1];


	//Here comes two dynamics sections - 1) media 2) sipHeaders + session params

	//Media section:
	//--------------
	//		-->	mediaIp;
	//			type;
	//			subType;
	//			content;
	//			label[32];
	//			numberOfCaps;
	//			lenOfDynamicSection;
	//			caps
	//			->(capBuffer*)	capTypeCode				  --
	//							sipPayloadType				|
	//							capLength					| * numberOfCaps
	//								ctCapStruct header		|
	//								other params		  --
	//
	//		-->	mediaIp;
	//			type;
	//			subType;
	//			content;
	//			label[32];
	//			numberOfCaps;
	//			lenOfDynamicSection;
	//			caps
	//			->(capBuffer*)	capTypeCode				  --
	//							sipPayloadType				|
	//							capLength					| * numberOfCaps
	//								ctCapStruct header		|
	//								other params		  --
	//
	//SipHeaders/session
	//------------------
	//		-->	numOfHeaders;
	//			headersListLength;
	//			->(sipMessageHeaders*)	eHeaderField; --
	//									filler;	   	    | * numOfHeaders
	//									position;	  --
	//									...
	//			HeadersFieldsList ...

}sipSdpAndHeadersSt;

////////////////////////////////////////////////////////////////////////
typedef struct sipContentAndHeadersBase{
	APIU32					sipHeadersOffset;		//contentLength = sipHeaders offset.
	APIU32					lenOfDynamicSection;	//Aggregate of content and sipHeaders.

}sipContentAndHeadersBaseSt;

typedef struct sipContentAndHeaders{
	APIU32					sipHeadersOffset;		//contentLength = sipHeaders offset.
	APIU32					lenOfDynamicSection;	//Aggregate of content and sipHeaders.
	APIS8					contentAndHeaders[1];

	//Here comes two dynamics sections - 1) content 2) sipHeaders + session params

	//Content section
	//---------------
	//		-->	Content	body
	//
	//SipHeaders/session
	//------------------
	//		-->	numOfHeaders;
	//			headersListLength;
	//			->(sipMessageHeaders*)	eHeaderField; --
	//									filler;	   	    | * numOfHeaders
	//									position;	  --
	//									...
	//			HeadersFieldsList ...

}sipContentAndHeadersSt;


//-S- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------//
typedef struct _sPART_MESSAGE_DECR
{
	unsigned int	dwFullMessLen;
	unsigned short	wPartLen      ;
	unsigned short  wCsId		  ;
	unsigned short  wOpcMessSeq   ;
	unsigned short  wPartSeq      ;
	unsigned short  wNumberOfParts;
	char            aReserve[2]   ;
 }
sPART_MESSAGE_DECR;

typedef struct sipContentAndHeaders_Part{
	APIU32					sipHeadersOffset;		//contentLength = sipHeaders offset.
    //----------------------------------------------//
    sPART_MESSAGE_DECR		sMessagePartDescr   ;
    //----------------------------------------------//
	APIU32					lenOfDynamicSection;	//Aggregate of content and sipHeaders.
	APIS8					contentAndHeaders[1];

	//Here comes two dynamics sections - 1) content 2) sipHeaders + session params

	//Content section
	//---------------
	//		-->	Content	body
	//
	//SipHeaders/session
	//------------------
	//		-->	numOfHeaders;
	//			headersListLength;
	//			->(sipMessageHeaders*)	eHeaderField; --
	//									filler;	   	    | * numOfHeaders
	//									position;	  --
	//									...
	//			HeadersFieldsList ...

}sipContentAndHeadersSt_Part;
//-E- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------//
#endif //__SIP_STRUCTURES__
