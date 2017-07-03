//+========================================================================+
//                     GideonGuiApi.h                                      |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonGuiApi.h                                              |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __GIDEOGUIAPI_
#define   __GIDEOGUIAPI_


////////////////////////////////////////////////////////////////////////////
///
///     CONSTANTS
///
	// limits
const unsigned long MAX_SIM_GUI_MSG_LEN		= 10000;
	// message types
const unsigned long SIM_GUI_MSG_TYPE_GET	= 1;
const unsigned long SIM_GUI_MSG_TYPE_SET	= 2;
	// action types
		// get actions
const unsigned long GET_CONFIG				= 1001;
const unsigned long GET_CARDS_LIST			= 1002;
const unsigned long GET_CARD_DETAILS		= 1003;
		// set actions
const unsigned long SET_CONFIG				= 2001;
const unsigned long SET_ADD_CARD			= 2002;
const unsigned long SET_UPDATE_CARD			= 2003;
const unsigned long SET_REMOVE_CARD			= 2004;


////////////////////////////////////////////////////////////////////////////
///
///     DATA TYPES
///



#endif /* __GIDEOGUIAPI_ */

