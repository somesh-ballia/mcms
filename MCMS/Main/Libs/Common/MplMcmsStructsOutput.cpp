#include "MplMcmsStructs.h"

static const char *sUserMsgCode_CS[] =
{
	"SipTLS_RegistrationHandshakeFailure",
	"SipTLS_FailedToLoadOrVerifyCertificateFiles",
	"SipTLS_RegistrationTransportError",
	"SipTLS_RegistrationServerNotResponding",
	"SipTLS_CerificateHasExpired",
	"SipTLS_CertificateWillExpireInLessThanAMonth",
	"SipTLS_CertificateSubjNameIsNotValid_Or_DnsFailed",
	"Cs_EdgeServerDnsFailed",
	"SipTLS_RemoteCertificateFailure",
	"SipTLS_RmxCertificateFailure"
};

static const char *sUserMsgCode_Emb[] =
{
	"Test",
	"UBootFlashFailure",
	"FpgaVersLoadesFailure",
	"FAN_SpeedBelowMinimum",
	"FAN_NoPower",
	"FSM_NoCard",
	"FSM4000_LoadingProblem",
	"FSM4000_NoLinkToCard1",
	"FSM4000_NoLinkToCard2",
	"FSM4000_NoLinkToCard3",
	"FSM4000_NoLinkToCard4",
	"FLASH_Erase",

};


static const char *sUserMsgLocation[] =
{
	"SysAlerts",
	"Faults"
};

static const char *sUserMsgOperation[] =
{
	"add",
	"remove"
};
static const char *sUserMsgAutoRemoval[] =
{
	"none",
	"cardRemove"
};
static const char *sUserMsgProcessType_CS[] =
{
	"sip",
	"H323",
	"Infrastructure"
};
static const char *sUserMsgProcessType_Emb[] =
{
	"1"
};

//////////////////////////////////////////////////////////////
const char *GetUserMsgCode_CsStr(enum eUserMsgCode_CS theCode)
{
	int arraySize = sizeof(sUserMsgCode_CS) / sizeof(sUserMsgCode_CS [0]);

	const char *str = ( 0 <= theCode && theCode < arraySize/*NUM_OF_USER_MSG_MESSAGE_CODES_CS*/
						?
						sUserMsgCode_CS[theCode] : "Invalid" );
	return str;
}

const char *GetUserMsgCodeEmbStr(enum eUserMsgCode_Emb theCode)
{
	int arraySize = sizeof(sUserMsgCode_Emb) / sizeof(sUserMsgCode_Emb [0]);
	const char *str = ( 0 <= theCode && theCode <= arraySize/*NUM_OF_USER_MSG_MESSAGE_CODES_EMB*/
						?
						sUserMsgCode_Emb[theCode] : "Invalid" );
	return str;
}

const char *GetUserMsgLocationStr(enum eUserMsgLocation theLocation)
{
	int arraySize = sizeof(sUserMsgLocation) / sizeof(sUserMsgLocation [0]);

	const char *str = ( 0 <= theLocation && theLocation <= arraySize/*NUM_OF_USER_MSG_LOCATIONS*/
						?
						sUserMsgLocation[theLocation] : "Invalid" );
	return str;
}
const char *GetUserMsgOperationStr(enum eUserMsgOperation theOperation)
{
	int arraySize = sizeof(sUserMsgOperation) / sizeof(sUserMsgOperation [0]);
	const char *str = ( 0 <= theOperation && theOperation <= arraySize /*NUM_OF_USER_MSG_OPERATIONS*/
						?
						sUserMsgOperation[theOperation] : "Invalid" );
	return str;
}

const char *GetUserMsgAutoRemovalStr(enum eUserMsgAutoRemoval theAutoRemoval)
{
	int arraySize = sizeof(sUserMsgAutoRemoval) / sizeof(sUserMsgAutoRemoval [0]);

	const char *str = ( 0 <= theAutoRemoval && theAutoRemoval <= arraySize /*NUM_OF_USER_MSG_AUTO_REMOVALS*/
						?
						sUserMsgAutoRemoval[theAutoRemoval] : "Invalid" );
	return str;
}
const char *GetUserMsgProcessType_CsStr(enum eUserMsgProcessType_CS theType)
{
	int arraySize = sizeof(sUserMsgProcessType_CS) / sizeof(sUserMsgProcessType_CS [0]);

	const char *str = ( 0 <= theType && theType <= arraySize /*NUM_OF_USER_MSG_PROCESS_TYPES_CS*/
						?
						sUserMsgProcessType_CS[theType] : "Invalid" );
	return str;
}

const char *GetUserMsgProcessType_EmbStr(enum eUserMsgProcessType_Emb theType)
{
	int arraySize = sizeof(sUserMsgProcessType_Emb) / sizeof(sUserMsgProcessType_Emb [0]);

	const char *str = ( 0 <= theType && theType <= arraySize /*NUM_OF_USER_MSG_PROCESS_TYPES_EMB*/
						?
						sUserMsgProcessType_Emb[theType] : "Invalid" );
	return str;
}


//////////////////////////////////////////////////////////////



