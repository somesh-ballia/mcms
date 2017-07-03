//+========================================================================+
//                     BFCPMessageNew.h                                   |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       BFCPMessageNew.h                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Lior Baram												   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 01/04/08   | This file is an extension to the BFCPMessage file    |
//     |            | with functions to complete the BFCP implementation   |
//+========================================================================+

#if !defined(_BFCP_MESSAGE_NEW_H)
#define _BFCP_MESSAGE_NEW_H
#include "BFCPMessage.h"
#include <errno.h>
#define getlasterror() errno
/*---------------------------------------------------------
// BuildBFCPSupportedAttributes
//
// Build BFCP SUPPORTED-Attributes element
//---------------------------------------------------------*/

int BuildBFCPSupportedAttributes (void *pHeader, int Mandatory);

/*---------------------------------------------------------
// BuildBFCPSupportedAttributes
//
// Build BFCP SUPPORTED-PRIMITIVES element
//---------------------------------------------------------*/
int BuildBFCPSupportedPrimitives (void *pHeader, int Mandatory);

#endif   /*  _BFCP_MESSAGE_NEW_H */
