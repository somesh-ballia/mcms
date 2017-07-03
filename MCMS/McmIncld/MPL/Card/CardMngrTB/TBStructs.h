/************************ START TB STRUCTS ********************************/
/*************************************************************************/
#ifndef   __TBSTRUCTS_H__
#define   __TBSTRUCTS_H__


#include "DataTypes.h"
#include "PhysicalResource.h"



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Connect structs
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

typedef struct
{
	MCMS_MPL_PHYSICAL_RESOURCE_INFO_S  physical_port1;
	MCMS_MPL_PHYSICAL_RESOURCE_INFO_S  physical_port2;
	
} TB_MSG_CONNECT_S;

typedef struct
{
	MCMS_MPL_PHYSICAL_RESOURCE_INFO_S  physical_port1;
	APIU32							   pcm_process_id;
	
} TB_MSG_CONNECT_PCM_S;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Add Party To VS Session and Delete structs
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct
{
	PHYSICAL_RESOURCE_INFO_S  physical_port;
	
} TB_MSG_ADD_PARTY_TO_VSW_SESSION_S;

typedef TB_MSG_ADD_PARTY_TO_VSW_SESSION_S	TB_MSG_DELETE_PARTY_FROM_VSW_SESSION_S;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//move rsrc params structs
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct
{
	APIU32 	newConfId; // Destination conf Id	
	APIU32 	confType;  // EConfType
 	APIS32	confAudioSampleRate;	
 	APIS32  enConfSpeakerChangeMode; // 0 - default   1 - Fast
} MOVE_RESOURCES_PARAMS_S;


typedef struct {
  PHYSICAL_UNIT_PARAMS_S unit_recover;
} UNIT_RECOVERY_S;


typedef struct {
  APIU32 status;
  PHYSICAL_UNIT_PARAMS_S unit_recover;
  PHYSICAL_UNIT_PARAMS_S unit_replacement;
} RECOVERY_REPLACEMENT_UNIT_S;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// party debug info
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// CM need 20k for Art information  
#define DEBUG_INFO_STRING_SIZE  20*1024

// 1)PARTY_DEBUG_INFO_REQ uses headrs same as KILL PORT to all ports (including Card Manager)

// 2) used by CONF_DEBUG_INFO_REQ to audio controler
typedef struct {
  APIU32 	confId;
} CONF_DEBUG_INFO_REQ_S;

// 3) used by PARTY_DEBUG_INFO_IND , CONF_DEBUG_INFO_IND , PARTY_CM_DEBUG_INFO_IND  
typedef struct {
  APIS8  debugInfo[DEBUG_INFO_STRING_SIZE];
} DEBUG_INFO_IND_S;





/*remove spreading
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//conf. spread structs
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define MAX_SPREAD_BOARDS 14

typedef struct
{
	
	APIU32                   num_boards_spreaded;
	PHYSICAL_UNIT_PARAMS_S  spread_params[MAX_SPREAD_BOARDS];

} CONF_SPREAD_NOTIFICATION_PARAMS_S;
end remove spreading*/

#endif //__TBSTRUCTS_H__

