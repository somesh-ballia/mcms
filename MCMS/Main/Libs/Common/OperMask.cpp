// OperMask.cpp: implementation of the COperMask class.
//
//////////////////////////////////////////////////////////////////////
//Revisions and Updates: 
//
//Date         Updated By         Description
//
//3/6/05		Yoella				Porting to Carmel
//========   ==============   =====================================================================

#include "OperMask.h"
#include "SharedDefines.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
COperMask::COperMask()
{
	for (int i=0;i<MAX_DWORD_MASKS;i++)
		 m_infoMask[i] = 0x00000000;
}

//----------------------------------------------------------------------------
COperMask::COperMask(const COperMask &other)
:CPObject(other)
{
	*this = other;
}

//----------------------------------------------------------------------------
COperMask& COperMask::operator = (const COperMask& other)
{
	for (int i=0;i<MAX_DWORD_MASKS;i++)
		 m_infoMask[i] = other.m_infoMask[i];

	return *this;
}
							  
//----------------------------------------------------------------------------
COperMask::~COperMask()
{
}
							

//----------------------------------------------------------------------------
void COperMask::SetAllBitsOn()
{
	DWORD initMask = 0xffffffff;

	for (int i=0;i<MAX_DWORD_MASKS;i++)
		 m_infoMask[i] = initMask;
}

//----------------------------------------------------------------------------
void COperMask::SetAllBitsOff()
{
	DWORD initMask = 0x00000000;
	
	for (int i=0;i<MAX_DWORD_MASKS;i++)
		 m_infoMask[i] = initMask;
}

	
//----------------------------------------------------------------------------
void COperMask::SetInfoBit(WORD bit, WORD onOff)
{
#ifdef __HIGHC__
        if (bit>=MAX_OPERATORS_IN_MCU){
            DBGPASSERT(1);
            return;
	}
#endif

	DWORD mask = 0x80000000;

	int i = 0, bitPos = 0; 

	int max_oper_in_mask = MAX_OPER_IN_MASK;

	if (bit < max_oper_in_mask){
		i = 0;
	    bitPos = bit;
	}
	else{
		i = bit/max_oper_in_mask;
		bitPos = bit%max_oper_in_mask;
	}

	for (int j=0; j<bitPos; j++)
		mask = mask >> 1;

	if (onOff)
		m_infoMask[i] = m_infoMask[i] | mask;
	else
		m_infoMask[i] = m_infoMask[i] & ~mask;
}

//----------------------------------------------------------------------------
WORD COperMask::GetInfoBit(WORD bit)  const
{
#ifdef __HIGHC__
        if (bit>=MAX_OPERATORS_IN_MCU){
		DBGPASSERT(1);
		return 0;
	}
#endif

	DWORD mask = 0x80000000;
	int i = 0, bitPos = 0;
	
	int max_oper_in_mask = MAX_OPER_IN_MASK;

	if (bit < max_oper_in_mask){
		i = 0;
	    bitPos = bit;
	}
	else{
		i = bit/max_oper_in_mask;
		bitPos = bit%max_oper_in_mask;
	}

	for (int j=0; j<bitPos; j++)
		 mask = mask >> 1;

	if (m_infoMask[i] & mask)
		return TRUE;
	else
		return FALSE;
}

