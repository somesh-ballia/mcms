#ifndef _CBridgeInterface_H_
#define _CBridgeInterface_H_

#include "PObject.h"
#include "Bridge.h"
#include "BridgeInitParams.h"

////////////////////////////////////////////////////////////////////////////
//                        CBridgeInterface
////////////////////////////////////////////////////////////////////////////
class CBridgeInterface : public CPObject
{
	CLASS_TYPE_1(CBridgeInterface, CPObject)

public:
	                  CBridgeInterface();
	                  CBridgeInterface(const CBridgeInterface& rOtherBridgeInterface);
	virtual          ~CBridgeInterface();

	CBridgeInterface& operator=(const CBridgeInterface& rOtherBridgeInterface);
	const char*       NameOf() const { return "CBridgeInterface"; }

	virtual void      Create(const CBridgeInitParams* pBridgeInitParams, const BYTE numOfBridgeImplementations = 1);
	virtual void      Destroy();
	virtual void      Disconnect();
	virtual void      Terminate();

	virtual void      HandleEvent(CSegment* pMsg) = 0;

	virtual void      ConnectParty(const CBridgePartyInitParams* pBridgePartyInitParams);
	virtual void      DisconnectParty(const CBridgePartyDisconnectParams* pBridgePartyDisconnectParams);

	virtual void      ExportParty(const CBridgePartyExportParams* pBridgePartyExportParams);

	BOOL              IsBridgeConnected() const;
	BOOL              IsPartyConnected(const CTaskApp* pParty) const;
	void              ArePortsOpened(const CParty* pParty, BOOL& rIsInPortOpened, BOOL& rIsOutPortOpened) const;
	void              ArePortsOpened(PartyRsrcID partyId, BOOL& rIsInPortOpened, BOOL& rIsOutPortOpened);
	void              ArePortsOpened(PartyRsrcID partyId, std::map<eLogicalResourceTypes, bool>& isOpenedRsrcMap);
	WORD              GetNumParties() const;
	CTaskApp*         GetPartyTaskApp(PartyRsrcID partyId);

protected:
	CBridge*          GetBridgeImplementation();
	virtual void      CreateImplementation(const CBridgeInitParams* pBridgeInitParams) = 0;

protected:
	CBridge*          m_pBridgeImplementation;
};

#endif
