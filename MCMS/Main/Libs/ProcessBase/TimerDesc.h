//+========================================================================+
//                            TIMEDESC.H                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       TIMEDESC.H                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Sami                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 2/1/96     |                                                      |
//+========================================================================+

#ifndef _TIMEDESC
#define _TIMEDESC

#include "PObject.h"
#include "DataTypes.h"
#include "SystemTick.h"
#include "MessageHeader.h"

class  CSegment;

//========================================================================

class CTimerDesc : public CPObject
{
	CLASS_TYPE_1(CTimerDesc, CPObject)
public: 
	// Constructors
	CTimerDesc();
	CTimerDesc(const CTimerDesc& other);
	virtual const char* NameOf() const { return "CTimerDesc";}
    
	CTimerDesc(OPCODE type,
               TICKS ticks,
               const StateMachineDescriptor & stateMachineDescriptor,
               CSegment* seg = NULL);
    
	virtual ~CTimerDesc();  
	
	// Initializations  
	
	// Operations

	CTimerDesc& operator=(const CTimerDesc& other);
	
	friend BOOL operator==(const CTimerDesc& first,const CTimerDesc& second);    
	friend BOOL operator<(const CTimerDesc& first,const CTimerDesc& second);
	friend BOOL operator>(const CTimerDesc& first,const CTimerDesc& second);

	virtual void Dump(std::ostream&) const;

	// Attributes	
	StateMachineDescriptor   m_client;         // timer service client
	TICKS            m_time;           // time out ticks       
	TICKS            m_expiredTime;    // absolute time in ticks when expired
	TICKS            m_startTime;      // absolute time in ticks when created 
	OPCODE           m_type;           // time out type
    CSegment*        m_pSegment;       // optional segment

protected:	
	// Operations   
};

#endif /* _TIMEDESC  */
