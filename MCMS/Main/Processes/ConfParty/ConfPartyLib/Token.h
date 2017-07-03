#ifndef _TOKEN_H
#define _TOKEN_H

#include "TaskApi.h"
#include "ConfApi.h"
#include "PartyApi.h"
#include "TaskApp.h"

////////////////////////////////////////////////////////////////////////////
//                        CTokenUser
////////////////////////////////////////////////////////////////////////////
class CTokenUser : public CPObject
{
	CLASS_TYPE_1(CTokenUser, CPObject)

public:
	                    CTokenUser();
	                    CTokenUser(PartyRsrcID partyId, BYTE mcuNumber = 0, BYTE terminalnumber = 0, BYTE randomNumber = 0);
	virtual            ~CTokenUser();
	virtual const char* NameOf() const { return "CTokenUser"; }

	void                Set(PartyRsrcID partyId, BYTE mcuNumber = 0, BYTE terminalnumber = 0, BYTE randomNumber = 0);
	const CTaskApp*     GetParty(void);
	void                SetToNone(void);
	PartyRsrcID         GetPartyId()            { return m_partyId;         }
	BYTE                GetTerminalNumber(void) { return m_terminalNumber;  }
	BYTE                GetMcuNumber(void)      { return m_mcuNumber;       }
	BYTE                GetRandomNumber(void)   { return m_randomNumber;    }
	BYTE                operator!(void);

protected:
	PartyRsrcID         m_partyId;
	BYTE                m_mcuNumber;
	BYTE                m_terminalNumber;
	BYTE                m_randomNumber;
};

////////////////////////////////////////////////////////////////////////////
//                        CRoleToken
////////////////////////////////////////////////////////////////////////////
class CRoleToken : public CStateMachine
{
	CLASS_TYPE_1(CRoleToken,CStateMachine)

public:
	enum STATE          {NOTHELD = (IDLE + 1), HELD, WITHDRAW};
	virtual void        InitToken(COsQueue& rcvMbx, const CTaskApi* pConfApi)     = 0;
	virtual void        HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)  = 0;
	virtual void        Acquire(CSegment* pPartyParams)                           = 0;
	virtual void        Release(PartyRsrcID partyId)                              = 0;
	virtual void        Withdraw(PartyRsrcID partyId)                             = 0;
	virtual void        WithdrawAck(PartyRsrcID partyId)                          = 0;

protected:
	BYTE                m_roleLabel;
	CTokenUser          m_currentTokenHolder;
	CTokenUser          m_lastTokenRequester;
	CTaskApi*           m_pConfApi;
};

#endif //_TOKEN_H
