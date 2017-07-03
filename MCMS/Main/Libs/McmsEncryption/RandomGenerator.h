#ifndef __RANDOMGENERATOR_H__
#define __RANDOMGENERATOR_H__

#include <time.h>
//#include "SystemFunctions.h"
//extern UINT32 SystemGetTickCount();


/***** Type Definition *****/
#if !defined(FAR)
	#define FAR 
#endif /* INT8 */


#if !defined(TYPE_INT8)
#if !defined(Int8)
	typedef char Int8;
	#define TYPE_INT8
#endif
#endif /* INT8 */

#if !defined(TYPE_INT16)
	typedef short Int16, *pInt16;
	#define TYPE_INT16
#endif /* INT16 */

#if !defined(TYPE_INT32)
	typedef int Int32;
	#define TYPE_INT32
#endif /* INT8 */

#if !defined(TYPE_INT40)
	typedef long Int40, *pInt40;
	#define TYPE_INT40
#endif /* INT40 */

#if !defined(TYPE_INT64)
	#if defined(_TMS320C6X)
		typedef long long Int64, *pInt64;
	#elif defined(_MSC_VER)
		typedef __int64 Int64, *pInt64;
	#endif /* GCC || SCC || VC++ */
	#define TYPE_INT64
#endif /* INT64 */

#if !defined(TYPE_Uint8)
	typedef unsigned char Uint8, *pUint8;
	#define TYPE_Uint8
#endif /* UINT8 */

#if !defined(TYPE_UINT16)
	typedef unsigned short Uint16;
	#define TYPE_UINT16
#endif /* UINT16 */

#if !defined(TYPE_Uint32)
	typedef unsigned int Uint32;
	#define TYPE_Uint32
#endif /* UINT8 */

#if !defined(TYPE_UINT40)
	typedef unsigned long Uint40;
	#define TYPE_UINT40
#endif /* UINT40 */

#if !defined(TYPE_UINT64)
	#if defined(_TMS320C6X)
		typedef long long Uint64, *pUint64;
	#elif defined(_MSC_VER)
		typedef __int64 Uint64, *pUint64;
	#endif /* GCC || SCC || VC++ */
	#define TYPE_UINT64
#endif /* UINT64 */

#if !defined(TYPE_BOOL)
	typedef Int32 Bool;
	#define TYPE_BOOL
#endif /* BOOL */

#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif

#ifndef UINT32
#define UINT32	unsigned long
#endif

#ifndef INT32
#define INT32	long
#endif

#ifndef UINT8
#define UINT8	unsigned char
#endif

#ifndef INT8
#define INT8	char
#endif

#ifndef BOOL
#define BOOL	INT32	
#endif

extern  UINT32 GetTime(void);

/* Period parameters */  
#define L 624
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */   
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

// Global variables 
void	sgenrand(UINT32 seed);
static UINT32	genrand();
void			InitSeedRegRng();
INT32			InitSeed();

INT32 CreateRandom128Bits(UINT8 *pucRandKey);
INT32 CreateRandom128BitsOpenSSL(UINT8 *pucRandKey);
INT32 CreateRandom112BitsOpenSSL(UINT8 *pucRandKey);
INT32 CreateRandom1024Bits(UINT8 *pucRandKey, UINT8 useOpenSSLFuncYesNo, char* traceBuffer);
void CreateRandomNLongs(UINT8 *pucRandKey, UINT32	unNumOfLongs);


/************************* FIPS 140 *******************************************/
/*
 * Needed types, APIs, and variables collected
 * from multiple files from the NSS code.
 */

// FIPS140 return code Opcodes
typedef enum{
	E_FIPS_RV_OK = 0,
	E_FIPS_RV_ERROR_PRG_INIT_FAILURE,
	E_FIPS_RV_ERROR_RANDOMIZATION_FAILURE,
	E_FIPS_RV_ERROR_SAME_SEED_DIFFERENT_RES,
	E_FIPS_RV_ERROR_SEED_VALUE_FAILURE,	
	E_FIPS_RV_ERROR_SHA1_FAILURE,
	E_FIPS_RV_ERROR_AES_FAILURE,
	E_FIPS_RV_ERROR_SHA256_FAILURE,
	E_FIPS_RV_END
} enFipsRvOpcodes;

/*
 * Steps taken from FIPS 186-1 Appendix 3.1
 */

/*
 * According to FIPS 186-1, 160 <= b <= 512
 * For our purposes, we will assume b == 160
 */
#define BSIZE			20	// 20 bytes

/*
 * Add two 160-bit numbers represented as arrays of 20 bytes.
 * The numbers are big-endian, MSB first, so addition is done
 * from the end of the buffer to the beginning.
 */
#define ADD_160BIT_PLUS_CARRY(dest, add1, add2, cy) \
    carry = cy; \
    for (i=BSIZE-1; i>=0; --i) { \
	carry += add1[i] + add2[i]; \
	dest[i] = (UINT8)carry; \
	carry >>= 8; \
    }

#define ADD_160BIT_2(dest, add1, add2) \
	ADD_160BIT_PLUS_CARRY(dest, add1, add2, 0)

/*
 * FIPS requires result from Step 3c to be reduced mod q when generating
 * random numbers for DSA.
 * by definition q >= 2^159 + 1, thus xj < 2q
 * thus reducing mod q is simple subtraction when xj > q
 */
#define dsa_reduce_mod_q(xj, q) \
    if (memcmp(xj,q,BSIZE) > 0) { \
	carry = 0; \
	for (i=BSIZE-1; i>=0; --i) { \
	    carry += (INT32)xj[i] - (INT32)q[i]; \
	    xj[i] = (UINT8)carry; \
	    carry >>= 8; \
	} \
    }

/*
 * Global RNG context
 */ 
typedef struct RNGContextStr {
    UINT8		XKEY[BSIZE]; /* Seed for next SHA iteration */
    UINT8		Xj[BSIZE];   /* Output from previous operation */
    UINT8		avail;       /* # bytes of output available, [0...20] */
    UINT32		seedCount;   /* number of seed bytes given to generator */
    BOOL		isValid;     /* false if RNG reaches an invalid state */
}RNGContext;


INT32 alg_fips186_1_x3_1(	RNGContext *rng,
							const UINT8 *pucXSEEDj,
							UINT8 *q);
INT32 alg_fips186_1_x3_1_A(	RNGContext *rng,
							const UINT8 *pucXSEEDj,
							UINT8 *q);


//INT32 TestFIPS140_RNG();
void InitPrgWithSameSeedValue();
INT32 TestFIPS140_RNGWithSameSeedValue();
//INT32 TestFIPS140_StartupTest();
/************************* FIPS 140 *******************************************/

//Simulation mlsec clock
#define GET_TIME  time((time_t*)NULL) 
#endif
