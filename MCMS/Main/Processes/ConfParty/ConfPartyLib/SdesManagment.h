//+========================================================================+
//                       SdesManagment.h                                   |
//            Copyright 1995 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SdesManagment.h	                                           |
// SUBSYSTEM:  ConfParty                                                   |
// PROGRAMMER: Boris S.													   |
// Date : 15/12/10														   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+
/*

This header file contains the encryption info within the Carmel.

*/

#ifndef SDESMANAGMENT_H_
#define SDESMANAGMENT_H_

#include "ConfPartyGlobals.h"
#include "EncryptionKeyServerAPI.h"

class CSegment;

#define MAX_NUM_OF_CRYPTO_LINES			3
#define MAX_SDES_MASTER_KEY_LEN			129
#define MAX_SDES_MASTER_SALT_KEY_LEN	113
#define SIZE_OF_112_SDES_KEY			14
#define SIZE_OF_CONC_SDES_KEY			30

void CreateBase64MasterSaltKey(char **pBase64MasterSaltKey);
BOOL IsMasterSaltKeyValid(char *pBase64MasterSaltKey, int *pDecodedLen, int *pEncodedLen);



#endif /*SDESMANAGMENT_H_*/
