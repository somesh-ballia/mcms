// file ConfMultimediaControlDefinitions.h
// Written by Mikhail Karasik

#ifndef __CONFERENCE_MULTIMEDIA_CONTROL_DEFINITIONS__
#define __CONFERENCE_MULTIMEDIA_CONTROL_DEFINITIONS__

//Includes
//------------


//Defines
//------------

//===================================================================================
// Conference Request definitions
// ===================================================================================

typedef enum{
	
	terminalListRequest,		//-- same as H.230 TCU (term->MC)

	makeMeChairRequest,				//-- same as H.230 CCA (term->MC)
	cancelMakeMeChairRequest,			//-- same as H.230 CIS (term->MC)

	dropTerminalRequest,				//-- same as H.230 CCD(term->MC)

	requestTerminalIDRequest,			//-- sames as TCP (term->MC)

	enterH243PasswordRequest,			//-- same as H.230 TCS1 (MC->term)
	enterH243TerminalIDRequest,		//-- same as H.230 TCS2/TCI (MC->term)

	enterH243ConferenceIDRequest,		//-- same as H.230 TCS3 (MC->term)

	enterExtensionAddressRequest,		//-- same as H.230 TCS4 (GW->term)
	requestChairTokenOwnerRequest,		//-- same as H.230 TCA (term->MC)
	requestTerminalCertificateRequest,

	broadcastMyLogicalChannelRequest,	//-- similar to H.230 MCV
	makeTerminalBroadcasterRequest,	//-- similar to H.230 VCB
	sendThisSourceRequest,				//-- similar to H.230 VCS
	requestAllTerminalIDsRequest,
	
	remoteMCRequestMasterActivateRequest,
	remoteMCRequestSlaveActivateRequest,
	remoteMCRequestDeActivateRequest,

}ConferenceRequestEnum;

typedef struct{	
	int					ConReqOpcode; // from ConferenceResponseEnum
	APIU32				val1;
	APIU32				val2;
}genConferenceRequestStruct;

//===================================================================================
// Conference Response definitions
//===================================================================================

typedef enum{
	mCTerminalIDResponse,		//-- response to TCP(same as TIP) sent by MC only
	terminalIDResponse,			//-- response to TCS2 or TCI

	conferenceIDResponse,		//-- response to TCS3

	passwordResponse,			//-- response to TCS1

	terminalListResponse,

	videoCommandRejectResponse,			//-- same as H.230 VCR
	terminalDropRejectResponse,			//-- same as H.230 CIR

	grantedChairTokenResponse,			//-- same as H.230 CIT
	deniedChairTokenResponse,			//-- same as H.230 CCR

	extensionAddressResponse,	//-- response to TCS4
	chairTokenOwnerResponse,	//-- response to TCA(same as TIR) sent by MC only
	terminalCertificateResponse,
	
	grantedBroadcastMyLogicalChannelResponse,	// MVA
	deniedBroadcastMyLogicalChannelResponse,	// MVR

	grantedMakeTerminalBroadcasterResponse,
	deniedMakeTerminalBroadcasterResponse,

	grantedSendThisSourceResponse,
	deniedSendThisSourceResponse,

	requestAllTerminalIDsResponse,
	
	remoteMCResponseAccept,
	remoteMCResponseUnspecifiedReject,
	remoteMCResponseFunctionNotSupportedReject,
}ConferenceResponseEnum;

typedef struct{	
	int					ConResOpcode;	// from ConferenceResponseEnum
	APIU32				val1;
	APIU32				val2;
	char				strID[128];
}genConferenceResponseStruct;
//===================================================================================
// Conference Commands definitions
//===================================================================================

typedef enum{
	broadcastMyLogicalChannelCommand,			//-- similar to H.230 MCV
	cancelBroadcastMyLogicalChannelCommand,		//-- similar to H.230 Cancel-MCV

	makeTerminalBroadcasterCommand,				//-- same as H.230 VCB
	cancelMakeTerminalBroadcasterCommand,		//-- same as H.230 Cancel-VCB
		
	sendThisSourceCommand,						//-- same as H.230 VCS
	cancelSendThisSourceCommand,				//-- same as H.230 cancel VCS

	dropConferenceCommand,						//-- same as H.230 CCK
	substituteConferenceIDCommand
}ConferenceCommandEnum;

typedef struct{
	int					ConComOpcode;	//from ConferenceCommandEnum
	APIU32				val1;
	APIU32				val2;
	char				conID[16];
}genConferenceCommandStruct;
//===================================================================================
// Conference Indication definitions
//===================================================================================

typedef enum{
	sbeNumber,					//-- same as H.230 SBE Number
		
	terminalNumberAssignIndication,		//-- same as H.230 TIA

	terminalJoinedConferenceIndication,	//-- same as H.230 TIN

	terminalLeftConferenceIndication,		//-- same as H.230 TID

	seenByAtLeastOneOtherIndication,		//-- same as H.230 MIV
	cancelSeenByAtLeastOneOtherIndication,//-- same as H.230 cancel MIV

	seenByAllIndication,					//-- like H.230 MIV
	cancelSeenByAllIndication,			//-- like H.230 MIV

	terminalYouAreSeeingIndication,		//-- same as H.230 VIN

	requestForFloorIndication,			//-- same as H.230 TIF

	withdrawChairTokenIndication,			//-- same as H.230 CCR-- MC-> chair
					
	floorRequestedIndication,				//-- same as H.230 TIF-- MC-> chair
					
	terminalYouAreSeeingInSubPictureNumberIndication,
	videoIndicateComposeIndication

}ConferenceIndicationEnum;

typedef struct{
	int					ConIndOpcode;	//from ConferenceIndicationEnum
	APIU32				val1;
	APIU32				val2;
}genConferenceIndicationStruct;


//===================================================================================
// Conference/Party RSS non standard Command request enum.
// ===================================================================================

typedef enum{
	eRssCmdExchangeID,	
	eRssCmdLiveStream 
	
}eRSSCommandRequestEnum;

#endif	//__CONFERENCE_MULTIMEDIA_CONTROL_DEFINITIONS__
