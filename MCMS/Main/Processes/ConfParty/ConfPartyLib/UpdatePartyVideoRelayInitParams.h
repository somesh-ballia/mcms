
#ifndef UPDATEPARTYVIDEORELAYINITPARAMS_H_
#define UPDATEPARTYVIDEORELAYINITPARAMS_H_

#include "UpdatePartyVideoInitParams.h"
#include "BridgePartyVideoRelayMediaParams.h"

class CUpdatePartyVideoRelayInitParams : public CUpdatePartyVideoInitParams
{
CLASS_TYPE_1(CUpdatePartyVideoRelayInitParams,CUpdatePartyVideoInitParams)
public:

	CUpdatePartyVideoRelayInitParams();
	CUpdatePartyVideoRelayInitParams(const CBridgePartyInitParams& rOtherUpdatePartyInitParams);
	virtual const char* NameOf() const { return "CUpdatePartyVideoRelayInitParams";}
	virtual ~CUpdatePartyVideoRelayInitParams();

	virtual void AllocateInParams();
	virtual void AllocateOutParams();


	virtual void InitiateMediaOutParams(const CBridgePartyVideoOutParams* pBridgePartyMediaParams);//assert and do nothing need to use the relay type
	virtual void InitiateMediaInParams(const CBridgePartyVideoInParams* pBridgePartyMediaParams);//assert and do nothing need to use the relay type

	virtual void InitiateMediaOutParams(const CBridgePartyVideoRelayMediaParams* pBridgePartyMediaParams);
	virtual void InitiateMediaInParams(const CBridgePartyVideoRelayMediaParams* pBridgePartyMediaParams);


	virtual EStat UpdateVideoInParams(const CBridgePartyVideoParams* pBridgePartyVideoParams);//assert and do nothing need to use the relay type
	virtual EStat UpdateVideoOutParams(const CBridgePartyVideoParams* pBridgePartyVideoParams);//assert and do nothing need to use the relay type

	virtual EStat UpdateVideoInParams(const CBridgePartyVideoRelayMediaParams* pBridgePartyVideoParams);//assert and do nothing need to use the relay type
	virtual EStat UpdateVideoOutParams(const CBridgePartyVideoRelayMediaParams* pBridgePartyVideoParams);



protected:



};



#endif /* UPDATEPARTYVIDEORELAYINITPARAMS_H_ */
