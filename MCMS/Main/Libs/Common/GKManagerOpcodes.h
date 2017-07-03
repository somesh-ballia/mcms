//+========================================================================+
//                  GKManagerOpcodes.h									   |
//					Copyright Polycom							           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GKManagerOpcodes.h                                          |
// SUBSYSTEM:  Libs/Common		                                           |
// PROGRAMMER: Yael A.	                                                   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |   The file includs opcode interface between GkManger |
//						 process and other processes in Carmel (ConfParty, |
//						 CsMngr..)                                         |
//+========================================================================+                        


#ifndef GKMANAGEROPCODES_
#define GKMANAGEROPCODES_

/**********************************************************************************************/
/*							    Interface between GkManger and CsMngr   					  */
/**********************************************************************************************/

/*
//Gatekeeper connection state
#define GATEKEEPER_NON_REGISTERED_STATE   0
#define GATEKEEPER_REGISTRATION_STATE     2
#define GATEKEEPER_REGISTERED_STATE       3
#define REGISTRATION_TO_ALT_GKS_FAILED    6 
#define GK_DNS_HOST_NOT_FOUND			  8
#define GK_DNS_CONNECTIVITY_PROBLEM		  9
#define GK_DNS_INTERAL_ERROR			  10
#define GK_DNS_CONN_REFUSED				  11
*/

#define  MAX_NUM_GK	   		5
#define  USER_NAME_LEN	   		    ONE_LINE_BUFFER_LEN
#define  PASSWORD_LEN	   		ONE_LINE_BUFFER_LEN

// if you add new enumerator's member, you should add string to array and update schema obj_dynamic_ip_service.xsd
enum eGKConnectionState
{
	eGKStateInvalid = 0,
	
	eGKNonRegistrated,
	eGKDiscovery,
	eGKRegistration,
	eGKRegistrated,
	eGKUnRegistration,
	eGKTimeoutRegistration,
	
    eGKNonRegistrated_SecurityDenial,

	NumOfGKConnectionStates
};

static char *GKConnectionStateNames[] = 
{
	"GK_State_Invalid",						    // eGKStateInvalid,
	"GK_Non_Registered", 					    // eGKNonRegistrated,
	"GK_Discovery", 						    // eGKDiscovery,
	"GK_Registration", 						    // eGKRegistration,
	"GK_Registered", 						    // eGKRegistrated,
	"GK_Unregistration", 					    // eGKUnRegistration,
	"GK_Timeout_Registration",				    // eGKTimeoutRegistration,
    "GK_Non_Registered. Authentication failed.",// eGKNonRegistrated_SecurityDenial
};
 

#define CARD_MAJOR_ERROR          0
#define CARD_MINOR_ERROR          1
#define CARD_NORMAL               3

/* faults and card opcodes: */
enum eGkFaultsAndServiceStatus
{
	/* General states: */
	eGkFaultsAndServiceStatusOk = 0,
	eGkFaultDiscovery,
	eGkFaultRegistrationToAltGkFailed,
	eGkFaultRegistrationSucceeded,
	eGkFaultUnregistration,
	eGkFaultRegistreationTimeout, 

	eGKFaultDnsHostNotFound,
/*	eGKFaultDnsConnectivityProblem,
	eGKFaultDnsInternalError,
	eGKFaultDnsConnectionRefused,*/

	/* GRQ reject reasons: */
	eGkFaultGkRejectedGrqReasonIsResourceUnavailable,
	eGkFaultGkRejectedGrqReasonIsInvalidRevision,
	eGkFaultGkRejectedGrqReasonIs18,
	eGkFaultGkRejectedGrqReasonIsTerminalExcluded,
	eGkFaultGkRejectedGrqReasonIsSecurityDenial,
	eGkFaultGkRejectedGrqReasonIsStatusIllegal,
	eGkFaultGkRejectedGrqReasonIsGenericData,
	eGkFaultGkRejectedGrqReasonIsNeededFeatureNotSupported,
	/* RRQ reject reasons: */
	eGkFaultGkRejectedRrqReasonIsResourceUnavailable,
	eGkFaultGkRejectedRrqReasonIsInvalidRevision,
	eGkFaultGkRejectedRrqReasonIsInvalidCallSignalAddress,
	eGkFaultGkRejectedRrqReasonIsInvalidRasAddress,
	eGkFaultGkRejectedRrqReasonIsTerminalType,
	eGkFaultGkRejectedRrqReasonIsEndpointId,
	eGkFaultGkRejectedRrqReasonIsDiscoveryRequired,
	eGkFaultGkRejectedRrqReasonIsDuplicateAlias,
	eGkFaultGkRejectedRrqReasonIsTransportNotSupported,
	eGkFaultGkRejectedRrqReasonIs18,
	eGkFaultGkRejectedRrqReasonIsSecurityDenial,
	eGkFaultGkRejectedRrqReasonIsTransportQOSNotSupported,
	eGkFaultGkRejectedRrqReasonIsInvalidAlias,
	eGkFaultGkRejectedRrqReasonIsStatusIllegal,
	eGkFaultGkRejectedRrqReasonIsFullRegistrationRequired,
	eGkFaultGkRejectedRrqReasonIsGenericData,
	eGkFaultGkRejectedRrqReasonIsAdditiveRegistrationNotSupported,
	eGkFaultGkRejectedRrqReasonIsInvalidTerminalAliases,
	eGkFaultGkRejectedRrqReasonIsNeededFeatureNotSupported,

	NumOfGkFaultsAndServiceStatus //must be last!!!
};


static char *GkFaultsAndServiceStatusNames[] = 
{
	"Status_OK", //should not be printed!!
	/* General states: */
	"GK_Discovery_State",
	"Registration_To_Alternate_GK_Failed",
	"Registration_Succeeded",
	"GK_Unregistration_State" ,
	"GK_Registration_Timeout",
	
	"GK_Dns_Host_Not_Found", 				// eGKDnsHostNotFound,
/*	"GK_Dns_Connectivity_Problem", 			// eGKDnsConnectivityProblem,
	"GK_Dns_Internal_Error", 				// eGKDnsInternalError,
	"GK_Dns_Connection_Refused", 			// eGKDnsConnectionRefused,*/
	
	/* GRQ reject reasons: */
	"GK_Rejected_GRQ_Reason_Is_Resource_Unavailable",
	"GK_Rejected_GRQ_Reason_Is_Invalid_Revision",
	"GK_Rejected_GRQ_Reason_Is_18",
	"GK_Rejected_GRQ_Reason_Is_Terminal_Excluded",
	"GK_Rejected_GRQ_Reason_Is_Security_Denial",
	"GK_Rejected_GRQ_Reason_Is_Status_Illegal",
	"GK_Rejected_GRQ_Reason_Is_Generic_Data",
	"GK_Rejected_GRQ_Reason_Is_Needed_Feature_Not_Supported",
	/* RRQ reject reasons: */
	"GK_Rejected_RRQ_Reason_Is_Resource_Unavailable",
	"GK_Rejected_RRQ_Reason_Is_Invalid_Revision",
	"GK_Rejected_RRQ_Reason_Is_Invalid_Call_Signal_Address",
	"GK_Rejected_RRQ_Reason_Is_Invalid_RAS_Address",
	"GK_Rejected_RRQ_Reason_Is_Terminal_Type",
	"GK_Rejected_RRQ_Reason_Is_Endpoint_Id",
	"GK_Rejected_RRQ_Reason_Is_Discovery_Required",
	"GK_Rejected_RRQ_Reason_Is_Duplicate_Alias",
	"GK_Rejected_RRQ_Reason_Is_Transport_Not_Supported",
	"GK_Rejected_RRQ_Reason_Is_18",
	"GK_Rejected_RRQ_Reason_Is_Security_Denial",
	"GK_Rejected_RRQ_Reason_Is_Transport_QOS_Not_Supported",
	"GK_Rejected_RRQ_Reason_Is_Invalid_Alias",
	"GK_Rejected_RRQ_Reason_Is_Status_Illegal",
	"GK_Rejected_RRQ_Reason_Is_Full_Registration_Required",
	"GK_Rejected_RRQ_Reason_Is_Generic_Data",
	"GK_Rejected_RRQ_Reason_Is_Additive_Registration_Not_Supported",
	"GK_Rejected_RRQ_Reason_Is_Invalid_Terminal_Aliases",
	"GK_Rejected_RRQ_Reason_Is_Needed_Feature_Not_Supported",
	
};

enum eDynamicGkRole
{
	eGKActive = 0,
	eGKBackup,
	
	NumOfGKRoles
};
static const char* DynamicGkRoleNames [] = 
{
	"Active",
	"Backup"
};
 

/**********************************************************************************************/
/*							    Interface between GkManger and ConfParty					  */
/**********************************************************************************************/
//h323 control:
typedef enum
{
	eDefaultStatus,
	eAltGkProcessStatus,
	eRegistrationProblemStatus,
	eArqTimeoutStatus,
}eGkFailOpcode;


#define  REMOVE_GK_CALL	   				 11500
#define  DRQ_IND_OR_FAIL   				 11501 
#define  STOP_IRRTIMER     				 11502
#define  HOLD_GK_REQ       				 11503
#define  RESEND_GK_REQ     				 11504
#define  GK_MANAGER_PARTY_KEEP_ALIVE_REQ 11505
#define  GK_MANAGER_PARTY_KEEP_ALIVE_IND 11505

#define CP_GK_CONF_ID_CLEAN_UP           11506
#define CP_GK_PARTY_ID_CLEAN_UP          11507
#define PARTY_GK_UPDATE_CONF_ID          11508


//lobby
#define  GK_MANAGER_SEARCH_CALL 		 12000
#define  LOBBY_LRQ_RESPONSE     		 12001



/**********************************************************************************************/
/*							    Internal Interface In GkManger Process   					  */
/**********************************************************************************************/
//CGkCall to CGkManager
#define  GK_CALL_NO_RESPONSE_FROM_PARTY	 13000



#define AVF_DEBUG_MODE FALSE


#endif /*GKMANAGEROPCODES_*/
