//+========================================================================+
//                    BridgePartyMediaParams.CPP                           |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       BridgePartyMediaParams.CPP                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  June-2005  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+


#include "BridgePartyMediaParams.h"

// ------------------------------------------------------------
CBridgePartyMediaParams::CBridgePartyMediaParams () :
	m_eTelePresenceMode(eTelePresencePartyNone),
	m_telepresenseEPInfo(new CTelepresenseEPInfo)

{
	// PRESERVE:BEGIN
	// Insert your preservable code here...
	
	// PRESERVE:END
}

// ------------------------------------------------------------
CBridgePartyMediaParams::CBridgePartyMediaParams (const CBridgePartyMediaParams& rOtherBridgePartyMediaParams)
        :CPObject(rOtherBridgePartyMediaParams)
{
	m_telepresenseEPInfo = new CTelepresenseEPInfo;
	if (rOtherBridgePartyMediaParams.m_telepresenseEPInfo)
		*m_telepresenseEPInfo = *rOtherBridgePartyMediaParams.m_telepresenseEPInfo;
	else
		m_telepresenseEPInfo->SetRoomID(0xFFFF);

	m_eTelePresenceMode = rOtherBridgePartyMediaParams.m_eTelePresenceMode;
}

// ------------------------------------------------------------
CBridgePartyMediaParams::~CBridgePartyMediaParams ()
{
	// PRESERVE:BEGIN
	// Insert your preservable code here...
	// PRESERVE:END
	POBJDELETE(m_telepresenseEPInfo)
}

// ------------------------------------------------------------
CBridgePartyMediaParams& CBridgePartyMediaParams::operator= (const CBridgePartyMediaParams& rOtherBridgePartyMediaParams)
{
	if ( &rOtherBridgePartyMediaParams == this ) return *this;

	// PRESERVE:BEGIN
	// Insert your preservable code here...
	if (rOtherBridgePartyMediaParams.m_telepresenseEPInfo)
		*m_telepresenseEPInfo = *rOtherBridgePartyMediaParams.m_telepresenseEPInfo;
	else
		m_telepresenseEPInfo->SetRoomID(0xFFFF);

	m_eTelePresenceMode = rOtherBridgePartyMediaParams.m_eTelePresenceMode;

	return *this;
	// PRESERVE:END
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyMediaParams::SetTelePresenceMode(eTelePresencePartyType eTelePresenceMode)
{
	if (m_telepresenseEPInfo)
		m_telepresenseEPInfo->SetEPtype(eTelePresenceMode);

	m_eTelePresenceMode = eTelePresenceMode;
}
////////////////////////////////////////////////////////////////////////////

eTelePresencePartyType CBridgePartyMediaParams::GetTelePresenceMode() const
{
	return m_telepresenseEPInfo ? m_telepresenseEPInfo->GetEPtype() : m_eTelePresenceMode;
}
////////////////////////////////////////////////////////////////////////////

void CBridgePartyMediaParams::SetTelePresenceEPInfo(CTelepresenseEPInfo* tpInfo)
{
	if (tpInfo)
		*m_telepresenseEPInfo = *tpInfo;
}
////////////////////////////////////////////////////////////////////////////

CTelepresenseEPInfo* CBridgePartyMediaParams::GetTelePresenceEPInfo() const
{
	return m_telepresenseEPInfo;
}
