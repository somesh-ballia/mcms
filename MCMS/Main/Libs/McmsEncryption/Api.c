//#include "DifHel.h"
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/dh.h>
#include <openssl/bn.h>
#include <openssl/engine.h>
#include <string.h>


#ifndef DWORD
typedef         unsigned long     DWORD; 
#endif
#ifndef BYTE
typedef         unsigned char     BYTE;                                            
#endif

#define MAX_KEY_ARRAY_LENGTH	128

int	GenerateHalfKeyWithPrime(DWORD unKeyGen, BYTE *pucRandomNum,BYTE *pucHalfKey, BYTE *pucPrime);
int 	GenerateSharedSecretWithPrime(BYTE *pstHKey, int pstLen, BYTE *pstRandNum, BYTE *pucShardKey, BYTE *pucPrime);
//Global variables
//extern UINT32			s_mp_defprec;
static const unsigned char pcPrime[]  = "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE649286651ECE65381FFFFFFFFFFFFFFFF";

/*  
 Function InsertHalfKeyToArr(UINT8 *pucTrg, UINT8 *punSrc)
 copy msb's to lsb's from source to target
 Receives : 
 pucTrg -> Pointer to target unsigned char arr
 punSrc -> Pointer to source unsigned char arr

 return value:
		 0 - success
		 1 - fail
 */
/*__inline */
//void InsertHalfKeyToArr(UINT8 *pucTrg, UINT8 *punSrc)
//{
//    int i;
//    int j = 127;
//
//    for(i=0;i<MAX_KEY_ARRAY_LENGTH;i++,j--)
//        pucTrg[i] = punSrc[j];
//}

//void main()
//{	
//	/* any nonzero integer can be used as a seed */
//	//Called only once after startup
//	srand((unsigned)time( NULL ));
//	sgenrand(rand());
//
//	//Add your task functions here
//
//	//Just for Dbg - basic flow
//	{
//		UINT8 pucRand[MAX_KEY_ARRAY_LENGTH];
//		UINT8 pucHKey[MAX_KEY_ARRAY_LENGTH];
//		UINT8 pucSharedSecret[MAX_KEY_ARRAY_LENGTH];
//		UINT32	unGen = 3;
//
//
//		while(1)
//		{		
//			//Create random number
//			CreateRandom1024Bits(pucRand);
//
//			//Generate half key
//			GenerateHalfKey(unGen,pucRand,pucHKey);
//
//			//Generate shared secret
//			GenerateSharedSecret(pucHKey,pucRand,pucSharedSecret);
//		}
//	}
//}

/*
 Function CreateRandom1024Bits(UINT8 *pucRandKey)
 Receives : 
 pstHKey -> Pointer to an empty unsigned char array that would be filled with 
 unKeyGen -> unsigned int containing the key generator

 return value:
		 0 - success
		 1 - fail
 */

/*void CreateRandom1024Bits(BYTE *pucRandKey)
{
	DWORD	i;
	DWORD	*punRandKey = (DWORD*)pucRandKey;
	
	//Create the random number while using the genrand() function
	for(i=0;i<MAX_KEY_ARRAY_LENGTH >> 2;i++ )
		punRandKey[i] = genrand();
}*/

/*
 Function GenerateHalfKey(UINT32 unKeyGenType, UINT8 *pucRandomNum,UINT8 *pucHalfKey);
 The function creates a half key according to the 

 Receives :  
	 unKeyGen		-> unsigned int containing the key generator
	 pucRandomNum	-> Pointer to an unsigned char array that contains the 1024 bit random number
	 pucHalfKey		-> Pointer to an unsigned char array that would store the 1024 bit half key

 return value:
		 0 - success
		 1 - fail
*/
//int		GenerateHalfKey(DWORD unKeyGen, BYTE *pucRandomNum,BYTE *pucHalfKey)
//{
//    stKeyStruct  a, b, m;
//    INT32   res;
//    BYTE    pucGenerator[4] = {0};
//    int     rval = 0;
//    int     i;
//
//    // Set the generator array
//    for(i=0;i<4;i++)
//        pucGenerator[i] = (unKeyGen >> (24 - (i * 8))) & 0xff;
//
//    mp_init_size(&a, s_mp_defprec);
//    mp_init_size(&b, s_mp_defprec);
//    mp_init_size(&m, s_mp_defprec);
//
//    ReadRadix(&a, pucGenerator, 4);
//    ReadRadix(&b, pucRandomNum, MAX_KEY_ARRAY_LENGTH);
//    mp_read_radix(&m, pcPrime,  16);
//
//    if((res = mp_exptmod(&a, &b, &m, &a)) != MP_OKAY)
//        rval = 1;
//    else
//    {
//#ifdef __DHSIM__
//        int len;
//        char   *str;
//        len = mp_radix_size(&a, 10);
//        str = calloc(len, sizeof(char));
//        mp_toradix(&a, str, 16);
//
//
//        printf("%s\n", str);
//        free(str);
//#endif
//        InsertHalfKeyToArr(pucHalfKey,(BYTE*)(MP_DIGITS(&a)));
//    }
//
//    mp_clear(&a);
//    mp_clear(&b);
//    mp_clear(&m);
//
//    return rval;
//}
//----------------------------------------------------------------------
//	Function name:	CreateRandomNLongs
//
//	Description:	Creates 32 bit random number * unNumOfLongs required
//	Input:  
//					1. Pointer to an unsigned char arr that would be filled with random number
//					2. Number of longs to generate randomly 
//
//	Output:			->1
//
//	Return value:	None
//----------------------------------------------------------------------
/*
void CreateRandomNLongs(UINT8 *pucRandKey, UINT32 unNumOfLongs)
{
	UINT32	i;
	UINT32	*punRandKey = (UINT32*)pucRandKey;

	//Create the random number while using the genrand() function
	for(i=0;i<unNumOfLongs;i++ )
		punRandKey[i] = genrand();
}
*/

/*
 Function GenerateSharedSecret(UINT32 unKeyGenType, UINT8 *pucRandomNum,UINT8 *pucHalfKey);
 The function creates a half key according to the 

 Receives :  
		 
	pucHalfKey		-> Pointer to an unsigned char array that contains the 1024 bit half key
	pucRandomNum	-> Pointer to an unsigned char array that contains the 1024 bit random number
	pucShardKey		-> Pointer to an unsigned char array that would store the 1024 bit shared secret
	
 return value:
		 0 - success
		 1 - fail
*/
//int GenerateSharedSecret(BYTE *pstHKey, BYTE *pstRandNum, BYTE *pucShardKey)
//{
//    stKeyStruct  a, b, m;
//    DWORD   res;
//    int     rval = 0;
//
//    mp_init_size(&a, s_mp_defprec);
//    mp_init_size(&b, s_mp_defprec);
//    mp_init_size(&m, s_mp_defprec);
//
//    ReadRadix(&a, pstHKey   , MAX_KEY_ARRAY_LENGTH);
//    ReadRadix(&b, pstRandNum, MAX_KEY_ARRAY_LENGTH);
//    mp_read_radix(&m, pcPrime,  16);
//
//    if((res = mp_exptmod(&a, &b, &m, &a)) != MP_OKAY)
//    {
//        rval = 1;
//    }
//    else
//    {
//#ifdef __DHSIM__
//        int len;
//        char   *str;
//        len = mp_radix_size(&a, 10);
//        str = calloc(len, sizeof(char));
//        mp_toradix(&a, str, 16);
//
//        printf("%s\n", str);
//        free(str);
//#endif
//        InsertHalfKeyToArr(pucShardKey,(BYTE*)(MP_DIGITS(&a)));
//    }
//
//    mp_clear(&a); mp_clear(&b); mp_clear(&m);
//
//    return rval;
//}

//Function creates the same key all the time - made for Dbg
//int GenerateKeyStatic()
//{
//    stKeyStruct  a, b, m;
//    INT32  res;
//    char   *str;
//    int     len, rval = 0;
//
//    char gen[] = "2";
//    char secret[] = "7e55430f556444440e357e33e8ee6c0218d2be9ef443494a5e040c14c9eadecf2147833fc776dfab5dd6aeab1a4662c421326318ac158a75960ae819c65f33f4b7e852a6317e0a5eb88e18547bd2edd4509c7a0504fccab1b29bfb072f78a1faf3e6bf60c924b964d18187b2744c5853d3c499e863d7c7c0c3156d720ef2ee8e";
//
//    mp_init_size(&a, s_mp_defprec);
//    mp_init_size(&b, s_mp_defprec);
//    mp_init_size(&m, s_mp_defprec);
//
//    mp_read_radix(&a, gen, 16);
//    mp_read_radix(&b, secret, 16);
//    mp_read_radix(&m, pcPrime, 16);
//
//    if((res = mp_exptmod(&a, &b, &m, &a)) != MP_OKAY) {
//        rval = 1;
//    } else {
//        len = mp_radix_size(&a, 10);
//        str = calloc(len, sizeof(char));
//        //shira - YossiG - Fix klocwork NPD issue
//        if (str != NULL)
//        {
//            mp_toradix(&a, str, 16);
//#ifdef __DHSIM__
//        printf("%s\n", str);
//#endif
//            free(str);
//        }
//    }
//
//    mp_clear(&a); mp_clear(&b); mp_clear(&m);
//    return rval;
//}

/*
 Function CopyDHKeyWithZerosPadding(void * dst, const void * src, int len)
 This static function will copy the actual key to a fixed length key buffer, if the source key length is
 less than the destination key buffer length, the 0x00 will be padded at the start of the the destination 
 key buffer.

 return value:
		 1 - success
		 0 - fail
*/
static int CopyDHKeyWithZerosPadding(void * dst, const void * src, int len)
{
	memset(dst, 0, MAX_KEY_ARRAY_LENGTH);
	if (len >= 0 && len < MAX_KEY_ARRAY_LENGTH)
	{
		memcpy((dst + (MAX_KEY_ARRAY_LENGTH - len)), src, len);
	}
	else if (len == MAX_KEY_ARRAY_LENGTH)
	{
		memcpy(dst, src, len);
	}
	else
	{
		return 0;
	}

	return 1;
}

/*
 Function GenerateHalfKeyWithPrime(DWORD unKeyGen, BYTE *pucRandomNum,BYTE *pucHalfKey, BYTE *pucPrime)
 The function creates a half key according to the 

 Receives :  
	 unKeyGen		-> unsigned int containing the key generator
	 pucRandomNum	-> Pointer to an unsigned char array that contains the 1024 bit random number
	 pucHalfKey		-> Pointer to an unsigned char array that would store the 1024 bit half key
	 pucPrime		-> Pointer to an unsigned char array that would store the 1024 bit prime number

 return value:
		int - the length of generated key
		-1 : failed in key generation
*/
int	GenerateHalfKeyWithPrime(DWORD unKeyGen, BYTE *pucRandomNum,BYTE *pucHalfKey, BYTE *pucPrime)
{
	DH *dh = 0;
	int rval = 0; 
	BYTE dhkey[MAX_KEY_ARRAY_LENGTH] = {0};


	dh = DH_new();
	dh->p = BN_new();
	dh->g = BN_new();
	dh->priv_key = BN_new();

	BN_set_word(dh->g, (unsigned long)unKeyGen);
	BN_bin2bn(pucPrime, MAX_KEY_ARRAY_LENGTH, dh->p);
	BN_bin2bn(pucRandomNum, MAX_KEY_ARRAY_LENGTH, dh->priv_key);

	rval = DH_compute_key(dhkey, dh->g, dh);

	if (rval != -1)
	{		
		if (CopyDHKeyWithZerosPadding((void *)pucHalfKey, (void *)dhkey, rval) == 1)
		{
			rval = MAX_KEY_ARRAY_LENGTH;
		}
		else
		{
			rval = -1;
		}
	}

	DH_free(dh);

	return rval;
}

/*
 Function GenerateSharedSecretWithPrime(BYTE *pstHKey, int pstLen, BYTE *pstRandNum, BYTE *pucShardKey, BYTE *pucPrime)
 The function creates a half key according to the 

 Receives :  
		 
	pucHalfKey		-> Pointer to an unsigned char array that contains the 1024 bit half key
	pstLen			-> The actual length of half key
	pucRandomNum	-> Pointer to an unsigned char array that contains the 1024 bit random number
	pucShardKey		-> Pointer to an unsigned char array that would store the 1024 bit shared secret
	pucPrime		-> Pointer to an unsigned char array that would store the 1024 bit prime number
	
 return value:
		 int - the length of generated key
		 -1 : failed in key generation
*/
int GenerateSharedSecretWithPrime(BYTE *pstHKey, int pstLen, BYTE *pstRandNum, BYTE *pucShardKey, BYTE *pucPrime)
{
	DH *dh = 0;
	int rval = 0; 
	BYTE halfkey[MAX_KEY_ARRAY_LENGTH] = {0};
	BYTE dhkey[MAX_KEY_ARRAY_LENGTH] = {0};

	if (CopyDHKeyWithZerosPadding((void *)halfkey, (void *)pstHKey, pstLen) == 0)
	{
		return -1;
	}

	dh = DH_new();
	dh->p = BN_new();
	dh->g = BN_new();
	dh->priv_key = BN_new();

	BN_bin2bn(halfkey, MAX_KEY_ARRAY_LENGTH, dh->g); // use halfkey instead of pstHKey since pstHKeyLen might be less than halfkeyLen (MAX_KEY_ARRAY_LENGTH).
	BN_bin2bn(pucPrime, MAX_KEY_ARRAY_LENGTH, dh->p);
	BN_bin2bn(pstRandNum, MAX_KEY_ARRAY_LENGTH, dh->priv_key);

	rval = DH_compute_key(dhkey, dh->g, dh);

	if (rval != -1)
	{		
		if (CopyDHKeyWithZerosPadding((void *)pucShardKey, (void *)dhkey, rval) == 1)
		{
			rval = MAX_KEY_ARRAY_LENGTH;
		}
		else
		{
			rval = -1;
		}
	}
	DH_free(dh);
	return rval;
}      
// TDD Function that enters the Dh engine a secret key and check the result with an expected one
// return values: 
//                                                          0 - Success, result match
//                                                          1 - Failure, result mismatch
//                                                          2 - Failure - lack of memory resources

int GenerateKeyTdd()
{
	unsigned char    *str;
       int      i;
	DH      *dh;
       DWORD    gen;
    	int 	rval = 0;

	const unsigned char secret[] =
			"7e55430f556444440e357e33e8ee6c0218d2be9ef443494a5e040c14c9eadecf2147833fc776dfab5dd6aeab1a4662c421326318ac158a75960ae819c65f33f4b7e852a6317e0a5eb88e18547bd2edd4509c7a0504fccab1b29bfb072f78a1faf3e6bf60c924b964d18187b2744c5853d3c499e863d7c7c0c3156d720ef2ee8e";
	const unsigned char acExpectedRes[] =
			"B776DB97E63B4064181517E5EF429073D56C3CA6AA7AD784130A9DDA4FE4A774ED53F4A374F5BDCE1F8B81712559D9FE6549A385CB5D77FCA2A3CAF1254436A36F375466B89CBC1073B1E3DAC7F829B6E2341E056AA34E8051342AFB2A7E7681E9385AA5299C1437BE2AD6A2888FA9A5313AD7F3B3DEFB1C0A7B51DB761A7F1D";
	gen = 2;
	str = (unsigned char*)malloc(sizeof(unsigned char)*1024);  //1024 is the size of the Enc key 
	if (str == NULL)
	{
		return 1;
	}

	dh = DH_new();
	dh->p = BN_new();
	dh->g = BN_new();
	dh->priv_key = BN_new();

	BN_set_word(dh->g, (unsigned long)gen);
	BN_bin2bn(pcPrime, MAX_KEY_ARRAY_LENGTH, dh->p);
	BN_bin2bn(secret, MAX_KEY_ARRAY_LENGTH, dh->priv_key);

	if(DH_generate_key(dh))
	{
	    BN_bn2bin(dh->pub_key, str);

		for(i=0; i< 256; i++)
		{
			if(str[i] != acExpectedRes[i])
			{
			   rval = 1;
			   break;
			}
		}
	}
        printf("\n%s\n", str);
        printf("\n%s\n", acExpectedRes);		
		
	DH_free(dh);
	free(str);
       return rval;

}

 

 

