#ifndef FECCBRIDGEPARTYINITPARAMS_H_
#define FECCBRIDGEPARTYINITPARAMS_H_

//class CBridgePartyInitParams;
#include "BridgePartyInitParams.h"

class CFECCBridgePartyInitParams : public CBridgePartyInitParams 
{
CLASS_TYPE_1(CFECCBridgePartyInitParams,CBridgePartyInitParams)
public:

	CFECCBridgePartyInitParams ();
	
	virtual const char* NameOf() const { return "CFECCBridgePartyInitParams";}
	//constructor for partyCntl to fill, CBridge will fill all other params
	CFECCBridgePartyInitParams (const char* pPartyName, const CTaskApp* pParty,  const PartyRsrcID partyRsrcID,
							const WORD wNetworkInterface,
							const CBridgePartyMediaParams * pMediaInParams = NULL ,
							const CBridgePartyMediaParams * pMediaOutParams = NULL,
							const CBridgePartyCntl* pBridgePartyCntl = NULL );

	CFECCBridgePartyInitParams (const CFECCBridgePartyInitParams &rOtherFECCBridgePartyInitParams);
	virtual ~CFECCBridgePartyInitParams ();
	CFECCBridgePartyInitParams& operator = (const CFECCBridgePartyInitParams &rOtherFECCBridgePartyInitParams);
	virtual BOOL IsValidParams() const;	

};

#endif /*FECCBRIDGEPARTYINITPARAMS_H_*/
