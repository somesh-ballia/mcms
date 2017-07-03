//+========================================================================+
//                            SIPPartyControlChangeMode.h                      |
//            Copyright 2005 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SIPPartyControlChangeMode.CPP                                     |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#ifndef __SIPPARTYPLUGINCONTROLCHANGEMODE__
#define __SIPPARTYPLUGINCONTROLCHANGEMODE__

#include "SIPPartyControlChangeMode.h"

class CSipPluginChangeModePartyCntl: public CSipChangeModePartyCntl
{
CLASS_TYPE_1(CSipPluginChangeModePartyCntl, CSipChangeModePartyCntl)
public:
	// Constructors
	CSipPluginChangeModePartyCntl();
	virtual ~CSipPluginChangeModePartyCntl();
	virtual const char*  NameOf() const;
	//virtual BOOL IsPluginPartyCall() const {return TRUE;}
	// Initializations  
	
    // Operations
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	virtual int  OnContentBrdgConnectedAnycase(CSegment* pParam);
	virtual DWORD GetPossibleContentRate() const;
	virtual BYTE ChangeVideoBrdgRsrcIfNeeded();
	virtual void SetPartyStateUpdateDbAndCdrAfterEndConnected(BYTE reason = NO);
	virtual BYTE IsChangeContentNeeded(BYTE bSetChangeModeStateIfNeeded);
	virtual void ChangeScm(CIpComMode* pScm);
	virtual void ChangeModeIdle(CSegment* pParam);
	virtual DWORD GetMinContentPartyRate(DWORD currContentRate);
protected:
	virtual void OnPartyAuthCompleate(CSegment* pParam);
	void  OnPartyAuthTOUT(CSegment* pParam);
	PDECLAR_MESSAGE_MAP;	
};

#endif


