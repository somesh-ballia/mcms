// Convertor.cpp
// Ami Noy

// Includes files
//----------------
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include <XmlDefs.h>
#include <XmlErrors.h>
#include <XmlPrintLog.h>
#include <MplMcmsStructs.h>
#include <IpCsOpcodes.h>
#include <IpMngrOpcodes.h>
#include <XmlEngineApi.h>
#include <FastIndexTable.h>

#include <XmlFormat.h>
#include <XmlBuilder.h>
#include <Parser.h>

// Macros
//--------
#undef SRC
#define SRC "Convertor"

#define MaxOpcodeNameLen		64

#define AddStartupIndicationsFormat(opcode, pFormat) \
	FastIndexTableAddIndex (						\
		pStartupIndicationsFormatTable,				\
		(unsigned long)opcode,						\
		(unsigned long)pFormat)

#define AddStartupRequestsFormat(opcode, pFormat)	\
	FastIndexTableAddIndex (						\
		pStartupRequestsFormatTable,				\
		(unsigned long)opcode,						\
		(unsigned long)pFormat)

#define AddServiceRequestsFormat(opcode, pFormat)	\
	FastIndexTableAddIndex (						\
		pServiceRequestsFormatTable,				\
		(unsigned long)opcode,		      			\
		(unsigned long)pFormat)

#define AddServiceIndicationsFormat(opcode, pFormat)  \
    FastIndexTableAddIndex (                        \
        pServiceIndicationsFormatTable,             \
        (int)opcode,                                \
        (int)pFormat)

#define AddMcmsRequestsToCallTaskFormat(opcode, pFormat)  \
    FastIndexTableAddIndex (                        \
        pMcmsRequestsToCallTaskFormatTable,         \
        (int)opcode,                                \
        (int)pFormat)

#define AddMcmsRequestsToGkIfTaskFormat(opcode, pFormat)  \
    FastIndexTableAddIndex (                        \
        pMcmsRequestsToGkIfTaskFormatTable,         \
        (int)opcode,                                \
        (int)pFormat)

#define AddCallTaskIndicationsFormat(opcode, pFormat)  \
    FastIndexTableAddIndex (                        \
        pCallTaskIndicationsFormatTable,            \
        (int)opcode,                                \
        (int)pFormat)

#define AddGkIfTaskIndicationsFormat(opcode, pFormat)  \
    FastIndexTableAddIndex (                        \
        pGkIfTaskIndicationsFormatTable,            \
        (int)opcode,                                \
        (int)pFormat)

#define AddSipTaskIndicationsFormat(opcode, pFormat)\
	FastIndexTableAddIndex (						\
		pSipTaskIndicationsFormatTable,				\
		(unsigned long)opcode,						\
		(unsigned long)pFormat)

#define AddSipTaskRequestsFormat(opcode, pFormat)	\
	FastIndexTableAddIndex (						\
		pSipTaskRequestsFormatTable,				\
		(unsigned long)opcode,						\
		(unsigned long)pFormat)

#define AddProxyIndicationsFormat(opcode, pFormat)	\
	FastIndexTableAddIndex (						\
		pProxyIndicationsFormatTable,				\
		(unsigned long)opcode,						\
		(unsigned long)pFormat)

#define AddProxyRequestsFormat(opcode, pFormat)		\
	FastIndexTableAddIndex (						\
		pProxyRequestsFormatTable,					\
		(unsigned long)opcode,						\
		(unsigned long)pFormat)

#define AddDnsIndicationsFormat(opcode, pFormat)	\
	FastIndexTableAddIndex (						\
		pDnsIndicationsFormatTable,				\
		(unsigned long)opcode,						\
		(unsigned long)pFormat)

#define AddDnsRequestsFormat(opcode, pFormat)		\
	FastIndexTableAddIndex (						\
		pDnsRequestsFormatTable,					\
		(unsigned long)opcode,						\
		(unsigned long)pFormat)

//_mccf_
#define AddMccfIndicationsFormat(opcode, pFormat)\
	FastIndexTableAddIndex (						\
		pMccfIndicationsFormatTable,				\
		(unsigned long)opcode,						\
		(unsigned long)pFormat)

//_mccf_
#define AddMccfRequestsFormat(opcode, pFormat)	\
	FastIndexTableAddIndex (						\
		pMccfRequestsFormatTable,				\
		(unsigned long)opcode,						\
		(unsigned long)pFormat)



//	Static variables
//-------------------

// Format tables
static	fastIndexTableStruct	*pStartupIndicationsFormatTable			= NULL;
static	fastIndexTableStruct	*pStartupRequestsFormatTable			= NULL;
static  fastIndexTableStruct	*pServiceRequestsFormatTable			= NULL;
static  fastIndexTableStruct    *pServiceIndicationsFormatTable         = NULL;

static  fastIndexTableStruct    *pMcmsRequestsToCallTaskFormatTable		= NULL;
static  fastIndexTableStruct    *pMcmsRequestsToGkIfTaskFormatTable		= NULL;
static  fastIndexTableStruct    *pCallTaskIndicationsFormatTable        = NULL;
static  fastIndexTableStruct    *pGkIfTaskIndicationsFormatTable        = NULL;
static	fastIndexTableStruct	*pSipTaskIndicationsFormatTable			= NULL;
static	fastIndexTableStruct	*pSipTaskRequestsFormatTable			= NULL;
static	fastIndexTableStruct	*pProxyIndicationsFormatTable			= NULL;
static	fastIndexTableStruct	*pProxyRequestsFormatTable				= NULL;

static	fastIndexTableStruct	*pDnsIndicationsFormatTable				= NULL;
static	fastIndexTableStruct	*pDnsRequestsFormatTable				= NULL;

static	fastIndexTableStruct	*pMccfIndicationsFormatTable			= NULL; //_mccf_
static	fastIndexTableStruct	*pMccfRequestsFormatTable				= NULL; //_mccf_

static	fastIndexTableStruct	*pCccpIndicationsFormatTable			= NULL; // cccp
static	fastIndexTableStruct	*pCccpRequestsFormatTable				= NULL; // cccp

static	char					*pTxXmlBuffer							= NULL;
static  char					*pRxXmlBuffer 							= NULL;
static  char					*pZipXmlBuffer							= NULL;
static  char					*pUnzipXmlBuffer						= NULL;

static  int						maxZipSize 								= 0;
static	BOOL					bZipUnzip								= 1; //TRUE;

// Global variables
//------------------
#ifdef __XmlTrace__
int xmlTraceLevel = emXmlTraceFull;
#else
int xmlTraceLevel = emXmlTraceErrWrn;
#endif

// External variables
//------------------
// startup indications formats
extern genXmlFormat suCsNewIndFormat[];
extern genXmlFormat suCsConfigParamIndFormat[];
extern genXmlFormat suCsEndConfigParamIndFormat[];
extern genXmlFormat suCsEndStartupIndFormat[];
extern genXmlFormat suCsReconnectIndFormat[];
extern genXmlFormat suCsKeepAliveIndFormat[];
extern genXmlFormat suCsCompStatusIndFormat[];

// startup requests formats
extern genXmlFormat suCsNewReqFormat[];
extern genXmlFormat suCsConfigParamReqFormat[];
extern genXmlFormat suCsEndConfigParamReqFormat[];
extern genXmlFormat suCsLanConfigReqFormat[];
extern genXmlFormat suCsReconnectReqFormat[];
extern genXmlFormat suCsKeepAliveReqFormat[];

// service requests formats
extern genXmlFormat srCsNewServiceInitReqFormat[];
extern genXmlFormat srCsCommonParamReqFormat[];
extern genXmlFormat srCsEndServiceInitReqFormat[];
extern genXmlFormat srCsDelServiceReqFormat[];
extern genXmlFormat srCsTerminalCommandReqFormat[];

// service indications formats
extern genXmlFormat srCsNewServiceInitIndFormat[];
extern genXmlFormat srCsCommonParamIndFormat[];
extern genXmlFormat srCsEndServiceInitIndFormat[];
extern genXmlFormat srCsDelServiceIndFormat[];

//Ping formats
extern genXmlFormat srCsPingReqFormat[];
extern genXmlFormat srCsPingIndFormat[];

//Call task indication formats
extern genXmlFormat ctDsIndGetPortFormat[];
extern genXmlFormat ctDsIndCallConnectedFormat[];
extern genXmlFormat ctDsIndCallOfferingFormat[];
extern genXmlFormat ctDsIndCapabilitiesFormat[];
extern genXmlFormat ctDsIndCallNewRateFormat[];
extern genXmlFormat ctDsIndCapResponseFormat[];
extern genXmlFormat ctDsIndIncomingChannelFormat[];
extern genXmlFormat ctDsIndIncomingChannelConnectedFormat[];
extern genXmlFormat ctDsIndOutgoingChannelResponseFormat[];
extern genXmlFormat ctDsIndChannelNewRateFormat[];
extern genXmlFormat ctDsIndCallIdleFormat[];
extern genXmlFormat ctDsIndChannelClosedFormat[];
extern genXmlFormat ctDsIndFlowControlIndicationFormat[];
extern genXmlFormat ctDsIndDBC2CommandFormat[];
extern genXmlFormat ctDsIndChannelMaxSkewFormat[];
extern genXmlFormat ctDsIndVideoUpdatePictureFormat[];
extern genXmlFormat ctDsIndChannelOffFormat[];
extern genXmlFormat ctDsIndChannelOnFormat[];
extern genXmlFormat ctDsIndRoleTokenFormat[];
extern genXmlFormat ctDsIndNonStandardMsgFormat[];
extern genXmlFormat ctDsIndStartChannelCloseFormat[];
extern genXmlFormat ctDsIndCallControlConnectedFormat[];
extern genXmlFormat ctDsIndDtmfBuffFormat[];
extern genXmlFormat ctDsIndKeepAliveBuffFormat[];
extern genXmlFormat ctDsIndBadSpontanFormat[];
extern genXmlFormat ctDsIndFacilityBuffFormat[];
extern genXmlFormat ctDsIndConferenceIndFormat[];
extern genXmlFormat ctDsIndLprModeChangeFormat[];
extern genXmlFormat ctDsIndLprModeChangeResFormat[];
extern genXmlFormat ctDsIndNewITPSpeakerFormat[];
extern genXmlFormat ctDsIndNewITPSpeakerAckFormat[];

//Call task request formats
extern genXmlFormat mcDsReqGetPortFormat[];
extern genXmlFormat mcDsReqReleasePortFormat[];
extern genXmlFormat mcDsReqCallSetupFormat[];
extern genXmlFormat mcDsReqCallAnswerFormat[];
extern genXmlFormat mcDsReqCreateControlFormat[];
extern genXmlFormat mcDsReqIncomingChannelResponseFormat[];
extern genXmlFormat mcDsReqOutgoingChannelFormat[];
extern genXmlFormat mcDsReqChannelMaxSkewFormat[];
extern genXmlFormat mcDsReqVideoUpdatePictureFormat[];
extern genXmlFormat mcDsReqChannelOffFormat[];
extern genXmlFormat mcDsReqChannelOnFormat[];
extern genXmlFormat mcDsReqUnexpectedMessageBaseFormat[];
extern genXmlFormat mcDsReqCallDropTimerExpiredFormat[];
extern genXmlFormat genConferenceResponseStructFormat[];
extern genXmlFormat genConferenceIndicationStructFormat[];
extern genXmlFormat mcDsReqChannelDropFormat[];
extern genXmlFormat mcDsReqCallDropFormat[];
extern genXmlFormat mcDsReqCallCloseConfirmFormat[];
extern genXmlFormat mcDsReqDBC2CommandFormat[];
extern genXmlFormat mcDsReqChannelNewRateFormat[];
extern genXmlFormat mcDsReqNonStandardMsgFormat[];
extern genXmlFormat mcDsReqRoundTripDelayFormat[];
extern genXmlFormat genConferenceRequestStructFormat[];
extern genXmlFormat genConferenceCommandStructFormat[];
extern genXmlFormat mcDsReqDtmfBuffFormat[];
extern genXmlFormat mcDsReqMultipointModeComTerminalIDMessageFormat[];
extern genXmlFormat mcDsReqConferenceResFormat[];
extern genXmlFormat mcDsReqConferenceIndFormat[];
extern genXmlFormat mcDsReqConferenceReqFormat[];
extern genXmlFormat mcDsReqConferenceComFormat[];
extern genXmlFormat mcDsReqKeepAliveFormat[];
extern genXmlFormat mcDsReqRoleTokenMessageFormat[];
extern genXmlFormat mcDsReqCapbabilitiesRes[];
extern genXmlFormat mcDsReqFacilityFormat[];
extern genXmlFormat mcDsReqLPRModeChangeResFormat[];
extern genXmlFormat mcDsReqLPRModeChangeFormat[];
extern genXmlFormat mcDsReqRssCommandFormat[];
extern genXmlFormat mcDsReqNewITPSpeakerAckFormat[];
extern genXmlFormat mcDsReqNewITPSpeakerFormat[];


//Gatekeeper indication formats
extern genXmlFormat	gkDsIndRasGRQFormat[];
extern genXmlFormat gkDsIndRasRRQFormat[];
extern genXmlFormat gkDsIndRasURQFormat[];
extern genXmlFormat gkDsIndRasARQFormat[];
extern genXmlFormat gkDsIndRasDRQFormat[];
extern genXmlFormat gkDsIndRasBRQFormat[];
extern genXmlFormat gkDsIndRasFailFormat[];
extern genXmlFormat gkDsIndRasTimeoutFormat[];
extern genXmlFormat gkDsIndURQFromGkFormat[];
extern genXmlFormat gkDsIndDRQFromGkFormat[];
extern genXmlFormat gkDsIndBRQFromGkFormat[];
extern genXmlFormat gkDsIndLRQFromGkFormat[];
extern genXmlFormat gkDsIndGKIRQFormat[];
extern genXmlFormat gkDsIndRACFormat[];

//Gatekeeper request formats
extern genXmlFormat gkDsReqRasGRQFormat[];
extern genXmlFormat gkDsReqRasRRQFormat[];
extern genXmlFormat gkDsReqRasURQFormat[];
extern genXmlFormat gkDsReqURQResponseFormat[];
extern genXmlFormat gkDsReqDRQResponseFormat[];
extern genXmlFormat gkDsReqRasARQFormat[];
extern genXmlFormat gkDsReqRasBRQFormat[];
extern genXmlFormat gkDsReqRasIRRFormat[];
extern genXmlFormat gkDsReqRasIRRFormat[];
extern genXmlFormat gkDsReqRasDRQFormat[];
extern genXmlFormat gkDsReqLRQResponseFormat[];
extern genXmlFormat gkDsReqBRQResponseFormat[];
extern genXmlFormat gkDsReqRemoveCallFormat[];
extern genXmlFormat gkDsReqRAIFormat[];

//SIP task request formats
extern genXmlFormat mcDsReqInviteFormat[];
extern genXmlFormat mcDsReqReInviteFormat[];
extern genXmlFormat mcDsReqInviteAckFormat[];
extern genXmlFormat mcDsReqInviteResponseFormat[];
extern genXmlFormat mcDsReqRingingFormat[];
extern genXmlFormat mcDsReqCancelFormat[];
extern genXmlFormat mcDsReqByeFormat[];
extern genXmlFormat mcDsReqBye200OkFormat[];
extern genXmlFormat mcDsReqVideoFastUpdateFormat[];
extern genXmlFormat mcDsReqDelNewCallFormat[];
extern genXmlFormat mcDsReqInstantMessageFormat[];
extern genXmlFormat mcDsReqRedirectDataFormat[];
extern genXmlFormat mcDsReqOptionsRespFormat[];
extern genXmlFormat mcDsReqReferRespFormat[];
extern genXmlFormat mcDsReqSipKeepAliveFormat[];
extern genXmlFormat mcDsReqInfoFormat[];
extern genXmlFormat mcDsReqInfoRespFormat[];

extern genXmlFormat mcDsReqBfcpMessageFormat[];
extern genXmlFormat mcDsReqDialogRecoveryFormat[];
extern genXmlFormat mcDsReqSendCrlfFormat[];
extern genXmlFormat mcDsReqSocketStatisticsFormat[];

//SIP task indication formats
extern genXmlFormat spDsIndInviteFormat[];
extern genXmlFormat spDsIndReinviteFormat[];
extern genXmlFormat spDsIndInviteAckFormat[];
extern genXmlFormat spDsIndInviteResponseFormat[];
extern genXmlFormat spDsIndProvResponseFormat[];
extern genXmlFormat spDsIndByeFormat[];
extern genXmlFormat spDsIndBye200OkFormat[];
extern genXmlFormat spDsIndCancelFormat[];
extern genXmlFormat spDsIndVideoFastUpdateFormat[];
extern genXmlFormat spDsIndDtmfDigitFormat[];
extern genXmlFormat spDsIndInstantMessageFormat[];
extern genXmlFormat spDsIndOptionsFormat[];
extern genXmlFormat spDsIndReferFormat[];
extern genXmlFormat spDsIndSessionTimerExpiredFormat[];
extern genXmlFormat spDsIndSessionTimerReinviteFormat[];
extern genXmlFormat spDsIndTraceInfoFormat[];
extern genXmlFormat spDsIndBadStatusFormat[];
extern genXmlFormat spDsIndTransportErrorFormat[];
extern genXmlFormat spDsIndCardDataFormat[];
extern genXmlFormat spDsIndKeepAliveFormat[];
extern genXmlFormat spDsIndInfoFormat[];
extern genXmlFormat spDsIndInfoRespFormat[];
extern genXmlFormat	spDsIndBfcpMessageFormat[];
extern genXmlFormat	spDsIndBfcpTransportFormat[];
extern genXmlFormat spDsIndDialogRecoveryFormat[];
extern genXmlFormat spDsIndSocketStatisticsFormat[];
extern genXmlFormat spDsIndSigCrlfErrorFormat[];

extern genXmlFormat spDsIndNotifyRespFormat[];
extern genXmlFormat spDsIndBenotifyFormat[];
//-S- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------//
extern genXmlFormat spDsIndBenotifyFormat_Part[];
//-E- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------//

//PROXY request formats
extern genXmlFormat mcDsReqRegisterFormat[];
extern genXmlFormat mcDsReqSubscribeFormat[];
extern genXmlFormat mcDsReqSubscribeRespFormat[];
extern genXmlFormat mcDsReqUnknownMethodFormat[];
extern genXmlFormat mcDsReqNotifyFormat[];
extern genXmlFormat mcDsReqServiceFormat[];

//PROXY indication formats
extern genXmlFormat prxDsIndRegisterRespFormat[];
extern genXmlFormat prxDsIndSubscribeRespFormat[];
extern genXmlFormat spDsIndSubscribeFormat[];
extern genXmlFormat prxDsIndNotifyFormat[];
extern genXmlFormat prxDsIndTraceInfoFormat[];
extern genXmlFormat prxDsIndBadStatusFormat[];
extern genXmlFormat prxDsIndTransportErrorFormat[];
extern genXmlFormat prxDsIndCardDataFormat[];
extern genXmlFormat prxDsIndServiceRespFormat[];
extern genXmlFormat prxDsIndProxyCrlfErrorFormat[];

//_mccf_
//MCCF Manager formats
extern genXmlFormat mccfDsReqInviteResponseFormat[];
extern genXmlFormat mccfIndInviteFormat[];
extern genXmlFormat mccfIndInviteAckFormat[];
extern genXmlFormat mccfIndByeFormat[];
extern genXmlFormat mccfDsReqBye200OkFormat[];
extern genXmlFormat mccfDsReqByeFormat[];
extern genXmlFormat mccfIndBye200OkFormat[];
extern genXmlFormat mccfIndSessionTimerExpiredFormat[];
extern genXmlFormat mccfIndBadStatusFormat[];
extern genXmlFormat mccfIndTransportErrorFormat[];

//CCCP Manager formats cccp
extern genXmlFormat cccpIndInviteFormat[];
extern genXmlFormat cccpIndAddUserInviteResponseFormat[];
extern genXmlFormat cccpInviteReqFormat[];

//Active alarms
extern genXmlFormat suCSActiveAlarmIndFormat[];
// DNS request formats
extern genXmlFormat srDnsCsResolveReqFormat[];
extern genXmlFormat srDnsCsServiceReqFormat[];

// DNS indication formats
extern genXmlFormat srDnsCsResolveIndFormat[];
extern genXmlFormat srDnsCsServiceIndFormat[];

//	External Routines:
//--------------------
extern 	int GenBinBuilder(
	char			**ppMessage,
	genXmlFormat	**ppFormat,
	char			**xml,
	int				nElements,
	int				binMsgSize,
	int				*pCurBinMsgSize);

extern int CreateDynamicCreateCntlFormat(
	int				opcode,
	char			*pMessage,
	genXmlFormat	*pFormat);

extern int CreateDynamicCreateCntlMessage(
   	int				opcode,
	char			*pMsg,
	char			*xml,
	genXmlFormat	*pFormat);

// Forward Declarations:
//----------------------
// Routines:
//----------
//----------
//---------------------------------------------------------------------------
//
//  Function name:  XmlZipMessage
//
//  Description:    Zip XML message without COMMON_HEADER
//
//  Return code:
//                  0                   - success
//                  negative value      - error
//---------------------------------------------------------------------------
static int XmlZipMessage(
	int 	*pXmlMsgLen,
	char	*psOpcode)
{
	int status;
	int zipMessageLen;
	int srcLenToCompress = *pXmlMsgLen - MaxUnzipHeaderSize;

	zipMessageLen = CompressXmlBufferSize;

	maxZipSize = compressBound((uLong) srcLenToCompress);

	if (maxZipSize > CompressXmlBufferSize) {
		XmlPrintLog("Expected zip size bound = %d to be bigger than %d", PERR, maxZipSize, zipMessageLen);

		free(pZipXmlBuffer);
		pZipXmlBuffer = NULL;

		pZipXmlBuffer = (char *) calloc(maxZipSize, 1);

		if (!pZipXmlBuffer) {
			XmlPrintLog("pZipXmlBuffer is NULL", PERR);
			return E_XML_NULL_PTR;
		}
	}

	status = compress2 (
			(Bytef *) pZipXmlBuffer,						// dest buffer
			(uLongf *) &zipMessageLen,						// dest len,IN - total size of the destination buffer.OUT - the actual size of the compressed buffer.
			(Bytef *) (pTxXmlBuffer + MaxUnzipHeaderSize),	// src buffer
			(uLong) srcLenToCompress, 						// src len
			Z_BEST_SPEED);                                  // level - Z_BEST_SPEED, Z_BEST_COMPRESSION
	if (status) {
		XmlPrintLog("Compress XML file failed, opcode = %s, status = %d", PERR, psOpcode, status);
		return status;
	}

	*(pZipXmlBuffer + zipMessageLen) = '\0';

	*pXmlMsgLen = zipMessageLen + MaxUnzipHeaderSize + 1;

 	return 0;

}// XmlZipMessage

//---------------------------------------------------------------------------
//
//  Function name:  XmlUnzipMessage
//
//  Description:    Unzip XML message and make full xml buffer (COMMON_HEADER and content)
//
//  Return code:
//                  0                   - success
//                  negative value      - error
//---------------------------------------------------------------------------
static int XmlUnzipMessage(
	char 	*pXmlMsg,
	int		rcvXmlMsgSize,
	int 	*pUnzipMsgLen)
{
	int status;

	*pUnzipMsgLen = UncompressXmlBufferSize;

	status = uncompress(
			(Bytef *) pUnzipXmlBuffer,						// dest buffer
			(uLongf *) pUnzipMsgLen,						// dest len. IN - the total size of the destination buffer. OUT - the actual size of the compressed buffer
		    (const Bytef *) (pXmlMsg + MaxUnzipHeaderSize), // src buffer
		    (uLong) rcvXmlMsgSize - MaxUnzipHeaderSize);    // src len
	if (status) {
	  	XmlPrintLog("Uncompress XML file failed, status = %d", PERR, status);
		return status;
    }

    memcpy(
    	pRxXmlBuffer,
    	pXmlMsg,
    	MaxUnzipHeaderSize);

    memcpy(
	    pRxXmlBuffer + MaxUnzipHeaderSize,
	    pUnzipXmlBuffer,
	    *pUnzipMsgLen);

	*pUnzipMsgLen += MaxUnzipHeaderSize;

	*(pRxXmlBuffer + *pUnzipMsgLen) = '\0';

	*pUnzipMsgLen += 1;

	return 0;

}// XmlUnzipMessage

//---------------------------------------------------------------------------
//
//  Function name:  InitCsIndFormatTable
//
//  Description:    Init format table for Ct GkIf and sip
//
//  Return code:
//                  0                   - success
//                  negative value      - error
//---------------------------------------------------------------------------
static int InitCsIndFormatTable()
{
    int status = 0;

    status = CreateFastIndexTable(
        &pCallTaskIndicationsFormatTable,
        H323_CS_SIG_FIRST_IND,
        H323_CS_SIG_LAST_IND - H323_CS_SIG_FIRST_IND);

    if(status) {
        XmlPrintLog("CreateFastIndexTable", PERR, status);
        return status;
    }

    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_GET_PORT_IND,		&ctDsIndGetPortFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_CALL_CONNECTED_IND,	&ctDsIndCallConnectedFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_CALL_OFFERING_IND,	&ctDsIndCallOfferingFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_CAPABILITIES_IND,	&ctDsIndCapabilitiesFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_CALL_NEW_RATE_IND,	&ctDsIndCallNewRateFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_CAP_RESPONSE_IND,	&ctDsIndCapResponseFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_CALL_CNTL_CONNECTED_IND, &ctDsIndCallControlConnectedFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_INCOMING_CHANNEL_IND,	&ctDsIndIncomingChannelFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_INCOMING_CHNL_CONNECTED_IND, &ctDsIndIncomingChannelConnectedFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND,	&ctDsIndOutgoingChannelResponseFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_CHAN_NEW_RATE_IND,	&ctDsIndChannelNewRateFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_CALL_IDLE_IND,		&ctDsIndCallIdleFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_CHANNEL_CLOSE_IND,	&ctDsIndChannelClosedFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_FLOW_CONTROL_IND_IND,&ctDsIndFlowControlIndicationFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_DBC2_COMMAND_CT_ON_IND, &ctDsIndDBC2CommandFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_DBC2_COMMAND_CT_OFF_IND, &ctDsIndDBC2CommandFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_CHAN_MAX_SKEW_IND,	&ctDsIndChannelMaxSkewFormat);
    status |= AddCallTaskIndicationsFormat(IP_CS_VIDEO_UPDATE_PIC_IND,		&ctDsIndVideoUpdatePictureFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_CHANNEL_OFF_IND,			&ctDsIndChannelOffFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_CHANNEL_ON_IND,			&ctDsIndChannelOnFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_CALL_ROLE_TOKEN_IND, &ctDsIndRoleTokenFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_NON_STANDARD_REQ_IND,	&ctDsIndNonStandardMsgFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_NON_STANDARD_COM_IND,	&ctDsIndNonStandardMsgFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_NON_STANDARD_RES_IND,	&ctDsIndNonStandardMsgFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_NON_STANDARD_IND_IND,	&ctDsIndNonStandardMsgFormat);
    status |= AddCallTaskIndicationsFormat(H323_CS_SIG_START_CHANNEL_CLOSE_IND, &ctDsIndStartChannelCloseFormat);
	status |= AddCallTaskIndicationsFormat(H323_CS_DTMF_INPUT_IND, &ctDsIndDtmfBuffFormat);
	status |= AddCallTaskIndicationsFormat(H323_CS_PARTY_KEEP_ALIVE_IND, &ctDsIndKeepAliveBuffFormat);
	status |= AddCallTaskIndicationsFormat(H323_CS_SIG_CALL_BAD_SPONTAN_IND, &ctDsIndBadSpontanFormat);
	status |= AddCallTaskIndicationsFormat(H323_CS_FACILITY_IND, &ctDsIndFacilityBuffFormat);
	status |= AddCallTaskIndicationsFormat(H323_CS_CONFERENCE_IND_IND, &ctDsIndConferenceIndFormat);
	status |= AddCallTaskIndicationsFormat(H323_CS_SIG_LPR_MODE_CHANGE_IND, &ctDsIndLprModeChangeFormat);
	status |= AddCallTaskIndicationsFormat(H323_CS_SIG_LPR_MODE_CHANGE_RES_IND, &ctDsIndLprModeChangeResFormat);
	//added by Jason for ITP-Multiple channels begin
	status |= AddCallTaskIndicationsFormat(H323_CS_SIG_NEW_ITP_SPEAKER_IND, &ctDsIndNewITPSpeakerFormat);
	status |= AddCallTaskIndicationsFormat(H323_CS_SIG_NEW_ITP_SPEAKER_ACK_IND, &ctDsIndNewITPSpeakerAckFormat);
	//added by Jason for ITP-Multiple channels end
	
	if(status) {
        XmlPrintLog("CreateFastIndexTable calltask Ind", PERR, status);
        return status;
    }
	status = CreateFastIndexTable(
        &pGkIfTaskIndicationsFormatTable,
        H323_CS_RAS_FIRST_IND,
        H323_CS_RAS_LAST_IND - H323_CS_RAS_FIRST_IND);

    if(status) {
        XmlPrintLog("CreateFastIndexTable", PERR, status);
        return status;
    }

	status |= AddGkIfTaskIndicationsFormat(H323_CS_RAS_GRQ_IND, &gkDsIndRasGRQFormat);
	status |= AddGkIfTaskIndicationsFormat(H323_CS_RAS_RRQ_IND, &gkDsIndRasRRQFormat);
	status |= AddGkIfTaskIndicationsFormat(H323_CS_RAS_URQ_IND, &gkDsIndRasURQFormat);
	status |= AddGkIfTaskIndicationsFormat(H323_CS_RAS_ARQ_IND, &gkDsIndRasARQFormat);
	status |= AddGkIfTaskIndicationsFormat(H323_CS_RAS_DRQ_IND, &gkDsIndRasDRQFormat);
	status |= AddGkIfTaskIndicationsFormat(H323_CS_RAS_BRQ_IND, &gkDsIndRasBRQFormat);
	status |= AddGkIfTaskIndicationsFormat(H323_CS_RAS_FAIL_IND, &gkDsIndRasFailFormat);
	status |= AddGkIfTaskIndicationsFormat(H323_CS_RAS_TIMEOUT_IND, &gkDsIndRasTimeoutFormat);
	status |= AddGkIfTaskIndicationsFormat(H323_CS_RAS_GKURQ_IND, &gkDsIndURQFromGkFormat);
	status |= AddGkIfTaskIndicationsFormat(H323_CS_RAS_GKDRQ_IND, &gkDsIndDRQFromGkFormat);
	status |= AddGkIfTaskIndicationsFormat(H323_CS_RAS_GKBRQ_IND, &gkDsIndBRQFromGkFormat);
	status |= AddGkIfTaskIndicationsFormat(H323_CS_RAS_GKLRQ_IND, &gkDsIndLRQFromGkFormat);
	status |= AddGkIfTaskIndicationsFormat(H323_CS_RAS_GKIRQ_IND, &gkDsIndGKIRQFormat);
	status |= AddGkIfTaskIndicationsFormat(H323_CS_RAS_RAC_IND,   &gkDsIndRACFormat);

    if (status) {
        XmlPrintLog("InitCallTaskIndFormatTable", PERR, status);
    }

    return status;

}// InitCSIndFormatTable

//---------------------------------------------------------------------------
//
//  Function name:  InitMcmsReqFormatTable
//
//  Description:    Init format table
//
//  Return code:
//                  0                   - success
//                  negative value      - error
//---------------------------------------------------------------------------

static int InitMcmsReqFormatTable()
{
    int status = 0;

    status = CreateFastIndexTable(
        &pMcmsRequestsToCallTaskFormatTable,
        H323_CS_SIG_FIRST_REQ,
        H323_CS_SIG_LAST_REQ - H323_CS_SIG_FIRST_REQ);

    if(status) {
        XmlPrintLog("CreateFastIndexTable", PERR, status);
        return status;
    }

    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_GET_PORT_REQ,	&mcDsReqGetPortFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_RELEASE_PORT_REQ,	&mcDsReqReleasePortFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CALL_SETUP_REQ,	&mcDsReqCallSetupFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CALL_ANSWER_REQ,	&mcDsReqCallAnswerFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_RE_CAPABILITIES_REQ,	&mcDsReqCreateControlFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CREATE_CNTL_REQ,	&mcDsReqCreateControlFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_INCOMING_CHNL_RESPONSE_REQ, &mcDsReqIncomingChannelResponseFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_OUTGOING_CHNL_REQ,	&mcDsReqOutgoingChannelFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CHAN_MAX_SKEW_REQ,	&mcDsReqChannelMaxSkewFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_VIDEO_UPDATE_PIC_REQ,	&mcDsReqVideoUpdatePictureFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CHANNEL_OFF_REQ,	&mcDsReqChannelOffFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CHANNEL_ON_REQ,	&mcDsReqChannelOnFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_UNEXPECTED_MESSAGE_REQ, &mcDsReqUnexpectedMessageBaseFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CALL_DROP_TIMER_EXPIRED_REQ, &mcDsReqCallDropTimerExpiredFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CONFERENCE_RES_REQ, &mcDsReqConferenceResFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CONFERENCE_IND_REQ, &mcDsReqConferenceIndFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CHNL_DROP_REQ,	&mcDsReqChannelDropFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CALL_DROP_REQ,	&mcDsReqCallDropFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CALL_CLOSE_CONFIRM_REQ, &mcDsReqCallCloseConfirmFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_DBC2_COMMAND_CT_ON_REQ, &mcDsReqDBC2CommandFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_DBC2_COMMAND_CT_OFF_REQ, &mcDsReqDBC2CommandFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CHNL_NEW_RATE_REQ,	&mcDsReqChannelNewRateFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_NON_STANDARD_REQ_REQ,	&mcDsReqNonStandardMsgFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_NON_STANDARD_RES_REQ,	&mcDsReqNonStandardMsgFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_NON_STANDARD_IND_REQ,	&mcDsReqNonStandardMsgFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_NON_STANDARD_COM_REQ,	&mcDsReqNonStandardMsgFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_ROUND_TRIP_DELAY_REQ, &mcDsReqRoundTripDelayFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CONFERENCE_REQ_REQ, &mcDsReqConferenceReqFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CONFERENCE_COM_REQ, &mcDsReqConferenceComFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_DTMF_INPUT_REQ, &mcDsReqDtmfBuffFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_MULTIPOINTMODECOM_TERMINALID_REQ, &mcDsReqMultipointModeComTerminalIDMessageFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_PARTY_KEEP_ALIVE_REQ, &mcDsReqKeepAliveFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_ROLE_TOKEN_REQ, &mcDsReqRoleTokenMessageFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_CAPABILITIES_RES_REQ, &mcDsReqCapbabilitiesRes);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_FACILITY_REQ, &mcDsReqFacilityFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_LPR_MODE_CHANGE_RES_REQ, &mcDsReqLPRModeChangeResFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_LPR_MODE_CHANGE_REQ, &mcDsReqLPRModeChangeFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_RSS_CMD_REQ, &mcDsReqRssCommandFormat);
    //added by Jason for ITP-Multiple channels begin
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_NEW_ITP_SPEAKER_ACK_REQ, &mcDsReqNewITPSpeakerAckFormat);
    status |= AddMcmsRequestsToCallTaskFormat(H323_CS_SIG_NEW_ITP_SPEAKER_REQ, &mcDsReqNewITPSpeakerFormat);
    //added by Jason for ITP-Multiple channels end

	if (status) {
        XmlPrintLog("CreateFastIndexTable for CallTask", PERR, status);
        return status;
    }
	status = CreateFastIndexTable(
   		  	&pMcmsRequestsToGkIfTaskFormatTable,
    		H323_CS_RAS_FIRST_REQ,
			H323_CS_RAS_LAST_REQ - H323_CS_RAS_FIRST_REQ);

    if (status) {
        XmlPrintLog("CreateFastIndexTable", PERR, status);
        return status;
    }

	status |= AddMcmsRequestsToGkIfTaskFormat(H323_CS_RAS_GRQ_REQ, &gkDsReqRasGRQFormat);
	status |= AddMcmsRequestsToGkIfTaskFormat(H323_CS_RAS_RRQ_REQ, &gkDsReqRasRRQFormat);
	status |= AddMcmsRequestsToGkIfTaskFormat(H323_CS_RAS_URQ_REQ, &gkDsReqRasURQFormat);
	status |= AddMcmsRequestsToGkIfTaskFormat(H323_CS_RAS_URQ_RESPONSE_REQ, &gkDsReqURQResponseFormat);
	status |= AddMcmsRequestsToGkIfTaskFormat(H323_CS_RAS_DRQ_RESPONSE_REQ, &gkDsReqDRQResponseFormat);
	status |= AddMcmsRequestsToGkIfTaskFormat(H323_CS_RAS_ARQ_REQ, &gkDsReqRasARQFormat);
	status |= AddMcmsRequestsToGkIfTaskFormat(H323_CS_RAS_BRQ_REQ, &gkDsReqRasBRQFormat);
	status |= AddMcmsRequestsToGkIfTaskFormat(H323_CS_RAS_IRR_REQ, &gkDsReqRasIRRFormat);
	status |= AddMcmsRequestsToGkIfTaskFormat(H323_CS_RAS_IRR_RESPONSE_REQ, &gkDsReqRasIRRFormat);
	status |= AddMcmsRequestsToGkIfTaskFormat(H323_CS_RAS_DRQ_REQ, &gkDsReqRasDRQFormat);
	status |= AddMcmsRequestsToGkIfTaskFormat(H323_CS_RAS_LRQ_RESPONSE_REQ, &gkDsReqLRQResponseFormat);
	status |= AddMcmsRequestsToGkIfTaskFormat(H323_CS_RAS_BRQ_RESPONSE_REQ, &gkDsReqBRQResponseFormat);
	status |= AddMcmsRequestsToGkIfTaskFormat(H323_CS_RAS_REMOVE_GK_CALL_REQ, &gkDsReqRemoveCallFormat);
	status |= AddMcmsRequestsToGkIfTaskFormat(H323_CS_RAS_RAI_REQ, &gkDsReqRAIFormat);

    if (status) {
        XmlPrintLog("InitMcmsReqFormatTable", PERR, status);
    }

    return status;

}// InitMcmsReqFormatTable

//---------------------------------------------------------------------------
//
//  Function name:  InitServiceIndFormatTable
//
//  Description:    Init format table
//
//  Return code:
//                  0                   - success
//                  negative value      - error
//---------------------------------------------------------------------------
static int InitServiceIndFormatTable()
{
    int status = 0;

    status = CreateFastIndexTable(
        &pServiceIndicationsFormatTable,
        CS_FIRST_SERVICE_IND,
        CS_LAST_SERVICE_IND - CS_FIRST_SERVICE_IND);

    if(status) {
        XmlPrintLog("CreateFastIndexTable", PERR, status);
        return status;
    }

    status |= AddServiceIndicationsFormat(CS_NEW_SERVICE_INIT_IND, &srCsNewServiceInitIndFormat);
    status |= AddServiceIndicationsFormat(CS_COMMON_PARAM_IND,     &srCsCommonParamIndFormat);
    status |= AddServiceIndicationsFormat(CS_END_SERVICE_INIT_IND, &srCsEndServiceInitIndFormat);
    status |= AddServiceIndicationsFormat(CS_DEL_SERVICE_IND,      &srCsDelServiceIndFormat);
    status |= AddServiceIndicationsFormat(CS_PING_IND,      &srCsPingIndFormat);

    if (status) {
        XmlPrintLog("InitServiceReqFormatTable", PERR, status);
    }

    return status;

}// InitServiceIndFormatTable

//---------------------------------------------------------------------------
//
//	Function name:	InitServiceReqFormatTable
//
//	Description:	Init format table
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
static int InitServiceReqFormatTable()
{
	int status = 0;

	status = CreateFastIndexTable(
		&pServiceRequestsFormatTable,
		CS_FIRST_SERVICE_REQ,
		CS_LAST_SERVICE_REQ - CS_FIRST_SERVICE_REQ);

    if(status) {
        XmlPrintLog("CreateFastIndexTable", PERR, status);
        return status;
    }

	status |= AddServiceRequestsFormat(CS_NEW_SERVICE_INIT_REQ, &srCsNewServiceInitReqFormat);
	status |= AddServiceRequestsFormat(CS_COMMON_PARAM_REQ,		&srCsCommonParamReqFormat);
	status |= AddServiceRequestsFormat(CS_END_SERVICE_INIT_REQ,	&srCsEndServiceInitReqFormat);
	status |= AddServiceRequestsFormat(CS_DEL_SERVICE_REQ,		&srCsDelServiceReqFormat);
    status |= AddServiceRequestsFormat(CS_TERMINAL_COMMAND_REQ, &srCsTerminalCommandReqFormat);
    status |= AddServiceRequestsFormat(CS_PING_REQ, &srCsPingReqFormat);

	if (status) {
		XmlPrintLog("InitServiceReqFormatTable", PERR, status);
	}

	return status;

}// InitServiceReqFormatTable


//---------------------------------------------------------------------------
//
//	Function name:	InitStartupReqFormatTable
//
//	Description:	Init format table
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
static int InitStartupReqFormatTable()
{
	int status = 0;

	status = CreateFastIndexTable(
		&pStartupRequestsFormatTable,
		CS_FIRST_STARTUP_REQ,
		CS_LAST_STARTUP_REQ - CS_FIRST_STARTUP_REQ);

	if(status) {
		XmlPrintLog("CreateFastIndexTable", PERR, status);
		return status;
	}

	status |= AddStartupRequestsFormat(CS_NEW_REQ,				&suCsNewReqFormat);
	status |= AddStartupRequestsFormat(CS_CONFIG_PARAM_REQ,		&suCsConfigParamReqFormat);
	status |= AddStartupRequestsFormat(CS_END_CONFIG_PARAM_REQ,	&suCsEndConfigParamReqFormat);
	status |= AddStartupRequestsFormat(CS_LAN_CFG_REQ,			&suCsLanConfigReqFormat);
    status |= AddStartupRequestsFormat(CS_RECONNECT_REQ,        &suCsReconnectReqFormat);
    status |= AddStartupRequestsFormat(CS_KEEP_ALIVE_REQ,       &suCsKeepAliveReqFormat);

	if(status) {
		XmlPrintLog("InitStartupReqFormatTable", PERR, status);
	}

	return status;

}// InitStartupReqFormatTable

//---------------------------------------------------------------------------
//
//	Function name:	InitStartupIndFormatTable
//
//	Description:	Init format table
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
static int InitStartupIndFormatTable()
{
	int status = 0;

	status = CreateFastIndexTable(
		&pStartupIndicationsFormatTable,
		CS_FIRST_STARTUP_IND,
		CS_LAST_STARTUP_IND - CS_FIRST_STARTUP_IND);

	if (status) {
		XmlPrintLog("CreateFastIndexTable", PERR, status);
		return status;
	}

	status |= AddStartupIndicationsFormat(CS_NEW_IND,				&suCsNewIndFormat);
	status |= AddStartupIndicationsFormat(CS_CONFIG_PARAM_IND,		&suCsConfigParamIndFormat);
	status |= AddStartupIndicationsFormat(CS_END_CONFIG_PARAM_IND,	&suCsEndConfigParamIndFormat);
	status |= AddStartupIndicationsFormat(CS_END_CS_STARTUP_IND,    &suCsEndStartupIndFormat);
    status |= AddStartupIndicationsFormat(CS_RECONNECT_IND,         &suCsReconnectIndFormat);
    status |= AddStartupIndicationsFormat(CS_KEEP_ALIVE_IND,       	&suCsKeepAliveIndFormat);
    status |= AddStartupIndicationsFormat(CS_COMP_STATUS_IND,      	&suCsCompStatusIndFormat);
    status |= AddStartupIndicationsFormat(CS_USER_MSG_IND,      	&suCSActiveAlarmIndFormat);

	if(status) {
		XmlPrintLog("AddStartupIndicationsFormat", PERR, status);
	}

	return status;

}// InitStartupIndFormatTable

//---------------------------------------------------------------------------
//
//	Function name:	InitSipTaskReqFormatTable
//
//	Description:	Init SIP task requests format table
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
static int InitSipTaskReqFormatTable()
{
	int status = 0;

	status = CreateFastIndexTable(
		&pSipTaskRequestsFormatTable,
		SIP_CS_SIG_FIRST_REQ,
		SIP_CS_SIG_LAST_REQ - SIP_CS_SIG_FIRST_REQ);

	if (status) {
		XmlPrintLog("CreateFastIndexTable", PERR, status);
		return status;
	}

	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_INVITE_REQ,			&mcDsReqInviteFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_REINVITE_REQ,			&mcDsReqReInviteFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_INVITE_ACK_REQ,		&mcDsReqInviteAckFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_INVITE_RESPONSE_REQ,	&mcDsReqInviteResponseFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_RINGING_REQ,			&mcDsReqRingingFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_CANCEL_REQ,			&mcDsReqCancelFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_BYE_REQ,				&mcDsReqByeFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_BYE_200_OK_REQ,		&mcDsReqBye200OkFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_VIDEO_FAST_UPDATE_REQ,&mcDsReqVideoFastUpdateFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_DEL_NEW_CALL_REQ,		&mcDsReqDelNewCallFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_INSTANT_MESSAGE_REQ,	&mcDsReqInstantMessageFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_REDIRECT_DATA_REQ,	&mcDsReqRedirectDataFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_OPTIONS_RESP_REQ,		&mcDsReqOptionsRespFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_REFER_RESP_REQ,		&mcDsReqReferRespFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_PARTY_KEEP_ALIVE_REQ,		&mcDsReqSipKeepAliveFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_SUBSCRIBE_RESP_REQ,	&mcDsReqSubscribeRespFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_INFO_REQ, 			&mcDsReqInfoFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_SIG_INFO_RESP_REQ,		&mcDsReqInfoRespFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_BFCP_MESSAGE_REQ, 		&mcDsReqBfcpMessageFormat);
  status |= AddSipTaskRequestsFormat(SIP_CS_SIG_DIALOG_RECOVERY_REQ,     &mcDsReqDialogRecoveryFormat);
  status |= AddSipTaskRequestsFormat(SIP_CS_SIG_SEND_CRLF_REQ,     &mcDsReqSendCrlfFormat);
  status |= AddSipTaskRequestsFormat(SIP_CS_SIG_SOCKET_ACTIVITY_REQ,     &mcDsReqSocketStatisticsFormat);
	status |= AddSipTaskRequestsFormat(SIP_CS_CCCP_SIG_INVITE_REQ,        &cccpInviteReqFormat);

	if(status) {
		XmlPrintLog("AddSipTaskRequestsFormat", PERR, status);
	}

	return status;

}//InitSipTaskReqFormatTable

//---------------------------------------------------------------------------
//
//	Function name:	InitSipTaskIndFormatTable
//
//	Description:	Init SIP task indications format table
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
static int InitSipTaskIndFormatTable()
{
	int status = 0;

	status = CreateFastIndexTable(
		&pSipTaskIndicationsFormatTable,
		SIP_CS_SIG_FIRST_IND,
		SIP_CS_SIG_LAST_IND - SIP_CS_SIG_FIRST_IND);

	if(status) {
		XmlPrintLog("CreateFastIndexTable", PERR, status);
		return status;
	}

	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_INVITE_IND,			&spDsIndInviteFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_REINVITE_IND,			&spDsIndReinviteFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_INVITE_ACK_IND,		&spDsIndInviteAckFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_INVITE_RESPONSE_IND,	&spDsIndInviteResponseFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_PROV_RESPONSE_IND,		&spDsIndProvResponseFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_BYE_IND,				&spDsIndByeFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_BYE_200_OK_IND,		&spDsIndBye200OkFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_CANCEL_IND,			&spDsIndCancelFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_VIDEO_FAST_UPDATE_IND,	&spDsIndVideoFastUpdateFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_DTMF_DIGIT_IND,		&spDsIndDtmfDigitFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_INSTANT_MESSAGE_IND,	&spDsIndInstantMessageFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_OPTIONS_IND,			&spDsIndOptionsFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_REFER_IND,				&spDsIndReferFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_SESSION_TIMER_EXPIRED_IND,	&spDsIndSessionTimerExpiredFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_SESSION_TIMER_REINVITE_IND, &spDsIndSessionTimerReinviteFormat);
 	status |= AddSipTaskIndicationsFormat(SIP_CS_PARTY_KEEP_ALIVE_IND,		&spDsIndKeepAliveFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_TRACE_INFO_IND,		&spDsIndTraceInfoFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_BAD_STATUS_IND,		&spDsIndBadStatusFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_TRANSPORT_ERROR_IND,	&spDsIndTransportErrorFormat);
 	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_CARD_DATA_IND,			&spDsIndCardDataFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_SUBSCRIBE_IND,			&spDsIndSubscribeFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_INFO_IND, 	            &spDsIndInfoFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_INFO_RESP_IND, 	    &spDsIndInfoRespFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_BFCP_MESSAGE_IND,			&spDsIndBfcpMessageFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_BFCP_TRANSPORT_IND,		&spDsIndBfcpTransportFormat);
  status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_DIALOG_RECOVERY_IND,    &spDsIndDialogRecoveryFormat);
  status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_SOCKET_ACTIVITY_IND,    &spDsIndSocketStatisticsFormat);
  status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_CRLF_ERR_IND,    &spDsIndSigCrlfErrorFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_NOTIFY_RESPONSE_IND,	&spDsIndNotifyRespFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_CCCP_SIG_INVITE_IND,								&cccpIndInviteFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_CCCP_SIG_ADD_USER_INVITE_RESPONSE_IND,			&cccpIndAddUserInviteResponseFormat);

	//-S- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------//
	//status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_BENOTIFY_IND,								&spDsIndBenotifyFormat);
	status |= AddSipTaskIndicationsFormat(SIP_CS_SIG_BENOTIFY_IND,									&spDsIndBenotifyFormat_Part);
	//-E- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------//


	if(status) {
		XmlPrintLog("AddSipTaskIndicationsFormat", PERR, status);
	}

	return status;

}//InitSipTaskIndFormatTable

//_mccf_
//---------------------------------------------------------------------------
//
//	Function name:	InitMccfReqFormatTable
//
//	Description:	Init Mccf requests format table
//
//	Return code:Ack
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
static int InitMccfReqFormatTable()
{
	int status = 0;

	status = CreateFastIndexTable(
		&pMccfRequestsFormatTable,
		SIP_CS_MCCF_SIG_FIRST_REQ,
		SIP_CS_MCCF_SIG_LAST_REQ - SIP_CS_MCCF_SIG_FIRST_REQ);

	if (status) {
		XmlPrintLog("CreateFastIndexTable", PERR, status);
		return status;
	}

	status |= AddMccfRequestsFormat(SIP_CS_MCCF_SIG_INVITE_RESPONSE_REQ, &mccfDsReqInviteResponseFormat);
	status |= AddMccfRequestsFormat(SIP_CS_MCCF_SIG_BYE_200_OK_REQ, 	 &mccfDsReqBye200OkFormat);
	status |= AddMccfRequestsFormat(SIP_CS_MCCF_SIG_BYE_REQ,			 &mccfDsReqByeFormat);

	if(status) {
		XmlPrintLog("AddMccfRequestsFormat", PERR, status);
	}

	return status;

}//InitMccfReqFormatTable

//_mccf_
//---------------------------------------------------------------------------
//
//	Function name:	InitMccfIndFormatTable
//
//	Description:	Init Mccf indications format table
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
static int InitMccfIndFormatTable()
{
	int status = 0;

	status = CreateFastIndexTable(
		&pMccfIndicationsFormatTable,
		SIP_CS_MCCF_SIG_FIRST_IND,
		SIP_CS_MCCF_SIG_LAST_IND - SIP_CS_MCCF_SIG_FIRST_IND);

	if(status) {
		XmlPrintLog("CreateFastIndexTable", PERR, status);
		return status;
	}

	status |= AddMccfIndicationsFormat(SIP_CS_MCCF_SIG_INVITE_IND,					&mccfIndInviteFormat);
	status |= AddMccfIndicationsFormat(SIP_CS_MCCF_SIG_INVITE_ACK_IND,				&mccfIndInviteAckFormat);
	status |= AddMccfIndicationsFormat(SIP_CS_MCCF_SIG_BYE_IND,						&mccfIndByeFormat);
	status |= AddMccfIndicationsFormat(SIP_CS_MCCF_SIG_BYE_200_OK_IND,				&mccfIndBye200OkFormat);
	status |= AddMccfIndicationsFormat(SIP_CS_MCCF_SIG_SESSION_TIMER_EXPIRED_IND,	&mccfIndSessionTimerExpiredFormat);
	status |= AddMccfIndicationsFormat(SIP_CS_MCCF_SIG_BAD_STATUS_IND,				&mccfIndBadStatusFormat);
	status |= AddMccfIndicationsFormat(SIP_CS_MCCF_SIG_TRANSPORT_ERROR_IND,			&mccfIndTransportErrorFormat);


	if(status) {
		XmlPrintLog("AddMccfIndicationsFormat", PERR, status);
	}

	return status;

}//InitMccfIndFormatTable


//---------------------------------------------------------------------------
//
//	Function name:	InitProxyReqFormatTable
//
//	Description:	Init PROXY requests format table
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
static int InitProxyReqFormatTable()
{
	int status = 0;

	status = CreateFastIndexTable(
		&pProxyRequestsFormatTable,
		SIP_CS_PROXY_FIRST_REQ,
		SIP_CS_PROXY_LAST_REQ - SIP_CS_PROXY_FIRST_REQ);

	if (status) {
		XmlPrintLog("CreateFastIndexTable", PERR, status);
		return status;
	}

	status |= AddProxyRequestsFormat(SIP_CS_PROXY_REGISTER_REQ,			&mcDsReqRegisterFormat);
	status |= AddProxyRequestsFormat(SIP_CS_PROXY_SUBSCRIBE_REQ,		&mcDsReqSubscribeFormat);
	status |= AddProxyRequestsFormat(SIP_CS_PROXY_UNKNOWN_METHOD_REQ,	&mcDsReqUnknownMethodFormat);
	status |= AddProxyRequestsFormat(SIP_CS_PROXY_NOTIFY_REQ,			&mcDsReqNotifyFormat);
	status |= AddProxyRequestsFormat(SIP_CS_PROXY_SERVICE_REQ,			&mcDsReqServiceFormat);
  status |= AddProxyRequestsFormat(SIP_CS_PROXY_SEND_CRLF_REQ,     &mcDsReqSendCrlfFormat);

	if(status) {
		XmlPrintLog("AddProxyRequestsFormat", PERR, status);
	}

	return status;

}//InitProxyReqFormatTable

//---------------------------------------------------------------------------
//
//	Function name:	InitProxyIndFormatTable
//
//	Description:	Init PROXY indications format table
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
static int InitProxyIndFormatTable()
{
	int status = 0;

	status = CreateFastIndexTable(
		&pProxyIndicationsFormatTable,
		SIP_CS_PROXY_FIRST_IND,
		SIP_CS_PROXY_LAST_IND - SIP_CS_PROXY_FIRST_IND);

	if(status) {
		XmlPrintLog("CreateFastIndexTable", PERR, status);
		return status;
	}

	status |= AddProxyIndicationsFormat(SIP_CS_PROXY_REGISTER_RESPONSE_IND,		&prxDsIndRegisterRespFormat);
	status |= AddProxyIndicationsFormat(SIP_CS_PROXY_SUBSCRIBE_RESPONSE_IND,	&prxDsIndSubscribeRespFormat);
	status |= AddProxyIndicationsFormat(SIP_CS_PROXY_NOTIFY_IND,				&prxDsIndNotifyFormat);
	status |= AddProxyIndicationsFormat(SIP_CS_PROXY_TRACE_INFO_IND,			&prxDsIndTraceInfoFormat);
	status |= AddProxyIndicationsFormat(SIP_CS_PROXY_BAD_STATUS_IND,			&prxDsIndBadStatusFormat);
	status |= AddProxyIndicationsFormat(SIP_CS_PROXY_TRANSPORT_ERROR_IND,		&prxDsIndTransportErrorFormat);
 	status |= AddProxyIndicationsFormat(SIP_CS_PROXY_CARD_DATA_IND,				&prxDsIndCardDataFormat);
 	status |= AddProxyIndicationsFormat(SIP_CS_PROXY_SERVICE_RESPONSE_IND,		&prxDsIndServiceRespFormat);
  status |= AddProxyIndicationsFormat(SIP_CS_PROXY_CRLF_ERR_IND,    &prxDsIndProxyCrlfErrorFormat);

	if(status) {
		XmlPrintLog("AddProxyIndicationsFormat", PERR, status);
	}

	return status;

}//InitProxyIndFormatTable

//---------------------------------------------------------------------------
//
//	Function name:	InitDnsReqFormatTable
//
//	Description:	Init DNS requests format table
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
static int InitDnsReqFormatTable()
{
	int status = 0;

	status = CreateFastIndexTable(
		&pDnsRequestsFormatTable,
		DNS_CS_FIRST_REQ,
		DNS_CS_LAST_REQ - DNS_CS_FIRST_REQ);

	if(status) {
		XmlPrintLog("CreateFastIndexTable", PERR, status);
		return status;
	}

	status |= AddDnsRequestsFormat(DNS_CS_RESOLVE_REQ,	&srDnsCsResolveReqFormat);
	status |= AddDnsRequestsFormat(DNS_CS_SERVICE_REQ,	&srDnsCsServiceReqFormat);

	if(status) {
		XmlPrintLog("AddDnsRequestsFormat", PERR, status);
	}

	return status;

}//InitDnsReqFormatTable

//---------------------------------------------------------------------------
//
//	Function name:	InitDnsIndFormatTable
//
//	Description:	Init DNS indications format table
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
static int InitDnsIndFormatTable()
{
	int status = 0;

	status = CreateFastIndexTable(
		&pDnsIndicationsFormatTable,
		DNS_CS_FIRST_IND,
		DNS_CS_LAST_IND - DNS_CS_FIRST_IND);

	if(status) {
		XmlPrintLog("CreateFastIndexTable", PERR, status);
		return status;
	}

	status |= AddDnsIndicationsFormat(DNS_CS_RESOLVE_IND,	&srDnsCsResolveIndFormat);
	status |= AddDnsIndicationsFormat(DNS_CS_SERVICE_IND,	&srDnsCsServiceIndFormat);

	if(status) {
		XmlPrintLog("AddDnsIndicationsFormat", PERR, status);
	}

	return status;

}//InitDnsIndFormatTable

//---------------------------------------------------------------------------
static int InitFormatTables()
{
	int		status		= 0;

	status |= InitStartupIndFormatTable();
	status |= InitStartupReqFormatTable();
	status |= InitServiceReqFormatTable();
    status |= InitServiceIndFormatTable();
    status |= InitCsIndFormatTable();
	status |= InitMcmsReqFormatTable();
	status |= InitSipTaskIndFormatTable();
	status |= InitSipTaskReqFormatTable();
	status |= InitProxyIndFormatTable();
	status |= InitProxyReqFormatTable();
	status |= InitDnsIndFormatTable();
	status |= InitDnsReqFormatTable();
	status |= InitMccfIndFormatTable(); //_mccf_
	status |= InitMccfReqFormatTable(); //_mccf_
	//status |= InitCccpIndFormatTable(); // cccp
	
	return status;

}//InitFormatTables

//---------------------------------------------------------------------------
//
//	Function name:	InitXmlEngine
//
//	Description:	Initiliaze xml engine
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
int InitXmlEngine()
{
	int status;

#ifdef __CS__
	if (status = XmlPrintLogInit()) {
		return status;
	}
#endif

	status = InitFormatTables();
	if (status) {

		XmlPrintLog("InitFormatTables fail, status = %x", PERR, status);
		return status;
	}

	pTxXmlBuffer = (char *) calloc(XmlMessageLen, 1);

	if (!pTxXmlBuffer) {
		XmlPrintLog("pTxXmlBuffer is NULL", PERR);
		return E_XML_NULL_PTR;
	}

	pRxXmlBuffer = (char *) calloc(XmlMessageLen, 1);

	if (!pRxXmlBuffer) {
		XmlPrintLog("pRxXmlBuffer is NULL", PERR);
		return E_XML_NULL_PTR;
	}

	pZipXmlBuffer = (char *) calloc(CompressXmlBufferSize, 1);

	if (!pZipXmlBuffer) {
		XmlPrintLog("pZipXmlBuffer is NULL", PERR);
		return E_XML_NULL_PTR;
	}

	pUnzipXmlBuffer = (char *) calloc(UncompressXmlBufferSize, 1);

	if (!pUnzipXmlBuffer) {
		XmlPrintLog("pUnzipXmlBuffer is NULL", PERR);
		return E_XML_NULL_PTR;
	}

	return 0;
}

//---------------------------------------------------------------------------
//
//	Function name:	GetXmlMessageLen
//
//	Description:	Get the request xml message length
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
int GetXmlMessageLen(
	char	*pMessage,
	int		*pXmlMessageLen)
{
	char			sOpcode[MaxOpcodeNameLen];
	char			opcodeName[64];
	char			*pBinToPrint;

	int 			status;
	int				binMsgSize;
	int				beginXmlLine;
	int				opcodeNameLen;
	int				curXmlLen = 0;
	int				*pCurXmlLen = &curXmlLen;

	COMMON_HEADER_S *pBinMessage;

	unsigned long	opcode = 0;
	genXmlFormat	*pFormat;

	pBinMessage = (COMMON_HEADER_S *) pMessage;

//	memset(
//		pTxXmlBuffer,
//		0,
//		XmlMessageLen);

	if (!pBinMessage) {

		XmlPrintLog("pMessage is null", PERR);
		status = E_XML_NULL_PTR;
		goto errorExit;
	}

	opcode = pBinMessage->opcode;

	if (IsStartupIndication(opcode)) {

		status = FastIndexTableGetByIndex(
						pStartupIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	} else if (IsStartupRequest(opcode)) {

		status = FastIndexTableGetByIndex(
						pStartupRequestsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	} else if (IsServiceRequest(opcode)) {

		status = FastIndexTableGetByIndex(
						pServiceRequestsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	} else if (IsServiceIndication(opcode)) {

		status = FastIndexTableGetByIndex(
						pServiceIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	} else if (IsMcmsRequestToCallTask(opcode)) {

		status = FastIndexTableGetByIndex(
						pMcmsRequestsToCallTaskFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	} else if (IsMcmsRequestToGkIfTask(opcode)) {

		status = FastIndexTableGetByIndex(
						pMcmsRequestsToGkIfTaskFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	} else if (IsCallTaskIndication(opcode)) {

		status = FastIndexTableGetByIndex(
						pCallTaskIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	} else if (IsGkIfTaskIndication(opcode)) {

		status = FastIndexTableGetByIndex(
						pGkIfTaskIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	} else if (IsSipTaskIndication(opcode)) {

		status = FastIndexTableGetByIndex(
						pSipTaskIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	} else if (IsSipTaskRequest(opcode)) {

		status = FastIndexTableGetByIndex(
						pSipTaskRequestsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	} else if (IsProxyIndication(opcode)) {

		status = FastIndexTableGetByIndex(
						pProxyIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	} else if (IsProxyRequest(opcode)) {

		status = FastIndexTableGetByIndex(
						pProxyRequestsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	} else if (IsDnsIndication(opcode)) {

		status = FastIndexTableGetByIndex(
						pDnsIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	} else if (IsDnsRequest(opcode)) {

		status = FastIndexTableGetByIndex(
						pDnsRequestsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	} else if (IsMccfIndication(opcode)) { //_mccf_

		status = FastIndexTableGetByIndex(
						pMccfIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	} else if (IsMccfRequest(opcode)) { //_mccf_

		status = FastIndexTableGetByIndex(
						pMccfRequestsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status){
			goto errorExit;
		}

	}  else {

		XmlPrintLog("opcode %d not found", PERR, opcode);
		status = E_XML_OPCODE_NOT_FOUND;
        goto errorExit;

	}

//	if (opcode == H323_CS_SIG_CAPABILITIES_IND) {
//		int a = 0;
//	}

	if (xmlTraceLevel >= emXmlTraceMessages) {

		binMsgSize = pBinMessage->payload_len;

		pBinToPrint = XmlPrintBytes((char *)pBinMessage, binVarType, binMsgSize);

		XmlPrintLog("%s", PLOG,"\n-----------------------------------------------------------------------------------\n");
		XmlPrintLog("\nBIN --> XML\nBinary Message:\n%s\n", PLOG, pBinToPrint);
		XmlPrintLog("\nXML message:\n",PLOG);
	}

	if (MaxOpcodeNameLen - 1 < strlen(pFormat->name))
		opcodeNameLen = MaxOpcodeNameLen - 1;
	else
		opcodeNameLen = strlen(pFormat->name);

	strncpy(
		&sOpcode[0],
		pFormat->name,
		opcodeNameLen);

	sOpcode[opcodeNameLen] = '\0';

	if (IsXmlBufferHasSpace(*pCurXmlLen, sOpcode, NULL, startRootType, 0)) {

		AddXmlRootStart(pTxXmlBuffer, sOpcode, &pCurXmlLen);

	} else {

		status = E_XML_XML_BUFFER_TOO_SHORT;
		goto errorExit;
	}

	status = GenXmlBuilder(
			(char**) &pBinMessage,
			(genXmlFormat**)&pFormat,
			(char*) pTxXmlBuffer,
			0,
			1,
			pCurXmlLen);
	if (status) {

		XmlPrintLog("opcode = %s [%d], bin msg = 0x%8X, xml msg = 0x%8X", PERR, sOpcode, opcode, pMessage, pTxXmlBuffer);
		goto errorExit;
	}

	if (IsXmlBufferHasSpace(*pCurXmlLen, sOpcode , NULL, endRootType, 0)) {
		AddXmlRootEnd(pTxXmlBuffer, sOpcode, &pCurXmlLen);

	} else {

		status = (E_XML_XML_BUFFER_TOO_SHORT + 11);
		goto errorExit;
	}

	*pXmlMessageLen = curXmlLen;

	pTxXmlBuffer[*pXmlMessageLen] = '\0';

	if (*pXmlMessageLen >= XmlMessageLen) {
		XmlPrintLog("for opcode = %d, msg len too big[%d]", PERR, opcode, *pXmlMessageLen);
		status = (E_XML_XML_BUFFER_TOO_SHORT + 12);
		goto errorExit;
	}

	if (bZipUnzip) {
		status = XmlZipMessage(pXmlMessageLen, sOpcode);
		if (status) {
			XmlPrintLog("XmlZipMessage failed", PERR);
			return status;
		}
	}

	if (xmlTraceLevel >= emXmlTraceMessages) {

		if (xmlTraceLevel == emXmlTraceMessages)
			XmlPrintLog("\n%s\n", PLOG, pTxXmlBuffer);

		XmlPrintLog("\nGen Xml message, opcode = %d, binary size = %d, unzip xml len = %d, zip xml len = %d\n",
			PLOG,
			opcode,
			binMsgSize,
			curXmlLen,
			*pXmlMessageLen);
		XmlPrintLog("%s", PLOG,"\n-----------------------------------------------------------------------------------\n");
	}

	return 0;

errorExit:

	XmlPrintLog("opcode  = %d, status = %x", PERR, opcode, status);

	return status;

}// GetXmlMessageLen


//---------------------------------------------------------------------------
//
//	Function name:	GetXmlMessage
//
//	Description:	Get the xml message from the binary message
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
int GetXmlMessage(
	int		xmlMessageLen,	// IN
	char	*pXmlMessage)	// OUT
{
	int status;

	if (!pXmlMessage) {
		XmlPrintLog("pXmlMessage is null", PERR);
		return E_XML_NULL_PTR;
	}

	if (bZipUnzip) {

	memcpy(
		pXmlMessage,
		pTxXmlBuffer,
		MaxUnzipHeaderSize);

	memcpy(
		pXmlMessage + MaxUnzipHeaderSize,
		pZipXmlBuffer,
		xmlMessageLen - MaxUnzipHeaderSize);

	} else {

		memcpy(
			pXmlMessage,
			pTxXmlBuffer,
			xmlMessageLen);
	}

	return 0;

}// GetXmlMessage

//---------------------------------------------------------------------------
//
//	Function name:	GetBinMessageSize
//
//	Description:	Get the request binary message size to enable buffer allocation
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
int GetBinMessageSize(
	char	*pXmlMessage,		// IN
	int		*pBinMessageSize,	// OUT
	int		*pOpcode)			// OUT
{
	//unsigned long	opcode;

	if (!pXmlMessage) {

		XmlPrintLog("pXmlMessage is null", PERR);
		return E_XML_NULL_PTR;
	}

	*pOpcode = GetXmlVal(pXmlMessage, "opcode");

	*pBinMessageSize = GetXmlVal(pXmlMessage, "payload_len");

	if (*pBinMessageSize <= 0) {
		XmlPrintLog("opcode = %d, size = %d, size error", PERR, *pOpcode, *pBinMessageSize);
		return E_XML_BIN_SIZE_TOO_SHORT;
	}

	return 0;

} // GetBinMessageSize

//---------------------------------------------------------------------------
//
//	Function name:	GetBinMessage
//
//	Description:	Get the binary message from the xml message
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------

int GetBinMessage(
	char	*pXmlMessage,			// IN
	int		rcvXmlMessageSize,		// IN
	char	*pBinMessage)			// OUT
{
	int				i;
	int				status;
	int				binMsgSize;
	int				curBinMsgSize	= 0;

	int				nBytesWritten;

	char			opcodeName[64];
	char			*pTempBin;
	char			*pTempXml;
	char 			*pBinToPrint;
	char			*pXmlToPrint;
	unsigned long   opcode;

	int				xmlLen 			= 0;
	int				unzipMessageLen = 0;

	genXmlFormat	*pFormat;

	if (!pBinMessage) {
		XmlPrintLog("pMessage is null", PERR);
		return E_XML_NULL_PTR;
	}

	if (!pXmlMessage) {
		XmlPrintLog("pMessage is null", PERR);
		return E_XML_NULL_PTR;
	}

	opcode = GetXmlVal(pXmlMessage, "opcode");

	if (IsStartupIndication(opcode)) {

		status = FastIndexTableGetByIndex(
						pStartupIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

	} else if (IsStartupRequest(opcode)) {

		status = FastIndexTableGetByIndex(
						pStartupRequestsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

 	} else if (IsServiceRequest(opcode)) {

 		status = FastIndexTableGetByIndex(
 						pServiceRequestsFormatTable,
 						(unsigned long)opcode,
 						(unsigned long*)&pFormat,
 						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

	} else if (IsServiceIndication(opcode)) {

		status = FastIndexTableGetByIndex(
						pServiceIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

	} else if (IsMcmsRequestToCallTask(opcode)) {

		status = FastIndexTableGetByIndex(
						pMcmsRequestsToCallTaskFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

	} else if (IsMcmsRequestToGkIfTask(opcode)) {

		status = FastIndexTableGetByIndex(
						pMcmsRequestsToGkIfTaskFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

	} else if (IsCallTaskIndication(opcode)) {

		status = FastIndexTableGetByIndex(
						pCallTaskIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

	} else if (IsGkIfTaskIndication(opcode)) {

		status = FastIndexTableGetByIndex(
						pGkIfTaskIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

	} else if (IsSipTaskIndication(opcode)) {

		status = FastIndexTableGetByIndex(
						pSipTaskIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

	} else if (IsSipTaskRequest(opcode)) {

		status = FastIndexTableGetByIndex(
						pSipTaskRequestsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

	} else if (IsProxyIndication(opcode)) {

		status = FastIndexTableGetByIndex(
						pProxyIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

	} else if (IsProxyRequest(opcode)) {

		status = FastIndexTableGetByIndex(
						pProxyRequestsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

	} else if (IsDnsIndication(opcode)) {

		status = FastIndexTableGetByIndex(
						pDnsIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

	} else if (IsDnsRequest(opcode)) {

		status = FastIndexTableGetByIndex(
						pDnsRequestsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

	} else if (IsMccfIndication(opcode)) { //_mccf_

		status = FastIndexTableGetByIndex(
						pMccfIndicationsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

	} else if (IsMccfRequest(opcode)) { //_mccf_

		status = FastIndexTableGetByIndex(
						pMccfRequestsFormatTable,
						(unsigned long)opcode,
						(unsigned long*)&pFormat,
						&opcodeName[0]);
		if (status) {
			goto errorExit;
		}

	}  else {
		XmlPrintLog("opcode %d not found", PERR, opcode);
		status = E_XML_OPCODE_NOT_FOUND;
        goto errorExit;

	}

//	if (opcode == H323_CS_SIG_CAPABILITIES_IND) {
//		int a = 0;
//	}

	binMsgSize = GetXmlVal(pXmlMessage, "payload_len");

	if (bZipUnzip) {
		status = XmlUnzipMessage(
				pXmlMessage,
				rcvXmlMessageSize,
				&unzipMessageLen);
		if (status) {
			XmlPrintLog("XmlUnzipMessage fail, opcode = %s, status = %d", PERR,
					pFormat->name, status);
			goto errorExit;
		}
	} else {
		memcpy(
			pRxXmlBuffer,
			pXmlMessage,
			rcvXmlMessageSize);
	}


	if (xmlTraceLevel >= emXmlTraceMessages) {

		XmlPrintLog("%s", PLOG,"\n-----------------------------------------------------------------------------------\n");

		XmlPrintLog("\nXML --> BIN\n\n", PLOG);
		XmlPrintLog("Gen Binary message, opcode = %d, binary size = %d, unzip xml len = %d, zip xml len = %d\n",
			PLOG,
			opcode,
			binMsgSize,
			unzipMessageLen,
			rcvXmlMessageSize);
		XmlPrintLog("\nXML Message:\n\n%s\n", PLOG, pRxXmlBuffer);
	}

	pTempBin = (char *) pBinMessage;
	pTempXml = (char *) pRxXmlBuffer;

	status = GenBinBuilder(
			(char**) &pTempBin,
			(genXmlFormat **)&pFormat,
			(char**) &pTempXml,
			1,
			binMsgSize,
			&curBinMsgSize);
	if (status) {

		XmlPrintLog("opcode = %d, xml = 0x%8X", PERR, opcode, pRxXmlBuffer);
		pBinToPrint = XmlPrintBytes(pBinMessage, binVarType, binMsgSize);

		XmlPrintLog("\nBinary:\n%s\n",PERR, pBinToPrint);
		goto errorExit;
	}

	if (xmlTraceLevel >= emXmlTraceMessages) {
		pBinToPrint = XmlPrintBytes(pBinMessage, binVarType, binMsgSize);

		XmlPrintLog("\n\nBinary Message:\n%s\n",PLOG, pBinToPrint);
		XmlPrintLog("%s", PLOG,"\n-----------------------------------------------------------------------------------\n");
	}

	return 0;

errorExit:

	XmlPrintLog("status = %x", PERR, status);
	return status;

}// GetBinMessage


//---------------------------------------------------------------------------
//
//	Function name:	InitXmlTraceLevel
//
//	Description:	For MCMS use - get trace level from ststem.cfg, TRUE = full, FALSE = errors and warnings
//
//	Return code:
//					0					- success
//					negative value		- error
//---------------------------------------------------------------------------
int InitXmlTraceLevel(BOOL bTraceLevel)
{
	if (bTraceLevel)
		xmlTraceLevel = emXmlTraceFull;
	else
		xmlTraceLevel = emXmlTraceErrWrn;

	return 0;

}// InitXmlTraceLevel
