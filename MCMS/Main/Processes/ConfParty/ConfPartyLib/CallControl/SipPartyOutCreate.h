//+========================================================================+
//                            SIPPartyOutCreate.h                          |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPPartyOutCreate.h                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 19/03/08   | This file contains								   |
//     |            |                                                      |
//+========================================================================+

#ifndef __SIPPARTYOUTCREATE__
#define __SIPPARTYOUTCREATE__

extern "C" void SipPartyOutCreateEntryPoint(void* appParam);

class CSipPartyOutCreate: public CSipParty
{
CLASS_TYPE_1(CSipPartyOutCreate, CSipParty)
public:
	CSipPartyOutCreate();
	virtual ~CSipPartyOutCreate();
	void  Create(CSegment& appParam);	

	virtual const char* NameOf() const {return "CSipPartyOutCreate";}                
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	virtual void   CleanUp();
//
	// idle state
	void OnConfInformResourceAllocatedIdle(CSegment * pParam);
	void OnConfEstablishCallIdle(CSegment * pParam);
	virtual void OnConfCloseCallIdle(CSegment * pParam);
	
	//disconnection functions
	virtual void OnConfCloseCall(CSegment * pParam);// maybe it should be part of the transaction class?
	void OnSipDisconnectIdle(CSegment * pParam);

	virtual void OnPartyCallFailed(CSegment * pParam);
	virtual void OnPartyBadStatusConnecting(CSegment* pParam);
	virtual void OnPartyTranslatorArtsConnected();
	void ContinueEstablishCall();
	void UpdateNetSetupToStrForSDP(CSipNetSetup* SipNetSetup);
//	void OnDnsResolutionCallIdle(CSegment * pParam);

protected:
	PDECLAR_MESSAGE_MAP; 
};


#endif /*__SIPPARTYOUTCREATE__*/

