//+========================================================================+
//                            TIMEDESC.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       TIMEDESC.CPP                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Sami                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 2/1/96     |                                                      |
//+========================================================================+

#include <ostream>
#include <iomanip>
#include "ProcessBase.h"
#include "TimerDesc.h"
#include "Segment.h"

/////////////////////////////////////////////////////////////////////////////
CTimerDesc::CTimerDesc(OPCODE type,
                       TICKS ticks,
                       const StateMachineDescriptor & statemach,
                       CSegment * pSeg)
        :m_client(statemach),
         m_time(ticks),
         m_expiredTime(0),
         m_startTime(0),
         m_type(type)
{
	m_pSegment = pSeg;
}

/////////////////////////////////////////////////////////////////////////////
CTimerDesc::CTimerDesc()
        :m_time(0xffffffff),
         m_expiredTime(0),
         m_startTime(0),
         m_type(0),
         m_pSegment(NULL)
{

}

/////////////////////////////////////////////////////////////////////////////
CTimerDesc::CTimerDesc(const CTimerDesc& other)
	:CPObject(other)
{
	//m_pSegment = NULL;
	*this = other;
}

/////////////////////////////////////////////////////////////////////////////
CTimerDesc& CTimerDesc::operator=(const CTimerDesc& other)
{
	if (this != &other)
	{
		m_type         = other.m_type;
		m_client       = other.m_client;
		m_expiredTime  = other.m_expiredTime;
		m_startTime    = other.m_startTime;
		m_time         = other.m_time;
		//if (CPObject::IsValidPObjectPtr(m_pSegment))

////	{
//			POBJDELETE(m_pSegment);
//		if (CPObject::IsValidPobjectPtr(other.m_pSegment)
//			*m_pSegment = *other.m_pSegment;
//		else
//
//	}
//	else
//	{
		//	POBJDELETE(m_pSegment);
			m_pSegment = other.m_pSegment;

		}

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CTimerDesc::~CTimerDesc()     // destructor
{
}


/////////////////////////////////////////////////////////////////////////////
BOOL operator==(const CTimerDesc& first,const CTimerDesc& second)
{
	BOOL rval = 0;

	if ( first.m_client == second.m_client &&
		first.m_type    == second.m_type  )
	{
		rval = 1;
	}

	if( rval &&  first.m_expiredTime!= 0 )
	{
		if( first.m_expiredTime != second.m_expiredTime )
			rval = 0;
	}
	return rval;
}

/////////////////////////////////////////////////////////////////////////////
BOOL operator<(const CTimerDesc& first,const CTimerDesc& second)
{
	BOOL rval = FALSE;

	if ( first.m_expiredTime < second.m_expiredTime )
		rval = TRUE;
	else
	{
		if ( first.m_expiredTime == second.m_expiredTime )
		{
			if ( first.m_client < second.m_client )
				rval = TRUE;
			else  {
				if ( first.m_client == second.m_client )
				{
					if ( first.m_type < second.m_type )
						rval = TRUE;
				}
			}
		}
	}
	return rval;
}

/////////////////////////////////////////////////////////////////////////////
BOOL operator>(const CTimerDesc& first,const CTimerDesc& second)
{
	return !(first < second);
}

/////////////////////////////////////////////////////////////////////////////
void CTimerDesc::Dump(std::ostream& stream) const
{
	stream
 		<< std::setw(40)  << m_client.m_pStateMachine->GetOpcodeAsString(m_type)
		//<< (std::dec) << std::setw(12) << m_type
		<< (std::dec) << std::setw(12) << m_time
		<< (std::dec) << std::setw(12) << m_expiredTime
		<< (std::dec) << std::setw(12) << m_startTime << std::endl;
}


