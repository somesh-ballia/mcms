//+========================================================================+
//                            ENCRAUTH.H                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ENCRAUTH.H                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Anat Elia                                              |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 23/6/04    | Utilities fot IP parties		                       |
//+========================================================================+


#ifndef _ENCRAUTH
#define _ENCRAUTH

#include "EncryptionKey.h"



typedef enum
{
	e_authenticationKey,
	e_encryptionKey,
}keyTypes;

#define GRQMODE 0
#define RRQMODE 0
#define ARQMODE 0
#define LRQMODE 0
#define GCFMODE 1
#define RCFMODE 1
#define ACFMODE 1
#define LCFMODE 1

#define PasswordLen 30
APIU32  GetEncrySectionLen(CIpDHKeyManagement	*pDHKeyManagment) ;
//void	SetEncryptionHeader(encTokensHeaderStruct& encryTokens,BOOL bIsTransmiting,CIpDHKeyManagement	*pDHKeyManagement);
//void	SetEncryptionTokenParams(encryptionToken& encryTokens,BOOL bIsTransmiting,CIpDHKeyManagement *pDHKeyManagment,CDHKey *pHalfKey=NULL);
void	SetEncryptionParams(encTokensHeaderStruct& encryTokens,BOOL bIsTransmiting,CIpDHKeyManagement	*pDHKeyManagement,CDHKey *pHalfKey=NULL,BOOL bNeedToCheck=TRUE);
void	CalculateSharedSecret(CIpDHKeyManagement *pDHKeyManagement);
void	GetAuthEncKey(APIU32 nonce1,APIU32 nonce2,unsigned char *pMaster,keyTypes keyType,unsigned char *pKey);
//UINT32  GetRandom(int numOfBit);
//void	EncryptedLocalHalfKey(BYTE *pHalfKey,e_encrHKeyMsg msgType,char *pPswKey,BYTE *pE_hKey,char *pIV);
void	DecryptedHalfKey(BYTE *pIv,BYTE *pE_RmtHalfKey,char *m_pswKey,BYTE *pHkey);
void	SetAuthEncrParams(CIpDHKeyManagement *pDHKeyManagement,BYTE *pAuthKey,BYTE *pEncKey,int indexTbl=-1);
#endif /* _ENCRAUTH  */
