
#include "time.h"
#include "SHA1.h"
#include "RandomGenerator.h"
#include "AESLib.h"
#include <stdio.h>

// added for warning flag -Wimplicit-function-declaration
extern int FIPS_rand_bytes(unsigned char *buf, int num);

 /* A C-program for MT19937: Integer     version                   */
/*  genrand() generates one pseudorandom unsigned integer (32bit) */
/* which is uniformly distributed among 0 to 2^32-1  for each     */
/* call. sgenrand(seed) set initial values to the working area    */
/* of 624 words. Before genrand(), sgenrand(seed) must be         */
/* called once. (seed is any 32-bit integer except for 0).        */
/*   Coded by Takuji Nishimura, considering the suggestions by    */
/* Topher Cooper and Marc Rieffel in July-Aug. 1997.              */

/* This library is free software; you can redistribute it and/or   */
/* modify it under the terms of the GNU Library General Public     */
/* License as published by the Free Software Foundation; either    */
/* version 2 of the License, or (at your option) any later         */
/* version.                                                        */
/* This library is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.            */
/* See the GNU Library General Public License for more details.    */
/* You should have received a copy of the GNU Library General      */
/* Public License along with this library; if not, write to the    */
/* Free Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA   */ 
/* 02111-1307  USA                                                 */

/* Copyright (C) 1997 Makoto Matsumoto and Takuji Nishimura.       */
/* Any feedback is very welcome. For any question, comments,       */
/* see http://www.math.keio.ac.jp/matumoto/emt.html or email       */
/* matumoto@math.keio.ac.jp                                        */


static BOOL	 bIsRandGenInitialized = FALSE;
extern UINT32 accordTicksCount;

/************************* FIPS 140 *******************************************/


/*
 * The minimum amount of seed data required before the generator will
 * provide data.
 * Note that this is a measure of the number of bytes sent to
 * RNG_RandomUpdate, not the actual amount of entropy present in the
 * generator.  Naturally, it is impossible to know (at this level) just
 * how much entropy is present in the provided seed data.  A bare minimum
 * of entropy would be 20 bytes, so by requiring 1K this code is making
 * the tacit assumption that at least 1 byte of pure entropy is provided
 * with every 8 bytes supplied to RNG_RandomUpdate.  The reality of this
 * assumption is left up to the caller.
 */

static RNGContext tGlobalRng;
static RNGContext *ptGlobalRng = NULL;


/*
INT32 TestFIPS140_RNG()
{
	INT32	i,nRv;
	UINT32	*punTmp;

	if(!ptGlobalRng->isValid)
		return E_FIPS_RV_ERROR_PRG_INIT_FAILURE;
	
	punTmp = (UINT32*)ptGlobalRng->XKEY;
	punTmp ++;

	// Adding some seeding values
	*punTmp = GET_TIME;

	if(!(*punTmp))
		return E_FIPS_RV_ERROR_SEED_VALUE_FAILURE;

	ptGlobalRng->seedCount += 4;
			
	for(i=0;i<100;i++)
	{
		nRv = alg_fips186_1_x3_1(ptGlobalRng, NULL, NULL);
		if(nRv != E_FIPS_RV_OK)
			return nRv;
	}

	return nRv;
}
*/

/*
 * Implementation of the algorithm in FIPS 186-1 appendix 3.1, heretofore
 * called alg3_1().  It is assumed a lock for the global rng context has
 * already been acquired.
 * Calling this function with pucXSEEDj == NULL is equivalent to saying there
 * is no optional user input, which is further equivalent to saying that
 * the optional user input is 0.
 */
INT32 alg_fips186_1_x3_1(	RNGContext *rng,
							const UINT8 *pucXSEEDj,
							UINT8 *q)
{
    /* SHA1 context for G(t, aucXVAL) function */
  //  SHA1Context sha1cx;
	
    /* input to hash function */
    UINT8 aucXVAL[BSIZE];

    /* store a copy of the output to compare with the previous output */
    UINT8 aucX_j[BSIZE];

    /* used by ADD_160BIT macros */
    INT32 i, carry;

    if (!rng->isValid) {
		/* RNG has alread entered an invalid state. */
		return E_FIPS_RV_ERROR_PRG_INIT_FAILURE;
    }
    /* 
     * <Step 2> Initialize t, taken care of in SHA-1 (same initial values) 
	 *
     * <Step 3a> pucXSEEDj is optional user input
     * 
     * <Step 3b> aucXVAL = (XKEY + pucXSEEDj) mod 2^b
     *     :always reduced mod 2^b, since storing as 160-bit value
     */
	
    if (pucXSEEDj) {
		/* pucXSEEDj > 0 */
		ADD_160BIT_2(aucXVAL, rng->XKEY, pucXSEEDj);
    } else {
		/* pucXSEEDj == 0 */
		memcpy(aucXVAL, rng->XKEY, BSIZE);
    }
    /* 
     * <Step 3c> Xj = G(t, aucXVAL) mod q
     *     :In this code, (mod q) is only understood for DSA ops, 
     *     :not general RNG (what would q be in non-DSA uses?).
     *     :If a q is specified, use it.
     */ 

	if(!Hashing_Fips186_1_x3_3(	aucXVAL, aucX_j, BSIZE))
		return E_FIPS_RV_ERROR_SHA1_FAILURE;
	
    if (q != NULL) {
		dsa_reduce_mod_q(aucX_j, q);
    }

    /*     [FIPS 140-1] verify output does not match previous output */
    if (memcmp(aucX_j, rng->Xj, BSIZE) == 0) {
		/* failed FIPS 140-1 continuous RNG condition.  RNG now invalid. */
		rng->isValid = 0;
		return E_FIPS_RV_ERROR_RANDOMIZATION_FAILURE;
    }
    /* Xj is the output */
    memcpy(rng->Xj, aucX_j, BSIZE);
    /* 
     * <Step 3d> XKEY = (1 + XKEY + Xj) mod 2^b
     *     :always reduced mod 2^b, since storing as 160-bit value 
     */
    ADD_160BIT_PLUS_CARRY(rng->XKEY, rng->XKEY, aucX_j, 1);
	
    /* Always have a full buffer after executing alg3_1() */
    rng->avail = BSIZE;

    /* housekeeping */
    memset(aucX_j, 0, BSIZE);
    memset(aucXVAL, 0, BSIZE);
    return E_FIPS_RV_OK;
}
/*
 * alg_fips186_1_x3_1_A (RNG-A) is an instance of alg_fips186_1_x3_1 (RNG-B).
 * Used for Initial creation of Random number to Seed alg_fips186_1_x3_1 (RNG-B)
 */
INT32 alg_fips186_1_x3_1_A(	RNGContext *rng,
							const UINT8 *pucXSEEDj,
							UINT8 *q)
{
    /* SHA1 context for G(t, aucXVAL) function */
  //  SHA1Context sha1cx;
	
    /* input to hash function */
    UINT8 aucXVAL[BSIZE];

    /* store a copy of the output to compare with the previous output */
    UINT8 aucX_j[BSIZE];

    /* used by ADD_160BIT macros */
    INT32 i, carry;

    if (!rng->isValid) {
		/* RNG has alread entered an invalid state. */
		return E_FIPS_RV_ERROR_PRG_INIT_FAILURE;
    }
    /* 
     * <Step 2> Initialize t, taken care of in SHA-1 (same initial values) 
	 *
     * <Step 3a> pucXSEEDj is optional user input
     * 
     * <Step 3b> aucXVAL = (XKEY + pucXSEEDj) mod 2^b
     *     :always reduced mod 2^b, since storing as 160-bit value
     */
	
    if (pucXSEEDj) {
		/* pucXSEEDj > 0 */
		ADD_160BIT_2(aucXVAL, rng->XKEY, pucXSEEDj);
    } else {
		/* pucXSEEDj == 0 */
		memcpy(aucXVAL, rng->XKEY, BSIZE);
    }
    /* 
     * <Step 3c> Xj = G(t, aucXVAL) mod q
     *     :In this code, (mod q) is only understood for DSA ops, 
     *     :not general RNG (what would q be in non-DSA uses?).
     *     :If a q is specified, use it.
     */ 

	if(!Hashing_Fips186_1_x3_3(	aucXVAL, aucX_j, BSIZE))
		return E_FIPS_RV_ERROR_SHA1_FAILURE;
	
    if (q != NULL) {
		dsa_reduce_mod_q(aucX_j, q);
    }

    /*     [FIPS 140-1] verify output does not match previous output */
    if (memcmp(aucX_j, rng->Xj, BSIZE) == 0) {
		/* failed FIPS 140-1 continuous RNG condition.  RNG now invalid. */
		rng->isValid = 0;
		return E_FIPS_RV_ERROR_RANDOMIZATION_FAILURE;
    }
    /* Xj is the output */
    memcpy(rng->Xj, aucX_j, BSIZE);
    /* 
     * <Step 3d> XKEY = (1 + XKEY + Xj) mod 2^b
     *     :always reduced mod 2^b, since storing as 160-bit value 
     */
    ADD_160BIT_PLUS_CARRY(rng->XKEY, rng->XKEY, aucX_j, 1);
	
    /* Always have a full buffer after executing alg3_1() */
    rng->avail = BSIZE;

    /* housekeeping */
    memset(aucX_j, 0, BSIZE);
    memset(aucXVAL, 0, BSIZE);
    return E_FIPS_RV_OK;
}


//Seed initialization Fips140Prg
INT32 InitSeed()
{
	INT32	nRv = E_FIPS_RV_OK;

	//Initialize the seed for random generator
	if(!bIsRandGenInitialized)
	{
		UINT32	*punTmp;
		bIsRandGenInitialized = TRUE;

		ptGlobalRng = &tGlobalRng;
		memset(ptGlobalRng, 0, sizeof(RNGContext));

		ptGlobalRng->isValid = 1;
		punTmp = (UINT32 *) ptGlobalRng->XKEY;
		
		*punTmp = GET_TIME;
		printf("\n********InitSeed time = %lu",*punTmp);
		printf("\n");
		
		
		if((*punTmp) == (UINT32)-1 )
		{
			return E_FIPS_RV_ERROR_RANDOMIZATION_FAILURE;
		}
		// Creating a first Random Number - not to be used.
		nRv = alg_fips186_1_x3_1_A(ptGlobalRng, NULL, NULL);	// RNG-A
		ptGlobalRng->seedCount += BSIZE;
		if(nRv != E_FIPS_RV_OK)
			return nRv;
		nRv = alg_fips186_1_x3_1_A(ptGlobalRng, NULL, NULL);	// RNG-A
		if(nRv != E_FIPS_RV_OK)
			return nRv;
		memcpy(ptGlobalRng->XKEY, ptGlobalRng->Xj, BSIZE);
		nRv = alg_fips186_1_x3_1  (ptGlobalRng, NULL, NULL);	// RNG-B
		if(nRv != E_FIPS_RV_OK)
			return nRv;
		ptGlobalRng->seedCount += BSIZE;
		// Initialize regular RNG seeding
		InitSeedRegRng();
	}
	return nRv;
}


//Seed initialization with same seed test used for Fips140
void InitPrgWithSameSeedValue()
{
	//Initialize the seed for random generator
	UINT32	*punTmp;

	ptGlobalRng = &tGlobalRng;
	memset(ptGlobalRng, 0, sizeof(RNGContext));

	ptGlobalRng->isValid = 1;
	punTmp = (UINT32*)ptGlobalRng->XKEY;

	*punTmp = 0x01020304;
	ptGlobalRng->seedCount += 4;
}

INT32 TestFIPS140_SHA_1_KAT()
{
	INT32	nRv = E_FIPS_RV_OK;
	UINT32	i;
	// Original Endian
//  UINT8 aucXVAL[BSIZE] =		{	0x51, 0xC8, 0x78, 0xFE, 0xD9,
//									0x16, 0x6C, 0xA9, 0xD0, 0xC1,
//									0xAE, 0x95, 0xD2, 0xBA, 0x20,
//									0xB1, 0x7C, 0x65, 0x55, 0xAA};
    UINT8 aucXVAL[BSIZE] =		{	0xFE, 0x78, 0xC8, 0x51, 0xA9, 
    								0x6C, 0x16, 0xD9, 0x95, 0xAE, 
    								0xC1, 0xD0, 0xB1, 0x20, 0xBA, 
    								0xD2, 0xAA, 0x55, 0x65, 0x7C };
    
    UINT8 aucX_j[BSIZE];
	// Original Endian
	//UINT8 aucExpectedRes[BSIZE] = {	0xbb, 0x76, 0xb5, 0xab, 0x55, 
   // 									0xd0, 0x68, 0x7d, 0x54, 0x90,
    //								 	0xf6, 0xc1, 0xca, 0x21, 0x7d,
	//								 	0x62, 0x34, 0x17, 0xcf, 0x6f};
	UINT8 aucExpectedRes[BSIZE] = {	0xab, 0xb5, 0x76, 0xbb, 0x7d, 
									0x68, 0xd0, 0x55, 0xc1, 0xf6, 
									0x90, 0x54, 0x62, 0x7d, 0x21, 
									0xca, 0x6f, 0xcf, 0x17, 0x34};    
	//SHA-1 KAT
	Hashing_Fips186_1_x3_3(	aucXVAL, aucX_j, BSIZE);
	
	//	AcPrintLog("Hashed msg = 0x%x", aucX_j[0]);
	
	if (memcmp(aucX_j, aucExpectedRes, BSIZE)!=0)
		nRv = E_FIPS_RV_ERROR_SHA1_FAILURE;
	
	//	for (i=1;i<BSIZE;i++)
	//	{
	//		AcPrintLog("%x", aucX_j[i]);
	//	}
	return nRv;
	
}
//Seed initialization with same seed test used for Fips140
INT32 TestFIPS140_RNGWithSameSeedValue()
{
	INT32	nRv = E_FIPS_RV_OK;
	BOOL	bFirstIteration = TRUE;

	//UINT8	aucResVector[BSIZE] = {	0x85, 0x5F, 0x58, 0xAA, 0x16,
	//								0x6C, 0x86, 0xB8, 0x29, 0x41,
	//								0x99, 0x73, 0xF5, 0x88, 0x25,
	//								0xD4, 0xF6, 0x7A, 0x04, 0xE9};
	// Original Endian
	//	UINT8	aucResVector[BSIZE] = {	0x5B, 0x38, 0xB8, 0x4E, 0xD9,
	//									0x16, 0x6C, 0x19, 0xD0, 0xCE,
	//									0xDE, 0x93, 0x92, 0xB9, 0xE0,
	//									0xD1, 0x5C, 0x30, 0x72, 0x06};
		UINT8	aucResVector[BSIZE] = {	0x4E, 0xB8, 0x38, 0x5B, 0x19,
										0x6C, 0x16, 0xD9, 0x93, 0xDE, 
										0xCE, 0xD0, 0xD1, 0xE0, 0xB9, 
										0x92, 0x06, 0x72, 0x30, 0x5C};

	InitPrgWithSameSeedValue();

	if(!ptGlobalRng->isValid)
		return E_FIPS_RV_ERROR_PRG_INIT_FAILURE;				

	nRv = alg_fips186_1_x3_1(ptGlobalRng, NULL, NULL);

	if(nRv != E_FIPS_RV_OK)
		return nRv;

	if(memcmp(aucResVector, ptGlobalRng->Xj, BSIZE))
		nRv = E_FIPS_RV_ERROR_SAME_SEED_DIFFERENT_RES;
		
	return nRv;
}
INT32 Fips_AES_CBC_StartUp_Test()
{
		UINT32	i;
	Uint8	pucKey[CIPHER_KEY_LENGTH]	= { 0x06, 0xa9, 0x21, 0x40, 
											0x36, 0xb8, 0xa1, 0x5b, 
											0x51, 0x2e, 0x03, 0xd5, 
											0x34, 0x12, 0x00, 0x06};
//	Uint8	pucIV[CIPHER_KEY_LENGTH]	= {0x3d, 0xaf, 0xba, 0x42, 
//											0x9d, 0x9e, 0xb4, 0x30, 
//											0xb4, 0x22, 0xda, 0x80, 
//											0x2c, 0x9f, 0xac, 0x41};
//	Uint8	pucKey[CIPHER_KEY_LENGTH]	= { 0x40, 0x21, 0xa9, 0x06, 
//											0x5b, 0xa1, 0xb8, 0x36, 
//											0xd5, 0x03, 0x2e, 0x51, 
//											0x06, 0x00, 0x12, 0x34};
//
	Uint8	pucIV[CIPHER_KEY_LENGTH]	= {0x42, 0xba, 0xaf, 0x3d, 
											0x30, 0xb4,0x9e, 0x9d,
											0x80, 0xda, 0x22, 0xb4,
											0x41, 0xac, 0x9f, 0x2c};
	Uint8	pucCT[CIPHER_KEY_LENGTH]	= {0xe3, 0x53, 0x77, 0x9c, 0x10, 0x79, 0xae, 0xb8, 0x27, 0x08, 0x94, 0x2d, 0xbe, 0x77, 0x18, 0x1a};
	Uint8	pTmpRes[CIPHER_KEY_LENGTH]	= {0};
	
	
	Uint32	punIVector[4] = {0};
	Uint32	punExpandedRoundKey[NUM_WORDS_FOR_EXPANDED_KEY];
	
	memcpy( pTmpRes, "Single block msg", CIPHER_KEY_LENGTH );
	
	punIVector[0] = GETU32(pucIV);
	punIVector[1] = GETU32(pucIV + 4);
	punIVector[2] = GETU32(pucIV + 8);
	punIVector[3] = GETU32(pucIV + 12);
	
	InitEncryptionTables();
	
	//Expand the cipherkey to roundkeys
	RijndaelKeySetupEnc(punExpandedRoundKey, pucKey);
/*	printf("\n AES IV  : ");
	 	for (i=1;i<CIPHER_KEY_LENGTH;i++) 
	 	{                                 
	 		printf("%x", pucIV[i]);     
	 	}	            
	printf("\n AES KEY  : ");
	 	for (i=1;i<CIPHER_KEY_LENGTH;i++) 
	 	{                                 
	 		printf("%x", pucKey[i]);     
	 	}	            	
*/
	EncryptCBC( pTmpRes,
		CIPHER_KEY_LENGTH,
		punIVector,
		punExpandedRoundKey);
/*	printf("\n AES RES  : ");
	 	for (i=1;i<CIPHER_KEY_LENGTH;i++) 
	 	{                                 
	 		printf("%x", pTmpRes[i]);     
	 	}	 
	printf("\n AES CT  : ");
	 	for (i=1;i<CIPHER_KEY_LENGTH;i++) 
	 	{                                 
	 		printf("%x", pucCT[i]);     
	 	}	 	
*/
	if (memcmp(pTmpRes, pucCT, CIPHER_KEY_LENGTH)!=0)
	{
		return E_FIPS_RV_ERROR_AES_FAILURE;
	}
	
	//	AcPrintLog("cipher key = 0x%x", pTmpRes[0]);
	//
	//	for (i=1;i<CIPHER_KEY_LENGTH;i++)
	//	{
	//		AcPrintLog("%x ", pTmpRes[i]);
	//	}
	
	RijndaelKeySetupDec(punExpandedRoundKey, pucKey);
	
	DecryptCBC( pTmpRes,
		CIPHER_KEY_LENGTH,
		punIVector,
		punExpandedRoundKey);
	
	if (memcmp(pTmpRes, "Single block msg", CIPHER_KEY_LENGTH)!=0)
	{
		return E_FIPS_RV_ERROR_AES_FAILURE;
	}
	//	AcPrintLog("decipher key = %s", pTmpRes);
	
	return  E_FIPS_RV_OK;
	
	/* 	for (i=1;i<CIPHER_KEY_LENGTH;i++) */
	/* 	{                                 */
	/* 		printf("%x", pTmpRes[i]);     */
	/* 	}	                              */
	
}

INT32 TestFIPS140_StartupTest()
{
	INT32	nRv = E_FIPS_RV_OK;
	// PRNG KAT startup test
	nRv = TestFIPS140_RNGWithSameSeedValue();
	if(nRv != E_FIPS_RV_OK)
		return nRv;

	// SHA1 KAT startup test
	nRv = TestFIPS140_SHA_1_KAT();
	if(nRv != E_FIPS_RV_OK)
		return nRv;

	// AES KAT startup test
	nRv = Fips_AES_CBC_StartUp_Test();
	if(nRv != E_FIPS_RV_OK)
		return nRv;

	InitSeed();

//	nRv = TestFIPS140_RNG();

	return nRv;
}

/************************* FIPS 140 *******************************************/

//Static variables
static UINT32 mt[L]; /* the array for the state vector  */
static INT32 mti=L+1; /* mti==L+1 means mt[L] is not initialized */

/* initializing the array with a NONZERO seed */
void sgenrand(UINT32 seed)
{
    /* setting initial seeds to mt[L] using         */
    /* the generator Line 25 of Table 1 in          */
    /* [KNUTH 1981, The Art of Computer Programming */
    /*    Vol. 2 (2nd Ed.), pp102]                  */
    mt[0]= seed & 0xffffffff;
    for (mti=1; mti<L; mti++)
        mt[mti] = (69069 * mt[mti-1]) & 0xffffffff;
}

static UINT32 genrand()
{
    UINT32 y;
    static UINT32 mag01[2]={0x0, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */
	
    if (mti >= L) { /* generate L words at one time */
        INT32 kk;
		
        if (mti == L+1)   /* if sgenrand() has not been called, */
        {
        	UINT32 punTmp = GET_TIME;
        	sgenrand(punTmp); /* a default initial seed is used   */
    		//UINT32 punTmp = GET_TIME;
    		//printf("\n*******genrand time = %d",punTmp);
    		//printf("\n  ");
        }
		
        for (kk=0;kk<L-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for (;kk<L-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-L)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (mt[L-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[L-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];
		
        mti = 0;
    }
	
    y = mt[mti++];
    y ^= TEMPERING_SHIFT_U(y);
    y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
    y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
    y ^= TEMPERING_SHIFT_L(y);
	
    return y; 
}

/*
 Function CreateRandom128Bits(UINT8 *pucRandKey)
 Receives : 
 pucRandKey - Pointer to an UINT8 arr[16] that would be filled with 
 random number
 */
INT32 CreateRandom128Bits(UINT8 *pucRandKey)
{
	INT32	i,nRv;

	//Initialize the seed for random generator
	nRv = InitSeed();
	if(nRv != E_FIPS_RV_OK)
		return nRv;
	// Used for FIPS140
	nRv = alg_fips186_1_x3_1(ptGlobalRng, NULL, NULL);
	
	//Create the random number while using the genrand() function
	for(i=0;i<16;i++ )
		pucRandKey[i] = ptGlobalRng->XKEY[i];

	return nRv;
}

INT32 CreateRandom128BitsOpenSSL(UINT8 *pucRandKey)
{
	return FIPS_rand_bytes(pucRandKey, 16);
}

INT32 CreateRandom112BitsOpenSSL(UINT8 *pucRandKey)
{
	return FIPS_rand_bytes(pucRandKey, 14);
}
/*
 Function CreateRandom1024Bits(UINT8 *pucRandKey)
 Receives : 
 pucRandKey - Pointer to an UINT8 arr[128] that would be filled with 
 random number
*/
INT32 CreateRandom1024Bits(BYTE *pucRandKey, UINT8 useOpenSSLFuncYesNo, char* traceBuffer)
{
	INT32 res = 0;
	INT32	i,j,nRv;
	
	if (useOpenSSLFuncYesNo)
	{
		sprintf(traceBuffer,"** CreateRandom1024Bits - NEW CODE (rand_bytes) **");
		res = FIPS_rand_bytes(pucRandKey,128);
		return !res;
	}
	else
	{
		sprintf(traceBuffer,"--- CreateRandom1024Bits - OLD CODE (InitSeed) ---");
		//Initialize the seed for random generator
		InitSeed();	

		//Create the random number while using the genrand() function
		for(j=0;j<8;j++)
		{
			// Used for FIPS140
			nRv = alg_fips186_1_x3_1(ptGlobalRng, NULL, NULL);

			for(i=0;i<16;i++ )
				pucRandKey[i] = ptGlobalRng->XKEY[i];

			pucRandKey += 16;
		}

		return nRv;
	}
}

//----------------------------------------------------------------------
//	Function name:	CreateRandomNLongs - Not for FIPS140
//
//	Description:	Creates 32 bit random number * unNumOfLongs required
//	Input:  
//					1. Pointer to an UINT8 arr that would be filled with random number
//					2. Number of longs to generate randomly 
//
//	Output:			->1
//
//	Return value:	None
//----------------------------------------------------------------------
void CreateRandomNLongs(UINT8 *pucRandKey, UINT32 unNumOfLongs)
{
	UINT32	i;
	UINT32	*punRandKey = (UINT32*)pucRandKey;

	//Initialize the seed for random generator
	InitSeed();
	
	//Create the random number while using the genrand() function
	for(i=0;i<unNumOfLongs;i++ )
		punRandKey[i] = genrand();
}


//Seed initialization Fips140Prg
void InitSeedRegRng()
{
	//Initialize the seed for random generator
	if(!bIsRandGenInitialized)
	{
		bIsRandGenInitialized = TRUE;
		
		UINT32 punTmp = GET_TIME;
		//printf("\n*******InitSeedRegRng time = %d",punTmp);
		//printf("\n");
		sgenrand(punTmp);
		//sgenrand(GET_TIME);
	}
}


