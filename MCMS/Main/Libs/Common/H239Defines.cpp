//+========================================================================+
//                            H239Defines.cpp                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H239Defines.cpp                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 12/26/2007     |                                                     |
//+========================================================================+

#include "H239Defines.h"



/////////////////////////////////////////////////////////////////////////////
// Function IntToMbeBytes receives integer and pointer to initiated Array of Bytes
// Fills the array with translation of integer to MBE byte and returns length of array
// (number of bytes filled) as described in ITU Standard H.239 (Annex A)
// Note -We assume that the Array is of length 6. (translation from max negative int will need 6 bytes)
////////////////////////////////////////////////////////////////////////////
int IntToMbeBytes(BYTE* byteArray,int val)
{
   int lenOfByteArray = 0, absoluteVal = 0;
   BYTE tempCalcByte = 0;

   if(val>=0)	//Non-Negative Integers
   {
	   BYTE firstTime =1;
	   while (val || firstTime)
	   {
			if(val<=127)					//last Byte
			{
				byteArray[lenOfByteArray] = (BYTE)val;
				val = 0;
			}
			else							//not last Byte
			{
				byteArray[lenOfByteArray] = (0x80); // 2 high order bits equal to binary'10'
				tempCalcByte = (BYTE)val;
				tempCalcByte &= (0x3F);			// least significant 6 bits of int in lease significant 6 bits of MBE BYTE
				byteArray[lenOfByteArray] |= tempCalcByte;
				val >>= 6;
			}
			lenOfByteArray++;				// increment place in ByteArray
			firstTime = 0;
	   }
   }
   else			// Negative Integers
   {
		BYTE firstTime =1;
		absoluteVal = val*(-1);					//absolute value of negative int
		while(absoluteVal)
		{
			if(absoluteVal<=127 && !firstTime)	// last Byte (can not be first byte)
			{
				byteArray[lenOfByteArray] = (BYTE)absoluteVal;
				absoluteVal = 0;
			}
			else							//not last Byte
			{
				byteArray[lenOfByteArray] = (0xC0);	// 3 high order bits equal to binary'110'
				tempCalcByte = (BYTE)absoluteVal;
				tempCalcByte &= (0x1F);					// least significant 5 bits of int in lease significant 5 bits of MBE BYTE
				byteArray[lenOfByteArray] |= tempCalcByte;
				absoluteVal >>= 5;
			}	
			lenOfByteArray++;			// increment place in ByteArray
			firstTime = 0;
		}
   }
   return lenOfByteArray;
}
/////////////////////////////////////////////////////////////////////////////
// Function IntToMbeBytes receives integer and Segment
// Fills the Segment with translation of integer to MBE byte 
// as described in ITU Standard H.239 (Annex A)
////////////////////////////////////////////////////////////////////////////
void IntToMbeBytes(CSegment *seg,int value)
{
	int lengthOfArray = 0;
	BYTE* byteArray = new BYTE[6];
	lengthOfArray = IntToMbeBytes(byteArray,value);
	for(int i=0; i<lengthOfArray; i++)
		*seg << (BYTE)byteArray[i];
	PDELETEA(byteArray);
}

/////////////////////////////////////////////////////////////////////////////
// Function MbeBytesToInt receives pointer to Array of Bytes
// Returns int -translation of of array, MBE bytes to integer
// as described in ITU Standard H.239 (Annex A)
////////////////////////////////////////////////////////////////////////////
int MbeBytesToInt(BYTE* pByteArray)
{
	int val = 0, tempCalcInt =0;

	BYTE placeInByteArray = 0, placeInInt =0, isLastByte = 0, tempCalcByte = 0;

	tempCalcByte = pByteArray[0];
	tempCalcByte &= (0xE0);
	if(tempCalcByte== (0xC0))	// Negative Integers
	{
		while(1)
		{
			isLastByte = !(pByteArray[placeInByteArray] & (0x80));	//final MBE byte will have the high order bit set to 0
			if(isLastByte)
			{	
				tempCalcInt = pByteArray[placeInByteArray];
				tempCalcInt<<=placeInInt;
				val |= tempCalcInt;
				return ((-1)*val);
			}
			else
			{
				tempCalcInt = (pByteArray[placeInByteArray] & (0x1F));// remove 2 high order bits 
				tempCalcInt<<=placeInInt;
				val |= tempCalcInt;
				placeInInt+=5;
			}
			placeInByteArray++;
		}
	}
	else					//Non-Negative Integers
	{
		while(1)
		{
			isLastByte = !(pByteArray[placeInByteArray] & (0x80));	//final MBE byte will have the high order bit set to 0
			if(isLastByte)
			{
				tempCalcInt = pByteArray[placeInByteArray];
				tempCalcInt<<=placeInInt;
				val |= tempCalcInt;
				return (val);
			}
			else
			{				
				tempCalcInt = (pByteArray[placeInByteArray] & (0x3F));// remove 2 high order bits 
				tempCalcInt<<=placeInInt;
				val |= tempCalcInt;
				placeInInt+=6;
			}
			placeInByteArray++;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// Function MbeBytesToInt receives Segment
// Returns int -translation of of array, MBE bytes to integer
// as described in ITU Standard H.239 (Annex A)
////////////////////////////////////////////////////////////////////////////
int MbeBytesToInt(CSegment* pParam)
{
	int val = 0, tempCalcInt =0;

	BYTE placeInInt =0, isLastByte = 0, tempCalcByteForNegative = 0, tempCalcByte = 0;
	*pParam >> tempCalcByte;
	tempCalcByteForNegative =tempCalcByte & (0xE0);
	if(tempCalcByteForNegative== (0xC0))	// Negative Integers
	{
		tempCalcByte = tempCalcByteForNegative;
		while(1)
		{
			isLastByte = !(tempCalcByte & (0x80));			//final MBE byte will have the high order bit set to 0
			if(isLastByte)
			{	
				tempCalcInt = tempCalcByte;
				tempCalcInt<<=placeInInt;
				val |= tempCalcInt;
				return ((-1)*val);
			}
			else
			{
				tempCalcInt = (tempCalcByte & (0x1F));// remove 2 high order bits 
				tempCalcInt<<=placeInInt;
				val |= tempCalcInt;
				placeInInt+=5;
				if (pParam->EndOfSegment())
					return ((-1)*val);
			}
			*pParam >> tempCalcByte;
		}
	}
	else					//Non-Negative Integers
	{
		while(1)
		{
			isLastByte = !(tempCalcByte & (0x80));		//final MBE byte will have the high order bit set to 0
			if(isLastByte)
			{
				tempCalcInt = tempCalcByte;
				tempCalcInt<<=placeInInt;
				val |= tempCalcInt;
				return (val);
			}
			else
			{				
				tempCalcInt = (tempCalcByte & (0x3F));// remove 2 high order bits 
				tempCalcInt<<=placeInInt;
				val |= tempCalcInt;
				placeInInt+=6;
				if (pParam->EndOfSegment())
					return val;
			}
			*pParam >> tempCalcByte;

		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Function SerializeGenericParameter receives PID VALUE and seg
// fills seg with GenericParameter formatted,
// as described in ITU Standard H.239 (Annex A)
////////////////////////////////////////////////////////////////////////////
void SerializeGenericParameter(BYTE PID, int value, CSegment *seg)
{
	int lengthOfArray = 0;
	
	if(1<=PID && PID<=39)		// PID/VALUE
	{
		*seg << (BYTE)PID;
		IntToMbeBytes(seg, value);
	}
	else if(40<=PID && PID<=79) // X/VALUE
	{
		IntToMbeBytes(seg,value);
	}
	else if(80<=PID && PID<=127) // PID/X
	{
		*seg << (BYTE)PID;
	}
	else
	{
		FPTRACE(eLevelError,"ILLEGAL PID received. ");
		if(PID)
			FPASSERT(PID);
		else
			FPASSERT(101);
	}
}


