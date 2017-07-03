#include <ostream>

#include "BondingMuxCntl.h"
//#include "ConfPartyManager.h"
#include "ConfPartyRoutingTable.h"
#include "DataTypes.h"
#include "Party.h"
#include "TraceStream.h"


/////////////////////////////////////////////////////////////////////////////

extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );


/////////////////////////////////////////////////////////////////////////////
CBondingMuxCntl::CBondingMuxCntl(CBondingCntl* pBnd, CMuxCntl* pMux, CRsrcParams& rsrcDesc, CParty* pParty)
{
	m_pMuxCntl = pMux;
	m_pBndCntl = pBnd;

    /* init party API */
    m_pTaskApi = new CPartyApi;
	m_pTaskApi->CreateOnlyApi(pParty->GetRcvMbx(),this);
	m_pTaskApi->SetLocalMbx(pParty->GetLocalQueue());

	/* this class will get all events for the resource/rsrcDesc since Mux and Bonding have same resource type */
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if(pRoutingTbl== NULL) {
 		PASSERT_AND_RETURN(101);
	}

	pRoutingTbl->AddStateMachinePointerToRoutingTbl(rsrcDesc, m_pTaskApi);
}

/////////////////////////////////////////////////////////////////////////////
CBondingMuxCntl::~CBondingMuxCntl()
{
	POBJDELETE(m_pTaskApi);
}

/////////////////////////////////////////////////////////////////////////////
const char* CBondingMuxCntl::NameOf() const
{
    return "CBondingMuxCntl";
}
/////////////////////////////////////////////////////////////////////////////
void CBondingMuxCntl::HandleEvent (CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
    if(m_pMuxCntl->IsMuxEvent(opCode, pMsg)) {
	    m_pMuxCntl->DispatchEvent(opCode, pMsg);
	}
	else if(m_pBndCntl->IsBondingEvent(opCode)) {
	    m_pBndCntl->DispatchEvent(opCode, pMsg);
	}
	else
	    TRACESTR(eLevelInfoNormal) << " CBondingMuxCntl::HandleEvent didn't recognize this event " << (DWORD)opCode << "\n";

}

/////////////////////////////////////////////////////////////////////////////
BOOL CBondingMuxCntl::DispatchEvent(OPCODE opCode, CSegment* pParam)
{
    if(m_pMuxCntl->IsMuxEvent(opCode, pParam)) {
	    return m_pMuxCntl->DispatchEvent(opCode, pParam);
	}
	else if(m_pBndCntl->IsBondingEvent(opCode)) {
	    return m_pBndCntl->DispatchEvent(opCode, pParam);
	}

    TRACESTR(eLevelInfoNormal) << " CBondingMuxCntl::DispatchEvent didn't recognize this event " << (DWORD)opCode << "\n";
	return FALSE;
}

