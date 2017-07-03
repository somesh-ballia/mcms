//+========================================================================+
//                            EncryptionKey.CPP                            |
//            Copyright 2003 Polycom Israel Ltd.				           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Ltd. and is protected by law.					   |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EncryptionKey.CPP                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Guy D.                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |19/12/05     |                                                     |
//+========================================================================+

#include "EncryptionKey.h"
#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "ProcessBase.h"
#include "SysConfig.h"
#include "ConfPartyProcess.h"
#include "SystemFunctions.h"

//////////////////////////////////////////////////////////////////////////////
/////////////////  CDHKey ///////////////////////////////////////////////////
CDHKey::CDHKey(DWORD len)       // constructor
{
	m_length = len;
	m_pKeyArray = new BYTE[m_length];
}
/////////////////////////////////////////////////////////////////////////////
CDHKey::CDHKey(CDHKey& other)
        :CPObject(other)
{
	if(other.m_pKeyArray!=NULL)
	{
		m_length	= other.m_length;
		m_pKeyArray = new BYTE[m_length];
		for(DWORD i=0;i<m_length;i++)
			m_pKeyArray[i] = other.m_pKeyArray[i];
	}
	else
	{
		m_pKeyArray = NULL;
		m_length = 0;
	}
}
/////////////////////////////////////////////////////////////////////////////
CDHKey& CDHKey::operator=(const CDHKey& other)
{
	if ( &other == this ) return *this;
	if(other.m_pKeyArray!=NULL)
	{
		PDELETEA(m_pKeyArray);			
		m_length	= other.m_length;
		m_pKeyArray = new BYTE[m_length];

		for(DWORD i=0;i<m_length;i++)
			m_pKeyArray[i] = other.m_pKeyArray[i];
	}
	else
	{
		PDELETEA(m_pKeyArray);
		m_length = 0;
	}

	return *this;
}
/////////////////////////////////////////////////////////////////////////////
CDHKey::~CDHKey()        // destructor
{
	PDELETEA(m_pKeyArray);
}

/////////////////////////////////////////////////////////////////////////////
const char*   CDHKey::NameOf()  const
{
    return "CDHKey";
}
/////////////////////////////////////////////////////////////////////////////
void CDHKey::Serialize(WORD format,CSegment &seg, BYTE bIsIPKey)
{
	if(m_pKeyArray==NULL)
		return;
	if (bIsIPKey)
		seg << (DWORD)m_length;
	else
		m_length = HALF_KEY_SIZE;

	for(DWORD i=0;i < m_length;i++)
		seg<<(BYTE)m_pKeyArray[i];
}
/////////////////////////////////////////////////////////////////////////////
void CDHKey::DeSerialize(WORD format,CSegment &seg, BYTE bIsIPKey)
{
	PDELETEA(m_pKeyArray);
	
	if (bIsIPKey)
		seg >> m_length;
	else
		m_length = HALF_KEY_SIZE;

	m_pKeyArray = new BYTE[m_length];
	for(DWORD i=0;i<m_length;i++)
		seg>> m_pKeyArray[i];
}
/////////////////////////////////////////////////////////////////////////////
BYTE*   CDHKey::GetArray()
{
    return m_pKeyArray;
}
/////////////////////////////////////////////////////////////////////////////
void   CDHKey::SetArray(BYTE *pString, DWORD len)
{
	PDELETEA(m_pKeyArray);

	m_length = len;
	m_pKeyArray = new BYTE[len];

	memcpy(m_pKeyArray,pString,len);
}
/////////////////////////////////////////////////////////////////////////////
DWORD CDHKey::GetLength() const
{
	return m_length;
}
/////////////////////////////////////////////////////////////////////////////
WORD  operator==(const CDHKey& rDHKey1,const CDHKey& rDHKey2)
{
	if(rDHKey1.GetLength() != rDHKey2.GetLength())
		return FALSE;

	DWORD len = rDHKey1.GetLength(); //the size are equal so we can take one of them.

    for(DWORD i=0;i < len;i++)
		if (rDHKey1.m_pKeyArray[i] != rDHKey2.m_pKeyArray[i])
			return FALSE;
		
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
WORD  CDHKey::IsEqualsZero()  const
{
    for(DWORD i=0;i < m_length;i++)
		if (m_pKeyArray[i]!=0)
			return FALSE;
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
void    CDHKey::PrintToTrace()
{	
	CLargeString trace;
	trace.SetFormat("%x"); 

	for(DWORD i=0;i < m_length;i++)
	{
		trace<<" 0x";
		trace<<(BYTE)m_pKeyArray[i];
	}

//	PTRACE2(eLevelInfoNormal/*|ECS_TRACE*/,"CDHKey::PrintToTrace: ", trace.GetString());

}

/////////////////  CDHKeyManagement ///////////////////////////////////////////////////
CDHKeyManagement::CDHKeyManagement()          // constructor
{
  	m_pDHLocalRnd			= NULL;	
	m_pDHRmtSharedSecret	= NULL;	
	m_pDHLocalSharedSecret	= NULL;
	m_pDHResultSharedSecret = NULL;	
	m_pRmtPrime				= NULL;
	m_generator				= 0;

}
/////////////////////////////////////////////////////////////////////////////
CDHKeyManagement::CDHKeyManagement(CDHKeyManagement& other):CPObject(other)
{
  	m_pDHLocalRnd			= other.m_pDHLocalRnd;	
	m_pDHRmtSharedSecret	= other.m_pDHRmtSharedSecret;	
	m_pDHLocalSharedSecret	= other.m_pDHLocalSharedSecret;
	m_pDHResultSharedSecret = other.m_pDHResultSharedSecret;	
	m_pRmtPrime = other.m_pRmtPrime;
	m_generator = other.m_generator;
}
/////////////////////////////////////////////////////////////////////////////
CDHKeyManagement& CDHKeyManagement::operator=(const CDHKeyManagement& other)
{
	if ( &other == this ) return *this;

  	m_pDHLocalRnd			= other.m_pDHLocalRnd;	
	m_pDHRmtSharedSecret	= other.m_pDHRmtSharedSecret;	
	m_pDHLocalSharedSecret	= other.m_pDHLocalSharedSecret;
	m_pDHResultSharedSecret = other.m_pDHResultSharedSecret;
	m_pRmtPrime = other.m_pRmtPrime;
	m_generator = other.m_generator;
	return *this;
}
/////////////////////////////////////////////////////////////////////////////
CDHKeyManagement::~CDHKeyManagement()        // destructor
{
	POBJDELETE(m_pDHLocalRnd);
	POBJDELETE(m_pDHRmtSharedSecret);
	POBJDELETE(m_pDHLocalSharedSecret);
	POBJDELETE(m_pDHResultSharedSecret);
	POBJDELETE(m_pRmtPrime);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CDHKeyManagement::CheckKeyManagementProcessCompleted()
{
	if(	m_pDHLocalRnd==NULL||m_pDHRmtSharedSecret==NULL||m_pDHLocalSharedSecret==NULL||m_pDHResultSharedSecret==NULL || m_pRmtPrime==NULL)
		return FALSE;

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////
void CDHKeyManagement::SetDHRmtSharedSecret(CDHKey &other)
{
	POBJDELETE(m_pDHRmtSharedSecret);
	m_pDHRmtSharedSecret = new CDHKey(other);
}
////////////////////////////////////////////////////////////////////////////
void CDHKeyManagement::SetDHResultSharedSecret(CDHKey &other)
{
	POBJDELETE(m_pDHResultSharedSecret);
	m_pDHResultSharedSecret = new CDHKey(other);
}
/////////////////  CIpDHKeyManagement ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
CIpDHKeyManagement::CIpDHKeyManagement()
{
	m_localHalfKeyAlg	= kHalfKeyUnKnownType;
	m_rmtHalfKeyAlg		= kHalfKeyUnKnownType;
	m_pMaster			= new CDHKey;
	m_pAuthCall			= new CDHKey;
	m_pEncCall			= new CDHKey;
}
/////////////////////////////////////////////////////////////////////////////
CIpDHKeyManagement::CIpDHKeyManagement(CIpDHKeyManagement& other):CDHKeyManagement(other)
{
	m_localHalfKeyAlg	= other.m_localHalfKeyAlg;
	m_rmtHalfKeyAlg		= other.m_rmtHalfKeyAlg;
	m_pMaster			= other.m_pMaster;
	m_pAuthCall			= other.m_pAuthCall;
	m_pEncCall			= other.m_pEncCall;
}
/////////////////////////////////////////////////////////////////////////////
CIpDHKeyManagement::~CIpDHKeyManagement()
{
	if (m_pMaster) POBJDELETE(m_pMaster);
	if (m_pAuthCall) POBJDELETE(m_pAuthCall);
	if (m_pEncCall) POBJDELETE(m_pEncCall);
}
/////////////////////////////////////////////////////////////////////////////
CIpDHKeyManagement& CIpDHKeyManagement::operator=(const CIpDHKeyManagement& other)
{
	if ( &other == this ) return *this;

	(CDHKeyManagement &)*this = (const CDHKeyManagement &)other;
	m_localHalfKeyAlg	= other.m_localHalfKeyAlg;
	m_rmtHalfKeyAlg		= other.m_rmtHalfKeyAlg;
	m_pMaster			= other.m_pMaster;
	m_pAuthCall			= other.m_pAuthCall;
	m_pEncCall			= other.m_pEncCall;
	
	return *this;
}
////////////////////////////////////////////////////////////////////////////
void CIpDHKeyManagement::InitLocalHalfKey(const EncryptedKey& encryptedKey)
{
	m_pDHLocalRnd			= new CDHKey; //g^X pSecretNumber;
	m_pDHLocalSharedSecret	= new CDHKey; //XpLocalHalfKey;
	memcpy(m_pDHLocalRnd->GetArray(),encryptedKey.GetRandomNumber(),HALF_KEY_SIZE);
	memcpy(m_pDHLocalSharedSecret->GetArray(),encryptedKey.GetHalfKey(),HALF_KEY_SIZE);
	PTRACE(eLevelInfoNormal/*|ECS_TRACE*/,"CIpDHKeyManagement::SetLocalHalfKey : Local Random X: ");
	PTRACE(eLevelInfoNormal/*|ECS_TRACE*/,"CIpDHKeyManagement::SetLocalHalfKey : Local HK: ");

}
////////////////////////////////////////////////////////////////////////////
void CIpDHKeyManagement::SetGenerator(BYTE generator)
{
	if(generator != POLYCOM_DH_GENERATOR)
		PTRACE(eLevelInfoNormal,"CIpDHKeyManagement::SetGenerator: generator different POLYCOM_DH_GENERATOR");

	if(!m_generator)
		m_generator	= (BYTE)generator;
}
////////////////////////////////////////////////////////////////////////////
DWORD CIpDHKeyManagement::GetLocalHalfKeyLen() const
{
	if(m_localHalfKeyAlg == kHalfKeyDH1024)
		return HALF_KEY_SIZE; //1024 / 8 -> we send 1024 bits that is 128 bytes
	else
		return 0;
}
////////////////////////////////////////////////////////////////////////////
void CIpDHKeyManagement::SetEncCallKey(unsigned char *pStr,int startIndex)
{
	if(pStr)
		m_pEncCall->SetArray(&pStr[startIndex],XMIT_RCV_KEY_LENGTH);
}

/*
////////////////////////////////////////////////////////////////////////////
void CIpDHKeyManagement::SetMasterKey(unsigned char *pStr)
{
	m_pMaster->SetArray(pStr,LengthMasterKey);
}
*/

