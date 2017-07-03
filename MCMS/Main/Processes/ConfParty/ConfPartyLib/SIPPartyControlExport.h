//+========================================================================+
//                            SipExportPartyControl.h                       |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SipExportPartyControl.h                                      |
// SUBSYSTEM:  ConfParty                                                   |
// PROGRAMMER: GuyD                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// GuyD| 11/11/05   | Create                                               |
//+========================================================================+
#ifndef SIPEXPORTPARTYCNTL_
#define SIPEXPORTPARTYCNTL_

#include "MplMcmsStructs.h"
#include "SIPPartyControl.h"
#include "AllocateStructs.h"
#include "OpcodesMcmsCommon.h"



// Self timers for Export process
#define EXPORT_PARTY_TOUT				((WORD)300)
#define EXPORT_RSRC_TOUT				((WORD)301)
#define EXPORT_MPL_TOUT					((WORD)302)
#define END_EXPORT_RSRC_TOUT			((WORD)303)
#define EXPORT_BRIDGES_TOUT				((WORD)304)
#define EXPORT_FAILED_TOUT				((WORD)305)

class CSipPartyCntl;

class CSipExportPartyCntl : public CSipPartyCntl 
{
CLASS_TYPE_1(CSipExportPartyCntl,CSipPartyCntl )
public: 
	// Constructors
	CSipExportPartyCntl();
	 ~CSipExportPartyCntl();  
	
	// Initializations  
	CSipExportPartyCntl& operator=(const CSipExportPartyCntl& other);
	
	// Operations
	virtual const char*  NameOf() const; 
	virtual void*	 GetMessageMap(); 
	
	
	// action functions
	void  OnAudioBridgeExported(CSegment* pParam);
	void  OnVideoBridgeExported(CSegment* pParam);
	void  OnFeccBridgeDisConnect(CSegment* pParam);
	void  BridgeExportCompleted();
	void  OnPartyDisconnet(CSegment* pParam);
	void  OnAudBrdgDisconnect(CSegment* pParam);
	void  OnVideoBrdgDisconnected(CSegment* pParam);
	//void  OnXCodeBridgeDisconnect(CSegment* pParam);

	void  OnXCodeBrdgDisconnectIdle(CSegment* pParam);
	void  OnXCodeBrdgDisconnectExportBrdgs(CSegment* pParam);
	void  OnXCodeBrdgDisconnectAnyCase(CSegment* pParam);

	void  OnPartyPcmStateChangedIdle(CSegment* pParam);
	void  OnContentBridgeDisconnect(CSegment* pParam);
	void  OnContentBridgeDisconnectAnycase(CSegment* pParam);

	void  OnPartyExport(CSegment* pParam);
	void  OnTimerExport(CSegment* pParam);
	void  Transfer(COsQueue* pDestRcvMbx,void* pComConf,DWORD destConfId,DWORD destPartyId,EMoveType eCurMoveType=eMoveDefault);  
	void  BridgeDisconnetCompleted();
	
	void  OnEndResourceAllocatorStartMove(CSegment* pParam);
	void  OnMplApiMoveAck(CSegment* pParam);
	WORD  CheckIfLogicalRsrcAckAccepted(eLogicalResourceTypes lrt);
	void  MfaAcksCompleted();
	void  OnResourceAllocatorEndMove(CSegment* pParam);	

	// Timer events
	void  OnTimerRAStartMove(CSegment* pParam);
	void  OnTimerMPLExport(CSegment* pParam);
	void  OnTimerRAEndMove(CSegment* pParam);
	void  OnTimerExportBridges(CSegment* pParam);
	void  OnTimerExportFailed(CSegment* pParam);
	void  OnTimerPcmDisconnect(CSegment* pParam);
	void  OnTimerDisconnectXCodeBrdg(CSegment* pParam);
	
	WORD  GetNumOfActiveLogicalRsrc() { return m_numOfActiveLogicalRsrc;};
	
protected:

//	WORD  m_isAudExport;
//	WORD  m_isVidExport;
	WORD  m_numOfActiveLogicalRsrc;
	
	WORD  m_activeLogicalRsrc[NUM_OF_LOGICAL_RESOURCE_TYPES];
	
	// Operations	
	// Attributes
	PDECLAR_MESSAGE_MAP                                    
};


#endif /*SIPEXPORTPARTYCNTL_*/
