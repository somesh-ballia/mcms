
//+========================================================================+
//                       AckParams.H                                       |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       AckParams.H                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                        |
//-------------------------------------------------------------------------|
// Who  | Date  March - 2008  | Description                                |
//-------------------------------------------------------------------------|
//			
//+========================================================================+
#ifndef ACKPARAMS_H_
#define ACKPARAMS_H_

#include "PObject.h"
#include "Segment.h"
#include "LPRData.h"

class COstrStream;
class CTaskApp;

class CAckParams : public CPObject 
{
CLASS_TYPE_1(ACKParams, CPObject)
public:
	CAckParams();
	CAckParams(const CAckParams &other);

    virtual ~CAckParams(void);
    
    virtual const char*  NameOf() const{ return "AckParams";}
	
	CAckParams&	operator= (const CAckParams& rOther);

	void Serialize(WORD format, CSegment &seg);
	void DeSerialize(WORD format, CSegment &seg);		
	
	void SetLPRParams(DWORD lossProtection,DWORD mtbf,DWORD congestionCeiling,DWORD fill,DWORD modeTimeout);	
	CLPRParams* GetLPRParams();
	BOOL IsLprOn() const { return IsValidPObjectPtr(m_pLPRParams) && m_pLPRParams->IsLprOn(); }

private:

	CLPRParams*	m_pLPRParams;
	
};

#endif /*ACKPARAMS_H_*/
