/*
 * H323TransUpgradeToMixed.h
 *
 *  Created on: Feb 18, 2013
 *      Author: asilver
 */

#ifndef H323TRANSUPGRADETOMIXED_H_
#define H323TRANSUPGRADETOMIXED_H_

#include "StateMachine.h"

class CH323Party;
class CH323Cntl;

class CH323TransUpgradeToMixed : public CStateMachine
{
	CLASS_TYPE_1(CH323TransUpgradeToMixed, CStateMachine)
public:
	CH323TransUpgradeToMixed();
	CH323TransUpgradeToMixed(CTaskApp * pOwnerTask);
	virtual ~CH323TransUpgradeToMixed();
	virtual void* GetMessageMap() { return (void*)m_msgEntries;}
	virtual const char* NameOf() const {return "CH323TransUpgradeToMixed";}
	void OnPartyUpgradeToMixed(CSegment* pParam);
	void OnPartyTranslatorArtsConnected();
	void OnPartyChannelsUpdated(CSegment* pParam);
	void OnEndVideoUpgradeToMix(CSegment* pParam);

	// general transaction functions
	void InitTransaction(CH323Party* pParty, CH323Cntl* pPartyH323Cntl, CIpComMode* pPartyCurrentMode, CIpComMode* pPartyTargetMode
			, char* pPartyConfName/*, WORD voice, char* alternativeAddrStr,CIpComMode* pTargetModeMaxAllocation, BYTE bTransactionSetContentOn, BYTE isContentSuggested*/);
	void SendMessageToParty(OPCODE event, CSegment *pSeg);
	void EndTransaction(DWORD retStatus = STATUS_OK);
	void CleanTransaction();
	void SendEndTransactionToParty(DWORD retStatus);
	BOOL HandlePartyEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);
	BOOL DispatchEvent(OPCODE event,CSegment* pParam);

protected:

	// Pointers from SipParty (including Party or IpParty). Don't allocate/delete them in Transaction:
	CH323Cntl*	m_pH323Cntl;
	CIpComMode* m_pCurrentMode;
	CIpComMode* m_pTargetMode;
	char*		m_pPartyConfName;
	CPartyApi*	m_pPartyApi;
	CH323Party*	m_pParty;

	PDECLAR_MESSAGE_MAP;
};

#endif /* H323TRANSUPGRADETOMIXED_H_ */
