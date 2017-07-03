#include "VideoRelayBridgePartyInitParams.h"
#include "PObject.h"
#include "ConfPartyGlobals.h"
#include "Bridge.h"

////////////////////////////////////////////////////////////////////////////
//                        CVideoRelayBridgePartyInitParams
////////////////////////////////////////////////////////////////////////////
CVideoRelayBridgePartyInitParams::CVideoRelayBridgePartyInitParams (CBridgePartyInitParams& bridgePartyInitParams) : CBridgePartyInitParams(bridgePartyInitParams)
{ }

////////////////////////////////////////////////////////////////////////////
CVideoRelayBridgePartyInitParams::CVideoRelayBridgePartyInitParams (const CVideoRelayBridgePartyInitParams& rOther)
  : CBridgePartyInitParams(rOther)
{ }


////////////////////////////////////////////////////////////////////////////
CVideoRelayBridgePartyInitParams::CVideoRelayBridgePartyInitParams (const char* pPartyName, const CTaskApp* pParty, const PartyRsrcID partyRsrcID,
                                                          const WORD wNetworkInterface,
                                                          const CBridgePartyMediaParams* pMediaInParams,
                                                          const CBridgePartyMediaParams* pMediaOutParams,
                                                          const CBridgePartyCntl* pBridgePartyCntl,
                                                          const char* pSiteName,
                                                          const BYTE bCascadeLinkMode)
  : CBridgePartyInitParams(pPartyName,
		  	  	  	  	   pParty,
		  	  	  	  	   partyRsrcID,
		  	  	  	  	   DUMMY_ROOM_ID,
		  	  	  	  	   wNetworkInterface,
		  	  	  	  	   pMediaInParams,
		  	  	  	  	   pMediaOutParams,
		  	  	  	  	   pBridgePartyCntl,
		  	  	  	  	   pSiteName,
		  	  	  	  	   bCascadeLinkMode,
		  	  	  	  	   true/*isVideoRelay*/)
{ }

////////////////////////////////////////////////////////////////////////////
CVideoRelayBridgePartyInitParams::~CVideoRelayBridgePartyInitParams ()
{ }

////////////////////////////////////////////////////////////////////////////

BYTE CVideoRelayBridgePartyInitParams::IsLegacyParty()
{
  BYTE isLegacyParty = NO;
  const CTaskApp* pParty = GetParty();
  if (!CPObject::IsValidPObjectPtr(pParty))
  {
    PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyInitParams::IsPartyLegacy() : Invalid Party TaskApp!!! ConfName - ", m_pConfName);
  }

  CPartyCntl* pPartyCntl = m_pConf->GetPartyCntl(pParty);

  if(pPartyCntl)
  {
    // Eitan (6/10)
    // isdn, h.323 party is legacy when remote and local has no content caps
    // sip is legacy when remote and local has no BFCP caps
    if (pPartyCntl->IsLegacyContentParty())
      isLegacyParty = YES;
  }
  else
  {
    PASSERTMSG(1, "GetPartyCntl return NULL"); 
  }

  return isLegacyParty;
}

