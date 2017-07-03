//+========================================================================+
//                        SimApi.h                                         |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SimApi.h                                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __SIMAPI_
#define   __SIMAPI_


////////////////////////////////////////////////////////////////////////////
///
///     INCLUDES
///
#include "DataTypes.h"


////////////////////////////////////////////////////////////////////////////
///
///     DECLARATIONS
///
//class CEndpointsSimSystemCfg;


////////////////////////////////////////////////////////////////////////////
///
///     GLOBALS
///
//static CEndpointsSimSystemCfg* g_pEpSystemCfg = NULL;


////////////////////////////////////////////////////////////////////////////
///
///     EXTERNALS
///
	//system configuration
//extern CEndpointsSimSystemCfg* GetEpSystemCfg();
//extern void SetEpSystemCfg(CEndpointsSimSystemCfg* p);


////////////////////////////////////////////////////////////////////////////
///
///     CONSTANTS
///


////////////////////////////////////////////////////////////////////////////
///
///     DATA TYPES
///
//enum eCardTypes {
//	eCardSwitch  = 0,
//	eCardMfa     = 1,
//	eCardMfaRtm  = 2,
//	eCardUnknown
//};


////////////////////////////////////////////////////////////////////////////
///
///     MESSAGES OPCODES
///
	// messages between Gideon Sim and Endpoints Sim
const OPCODE SIM_API_PSTN_MSG		= 70001;
const OPCODE SIM_API_ISDN_MSG		= 70002;
const OPCODE SIM_API_T1CAS_MSG		= 70003;
const OPCODE SIM_API_H323_MSG		= 70004;
const OPCODE SIM_API_SIP_MSG		= 70005;
const OPCODE SIM_API_CS_MSG			= 70006;
const OPCODE SIM_API_AUDIO_MSG		= 70007;
const OPCODE SIM_API_MUX_MSG		= 70008;

	//messages between GideonSim and MediaMngr
const OPCODE MM_INDICATIONS_MSG		= 70009;
const OPCODE MM_GS_ICE_MSG			= 70010;

    // more messages between Gideon Sim and Endpoints Sim
const OPCODE SIM_API_MRM_MSG        = 70011;

	// messages between Gideon Sim and Gideon GUI
const OPCODE AAAAAAAAAAA			= 40001;

	// messages between Endpoints Sim and Endpoints GUI
const OPCODE BBBBBBBBBBB			= 50001;

	// messages between Gideon Sim and MPL API
const OPCODE SIM_WAIT_TIMER					= 11002;
const OPCODE SIM_GUI_SOCKET_RCV_MSG			= 11003;
const OPCODE SIM_MESSAGE_FOR_LOGGER			= 11004;
const OPCODE SIM_PCM_SOCKET_RCV_MSG			= 11005;
const OPCODE SIM_PCM_TIMER					= 11006;
const OPCODE EPGUI_PCM_SOCKET_RCV_MSG		= 11007;
	// messages between Ema-Engine and MediaMngr
const OPCODE SIM_API_MEDIA_MSG				= 60001;

	// messages between command line and MediaMngr
const OPCODE SIM_API_CMD_MEDIA_MSG			= 60002;

	// messages between python and MediaMngr
const OPCODE SIM_API_RESET_CHANNEL_OUT_MSG	= 60003;
const OPCODE SIM_API_VIDEO_UPDATE_PIC_MSG	= 60004;





#endif /* __SIMAPI_ */

