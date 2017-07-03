//+========================================================================+
//                            SIPPartyControlDelete.h                      |
//            Copyright 2005 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SIPPartyControlDelete.h                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#ifndef __SIPPARTYCONTORLDELETE__
#define __SIPPARTYCONTORLDELETE__


class CSipDelPartyCntl: public CSipPartyCntl
{
CLASS_TYPE_1(CSipDelPartyCntl, CSipPartyCntl)
public:
	// Constructors
	CSipDelPartyCntl();
	virtual ~CSipDelPartyCntl();
	// Initializations  
	  
	
    // Operations
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	virtual const char* NameOf() const {return "CSipDelPartyCntl";}
	void Disconnect(WORD mode, WORD cause, const char* alternativeAddrStr,DWORD disconnectionDelay);
	void OnAudioBrdgDisconnected(CSegment* pParam);
	void OnXCodeBrdgDisconnected(CSegment* pParam);
	void OnVideoBrdgDisconnected(CSegment* pParam);
	void OnFeccBridgeDisConnect(CSegment* pParam);
	int OnContentBrdgDisconnected(CSegment* pParam);
	void OnPartyDisconneted(CSegment* pParam);
	void OnMplAckDeleteFromHw(CSegment* pParam);
	void OnRsrcDeallocatePartyRspDeallocate(CSegment* pParam);

	void OnPartyPcmStateChangedIdle(CSegment* pParam);
	void OnSlaveToMasterMessage(CSegment* pParam);
	void OnSlaveToMasterAckMessage(CSegment* pParam);
	void SendDisconnectSlaveToConf(WORD tipPartyType);
	void DisconnectSlavesIfNeeded();
	BOOL DisconnectMsSlavesIfNeeded();
	void OnAllMsSlavesDeleted(CSegment* pParam);
	void OnAllMsSlavesDeletedTOUT(CSegment* pParam);

	// timers functions
	void OnTimerBridgesDisconnect(CSegment* pParam);
	void OnTimerPcmDisconnect(CSegment* pParam);
	void OnTimerPartyDisconnect(CSegment* pParam);
	void OnTimerRsrcAllocatorDisconnect(CSegment* pParam);
	void OnTimerMplDisconnectDeleteFromHw(CSegment* pParam);
	
	void OnTimerDelayDisconnectIdle(CSegment* pParam);
	void OnTimerDisconnectSlavesAck(CSegment* pParam);
	BOOL GetIsViolentDestroy();
	void SetIsViolentDestroy(BOOL isViolent);
	DWORD GetPartyTaskId();
	void  SetPartyTaskId(DWORD taskId);
	void OnMSFocusEndDisConnection(CSegment* pParam);

	void OnMSSubscriberEndDisConnection(CSegment* pParam);
	void OnPartyDisconnetedIdle(CSegment* pParam);

protected:
	PDECLAR_MESSAGE_MAP;	

	void DisconnectBridges();
	void BridgeDisconnetCompleted();                                        
	void DestroyParty();
	void DeallocatePartyResources();
	void DeletePartyFromHW();
	void DisconnectParty(WORD mode);
	void PcmDisconnectionCompleted();
	virtual void SendDisconnectMessageFromMasterToSlaves(CSegment* pParam);
	BOOL m_isViolentDestroy;
	DWORD m_partyTaskId;

	WORD m_DisConnectionModeForMasterInTip;

	BOOL m_IsPartyDisconnectEnded;

};



#endif







