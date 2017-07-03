// RtmIsdnMngrInternalStructs.h
//
//////////////////////////////////////////////////////////////////////

#ifndef _RtmIsdnMngrInternalStructs_H__
#define _RtmIsdnMngrInternalStructs_H__


#include "StringsLen.h"
#include "RtmIsdnMngrInternalDefines.h"
#include "SharedDefines.h"


// used in:
//	RTM_ISDN_PARAMS_IND
typedef struct
{
	WORD	boardId;
	DWORD	ipAddress_Rtm;
	DWORD	ipAddress_RtmMedia;
} RTM_ISDN_IP_ADDRESSES_S;

typedef struct
{
	WORD	spanType;
	WORD	serviceType;
	WORD	framing;
	WORD	side;
	WORD	lineCoding;
	WORD	switchType;
} RTM_ISDN_SPAN_DEFINITION_S;


typedef struct
{
    WORD	dialInGroupId;
    BYTE	firstPhoneNumber[ISDN_PHONE_NUMBER_DIGITS_LEN];  
    BYTE	lastPhoneNumber[ISDN_PHONE_NUMBER_DIGITS_LEN];  
    DWORD	category;
    BYTE	firstPortId;
} RTM_ISDN_PHONE_RANGE_S;



typedef struct
{
	BYTE	serviceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN];
	BYTE	dialOutPrefix[ISDN_PHONE_NUMBER_DIGITS_LEN];
	BYTE	mcuCli[ISDN_PHONE_NUMBER_DIGITS_LEN];
	WORD	dfltNumType;
	WORD	numPlan;
	WORD	voice;
	WORD	netSpecFacility;

	RTM_ISDN_SPAN_DEFINITION_S	spanDef;
	RTM_ISDN_PHONE_RANGE_S		phoneRangesList[MAX_ISDN_PHONE_NUMBER_IN_SERVICE];
	RTM_ISDN_IP_ADDRESSES_S		ipAddressesList[MAX_NUM_OF_BOARDS];
} RTM_ISDN_PARAMS_MCMS_S;


// used in:
//	RTM_ISDN_DEFAULT_SERVICE_NAME_IND
//	RTM_ISDN_DELETE_SERVICE_IND
typedef struct
{
	BYTE	serviceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN];
} RTM_ISDN_SERVICE_NAME_S;


// used in:
//	RTM_ISDN_ATTACH_SPAN_MAP_IND
typedef struct
{
	WORD	boardId;
	WORD	spanId;
	BYTE	serviceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN];
} RTM_ISDN_SPAN_MAP_S;

typedef struct
{
	RTM_ISDN_SPAN_MAP_S		spanMap[MAX_ISDN_SPAN_MAPS_IN_LIST];
} RTM_ISDN_SPAN_MAPS_LIST_S;


// used in:
//	RTM_ISDN_SPAN_ENABLED_IND
typedef struct
{
	WORD	boardId;
	WORD	spanId;
	BYTE	serviceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN];
	WORD	spanEnabledStatus;
} SPAN_ENABLED_S;



// used in:
//	RTM_ISDN_DETACH_SPAN_MAP_IND
//	RTM_ISDN_SPAN_DISABLE_IF_UPDATABLE_REQ
//	RTM_ISDN_SPAN_DISABLE_IF_UPDATABLE_IND
typedef struct
{
	WORD	boardId;
	WORD	spanId;
	DWORD	status;
} SPAN_DISABLE_S;



// used in:
//	RTM_ISDN_ENTITY_LOADED_IND
typedef struct
{
	WORD	boardId;
	WORD	subBoardId;
	WORD	numOfSpans;
} RTM_ISDN_ENTITY_LOADED_S;


// used in:
//	RTM_ISDN_SPAN_STATUS_MCMS_IND
typedef struct
{
	WORD boardId;
	WORD subBoardId;
	WORD spanId;
 	WORD alarm_status;
	WORD d_chnl_status;
	WORD clocking_status;
} RTM_ISDN_SPAN_STATUS_MCMS_S;


// used in:
//	RTM_ISDN_DISABLE_ALL_SPANS_IND
typedef struct
{
	WORD	boardId;
	WORD	subBoardId;
} RTM_ISDN_BOARD_ID_S;


// used in:
//	RTM_ISDN_ADD_PHONE_RANGE_REQ
//	RTM_ISDN_PHONE_RANGE_DELETE_IF_UPDATABLE_REQ
//	RTM_ISDN_PHONE_RANGE_DELETE_IF_UPDATABLE_IND
typedef struct
{
	BYTE						serviceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN];
	RTM_ISDN_PHONE_RANGE_S		phoneRange;
	DWORD						status;
} RTM_ISDN_PHONE_RANGE_UPDATE_S;


// used in:
//	RTM_ISDN_SERVICE_CANCEL_IF_UPDATABLE_REQ
//	RTM_ISDN_SERVICE_CANCEL_IF_UPDATABLE_IND
typedef struct
{
	BYTE	serviceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN];
	DWORD	status;
} RTM_ISDN_SERVICE_CANCEL_S;




#endif /*_RtmIsdnMngrStructs_H__*/
