//+========================================================================+
//                            SIPPartyInCreate.h                           |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPPartyInCreate.h                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 18/12/06   | This file contains								   |
//     |            |                                                      |
//+========================================================================+
#ifndef __SIPPARTYINCREATE__
#define __SIPPARTYINCREATE__

#include "SIPParty.h"

extern "C" void SipPartyInCreateEntryPoint(void* appParam);
extern "C" void SipPartyInWebRtcCreateEntryPoint(void* appParam);

class CSipPartyInCreate: public CSipParty
{
CLASS_TYPE_1(CSipPartyInCreate, CSipParty)
public:

	CSipPartyInCreate(ESipPartyCntlType sipCntlType);
	virtual ~CSipPartyInCreate();

	virtual void Create(CSegment& appParam);
	virtual const char* NameOf() const {return "CSipPartyInCreate";}                
	virtual void* GetMessageMap() { return (void*)m_msgEntries;}

	// state idle
	void OnLobbyRejectIdle(CSegment* pParam);
	void OnLobbyIdentIdle(CSegment* pParam);
	void OnLobbyTransferIdle(CSegment* pParam);
	eIsUseOperationPointsPreset IsUseOperationPointesPresets();
	
	//state wait for conf init call
	void OnConfAllocateResourcesIdle(CSegment * pParam);
	virtual void OnPartyBadStatusConnecting(CSegment* pParam);
	virtual void OnConfEstablishCallIdle(CSegment* pParam);

//	// state ANYCASE
	virtual void OnConfCloseCall(CSegment* pParam);
	virtual void OnPartyCallFailed(CSegment* pParam);
	virtual void OnPartyCallClosed(CSegment* pParam);

	//CDR_MCCF:
	virtual void ContinueToCloseTIPcallReq();
	virtual void forDailInSendStatisticsInfoOfThisEP();

	void OnSipDisconnectSetupIdle(CSegment* pParam);
	
	virtual void DestroyPartyTask();
	virtual void CleanUp();
	virtual void LogicalChannelDisconnect(DWORD eChannelType);
	void OnTransCheckCompleteInd(CSegment* pParam);
	void OnIceInviteReceiveMakeOfferInd(CSegment* pParam);
	virtual void OnPartyTranslatorArtsConnected();
	void ContinueEstablishCall(CSegment* pParam);
	BOOL IsRssDialinRejected();

protected:
	PDECLAR_MESSAGE_MAP; 

	virtual BOOL inline IsPartyIn() const {return TRUE;}

	EResponsibility m_eResponsibility;	
	CLobbyApi* m_pLobbyApi; 
	enSipCodes m_eLobbyRejectReason;//only if lobby rejects call in
	DWORD	   m_DialInRejectConnectionId;
	BYTE	   m_bSetRsrcParam;
};

#endif //__SIPPARTYINCREATE__

