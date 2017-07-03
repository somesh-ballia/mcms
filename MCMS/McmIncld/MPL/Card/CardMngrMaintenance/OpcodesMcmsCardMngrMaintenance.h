#ifndef OPCODESMCMSCARDMAINTENANCE_H_
#define OPCODESMCMSCARDMAINTENANCE_H_




// =======================================
// ============= CARDS range =============
// =======================================
//#define CARDS_CARDMNGR_MNTNCE_FIRST_OPCODE		5030000

//#define CARDS_CARDMNGR_MNTNCE_REQ_FIRST_OPCODE	5030001
#define  CM_UNIT_CONFIG_REQ                         5030010   // sends a CM_UNITS_CONFIG_S structure
#define  CM_MEDIA_IP_CONFIG_REQ                     5030011   // sends a MEDIA_IP_PARAMS_S structure
#define  CM_MEDIA_IP_CONFIG_END_REQ                 5030012
#define  CM_KEEP_ALIVE_REQ                          5030013
#define  CARDS_RESET_MFA_REQ                        5030014   // sends an MFA_RESET_S structure
#define  CM_UNIT_RECONFIG_REQ                      	5030015   // sends a UNIT_RECONFIG_S structure
//#define  CARDS_CM_MEDIA_RECORDING_REQ				5030016   // using STARTUP_DEBUG_RECORDING_PARAM_REQ instead
#define  ART_FIPS_140_REQ                           5030016   // sends an MFA_RESET_S structure
#define  SYSTEM_CARDS_MODE_REQ                     	5030017
#define  CM_RTM_MEDIA_IP_CONFIG_REQ               	5030018
#define  CM_CS_EXTERNAL_IP_CONFIG_REQ               5030019
#define  CM_CS_INTERNAL_IP_CONFIG_REQ               5030020
#define  CM_DNAT_CONFIG_REQ							5030021
#define  CM_MEDIA_CONFIGURATION_COMPLETED_REQ       5030022
#define  CM_DNS_MEDIA_CONFIG_REQ       				5030023
//#define CARDS_CARDMNGR_MNTNCE_REQ_LAST_OPCODE		5034999

//#define CARDS_CARDMNGR_MNTNCE_IND_FIRST_OPCODE	5035000
#define  CM_UNIT_LOADED_IND                         5035010   // sends a CM_UNIT_LOADED_S structure
#define  CM_MEDIA_IP_CONFIG_IND                     5035011   // sends a MEDIA_IP_CONFIG_S structure
#define  CM_KEEP_ALIVE_IND                          5035012   // sends a KEEP_ALIVE_S structure
#define  CM_CARD_MNGR_RECONNECT_IND					5035013   // sends no data (only opcode)
#define  CM_UNIT_RECONFIG_IND              			5035014   // sends a UNIT_RECONFIG_S structure
#define  ART_FIPS_140_IND							5035015
#define  CM_CS_EXTERNAL_IP_CONFIG_IND               5035016
#define  CM_CS_INTERNAL_IP_CONFIG_IND               5035017
#define  CM_DNS_CONFIG_IND							5035018
#define  CM_MEDIA_CONFIGURATION_COMPLETED_IND		5035019
// upgrade related
#define  CM_UPGRADE_NEW_VERSION_READY_REQ          5036020 //MCMS->CM //new version is ready to read from NF
#define  CM_UPGRADE_NEW_VERSION_READY_ACK_IND      5036021 //CM->MCMS ack
#define  CM_UPGRADE_PROGRESS_IND                   5036022 //CM->MCMS (contain UPGRADE_PROGRESS_IND_S) will be sent a gain and again until finished
#define  CM_UPGRADE_IPMC_IND                       5036023 //CM->MCMS (contain UPGRADE_IPMC_IND_S)
#define  CM_UPGRADE_START_WRITE_IPMC_REQ           5036024 //MCMS->CS  start to write the IPMC software (will be send only to HW that need)
#define  CM_UPGRADE_IPMC_PROGRESS_IND              5036025 //CM->MCMS (contain UPGRADE_PROGRESS_IND_S) will be sent a again and again until finished

//startup message    
#define CM_MEDIA_OVER_TCP_SERVER_LISTEN_PORTS 		5036027 //MCMS->CM for media over tcp - MCMS tells to mfa the port numbers for listen server  

//#define  CM_UPGRADE_IPMC_ALMOST_COMPLETE_IND       5036026 //CM->MCMS indicates that the IPMC write is going to finish in the near seconds.
//#define  OLD_SYSCFG_PARAMS_IND						5035017	  // sends a SYSCFG_PARAMS_S structure
//#define CARDS_CARDMNGR_MNTNCE_IND_LAST_OPCODE		5039998

//#define CARDS_CARDMNGR_MNTNCE_LAST_OPCODE			5039999




#endif /*OPCODESMCMSCARDMAINTENANCE_H_*/
