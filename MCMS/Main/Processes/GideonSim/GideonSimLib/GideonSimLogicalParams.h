/*
 * GideonSimLogicalParams.h
 *
 *  Created on: Feb 3, 2010
 *      Author: elads
 */

#ifndef GIDEONSIMLOGICALPARAMS_H_
#define GIDEONSIMLOGICALPARAMS_H_


//#define	LICENSE_FILE_NAME  "Simulation/License.cfs"
//#define VASILY_MAX_LICENSE_LEN  1024

extern char* CardUnitConfiguredTypeToString(APIU32 unitConfigType);
extern const char* UnitReconfigStatusToString(APIU32 unitReconfigStatus);

static DWORD st_AudioControllerBoardId = 0;
static DWORD st_AudioControllerSubBoardId = 0;


/////////////////////////////////////////////////////////////////////////////
//
//   STATES:
//
//const WORD  IDLE        = 0;        // default state -  defined in base class
const WORD  SETUP         = 1; // (?)
const WORD  STARTUP       = 2;
const WORD  CONFIG        = 3;
const WORD  CONNECT       = 4;
const WORD  RECOVERY      = 5; // (?)
const WORD  PAUSE         = 6;
//const WORD  ANYCASE     = 0xFFFF;   // any other state -  defined in base class


/////////////////////////////////////////////////////////////////////////////
//
//   EVENTS:
//
const OPCODE CM_STARTUP      = 10101;
const OPCODE CM_MCMS_MSG     = 10102;
const OPCODE CM_EP_SIM_MSG   = 10103;
const OPCODE CM_MM_MSG		 = 10104;
const OPCODE BARAK_CS_INTERNAL_CONFIG_COMPLETE            = 10105;
const OPCODE BARAK_CS_EXTERNAL_CONFIG_COMPLETE            = 10106;
const OPCODE BARAK_DNAT_CONFIG_COMPLETE            = 10107;

const OPCODE SWITCH_CONFIG_TIMER		= 10201;
const OPCODE MFA_MEDIA_CONFIG_COMPLETE_TIMER   = 10210;
const OPCODE MFA_UNIT_CONFIG_TIMER		= 10211;
const OPCODE MFA_MEDIA_CONFIG_TIMER		= 10221;
const OPCODE CARD_NOT_READY_DELAY_TIMER	= 10222;
const OPCODE MFA_SPEAKER_CHANGE_TIMER	= 10223;
const OPCODE CM_LOADED_TEST_TIMER		= 10224; // for testing VNGR-1150
const OPCODE SPAN_STATUS_TIMER			= 10225; // for testing VNGR-1150

const OPCODE BARAK_UNIT_CONFIG_TIMER	= 10226;
const OPCODE BARAK_MEDIA_CONFIG_TIMER	= 10227;
const OPCODE BARAK_SPEAKER_CHANGE_TIMER	= 10228;

const OPCODE ISDN_CFG_TIMER	    		= 10229; // olga for testing


const OPCODE PACKET_LOSS_TIMER	        = 10240;

const OPCODE SOFTWARE_UPGRADE_TIMER          = 10241;
const OPCODE SOFTWARE_IPMC_UPGRADE_TIMER     = 10242;

const OPCODE CHECK_COMPLETE_TIMER       = 10243;
// IC events
const OPCODE IC_IVR_PLAY_RESPONSE_TOUT	= 11001;

//BFCP events
const OPCODE BFCP_HELLO_TIMER			= 10251;
const OPCODE BFCP_HELLO_ACK_TIMER		= 10252;

//const OPCODE SET_UNIT_STATUS_FOR_KEEP_ALIVE    = 10224;
const DWORD  MFA_CARDS_DELAY_TIME	= 1 * SECOND;
const DWORD  BARAK_CARDS_DELAY_TIME	= 3 * SECOND;
const DWORD  SWITCH_CARDS_DELAY_TIME  = 1 * SECOND;
const DWORD  RTM_CARDS_DELAY_TIME = 3 * SECOND;

const DWORD  MFA_CONFIG_COMPLETED_TIME	= 1 * SECOND;

// IC timer
const DWORD  IC_IVR_RESPONSE_TIME	= 1 * SECOND;

//BFCP
const DWORD BFCP_SEND_HELLO_DELAY_TIME = 30 * SECOND;
const DWORD BFCP_RECEIVE_HELLO_ACK_TIMEOUT = 10 * SECOND;

#define IC_REMAIN_TIMES_DEFAULT           2 // for 3 minutes
#define IC_REMAIN_TIMES_ROLLCALL_RECORD   4 // for 5 minutes

static DWORD g_bondingAlignmentTime = 100;

const int MAX_AUDIO_PARTIES = 800;

enum eBurnTypes
{
	eIpmcBurnType = 0,
	eVersionBurnType,
};

enum eBurnActionTypes
{
	eStopBurn = 0,
	eStartBurn,
	ePauseBurn,
	eResumeBurn

};


#endif /* GIDEONSIMLOGICALPARAMS_H_ */
