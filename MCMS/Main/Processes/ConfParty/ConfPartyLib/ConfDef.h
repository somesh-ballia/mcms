//+========================================================================+
//                            CONFDEF.H                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CONFDEF.H                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Zohar                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//Zohar| 18/1/2004  | Common definition for CONF1.cpp + CONF2 files        |
//+========================================================================+

#ifndef _CONFDEF_H__
#define _CONFDEF_H__



extern "C" void PartyEntryPoint(void* appParam);
extern  const char* GetStatusAsString(const int status);


// conference states
const WORD SETUP         = 1;
const WORD TERMINATION   = 2;
const WORD CONNECT       = 3;
const WORD CHANGEMODE    = 4;

// data & mlp requests types
const WORD  OPEN           = 1;
const WORD  CLOSE          = 2;
const WORD  CHANGEBITRATE  = 3;

// volume flags
static const BYTE	INCREASE_VOLUME = 120;
static const BYTE	DECREASE_VOLUME = 121;

// if there is a problem with this constant speak with Yuri R
//#define   PASSWORD  ""   // "STAMPASS"
#define   DELAY_BETWEEN_PARTIES_CHANGE_SPEED SECOND

//Added to the join conference feature to mark a conference that an operator is not
// joining to her.
#define		INVALID_CONF_ID		 0xffffffff


#endif //_CONFDEF_H__
