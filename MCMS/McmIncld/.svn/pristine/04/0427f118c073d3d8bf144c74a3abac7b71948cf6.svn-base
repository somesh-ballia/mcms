//	SipHeadersList.h

#ifndef	__SIP_HEADERS_LIST__
#define	__SIP_HEADERS_LIST__



//	Macros:
//---------
#define MaxSipHeadersListSize	2048

// flags:
//-------
#define HlResetMask		0x00	// 0000 0000
#define HlHeaderTaken	0x80	// 1000 0000

// Types:
//-------

typedef char*	(*sourcePtrRoutinePtr)(APIS8, void*);
typedef int		(*numSourcePtrRoutinePtr)(APIS8, void*);

typedef enum {

	//Sip headers fields

	kFirstField	= 0,
	kToDisplay,
	kTo,
	kToTag,
	kFromDisplay,
	kFrom,
	kFromTag,
	kContactDisplay,
	kContact,
	kContactRendering,
	kMsDiagnostics,
	kPlcmDiagnostics,
	kVia,
	kReqLine,
	kUserAgent,
	kAllowEvent,
	kRosterManager,		//Propriety for messenger
	kEndPoints,			//Propriety for messenger
	kProprietyHeader,	//Any new header. Value format for these headers must be "name: value".
	kPrivateHeader,		//P-xxx headers.
	kReferredBy,
	kWarning,
	kCallId,
	kEvent,
	kAccept,
	kReferTo,
	kSupported,
	kSubscrpState,
	kAuthorization,
	kAllow,
	kRequire,
	kContentType,
	kMinSe,
	kSessionExpires,
	kRetryAfter,
	kMsConversationId,
	kMsKeepAliveTimeout,  //SIP-Header: ms-keep-alive:....  Note: CS sends to MCMS parameter "timeout" as string.

	//svc
	kMrd,

	// Ice Xml Body
	kMrasUri,
	kCredUser,
	kCredPass,
	kCredDuration,
	kMRelayLocation,
	kMRelayHostName,
	kMRelayUdpPort,
	kMRelayTcpPort,
	kEnableBWPolicyCheck,
    kUcMaxVideoRateAllowed,

    // SIMPLE Pres
	kAclListDelta,
	kContactListDelta,

	//Self presence
	kRichPresence,
	
	//WebRTC
	kWebRtcSdp,

	//SDP headers fields

	kSdpSession_v,		//version
	kSdpSession_o,		//owner/creator
	kSdpSession_o_display, //owner/creator
	kSdpSession_o_version, //owner/version change y/n
	kSdpSession_s,		//session name
	kSdpSession_i,		//session info
	kSdpSession_t,		//time the session is active
	kSdpSession_u,		//uri of description
	kSdpSession_e,		//email
	kSdpSession_b_AS,	//call rate (App Specific Maximum).
	kSdpSession_b_CT,	//call rate (Conf Total).
	kSdpSession_b_X,	//call rate (extention).
	kSdpSession_b_Default, //call rate with no prefix.
	kTargetDialog, 		//for correlate between subscribe and invite in MRX env (RFC-4538)
	kServer,          	// probably only from server 2013
	kClick2Conf,		// virtual header given from To header for Click 2 conf feature
	enLastField,
} enHeaderField;

#define HdrLstTerminator	enLastField

#define propHdrUserName		"UserName: "
#define propHdrUserPassword	"UserPassword: "


#define OCS_MEDIA_RELAY_SERV_PACKAGE 	"application/msrtc-media-relay-auth+xml"
#define OCS_PROV_CONFIGURATION_PACKAGE 	"application/vnd-microsoft-roaming-provisioning-v2+xml"
#define OCS_PROV_EVENT_V2	 "vnd-microsoft-provisioning-v2"
#define OCS_PROV_SUPPORTED 	"ms-piggyback-first-notify"


// Headers list structs

typedef struct{
	APIS8	eHeaderField;	//EHeaderField
	APIU8	flags;
	APIU16	position;
} sipHeaderElement;

typedef struct{
	int					numOfHeaders;
	int					headersListLength;
} sipMessageHeadersBase;

typedef struct{
	int					numOfHeaders;
	int					headersListLength;
	char				headersList[1];		//list of numOfHeaders * sipHeaderElement
} sipMessageHeaders;

// struct to work with list
typedef struct {
	int						curLength;
	int						maxLength;
	sipMessageHeadersBase	*pList;
	char					*pCur;
} sipListSt;

typedef struct  {
	int						numMan;			// number of requested mandatory headers
	int						numOpt;			// number of requested optional  headers
	APIS8					*pManReq;		// list of requested mandatory headers
	APIS8					*pOptReq;		// list of requested optional headers

	numSourcePtrRoutinePtr	getNumSourceFunc;	// function to get number of headers
	sourcePtrRoutinePtr		getSourceFunc;	// function to get header
	void					*pGen;
	int						filler;
}sipListInitParams;


// Routines:
//----------

//---------------------------------------------------------------------------
//
//	Function name:	hlInitNewList
//
//	Description:	Initialize new list
//
//	Return code:
//			0					- success
//			negative value		- error
//
//---------------------------------------------------------------------------
int hlInitNewList(
	sipListSt	*pListSt,
	char		*pBuffer,
	int			maxLength);

//---------------------------------------------------------------------------
//
//	Function name:	hlBuildList
//
//	Description:	Build list appropriate to requested params
//
//
//	Return code:
//			0					- success
//			negative value		- error
//
//---------------------------------------------------------------------------
int hlBuildList(
	sipListSt			*pListSt,
	sipListInitParams	*pParams);

int hlAddToList(
	sipListSt			*pListSt,
	enHeaderField		field,
	char				*pSrc,
	int					len);

char *hlGetElemOrdered(
	sipListSt			*pListSt,
	APIS8				code,
	int					order);

char *hlGetElemOrderedEx(
	sipListSt			*pListSt,
	INT8				code,
	int					order,
	int					*pLength);
#endif//__SIP_HEADERS_LIST__
