//+========================================================================+
//                   BridgePartyInitParams.CPP                             |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       BridgePartyInitParams.CPP                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  June-2005  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+


#include "FECCBridgePartyInitParams.h"
#include "PObject.h"
#include "ConfPartyGlobals.h"
#include "Bridge.h"

// ------------------------------------------------------------
CFECCBridgePartyInitParams::CFECCBridgePartyInitParams ()		
{
}

// ------------------------------------------------------------
	//constructor for partyCntl to fill, CBridge will fill all other params
CFECCBridgePartyInitParams::CFECCBridgePartyInitParams (const char* pPartyName, const CTaskApp* pParty, const PartyRsrcID partyRsrcID,
												const WORD wNetworkInterface,
												const CBridgePartyMediaParams * pMediaInParams ,
												const CBridgePartyMediaParams * pMediaOutParams,
												const CBridgePartyCntl* pBridgePartyCntl) :
												CBridgePartyInitParams(pPartyName,pParty,partyRsrcID, DUMMY_ROOM_ID, wNetworkInterface,pMediaInParams,
												pMediaOutParams,pBridgePartyCntl)
{
}
// ------------------------------------------------------------
CFECCBridgePartyInitParams::CFECCBridgePartyInitParams (const CFECCBridgePartyInitParams &rOtherFECCBridgePartyInitParams)
        :CBridgePartyInitParams(rOtherFECCBridgePartyInitParams)
{
}

// ------------------------------------------------------------
CFECCBridgePartyInitParams::~CFECCBridgePartyInitParams ()
{
}

// ------------------------------------------------------------
CFECCBridgePartyInitParams& CFECCBridgePartyInitParams::operator = (const CFECCBridgePartyInitParams &rOtherFECCBridgePartyInitParams)
{
	// Operator= is not available for this class because all members are const
	return *this;
}
// ------------------------------------------------------------
BOOL CFECCBridgePartyInitParams::IsValidParams() const
{
	if ( NULL == GetPartyName() )
		return FALSE;
	if ( NULL == GetConfName() )
		return FALSE;
	if ( ! CPObject::IsValidPObjectPtr(GetParty()) )
		return FALSE;
	if ( ! CPObject::IsValidPObjectPtr(GetBridge()) )
		return FALSE;		
	if ( ! CPObject::IsValidPObjectPtr(GetConf()) )
		return FALSE;
	if ( ! ::IsValidRsrcID(m_partyRsrcID) )
		return FALSE;
	if ( ! ::IsValidRsrcID(m_confRsrcID) )
		return FALSE;
	if ( ! ::IsValidInterfaceType(m_wNetworkInterface) )
		return FALSE;	
		
	return TRUE;			
}

