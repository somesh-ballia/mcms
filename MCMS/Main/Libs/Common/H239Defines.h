//+========================================================================+
//                            H239Defines.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H239Defines.h                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 12/26/2007     |                                                     |
//+========================================================================+

#ifndef H239DEFINES_H_
#define H239DEFINES_H_

#include "DataTypes.h"
#include "Segment.h"
#include "Macros.h"


const BYTE RoleLabelParamIdent					= 0x01;
const BYTE BitRateParamIdent					= 0x29;
const BYTE ChannelIdParamIdent					= 0x2A;
const BYTE SymmetryBreakingParamIdent			= 0x2B;
const BYTE TerminalLabelParamIdent				= 0x2C;
const BYTE AcknowledgeParamIdent				= 0x7E;
const BYTE RejectParamIdent						= 0x7F;

const BYTE PresentationRoleLabel				= 0x01;
const BYTE LiveRoleLabel						= 0x02;

const BYTE AMC_CAP_LEN							= 0x03;

//H.239-message Sub Message Identifiers
const BYTE FlowControlReleaseRequest			= 0x01;
const BYTE FlowControlReleaseResponse			= 0x02;
const BYTE PresentationTokenRequest				= 0x03;
const BYTE PresentationTokenResponse			= 0x04;
const BYTE PresentationTokenRelease				= 0x05;
const BYTE PresentationTokenIndicateOwner		= 0x06;

//H.239-message AMC Channel ID values
const BYTE MainVideoChannel						= 0x01;
const BYTE SecondVideoChannel					= 0x02;

#define TERMINAL_LABEL_MULTIPLIER 256
#define SYMMETRY_BREAKING_FOR_WITHDRAW_TOKEN 0
#define SYMMETRY_BREAKING_FOR_AQCUIRE_TOKEN 127

int  IntToMbeBytes(BYTE* byteArray,int val);
void IntToMbeBytes(CSegment *seg,int value);
int  MbeBytesToInt(BYTE* pByteArray);
int  MbeBytesToInt(CSegment* pParam);
void SerializeGenericParameter(BYTE PID, int value, CSegment *seg);



#endif /*H239DEFINES_H_*/
