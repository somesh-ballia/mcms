#ifndef __BONDING_H
#define __BONDING_H


#define DIRECTION_DIAL_IN  0
#define DIRECTION_DIAL_OUT 1

#define MAX_ADDITIONAL_PHONE_NUM 30

#define BND_STAR 0xA
#define BND_DIEZ 0xB
#define BND_EON  0xC
#define BND_CAUSEIND 0xE
#define BND_PAD  0xF

/*============================================================================================*/
/*==== general data structs ====*/
/*============================================================================================*/
typedef struct
{
  APIS8 direction;               /* 1 for a DialOut call & 0 for a DialIn call. */
  APIS8 NumOfBndChnls;           /* 1 for B channels, 6 for H0 channels etc.    */
  APIS8 restrictType;            /* 1 - Restricted, 0 - Not                     */
  APIS8 dummy;                   /* alligned to 4 bytes (ART restriction)       */
} BND_CALL_PARAMS_S;
/*============================================================================================*/
typedef struct
{
  APIS8 channelWidthNegotiation; /* 0- Non negotiable  ;  1- Negotiable.        */
  APIS8 restrictTypeNegotiation; /* 0- Non negotiable  ;  1- Negotiable.        */
  APIS8 IsDownspeedSupport;      /* 0- Not supported   ;  1- supported.         */
  APIS8 dummy;                   /* alligned to 4 bytes (ART restriction)       */
} BND_NEGOTIATION_PARAMS_S;
/*============================================================================================*/
#define  BND_MAX_PHONE_LEN 7 /* WAS 12 */
typedef struct
{
  APIS8 digits[BND_MAX_PHONE_LEN];
  APIS8 dummy;                   /* alligned to 4 bytes (ART restriction)       */  
} BND_PHONE_NUM_S;
/*============================================================================================*/
/* bondind phone list defines number of phone numbers, */
/* following BND_PHONE_NUM_S x numberOfPhoneNums       */
typedef struct
{
  BND_PHONE_NUM_S  startOfPhoneList[MAX_ADDITIONAL_PHONE_NUM];  
  APIS8 numberOfPhoneNums;
  APIS8 dummy[3];                /* alligned to 4 bytes (ART restriction)       */ 
    //APIU32 startOfData;            /* not serialize and send to ART startpoint for mcms       */ 
} BND_PHONE_LIST_S;
/*============================================================================================*/


/*============================================================================================*/
/*==== negotiation ====*/
/*============================================================================================*/
/* opcodes uses the struct: BND_CONNECTION_INIT  */
/* direction: mcms ==> mux    */
/* description: start of bonding negotiation */
/* change from mgc: added additional dial in phone number.*/
/*--------------------------------------------------------------------------------------------*/
typedef struct
{
  BND_CALL_PARAMS_S callParams;
  BND_NEGOTIATION_PARAMS_S negotiationParams;
  BND_PHONE_NUM_S additional_dial_in_phone_num;
} BND_CONNECTION_INIT_REQUEST_S;
/*============================================================================================*/
/* opcodes uses the struct: BND_END_NEGOTIATION */
/* direction: mux ==> mcms    */
/* description: end of bonding negotiation */
/* change from mgc: no major change*/
/*--------------------------------------------------------------------------------------------*/
typedef struct
{
  BND_CALL_PARAMS_S callParams;
  BND_PHONE_LIST_S phoneList;		
} BND_END_NEGOTIATION_INDICATION_S;
/*============================================================================================*/
/* opcodes uses the struct:   */
/* direction: mux ==> mcms BND_REQ_PARAMS */
/* description: request additional dial in phone numbers list - will not be used in RMX flow*/
/* change from mgc: not in use at RMX*/
/*--------------------------------------------------------------------------------------------*/
typedef struct
{
  BND_CALL_PARAMS_S callParams;
} BND_REQ_PARAMS_INDICATION_S;
/*============================================================================================*/
/* opcodes uses the struct: BND_ACK_PARAMS */
/* direction: mcms ==> mux    */
/* description: sending additional dial in phone numbers list - will not be used in RMX flow*/
/* change from mgc: not in use at RMX*/
/*--------------------------------------------------------------------------------------------*/
typedef BND_END_NEGOTIATION_INDICATION_S BND_ACK_PARAMS_REQUEST_S;
/*============================================================================================*/


/*============================================================================================*/
/*==== alignment ====*/
/*============================================================================================*/
/* opcodes uses the struct:   */
/* direction: mcms ==> mux BND_ADD_CHANNEL */
/* description: net channel connected*/
/* change from mgc: no major change*/
/*--------------------------------------------------------------------------------------------*/
/* struct CBndMsgAddChannel */
typedef struct
{
  BND_CALL_PARAMS_S callParams;
} BND_ADD_CHANNEL_REQUEST_S;
/*============================================================================================*/
/* opcodes uses the struct: BND_REMOTE_LOCAL_ALIGNMENT  */
/* direction: mux ==> mcms    */
/* description: bonding alignment  */
/* change from mgc: no major change*/
/*--------------------------------------------------------------------------------------------*/
typedef struct
{
  APIU8 indicator; /* do we still need it? */	                                                    
  APIU8 numOfChannels;
  APIS8 dummy[2];                /* alligned to 4 bytes (ART restriction)       */  
  /* downspeed currently not supported in RMX field left for futere implementation */
//  WORD DownSpeedStatus;		/* 1=Down-Speed had been made; 0=Otherwize */
//  DWORD AlignedChannelsMask;	/* Bit-Array-Mask for aligned channels (bit=1->channel active; bit=0->channel not active) */
				/* this field is in use only if DownSpeedStatus=1; */
} BND_REMOTE_LOCAL_ALIGNMENT_INDICATION_S;
/*============================================================================================*/
/* opcodes uses the struct: BND_CHANEL_DISCONNECT BND_CHANEL_REJECT  */
/* direction: mux ==> mcms    */
/* description: net channel disconnected / rejected during bonding alignment process - not in use at RMX  */
/* change from mgc: not in use at RMX*/
/*--------------------------------------------------------------------------------------------*/
typedef BND_ADD_CHANNEL_REQUEST_S BND_DISCONNECT_CHANNEL_REQUEST_S;
typedef BND_ADD_CHANNEL_REQUEST_S BND_REJECT_CHANNEL_REQUEST_S;
/*============================================================================================*/


/*============================================================================================*/
/*==== failure and disconnection ====*/
/*============================================================================================*/
/* opcodes uses the struct: BND_ABORT_CALL  */
/* direction: mcms ==> mux    */
/* description: call disconnection during bonding process*/
/* change from mgc: no major change*/
/*--------------------------------------------------------------------------------------------*/
// typedef struct
// {
// /*  INT_COMMON_HEADER_S   mCommonHeader; */
// } BND_ABORT_CALL_REQUEST_S;
/*============================================================================================*/
/* opcodes uses the struct: BND_CALL_FAIL */
/* direction: mux ==> mcms    */
/* description: bonding alignment failure */
/* change from mgc: no major change*/
/*--------------------------------------------------------------------------------------------*/
typedef struct
{
    APIU8 indicator;
	APIS8 dummy[3];                /* alligned to 4 bytes (ART restriction)       */ 
} BND_CALL_FAIL_INDICATION_S;
/*============================================================================================*/
/* opcodes uses the struct: BND_CALL_DISCONNECTED */
/* direction: mux ==> mcms    */
/* description:  failure */
/* change from mgc: no major change*/
/*--------------------------------------------------------------------------------------------*/
typedef BND_CALL_FAIL_INDICATION_S BND_CALL_DISCONNECTED_INDICATION_S;
/*============================================================================================*/

#endif
