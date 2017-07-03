//+========================================================================+
//                            H323DelPartyControl.H                        |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323DelPartyControl.H                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Sami                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 7/4/95     |                                                      |
//+========================================================================+
#ifndef _H323DelPartyControl
#define _H323DelPartyControl


#include "H323PartyControl.h"

class CH323DelPartyCntl : public CH323PartyCntl
{
CLASS_TYPE_1(CH323DelPartyCntl,CH323PartyCntl )
public: 
	// Constructors
	CH323DelPartyCntl();
	virtual ~CH323DelPartyCntl();  
	virtual const char* NameOf() const { return "CH323DelPartyCntl";}
	
	// Initializations: 
 	CH323DelPartyCntl& operator=(const CH323PartyCntl& other);
	CH323DelPartyCntl& operator=(const CH323DelPartyCntl& other);
					
  	// Operations
	virtual void*  GetMessageMap(); 
	void		   DisconnectH323(WORD mode = 0,DWORD disconnectionDelay = 0);
	void           BridgeDisconnetCompleted();                                        
	
	// action functions
	void  OnPartyDisconnectDestroyParty(CSegment* pParam);
	void  OnPartyIncreaseDisconnctingTimerDestroyParty(CSegment* pParam);
	void  OnAudBrdgDisconnect(CSegment* pParam);
	void  OnVidBrdgDisconnect(CSegment* pParam);
	virtual int  OnContentBrdgDisconnected(CSegment* pParam);
	virtual void OnXCodeBrdgDisconnected(CSegment* pParam);
	void  OnFeccBridgeDisConnect(CSegment* pParam);
	void  OnDataDisConnect(CSegment* pParam);
	void  OnChairDisConnect(CSegment* pParam);
	void  OnTimerBridgesDisconnect(CSegment* pParam);
	void  OnTimerPcmDisconnect(CSegment* pParam);
	void  OnTimerPartyDisconnectDestroyParty(CSegment* pParam);
	void  OnTimerMplDisconnectDeleteFromHw(CSegment* pParam);
  	void  OnPartyChannelDisconnectAnycase(CSegment* pParam);
  	void  OnMainPartyDoNothing(CSegment* pParam); //Multiple links for ITP in cascaded conference feature
  	void  OnPartyReceivedFaultyRsrc(CSegment* pSeg);
	void  OnAudDecoderChanged(CSegment* pParam);
    void  OnPartyMuteVideo(CSegment* pParam);
	void  OnMplAckDeleteFromHw(CSegment* pParam);
	void  OnRsrcDeallocatePartyRspDeallocate(CSegment* pParam);
	void  OnTimerRsrcAllocatorDisconnect(CSegment* pParam);
	
	void  OnPartyDelayDisconnectIdle(CSegment* pParam);
	void OnPartyPcmStateChangedIdle(CSegment* pParam);
	BOOL GetIsViolentDestroy();
	void SetIsViolentDestroy(BOOL isViolent);

	DWORD GetPartyTaskId();
	void  SetPartyTaskId(DWORD taskId);
	void  OnPartyExport(CSegment* pParam);
protected:
	// Operations	
	void  DestroyParty();
	void  DeallocatePartyResources();
	void  DeletePartyFromHW();
	void  DisconnectParty(WORD mode);
	void  PcmDisconnectionCompleted();
	BOOL  m_isViolentDestroy;
	DWORD m_partyTaskId;
	// Attributes
	PDECLAR_MESSAGE_MAP                                    
};

#endif //_H323DelPartyControl

