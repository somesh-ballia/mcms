#ifndef _DHTABLE
#define _DHTABLE

//#include "DHTask.h"
#include <list>
#include "ObjString.h"
#include  "Segment.h"
#include "EncryptionKeyServerAPI.h"

class CPObject;


#define POLYCOM_DH_GENERATOR 2
#define TANBERG_H320_DH_GENERATOR 5
#define TANBERG_H323_DH_GENERATOR 3

/////////////////////////////////////////////////////////////////////////////

class CDHKey : public CPObject
{
CLASS_TYPE_1(CDHKey, CPObject)
public:

    CDHKey(DWORD len = SHARED_SECRET_LENGTH);
    CDHKey(CDHKey& other);
    virtual ~CDHKey();
	CDHKey& operator= (const CDHKey& other);
    virtual const char*  NameOf() const;

	void  DeSerialize(WORD format,CSegment& seg, BYTE bIsIPKey = FALSE);
	void  Serialize(WORD format,CSegment& seg, BYTE bIsIPKey = FALSE);
	BYTE* GetArray();
	void  SetArray(BYTE *pString,DWORD len=SHARED_SECRET_LENGTH);
	DWORD GetLength() const;
	WORD  IsEqualsZero()  const;
	friend WORD  operator==(const CDHKey& rDHKey1,const CDHKey& rDHKey2);
	void    PrintToTrace();
	void CopyKeyToArray(BYTE * targetArr)const;

protected:

	BYTE   *m_pKeyArray;
	DWORD	m_length;
};

/////////////////////////////////////////////////////////////////////////////
class CdhEntry : public CPObject
{
CLASS_TYPE_1(CdhEntry,CPObject )
public:

	// Constructors
	CdhEntry();
	virtual ~CdhEntry() {};
	virtual const char*  NameOf() const {return "CdhEntry";};
	const CDHKey& GetHalfKey() const {return m_halfkey;};
	const CDHKey& GetRandomX() const {return m_random_x;};
    friend WORD operator==(const CdhEntry&,const CdhEntry&);
	WORD CreateRandom();
	int  GenerateHKWithPrime(DWORD g, BYTE* prime);
	void  DeSerialize(WORD format,CSegment& seg) ;
	void  Serialize(WORD format,CSegment& seg)   ;

protected:
	CDHKey m_halfkey;	//g^X
	CDHKey m_random_x;	//X
};

// typedef std::list < CdhEntry * > List_OF_CdhEntry;
void GetHalfKeyWithPrime(DWORD g,CDHKey* rnd, CDHKey* hk, BYTE* prime);

#endif

