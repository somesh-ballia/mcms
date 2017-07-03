//+========================================================================+
//                            TIMER.H                                      |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       TIMER.H                                                     |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Sami                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 4/23/95     |                                                     |
//+========================================================================+

#if !defined(_TIMER_H__)
#define _TIMER_H__

#include <vector>

#include "TaskApp.h"
#include "TimerDesc.h"

typedef std::vector<CTimerDesc> TimerVector;

#define INFINIT	0xFFFFFFFF

class CTimer : public CPObject
{
CLASS_TYPE_1(CTimer,CPObject )
public:             

	CTimer();    
	virtual ~CTimer();  
	virtual const char* NameOf() const { return "CTimer";}
	void  AddTimer(OPCODE type,
                   TICKS ticks,
                   const StateMachineDescriptor &  stateMachineDesc,
                   CSegment * pSeg = NULL);
    
	void  DeleteTimers(const StateMachineDescriptor &  stateMachineDesc,
                       OPCODE type = 0);
    
    BOOL  IsValid(const StateMachineDescriptor &   stateMachineDescriptor,
                  OPCODE type);
    void  Dump(std::ostream& ostr) const;
	// Operations
	void  CreateTimerQueue();
	void  TimerExpired(CTimerDesc&);
	TICKS CalcExpiredTimeout();
protected: 		
	
	TimerVector m_TimerQueue;
};

#endif // !defined(_TIMER_H__)
