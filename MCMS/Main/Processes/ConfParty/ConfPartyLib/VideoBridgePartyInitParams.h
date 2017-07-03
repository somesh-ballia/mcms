#ifndef VIDEOBRIDGEPARTYINITPARAMS_H_
#define VIDEOBRIDGEPARTYINITPARAMS_H_

#include "BridgePartyInitParams.h"

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgePartyInitParams
////////////////////////////////////////////////////////////////////////////
class CVideoBridgePartyInitParams : public CBridgePartyInitParams
{
  CLASS_TYPE_1(CVideoBridgePartyInitParams, CBridgePartyInitParams)

public:
                      CVideoBridgePartyInitParams(CBridgePartyInitParams& bridgePartyInitParams);
                      CVideoBridgePartyInitParams(const CVideoBridgePartyInitParams& other);
                      CVideoBridgePartyInitParams(const char* pPartyName, const CTaskApp* pParty, const PartyRsrcID partyRsrcID,
                                                  const WORD wNetworkInterface,
                                                  const CBridgePartyMediaParams* pMediaInParams = NULL,
                                                  const CBridgePartyMediaParams* pMediaOutParams = NULL,
                                                  const CBridgePartyCntl* pBridgePartyCntl = NULL,
                                                  const char* pSiteName = "", const BYTE bCascadeLinkMode = NONE);
  virtual            ~CVideoBridgePartyInitParams();
  virtual const char* NameOf() const { return "CVideoBridgePartyInitParams";}

  BYTE                IsLegacyParty();
};

#endif /*VIDEOBRIDGEPARTYINITPARAMS_H_*/

