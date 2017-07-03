// +========================================================================+
// BridgePartyInitParams.CPP                                                |
// Copyright 2005 Polycom, Inc                                              |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// FILE:       BridgePartyInitParams.CPP                                    |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER: Matvey                                                       |
// -------------------------------------------------------------------------|
// Who  | Date  June-2005  | Description                                    |
// -------------------------------------------------------------------------|
// +========================================================================+

#include "VideoBridgePartyInitParams.h"
#include "PObject.h"
#include "ConfPartyGlobals.h"
#include "Bridge.h"

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgePartyInitParams
////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyInitParams::CVideoBridgePartyInitParams (CBridgePartyInitParams& bridgePartyInitParams) : CBridgePartyInitParams(bridgePartyInitParams)
{ }

////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyInitParams::CVideoBridgePartyInitParams (const CVideoBridgePartyInitParams& rOther)
  : CBridgePartyInitParams(rOther)
{ }


////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyInitParams::CVideoBridgePartyInitParams (const char* pPartyName, const CTaskApp* pParty, const PartyRsrcID partyRsrcID,
                                                          const WORD wNetworkInterface,
                                                          const CBridgePartyMediaParams* pMediaInParams,
                                                          const CBridgePartyMediaParams* pMediaOutParams,
                                                          const CBridgePartyCntl* pBridgePartyCntl,
                                                          const char* pSiteName, const BYTE bCascadeLinkMode)
  : CBridgePartyInitParams(pPartyName, pParty, partyRsrcID, DUMMY_ROOM_ID, wNetworkInterface, pMediaInParams, pMediaOutParams, pBridgePartyCntl, pSiteName, bCascadeLinkMode)
{ }

////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyInitParams::~CVideoBridgePartyInitParams ()
{ }

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgePartyInitParams::IsLegacyParty()
{
  BYTE isLegacyParty = NO;
  const CTaskApp* pParty = GetParty();
  if (!CPObject::IsValidPObjectPtr(pParty))
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgePartyInitParams::IsPartyLegacy() : Invalid Party TaskApp!!! ConfName - ", m_pConfName);
    return isLegacyParty;
  }

  CPartyCntl* pPartyCntl = m_pConf->GetPartyCntl(pParty);
  // Romem  klocwork
  if (!CPObject::IsValidPObjectPtr(pPartyCntl))
  {
	  PTRACE2(eLevelInfoNormal, "CVideoBridgePartyInitParams::IsPartyLegacy() : Invalid PartyCntl!!! ConfName - ", m_pConfName);
	  return isLegacyParty;
  }
  // Eitan (6/10)
  // isdn, h.323 party is legacy when remote and local has no content caps
  // sip is legacy when remote and local has no BFCP caps
  if (pPartyCntl->IsLegacyContentParty())
    isLegacyParty = YES;

  return isLegacyParty;
}

