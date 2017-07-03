#ifndef VIDEORELAYBRIDGEPARTYINITPARAMS_H_
#define VIDEORELAYBRIDGEPARTYINITPARAMS_H_

#include "BridgePartyInitParams.h"

////////////////////////////////////////////////////////////////////////////
//                        CVideoRelayBridgePartyInitParams
////////////////////////////////////////////////////////////////////////////
class CVideoRelayBridgePartyInitParams : public CBridgePartyInitParams
{
  CLASS_TYPE_1(CVideoRelayBridgePartyInitParams, CBridgePartyInitParams)

public:
                      CVideoRelayBridgePartyInitParams(CBridgePartyInitParams& bridgePartyInitParams);
                      CVideoRelayBridgePartyInitParams(const CVideoRelayBridgePartyInitParams& other);
                      CVideoRelayBridgePartyInitParams(const char* pPartyName,
                    		  	  	  	  	  	  	   const CTaskApp* pParty,
                    		  	  	  	  	  	  	   const PartyRsrcID partyRsrcID,
                    		  	  	  	  	  	  	   const WORD wNetworkInterface,
                    		  	  	  	  	  	  	   const CBridgePartyMediaParams* pMediaInParams = NULL,
                    		  	  	  	  	  	  	   const CBridgePartyMediaParams* pMediaOutParams = NULL,
                    		  	  	  	  	  	  	   const CBridgePartyCntl* pBridgePartyCntl = NULL,
                    		  	  	  	  	  	  	   const char* pSiteName = "",
                    		  	  	  	  	  	  	   const BYTE bCascadeLinkMode = NONE);
  virtual            ~CVideoRelayBridgePartyInitParams();
  virtual const char* NameOf() const { return "CVideoRelayBridgePartyInitParams";}

  BYTE                IsLegacyParty();
};

#endif /*VIDEORELAYBRIDGEPARTYINITPARAMS_H_*/

