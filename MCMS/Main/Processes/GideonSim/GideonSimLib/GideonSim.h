//+========================================================================+
//                       GideonSim.h                                       |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSim.h                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __GIDEONSIM_
#define   __GIDEONSIM_


////////////////////////////////////////////////////////////////////////////
///
///     INCLUDES
///
#include "DataTypes.h"
#include "TraceHeader.h"


////////////////////////////////////////////////////////////////////////////
///
///     DECLARATIONS
///
class CPObject;
class CSegment;
class CGideonSimSystemCfg;
class CGideonSimCardAckStatusList;

////////////////////////////////////////////////////////////////////////////
///
///     GLOBALS
///
static CGideonSimSystemCfg* g_pGideonSystemCfg = NULL;
static CGideonSimCardAckStatusList* g_pCardAckStatusList = NULL;


////////////////////////////////////////////////////////////////////////////
///
///     EXTERNALS
///
	//system configuration
extern CGideonSimSystemCfg* GetGideonSystemCfg();
extern void SetGideonSystemCfg(CGideonSimSystemCfg* p);
extern CGideonSimCardAckStatusList* GetCardAckStatusList();
extern void SetCardAckStatusList(CGideonSimCardAckStatusList* p);

extern void SendIsdnMessageToEndpointsSimApp(CSegment& pParam);
extern void SendAudioMessageToEndpointsSimApp(CSegment& pParam);
extern void SendMuxMessageToEndpointsSimApp(CSegment& pParam);
extern void SendMRMMessageToEndpointsSimApp(CSegment& rParam);
extern void SendMessageToMediaMngrApp(CSegment& pParam);


extern void SendMessageToLogger(const WORD level,
				 const CPObject * pObj,
				 const char * message1,
				 const char * message2,
				 const DWORD  topic_id 	= DEFAULT_TOPIC_ID,
				 const char * terminalName = NULL,
				 const DWORD  unit_id 	= DEFAULT_UNIT_ID,
				 const DWORD  conf_id 	= DEFAULT_CONF_ID,
				 const DWORD  party_id 	= DEFAULT_PARTY_ID,
				 const OPCODE opcode 	= DEFAULT_OPCODE,
				 const char * str_opcode = NULL);

//added by huiyu
extern void SendMessageToPCMSimApp(CSegment& rParam);
////////////////////////////////////////////////////////////////////////////
///
///     DEFINES
///
#define LOGGER_TRACE(a,b)				SendMessageToLogger(a,this,b,NULL)
#define LOGGER_TRACE1(a,b)				SendMessageToLogger(a,NULL,b,NULL)
#define LOGGER_TRACE2(a,b,c)			SendMessageToLogger(a,this,b,c)
#define LOGGER_TRACE2INT(a,b,c)			SendMessageToLogger(a,this,b,c)
#define LOGGER_TRACE2HINT(a,b,c)		SendMessageToLogger(a,this,b,c)
#define LOGGER_TRACE2INT2(a,b,c)		SendMessageToLogger(a,this,b,c)

////////////////////////////////////////////////////////////////////////////
///
///     DATA TYPES
///
enum eCardTypes {
	eCardSwitch  	= 0,
	eCardMfa     	= 1,
	eCardGideonLite = 2,
	eCardBarak		= 3,
	eCardBreeze		= 4,
	eCardMpmRx      = 5,
	eCardUnknown
};


////////////////////////////////////////////////////////////////////////////
///
///     CONSTANTS
///

////////////////////////////////////////////////////////////////////////////
///
///     EVENT OPCODES
///
const OPCODE MNGR_NEW_CARD				= 10001;
const OPCODE CONNECT_CARD				= 10002;
const OPCODE FORWARD_MSG_MPLAPI			= 10003;
const OPCODE PCM_MSG_TO_MPLAPI			= 10004;

//  switch reports
const OPCODE SWITCH_CONNECTED			= 10101;

const OPCODE SWITCH_SEND_REMOVE_CARD_IND = 10300;
const OPCODE SWITCH_SEND_INSERT_CARD_IND = 10301;

//  mfa reports
const OPCODE MFA_CONNECTED				= 10102;
const OPCODE MFA_DELAY_TOUT				= 10103;
const OPCODE MFA_INSERT_SUB_CARD		= 10104;

//  Barak reports
const OPCODE BARAK_CONNECTED			= 10105;
const OPCODE BARAK_OR_BREEZE_INSERT_SUB_CARD = 10106;

//UPGRADE VERSION
const OPCODE UPDATE_BURN_RATE    		= 10200;
const OPCODE SET_BURN_ACTIONS    		= 10201;

//const OPCODE WAIT_TIMER					= 11002;
//const OPCODE GUI_SOCKET_RCV_MSG			= 11003;
//const OPCODE MESSAGE_FOR_LOGGER			= 11004;

//const OPCODE SWITCH_CONFIG_TIMER		= 11003;
//const OPCODE MFA_UNIT_CONFIG_TIMER		= 11004;
//const OPCODE MFA_MEDIA_CONFIG_TIMER		= 11005;

// batch commands  11201 - 11400
const OPCODE BATCH_GIDEON_SIM_COMMAND			= 11201;
const OPCODE BATCH_SOCKETS_ACTION				= 11202;
const OPCODE SET_UNIT_STATUS_FOR_KEEP_ALIVE_NEW	= 11203;
const OPCODE SIM_API_MEDIA_CARD_SET_PARAMS		= 11204;
const OPCODE SIM_API_ISDN_SET_TIMERS			= 11205;
const OPCODE SIM_API_RTM_ENABLE_DISABLE_PORTS	= 11206;
const OPCODE SIM_USER_LDAP_LOGIN				= 11207;
const OPCODE SIM_RESET_CARD						= 11208;

////////////////////////////////////////////////////////////////////////////
///
///     TEMP INDICATIONS


#endif /* __GIDEONSIM_ */
