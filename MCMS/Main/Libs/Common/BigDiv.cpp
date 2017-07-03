//+========================================================================+
//                            BIGDIV.CPP                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       BIGDIV.CPP                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Ilan Yona                                                   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |09/07/02     |                                                     |
//+========================================================================+

#include "BigDiv.h"

//#ifndef _BIGDIVISION
//#include <bigdiv.h>
//#endif

typedef struct Number{
	DWORD unLeft;
	DWORD unRight;
}BigNumber;

DWORD GetMsb(DWORD x)
{
	DWORD unMsb = 0;

	while(x)
	{
		unMsb++;
		x >>= 1;
	}
	return unMsb;
}

DWORD ShiftDivisor(BigNumber A, DWORD B, BigNumber* pC)
{
	DWORD msbA,msbB,unShiftSize;

	msbA  = GetMsb(A.unLeft);
	msbB  = GetMsb(B);
	if(msbA >= msbB)//the quotient is bigger than 2^32
	{
		*pC = A;
		return 0xFFFFFFFF;
	}
	unShiftSize = msbB - msbA;
	do{
		pC->unLeft = B >> unShiftSize;
		pC->unRight = B << (32-unShiftSize);
		unShiftSize++;
	}while( (pC->unLeft > A.unLeft) || ( (pC->unLeft == A.unLeft) && (pC->unRight > A.unRight) ) );
	unShiftSize--;
	return (1 << (32-unShiftSize));
}

BigNumber SubstractBigNumbers(BigNumber A, BigNumber B)
{
	BigNumber C;

	if(A.unLeft > B.unLeft)
	{
		C.unLeft = A.unLeft - B.unLeft;
		if(A.unRight >= B.unRight)
			C.unRight = A.unRight - B.unRight;
		else{
			C.unRight = 0xFFFFFFFF -(B.unRight - A.unRight)+1;
			C.unLeft--;
		}
	}
	else{
		C.unLeft = 0;
		C.unRight = A.unRight - B.unRight;
	}
	return C;
}

DWORD DivideNumbers(BigNumber numerator, DWORD denominator)
{
	DWORD unQuotient = 0;
	BigNumber shiftedDenominator;

	while(numerator.unLeft)
	{
		unQuotient += ShiftDivisor(numerator,denominator,&shiftedDenominator);
		numerator = SubstractBigNumbers(numerator,shiftedDenominator);
	}
	unQuotient += numerator.unRight / denominator;
	return unQuotient;
}

BigNumber Multiple (DWORD A, DWORD B)
{
	BigNumber multiplication;
	DWORD unTmp;

	multiplication.unRight = (A & 0xFFFF)*(B & 0xFFFF);
	multiplication.unLeft = ((A & 0xFFFF0000)>>16)*((B & 0xFFFF0000)>>16);
	unTmp = (A & 0xFFFF)*((B & 0xFFFF0000)>>16) + (B & 0xFFFF)*((A & 0xFFFF0000)>>16);
	multiplication.unLeft += unTmp >> 16;
	multiplication.unRight += unTmp << 16;
	if(multiplication.unRight < (unTmp<<16))
		multiplication.unLeft++;

	return multiplication;
}

DWORD GetAverageRate(DWORD unCount,DWORD unTicks, DWORD multiplyValue)
{
	DWORD unRate;

	unRate = DivideNumbers( Multiple(unCount,multiplyValue), unTicks );
	return unRate;
}

#if 0
int main(void)
{

	//BigNumber a;
	DWORD b,c,rate;

	b = 0xf593;
	c = 0x4123;
	rate = GetAverageRate(b,c,800);

	return 1;
}

#endif
