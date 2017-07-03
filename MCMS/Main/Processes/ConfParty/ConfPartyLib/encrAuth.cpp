//+========================================================================+
//                            ENCRAUTH.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ENCRAUTH.CPP                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Anat Elia                                                   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 23/6/04    | Utilities fot IP parties		                       |
//+========================================================================+


  
#include "encrAuth.h"


extern "C" int	GenerateSharedSecretWithPrime(BYTE *pstHKey, int pstLen, BYTE *pstRandNum, BYTE *pucShardKey, BYTE *pucPrime);
//extern void GetHalfKeyWithPrime(BYTE g,CDHKey* rnd, CDHKey* hk, BYTE* prime);

////////////////////////////////////////////////////////////////////////////
APIU32  GetEncrySectionLen(CIpDHKeyManagement	*pDHKeyManagment) 
{
	APIU32 len = sizeof(encryptionTokenBase);
	len += pDHKeyManagment->GetLocalHalfKeyLen();

	return len;
}
/*
////////////////////////////////////////////////////////////////////////////
void SetEncryptionHeader(encTokensHeaderStruct& encryTokens,BOOL bIsTransmiting,CIpDHKeyManagement	*pDHKeyManagement)
{
	BOOL bIsEncrypted = TRUE;
	EenHalfKeyType localHkeyAlg = (EenHalfKeyType)pDHKeyManagement->GetLocalHalfKeyAlg();
	CDHKey *pLocalHKey = new CDHKey;

	if(!bIsTransmiting) //dial in
	{
		if(pDHKeyManagement->GetRmtHalfKeyAlg() != kHalfKeyDH1024)
			bIsEncrypted= FALSE;
	}

	if(bIsEncrypted && localHkeyAlg == kHalfKeyDH1024)
	{
		encryTokens.numberOfTokens = 1;	//For now we support only for one token.
		encryTokens.dynamicTokensLen = GetEncrySectionLen(pDHKeyManagement);
	}
	else
	{
		encryTokens.numberOfTokens	= 0;	
		encryTokens.dynamicTokensLen = 0;
	}

}
////////////////////////////////////////////////////////////////////////////
void  SetEncryptionTokenParams(encryptionToken& token,BOOL bIsTransmiting,CIpDHKeyManagement	*pDHKeyManagement,
						  CDHKey *pHalfKey)
{
	BOOL bIsEncrypted = TRUE;
	EenHalfKeyType localHkeyAlg = (EenHalfKeyType)pDHKeyManagement->GetLocalHalfKeyAlg();
	CDHKey *pLocalHKey = new CDHKey;

	if(!bIsTransmiting) //dial in
	{
		if(pDHKeyManagement->GetRmtHalfKeyAlg() != kHalfKeyDH1024)
			bIsEncrypted= FALSE;
	}

	if(bIsEncrypted && localHkeyAlg == kHalfKeyDH1024)
	{
		if(pHalfKey)
			*pLocalHKey = *pHalfKey;
		else 
			*pLocalHKey = *pDHKeyManagement->GetDHLocalSharedSecret();
		
		token.tokenOID	= localHkeyAlg;
		token.generator	= (UINT32)pDHKeyManagement->GetGenerator(); //In dial out we will chose generator 2 - as polycom standard.
		token.modSize	= 0;	
		token.hkLen		= pDHKeyManagement->GetLocalHalfKeyLen();
		token.filler		= 0;
					
		memcpy(&token.halfKey,pLocalHKey->GetArray(),token.hkLen);

		POBJDELETE(pLocalHKey);

	}
}*/

////////////////////////////////////////////////////////////////////////////
void  SetEncryptionParams(encTokensHeaderStruct& encryTokens,BOOL bIsTransmiting,CIpDHKeyManagement	*pDHKeyManagement,
						  CDHKey *pHalfKey,BOOL bNeedToCheck)
{
	BOOL bIsEncrypted = TRUE;
	EenHalfKeyType localHkeyAlg = (EenHalfKeyType)pDHKeyManagement->GetLocalHalfKeyAlg();
	CDHKey *pLocalHKey = new CDHKey;

	if(!bIsTransmiting) //dial in
	{
		if(bNeedToCheck && (pDHKeyManagement->GetRmtHalfKeyAlg() != kHalfKeyDH1024))
			bIsEncrypted= FALSE;
	}

	if(bIsEncrypted && localHkeyAlg == kHalfKeyDH1024)
	{
		if(pHalfKey)
			*pLocalHKey = *pHalfKey;
		else 
			*pLocalHKey = *pDHKeyManagement->GetDHLocalSharedSecret();
		
		encryTokens.xmlDynamicProps.numberOfDynamicParts = 1;
		encryTokens.xmlDynamicProps.sizeOfAllDynamicParts = sizeof(encryptionTokenBase) + pDHKeyManagement->GetLocalHalfKeyLen();
		encryTokens.numberOfTokens = 1;	//For now we support only for one token.
		encryTokens.dynamicTokensLen = GetEncrySectionLen(pDHKeyManagement);

		((encryptionToken*)(encryTokens.token))->xmlHeader.dynamicType = tblEncToken;
		((encryptionToken*)(encryTokens.token))->xmlHeader.dynamicLength = sizeof(encryptionTokenBase) + pDHKeyManagement->GetLocalHalfKeyLen();	
		((encryptionToken*)(encryTokens.token))->tokenOID	= localHkeyAlg;
		((encryptionToken*)(encryTokens.token))->generator	= (APIU32)pDHKeyManagement->GetGenerator(); //In dial out we will chose generator 2 - as polycom standard.
		((encryptionToken*)(encryTokens.token))->modSize	= 0;	
		((encryptionToken*)(encryTokens.token))->hkLen		= pDHKeyManagement->GetLocalHalfKeyLen();
		((encryptionToken*)(encryTokens.token))->filler		= 0;

		//m_pKeyArray set by 'new' so if it's NULL we have a much bigger problem.
		if (pLocalHKey->GetArray() != NULL)
		{
			memcpy(
					&(((encryptionToken*)(encryTokens.token))->halfKey),
					pLocalHKey->GetArray(),
					((encryptionToken*)(encryTokens.token))->hkLen);
		}

	}
	else //No encryption
	{
		encryTokens.numberOfTokens	= 0;	
		encryTokens.dynamicTokensLen = 0;
	}
	POBJDELETE(pLocalHKey);
}

////////////////////////////////////////////////////////////////////////////
void CalculateSharedSecret(CIpDHKeyManagement *pDHKeyManagement)
{
	CDHKey *pResultSharedSecret = new CDHKey; 

	FPTRACE(eLevelInfoNormal,"::CalculateSharedSecret : Remote half key g^x: ");
	pDHKeyManagement->GetDHRmtSharedSecret()->PrintToTrace();
	FPTRACE(eLevelInfoNormal,"::CalculateSharedSecret : Local half key g^x: ");
	pDHKeyManagement->GetDHLocalSharedSecret()->PrintToTrace();
	FPTRACE(eLevelInfoNormal,"::CalculateSharedSecret : Local secret number X: ");
	pDHKeyManagement->m_pDHLocalRnd->PrintToTrace();
	
	GenerateSharedSecretWithPrime(pDHKeyManagement->GetDHRmtSharedSecret()->GetArray(),
		pDHKeyManagement->GetDHRmtSharedSecret()->GetLength(), 
		pDHKeyManagement->m_pDHLocalRnd->GetArray(), 
		pResultSharedSecret->GetArray(), polycomPrime);
	
	pDHKeyManagement->SetDHResultSharedSecret(*pResultSharedSecret);	

	FPTRACE(eLevelInfoNormal,"::CalculateSharedSecret : Result shared secret: ");
	pResultSharedSecret->PrintToTrace();
	POBJDELETE(pResultSharedSecret);

}
////////////////////////////////////////////////////////////////////////////
void SetAuthEncrParams(CIpDHKeyManagement *pDHKeyManagement,BYTE *pAuthKey,BYTE *pEncKey,int indexTbl)
{
	if(pAuthKey)
		pDHKeyManagement->GetAuthCallKey()->SetArray(pAuthKey,LengthEncAuthKey);
	if(pEncKey)
		pDHKeyManagement->GetEncrCallKey()->SetArray(pEncKey,LengthEncAuthKey);

	if(indexTbl != -1)
		pDHKeyManagement->SetIndexTblAuth(indexTbl);
}

////////////////////////////////////////////////////////////////////////////
void GetAuthEncKey(APIU32 nonce1,APIU32 nonce2,unsigned char *pMaster,keyTypes keyType,unsigned char *pKey)
{
	//call Gil function that initiate pAuthRasKey

	//Anat debug
	CDHKey *pLocalHalfKey	= new CDHKey; //g^X
	CDHKey *pSecretNumber	= new CDHKey; //X

//	::GetHalfKeyWithPrime(2, pSecretNumber, pLocalHalfKey, polycomPrime);

	memcpy(pKey,pLocalHalfKey,LengthEncAuthKey);
	POBJDELETE(pLocalHalfKey);
	POBJDELETE(pSecretNumber);

	//call Gil function that initiate pAuthRasKey
}
/*
//////////////////////////////////////////////////////////////////////////////
void EncryptedLocalHalfKey(BYTE *pHalfKey,e_encrHKeyMsg msgType,char *pPswKey,BYTE *pE_hKey,char *pIV)
{
	//Call Gil functions...
	memcpy(pE_hKey,pHalfKey,128);
}*/

//////////////////////////////////////////////////////////////////////////////
void DecryptedHalfKey(BYTE *pIv,BYTE *pE_RmtHalfKey,char *m_pswKey,BYTE *pHkey)
{
	//Calll Gil functions..
	memcpy(pHkey,pE_RmtHalfKey,128);

}
