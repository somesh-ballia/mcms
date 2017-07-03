// CtMsgPhFormat.cpp
// Anat Elia

// Includes files
//----------------
#include "XmlDefs.h"
#include "XmlFormat.h"


// Macros
//--------

#undef	SRC
#define SRC	"CtMsgPhFormat"

//	Static variables
//-------------------

// Global variables
//------------------

// External variables
//------------------

//	External Routines:
//--------------------
extern genXmlFormat commonHeaderFormat[];
extern genXmlFormat mcIndGetPortFormat[];
extern genXmlFormat mcIndCallOfferingFormat[];
extern genXmlFormat mcIndCallConnectedFormat[];
extern genXmlFormat mcIndCallNewRateFormat[];
extern genXmlFormat mcIndCapabilitiesFormat[];
extern genXmlFormat mcIndCallControlConnectedFormat[];
extern genXmlFormat mcIndIncomingChannelFormat[];
extern genXmlFormat mcIndIncomingChannelConnectedFormat[];
extern genXmlFormat mcIndOutgoingChannelResponseFormat[];
extern genXmlFormat mcIndChannelNewRateFormat[];
extern genXmlFormat mcIndStartChannelCloseFormat[];
extern genXmlFormat mcIndChannelClosedFormat[];
extern genXmlFormat mcIndChannelMaxSkewFormat[];
extern genXmlFormat mcIndFlowControlIndicationFormat[];
extern genXmlFormat mcIndDBC2CommandFormat[];
extern genXmlFormat mcIndBadSpontanFormat[];
extern genXmlFormat mcIndAuthenticationFormat[];
extern genXmlFormat mcIndRoleTokenFormat[];
extern genXmlFormat mcIndDtmfBuffFormat[];
extern genXmlFormat mcIndKeepAliveBuffFormat[];
extern genXmlFormat mcIndFacilityBuffFormat[];
extern genXmlFormat mcIndLPRModeChangeFormat[];
extern genXmlFormat mcIndNewITPSpeakerFormat[];
//-----------------------------------------------------------------
extern genXmlFormat mcReqGetPortFormat[];
extern genXmlFormat mcReqReleasePortFormat[];
extern genXmlFormat mcReqCallSetupFormat[];
extern genXmlFormat mcReqCallAnswerFormat[];
extern genXmlFormat mcReqCreateControlFormat[];
extern genXmlFormat mcReqIncomingChannelResponseFormat[];
extern genXmlFormat mcReqOutgoingChannelFormat[];
extern genXmlFormat mcReqChannelNewRateFormat[];
extern genXmlFormat mcReqChannelMaxSkewFormat[];
extern genXmlFormat mcReqChannelOffFormat[];
extern genXmlFormat mcReqMultipointModeComTerminalIDMessageFormat[];
extern genXmlFormat mcReqChannelOnFormat[];
extern genXmlFormat mcReqRoleTokenMessageFormat[];
extern genXmlFormat mcReqUnexpectedMessageBaseFormat[];
extern genXmlFormat mcReqCallDropTimerExpiredFormat[];
extern genXmlFormat mcReqChannelDropFormat[];
extern genXmlFormat mcReqCallDropFormat[];
extern genXmlFormat mcReqDBC2CommandFormat[];
extern genXmlFormat mcReqDtmfBuffFormat[];
extern genXmlFormat mcReqAuthenticationFormat[];
extern genXmlFormat mcReqRoundTripDelayFormat[];
extern genXmlFormat mcNonStandardStFormat[];
extern genXmlFormat genConferenceCommandStructFormat[];
extern genXmlFormat genConferenceResponseStructFormat[];
extern genXmlFormat genConferenceIndicationStructFormat[];
extern genXmlFormat genConferenceRequestStructFormat[];
extern genXmlFormat mcReqFacilityFormat[];
extern genXmlFormat mcReqLPRModeChangeFormat[];
extern genXmlFormat mcReqNewITPSpeakerFormat[];

// Forward Declarations:
//----------------------


// Routines:
//----------
//H323_CS_SIG_GET_PORT_IND
//typedef struct {
//	cntlHeaderSt				csHdrs;
//	mcIndGetPort				ctMsg;
//
//} ctDsIndGetPort;

genXmlFormat ctDsIndGetPortFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_SIG_GET_PORT_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndGetPortFormat,0,0,0}
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CALL_OFFERING_IND
//typedef struct  {	
//	cntlHeaderSt				csHdrs;
//	mcIndCallOffering			ctMsg;
//	
//} ctDsIndCallOffering;

genXmlFormat ctDsIndCallOfferingFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CALL_OFFERING_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndCallOfferingFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CALL_CONNECTED_IND
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcIndCallConnected			ctMsg;
//
//} ctDsIndCallConnected;

genXmlFormat ctDsIndCallConnectedFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CALL_CONNECTED_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndCallConnectedFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CALL_NEW_RATE_IND
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcIndCallNewRate			ctMsg;
//
//} ctDsIndCallNewRate;

genXmlFormat ctDsIndCallNewRateFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CALL_NEW_RATE_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndCallNewRateFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------


//H323_CS_SIG_CAPABILITIES_IND
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcIndCapabilities			ctMsg;
//
//} ctDsIndCapabilities;

genXmlFormat ctDsIndCapabilitiesFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CAPABILITIES_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndCapabilitiesFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CAP_RESPONSE_IND;
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//} ctDsIndCapResponse;

genXmlFormat ctDsIndCapResponseFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CAP_RESPONSE_IND",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------


//H323_CS_SIG_CALL_CNTL_CONNECTED_IND
//typedef struct  {
//	cntlHeaderSt					csHdrs;
//	mcIndCallControlConnected		ctMsg;
//
//} ctDsIndCallControlConnected;
//
genXmlFormat ctDsIndCallControlConnectedFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CALL_CNTL_CONNECTED_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndCallControlConnectedFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_INCOMING_CHANNEL_IND
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcIndIncomingChannel		ctMsg;
//		
//} ctDsIndIncomingChannel;

genXmlFormat ctDsIndIncomingChannelFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_INCOMING_CHANNEL_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndIncomingChannelFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_INCOMING_CHNL_CONNECTED_IND
//typedef struct  {
//	cntlHeaderSt					csHdrs;
//	mcIndIncomingChannelConnected	ctMsg;
//
//} ctDsIndIncomingChannelConnected;

genXmlFormat ctDsIndIncomingChannelConnectedFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_INCOMING_CHNL_CONNECTED_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndIncomingChannelConnectedFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND
//typedef struct  {
//	cntlHeaderSt					csHdrs;
//	mcIndOutgoingChannelResponse	ctMsg;
//
//} ctDsIndOutgoingChannelResponse; 

genXmlFormat ctDsIndOutgoingChannelResponseFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndOutgoingChannelResponseFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CHAN_NEW_RATE_IND
//typedef struct  {
//	cntlHeaderSt			csHdrs;
//	mcIndChannelNewRate		ctMsg;
//
//} ctDsIndChannelNewRate;

genXmlFormat ctDsIndChannelNewRateFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CHAN_NEW_RATE_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndChannelNewRateFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_START_CHANNEL_CLOSE_IND
//typedef struct  {
//	cntlHeaderSt			csHdrs;
//	mcIndStartChannelClose	ctMsg;
//
//} ctDsIndStartChannelClose; 

genXmlFormat ctDsIndStartChannelCloseFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_START_CHANNEL_CLOSE_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndStartChannelCloseFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CHANNEL_CLOSE_IND
//typedef struct  {
//	cntlHeaderSt			csHdrs;
//	mcIndChannelClosed		ctMsg;
//
//} ctDsIndChannelClosed;

genXmlFormat ctDsIndChannelClosedFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CHANNEL_CLOSE_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndChannelClosedFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CHANNEL_OFF_IND
//typedef struct  {
//	cntlHeaderSt			csHdrs;
//	
//} ctDsIndChannelOff;

genXmlFormat ctDsIndChannelOffFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CHANNEL_OFF_IND",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CHANNEL_ON_IND
//typedef struct  {
//	cntlHeaderSt			csHdrs;
//	
//} ctDsIndChannelOn;

genXmlFormat ctDsIndChannelOnFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CHANNEL_ON_IND",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CHAN_MAX_SKEW_IND
//typedef struct  {
//	cntlHeaderSt			csHdrs;
//	mcIndChannelMaxSkew     ctMsg;
//} ctDsIndChannelMaxSkew;

genXmlFormat ctDsIndChannelMaxSkewFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CHAN_MAX_SKEW_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndChannelMaxSkewFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_FLOW_CONTROL_IND_IND
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcIndFlowControlIndication	ctMsg;
//
//} ctDsIndFlowControlIndication;

genXmlFormat ctDsIndFlowControlIndicationFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_FLOW_CONTROL_IND_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndFlowControlIndicationFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

// H232_CS_SIG_DBC2_COMMAND_CT_ON_IND
// H323_CS_SIG_DBC2_COMMAND_CT_OFF_IND
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcIndDBC2Command			ctMsg;
//
//} ctDsIndDBC2Command;

genXmlFormat ctDsIndDBC2CommandFormat[] = {
	{rootElemType,	tagVarType,	"H232_CS_SIG_DBC2_COMMAND_CT_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndDBC2CommandFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CALL_IDLE_IND(Empty)
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	
//} ctDsIndCallIdle;

genXmlFormat ctDsIndCallIdleFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CALL_IDLE_IND",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};

//-------------------------------------------------------------
// H323_CS_PARTY_KEEP_ALIVE_IND
//typedef struct {
//} mcIndKeepAlive;
genXmlFormat mcIndKeepAliveBuffFormat[] = {
	{parentElemType,	tagVarType,	"H323_CS_PARTY_KEEP_ALIVE_IND", 1,	0,0,0,0},
	{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
	
};

//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CALL_BAD_SPONTAN_IND
//typedef struct {
//	cntlHeaderSt				csHdrs;
//	mcIndBadSpontan				ctMsg;
//
//} ctDsIndBadSpontan;

genXmlFormat ctDsIndBadSpontanFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CALL_BAD_SPONTAN_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndBadSpontanFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CALL_AUTHENTICATION_IND
//typedef struct {
//	cntlHeaderSt				csHdrs;
//	mcIndAuthentication			ctMsg;
//   
//} ctDsIndAuthentication;

genXmlFormat ctDsIndAuthenticationFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CALL_AUTHENTICATION_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndAuthenticationFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CALL_ROLE_TOKEN_IND
//typedef struct {
//	cntlHeaderSt				csHdrs;
//	mcIndRoleToken				ctMsg;
//
//} ctDsIndRoleToken;

genXmlFormat ctDsIndRoleTokenFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CALL_ROLE_TOKEN_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndRoleTokenFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//IP_CS_VIDEO_UPDATE_PIC_IND   
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//} ctDsIndVideoUpdatePicture;

genXmlFormat ctDsIndVideoUpdatePictureFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_VIDEO_UPDATE_PIC_IND",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};

// H323_CS_SIG_NON_STANDARD_REQ_IND
// H323_CS_SIG_NON_STANDARD_COM_IND
// H323_CS_SIG_NON_STANDARD_RES_IND
// H323_CS_SIG_NON_STANDARD_IND_IND
//typedef struct	{
//	cntlHeaderSt			csHdrs;
//	mcNonStandardSt			ctMsg;
//
//} ctDsIndNonStandardMsg;

genXmlFormat ctDsIndNonStandardMsgFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_NON_STANDARD_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcNonStandardStFormat,0,0,0},
};

//-------------------------------------------------------------------------------------------------
// H323_CS_DTMF_INPUT_IND
//typedef struct	{
//	cntlHeaderSt			csHdrs;
//	mcIndDtmfBuff			ctMsg;

//} ctDsIndDtmfBuff;

genXmlFormat ctDsIndDtmfBuffFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_DTMF_INPUT_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndDtmfBuffFormat,0,0,0},
};

// H323_CS_PARTY_KEEP_ALIVE_IND
//typedef struct	{
//	cntlHeaderSt			csHdrs;
//
//} ctDsIndKeepAlive;

genXmlFormat ctDsIndKeepAliveBuffFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_PARTY_KEEP_ALIVE_IND",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};

// H323_CS_FACILITY_IND
//typedef struct {
//	h460AvayaFeVndrIndSt    		 avfFeVndIdInd;
//	h460AvayaFeMaxNonAudioBitRateInd avfFeMaxNonAudioBitRateInd;
//
//} mcIndFacility;
genXmlFormat ctDsIndFacilityBuffFormat[] = {
		{rootElemType,	tagVarType,	"H323_CS_FACILITY_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndFacilityBuffFormat,0,0,0},
	
};


//H323_CS_SIG_CONFERENCE_IND_IND
//typedef struct {
//	cntlHeaderSt					csHdrs;
//	genConferenceIndicationStruct   ctMsg;
//
//} mcDsReqConferenceInd;

genXmlFormat ctDsIndConferenceIndFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CONFERENCE_IND_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &genConferenceIndicationStructFormat,0,0,0},
};

// H323_CS_SIG_LPR_MODE_CHANGE_IND
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcIndLPRModeChange			ctMsg;
//
//}; ctDsIndLprModeChange
genXmlFormat ctDsIndLprModeChangeFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_LPR_MODE_CHANGE_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndLPRModeChangeFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------


// H323_CS_SIG_LPR_MODE_CHANGE_RES_IND
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	
//} ctDsIndLprModeChangeRes;
genXmlFormat ctDsIndLprModeChangeResFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_LPR_MODE_CHANGE_RES_IND",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};

//added by Jason for ITP-Multiple channels begin
// H323_CS_SIG_NEW_ITP_SPEAKER_IND
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcIndNewITPSpeaker		ctMsg;
//
//} ctDsIndNewITPSpeaker;
genXmlFormat ctDsIndNewITPSpeakerFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_NEW_ITP_SPEAKER_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcIndNewITPSpeakerFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------


// H323_CS_SIG_NEW_ITP_SPEAKER_ACK_IND
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	
//} ctDsIndNewITPSpeakerAck;
genXmlFormat ctDsIndNewITPSpeakerAckFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_NEW_ITP_SPEAKER_ACK_IND",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------
//added by Jason for ITP-Multiple channels end

//*****************************************************************
//*****************************************************************
// CallTask requests:
//*****************************************************************
//*****************************************************************

//H323_CS_SIG_GET_PORT_REQ
//typedef struct  {
//	cntlHeaderSt			csHdrs;
//	mcReqGetPort			ctMsg;
//	
//} mcDsReqGetPort;

genXmlFormat mcDsReqGetPortFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_GET_PORT_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqGetPortFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------
//H323_CS_SIG_RELEASE_PORT_REQ 
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcReqReleasePort			ctMsg;
//
//} mcDsReqReleasePort;

genXmlFormat mcDsReqReleasePortFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_RELEASE_PORT_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqReleasePortFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CALL_SETUP_REQ
//typedef struct {
//	cntlHeaderSt				csHdrs;
//	mcReqCallSetup				ctMsg;
//
//} mcDsReqCallSetup;
 
genXmlFormat mcDsReqCallSetupFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CALL_SETUP_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqCallSetupFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CALL_ANSWER_REQ
//typedef struct {
//	cntlHeaderSt				csHdrs;
//	mcReqCallAnswer				ctMsg;
//
//} mcDsReqCallAnswer;

genXmlFormat mcDsReqCallAnswerFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CALL_ANSWER_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqCallAnswerFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

// H323_CALL_CLOSE_CONFIRM_REQ
//typedef struct {
//	cntlHeaderSt				csHdrs;
//
//} mcDsReqCallCloseConfirm;

genXmlFormat mcDsReqCallCloseConfirmFormat[] = {
	{rootElemType,	tagVarType,	"H323_CALL_CLOSE_CONFIRM_REQ",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------


// H323_CS_SIG_CREATE_CNTL_REQ
// H323_CS_SIG_RE_CAPABILITIES_REQ
//typedef struct {
//	cntlHeaderSt				csHdrs;
//	mcReqCreateControl			ctMsg;
//
//} mcDsReqCreateControl;

genXmlFormat mcDsReqCreateControlFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CREATE_CNTL_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqCreateControlFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

// H323_CS_SIG_INCOMING_CHNL_RESPONSE_REQ
//typedef struct {
//	cntlHeaderSt				 csHdrs;
//	mcReqIncomingChannelResponse ctMsg;
//
//} mcDsReqIncomingChannelResponse;

genXmlFormat mcDsReqIncomingChannelResponseFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_INCOMING_CHNL_RESPONSE_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqIncomingChannelResponseFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

// H323_CS_SIG_OUTGOING_CHNL_REQ
//typedef struct {
//	cntlHeaderSt				csHdrs;
//	mcReqOutgoingChannel		ctMsg;
//
//} mcDsReqOutgoingChannel;

genXmlFormat mcDsReqOutgoingChannelFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_OUTGOING_CHNL_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqOutgoingChannelFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

// H323_CS_SIG_CHAN_NEW_RATE_REQ
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcReqChannelNewRate			ctMsg;
//
//} mcDsReqChannelNewRate;

genXmlFormat mcDsReqChannelNewRateFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CHAN_NEW_RATE_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqChannelNewRateFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

// H323_CS_SIG_CHAN_MAX_SKEW_REQ
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcReqChannelMaxSkew			ctMsg;
//
//} mcDsReqChannelMaxSkew;

genXmlFormat mcDsReqChannelMaxSkewFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CHAN_MAX_SKEW_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqChannelMaxSkewFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CHANNEL_OFF_REQ	  
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcReqChannelOff				ctMsg;
//
//} mcDsReqChannelOff;

genXmlFormat mcDsReqChannelOffFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CHANNEL_OFF_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqChannelOffFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_MULTIPOINTMODECOM_TERMINALID_REQ
//typedef struct  {
//	cntlHeaderSt							csHdrs;
//	mcReqMultipointModeComTerminalIDMessage ctMsg;
//
//} mcDsReqMultipointModeComTerminalIDMessage;

genXmlFormat mcDsReqMultipointModeComTerminalIDMessageFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_MULTIPOINTMODECOM_TERMINALID_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqMultipointModeComTerminalIDMessageFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CHANNEL_ON_REQ	  
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcReqChannelOn				ctMsg;
//
//} mcDsReqChannelOn;

genXmlFormat mcDsReqChannelOnFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CHANNEL_ON_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqChannelOnFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

// H323_CS_SIG_ROLE_TOKEN_REQ
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcReqRoleTokenMessage		ctMsg;
//} mcDsReqRoleTokenMessage;

genXmlFormat mcDsReqRoleTokenMessageFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_ROLE_TOKEN_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqRoleTokenMessageFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_UNEXPECTED_MESSAGE_REQ
//typedef struct {
//	cntlHeaderSt				csHdrs;
//	mcReqUnexpectedMessageBase	ctMsg;
//
//} mcDsReqUnexpectedMessageBase;

genXmlFormat mcDsReqUnexpectedMessageBaseFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_UNEXPECTED_MESSAGE_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqUnexpectedMessageBaseFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CALL_DROP_TIMER_EXPIRED_REQ
//typedef struct {
//	cntlHeaderSt				csHdrs;
//	mcReqCallDropTimerExpired	ctMsg;
//
//} mcDsReqCallDropTimerExpired;

genXmlFormat mcDsReqCallDropTimerExpiredFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CALL_DROP_TIMER_EXPIRED_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqCallDropTimerExpiredFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

// H323_CS_SIG_CHNL_DROP_REQ
//typedef struct {
//	cntlHeaderSt				csHdrs;
//	mcReqChannelDrop			ctMsg;
//
//} mcDsReqChannelDrop;

genXmlFormat mcDsReqChannelDropFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CHNL_DROP_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqChannelDropFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CALL_DROP_REQ
//typedef struct {
//	cntlHeaderSt				csHdrs;
//	mcReqCallDrop				ctMsg;
//
//} mcDsReqCallDrop;

genXmlFormat mcDsReqCallDropFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CALL_DROP_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqCallDropFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

// H323_CS_SIG_DBC2_COMMAND_CT_ON_REQ
// H323_CS_SIG_DBC2_COMMAND_CT_OFF_REQ
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcReqDBC2Command			ctMsg;
//
//} mcDsReqDBC2Command;

genXmlFormat mcDsReqDBC2CommandFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_DBC2_COMMAND_CT_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqDBC2CommandFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_H323_CT_AUTHENTICATION_REQ
//typedef struct {
//	cntlHeaderSt				csHdrs;
//	mcReqAuthentication			ctMsg;
//		
//} mcDsReqAuthentication;

genXmlFormat mcDsReqAuthenticationFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_H323_CT_AUTHENTICATION_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqAuthenticationFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_ROUND_TRIP_DELAY_REQ
//typedef struct {
//	cntlHeaderSt				csHdrs;
//	mcReqRoundTripDelay 		ctMsg;
//	
//} mcDsReqRoundTripDelay;

genXmlFormat mcDsReqRoundTripDelayFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_ROUND_TRIP_DELAY_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqRoundTripDelayFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

// H323_CS_SIG_NON_STANDARD_REQ_REQ
// H323_CS_SIG_NON_STANDARD_COM_REQ
// H323_CS_SIG_NON_STANDARD_RES_REQ
// H323_CS_SIG_NON_STANDARD_IND_REQ
//typedef struct	{
//	cntlHeaderSt			csHdrs;
//	mcNonStandardSt			ctMsg;
//
//} mcDsReqNonStandardMsg;

genXmlFormat mcDsReqNonStandardMsgFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_NON_STANDARD_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcNonStandardStFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_VIDEO_UPDATE_PIC_REQ  - FastUpdate req from Mcms
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//} mcDsReqVideoUpdatePicture;

genXmlFormat mcDsReqVideoUpdatePictureFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_VIDEO_UPDATE_PIC_REQ",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------


// H323_CS_SIG_LPR_MODE_CHANGE_RES_REQ
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//} mcDsReqLPRModeChangeRes;

genXmlFormat mcDsReqLPRModeChangeResFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_LPR_MODE_CHANGE_RES_REQ",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------


// H323_CS_SIG_LPR_MODE_CHANGE_REQ
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcReqLPRModeChange			ctMsg;
//} mcDsReqLPRModeChange;
genXmlFormat mcDsReqLPRModeChangeFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_LPR_MODE_CHANGE_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqLPRModeChangeFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------


//added by Jason for ITP-Multiple channels begin
// H323_CS_SIG_NEW_ITP_SPEAKER_ACK_REQ
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//} mcDsReqNewITPSpeakerAck;

genXmlFormat mcDsReqNewITPSpeakerAckFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_NEW_ITP_SPEAKER_ACK_REQ",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------


// H323_CS_SIG_NEW_ITP_SPEAKER_REQ
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcReqNewITPSpeaker			ctMsg;
//} mcDsReqNewITPSpeaker;
genXmlFormat mcDsReqNewITPSpeakerFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_NEW_ITP_SPEAKER_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqNewITPSpeakerFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------
//added by Jason for ITP-Multiple channels end


// H323_CS_SIG_RSS_CMD_REQ
//typedef struct  {
//	cntlHeaderSt				csHdrs;
//	mcReqRssCommand				ctMsg;
//
//}mcDsReqRssCommand;
//typedef struct  {
//	APIU32						subOpcode;
//	CS_RssCommon_Param_Req_S	data;
//} mcReqRssCommand;

//-------------- Common param format -----------------------------
genXmlFormat commonRssParamsFormat[] = {
    {parentElemType,    tagVarType, "commonRssParam",  2,  0,0,0,0},
        {childElemType,     longVarType,            "length",       0,  0,0,0,0},
        {siblingElemType,   charArrayVarType,   	"paramBuffer",  0,  0,0,0,0},
};


genXmlFormat mcDsReqRssCommandFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_RSS_CMD_REQ",	3,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{siblingElemType,	longVarType,	"subOpcode",	0,	0,0,0,0},
		{parentElemType, dynamicDataVarType,   NULL,   1, (int) &commonRssParamsFormat,0,0,0}
};


//*****************************************************************
//*****************************************************************
// Conference Mcm requests/indications:
//*****************************************************************
//*****************************************************************

// H323_CS_SIG_CONFERENCE_REQ_REQ
//typedef struct {
//	cntlHeaderSt				csHdrs;
//	genConferenceRequestStruct  ctMsg;
//
//} mcDsReqConferenceReq;

genXmlFormat mcDsReqConferenceReqFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CONFERENCE_REQ_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &genConferenceRequestStructFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CONFERENCE_COM_REQ
//typedef struct {
//	cntlHeaderSt					csHdrs;
//	genConferenceCommandStruct		ctMsg;
//
//} mcDsReqConferenceCom;

genXmlFormat mcDsReqConferenceComFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CONFERENCE_COM_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &genConferenceCommandStructFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------
//
//H323_CS_SIG_CONFERENCE_RES_REQ
//typedef struct {
//	cntlHeaderSt					csHdrs;
//	genConferenceResponseStruct		ctMsg;
//} mcDsReqConferenceRes;

genXmlFormat mcDsReqConferenceResFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CONFERENCE_REQ_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &genConferenceResponseStructFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------

//H323_CS_SIG_CONFERENCE_IND_REQ
//typedef struct {
//	cntlHeaderSt					csHdrs;
//	genConferenceIndicationStruct   ctMsg;
//
//} mcDsReqConferenceInd;

genXmlFormat mcDsReqConferenceIndFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CONFERENCE_IND_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &genConferenceIndicationStructFormat,0,0,0},
};
//-------------------------------------------------------------------------------------------------
// H323_CS_DTMF_INPUT_REQ
//typedef struct	{
//	cntlHeaderSt			csHdrs;
//	mcReqDtmfBuff			ctMsg;

//} mcDsReqDtmfBuff;

genXmlFormat mcDsReqDtmfBuffFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_DTMF_INPUT_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqDtmfBuffFormat,0,0,0},
};

// H323_CS_FACILITY_REQ
//typedef struct	{
//	cntlHeaderSt			csHdrs;
//	mcReqFacility 			ctMsg;

//} mcDsReqFacility;
genXmlFormat mcDsReqFacilityFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_FACILITY_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcReqFacilityFormat,0,0,0},
};

// H323_CS_PARTY_KEEP_ALIVE_REQ
//typedef struct {
//	cntlHeaderSt			csHdrs;
//
//} mcDsReqKeepAlive;

genXmlFormat mcDsReqKeepAliveFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_PARTY_KEEP_ALIVE_REQ",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};

// H323_CS_SIG_CAPABILITIES_RES_REQ(Empty)
//typedef struct {
//	cntlHeaderSt			csHdrs;
//
//} mcDsReqCapbabilitiesRes;

genXmlFormat mcDsReqCapbabilitiesRes[] = {
	{rootElemType,	tagVarType,	"H323_CS_SIG_CAPABILITIES_RES_REQ",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};


// EOF
