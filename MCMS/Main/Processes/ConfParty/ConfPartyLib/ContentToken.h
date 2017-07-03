#ifndef _CONTENTTOKEN_H
#define _CONTENTTOKEN_H

#include "Token.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyDefines.h"
#include "Segment.h"

////////////////////////////////////////////////////////////////////////////
//                        CContentToken
////////////////////////////////////////////////////////////////////////////
class CContentToken : public CRoleToken
{
CLASS_TYPE_1(CContentToken, CRoleToken)
public:
	                    CContentToken();
	virtual            ~CContentToken();
	virtual const char* NameOf() const { return "CContentToken"; }
	CContentToken&      operator=(const CContentToken& rContentToken);

	virtual void        HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);
	virtual void        InitToken(COsQueue& rcvMbx, const CTaskApi* pConfApi);
	virtual void*       GetMessageMap(void);

	// Main token interface functions
	virtual void        Acquire(CSegment* pPartyParams)  {}
	virtual void        Release(PartyRsrcID partyId)     {}
	virtual void        Release(PartyRsrcID partyId, BYTE mcuNum, BYTE terminalNum);
	virtual void        Withdraw(PartyRsrcID partyId);
	virtual void        WithdrawAck(PartyRsrcID partyId);
	virtual void        Acquire(PartyRsrcID partyId, BYTE mcuNum, BYTE terminalNum, BYTE randomNum);
	virtual void        BadAcquire(PartyRsrcID partyId, BYTE mcuNum, BYTE terminalNum);
	virtual void        BadAcquireBridgeDisconnecting(PartyRsrcID partyId, BYTE mcuNum, BYTE terminalNum);

	virtual void        DropTokenHolder(PartyRsrcID partyId, BYTE isImmediate = 0); // Use this func only when Abort Presentation
	virtual void        DropLastContentTokenRequester(); // Bridge-1127, Use it when conf termination
	virtual void        PartyDisconnects(PartyRsrcID partyId);// Use this function when any EP is disconnecting

	// Utilities
	const CTaskApp*     GetTokenHolderParty(void);
	PartyRsrcID         GetTokenHolderPartyId(void)        { return m_currentTokenHolder.GetPartyId(); }

	const CTaskApp*     GetLastTokenRequesterParty(void);
	PartyRsrcID         GetLastTokenRequesterPartyId(void) { return m_lastTokenRequester.GetPartyId(); }

	BYTE                GetTokenHolderMcuNumber(void)      { return m_currentTokenHolder.GetMcuNumber(); }
	BYTE                GetTokenHolderTermNumber(void)     { return m_currentTokenHolder.GetTerminalNumber(); }

	BYTE                IsLinkToSlave(PartyRsrcID partyId);
	BYTE                IsLinkToMaster(PartyRsrcID partyId);

	virtual void        AcquireAckFromMaster(CSegment* pPartyParams)  {}
	virtual void        RateChangeFromMaster(CSegment* pPartyParams)  {}
	virtual void        SlaveAcquire(CSegment* pPartyParams)          {}
	virtual void        WithdrawRequestFromMaster(CSegment* pParam)   {}
	void                DumpToken(const char* str);

	void                NoTokenHolderImmediate();
protected:
	//  NOTHELD state
	virtual void        OnContentBrdgAcquireTokenNOTHELD(CSegment* pParam);
	virtual void        OnContentBrdgReleaseTokenNOTHELD(CSegment* pParam);
	void                OnTimerNoRoleProviderNOTHELD(CSegment* pParam);
	void                OnTimerWithdrawReTransmitNOTHELD(CSegment* pParam);
	//  HELD state
	virtual void        OnContentBrdgAcquireTokenHELD(CSegment* pParam);
	virtual void        OnContentBrdgReleaseTokenHELD(CSegment* pParam);
	virtual void        OnContentBrdgWithdrawTokenHELD(CSegment* pParam);
	virtual void        OnContentBrdgRoleProviderInfoHELD(CSegment* pParam);
	virtual void        OnContentBrdgDropTokenHolderHELD(CSegment* pParam);
	virtual void        OnContentBrdgDropLastContentTokenRequesterHELD(CSegment* pParam);
	virtual void        OnContentBrdgPartyDisconnectHELD(CSegment* pParam);
	void                OnTimerWithdrawReTransmitHELD(CSegment* pParam);
	//  WITHDRAW state
	virtual void        OnContentBrdgAcquireTokenWithdraw(CSegment* pParam);
	virtual void        OnContentBrdgReleaseTokenWithdraw(CSegment* pParam);
	virtual void        OnContentBrdgWithdrawTokenWithdraw(CSegment* pParam);
	virtual void        OnContentBrdgWithdrawAckWithdraw(CSegment* pParam);
	virtual void        OnContentBrdgRoleProviderInfoWithdraw(CSegment* pParam);
	virtual void        OnContentBrdgDropTokenHolderWithdraw(CSegment* pParam);
	virtual void        OnContentBrdgDropLastContentTokenRequesterWithdraw(CSegment* pParam);
	virtual void        OnContentBrdgPartyDisconnectWithdraw(CSegment* pParam);
	void                OnTimerWithdrawReTransmitWithdraw(CSegment* pParam);

	//////////////////////////////////////////////////////////////////////////
	// Only three kinds of messages can be sent to Content Bridge:
	//   1. New Content Token Holder
	//   2. No Content Token Holder
	//   3. Broadcast NoRoleProvider
	void                NewTokenHolder(BYTE mcuNum, BYTE terminalNum);
	void                NoTokenHolder(void);
	void                BroadcastNoRoleProvider(void);
	//////////////////////////////////////////////////////////////////////////

	// Internal usage only
	virtual void        AcquireAck(PartyRsrcID partyId, BYTE mcuNum, BYTE terminalNum);
	virtual void        AcquireNak(PartyRsrcID partyId, BYTE mcuNum, BYTE terminalNum);
	virtual void        ReleaseAck(PartyRsrcID partyId, BYTE mcuNum, BYTE terminalNum);
	virtual void        WithdrawRequest(PartyRsrcID partyId, WORD isSendToParty = TRUE);
	virtual void        WithdrawAckHolderAction(BYTE immediate=0);
	virtual void        SendFreezeContent();

	PDECLAR_MESSAGE_MAP
};

////////////////////////////////////////////////////////////////////////////
//                        CContentCascadeSlaveToken
////////////////////////////////////////////////////////////////////////////
class CContentCascadeSlaveToken : public CContentToken
{
CLASS_TYPE_1(CContentCascadeSlaveToken, CContentToken)
public:
	                    CContentCascadeSlaveToken();
	virtual            ~CContentCascadeSlaveToken();
	virtual const char* NameOf() const { return "CContentCascadeSlaveToken"; }

	void  ResetCurrentTokenHolder();

protected:
	void                OnSlaveAcquireTokenNOTHELD(CSegment* pParam);
	void                OnSlaveAcquireTokenHELD(CSegment* pParam);
	void                OnSlaveAcquireTokenWITHDRAW(CSegment* pParam);
	virtual void        OnContentBrdgReleaseTokenNOTHELD(CSegment* pParam);
	virtual void        OnContentBrdgReleaseTokenHELD(CSegment* pParam);
	virtual void        OnContentBrdgReleaseTokenWITHDRAW(CSegment* pParam);

	void                OnStartContentDelayNOTHELD(CSegment* pPartyParams);
	void                OnStartContentDelayHELD(CSegment* pPartyParams);
	void                OnContentBrdgAcquireAckFromMasterNOTHELD(CSegment* pPartyParams);
	void                OnContentBrdgRateChangeFromMasterNOTHELD(CSegment* pPartyParams);
	void                OnContentBrdgRateChangeFromMasterHELD(CSegment* pPartyParams);

	// Internal usage only
	void                ForwardContentTokenMsgToMaster(CSegment* pPartyParams, OPCODE opcode);
	virtual void        AcquireAckFromMaster(CSegment* pPartyParams);
	virtual void        RateChangeFromMaster(CSegment* pPartyParams);
	void                ContentBrdgAcquireToken(CSegment* pPartyParams);
	void                ContentBrdgReleaseToken(CSegment* pPartyParams);
	virtual void        SlaveAcquire(CSegment* pPartyParams);
	void                SlaveAcquireToken(CSegment* pPartyParams);

	void                OnContentBrdgPartyDisconnectNOTHELD(CSegment* pParam);
	void                OnContentBrdgPartyDisconnectHELD(CSegment* pParam);
	void                OnContentBrdgPartyDisconnectWithdraw(CSegment* pParam);
	virtual void        WithdrawAckHolderAction(BYTE immediate=0);

	BYTE                RateChangeArrivedBeforeAcquireAck();
	void                SetRateChangeArrivedBeforeAcquireAck(BYTE YesNo);
	DWORD               GetNewRateArrivedBeforeAcquireAck();
	void                SetNewRateArrivedBeforeAcquireAck(DWORD rate);

	BYTE                m_rateChangeArrivedBeforeAcquireAck;
	DWORD               m_newRateArrivedBeforeAcquireAck;

	PDECLAR_MESSAGE_MAP
};

////////////////////////////////////////////////////////////////////////////
//                        CContentCascadeMasterToken
////////////////////////////////////////////////////////////////////////////
class CContentCascadeMasterToken : public CContentToken
{
CLASS_TYPE_1(CContentCascadeMasterToken, CContentToken)
public:
	                    CContentCascadeMasterToken();
	virtual            ~CContentCascadeMasterToken();
	virtual const char* NameOf() const { return "CContentCascadeMasterToken";}

protected:

	PDECLAR_MESSAGE_MAP
};

#endif //_CONTENTTOKEN_H
