//SipDefinitions.h
//Uri Avni
#ifndef __SIP_DEFINITIONS__
#define __SIP_DEFINITIONS__


#define SIP_MAX_MSG_HEADERS				32 
#define SIP_MAX_ADDRESSES_PER_EP		4
#define SIP_MAX_CAPS_PER_EP				32

//#define NUM_OF_IPS		  				3

#define MaxLengthOfSipMethosName		16
#define MaxLengthOfNonSdpContent		3072
#define MaxLengthOfNonSdpContentType	64
#define MaxLengthOfSingleUrl			128
#define MaxLengthOfSipCallId			128
#define MaxLengthOfUserName				32
#define MaxLengthOfDigits				32
#define SipRetransmissionsAmount		2	//2 ==> timeout of 35 sec
#define SipPostponeTimeoutDelta			15
#define MaxLengthOfMrasUri				256
#define MAXLengthOfConfPwd			32


typedef enum{	//This enum is a copy of en_StatusCode that defines in file sipstruct.h
				//This allows access to mcms code.

	SipCodesTrying 				= 100,
	SipCodesRinging 			= 180,
	SipCodesForwarded 			= 181,
	SipCodesQueued				= 182,
	SipCodesSessionProgress		= 183,
	SipCodesOk					= 200,
	SipCodesAccepted			= 202,
	SipCodesMultiChoice			= 300,
	SipCodesMovedPerm			= 301,
	SipCodesMovedTemp			= 302,
	SipCodesSeeOther			= 303,
	SipCodesUseProxy			= 305,
	SipCodesAltService			= 380,
	SipCodesBadRequest			= 400,
	SipCodesUnauthorized		= 401,
	SipCodesPaymentReq			= 402,
	SipCodesForbidden			= 403,
	SipCodesNotFound			= 404,
	SipCodesMethNotAllowed		= 405,
	SipCodesNotAcceptable		= 406,
	SipCodesProxyAuthReq		= 407,
	SipCodesRequestTimeout		= 408,
	SipCodesConflict			= 409,
	SipCodesGone				= 410,
	SipCodesLengthReq			= 411,
	SipCodesReqEntTooLarge		= 413,
	SipCodesReqUriTooLarge		= 414,
	SipCodesUnsuppMediaType		= 415,
	SipCodesBadExtension		= 420,
	SipCodesIntervalTooSmall	= 422,
	SipCodesTempNotAvail		= 480,
	SipCodesCallNotExist		= 481,
	SipCodesLoopDetected		= 482,
	SipCodesTooManyHops			= 483,
	SipCodesAddressIncomp		= 484,
	SipCodesAmbiguous			= 485,
	SipCodesBusyHere			= 486,
	SipCodesRequestTerminated	= 487,
	SipCodesNotAcceptedInHere	= 488,
	SipCodesRequestPending		= 491,
	SipCodesInternalSrvErr		= 500,
	SipCodesNotImplemented		= 501,
	SipCodesBadGateway			= 502,
	SipCodesServiceUnavail		= 503,
	SipCodesGwTimeout			= 504,
	SipCodesSipVerNotSupp		= 505,
	SipCodesBusyEveryWhere		= 600,
	SipCodesDecline				= 603,
	SipCodesDoesNotExist		= 604,
	SipCodesGlobNotAcceptable	= 606,
	SipCodesSipUnknownStatus	= 2000

}enSipCodes;

typedef enum{	
	SipWarningIncompNetProtocol		= 300,
	SipWarningIncompNetAddrFormats	= 301,
	SipWarningIncompTransProtocol	= 302,
	SipWarningIncompBandwidthUnits	= 303,
	SipWarningMediaTypeNotAvail		= 304,
	SipWarningIncompMediaFormat		= 305,
	SipWarningAttrNotUnderstood		= 306,
	SipWarningSdpParamNotUnderstood = 307,
	SipWarningMulticastNotAvail		= 330,
	SipWarningUnicastNotAvail		= 331,
	SipWarningInsufficientBandwidth = 370,
	SipWarningMiscellaneous			= 399,
	SipWarningUnknown				= 2000
}enSipWarningCodes;

typedef enum {
	kInactive			= 0,					
	kRecvOnly			= 1,				
	kSendOnly			= 2,			
	kSendRecv			= 3,	
} enMediaDirection;

typedef enum { 
	kSdpAudio = 1,	// cmCapAudio
	kSdpVideo,		// cmCapVideo
	kSdpData,
//	kSdpApplication,
//	kSdpControl,
	kSdpGeneric	= 9,
	kSdpLastType,
}enSdpMediaType;

#define GetSdpArrayIndex(x) (x-1)  //Used to get indexes from enSdpMediaType

#define MaxCapsPerMedia 16

typedef enum
{
	SipTransportErrorClose	 = 1,
	SipTransportErrorDelete	 = 2

} sipTransportErrorExpectedReq;


// The enum used as the value of the cardDataCodes in tmDsIndSipSipCardData 
// struct for the opcode SIP_CARD_DATA_IND, this opcode is used to update 
// the MCMS about flags and data that were initialized in the IP-card.	
typedef enum
{
	SipCardStart=0,
	SipNortel
	// later more Card Data information will be added to this list
} enCardDataCodes;		

// Type used for value of the  SipNortel flag.
typedef enum
{
    Nortel_No = 0,
    Nortel_Yes
} enSipNortelValueType;

typedef enum
{
	GeneralInfoNull				= 0,
	VideoFastUpdate				= 1,
	VideoFastUpdate_v2 			= 2,
	FlowControl					= 3,
	LprChangeMode				= 4,
	PlcmIvrService				= 5,
	XmlTransMode        		= 6, //raw xml msg, for future use
	VideoFastUpdateV2StreamId 	= 7,
	CcsPluginConfPwd			= 8,
	CcsPluginConfPwdResult		= 9,
	VideoFastUpdateSametime		= 10,	
	//for eFeatureRssDialin -- SRS: Session Recording Server 
	SrsCommand				=11,  //MCU to SRS
	SrsIndication				=12,  //SRS to MCU
	SrsVideoLayout			=13	  //MCU to SRS
	
} enGeneralInfoOpcode;

typedef enum
{
	mainType			= 1,
	slideType			= 2
	
} enVideoType;

typedef enum
{
	GeneralSubscribeNull 				= 0,
	SubscribeOCSServerConfiguration		= 1,	//"vnd-microsoft-provisioning-v2"
	SubscribeOCSPresenceRoamingContacts	= 2, 	//"application/vnd-microsoft-roaming-contacts"
	SubscribeOCSPresenceAcl			= 3, 	//"application/vnd-microsoft-roaming-acls+xml"
	SubscribeLyncRoamingSelf		= 4,	//"application/vnd-microsoft-roaming-self+xml" - CT
	SubscribeLyncEventPackage               = 5     // subscribe for event package
} enGeneralSubscribeOpcode;

typedef enum
{
	GeneralServiceNull		= 0,
	ServiceEdgeConfiguration	= 1,
	ServiceSetPresenceUri		= 2,
	ServiceSetPresenceAce 		= 3,
	ServiceSetPresenceRich		= 4,
	ServiceUnsetPresenceRich	= 5,
	ServiceSetMembers		= 6,
	ServiceSetPresenceOnlineRich	= 7,
	ServiceSetPresenceBusyRich	= 8,
		
} enGeneralServiceOpcode;

typedef enum
{
	GeneralRegisterNull 				= 0,
	RegisterConf						= 1,	// register regular conference
	RegisterIce							= 2, 	// register Ice user
} enGeneralRegisterOpcode;

#endif //__SIP_DEFINITIONS__
