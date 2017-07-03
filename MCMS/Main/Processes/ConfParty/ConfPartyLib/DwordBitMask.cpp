

#include "DwordBitMask.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	The class is a simple bit-mask data structure made as a part of the bonding
//	downspeed scenario.
//	date:	12/2000
//	programer:	ron
//	
//	index order: lsb=1 to msb=32
//	bit index:		32 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9  8  7  6  5  4  3  2  1
//	mask(example):	0  0  1  0  0  1  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1  1
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int  CDwordBitMask::GetNumberOfSetBits()const
{
	int NumberOfSetBits=0;


	for(int index=0;index<BITS_IN_DWORD_MASK;index++){
		unsigned long IndexBitMask= (unsigned long)(FIRST_BIT_MASK>>index);
		unsigned long CurrentBitValue(m_BitMask);
		CurrentBitValue &= IndexBitMask;
		if(CurrentBitValue == IndexBitMask)
			NumberOfSetBits++;

	}
	return NumberOfSetBits;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDwordBitMask& CDwordBitMask::operator = (const CDwordBitMask& rOther)
{
	if ( &rOther == this )
		return *this;

	m_BitMask = rOther.m_BitMask;

	return *this;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int  CDwordBitMask::GetFirstTrueBitIndex()const
{
	int FirstTrueBitIndex = -1; //case of all bits == false.
	int BitIndex = 1;
	int TrueBitFound = 0;

	while( (BitIndex<=BITS_IN_DWORD_MASK) && (TrueBitFound==0) ){
		unsigned long CurrentBitValue=(m_BitMask & ((unsigned long)(FIRST_BIT_MASK >> (BITS_IN_DWORD_MASK-BitIndex))));
		if(CurrentBitValue == ((unsigned long)( FIRST_BIT_MASK >> (BITS_IN_DWORD_MASK-BitIndex)) )){
			TrueBitFound = 1;
			FirstTrueBitIndex = BitIndex;
		}
		BitIndex++;
	}
	return FirstTrueBitIndex;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int  CDwordBitMask::GetFirstFalseBitIndex()const
{
	int FirstFalseBitIndex = -1; //case of all bits == true.
	int BitIndex = 1;
	int FalseBitFound = 0;

	while( (BitIndex<=BITS_IN_DWORD_MASK) && (FalseBitFound==0) ){
		unsigned long CurrentBitValue=(m_BitMask & ((unsigned long)(FIRST_BIT_MASK >> (BITS_IN_DWORD_MASK-BitIndex))));
		if(CurrentBitValue != ((unsigned long)( FIRST_BIT_MASK >> (BITS_IN_DWORD_MASK-BitIndex)) )){
			FalseBitFound = 1;
			FirstFalseBitIndex = BitIndex;
		}
		BitIndex++;
	}
	return FirstFalseBitIndex;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int  CDwordBitMask::GetLastTrueBitIndex()const
{
	int LastTrueBitIndex=-1; //case of all bits == false.
	int moveIndex = 0;
	int TrueBitFound = 0;

	while( (moveIndex<BITS_IN_DWORD_MASK) && (TrueBitFound==0) ){
		unsigned long IndexBitMask= (unsigned long)(FIRST_BIT_MASK>>moveIndex);
		unsigned long CurrentBitValue(m_BitMask);
		CurrentBitValue &= IndexBitMask;
		if(CurrentBitValue == IndexBitMask){
			TrueBitFound = 1;
			LastTrueBitIndex=BITS_IN_DWORD_MASK-moveIndex;
		}
		moveIndex++;

	}
	return LastTrueBitIndex;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CDwordBitMask::Dump(COstrStream& msg)const
{

	
	
	for(int index=0;index<BITS_IN_DWORD_MASK;index++){
	unsigned long IndexBitMask= (unsigned long)(FIRST_BIT_MASK>>index);
	unsigned long CurrentBitValue(m_BitMask);
	CurrentBitValue &= IndexBitMask;
		if(CurrentBitValue == IndexBitMask){
			msg<<"1";
		}
		else
			msg<<"0";

	}
return;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CDwordBitMask::Dump()const
{

	 char s[BITS_IN_DWORD_MASK+1];

	
	for(int index=0;index<BITS_IN_DWORD_MASK;index++){
	unsigned long IndexBitMask= (unsigned long)(FIRST_BIT_MASK>>index);
	unsigned long CurrentBitValue(m_BitMask);
	CurrentBitValue &= IndexBitMask;
		if(CurrentBitValue == IndexBitMask){
			s[index] = '1';
		}
		else
			s[index] = '0';

	}
	s[BITS_IN_DWORD_MASK] = '\0';

	FPTRACE2(eLevelInfoNormal,"CDwordBitMask::Dump = ",s);	
	

return;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
