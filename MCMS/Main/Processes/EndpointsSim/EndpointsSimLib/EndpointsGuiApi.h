//+========================================================================+
//                    EndpointsGuiApi.h                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EndpointsGuiApi.h                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __ENDPOINTSGUIAPI_
#define   __ENDPOINTSGUIAPI_


////////////////////////////////////////////////////////////////////////////
///
///     CONSTANTS
///
	// limits
const DWORD MAX_SIM_GUI_MSG_LEN		= 50000;
	// message types
const DWORD SIM_GUI_MSG_TYPE_GET	= 1;
const DWORD SIM_GUI_MSG_TYPE_SET	= 2;
	// action types
		// get actions
const DWORD GET_TEST				= 1001;
const DWORD GET_CONFIG				= 1002;
		// set actions
const DWORD SET_TEST				= 2001;
const DWORD SET_CONFIG				= 2002;


//+++++
///     EVENT OPCODES
///
// 10201 - 10250
const DWORD ADD_CAP_REQ				= 10201;
const DWORD DELETE_CAP_REQ			= 10202;
const DWORD UPDATE_CAP_REQ			= 10203;
const DWORD GET_CAP_LIST_REQ		= 10204;
const DWORD GET_ONE_CAP_REQ			= 10205;
const DWORD ADD_EP_REQ				= 10206;
const DWORD GET_EP_LIST_REQ			= 10207;
const DWORD DELETE_EP_REQ			= 10208;
const DWORD UPDATE_EP_REQ			= 10209;
const DWORD ADD_BEHAVIOR_REQ		= 10210;
const DWORD DELETE_BEHAVIOR_REQ		= 10211;
const DWORD UPDATE_BEHAVIOR_REQ		= 10212;
const DWORD GET_BEHAVIOR_LIST_REQ	= 10213;
const DWORD SEND_DTMF_REQ			= 10214;
const DWORD SEND_DTMF_IND			= 10215;
const DWORD EP_CONNECT_REQ			= 10216;
const DWORD EP_DISCONNECT_REQ		= 10217;
const DWORD PLAY_SOUND_REQ			= 10218;
const DWORD ADD_SCRIPT_REQ			= 10219;
const DWORD PLAY_SOUND_REQ_REQ		= 10220; /*for test only*/
/* not implemented yet
const DWORD GET_SCRIPT_LIST_REQ		= 10221;
const DWORD START_SCRIPT_REQ		= 10222;
const DWORD GET_SCRIPT_LIST_REQ		= 10223;
const DWORD GET_SCRIPT_LIST_REQ		= 10224;
const DWORD GET_SCRIPT_LIST_REQ		= 10225;
const DWORD GET_SCRIPT_LIST_REQ		= 10226;
const DWORD GET_SCRIPT_LIST_REQ		= 10227;
const DWORD GET_SCRIPT_LIST_REQ		= 10228;
const DWORD GET_SCRIPT_LIST_REQ		= 10229;
const DWORD GET_SCRIPT_LIST_REQ		= 10230;
 */
const DWORD EP_MUTE_REQ					= 10231;
const DWORD EP_UNMUTE_REQ				= 10232;
const DWORD EP_AUDIO_SPEAKER_REQ		= 10233;
const DWORD EP_ACTIVE_SPEAKER_REQ		= 10234;
const DWORD EP_FECC_ASK_TOKEN_REQ		= 10235;
const DWORD EP_FECC_RELEASE_TOKEN_REQ	= 10236;
const DWORD EP_H239_ASK_TOKEN_REQ		= 10237;
const DWORD EP_H239_RELEASE_TOKEN_REQ	= 10238;
const DWORD EP_DETAILS_REQ				= 10239;
const DWORD EP_ENDPOINT_CHANNELS_REQ	= 10240;
const DWORD EP_SIP_CS_SIG_REINVITE_IND	= 10241;
const DWORD EP_FECC_KEY_REQ             = 10242;

const DWORD GUI_PLAY_MUSIC				= 10301;
const DWORD GUI_STOP_PLAY_MUSIC			= 10302;
const DWORD GUI_ADD_EP_RANGE_REQ		= 10303;

// External database opcodes
const DWORD GUI_EXTDB_STATUS			= 10401;
const DWORD GUI_EXTDB_RECORDS_LIST		= 10402;


// GateKeeper
const DWORD GATEKEEPER_EPSIM_UNREGISTER_MCU = 10403;



////////////////////////////////////////////////////////////////////////////
///
///     DATA TYPES
///
//typedef struct
//{
//   WORD  packet_number;
//   WORD  number_of_packets;
//} GUI_PACKET_INFO_S;




#endif /* __ENDPOINTSGUIAPI_ */

