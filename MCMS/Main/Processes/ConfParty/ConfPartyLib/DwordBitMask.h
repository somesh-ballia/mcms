
#ifndef __DWORD_BIT_MASK_H__
#define __DWORD_BIT_MASK_H__

#include "NStream.h"


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


#define BITS_IN_DWORD_MASK 32
#define FIRST_BIT_MASK 0x80000000

class CDwordBitMask{
public:
	//constructors
	CDwordBitMask(unsigned long UserBitMask=0):m_BitMask(UserBitMask){};
	~CDwordBitMask(){};

	CDwordBitMask& operator = (const CDwordBitMask& rOther);


	//actions
	void SetBit(const int index);
	void ResetBit(const int index);
	void ResetMask(){m_BitMask=0;};

	//indications
	int  IsBitSet(const int index)const;
	int  GetNumberOfSetBits()const;
	int  GetFirstTrueBitIndex()const;
    int  GetLastTrueBitIndex()const;
	int  GetFirstFalseBitIndex()const;
	unsigned long GetDWORD()const;
	void Dump(COstrStream& msg)const;
	void Dump()const;

	//operators

protected:
	//internal utilities


private:
	//attributes
	unsigned long m_BitMask;
};





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		inline function implementation			//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
void CDwordBitMask::SetBit(const int index){
		if(index>BITS_IN_DWORD_MASK || index<0) return;
		m_BitMask |= (unsigned long)( FIRST_BIT_MASK >> (BITS_IN_DWORD_MASK-index) );
		return;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
void CDwordBitMask::ResetBit(const int index){
		if(index>BITS_IN_DWORD_MASK || index<0) return;
		m_BitMask &= (unsigned long)(~( FIRST_BIT_MASK >> (BITS_IN_DWORD_MASK-index)));
		return;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
int CDwordBitMask::IsBitSet(const int index)const{
		if(index>BITS_IN_DWORD_MASK || index<0) return -1;
		unsigned long CurrentBitValue=(m_BitMask & ((unsigned long)(FIRST_BIT_MASK >> (BITS_IN_DWORD_MASK-index))));
		return (CurrentBitValue == ((unsigned long)( FIRST_BIT_MASK >> (BITS_IN_DWORD_MASK-index)) ));
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline
unsigned long CDwordBitMask::GetDWORD()const{
	return 	m_BitMask;	
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#endif	//#ifndef __DWORD_BIT_MASK_H__
