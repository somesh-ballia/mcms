/* It may not be copied or distributed in any form or medium, disclosed  to   */
/* third parties, reverse engineered or used in any manner without prior      */
/* written authorization from Polycom Israel Ltd.                             */
/*----------------------------------------------------------------------------*/
/* FILE:     	IpmiShared.h                                                  */
/* PROJECT:  	Shared module - IPMI API 									  */
/* PROGRAMMER:  Edwin Dong												  */
/* DESCRIPTION: 															*/
/* 																              */
/*----------------------------------------------------------------------------*/
/* Who     |      Date       |         Description                            */
/*----------------------------------------------------------------------------*/
/*         |                 |                                       		  */
/*============================================================================*/

#ifndef IPMI_SHARED_H_
#define IPMI_SHARED_H_

// IpmiRequest.h
/////////////////////////////////////////////////////////////
//Ipmi Request Types
#define IPMI_ENTITY_LIST_REQ		 512
#define IPMI_FRU_REQ				 513
#define IPMI_SENSOR_LIST_REQ		 514
#define IPMI_SENSOR_READING_LIST_REQ 515
#define IPMI_FAN_INFO_REQ			 516
#define IPMI_GET_FAN_LEVEL_REQ		 517
#define IPMI_SET_FAN_LEVEL_REQ		 518
#define IPMI_GET_LED_INFO_REQ		 519
#define IPMI_GET_LED_STATE_REQ		 520
#define IPMI_DO_RESET_REQ			 521
#define IPMI_GET_EVENT_LOG_REQ		 522

//Ipmi Status Types
/////////////////////////////////////////////////////////////
#define IPMI_CARD_STATUS_NORMAL		 		 0
#define IPMI_CARD_STATUS_MAJOR				 1
#define IPMI_CARD_STATUS_EMPTY				 2
#define IPMI_CARD_STATUS_RESETTING			 3
#define IPMI_CARD_STATUS_DIAGNOSTICS		 4


#define IPMI_SLOT_ID_TYPE_RMX_2000			0
#define IPMI_SLOT_ID_TYPE_MFA_1				1
#define IPMI_SLOT_ID_TYPE_MFA_2				2
#define IPMI_SLOT_ID_TYPE_CPU				3
#define IPMI_SLOT_ID_TYPE_IMA				4
#define IPMI_SLOT_ID_TYPE_SWITCH			5



// IpmiRequest.h
/////////////////////////////////////////////////////////////
//LanInfo Request Types
#define LAN_STAT_INFO_REQ			256
#define LAN_STAT_CLEAR_MAX_REQ		257
#define LAN_STAT_GET_PORTS_LIST_REQ 258

#define LAN_STAT_INFO_IND			256
#define LAN_STAT_CLEAR_MAX_IND		257
#define LAN_STAT_GET_PORTS_LIST_IND 258

#define PRIVATE_OPCODE				0x401

#endif /*IPMI_SHARED_H_*/

