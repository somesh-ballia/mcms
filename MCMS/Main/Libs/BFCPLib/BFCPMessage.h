#ifndef _BFCP_MESSAGE_H_
#define _BFCP_MESSAGE_H_

//---------------------------------------------------------
//
//  Filename: BFCPMessage.h
//
//  Description: This module defines the BFCP Messaage support
//               for SIP P+C.
//
//  Copyright:   Polycom, Inc. 2007
//  Revision 1 - by Lior Baram 1/4/2008
//   Memory should be allocated outside this module instead
//   the use of static memory as in the original version
//---------------------------------------------------------

//---------------------------------------------------------
//                     INCLUDES
//---------------------------------------------------------
#include "Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

//---------------------------------------------------------
//                     TYPEDEFS & DEFINITIONS
//---------------------------------------------------------

typedef enum {
    kBFCP_Invalid = 0,
    kBFCP_TCP,
    kBFCP_UDP,
    kBFCP_TLS,
    kBFCP_INFO,
    kBFCP_Max,
} eBFCPSignalType;

typedef enum {
    kSetupActive,
    kSetupPassive,
    kSetupActPass,
    kSetupHoldConn
} eBFCPSetup;

typedef enum {
    kConnectionNew,
    kConnectionExisting
} eBFCPConnection;

typedef enum
{
	kBFCPErrorNoError = 0,
	kBFCPErrorConfDoesNotExist,
	kBFCPErrorUserDoesNotExist,
	kBFCPErrorUnknownPrimitive,
	kBFCPErrorUnknownMandatoryAttr,
	kBFCPErrorUnathorizedOp,
	kBFCPErrorInvalidFloorID,
	kBFCPErrorFloorReqIDDoesNotExist,
	kBFCPErrorMaxNbrFloorRequests,
	kBFCPErrorUseTLS,
	kBFCPErrorUnableToParseMessage,
	kBFCPErrorMaximum
} eBFCPErrors;

typedef enum
{
	kBFCPStatusInvalid = 0,
	kBFCPStatusPending,
	kBFCPStatusAccepted,
	kBFCPStatusGranted,
	kBFCPStatusDenied,
	kBFCPStatusCancelled,
	kBFCPStatusReleased,
	kBFCPStatusRevoked,
	kBFCPStatusMaximum
} eBFCPStatusValues;

typedef enum
{
	kBFCPPriorityLowest = 0,
	kBFCPPriorityLow,
	kBFCPPriorityNormal,
	kBFCPPriorityHigh,
	kBFCPPriorityHighest,
	kBFCPPriorityMaximum
} eBFCPPriority;

typedef enum					// Command Direction
{
	kBFCPInvalidPrimitive = 0,
	kBFCPFloorRequest,			// P -> S 								// 1
	kBFCPFloorRelease,			// P -> S 								// 2
	kBFCPFloorRequestQuery,		// P -> S; Ch -> S 						// 3
	kBFCPFloorRequestStatus,	// P <- S; Ch <- S 						// 4
	kBFCPUserQuery,				// P -> S; Ch -> S 						// 5
	kBFCPUserStatus,			// P <- S; Ch <- S 						// 6
	kBFCPFloorQuery,			// P -> S; Ch -> S 						// 7
	kBFCPFloorStatus,			// P <- S; Ch <- S 						// 8
	kBFCPChairAction,			// Ch -> S 								// 9
	kBFCPChairActionAck,		// Ch <- S 								// 10
	kBFCPHello,					// P -> S; Ch -> S 						// 11
	kBFCPHelloAck,				// P <- S; Ch <- S 						// 12
	kBFCPError,					// P <- S; Ch <- S 						// 13
	kBFCPFloorRequestStatusAck, // P -> S ; Ch -> S 					// 14
	kBFCPErrorAck,				// P -> S ; Ch -> S 					// 15
	kBFCPFloorStatusAck,		// P -> S ; Ch -> S 					// 16
	kBFCPGoodbye,				// P -> S ; Ch -> S ; S -> P ; S -> Ch 	// 17
	kBFCPGoodbyeAck,			// P -> S ; Ch -> S ; S -> P ; S -> Ch 	// 18

	//kTandbergBFCPFloorRequestStatusAck,
	//kTandbergBFCPFloorStatusAck,

	kBFCPMaxPrimitive
} eBFCPPrimitive;

typedef enum
{
	kTandbergBFCPFloorRequestStatusAck = 0x5b,
	kTandbergBFCPFloorStatusAck = 0x5d,

} eTandbergBFCPPrimitive;

typedef enum
{
	kBFCPAttrInvalid = 0,
	kBFCPAttrBeneficiaryID,
	kBFCPAttrFloorID,
	kBFCPAttrFloorRequestID,
	kBFCPAttrPriority,
	kBFCPAttrRequestStatus,
	kBFCPAttrErrorCode,
	kBFCPAttrErrorInfo,
	kBFCPAttrPartProvInfo,
	kBFCPAttrStatusInfo,
	kBFCPAttrSupportedAttrs,
	kBFCPAttrSupportedPrimitives,
	kBFCPAttrUserDisplayName,
	kBFCPAttrUserURI,
	kBFCPAttrBeneficiaryInfo,
	kBFCPAttrFloorRequestInfo,
	kBFCPAttrRequestedByInfo,
	kBFCPAttrFloorRequestStatus,
	kBFCPAttrOverallRequestStatus,
	kBFCPAttrMaxAttribute
} eBFCPAttributeID;

typedef enum
{
	kBFCPFloorCtrlInvalid = 0,
	kBFCPFloorCtrlClient,
	kBFCPFloorCtrlServer,
	kBFCPFloorCtrlBoth
} eBFCPFloorCtrlType;

typedef struct BFCP_COMMON_HEADER_S
{
	unsigned int/*char*/ Version : 3;
	unsigned int/*char*/ Reserved : 5;
	unsigned int/*char*/ Primitive : 8;
	UInt16 PayloadLength;	// Expressed in 4-octet units, excluding the common header
	unsigned int ConferenceID;
	UInt16 TransactionID;
	UInt16 UserID;
} BFCPCommonHeader_T;

typedef struct BFCPFloorIDAttr_S
{
	unsigned int/*char*/ AttrType : 7;
	unsigned int/*char*/ Mandatory : 1;
	unsigned int/*char*/ Length:8;
	UInt16 FloorID;
} BFCPFloorIDAttr_T;

typedef struct BFCPFloorReqIDAttr_S
{
	unsigned int/*char*/ AttrType : 7;
	unsigned int/*char*/ Mandatory : 1;
	unsigned int/*char*/ Length:8;
	UInt16 FloorReqID;
} BFCPFloorReqIDAttr_T;

typedef struct BFCPFloorReqInfoHdrAttr_S
{
	unsigned int/*char*/ AttrType : 7;
	unsigned int/*char*/ Mandatory : 1;
	unsigned int/*char*/ Length:8;
	UInt16 FloorReqID;
} BFCPFloorReqInfoHdrAttr_T;

typedef struct BFCPOverallReqStatusHdrAttr_S
{
	unsigned int/*char*/ AttrType : 7;
	unsigned int/*char*/ Mandatory : 1;
	unsigned int/*char*/ Length:8;
	UInt16 FloorReqID;
} BFCPOverallReqStatusHdrAttr_T;

typedef struct BFCPFloorReqStatusAttr_S
{
	unsigned int/*char*/ AttrType : 7;
	unsigned int/*char*/ Mandatory : 1;
	unsigned int/*char*/ Length:8;
	UInt16 FloorID;
} BFCPFloorReqStatusAttr_T;

typedef struct BFCPReqStatusAttr_S
{
	unsigned int/*char*/ AttrType : 7;
	unsigned int/*char*/ Mandatory : 1;
	unsigned short int/*char*/ Length:8;
	unsigned short int/*char*/ Status:8;
	unsigned short int/*char*/ QueuePosition:8;
} BFCPReqStatusAttr_T;


typedef struct BFCP_MESSAGE_S
{
	BFCPCommonHeader_T Header;
} BFCPMessage_T;


typedef struct BFCPFloorInfoS
{
	eBFCPPrimitive	  transactionType;
    UInt32            floorID;
	UInt32			  conferenceID;
	UInt16			  userID;
	UInt16			  transactionID;
	UInt16			  floorRequestID;
	eBFCPStatusValues floorStatus;
	eBFCPPriority	  priority;
	UInt8			  errorCode;
} BFCPFloorInfoT;

//Callback function when data arrives on the BFCP socket
typedef void (*BFCPMsgEvent)(void *pUserDefined, BFCPFloorInfoT *pFloorInfo, eBFCPPrimitive primitive);

typedef struct BFCP_OBJECT_S
{
	UInt32			   streamLabel;
	eBFCPSignalType    signalType;
    eBFCPSignalType    initialOfferSignalType;
    int                listenSockfd;
    int                sockfd;
    char               remoteAddr[256];
	int				   port;
    BFCPMsgEvent       msgRcvdCallback;
    BFCPMsgEvent       connFailCallback;
    BFCPMsgEvent       connUpCallback;
    void               *pUserData;
    eBFCPSetup         setup;
    eBFCPConnection    connection;
	int				   TCPTimeOut;
	eBFCPFloorCtrlType floorCtrlType;
    BFCPFloorInfoT     floorInfo;
    UInt32             lastFakeRoleProvIDTime;  //Last time we sent a role provider ID
    Boolean            blockFloorStatus;  //Flag used to block sending an extra floor status when server sends content
    Boolean            helloSent;  //Flag used to indicate that the HEllo was sent        sipit

} BFCPObject_T;


//---------------------------------------------------------
//               External Functions
//---------------------------------------------------------

//	Initialization
void BFCPInit(BFCPObject_T *pBFCPObject);

int BFCPCreateServerSocket(BFCPObject_T *pBFCP, char * pAddress);
int BFCPCloseServerSocket(BFCPObject_T *pBFCP);
int BFCPCreateConnection(BFCPObject_T *pBFCP, char * pLocalAddress);
int BFCPDisconnect(BFCPObject_T *pBFCP);
int BFCPSendMsg(BFCPObject_T *pBFCPObject, unsigned char * msg, int len);
int BFCPSetMsgHandler(BFCPObject_T *pBFCPObject, void *pUserDefined, BFCPMsgEvent msgEvent);
int BFCPSetConnHandler(BFCPObject_T *pBFCP, void *pUserDefined, BFCPMsgEvent msgEvent);
void BFCPPollForData(BFCPObject_T *pBFCP);
int BFCPSetConnUpHandler(BFCPObject_T *pBFCP, void *pUserDefined, BFCPMsgEvent msgEvent);

//Debug functions
void BFCPDumpMsg(UInt8 * pMsg, int len);

//	Accessor functions
void BFCPSetPort(BFCPObject_T *pBFCPObject, int port);
void BFCPSetConferenceID(BFCPObject_T *pBFCPObject, int ConfID);
void BFCPSetUserID(BFCPObject_T *pBFCPObject, int UserID);
UInt32 BFCPGetFloorStatus(BFCPObject_T *pBFCPObject);

void BFCP_BuildHello(BFCPFloorInfoT *pFloorInfo, UInt8 *pMsg, UInt32 *pMsgLen);
void BFCP_BuildHelloAck(BFCPFloorInfoT *pFloorInfo, UInt8 *pMsg, UInt32 *pMsgLen, char * pRemoteProduct, eBFCPSignalType bfcpTransType);
void BFCP_BuildFloorRequest(BFCPFloorInfoT *pFloorInfo, UInt8 *pMsg, UInt32 *pMsgLen);
void BFCP_BuildFloorRelease(BFCPFloorInfoT *pFloorInfo, UInt8 *pMsg, UInt32 *pMsgLen);
void BFCP_BuildFloorStatus(BFCPFloorInfoT *pFloorInfo, eBFCPStatusValues status, UInt8 *pMsg, UInt32 *pMsgLen, eBFCPSignalType bfcpTransType, Boolean bIsServerInitiator);
void BFCP_BuildFloorStatusAck(BFCPFloorInfoT *pFloorInfo, eBFCPStatusValues status, UInt8 *pMsg, UInt32 *pMsgLen);
void BFCP_BuildFloorRequestStatus(BFCPFloorInfoT *pFloorInfo, eBFCPStatusValues status, UInt8 *pMsg, UInt32 *pMsgLen, eBFCPSignalType bfcpTransType, Boolean bIsServerInitiator);
void BFCP_BuildGoodbye(BFCPFloorInfoT *pFloorInfo, UInt8 *pMsg, UInt32 *pMsgLen);
void BFCP_BuildGoodbyeAck(BFCPFloorInfoT *pFloorInfo, UInt8 *pMsg, UInt32 *pMsgLen, char * pRemoteProduct, eBFCPSignalType bfcpTransType);
void BFCP_ParseMsgPrimitive(BFCPObject_T *pBFCPObject, UInt8 *pMsg, int msgLen);
void BFCPIncrementFloorRequestID(BFCPObject_T *pBFCPObject);

#ifdef __cplusplus
}
#endif	//	__cplusplus

#endif	//	_BFCP_MESSAGE_H_
