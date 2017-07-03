// File       : SHA1.c
// Date       : April 2004
// Author     : Gil Mazor

#include <SHA1.h>

//Const variables
static const  UINT32	K0 = 0x5a827999;
static const  UINT32	K1 = 0x6ed9eba1;
static const  UINT32	K2 = 0x8f1bbcdc;
static const  UINT32	K3 = 0xca62c1d6;

static const  UINT32	punInitHValues[DIGESTED_SIZE_IN_WORDS] = {0x67452301,0xefcdab89,0x98badcfe,0x10325476,0xc3d2e1f0};

static UINT8	ipad[HASH_BLOCK_SIZE_IN_BYTES] = {0};
static UINT8	opad[HASH_BLOCK_SIZE_IN_BYTES] = {0};

//Global variables
static UINT32	punHashBuff[80]	= {0};
static UINT32	punDigestedMessage[DIGESTED_SIZE_IN_WORDS]	= {0};
static UINT8	pucRoundLabel[DIGESTED_SIZE_IN_BYTES]		= {0};
static UINT8	aucZeroPaddedInMessage[HASH_BLOCK_SIZE_IN_BYTES] = {0};

//----------------------------------------------------------------------
//	Function name:	Hashing_Fips186_1_x3_3
//
//	Description:	Hashing the message using SHA-1 according to FIPS186 appendix 3.3
//					Input:  1. Pointer to an unsigned char array of the origin message
//							2. Pointer to a 20 bytes of digested message.
//							3. Length of the input message in bytes.
//
//	Return value:	1 - Success, 0 - Otherwise.
//----------------------------------------------------------------------

BOOL Hashing_Fips186_1_x3_3(	UINT8 *pucMessage,
				UINT8 *pucDigestedMessage,
				UINT32 unLength)
{
	UINT32		i;

	//Check for message length validity
	if(!unLength || (unLength > 0xfffff))
		return FALSE;

	//Initialize the input array to zero and add relevant input
	for(i=0;i<HASH_BLOCK_SIZE_IN_BYTES;i++)
	{
		aucZeroPaddedInMessage[i] = 0x00;

		if(i<unLength)
			aucZeroPaddedInMessage[i] += pucMessage[i];
	}

	//Init H0,H1,H2,H3,H4 parameters
	for(i=0;i<DIGESTED_SIZE_IN_WORDS;i++)
		punDigestedMessage[i] = punInitHValues[i];

	//Digest messaged that do not need padding changes - in each iteration we digest 64 bytes
	DigestBlock(aucZeroPaddedInMessage, punDigestedMessage);

	//Insert the digested WORD message to BYTE message buffer
	for(i=0;i<DIGESTED_SIZE_IN_WORDS;i++)
		PUTU32(&pucDigestedMessage[i<<2], punDigestedMessage[i]);

	return TRUE;
}

//----------------------------------------------------------------------
//	Function name:	HashingMessage
//
//	Description:	Hashing the message using SHA-1
//					Input:  1. Pointer to an unsigned char array of the origin message
//							2. Pointer to a 20 bytes of digested message.
//							3. Length of the message in bytes.
//
//	Return value:	1 - Success, 0 - Otherwise.
//----------------------------------------------------------------------
BOOL HashingMessage(	UINT8 *pucMessage,
						UINT8 *pucDigestedMessage,
						UINT32 unLength)
{
	UINT32		i;
	UINT32		unNumOfPaddedBlocksToHash = 0;
	UINT32		unNumOfMessageBlocks;
	UINT8		*pucTmpMessageBuff;
	INT32		nOverSize = 0;
	//Check for message length validity
//	if(!unLength || (unLength > 0xfffff))
//	{
////		AcPrintLog("Message length is out of range, length = %d",unLength);
//
//		return FALSE;
//	}

	//Get the number of message blocks - each message block is 512 bits - 64 bytes
	unNumOfMessageBlocks = (unLength >> 6);
	if ( (unLength - 64 * unNumOfMessageBlocks) > 55) nOverSize=1;

	//Allocated memory in order to support Iterations
	if(!(pucTmpMessageBuff = (UINT8*)calloc((HASH_BLOCK_SIZE_IN_BYTES * (unNumOfMessageBlocks + 1+nOverSize)), sizeof(UINT8))))
	{
		//AcPrintLog("pucTmpMessageBuff wasn't allocated",0);
		return FALSE;
	}

	//Init tmp message
	for(i=0;i<unLength;i++)
		pucTmpMessageBuff[i] = pucMessage[i];

	//Init H0,H1,H2,H3,H4 parameters
	for(i=0;i<DIGESTED_SIZE_IN_WORDS;i++)
		punDigestedMessage[i] = punInitHValues[i];

	//Digest messaged that do not need padding changes - in each iteration we digest 64 bytes
	for(i=0; i<unNumOfMessageBlocks; i++)
		DigestBlock(&pucTmpMessageBuff[i << 6],punDigestedMessage);

	//Padding the message leftovers
	unNumOfPaddedBlocksToHash = PaddingMessage(&pucTmpMessageBuff[unNumOfMessageBlocks << 6], unLength);

	//Digest Padded messages- can be at most 2 blocks
	for(i=0;i<unNumOfPaddedBlocksToHash;i++)
		DigestBlock(&pucTmpMessageBuff[(unNumOfMessageBlocks + i) << 6], punDigestedMessage);

	//Insert the digested WORD message to BYTE message buffer
	for(i=0;i<DIGESTED_SIZE_IN_WORDS;i++)
		PUTU32(&pucDigestedMessage[i<<2], punDigestedMessage[i]);

	free(pucTmpMessageBuff);

	return TRUE;
}


//----------------------------------------------------------------------
//
//	Function name:	PaddingMessage
//
//	Description:	Padding the message to 512 bits block - Last 64 bits are used for the padding length
//					Input:  1. Pointer to a 16 unsigned long array
//
//	Return value:	Num of blocks to hush
//
//----------------------------------------------------------------------
UINT32 PaddingMessage(UINT8 *pucMessage, UINT32 unLength)
{
	UINT32		i;
	//Get the number of bytes in the last message block
	UINT32		unNumberOfBytesInLastBlock	= unLength & 0x3f;
	UINT32		unNumOfBytesCtr				= unNumberOfBytesInLastBlock;
	UINT32		unNumOfBlocksToHash			= 1;

	pucMessage[unNumOfBytesCtr]	= 0x80;
	unNumOfBytesCtr ++;

	while(unNumOfBytesCtr < HASH_BLOCK_SIZE_IN_BYTES)
	{
		pucMessage[unNumOfBytesCtr]	= 0x00;
		unNumOfBytesCtr++;
	}

	//	If the number of bytes is 55 to 64 we have to use another hash block
	//	55 place is for the 0x80 and the others for the length
	if(unNumberOfBytesInLastBlock > 55)
	{
		pucMessage	+= HASH_BLOCK_SIZE_IN_BYTES;
		unNumOfBytesCtr = 0;

		if(unNumberOfBytesInLastBlock == HASH_BLOCK_SIZE_IN_BYTES)
			unNumOfBytesCtr = 1;

		for(i=unNumOfBytesCtr;i<HASH_BLOCK_SIZE_IN_BYTES;i++)
			pucMessage[i]	= 0x00;

		unNumOfBlocksToHash = 2;
	}

	//Set message length in the last DWORD
	PUTU32(&pucMessage[HASH_BLOCK_SIZE_IN_BYTES - 4] , (unLength << 3));

	return unNumOfBlocksToHash;
}

//----------------------------------------------------------------------
//	Function name:	DigestBlock
//
//	Description:	Digesting 64 byte block message
//					Input:  1. Pointer to an unsigned char array of the origin message
//							2. Pointer to a 20 bytes of digested message.
//
//	Return value:	None
//----------------------------------------------------------------------
void DigestBlock(UINT8 *pucMessage, UINT32 *punDigestedMessage)
{
	int i;
	register UINT32	unA;
	register UINT32	unB;
	register UINT32	unC;
	register UINT32	unD;
	register UINT32	unE;

	register UINT32	unTmp;

	//Stage c of the message digest
	//Setting the registers values according to - H0 to H4
	unA = punDigestedMessage[0];
	unB = punDigestedMessage[1];
	unC = punDigestedMessage[2];
	unD = punDigestedMessage[3];
	unE = punDigestedMessage[4];

	//Init the first 16 words with the message words
	for(i=0;i<HASH_BLOCK_SIZE_IN_WORDS;i++)
		punHashBuff[i] = GETU32(&pucMessage[i<<2]);

	//Stage b of the message digest
	for(i=16;i<80;i++)
		punHashBuff[i] = SHIFT_X_CIRCULAR_LEFT((punHashBuff[i-3] ^ punHashBuff[i-8] ^ punHashBuff[i-14] ^ punHashBuff[i-16]),1);

	//Stage d of the message digest
	for(i=0;i<20;i++)
	{
		unTmp	= SHIFT_X_CIRCULAR_LEFT(unA,5) + Func1(unB,unC,unD) + unE + punHashBuff[i] + K0;
		unE		= unD;
		unD		= unC;
		unC		= SHIFT_X_CIRCULAR_LEFT(unB,30);
		unB		= unA;
		unA		= unTmp;
	}
	for(i=20;i<40;i++)
	{
		unTmp	= SHIFT_X_CIRCULAR_LEFT(unA,5) + Func2(unB,unC,unD) + unE + punHashBuff[i] + K1;
		unE		= unD;
		unD		= unC;
		unC		= SHIFT_X_CIRCULAR_LEFT(unB,30);
		unB		= unA;
		unA		= unTmp;
	}
	for(i=40;i<60;i++)
	{
		unTmp	= SHIFT_X_CIRCULAR_LEFT(unA,5) + Func3(unB,unC,unD) + unE + punHashBuff[i] + K2;
		unE		= unD;
		unD		= unC;
		unC		= SHIFT_X_CIRCULAR_LEFT(unB,30);
		unB		= unA;
		unA		= unTmp;
	}
	for(i=60;i<80;i++)
	{
		unTmp	= SHIFT_X_CIRCULAR_LEFT(unA,5) + Func4(unB,unC,unD) + unE + punHashBuff[i] + K3;
		unE		= unD;
		unD		= unC;
		unC		= SHIFT_X_CIRCULAR_LEFT(unB,30);
		unB		= unA;
		unA		= unTmp;
	}

	punDigestedMessage[0] += unA;
	punDigestedMessage[1] += unB;
	punDigestedMessage[2] += unC;
	punDigestedMessage[3] += unD;
	punDigestedMessage[4] += unE;
}


//----------------------------------------------------------------------
//	Function name:	HMAC
//
//	Description:	HMAC SHA-1
//					Input:
//							1. Pointer to unsigned char array of the HMAC Key.
//							2. Key length.
//							3. Pointer to an unsigned char array of the origin data
//							4. Origin data length.
//							5. Pointer to a 20 bytes arr of digested message.
//
//	Return value:	1 - Success, 0 - Otherwise.
//----------------------------------------------------------------------
/*BOOL HMAC(	UINT8 *pucKey,
			UINT32 unKeyLength,
			UINT8 *pucData,
			UINT32 unDataLength,
			UINT8 *pucDigested)
{
	UINT32	i;
	UINT8*	pucAppendBuff = NULL;
	UINT8*	pucTmpBuff	  = NULL;

	//Allocated memory in order to append the message buffer into the ipad
	if(!(pucAppendBuff = (UINT8*)calloc((unDataLength + 20 + 128), sizeof(UINT8))))
	{
//		AcPrintLog("pucAppendBuff wasn't allocated",0);
		return FALSE;
	}

	//Initialize the ipad and the opad arrays with 0x36 and 0x5c
	for(i=0;i<HASH_BLOCK_SIZE_IN_BYTES;i++)
	{
		ipad[i] = 0x36;
		opad[i] = 0x5c;
	}

	//Xor the key with the ipad and opad
	for(i=0;i<unKeyLength;i++)
	{
		ipad[i] ^= pucKey[i];
		opad[i] ^= pucKey[i];
	}

	//Initialize the tmpbuff with the ipad bytes
	for(i=0;i<HASH_BLOCK_SIZE_IN_BYTES;i++)
		pucAppendBuff[i] = ipad[i];

	//Jump to the end of the ipad part
	pucTmpBuff = &pucAppendBuff[64];

	//Append the message to the buffer
	for(i=0;i<unDataLength;i++)
		pucTmpBuff[i] = pucData[i];

	HashingMessage(pucAppendBuff, pucDigested, (unDataLength + 64));

	//Initialize the tmpbuff with the ipad bytes
	for(i=0;i<HASH_BLOCK_SIZE_IN_BYTES;i++)
		pucAppendBuff[i] = opad[i];

	//Jump to the end of the ipad part
	pucTmpBuff = &pucAppendBuff[64];

	//Append the digested message to the buffer
	for(i=0;i<20;i++)
		pucTmpBuff[i] = pucDigested[i];

	HashingMessage(pucAppendBuff, pucDigested, 84);

	free(pucAppendBuff);
	return TRUE;
}*/


//----------------------------------------------------------------------
//	Function name:	PRF
//
//	Description:	Pseudo Random Function
//	Input:			1. unsigned char source buffer buffer
//					2. unsigned long source length
//					3. unsigned char label buffer
//					4. unsigned long label length
//					5. unsigned char target buffer arr
//					6. unsigned long target length
//
//
//	Return value:	TRUE - success, FALSE - otherwise.
//----------------------------------------------------------------------
BOOL PRF(	UINT8	*pucSrc,
			UINT32	unSrcLength,
			UINT8	*pucLabel,
			UINT32	unLabelLength,
			UINT8	*pucTrg,
			UINT32	unTrgLength)
{
	UINT32	i,j;
	UINT32	unNumOfIterations;
	UINT32	unNumOfChunks;
	UINT32	unSrcRoundLength;
	UINT8*	pucTmpTrgBuff	  = NULL;


	//Checking input params
	if(!pucSrc || !pucTrg || !pucLabel)
	{
//		AcPrintLog3("PRF input failure, pucSrc = %d, pucTrg = %x, pucLabel = %x",
//						pucSrc, pucTrg, pucLabel);
		return FALSE;
	}

	// Source length/64 rounded up to the nearest integer
	unNumOfIterations = (unSrcLength + 63) >> 6;

	// Target length/20 rounded up to the nearest integer
	unNumOfChunks = (unTrgLength + 19) / 20;

	//Allocated memory in order to support Iterations
	if(!(pucTmpTrgBuff = (UINT8*)calloc((DIGESTED_SIZE_IN_BYTES * unNumOfChunks), sizeof(UINT8))))
	{
//		AcPrintLog("pucTmpTrgBuff wasn't allocated",0);
		return FALSE;
	}

	for(i=0;i<unNumOfIterations;i++)
	{
		//See that the max size in less than 64 bytes
		if((unSrcLength - (i << 6)) > HASH_BLOCK_SIZE_IN_BYTES)
			unSrcRoundLength = HASH_BLOCK_SIZE_IN_BYTES;
		else
			unSrcRoundLength = (unSrcLength - (i << 6));

		if(!PRF_Round(&pucSrc[i << 6], unSrcRoundLength, pucLabel, unLabelLength, pucTmpTrgBuff, unNumOfChunks))
		{
//			AcPrintLog("PRF_Round main failure",0);
			free(pucTmpTrgBuff);
			return FALSE;
		}

		if(i)
		{
			for(j=0;j<unTrgLength;j++)
				pucTrg[j] ^= pucTmpTrgBuff[j];
		}
		else
			for(j=0;j<unTrgLength;j++)
				pucTrg[j] = pucTmpTrgBuff[j];
	}

	free(pucTmpTrgBuff);
	return TRUE;
}

//----------------------------------------------------------------------
//	Function name:	PRF_Round
//
//	Description:	Pseudo Random Function
//	Input:			1. unsigned char source buffer buffer
//					2. unsigned char label buffer
//					3. unsigned char target buffer length
//					4. unsigned long target length
//
//
//	Return value:	TRUE - success, FALSE - otherwise.
//----------------------------------------------------------------------
BOOL PRF_Round(	UINT8	*pucSrc,
				UINT32	unSrcLength,
				UINT8	*pucLabel,
				UINT32	unLabelLength,
				UINT8	*pucTrg,
				UINT32	unNumOfIterations)
{
	UINT32	i,j,k=0;
	UINT32	unTmpLabelLength;
	UINT8	*pucTmpLabel;

	//Allocated memory in order to support Iterations
	if(!(pucTmpLabel = (UINT8*)calloc((DIGESTED_SIZE_IN_BYTES  + unLabelLength), sizeof(UINT8))))
	{
//		AcPrintLog("pucTmpLabel wasn't allocated",0);
		return FALSE;
	}

	//Copy the label into tmp buffer for first iteration
	for(i=0;i<unLabelLength;i++)
		pucTmpLabel[i] = pucLabel[i];

	//Init the tmp label length
	unTmpLabelLength = unLabelLength;

	// Until we reach the desired length
	for(i=0;i<unNumOfIterations;i++)
	{
		//First Iteration
		if(!HMAC(	pucSrc,
					unSrcLength,
					pucTmpLabel,
					unTmpLabelLength,
					pucRoundLabel))
		{
			free(pucTmpLabel);
			return FALSE;
		}

		for(j = 0; j < DIGESTED_SIZE_IN_BYTES; j++)
			pucTmpLabel[j] = pucRoundLabel[j];

		for(j = 0; j < unLabelLength; j++)
			pucTmpLabel[j + DIGESTED_SIZE_IN_BYTES] = pucLabel[j];

		if( !HMAC (	pucSrc,
					unSrcLength,
					pucTmpLabel,
					(unLabelLength + DIGESTED_SIZE_IN_BYTES),
					&pucTrg[k]))
		{
			free(pucTmpLabel);
			return FALSE;
		}

		k += DIGESTED_SIZE_IN_BYTES;
		unTmpLabelLength = DIGESTED_SIZE_IN_BYTES;
	}

	free(pucTmpLabel);
	return TRUE;
}

void TestPRFModule()
{
	UINT32	unResult;

	unResult = TestVectorsPRF();
}

BOOL TestVectorsPRF()
{
	UINT8	key[80];
	UINT8	label[80];

	UINT8 r1[] = {
		0x0c, 0x06, 0x04, 0x8b, 0x8f, 0xfa, 0xb2, 0x49, 0x32, 0x2d,
		0xaf, 0xe4, 0x11, 0xae, 0x2a, 0x66, 0x53, 0x29, 0x48, 0x3f, 0xef, 0x88, 0xc3,
		0x7a, 0x71, };

	UINT8 r2[] = {
		0xba, 0xc4, 0x69, 0xf1, 0xa0, 0xc5, 0x65, 0x4f, 0x84, 0x34,
		0xb6, 0xbd, 0x25, 0x4c, 0xe1, 0xfe, 0xef, 0x08, 0x00, 0x1c, 0x5e, 0x6c, 0xaf,
		0xbb, 0x4e };

	UINT8 r3[] = {
		0x13, 0x0d, 0x11, 0x51, 0x37, 0x28, 0xc1, 0xad, 0x7f, 0x3e,
		0x25, 0x6d, 0xfc, 0x08, 0xb6, 0xb9, 0x24, 0xb1, 0x4d, 0x47, 0xe4, 0x9c, 0xcd,
		0x59, 0x0e };

	UINT8 r4[] = {
		0x60, 0x1d, 0xe4, 0x98, 0x51, 0xa1, 0xdb, 0x57, 0xc5, 0x61,
		0xa9, 0x40, 0x8f, 0x51, 0x86, 0xfc, 0x17, 0xca, 0xda, 0x1a, 0xdd, 0xb8, 0xab,
		0x94, 0x4d };

	UINT8 r5[] = {
		0x9a, 0xfe, 0x6b, 0x70, 0x86, 0x2d, 0x4c, 0x30, 0xc8, 0x8d,
		0x7f, 0x69, 0x8e, 0xe1, 0xa1, 0x03, 0x51, 0x2a, 0x1e, 0x29, 0x61, 0x49, 0x2e,
		0x4b, 0x82 };

	memset(key, 0xaa, 20);
	memcpy(label, "what do ya want for nothing?", 28);
	TEST_PRF(key, 20, label, 28, r1, 1);

	memcpy(key, "Jefe", 4);
	memcpy(label, "what do ya want for nothing?", 28);
	TEST_PRF(key, 4, label, 28, r2, 2);

	memset(key, 0x0b, 20);
	memcpy(label, "Hi There", 8);
	TEST_PRF(key, 20, label, 8, r3, 3);

	memset(key, 0x0c, 20);
	memcpy(label, "Test With Truncation", 20);
	TEST_PRF(key, 20, label, 20, r4, 4);

	memset(key, 0xaa, 80);
	memcpy(label, "Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data", 73);
	TEST_PRF(key, 80, label, 73, r5, 5);

	return FALSE;
}

BOOL testPRFcase(	UINT8 *key,
					UINT32 keylen,
					UINT8 *text,
					UINT32 textlen,
					UINT8 *result)
{
	int i;
	UINT8	pucResult[DH_KEY_SIZE_IN_BYTES] = {0};

	PRF(key, keylen, text, textlen, pucResult, DH_KEY_SIZE_IN_BYTES);

	for(i=0;i<25;i++)
	{
		if(pucResult[i] != result[i])
			return FALSE;
	}

	return TRUE;
}

void TestHashingModule()
{
	UINT32	unResult;

	unResult = TestVectors();
}

BOOL TestVectors()
{
	UINT8	key[80];
	UINT8	data[80];
	UINT32	i;

	UINT8 r1[] = {
		0xb6, 0x17, 0x31, 0x86, 0x55, 0x05, 0x72, 0x64, 0xe2, 0x8b,
		0xc0, 0xb6, 0xfb, 0x37, 0x8c, 0x8e, 0xf1, 0x46, 0xbe, 0x00 };

	UINT8 r2[] = {
		0xef, 0xfc, 0xdf, 0x6a, 0xe5, 0xeb, 0x2f, 0xa2, 0xd2, 0x74,
		0x16, 0xd5, 0xf1, 0x84, 0xdf, 0x9c, 0x25, 0x9a, 0x7c, 0x79 };

	UINT8 r3[] = {
		0x12, 0x5d, 0x73, 0x42, 0xb9, 0xac, 0x11, 0xcd, 0x91, 0xa3,
		0x9a, 0xf4, 0x8a, 0xa1, 0x7b, 0x4f, 0x63, 0xf1, 0x75, 0xd3 };

	UINT8 r4[] = {
		0x4c, 0x90, 0x07, 0xf4, 0x02, 0x62, 0x50, 0xc6, 0xbc, 0x84,
		0x14, 0xf9, 0xbf, 0x50, 0xc8, 0x6c, 0x2d, 0x72, 0x35, 0xda };

	UINT8 r5[] = {
		0x4c, 0x1a, 0x03, 0x42, 0x4b, 0x55, 0xe0, 0x7f, 0xe7, 0xf2,
		0x7b, 0xe1, 0xd5, 0x8b, 0xb9, 0x32, 0x4a, 0x9a, 0x5a, 0x04 };

	UINT8 r6[] = {
		0xaa, 0x4a, 0xe5, 0xe1, 0x52, 0x72, 0xd0, 0x0e, 0x95, 0x70,
		0x56, 0x37, 0xce, 0x8a, 0x3b, 0x55, 0xed, 0x40, 0x21, 0x12 };

	UINT8 r7[] = {
		0xe8, 0xe9, 0x9d, 0x0f, 0x45, 0x23, 0x7d, 0x78, 0x6d, 0x6b,
		0xba, 0xa7, 0x96, 0x5c, 0x78, 0x08, 0xbb, 0xff, 0x1a, 0x91 };

	memset(key, 0x0b, 20);
	memcpy(data, "Hi There", 8);
	TEST(key, 20, data, 8, r1, 1);

	memcpy(key, "Jefe", 4);
	memcpy(data, "what do ya want for nothing?", 28);
	TEST(key, 4, data, 28, r2, 2);

	memset(key, 0xaa, 20);
	memset(data, 0xdd, 50);
	TEST(key, 20, data, 50, r3, 3);

	for (i = 0; i < 25; i++) {
		key[i] = (UINT8)i + 1;
	}
	memset(data, 0xcd, 50);
	TEST(key, 25, data, 50, r4, 4);

	memset(key, 0x0c, 20);
	memcpy(data, "Test With Truncation", 20);
	TEST(key, 20, data, 20, r5, 5);

	memset(key, 0xaa, 80);
	memcpy(data, "Test Using Larger Than Block-Size Key - Hash Key First", 54);
	TEST(key, 80, data, 54, r6, 6);

	memset(key, 0xaa, 80);
	memcpy(data, "Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data", 73);
	TEST(key, 80, data, 73, r7, 7);

	return FALSE;
}

BOOL testcase(	UINT8 *key,
				UINT32 keylen,
				UINT8 *text,
				UINT32 textlen,
				UINT8 *result)
{
	int i;
	UINT8	pucDigested[20] = {0};

	HMAC(key, keylen, text, textlen, pucDigested);

	for(i=0;i<20;i++)
	{
		if(pucDigested[i] != result[i])
			return FALSE;
	}

	return TRUE;
}























