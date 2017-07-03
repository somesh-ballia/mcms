#ifndef OPCODESSHELFMNGR_H_
#define OPCODESSHELFMNGR_H_




// =======================================
// ============ MCUMNGR range ============
// =======================================
//#define MCUMNGR_SHELFMNGR_FIRST_OPCODE				4020000

//#define MCUMNGR_SHELFMNGR_REQ_FIRST_OPCODE			4020001
#define  MPL_SW_LOCATION_UPDATE_REQ						4020010   // sends an MPL_SW_LOCATION_S structure
#define  MPL_MNGMNT_IP_PARAMS_UPDATE_REQ				4020011   // sends an IP_PARAMS_S structure
#define  MPL_SHELF_MNGMNT_IP_PARAMS_UPDATE_REQ			4020012   // sends an IP_PARAMS_S structure
#define  MCUMNGR_AUTHENTICATION_FAILURE_REQ				4020013
#define  MCUMNGR_RESET_ALL_MFA_BOARDS_REQ				4020014
#define  MCUMNGR_SET_SYSTEM_TIME_REQ					4020015
#define  MCUMNGR_UPDATE_CFS_KEYCODE_REQ					4020016   // sends an CFS_KEYCODE_S structure
#define  MCUMNGR_RESET_MNGMNT_SERVICE_PARAMS_REQ		4020017
#define  MCUMNGR_RESET_USR_LIST_PARAMS_REQ				4020018
#define  MCUMNGR_RESET_ALL_PARAMS_REQ					4020019
#define  RESTORE_FACTORY_DEFAULT_REQ					4020020
#define  MPL_AUTHENTICATION_REQ							4020021

//802.1x opcodes REQs
#define Op802_1x_NEW_CONFIG_REQ                         4020022
#define Op802_1x_CONNECTION_STATUS_REQ                  4020023

//#define MCUMNGR_SHELFMNGR_REQ_LAST_OPCODE				4024999

//#define MCUMNGR_SHELFMNGR_IND_FIRST_OPCODE			4025000
#define  MPL_AUTHENTICATION_IND							4025010   // sends an MPL_AUTHENTICATION_S structure
#define  MPL_MNGMNT_IP_CONFIG_IND						4025011   // sends an IP_PARAMS_S structure
#define  MPL_SW_LOCATION_IND							4025012   // sends an MPL_SW_LOCATION_S structure
#define  MPL_SW_LOCATION_UPDATE_IND						4025013
#define  MPL_SHELF_MNGMNT_IP_CONFIG_IND					4025014   // sends an IP_PARAMS_S structure
#define  MPL_SHELF_MNGMNT_IP_UPDATE_IND					4025015
#define  MPL_SHELF_ENTER_DIAGNOSTICS_IND				4025016
#define  MPL_MNGMNT_IP_PARAMS_UPDATE_IND				4025017   // sends an IP_PARAMS_S structure
#define  RESTORE_FACTORY_DEFAULT_IND                    4025018
#define  OUT_OF_SECURE_MODE_IND              			4025019
#define  MPL_SW_SET_NTP_DSCP_IND                        4025020

#define  MPL_SW_SET_NTP_DSCP_REQ                        4025022

//802.1x opcodes INDs
#define Op802_1x_NEW_CONFIG_IND                         4025026
#define Op802_1x_CONNECTION_STATUS_IND                  4025027
#define Op802_1x_CONNECTION_STATUS_UNSOLICITED_IND      4025028




//#define MCUMNGR_SHELFMNGR_IND_LAST_OPCODE				4029998

//#define MCUMNGR_SHELFMNGR_LAST_OPCODE					4029999



// =======================================
// ============= CARDS range =============
// =======================================
//#define CARDS_SHELFMNGR_FIRST_OPCODE					5040000

//#define CARDS_SHELFMNGR_REQ_FIRST_OPCODE				5040001
#define  CM_DELETE_IP_SERVICE_REQ						5040010   // sends a Del_Ip_Service_S structure
#define  SM_KEEP_ALIVE_REQ								5040011
#define  CARDS_RESTART_AUTHENTICATION_PROCEDURE_REQ		5040012
#define  SLOTS_NUMBERING_CONVERSION_REQ					5040013
//#define CARDS_SHELFMNGR_REQ_LAST_OPCODE				5044999

//#define CARDS_SHELFMNGR_IND_FIRST_OPCODE				5045000
#define  CM_DELETE_IP_SERVICE_IND						5045010   // sends a CM_DELETE_IP_SERVICE_S structure
#define  SM_FATAL_FAILURE_IND							5045011   // sends a SWITCH_SM_KEEP_ALIVE_S structure
#define  SM_KEEP_ALIVE_IND								5045012   // sends a SWITCH_SM_KEEP_ALIVE_S structure
#define  SM_CARD_MNGR_RECONNECT_IND						5045013   // sends no data (only opcode)
#define  SM_LAN_FAILURE_IND								5045014   // sends a SM_COMPONENT_STATUS_S structure
#define  SLOTS_NUMBERING_CONVERSION_IND					5045015   // sends a SLOTS_NUMBERING_CONVERSION_TABLE_S structure
//#define CARDS_SHELFMNGR_IND_LAST_OPCODE				5049998

//#define CARDS_SHELFMNGR_LAST_OPCODE					5049998



// =======================================
// ============= MPLAPI range ============
// =======================================
//#define MPLAPI_SHELFMNGR_FIRST_OPCODE					2020000

//#define MPLAPI_SHELFMNGR_REQ_FIRST_OPCODE				2020001
#define MPL_CTRL_IP_PARAMS_UPDATE_REQ					2020010   // sends an IP_PARAMS_S structure
//#define MPLAPI_SHELFMNGR_REQ_LAST_OPCODE				2024999

//#define MPLAPI_SHELFMNGR_IND_FIRST_OPCODE				2025000
#define MPL_CTRL_IP_CONFIG_IND							2025010   // sends an IP_PARAMS_S structure
//#define MPLAPI_SHELFMNGR_IND_LAST_OPCODE				2029998

//#define MPLAPI_SHELFMNGR_LAST_OPCODE					2029999



// =======================================
// ========= AUTHENTICATION range ========
// =======================================
//#define AUTHENTICATION_SHELFMNGR_FIRST_OPCODE			15020000
#define AUTHENTICATION_USERS_LIST_REQ					15020010   // sends a USERS_LIST_S structure
#define AUTHENTICATION_NEW_USER_REQ						15020011   // Not used! only AUTHENTICATION_USERS_LIST_REQ is used
#define AUTHENTICATION_UPDATE_USER_REQ					15020012   // Not used! only AUTHENTICATION_USERS_LIST_REQ is used
#define AUTHENTICATION_DELETE_USER_REQ					15020013   // Not used! only AUTHENTICATION_USERS_LIST_REQ is used
//#define AUTHENTICATION_SHELFMNGR_LAST_OPCODE			15029999


// ===========================================
// ============= DSP FATAL STATUS ============
// ===========================================
#define UNIT_FATAL_IND 						9020021
#define UNIT_UNFATAL_IND					9020022



#endif /*OPCODESSHELFMNGR_H_*/
