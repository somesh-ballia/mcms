#ifndef _BYPASS_H_
#define	_BYPASS_H_

// include files
#include "PObject.h"

// definitions
#define TEST_RESULT unsigned char
#define TEST_PASSED 0
#define TEST_FAILED 1

#define HASH_BSIZE 20
#define MAX_PROTECTED_DATA_ARRAY_LENGTH		HASH_BSIZE*8


class CObjString;

// double definitions from DHLib - to remove after include issue solved
//#define BSIZE	20
//typedef struct RNGContextStr {
//    unsigned char		XKEY[BSIZE]; /* Seed for next SHA iteration */
//    unsigned char		Xj[BSIZE];   /* Output from previous operation */
//    unsigned char		avail;       /* # bytes of output available, [0...20] */
//    unsigned int		seedCount;   /* number of seed bytes given to generator */
//    int		isValid;     /* false if RNG reaches an invalid state */
//}RNGContext;



////////////////////////////////////////////////////////////////////////////////////////////
// Class CProtectedArray created: 05/2006 feature: FIPS 140 (bypass test) programer: ron l.
// Array of bytes that protected by test (to be implemented in derived classes, like Hash in our case, Checksum ...) 
// All the functions that updates the protected data must test the protected data before and after update it
////////////////////////////////////////////////////////////////////////////////////////////
class CProtectedArray : public CPObject
{
CLASS_TYPE_1(CProtectedArray,CPObject)
public:
	// constructor
	CProtectedArray();
	virtual ~CProtectedArray();

	// CPObject functions
	const char*  NameOf() const;

	// API to user
	// must be only const functions
	BYTE GetByte(WORD index) const;
	void Dump(CObjString* ostr = NULL) const;
	// all functions that updates protected data must take the tests
	// and return the first failure result if failed / 0 if pass
	virtual TEST_RESULT SetByte(WORD index, BYTE value);


protected:
	// interface for derived classes that implement tests (Hash, checksum..)
	virtual TEST_RESULT TestBeforeUpdate()=0;
	virtual TEST_RESULT TestAfterUpdate()=0;

private:

	// attributes
	BYTE m_protected_data_Array[MAX_PROTECTED_DATA_ARRAY_LENGTH];
};

////////////////////////////////////////////////////////////////////////////////////////////
// Class CHashType created: 05/2006 feature: FIPS 140 (bypass test) programer: ron l.
// This class implement array of bits used as input and output of algorithm's hash function
// We are translating (protected data)bytes array -> bits array -(hash)-> hashd value bits array 
////////////////////////////////////////////////////////////////////////////////////////////
class CHashType : public CPObject
{
CLASS_TYPE_1(CHashType,CPObject)
public:
	// constructors
	CHashType();
	CHashType(BYTE* hashBytesArray);
	virtual ~CHashType();
	// CPObject functions
	const char*  NameOf() const;
	// operators
	CHashType& operator=(const CHashType& other);
	friend int operator==(const CHashType& hash1,const CHashType& hash2);
	friend int operator!=(const CHashType& hash1,const CHashType& hash2);
	// API to build the bits mask
	void SetBit(WORD index);
	void ResetBit(WORD index);
	// API to compare
	BYTE GetByte(WORD index)const;
	// API to use for alg hash function
	BYTE* GetBytesArray();
	// print function
	void Dump(DWORD level);

private:
	// attributes
	unsigned char m_pHashDat[HASH_BSIZE];
};

////////////////////////////////////////////////////////////////////////////////////////////
// Class CHashedArray created: 05/2006 feature: FIPS 140 (bypass test) programer: ron l.
// This class implement Hash test for base class CProtectedArray
// It uses algorithm's function Hashing_Fips186_1_x3_3 defined in AuthLib\SHA1.c
// before any change of the protected data it test that the hased value did not change
////////////////////////////////////////////////////////////////////////////////////////////
class CHashedArray : public CProtectedArray
{
CLASS_TYPE_1(CHashedArray,CProtectedArray)
public:
	// constructor
	CHashedArray();
	virtual ~CHashedArray();

	// CPObject functions
	const char*  NameOf() const;
	// dump to trace
	void Dump(DWORD level);

	virtual TEST_RESULT SetByte(WORD index, BYTE value);
	
protected:
	// pure virtual function of CProtectedArray
	TEST_RESULT TestBeforeUpdate(); // implement the hash test
	TEST_RESULT TestAfterUpdate(); // will be empty - always pass

private:
	CHashType Hash() const;

	// attributes
	CHashType m_hashed_value; // last Hashed value of protected data
};
////////////////////////////////////////////////////////////////////////////////////////////

#endif// _BYPASS_H_
