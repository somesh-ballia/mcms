#ifndef _CBridge_H_
#define _CBridge_H_

#include "StateMachine.h"
#include "BridgePartyList.h"
#include "BridgeDefs.h"

class CBridgeInitParams;
class CBridgePartyDisconnectParams;
class CBridgePartyExportParams;

////////////////////////////////////////////////////////////////////////////
//                        CBridge
////////////////////////////////////////////////////////////////////////////
class CBridge : public CStateMachineValidation
{
	CLASS_TYPE_1(CBridge, CStateMachineValidation)

public:
	                    CBridge();
	                    CBridge(const CBridge& rOtherBridge);
	virtual            ~CBridge();

	virtual const char* NameOf() const { return "CBridge";}
	CBridge&            operator= (const CBridge& rOtherBridge);

	virtual void        Create(const CBridgeInitParams* pBridgeInitParams);
	virtual void        Destroy();
	virtual  void       Disconnect();
	virtual void        Terminate();
	virtual void        HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	virtual void*       GetMessageMap();

	virtual void        ConnectParty(const CBridgePartyInitParams* pBridgePartyInitParams);
	virtual void        InitBridgeParams(const CBridgePartyInitParams* pBridgePartyInitParams);

	virtual void        DisconnectParty(const CBridgePartyDisconnectParams* pBridgePartyDisconnectParams);

	virtual void        ExportParty(const CBridgePartyExportParams* pBridgePartyExportParams);

	virtual BOOL        IsConnected() const;
	virtual BOOL        IsPartyConnected(const CTaskApp* pParty) const;
	void                ArePortsOpened(const CParty* pParty, BOOL &rIsInPortOpened, BOOL &rIsOutPortOpened) const;
	void                ArePortsOpened(PartyRsrcID partyId, BOOL &rIsInPortOpened, BOOL &rIsOutPortOpened);
	void                ArePortsOpened(PartyRsrcID partyId, std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap);

	virtual BOOL        IsStandaloneConf() const;

	WORD                GetNumParties() const;
	const char*         GetConfName() const {return m_pConfName;}

	//LEGACY
	CConf*              GetConf() const {return m_pConf;};
	CConfApi*           GetConfApi() const {return m_pConfApi;};

	CBridgePartyCntl*   GetPartyCntl(const char* name);
	CBridgePartyCntl*   GetPartyCntl(const CTaskApp*  pParty);
	CBridgePartyCntl*   GetPartyCntl(const PartyRsrcID partyId);

	BOOL                IsConnectionDirectionNeeded(WORD responseOpcode);
	BOOL                IsDisConnectionDirectionNeeded(WORD responseOpcode);

	BYTE                GetIsGateWay() {return m_pConf->GetIsGateWay();}
	CTaskApp*           GetPartyTaskApp(PartyRsrcID partyId);

	CBridgePartyList*   GetBridgePartyList() { return m_pPartyList; }

protected:
	void                ConnectParty(CBridgePartyCntl* pPartyCntl);
	void                DisconnectParty(PartyRsrcID partyId);
	void                ExportParty(PartyRsrcID partyId);

	void                EndPartyConnect(CSegment* pParam, WORD responseOpcode);
	void                EndPartyDisConnect(CSegment* pParam, WORD responseOpcode);
	void                EndPartyExport(CSegment* pParam, WORD responseOpcode);

	void                RejectPartyConnect(const CTaskApp* pParty, WORD responseOpcode, BYTE rejectReason);

	void                AllocatePartyList();
	void                DumpParties();

	virtual EBridgeImplementationTypes  GetBridgeImplementationType () { return m_eBridgeImplementationType; }

protected:
	EBridgeImplementationTypes  m_eBridgeImplementationType;
	CBridgePartyList*           m_pPartyList;
	CConfApi*                   m_pConfApi;
	CConf*                      m_pConf;
	char                        m_pConfName[H243_NAME_LEN];
	ConfRsrcID                  m_confRsrcID;
};

#endif
