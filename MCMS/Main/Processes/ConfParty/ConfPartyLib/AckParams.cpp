//+========================================================================+
//                       AckParams.CPP                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       AckParams.CPP                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                        |
//-------------------------------------------------------------------------|
// Who  | Date March - 2008  | Description                                 |
//-------------------------------------------------------------------------|
//			
//+========================================================================+


#include "AckParams.h"
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CAckParams::CAckParams()
{
	m_pLPRParams = NULL;
	
}

/////////////////////////////////////////////////////////////////////////////
CAckParams::CAckParams(const CAckParams &other)
:CPObject(other){
	if(other.m_pLPRParams)
		m_pLPRParams = new CLPRParams(*other.m_pLPRParams);
	else
	{
		m_pLPRParams = NULL;
	}
	
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CAckParams::~CAckParams(void)
{
	POBJDELETE(m_pLPRParams);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CAckParams& CAckParams::operator= (const CAckParams& rOtherAckParams)
{
	if (&rOtherAckParams == this ) 
		return *this;
	
	if (m_pLPRParams)
		POBJDELETE(m_pLPRParams);

	if (rOtherAckParams.m_pLPRParams)
		m_pLPRParams = new CLPRParams(*rOtherAckParams.m_pLPRParams);
	else
		m_pLPRParams = NULL;
    	
	return *this;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CAckParams::Serialize(WORD format, CSegment &seg)	
{
	if (format == NATIVE)
    {
		DWORD IsLPRParams1 = FALSE; 
    	 
    	if(m_pLPRParams)
    	{
    		IsLPRParams1 = TRUE;
    		seg<< IsLPRParams1;
    		m_pLPRParams->Serialize(format, seg);
    	}
    	else 
    		seg<<  IsLPRParams1;
    		
          
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CAckParams::DeSerialize(WORD format,CSegment &seg)
{
	if (format == NATIVE)
    {   
		DWORD IsLPRParams1 = FALSE;
    	
    	seg >>  IsLPRParams1;
    	if(IsLPRParams1)
    	{
    		m_pLPRParams = new CLPRParams;  	
    		m_pLPRParams->DeSerialize(format, seg);
    	}
   	
    
    }   
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CLPRParams* CAckParams::GetLPRParams()
{
	return m_pLPRParams;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CAckParams::SetLPRParams(DWORD lossProtection,DWORD mtbf,DWORD congestionCeiling,DWORD fill,DWORD modeTimeout)
{
	if (m_pLPRParams)
   	 POBJDELETE(m_pLPRParams);
    
    m_pLPRParams = new CLPRParams(lossProtection, mtbf,congestionCeiling, fill, modeTimeout);

}














