#include "BridgeInterface.h"

////////////////////////////////////////////////////////////////////////////
//                        CBridgeInterface
////////////////////////////////////////////////////////////////////////////
CBridgeInterface::CBridgeInterface()
{
	m_pBridgeImplementation = NULL;
}

// ------------------------------------------------------------
CBridgeInterface::~CBridgeInterface()
{
	POBJDELETE(m_pBridgeImplementation);
}

// ------------------------------------------------------------
CBridgeInterface::CBridgeInterface(const CBridgeInterface& rOtherBridgeInterface)
        :CPObject(rOtherBridgeInterface)
{
	if (NULL == rOtherBridgeInterface.m_pBridgeImplementation)
		m_pBridgeImplementation = NULL;
	else
		m_pBridgeImplementation = new CBridge(*rOtherBridgeInterface.m_pBridgeImplementation);
}

// ------------------------------------------------------------
CBridgeInterface& CBridgeInterface::operator=(const CBridgeInterface& rOtherBridgeInterface)
{
	if (&rOtherBridgeInterface == this)
		return *this;

	POBJDELETE(m_pBridgeImplementation);

	if (NULL == rOtherBridgeInterface.m_pBridgeImplementation)
		m_pBridgeImplementation = NULL;
	else
		m_pBridgeImplementation = new CBridge(*rOtherBridgeInterface.m_pBridgeImplementation);

	return *this;
}

// ------------------------------------------------------------
void CBridgeInterface::Create(const CBridgeInitParams* pBridgeInitParams, const BYTE numOfBridgeImplementations)
{
	// Currently only one Bridge implementation can be created for each interface
	if (1 != numOfBridgeImplementations)
	{
		DBGPASSERT(1);
		return;
	}

	CreateImplementation(pBridgeInitParams);
}

// ------------------------------------------------------------
void CBridgeInterface::Destroy()
{
	if (m_pBridgeImplementation)
		m_pBridgeImplementation->Destroy();
}

// ------------------------------------------------------------
BOOL CBridgeInterface:: IsBridgeConnected() const
{
	if (m_pBridgeImplementation)
		return m_pBridgeImplementation->IsConnected();
	return FALSE;
}

// ------------------------------------------------------------
BOOL CBridgeInterface::IsPartyConnected(const CTaskApp* pParty) const
{
	if (m_pBridgeImplementation)
		return m_pBridgeImplementation->IsPartyConnected(pParty);
	return FALSE;
}

// ------------------------------------------------------------
void CBridgeInterface::ArePortsOpened(const CParty* pParty, BOOL &rIsInPortOpened, BOOL &rIsOutPortOpened) const
{
	rIsInPortOpened = FALSE;
	rIsOutPortOpened = FALSE;

	if (m_pBridgeImplementation)
		m_pBridgeImplementation->ArePortsOpened(pParty, rIsInPortOpened, rIsOutPortOpened);
}

// ------------------------------------------------------------
void CBridgeInterface::ArePortsOpened(PartyRsrcID partyId, BOOL &rIsInPortOpened, BOOL &rIsOutPortOpened)
{
	rIsInPortOpened = FALSE;
	rIsOutPortOpened = FALSE;

	if (m_pBridgeImplementation)
		m_pBridgeImplementation->ArePortsOpened(partyId, rIsInPortOpened, rIsOutPortOpened);
}

// ------------------------------------------------------------
void CBridgeInterface::ArePortsOpened(PartyRsrcID partyId, std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap)
{
	if (m_pBridgeImplementation)
		m_pBridgeImplementation->ArePortsOpened(partyId, isOpenedRsrcMap);
}

// ------------------------------------------------------------
WORD CBridgeInterface::GetNumParties() const
{
	if (m_pBridgeImplementation)
		return m_pBridgeImplementation->GetNumParties();
	return 0;
}

// ------------------------------------------------------------
void CBridgeInterface::Disconnect()
{
	if (m_pBridgeImplementation)
		m_pBridgeImplementation->Disconnect();
}

// ------------------------------------------------------------
void CBridgeInterface::Terminate()
{
	if (m_pBridgeImplementation)
		m_pBridgeImplementation->Terminate();
}

// ------------------------------------------------------------
void CBridgeInterface::ConnectParty(const CBridgePartyInitParams* pBridgePartyInitParams)
{
	if (m_pBridgeImplementation)
		m_pBridgeImplementation->ConnectParty(pBridgePartyInitParams);
}

// ------------------------------------------------------------
void CBridgeInterface::DisconnectParty(const CBridgePartyDisconnectParams* pBridgePartyDisconnectParams)
{
	if (m_pBridgeImplementation)
		m_pBridgeImplementation->DisconnectParty(pBridgePartyDisconnectParams);
}

// ------------------------------------------------------------
void CBridgeInterface::ExportParty (const CBridgePartyExportParams* pBridgePartyExportParams)
{
	if (m_pBridgeImplementation)
		m_pBridgeImplementation->ExportParty(pBridgePartyExportParams);
}

// ------------------------------------------------------------
CBridge* CBridgeInterface::GetBridgeImplementation()
{
	return m_pBridgeImplementation;
}

// -----------------------------------------------------------
CTaskApp* CBridgeInterface::GetPartyTaskApp(PartyRsrcID partyId)
{
	if (m_pBridgeImplementation)
		return m_pBridgeImplementation->GetPartyTaskApp(partyId);
	return NULL;
}
