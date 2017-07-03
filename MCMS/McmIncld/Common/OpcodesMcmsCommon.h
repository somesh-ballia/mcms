#ifndef OPCODESCOMMON_H_
#define OPCODESCOMMON_H_



//API of all tasks
#define DESTROY					1
#define TIMER					2
#define SET_SYS_ID				3
#define TRACE_MESSAGE			4
#define XML_REQUEST				5
#define XML_RESPONSE			6
#define MPLAPI_MSG				7
#define KILLED_BY_TWIN			8
#define MSG_WITH_STATE_MACHINE	9
#define TERMINAL_COMMAND		10
#define SUBSCRIBE				11
#define UNSUBSCRIBE				12
#define CSAPI_MSG				13
#define WD_KEEP_ALIVE			14
#define ACK_IND					15
#define OBSERVER_UPDATE			16
#define PROCESS_UP              17
#define EXT_DB_REQUEST_FAILED	18
#define EXT_DB_REQUEST			19
#define EXT_DB_RESPONSE			20
#define HLOG_LOGGER				21
#define HLOG_LOGGER_FULL_ONLY	22
#define SIGNALING_MANAGER_MSG			25
#define HANDLE_EXCEPTION		27
#define RESETMCU_REQUEST        28
#define INFORM_HTTP_GET_FILE    29
#define CLOSE_SOCKET_CONNECTION 100
#define OPEN_SOCKET_CONNECTION 	101
#define REMOVE_ALL_AA			102
#define SET_ENABLE_DISABLE_AA	103
#define UPGRADE_STARTED_IND        104
#define CLOSE_SOCKET_BY_TWIN	105



// =======================================
// ============= CARDS range =============
// =======================================
//#define  CARDS_MNGR_FIRST_OPCODE_IN_RANGE			5001000
//   for messages to Manager task (of Cards process) only!!
#define  CM_CARD_MNGR_LOADED_IND						5001010   // sends a CM_CARD_MNGR_LOADED_S structure
#define  CM_CARD_FOLDER_MOUNT_IND						5001011   // sends CM_FOLDER_MOUNT_S structure
#define  CM_IVR_FOLDER_MOUNT_IND						5001012   // sends CM_FOLDER_MOUNT_S structure
#define  DEBUG_MODE_YES_REQ								5001013
#define  DEBUG_MODE_NO_REQ								5001014

#define ETHERNET_SETTINGS_ACTIVE_PORTS_SWITCH_IND		5001020	// Switch asks for info. sends opcode only
#define ETHERNET_SETTINGS_MONITORING_SWITCH_IND			5001021	// Switch asks for info. sends ETH_SETTINGS_PORT_DESC_S
#define ETHERNET_SETTINGS_CLEAR_MAX_COUNTERS_SWITCH_IND	5001022	// Switch asks for clearing. sends ETH_SETTINGS_PORT_DESC_S
#define ETHERNET_SETTINGS_ACTIVE_PORTS_SWITCH_REQ		5001023	// Mcms provides info. sends ETH_SETTINGS_ACTIVE_PORTS_LIST_S
#define ETHERNET_SETTINGS_MONITORING_SWITCH_REQ			5001024	// Mcms provides info. sends ETH_SETTINGS_SPEC_S
#define ETHERNET_SETTINGS_CLEAR_MAX_COUNTERS_SWITCH_REQ	5001025	// Mcms provides info. sends ETH_SETTINGS_STATE_S

#define START_TCP_DUMP_REQ 								5001026
#define START_TCP_DUMP_IND 								5001027

#define STOP_TCP_DUMP_REQ 								5001028
#define STOP_TCP_DUMP_IND 								5001029

//#define  CARDS_MNGR_LAST_OPCODE_IN_RANGE				5001999

//#define CARDS_COMMON_FIRST_OPCODE						5010001
#define  EMB_USER_MSG_IND							5001015   // sends USER_MSG_S structure
#define CM_SYSTEM_RESOURCES_EMPTY 5001016

//#define CARDS_DISPATCHER_FIRST_OPCODE_IN_RANGE	5010000

//   for messages to Dispatcher task (of Cards process)
#define  CARDS_NOT_READY_REQ							5010010
#define  MFA_TASK_NOT_CREATED_REQ						5010011
#define  BAD_SPONTANEOUS_IND							5010012

#define  SYSCFG_PARAMS_REQ							5010013
#define  OLD_SYSCFG_PARAMS_IND						5010014	  // sends a SYSCFG_PARAMS_S structure
#define  RESET_CARD_REQ                             5010015
#define  CARD_CONFIG_REQ                      		5010016 //2 modes cop/cp

#define  CARD_NOT_SUPPORTED_IND							5010017

#define ETHERNET_SETTINGS_IND							5010020	// sends ETH_SETTINGS_SPEC_S
#define ETHERNET_SETTINGS_CLEAR_MAX_COUNTERS_REQ		5010021	// sends ETH_SETTINGS_PORT_DESC_S
#define ETHERNET_SETTINGS_CLEAR_MAX_COUNTERS_IND		5010022	// sends ETH_SETTINGS_STATE_S

#define ACTIVE_DIRECTORY_STATUS_REQ					    5010030
#define IS_USER_EXIST_ON_ACTIVE_DIRECTORY_IND			5010031
#define IS_USER_EXIST_ON_ACTIVE_DIRECTORY_REQ			5010032
#define ETHERNET_SETTINGS_SYSTEM_MONITORING_REQ		    5010033


//#define CARDS_COMMON_LAST_OPCODE						5019999

//#define CARDS_DISPATCHER_LAST_OPCODE_IN_RANGE		5059999



// =======================================
// ============= MPLAPI range ============
// =======================================
//#define MPLAPI_COMMON_FIRST_OPCODE				2010000
#define MOVE_RSRC_REQ								2010010
// Opcodes for log leveling feature
#define SET_LOG_LEVEL_REQ 							2010011
#define SET_LOG_LEVEL_IND 							2010012
#define NACK_LOG_LEVEL_OUT_OF_RANGE					2010013
// CM <-> MRMP API
#define CM_API_MRMP_MAX_NUM_OF_PARTICIPANTS_TO_CHANGE_LOG_LEVEL_REQ 		2010100


//#define MPLAPI_COMMON_LAST_OPCODE					2019999



// =======================================
// ============= LOGGER range ============
// =======================================
//#define LOGGER_COMMON_FIRST_OPCODE				3010000
#define TRACE_TO_LOGGER								3010010
//#define LOGGER_COMMON_LAST_OPCODE					3019999



// =======================================
// ============ MCUMNGR range ============
// =======================================
//#define MCUMNGR_COMMON_FIRST_OPCODE					4010000
#define FAULT_GENERAL_IND								4010010
#define ETHERNET_SETTINGS_CONFIG_IND					4010011	// sends ETH_SETTINGS_STATE_S

#define ETHERNET_SETTINGS_CONFIG_REQ					4010110	// sends ETH_SETTINGS_CONFIG_S
//#define MCUMNGR_COMMON_LAST_OPCODE					4019999

// =======================================
// ============ FAILOVER range ============
// =======================================
//#define MCUMNGR_COMMON_FIRST_OPCODE					6010000
#define FAILOVER_GENERAL_IND							6010010
//#define MCUMNGR_COMMON_LAST_OPCODE					6019999


#endif /*OPCODESCOMMON_H_*/
