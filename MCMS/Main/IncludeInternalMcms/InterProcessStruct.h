//+========================================================================+
//                   InterProcessStruct.h                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       InterProcessStruct.h                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __INTERPROCESSSTRUCT_
#define   __INTERPROCESSSTRUCT_


////////////////////////////////////////////////////////////////////////////
///
///     INCLUDES
///
//#include "DataTypes.h"


////////////////////////////////////////////////////////////////////////////
///
///     DECLARATIONS
///


////////////////////////////////////////////////////////////////////////////
///
///     GLOBALS
///


////////////////////////////////////////////////////////////////////////////
///
///     EXTERNALS
///


////////////////////////////////////////////////////////////////////////////
///
///     DATA TYPES
///



////////////////////////////////////////////////////////////////////////////
///
///     CONSTANTS
///



////////////////////////////////////////////////////////////////////////////
///
///     ERROR CODES
///
//const DWORD ERR_CONF_MPL_PARTY_ALREADY_CONNECTED	= 1001;
//const DWORD ERR_CONF_MPL_PARTY_NOT_CONNECTED		= 1002;



////////////////////////////////////////////////////////////////////////////
///
///     MESSAGES OPCODES
///
//const DWORD CONF_MPL_ADD_NEW_PARTY		= 9001;
//const DWORD CONF_MPL_OPEN_PORT			= 9002;
//const DWORD CONF_MPL_CLOSE_PORT			= 9003;
//const DWORD CONF_MPL_DELETE_PARTY		= 9004;
//const DWORD CONF_MPL_CONNECT			= 9011;
//const DWORD CONF_MPL_DISCONNECT			= 9012;

//const DWORD CONF_MPL_ACK_REQ			= 9098;
//const DWORD CONF_MPL_BAD_REQ			= 9099;

//const DWORD CONF_MPL_ADD_NEW_PARTY_ACK	= 9101;
//const DWORD CONF_MPL_OPEN_PORT_ACK		= 9102;


////////////////////////////////////////////////////////////////////////////
///
///     REQUEST / INDICATION SPECIFIC STRUCTURES
///
//typedef struct
//{
//	BYTE   nParam1;
//	BYTE   nParam2;
//	DWORD  dwParam1;
//	DWORD  dwParam2;
//} CONF_MPL_ADD_NEW_PARTY_S;
//
//typedef struct
//{
//	DWORD  dwParam1;
//	DWORD  dwParam2;
//} CONF_MPL_OPEN_PORT_S;
//
//typedef struct
//{
//	DWORD  dwParam3;
//	DWORD  dwParam4;
//	DWORD  dwParam5;
//} CONF_MPL_CLOSE_PORT_S;
//
//typedef struct
//{
//	DWORD  dwParam1;
//	DWORD  dwParam2;
//} CONF_MPL_DELETE_PARTY_S;
//
//typedef struct
//{
//	DWORD  dwParam1;
//	DWORD  dwParam2;
//} CONF_MPL_CONNECT_S;
//
//typedef struct
//{
//	DWORD  dwParam1;
//	DWORD  dwParam2;
//} CONF_MPL_DISCONNECT_S;
//
//typedef struct
//{
//	DWORD  ack_opcode;
//} CONF_MPL_ACK_REQ_S;
//
//typedef struct
//{
//	DWORD  bad_opcode;
//	DWORD  error_num;
//} CONF_MPL_BAD_REQ_S;
//

#endif /* __INTERPROCESSSTRUCT_ */


