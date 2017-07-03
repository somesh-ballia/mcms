// DelayedH323Call.h: API of CDelayedH323Call class.
//////////////////////////////////////////////////////////////////////
//Revisions and Updates: 
//
//Date         Updated By         Description
//
//22/10/10		Lior			   
//========   ==============   =====================================================================


#ifndef _DelayedH323Call_H_
#define _DelayedH323Call_H_

#include "StateMachine.h"
#include "TaskApp.h"
class CLobby;
class CDelayedH323Call : public CStateMachine
{
    CLASS_TYPE_1(CDelayedH323Call, CStateMachine)
    CDelayedH323Call(CTaskApp *pOwnerTask);
    virtual ~CDelayedH323Call();
    virtual const char*  NameOf() const {return "CDelayedH323Call";}
    void Create (CLobby* pLobby, DWORD callIndex, CSegment * msg);
    friend bool operator==(const CDelayedH323Call& first,const CDelayedH323Call& second);
    CSegment * GetMessage() {return m_msg;}
    virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);
    PDECLAR_MESSAGE_MAP

protected:
    void OnTimerDispatchOffering (CSegment * pMsg);
    
        
    DWORD m_callIndex;
    CSegment * m_msg;
    CLobby* m_pLobby;
   
};


#endif /*_DelayedH323Call_H_*/

