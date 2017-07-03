//+========================================================================+
//                  CsPinger.H                                             |
//		     Copyright 2009 Polycom, Inc                                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       CsPinger.h                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Lior                                                        |
//-------------------------------------------------------------------------|
// Who  | Date  6-2009  | Description                                      |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#ifndef _CS_PINGER_H_
#define _CS_PINGER_H_

//==============================================================================================================//

#include "StateMachine.h"
#include "IpCsOpcodes.h"
#include "PingData.h"
#include "CommServiceService.h"
#include "CSMngrMplMcmsProtocolTracer.h"
#include "Segment.h"
#include "CSMngrProcess.h"

//==============================================================================================================//
// CCsPinger
//==============================================================================================================//
//internal states
const WORD   PINGING = 10;
const WORD   PINGED = 20;

//internal timer
const WORD PING_TIMER = 100;
const WORD PING_EMA_QUERY_TIMER = 200;

class CCsPinger : public CStateMachine 
{
CLASS_TYPE_1(CCsPinger,CStateMachine)

public:

// constructors
CCsPinger(CTaskApp* pOwnerTask);
    virtual ~CCsPinger();
    const char*   NameOf() const {return "CCsPinger";}
    virtual void*  GetMessageMap();
    virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

    //accesory functions
    virtual STATUS SetPing (CPingData * pNewPingData);
    virtual void SetProcess(CCSMngrProcess *pCSMngrProcess) {m_pCSMngrProcess = pCSMngrProcess;}
    
protected:
    //internal event handlers
    PDECLAR_MESSAGE_MAP
    void HandlePingIndIdle (CSegment* pParam);
    void HandlePingIndPinging (CSegment* pParam);
    void PingTimeout (CSegment* pParam);
    void PingEmaQueryTimeout(CSegment* pParam);
    
private:
    DWORD m_currentPingId;
    CCSMngrMplMcmsProtocolTracer * m_CSMngrMplMcmsProtocolTracer;
    CCommServiceService * m_CommServiceService;
    CCSMngrProcess *m_pCSMngrProcess;
};

#endif
