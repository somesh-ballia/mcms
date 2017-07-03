#include "DelayedH323Call.h"
#include "OpcodesMcmsCommon.h"
#include "TraceStream.h"
#include "Lobby.h"

#define STATE_WAITING 100
#define TIMER_DISPATCH_OFFERING_AGAIN 1001

PBEGIN_MESSAGE_MAP(CDelayedH323Call)
  // events from party layer
  ONEVENT(TIMER_DISPATCH_OFFERING_AGAIN ,STATE_WAITING ,CDelayedH323Call::OnTimerDispatchOffering)  
PEND_MESSAGE_MAP(CDelayedH323Call,CStateMachine)       


/////////////////////////////////////////////////////////////////////////////
CDelayedH323Call::CDelayedH323Call(CTaskApp *pOwnerTask)
        :CStateMachine(pOwnerTask)
{
    m_callIndex = 0;
    m_msg = NULL;
    m_state = STATE_WAITING;
    m_pLobby    = NULL;
}

CDelayedH323Call::~CDelayedH323Call()
{
    DeleteAllTimers();
    m_callIndex = 0;
    POBJDELETE(m_msg);
}

void CDelayedH323Call::Create (CLobby* pLobby, DWORD callIndex, CSegment * msg)
{
    m_pLobby    = pLobby;
    m_callIndex = callIndex;
    m_msg = msg;
    StartTimer (TIMER_DISPATCH_OFFERING_AGAIN, 3*SECOND);
}

void CDelayedH323Call::OnTimerDispatchOffering (CSegment * pMsg)
{
    m_pLobby->OnDelayedH323Call (this);
}

bool operator==(const CDelayedH323Call& first,const CDelayedH323Call& second)
{
    if (first.m_callIndex != 0 && second.m_callIndex !=0 && first.m_callIndex == second.m_callIndex)
        return true;
    else
        return false;
}

void  CDelayedH323Call::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode,pMsg);
}
