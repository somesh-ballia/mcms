
#include "DHTable.h"
#include "DHTask.h"
#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "ProcessBase.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ObjString.h"


void GetHalfKeyWithPrime(DWORD g,CDHKey* rnd, CDHKey* hk, BYTE* prime);

// DH Half Keys
extern "C" int	GenerateHalfKeyWithPrime(DWORD unKeyGen, BYTE *pucRandomNum,BYTE *pucHalfKey, BYTE *prime);
extern "C" int	CreateRandom1024Bits(BYTE *pucRandKey, UINT8 useOpenSSLFuncYesNo, char* traceBuffer);
/////////////////////////////////////////////////////////////////////////////////////////////////
// Encryption
//////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////  CDHEntry ///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CdhEntry::CdhEntry()
{
}
/////////////////////////////////////////////////////////////////////////////
void  CdhEntry::Serialize(WORD format,CSegment& seg)   
{
	m_halfkey.Serialize(format,seg);
	m_random_x.Serialize(format,seg);
}
/////////////////////////////////////////////////////////////////////////////
void  CdhEntry::DeSerialize(WORD format,CSegment& seg) 
{
	m_halfkey.DeSerialize(format,seg);
	m_random_x.DeSerialize(format,seg);
}

/////////////////////////////////////////////////////////////////////////////
WORD CdhEntry::CreateRandom()
{
	WORD fips140Status = STATUS_OK;
    
	BOOL isOpenSSLFunc = NO;
	CProcessBase* pProcess = CProcessBase::GetProcess();
	if (pProcess!=NULL && pProcess->GetSysConfig()!=NULL)
		 pProcess->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_OPENSSL_ENC_FUNC, isOpenSSLFunc);
	//CLargeString cstr
	//cstr << "openssl flag is: " << isOpenSSLFunc << "\n";
															
	ALLOCBUFFER(bufferToTrace,256);
	fips140Status = CreateRandom1024Bits(m_random_x.GetArray(),isOpenSSLFunc,bufferToTrace);
/////////////////////////////////////////////////////////////////////
//  
//       debug traces -
//		
//			cstr << bufferToTrace << "\n";
//			cstr << "RANDOM AFTER: 0x";

//			for(int i=0;i<HALF_KEY_SIZE;i++)
//				cstr<<(BYTE)m_random_x.GetArray()[i];
//			PTRACE(eLevelInfoNormal,cstr.GetString());
		/////////////////////////////////////////////////////////
	DEALLOCBUFFER(bufferToTrace);
		/////////////////////////////////////////////////////////
	
	return fips140Status;
}
/////////////////////////////////////////////////////////////////////////////
int CdhEntry::GenerateHKWithPrime(DWORD g, BYTE* prime)
{
	return GenerateHalfKeyWithPrime(g,m_random_x.GetArray(),m_halfkey.GetArray(), prime);
}

/////////////////////////////////////////////////////////////////////////////
WORD operator==(const CdhEntry& lhs, const CdhEntry& rhs)
{

	if(!(lhs.m_random_x  == rhs.m_random_x))
		return FALSE;

	if(!(lhs.m_halfkey  == rhs.m_halfkey))
		return FALSE;

	return TRUE;
}
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
		m_length = SHARED_SECRET_LENGTH;

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
		m_length = SHARED_SECRET_LENGTH;

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
/////////////////////////////////////////////////////////////////////////////
void CDHKey::CopyKeyToArray(BYTE * targetArr)const
{
	std::copy(m_pKeyArray , m_pKeyArray +  m_length , targetArr);
}




