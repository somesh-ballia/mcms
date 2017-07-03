// CsStructs.h
//
//////////////////////////////////////////////////////////////////////
#if !defined(_CsStructs_H__)
#define _CsStructs_H__


#include "DataTypes.h"
#include "VersionStruct.h"
#include "IpCsSizeDefinitions.h"


//////////////////////////////
//   CONSTANTS DEFINITIONS
//////////////////////////////

#define H323_CONFIG_DATA_SIZE		128
#define H323_CONFIG_KEY_SIZE		32

 

//////////////////////////////
//       ENUMERATORS
//////////////////////////////

/*-----------------------------
	eDHCPState.
	used in CS_IP_INFO_S
 ------------------------------*/
enum eDHCPState
{
	eDHCPStateDisable = 0,
    eDHCPStateInit,
    eDHCPStateRestart,
    eDHCPStateSelecting,
    eDHCPStateRequesting,
    eDHCPStateBinding,
    eDHCPStateBound,
    eDHCPStateRenew,
    eDHCPStateRebind,
	// add new here
	NumDHCPStates,


    eDHCPStateUnkown = 25
};

enum eServiceStatusInd
{
    eSerStatusOk = 0,		// Both Networks are OK
    eSerStatusPrimaryOk = 1,	// Only primary Network is OK
    eSerStatusSecondaryOk = 2,	// Only secondary Network is OK
    eSerStatusFailed = 0xFF	// Both Networks are failed
} ;


/*-----------------------------
	eServiceState.
	used in SERVICE_INFO
 ------------------------------*/
enum eIpServiceState
{
	eServiceStateConfig = 0,
	eServiceStateOk,
	eServiceStateFailed,
	// add new here
	NumServiceStates
};


//////////////////////////////
//    API STRUCTURES
//////////////////////////////



/*************************************************************************************/
/*
 *		Startup state structures, (structs)
 */
/*************************************************************************************/

/*-----------------------------------------------------------------------------------
	CS_New_Ind_S. 
	MCMS <-- CS Module
-----------------------------------------------------------------------------------*/
typedef struct
{
	APIU32 id;			// Central Signaling ID
	VERSION_S version;	// Central Signaling version
	// LANCfgContent will be here in a future
}CS_New_Ind_S;



/*-----------------------------------------------------------------------------------
	CS_New_Req_S. 
	MCMS --> CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
	APIU16 num_h323_ports;
	APIU16 num_sip_ports;
}CS_New_Req_S;




/*-----------------------------------------------------------------------------------
	CS_Config_Req_S. 
	MCMS --> CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
	APIS8	section[STR_LEN];					// name of config sections
	APIS8	key	[H323_CONFIG_KEY_SIZE];
	APIS8	data[H323_CONFIG_DATA_SIZE];

}CS_Config_Req_S;



/*-----------------------------------------------------------------------------------
	CS_Config_Ind_S. 
	MCMS <-- CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
	APIS8  str_section_name	[STR_LEN];
	APIS8  str_key_name		[STR_LEN];
	APIU32 result;
	APIS8  str_error_reason	[STR_LEN];
}CS_Config_Ind_S;



/*-----------------------------------------------------------------------------------
	CS_End_Config_Req_S. 
	MCMS --> CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
	// NONE
}CS_End_Config_Req_S;



/*-----------------------------------------------------------------------------------
	CS_End_Config_Ind_S. 
	MCMS <-- CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
	APIU32 result;
	APIS8 str_error_reason[STR_LEN];
}CS_End_Config_Ind_S;




/*-----------------------------------------------------------------------------------
	CS_Lan_Cfg_Req_S. 
	MCMS --> CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
	// LanCfg
}CS_Lan_Cfg_Req_S;




/*-----------------------------------------------------------------------------------
	CS_End_StartUp_Ind_S. 
	MCMS <-- CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
	APIU32 result;
	APIS8 str_error_reason[STR_LEN];
}CS_End_StartUp_Ind_S;


/*-----------------------------------------------------------------------------------
	CS_Reconnect_Req_S. 
	MCMS --> CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
	// in future
}CS_Reconnect_Req_S;



/*-----------------------------------------------------------------------------------
	CS_Reconnect_Ind_S. 
	MCMS <-- CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
	// in future
}CS_Reconnect_Ind_S;





/*************************************************************************************/
/*
 *		Service structures, (buffers)
 */
/*************************************************************************************/

/*-----------------------------------------------------------------------------------
	CS_New_Service_Init_Req_S. 
	MCMS --> CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
	APIU32  length;
    char    paramBuffer[1];
} CS_New_Service_Init_Req_S;


/*-----------------------------------------------------------------------------------
	CS_New_Service_Init_Ind_S. 
	MCMS <-- CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
    APIU32  length;
    char    paramBuffer[1];
} CS_New_Service_Init_Ind_S;


/*-----------------------------------------------------------------------------------
	CS_Common_Param_Req_S.
	MCMS --> CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
    APIU32  length;
    char    paramBuffer[1];
} CS_Common_Param_Req_S;
 

/*-----------------------------------------------------------------------------------
	CS_Common_Param_Ind_S. 
	MCMS <-- CS Module
-----------------------------------------------------------------------------------*/
typedef struct
{
    APIU32  length;
    char    paramBuffer[1];
} CS_Common_Param_Ind_S;


/*-----------------------------------------------------------------------------------
	CS_End_Service_Init_Req_S. 
	MCMS --> CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
    APIU32  length;
    char    paramBuffer[1];
} CS_End_Service_Init_Req_S;


/*-----------------------------------------------------------------------------------
	CS_End_Service_Init_Ind_S. 
	MCMS <-- CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
    APIU32  length;
    char    paramBuffer[1];
} CS_End_Service_Init_Ind_S;


/*-----------------------------------------------------------------------------------
	CS_Del_Service_Req. 
	MCMS --> CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
    APIU32  length;
    char    paramBuffer[1];
} CS_Del_Service_Req;


/*-----------------------------------------------------------------------------------
	CS_End_Service_Init_Ind_S. 
	MCMS <-- CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
    APIU32  length;
    char    paramBuffer[1];
} CS_Del_Service_Ind;






/*************************************************************************************/
/*
 *		Other structures, (buffers)
 */
/*************************************************************************************/

/*-----------------------------------------------------------------------------------
	CS_Terminal_Command_Req. 
	MCMS --> CS Module
-----------------------------------------------------------------------------------*/
typedef struct 
{
    APIU32  length;
    char    paramBuffer[1];
} CS_Terminal_Command_Req;




/*************************************************************************************/
/*
 *		KeepAlive structures, (buffers)
 */
/*************************************************************************************/


// Component types
typedef enum
{
	 emNonComponent = 0,
     emCompCSMngnt  = 1,
     emCompCSH323   = 2,
     emCompCSSIP    = 3,
     emCompCSSignalPort =  4,	
} compTypes;

// Component statuses
typedef enum
{
     emCompOk			= 0,
     emCompFailed		= 1,
     emCompRecovered	= 2,
} compStatuses;

//recovery reason
typedef enum
{
	emRecoveryUnknown			= 1,
	emRecoveryFromException		= 2,
	emRecoveryFromStuck			= 3,
	emExitTooManyRecoveries		= 4,
	emExternalTermination		= 5,
} recoveryReasons;

typedef struct
{
     APIU32             bActive;      // Is active component
     APIU32             id;           // Service ID
     compTypes          type;         // Component type 
     compStatuses       status;       // Component status
     APIU32             reason;
} compKeepAliveSt;

// Keep Alive indication structure (CS_KEEP_ALIVE_IND)
typedef struct
{
	compKeepAliveSt   componentTbl[NumOfComponents];
} csKeepAliveSt;

// Component status indication (CS_COMP_STATUS_IND)
typedef struct
{
     compTypes			type;		// Component type						(enum)
     compStatuses		status;		// Component status (recovered, etc.)	(enum)
     recoveryReasons	reason;		// error code							(enum)
     UINT32				unitId;	
     char				unitName[MAX_UNIT_NAME_SIZE];	
     char				errorStr[MaxRecoveryErrorStrSize];	
} csCompStatusSt;

/*************************************************************************************/
/*
 *		Ping structures, (buffers)
 */
/*************************************************************************************/

/*-----------------------------
	ePingIpType.
	used in CS_Ping_S
 ------------------------------*/
enum ePingIpType
{
	ePingIpType_IPv4	= 0,
	ePingIpType_IPv6,
	// add new here
	NumPingIpTypes
};

/*-----------------------------
	ePingStatus.
------------------------------*/
enum ePingStatus
{
	ePingStatus_ok	= 0,
	ePingStatus_fail,
	// add new here
	NumPingStatuses
};

/*---------------------------------------------------
	CS_Ping_req_S. 
	MCMS --> CS Module
----------------------------------------------------*/
typedef struct
{
	APIU32	ipType;
	APIS8	destination[STR_LEN/*256*/];
}CS_Ping_req_S;


/*---------------------------------------------------
	CS_Ping_ind_S. 
	MCMS --> CS Module
----------------------------------------------------*/
typedef struct
{
	enum ePingStatus	pingStatus;
}CS_Ping_ind_S;


#endif // !defined(_CsStructs_H__)
