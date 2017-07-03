//+========================================================================+
//                            SIPPluginPartyControlAdd.h                         |
//            Copyright 2012 Polycom China Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPPluginPartyControlAdd.h                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#ifndef __SIPPLUGINPARTYCONTORLADD__
#define __SIPPLUGINPARTYCONTORLADD__

#include "SIPPartyControlAdd.h"

class CSipPluginAddPartyCntl: public CSipAddPartyCntl
{
CLASS_TYPE_1(CSipPluginAddPartyCntl, CSipAddPartyCntl)

public:
	CSipPluginAddPartyCntl();
	virtual ~CSipPluginAddPartyCntl();
	const char*   NameOf() const {return "CSipPluginAddPartyCntl";}
	virtual void Create(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	//virtual BOOL IsPluginPartyCall() const {return TRUE;}
	virtual void HandleReAllocationForNotOfferer();
	virtual void EstablishCall(BYTE eTransportType = 0);
	virtual DWORD GetPossibleContentRate() const;	
	virtual DWORD SendCreateParty(ENetworkType networkType, BYTE bIsMrcCall) const;
	virtual DWORD GetMinContentPartyRate(DWORD currContentRate);
protected:

	PDECLAR_MESSAGE_MAP;
	virtual void AllocatePartyResources();
	//char*	m_pConfEntryPwd;
};


#endif

