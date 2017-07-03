//+========================================================================+
//                       LPRData.H                                       |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       LPRData.H                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                        |
//-------------------------------------------------------------------------|
// Who  | Date  March - 2008  | Description                                |
//-------------------------------------------------------------------------|
//			
//+========================================================================+
#ifndef LPRDATA_H_
#define LPRDATA_H_

#include "PObject.h"
#include "Segment.h"

class COstrStream;
class CTaskApp;

class CLPRParams : public CPObject 
{
CLASS_TYPE_1(CLPRParams, CPObject)
public:
	CLPRParams();
	CLPRParams(const CLPRParams& rOtherLPRParams);
	CLPRParams(DWORD lossProtection,DWORD mtbf,DWORD congestionCeiling,DWORD fill,DWORD modeTimeout);
    virtual ~CLPRParams(void);
    
    virtual const char*  NameOf() const{ return "CLPRParams";}
	
	CLPRParams&	operator= (const CLPRParams& rOther);

	void Serialize(WORD format, CSegment &seg);
	void DeSerialize(WORD format, CSegment &seg);		
	
	DWORD GetLossProtection()const {return m_lossProtection;};
	DWORD GetMTBF()const {return m_mtbf;};
	DWORD GetCongestionCeiling()const {return m_congestionCeiling;};
	DWORD GetFill()const {return m_fill;};
	DWORD GetModeTimeout()const {return m_modeTimeout;};
	
	void SetLossProtection(DWORD LossProtection);
	void SetMTBF(DWORD MTBF);
	void SetCongestionCeiling(DWORD CongestionCeiling);
	void SetFill(DWORD Fill);
	void SetModeTimeout(DWORD ModeTimeout);
	BOOL IsLprOn();
private:

	DWORD	m_lossProtection;
	DWORD	m_mtbf;
	DWORD	m_congestionCeiling;
	DWORD	m_fill;
	DWORD	m_modeTimeout;

};


#endif /*LPRDATA_H_*/
