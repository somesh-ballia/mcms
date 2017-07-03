#include <algorithm> // for heap functions
#include <functional>
#include <ostream>
#include "Timer.h"
#include "TimerDesc.h"
#include "SystemFunctions.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "TraceStream.h"
#include "Segment.h"


std::greater< CTimerDesc > TimerCompare;

////////////////////////////////////////////////////////////////////////////
//                        CTimer
////////////////////////////////////////////////////////////////////////////
CTimer::CTimer()
{
}

//--------------------------------------------------------------------------
CTimer::~CTimer()
{
	while (!m_TimerQueue.empty())
	{
		TimerVector::iterator itr = m_TimerQueue.begin();
		delete itr->m_pSegment;
		itr->m_pSegment = NULL;
		m_TimerQueue.erase(itr);
	}
}


//--------------------------------------------------------------------------7
void CTimer::Dump(std::ostream& ostr) const
{
	for (TimerVector::const_iterator itr = m_TimerQueue.begin(); itr != m_TimerQueue.end(); itr++)
	{
		if (itr->m_type != INFINIT)
			itr->Dump(ostr);
	}
}

//--------------------------------------------------------------------------
void CTimer::AddTimer(OPCODE type,
                      TICKS ticks,
                      const StateMachineDescriptor& stateMachineDesc,
                      CSegment* pSeg)
{
	CTimerDesc timerDesc(type, ticks, stateMachineDesc, pSeg);
	timerDesc.m_startTime   = SystemGetTickCount();
	timerDesc.m_expiredTime = timerDesc.m_startTime + timerDesc.m_time;

	BOOL foundTimer = FALSE;

	for (TimerVector::iterator itr = m_TimerQueue.begin(); itr != m_TimerQueue.end(); itr++)
	{
		if (timerDesc.m_client == itr->m_client &&
		    timerDesc.m_type == itr->m_type)
		{
			if (pSeg)
				TRACEINTO << "found the same timer, opcode: " << type << " object name: " << stateMachineDesc.m_pStateMachine->NameOf() << "\n";

			// PTRACE2INT(eLevelInfoNormal,"found the same timer, opcode: ", type);
			foundTimer = TRUE;
			*itr       = timerDesc;
			break;
		}
	}

	if (!foundTimer)
	{
		m_TimerQueue.push_back(timerDesc);
	}
}

//--------------------------------------------------------------------------
BOOL CTimer::IsValid(const StateMachineDescriptor& stateMachineDesc, OPCODE type)
{
	if (m_TimerQueue.empty())
		return FALSE;

	for (TimerVector::iterator itr = m_TimerQueue.begin(); itr != m_TimerQueue.end(); itr++)
	{
		if (stateMachineDesc == itr->m_client && type == itr->m_type)
		{
			return TRUE;
		}
	}

	return FALSE;
}

//--------------------------------------------------------------------------
void CTimer::DeleteTimers(const StateMachineDescriptor& stateMachineDesc, OPCODE type)
{
	if (m_TimerQueue.empty())
		return;

	TimerVector::iterator itr = m_TimerQueue.begin();
	while (itr != m_TimerQueue.end())
	{
		if (stateMachineDesc == itr->m_client  && (type == 0 || type == itr->m_type) )
		{
			delete itr->m_pSegment;
			itr = m_TimerQueue.erase(itr);
		}
		else
			itr++;
	}
}

//--------------------------------------------------------------------------
void CTimer::TimerExpired(CTimerDesc& timerDesc)
{
	TICKS minTime = NEVER;
	TimerVector::iterator found;
	BOOL isFound = FALSE;
	for (TimerVector::iterator itr = m_TimerQueue.begin(); itr != m_TimerQueue.end(); itr++)
	{
		if (minTime > itr->m_expiredTime || !isFound)
		{
			minTime = itr->m_expiredTime;
			found   = itr;
			isFound = TRUE;
		}
	}

	if (isFound)
	{
		timerDesc = *found;
		m_TimerQueue.erase(found);
	}
}

//--------------------------------------------------------------------------
TICKS CTimer::CalcExpiredTimeout()
{
	TICKS curAbsTime = SystemGetTickCount();
	TICKS minTime;
	BOOL  found = FALSE;
	for (TimerVector::iterator itr = m_TimerQueue.begin(); itr != m_TimerQueue.end(); itr++)
	{
		if (minTime > itr->m_expiredTime || !found)
		{
			found   = TRUE;
			minTime = itr->m_expiredTime;
		}
	}

	if (!found)
		return NEVER;
	else
	{
		if (curAbsTime < minTime)
			return minTime - curAbsTime;
		else
			return 0;
	}
}
