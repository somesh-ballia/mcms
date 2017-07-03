
#ifndef   __Q931STRUCTS_H__
#define   __Q931STRUCTS_H__

#define PRI_LIMIT_PHONE_DIGITS_LEN 24

#define G_2(b2,b1) (unsigned char)((b2<<1)+b1)
#define G_3(b3,b2,b1) (unsigned char)((((b3<<1)+b2)<<1)+b1)
#define G_4(b4,b3,b2,b1) (unsigned char)((((((b4<<1)+b3)<<1)+b2)<<1)+b1)

/*************************************************************************/
/*						COMMON STRUCTURES								 */
/*************************************************************************/
#define MAX_NUM_SPANS_ORDER 12 // physical structs in MPM card

typedef struct
{
    APIU32	spans_order[MAX_NUM_SPANS_ORDER];		//Span order
    APIU32	net_connection_id;	    //Set by card in dial in
    APIU32	virtual_port_number;	//Set by MCMS (Resource Allocator)
    APIU32	physical_port_number;	//Set by card (Physical Port ID) (left in API for flexability - enable mcms to set physical port)

} NET_SETUP_REQ_HEADER_S;

typedef struct
{
    APIU32	span_id;				//Span ID
    APIU32	net_connection_id;		//Set by card in dial in
    APIU32	virtual_port_number;	//Set by MCMS (Resource Allocator)
    APIU32	physical_port_number;	//Set by card (Physical Port ID)

} NET_COMMON_PARAM_S; 

/*************************************************************************/

typedef struct
{

    APIU32	num_digits;
    APIU32	num_type;
    APIU32	num_plan;
    APIU32	presentation_ind;
    APIU32	screening_ind;
    APIU8	digits[PRI_LIMIT_PHONE_DIGITS_LEN];

} NET_CALLING_PARTY_S;  
  
/* definitions for num_type */
#define	ACU_NB_TYPE_UNKNOWN				G_3 (0,0,0)	/* unknown */
#define	ACU_NB_TYPE_INTERNATIONAL		G_3 (0,0,1)	/* international number */
#define	ACU_NB_TYPE_NATIONAL			G_3 (0,1,0)	/* national number */
#define	ACU_NB_TYPE_SUBSCRIBER			G_3 (1,0,0)	/* subscriber number */
#define	ACU_NB_TYPE_ABBREVIATED			G_3 (1,1,0)	/* abbreviated number */

/* definitions for presentation_ind */
#define	ACU_NB_PRES_ALLOWED				G_2 (0,0)	/* presentation allowed */
#define	ACU_NB_PRES_RESTRICTED			G_2 (0,1)	/* presentation restricted */
#define	ACU_NB_PRES_NOT_AVAILABLE		G_2 (1,0)	/* number not available due to interworking */

/* definitions for screening_ind */
#define ACU_NB_SCREEN_USER_PROVIDED		G_2 (0,0)	/* user-provided, not screened */
#define ACU_NB_SCREEN_USER_PASSED		G_2 (0,1)	/* user-provided, verified and passed */
#define ACU_NB_SCREEN_USER_FAILED		G_2 (1,0)	/* user-provided, verified and failed */
#define ACU_NB_SCREEN_NETWORK_PROVIDED 	G_2 (1,1)	/* network provided */

/* definitions for num_plan */
#define ACU_NB_PLAN_UNKNOWN				G_4 (0,0,0,0) /* unknown */
#define ACU_NB_PLAN_ISDN				G_4 (0,0,0,1) /* ISDN/telephony numbering plan (CCITT E.164/E.163) */
#define ACU_NB_PLAN_TELEPHONE			G_4 (0,0,1,0) /* telephony - not in CEPT */
#define ACU_NB_PLAN_PRIVATE				G_4 (1,0,0,1) /* private numbering plan */

/*************************************************************************/

typedef struct
{

    APIU32  num_digits;
    APIU32  num_type;
    APIU32  num_plan;
    APIU8	digits[PRI_LIMIT_PHONE_DIGITS_LEN];

} NET_CALLED_PARTY_S;


/*************************************************************************/

typedef struct
{

    APIU32	cause_val; /* = 0 a list is found in Appendix B of PRI */

} CAUSE_DATA_S;


/* definitions of cause_val */

#define causDEFAULT_VAL        0
#define causUNALOC_VAL         1   /* unallocated number*/
#define causNO_ROUTE_VAL       2   /*no route specified to transit net */
#define causBAD_CHAN_VAL       6   /*channel unacceptable*/
#define causNORMAL_CLR_VAL     16  /*normal call clearing*/
#define caus_USER_BUSY_VAL     17  /*called party busy*/
#define causNO_USER_RSP_V      18  /*no response*/
#define causCALL_REJ_VAL       21  /*call rejected*/
#define causNUM_CHANGED_VAL    22  /*number has been changed*/
#define causINVALID_NUM_VAL    28  /*invalid number format*/
#define causFACIL_REJ_VAL      29  /*facility rejected*/
#define causSTAT_RESP_VAL      30  /*response to status inquiry*/
#define causNRML_UNSPEC_VAL    31  /*normal , unspecified*/
#define causNO_CHAN_AVL_VAL    34  /*no circuit channel available*/
#define causNET_OOS_VAL        38  /*net out of order*/
#define causTMP_FAILURE_VAL    41  /*temporary failure*/

#define causeBUSY_GLARE_VAL    91  /*temporary failure*/

#define causTIMER_EXPIR_VAL    102 /*timer expiration*/


/*************************************************************************/
/*						REQUESTS STRUCTURES								 */
/*************************************************************************/
			
			/********************/
			/* SETUP REQ MESSAGE */
			/********************/

/* Equal to call req message in PRI. Starts a dialout call */

typedef struct
{
  //NET_COMMON_PARAM_S	net_common_header;
    NET_SETUP_REQ_HEADER_S  net_setup_header; /* net_connection_id = DONT_CARE_VAL */ 
    APIU32					net_spfc;
    APIU32					call_type;  
    NET_CALLING_PARTY_S		calling_party;
    NET_CALLED_PARTY_S		called_party;

} NET_SETUP_REQ_S;  /* host starts an outgoing call */

/* definitions for call_type */
#define	ACU_VOICE_SERVICE				'V'	/* call for voice service */
#define	ACU_MODEM_SERVICE				'M'	/* call for modem data service */
#define	ACU_DATA_56KBS_SERVICE			'K'	/* call for 'data at 56 KBS' service */
#define	ACU_DATA_SERVICE				'D'	/* call for 'data' service */
#define	ACU_DATA_H0_SERVICE				'0' /* call for Data using H0  (384kbs)  channel service (PRI only) */
#define	ACU_DATA_H11_SERVICE			'H' /* call for Data using H11 (1536kbs) channel service (PRI only) */

/*************************************************************************/

			/*********************/
			/* CLEAR REQ MESSAGE */
			/*********************/

/* Mandatory. Indicates that host starts disconnection */

typedef struct
{

    NET_COMMON_PARAM_S	net_common_header; 
    CAUSE_DATA_S        cause;    

} NET_CLEAR_REQ_S;

/*************************************************************************/

			/******************************/
			/* DISCONNECT ACK REQ MESSAGE */
			/******************************/

/* Mandatory. In response to remote disconnect ind */

typedef struct
{

    NET_COMMON_PARAM_S	net_common_header; 
    CAUSE_DATA_S        cause;    

} NET_DISCONNECT_ACK_REQ_S;

/***************************************************************************/

			/*********************/
			/* ALERT REQ MESSAGE */
			/*********************/

/* Optional req in response to setup ind.*/ 

typedef struct
{

    NET_COMMON_PARAM_S net_common_header;

} NET_ALERT_REQ_S;

/*************************************************************************/

		    /***********************/
		    /* CONNECT REQ MESSAGE */
		    /***********************/

/* Mandatory req in response to setup ind. Indicates that session of
   setup ind is completed and the call is established. */

typedef struct
{

    NET_COMMON_PARAM_S net_common_header;

} NET_CONNECT_REQ_S;


/*************************************************************************/
/*						INDICATIONS STRUCTURES							 */
/*************************************************************************/
 
			/*************************/
			/* NET CLEAR IND MESSAGE */
			/*************************/

/* Mandatory. Indicates that disconnect session is completed */

typedef struct
{

    NET_COMMON_PARAM_S	net_common_header; 
    CAUSE_DATA_S        cause;    

} NET_CLEAR_IND_S;

/***************************************************************************/

			/****************************/
			/* NET PROGRESS IND MESSAGE */
			/****************************/
				   
/* optional ind in response to call request. Indicates generally that
   remote is not isdn and that more info is found in inband */

typedef struct
{

    NET_COMMON_PARAM_S	net_common_header;
    APIU32				progress_dscr;

} NET_PROGRESS_IND_S;

/* definitions for progress_dscr */

#define progUNKNOWN_VAL          0
#define progNOT_ISDN_INBAND_VAL  1
#define progDEST_NOT_ISDN_VAL    2
#define progORIG_NOT_ISDN_VAL    3
#define progRETURNED_ISDN_VAL    4
#define progINBAND_INFO_AVL_VAL  8

/***************************************************************************/

			/*************************/
			/* NET ALERT IND MESSAGE */
			/*************************/

/* Optional ind in response to call request. Indicates that remote is 
   ringing. Received only if remote is isdn. */

typedef struct
{

    NET_COMMON_PARAM_S	net_common_header;

} NET_ALERT_IND_S;

/***************************************************************************/

			/***************************/
			/* NET PROCEED IND MESSAGE */
			/***************************/

/* Optional ind in response to call request. Relevant only at preffered mode
   and indicates that remote changed ports. */

typedef struct
{

    NET_COMMON_PARAM_S	net_common_header;

} NET_PROCEED_IND_S;

/****************************************************************************/

		    /***************************/
		    /* NET CONNECT IND MESSAGE */
		    /***************************/

/* Mandatory ind in response to call request. Indicates that session of call
   request is completed and the call is established. */

typedef struct
{

    NET_COMMON_PARAM_S	net_common_header;

} NET_CONNECT_IND_S;

/***************************************************************************/

			/***************************/
			/* NET SETUP IND MESSAGE   */
			/***************************/

/* Mandatory. Indicates that an incoming call request has been received 
   from network. */

typedef struct
{

    NET_COMMON_PARAM_S		net_common_header;
    APIU32					net_spfc;          /* values defined as in call request */
    APIU32					call_type;         /* bit mask values defined as in call request */ 
    NET_CALLING_PARTY_S		calling_party;
    NET_CALLED_PARTY_S		called_party;
    
} NET_SETUP_IND_S;

/***************************************************************************/

				 /******************************/
				 /* NET DISCONNECT IND MESSAGE */
				 /******************************/

/* Mandatory. Indicates that remote starts a disconnection session */

typedef struct
{

    NET_COMMON_PARAM_S	net_common_header; 
    CAUSE_DATA_S		cause;    

} NET_DISCONNECT_IND_S;

/*************************************************************************/

			/******************************/
			/* DISCONNECT ACK IND MESSAGE */
			/******************************/

/* Mandatory. Indicates completion of disconnection inititated by remote 
   (response for NET_DISCONNECT_ACK_REQ_S) */

typedef struct
{

    NET_COMMON_PARAM_S	net_common_header; 
    CAUSE_DATA_S        cause;    

} NET_DISCONNECT_ACK_IND_S;

/***************************************************************************/

#endif//__Q931STRUCTS_H__
