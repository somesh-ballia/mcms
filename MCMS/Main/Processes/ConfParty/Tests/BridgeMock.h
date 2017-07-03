//+========================================================================+
//                      BridgeMock.h                                       |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       BridgeMock.h	                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2005  |                                                      |
//+========================================================================+


#ifndef _BRIDGEMOCK_H__
#define _BRIDGEMOCK_H__


#include "Bridge.h"


class CBridgeMock : public CBridge
{
public:
	virtual ~CBridgeMock () {}
	CBridgeMock () : CBridge() {VALIDATEMESSAGEMAP;}
	CBridgeMock (const CBridgeMock& rBridgePartyCntl): CBridge(rBridgePartyCntl) {}
	CBridgeMock&	operator= (const CBridgeMock& rOther) { (CBridge&)(*this) = (CBridge&)rOther; return *this; }

	CBridgePartyList* GetPartyList() const {return m_pPartyList;}
	CConfApi* GetConfApi() const {return m_pConfApi;}
	CConf* GetConf() const {return m_pConf;}
	char* GetConfName() const {return (char*)m_pConfName;}

	void ChangeState (WORD wNewState) {m_state = wNewState;}

	void	ConnectParty (CBridgePartyCntl*  pPartyCntl) {CBridge::ConnectParty(pPartyCntl);}
	void	DisconnectParty (PartyRsrcID partyId) {CBridge::DisconnectParty(partyId);}

	void	OnEndPartyConnect (CSegment* pParam) {CBridge::EndPartyConnect(pParam, 0);}
	void	OnEndPartyDisConnect (CSegment* pParam) {CBridge::EndPartyDisConnect(pParam, 0);}

	virtual EBridgeImplementationTypes	GetBridgeImplementationType () { return CBridge::GetBridgeImplementationType(); }

protected:
	PDECLAR_MESSAGE_MAP
};


#endif /* _BRIDGEMOCK_H__ */
