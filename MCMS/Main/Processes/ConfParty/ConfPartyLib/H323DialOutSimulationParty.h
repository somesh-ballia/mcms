/*$Header: /MCMS/MAIN/subsys/mcms/PRT323OU.H 18    26/03/01 20:49 Matvey $*/
//+========================================================================+
//                            PRT323OU.CPP                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323DialOutSimulationParty.h                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Uri A.                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 05/07/05     |                                                     |
//+========================================================================+

#ifndef __H323OUTPARTYSIMULATION__
#define __H323OUTPARTYSIMULATION__


//#ifndef _TASKAPP
#include "ConfApi.h"
#include "Party.h"


extern "C" void CH323DialOutSimulationPartyEntryPoint(void* appParam);

class CPrtMontrBaseParams;
class CComModeH323;
class CCapH323;

class CH323DialOutSimulationParty : public CParty
{
	CLASS_TYPE_1(CH323DialOutSimulationParty, CParty)
public:             

						// Constructors
	CH323DialOutSimulationParty();
	virtual ~CH323DialOutSimulationParty();
	virtual const char* NameOf() const { return "CH323DialOutSimulationParty";}
	virtual void  Create(CSegment& appParam);    
	void  OnConfEstablishCallIdleH323(CSegment* pParam);
	void  OnH323EndChannelConnectSetupOrConnect();
	void  OnConfChangeModeConnect(CSegment* pParam);
	void  OnConfDisconnect();
 

protected:
	void  OnH323LogicalChannelConnect(DWORD channelType);
	void  H323EndChannelConnect();
	void  H323UpdateBaudRate();
	void  OnEndH323Disconnect();
	void  H323LogicalChannelDisConnect(DWORD channelType, WORD dataType, WORD bTransmitting, BYTE roleLabel, BYTE bUpdateCommMode);
	void  InitAndSetPartyMonitor(CPrtMontrBaseParams *pPrtMonitrParams, DWORD channelType);


	CComModeH323*	m_pInitiateScm;
	CCapH323*		m_pLocalCap;
	DWORD		    m_videoRate;  // 100 bits/sec
	enIpVersion		m_ipVersion;
	
	PDECLAR_MESSAGE_MAP   
};



#endif//__H323OUTPARTYSIMULATION__













