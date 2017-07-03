// File       : SHA1.h
// Date       : April 2004 
// Author     : Gil Mazor
#ifndef __SHA1_H__
#define __SHA1_H__

#ifdef __IpPlus__
#include <CommonDefs.h>
#include <RtpProcessorDefs.h>
#include <DecodeGlobals.h>
#include <TdmIf.h>
#include <TdmParams.h>
#include <TimerRequests.h>
#else

#include <stdlib.h>
//#include <malloc.h>
//#include <memory.h>
#include <string.h>

#ifndef BOOL
typedef int					BOOL;
#endif

#ifndef UINT32
typedef unsigned int		UINT32;
#endif

#ifndef DWORD
typedef         unsigned long     DWORD; 
#endif

#ifndef BYTE
typedef         unsigned char     BYTE;                                            
#endif

#ifndef INT32
typedef int					INT32;
#endif

#ifndef UINT8
typedef unsigned char		UINT8;
#endif
#ifndef NULL
#define NULL    ((void *)0)
#endif
	
#ifndef TRUE
#define TRUE				1
#endif

#ifndef FALSE
#define FALSE				0
#endif

#endif


//Macros
#define HASH_BLOCK_SIZE_IN_BYTES	64	// There are 64 bytes in hash block
#define HASH_BLOCK_SIZE_IN_WORDS	16	// There are 16 WORDS in hash block
#define DIGESTED_SIZE_IN_WORDS		5	// There are 5  WORDS in the digested message
#define DIGESTED_SIZE_IN_BYTES		20	// There are 20 Bytes in the digested message
#define DH_KEY_SIZE_IN_BYTES		128

#define SHIFT_X_CIRCULAR_LEFT(val,x) ((val << x)|(val >> (32 - x)))

#define Func1(b,c,d) ((b & c) | ((~b) & d))
#define Func2(b,c,d) (b ^ c ^ d)
#define Func3(b,c,d) ((b & c) | (b & d) | (c & d))
#define Func4(b,c,d) Func2(b,c,d)

//Source Code
#ifdef __SIM__
#define GETU32(p) SWAP(*((UINT32 *)(p)))
#define PUTU32(ct, st) { *((UINT32 *)(ct)) = SWAP((st)); }
#define SWAP(a)	((((UINT32)(a) & 0x000000FF)<<24)	| \
				(((UINT32)(a) & 0x0000FF00)<<8)		| \
				(((UINT32)(a) & 0x00FF0000)>>8)		| \
				(((UINT32)(a) & 0xFF000000)>>24))
#else
#define GETU32(pt) (*((UINT32*)(pt)))
#define PUTU32(Tgt, Src) {(*((UINT32*)(Tgt))) = (Src);}
#endif

#define TEST(key,keylen,text,textlen,result,id) \
{\
	if (!testcase(key,keylen,text,textlen,result)) \
		return id;\
}

#define TEST_PRF(key,keylen,text,textlen,result,id) \
{\
	if (!testPRFcase(key,keylen,text,textlen,result)) \
	return id;\
}

//Function defenition
BOOL	HashingMessage(	UINT8	*pucMessage,
						UINT8	*pucDigestedMessage,
						UINT32	unLength);

UINT32	PaddingMessage(	UINT8	*pucMessage,
						UINT32	unLength);

void	DigestBlock(	UINT8	*pucMessage,
						UINT32	*punDigestedMessage);

BOOL	HMAC(			UINT8	*pucKey,
						UINT32	unKeyLength,
						UINT8	*pucData,
						UINT32	unDataLength,
						UINT8	*pucDigested);

BOOL	PRF(			UINT8	*pucSrc,
						UINT32	unSrcLength,
						UINT8	*pucLabel,
						UINT32	unLabelLength,
						UINT8	*pucTrg,
						UINT32	unTrgLength);

BOOL	PRF_Round(		UINT8	*pucSrc,
						UINT32	unSrcLength,
						UINT8	*pucLabel,
						UINT32	unLabelLength,
						UINT8	*pucTrg,
						UINT32	unNumOfIterations);

BOOL Hashing_Fips186_1_x3_3(	UINT8 *pucMessage, 
				UINT8 *pucDigestedMessage, 
				UINT32 unLength);
void	TestPRFModule();

BOOL	TestVectorsPRF();

BOOL	testPRFcase(	UINT8 *key,
						UINT32 keylen,
						UINT8 *text,
						UINT32 textlen,
						UINT8 *result);

BOOL	testcase(		UINT8 *key,
						UINT32 keylen,
						UINT8 *text,
						UINT32 textlen,
						UINT8 *result);

BOOL	TestVectors();
#endif


