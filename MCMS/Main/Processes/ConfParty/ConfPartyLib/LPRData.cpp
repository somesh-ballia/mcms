//+========================================================================+
//                       LPRData.CPP                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       LPRData.CPP                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                        |
//-------------------------------------------------------------------------|
// Who  | Date March - 2008  | Description                                 |
//-------------------------------------------------------------------------|
//			
//+========================================================================+


#include "LPRData.h"
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CLPRParams::CLPRParams()
{
	m_lossProtection =0;
    m_mtbf =0;
	m_congestionCeiling=0;
	m_fill=0;
	m_modeTimeout=0;
	
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CLPRParams::CLPRParams(DWORD lossProtection,DWORD mtbf,DWORD congestionCeiling,DWORD fill,DWORD modeTimeout)
{
	m_lossProtection = lossProtection;
	m_mtbf = mtbf;
	m_congestionCeiling = congestionCeiling;
	m_fill = fill;
	m_modeTimeout = modeTimeout;
	
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CLPRParams::CLPRParams(const CLPRParams& rOtherLPRParams):CPObject(rOtherLPRParams)
{
	m_lossProtection = rOtherLPRParams.m_lossProtection;
    m_mtbf = rOtherLPRParams.m_mtbf;
	m_congestionCeiling = rOtherLPRParams.m_congestionCeiling ;
	m_fill = rOtherLPRParams.m_fill;
	m_modeTimeout = rOtherLPRParams.m_modeTimeout;
	
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CLPRParams::~CLPRParams(void)
{
	
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CLPRParams& CLPRParams::operator= (const CLPRParams& rOtherLPRParams)
{
	m_lossProtection = rOtherLPRParams.m_lossProtection;
	m_mtbf = rOtherLPRParams.m_mtbf;
	m_congestionCeiling = rOtherLPRParams.m_congestionCeiling ;
	m_fill = rOtherLPRParams.m_fill;
	m_modeTimeout = rOtherLPRParams.m_modeTimeout;
    	
	return *this;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CLPRParams::Serialize(WORD format, CSegment &seg)	
{
	if (format == NATIVE)
    {  
       seg << m_lossProtection
           << m_mtbf
           << m_congestionCeiling
           << m_fill
           << m_modeTimeout;
       
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CLPRParams::DeSerialize(WORD format,CSegment &seg)
{
	if (format == NATIVE)
    { 
       seg >> m_lossProtection
           >> m_mtbf
           >> m_congestionCeiling
           >> m_fill
           >> m_modeTimeout;
    }   
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CLPRParams::SetLossProtection(DWORD lossProtection)
{
	m_lossProtection = lossProtection;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CLPRParams::SetMTBF(DWORD mtbf)
{
	m_mtbf = mtbf;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CLPRParams::SetCongestionCeiling(DWORD CongestionCeiling)
{
	m_congestionCeiling = CongestionCeiling;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CLPRParams::SetFill(DWORD Fill)
{
	m_fill = Fill;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CLPRParams::SetModeTimeout(DWORD ModeTimeout)
{
	m_modeTimeout = ModeTimeout;
}

BOOL CLPRParams::IsLprOn()
{
	return ((m_mtbf > 0) || (m_lossProtection > 0));
}















